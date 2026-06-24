#include "onenet.h"
#include "esp8266.h"

char MQTT_ClientID[100]; //MQTT_客户端ID
char MQTT_UserName[100]; //MQTT_用户名
char MQTT_PassWord[200]; //MQTT_密码

uint8_t *mqtt_rxbuf;
uint8_t *mqtt_txbuf;
uint16_t mqtt_rxlen;
uint16_t mqtt_txlen;
uint8_t _mqtt_txbuf[512];//发送数据缓存区
uint8_t _mqtt_rxbuf[512];//接收数据缓存区

typedef enum
{
    //名字         值             报文流动方向     描述
    M_RESERVED1    =0    ,    //    禁止    保留
    M_CONNECT        ,    //    客户端到服务端    客户端请求连接服务端
    M_CONNACK        ,    //    服务端到客户端    连接报文确认
    M_PUBLISH        ,    //    两个方向都允许    发布消息
    M_PUBACK        ,    //    两个方向都允许    QoS 1消息发布收到确认
    M_PUBREC        ,    //    两个方向都允许    发布收到（保证交付第一步）
    M_PUBREL        ,    //    两个方向都允许    发布释放（保证交付第二步）
    M_PUBCOMP        ,    //    两个方向都允许    QoS 2消息发布完成（保证交互第三步）
    M_SUBSCRIBE        ,    //    客户端到服务端    客户端订阅请求
    M_SUBACK        ,    //    服务端到客户端    订阅请求报文确认
    M_UNSUBSCRIBE    ,    //    客户端到服务端    客户端取消订阅请求
    M_UNSUBACK        ,    //    服务端到客户端    取消订阅报文确认
    M_PINGREQ        ,    //    客户端到服务端    心跳请求
    M_PINGRESP        ,    //    服务端到客户端    心跳响应
    M_DISCONNECT    ,    //    客户端到服务端    客户端断开连接
    M_RESERVED2        ,    //    禁止    保留
}_typdef_mqtt_message;

//连接成功服务器回应 20 02 00 00
//客户端主动断开连接 e0 00
const uint8_t parket_connetAck[] = {0x20,0x02,0x00,0x00};
const uint8_t parket_disconnet[] = {0xe0,0x00};
const uint8_t parket_heart[] = {0xc0,0x00};
const uint8_t parket_heart_reply[] = {0xc0,0x00};
const uint8_t parket_subAck[] = {0x90,0x03};

/*
函数功能: 初始化阿里云物联网服务器的登录参数
*/


//密码
//加密之前的数据格式:  clientId*deviceName*productKey#
// *替换为DeviceName  #替换为ProductKey  加密密钥是DeviceSecret  加密方式是HmacSHA1  
//PassWord明文=  clientIdiot_devicedeviceNameiot_deviceproductKeya1VMIfYeEEE
//hmacsha1加密网站：http://encode.chahuo.com/
//加密的密钥：DeviceSecret

void mqtt_login_init(char *ProductKey,char *DeviceName,char *DeviceSecret)
{
//    sprintf(MQTT_ClientID,"%s.%s|securemode=2,signmethod=hmacsha256,timestamp=1695871022945|",ProductKey,DeviceName);
//    sprintf(MQTT_UserName,"%s&%s",DeviceName,ProductKey);
// Example credentials intentionally omitted from the public repository.
    sprintf(MQTT_ClientID,"%s", DeviceName);
    sprintf(MQTT_UserName,"%s", ProductKey);
    sprintf(MQTT_PassWord,"version=2018-10-31&res=products%%2F%s%%2Fdevices%%2F%s&et=2017881776&method=sha1&sign=%s",ProductKey,DeviceName,DEVICE_SECRET);
}

