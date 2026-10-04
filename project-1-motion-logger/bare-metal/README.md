# Bare Metal Firmware

This folder is an independently importable STM32CubeIDE project. Its main application entry point is `Src/Project1.c`.

The application uses a cooperative superloop. It checks the UART RX ring buffer, consumes pending tap events and reads the LIS3DSH whenever new data is ready. Orientation, tilt LEDs, telemetry scheduling and CLI processing all run from this loop. Short interrupt handlers record UART bytes, finish DMA frames or post a MEMS event; SPI sensor work stays in the main context.

## Important source folders

| Folder | Purpose |
| --- | --- |
| `Src` and `Inc` | Application, sensor, CLI, UART and signal processing |
| `drivers/Src` and `drivers/Inc` | Register-level GPIO, RCC, SPI and USART drivers |
| `Startup` | STM32F407 startup assembly |

`Src/application.c` and `Inc/application.h` are an early application-layer scaffold. The completed firmware starts from `Src/Project1.c`.

## Runtime flow

1. Initialize USART2, DMA, SPI1, LEDs and LIS3DSH.
2. Configure LIS3DSH hardware double-tap detection and EXTI0.
3. Service UART commands from the RX ring buffer.
4. Convert MEMS interrupt events into tap counts.
5. Poll the data-ready bit and read XYZ acceleration.
6. Update orientation and tilt LEDs.
7. Queue telemetry at approximately 10 frames per second.

See [the common build instructions](../docs/build-and-run.md) for import, wiring and terminal settings.

