#ifndef APP_CONTROL_H
#define APP_CONTROL_H
#include "common.h"
typedef struct { int16_t temperature; uint8_t humidity; uint16_t air_quality; uint16_t light; uint16_t soil; uint8_t valid; } SensorData;
typedef struct { int16_t temp_min,temp_max; uint8_t humi_min,humi_max; uint16_t air_max,light_min,soil_min; } ThresholdConfig;
typedef struct { bool led1,led2,led3,led4,pump,fan,light,buzzer; } OutputState;
typedef enum { UI_VALUES,UI_THRESHOLDS,UI_TEMP_MIN,UI_TEMP_MAX,UI_HUMI_MIN,UI_HUMI_MAX,UI_AIR_MAX,UI_LIGHT_MIN,UI_SOIL_MIN } UiPage;
typedef enum { OUTPUT_AUTO=0,OUTPUT_FORCE_OFF=1,OUTPUT_FORCE_ON=2 } OutputMode;
typedef struct { OutputMode pump,fan,light,buzzer; uint32_t pump_until,fan_until,light_until,buzzer_until; } OverrideState;
extern SensorData g_sensor;
extern ThresholdConfig g_thresholds;
extern OutputState g_auto_outputs;
extern OutputState g_outputs;
extern OverrideState g_overrides;
extern UiPage g_ui_page;
void app_init(void);
void app_evaluate(void);
void app_service_overrides(uint32_t now);
bool app_set_thresholds(const ThresholdConfig *candidate);
bool app_set_override(const char *name,OutputMode mode,uint16_t ttl,uint32_t now);
uint16_t app_override_remaining(OutputMode mode,uint32_t until,uint32_t now);
void app_all_auto(void);
void app_next_page(void);
void app_next_field(void);
void app_adjust(int8_t direction);
bool app_editing(void);
#endif
