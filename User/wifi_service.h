#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H
#include "common.h"
typedef enum { WIFI_OFF=0,WIFI_BOOTING,WIFI_AT_SYNC,WIFI_JOINING,WIFI_READY,WIFI_SENDING,WIFI_BACKOFF,WIFI_NO_AT } WiFiState;
typedef struct { bit enabled; char ssid[33]; char password[65]; char host[40]; uint16_t port; char path[48]; char token[33]; uint16_t period_s; } WiFiConfig;
typedef struct { WiFiState state; int16_t rssi; uint16_t error; uint32_t upload_ok; uint32_t upload_fail; bit at_present; bit connected; } WiFiStatus;
void wifi_service_init(void);
void wifi_service_poll(void);
void wifi_service_power(bit on);
bit wifi_service_configure(const WiFiConfig *cfg);
const WiFiConfig *wifi_service_config(void);
const WiFiStatus *wifi_service_status(void);
#endif
