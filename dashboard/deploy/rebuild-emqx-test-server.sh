#!/usr/bin/env bash
set -euo pipefail

# Rebuild the dedicated, disposable SmartAgri EMQX instance.
# Run only on the authorized test server as root.
BASE=/www/wwwroot/mqtt
STAMP="$(date +%Y%m%d-%H%M%S)"
BACKUP="/root/smartagri-emqx-backups/${STAMP}"

mkdir -p "$BACKUP" "$BASE/config" "$BASE/certs" "$BASE/secrets"
cp -a "$BASE/compose.yml" "$BACKUP/compose.yml"
cp -a "$BASE/data" "$BACKUP/data" 2>/dev/null || true

openssl rand -hex 32 > "$BASE/secrets/dashboard_password"
openssl rand -hex 24 > "$BASE/secrets/gateway_password"
openssl rand -hex 24 > "$BASE/secrets/backend_password"
chmod 600 "$BASE/secrets"/*

cat > "$BASE/config/emqx.conf" <<'EOF'
node {
  name = "emqx@prod-node1"
  cookie = "smartagri-emqx-cookie-change-on-rebuild"
  data_dir = "data"
}
cluster { name = smartagri discovery_strategy = manual }
listeners.ssl.default {
  bind = "0.0.0.0:8883"
  max_connections = 1024
  ssl_options {
    keyfile = "/opt/emqx/etc/certs/server.key"
    certfile = "/opt/emqx/etc/certs/server.crt"
    cacertfile = "/opt/emqx/etc/certs/ca.crt"
    verify = verify_none
    fail_if_no_peer_cert = false
  }
}
listeners.tcp.default.bind = "127.0.0.1:1883"
listeners.ws.default.bind = "127.0.0.1:8083"
listeners.wss.default.bind = "127.0.0.1:8084"
dashboard { listeners.http.bind = "127.0.0.1:18083" }
authentication = [{mechanism = password_based, backend = built_in_database, user_id_type = username, enable = true}]
authorization { no_match = deny deny_action = disconnect sources = [{type = file, enable = true, path = "/opt/emqx/etc/acl.conf"}] }
retainer { enable = true max_retained_messages = 1000 }
EOF

cat > "$BASE/config/acl.conf" <<'EOF'
{allow, {username, "smartagri-backend"}, all, ["smartagri/v1/devices/#"]}.
{allow, {username, "gateway-greenhouse-001"}, publish, ["smartagri/v1/devices/greenhouse-001/telemetry", "smartagri/v1/devices/greenhouse-001/state", "smartagri/v1/devices/greenhouse-001/availability", "smartagri/v1/devices/greenhouse-001/command-results"]}.
{allow, {username, "gateway-greenhouse-001"}, subscribe, ["smartagri/v1/devices/greenhouse-001/commands"]}.
{deny, all}.
EOF

if [ ! -f "$BASE/certs/ca.key" ]; then
  openssl genrsa -out "$BASE/certs/ca.key" 4096
  openssl req -x509 -new -nodes -key "$BASE/certs/ca.key" -sha256 -days 3650 -out "$BASE/certs/ca.crt" -subj "/CN=SmartAgri EMQX Root CA"
fi

cat > "$BASE/certs/server.cnf" <<'EOF'
[req]
distinguished_name=req_dn
req_extensions=req_ext
prompt=no
[req_dn]
CN=emqx.x1a0yu.top
[req_ext]
subjectAltName=@alt_names
[alt_names]
DNS.1=emqx.x1a0yu.top
IP.1=82.157.54.145
EOF
openssl genrsa -out "$BASE/certs/server.key" 2048
openssl req -new -key "$BASE/certs/server.key" -out "$BASE/certs/server.csr" -config "$BASE/certs/server.cnf"
openssl x509 -req -in "$BASE/certs/server.csr" -CA "$BASE/certs/ca.crt" -CAkey "$BASE/certs/ca.key" -CAcreateserial -out "$BASE/certs/server.crt" -days 825 -sha256 -extensions req_ext -extfile "$BASE/certs/server.cnf"
chmod 600 "$BASE/certs"/*.key

cat > "$BASE/compose.yml" <<'EOF'
services:
  emqx:
    image: emqx/emqx:5.8.7
    container_name: emqx
    restart: unless-stopped
    ports:
      - "8883:8883"
      - "127.0.0.1:18085:18083"
    environment:
      EMQX_NODE_NAME: emqx@prod-node1
    volumes:
      - ./data:/opt/emqx/data
      - ./log:/opt/emqx/log
      - ./config/emqx.conf:/opt/emqx/etc/emqx.conf:ro
      - ./config/acl.conf:/opt/emqx/etc/acl.conf:ro
      - ./certs:/opt/emqx/etc/certs:ro
EOF

cd "$BASE"
docker compose down
rm -rf "$BASE/data"
mkdir -p "$BASE/data"
docker compose up -d

for _ in $(seq 1 30); do
  if curl -fsS http://127.0.0.1:18085/api/v5/status >/tmp/emqx-status 2>/dev/null; then
    break
  fi
  sleep 2
done
curl -fsS http://127.0.0.1:18085/api/v5/status
printf 'BACKUP=%s\n' "$BACKUP"
printf 'CA=%s/certs/ca.crt\n' "$BASE"
