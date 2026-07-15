from __future__ import annotations

from .bridge import CommandBridge
from .config import GatewayConfig
from .mqtt_transport import MqttTransport
from .serial_transport import SerialTransport
from .spool import SQLiteSpool


class GatewayService:
    def __init__(self, config: GatewayConfig) -> None:
        self.config = config
        self.spool = SQLiteSpool(config.spool_path, config.spool_max_rows)
        self.mqtt: MqttTransport | None = None
        self.serial = SerialTransport(config.serial_port, config.serial_baud, self._serial_event)
        self.bridge: CommandBridge | None = None

    def start(self) -> None:
        self.spool.open()
        self.mqtt = MqttTransport(self.config, self.spool, self._command)
        self.bridge = CommandBridge(self.config, self.spool, self.serial, self._publish)
        self.mqtt.start()
        self.serial.start()

    def stop(self) -> None:
        self.serial.stop()
        if self.mqtt:
            self.mqtt.stop()
        self.spool.close()

    def _publish(self, topic: str, payload: dict, retain: bool = False, priority: int = 0) -> None:
        if self.mqtt:
            self.mqtt.publish(topic, payload, retain, priority)

    def _command(self, payload: bytes) -> None:
        if self.bridge:
            self.bridge.handle(payload)

    def _serial_event(self, event: dict) -> None:
        if event["type"] == "telemetry":
            self._publish(self.config.telemetry_topic, {"data": event["data"], "raw": event["raw"]})
            self._publish(self.config.state_topic, {"data": event["data"]}, retain=True, priority=5)
        elif event["type"] == "serial_status":
            self._publish(self.config.availability_topic, {"online": bool(event["connected"]), "serial": event}, retain=True, priority=10)
