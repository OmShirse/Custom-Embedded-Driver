#include <stdint.h>
// ==================== Peripheral Base Addresses ====================
#define GPIOA_BASE  0x40020000 // for Accessing GPIOA | Pre-Defined
#define GPIOB_BASE  0x40020400 // for Accessing GPIOB | Pre-Defined
#define USART1_BASE 0x40011000 // for Accessing USART | Pre-Defined
#define SPI1_BASE   0x40013000 // for Accessing SPI   | Pre-Defined 
#define I2C1_BASE   0x40005400 // for Accessing I2C   | Pre-Defined
// ==================== GPIO Offsets ====================
#define GPIO_MODER_OFFSET 0x00 // Pre-Defined
#define GPIO_ODR_OFFSET   0x14 // Pre-Defined
#define GPIO_IDR_OFFSET   0x10 // Pre-Defined
#define GPIOA_MODER (*(volatile uint32_t*)(GPIOA_BASE + GPIO_MODER_OFFSET))//For Convenience
#define GPIOA_ODR   (*(volatile uint32_t*)(GPIOA_BASE + GPIO_ODR_OFFSET))//For Convenience
#define GPIOA_IDR   (*(volatile uint32_t*)(GPIOA_BASE + GPIO_IDR_OFFSET))//For Convenience

// ==================== USART Offsets ====================
#define USART_SR_OFFSET   0x00
#define USART_DR_OFFSET   0x04
#define USART_CR1_OFFSET  0x0C

#define USART1_SR  (*(volatile uint32_t*)(USART1_BASE + USART_SR_OFFSET))
#define USART1_DR  (*(volatile uint32_t*)(USART1_BASE + USART_DR_OFFSET))
#define USART1_CR1 (*(volatile uint32_t*)(USART1_BASE + USART_CR1_OFFSET))

// ==================== SPI Offsets ====================
#define SPI_CR1_OFFSET 0x00
#define SPI_DR_OFFSET  0x0C

#define SPI1_CR1 (*(volatile uint32_t*)(SPI1_BASE + SPI_CR1_OFFSET))
#define SPI1_DR  (*(volatile uint32_t*)(SPI1_BASE + SPI_DR_OFFSET))

// ==================== I2C Offsets ====================
#define I2C_CR1_OFFSET 0x00
#define I2C_DR_OFFSET  0x10

#define I2C1_CR1 (*(volatile uint32_t*)(I2C1_BASE + I2C_CR1_OFFSET))
#define I2C1_DR  (*(volatile uint32_t*)(I2C1_BASE + I2C_DR_OFFSET))

// ==================== Utility Macros ====================
#define PIN5 5 // Create a Named Constant for PIN 5

// Set GPIO pin high
static inline void gpio_set_pin(volatile uint32_t *odr, int pin) {
    *odr |= (1 << pin); 
}

// Set GPIO pin low
static inline void gpio_clear_pin(volatile uint32_t *odr, int pin) {
    *odr &= ~(1 << pin); 
}

// Read GPIO pin
static inline int gpio_read_pin(volatile uint32_t *idr, int pin) {
    return (*idr & (1 << pin)) >> pin;
}

// Send USART byte
static inline void usart_send_byte(volatile uint32_t *dr, uint8_t data) {
    *dr = data; // dr = data register
}

// Send SPI byte
static inline void spi_send_byte(volatile uint32_t *dr, uint8_t data) {
    *dr = data;
}

// Send I2C byte
static inline void i2c_send_byte(volatile uint32_t *dr, uint8_t data) {
    *dr = data;
}

// ==================== Main Program ====================
int main(void) {

    // --- GPIO Example ---
    // Set PA5 as output
    GPIOA_MODER &= ~(0x3 << (PIN5 * 2));
    GPIOA_MODER |=  (0x1 << (PIN5 * 2)); // 01 = output
    gpio_set_pin(&GPIOA_ODR, PIN5);      // Turn PA5 HIGH

    // --- USART Example ---
    USART1_CR1 |= (1 << 13);   // Enable USART
    usart_send_byte(&USART1_DR, 'A');  // Send character 'A'

    // --- SPI Example ---
    SPI1_CR1 |= (1 << 6);      // Enable SPI
    spi_send_byte(&SPI1_DR, 0xAB);     // Send byte over SPI

    // --- I2C Example ---
    I2C1_CR1 |= (1 << 0);      // Enable I2C
    i2c_send_byte(&I2C1_DR, 0x55);     // Send byte over I2C

    // --- Read GPIO Input ---
    int pin_state = gpio_read_pin(&GPIOA_IDR, PIN5);

    while(1) {
        // Infinite loop
    }

    return 0;
}