#include "board.h"
#include "system_tick.h"
#include "app_control.h"
#include "command_protocol.h"
#include "actuators.h"
#include "buzzer.h"
#include "adc.h"
#include "dht11.h"
#include "key.h"
#include "oled.h"
#include "serial2_printf.h"
#include "lora_service.h"
#include "wifi_service.h"
#include "version.h"
int main(void){uint32_t tk=0,ta=0,td=0,to=0,ts=0,tv=0;int16_t temp;uint8_t hum;SystemInit();board_init();buzzer_init();system_tick_init();app_init();actuators_init();adc_init();dht11_init();key_init();serial2_init();oled_init();lora_service_init();wifi_service_init();printf_20("%s @ 0x08000000\r\n",FW_STRING);while(1){
 command_protocol_poll();
 lora_service_poll();
 wifi_service_poll();
 if(system_due(&tv,100U)){app_service_overrides(system_millis());actuators_apply(&g_outputs);}
 if(system_due(&tk,KEY_PERIOD_MS)){KeyEvent e=key_scan();if(e==KEY_NEXT)app_next_field();else if(e==KEY_ENTER_EXIT)app_next_page();else if(e==KEY_INC)app_adjust(1);else if(e==KEY_DEC)app_adjust(-1);actuators_apply(&g_outputs);}
 if(system_due(&ta,ADC_PERIOD_MS)){adc_read_environment(&g_sensor.air_quality,&g_sensor.light,&g_sensor.soil);g_sensor.valid|=2U;app_evaluate();actuators_apply(&g_outputs);}
 if(system_due(&td,DHT_PERIOD_MS)){if(dht11_read(&temp,&hum)){g_sensor.temperature=temp;g_sensor.humidity=hum;g_sensor.valid|=1U;}app_evaluate();actuators_apply(&g_outputs);}
 if(system_due(&to,DISPLAY_PERIOD_MS))oled_update(&g_sensor,&g_thresholds,g_ui_page);
 if(system_due(&ts,SERIAL_PERIOD_MS))serial2_report_all(&g_sensor,&g_thresholds,&g_outputs);
 }}
