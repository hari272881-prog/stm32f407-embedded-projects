# FreeRTOS Firmware

This folder is an independently importable STM32CubeIDE project generated around CMSIS-RTOS2 and FreeRTOS. The application task code is in `Core/Src/main.c`; reusable modules are in `Application`, and register-level peripheral support is in `CustomDrivers`.

## Tasks and communication

| Task | Priority | Stack allocation | Responsibility |
| --- | --- | ---: | --- |
| Sensor | Normal | 1,024 bytes | Read the LIS3DSH, place samples in the queue and service tap events |
| CLI | Normal | 2,048 bytes | Consume UART RX bytes and execute commands |
| Processing | Below normal | 1,024 bytes | Convert samples to mg, update orientation and drive LEDs |
| Telemetry | Below normal | 2,048 bytes | Format periodic status frames and queue them for DMA |
| Default monitor | Low | 1,024 bytes | Record heap and task stack headroom once per second |

The design uses:

- an eight-element sensor message queue;
- a binary semaphore from EXTI0 to the sensor task;
- a thread flag from the sensor task to the processing task;
- a sensor mutex around SPI configuration and register access;
- a UART mutex around the asynchronous TX queue.

The asynchronous UART layer uses a 256-byte RX ring buffer and an eight-frame TX queue. DMA1 Stream 6 sends complete frames without making tasks busy-wait on USART2.

See [architecture.md](../docs/architecture.md) for the full design and [validation-results.md](../docs/validation-results.md) for the soak-test evidence.

