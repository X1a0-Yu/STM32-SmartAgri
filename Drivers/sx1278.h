#ifndef SX1278_H
#define SX1278_H
#include "common.h"
typedef enum { LORA_OFF=0,LORA_DETECTING,LORA_READY,LORA_TX,LORA_RX,LORA_ERROR } LoRaState;
typedef struct { LoRaState state; uint8_t version; int16_t rssi; int16_t snr10; uint16_t error; uint32_t tx_count; uint32_t rx_count; uint32_t frequency_hz; } LoRaStatus;
void sx1278_init(void);
void sx1278_power(bit on);
bit sx1278_detect(void);
bit sx1278_config(uint32_t frequency_hz,uint8_t sf,uint8_t tx_power);
bit sx1278_send(const uint8_t *data,uint8_t len);
uint8_t sx1278_poll_receive(uint8_t *data,uint8_t max_len);
void sx1278_service(void);
const LoRaStatus *sx1278_status(void);
#endif
