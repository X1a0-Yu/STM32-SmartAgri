# SmartAgri V1.2 MQTT contract

## Topology

The STM32 host protocol remains unchanged: CH340/USART1, 19200 8N1, CRLF, Protocol 2. `gateway` is the only process allowed to open the MCU serial port. The ESP-01S HTTP uploader stays disabled for V1.2.

```text
STM32 <-> local gateway <-> EMQX TLS <-> remote MQTT worker / Flask dashboard
```

## Topics

For `greenhouse-001`:

- `smartagri/v1/devices/greenhouse-001/telemetry` — QoS 1, non-retained gateway telemetry.
- `smartagri/v1/devices/greenhouse-001/state` — QoS 1, retained latest state.
- `smartagri/v1/devices/greenhouse-001/availability` — QoS 1, retained online/serial status and Last Will.
- `smartagri/v1/devices/greenhouse-001/commands` — QoS 1, non-retained remote commands.
- `smartagri/v1/devices/greenhouse-001/command-results` — QoS 1, non-retained command lifecycle results.

MQTT payloads are JSON with `schema: 1`, `device_id`, `gateway_id`, and UTC `reported_at` values. MQTT input is never raw UART text.

## Command lifecycle

The remote API creates a UUID `request_id`, records the audit entry, and returns HTTP 202 after publishing. The gateway reports `received`, followed by `succeeded`, `failed`, `timed_out`, or `rejected`. QoS 1 can redeliver messages, so terminal command results are stored locally by `request_id` and duplicate controls are not sent to the MCU.

## Local gateway setup

1. Install `pip install -r gateway/requirements.txt`.
2. Copy `gateway/.env.example` to a protected environment file and configure the actual CH340 `COM` port, device ID, broker hostname, certificate authority, and generated gateway credential.
3. Start with `python -m gateway.run_gateway` from the `dashboard` directory.
4. Do not run `run.py` against the same serial port; it is retained only as the V1.1 local compatibility dashboard.

## Remote bootstrap

Deploy `deploy/docker-compose.yml`, configure `.env` and the EMQX CA secret, then call `POST /api/v1/auth/bootstrap` exactly once with an operator username and a password of at least 12 characters. The endpoint is deliberately disabled after the first account exists. Viewer accounts should be inserted/administered through a controlled server-side path before wider use.

## Security requirements

- Gateway and remote backend use separate MQTT identities.
- TLS certificate validation is mandatory; plaintext MQTT is rejected by `gateway/config.py`.
- EMQX must deny anonymous connections and deny unmatched publish/subscribe authorization.
- A gateway can subscribe only to its own commands and publish only its own telemetry/state/availability/results.
- The backend may publish commands and consume SmartAgri device topics; browsers never receive MQTT credentials.
- Rotate all prior dashboard/MQTT defaults before deployment and do not place passwords in source, images, retained MQTT messages, or frontend code.
