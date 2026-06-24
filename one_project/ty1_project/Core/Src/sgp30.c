#include "sgp30.h"
#include "i2c.h"

#define SGP30_ADDR (0x58 << 1)

// Initialize SGP30 sensor
HAL_StatusTypeDef sgp30_init() {
    uint8_t data[2];

    // Send iaq init command
    data[0] = (0x2003) >> 8;
    data[1] = (0x2003) & 0xFF;

    return HAL_I2C_Master_Transmit(&hi2c1, SGP30_ADDR, data, 2, 50);
}

// Get data sample
HAL_StatusTypeDef sgp30_sample(float *co2, float *tvoc) {
    uint8_t data[2];
    uint8_t readbuff[6];

    // Send measure iaq command
    data[0] = (0x2008) >> 8;
    data[1] = (0x2008) & 0xFF;

    if (HAL_I2C_Master_Transmit(&hi2c1, SGP30_ADDR, data, 2, 50) != HAL_OK) {
        printf("I2C transmit failed during measurement command\r\n");
        return HAL_ERROR;
    }

    // Wait for data availability
    int timeout = 10;
    while (1) {
        if (HAL_I2C_Master_Receive(&hi2c1, SGP30_ADDR | 0x01, readbuff, 6, 50) != HAL_OK) {
            timeout--;
            HAL_Delay(12);  // Ensure adequate delay per datasheet
        } else {
            break;
        }

        if (timeout <= 0) {
            printf("Timeout while waiting for data\r\n");
            return HAL_ERROR;
        }
    }

    // Check CRC  ÓÃ²»µ½
//    if (CheckCrc8(readbuff, 2) != readbuff[2] || CheckCrc8(&readbuff[3], 2) != readbuff[5]) {
//        SEGGER_RTT_printf(0, "CRC check failed\r\n");
//        return HAL_ERROR;
//    }

    // Parse data
    *co2 = (readbuff[0] << 8) | readbuff[1];
    *tvoc = (readbuff[3] << 8) | readbuff[4];

    return HAL_OK;
}
