#include "dht11.h"
#include "gpio.h"
#include "delay.h"

#define DHT_HIGH HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET)
#define DHT_LOW HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET)
#define DHT_VALUE HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11)

uint8_t datas[5];



void DHT_GPIO_Init(uint32_t mode)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = mode;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void DHT11_Start()
{
	DHT_GPIO_Init(GPIO_MODE_OUTPUT_PP);
	DHT_HIGH;
	DHT_LOW;
	HAL_Delay(30);
	DHT_HIGH;
	
	DHT_GPIO_Init(GPIO_MODE_INPUT);
	while(DHT_VALUE);
	while(!DHT_VALUE);
	while(DHT_VALUE);
}

void Read_Data_From_DHT(void)
{
	int i;//轮
	int j;//每一轮读多少次
	char tmp;
	char flag;
	
	DHT11_Start();
	DHT_GPIO_Init(GPIO_MODE_INPUT);
	for(i= 0;i < 5;i++){
		//卡g点：while(!dht)       有效数据都是高电平，持续时间不一样，50us读，低电平0 高电平
		for(j=0;j<8;j++){
			while(!DHT_VALUE);//等待卡g点
			delay_us(40);
			if(DHT_VALUE == 1){
				flag = 1;
				while(DHT_VALUE);
			}else{
				flag = 0;
			} 
			tmp = tmp << 1;
			tmp |= flag;
		}
		datas[i] = tmp;
	}
}
