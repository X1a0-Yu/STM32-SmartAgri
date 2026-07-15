#include "buzzer.h"
#include "board_config.h"
#include "board.h"
#include "system_tick.h"
static bit g_buzzer_on;
static uint8_t g_mode; /* 0=off 1=toggle-PWM 2=active-HIGH 3=slow-toggle 4=toggle-inverse */
static uint32_t g_test_count;
void buzzer_init(void){
 RCC->APB1ENR|=RCC_APB1ENR_TIM3EN;
 TIM3->CR1=0;TIM3->DIER=0;TIM3->SR=0;
 g_buzzer_on=0;g_mode=0;g_test_count=0;
 gpio_write(BUZZER_PORT,BUZZER_PIN,0);
 gpio_config_output(BUZZER_PORT,BUZZER_PIN,0x2U);
}
void buzzer_set(bit on){
 buzzer_set_mode2(on?1:0);
}
void buzzer_set_mode2(uint8_t mode){
 if(mode==g_mode && (mode==0||mode==2))return;
 g_mode=mode;g_buzzer_on=(mode!=0);
 if(mode==0){TIM3->CR1=0;TIM3->DIER=0;TIM3->SR=0;gpio_write(BUZZER_PORT,BUZZER_PIN,0);}
 else if(mode==1){TIM3->PSC=(SystemCoreClock/1000000U)-1U;TIM3->ARR=(1000000U/350U/2U)-1U;TIM3->EGR=TIM_EGR_UG;TIM3->SR=0;TIM3->DIER=TIM_DIER_UIE;NVIC_EnableIRQ(TIM3_IRQn);TIM3->CR1=TIM_CR1_CEN;TIM3->CR1|=TIM_CR1_ARPE;}
 else if(mode==2){TIM3->CR1=0;TIM3->DIER=0;TIM3->SR=0;gpio_write(BUZZER_PORT,BUZZER_PIN,1);}
 else if(mode==3){TIM3->PSC=(SystemCoreClock/1000000U)-1U;TIM3->ARR=4999U;TIM3->EGR=TIM_EGR_UG;TIM3->SR=0;TIM3->DIER=TIM_DIER_UIE;NVIC_EnableIRQ(TIM3_IRQn);TIM3->CR1=TIM_CR1_CEN;}
 else if(mode==4){TIM3->PSC=(SystemCoreClock/1000000U)-1U;TIM3->ARR=(1000000U/350U/2U)-1U;TIM3->EGR=TIM_EGR_UG;TIM3->SR=0;TIM3->DIER=TIM_DIER_UIE;NVIC_EnableIRQ(TIM3_IRQn);gpio_write(BUZZER_PORT,BUZZER_PIN,1);TIM3->CR1=TIM_CR1_CEN;}
}
uint8_t buzzer_get_mode(void){return g_mode;}
void beep_stop(void){buzzer_set_mode2(0);}
void TIM3_IRQHandler(void){
 if(TIM3->SR & TIM_SR_UIF){
  TIM3->SR &= ~TIM_SR_UIF;
  if(!g_buzzer_on)return;
  if(g_mode==1){BUZZER_PORT->ODR ^= (1UL<<BUZZER_PIN);GPIOC->ODR ^= (1UL<<14U);}
  else if(g_mode==3){BUZZER_PORT->ODR ^= (1UL<<BUZZER_PIN);}
  else if(g_mode==4){BUZZER_PORT->ODR ^= (1UL<<BUZZER_PIN);GPIOC->ODR ^= (1UL<<14U);}
  g_test_count++;
 }
}
