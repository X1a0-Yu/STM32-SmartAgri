from __future__ import annotations

import json
import ssl


class MqttCommandPublisher:
    def __init__(self, host: str, port: int, username: str, password: str, ca_path: str | None = None) -> None:
        self.host, self.port, self.username, self.password, self.ca_path = host, port, username, password, ca_path
        self.client = None

    def start(self) -> None:
        import paho.mqtt.client as mqtt
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="smartagri-remote-publisher", protocol=mqtt.MQTTv311)
        self.client.username_pw_set(self.username, self.password)
        self.client.tls_set_context(ssl.create_default_context(cafile=self.ca_path))
        self.client.connect(self.host, self.port, 30)
        self.client.loop_start()

    def stop(self) -> None:
        if self.client:
            self.client.disconnect()
            self.client.loop_stop()

    def publish(self, device_id: str, payload: dict) -> None:
        if not self.client:
            raise RuntimeError("MQTT publisher is not started")
        result = self.client.publish(f"smartagri/v1/devices/{device_id}/commands", json.dumps(payload), qos=1, retain=False)
        if result.rc != 0:
            raise RuntimeError("MQTT command publish failed")
