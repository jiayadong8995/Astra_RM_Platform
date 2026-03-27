#include "BMI088driver.h"
#include <string.h>

IMU_Data_t BMI088;

void BMI088_Init(SPI_HandleTypeDef *bmi088_SPI, uint8_t calibrate) {
    (void)bmi088_SPI;
    (void)calibrate;
    memset(&BMI088, 0, sizeof(BMI088));
    BMI088.AccelScale = 1.0f;
    BMI088.gNorm = 9.81f;
    BMI088.Accel[2] = 9.81f; // Simulated gravity
}

uint8_t BMI088_init(SPI_HandleTypeDef *bmi088_SPI, uint8_t calibrate) {
    BMI088_Init(bmi088_SPI, calibrate);
    return 0; // BMI088_NO_ERROR
}

uint8_t bmi088_accel_init(void) {
    return 0;
}

uint8_t bmi088_gyro_init(void) {
    return 0;
}

void BMI088_Read(IMU_Data_t *bmi088) {
    if (bmi088) {
        // Default to zero state in dummy stub
        // Sim physics will override this struct directly or via socket later
        bmi088->Accel[2] = 9.81f; 
    }
}
