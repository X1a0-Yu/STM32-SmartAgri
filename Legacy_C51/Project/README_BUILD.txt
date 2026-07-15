SmartAgri STC89C52RC Keil C51 Project
======================================

This firmware is implemented for Keil C51/8052-compatible STC89C52RC development.

Build from command line:
  D:\CXCY-RJZZQ\STC89C52RC\Works\JiaLiChuang\new\work2\Project\build.bat

Generated HEX:
  D:\CXCY-RJZZQ\STC89C52RC\Works\JiaLiChuang\new\work2\Objects\SmartAgri.hex

Source groups to add in uVision if creating/opening manually:

User:
  ..\User\main.c
  ..\User\app_control.c
  ..\User\system_tick.c

Drivers:
  ..\Drivers\serial2_printf.c
  ..\Drivers\key.c
  ..\Drivers\display.c
  ..\Drivers\actuators.c
  ..\Drivers\dht11.c
  ..\Drivers\adc0832.c

Include paths:
  ..\User
  ..\Drivers
  D:\Keil_v5\C51\INC

Important hardware configuration:
  All STC89C52RC pins, active levels, default thresholds, oscillator and periods are in ..\User\board_config.h.

Current display backend:
  ..\Drivers\display.c mirrors display pages to printf_20 because the actual display module is not confirmed.
  Replace this backend with LCD1602/OLED code once the real display wiring is known.

Current serial2 behavior:
  printf_20 is implemented on the standard 8051 UART using SBUF/TI because classic STC89C52RC UART2 support is not confirmed.
