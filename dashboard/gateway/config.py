from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import os
import re

_DEVICE_ID = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$")


@dataclass(frozen=True)
class GatewayConfig:
    device_id: str
    gateway_id: str
    serial_port: str
    serial_baud: int
    mqtt_host: str
    mqtt_port: int
    mqtt_username: str | None
    mqtt_password: str | None
    mqtt_tls: bool
    mqtt_ca_path: str | None
    mqtt_keepalive: int
    spool_path: Path
    spool_max_rows: int
    command_timeout_seconds: float

    @property
    def topic_root(self) -> str:
        return f"smartagri/v1/devices/{self.device_id}"

    @property
    def telemetry_topic(self) -> str:
        return f"{self.topic_root}/telemetry"

    @property
    def state_topic(self) -> str:
        return f"{self.topic_root}/state"

    @property
    def availability_topic(self) -> str:
        return f"{self.topic_root}/availability"

    @property
    def command_topic(self) -> str:
        return f"{self.topic_root}/commands"

    @property
    def command_result_topic(self) -> str:
        return f"{self.topic_root}/command-results"


def _integer(env: dict[str, str], name: str, default: int) -> int:
    value = int(env.get(name, str(default)))
    if value <= 0:
        raise ValueError(f"{name} must be positive")
    return value


def _decimal(env: dict[str, str], name: str, default: float) -> float:
    value = float(env.get(name, str(default)))
    if value <= 0:
        raise ValueError(f"{name} must be positive")
    return value


def _boolean(value: str) -> bool:
    return value.strip().lower() in {"1", "true", "yes", "on"}


def load_config(env: dict[str, str] | None = None) -> GatewayConfig:
    env = dict(os.environ if env is None else env)
    device_id = env.get("SMARTAGRI_GATEWAY_DEVICE_ID", "").strip()
    if not _DEVICE_ID.fullmatch(device_id):
        raise ValueError("SMARTAGRI_GATEWAY_DEVICE_ID must be an MQTT-safe identifier")
    serial_port = env.get("SMARTAGRI_GATEWAY_SERIAL_PORT", "").strip()
    mqtt_host = env.get("SMARTAGRI_GATEWAY_MQTT_HOST", "").strip()
    if not serial_port or not mqtt_host:
        raise ValueError("serial port and MQTT host are required")
    tls = _boolean(env.get("SMARTAGRI_GATEWAY_MQTT_TLS", "true"))
    if not tls:
        raise ValueError("plaintext MQTT is disabled; configure TLS")
    return GatewayConfig(
        device_id=device_id,
        gateway_id=env.get("SMARTAGRI_GATEWAY_ID", f"gateway-{device_id}").strip(),
        serial_port=serial_port,
        serial_baud=_integer(env, "SMARTAGRI_GATEWAY_SERIAL_BAUD", 19200),
        mqtt_host=mqtt_host,
        mqtt_port=_integer(env, "SMARTAGRI_GATEWAY_MQTT_PORT", 8883),
        mqtt_username=env.get("SMARTAGRI_GATEWAY_MQTT_USERNAME") or None,
        mqtt_password=env.get("SMARTAGRI_GATEWAY_MQTT_PASSWORD") or None,
        mqtt_tls=tls,
        mqtt_ca_path=env.get("SMARTAGRI_GATEWAY_MQTT_CA_PATH") or None,
        mqtt_keepalive=_integer(env, "SMARTAGRI_GATEWAY_MQTT_KEEPALIVE", 30),
        spool_path=Path(env.get("SMARTAGRI_GATEWAY_SPOOL_PATH", "./data/gateway-spool.sqlite3")),
        spool_max_rows=_integer(env, "SMARTAGRI_GATEWAY_SPOOL_MAX_ROWS", 10000),
        command_timeout_seconds=_decimal(env, "SMARTAGRI_GATEWAY_COMMAND_TIMEOUT", 5),
    )
