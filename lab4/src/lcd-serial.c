#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/adc.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "clock.h"
#include "console.h"
#include "sdram.h"
#include "lcd-spi.h"
#include "gfx.h"


uint16_t read_reg(int reg);
void write_reg(uint8_t reg, uint8_t value);
static uint16_t read_adc_naiive(uint8_t channel);

void write_reg(uint8_t reg, uint8_t value){
	gpio_clear(GPIOC, GPIO1); /* CS* select */
	spi_send(SPI5, reg);
	(void) spi_read(SPI5);
	spi_send(SPI5, value);
	(void) spi_read(SPI5);
	gpio_set(GPIOC, GPIO1); /* CS* deselect */
	return;
}

uint16_t read_reg(int reg){
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

static uint16_t read_adc_naiive(uint8_t channel){
	uint8_t channel_array[16];
	channel_array[0] = channel;
	adc_set_regular_sequence(ADC1, 1, channel_array);
	adc_start_conversion_regular(ADC1);
	while (!adc_eoc(ADC1));
	uint16_t reg16 = adc_read_regular(ADC1);
	return reg16;
}


int main(void)
{
	int valor_X=0;
	int valor_Y=15;
	int valor_Z=-2;
	int bateria=0;
	int counter=0;
	int counter2=0;
	char str_X[12];
	char str_Y[12];
	char str_Z[12];
	char str_adc[12];
	uint16_t input_adc0;

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

	// Configurar gpio para led
	rcc_periph_clock_enable(RCC_GPIOG);
	gpio_mode_setup(GPIOG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
	// Configurar gpio para botón
	rcc_periph_clock_enable(RCC_GPIOA);
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO0);
	
	//Configurar led para notificar bateria baja
	gpio_mode_setup(GPIOG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO14);

	// Configurar puerto ADC
	rcc_periph_clock_enable(RCC_ADC1);
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);
	adc_power_off(ADC1);
	adc_disable_scan_mode(ADC1);
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_3CYC);
	adc_power_on(ADC1);


	while (1) {

		// Leer botón para ver si hay que activar la comunicación USART
		if(gpio_get(GPIOA, GPIO0)){
			gpio_toggle(GPIOG, GPIO13);
		}
	
		// Leer registros de X, Y y Z
		// Estas lecturas se comentan porque parece que en la tarjeta no funciona el giroscopio
		// así que mejor se simulan estos datos
		//valor_X = read_reg(0x28);
		//valor_Y = read_reg(0x2A);
		//valor_Z = read_reg(0x2C);

		//Se agrega un contador a modo de delay
		counter++;
		if (counter==10){
			valor_X++;
			valor_Y++;
			valor_Z++;
			if (valor_X>=20){
				valor_X=-15;
				valor_Y=-15;
				valor_Z=-15;
			}
			counter=0;
		}

		// Convertir a String
		sprintf(str_X, "%d", valor_X);
		sprintf(str_Y, "%d", valor_Y);
		sprintf(str_Z, "%d", valor_Z);

		//Leer datos de adc
		bateria = read_adc_naiive(1)*9000/4095;
		sprintf(str_adc, "%u", bateria);

		//Encender led rojo si bateria < 7500
		if (bateria < 7500){
			if (counter2==4){
				gpio_toggle(GPIOG, GPIO14);
				counter2=0;
			}
		} 
		if (bateria>7500){
			gpio_clear(GPIOG, GPIO14);
		}
		counter2++;

		gfx_fillScreen(LCD_BLACK);
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

		gfx_setCursor(15, 160);
		gfx_puts("Bat.: ");
		gfx_puts(str_adc);

		lcd_show_frame();

		// Enviar datos a pc
		if (gpio_get(GPIOG, GPIO13)){
			console_puts("Valor X: ");
			console_puts(str_X);
			console_puts("\n");
			
			console_puts("Valor Y: ");
			console_puts(str_Y);
			console_puts("\n");
		
			console_puts("Valor Z: ");
			console_puts(str_Z);
			console_puts("\n");

			console_puts("Bateria: ");
			console_puts(str_adc);
			console_puts("\n");
		}

	}
}


