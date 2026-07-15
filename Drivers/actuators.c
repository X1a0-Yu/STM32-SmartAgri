#include "actuators.h"
#include "board.h"
#include "buzzer.h"
void actuators_init(void){OutputState off={0};buzzer_set(0);actuators_apply(&off);}
void actuators_apply(const OutputState*s){
 static bit last_buzzer=0;
 gpio_write(LED1_PORT,LED1_PIN,!s->led1);gpio_write(LED2_PORT,LED2_PIN,!s->led2);gpio_write(LED3_PORT,LED3_PIN,!s->led3);gpio_write(LED4_PORT,LED4_PIN,!s->led4);
 gpio_write(PUMP_PORT,PUMP_PIN,s->pump);gpio_write(FAN_PORT,FAN_PIN,s->fan);gpio_write(LIGHT_PORT,LIGHT_PIN,s->light);
 if(s->buzzer!=last_buzzer){buzzer_set(s->buzzer);last_buzzer=s->buzzer;}
}
