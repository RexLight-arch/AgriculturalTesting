# 基于 STM32 的农业物联网环境监测与数据上云系统

一个面向农业大棚和农田环境监测场景的物联网采集系统。项目基于 STM32F103 构建空气环境节点与土壤环境节点，采集温湿度、光照、空气质量、CO₂、TVOC、土壤温湿度、pH 和氮磷钾等数据，并通过 ESP8266 联网，以 MQTT/JSON 方式上报云平台。

> 该仓库主要展示 STM32 外设驱动、多传感器采集、ESP8266 AT 通信、MCU 侧 MQTT 报文封装和物联网数据上云能力。

## 功能特性

- 空气环境采集：温湿度、CO₂、TVOC、空气质量等。
- 光照采集：采集农业环境中的光照强度。
- 土壤参数采集：土壤温度、湿度、pH、氮磷钾等。
- ESP8266 联网：通过 AT 指令配置 Wi-Fi、TCP 连接和透传模式。
- MQTT 上云：MCU 侧手写 MQTT CONNECT、PUBLISH、SUBSCRIBE、PINGREQ 等报文。
- JSON 上报：将多传感器数据封装为结构化 JSON 上传。
- 串口调试：通过串口日志观察初始化、联网、心跳和上报状态。

## 技术栈

- 开发语言：C
- MCU 平台：STM32F103
- 开发工具：STM32CubeMX、Keil MDK
- 底层库：STM32 HAL、CMSIS
- 通信接口：UART、I2C、ADC、GPIO、RS485/Modbus
- 联网模块：ESP8266、AT Command
- 云端协议：MQTT、JSON

## 系统架构

```text
空气节点：DHT11 / SGP30 / CO₂ / ADC
                              │
土壤节点：温湿度 / pH / NPK / BH1750
                              ├── STM32F103 ─ UART ─ ESP8266 ─ MQTT 云平台
其他气体与空气质量传感器       │
```

## 采集数据

| 节点 | 数据项 |
| --- | --- |
| 空气环境节点 | 温度、湿度、CO₂、TVOC、空气质量 |
| 土壤/综合节点 | 土壤温度、土壤湿度、pH、N/P/K、光照、空气质量、CO₂、TVOC |

## 项目结构

```text
.
├── one_project/
│   └── ty1_project/          # 空气环境监测节点
│       ├── Core/             # 业务逻辑与传感器驱动
│       └── Drivers/          # STM32 HAL/CMSIS
└── twopro_project/           # 土壤与综合环境监测节点
    ├── Core/                 # NPK、pH、温湿度、光照、MQTT 等逻辑
    └── Drivers/              # STM32 HAL/CMSIS
```

## 快速开始

### 1. 打开工程

使用 STM32CubeMX 打开对应 `.ioc` 文件，确认时钟、UART、I2C、ADC 和 GPIO 配置。然后使用 Keil MDK 打开 `MDK-ARM/*.uvprojx`。

### 2. 修改联网配置

根据自己的 Wi-Fi 和云平台信息修改：

- `Core/Inc/esp8266.h`：Wi-Fi SSID、密码、MQTT 服务器地址。
- `Core/Inc/mqtt.h` 或 `Core/Inc/onenet.h`：设备证书、产品信息和 Topic。

建议不要把真实 Wi-Fi 密码和云平台密钥提交到 Git 仓库，可使用占位值或本地私有配置文件。

### 3. 编译烧录

在 Keil MDK 中编译工程，将程序烧录到 STM32F103。上电后通过串口日志检查：

- 传感器初始化是否成功
- ESP8266 是否成功连接 Wi-Fi
- MQTT 是否登录成功
- 心跳和数据上报是否正常

## 设计亮点

- 基于 STM32 HAL 封装 I2C、ADC、UART 和 GPIO，接入多类农业环境传感器。
- 通过 UART/RS485 向土壤传感器发送 Modbus 指令，解析土壤温度、湿度、pH 和氮磷钾数据。
- 读取 ADC 原始值，并根据电压、传感器电阻和标定参数计算空气质量相关浓度。
- 通过 UART 发送 AT 指令驱动 ESP8266 完成 STA 模式、TCP 连接和透传配置。
- 在 MCU 侧手写 MQTT 报文，完成连接、发布、订阅、断开和心跳保活。
- 将多类环境数据封装为 JSON，周期性发布到物联网云平台。

## 截图与演示

后续可在此处补充硬件实物图、串口日志截图、云平台数据展示截图和现场部署照片。

## 安全说明

公开仓库中的 Wi-Fi 和云平台凭据应使用占位值。若历史提交中出现过真实凭据，应及时在平台侧轮换密钥，避免继续使用已暴露的账号或设备证书。
