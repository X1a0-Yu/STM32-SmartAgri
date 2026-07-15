import json
import tempfile
import unittest
from pathlib import Path

from gateway.config import load_config
from gateway.spool import SQLiteSpool
from gateway.bridge import CommandBridge


class ConfigTests(unittest.TestCase):
    def test_tls_is_required(self):
        with self.assertRaises(ValueError):
            load_config({"SMARTAGRI_GATEWAY_DEVICE_ID": "greenhouse-001", "SMARTAGRI_GATEWAY_SERIAL_PORT": "COM6", "SMARTAGRI_GATEWAY_MQTT_HOST": "broker", "SMARTAGRI_GATEWAY_MQTT_TLS": "false"})


class SpoolTests(unittest.TestCase):
    def test_spool_prioritizes_command_results_and_deduplicates(self):
        with tempfile.TemporaryDirectory() as directory:
            spool = SQLiteSpool(Path(directory) / "spool.sqlite3", max_rows=2)
            spool.open()
            spool.enqueue("telemetry", b"one")
            spool.enqueue("telemetry", b"two")
            spool.enqueue("result", b"three", priority=10)
            self.assertEqual([item.payload for item in spool.peek()], [b"three", b"two"])
            spool.remember_completed("request-1", b'{"status":"succeeded"}')
            self.assertEqual(spool.completed_result("request-1"), b'{"status":"succeeded"}')
            spool.close()


class FakeSerial:
    def __init__(self):
        self.command = None

    def execute(self, factory, timeout):
        self.command = factory("123")
        return {"type": "ack", "request_id": "123", "parts": ["CTRL"], "raw": "ACK,123,CTRL"}


class BridgeTests(unittest.TestCase):
    def setUp(self):
        self.config = load_config({"SMARTAGRI_GATEWAY_DEVICE_ID": "greenhouse-001", "SMARTAGRI_GATEWAY_SERIAL_PORT": "COM6", "SMARTAGRI_GATEWAY_MQTT_HOST": "broker"})
        self.directory = tempfile.TemporaryDirectory()
        self.spool = SQLiteSpool(Path(self.directory.name) / "spool.sqlite3")
        self.spool.open()
        self.serial = FakeSerial()
        self.messages = []
        self.bridge = CommandBridge(self.config, self.spool, self.serial, lambda *args: self.messages.append(args))

    def tearDown(self):
        self.spool.close()
        self.directory.cleanup()

    def test_control_is_serialized_and_duplicate_is_not_repeated(self):
        payload = json.dumps({"request_id": "api-1", "device_id": "greenhouse-001", "command": "ctrl", "controls": {"pump": "on"}, "ttl_seconds": 60}).encode()
        self.bridge.handle(payload)
        self.assertEqual(self.serial.command, "CTRL,123,PUMP=ON,TTL=60\r\n")
        self.bridge.handle(payload)
        self.assertEqual(self.serial.command, "CTRL,123,PUMP=ON,TTL=60\r\n")
        self.assertEqual(self.messages[-1][1]["status"], "succeeded")

    def test_bad_command_never_reaches_serial(self):
        self.bridge.handle(b'{"request_id":"api-2","command":"raw","raw":"CTRL"}')
        self.assertIsNone(self.serial.command)
        self.assertEqual(self.messages[-1][1]["status"], "rejected")


if __name__ == "__main__":
    unittest.main()
