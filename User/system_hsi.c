#include "stm32f10x.h"
uint32_t SystemCoreClock = 8000000U;
void SystemCoreClockUpdate(void){}
void SystemInit(void){
 RCC->CR |= RCC_CR_HSION;
 while(!(RCC->CR & RCC_CR_HSIRDY)){}
 RCC->CFGR = 0U;
 RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSON | RCC_CR_PLLON);
 RCC->CIR = 0U;
 SystemCoreClock = 8000000U;
 SCB->VTOR = FLASH_BASE | 0U;
}
