/*
 * ausgabe.h
 *
 *  Created on: Oct 10, 2025
 *      Author: ernst
 */

#ifndef AUSGABE_H
#define AUSGABE_H

#include "stm32l4xx_hal.h"
#include "save.h"

#ifdef __cplusplus
extern "C" {
#endif

void Ausgabe_Init(UART_HandleTypeDef *huart1);
void Ausgabe_RxCpltCallback(UART_HandleTypeDef *huart);
void Ausgabe_TxCpltCallback(UART_HandleTypeDef *huart);
void Ausgabe_Process(void);

extern volatile uint8_t ausgabe_aktiv;

#ifdef __cplusplus
}
#endif


#endif /* INC_AUSGABE_H_ */
