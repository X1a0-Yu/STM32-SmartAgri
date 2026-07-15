from __future__ import annotations

import logging
import signal
import threading

from .config import load_config
from .service import GatewayService


def main() -> None:
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(name)s: %(message)s")
    service = GatewayService(load_config())
    stopped = threading.Event()

    def stop(*_args) -> None:
        stopped.set()

    signal.signal(signal.SIGINT, stop)
    signal.signal(signal.SIGTERM, stop)
    service.start()
    try:
        stopped.wait()
    finally:
        service.stop()


if __name__ == "__main__":
    main()
