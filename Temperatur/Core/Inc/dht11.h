#pragma once
#include "stm32l4xx_hal.h"
#include <stdint.h>

/* Statuscodes zuerst definieren */
typedef enum {
  DHT11_OK = 0,
  DHT11_ERR_PRES_HI  = -11, // blieb vor Antwort high
  DHT11_ERR_PRES_LO  = -12, // erstes low zu lang
  DHT11_ERR_PRES_HI2 = -13, // high nach erstem low zu lang
  DHT11_ERR_BIT_TO   = -14, // Timeout in Bit-Phase
  DHT11_ERR_CSUM     = -20  // Checksumme falsch
} DHT11_Status;

/* Handle-Struktur */
typedef struct {
    GPIO_TypeDef* GPIOx;
    uint16_t      GPIO_Pin;
} DHT11_HandleTypeDef;

/* Öffentliche API */
int DHT11_Presence(GPIO_TypeDef* port, uint16_t pin);
DHT11_Status DHT11_Read(DHT11_HandleTypeDef* dht, float* temperature, uint8_t* humidity);

extern volatile int16_t dht11_fail_bit;
extern volatile int8_t  dht11_fail_stage;
