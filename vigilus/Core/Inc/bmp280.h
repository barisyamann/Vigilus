#ifndef BMP280_H
#define BMP280_H

#include "stm32f4xx_hal.h"

// GY-91 üzerindeki BMP280 adresi
#define BMP280_ADDR  (0x76 << 1)
#define BMP280_ID    0x60

typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2, dig_P3, dig_P4, dig_P5;
    int16_t  dig_P6, dig_P7, dig_P8, dig_P9;
    int32_t  t_fine;
} BMP280_Calib;

typedef struct {
    float temperature;  // °C
    float pressure;     // hPa
    float altitude;     // metre
} BMP280_Data;

HAL_StatusTypeDef BMP280_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BMP280_Read(I2C_HandleTypeDef *hi2c, BMP280_Data *data);

#endif
