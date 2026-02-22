#ifndef NEO_M8N_H
#define NEO_M8N_H

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define GPS_BUF_SIZE  256

typedef struct {
    float    latitude;
    float    longitude;
    float    altitude;
    float    speed;
    uint8_t  fix;
    uint8_t  satellites;
    char     time[10];
} GPS_Data;

// Interrupt'tan ana koda veri aktarımı için flag
extern volatile uint8_t gps_data_ready;
extern GPS_Data gps_parsed;

void GPS_Init(UART_HandleTypeDef *huart);
void GPS_UART_IRQHandler(UART_HandleTypeDef *huart);  // callback içinde çağrılacak
void GPS_GetLatest(GPS_Data *data);

#endif
