# STM32F407 Embedded Projects

**Author:** R.S. Hari Naveen

This repository contains embedded-system projects developed using the STM32F407 Discovery board. The projects focus on sensor interfacing, bare-metal firmware, FreeRTOS, communication protocols, low-power operation and hardware-based testing.

Each project has its own README, source code, build instructions and captured test evidence.

## Projects

### 1. Motion Logger

The motion logger reads the onboard LIS3DSH accelerometer and calculates acceleration, face orientation, tilt direction and double-tap events.

I first developed the application as a bare-metal superloop and then implemented a FreeRTOS version using separate sensor, processing, telemetry and CLI tasks.

Main features include:

- SPI communication with the LIS3DSH accelerometer
- Acceleration conversion to mg
- Orientation and tilt detection
- Hardware double-tap detection
- Selectable 25, 50, 100 and 400 Hz sensor data rates
- USART2 command-line interface
- Interrupt-driven UART reception
- DMA-based UART transmission
- FreeRTOS queues, mutexes, semaphores and task notifications
- One-hour FreeRTOS soak test
- Logic-analyser captures of SPI and UART communication

[View Project 1 Motion Logger](project-1-motion-logger/)

### 2. Low-Power Sensor Node

This bare-metal project turns the STM32F407 Discovery board into a periodic low-power motion and orientation sensor.

The MCU spends most of its time in Stop mode. It wakes using the RTC wake-up timer, starts the LIS3DSH, collects and processes one fresh sample, transmits the result over USART2 and returns to Stop mode.

Main features include:

- STM32 Stop mode with the low-power regulator
- RTC wake-up timer using the internal LSI clock
- Sensor power-down between measurements
- SPI accelerometer acquisition
- Orientation and tilt classification
- USART2 telemetry
- Logic-analyser measurement of active time and wake period
- Current measurements through the Discovery board JP1 IDD link
- Measured active duty cycle of approximately 0.288%
- Measured current reduction of approximately 81.6% compared with continuous operation

[View Project 2 Low-Power Sensor Node](project-2-low-power-sensor-node/)

## Repository Structure

```text
stm32f407-embedded-projects/
├── project-1-motion-logger/
│   ├── bare-metal/
│   ├── freertos/
│   ├── docs/
│   └── README.md
├── project-2-low-power-sensor-node/
│   ├── Inc/
│   ├── Src/
│   ├── drivers/
│   ├── Startup/
│   ├── docs/
│   └── README.md
├── .gitattributes
└── README.md
```

## Hardware and Tools

- STM32F407 Discovery board
- STM32F407VGT6 microcontroller
- Onboard LIS3DSH accelerometer
- STM32CubeIDE
- ST-LINK debugger
- USB-to-UART adapter
- PuTTY serial terminal
- Logic analyser with PulseView
- Digital multimeter for current measurements

## Documentation

Detailed architecture, build instructions, measurements, test procedures and captured evidence are available inside each project folder.

The included results were obtained from testing the firmware on the STM32F407 Discovery hardware.