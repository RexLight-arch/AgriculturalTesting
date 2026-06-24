/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* USER CODE END Header */ 
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "dht11.h"
#include "esp8266.h"
#include "onenet.h"
#include "delay.h"
#include "sgp30.h"
#include "math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern float lux;//光照强度值
float ST ;  // 土壤温度
float SM ;  // 土壤湿度
float PH ;  // 土壤 PH 值
float N ;   // 氮含量
float P ;   // 磷含量
float K ;   // 钾含量
uint8_t soilSensorData[25];// 接收土壤传感器返回数据的缓冲区
uint8_t command[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x0A, 0xC5, 0xCD};
double air_q=0.0;//空气质量

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
void co2_get(char* co2str,char* tvocstr)
{
	float co2;
	float tvoc;
	if( sgp30_sample(&co2, &tvoc) != HAL_OK){
			printf( "sgp30 samle error\r\n");
		}
	sprintf(co2str, "%.2f", co2);
	sprintf(tvocstr, "%.2f", tvoc);
	printf("co2:%s, tvoc:%s\r\n", co2str, tvocstr);
	
}
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
void StartUART3Receive(void) {
    // 启动中断接收，接收 25 字节的数据
    HAL_UART_Receive_IT(&huart3, soilSensorData, sizeof(soilSensorData)); 
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	char co2str[6] = {0};
	char tvocstr[6] = {0};
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
	StartUART3Receive();
	BH1750_Init();
	if(sgp30_init() != HAL_OK){
      printf( "sgp30 init error\r\n");
	}else{
			printf("sgp30 init ok\r\n");
	}
	HAL_Delay(1000);
	esp8266_init(115200);
  printf("hello world!\r\n");
    
  printf("MQTT初始化...\r\n");
  mqtt_init();
    
  printf("MQTT连接...\r\n");
  mqtt_connect(MQTT_ClientID, MQTT_UserName, MQTT_PassWord);

  uint8_t data_send[512] = {0};
  uint8_t dht11_data[4] = {0};
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		 co2_get(co2str,tvocstr);//二氧化碳浓度读取
		 memset(dht11_data, 0, 4);
     dht11_read(dht11_data);//温湿度读取
		 start_read_BH1750();//光照强度监测
		 HAL_ADC_Start(&hadc1);
		 air_q=HAL_ADC_GetValue(&hadc1);//空气质量采集
		 HAL_ADC_Stop(&hadc1);
		 float vol=(float)air_q*(3.3/4096);//电压值   
     double RS=(5-vol)/vol*0.5;
     double R0=6.64;
     double ppm=pow(11.5428*R0/RS,0.6549f)*100; 
		 HAL_Delay(100);
		 HAL_UART_Transmit(&huart3, command, sizeof(command), HAL_MAX_DELAY);//soil,发送指令
     sprintf((char *)data_send, "{ \"AEID\": 1,\"LI\": %lf, \"PH\": %.2f,\"ST\": %.2f,\"SM\": %.2f,\"N\": %.2f,\"P\": %.2f,\"K\": %.2f,\"AT\": %d.%d, \"AH\": %d.%d,\"AQ\": %.2lf,\"CO2\": %s, \"CH4O\": %s}",
            lux,PH,ST,SM,N,P,K,dht11_data[2], dht11_data[3], dht11_data[0], dht11_data[1],ppm,co2str,tvocstr);
     //printf("%s\n",data_send);   
     mqtt_publish_data(POST_TOPIC, (char *)data_send, 0);
        
     delay_ms(3000);
        
     //printf("\r\n~~~~~~~~~~~~~~~~~发送心跳包~~~~~~~~~~~~~~~~~\r\n");
     mqtt_send_heart();
     //printf("\r\n~~~~~~~~~~~~~~~~~发送心跳包结束~~~~~~~~~~~~~~~~~\r\n");
	 

     HAL_Delay(3000);  // 延迟 30秒
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
