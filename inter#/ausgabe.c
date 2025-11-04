/*
 * delay_us.c
 *
 *  Created on: Okt 1, 2025
 *      Author: ernst
 */


#include "ausgabe.h"
#include "save.h"
#include <string.h>
#include <stdio.h>

#define RX_BUFFER_LEN   32
#define CMD_KEYWORD     "save"

static UART_HandleTypeDef *g_huart1;
static uint8_t  rx_buf[RX_BUFFER_LEN];
static uint8_t  rx_index = 0;
static uint8_t  rx_byte;
static uint8_t  cmd_received = 0;

static char     tx_buf[128];
static uint8_t  dma_busy = 0;

static uint32_t cur_page = 0;
static uint32_t cur_rec  = 0;

volatile uint8_t ausgabe_aktiv = 0;

/* ---------- Initialisierung ---------- */
void Ausgabe_Init(UART_HandleTypeDef *huart1)
{
    g_huart1 = huart1;
    rx_index = 0;
    cmd_received = 0;
    dma_busy = 0;
    memset(rx_buf, 0, sizeof(rx_buf));

    /* Start RX Interrupt */
    HAL_UART_Receive_IT(g_huart1, &rx_byte, 1);
}

/* ---------- RX Callback ---------- */
void Ausgabe_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != g_huart1) return;

    // Debug optional – zeig empfangene Zeichen
     HAL_UART_Transmit(g_huart1, &rx_byte, 1, 10);

    // Bei Zeilenende (CR oder LF)
    if (rx_byte == '\r' || rx_byte == '\n')
    {
        // Nur wenn wirklich Text im Buffer ist
        if (rx_index > 0)
        {
            rx_buf[rx_index] = 0; // Nullterminierung

            // Prüfe Kommando (Groß-/Kleinschreibung ignorieren)
            if (strcasecmp((char*)rx_buf, CMD_KEYWORD) == 0) {
                cmd_received = 1;
                const char *ack = "\r\nKommando erkannt: save\r\n";
                HAL_UART_Transmit(g_huart1, (uint8_t*)ack, strlen(ack), 100);
            } else {
                // Unbekanntes Kommando – Rückmeldung
                char msg[64];
                snprintf(msg, sizeof(msg),
                         "\r\nUnbekanntes Kommando: \"%s\"\r\n", rx_buf);
                HAL_UART_Transmit(g_huart1, (uint8_t*)msg, strlen(msg), 100);
            }

            // Buffer zurücksetzen
            rx_index = 0;
            memset(rx_buf, 0, sizeof(rx_buf));
        }
    }
    else
    {
        // Normales Zeichen in den Buffer schreiben
        if (rx_index < RX_BUFFER_LEN - 1)
            rx_buf[rx_index++] = rx_byte;
    }

    // Nächstes Byte per Interrupt empfangen
    HAL_UART_Receive_IT(g_huart1, &rx_byte, 1);
}


/* ---------- TX Callback ---------- */
void Ausgabe_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != g_huart1) return;
    dma_busy = 0;
}

/* ---------- DMA Send Helper ---------- */
static void uart1_send_dma(const char *text)
{
    if (!text || dma_busy) return;
    size_t len = strlen(text);
    if (len == 0) return;

    dma_busy = 1;
    HAL_UART_Transmit_DMA(g_huart1, (uint8_t*)text, len);
}

/* ---------- Hauptprozess ---------- */
void Ausgabe_Process(void)
{
    static enum {IDLE, HEADER,WAIT_HEADER,DUMPING, FOOTER} state = IDLE;

    if (cmd_received && state == IDLE)
    {
        cmd_received = 0;
        cur_page = 0;
        cur_rec  = 0;

        ausgabe_aktiv=1;

        // Zähle, wie viele gültige Datensätze vorhanden sind
        uint32_t total_records = 0;
        // Anzahl Datensätze Zählen
        for (uint32_t p = 0; p < LOG_MAX_PAGES; p++) {
            uint32_t pageAddr = LOG_START_ADDR + p * FLASH_PAGE_SIZE;
            const DataRecord *rec = (const DataRecord *)pageAddr;

            if (*(uint32_t*)pageAddr == 0xFFFFFFFF) break;

            for (uint32_t r = 0; r < RECORDS_PER_PAGE; r++) {
                if (rec[r].timestamp == 0xFFFFFFFF) break;
                total_records++;
            }
        }

        // Header mit Datensatzanzahl und CSV-Spaltenüberschrift

        int n = snprintf(tx_buf, sizeof(tx_buf),
            "\r\nKommando erkannt: save\r\n"
            "Datensaetze im Speicher: %lu\r\n"
            "--- CSV Dump ---\r\n"
            "timestamp,t1,t2,t3,h1,h2,h3,adc,sb0,sb1,sb2,sb3\r\n",
            (unsigned long)total_records);

        if (n < 0) n = 0;
        if ((size_t)n > sizeof(tx_buf)) n = sizeof(tx_buf);
        uart1_send_dma(tx_buf);
        state = WAIT_HEADER;

        return;
    }
    if (state==WAIT_HEADER){
    	if(!dma_busy){
    		state = DUMPING;
    	}else{
    		return;
    	}
    }

    if (dma_busy) return; // warte bis DMA fertig

    switch (state)
    {
        case HEADER:
            state = DUMPING;
            break;

        case DUMPING: {
            const DataRecord *rec = NULL;
            char line[160];

            if (cur_page >= LOG_MAX_PAGES) {
                state = FOOTER;
                break;
            }

            uint32_t pageAddr = LOG_START_ADDR + cur_page * FLASH_PAGE_SIZE;
            rec = (const DataRecord *)pageAddr;

            if (*(uint32_t*)pageAddr == 0xFFFFFFFF) {
                state = FOOTER;
                break;
            }

            if (rec[cur_rec].timestamp == 0xFFFFFFFF) {
                cur_page++;
                cur_rec = 0;
                break;
            }

            // ADC (untere 12 Bit), State (obere 4 Bit)
            uint16_t adc12 = rec[cur_rec].adc_state & 0x0FFFu;
            uint8_t  st    = (rec[cur_rec].adc_state >> 12) & 0x0Fu;

            uint8_t s0 = (st >> 0) & 1u;
            uint8_t s1 = (st >> 1) & 1u;
            uint8_t s2 = (st >> 2) & 1u;
            uint8_t s3 = (st >> 3) & 1u;

            int ln = snprintf(tx_buf, sizeof(tx_buf),
                "%lu,%d,%d,%d,%d,%d,%d,%u,%u,%u,%u,%u\r\n",
                (unsigned long)rec[cur_rec].timestamp,
                rec[cur_rec].temp[0], rec[cur_rec].temp[1], rec[cur_rec].temp[2],
                rec[cur_rec].hum[0],  rec[cur_rec].hum[1],  rec[cur_rec].hum[2],
                adc12, s0, s1, s2, s3);

            if (ln < 0) ln = 0;
            if ((size_t)ln > sizeof(tx_buf)) ln = sizeof(tx_buf);
            uart1_send_dma(tx_buf);

            cur_rec++;
            break;
        }

        case FOOTER:
            uart1_send_dma("--- Ende ---\r\n");
            state = IDLE;
            ausgabe_aktiv=0;
            break;

        default:
            break;
    }
}

