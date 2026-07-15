# STM32 SmartAgri

智慧农业控制系统，基于 **STM32F103C8T6**。系统采集温湿度、空气质量、光照和土壤数据，并控制水泵、风扇、补光灯与蜂鸣器。

> 当前主线：**V1.2 MQTT 远程架构（开发中）**。MCU 继续使用已验证的串口 Protocol 2；本地网关独占 CH340 串口，通过 MQTT 与远端 Flask 服务同步数据和控制指令。

## 项目架构

```text
STM32F103C8T6
  └─ USART1 / CH340 · 19200 8N1 · Protocol 2
       └─ 本地 Python Gateway（唯一串口拥有者）
            └─ MQTT over TLS / EMQX
                 └─ Flask + PostgreSQL + Web Dashboard
```

## 版本概览

| 版本 | 状态 | 主要能力 |
| --- | --- | --- |
| [V1.0.0](releases/v1.0.0.md) | 已归档 | 基础传感器、OLED、本地执行器、USB 命令协议。 |
| [V1.1.0](releases/v1.1.0.md) | 已归档 | Flask 仪表盘、LoRa、ESP-01S HTTP 上传与 Protocol 2。 |
| [V1.2.0](releases/v1.2.0.md) | 预发布 | 本地串口 MQTT Gateway、远端 Flask/PostgreSQL、EMQX TLS 与指令审计。 |

## 目录说明

- `User/`：应用层与控制逻辑。
- `Drivers/`：传感器、执行器、串口、ESP、LoRa 驱动。
- `Project/`：Keil MDK 工程及构建脚本。
- `dashboard/`：V1.1 本地 Flask 仪表盘、V1.2 网关和远端服务。
- `烧录/`：历史版本固件、源代码归档、校验和及烧录说明。
- `Legacy_C51/`：已归档的 C51 实现，不是当前 STM32 主线。

## V1.2 快速开始（开发环境）

### 1. 本地 Gateway

```bash
cd dashboard
pip install -r gateway/requirements.txt
# 复制 gateway/.env.example 并设置 COM 端口、MQTT 地址与凭据
python -m gateway.run_gateway
```

Gateway 是 MCU 的**唯一**串口拥有者。不能与 V1.1 的 `run.py` 同时打开同一个 CH340 端口。

### 2. 远端服务

```bash
cd dashboard/deploy
cp .env.example .env
# 填写 PostgreSQL、EMQX TLS 与服务凭据
# 将 EMQX CA 证书放到 MQTT_CA_FILE 指定路径
docker compose --env-file .env up -d --build
```

详见 [V1.2 MQTT 文档](dashboard/README_V1.2_MQTT.md)。

## MCU 串口协议

主机接口：`USART1 PA9/PA10`，经 CH340 暴露，`19200 8N1`，ASCII + CRLF。

- MCU 上报：`DATA,...`、`ACK,<id>,...`、`ERR,<id>,...`
- 主机命令：`GET`、`SET`、`CTRL`、`LORA`、`WIFI`

完整 V1.1 协议见 [烧录/V1.1.0/docs/PROTOCOL_V1.1.txt](烧录/V1.1.0/docs/PROTOCOL_V1.1.txt)。V1.2 保持 MCU Protocol 2 不变，在网关侧完成 MQTT 转换。

## 构建与烧录

- IDE/工具链：Keil MDK，ARMCC 5.06。
- 工程文件：[Project/SmartAgri_STM32.uvprojx](Project/SmartAgri_STM32.uvprojx)
- 构建脚本：[Project/build_stm32.bat](Project/build_stm32.bat)
- 烧录地址：`0x08000000`
- 烧录工具：FlyMcu / STM32 系统 Bootloader。

请在 Gateway 打开 CH340 前关闭 FlyMcu。

## 验证

在 `dashboard` 目录执行：

```bash
python -m unittest discover -s tests -v
```

当前 V1.2 代码层测试包含串口协议回归、Gateway 离线队列/去重、MQTT 指令桥接与远端 API 审计路径。

## 安全说明

- MQTT 必须使用 TLS、唯一账号和按设备主题的 ACL。
- 浏览器不应持有 MQTT 凭据。
- 远端控制必须记录操作者、请求、MQTT 指令 ID 和 MCU ACK/ERR 结果。
- ESP-01S HTTP 上传在 V1.2 默认停用；Gateway 是唯一云同步路径。
