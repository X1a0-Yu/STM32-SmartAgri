from __future__ import annotations

import json
import ssl
import threading
from datetime import datetime, timezone

from .config import GatewayConfig
from .spool import SQLiteSpool


def utcnow() -> str:
    return datetime.now(timezone.utc).isoformat()


class MqttTransport:
    def __init__(self, config: GatewayConfig, spool: SQLiteSpool, on_command) -> None:
        self.config, self.spool, self.on_command = config, spool, on_command
        self.client = None
        self.connected = threading.Event()

    def start(self) -> None:
        import paho.mqtt.client as mqtt
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=self.config.gateway_id, protocol=mqtt.MQTTv311)
        if self.config.mqtt_username:
            self.client.username_pw_set(self.config.mqtt_username, self.config.mqtt_password)
        context = ssl.create_default_context(cafile=self.config.mqtt_ca_path)
        self.client.tls_set_context(context)
        self.client.will_set(self.config.availability_topic, self._payload(False, "mqtt_connection_lost"), qos=1, retain=True)
        self.client.on_connect = self._on_connect
        self.client.on_disconnect = self._on_disconnect
        self.client.on_message = self._on_message
        self.client.connect_async(self.config.mqtt_host, self.config.mqtt_port, self.config.mqtt_keepalive)
        self.client.loop_start()

    def stop(self) -> None:
        if self.client:
            self.publish(self.config.availability_topic, {"online": False, "reason": "gateway_stopped"}, retain=True, priority=10)
            self.drain()
            self.client.disconnect()
            self.client.loop_stop()
        self.connected.clear()

    def _on_connect(self, client, userdata, flags, reason_code, properties=None) -> None:
        if reason_code != 0:
            return
        self.connected.set()
        client.subscribe(self.config.command_topic, qos=1)
        self.publish(self.config.availability_topic, {"online": True, "reason": "connected"}, retain=True, priority=10)
        self.drain()

    def _on_disconnect(self, client, userdata, disconnect_flags, reason_code, properties=None) -> None:
        self.connected.clear()

    def _on_message(self, client, userdata, message) -> None:
        if message.topic == self.config.command_topic:
            self.on_command(message.payload)

    def _payload(self, online: bool, reason: str) -> bytes:
        return json.dumps({"schema": 1, "device_id": self.config.device_id, "gateway_id": self.config.gateway_id, "online": online, "reason": reason, "reported_at": utcnow()}, separators=(",", ":")).encode()

    def publish(self, topic: str, data: dict, retain: bool = False, priority: int = 0) -> None:
        payload = dict(data)
        payload.setdefault("schema", 1)
        payload.setdefault("device_id", self.config.device_id)
        payload.setdefault("gateway_id", self.config.gateway_id)
        payload.setdefault("reported_at", utcnow())
        encoded = json.dumps(payload, ensure_ascii=False, separators=(",", ":")).encode()
        self.spool.enqueue(topic, encoded, retain=retain, priority=priority)
        self.drain()

    def drain(self) -> None:
        if not self.client or not self.connected.is_set():
            return
        for item in self.spool.peek():
            result = self.client.publish(item.topic, item.payload, qos=item.qos, retain=item.retain)
            if result.rc != 0:
                self.spool.record_attempt(item.id)
                break
            result.wait_for_publish(timeout=5)
            if result.is_published():
                self.spool.acknowledge(item.id)
            else:
                self.spool.record_attempt(item.id)
                break
