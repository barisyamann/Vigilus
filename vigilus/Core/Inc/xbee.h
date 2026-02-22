#ifndef XBEE_H
#define XBEE_H

#include "stm32f4xx_hal.h"
#include "mpu9250.h"
#include "bmp280.h"
#include "neo_m8n.h"
#include <stdio.h>

#define XBEE_BUF_SIZE  200

// XCTU'da görünecek paket formatı
// $AX:%.2f,AY:%.2f,AZ:%.2f,GX:%.1f,GY:%.1f,GZ:%.1f,T:%.1f,P:%.1f,ALT:%.1f,LAT:%.5f,LON:%.5f,FIX:%d*\r\n

void XBee_SendPacket(UART_HandleTypeDef *huart,
                     MPU9250_Data *imu,
                     BMP280_Data  *bmp,
                     GPS_Data     *gps);

#endif
