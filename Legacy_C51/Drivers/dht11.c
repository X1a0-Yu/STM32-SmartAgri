#include <intrins.h>
#include "dht11.h"
#include "board_config.h"
#include "system_tick.h"

static void delay_us_approx(u16 us)
{
    while (us--) {
        _nop_();
        _nop_();
    }
}

static bit wait_level(bit level, u16 timeout)
{
    while (PIN_DHT11 != level) {
        if (timeout-- == 0) {
            return 0;
        }
        delay_us_approx(1);
    }
    return 1;
}

static u8 read_byte(void)
{
    u8 i;
    u8 value = 0;

    for (i = 0; i < 8; i++) {
        wait_level(0, 100);
        wait_level(1, 100);
        delay_us_approx(35);
        value <<= 1;
        if (PIN_DHT11) {
            value |= 1;
        }
        wait_level(0, 100);
    }

    return value;
}

void dht11_init(void)
{
    PIN_DHT11 = 1;
}

bit dht11_read(s16 *temperature, u8 *humidity)
{
    u8 humi_int;
    u8 humi_dec;
    u8 temp_int;
    u8 temp_dec;
    u8 checksum;
    bit ok;

    PIN_DHT11 = 0;
    delay_ms(20);
    PIN_DHT11 = 1;
    delay_us_approx(30);

    EA = 0;
    ok = wait_level(0, 100);
    if (ok) {
        ok = wait_level(1, 100);
    }
    if (ok) {
        ok = wait_level(0, 100);
    }

    if (!ok) {
        EA = 1;
        return 0;
    }

    humi_int = read_byte();
    humi_dec = read_byte();
    temp_int = read_byte();
    temp_dec = read_byte();
    checksum = read_byte();
    EA = 1;

    if ((u8)(humi_int + humi_dec + temp_int + temp_dec) != checksum) {
        return 0;
    }

    *humidity = humi_int;
    *temperature = temp_int;
    return 1;
}
