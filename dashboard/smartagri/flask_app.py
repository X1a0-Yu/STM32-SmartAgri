from __future__ import annotations
import json, queue, threading, time
from pathlib import Path
from flask import Flask, jsonify, request, Response, send_from_directory
from .protocol import set_command, ctrl_command
from .state_store import StateStore
from .flask_serial import FlaskSerialManager

VERSION="1.1.0"

def create_app(port=None,baud=19200,demo=False,ingest_token=""):
    static=Path(__file__).resolve().parent.parent/"static"
    app=Flask(__name__,static_folder=str(static),static_url_path="/static")
    store=StateStore();manager=FlaskSerialManager(store,port,baud,demo);manager.start();app.extensions["smartagri"]={"store":store,"manager":manager,"token":ingest_token}
    @app.get("/")
    def index():return send_from_directory(static,"index.html")
    @app.get("/api/v1/health")
    def health():return jsonify(ok=True,version=VERSION,serial=store.serial,has_data=store.telemetry is not None)
    @app.get("/api/v1/system")
    def system():return jsonify(version=VERSION,protocol=2,mcu="STM32F103C8T6",baud=baud)
    @app.get("/api/v1/state")
    def state():return jsonify(store.snapshot())
    @app.get("/api/v1/telemetry")
    def telemetry():return jsonify(data=store.telemetry,received_at=store.received_at)
    @app.get("/api/v1/history")
    def history():
        limit=max(1,min(int(request.args.get("limit",300)),300));return jsonify(list(store.history)[-limit:])
    @app.get("/api/v1/thresholds")
    def thresholds():return jsonify(store.thresholds())
    @app.get("/api/v1/controls")
    def controls():return jsonify(store.controls())
    @app.get("/api/v1/warnings")
    def warnings():return jsonify(store.warnings())
    @app.get("/api/v1/serial/status")
    def serial_status():return jsonify(store.serial)
    @app.get("/api/v1/serial/ports")
    def ports():return jsonify(manager.ports())
    @app.post("/api/v1/serial/connect")
    def serial_connect():
        body=request.get_json(force=True);manager.connect(body["port"],int(body.get("baud",19200)));return jsonify(ok=True)
    @app.delete("/api/v1/serial/connect")
    def serial_disconnect():manager.disconnect();return jsonify(ok=True)
    def command(factory):
        try:return jsonify(manager.command(factory))
        except ConnectionError as e:return jsonify(detail=str(e)),503
        except TimeoutError:return jsonify(detail="MCU command timeout"),504
    @app.put("/api/v1/thresholds")
    def set_thresholds():
        b=request.get_json(force=True);keys={"TMIN":"temp_min","TMAX":"temp_max","HMIN":"humi_min","HMAX":"humi_max","AIRMAX":"air_max","LMIN":"light_min","SMIN":"soil_min"}
        try:data={k:int(b[v]) for k,v in keys.items()}
        except Exception:return jsonify(detail="invalid threshold body"),422
        if data["TMIN"]>data["TMAX"] or data["HMIN"]>data["HMAX"]:return jsonify(detail="threshold conflict"),409
        return command(lambda rid:set_command(rid,data))
    @app.put("/api/v1/controls/<actuator>")
    def control(actuator):
        if actuator not in {"pump","fan","lamp","beep"}:return jsonify(detail="unknown actuator"),404
        b=request.get_json(force=True);mode=b.get("mode","auto");ttl=b.get("ttl_seconds")
        if mode not in {"auto","on","off"}:return jsonify(detail="invalid mode"),422
        if mode!="auto" and ttl is None:ttl=60 if actuator in {"pump","beep"} else 300
        return command(lambda rid:ctrl_command(rid,{actuator:mode},ttl))
    @app.post("/api/v1/controls/auto")
    def all_auto():return command(lambda rid:ctrl_command(rid,{x:"auto" for x in ("pump","fan","lamp","beep")}))
    @app.get("/api/v1/wireless")
    def wireless():return jsonify(store.wireless())
    @app.post("/api/v1/wireless/lora/power")
    def lora_power():
        enabled=bool(request.get_json(force=True).get("enabled"));return command(lambda rid:f"LORA,{rid},POWER,ON={1 if enabled else 0}\r\n")
    @app.post("/api/v1/wireless/lora/test")
    def lora_test():return command(lambda rid:f"LORA,{rid},PING\r\n")
    @app.post("/api/v1/wireless/wifi/power")
    def wifi_power():
        enabled=bool(request.get_json(force=True).get("enabled"));return command(lambda rid:f"WIFI,{rid},POWER,ON={1 if enabled else 0}\r\n")
    @app.post("/api/v1/ingest/telemetry")
    def ingest():
        if ingest_token and request.headers.get("X-SmartAgri-Token")!=ingest_token:return jsonify(detail="unauthorized"),401
        data=request.get_json(force=True);store.update_remote(data);return jsonify(ok=True)
    @app.get("/api/v1/events")
    def events():
        q=store.subscribe()
        def stream():
            try:
                yield f"event: snapshot\ndata: {json.dumps(store.snapshot(),ensure_ascii=False)}\n\n"
                while True:
                    try:event=q.get(timeout=15);yield f"event: snapshot\ndata: {json.dumps(event,ensure_ascii=False)}\n\n"
                    except queue.Empty:yield ": heartbeat\n\n"
            finally:store.unsubscribe(q)
        return Response(stream(),mimetype="text/event-stream",headers={"Cache-Control":"no-cache","X-Accel-Buffering":"no"})
    return app
