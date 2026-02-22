#include "bmp280.h"
#include <math.h>

static BMP280_Calib calib;

static HAL_StatusTypeDef ReadRegs(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *buf, uint16_t len) {
    if (HAL_I2C_Master_Transmit(hi2c, BMP280_ADDR, &reg, 1, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;
    return HAL_I2C_Master_Receive(hi2c, BMP280_ADDR, buf, len, HAL_MAX_DELAY);
}

static HAL_StatusTypeDef WriteReg(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    return HAL_I2C_Master_Transmit(hi2c, BMP280_ADDR, buf, 2, HAL_MAX_DELAY);
}

static void LoadCalib(I2C_HandleTypeDef *hi2c) {
    uint8_t buf[24];
    if (ReadRegs(hi2c, 0x88, buf, 24) != HAL_OK) return;
    ReadRegs(hi2c, 0x88, buf, 24);

    calib.dig_T1 = (uint16_t)(buf[1]  << 8 | buf[0]);
    calib.dig_T2 = (int16_t) (buf[3]  << 8 | buf[2]);
    calib.dig_T3 = (int16_t) (buf[5]  << 8 | buf[4]);
    calib.dig_P1 = (uint16_t)(buf[7]  << 8 | buf[6]);
    calib.dig_P2 = (int16_t) (buf[9]  << 8 | buf[8]);
    calib.dig_P3 = (int16_t) (buf[11] << 8 | buf[10]);
    calib.dig_P4 = (int16_t) (buf[13] << 8 | buf[12]);
    calib.dig_P5 = (int16_t) (buf[15] << 8 | buf[14]);
    calib.dig_P6 = (int16_t) (buf[17] << 8 | buf[16]);
    calib.dig_P7 = (int16_t) (buf[19] << 8 | buf[18]);
    calib.dig_P8 = (int16_t) (buf[21] << 8 | buf[20]);
    calib.dig_P9 = (int16_t) (buf[23] << 8 | buf[22]);
}

HAL_StatusTypeDef BMP280_Init(I2C_HandleTypeDef *hi2c) {
    // WHO_AM_I kontrolü kaldırıldı
    LoadCalib(hi2c);
    WriteReg(hi2c, 0xF4, 0x27);
    WriteReg(hi2c, 0xF5, 0x00);
    return HAL_OK;
}

HAL_StatusTypeDef BMP280_Read(I2C_HandleTypeDef *hi2c, BMP280_Data *data) {
    uint8_t buf[6];
    if (ReadRegs(hi2c, 0xF7, buf, 6) != HAL_OK) return HAL_ERROR;

    int32_t adc_P = (int32_t)((buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4));
    int32_t adc_T = (int32_t)((buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4));

    // Sıcaklık hesabı (Bosch datasheet compensation formulas)
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1)))
                    * ((int32_t)calib.dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1))
                    * ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12)
                    * ((int32_t)calib.dig_T3)) >> 14;
    calib.t_fine = var1 + var2;
    data->temperature = (float)((calib.t_fine * 5 + 128) >> 8) / 100.0f;

    // Basınç hesabı
    int64_t p1 = ((int64_t)calib.t_fine) - 128000;
    int64_t p2 = p1 * p1 * (int64_t)calib.dig_P6;
    p2 = p2 + ((p1 * (int64_t)calib.dig_P5) << 17);
    p2 = p2 + (((int64_t)calib.dig_P4) << 35);
    p1 = ((p1 * p1 * (int64_t)calib.dig_P3) >> 8) + ((p1 * (int64_t)calib.dig_P2) << 12);
    p1 = (((((int64_t)1) << 47) + p1)) * ((int64_t)calib.dig_P1) >> 33;
    if (p1 == 0) return HAL_ERROR;
    int64_t p = 1048576 - adc_P;
    p = (((p << 31) - p2) * 3125) / p1;
    p1 = (((int64_t)calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    p2 = (((int64_t)calib.dig_P8) * p) >> 19;
    p = ((p + p1 + p2) >> 8) + (((int64_t)calib.dig_P7) << 4);
    data->pressure = (float)p / 25600.0f;

    // İrtifa (deniz seviyesi 1013.25 hPa referans)
    data->altitude = 44330.0f * (1.0f - powf(data->pressure / 1013.25f, 0.1903f));

    return HAL_OK;
}
