/*
 * sensors.h
 *
 *  Created on: Oct 21, 2025
 *      Author: ernst
 */

#ifndef SENSORS_H
#define SENSORS_H

#include "main.h"
#include "dht11.h"
#include "adc.h"

// Liest Sensoren und ADC, gibt Messwerte an die Main zurück
void read_sensors(int8_t *t1, int8_t *t2, int8_t *t3,
                  int8_t *h1, int8_t *h2, int8_t *h3,
                  uint16_t *adc12,
				  DHT11_Status *d1, DHT11_Status *d2, DHT11_Status *d3);


#endif /* INC_SENSORS_H_ */
