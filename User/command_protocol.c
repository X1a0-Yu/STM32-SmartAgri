#include "command_protocol.h"
#include "serial2_printf.h"
#include "app_control.h"
#include "actuators.h"
#include "system_tick.h"
#include "wifi_service.h"
#include "lora_service.h"
#include "buzzer.h"
#include "version.h"
#include <string.h>
#include <stdlib.h>
static char line[320];static uint16_t len;static bool dropping;
static bool number(const char*s,long*v){char*e;*v=strtol(s,&e,10);return *s&&*e=='\0';}
static OutputMode parse_output_mode(const char*s){if(s&&(!strcmp(s,"AUTO")||!strcmp(s,"0")))return OUTPUT_AUTO;if(s&&(!strcmp(s,"OFF")||!strcmp(s,"1")))return OUTPUT_FORCE_OFF;if(s&&(!strcmp(s,"ON")||!strcmp(s,"2")))return OUTPUT_FORCE_ON;return(OutputMode)255;}
static uint8_t buzzer_parse_mode(const char*s,uint8_t max){
  if(!s||!*s)return 255;
  if(!strncmp(s,"MODE,",5)){char*comma=strchr(s+5,',');const char*val=comma?comma+1:s+5;return(val[0]>='0'&&val[0]<='0'+max&&!val[1])?(uint8_t)(val[0]-'0'):255;}
  return(s[0]>='0'&&s[0]<='0'+max&&!s[1])?(uint8_t)(s[0]-'0'):255;
}
static void buzzer_mode_cmd(char*id,char*rest){
  uint8_t mode;
  if(!rest||!*rest){printf_20("ERR,%s,MALFORMED\r\n",id);return;}
  mode=buzzer_parse_mode(rest,4);
  if(mode>4){printf_20("ERR,%s,BAD_VALUE\r\n",id);return;}
  buzzer_set_mode2(mode);printf_20("ACK,%s,BEEP,MODE=%u\r\n",id,mode);
}
static void report_now(void){serial2_report_all(&g_sensor,&g_thresholds,&g_outputs);}
static void set_cmd(char*id,char*rest){ThresholdConfig c=g_thresholds;char*tok;long v;bool any=false;for(tok=strtok(rest,",");tok;tok=strtok(0,",")){char*eq=strchr(tok,'=');if(!eq){printf_20("ERR,%s,MALFORMED\r\n",id);return;}*eq++=0;if(!number(eq,&v)){printf_20("ERR,%s,BAD_VALUE,FIELD=%s\r\n",id,tok);return;}if(!strcmp(tok,"TMIN"))c.temp_min=(int16_t)v;else if(!strcmp(tok,"TMAX"))c.temp_max=(int16_t)v;else if(!strcmp(tok,"HMIN"))c.humi_min=(uint8_t)v;else if(!strcmp(tok,"HMAX"))c.humi_max=(uint8_t)v;else if(!strcmp(tok,"AIRMAX"))c.air_max=(uint16_t)v;else if(!strcmp(tok,"LMIN"))c.light_min=(uint16_t)v;else if(!strcmp(tok,"SMIN"))c.soil_min=(uint16_t)v;else{printf_20("ERR,%s,UNKNOWN_KEY,FIELD=%s\r\n",id,tok);return;}any=true;}if(!any||!app_set_thresholds(&c)){printf_20("ERR,%s,RANGE\r\n",id);return;}actuators_apply(&g_outputs);printf_20("ACK,%s,SET\r\n",id);report_now();}
static void ctrl_cmd(char*id,char*rest){char*tok;char names[4][6];OutputMode modes[4];uint8_t count=0,i;long ttl=0;for(tok=strtok(rest,",");tok;tok=strtok(0,",")){char*eq=strchr(tok,'=');if(!eq){printf_20("ERR,%s,MALFORMED\r\n",id);return;}*eq++=0;if(!strcmp(tok,"TTL")){if(!number(eq,&ttl)){printf_20("ERR,%s,BAD_VALUE,FIELD=TTL\r\n",id);return;}}else{OutputMode m=parse_output_mode(eq);if(m==(OutputMode)255||count>=4){printf_20("ERR,%s,BAD_VALUE,FIELD=%s\r\n",id,tok);return;}strncpy(names[count],tok,5);names[count][5]=0;modes[count++]=m;}}if(!count){printf_20("ERR,%s,MALFORMED\r\n",id);return;}for(i=0;i<count;i++)if(!app_set_override(names[i],modes[i],(uint16_t)ttl,system_millis())){printf_20("ERR,%s,RANGE,FIELD=%s\r\n",id,names[i]);return;}actuators_apply(&g_outputs);printf_20("ACK,%s,CTRL\r\n",id);report_now();}
static void wireless_cmd(char*cmd,char*id,char*rest){long v;if(!rest){printf_20("ERR,%s,MALFORMED\r\n",id);return;}if(!strcmp(cmd,"LORA")){if(!strcmp(rest,"PING")){printf_20(lora_service_test()?"ACK,%s,LORA,PING\r\n":"ERR,%s,LORA,OFF\r\n",id);return;}if(!strncmp(rest,"POWER,ON=",9)&&number(rest+9,&v)){lora_service_set_enabled(v?1:0);printf_20("ACK,%s,LORA,POWER\r\n",id);return;}}else if(!strcmp(cmd,"WIFI")){if(!strncmp(rest,"POWER,ON=",9)&&number(rest+9,&v)){wifi_service_power(v?1:0);printf_20("ACK,%s,WIFI,POWER\r\n",id);return;}}printf_20("ERR,%s,UNKNOWN_CMD\r\n",id);}
static void process(char*l){char*cmd=strtok(l,",");char*id=strtok(0,",");char*rest=strtok(0,"");if(!cmd||!id){printf_20("ERR,0,MALFORMED\r\n");return;}if(!strcmp(cmd,"SET")){if(!rest){printf_20("ERR,%s,MALFORMED\r\n",id);return;}set_cmd(id,rest);}else if(!strcmp(cmd,"CTRL")){if(!rest){printf_20("ERR,%s,MALFORMED\r\n",id);return;}ctrl_cmd(id,rest);}else if(!strcmp(cmd,"GET")&&rest&&!strcmp(rest,"STATE")){printf_20("ACK,%s,GET\r\n",id);report_now();}else if(!strcmp(cmd,"GET")&&rest&&!strcmp(rest,"INFO")){printf_20("ACK,%s,INFO,FW=%u.%u.%u,PROTO=%u,MCU=STM32F103C8\r\n",id,FW_VERSION_MAJOR,FW_VERSION_MINOR,FW_VERSION_PATCH,FW_PROTOCOL_VERSION);}else if(!strcmp(cmd,"LORA")||!strcmp(cmd,"WIFI")){wireless_cmd(cmd,id,rest);}else if(!strcmp(cmd,"BEEP")&&rest){buzzer_mode_cmd(id,rest);}else{printf_20("ERR,%s,UNKNOWN_CMD\r\n",id);}}
void command_protocol_poll(void){uint8_t c,e=serial2_rx_errors();if(e)printf_20("ERR,0,UART_ERROR,CODE=%u\r\n",e);while(serial2_rx_get(&c)){if(c=='\n'){if(!dropping&&len){line[len]=0;if(len&&line[len-1]=='\r')line[--len]=0;process(line);}len=0;dropping=false;}else if(!dropping){if(len<sizeof(line)-1)line[len++]=(char)c;else{dropping=true;printf_20("ERR,0,LINE_TOO_LONG\r\n");}}}}
