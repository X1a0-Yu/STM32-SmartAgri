from __future__ import annotations

from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
import sqlite3
import threading


@dataclass(frozen=True)
class SpoolItem:
    id: int
    topic: str
    payload: bytes
    qos: int
    retain: bool
    priority: int


class SQLiteSpool:
    def __init__(self, path: Path, max_rows: int = 10000) -> None:
        self.path = path
        self.max_rows = max_rows
        self.connection: sqlite3.Connection | None = None
        self.lock = threading.RLock()

    def open(self) -> None:
        self.path.parent.mkdir(parents=True, exist_ok=True)
        self.connection = sqlite3.connect(self.path, check_same_thread=False)
        self.connection.execute("PRAGMA journal_mode=WAL")
        self.connection.execute("PRAGMA synchronous=FULL")
        self.connection.executescript(
            """
            CREATE TABLE IF NOT EXISTS mqtt_spool (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                topic TEXT NOT NULL,
                payload BLOB NOT NULL,
                qos INTEGER NOT NULL CHECK(qos IN (0, 1)),
                retain INTEGER NOT NULL CHECK(retain IN (0, 1)),
                priority INTEGER NOT NULL DEFAULT 0,
                created_at TEXT NOT NULL,
                attempts INTEGER NOT NULL DEFAULT 0
            );
            CREATE INDEX IF NOT EXISTS mqtt_spool_order_idx ON mqtt_spool(priority DESC, id);
            CREATE TABLE IF NOT EXISTS completed_commands (
                request_id TEXT PRIMARY KEY,
                result BLOB NOT NULL,
                completed_at TEXT NOT NULL
            );
            """
        )
        self.connection.commit()

    def close(self) -> None:
        if self.connection:
            self.connection.close()
            self.connection = None

    def _db(self) -> sqlite3.Connection:
        if not self.connection:
            raise RuntimeError("spool is not open")
        return self.connection

    def enqueue(self, topic: str, payload: bytes, *, qos: int = 1, retain: bool = False, priority: int = 0) -> int:
        with self.lock:
            cursor = self._db().execute(
                "INSERT INTO mqtt_spool(topic,payload,qos,retain,priority,created_at) VALUES(?,?,?,?,?,?)",
                (topic, payload, qos, int(retain), priority, datetime.now(timezone.utc).isoformat()),
            )
            self._trim()
            self._db().commit()
            return int(cursor.lastrowid)

    def _trim(self) -> None:
        overflow = self.depth() - self.max_rows
        if overflow > 0:
            self._db().execute(
                "DELETE FROM mqtt_spool WHERE id IN (SELECT id FROM mqtt_spool ORDER BY priority ASC, id ASC LIMIT ?)",
                (overflow,),
            )

    def peek(self, limit: int = 100) -> list[SpoolItem]:
        with self.lock:
            rows = self._db().execute(
                "SELECT id,topic,payload,qos,retain,priority FROM mqtt_spool ORDER BY priority DESC,id LIMIT ?", (limit,)
            ).fetchall()
        return [SpoolItem(row[0], row[1], row[2], row[3], bool(row[4]), row[5]) for row in rows]

    def acknowledge(self, item_id: int) -> None:
        with self.lock:
            self._db().execute("DELETE FROM mqtt_spool WHERE id=?", (item_id,))
            self._db().commit()

    def record_attempt(self, item_id: int) -> None:
        with self.lock:
            self._db().execute("UPDATE mqtt_spool SET attempts=attempts+1 WHERE id=?", (item_id,))
            self._db().commit()

    def depth(self) -> int:
        return int(self._db().execute("SELECT COUNT(*) FROM mqtt_spool").fetchone()[0])

    def completed_result(self, request_id: str) -> bytes | None:
        row = self._db().execute("SELECT result FROM completed_commands WHERE request_id=?", (request_id,)).fetchone()
        return row[0] if row else None

    def remember_completed(self, request_id: str, result: bytes) -> None:
        with self.lock:
            self._db().execute(
                "INSERT OR REPLACE INTO completed_commands(request_id,result,completed_at) VALUES(?,?,?)",
                (request_id, result, datetime.now(timezone.utc).isoformat()),
            )
            self._db().execute(
                "DELETE FROM completed_commands WHERE request_id NOT IN (SELECT request_id FROM completed_commands ORDER BY completed_at DESC LIMIT 1000)"
            )
            self._db().commit()
