#include "main.h"
#include "usart.h"
#include "soil.h"
float ST = 0;
float SM = 0;
float PH = 0;
float N = 0;
float P = 0;
float K = 0;

void read_soil_sensor_data() {
    uint8_t command[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x0B, 0x04, 0x0D}; // 查询命令
    uint8_t data[27]; // 接收数据缓冲区

    // 发送命令
    HAL_UART_Transmit(&huart1, command, sizeof(command), 100);

    // 延时等待传感器回复
    HAL_Delay(300);

    // 接收传感器数据
    if (HAL_UART_Receive(&huart1, data, 27, 500) == HAL_OK) {
        // 提取土壤温度、湿度、PH值、氮、磷、钾数据
        ST = ((data[3] << 8) | data[4]) / 10.0;
        SM = ((data[5] << 8) | data[6]) / 10.0;
        PH = ((data[9] << 8) | data[10]) / 10.0;
        N = (data[11] << 8) | data[12];
        P = (data[13] << 8) | data[14];
        K = (data[15] << 8) | data[16];
    } else {
        // 接收失败处理
        printf("Failed to receive sensor data.\n");
    }
		printf("ST=%f,SM=%f,PH=%f,N=%f,P=%f,K=%f",ST,SM,PH,N,P,K);
    // 延时以避免过于频繁地读取传感器数据
    HAL_Delay(100);
}
