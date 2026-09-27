<div align="center">
<img src="https://img.shields.io/badge/ESP32-ESP--IDF-red?style=for-the-badge&logo=espressif&logoColor=white"/>
<img src="https://img.shields.io/badge/Language-C-blue?style=for-the-badge&logo=c&logoColor=white"/>
<img src="https://img.shields.io/badge/Driver-GPIO%20Fast--Path-orange?style=for-the-badge"/>
<img src="https://img.shields.io/badge/RTOS-FreeRTOS-green?style=for-the-badge"/>
<img src="https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge"/>

<br/><br/>

<h1>⚡ ESP32 Fast GPIO Driver</h1>
<h3>Register-level GPIO driver for ESP32 — zero lock overhead, IRAM-resident, atomic batch ops</h3>
<p>
  Bypasses the ESP-IDF <code>gpio_set_level()</code> critical section entirely.<br/>
  Single-pin and multi-pin operations compiled to <strong>one or two register writes</strong>.
</p>
</div>

---

## 📖 Table of Contents

- [Overview](#overview)
- [Why Not gpio_set_level?](#why-not-gpio_set_level)
- [Files](#files)
- [API Reference](#api-reference)
- [Usage Examples](#usage-examples)
- [Throughput Benchmark](#throughput-benchmark)
- [Pin Constraints](#pin-constraints)
- [Concepts Demonstrated](#concepts-demonstrated)

---

<h2 id="overview">🧭 Overview</h2>

This project implements a **register-level fast GPIO driver** for the ESP32, using the ESP-IDF framework in C.

Instead of calling `gpio_set_level()` — which enters a spinlock critical section on every call — this driver writes directly to the ESP32's **W1TS (Write-1-To-Set)** and **W1TC (Write-1-To-Clear)** atomic GPIO output registers via `REG_WRITE`. The functions are marked `IRAM_ATTR` so they execute from IRAM and are safe during flash cache misses.

**Architecture:**

```
+----------------------------------+
|      Application (main.c)        |
+----------------------------------+
|  Fast GPIO Driver (gpio.h/.c)    |  <- This project
|  IRAM_ATTR inline functions      |
|  Direct REG_WRITE / REG_READ     |
+----------------------------------+
|      ESP32 GPIO Hardware         |
|   W1TS / W1TC atomic registers   |
+----------------------------------+
```

---

<h2 id="why-not-gpio_set_level">⚡ Why Not gpio_set_level()?</h2>

`gpio_set_level()` is safe and general-purpose, but has overhead that matters in tight loops:

| | `gpio_set_level()` | `gpio_fast_set()` |
|---|---|---|
| Critical section | Yes (portENTER_CRITICAL) | None |
| Validation on every call | Yes | None (done once at init) |
| Flash cache safe | May stall | IRAM_ATTR |
| Batch N pins | N calls, N locks | 1 register write |
| Typical throughput | ~1–2 MHz toggle | ~10–20 MHz toggle |

---

<h2 id="files">📁 Files</h2>

```
Custom-Embedded-Driver/
├── gpio.h          <- Full API — all inlined hot-path functions (IRAM_ATTR)
├── gpio.c          <- One-time config: validation, GPIO matrix routing, pull config
├── main.c          <- Usage demo: set, clear, toggle, batch ops, benchmark
└── peripheral.h    <- STM32 bare-metal register map (separate reference — NOT ESP32)
```

> **Note:** `peripheral.h` contains STM32 Cortex-M base addresses (`0x40020000` etc.) for reference. It is not used by the ESP32 driver — see the file header for details.

---

<h2 id="api-reference">📚 API Reference</h2>

### Setup (call once at boot — not latency-critical)

```c
// Configure one pin: direction, pull resistors, GPIO matrix routing
esp_err_t gpio_fast_config(uint8_t gpio_num, gpio_fast_mode_t mode);

// Configure multiple pins sharing the same mode in one call
esp_err_t gpio_fast_config_mask(uint64_t pin_mask, gpio_fast_mode_t mode);
```

**Modes:**

| `gpio_fast_mode_t` | Description |
|---|---|
| `GPIO_FAST_MODE_INPUT` | Floating input |
| `GPIO_FAST_MODE_INPUT_PULLUP` | Input with internal pull-up |
| `GPIO_FAST_MODE_INPUT_PULLDOWN` | Input with internal pull-down |
| `GPIO_FAST_MODE_OUTPUT` | Push-pull output |

---

### Single-Pin Hot Path (bank 0 and bank 1)

```c
void  gpio_fast_set(uint8_t gpio_num);              // Drive HIGH — 1 register write
void  gpio_fast_clear(uint8_t gpio_num);            // Drive LOW  — 1 register write
void  gpio_fast_write(uint8_t gpio_num, bool lvl);  // Set or clear based on bool
void  gpio_fast_toggle(uint8_t gpio_num);           // Flip current state — 1 read + 1 write
bool  gpio_fast_read(uint8_t gpio_num);             // Read instantaneous input level
```

---

### Batch Hot Path — Bank 0 (pins 0-31)

All N pins toggled for the cost of **1 or 2 register writes** — no loop, no glitch window.

```c
void     gpio_fast_set_mask(uint32_t mask);
void     gpio_fast_clear_mask(uint32_t mask);
void     gpio_fast_write_mask(uint32_t set_mask, uint32_t clr_mask);
void     gpio_fast_toggle_mask(uint32_t mask);       // NEW: toggle all pins in mask
uint32_t gpio_fast_read_bank0(void);                 // Read all 32 pins at once
```

---

### Batch Hot Path — Bank 1 (pins 32-39)

Mirror API for the upper 8 GPIO pins. Mask bits are **right-aligned**: bit 0 = GPIO32, bit 7 = GPIO39.

```c
void     gpio_fast_set_mask1(uint32_t mask);         // NEW
void     gpio_fast_clear_mask1(uint32_t mask);       // NEW
void     gpio_fast_write_mask1(uint32_t set_mask, uint32_t clr_mask); // NEW
void     gpio_fast_toggle_mask1(uint32_t mask);      // NEW
uint32_t gpio_fast_read_bank1(void);                 // Bits 0-7 = GPIO32-39
```

---

<h2 id="usage-examples">💡 Usage Examples</h2>

```c
#include "gpio.h"

#define LED_A   4
#define LED_B   5
#define LED_C   18
#define BUTTON  19
#define LED_MASK ((1UL << LED_A) | (1UL << LED_B) | (1UL << LED_C))

void app_main(void)
{
    // One-time setup
    gpio_fast_config_mask(LED_MASK, GPIO_FAST_MODE_OUTPUT);
    gpio_fast_config(BUTTON, GPIO_FAST_MODE_INPUT_PULLUP);

    // Single-pin ops
    gpio_fast_set(LED_A);
    gpio_fast_clear(LED_A);
    gpio_fast_toggle(LED_A);          // flip without knowing current state

    // Batch: all 3 LEDs on in ONE register write
    gpio_fast_set_mask(LED_MASK);
    gpio_fast_toggle_mask(LED_MASK);  // all off in two register writes
    gpio_fast_toggle_mask(LED_MASK);  // all on again

    // Mixed: A and C high, B low — 2 register writes total
    gpio_fast_write_mask((1UL << LED_A) | (1UL << LED_C), (1UL << LED_B));

    // Read input
    if (!gpio_fast_read(BUTTON)) {    // active-low with pull-up
        // button pressed
    }

    // Read all bank 0 pins at once
    uint32_t bank = gpio_fast_read_bank0();
    bool led_a_state = (bank >> LED_A) & 1;
}
```

---

<h2 id="throughput-benchmark">📊 Throughput Benchmark</h2>

Run `main.c` to measure toggle throughput. Typical results on ESP32 at 240 MHz:

```
I (312) example: 100000 toggles in 9800 us (98.0 ns/toggle)   <- gpio_fast_set + clear
I (320) example: 100000 gpio_fast_toggle calls in 12100 us (121.0 ns/call)
```

Compare against `gpio_set_level()` which typically measures **400–600 ns/call** due to critical section overhead — a **4–6x speedup**.

---

<h2 id="pin-constraints">⚠️ Pin Constraints</h2>

| Range | Constraint |
|---|---|
| GPIO 6–11 | Reserved for SPI flash — driver **refuses** to configure these |
| GPIO 34–39 | Input-only (no output driver, no pull resistors) |
| GPIO 0–31 | Bank 0 — full API available |
| GPIO 32–39 | Bank 1 — batch API available (8 pins, right-aligned mask) |

---

<h2 id="concepts-demonstrated">🎓 Concepts Demonstrated</h2>

| Concept | Where |
|---|---|
| Direct memory-mapped register access (`REG_WRITE` / `REG_READ`) | `gpio.h` |
| Atomic W1TS / W1TC GPIO registers (no read-modify-write) | `gpio.h` |
| `IRAM_ATTR` for flash-cache-safe ISR / hot-path code | All inline functions |
| `static inline` for zero-overhead function calls | `gpio.h` |
| `esp_rom_gpio_pad_select_gpio` for GPIO matrix routing | `gpio.c` |
| Input validation + `ESP_LOGE` error reporting | `gpio.c` |
| Fixed-width types (`uint8_t`, `uint32_t`, `uint64_t`) | Throughout |
| Bit manipulation: set, clear, mask, XOR/toggle patterns | `gpio.h` |
| Two-bank GPIO architecture (ESP32 bank 0 / bank 1) | `gpio.h` |
| FreeRTOS `vTaskDelay` + `esp_timer_get_time` benchmarking | `main.c` |
| `#pragma once` vs `#ifndef` include guards | `gpio.h` vs `peripheral.h` |

---

<img src="https://img.shields.io/badge/Built%20with-ESP--IDF-red?style=flat-square&logo=espressif"/>
<img src="https://img.shields.io/badge/Language-C99-blue?style=flat-square&logo=c"/>
<img src="https://img.shields.io/badge/RTOS-FreeRTOS-green?style=flat-square"/>
<img src="https://img.shields.io/badge/GPIO-Register--Level-orange?style=flat-square"/>

<br/><br/>
<p style="color:#B4B2A9; font-size:12px;">ESP32 · ESP-IDF · C · Fast GPIO · Register-Level · Custom Embedded Driver</p>