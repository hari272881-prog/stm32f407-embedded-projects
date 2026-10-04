# Evidence Index

## Soak test

- `soak-test/Project1_test_final_soak_60mins_5.log` contains the complete PuTTY session.
- `soak-test/live-expressions/checkpoint-01-start.png` through `checkpoint-05-final.png` show the Project 1 FreeRTOS counters during the run.
- `soak-test/live-expressions/putty-final-status.png` shows the final terminal commands and status output.

Only the screenshots relevant to this motion-logger project were extracted from the supplied Word document.

## Logic analyzer

The `logic-analyzer/freertos` and `logic-analyzer/bare-metal` folders contain PulseView `.sr` recordings, `.pvs` setup data and PNG screenshots.

The capture wiring was:

```text
D0 PE3 SPI chip select
D1 PA5 SPI clock
D2 PA7 SPI MOSI
D3 PA6 SPI MISO
D4 PA2 USART2 TX
D5 PA3 USART2 RX
```

The sessions were sampled at 8 MHz. SPI was decoded in mode 3, and UART was decoded at 115200 8-N-1.
