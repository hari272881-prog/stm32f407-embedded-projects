/*
 * cli.c
 *
 *  Created on: Sep 30, 2026
 *      Author: R.S. Hari Naveen
 */

#include "cli.h"
#include "uart_async.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define CLI_LINE_BUFFER_SIZE      64U
#define CLI_RESPONSE_SIZE         160U

static CLI_Context_t g_cliContext;

static char g_cliLine[CLI_LINE_BUFFER_SIZE];
static uint32_t g_cliLineLength;
static uint8_t g_cliLineOverflow;
static uint8_t g_previousWasCarriageReturn;

static void CLI_SendText(const char *text)
{
    uint32_t length;

    if (text == 0)
    {
        return;
    }

    length = (uint32_t)strlen(text);

    if ((length > 0U) && (length <= CLI_RESPONSE_SIZE))
    {
        (void)UART2_AsyncQueueTx((const uint8_t *)text, (uint16_t)length);
    }
}

static void CLI_SendFormatted(const char *format, ...)
{
    char response[CLI_RESPONSE_SIZE];
    va_list arguments;
    int length;

    va_start(arguments, format);
    length = vsnprintf(response, sizeof(response), format, arguments);
    va_end(arguments);

    if ((length > 0) && ((uint32_t)length < sizeof(response)))
    {
        (void)UART2_AsyncQueueTx((const uint8_t *)response, (uint16_t)length);
    }
}

static void CLI_SendPrompt(void)
{
    CLI_SendText("> ");
}

static char *CLI_SkipSpaces(char *text)
{
    while ((*text == ' ') || (*text == '\t'))
    {
        text++;
    }

    return text;
}

static void CLI_RemoveTrailingSpaces(char *text)
{
    uint32_t length;

    length = (uint32_t)strlen(text);

    while (length > 0U)
    {
        if ((text[length - 1U] != ' ') && (text[length - 1U] != '\t'))
        {
            break;
        }

        text[length - 1U] = '\0';
        length--;
    }
}

static void CLI_ConvertToLowercase(char *text)
{
    while (*text != '\0')
    {
        if ((*text >= 'A') && (*text <= 'Z'))
        {
            *text = (char)(*text - 'A' + 'a');
        }

        text++;
    }
}

static void CLI_CommandHelp(void)
{
    CLI_SendText("Commands:\r\n"
                 "  help        - show commands\r\n"
                 "  read        - show latest acceleration\r\n"
                 "  status      - show counters and communication status\r\n");

    CLI_SendText("  rate N      - set LIS3DSH ODR: 25, 50, 100 or 400 Hz\r\n"
                 "  monitor on  - enable continuous telemetry\r\n");

    CLI_SendText("  monitor off - pause continuous telemetry\r\n"
                 "  clear       - clear terminal screen\r\n");
}

static void CLI_CommandRead(void)
{
    CLI_SendFormatted("X=%ld mg,Y=%ld mg,Z=%ld mg,O=%s,T=%s,Taps=%lu\r\n",
                      (long)g_cliContext.pAcceleration->x,
                      (long)g_cliContext.pAcceleration->y,
                      (long)g_cliContext.pAcceleration->z,
                      Orientation_GetName(*g_cliContext.pOrientation),
                      MotionDetection_GetTiltName(*g_cliContext.pTiltDirection),
                      (unsigned long)*g_cliContext.pTapCount);
}
static const char *CLI_GetOdrName(uint8_t controlRegister4)
{
    switch (controlRegister4 & 0xF0U)
    {
        case 0x00U:
            return "POWER DOWN";

        case 0x10U:
            return "3.125";

        case 0x20U:
            return "6.25";

        case 0x30U:
            return "12.5";

        case 0x40U:
            return "25";

        case 0x50U:
            return "50";

        case 0x60U:
            return "100";

        case 0x70U:
            return "400";

        case 0x80U:
            return "800";

        case 0x90U:
            return "1600";

        default:
            return "UNKNOWN";
    }
}

static void CLI_CommandStatus(void)
{
    const UART2_AsyncStats_t *uartStats;
    const char *monitorState;
    const char *actualOdr;
    uint8_t controlRegister4;

    uartStats = UART2_AsyncGetStats();

    if (*g_cliContext.pTelemetryEnabled != 0U)
    {
        monitorState = "ON";
    }
    else
    {
        monitorState = "OFF";
    }

    /*
     * Read the actual CTRL_REG4 value from the LIS3DSH
     * through SPI.
     */
    controlRegister4 = LIS3DSH_ReadRegister(LIS3DSH_CTRL_REG4);
    actualOdr = CLI_GetOdrName(controlRegister4);

    /*
     * Split the output into two frames so neither frame
     * exceeds the 160-byte DMA frame limit.
     */
    CLI_SendFormatted("Sensor: Requested=%uHz,Actual=%sHz,CTRL4=0x%02X,Monitor=%s,Samples=%lu\r\n",
                      (unsigned int)*g_cliContext.pSensorRateHz,
                      actualOdr,
                      (unsigned int)controlRegister4,
                      monitorState,
                      (unsigned long)*g_cliContext.pSampleCount);

    CLI_SendFormatted("UART: Frames=%lu,AppDrop=%lu,DMA=%lu,DMAErr=%lu,QFull=%lu,RX=%lu,RXOvf=%lu\r\n",
                      (unsigned long)*g_cliContext.pFramesGenerated,
                      (unsigned long)*g_cliContext.pFramesDropped,
                      (unsigned long)uartStats->dmaCompletions,
                      (unsigned long)uartStats->dmaErrors,
                      (unsigned long)uartStats->txQueueFull,
                      (unsigned long)uartStats->rxBytes,
                      (unsigned long)uartStats->rxOverflow);
}

