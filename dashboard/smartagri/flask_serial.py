from __future__ import annotations
import os,sys,threading,time,random,math
from pathlib import Path
from .protocol import parse_line,get_command
class FlaskSerialManager:
 def __init__(self,store,port=None,baud=19200,demo=False):
  self.store=store;self.port=port;self.baud=baud;self.demo=demo;self.stop_event=threading.Event();self.thread=None;self.serial=None;self.lock=threading.Lock();self.pending={};self.counter=int(time.time())%900000+100000
 def _load_serial(self):
  try:import serial;return serial
  except ImportError:
   p=Path(__file__).resolve().parents[5]/"test1"/"pytools"
   if p.exists():sys.path.insert(0,str(p));import serial;return serial
   raise
 def start(self):self.thread=threading.Thread(target=self._demo if self.demo else self._worker,daemon=True);self.thread.start()
 def stop(self):self.stop_event.set();self.disconnect()
 def _next(self):self.counter+=1;return str(self.counter)
 def _handle(self,line):
  try:event=parse_line(line)
  except Exception:return
  if event["type"]=="telemetry":self.store.update_telemetry(event["data"],event["raw"])
  elif event["type"] in {"ack","err"}:
   p=self.pending.get(event["request_id"])
   if p:p["result"]=event;p["event"].set()
 def _worker(self):
  delay=1
  while not self.stop_event.is_set():
   if not self.port:time.sleep(.25);continue
   try:
    serial=self._load_serial();self.serial=serial.Serial(self.port,self.baud,timeout=1);self.store.serial.update(connected=True,port=self.port,baud=self.baud,error=None);delay=1
    with self.lock:self.serial.write(get_command(self._next()).encode("ascii"))
    while not self.stop_event.is_set() and self.serial.is_open:
     raw=self.serial.readline()
     if raw:self._handle(raw.decode("ascii","replace"))
   except Exception as exc:self.store.serial.update(connected=False,error=str(exc),retries=self.store.serial["retries"]+1);time.sleep(delay);delay=min(delay*2,10)
 def _demo(self):
  self.store.serial.update(connected=True,port="DEMO",baud=19200,error=None);n=0
  while not self.stop_event.is_set():
   n+=1;t=25+round(3*math.sin(n/15));h=58+round(8*math.sin(n/20));air=350+random.randint(-30,30);light=500+round(280*math.sin(n/11));soil=430+round(90*math.sin(n/22));d={"T":t,"H":h,"AIR":air,"LIGHT":max(0,light),"SOIL":soil,"TMIN":18,"TMAX":32,"HMIN":40,"HMAX":80,"AIRMAX":600,"LMIN":300,"SMIN":350,"VALID":3,"LED1":0,"LED2":0,"LED3":0,"LED4":0,"PUMP":int(soil<350),"FAN":int(air>600),"LAMP":int(light<300),"BEEP":0,"SEQ":n,"UPTIME":n*2000,"PMODE":0,"PREM":0,"FMODE":0,"FREM":0,"LAMPMODE":0,"LREM":0,"BMODE":0,"BREM":0,"FWMAJ":1,"FWMIN":1,"FWPATCH":0,"PROTO":2,"WSTATE":4,"WRSSI":-55,"WERR":0,"WTXOK":n//15,"WTXFAIL":0,"LSTATE":2,"LFREQ":433000000,"LRSSI":-78,"LSNR10":85,"LERR":0,"LTX":n//15,"LRX":n//30};self.store.update_telemetry(d,"DEMO V1.1");time.sleep(2)
 def command(self,factory,timeout=4):
  rid=self._next()
  if self.demo:return {"type":"ack","request_id":rid,"parts":["DEMO"]}
  if not self.serial or not self.store.serial["connected"]:raise ConnectionError("serial disconnected")
  p={"event":threading.Event(),"result":None};self.pending[rid]=p
  try:
   with self.lock:self.serial.write(factory(rid).encode("ascii"))
   if not p["event"].wait(timeout):raise TimeoutError()
   return p["result"]
  finally:self.pending.pop(rid,None)
 def connect(self,port,baud=19200):self.disconnect();self.demo=False;self.port=port;self.baud=baud
 def disconnect(self):
  if self.serial:
   try:self.serial.close()
   except Exception:pass
  self.serial=None;self.store.serial["connected"]=False
 def ports(self):
  try:self._load_serial();from serial.tools import list_ports;return [{"device":p.device,"description":p.description,"hwid":p.hwid} for p in list_ports.comports()]
  except Exception:return []
