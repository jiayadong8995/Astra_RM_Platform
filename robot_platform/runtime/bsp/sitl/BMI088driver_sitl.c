#include "../../device/imu/bmi088/BMI088driver.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

static int imu_sock = -1;
IMU_Data_t BMI088;

void BMI088_Init(SPI_HandleTypeDef *bmi088_SPI, uint8_t calibrate) {
    (void)bmi088_SPI;
    (void)calibrate;
    
    imu_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (imu_sock >= 0) {
        int flags = fcntl(imu_sock, F_GETFL, 0);
        fcntl(imu_sock, F_SETFL, flags | O_NONBLOCK);
        
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(9001); // IMU port
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        bind(imu_sock, (struct sockaddr *)&addr, sizeof(addr));
    }
}

void BMI088_Read(IMU_Data_t *bmi088) {
    if (!bmi088) return;

    if (imu_sock >= 0) {
        float buf[7];
        int n;
        int received = 0;
        // Non-blocking recv loop to get the latest telemetry packet
        while ((n = recv(imu_sock, buf, sizeof(buf), 0)) == sizeof(buf)) {
            received = 1;
        }
        
        if (received) {
            bmi088->Gyro[0] = buf[0];
            bmi088->Gyro[1] = buf[1];
            bmi088->Gyro[2] = buf[2];
            bmi088->Accel[0] = buf[3];
            bmi088->Accel[1] = buf[4];
            bmi088->Accel[2] = buf[5];
            bmi088->Temperature = buf[6];
        } else if (bmi088->Accel[2] == 0.0f) {
            // First time init default if no data received yet
            bmi088->Accel[2] = 9.81f;
        }
    } else {
        bmi088->Accel[2] = 9.81f; 
    }
}

uint8_t bmi088_accel_init(void) { return 0; }
uint8_t bmi088_gyro_init(void) { return 0; }
void BMI088_Delay_ms(uint16_t ms) { (void)ms; }
void BMI088_Delay_us(uint16_t us) { (void)us; }
