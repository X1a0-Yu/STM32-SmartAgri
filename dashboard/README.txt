SmartAgri V1.1.0 Flask Web Platform
====================================

DEMO MODE
---------
cd /d D:\CXCY-RJZZQ\STC89C52RC\Works\JiaLiChuang\new\work2\dashboard
python run.py --demo --web-port 8080

REAL BOARD (CH340 / USART1)
---------------------------
python run.py --port COM6 --baud 19200 --web-port 8080

Open: http://127.0.0.1:8080

ESP-01S UPLOAD TO FLASK
-----------------------
To let ESP-01S upload from the LAN, run Flask on all interfaces:
  python run.py --port COM6 --baud 19200 --host 0.0.0.0 --web-port 8080 --ingest-token YOUR_TOKEN
Then configure the MCU WiFi target with the computer LAN IPv4 address, not 127.0.0.1.

REAL-TIME CHANNEL
-----------------
GET /api/v1/events (Server-Sent Events)

KEY REST APIs
-------------
GET  /api/v1/state
GET  /api/v1/wireless
PUT  /api/v1/thresholds
PUT  /api/v1/controls/{pump|fan|lamp|beep}
POST /api/v1/controls/auto
POST /api/v1/wireless/wifi/power
POST /api/v1/wireless/lora/power
POST /api/v1/wireless/lora/test
POST /api/v1/ingest/telemetry

SECURITY
--------
- Default bind is 127.0.0.1.
- Use --ingest-token when exposing the ingest API to the LAN.
- WiFi password and token are not returned in telemetry or SSE snapshots.
