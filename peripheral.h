#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include <stdint.h>

// ==================== Base Addresses ====================
#define GPIOA_BASE  0x40020000
#define GPIOB_BASE  0x40020400
#define USART1_BASE 0x40011000
#define SPI1_BASE   0x40013000
#define I2C1_BASE   0x40005400

// ==================== GPIO Offsets ====================
#define GPIO_MODER_OFFSET 0x00  // Mode register
#define GPIO_ODR_OFFSET   0x14  // Output data register
#define GPIO_IDR_OFFSET   0x10  // Input data register

// ==================== USART Offsets ====================
#define USART_SR_OFFSET   0x00  // Status register
#define USART_DR_OFFSET   0x04  // Data register
#define USART_CR1_OFFSET  0x0C  // Control register

// ==================== SPI Offsets ====================
#define SPI_CR1_OFFSET 0x00  // Control register 1
#define SPI_DR_OFFSET  0x0C  // Data register

// ==================== I2C Offsets ====================
#define I2C_CR1_OFFSET 0x00  // Control register 1
#define I2C_DR_OFFSET  0x10  // Data register

// ==================== GPIO Registers ====================
#define GPIOA_MODER (*(volatile uint32_t*)(GPIOA_BASE + GPIO_MODER_OFFSET))
#define GPIOA_ODR   (*(volatile uint32_t*)(GPIOA_BASE + GPIO_ODR_OFFSET))
#define GPIOA_IDR   (*(volatile uint32_t*)(GPIOA_BASE + GPIO_IDR_OFFSET))

#define GPIOB_MODER (*(volatile uint32_t*)(GPIOB_BASE + GPIO_MODER_OFFSET))
#define GPIOB_ODR   (*(volatile uint32_t*)(GPIOB_BASE + GPIO_ODR_OFFSET))
#define GPIOB_IDR   (*(volatile uint32_t*)(GPIOB_BASE + GPIO_IDR_OFFSET))

// ==================== USART Registers ====================
#define USART1_SR  (*(volatile uint32_t*)(USART1_BASE + USART_SR_OFFSET))
#define USART1_DR  (*(volatile uint32_t*)(USART1_BASE + USART_DR_OFFSET))
#define USART1_CR1 (*(volatile uint32_t*)(USART1_BASE + USART_CR1_OFFSET))

// ==================== SPI Registers ====================
#define SPI1_CR1 (*(volatile uint32_t*)(SPI1_BASE + SPI_CR1_OFFSET))
#define SPI1_DR  (*(volatile uint32_t*)(SPI1_BASE + SPI_DR_OFFSET))

// ==================== I2C Registers ====================
#define I2C1_CR1 (*(volatile uint32_t*)(I2C1_BASE + I2C_CR1_OFFSET))
#define I2C1_DR  (*(volatile uint32_t*)(I2C1_BASE + I2C_DR_OFFSET))

// ==================== Pin Macros ====================
#define PIN0 0
#define PIN1 1
#define PIN2 2
#define PIN3 3
#define PIN4 4
#define PIN5 5
#define PIN6 6
#define PIN7 7
#define PIN8 8
#define PIN9 9
#define PIN10 10
#define PIN11 11
#define PIN12 12
#define PIN13 13
#define PIN14 14
#define PIN15 15

// ==================== GPIO Functions ====================
static inline void gpio_set_pin(volatile uint32_t *odr, int pin) {
    *odr |= (1 << pin);
}

static inline void gpio_clear_pin(volatile uint32_t *odr, int pin) {
    *odr &= ~(1 << pin);
}

static inline int gpio_read_pin(volatile uint32_t *idr, int pin) {
    return (*idr & (1 << pin)) >> pin;
}

static inline void gpio_set_mode(volatile uint32_t *moder, int pin, int mode) {
    *moder &= ~(0x3 << (pin * 2));    // Clear current mode
    *moder |= ((mode & 0x3) << (pin * 2)); // Set new mode (0=Input, 1=Output, 2=AF, 3=Analog)
}

// ==================== USART Functions ====================
static inline void usart_send_byte(volatile uint32_t *dr, uint8_t data) {
    *dr = data;
}

// ==================== SPI Functions ====================
static inline void spi_send_byte(volatile uint32_t *dr, uint8_t data) {
    *dr = data;
}

// ==================== I2C Functions ====================
static inline void i2c_send_byte(volatile uint32_t *dr, uint8_t data) {
    *dr = data;
}

#endif // PERIPHERALS_H