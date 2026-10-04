# STM32F407 Motion Logger Project Report

**Author:** R.S. Hari Naveen  
**Platform:** STM32F407 Discovery  
**Sensor:** LIS3DSH three-axis accelerometer  
**Development environment:** STM32CubeIDE 1.19

## Project summary

I developed a motion logger for the STM32F407 Discovery board in two stages. The first version uses a bare-metal superloop. The second keeps the same sensor, motion-processing and communication features but divides the work into FreeRTOS tasks. The application reads the onboard LIS3DSH over SPI, reports acceleration and orientation through USART2, drives the four board LEDs according to tilt, and uses the sensor's hardware state machine for double-tap detection.

The main goal was to practise peripheral drivers and asynchronous communication first, then learn how queues, semaphores, mutexes and task scheduling change the structure of an embedded application. I validated the final FreeRTOS design with a one-hour variable-rate soak test and captured SPI and UART activity using PulseView.

## Hardware and communication

The LIS3DSH uses SPI1 with PE3 as chip select and PA5, PA6 and PA7 as clock, MISO and MOSI. Its INT1 output is connected to PE0 and reaches the processor through EXTI0. USART2 uses PA2 for TX and PA3 for RX at 115200 baud. PD12 to PD15 drive the green, orange, red and blue LEDs.

The SPI driver performs register reads and writes directly. XYZ acceleration is read as a multi-byte transfer and converted from signed raw values to mg. The orientation module filters the samples before deciding which face is upward. A separate motion module applies thresholds for forward, backward, left and right tilt.

USART reception is interrupt-driven. Received bytes go into a 256-byte circular buffer and are parsed by the command-line interface outside the interrupt. Transmit messages are copied into an eight-frame queue and sent by DMA1 Stream 6. This prevents the main processing path or an RTOS task from waiting for every UART byte to leave the peripheral.

## Bare-metal implementation

The bare-metal program initializes the UART, DMA, SPI, LEDs, LIS3DSH and EXTI, then remains in one loop. Each iteration consumes available CLI bytes, processes pending tap events and checks the sensor data-ready flag. When a sample is available, the program reads and converts it, updates orientation and LEDs, and periodically queues a telemetry frame.

I kept the interrupt routines short. The MEMS interrupt records an event, the UART interrupt stores an incoming byte, and the DMA interrupt completes one frame and starts the next. Sensor register access happens in the main context.

This version made the complete control flow easy to follow, but the responsibilities share one loop. As the CLI, telemetry and motion processing grew, it became harder to describe ownership and timing only through loop order.

## FreeRTOS implementation

The FreeRTOS version has five tasks. The sensor task reads raw samples and sends them through an eight-element message queue. The processing task converts samples, updates orientation and drives the LEDs. The telemetry task formats data every 100 ms. The CLI task consumes UART RX bytes. A low-priority monitor task records heap and stack headroom.

The EXTI callback releases a binary semaphore instead of accessing the sensor. The sensor task takes that semaphore, reads the LIS3DSH interrupt-source register and sets a thread flag for the processing task. The flag starts a 200 ms all-LED tap indication.

Two mutexes define peripheral ownership. The sensor mutex protects SPI when a CLI command reads or changes the ODR while acquisition is active. The UART mutex protects the shared TX queue used by the telemetry and CLI tasks. The UART's own interrupts and DMA continue to handle byte movement.

## Runtime commands

The terminal supports `help`, `read`, `status`, `rate 25|50|100|400`, `monitor on`, `monitor off` and `clear`. The rate command changes both the LIS3DSH ODR and the tap timing. During that update the firmware temporarily disables EXTI0, clears the latched sensor source and pending interrupt state, and then re-enables the interrupt. This fixed a false tap that previously appeared after reset or an ODR change.

## Verification

The final warning-cleanup pass removed collisions between names in my register-level drivers and definitions supplied by CMSIS/HAL. FreeRTOS Debug, FreeRTOS Release and bare-metal Release then built with zero errors and zero warnings.

The FreeRTOS soak test changed the sensor between 25, 50, 100 and 400 Hz while telemetry, CLI activity, movement and hardware tap detection were exercised. The final debugger snapshot showed 506,818 reads, 506,816 processed samples and two queue drops. All 33,475 UART frames accepted by the asynchronous layer completed through DMA. Telemetry drops, DMA errors, TX queue-full events and RX overflows remained at zero.

Current and minimum-ever free heap both remained at 6,864 bytes. Every monitored task retained stack headroom, and neither the stack-overflow hook nor the allocation-failure hook fired. The EXTI count and processed tap count both reached 35.

The two sample drops occurred before application processing and are different from UART telemetry-frame drops. Based on the counters, the sensor queue delivered approximately 99.9996 percent of reads during the variable-rate run.

I also repeated a short manual tap test while recording attempts. Detection was 10/19 at 25 Hz, 10/14 at 50 Hz, 15/17 at 100 Hz and 15/16 at 400 Hz. No false-positive tap was observed during normal manual use. Since the taps were applied by hand, these figures show practical behaviour rather than controlled sensor characterization.

## Logic-analyzer results

PulseView captures were recorded for both firmware versions at 8 MHz. The FreeRTOS SPI close-up contains the `0xA8` multi-byte sensor read, and the UART captures show the `read` command on RX followed by the formatted acceleration response on TX. The combined captures show that sensor transfers continue while UART output is active. Raw PulseView sessions are included so the decoder settings and timing can be checked again.

## What I learned

The main change from bare metal to FreeRTOS was not the sensor algorithm. It was deciding which context owns each resource and how events move between contexts. A semaphore was suitable for waking task-level tap handling from an interrupt, a queue carried complete sensor samples, and mutexes protected SPI and UART producers. Adding counters for every boundary made it possible to verify the design instead of assuming that no data was lost.

The validation also showed a limitation. Low sensor rates gave poorer manually observed tap detection, and two samples were lost at the RTOS queue during the long variable-rate run. These results suggest that future work should tune the tap timing with a controlled fixture and examine queue behaviour around rate changes or temporary processing delays.

## Conclusion

The project produced working bare-metal and FreeRTOS implementations of the same motion-logging application. Both use asynchronous UART communication and hardware double-tap detection, while the FreeRTOS version adds explicit task separation and synchronization. The final build and test evidence supports stable UART operation, bounded memory use and complete processing of all but two of more than half a million sensor reads during the soak test.

