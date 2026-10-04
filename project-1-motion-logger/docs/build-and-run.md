# Build and Run Guide

## Required hardware and software

- STM32F407 Discovery board
- Onboard LIS3DSH accelerometer
- ST-LINK connection
- 3.3 V compatible USB to UART adapter for PA2 and PA3
- STM32CubeIDE 1.19 or a compatible release
- Serial terminal such as PuTTY
- Optional eight-channel logic analyzer and PulseView

## Importing the projects

Import only one implementation at a time if the workspace already contains projects with similar names.

1. Open STM32CubeIDE.
2. Select **File > Import > General > Existing Projects into Workspace**.
3. For the bare-metal implementation, select the `bare-metal` folder.
4. For the FreeRTOS implementation, select the `freertos` folder.
5. Keep **Copy projects into workspace** unchecked if you want to build directly from the repository.
6. Finish the import and select either Debug or Release.
7. Use **Project > Clean**, followed by **Build Project**.

The final checked builds produced zero errors and zero warnings for FreeRTOS Debug, FreeRTOS Release and bare-metal Release.

## Programming and terminal setup

1. Connect the board through ST-LINK.
2. Connect the UART adapter ground to board ground.
3. Connect adapter RX to PA2, the STM32 USART2 TX pin.
4. Connect adapter TX to PA3, the STM32 USART2 RX pin.
5. Program and run the selected firmware.
6. Open the serial port with these settings:

```text
115200 baud
8 data bits
No parity
1 stop bit
No flow control
```

## Basic functional test

Run:

```text
help
read
status
rate 25
rate 50
rate 100
rate 400
monitor on
monitor off
```

Check that `status` reports the same requested and actual ODR. Move the board and confirm that the orientation text and direction LEDs respond. Perform a double tap and confirm that the tap count increases once and all four LEDs flash briefly.

## PulseView connection

| Logic channel | Signal | STM32 pin |
| --- | --- | --- |
| D0 | SPI chip select | PE3 |
| D1 | SPI clock | PA5 |
| D2 | SPI MOSI | PA7 |
| D3 | SPI MISO | PA6 |
| D4 | USART2 TX | PA2 |
| D5 | USART2 RX | PA3 |

Connect the analyzer ground to the board ground. Do not connect a 5 V analyzer supply to the STM32 signals.

The published captures used 8 MHz sampling. Configure the SPI decoder for mode 3, eight bits, MSB first and active-low chip select. Configure the UART decoder for 115200 8-N-1 with normal polarity.

## Reproducing a combined capture

Before acquisition:

```text
monitor off
rate 100
status
```

Start PulseView, then paste:

```text
read
status
monitor on
```

This places CLI traffic on USART2 RX, responses and telemetry on USART2 TX, and periodic sensor traffic on SPI1 within the same capture.

