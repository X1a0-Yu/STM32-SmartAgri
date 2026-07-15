#include "adc0832.h"
#include "board_config.h"

static void adc_delay(void)
{
    u8 i;
    for (i = 0; i < 8; i++) {
    }
}

void adc0832_init(void)
{
    PIN_ADC_CS = 1;
    PIN_ADC_CLK = 0;
    PIN_ADC_DIO = 1;
}

u16 adc0832_read(u8 channel)
{
    u8 i;
    u8 value = 0;

    channel = channel ? 1 : 0;
    PIN_ADC_CS = 0;
    adc_delay();

    PIN_ADC_DIO = 1;
    PIN_ADC_CLK = 1;
    adc_delay();
    PIN_ADC_CLK = 0;
    adc_delay();

    PIN_ADC_DIO = 1;
    PIN_ADC_CLK = 1;
    adc_delay();
    PIN_ADC_CLK = 0;
    adc_delay();

    PIN_ADC_DIO = channel;
    PIN_ADC_CLK = 1;
    adc_delay();
    PIN_ADC_CLK = 0;
    adc_delay();

    PIN_ADC_DIO = 1;
    for (i = 0; i < 8; i++) {
        PIN_ADC_CLK = 1;
        adc_delay();
        value <<= 1;
        if (PIN_ADC_DIO) {
            value |= 0x01;
        }
        PIN_ADC_CLK = 0;
        adc_delay();
    }

    PIN_ADC_CS = 1;
    return ((u16)value) * 4U;
}

u16 sensor_read_air_quality(void)
{
    return adc0832_read(ADC_CHANNEL_AIR);
}

u16 sensor_read_light(void)
{
    return adc0832_read(ADC_CHANNEL_LIGHT);
}

u16 sensor_read_soil(void)
{
    return adc0832_read(ADC_CHANNEL_SOIL);
}
