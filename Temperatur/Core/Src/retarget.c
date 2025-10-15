/*
 * retarget.c
 *
 *  Created on: Aug 10, 2025
 *      Author: ernst
 */


#include "usart.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>

int __io_putchar(int ch)
{
    uint8_t c = (uint8_t)ch;
    // CRLF korrekt als \r\n ausgeben
    if (c == '\n') {
        uint8_t cr = '\r';
 //       HAL_UART_Transmit(&huart1, &cr, 1, HAL_MAX_DELAY); // erst \r
        HAL_UART_Transmit(&huart2, &cr, 1, HAL_MAX_DELAY); // erst \r
    }
  //  HAL_UART_Transmit(&huart1, &c, 1, HAL_MAX_DELAY);       // dann Zeichen
    HAL_UART_Transmit(&huart2, &c, 1, HAL_MAX_DELAY);       // dann Zeichen
    return ch;
}
