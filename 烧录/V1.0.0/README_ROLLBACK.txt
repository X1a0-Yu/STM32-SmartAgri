SmartAgri V1.0.0 - Baseline Release
====================================

This is the known-working firmware frozen before V1.1 wireless and UI work.

HOW TO ROLL BACK
----------------
1. Open FlyMcu (D:\CTF&AWD工具包\1#环境安装\物联网\嵌入式\flyMCU\FlyMcu.exe).
2. Select firmware/SmartAgri_STM32_V1.0.0.hex.
3. Connect to the MCU via CH340 and flash.
4. Power-cycle the board after flashing.

COMPATIBLE HOST SETTINGS
------------------------
- Port: CH340 serial port (PA9/PA10)
- Baud: 19200
- Data bits: 8
- Parity: None
- Stop bits: 1

VERIFICATION AFTER FLASH
------------------------
Run a serial terminal at 19200 8N1.
You should see, every ~2 seconds:
  DATA,T=..,H=..,AIR=..,LIGHT=..,SOIL=..,TMIN=..,TMAX=..,...

EXPECTED LIMITATIONS
--------------------
- Passive buzzer produces only a click/alarm, not a continuous tone.
- OLED will visibly blank momentarily every ~250 ms.
- No LoRa or WiFi functionality.

FILES INCLUDED
--------------
- firmware/SmartAgri_STM32_V1.0.0.hex
- firmware/SHA256 = 6f0acf66f7e7ff58d0e50f3d1624515441eca75cda5ffef4041303b52a48a6de
- source/SmartAgri_Source_V1.0.0.zip
- dashboard/SmartAgri_Dashboard_V1.0.0.zip
- manifest.json
