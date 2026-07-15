#!/usr/bin/env bash
set -euo pipefail
BASE=/www/wwwroot/mqtt/certs
cat > "$BASE/ca.cnf" <<'EOF'
[req]
distinguished_name = dn
x509_extensions = v3_ca
prompt = no
[dn]
CN = SmartAgri EMQX Root CA
[v3_ca]
basicConstraints = critical,CA:true
keyUsage = critical,keyCertSign,cRLSign
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer
EOF
cat > "$BASE/server.cnf" <<'EOF'
[req]
distinguished_name = dn
prompt = no
[dn]
CN = emqx.x1a0yu.top
[v3_server]
basicConstraints = critical,CA:false
keyUsage = critical,digitalSignature,keyEncipherment
extendedKeyUsage = serverAuth
subjectAltName = @alt_names
[alt_names]
DNS.1 = emqx.x1a0yu.top
IP.1 = 82.157.54.145
EOF
rm -f "$BASE/ca.key" "$BASE/ca.crt" "$BASE/ca.srl" "$BASE/server.key" "$BASE/server.csr" "$BASE/server.crt"
openssl genrsa -out "$BASE/ca.key" 4096
openssl req -x509 -new -key "$BASE/ca.key" -sha256 -days 3650 -out "$BASE/ca.crt" -config "$BASE/ca.cnf"
openssl genrsa -out "$BASE/server.key" 2048
openssl req -new -key "$BASE/server.key" -out "$BASE/server.csr" -config "$BASE/server.cnf"
openssl x509 -req -in "$BASE/server.csr" -CA "$BASE/ca.crt" -CAkey "$BASE/ca.key" -CAcreateserial -out "$BASE/server.crt" -days 825 -sha256 -extfile "$BASE/server.cnf" -extensions v3_server
chown -R ubuntu:netdev "$BASE"
chmod 640 "$BASE"/*.key
chmod 644 "$BASE"/*.crt
cp "$BASE/ca.crt" /www/wwwroot/smartagri/secrets/emqx-ca.crt
chmod 644 /www/wwwroot/smartagri/secrets/emqx-ca.crt
cd /www/wwwroot/mqtt
docker compose restart emqx
