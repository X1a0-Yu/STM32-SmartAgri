#include "serial2_printf.h"
#include "board_config.h"
#include "system_tick.h"
#include "wifi_service.h"
#include "lora_service.h"
#include "version.h"
#include <stdarg.h>
#include <stdio.h>
static volatile uint8_t rx[256],head,tail,errors;static uint32_t seq;
static void app_uart_init_gpio(void){RCC->APB2ENR|=RCC_APB2ENR_IOPAEN|APP_UART_APB_MASK;APP_UART_PORT->CRH=(APP_UART_PORT->CRH&~(0xFFUL<<((APP_UART_TX_PIN-8U)*4U)))|(0xBUL<<((APP_UART_TX_PIN-8U)*4U))|(0x4UL<<((APP_UART_RX_PIN-8U)*4U));}
void serial2_init(void){
 uint32_t fclk=SystemCoreClock;
 uint32_t usartdiv=fclk/(APP_UART_BAUD*16U);
 uint32_t frac=(fclk%(APP_UART_BAUD*16U)*16U)/(APP_UART_BAUD*16U);
 app_uart_init_gpio();
 APP_UART_PERIPH->BRR=(uint16_t)((usartdiv<<4)|frac);
 APP_UART_PERIPH->CR1=USART_CR1_TE|USART_CR1_RE|USART_CR1_RXNEIE|USART_CR1_UE;
 NVIC_SetPriority(APP_UART_IRQn,2);NVIC_EnableIRQ(APP_UART_IRQn);}
void USART1_IRQHandler(void){uint16_t sr=APP_UART_PERIPH->SR;uint8_t v=(uint8_t)APP_UART_PERIPH->DR;if(sr&(USART_SR_ORE|USART_SR_FE|USART_SR_NE|USART_SR_PE)){errors|=1U;return;}if(sr&USART_SR_RXNE){uint8_t n=(uint8_t)(head+1U);if(n==tail)errors|=2U;else{rx[head]=v;head=n;}}}
bool serial2_rx_get(uint8_t*v){if(head==tail)return false;*v=rx[tail];tail=(uint8_t)(tail+1U);return true;}uint8_t serial2_rx_errors(void){uint8_t e=errors;errors=0;return e;}
void serial2_putc(char c){while(!(APP_UART_PERIPH->SR&USART_SR_TXE)){}APP_UART_PERIPH->DR=(uint8_t)c;}
void printf_20(const char*f,...){char b[320];int n,i;va_list a;va_start(a,f);n=vsnprintf(b,sizeof(b),f,a);va_end(a);if(n<0)return;if(n>(int)sizeof(b)-1)n=sizeof(b)-1;for(i=0;i<n;i++)serial2_putc(b[i]);}
void serial2_report_all(const SensorData*s,const ThresholdConfig*t,const OutputState*o){
 uint32_t now=system_millis();const WiFiStatus*w=wifi_service_status();const LoRaStatus*l=sx1278_status();
 printf_20("DATA,T=%d,H=%u,AIR=%u,LIGHT=%u,SOIL=%u,TMIN=%d,TMAX=%d,HMIN=%u,HMAX=%u,",s->temperature,s->humidity,s->air_quality,s->light,s->soil,t->temp_min,t->temp_max,t->humi_min,t->humi_max);
 printf_20("AIRMAX=%u,LMIN=%u,SMIN=%u,VALID=%u,LED1=%u,LED2=%u,LED3=%u,LED4=%u,",t->air_max,t->light_min,t->soil_min,s->valid,o->led1,o->led2,o->led3,o->led4);
 printf_20("PUMP=%u,FAN=%u,LAMP=%u,BEEP=%u,SEQ=%lu,UPTIME=%lu,PMODE=%u,PREM=%u,FMODE=%u,FREM=%u,",o->pump,o->fan,o->light,o->buzzer,seq++,now,g_overrides.pump,app_override_remaining(g_overrides.pump,g_overrides.pump_until,now),g_overrides.fan,app_override_remaining(g_overrides.fan,g_overrides.fan_until,now));
 printf_20("LAMPMODE=%u,LREM=%u,BMODE=%u,BREM=%u,FWMAJ=%u,FWMIN=%u,FWPATCH=%u,PROTO=%u,",g_overrides.light,app_override_remaining(g_overrides.light,g_overrides.light_until,now),g_overrides.buzzer,app_override_remaining(g_overrides.buzzer,g_overrides.buzzer_until,now),FW_VERSION_MAJOR,FW_VERSION_MINOR,FW_VERSION_PATCH,FW_PROTOCOL_VERSION);
 printf_20("WSTATE=%u,WRSSI=%d,WERR=%u,WTXOK=%lu,WTXFAIL=%lu,LSTATE=%u,LFREQ=%lu,LRSSI=%d,LSNR10=%d,LERR=%u,LTX=%lu,LRX=%lu\r\n",w->state,w->rssi,w->error,w->upload_ok,w->upload_fail,l->state,l->frequency_hz,l->rssi,l->snr10,l->error,l->tx_count,l->rx_count);
}
