from __future__ import annotations

import os
import signal
import threading

from .mqtt_worker import RemoteMqttWorker
from .remote_database import create_session_factory


def main() -> None:
    worker = RemoteMqttWorker(
        create_session_factory(os.environ["DATABASE_URL"]),
        os.environ["MQTT_HOST"],
        int(os.environ.get("MQTT_PORT", "8883")),
        os.environ["MQTT_USERNAME"],
        os.environ["MQTT_PASSWORD"],
        os.environ.get("MQTT_CA_PATH") or None,
    )
    stopped = threading.Event()
    signal.signal(signal.SIGINT, lambda *_: stopped.set())
    signal.signal(signal.SIGTERM, lambda *_: stopped.set())
    worker.start()
    try:
        stopped.wait()
    finally:
        worker.stop()


if __name__ == "__main__":
    main()
