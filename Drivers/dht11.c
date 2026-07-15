#include "dht11.h"
#include "board.h"
#include "system_tick.h"
static void dht_output(void){gpio_config_output(DHT_PORT,DHT_PIN,0x6U);}
static void dht_input(void){gpio_config_input(DHT_PORT,DHT_PIN,0x4U);}
static bool wait_level(bool level,uint16_t timeout){uint16_t start=micros16();while(gpio_read(DHT_PORT,DHT_PIN)!=level){if((uint16_t)(micros16()-start)>timeout)return false;}return true;}
static bool read_byte(uint8_t*out){uint8_t i,v=0;for(i=0;i<8;i++){uint16_t start;if(!wait_level(true,100))return false;start=micros16();if(!wait_level(false,100))return false;v=(uint8_t)(v<<1);if((uint16_t)(micros16()-start)>45U)v|=1U;}*out=v;return true;}
void dht11_init(void){dht_output();gpio_write(DHT_PORT,DHT_PIN,true);}
bool dht11_read(int16_t*t,uint8_t*h){uint8_t b[5],i;dht_output();gpio_write(DHT_PORT,DHT_PIN,false);delay_ms(20);gpio_write(DHT_PORT,DHT_PIN,true);delay_us(30);dht_input();if(!wait_level(false,100)||!wait_level(true,100)||!wait_level(false,100))return false;for(i=0;i<5;i++)if(!read_byte(&b[i]))return false;if((uint8_t)(b[0]+b[1]+b[2]+b[3])!=b[4])return false;*h=b[0];*t=b[2];return true;}
