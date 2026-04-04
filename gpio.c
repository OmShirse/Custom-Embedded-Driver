#include "gpio.h"

void gpio_init_output(int pin) {
    // Each pin uses 2 bits in MODER
    GPIOA_MODER &= ~(0x3 << (pin * 2));   // Clear bits
    GPIOA_MODER |=  (0x1 << (pin * 2));   // Set as output
}

void gpio_set_pin(int pin) {
    GPIOA_ODR |= (1 << pin);
}

void gpio_clear_pin(int pin) {
    GPIOA_ODR &= ~(1 << pin);
}