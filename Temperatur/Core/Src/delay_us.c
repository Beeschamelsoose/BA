/*
 * delay_us.c
 *
 *  Created on: Aug 10, 2025
 *      Author: ernst
 */


#include "stm32l4xx.h"

void delay_us_init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = (SystemCoreClock / 1000000U) * us;
    while ((DWT->CYCCNT - start) < ticks) {
        __NOP();
    }
}
