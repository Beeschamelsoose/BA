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

    HAL_UART_Receive_IT(g_huart1, &rx_byte, 1);
}

/* ---------- RX Callback ---------- */
void Ausgabe_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != g_huart1) return;

    // Debug optional – zeig empfangene Zeichen
    HAL_UART_Transmit(g_huart1, &rx_byte, 1, 10);

    if (rx_byte == '\r' || rx_byte == '\n')
    {
        if (rx_index > 0)
        {
            rx_buf[rx_index] = 0;

            if (strcasecmp((char*)rx_buf, CMD_KEYWORD) == 0) {
                cmd_received = 1;
                const char *ack = "\r\nKommando erkannt: save\r\n";
                HAL_UART_Transmit(g_huart1, (uint8_t*)ack, strlen(ack), 100);
            } else {
                char msg[64];
                snprintf(msg, sizeof(msg),
                         "\r\nUnbekanntes Kommando: \"%s\"\r\n", rx_buf);
                HAL_UART_Transmit(g_huart1, (uint8_t*)msg, strlen(msg), 100);
            }

            rx_index = 0;
            memset(rx_buf, 0, sizeof(rx_buf));
        }
    }
    else
    {
        if (rx_index < RX_BUFFER_LEN - 1)
            rx_buf[rx_index++] = rx_byte;
    }

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
    static enum {IDLE, HEADER, WAIT_HEADER, DUMPING, FOOTER} state = IDLE;

    if (cmd_received && state == IDLE)
    {
        cmd_received = 0;
        cur_page = Save_GetStartPageIndex();
        cur_rec  = 0;
        ausgabe_aktiv = 1;

        /* Aktuelle Session-Flag und Gesamtzahl zählen */
        uint16_t flag = Save_GetSessionFlag();
        uint32_t num_pages = Save_GetNumPages();
        uint32_t total_records = 0;

        for (uint32_t p = 0; p < num_pages; p++) {
            uint16_t magic = 0, f = 0;
            Save_ReadPageHeader(p, &magic, &f);
            if (!(magic == 0xA55A && f == flag)) continue;

            for (uint32_t r = 0; r < Save_GetRecordsPerPage(); r++) {
                DataRecord rec;
                Save_ReadRecord(p, r, &rec);
                if (Save_RecordIsEmpty(&rec)) break;
                total_records++;
            }
        }

        int n = snprintf(tx_buf, sizeof(tx_buf),
            "Datensaetze im Speicher: %lu\r\n"
            "--- CSV Dump ---\r\n"
            "timestamp,t in s,t1,t2,t3,h1,h2,h3,adc,sb0,sb1,sb2,sb3\r\n",
            (unsigned long)total_records);

        if (n < 0) n = 0;
        if ((size_t)n > sizeof(tx_buf)) n = sizeof(tx_buf);
        uart1_send_dma(tx_buf);
        state = WAIT_HEADER;
        return;
    }

    if (state == WAIT_HEADER) {
        if (!dma_busy) state = DUMPING;
        else return;
    }

    if (dma_busy) return;

    switch (state)
    {
        case DUMPING: {
            const uint16_t flag = Save_GetSessionFlag();
            const uint32_t num_pages = Save_GetNumPages();
            const uint32_t recs_per_page = Save_GetRecordsPerPage();
            uint16_t magic = 0, f = 0;

            while (cur_page < num_pages) {
                Save_ReadPageHeader(cur_page, &magic, &f);
                if (!(magic == 0xA55A && f == flag)) {
                    state = FOOTER;
                    break;
                }

                DataRecord rec;
                if (Save_ReadRecord(cur_page, cur_rec, &rec) != 0) {
                    state = FOOTER;
                    break;
                }
                if (Save_RecordIsEmpty(&rec)) {
                    cur_rec = 0;
                    cur_page++;
                    continue;
                }

                uint16_t adc_state = rec.adc_state;
                uint16_t adc12 = adc_state & 0x0FFFu;
                uint8_t st = (adc_state >> 12) & 0x0Fu;

                uint8_t s0 = (st >> 0) & 1u;
                uint8_t s1 = (st >> 1) & 1u;
                uint8_t s2 = (st >> 2) & 1u;
                uint8_t s3 = (st >> 3) & 1u;
                //Umrechnen Zeit in s, für Diagramme
                float time_s = rec.timestamp/1000.0f;
                snprintf(tx_buf, sizeof(tx_buf),
                    "%lu,%.0f,%d,%d,%d,%d,%d,%d,%u,%u,%u,%u,%u\r\n",
                    (unsigned long)rec.timestamp,time_s,
                    rec.temp[0], rec.temp[1], rec.temp[2],
                    rec.hum[0],  rec.hum[1],  rec.hum[2],
                    adc12, s0, s1, s2, s3);

                uart1_send_dma(tx_buf);
                cur_rec++;
                return;  // warte bis DMA fertig → nächste Zeile
            }

            state = FOOTER;
            break;
        }

        case FOOTER:
            uart1_send_dma("--- Ende ---\r\n");
            state = IDLE;
            ausgabe_aktiv = 0;
            break;

        default:
            break;
    }
}
