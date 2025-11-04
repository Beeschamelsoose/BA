/*
 * delay.h
 *
 *  Created on: 5 Aug 2025
 *      Author: ernst
 */

#ifndef INC_DELAY_H_
#define INC_DELAY_H_

#include <stdint.h>
uint64_t millis(void);
void systick_init_ms(uint32_t freq);
void delay(uint32_t delay);



#endif /* INC_DELAY_H_ */
