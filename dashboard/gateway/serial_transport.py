from __future__ import annotations

import threading
import time
from pathlib import Path
from typing import Callable

from smartagri.protocol import get_command, parse_line


class SerialTransport:
    """The gateway's exclusive owner of the MCU serial port."""

    def __init__(self, port: str, baud: int, on_event: Callable[[dict], None]) -> None:
        self.port, self.baud, self.on_event = port, baud, on_event
        self.stop_event = threading.Event()
        self.connected = threading.Event()
        self.thread: threading.Thread | None = None
        self.serial = None
        self.lock = threading.RLock()
        self.pending: dict[str, dict] = {}
        self.counter = int(time.time()) % 900000 + 100000

    def _load_serial(self):
        import serial
        return serial

    def start(self) -> None:
        self.thread = threading.Thread(target=self._worker, name="smartagri-serial", daemon=True)
        self.thread.start()

    def stop(self) -> None:
        self.stop_event.set()
        with self.lock:
            if self.serial:
                self.serial.close()
                self.serial = None
        self.connected.clear()

    def _next_request_id(self) -> str:
        self.counter += 1
        return str(self.counter)

    def _worker(self) -> None:
        delay = 1.0
        while not self.stop_event.is_set():
            try:
                serial = self._load_serial()
                with self.lock:
                    self.serial = serial.Serial(self.port, self.baud, timeout=1)
                    self.connected.set()
                    self.serial.write(get_command(self._next_request_id()).encode("ascii"))
                self.on_event({"type": "serial_status", "connected": True, "port": self.port, "baud": self.baud})
                delay = 1.0
                while not self.stop_event.is_set() and self.serial and self.serial.is_open:
                    raw = self.serial.readline()
                    if raw:
                        self._handle(raw.decode("ascii", "replace"))
            except Exception as exc:
                self.connected.clear()
                self.on_event({"type": "serial_status", "connected": False, "port": self.port, "baud": self.baud, "error": str(exc)})
                self.stop_event.wait(delay)
                delay = min(delay * 2, 20)

    def _handle(self, line: str) -> None:
        try:
            event = parse_line(line)
        except ValueError:
            return
        if event["type"] in {"ack", "err"}:
            pending = self.pending.get(event["request_id"])
            if pending:
                pending["result"] = event
                pending["event"].set()
        self.on_event(event)

    def execute(self, factory: Callable[[str], str], timeout: float) -> dict:
        if not self.connected.is_set() or not self.serial:
            raise ConnectionError("MCU serial port is disconnected")
        request_id = self._next_request_id()
        pending = {"event": threading.Event(), "result": None}
        self.pending[request_id] = pending
        try:
            with self.lock:
                if not self.serial:
                    raise ConnectionError("MCU serial port is disconnected")
                self.serial.write(factory(request_id).encode("ascii"))
            if not pending["event"].wait(timeout):
                raise TimeoutError("MCU command timed out")
            return pending["result"]
        finally:
            self.pending.pop(request_id, None)
