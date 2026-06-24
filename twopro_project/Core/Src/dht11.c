#include "dht11.h"
#include "delay.h"
#include "string.h"
#include "stdio.h"

char dht11_data[5] = {0};

void dht11_gpio_input(void)
{
    GPIO_InitTypeDef gpio_initstruct;
    DHT11_CLK_ENABLE();                         
    
    gpio_initstruct.Pin = DHT11_PIN;        
    gpio_initstruct.Mode = GPIO_MODE_INPUT;           
    gpio_initstruct.Speed = GPIO_SPEED_FREQ_HIGH;         
    HAL_GPIO_Init(DHT11_PORT, &gpio_initstruct);
}

void dht11_gpio_output(void)
{
    GPIO_InitTypeDef gpio_initstruct;
    DHT11_CLK_ENABLE();                         
    
    gpio_initstruct.Pin = DHT11_PIN;        
    gpio_initstruct.Mode = GPIO_MODE_OUTPUT_PP;           
    gpio_initstruct.Speed = GPIO_SPEED_FREQ_HIGH;         
    HAL_GPIO_Init(DHT11_PORT, &gpio_initstruct);
}

void dht11_start(void)
{
    dht11_gpio_output();
    DHT11_DQ_OUT(1);
    DHT11_DQ_OUT(0);
    delay_ms(20);
    DHT11_DQ_OUT(1);
    
    dht11_gpio_input();
    while(DHT11_DQ_IN);     //等待DHT11拉低电平
    while(!DHT11_DQ_IN);    //等待DHT11拉高电平
    while(DHT11_DQ_IN);     //等待DHT11拉低电平
}

uint8_t dht11_read_byte(void)
{
    uint8_t temp = 0;
    uint8_t i = 0;
    uint8_t read_data = 0;
    
    for(i = 0; i < 8; i++)
    {
        while(!DHT11_DQ_IN);
        delay_us(50);
        if(DHT11_DQ_IN == 1)
        {
            temp = 1;
            while(DHT11_DQ_IN);
        }
        else
            temp = 0;
        
        read_data = read_data << 1;
        read_data |= temp;
    }
    
    return read_data;
}

void dht11_read(uint8_t *result)
{
    uint8_t i = 0;
    
    dht11_start();
    dht11_gpio_input();
    
    for(i = 0; i < 5; i++)
        dht11_data[i] = dht11_read_byte();
    
    if(dht11_data[0] + dht11_data[1] + dht11_data[2] + dht11_data[3] == dht11_data[4])
    {
        memcpy(result, dht11_data, 4);
        printf("湿度：%d.%dRH ,", dht11_data[0], dht11_data[1]);
        printf("温度：%d.%d℃\r\n", dht11_data[2], dht11_data[3]);
    }
    
    delay_ms(2000);
}

