# From Bare Metal to FreeRTOS

## What was reused

I treated the FreeRTOS version as a restructuring of the working bare-metal application rather than a complete rewrite. The following code and ideas were reused:

- LIS3DSH SPI register driver and raw-to-mg conversion
- orientation filtering and naming
- tilt classification and LED direction mapping
- hardware double-tap state-machine configuration
- UART RX ring buffer, TX frame queue and DMA transmission
- CLI commands and telemetry formatting
- custom RCC, GPIO, SPI and USART register-level drivers

These modules are mostly independent of whether the scheduler is a superloop or FreeRTOS.

## What changed

| Bare-metal design | FreeRTOS design |
| --- | --- |
| One main superloop | Five tasks with separate responsibilities |
| Shared state updated sequentially | Queue transfers samples between acquisition and processing |
| MEMS event flag consumed by main loop | EXTI releases a semaphore consumed by the sensor task |
| LED tap indication counted in samples | Processing task receives a tap thread flag and calculates a 200 ms duration |
| CLI and sensor code cannot run concurrently | Sensor mutex protects SPI access during status and ODR commands |
| CLI and telemetry take turns naturally in one loop | UART mutex protects the shared asynchronous TX queue |
| No RTOS memory instrumentation | Heap, stack, overflow and allocation-failure counters are monitored |

## Code not carried into the FreeRTOS application

The completed FreeRTOS project does not use the bare-metal `Project1.c` superloop or the separate `mems_interrupt.c` event module. CubeMX/HAL supplies the startup and EXTI dispatch path, and the active tap callback is in `Core/Src/main.c`. The old `application.c` scaffold from the bare-metal project is also not used in the FreeRTOS folder.

## Main lesson from the conversion

The sensor and UART drivers were already non-blocking enough to reuse. Most of the conversion work was therefore about ownership: deciding which task reads the sensor, which context clears the interrupt, how samples move to processing, and how two tasks share UART and SPI safely. The counters added for the soak test were useful because they showed whether those ownership decisions held under changing sensor rates.

