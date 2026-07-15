#include "adc.h"
#include "stm32f10x.h"
#include "board.h"
#include "system_tick.h"

void adc_init(void){
 RCC->APB2ENR|=RCC_APB2ENR_ADC1EN|RCC_APB2ENR_IOPAEN|RCC_APB2ENR_IOPBEN;
 RCC->CFGR=(RCC->CFGR&~RCC_CFGR_ADCPRE)|RCC_CFGR_ADCPRE_DIV6;
 gpio_config_input(GPIOA,1,0);gpio_config_input(GPIOA,4,0);gpio_config_input(GPIOB,1,0);
 ADC1->CR2=ADC_CR2_ADON;delay_us(10);ADC1->CR2|=ADC_CR2_RSTCAL;while(ADC1->CR2&ADC_CR2_RSTCAL){}
 ADC1->CR2|=ADC_CR2_CAL;while(ADC1->CR2&ADC_CR2_CAL){}
}
static uint16_t adc_once(uint8_t ch){
 uint8_t shift=(uint8_t)((ch%10U)*3U);if(ch<10U)ADC1->SMPR2=(ADC1->SMPR2&~(7UL<<shift))|(7UL<<shift);else ADC1->SMPR1=(ADC1->SMPR1&~(7UL<<shift))|(7UL<<shift);
 ADC1->SQR1=0;ADC1->SQR3=ch;ADC1->CR2|=ADC_CR2_ADON;while(!(ADC1->SR&ADC_SR_EOC)){}return(uint16_t)ADC1->DR;
}
uint16_t adc_read10(uint8_t channel){uint32_t sum=0;uint8_t i;for(i=0;i<8;i++)sum+=adc_once(channel);return(uint16_t)(((sum/8U)+2U)>>2);}
void adc_read_environment(uint16_t*air,uint16_t*light,uint16_t*soil){*soil=adc_read10(1);*air=adc_read10(4);*light=adc_read10(9);}
