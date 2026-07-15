from __future__ import annotations

import os
import secrets
import uuid
from functools import wraps
from pathlib import Path

from flask import Flask, jsonify, request, send_from_directory, session
from werkzeug.security import check_password_hash, generate_password_hash
from sqlalchemy import select

from .mqtt_publisher import MqttCommandPublisher
from .remote_database import Command, Device, DeviceState, TelemetrySample, User, create_session_factory

VERSION = "1.2.0"


def create_remote_app(config: dict | None = None) -> Flask:
    config = dict(config or {})
    static = Path(__file__).resolve().parent.parent / "static"
    app = Flask(__name__, static_folder=str(static), static_url_path="/static")
    app.secret_key = config.get("SECRET_KEY") or os.environ.get("FLASK_SECRET_KEY") or secrets.token_urlsafe(32)
    database_url = config.get("DATABASE_URL") or os.environ.get("DATABASE_URL", "sqlite:///smartagri.db")
    session_factory = config.get("SESSION_FACTORY") or create_session_factory(database_url)
    publisher = config.get("PUBLISHER")
    app.extensions["smartagri_remote"] = {"sessions": session_factory, "publisher": publisher}

    def database():
        return session_factory()

    def current_user():
        username = session.get("username")
        if not username:
            return None
        with database() as db:
            return db.scalar(select(User).where(User.username == username, User.active.is_(True)))

    def require_role(*roles):
        def decorator(view):
            @wraps(view)
            def wrapped(*args, **kwargs):
                user = current_user()
                if not user:
                    return jsonify(detail="authentication required"), 401
                if user.role not in roles:
                    return jsonify(detail="operator role required"), 403
                return view(*args, **kwargs)
            return wrapped
        return decorator

    def device_or_404(db, device_id: str):
        device = db.scalar(select(Device).where(Device.device_id == device_id, Device.enabled.is_(True)))
        if not device:
            return None
        return device

    def command_payload(device_id: str, body: dict, kind: str) -> dict:
        payload = {"schema": 1, "device_id": device_id, "request_id": str(uuid.uuid4()), "command": kind}
        if kind == "set":
            mapping = {"TMIN": "temp_min", "TMAX": "temp_max", "HMIN": "humi_min", "HMAX": "humi_max", "AIRMAX": "air_max", "LMIN": "light_min", "SMIN": "soil_min"}
            try:
                data = {key: int(body[value]) for key, value in mapping.items()}
            except (KeyError, TypeError, ValueError):
                raise ValueError("invalid threshold body")
            if data["TMIN"] > data["TMAX"] or data["HMIN"] > data["HMAX"]:
                raise ValueError("threshold conflict")
            payload["data"] = data
        elif kind == "ctrl":
            actuator = body.get("actuator")
            mode = body.get("mode", "auto")
            if actuator not in {"pump", "fan", "lamp", "beep"} or mode not in {"auto", "on", "off"}:
                raise ValueError("invalid actuator command")
            ttl = body.get("ttl_seconds")
            if ttl is not None:
                ttl = int(ttl)
            elif mode != "auto":
                ttl = 60 if actuator in {"pump", "beep"} else 300
            payload.update(controls={actuator: mode}, ttl_seconds=ttl)
        else:
            raise ValueError("unsupported command")
        return payload

    @app.get("/")
    def index():
        return send_from_directory(static, "index.html")

    @app.get("/api/v1/health")
    def health():
        return jsonify(ok=True, version=VERSION, mode="remote-mqtt")

    @app.post("/api/v1/auth/bootstrap")
    def bootstrap():
        with database() as db:
            if db.scalar(select(User.id).limit(1)) is not None:
                return jsonify(detail="bootstrap already completed"), 409
            body = request.get_json(force=True)
            username, password = body.get("username", ""), body.get("password", "")
            if not username or len(password) < 12:
                return jsonify(detail="username and a 12-character password are required"), 422
            db.add(User(username=username, password_hash=generate_password_hash(password), role="operator"))
            db.commit()
        return jsonify(ok=True), 201

    @app.post("/api/v1/auth/login")
    def login():
        body = request.get_json(force=True)
        with database() as db:
            user = db.scalar(select(User).where(User.username == body.get("username", ""), User.active.is_(True)))
            if not user or not check_password_hash(user.password_hash, body.get("password", "")):
                return jsonify(detail="invalid credentials"), 401
            session.clear()
            session["username"] = user.username
        return jsonify(username=user.username, role=user.role)

    @app.post("/api/v1/auth/logout")
    def logout():
        session.clear()
        return jsonify(ok=True)

    @app.get("/api/v1/auth/me")
    def me():
        user = current_user()
        if not user:
            return jsonify(authenticated=False)
        return jsonify(authenticated=True, username=user.username, role=user.role)

    @app.get("/api/v1/devices")
    @require_role("viewer", "operator")
    def devices():
        with database() as db:
            records = db.scalars(select(Device).where(Device.enabled.is_(True)).order_by(Device.device_id)).all()
            return jsonify([{"device_id": item.device_id, "display_name": item.display_name, "gateway_id": item.gateway_id, "last_seen_at": item.last_seen_at} for item in records])

    @app.get("/api/v1/devices/<device_id>/state")
    @require_role("viewer", "operator")
    def state(device_id):
        with database() as db:
            if not device_or_404(db, device_id):
                return jsonify(detail="device not found"), 404
            item = db.get(DeviceState, device_id)
            return jsonify(device_id=device_id, data=item.data if item else None, availability=item.availability if item else {}, received_at=item.received_at if item else None)

    @app.get("/api/v1/devices/<device_id>/telemetry")
    @require_role("viewer", "operator")
    def telemetry(device_id):
        limit = max(1, min(int(request.args.get("limit", 300)), 1000))
        with database() as db:
            if not device_or_404(db, device_id):
                return jsonify(detail="device not found"), 404
            records = db.scalars(select(TelemetrySample).where(TelemetrySample.device_id == device_id).order_by(TelemetrySample.id.desc()).limit(limit)).all()
            return jsonify([{"received_at": item.received_at, "data": item.data} for item in reversed(records)])

    @app.post("/api/v1/devices/<device_id>/commands")
    @require_role("operator")
    def issue_command(device_id):
        body = request.get_json(force=True)
        try:
            payload = command_payload(device_id, body, body.get("command"))
        except (ValueError, TypeError) as exc:
            return jsonify(detail=str(exc)), 422
        with database() as db:
            if not device_or_404(db, device_id):
                return jsonify(detail="device not found"), 404
            command = Command(request_id=payload["request_id"], device_id=device_id, actor=session["username"], payload=payload, status="queued")
            db.add(command)
            db.commit()
        try:
            if not publisher:
                raise RuntimeError("MQTT publisher unavailable")
            publisher.publish(device_id, payload)
        except Exception as exc:
            with database() as db:
                item = db.get(Command, payload["request_id"])
                item.status, item.result = "publish_failed", {"error": str(exc)}
                db.commit()
            return jsonify(request_id=payload["request_id"], status="publish_failed"), 503
        with database() as db:
            item = db.get(Command, payload["request_id"])
            item.status = "published"
            db.commit()
        return jsonify(request_id=payload["request_id"], status="published"), 202

    @app.get("/api/v1/devices/<device_id>/commands/<request_id>")
    @require_role("viewer", "operator")
    def command_status(device_id, request_id):
        with database() as db:
            command = db.get(Command, request_id)
            if not command or command.device_id != device_id:
                return jsonify(detail="command not found"), 404
            return jsonify(request_id=command.request_id, device_id=command.device_id, actor=command.actor, status=command.status, payload=command.payload, result=command.result, created_at=command.created_at, updated_at=command.updated_at)

    return app


def create_configured_publisher() -> MqttCommandPublisher:
    publisher = MqttCommandPublisher(os.environ["MQTT_HOST"], int(os.environ.get("MQTT_PORT", "8883")), os.environ["MQTT_USERNAME"], os.environ["MQTT_PASSWORD"], os.environ.get("MQTT_CA_PATH") or None)
    publisher.start()
    return publisher
