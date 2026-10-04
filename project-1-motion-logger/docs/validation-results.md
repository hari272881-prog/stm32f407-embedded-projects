# Validation Results

## Build results

The warning-cleanup pass removed naming collisions between the register-level driver definitions and STM32 CMSIS/HAL definitions. The custom peripheral and bit-position macros were given `CUSTOM_` prefixes. Include paths and source locations were then corrected independently for Debug and Release configurations.

| Firmware | Configuration | Result |
| --- | --- | --- |
| FreeRTOS | Debug | 0 errors and 0 warnings |
| FreeRTOS | Release | 0 errors and 0 warnings |
| Bare metal | Release | 0 errors and 0 warnings |

## FreeRTOS soak-test procedure

The system ran for approximately one hour while continuous telemetry, CLI access, orientation processing and hardware tap detection remained enabled. The sensor rate was changed during the run.

| Approximate interval | ODR | Activity |
| --- | ---: | --- |
| 0 to 10 minutes | 25 Hz | Telemetry, CLI checks and five accepted tap events |
| 10 to 20 minutes | 50 Hz | Telemetry, CLI checks and five accepted tap events |
| 20 to 35 minutes | 100 Hz | Normal operation and ten accepted tap events |
| 35 to 55 minutes | 400 Hz | Maximum acquisition load, movement and ten accepted tap events |
| 55 to 60 minutes | 100 Hz | Recovery, final commands and five accepted tap events |

The number of physical tap attempts during the soak test was not recorded. The value of 35 therefore represents accepted hardware events, not a detection-rate measurement.

## Final soak-test counters

The final paused debugger snapshot is stored at `evidence/soak-test/live-expressions/checkpoint-05-final.png`.

| Counter | Final value | Interpretation |
| --- | ---: | --- |
| Sensor read count | 506,818 | Samples read from LIS3DSH |
| Queue send count | 506,816 | Samples accepted by the RTOS queue |
| Queue receive count | 506,816 | Samples removed by processing task |
| Processed sample count | 506,816 | Samples fully processed |
| Sensor queue drop count | 2 | Queue could not accept two samples |
| UART TX frames queued | 33,475 | Application and CLI frames accepted by UART layer |
| UART DMA completions | 33,475 | Every accepted frame completed |
| UART DMA errors | 0 | No DMA transfer error recorded |
| UART TX queue full | 0 | No frame rejected by the UART queue |
| Telemetry frames dropped | 0 | No formatted telemetry frame was lost |
| UART RX overflow | 0 | RX ring buffer did not overflow |
| Current free heap | 6,864 bytes | Heap remaining at final checkpoint |
| Minimum-ever free heap | 6,864 bytes | No observed heap erosion |
| Stack overflow detected | 0 | Overflow hook did not fire |
| Allocation failure detected | 0 | Malloc-failure hook did not fire |
| Tap ISR count | 35 | EXTI events received |
| Processed tap count | 35 | Tap events accepted by task-level logic |

The accounting identity is:

```text
506,818 sensor reads = 506,816 processed samples + 2 queue drops
```

The queue delivery rate was approximately 99.9996 percent and the sample drop rate was approximately 0.0004 percent. Those two drops should not be described as UART frame drops. The UART evidence shows zero application telemetry drops, zero queue-full events and zero DMA errors.

## Stack and heap checkpoint

| Task | Free stack space at final soak checkpoint |
| --- | ---: |
| Default monitor task | 856 bytes |
| Sensor task | 816 bytes |
| Processing task | 724 bytes |
| Telemetry task | 1,364 bytes |
| CLI task | 1,288 bytes |

All values remained above zero, and the monitored heap stayed at 6,864 bytes.

## Manual double-tap test

This was a separate short test after the soak run. The number of physical attempts was recorded, and no false-positive tap was observed during normal manual use.

| ODR | Detected | Attempts | Detection rate | Misses |
| --- | ---: | ---: | ---: | ---: |
| 25 Hz | 10 | 19 | 52.6% | 9 |
| 50 Hz | 10 | 14 | 71.4% | 4 |
| 100 Hz | 15 | 17 | 88.2% | 2 |
| 400 Hz | 15 | 16 | 93.8% | 1 |
| Total | 50 | 66 | 75.8% | 16 |

The higher success rate at 100 and 400 Hz is consistent with finer sampling of the tap waveform. At 25 and 50 Hz, taps needed to be slower and more distinct. This test used hand-applied taps, so force and spacing were not controlled; it is useful as a practical observation rather than a laboratory characterization.

## Logic-analyzer evidence

The PulseView archive contains raw sessions and decoder setup files for both implementations. The FreeRTOS close-up shows an `0xA8` multi-byte read followed by six dummy MOSI bytes while MISO returns sensor data. The bare-metal close-up supplied with the capture shows an `0xA7` status-register read; the complete raw session is included for further inspection.

UART captures show the `read` command on the STM32 RX line and the formatted sensor response on TX. The combined views show UART traffic while SPI acquisition continues.

## Limitations

- The manual tap test does not control tap force, contact location or time between impacts.
- Two samples were dropped at the sensor-to-processing queue during the one-hour variable-rate run.
- The soak log ends with CLI counters at 505,838 processed samples; the later paused debugger snapshot records the final 506,818 reads and 506,816 processed samples.
- Logic-analyzer screenshots provide protocol evidence, while the raw `.sr` files are needed for detailed timing measurements.

