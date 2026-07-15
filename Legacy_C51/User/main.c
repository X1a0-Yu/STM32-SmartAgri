#include "board_config.h"
#include "system_tick.h"
#include "app_control.h"
#include "key.h"
#include "display.h"
#include "actuators.h"
#include "dht11.h"
#include "adc0832.h"
#include "serial2_printf.h"

static u32 g_last_key_tick;
static u32 g_last_sensor_tick;
static u32 g_last_display_tick;
static u32 g_last_serial_tick;

static void handle_key_event(KeyEvent event)
{
    switch (event) {
        case KEY_EVENT_MODE:
            app_next_page();
            break;
        case KEY_EVENT_SELECT:
            app_next_item();
            break;
        case KEY_EVENT_UP:
            if (!app_is_setting_page()) {
                app_next_item();
            }
            app_adjust_selected(1);
            break;
        case KEY_EVENT_DOWN:
            if (!app_is_setting_page()) {
                app_next_item();
            }
            app_adjust_selected(-1);
            break;
        default:
            break;
    }
}

static void read_sensors(void)
{
    s16 temperature;
    u8 humidity;

    if (dht11_read(&temperature, &humidity)) {
        g_sensor_data.temperature = temperature;
        g_sensor_data.humidity = humidity;
    }

    g_sensor_data.air_quality = sensor_read_air_quality();
    g_sensor_data.light = sensor_read_light();
    g_sensor_data.soil = sensor_read_soil();
    app_update_outputs();
    actuators_apply(&g_output_state);
}

void main(void)
{
    serial2_init();
    system_tick_init();
    key_init();
    adc0832_init();
    dht11_init();
    app_init();
    actuators_init();
    display_init();

    printf_20("SmartAgri STC89C52RC firmware start\r\n");

    while (1) {
        if (system_due(&g_last_key_tick, KEY_TASK_PERIOD_MS)) {
            handle_key_event(key_scan());
        }

        if (system_due(&g_last_sensor_tick, SENSOR_TASK_PERIOD_MS)) {
            read_sensors();
        }

        if (system_due(&g_last_display_tick, DISPLAY_TASK_PERIOD_MS)) {
            display_update(&g_sensor_data, &g_thresholds, g_ui_page);
        }

        if (system_due(&g_last_serial_tick, SERIAL_TASK_PERIOD_MS)) {
            serial2_report_all(&g_sensor_data, &g_thresholds, &g_output_state);
        }
    }
}
