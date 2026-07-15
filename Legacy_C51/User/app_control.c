#include "app_control.h"
#include "board_config.h"

xdata SensorData g_sensor_data;
xdata ThresholdConfig g_thresholds;
xdata OutputState g_output_state;
UiPage g_ui_page;

static s16 clamp_s16(s16 value, s16 min_value, s16 max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static u16 clamp_u16(s16 value, u16 min_value, u16 max_value)
{
    if (value < (s16)min_value) {
        return min_value;
    }
    if (value > (s16)max_value) {
        return max_value;
    }
    return (u16)value;
}

void app_init(void)
{
    g_sensor_data.temperature = 25;
    g_sensor_data.humidity = 55;
    g_sensor_data.air_quality = 300;
    g_sensor_data.light = 500;
    g_sensor_data.soil = 500;

    g_thresholds.temp_min = DEFAULT_TEMP_MIN;
    g_thresholds.temp_max = DEFAULT_TEMP_MAX;
    g_thresholds.humi_min = DEFAULT_HUMI_MIN;
    g_thresholds.humi_max = DEFAULT_HUMI_MAX;
    g_thresholds.air_quality_max = DEFAULT_AIR_QUALITY_MAX;
    g_thresholds.light_min = DEFAULT_LIGHT_MIN;
    g_thresholds.soil_min = DEFAULT_SOIL_MIN;

    g_ui_page = UI_PAGE_VALUES;
    app_update_outputs();
}

void app_update_outputs(void)
{
    g_output_state.led1_temp_high = (g_sensor_data.temperature > g_thresholds.temp_max);
    g_output_state.led2_temp_low = (g_sensor_data.temperature < g_thresholds.temp_min);
    g_output_state.led3_humi_high = (g_sensor_data.humidity > g_thresholds.humi_max);
    g_output_state.led4_humi_low = (g_sensor_data.humidity < g_thresholds.humi_min);
    g_output_state.relay_pump = (g_sensor_data.soil < g_thresholds.soil_min);
    g_output_state.relay_fan = (g_sensor_data.air_quality > g_thresholds.air_quality_max);
    g_output_state.relay_light = (g_sensor_data.light < g_thresholds.light_min);
    g_output_state.buzzer = g_output_state.relay_fan;
}

void app_next_page(void)
{
    if (app_is_setting_page()) {
        g_ui_page = UI_PAGE_VALUES;
        return;
    }

    if (g_ui_page == UI_PAGE_VALUES) {
        g_ui_page = UI_PAGE_THRESHOLDS;
    } else {
        g_ui_page = UI_PAGE_VALUES;
    }
}

void app_next_item(void)
{
    if (!app_is_setting_page()) {
        g_ui_page = UI_SET_TEMP_MIN;
        return;
    }

    if (g_ui_page == UI_SET_SOIL_MIN) {
        g_ui_page = UI_SET_TEMP_MIN;
    } else {
        g_ui_page = (UiPage)((u8)g_ui_page + 1U);
    }
}

void app_adjust_selected(s8 delta)
{
    s16 value_delta = delta;

    switch (g_ui_page) {
        case UI_SET_TEMP_MIN:
            g_thresholds.temp_min = clamp_s16(g_thresholds.temp_min + value_delta, TEMP_MIN_LIMIT, g_thresholds.temp_max);
            break;
        case UI_SET_TEMP_MAX:
            g_thresholds.temp_max = clamp_s16(g_thresholds.temp_max + value_delta, g_thresholds.temp_min, TEMP_MAX_LIMIT);
            break;
        case UI_SET_HUMI_MIN:
            g_thresholds.humi_min = (u8)clamp_u16((s16)g_thresholds.humi_min + value_delta, HUMI_MIN_LIMIT, g_thresholds.humi_max);
            break;
        case UI_SET_HUMI_MAX:
            g_thresholds.humi_max = (u8)clamp_u16((s16)g_thresholds.humi_max + value_delta, g_thresholds.humi_min, HUMI_MAX_LIMIT);
            break;
        case UI_SET_AIR_MAX:
            g_thresholds.air_quality_max = clamp_u16((s16)g_thresholds.air_quality_max + value_delta * 10, ADC_MIN_LIMIT, ADC_MAX_LIMIT);
            break;
        case UI_SET_LIGHT_MIN:
            g_thresholds.light_min = clamp_u16((s16)g_thresholds.light_min + value_delta * 10, ADC_MIN_LIMIT, ADC_MAX_LIMIT);
            break;
        case UI_SET_SOIL_MIN:
            g_thresholds.soil_min = clamp_u16((s16)g_thresholds.soil_min + value_delta * 10, ADC_MIN_LIMIT, ADC_MAX_LIMIT);
            break;
        default:
            break;
    }

    app_update_outputs();
}

bit app_is_setting_page(void)
{
    return (g_ui_page >= UI_SET_TEMP_MIN);
}
