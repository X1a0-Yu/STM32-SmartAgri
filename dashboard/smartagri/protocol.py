"""Wire protocol parsing and command construction."""
from __future__ import annotations
from dataclasses import dataclass

REQUIRED = {"T","H","AIR","LIGHT","SOIL","TMIN","TMAX","HMIN","HMAX","AIRMAX","LMIN","SMIN","VALID","PUMP","FAN","LAMP","BEEP"}

def parse_line(line: str) -> dict:
    line = line.strip()
    parts = line.split(",")
    kind = parts[0] if parts else ""
    if kind == "DATA":
        values = {}
        for token in parts[1:]:
            if "=" not in token: raise ValueError("malformed telemetry token")
            key, value = token.split("=", 1)
            values[key] = int(value)
        missing = REQUIRED - values.keys()
        if missing: raise ValueError(f"missing fields: {sorted(missing)}")
        return {"type":"telemetry","data":values,"raw":line}
    if kind in {"ACK","ERR"} and len(parts) >= 3:
        return {"type":kind.lower(),"request_id":parts[1],"parts":parts[2:],"raw":line}
    return {"type":"info","raw":line}

def set_command(request_id: str, data: dict) -> str:
    order=("TMIN","TMAX","HMIN","HMAX","AIRMAX","LMIN","SMIN")
    return f"SET,{request_id}," + ",".join(f"{k}={int(data[k])}" for k in order) + "\r\n"

def ctrl_command(request_id: str, controls: dict, ttl: int | None = None) -> str:
    fields=[f"{k.upper()}={v.upper()}" for k,v in controls.items()]
    if ttl is not None: fields.append(f"TTL={int(ttl)}")
    return f"CTRL,{request_id}," + ",".join(fields) + "\r\n"

def get_command(request_id: str) -> str:
    return f"GET,{request_id},STATE\r\n"
