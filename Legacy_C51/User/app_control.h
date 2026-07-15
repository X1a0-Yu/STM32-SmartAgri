#ifndef APP_CONTROL_H
#define APP_CONTROL_H

#include "common.h"

typedef struct {
    s16 temperature;
    u8 humidity;
    u16 air_quality;
    u16 light;
    u16 soil;
} SensorData;

typedef struct {
    s16 temp_min;
    s16 temp_max;
    u8 humi_min;
    u8 humi_max;
    u16 air_quality_max;
    u16 light_min;
    u16 soil_min;
} ThresholdConfig;

typedef struct {
    u8 led1_temp_high;
    u8 led2_temp_low;
    u8 led3_humi_high;
    u8 led4_humi_low;
    u8 relay_pump;
    u8 relay_fan;
    u8 relay_light;
    u8 buzzer;
} OutputState;

typedef enum {
    UI_PAGE_VALUES = 0,
    UI_PAGE_THRESHOLDS,
    UI_SET_TEMP_MIN,
    UI_SET_TEMP_MAX,
    UI_SET_HUMI_MIN,
    UI_SET_HUMI_MAX,
    UI_SET_AIR_MAX,
    UI_SET_LIGHT_MIN,
    UI_SET_SOIL_MIN
} UiPage;

extern xdata SensorData g_sensor_data;
extern xdata ThresholdConfig g_thresholds;
extern xdata OutputState g_output_state;
extern UiPage g_ui_page;

void app_init(void);
void app_update_outputs(void);
void app_next_page(void);
void app_next_item(void);
void app_adjust_selected(s8 delta);
bit app_is_setting_page(void);

#endif