void mqtt_init(void)
{
    mqtt_login_init(PRODUCT_KEY,DEVICE_NAME,DEVICE_SECRET);
    //缓冲区赋值
    mqtt_rxbuf = _mqtt_rxbuf;
    mqtt_rxlen = sizeof(_mqtt_rxbuf);
    mqtt_txbuf = _mqtt_txbuf;
    mqtt_txlen = sizeof(_mqtt_txbuf);
    memset(mqtt_rxbuf,0,mqtt_rxlen);
    memset(mqtt_txbuf,0,mqtt_txlen);
    
    //无条件先主动断开
    mqtt_disconnect();
    delay_ms(100);
    mqtt_disconnect();
    delay_ms(100);
}

/*
函数功能: 登录服务器
函数返回值: 0表示成功 1表示失败
*/
uint8_t mqtt_connect(char *ClientID,char *Username,char *Password)
{
//    uint8_t i;
    uint8_t j;
    int ClientIDLen = strlen(ClientID);
    int UsernameLen = strlen(Username);
    int PasswordLen = strlen(Password);
    int DataLen;
    mqtt_txlen=0;
    //可变报头+Payload  每个字段包含两个字节的长度标识
    DataLen = 10 + (ClientIDLen+2) + (UsernameLen+2) + (PasswordLen+2);
    
    //固定报头
    //控制报文类型
    mqtt_txbuf[mqtt_txlen++] = 0x10;        //MQTT Message Type CONNECT
    //剩余长度(不包括固定头部)
    do
    {
        uint8_t encodedByte = DataLen % 128;
        DataLen = DataLen / 128;
        // if there are more data to encode, set the top bit of this byte
        if ( DataLen > 0 )
            encodedByte = encodedByte | 128;
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    }while ( DataLen > 0 );
        
    //可变报头
    //协议名
    mqtt_txbuf[mqtt_txlen++] = 0;            // Protocol Name Length MSB    
    mqtt_txbuf[mqtt_txlen++] = 4;           // Protocol Name Length LSB    
    mqtt_txbuf[mqtt_txlen++] = 'M';            // ASCII Code for M    
    mqtt_txbuf[mqtt_txlen++] = 'Q';            // ASCII Code for Q    
    mqtt_txbuf[mqtt_txlen++] = 'T';            // ASCII Code for T    
    mqtt_txbuf[mqtt_txlen++] = 'T';            // ASCII Code for T    
    //协议级别
    mqtt_txbuf[mqtt_txlen++] = 4;                // MQTT Protocol version = 4    
    //连接标志
    mqtt_txbuf[mqtt_txlen++] = 0xc2;            // conn flags 
    mqtt_txbuf[mqtt_txlen++] = 0;                // Keep-alive Time Length MSB    
    mqtt_txbuf[mqtt_txlen++] = 100;            // Keep-alive Time Length LSB  100S心跳包  
    
    mqtt_txbuf[mqtt_txlen++] = BYTE1(ClientIDLen);// Client ID length MSB    
    mqtt_txbuf[mqtt_txlen++] = BYTE0(ClientIDLen);// Client ID length LSB      
    memcpy(&mqtt_txbuf[mqtt_txlen],ClientID,ClientIDLen);
    mqtt_txlen += ClientIDLen;
    
    if(UsernameLen > 0)
    {   
        mqtt_txbuf[mqtt_txlen++] = BYTE1(UsernameLen);        //username length MSB    
        mqtt_txbuf[mqtt_txlen++] = BYTE0(UsernameLen);        //username length LSB    
        memcpy(&mqtt_txbuf[mqtt_txlen],Username,UsernameLen);
        mqtt_txlen += UsernameLen;
    }
    
    if(PasswordLen > 0)
    {    
        mqtt_txbuf[mqtt_txlen++] = BYTE1(PasswordLen);        //password length MSB    
        mqtt_txbuf[mqtt_txlen++] = BYTE0(PasswordLen);        //password length LSB  
        memcpy(&mqtt_txbuf[mqtt_txlen],Password,PasswordLen);
        mqtt_txlen += PasswordLen; 
    }    
    
//    for(i=0;i<10;i++)
//    {
        memset(mqtt_rxbuf,0,mqtt_rxlen);
        mqtt_send_data(mqtt_txbuf,mqtt_txlen);
//        for(j=0;j<10;j++)
//            printf("%c",mqtt_txbuf[j]);
        for(j=0;j<10;j++)
        {
            delay_ms(50);
            if (esp8266_wait_receive() == ESP8266_EOK)
                esp8266_copy_rxdata((char *)mqtt_rxbuf);

            //CONNECT
            if(mqtt_rxbuf[0]==parket_connetAck[0] && mqtt_rxbuf[1]==parket_connetAck[1] && mqtt_rxbuf[2]==parket_connetAck[2]) //连接成功
            {
                return 0;//连接成功
            }
        }
//    }
    return 1;
}

