# Measurement Notes

These measurements were taken on the STM32F407 Discovery board after the firmware had been flashed and the debugger had been disconnected.

## Raw readings

### Stop-mode current through JP1

| Reading | Current |
|---:|---:|
| 1 | 1.75 mA |
| 2 | 1.76 mA |
| 3 | 1.75 mA |
| 4 | 1.75 mA |
| 5 | 1.75 mA |
| **Average** | **1.752 mA** |

### Continuously active current through JP1

| Reading | Current |
|---:|---:|
| 1 | 9.62 mA |
| 2 | 9.64 mA |
| 3 | 9.63 mA |
| 4 | 9.62 mA |
| 5 | 9.64 mA |
| **Average** | **9.63 mA** |

### PE0 active-pulse width

| Capture | Time |
|---:|---:|
| 1 | 28.064 ms |
| 2 | 28.048 ms |
| 3 | 28.080 ms |
| 4 | 28.064 ms |
| 5 | 28.064 ms |
| **Average** | **28.064 ms** |

The measured RTC wake period was **9.74848 s**.

## Duty-cycle calculation

```text
Duty cycle = active time / wake period
           = 0.028064 s / 9.74848 s
           = 0.002879
           = 0.288%
```

## Estimated average MCU current

```text
Iavg = Istop + (Iactive - Istop) x duty cycle
     = 1.752 mA + (9.63 mA - 1.752 mA) x 0.002879
     = 1.775 mA
```

Compared with continuous active operation:

```text
Current reduction = (9.63 mA - 1.775 mA) / 9.63 mA x 100
                  = 81.6%
```

For a simplified 2000 mAh source, ignoring regulator losses, self-discharge, and the rest of the development board:

```text
Ideal MCU-domain life = 2000 mAh / 1.775 mA = 1127 h = 46.9 days
80% usable-capacity estimate = 37.5 days
```

These battery-life values are estimates for comparison, not a prediction for the complete Discovery board. JP1 isolates the MCU current path; the board LEDs, ST-LINK section, voltage regulator, sensor, and USB-to-UART adapter can add current outside that measurement.

## Interpretation

The logic-analyser trace confirms that the MCU is active for only about 28 ms during each roughly 9.75-second cycle. This makes the active duty cycle very small. The remaining average-current floor is therefore dominated by the measured Stop current. Increasing the wake period further would reduce the average only slightly unless the Stop-mode current is also reduced.

