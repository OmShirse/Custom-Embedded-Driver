<div align="center"> <img src="https://img.shields.io/badge/ESP32-ESP--IDF-red?style=for-the-badge&logo=espressif&logoColor=white"/> <img src="https://img.shields.io/badge/Language-C-blue?style=for-the-badge&logo=c&logoColor=white"/> <img src="https://img.shields.io/badge/Protocol-I2C-orange?style=for-the-badge"/> <img src="https://img.shields.io/badge/RTOS-FreeRTOS-green?style=for-the-badge"/> <img src="https://img.shields.io/badge/Sensor-MPU6050-purple?style=for-the-badge"/> <img src="https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge"/>

<br/><br/>

<h1>🔧 ESP32 Custom IMU Driver</h1> <h3>Register-level I2C driver for MPU6050 — built from scratch using ESP-IDF</h3> <p> No third-party sensor library used. Every register accessed manually.<br/> Raw → filtered → RTOS-managed sensor pipeline in pure C. </p> </div>

---

## 📖 Table of Contents

- [Overview](https://claude.ai/chat/9c150638-ea78-47c0-b997-e2786b086739#overview)
- [Features](https://claude.ai/chat/9c150638-ea78-47c0-b997-e2786b086739#features)
- [Hardware Required](https://claude.ai/chat/9c150638-ea78-47c0-b997-e2786b086739#hardware-required)
- [Wiring Diagram](https://claude.ai/chat/9c150638-ea78-47c0-b997-e2786b086739#wiring-diagram)
- [Project Structure](https://claude.ai/chat/9c150638-ea78-47c0-b997-e2786b086739#project-structure)
- [Driver Architecture](https://claude.ai/chat/9c150638-ea78-47c0-b997-e2786b086739#driver-architecture)
- [Getting Started](https://claude.ai/chat/9c150638-ea78-47c0-b997-e2786b086739#getting-started)
- [API Reference](https://claude.ai/chat/9c150638-ea78-47c0-b997-e2786b086739#api-reference)
- [Serial Output](https://claude.ai/chat/9c150638-ea78-47c0-b997-e2786b086739#serial-output)
- [FreeRTOS Task Pipeline](https://claude.ai/chat/9c150638-ea78-47c0-b997-e2786b086739#freertos-task-pipeline)
- [Register Map](https://claude.ai/chat/9c150638-ea78-47c0-b997-e2786b086739#register-map)
- [Concepts Demonstrated](https://claude.ai/chat/9c150638-ea78-47c0-b997-e2786b086739#concepts-demonstrated)

---

<h2 id="overview"> <span style="color:#534AB7;">🧭 Overview</span> </h2>

This project implements a **complete embedded driver** for the MPU6050 6-axis IMU (accelerometer + gyroscope) on an **ESP32 microcontroller**, using the **ESP-IDF framework** in C.

The driver is written entirely from the register level — no Adafruit, no Arduino libraries, no abstraction layers hiding the hardware. Every I2C transaction, every register bit, every byte combination is written and understood by hand.

The project demonstrates a full production-quality embedded firmware architecture:

```
┌──────────────────────────────────────────────────────────────┐
│                     Application Layer                         │
│                       app/main.c                             │
├────────────────────────────┬─────────────────────────────────┤
│     Sensor Driver Layer    │        RTOS Task Layer           │
│   drivers/mpu6050.c/.h     │  sensor / processing / uart     │
├────────────────────────────┴─────────────────────────────────┤
│               Peripheral Driver Layer                         │
│             peripherals/i2c.c/.h                             │
├──────────────────────────────────────────────────────────────┤
│               ESP-IDF Hardware Abstraction                    │
│         (GPIO, I2C peripheral, UART, Timers)                 │
├──────────────────────────────────────────────────────────────┤
│                    ESP32 Hardware                             │
│              I2C Bus → MPU6050 Sensor                        │
└──────────────────────────────────────────────────────────────┘
```

---

<h2 id="features"> <span style="color:#185FA5;">✨ Features</span> </h2>

|Feature|Details|
|---|---|
|**Raw I2C driver**|Single-byte write, single-byte read, multi-byte burst read|
|**MPU6050 driver**|Init, accel read, gyro read, all-axes read in one call|
|**Data conversion**|Raw → g-force (`÷ 16384`), raw → °/s (`÷ 131`)|
|**Interrupt support**|Data-ready interrupt via `INT` pin, ISR → task notify|
|**Moving average filter**|Circular buffer, N=10 samples, applied to all accel axes|
|**Complementary filter**|Roll and pitch angle estimation (accel + gyro fusion)|
|**FreeRTOS pipeline**|3-task design: sensor → processing → UART, queue-connected|
|**Modular architecture**|Clean layer separation, fully independent driver modules|
|**Doxygen comments**|All public API functions documented|
|**Zero warnings**|Builds clean with `-Wall -Wextra`|

---

<h2 id="hardware-required"> <span style="color:#3B6D11;">🛒 Hardware Required</span> </h2>

|Component|Quantity|Notes|
|---|---|---|
|ESP32 Dev Board|1|Any standard 38-pin ESP32 board|
|MPU6050 Module|1|GY-521 breakout board works|
|Jumper Wires|4|Female-to-female|
|USB Cable|1|Micro-USB or USB-C depending on board|

> **Voltage:** ESP32 operates at **3.3V**. The GY-521 module has an onboard regulator and is safe to power from the ESP32's 3.3V pin. Do not connect to 5V directly.

---

<h2 id="wiring-diagram"> <span style="color:#854F0B;">🔌 Wiring Diagram</span> </h2>

```
ESP32 Dev Board          MPU6050 (GY-521)
─────────────────        ─────────────────
3.3V        ────────────  VCC
GND         ────────────  GND
GPIO 21     ────────────  SDA
GPIO 22     ────────────  SCL
GPIO 4      ────────────  INT    (optional — for interrupt mode)
GND         ────────────  AD0    (sets I2C address to 0x68)
```

> **I2C Address:** `AD0` pin LOW → address `0x68` (default). `AD0` HIGH → address `0x69`.

### Pin Summary Table

|ESP32 GPIO|MPU6050 Pin|Function|
|---|---|---|
|`GPIO_NUM_21`|`SDA`|I2C data line|
|`GPIO_NUM_22`|`SCL`|I2C clock line|
|`GPIO_NUM_4`|`INT`|Data-ready interrupt _(optional)_|
|`3V3`|`VCC`|3.3V power|
|`GND`|`GND`|Ground|
|`GND`|`AD0`|I2C address select → 0x68|

---

<h2 id="project-structure"> <span style="color:#993556;">📁 Project Structure</span> </h2>

```
imu-driver-project/
│
├── drivers/                    ← Sensor driver layer
│   ├── mpu6050.h               │  Public API, data struct, register defines
│   └── mpu6050.c               │  Init, read accel/gyro, all-axes read
│
├── peripherals/                ← Low-level hardware driver layer
│   ├── i2c.h                   │  Public I2C API
│   └── i2c.c                   │  Init, write byte, read byte, burst read
│
├── app/                        ← Application layer
│   └── main.c                  │  FreeRTOS tasks, filters, entry point
│
├── docs/                       ← Documentation
│   └── driver_architecture.md  │  Layer design decisions explained
│
├── CMakeLists.txt              ← Top-level ESP-IDF build file
├── sdkconfig                   ← ESP-IDF project configuration
├── .gitignore                  ← Excludes build/, sdkconfig.old
└── README.md                   ← This file
```

---

<h2 id="driver-architecture"> <span style="color:#0F6E56;">🏗️ Driver Architecture</span> </h2>

The project uses a **3-layer driver model**. Each layer only depends on the layer below it — never upward.

<h4 style="color:#0F6E56;">Layer 1 — Peripheral Driver (i2c.c / i2c.h)</h4>

The lowest layer. Knows nothing about MPU6050. Only knows how to send and receive bytes over I2C using ESP-IDF's `i2c_cmd_link` API.

```c
void    i2c_master_init(void);
void    i2c_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
uint8_t i2c_read_byte(uint8_t dev_addr, uint8_t reg_addr);
void    i2c_read_burst(uint8_t dev_addr, uint8_t reg_addr,
                       uint8_t *buf, uint8_t len);
```

<h4 style="color:#0F6E56;">Layer 2 — Sensor Driver (mpu6050.c / mpu6050.h)</h4>

Knows about MPU6050 registers. Calls the I2C layer to communicate. Returns structured data to the application. No FreeRTOS dependency.

```c
void mpu6050_init(void);
void mpu6050_read_accel(mpu6050_data_t *data);
void mpu6050_read_gyro(mpu6050_data_t *data);
void mpu6050_read_all(mpu6050_data_t *data);
```

<h4 style="color:#0F6E56;">Layer 3 — Application (main.c)</h4>

Knows about FreeRTOS. Runs tasks, queues, filters. Calls the sensor driver. Prints output.

---

<h2 id="getting-started"> <span style="color:#534AB7;">🚀 Getting Started</span> </h2>

### Prerequisites

- [ESP-IDF v5.x](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) installed
- ESP32 dev board
- MPU6050 sensor wired as shown above

### Clone the Repository

```bash
git clone https://github.com/yourusername/imu-driver-project.git
cd imu-driver-project
```

### Set Target

```bash
idf.py set-target esp32
```

### Build

```bash
idf.py build
```

### Flash

```bash
idf.py -p COM3 flash
# Replace COM3 with your actual port (Windows)
# Linux/Mac: /dev/ttyUSB0 or /dev/tty.usbserial-XXXXX
```

### Monitor

```bash
idf.py -p COM3 monitor
# Exit with Ctrl + ]
```

### All at once

```bash
idf.py -p COM3 build flash monitor
```

### Find your COM port

|OS|Command|
|---|---|
|Windows|Device Manager → Ports (COM & LPT)|
|Linux|`ls /dev/ttyUSB*`|
|macOS|`ls /dev/tty.usbserial*`|

---

<h2 id="api-reference"> <span style="color:#185FA5;">📚 API Reference</span> </h2> <h3 style="color:#185FA5;">I2C Driver — peripherals/i2c.h</h3>

```c
/**
 * @brief Initialize I2C master on SDA=GPIO21, SCL=GPIO22, 400kHz
 */
void i2c_master_init(void);

/**
 * @brief Write a single byte to a register on an I2C device
 * @param dev_addr  7-bit I2C device address
 * @param reg_addr  Target register address
 * @param data      Byte to write
 */
void i2c_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);

/**
 * @brief Read a single byte from a register on an I2C device
 * @param dev_addr  7-bit I2C device address
 * @param reg_addr  Source register address
 * @return          Byte read from register
 */
uint8_t i2c_read_byte(uint8_t dev_addr, uint8_t reg_addr);

/**
 * @brief Burst read multiple bytes starting from a register
 * @param dev_addr  7-bit I2C device address
 * @param reg_addr  Starting register address
 * @param buf       Output buffer (caller-allocated)
 * @param len       Number of bytes to read
 */
void i2c_read_burst(uint8_t dev_addr, uint8_t reg_addr,
                    uint8_t *buf, uint8_t len);
```

<h3 style="color:#185FA5;">Sensor Driver — drivers/mpu6050.h</h3>

```c
/**
 * @brief Sensor data structure — raw 16-bit signed values
 */
typedef struct {
    int16_t ax, ay, az;   /* Raw accelerometer (±2g → ÷16384 for g)   */
    int16_t gx, gy, gz;   /* Raw gyroscope    (±250°/s → ÷131 for °/s)*/
} mpu6050_data_t;

/**
 * @brief Initialize MPU6050: start I2C, wake sensor, verify WHO_AM_I
 */
void mpu6050_init(void);

/**
 * @brief Read raw 3-axis accelerometer data (6-byte burst from 0x3B)
 * @param data  Pointer to mpu6050_data_t — fills ax, ay, az
 */
void mpu6050_read_accel(mpu6050_data_t *data);

/**
 * @brief Read raw 3-axis gyroscope data (6-byte burst from 0x43)
 * @param data  Pointer to mpu6050_data_t — fills gx, gy, gz
 */
void mpu6050_read_gyro(mpu6050_data_t *data);

/**
 * @brief Read all 6 axes in a single 12-byte burst (most efficient)
 * @param data  Pointer to mpu6050_data_t — fills all fields
 */
void mpu6050_read_all(mpu6050_data_t *data);
```

### Data Conversion

```c
// Raw accelerometer → g-force (±2g full scale, default)
float ax_g = data.ax / 16384.0f;

// Raw gyroscope → degrees per second (±250°/s full scale, default)
float gx_dps = data.gx / 131.0f;
```

---

<h2 id="serial-output"> <span style="color:#3B6D11;">🖥️ Serial Output</span> </h2>

When running, the serial monitor shows:

```
I (0)    boot: ESP-IDF v5.2.0
I (312)  MPU6050: WHO_AM_I = 0x68  ✓
I (313)  MPU6050: Sensor initialized OK
I (323)  SENSOR: Accel X:  0.02g  | Y:  0.01g  | Z:  0.99g
I (323)  SENSOR: Gyro  X:  0.23/s | Y: -0.11/s | Z:  0.04/s
I (323)  FILTER: Roll:  1.2°  | Pitch: -0.8°
I (333)  SENSOR: Accel X:  0.02g  | Y:  0.01g  | Z:  0.99g
...
```

> Z-axis accelerometer reads ~1.0g when sensor is flat (Earth gravity). X and Y read ~0g. Tilt the sensor to verify axes respond correctly.

---

<h2 id="freertos-task-pipeline"> <span style="color:#854F0B;">⚙️ FreeRTOS Task Pipeline</span> </h2>

The application uses 3 FreeRTOS tasks connected by queues:

```
  [MPU6050 INT pin]
        │ interrupt
        ▼
┌──────────────────┐         ┌─────────────────────┐         ┌──────────────┐
│   sensor_task    │─queue1─►│  processing_task    │─queue2─►│  uart_task   │
│                  │         │                     │         │              │
│ Priority:  3     │         │ Priority:  2        │         │ Priority: 1  │
│ Stack: 2048 B    │         │ Stack: 4096 B       │         │ Stack: 2048B │
│                  │         │                     │         │              │
│ Woken by ISR     │         │ Convert raw → g/°s  │         │ ESP_LOGI()   │
│ Read mpu6050     │         │ Moving avg filter   │         │ Print values │
│ Send raw data    │         │ Complementary filter│         │              │
└──────────────────┘         │ Send processed data │         └──────────────┘
                             └─────────────────────┘
```

|Task|Priority|Stack|Responsibility|
|---|---|---|---|
|`sensor_task`|3 (highest)|2048 B|ISR-triggered, reads raw sensor data|
|`processing_task`|2|4096 B|Converts + filters data|
|`uart_task`|1 (lowest)|2048 B|Prints to serial monitor|

---

<h2 id="register-map"> <span style="color:#993556;">🗺️ Register Map</span> </h2>

Key MPU6050 registers used in this driver:

|Register Name|Address|Description|
|---|---|---|
|`WHO_AM_I`|`0x75`|Device ID — always returns `0x68`|
|`PWR_MGMT_1`|`0x6B`|Power control — bit 6 = SLEEP|
|`ACCEL_XOUT_H`|`0x3B`|Accel X high byte (start of 6-byte block)|
|`ACCEL_XOUT_L`|`0x3C`|Accel X low byte|
|`ACCEL_YOUT_H`|`0x3D`|Accel Y high byte|
|`ACCEL_YOUT_L`|`0x3E`|Accel Y low byte|
|`ACCEL_ZOUT_H`|`0x3F`|Accel Z high byte|
|`ACCEL_ZOUT_L`|`0x40`|Accel Z low byte|
|`TEMP_OUT_H`|`0x41`|Temperature high byte|
|`GYRO_XOUT_H`|`0x43`|Gyro X high byte (start of 6-byte block)|
|`GYRO_XOUT_L`|`0x44`|Gyro X low byte|
|`GYRO_YOUT_H`|`0x45`|Gyro Y high byte|
|`GYRO_YOUT_L`|`0x46`|Gyro Y low byte|
|`GYRO_ZOUT_H`|`0x47`|Gyro Z high byte|
|`GYRO_ZOUT_L`|`0x48`|Gyro Z low byte|
|`INT_ENABLE`|`0x38`|Interrupt enable — bit 0 = DATA_RDY_EN|
|`INT_PIN_CFG`|`0x37`|INT pin configuration|

### Reading 16-bit values from HIGH/LOW byte pairs

```c
// MPU6050 stores 16-bit values split across two consecutive registers
// High byte first, then low byte
int16_t ax = (int16_t)((buf[0] << 8) | buf[1]);
//                      ^^^^^^^^^^^^   ^^^^^^
//                      HIGH byte      LOW byte
//                      shifted left   or'd in
```

---

<h2 id="concepts-demonstrated"> <span style="color:#5F5E5A;">🎓 Concepts Demonstrated</span> </h2>

|Concept|Where|
|---|---|
|Register-level hardware programming|`peripherals/i2c.c`, `drivers/mpu6050.c`|
|Bit manipulation (SET/CLEAR/READ bits)|`mpu6050_init()` — wake up via PWR_MGMT_1|
|Fixed-width types (`int16_t`, `uint8_t`)|`mpu6050_data_t` struct|
|`volatile` keyword|ISR-shared flags|
|`static` encapsulation|Internal helpers in `.c` files|
|Header/source separation + include guards|All `.h` files|
|Struct + typedef driver pattern|`mpu6050_data_t`|
|Passing structs by pointer|All read functions|
|I2C bus communication protocol|`peripherals/i2c.c`|
|MPU6050 burst read (12 bytes, 1 transaction)|`mpu6050_read_all()`|
|ESP-IDF GPIO driver|LED + INT pin|
|ESP_LOG macros + log levels|Throughout|
|FreeRTOS tasks + priorities|`app/main.c`|
|FreeRTOS queues (producer-consumer)|task pipeline|
|ISR → task notification pattern|`xTaskNotifyFromISR()`|
|Moving average filter|`processing_task`|
|Complementary filter|Roll + pitch angle fusion|
|Modular CMakeLists.txt|Multi-folder build|

---

<img src="https://img.shields.io/badge/Built%20with-ESP--IDF-red?style=flat-square&logo=espressif"/> <img src="https://img.shields.io/badge/Language-C99-blue?style=flat-square&logo=c"/> <img src="https://img.shields.io/badge/RTOS-FreeRTOS-green?style=flat-square"/> <img src="https://img.shields.io/badge/Sensor-MPU6050-purple?style=flat-square"/> <img src="https://img.shields.io/badge/Protocol-I2C%20400kHz-orange?style=flat-square"/>

<br/><br/>

<p style="color:#B4B2A9; font-size:12px;">ESP32 · ESP-IDF · C · I2C · FreeRTOS · MPU6050 · Custom Embedded Driver</p> </div>