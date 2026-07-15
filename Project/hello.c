#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f10x.h"
int main(void){
 volatile uint32_t i;
 const char msg[] = "HELLO FROM USART1\r\n";
 RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
 GPIOA->CRL = (GPIOA->CRL & ~(0xFUL << 20)) | (0x3UL << 20);
 USART1->BRR = 72000000U / 9600U;
 USART1->CR1 = USART_CR1_TE | USART_CR1_UE;
 while(1){
  for(i=0; msg[i]; i++){while(!(USART1->SR & USART_SR_TXE)){}USART1->DR = msg[i];}
  GPIOA->ODR ^= (1UL << 5);
  for(i=0;i<2000000;i++){}
 }
}
#ifdef __cplusplus
}
#endif
