from pydantic import BaseModel, Field, model_validator
from typing import Literal
class Thresholds(BaseModel):
    temp_min:int=Field(-20,ge=-20,le=80);temp_max:int=Field(32,ge=-20,le=80);humi_min:int=Field(40,ge=0,le=100);humi_max:int=Field(80,ge=0,le=100);air_max:int=Field(600,ge=0,le=1023);light_min:int=Field(300,ge=0,le=1023);soil_min:int=Field(350,ge=0,le=1023)
    @model_validator(mode="after")
    def pairs(self):
        if self.temp_min>self.temp_max or self.humi_min>self.humi_max:raise ValueError("最低阈值不能高于最高阈值")
        return self
class ControlRequest(BaseModel): mode:Literal["auto","on","off"];ttl_seconds:int|None=Field(None,ge=1,le=1800)
class MultiControlRequest(BaseModel): controls:dict[str,Literal["auto","on","off"]];ttl_seconds:int|None=Field(None,ge=1,le=1800)
class SerialConnect(BaseModel): port:str;baud:int=Field(9600,ge=1200,le=921600)
