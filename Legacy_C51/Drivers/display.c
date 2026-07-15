#include "display.h"
#include "serial2_printf.h"

static UiPage g_last_page = (UiPage)255;
static s16 g_last_temp = -32768;
static u8 g_last_humi = 255;
static u16 g_last_air = 65535U;
static u16 g_last_light = 65535U;
static u16 g_last_soil = 65535U;
static ThresholdConfig g_last_cfg;
static bit g_cfg_valid;

static bit thresholds_changed(const ThresholdConfig *cfg)
{
    if (!g_cfg_valid) {
        return 1;
    }
    return cfg->temp_min != g_last_cfg.temp_min ||
           cfg->temp_max != g_last_cfg.temp_max ||
           cfg->humi_min != g_last_cfg.humi_min ||
           cfg->humi_max != g_last_cfg.humi_max ||
           cfg->air_quality_max != g_last_cfg.air_quality_max ||
           cfg->light_min != g_last_cfg.light_min ||
           cfg->soil_min != g_last_cfg.soil_min;
}

static void remember(const SensorData *sensor, const ThresholdConfig *cfg, UiPage page)
{
    g_last_page = page;
    g_last_temp = sensor->temperature;
    g_last_humi = sensor->humidity;
    g_last_air = sensor->air_quality;
    g_last_light = sensor->light;
    g_last_soil = sensor->soil;
    g_last_cfg = *cfg;
    g_cfg_valid = 1;
}

void display_init(void)
{
    g_cfg_valid = 0;
    printf_20("DISPLAY READY\r\n");
}

void display_update(const SensorData *sensor, const ThresholdConfig *cfg, UiPage page)
{
    bit changed = 0;

    if (page != g_last_page || sensor->temperature != g_last_temp || sensor->humidity != g_last_humi ||
        sensor->air_quality != g_last_air || sensor->light != g_last_light || sensor->soil != g_last_soil ||
        thresholds_changed(cfg)) {
        changed = 1;
    }

    if (!changed) {
        return;
    }

    switch (page) {
        case UI_PAGE_VALUES:
            printf_20("LCD VALUES T:%d H:%u AIR:%u L:%u S:%u\r\n", sensor->temperature, sensor->humidity, sensor->air_quality, sensor->light, sensor->soil);
            break;
        case UI_PAGE_THRESHOLDS:
            printf_20("LCD TH T:%d-%d H:%u-%u AIR<%u L>%u S>%u\r\n", cfg->temp_min, cfg->temp_max, cfg->humi_min, cfg->humi_max, cfg->air_quality_max, cfg->light_min, cfg->soil_min);
            break;
        case UI_SET_TEMP_MIN:
            printf_20("SET TEMP MIN %d\r\n", cfg->temp_min);
            break;
        case UI_SET_TEMP_MAX:
            printf_20("SET TEMP MAX %d\r\n", cfg->temp_max);
            break;
        case UI_SET_HUMI_MIN:
            printf_20("SET HUMI MIN %u\r\n", cfg->humi_min);
            break;
        case UI_SET_HUMI_MAX:
            printf_20("SET HUMI MAX %u\r\n", cfg->humi_max);
            break;
        case UI_SET_AIR_MAX:
            printf_20("SET AIR MAX %u\r\n", cfg->air_quality_max);
            break;
        case UI_SET_LIGHT_MIN:
            printf_20("SET LIGHT MIN %u\r\n", cfg->light_min);
            break;
        case UI_SET_SOIL_MIN:
            printf_20("SET SOIL MIN %u\r\n", cfg->soil_min);
            break;
        default:
            break;
    }

    remember(sensor, cfg, page);
}
