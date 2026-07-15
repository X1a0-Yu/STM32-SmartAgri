#include "sx1278.h"
#include "board.h"
#include "system_tick.h"
#include <string.h>
#define NSS_PORT GPIOA
#define NSS_PIN 8U
#define RST_PORT GPIOA
#define RST_PIN 11U
#define POWER_PORT GPIOB
#define POWER_PIN 9U
#define REG_FIFO 0x00
#define REG_OPMODE 0x01
#define REG_FRF_MSB 0x06
#define REG_PA_CONFIG 0x09
#define REG_LNA 0x0C
#define REG_FIFO_ADDR_PTR 0x0D
#define REG_FIFO_TX_BASE 0x0E
#define REG_FIFO_RX_BASE 0x0F
#define REG_FIFO_RX_CURRENT 0x10
#define REG_IRQ_FLAGS 0x12
#define REG_RX_NB_BYTES 0x13
#define REG_PKT_SNR 0x19
#define REG_PKT_RSSI 0x1A
#define REG_MODEM_CONFIG1 0x1D
#define REG_MODEM_CONFIG2 0x1E
#define REG_PREAMBLE_MSB 0x20
#define REG_PREAMBLE_LSB 0x21
#define REG_PAYLOAD_LEN 0x22
#define REG_MODEM_CONFIG3 0x26
#define REG_SYNC_WORD 0x39
#define REG_VERSION 0x42
static LoRaStatus status;
static uint8_t spi(uint8_t v){while(!(SPI1->SR&SPI_SR_TXE)){}SPI1->DR=v;while(!(SPI1->SR&SPI_SR_RXNE)){}return(uint8_t)SPI1->DR;}
static void nss(bit high){gpio_write(NSS_PORT,NSS_PIN,high);}
static void wr(uint8_t reg,uint8_t val){nss(0);spi(reg|0x80);spi(val);nss(1);}
static uint8_t rd(uint8_t reg){uint8_t v;nss(0);spi(reg&0x7F);v=spi(0);nss(1);return v;}
static void burst_write(uint8_t reg,const uint8_t*d,uint8_t len){uint8_t i;nss(0);spi(reg|0x80);for(i=0;i<len;i++)spi(d[i]);nss(1);}
static void burst_read(uint8_t reg,uint8_t*d,uint8_t len){uint8_t i;nss(0);spi(reg&0x7F);for(i=0;i<len;i++)d[i]=spi(0);nss(1);}
void sx1278_power(bit on){gpio_write(POWER_PORT,POWER_PIN,on?0:1);status.state=on?LORA_DETECTING:LORA_OFF;}
void sx1278_init(void){RCC->APB2ENR|=RCC_APB2ENR_IOPAEN|RCC_APB2ENR_IOPBEN|RCC_APB2ENR_SPI1EN;gpio_config_output(GPIOA,5,0xBU);gpio_config_input(GPIOA,6,0x4U);gpio_config_output(GPIOA,7,0xBU);gpio_config_output(NSS_PORT,NSS_PIN,0x2U);gpio_config_output(RST_PORT,RST_PIN,0x2U);gpio_config_output(POWER_PORT,POWER_PIN,0x2U);nss(1);gpio_write(RST_PORT,RST_PIN,1);gpio_write(POWER_PORT,POWER_PIN,1);SPI1->CR1=SPI_CR1_MSTR|SPI_CR1_BR_1|SPI_CR1_SSM|SPI_CR1_SSI|SPI_CR1_SPE;memset(&status,0,sizeof(status));status.state=LORA_OFF;status.frequency_hz=433000000U;}
bit sx1278_detect(void){gpio_write(RST_PORT,RST_PIN,0);delay_ms(2);gpio_write(RST_PORT,RST_PIN,1);delay_ms(10);status.version=rd(REG_VERSION);if(status.version==0x12){status.state=LORA_READY;return 1;}status.state=LORA_ERROR;status.error=1;return 0;}
bit sx1278_config(uint32_t freq,uint8_t sf,uint8_t power){uint64_t frf;if(status.version!=0x12)return 0;if(freq<410000000U||freq>525000000U||sf<6||sf>12||power>17)return 0;wr(REG_OPMODE,0x80);frf=((uint64_t)freq<<19)/32000000U;wr(REG_FRF_MSB,(uint8_t)(frf>>16));wr(REG_FRF_MSB+1,(uint8_t)(frf>>8));wr(REG_FRF_MSB+2,(uint8_t)frf);wr(REG_FIFO_TX_BASE,0);wr(REG_FIFO_RX_BASE,0);wr(REG_LNA,0x23);wr(REG_MODEM_CONFIG1,0x72);wr(REG_MODEM_CONFIG2,(uint8_t)((sf<<4)|0x04));wr(REG_MODEM_CONFIG3,0x04);wr(REG_PREAMBLE_MSB,0);wr(REG_PREAMBLE_LSB,8);wr(REG_SYNC_WORD,0x12);wr(REG_PA_CONFIG,(uint8_t)(0x80|(power+2)));wr(REG_OPMODE,0x85);status.frequency_hz=freq;status.state=LORA_RX;return 1;}
bit sx1278_send(const uint8_t*d,uint8_t len){if(status.state!=LORA_READY&&status.state!=LORA_RX)return 0;wr(REG_OPMODE,0x81);wr(REG_FIFO_ADDR_PTR,0);burst_write(REG_FIFO,d,len);wr(REG_PAYLOAD_LEN,len);wr(REG_IRQ_FLAGS,0xFF);wr(REG_OPMODE,0x83);status.state=LORA_TX;return 1;}
uint8_t sx1278_poll_receive(uint8_t*d,uint8_t max){uint8_t flags,len;if(status.state==LORA_TX)return 0;flags=rd(REG_IRQ_FLAGS);if(!(flags&0x40))return 0;wr(REG_IRQ_FLAGS,flags);if(flags&0x20){status.error=2;return 0;}len=rd(REG_RX_NB_BYTES);if(len>max)len=max;wr(REG_FIFO_ADDR_PTR,rd(REG_FIFO_RX_CURRENT));burst_read(REG_FIFO,d,len);status.snr10=(int8_t)rd(REG_PKT_SNR)*25/10;status.rssi=(int16_t)rd(REG_PKT_RSSI)-164;status.rx_count++;return len;}
void sx1278_service(void){if(status.state==LORA_TX){uint8_t flags=rd(REG_IRQ_FLAGS);if(flags&0x08){wr(REG_IRQ_FLAGS,0x08);status.tx_count++;wr(REG_OPMODE,0x85);status.state=LORA_RX;}}}
const LoRaStatus*sx1278_status(void){return &status;}
