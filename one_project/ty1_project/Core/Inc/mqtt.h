#ifndef _ONENET_H_
#define _ONENET_H_

#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "delay.h"

#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))
    
extern char MQTT_ClientID[100]; //MQTT_客户端ID
extern char MQTT_UserName[100]; //MQTT_用户名
extern char MQTT_PassWord[200]; //MQTT_密码

typedef struct
{
    uint8_t topic[512];
    uint16_t topic_len;
    uint8_t payload[512];
    uint16_t payload_len;
} Mqtt_RxData_Type;

//云服务器的设备证书
#define PRODUCT_KEY "YOUR_PRODUCT_KEY"
#define DEVICE_NAME "YOUR_DEVICE_NAME"
#define DEVICE_SECRET "YOUR_DEVICE_SECRET"

//订阅与发布的主题
#define RELY_PUBLISH_TOPIC  "Go_IoTAS"  //属性设置应答订阅主题,onenet studio定义好的
#define SET_TOPIC  "Go_IoTAS"
#define POST_TOPIC "Go_IoTAS"

//事件上报主题
#define EVENT_PUBLISH_TOPIC   "Go_IoTAS"  //发布主题,onenet studio定义好的

//阿里云用户名初始化
void mqtt_login_init(char *ProductKey,char *DeviceName,char *DeviceSecret);
//MQTT协议相关函数声明
uint8_t mqtt_publish_data(char *topic, char *message, uint8_t qos);
uint8_t mqtt_subscribe_topic(char *topic,uint8_t qos,uint8_t whether);
void mqtt_init(void);
uint8_t mqtt_connect(char *ClientID,char *Username,char *Password);
void mqtt_send_heart(void);
void mqtt_disconnect(void);
void mqtt_send_data(uint8_t *buf,uint16_t len);
void mqtt_send_response(uint8_t *id);
uint8_t mqtt_receive_handle(uint8_t *data_received, Mqtt_RxData_Type *rx_data);
#endif
