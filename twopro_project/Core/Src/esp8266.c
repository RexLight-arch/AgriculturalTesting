#include "esp8266.h"
#include "stdio.h"
#include "string.h"
#include "delay.h"
#include "stdarg.h"
#include "usart.h"

uint8_t esp8266_rx_buf[ESP8266_RX_BUF_SIZE];
uint8_t esp8266_tx_buf[ESP8266_TX_BUF_SIZE];
uint16_t esp8266_cnt = 0, esp8266_cntPre = 0;

#define esp8266_handle huart2
void esp8266_uart_init(uint32_t baudrate)
{
    esp8266_handle.Instance = USART2;
    esp8266_handle.Init.BaudRate = baudrate;
    esp8266_handle.Init.WordLength = UART_WORDLENGTH_8B;
    esp8266_handle.Init.StopBits = UART_STOPBITS_1;
    esp8266_handle.Init.Parity = UART_PARITY_NONE;
    esp8266_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    esp8266_handle.Init.Mode = UART_MODE_TX_RX;
    HAL_UART_Init(&esp8266_handle);
}

void USART2_IRQHandler(void)
{
    uint8_t receive_data = 0;
    if(__HAL_UART_GET_FLAG(&esp8266_handle, UART_FLAG_RXNE) != RESET)
    {
        if(esp8266_cnt >= sizeof(esp8266_rx_buf))
            esp8266_cnt = 0;
        HAL_UART_Receive(&esp8266_handle, &receive_data, 1, 1000);
        esp8266_rx_buf[esp8266_cnt++] = receive_data;
        //uart1_cnt++;
        //HAL_UART_Transmit(&uart1_handle, &receive_data, 1, 1000);
    }
}

uint8_t esp8266_wait_receive(void)
{
    if(esp8266_cnt == 0)
        return ESP8266_ERROR;
    
    if(esp8266_cnt == esp8266_cntPre)
    {
        esp8266_cnt = 0;
        return ESP8266_EOK;
    }
    
    esp8266_cntPre = esp8266_cnt;
    return ESP8266_ERROR;
}

void esp8266_rx_clear(void)
{
    memset(esp8266_rx_buf, 0, sizeof(esp8266_rx_buf));
    esp8266_cnt = 0;
}

void esp8266_receive_data(void)
{
    if(esp8266_wait_receive() == ESP8266_EOK)
    {
        printf("esp8266 recv: %s\r\n", esp8266_rx_buf);
        esp8266_rx_clear();
    }
}

//void esp8266_send_data(char *fmt, ...)
//{
//    va_list ap;
//    uint16_t len;
//    
//    va_start(ap, fmt);
//    vsprintf((char *)esp8266_tx_buf, fmt, ap);
//    va_end(ap);
//    
//    len = strlen((const char *)esp8266_tx_buf);
//    HAL_UART_Transmit(&esp8266_handle, esp8266_tx_buf, len, 100);
//}

void esp8266_send_data(char *data, uint16_t len)
{
    esp8266_rx_clear();
    HAL_UART_Transmit(&esp8266_handle, (unsigned char *)data, len, 100);
}

uint16_t esp8266_copy_rxdata(char *data)
{
    memcpy(data, esp8266_rx_buf, esp8266_cntPre);
    return esp8266_cntPre;
}

uint8_t esp8266_send_command(char *cmd, char *res)
{
    uint8_t time_out = 250;
    esp8266_rx_clear();
    HAL_UART_Transmit(&esp8266_handle, (uint8_t *)cmd, strlen(cmd), 100);
    
    while(time_out--)
    {
        if(esp8266_wait_receive() == ESP8266_EOK)
        {
            if(strstr((const char*)esp8266_rx_buf, res) != NULL)
                return ESP8266_EOK;
        }
        delay_ms(10);
    }
    
    return ESP8266_ERROR;
}

uint8_t esp8266_at_test(void)
{
    return esp8266_send_command("AT\r\n", "OK");
}

uint8_t esp8266_set_mode(uint8_t mode)
{
    switch(mode)
    {
        case ESP8266_STA_MODE:
            return esp8266_send_command("AT+CWMODE=1\r\n", "OK");
        
        case ESP8266_AP_MODE:
            return esp8266_send_command("AT+CWMODE=2\r\n", "OK");
        
        case ESP8266_STA_AP_MODE:
            return esp8266_send_command("AT+CWMODE=3\r\n", "OK");
        
        default:
            return ESP8266_EINVAL;
    }
}

uint8_t esp8266_join_ap(char *ssid, char *pwd)
{
    char cmd[64];
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);
    return esp8266_send_command(cmd, "WIFI GOT IP");
}

uint8_t esp8266_connection_mode(uint8_t mode)
{
    char cmd[64];
    sprintf(cmd, "AT+CIPMUX=%d\r\n", mode);
    return esp8266_send_command(cmd, "OK");
}

uint8_t esp8266_connect_tcp_server(char *server_ip, char *server_port)
{
    char cmd[64];
    sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", server_ip, server_port);
    return esp8266_send_command(cmd, "CONNECT");
}

uint8_t esp8266_enter_unvarnished(void)
{
    uint8_t ret;
    ret = esp8266_send_command("AT+CIPMODE=1\r\n", "OK");
    ret += esp8266_send_command("AT+CIPSEND\r\n", ">");
    if (ret == ESP8266_EOK)
        return ESP8266_EOK;
    else
        return ESP8266_ERROR;
}


void esp8266_init(uint32_t baudrate)
{
    printf("esp8266初始化开始...\r\n");
    esp8266_uart_init(baudrate);
    
    //esp8266的其它初始化
    printf("1. 测试esp8266是否存在...\r\n");
    while(esp8266_at_test())
        delay_ms(500);
    
    printf("2. 设置工作模式为STA...\r\n");
    while(esp8266_set_mode(ESP8266_STA_MODE))
        delay_ms(500);
    
    printf("3. 设置单路链接模式...\r\n");
    while(esp8266_connection_mode(ESP8266_SINGLE_CONNECTION))
        delay_ms(500);
    
    printf("4. 连接wifi，SSID: %s, PWD: %s\r\n", WIFI_SSID, WIFI_PWD);
    while(esp8266_join_ap(WIFI_SSID, WIFI_PWD))
        delay_ms(1500);
    
    printf("5. 连接TCP服务器，server_ip:%s, server_port:%s\r\n", TCP_SERVER_IP, TCP_SERVER_PORT);
    while(esp8266_connect_tcp_server(TCP_SERVER_IP, TCP_SERVER_PORT))
        delay_ms(500);
    
    printf("6. 进入到透传模式...\r\n");
    while(esp8266_enter_unvarnished())
        delay_ms(500);
    
    printf("ESP8266已连接上TCP服务器并进入透传模式\r\n");
    printf("ESP8266初始化完成！\r\n");
}



//void esp8266_test(void)
//{
//    esp8266_send_data("this is from esp8266\r\n");
//    esp8266_receive_data();
//}

