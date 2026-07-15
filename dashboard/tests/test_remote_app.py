import unittest

from smartagri.remote_app import create_remote_app


class FakePublisher:
    def __init__(self):
        self.messages = []

    def publish(self, device_id, payload):
        self.messages.append((device_id, payload))


class RemoteAppTests(unittest.TestCase):
    def setUp(self):
        self.publisher = FakePublisher()
        self.app = create_remote_app({"DATABASE_URL": "sqlite:///:memory:", "SECRET_KEY": "test", "PUBLISHER": self.publisher})
        self.client = self.app.test_client()
        self.client.post("/api/v1/auth/bootstrap", json={"username": "operator", "password": "long-enough-password"})
        self.client.post("/api/v1/auth/login", json={"username": "operator", "password": "long-enough-password"})
        sessions = self.app.extensions["smartagri_remote"]["sessions"]
        from smartagri.remote_database import Device
        with sessions() as db:
            db.add(Device(device_id="greenhouse-001", display_name="Test greenhouse"))
            db.commit()

    def test_command_is_audited_and_published(self):
        response = self.client.post("/api/v1/devices/greenhouse-001/commands", json={"command": "ctrl", "actuator": "pump", "mode": "on", "ttl_seconds": 60})
        self.assertEqual(response.status_code, 202)
        self.assertEqual(response.json["status"], "published")
        self.assertEqual(self.publisher.messages[0][0], "greenhouse-001")
        request_id = response.json["request_id"]
        status = self.client.get(f"/api/v1/devices/greenhouse-001/commands/{request_id}")
        self.assertEqual(status.json["actor"], "operator")

    def test_login_is_required(self):
        client = create_remote_app({"DATABASE_URL": "sqlite:///:memory:", "SECRET_KEY": "test"}).test_client()
        self.assertEqual(client.get("/api/v1/devices").status_code, 401)


if __name__ == "__main__":
    unittest.main()
