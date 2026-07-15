#!/usr/bin/env bash
set -euo pipefail
BASE=/www/wwwroot/mqtt
TOKEN="$1"
api="http://127.0.0.1:18085/api/v5"
create_user() {
  local username="$1" password="$2"
  curl -fsS -X POST \
    -H "Authorization: Bearer ${TOKEN}" \
    -H 'Content-Type: application/json' \
    "${api}/authentication/password_based:built_in_database/users" \
    --data "{\"user_id\":\"${username}\",\"password\":\"${password}\",\"is_superuser\":false}"
}
create_user gateway-greenhouse-001 "$(cat "${BASE}/secrets/gateway_password")"
create_user smartagri-backend "$(cat "${BASE}/secrets/backend_password")"
curl -fsS -X PUT \
  -H "Authorization: Bearer ${TOKEN}" \
  -H 'Content-Type: application/json' \
  "${api}/users/admin" \
  --data "{\"password\":\"$(cat "${BASE}/secrets/dashboard_password")\"}"
