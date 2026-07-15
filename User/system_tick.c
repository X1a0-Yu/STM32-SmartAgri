#include "stm32f10x.h"
#include "system_tick.h"
static volatile uint32_t ticks;

void system_tick_init(void)
{
    SysTick_Config(SystemCoreClock / 1000U);
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = (SystemCoreClock / 1000000U) - 1U;
    TIM2->ARR = 0xFFFFU;
    TIM2->CR1 = TIM_CR1_CEN;
}

void SysTick_Handler(void) { ticks++; }
uint32_t system_millis(void) { return ticks; }
bool system_due(uint32_t *last, uint32_t period)
{
    uint32_t now = ticks;
    if ((uint32_t)(now - *last) >= period) { *last = now; return true; }
    return false;
}
void delay_ms(uint32_t ms) { uint32_t start = ticks; while ((uint32_t)(ticks - start) < ms) {} }
uint16_t micros16(void) { return (uint16_t)TIM2->CNT; }
void delay_us(uint16_t us) { uint16_t start = micros16(); while ((uint16_t)(micros16() - start) < us) {} }
