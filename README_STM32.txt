STM32F103C8T6 智慧农业固件
==========================

目标芯片：STM32F103C8T6（Medium-density，64KB Flash，20KB SRAM）
程序地址：0x08000000
工程文件：Project\SmartAgri_STM32.uvprojx
命令行构建：Project\build_stm32.bat
烧录文件：ObjectsSTM32\SmartAgri_STM32.hex

已实现：
- PA0 DHT11 温湿度
- PA1 ADC1_CH1 土壤湿度
- PA4 ADC1_CH4 空气质量
- PB1 ADC1_CH9 光照
- PA15/PC13/PC14/PC15 LED1~LED4
- PB12 水泵继电器请求
- PB14 风扇继电器
- PB0 补光请求
- PA12 蜂鸣器
- PB6/PB7 两按键阈值设置
- PB10/PB11 I2C2 OLED
- PA2/PA3 USART2 printf_20 全数据和阈值上报

按键：
- KEY1(PB7)短按：下一页面/下一阈值
- KEY1长按：进入或退出设置模式
- KEY2(PB6)短按：阈值增加
- 两键同时按：阈值减少

串口2：
- USART2 PA2(TX)/PA3(RX)
- 默认 9600bps，8N1
- 注意：FlyMcu 下载时 COM6 的 115200bps 是 BootLoader 下载速率，不是应用串口速率。

烧录：
1. FlyMcu 选择 ObjectsSTM32\SmartAgri_STM32.hex。
2. 目标地址应为 0x08000000。
3. BOOT0 进入系统 BootLoader，按正常流程下载。
4. 下载后将 BOOT0 恢复为 0 并复位，从 Flash 启动。

构建结果：
- ROM 约 11KB，小于 64KB。
- HEX 第一条扩展地址记录为 0x0800，向量表位于 0x08000000。
- 初始 SP 为 0x200006A0，处于 STM32F103C8 SRAM 范围。

硬件注意：
- PB14 根据原理图网络名 PB14_FS 作为风扇。
- PB12 默认作为水泵，请结合接线端子实物确认。
- PB0 是补光请求输出；是否能够直接驱动功率补光灯必须根据实物驱动电路确认。
- PA4 空气质量模拟输入必须确保最高电压不超过 3.3V。
- ADC 数值当前统一为 0~1023，需根据干湿、明暗和空气质量实测校准阈值。
