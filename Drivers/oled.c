#include "oled.h"
#include "board_config.h"
#include "system_tick.h"
#include <stdio.h>
#include <string.h>
static bool present;
static uint8_t fb[1024];
static uint8_t shadow[1024];
static UI_PAGE last_page;
static SensorData last_s;
static ThresholdConfig last_t;
static bit cache_ok;
static const uint8_t ascii[][5]={{0,0,0,0,0},{0,0,0x5F,0,0},{0x7E,0x11,0x11,0x11,0x7E},{0x7F,0x49,0x49,0x49,0x36},{0x3E,0x41,0x41,0x41,0x22},{0x7F,0x41,0x41,0x22,0x1C},{0x7F,0x49,0x49,0x49,0x41},{0x7F,0x09,0x09,0x09,0x01},{0x3E,0x41,0x49,0x49,0x7A},{0x7F,0x08,0x08,0x08,0x7F},{0x41,0x41,0x7F,0x41,0x41},{0x20,0x40,0x41,0x3F,0x01},{0x7F,0x08,0x14,0x22,0x41},{0x7F,0x40,0x40,0x40,0x40},{0x7F,0x02,0x0C,0x02,0x7F},{0x7F,0x04,0x08,0x10,0x7F},{0x3E,0x41,0x41,0x41,0x3E},{0x7F,0x09,0x09,0x09,0x06},{0x3E,0x41,0x51,0x21,0x5E},{0x7F,0x09,0x19,0x29,0x46},{0x46,0x49,0x49,0x49,0x31},{0x01,0x01,0x7F,0x01,0x01},{0x3F,0x40,0x40,0x40,0x3F},{0x1F,0x20,0x40,0x20,0x1F},{0x7F,0x20,0x18,0x20,0x7F},{0x63,0x14,0x08,0x14,0x63},{0x03,0x04,0x78,0x04,0x03},{0x61,0x51,0x49,0x45,0x43}};
static bool wait_flag(volatile uint16_t*r,uint16_t mask){uint32_t n=200000;while(!(*r&mask)&&--n){}return n!=0;}
static bool tx(uint8_t control,const uint8_t*d,uint16_t n){uint16_t i;if(!present&&control)return false;I2C2->CR1|=I2C_CR1_START;if(!wait_flag(&I2C2->SR1,I2C_SR1_SB))return false;I2C2->DR=(OLED_ADDRESS<<1);if(!wait_flag(&I2C2->SR1,I2C_SR1_ADDR))return false;(void)I2C2->SR2;I2C2->DR=control;for(i=0;i<n;i++){if(!wait_flag(&I2C2->SR1,I2C_SR1_TXE))return false;I2C2->DR=d[i];}if(!wait_flag(&I2C2->SR1,I2C_SR1_BTF))return false;I2C2->CR1|=I2C_CR1_STOP;return true;}
static void cmd(uint8_t c){tx(0,&c,1);}static void pos(uint8_t p,uint8_t x){cmd(0xB0|p);cmd((x&0xF));cmd(0x10|(x>>4));}
static void char_at(uint8_t p,uint8_t x,const char*s){uint8_t i,b[6];while(*s&&x<122){const uint8_t*f=ascii[0];char c=*s++;if(c>='A'&&c<='Z')f=ascii[(uint8_t)(c-'A'+2)];else if(c>='0'&&c<='9'){static const uint8_t num[][5]={{0x3E,0x51,0x49,0x45,0x3E},{0,0x42,0x7F,0x40,0},{0x42,0x61,0x51,0x49,0x46},{0x21,0x41,0x45,0x4B,0x31},{0x18,0x14,0x12,0x7F,0x10},{0x27,0x45,0x45,0x45,0x39},{0x3C,0x4A,0x49,0x49,0x30},{1,0x71,0x9,5,3},{0x36,0x49,0x49,0x49,0x36},{0x6,0x49,0x49,0x29,0x1E}};f=num[(unsigned)(c-'0')];}for(i=0;i<5;i++)b[i]=f[i];b[5]=0;memcpy(fb+p*128+x,b,6);x+=6;}}
static void render(const SensorData*s,const ThresholdConfig*t,UI_PAGE p){
 char buf[24];
 memset(fb,0,1020);
 if(p==UI_VALUES){
  sprintf(buf,"T%d H%u",(int)s->temperature,s->humidity);char_at(0,0,buf);
  sprintf(buf,"AIR%u",s->air_quality);char_at(2,0,buf);
  sprintf(buf,"L%u SOIL%u",s->light,s->soil);char_at(4,0,buf);
  sprintf(buf,"V%d %s",s->valid,s->valid==3?"OK":"--");char_at(6,0,buf);
 }else{
  sprintf(buf,"T:%d-%d",(int)t->temp_min,(int)t->temp_max);char_at(0,0,buf);
  sprintf(buf,"H:%u-%u",t->humi_min,t->humi_max);char_at(2,0,buf);
  sprintf(buf,"A%u L%u",t->air_max,t->light_min);char_at(4,0,buf);
  sprintf(buf,"S%u",t->soil_min);char_at(6,0,buf);
 }
}
static bit cache_equal(const SensorData*s,const ThresholdConfig*t,UI_PAGE p){
 if(!cache_ok)return 0;
 return p==last_page && s->temperature==last_s.temperature && s->humidity==last_s.humidity &&
  s->air_quality==last_s.air_quality && s->light==last_s.light && s->soil==last_s.soil && s->valid==last_s.valid &&
  t->temp_min==last_t.temp_min && t->temp_max==last_t.temp_max && t->humi_min==last_t.humi_min &&
  t->humi_max==last_t.humi_max && t->air_max==last_t.air_max && t->light_min==last_t.light_min &&
  t->soil_min==last_t.soil_min;
}
static void remember(const SensorData*s,const ThresholdConfig*t,UI_PAGE p){last_page=p;last_s=*s;last_t=*t;cache_ok=1;}
void oled_init(void){static const uint8_t init[]={0xAE,0x20,0x02,0xA1,0xC8,0x81,0x7F,0xA6,0xA8,0x3F,0xD3,0,0xD5,0x80,0xD9,0xF1,0xDA,0x12,0xDB,0x40,0x8D,0x14,0xAF};uint8_t i;RCC->APB2ENR|=RCC_APB2ENR_IOPBEN;RCC->APB1ENR|=RCC_APB1ENR_I2C2EN;GPIOB->CRH=(GPIOB->CRH&~(0xFF00UL))|(0xFF00UL);I2C2->CR2=36;I2C2->CCR=180;I2C2->TRISE=37;I2C2->CR1=I2C_CR1_PE;memset(fb,0,sizeof(fb));memset(shadow,0xFF,sizeof(shadow));present=true;cache_ok=0;for(i=0;i<sizeof(init);i++)cmd(init[i]);}
void oled_update(const SensorData*s,const ThresholdConfig*t,UI_PAGE p){
 uint8_t page;
 if(cache_equal(s,t,p))return;
 render(s,t,p);
 for(page=0;page<8;page++){
  if(fb[page*128]==shadow[page*128] && !memcmp(&fb[page*128],&shadow[page*128],128))continue;
  pos(page,0);tx(0x40,&fb[page*128],128);
  memcpy(&shadow[page*128],&fb[page*128],128);
 }
 remember(s,t,p);
}
