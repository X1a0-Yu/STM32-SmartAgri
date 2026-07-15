#include "wifi_service.h"
#include "esp_uart.h"
#include "app_control.h"
#include "system_tick.h"
#include "board.h"
#include <string.h>
#include <stdio.h>
#define ESP_POWER_PORT GPIOB
#define ESP_POWER_PIN 15U
static WiFiConfig cfg;
static WiFiStatus stat;
static uint32_t deadline,last_upload,backoff_until;
static char response[256];static uint16_t response_len;
static char request[420];static uint16_t request_len;static uint8_t send_phase;
static bit has(const char*s){return strstr(response,s)!=0;}
static void clear_resp(void){response_len=0;response[0]=0;}
static void send_cmd(const char*s,uint32_t timeout){clear_resp();esp_uart_write(s);esp_uart_write("\r\n");deadline=system_millis()+timeout;}
static void power(bit on){gpio_write(ESP_POWER_PORT,ESP_POWER_PIN,on?0:1);stat.state=on?WIFI_BOOTING:WIFI_OFF;stat.connected=0;deadline=system_millis()+2000U;}
void wifi_service_init(void){memset(&cfg,0,sizeof(cfg));memset(&stat,0,sizeof(stat));cfg.port=5000;cfg.period_s=30;strcpy(cfg.path,"/api/v1/ingest/telemetry");RCC->APB2ENR|=RCC_APB2ENR_IOPBEN;gpio_config_output(ESP_POWER_PORT,ESP_POWER_PIN,0x2U);gpio_write(ESP_POWER_PORT,ESP_POWER_PIN,1);esp_uart_init(115200U);}
void wifi_service_power(bit on){cfg.enabled=on;power(on);}
bit wifi_service_configure(const WiFiConfig*c){if(strlen(c->ssid)>32||strlen(c->password)>64||strlen(c->host)>39||strlen(c->path)>47||c->port==0||c->period_s<10)return 0;cfg=*c;if(cfg.enabled)power(1);return 1;}
static void collect(void){uint8_t c;while(esp_uart_get(&c)){if(response_len<sizeof(response)-1){response[response_len++]=(char)c;response[response_len]=0;}}}
static void http_send(void){char body[240],cmd[96];int bl,rl;bl=snprintf(body,sizeof(body),"{\"node\":1,\"fw\":\"1.1.0\",\"t\":%d,\"h\":%u,\"air\":%u,\"light\":%u,\"soil\":%u,\"valid\":%u,\"pump\":%u,\"fan\":%u,\"lamp\":%u}",g_sensor.temperature,g_sensor.humidity,g_sensor.air_quality,g_sensor.light,g_sensor.soil,g_sensor.valid,g_outputs.pump,g_outputs.fan,g_outputs.light);rl=snprintf(request,sizeof(request),"POST %s HTTP/1.1\r\nHost: %s:%u\r\nContent-Type: application/json\r\nX-SmartAgri-Token: %s\r\nContent-Length: %d\r\nConnection: close\r\n\r\n%s",cfg.path,cfg.host,cfg.port,cfg.token,bl,body);if(rl<0)rl=0;if(rl>(int)sizeof(request)-1)rl=sizeof(request)-1;request_len=(uint16_t)rl;snprintf(cmd,sizeof(cmd),"AT+CIPSTART=\"TCP\",\"%s\",%u",cfg.host,cfg.port);send_cmd(cmd,5000U);stat.state=WIFI_SENDING;send_phase=0;deadline=system_millis()+5000U;}
void wifi_service_poll(void){uint32_t now=system_millis();collect();if(!cfg.enabled)return;switch(stat.state){case WIFI_BOOTING:if((int32_t)(now-deadline)>=0){send_cmd("AT",1000);stat.state=WIFI_AT_SYNC;}break;case WIFI_AT_SYNC:if(has("OK")){stat.at_present=1;send_cmd("ATE0",1000);if(cfg.ssid[0]){char cmd[120];snprintf(cmd,sizeof(cmd),"AT+CWJAP=\"%s\",\"%s\"",cfg.ssid,cfg.password);send_cmd(cmd,15000);stat.state=WIFI_JOINING;}else{stat.state=WIFI_READY;stat.connected=1;}}else if((int32_t)(now-deadline)>=0){stat.error=1;stat.state=WIFI_NO_AT;}break;case WIFI_JOINING:if(has("WIFI GOT IP")||has("OK")){stat.connected=1;stat.state=WIFI_READY;last_upload=now;}else if(has("FAIL")||(int32_t)(now-deadline)>=0){stat.error=2;stat.upload_fail++;backoff_until=now+10000;stat.state=WIFI_BACKOFF;}break;case WIFI_READY:if(stat.connected&&cfg.host[0]&&(uint32_t)(now-last_upload)>=(uint32_t)cfg.period_s*1000U){last_upload=now;http_send();}break;case WIFI_SENDING:if(send_phase==0&&(has("CONNECT")||has("ALREADY CONNECTED"))){char cmd[32];snprintf(cmd,sizeof(cmd),"AT+CIPSEND=%u",request_len);send_cmd(cmd,2000);send_phase=1;deadline=now+2000;}else if(send_phase==1&&strchr(response,'>')){clear_resp();esp_uart_write(request);send_phase=2;deadline=now+8000;}if(send_phase==2&&has("SEND OK")){stat.upload_ok++;stat.state=WIFI_READY;}else if(has("ERROR")||has("FAIL")||(int32_t)(now-deadline)>=0){stat.upload_fail++;stat.error=3;backoff_until=now+10000;stat.state=WIFI_BACKOFF;}break;case WIFI_BACKOFF:if((int32_t)(now-backoff_until)>=0)power(1);break;default:break;}}
const WiFiConfig*wifi_service_config(void){return &cfg;}
const WiFiStatus*wifi_service_status(void){return &stat;}
