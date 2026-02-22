#include "mpu9250.h"

static HAL_StatusTypeDef WriteReg(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    return HAL_I2C_Master_Transmit(hi2c, MPU9250_ADDR, buf, 2, HAL_MAX_DELAY);
}

static HAL_StatusTypeDef ReadRegs(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *buf, uint16_t len) {
    if (HAL_I2C_Master_Transmit(hi2c, MPU9250_ADDR, &reg, 1, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;
    return HAL_I2C_Master_Receive(hi2c, MPU9250_ADDR, buf, len, HAL_MAX_DELAY);
}

HAL_StatusTypeDef MPU9250_Init(I2C_HandleTypeDef *hi2c) {
    WriteReg(hi2c, MPU9250_PWR_MGMT_1, 0x00);
    HAL_Delay(100);
    WriteReg(hi2c, 0x1C, 0x00);
    WriteReg(hi2c, 0x1B, 0x00);
    return HAL_OK;
}
HAL_StatusTypeDef MPU9250_ReadAll(I2C_HandleTypeDef *hi2c, MPU9250_Data *data) {
    uint8_t buf[14];
    if (ReadRegs(hi2c, MPU9250_ACCEL_XOUT_H, buf, 14) != HAL_OK) return HAL_ERROR;

    int16_t raw_ax = (int16_t)(buf[0]  << 8 | buf[1]);
    int16_t raw_ay = (int16_t)(buf[2]  << 8 | buf[3]);
    int16_t raw_az = (int16_t)(buf[4]  << 8 | buf[5]);
    int16_t raw_t  = (int16_t)(buf[6]  << 8 | buf[7]);
    int16_t raw_gx = (int16_t)(buf[8]  << 8 | buf[9]);
    int16_t raw_gy = (int16_t)(buf[10] << 8 | buf[11]);
    int16_t raw_gz = (int16_t)(buf[12] << 8 | buf[13]);

    data->accel_x = raw_ax / 16384.0f;
    data->accel_y = raw_ay / 16384.0f;
    data->accel_z = raw_az / 16384.0f;
    data->gyro_x  = raw_gx / 131.0f;
    data->gyro_y  = raw_gy / 131.0f;
    data->gyro_z  = raw_gz / 131.0f;
    data->temp    = (raw_t / 333.87f) + 21.0f;

    return HAL_OK;
}
