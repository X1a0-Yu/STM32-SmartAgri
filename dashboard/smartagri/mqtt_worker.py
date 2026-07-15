from __future__ import annotations

import json
import logging
import ssl
from datetime import datetime, timezone

from sqlalchemy import select

from .remote_database import Command, Device, DeviceState, TelemetrySample


class RemoteMqttWorker:
    def __init__(self, session_factory, host: str, port: int, username: str, password: str, ca_path: str | None = None) -> None:
        self.session_factory, self.host, self.port = session_factory, host, port
        self.username, self.password, self.ca_path = username, password, ca_path
        self.client = None

    def start(self) -> None:
        import paho.mqtt.client as mqtt
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="smartagri-remote-worker", protocol=mqtt.MQTTv311)
        self.client.username_pw_set(self.username, self.password)
        self.client.tls_set_context(ssl.create_default_context(cafile=self.ca_path))
        self.client.on_connect = self._connected
        self.client.on_message = self._message
        self.client.connect_async(self.host, self.port, 30)
        self.client.loop_start()

    def stop(self) -> None:
        if self.client:
            self.client.disconnect()
            self.client.loop_stop()

    def publish_command(self, device_id: str, payload: dict) -> None:
        if not self.client:
            raise RuntimeError("MQTT worker is not started")
        info = self.client.publish(f"smartagri/v1/devices/{device_id}/commands", json.dumps(payload), qos=1, retain=False)
        if info.rc != 0:
            raise RuntimeError("MQTT command publish failed")

    def _connected(self, client, userdata, flags, reason_code, properties=None) -> None:
        if reason_code == 0:
            client.subscribe("smartagri/v1/devices/+/telemetry", qos=1)
            client.subscribe("smartagri/v1/devices/+/state", qos=1)
            client.subscribe("smartagri/v1/devices/+/availability", qos=1)
            client.subscribe("smartagri/v1/devices/+/command-results", qos=1)

    def _message(self, client, userdata, message) -> None:
        try:
            payload = json.loads(message.payload.decode("utf-8"))
            device_id = payload["device_id"]
            topic_device_id = message.topic.split("/")[4]
            if device_id != topic_device_id:
                raise ValueError("topic/device mismatch")
            with self.session_factory() as session:
                device = session.scalar(select(Device).where(Device.device_id == device_id))
                if not device:
                    device = Device(device_id=device_id, display_name=device_id)
                    session.add(device)
                device.last_seen_at = datetime.now(timezone.utc)
                if message.topic.endswith("/telemetry"):
                    data = payload["data"]
                    session.add(TelemetrySample(device_id=device_id, data=data, raw=payload.get("raw")))
                    state = session.get(DeviceState, device_id) or DeviceState(device_id=device_id, data={}, availability={})
                    state.data = data
                    state.received_at = datetime.now(timezone.utc)
                    session.merge(state)
                elif message.topic.endswith("/state"):
                    state = session.get(DeviceState, device_id) or DeviceState(device_id=device_id, data={}, availability={})
                    state.data = payload.get("data", {})
                    state.received_at = datetime.now(timezone.utc)
                    session.merge(state)
                elif message.topic.endswith("/availability"):
                    state = session.get(DeviceState, device_id) or DeviceState(device_id=device_id, data={}, availability={})
                    state.availability = payload
                    session.merge(state)
                elif message.topic.endswith("/command-results"):
                    command = session.get(Command, payload.get("request_id"))
                    if command:
                        command.status = payload.get("status", command.status)
                        command.result = payload
                session.commit()
        except Exception:
            logging.exception("Rejected MQTT message on %s", message.topic)
