#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

// Base address (example)
#define GPIOA_BASE 0x40020000

// Register offsets
#define GPIO_MODER_OFFSET 0x00
#define GPIO_ODR_OFFSET   0x14

// Register access macros
#define GPIOA_MODER (*(volatile uint32_t*)(GPIOA_BASE + GPIO_MODER_OFFSET))
#define GPIOA_ODR   (*(volatile uint32_t*)(GPIOA_BASE + GPIO_ODR_OFFSET))

// Function prototypes
void gpio_init_output(int pin);
void gpio_set_pin(int pin);
void gpio_clear_pin(int pin);

#endif