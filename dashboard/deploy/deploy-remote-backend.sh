#!/usr/bin/env bash
set -euo pipefail
BASE=/www/wwwroot/smartagri
MQTT_BASE=/www/wwwroot/mqtt
GW_PASS="$(cat "${MQTT_BASE}/secrets/gateway_password")"
BE_PASS="$(cat "${MQTT_BASE}/secrets/backend_password")"
DB_PASS="$(openssl rand -hex 24)"
FLASK_SECRET="$(openssl rand -hex 32)"

cat > "${BASE}/.env" <<EOF
POSTGRES_DB=smartagri
POSTGRES_USER=smartagri
POSTGRES_PASSWORD=${DB_PASS}
FLASK_SECRET_KEY=${FLASK_SECRET}
MQTT_HOST=82.157.54.145
MQTT_PORT=8883
MQTT_BACKEND_USERNAME=smartagri-backend
MQTT_BACKEND_PASSWORD=${BE_PASS}
MQTT_CA_FILE=${BASE}/secrets/emqx-ca.crt
EOF

cat > "${BASE}/gateway-greenhouse-001.env" <<EOF
SMARTAGRI_GATEWAY_DEVICE_ID=greenhouse-001
SMARTAGRI_GATEWAY_ID=gateway-greenhouse-001
SMARTAGRI_GATEWAY_SERIAL_PORT=COM6
SMARTAGRI_GATEWAY_SERIAL_BAUD=19200
SMARTAGRI_GATEWAY_MQTT_HOST=emqx.x1a0yu.top
SMARTAGRI_GATEWAY_MQTT_PORT=8883
SMARTAGRI_GATEWAY_MQTT_USERNAME=gateway-greenhouse-001
SMARTAGRI_GATEWAY_MQTT_PASSWORD=${GW_PASS}
SMARTAGRI_GATEWAY_MQTT_CA_PATH=emqx-ca.crt
SMARTAGRI_GATEWAY_SPOOL_PATH=./data/gateway-spool.sqlite3
SMARTAGRI_GATEWAY_SPOOL_MAX_ROWS=10000
SMARTAGRI_GATEWAY_COMMAND_TIMEOUT=5
EOF
chmod 600 "${BASE}/.env" "${BASE}/gateway-greenhouse-001.env"

cd "${BASE}/deploy"
docker compose --env-file ../.env config >/dev/null
docker compose --env-file ../.env up -d --build
