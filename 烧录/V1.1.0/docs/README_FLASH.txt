Flashing SmartAgri V1.1.0
==========================
1. Open FlyMcu.
2. Select firmware/SmartAgri_STM32_V1.1.0.hex.
3. Flash the STM32F103C8T6 from 0x08000000.
4. Reset/power-cycle the board.
5. Close FlyMcu before opening the dashboard (COM port is exclusive).
6. Verify at 19200 8N1:
   SmartAgri V1.1.0 @ 0x08000000
   DATA,T=...,FWMAJ=1,FWMIN=1,FWPATCH=0,PROTO=2,...

Dashboard:
cd /d D:\CXCY-RJZZQ\STC89C52RC\Works\JiaLiChuang\new\work2\dashboard
python run.py --port COM6 --baud 19200 --web-port 8080
Open http://127.0.0.1:8080

Rollback:
Flash ../V1.0.0/firmware/SmartAgri_STM32_V1.0.0.hex.
