#include <REG52.H>
#include <stdarg.h>
#include "serial2_printf.h"

static void serial2_send_uint(u16 value)
{
    char buffer[6];
    char index = 0;

    if (value == 0) {
        serial2_send_char('0');
        return;
    }

    while (value > 0 && index < 5) {
        buffer[index++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (index > 0) {
        serial2_send_char(buffer[--index]);
    }
}

static void serial2_send_int(s16 value)
{
    if (value < 0) {
        serial2_send_char('-');
        serial2_send_uint((u16)(-value));
    } else {
        serial2_send_uint((u16)value);
    }
}

void serial2_init(void)
{
    SCON = 0x50;
    PCON |= 0x80;
    TMOD &= 0x0F;
    TMOD |= 0x20;
    TH1 = 0xFA;
    TL1 = 0xFA;
    TI = 0;
    RI = 0;
    TR1 = 1;
}

void serial2_send_char(char value)
{
    SBUF = value;
    while (TI == 0) {
    }
    TI = 0;
}

void serial2_send_string(const char *text)
{
    while (*text != '\0') {
        serial2_send_char(*text++);
    }
}

void printf_20(const char *fmt, ...)
{
    va_list args;
    char c;
    char *text;

    va_start(args, fmt);

    while (*fmt != '\0') {
        if (*fmt != '%') {
            serial2_send_char(*fmt++);
            continue;
        }

        fmt++;
        c = *fmt++;
        switch (c) {
            case 'd':
                serial2_send_int((s16)va_arg(args, int));
                break;
            case 'u':
                serial2_send_uint((u16)va_arg(args, int));
                break;
            case 's':
                text = va_arg(args, char *);
                if (text != 0) {
                    serial2_send_string(text);
                }
                break;
            case 'c':
                serial2_send_char((char)va_arg(args, int));
                break;
            case '%':
                serial2_send_char('%');
                break;
            default:
                serial2_send_char('%');
                serial2_send_char(c);
                break;
        }
    }

    va_end(args);
}

void serial2_report_all(const SensorData *sensor, const ThresholdConfig *cfg, const OutputState *state)
{
    u8 alarm_bits = 0;

    if (state->led1_temp_high || state->led2_temp_low) {
        alarm_bits |= 0x01;
    }
    if (state->led3_humi_high || state->led4_humi_low) {
        alarm_bits |= 0x02;
    }
    if (state->relay_fan) {
        alarm_bits |= 0x04;
    }
    if (state->relay_light) {
        alarm_bits |= 0x08;
    }
    if (state->relay_pump) {
        alarm_bits |= 0x10;
    }

    printf_20("DATA,T=%d,H=%u,AIR=%u,LIGHT=%u,SOIL=%u,", sensor->temperature, sensor->humidity, sensor->air_quality, sensor->light, sensor->soil);
    printf_20("TMIN=%d,TMAX=%d,HMIN=%u,HMAX=%u,", cfg->temp_min, cfg->temp_max, cfg->humi_min, cfg->humi_max);
    printf_20("AIRMAX=%u,LMIN=%u,SMIN=%u,ALM=%u,", cfg->air_quality_max, cfg->light_min, cfg->soil_min, alarm_bits);
    printf_20("LED1=%u,LED2=%u,LED3=%u,LED4=%u,PUMP=%u,FAN=%u,LAMP=%u,BEEP=%u\r\n",
              state->led1_temp_high, state->led2_temp_low, state->led3_humi_high, state->led4_humi_low,
              state->relay_pump, state->relay_fan, state->relay_light, state->buzzer);
}
