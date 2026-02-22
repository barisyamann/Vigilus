#include "neo_m8n.h"

static UART_HandleTypeDef *gps_huart;
static uint8_t  rx_byte;
static char     rx_buf[GPS_BUF_SIZE];
static uint16_t rx_idx = 0;
static uint8_t  in_sentence = 0;

volatile uint8_t gps_data_ready = 0;
GPS_Data gps_parsed = {0};

static void Parse_GGA(char *line) {
    if (strncmp(line, "$GNGGA", 6) != 0 && strncmp(line, "$GPGGA", 6) != 0)
        return;

    char tmp[GPS_BUF_SIZE];
    strncpy(tmp, line, GPS_BUF_SIZE - 1);

    char *tok = strtok(tmp, ",");
    int field = 0;
    float raw_lat = 0, raw_lon = 0;
    char lat_dir = 'N', lon_dir = 'E';

    while (tok != NULL) {
        switch (field) {
            case 1: strncpy(gps_parsed.time, tok, 9); break;
            case 2: raw_lat = atof(tok); break;
            case 3: lat_dir = tok[0]; break;
            case 4: raw_lon = atof(tok); break;
            case 5: lon_dir = tok[0]; break;
            case 6: gps_parsed.fix        = atoi(tok); break;
            case 7: gps_parsed.satellites = atoi(tok); break;
            case 9: gps_parsed.altitude   = atof(tok); break;
        }
        field++;
        tok = strtok(NULL, ",");
    }

    int lat_deg = (int)(raw_lat / 100);
    gps_parsed.latitude = lat_deg + (raw_lat - lat_deg * 100) / 60.0f;
    if (lat_dir == 'S') gps_parsed.latitude *= -1;

    int lon_deg = (int)(raw_lon / 100);
    gps_parsed.longitude = lon_deg + (raw_lon - lon_deg * 100) / 60.0f;
    if (lon_dir == 'W') gps_parsed.longitude *= -1;

    gps_data_ready = 1;
}

void GPS_Init(UART_HandleTypeDef *huart) {
    gps_huart = huart;
    // İlk interrupt başlat
    HAL_UART_Receive_IT(gps_huart, &rx_byte, 1);
}

// Her byte geldiğinde bu fonksiyon çağrılır
void GPS_UART_IRQHandler(UART_HandleTypeDef *huart) {
    if (huart->Instance != gps_huart->Instance) return;

    char c = (char)rx_byte;

    if (c == '$') {
        // Yeni cümle başlangıcı
        rx_idx = 0;
        in_sentence = 1;
    }

    if (in_sentence) {
        if (c == '\n') {
            rx_buf[rx_idx] = '\0';
            Parse_GGA(rx_buf);
            rx_idx = 0;
            in_sentence = 0;
        } else if (rx_idx < GPS_BUF_SIZE - 1) {
            rx_buf[rx_idx++] = c;
        }
    }

    // Bir sonraki byte için interrupt'ı yeniden başlat
    HAL_UART_Receive_IT(gps_huart, &rx_byte, 1);
}

void GPS_GetLatest(GPS_Data *data) {
    *data = gps_parsed;
    gps_data_ready = 0;
}
