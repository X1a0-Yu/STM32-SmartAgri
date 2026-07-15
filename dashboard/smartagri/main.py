from __future__ import annotations
from contextlib import asynccontextmanager
from pathlib import Path
import asyncio
from fastapi import FastAPI, HTTPException, WebSocket, WebSocketDisconnect
from fastapi.responses import FileResponse
from fastapi.staticfiles import StaticFiles
from .state_store import StateStore
from .serial_manager import SerialManager
from .models import Thresholds,ControlRequest,MultiControlRequest,SerialConnect
from .protocol import set_command,ctrl_command

store=StateStore();manager:SerialManager|None=None

def configure(port=None,baud=9600,demo=False):
    global manager;manager=SerialManager(store,port,baud,demo)

@asynccontextmanager
async def lifespan(app):
    global manager
    if manager is None:configure(demo=True)
    manager.start(asyncio.get_running_loop());yield;manager.stop()

app=FastAPI(title="智慧农场控制 API",version="1.0.0",lifespan=lifespan)
static=Path(__file__).resolve().parent.parent/"static"
app.mount("/static",StaticFiles(directory=static),name="static")
@app.get("/")
def index():return FileResponse(static/"index.html")
@app.get("/api/v1/health")
def health():return {"ok":True,"serial":store.serial,"has_data":store.telemetry is not None}
@app.get("/api/v1/state")
def state():return store.snapshot()
@app.get("/api/v1/telemetry")
def telemetry():return {"data":store.telemetry,"received_at":store.received_at}
@app.get("/api/v1/history")
def history(limit:int=300):return list(store.history)[-max(1,min(limit,300)):]
@app.get("/api/v1/thresholds")
def thresholds():return store.thresholds()
@app.get("/api/v1/controls")
def controls():return store.controls()
@app.get("/api/v1/warnings")
def warnings():return store.warnings()
@app.get("/api/v1/serial/status")
def serial_status():return store.serial
@app.get("/api/v1/serial/ports")
def ports():return manager.ports()
async def issue(factory):
    try:return await manager.command(factory)
    except ConnectionError as e:raise HTTPException(503,str(e))
    except asyncio.TimeoutError:raise HTTPException(504,"MCU command timeout")
@app.put("/api/v1/thresholds")
async def put_thresholds(body:Thresholds):
    data={"TMIN":body.temp_min,"TMAX":body.temp_max,"HMIN":body.humi_min,"HMAX":body.humi_max,"AIRMAX":body.air_max,"LMIN":body.light_min,"SMIN":body.soil_min};return await issue(lambda rid:set_command(rid,data))
@app.patch("/api/v1/thresholds")
async def patch_thresholds(body:dict):
    current=store.thresholds()
    if not current:raise HTTPException(409,"no authoritative state")
    try:model=Thresholds(**{**current,**body})
    except Exception as e:raise HTTPException(422,str(e))
    return await put_thresholds(model)
@app.put("/api/v1/controls/{actuator}")
async def put_control(actuator:str,body:ControlRequest):
    actuator=actuator.lower()
    if actuator not in {"pump","fan","lamp","beep"}:raise HTTPException(404,"unknown actuator")
    ttl=body.ttl_seconds
    if body.mode!="auto" and ttl is None:ttl=60 if actuator in {"pump","beep"} else 300
    return await issue(lambda rid:ctrl_command(rid,{actuator:body.mode},ttl))
@app.put("/api/v1/controls")
async def put_controls(body:MultiControlRequest):return await issue(lambda rid:ctrl_command(rid,body.controls,body.ttl_seconds))
@app.post("/api/v1/controls/auto")
async def all_auto():return await issue(lambda rid:ctrl_command(rid,{x:"auto" for x in ("pump","fan","lamp","beep")}))
@app.post("/api/v1/serial/connect")
def connect(body:SerialConnect):manager.connect(body.port,body.baud);return {"ok":True}
@app.delete("/api/v1/serial/connect")
def disconnect():manager.disconnect();return {"ok":True}
@app.websocket("/ws/live")
async def live(ws:WebSocket):
    await ws.accept();q=asyncio.Queue(maxsize=8);store.subscribers.add(q)
    try:
        await ws.send_json({"type":"snapshot","data":store.snapshot()})
        while True:await ws.send_json(await asyncio.wait_for(q.get(),15))
    except asyncio.TimeoutError:
        try:await ws.send_json({"type":"heartbeat"})
        except Exception:pass
    except WebSocketDisconnect:pass
    finally:store.subscribers.discard(q)
