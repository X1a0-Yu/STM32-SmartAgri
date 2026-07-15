from __future__ import annotations
from collections import deque
from datetime import datetime, timezone
import queue, threading

def utcnow():return datetime.now(timezone.utc).isoformat()
class StateStore:
 def __init__(self):
  self.telemetry=None;self.remote=None;self.raw="";self.received_at=None;self.history=deque(maxlen=300);self.serial={"connected":False,"port":None,"error":None,"retries":0,"baud":19200};self.subscribers=set();self.lock=threading.RLock();self.revision=0
 def _publish(self):
  snap=self.snapshot();dead=[]
  for q in list(self.subscribers):
   try:
    if q.full():q.get_nowait()
    q.put_nowait(snap)
   except Exception:dead.append(q)
  for q in dead:self.subscribers.discard(q)
 def update_telemetry(self,data,raw=""):
  with self.lock:
   self.telemetry=dict(data);self.raw=raw;self.received_at=utcnow();self.history.append({"received_at":self.received_at,**self.telemetry});self.revision+=1
  self._publish()
 def update_remote(self,data):
  with self.lock:self.remote={"received_at":utcnow(),"data":dict(data)};self.revision+=1
  self._publish()
 def subscribe(self):q=queue.Queue(maxsize=2);self.subscribers.add(q);return q
 def unsubscribe(self,q):self.subscribers.discard(q)
 def snapshot(self):
  with self.lock:return {"revision":self.revision,"telemetry":self.telemetry,"thresholds":self.thresholds(),"controls":self.controls(),"warnings":self.warnings(),"serial":dict(self.serial),"wireless":self.wireless(),"received_at":self.received_at,"raw":self.raw,"remote":self.remote}
 def thresholds(self):
  t=self.telemetry
  required=("TMIN","TMAX","HMIN","HMAX","AIRMAX","LMIN","SMIN")
  if not t or any(k not in t for k in required):return None
  return {"temp_min":t["TMIN"],"temp_max":t["TMAX"],"humi_min":t["HMIN"],"humi_max":t["HMAX"],"air_max":t["AIRMAX"],"light_min":t["LMIN"],"soil_min":t["SMIN"]}
 def controls(self):
  t=self.telemetry
  if not t or any(k not in t for k in ("PUMP","FAN","LAMP","BEEP")):return None
  out={}
  for name,prefix,field in (("pump","P","PUMP"),("fan","F","FAN"),("lamp","LAMP","LAMP"),("beep","B","BEEP")):out[name]={"active":bool(t[field]),"mode":{0:"auto",1:"off",2:"on"}.get(t.get(prefix+"MODE",0),"auto"),"remaining":t.get(prefix+"REM",0)}
  return out
 def wireless(self):
  t=self.telemetry or {}
  return {"wifi":{"state":t.get("WSTATE",0),"rssi":t.get("WRSSI",0),"error":t.get("WERR",0),"upload_ok":t.get("WTXOK",0),"upload_fail":t.get("WTXFAIL",0)},"lora":{"state":t.get("LSTATE",0),"frequency":t.get("LFREQ",433000000),"rssi":t.get("LRSSI",0),"snr10":t.get("LSNR10",0),"error":t.get("LERR",0),"tx":t.get("LTX",0),"rx":t.get("LRX",0)}}
 def warnings(self):
  if not self.serial["connected"]:return [{"code":"serial_disconnected","severity":"critical","label":"设备串口未连接"}]
  t=self.telemetry
  required=("T","TMIN","TMAX","H","HMIN","HMAX","AIR","AIRMAX","LIGHT","LMIN","SOIL","SMIN")
  if not t or any(k not in t for k in required):return [{"code":"no_data","severity":"warning","label":"等待完整农场数据"}]
  w=[];checks=((t["T"]>t["TMAX"],"temp_high","温度超过上限",t["T"],t["TMAX"]),(t["T"]<t["TMIN"],"temp_low","温度低于下限",t["T"],t["TMIN"]),(t["H"]>t["HMAX"],"humi_high","湿度超过上限",t["H"],t["HMAX"]),(t["H"]<t["HMIN"],"humi_low","湿度低于下限",t["H"],t["HMIN"]),(t["AIR"]>t["AIRMAX"],"air_high","空气质量异常",t["AIR"],t["AIRMAX"]),(t["LIGHT"]<t["LMIN"],"light_low","光照不足",t["LIGHT"],t["LMIN"]),(t["SOIL"]<t["SMIN"],"soil_low","土壤湿度不足",t["SOIL"],t["SMIN"]))
  for active,code,label,val,limit in checks:
   if active:w.append({"code":code,"severity":"warning","label":label,"value":val,"threshold":limit})
  if t.get("VALID",0)!=3:w.append({"code":"sensor_invalid","severity":"critical","label":"部分传感器数据无效"})
  return w
