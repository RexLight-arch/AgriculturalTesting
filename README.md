# 基于 STM32 的农业物联网环境监测与数据上云系统

面向农业大棚和农田环境监测场景，基于 STM32F103 构建空气环境节点与土壤环境节点，采集多类环境和土壤参数，通过 ESP8266 联网，并以 MQTT/JSON 方式周期性上报云平台。

## 核心功能

- 空气温湿度、CO₂、TVOC、O₂ 和空气质量采集
- 光照强度采集
- 土壤温度、湿度、pH 和氮磷钾采集
- ESP8266 Wi-Fi 联网与 TCP 透传
- MCU 侧 MQTT 报文封装、发布与心跳
- JSON 多传感器数据上报
- 串口日志和异常状态调试

## 技术亮点

1. **多传感器驱动**：基于 STM32 HAL 驱动 I²C、ADC、UART 和 GPIO，接入 DHT11、SGP30、BH1750、气体传感器及土壤多参数传感器。
2. **土壤参数解析**：通过 UART/RS485 向土壤传感器发送 Modbus 指令，使用串口中断接收数据，并解析土壤温度、湿度、pH 及氮磷钾。
3. **气体浓度换算**：读取 ADC 原始值，根据电压、传感器电阻和标定参数计算空气质量浓度。
4. **Wi-Fi 通信封装**：通过 UART 发送 AT 指令驱动 ESP8266，完成 STA 模式、单连接、AP 接入、TCP 连接和透传模式配置。
5. **手写 MQTT 报文**：在 MCU 侧完成 CONNECT、PUBLISH、SUBSCRIBE、DISCONNECT 和 PINGREQ 心跳报文的编码与发送。
6. **结构化云端上报**：将光照、空气、土壤和气体数据封装为 JSON，周期性发布到物联网平台。
7. **长期连接保活**：通过 MQTT 心跳和现场联调验证维持设备在线，保障终端持续上传。

## 系统架构

```text
空气节点：DHT11 / SGP30 / O₂ / ADC
                              ┐
土壤节点：温湿度 / pH / NPK / BH1750
                              ├─ STM32F103 ─ UART ─ ESP8266 ─ MQTT 云平台
其他气体与空气质量传感器       ┘
```

## 技术栈

`C` · `STM32F103` · `STM32 HAL` · `STM32CubeMX` · `Keil MDK` · `ESP8266` · `AT Command` · `MQTT` · `JSON` · `UART` · `I²C` · `ADC` · `RS485/Modbus`

## 采集数据

| 节点 | 数据项 |
| --- | --- |
| 空气节点 | 温度、湿度、CO₂、TVOC、O₂ |
| 土壤/综合节点 | 土壤温度、土壤湿度、pH、N/P/K、光照、空气质量、CO₂、TVOC |

## 目录结构

```text
.
├─ one_project/ty1_project/  # 空气环境监测节点
│  ├─ Core/                  # 业务逻辑与传感器驱动
│  └─ Drivers/               # STM32 HAL/CMSIS
└─ twopro_project/           # 土壤与综合环境监测节点
   ├─ Core/                  # NPK、pH、温湿度、光照、MQTT
   └─ Drivers/               # STM32 HAL/CMSIS
```

## 构建与使用

1. 使用 STM32CubeMX 打开对应 `.ioc` 文件，确认时钟、UART、I²C、ADC 和 GPIO 配置。
2. 在 `Core/Inc/esp8266.h` 中填写自己的 Wi-Fi SSID、密码与 MQTT 服务器地址。
3. 在 `Core/Inc/mqtt.h` 或 `Core/Inc/onenet.h` 中填写设备证书和 Topic。
4. 使用 Keil MDK 打开 `MDK-ARM/*.uvprojx`，编译并烧录到 STM32F103。
5. 通过串口日志检查传感器初始化、Wi-Fi 连接、MQTT 登录、心跳及数据上报状态。

## 安全说明

公开仓库中的 Wi-Fi 和云平台凭据已替换为占位值。实际部署应使用独立测试设备证书，避免将真实密钥提交到版本库；历史凭据若仍有效，应及时轮换。

