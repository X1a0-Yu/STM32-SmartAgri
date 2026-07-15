from __future__ import annotations

import json
from datetime import datetime, timezone
from typing import Callable

from smartagri.protocol import ctrl_command, get_command, set_command

from .config import GatewayConfig
from .spool import SQLiteSpool


def utcnow() -> str:
    return datetime.now(timezone.utc).isoformat()


class CommandBridge:
    def __init__(self, config: GatewayConfig, spool: SQLiteSpool, serial, publish: Callable[[str, dict, bool, int], None]) -> None:
        self.config, self.spool, self.serial, self.publish = config, spool, serial, publish

    def handle(self, raw_payload: bytes) -> None:
        request_id = self._safe_request_id(raw_payload)
        try:
            message = json.loads(raw_payload.decode("utf-8"))
            request_id = self._request_id(message)
            cached = self.spool.completed_result(request_id)
            if cached:
                self.publish(self.config.command_result_topic, json.loads(cached), False, 1)
                return
            factory = self._factory(message)
            self._result(request_id, "received", accepted=True)
            result = self.serial.execute(factory, self.config.command_timeout_seconds)
            status = "succeeded" if result["type"] == "ack" else "failed"
            self._result(request_id, status, accepted=result["type"] == "ack", mcu_result=result)
        except (ValueError, KeyError, TypeError) as exc:
            request_id = self._safe_request_id(raw_payload)
            self._result(request_id, "rejected", accepted=False, error=str(exc))
        except ConnectionError as exc:
            self._result(request_id, "failed", accepted=False, error=str(exc))
        except TimeoutError as exc:
            self._result(request_id, "timed_out", accepted=False, error=str(exc))

    def _request_id(self, message: dict) -> str:
        request_id = message.get("request_id")
        if not isinstance(request_id, str) or not request_id or len(request_id) > 100:
            raise ValueError("request_id is required and must be at most 100 characters")
        if any(character in request_id for character in ",\r\n"):
            raise ValueError("request_id contains invalid characters")
        if message.get("device_id") not in {None, self.config.device_id}:
            raise ValueError("command targets another device")
        return request_id

    def _safe_request_id(self, raw_payload: bytes) -> str:
        try:
            value = json.loads(raw_payload.decode("utf-8")).get("request_id", "unknown")
            return value if isinstance(value, str) and value else "unknown"
        except Exception:
            return "unknown"

    def _factory(self, message: dict):
        command = message.get("command")
        if command == "get":
            return get_command
        if command == "set":
            data = message.get("data")
            required = ("TMIN", "TMAX", "HMIN", "HMAX", "AIRMAX", "LMIN", "SMIN")
            if not isinstance(data, dict) or any(key not in data for key in required):
                raise ValueError("set command requires complete threshold data")
            normalized = {key: int(data[key]) for key in required}
            if normalized["TMIN"] > normalized["TMAX"] or normalized["HMIN"] > normalized["HMAX"]:
                raise ValueError("threshold bounds conflict")
            return lambda mcu_request_id: set_command(mcu_request_id, normalized)
        if command == "ctrl":
            controls = message.get("controls")
            if not isinstance(controls, dict) or not controls:
                raise ValueError("ctrl command requires controls")
            normalized = {str(key).lower(): str(value).lower() for key, value in controls.items()}
            if set(normalized) - {"pump", "fan", "lamp", "beep"} or any(value not in {"auto", "on", "off"} for value in normalized.values()):
                raise ValueError("invalid actuator control")
            ttl = message.get("ttl_seconds")
            if ttl is not None:
                ttl = int(ttl)
                if not 1 <= ttl <= 3600:
                    raise ValueError("ttl_seconds must be between 1 and 3600")
            return lambda mcu_request_id: ctrl_command(mcu_request_id, normalized, ttl)
        raise ValueError("unsupported command")

    def _result(self, request_id: str, status: str, *, accepted: bool, mcu_result: dict | None = None, error: str | None = None) -> None:
        payload = {"schema": 1, "device_id": self.config.device_id, "gateway_id": self.config.gateway_id, "request_id": request_id, "status": status, "accepted": accepted, "reported_at": utcnow()}
        if mcu_result is not None:
            payload["mcu_result"] = mcu_result
        if error:
            payload["error"] = error
        encoded = json.dumps(payload, ensure_ascii=False, separators=(",", ":")).encode()
        if status in {"succeeded", "failed", "timed_out", "rejected"}:
            self.spool.remember_completed(request_id, encoded)
        self.publish(self.config.command_result_topic, payload, False, 1)
