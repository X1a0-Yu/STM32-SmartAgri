#include <REG52.H>
#include "system_tick.h"

static xdata volatile u32 g_ms_ticks;

void system_tick_init(void)
{
    TMOD &= 0xF0;
    TMOD |= 0x01;
    TH0 = 0xFC;
    TL0 = 0x66;
    ET0 = 1;
    TR0 = 1;
    EA = 1;
}

u32 system_millis(void)
{
    u32 value;
    EA = 0;
    value = g_ms_ticks;
    EA = 1;
    return value;
}

bit system_due(u32 *last_tick, u16 period_ms)
{
    u32 now = system_millis();
    if ((u32)(now - *last_tick) >= period_ms) {
        *last_tick = now;
        return 1;
    }
    return 0;
}

void delay_ms(u16 ms)
{
    u32 start = system_millis();
    while ((u32)(system_millis() - start) < ms) {
    }
}

void timer0_isr(void) interrupt 1
{
    TH0 = 0xFC;
    TL0 = 0x66;
    g_ms_ticks++;
}
