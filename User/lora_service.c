#include "lora_service.h"
#include "app_control.h"
#include "system_tick.h"
#include <string.h>
static LoRaConfig cfg;
static uint32_t last_tx;
static uint16_t sequence;
static void put16(uint8_t*p,uint16_t v){p[0]=(uint8_t)(v>>8);p[1]=(uint8_t)v;}
void lora_service_init(void){memset(&cfg,0,sizeof(cfg));cfg.frequency_hz=433000000U;cfg.sf=9;cfg.tx_power=10;cfg.period_s=30;cfg.network_id=1;cfg.node_id=1;sx1278_init();}
void lora_service_set_enabled(bit enabled){cfg.enabled=enabled;if(enabled){sx1278_power(1);delay_ms(20);if(sx1278_detect())sx1278_config(cfg.frequency_hz,cfg.sf,cfg.tx_power);}else sx1278_power(0);}
bit lora_service_configure(const LoRaConfig*c){if(c->frequency_hz<410000000U||c->frequency_hz>525000000U||c->sf<6||c->sf>12||c->tx_power>17||c->period_s<5)return 0;cfg=*c;if(cfg.enabled)return sx1278_config(cfg.frequency_hz,cfg.sf,cfg.tx_power);return 1;}
static uint8_t build_telemetry(uint8_t*b){uint8_t n=0;b[n++]='S';b[n++]='A';b[n++]=2;b[n++]=1;put16(&b[n],cfg.network_id);n+=2;put16(&b[n],cfg.node_id);n+=2;put16(&b[n],sequence++);n+=2;put16(&b[n],(uint16_t)g_sensor.temperature);n+=2;b[n++]=g_sensor.humidity;put16(&b[n],g_sensor.air_quality);n+=2;put16(&b[n],g_sensor.light);n+=2;put16(&b[n],g_sensor.soil);n+=2;b[n++]=g_sensor.valid;b[n++]=(g_outputs.pump?1:0)|(g_outputs.fan?2:0)|(g_outputs.light?4:0)|(g_outputs.buzzer?8:0);return n;}
void lora_service_poll(void){uint8_t buf[64],len;sx1278_service();if(!cfg.enabled)return;len=sx1278_poll_receive(buf,sizeof(buf));if(len>=4&&buf[0]=='S'&&buf[1]=='A'&&buf[3]==5){uint8_t pong[8]={'S','A',2,6,0,0,0,0};sx1278_send(pong,8);}if((uint32_t)(system_millis()-last_tx)>=(uint32_t)cfg.period_s*1000U){last_tx=system_millis();len=build_telemetry(buf);sx1278_send(buf,len);}}
bit lora_service_test(void){uint8_t p[8]={'S','A',2,5,0,0,0,0};return cfg.enabled?sx1278_send(p,8):0;}
const LoRaConfig*lora_service_config(void){return &cfg;}
