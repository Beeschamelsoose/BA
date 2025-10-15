#ifndef SAVE_H
#define SAVE_H

#include "stm32l4xx_hal.h"
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

// Flash-Speicherlayout für Logger (sichtbar für andere Module)
#define FLASH_PAGE_SIZE     2048u
#define FLASH_BASE_ADDR     0x08000000u
#define LOG_START_ADDR      0x0801F800u  // Startadresse des Logbereichs (anpassen falls nötig)
#define LOG_TOTAL_BYTES     (128 * 1024u - (LOG_START_ADDR - FLASH_BASE_ADDR))
#define LOG_MAX_PAGES       (LOG_TOTAL_BYTES / FLASH_PAGE_SIZE)
#define RECORD_SIZE         ((uint32_t)sizeof(DataRecord))
#define RECORDS_PER_PAGE    (FLASH_PAGE_SIZE / RECORD_SIZE)

#pragma pack(push,1)
typedef struct {
    uint32_t timestamp;   // 4 Byte
    int8_t   temp[3];     // 3 Byte
    int8_t   hum[3];      // 3 Byte
    uint16_t adc_state;   // 12 Bit ADC + 4 Bit Status
} DataRecord;
#pragma pack(pop)

/* Initialisierung: nur RAM zurücksetzen */
void Save_Init(void);

/* Optional: gesamten Speicherbereich löschen */
void Save_WipeAll(void);

/* Datensatz anhängen (RAM -> bei 2kB flush ins Flash) */
HAL_StatusTypeDef Save_Append(const DataRecord *rec);

/* Komfortfunktion: baut Record aus Einzelwerten */
HAL_StatusTypeDef save_push_from_main_ts(uint8_t t1, uint8_t t2, uint8_t t3,
                                         uint8_t h1, uint8_t h2, uint8_t h3,
                                         uint16_t adc12, uint8_t st,
                                         uint8_t reserved, uint32_t ts);

/* RAM-Puffer manuell schreiben (wenn nicht voll) */
HAL_StatusTypeDef Save_Flush(void);

/* Inhalt über eine UART ausgeben */
void Save_Dump(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif
#endif
