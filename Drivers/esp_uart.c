#include "esp_uart.h"
#include "board.h"
#define RX_SIZE 512
static volatile uint8_t rx[RX_SIZE];
static volatile uint16_t head,tail;
static volatile uint8_t errors;
static void set_brr(uint32_t baud){uint32_t fclk=SystemCoreClock;uint32_t div=fclk/(baud*16U);uint32_t frac=(fclk%(baud*16U)*16U)/(baud*16U);USART2->BRR=(div<<4)|frac;}
void esp_uart_init(uint32_t baud){RCC->APB2ENR|=RCC_APB2ENR_IOPAEN;RCC->APB1ENR|=RCC_APB1ENR_USART2EN;GPIOA->CRL=(GPIOA->CRL&~(0xFFUL<<8))|(0xBUL<<8)|(0x4UL<<12);set_brr(baud);USART2->CR1=USART_CR1_TE|USART_CR1_RE|USART_CR1_RXNEIE|USART_CR1_UE;NVIC_SetPriority(USART2_IRQn,2);NVIC_EnableIRQ(USART2_IRQn);}
void esp_uart_set_baud(uint32_t baud){USART2->CR1&=~USART_CR1_UE;set_brr(baud);USART2->CR1|=USART_CR1_UE;}
void USART2_IRQHandler(void){uint16_t sr=USART2->SR;uint8_t v=(uint8_t)USART2->DR;if(sr&(USART_SR_ORE|USART_SR_FE|USART_SR_NE|USART_SR_PE)){errors|=1;return;}if(sr&USART_SR_RXNE){uint16_t n=(head+1U)%RX_SIZE;if(n==tail)errors|=2;else{rx[head]=v;head=n;}}}
void esp_uart_putc(char c){while(!(USART2->SR&USART_SR_TXE)){}USART2->DR=(uint8_t)c;}
void esp_uart_write(const char*s){while(*s)esp_uart_putc(*s++);}
bit esp_uart_get(uint8_t*v){if(head==tail)return 0;*v=rx[tail];tail=(tail+1U)%RX_SIZE;return 1;}
uint8_t esp_uart_errors(void){uint8_t e=errors;errors=0;return e;}
