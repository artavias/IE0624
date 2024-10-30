#include <libopencm3/cm3/common.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

// USART Configuration Constants
#define USART_PORT USART1
#define USART_GPIO_PORT GPIOA
#define USART_TX_PIN GPIO9
#define USART_RX_PIN GPIO10
#define SWITCH_PORT GPIOC
#define SWITCH_PIN GPIO0

void clock_setup(void);
void usart_setup(void);
void gpio_setup(void);
void send_gyroscope_data(void);
void enable_usart(void);
void disable_usart(void);


int main(void) {
    clock_setup();
    gpio_setup();
    usart_setup();

    bool usart_enabled = false;

    while (1) {
        if (gpio_get(SWITCH_PORT, SWITCH_PIN)) {
            if (!usart_enabled) {
                enable_usart();
                usart_enabled = true;
            }
            send_gyroscope_data();
        } else {
            if (usart_enabled) {
                disable_usart();
                usart_enabled = false;
            }
        }
    }

    return 0;
}

// Setup system clock
void clock_setup(void) {
    rcc_clock_setup_pll(&rcc_3v3[RCC_CLOCK_3V3_168MHZ]);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_USART1);
}

// Setup GPIO for USART and switch
void gpio_setup(void) {
    // Configure USART TX pin as alternate function
    gpio_mode_setup(USART_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART_TX_PIN);
    gpio_set_af(USART_GPIO_PORT, GPIO_AF7, USART_TX_PIN);

    // Configure switch pin as input
    gpio_mode_setup(SWITCH_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SWITCH_PIN);
}

// Setup USART for communication
void usart_setup(void) {
    usart_set_baudrate(USART_PORT, 115200);
    usart_set_databits(USART_PORT, 8);
    usart_set_stopbits(USART_PORT, USART_STOPBITS_1);
    usart_set_mode(USART_PORT, USART_MODE_TX);
    usart_set_parity(USART_PORT, USART_PARITY_NONE);
    usart_set_flow_control(USART_PORT, USART_FLOWCONTROL_NONE);
}

// Enable USART
void enable_usart(void) {
    usart_enable(USART_PORT);
}

// Disable USART
void disable_usart(void) {
    usart_disable(USART_PORT);
}

// Simulate gyroscope data sending over USART
void send_gyroscope_data(void) {
    int16_t gyro_x = 123;  // Placeholder data
    int16_t gyro_y = 456;  // Placeholder data
    int16_t gyro_z = 789;  // Placeholder data

    char buffer[64];
    int len = snprintf(buffer, sizeof(buffer), "X: %d, Y: %d, Z: %d\r\n", gyro_x, gyro_y, gyro_z);

    for (int i = 0; i < len; i++) {
        usart_send_blocking(USART_PORT, buffer[i]);
    }
}