/*
函数功能: MQTT订阅/取消订阅数据打包函数
函数参数:
    topic       主题   
    qos         消息等级 0:最多分发一次  1: 至少分发一次  2: 仅分发一次
    whether     订阅/取消订阅请求包 (1表示订阅,0表示取消订阅)
返回值: 0表示成功 1表示失败
*/
uint8_t mqtt_subscribe_topic(char *topic,uint8_t qos,uint8_t whether)
{    
//    uint8_t i;
    uint8_t j;
    mqtt_txlen=0;
    int topiclen = strlen(topic);
    
    int DataLen = 2 + (topiclen+2) + (whether?1:0);//可变报头的长度（2字节）加上有效载荷的长度
    //固定报头
    //控制报文类型
    if(whether)mqtt_txbuf[mqtt_txlen++] = 0x82; //消息类型和标志订阅
    else    mqtt_txbuf[mqtt_txlen++] = 0xA2;    //取消订阅

    //剩余长度
    do
    {
        uint8_t encodedByte = DataLen % 128;
        DataLen = DataLen / 128;
        // if there are more data to encode, set the top bit of this byte
        if ( DataLen > 0 )
            encodedByte = encodedByte | 128;
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    }while ( DataLen > 0 );    
    
    //可变报头
    mqtt_txbuf[mqtt_txlen++] = 0;            //消息标识符 MSB
    mqtt_txbuf[mqtt_txlen++] = 0x01;        //消息标识符 LSB
    //有效载荷
    mqtt_txbuf[mqtt_txlen++] = BYTE1(topiclen);//主题长度 MSB
    mqtt_txbuf[mqtt_txlen++] = BYTE0(topiclen);//主题长度 LSB   
    memcpy(&mqtt_txbuf[mqtt_txlen],topic,topiclen);
    mqtt_txlen += topiclen;
    
    if(whether)
    {
       mqtt_txbuf[mqtt_txlen++] = qos;//QoS级别
    }
    
//    for(i=0;i<10;i++)
//    {
        memset(mqtt_rxbuf,0,mqtt_rxlen);
        mqtt_send_data(mqtt_txbuf,mqtt_txlen);

        for(j=0;j<10;j++)
        {
            delay_ms(50);
            if (esp8266_wait_receive() == ESP8266_EOK)
                esp8266_copy_rxdata((char *)mqtt_rxbuf);

            if(mqtt_rxbuf[0]==parket_subAck[0] && mqtt_rxbuf[1]==parket_subAck[1]) //订阅成功               
            {
                return 0;//订阅成功
            }
        }
//    }
    return 1; //失败
}

