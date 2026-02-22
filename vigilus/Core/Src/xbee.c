#include "xbee.h"

static char tx_buf[XBEE_BUF_SIZE];

void XBee_SendPacket(UART_HandleTypeDef *huart,
                     MPU9250_Data *imu,
                     BMP280_Data  *bmp,
                     GPS_Data     *gps)
{
    int len = snprintf(tx_buf, XBEE_BUF_SIZE,
        "$AX:%.2f,AY:%.2f,AZ:%.2f,"
        "GX:%.1f,GY:%.1f,GZ:%.1f,"
        "T:%.1f,P:%.1f,ALT:%.1f,"
        "LAT:%.5f,LON:%.5f,FIX:%d*\r\n",
        imu->accel_x, imu->accel_y, imu->accel_z,
        imu->gyro_x,  imu->gyro_y,  imu->gyro_z,
        bmp->temperature, bmp->pressure, bmp->altitude,
        gps->latitude, gps->longitude, gps->fix
    );

    HAL_UART_Transmit(huart, (uint8_t*)tx_buf, len, HAL_MAX_DELAY);
}
