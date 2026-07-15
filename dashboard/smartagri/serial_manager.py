from __future__ import annotations
import asyncio, os, sys, threading, time, random
from pathlib import Path
from .protocol import parse_line, get_command

class SerialManager:
    def __init__(self, store, port=None, baud=9600, demo=False):
        self.store=store;self.port=port;self.baud=baud;self.demo=demo;self.loop=None;self.thread=None;self.stop_event=threading.Event();self.serial=None;self.pending={};self.counter=int(time.time())%900000+100000
    def start(self, loop):
        self.loop=loop;self.stop_event.clear();self.thread=threading.Thread(target=self._demo if self.demo else self._worker,daemon=True);self.thread.start()
    def stop(self):
        self.stop_event.set()
        if self.serial:
            try:self.serial.close()
            except Exception:pass
    def _publish(self,event):
        if self.loop:self.loop.call_soon_threadsafe(asyncio.create_task,self.store.broadcast(event))
    def _handle(self,line):
        try:event=parse_line(line)
        except Exception:return
        if event["type"]=="telemetry":
            self.store.update_telemetry(event["data"],event["raw"]);self._publish({"type":"telemetry","data":self.store.snapshot()})
        elif event["type"] in {"ack","err"}:
            fut=self.pending.pop(event["request_id"],None)
            if fut:self.loop.call_soon_threadsafe(fut.set_result,event)
            self._publish({"type":"command_result","data":event})
    def _load_serial(self):
        try:import serial;return serial
        except ImportError:
            candidates=[]
            if os.getenv("SMARTAGRI_PYSERIAL_PATH"):candidates.append(Path(os.environ["SMARTAGRI_PYSERIAL_PATH"]))
            candidates.append(Path(__file__).resolve().parents[5]/"test1"/"pytools")
            for p in candidates:
                if p.exists():sys.path.insert(0,str(p));import serial;return serial
            raise
    def _worker(self):
        delay=1
        while not self.stop_event.is_set():
            if not self.port:time.sleep(.5);continue
            try:
                serial=self._load_serial();self.serial=serial.Serial(self.port,self.baud,timeout=1);self.store.serial.update(connected=True,port=self.port,error=None);self._publish({"type":"serial_status","data":self.store.serial});delay=1
                self.serial.write(get_command(self._next_id()).encode("ascii"))
                while not self.stop_event.is_set() and self.serial.is_open:
                    raw=self.serial.readline()
                    if raw:self._handle(raw.decode("ascii","replace"))
            except Exception as exc:
                self.store.serial.update(connected=False,error=str(exc),retries=self.store.serial["retries"]+1);self._publish({"type":"serial_status","data":self.store.serial});time.sleep(delay);delay=min(delay*2,10)
    def _demo(self):
        self.store.serial.update(connected=True,port="DEMO",error=None)
        t=0
        while not self.stop_event.is_set():
            t+=1;temp=24+round(4*__import__('math').sin(t/12));hum=58+round(8*__import__('math').sin(t/17));air=380+random.randint(-35,35);light=420+round(230*__import__('math').sin(t/8));soil=430+round(90*__import__('math').sin(t/20));d={"T":temp,"H":hum,"AIR":air,"LIGHT":max(0,light),"SOIL":soil,"TMIN":18,"TMAX":32,"HMIN":40,"HMAX":80,"AIRMAX":600,"LMIN":300,"SMIN":350,"VALID":3,"LED1":0,"LED2":0,"LED3":0,"LED4":0,"PUMP":int(soil<350),"FAN":int(air>600),"LAMP":int(light<300),"BEEP":int(air>600),"SEQ":t,"UPTIME":t*2000,"PMODE":0,"PREM":0,"FMODE":0,"FREM":0,"LAMPMODE":0,"LREM":0,"BMODE":0,"BREM":0};self.store.update_telemetry(d,"DEMO");self._publish({"type":"telemetry","data":self.store.snapshot()});time.sleep(2)
    def _next_id(self):self.counter+=1;return str(self.counter)
    async def command(self,text_factory,timeout=3):
        if self.demo:
            rid=self._next_id();return {"type":"ack","request_id":rid,"parts":["DEMO"]}
        if not self.serial or not self.store.serial["connected"]:raise ConnectionError("serial disconnected")
        rid=self._next_id();future=self.loop.create_future();self.pending[rid]=future;self.serial.write(text_factory(rid).encode("ascii"));return await asyncio.wait_for(future,timeout)
    def connect(self,port,baud=9600):
        if self.serial:
            try:self.serial.close()
            except Exception:pass
        self.port=port;self.baud=baud
    def disconnect(self):
        self.port=None
        if self.serial:
            try:self.serial.close()
            except Exception:pass
        self.store.serial["connected"]=False
    def ports(self):
        try:
            serial=self._load_serial();from serial.tools import list_ports
            return [{"device":p.device,"description":p.description,"hwid":p.hwid} for p in list_ports.comports()]
        except Exception:return []