//MQTT发布数据打包函数
//topic   主题 
//message 消息
//qos     消息等级 
uint8_t mqtt_publish_data(char *topic, char *message, uint8_t qos)
{  
    int topicLength = strlen(topic);    
    int messageLength = strlen(message);     
    static uint16_t id=0;
    int DataLen;
    mqtt_txlen=0;
    //有效载荷的长度这样计算：用固定报头中的剩余长度字段的值减去可变报头的长度
    //QOS为0时没有标识符
    //数据长度             主题名   报文标识符   有效载荷
    if(qos)    DataLen = (2+topicLength) + 2 + messageLength;       
    else    DataLen = (2+topicLength) + messageLength;   

    //固定报头
    //控制报文类型
    mqtt_txbuf[mqtt_txlen++] = 0x30;    // MQTT Message Type PUBLISH  

    //剩余长度
    do
    {
        uint8_t encodedByte = DataLen % 128;
        DataLen = DataLen / 128;
        // if there are more data to encode, set the top bit of this byte
        if ( DataLen > 0 )
            encodedByte = encodedByte | 128;
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    }while ( DataLen > 0 );    
    
    mqtt_txbuf[mqtt_txlen++] = BYTE1(topicLength);//主题长度MSB
    mqtt_txbuf[mqtt_txlen++] = BYTE0(topicLength);//主题长度LSB 
    memcpy(&mqtt_txbuf[mqtt_txlen],topic,topicLength);//拷贝主题
    mqtt_txlen += topicLength;
        
    //报文标识符
    if(qos)
    {
        mqtt_txbuf[mqtt_txlen++] = BYTE1(id);
        mqtt_txbuf[mqtt_txlen++] = BYTE0(id);
        id++;
    }
    memcpy(&mqtt_txbuf[mqtt_txlen],message,messageLength);
    mqtt_txlen += messageLength;

//    int i = 0;
//    for(i=0;i<mqtt_txlen;i++)
//        printf("%02X ", mqtt_txbuf[i]);
//    printf("\r\n");
    mqtt_send_data(mqtt_txbuf,mqtt_txlen);
    return mqtt_txlen;
}

uint8_t mqtt_receive_handle(uint8_t *data_received, Mqtt_RxData_Type *rx_data)
{
    uint8_t *p;
    uint8_t encodeByte = 0;
    uint32_t multiplier = 1, Remaining_len = 0;
    uint8_t QS_level = 0;
    
    p = data_received;
    memset(rx_data, 0, sizeof(Mqtt_RxData_Type));
    
    //解析接收数据
    if((*p != 0x30)&&(*p != 0x32)&&(*p != 0x34))   //不是发布报文头
        return 1;
    
    if(*p != 0x30) QS_level = 1;    //标记qs等级不为0
    
    p++;
    //提取剩余数据长度
    do{
        encodeByte = *p++;
        Remaining_len += (encodeByte & 0x7F) * multiplier;
        multiplier *= 128;
        
        if(multiplier > 128*128*128) //超出剩余长度最大4个字节的要求,错误
            return 2;
    }while((encodeByte & 0x80) != 0);
    
    //提取主题数据长度
    rx_data->topic_len = *p++;
    rx_data->topic_len = rx_data->topic_len * 256 + *p++;
    //提取主题
    memcpy(rx_data->topic,p,rx_data->topic_len);
    p += rx_data->topic_len;
    
    if(QS_level != 0)  //跳过报文标识符
        p += 2;
    
    //提取payload
    rx_data->payload_len = Remaining_len - rx_data->topic_len - 2;
    memcpy(rx_data->payload, p, rx_data->payload_len);
    
//    printf("topic: %s\r\n", rx_data->topic);
//    printf("topic_len: %d\r\n", rx_data->topic_len);
//    printf("payload: %s\r\n", rx_data->payload);
//    printf("payload_len: %d\r\n", rx_data->payload_len);

    return 0;
}

void mqtt_send_response(uint8_t *id)
{
    char buf[128] = {0};
    sprintf(buf,"{\"id\":\"%s\",\"code\":200,\"msg\":\"success\"}",id);
    
    mqtt_publish_data(RELY_PUBLISH_TOPIC,(char *)buf,0);
    
    printf("\r\n发布数据:\r\n");
    printf((const char *)buf);    //发布的数据打印出来
    printf("\r\n");
}

void mqtt_send_heart(void)
{
    mqtt_send_data((uint8_t *)parket_heart,sizeof(parket_heart));
}

void mqtt_disconnect(void)
{
    mqtt_send_data((uint8_t *)parket_disconnet,sizeof(parket_disconnet));
}

void mqtt_send_data(uint8_t *buf,uint16_t len)
{
    esp8266_send_data((char *)buf, len);
}