static void CLI_CommandRate(char *argument)
{
    unsigned long requestedRate;
    char *endPointer;

    argument = CLI_SkipSpaces(argument);

    if (*argument == '\0')
    {
        CLI_SendText("Usage: rate 25|50|100|400\r\n");
        return;
    }

    requestedRate = strtoul(argument, &endPointer, 10);
    endPointer = CLI_SkipSpaces(endPointer);

    if (*endPointer != '\0')
    {
        CLI_SendText("Invalid rate argument\r\n");
        return;
    }

    if ((requestedRate != 25UL) &&
        (requestedRate != 50UL) &&
        (requestedRate != 100UL) &&
        (requestedRate != 400UL))
    {
        CLI_SendText("Supported rates: 25, 50, 100 and 400 Hz\r\n");
        return;
    }

    if (LIS3DSH_SetDoubleTapOutputDataRate(
        (uint16_t)requestedRate) == 0U)
    {
    	CLI_SendText("Failed to update sensor ODR and tap timing\r\n");
    	return;
    }

    *g_cliContext.pSensorRateHz = (uint16_t)requestedRate;

    CLI_SendFormatted("Sensor ODR and hardware tap timing changed to %lu Hz\r\n",requestedRate);
}

static void CLI_CommandMonitor(uint8_t enable)
{
    if (g_cliContext.pTelemetryEnabled == 0)
    {
        CLI_SendText("Telemetry control is unavailable\r\n");
        return;
    }

    *g_cliContext.pTelemetryEnabled = enable;

    if (enable != 0U)
    {
        CLI_SendText("Continuous telemetry enabled\r\n");
    }
    else
    {
        CLI_SendText("Continuous telemetry paused\r\n");
    }
}

static void CLI_CommandClear(void)
{
    /*
     * ANSI terminal sequence:
     * ESC[2J clears the screen.
     * ESC[H moves the cursor to the top-left corner.
     */
    CLI_SendText("\033[2J\033[H");
}

static void CLI_ProcessCommand(char *command)
{
    command = CLI_SkipSpaces(command);

    CLI_RemoveTrailingSpaces(command);
    CLI_ConvertToLowercase(command);

    if (*command == '\0')
    {
        return;
    }

    if (strcmp(command, "help") == 0)
    {
        CLI_CommandHelp();
    }
    else if (strcmp(command, "read") == 0)
    {
        CLI_CommandRead();
    }
    else if (strcmp(command, "status") == 0)
    {
        CLI_CommandStatus();
    }
    else if (strcmp(command, "monitor on") == 0)
    {
        CLI_CommandMonitor(1U);
    }
    else if (strcmp(command, "monitor off") == 0)
    {
        CLI_CommandMonitor(0U);
    }
    else if (strcmp(command, "clear") == 0)
    {
        CLI_CommandClear();
    }
    else if ((strncmp(command, "rate", 4U) == 0) &&
             ((command[4] == ' ') || (command[4] == '\t')))
    {
        CLI_CommandRate(&command[4]);
    }
    else
    {
        CLI_SendText("Unknown command. Type help\r\n");
    }
}

static void CLI_ProcessCompletedLine(void)
{
    if (g_cliLineOverflow != 0U)
    {
        CLI_SendText("\r\nCommand too long\r\n");

        g_cliLineLength = 0U;
        g_cliLineOverflow = 0U;
        CLI_SendPrompt();
        return;
    }

    g_cliLine[g_cliLineLength] = '\0';

    CLI_SendText("\r\n");
    CLI_ProcessCommand(g_cliLine);

    g_cliLineLength = 0U;
    CLI_SendPrompt();
}

void CLI_Init(const CLI_Context_t *context)
{
    if (context == 0)
    {
        return;
    }

    g_cliContext = *context;

    g_cliLineLength = 0U;
    g_cliLineOverflow = 0U;
    g_previousWasCarriageReturn = 0U;

    CLI_SendText("UART CLI ready. Type help\r\n");
    CLI_SendPrompt();
}

void CLI_ProcessByte(uint8_t receivedByte)
{
    if (receivedByte == '\r')
    {
        CLI_ProcessCompletedLine();
        g_previousWasCarriageReturn = 1U;
        return;
    }

    if (receivedByte == '\n')
    {
        if (g_previousWasCarriageReturn != 0U)
        {
            g_previousWasCarriageReturn = 0U;
            return;
        }

        CLI_ProcessCompletedLine();
        return;
    }

    g_previousWasCarriageReturn = 0U;

    if ((receivedByte == '\b') || (receivedByte == 0x7FU))
    {
        if (g_cliLineLength > 0U)
        {
            g_cliLineLength--;
        }

        return;
    }

    if ((receivedByte < 0x20U) || (receivedByte > 0x7EU))
    {
        return;
    }
    /*
 * Pause periodic telemetry as soon as the user starts
 * entering a command. Sensor sampling continues.
 */
    if (g_cliContext.pTelemetryEnabled != 0)
    {
    	*g_cliContext.pTelemetryEnabled = 0U;
    }

    if (g_cliLineLength < (CLI_LINE_BUFFER_SIZE - 1U))
    {
        g_cliLine[g_cliLineLength] = (char)receivedByte;
        g_cliLineLength++;
    }
    else
    {
        g_cliLineOverflow = 1U;
    }
}
