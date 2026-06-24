# 农业物联网环境监测与数据上云系统

基于 STM32F103 与 ESP8266 的农业环境监测终端。项目由空气环境监测节点和土壤环境监测节点组成，完成多传感器采集、数据解析、MQTT 封包和云端上报，可用于温室、农田及智慧种植场景。

## 项目亮点

- 使用 STM32CubeMX/HAL 搭建多串口、ADC、I²C 与 GPIO 驱动框架。
- 采集空气温湿度、CO₂、TVOC、O₂、光照和空气质量数据。
- 通过 RS485/Modbus 读取土壤温湿度、pH 及氮磷钾数据。
- 驱动 ESP8266 完成 Wi-Fi 联网与 TCP/MQTT 通信。
- 在 MCU 侧实现 MQTT CONNECT、PUBLISH、心跳等报文封装。
- 将多源数据组织为 JSON，周期性上报物联网平台。

## 系统架构

```text
空气节点：DHT11 / SGP30 / O₂ / ADC
                              ┐
土壤节点：温湿度 / pH / NPK / BH1750 ─ STM32F103 ─ ESP8266 ─ MQTT 云平台
                              ┘
```

## 技术栈

`C` · `STM32F103` · `STM32 HAL` · `STM32CubeMX` · `Keil MDK` · `ESP8266` · `MQTT` · `JSON` · `UART` · `I²C` · `ADC` · `RS485/Modbus`

## 目录说明

```text
.
├─ one_project/ty1_project/  # 空气环境监测节点
│  ├─ Core/                  # 业务逻辑与传感器驱动
│  └─ Drivers/               # STM32 HAL/CMSIS
└─ twopro_project/           # 土壤与综合环境监测节点
   ├─ Core/                  # NPK、pH、温湿度、光照、MQTT
   └─ Drivers/               # STM32 HAL/CMSIS
```

## 采集数据

| 节点 | 数据项 |
| --- | --- |
| 空气节点 | 温度、湿度、CO₂、TVOC、O₂ |
| 土壤/综合节点 | 土壤温度、土壤湿度、pH、N/P/K、光照、空气质量、CO₂、TVOC |

## 构建与使用

1. 使用 STM32CubeMX 打开对应 `.ioc` 文件，确认时钟、串口、I²C 和 ADC 配置。
2. 在 `Core/Inc/esp8266.h` 中填写自己的 Wi-Fi SSID、密码与 MQTT 服务器地址。
3. 在 `Core/Inc/mqtt.h` 或 `Core/Inc/onenet.h` 中填写自己的设备证书与 Topic。
4. 使用 Keil MDK 打开 `MDK-ARM/*.uvprojx`，编译并烧录到 STM32F103。
5. 通过串口日志检查传感器初始化、Wi-Fi 连接、MQTT 登录及数据上报状态。

## 安全说明

公开仓库中的 Wi-Fi 和云平台凭据已替换为占位值。实际部署时请使用独立测试设备证书，避免把真实密钥提交到版本库；若历史凭据仍有效，应在平台侧立即轮换。

