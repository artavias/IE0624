
///*
// * This file is part of the libopencm3 project.
// *
// * Copyright (C) 2014 Chuck McManis <cmcmanis@mcmanis.com>
// *
// * This library is free software: you can redistribute it and/or modify
// * it under the terms of the GNU Lesser General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This library is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// * GNU Lesser General Public License for more details.
// *
// * You should have received a copy of the GNU Lesser General Public License
// * along with this library.  If not, see <http://www.gnu.org/licenses/>.
// */
//
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "clock.h"
#include "console.h"
#include "sdram.h"
#include "lcd-spi.h"
#include "gfx.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

/* Convert degrees to radians */
#define d2r(d) ((d) * 6.2831853 / 360.0)

uint16_t read_reg(int reg);
void write_reg(uint8_t reg, uint8_t value);

void write_reg(uint8_t reg, uint8_t value)
{

	gpio_clear(GPIOC, GPIO1); /* CS* select */
	spi_send(SPI5, reg);
	(void) spi_read(SPI5);
	spi_send(SPI5, value);
	(void) spi_read(SPI5);
	gpio_set(GPIOC, GPIO1); /* CS* deselect */

	return;
}

uint16_t read_reg(int reg)
{
	uint16_t d1, d2;
	d1 = 0x80 | (reg & 0x3f); /* Read operation */
	/* Nominallly a register read is a 16 bit operation */
	gpio_clear(GPIOC, GPIO1);
	spi_send(SPI5, d1);
	d2 = spi_read(SPI5);
	d2 <<= 8;
	/*
	 * You have to send as many bits as you want to read
	 * so we send another 8 bits to get the rest of the
	 * register.
	 */
	spi_send(SPI5, 0);
	d2 |= spi_read(SPI5);
	gpio_set(GPIOC, GPIO1);
	return d2;
}



/*
 * This is our example, the heavy lifing is actually in lcd-spi.c but
 * this drives that code.
 */
int main(void)
{
	uint16_t valor_X;
	uint16_t valor_Y;
	uint16_t valor_Z;
	char str_X[8];
	char str_Y[8];
	char str_Z[8];

	// Iniciar varios
	clock_setup();
	console_setup(115200);

	//Pines de Giroscopio
	// CS   = GPIOC -> GPIO1
	// SCLK = GPIOF -> GPIO7
	// MISO = GPIOF -> GPIO8
	// MOSI = GPIOF -> GPIO9

	// Configurar pines para Giroscopio
	rcc_periph_clock_enable(RCC_GPIOC | RCC_GPIOF);
	rcc_periph_clock_enable(RCC_SPI5);
	gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7 | GPIO8 | GPIO9);
	gpio_set_af(GPIOF, GPIO_AF5, GPIO7 | GPIO8 | GPIO9);
	gpio_set_output_options(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ,
				GPIO7 | GPIO9);
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
	gpio_set(GPIOC, GPIO1);

	// Configurar SPI5 para giroscopio
	spi_set_master_mode(SPI5);
	spi_set_baudrate_prescaler(SPI5, SPI_CR1_BAUDRATE_FPCLK_DIV_8);
    spi_set_clock_polarity_0(SPI5);
	spi_set_clock_phase_0(SPI5);
    spi_set_full_duplex_mode(SPI5); 
	spi_set_unidirectional_mode(SPI5);
    spi_enable_software_slave_management(SPI5);
    spi_send_msb_first(SPI5);
	spi_set_nss_high(SPI5);
	spi_enable(SPI5);
	spi_set_dff_8bit(SPI5);
	spi_enable_ss_output(SPI5);

	// Configurar registros de giroscopio
	write_reg(0x20, 0xcf);  
	write_reg(0x21, 0x07);  
	write_reg(0x23, 0xb0);

	//Iniciar display
	sdram_init();
	lcd_spi_init();
	gfx_init(lcd_draw_pixel, 240, 320);

	//  Configurar texto en pantalla
	gfx_setTextColor(LCD_GREEN, LCD_BLACK);
	gfx_setTextSize(2);
	gfx_fillScreen(LCD_BLACK);

	while (1) {
	
		// Leer registros de X, Y y Z
		valor_X = read_reg(0x28);
		valor_Y = read_reg(0x2A);
		valor_Z = read_reg(0x2C);

		// Convertir a String
		sprintf(str_X, "%u", valor_X);
		sprintf(str_Y, "%u", valor_Y);
		sprintf(str_Z, "%u", valor_Z);

		// Escribir en pantalla
		gfx_setCursor(15, 40);
		gfx_puts("X: ");
		gfx_puts(str_X);
		
		gfx_setCursor(15, 80);
		gfx_puts("Y: ");
		gfx_puts(str_Y);

		gfx_setCursor(15, 120);
		gfx_puts("Z: ");
		gfx_puts(str_Z);

		lcd_show_frame();
	}
}
