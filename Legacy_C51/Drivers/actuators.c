#include "actuators.h"
#include "board_config.h"

#define LEVEL_ON(active) ((active) ? 1 : 0)
#define LEVEL_OFF(active) ((active) ? 0 : 1)

static void write_leds_off(void)
{
    PIN_LED1 = LEVEL_OFF(LED_ACTIVE_LEVEL);
    PIN_LED2 = LEVEL_OFF(LED_ACTIVE_LEVEL);
    PIN_LED3 = LEVEL_OFF(LED_ACTIVE_LEVEL);
    PIN_LED4 = LEVEL_OFF(LED_ACTIVE_LEVEL);
}

static void write_relays_off(void)
{
    PIN_RELAY_PUMP = LEVEL_OFF(RELAY_ACTIVE_LEVEL);
    PIN_RELAY_FAN = LEVEL_OFF(RELAY_ACTIVE_LEVEL);
    PIN_RELAY_LIGHT = LEVEL_OFF(RELAY_ACTIVE_LEVEL);
}

void actuators_init(void)
{
    write_leds_off();
    write_relays_off();
    PIN_BUZZER = LEVEL_OFF(BUZZER_ACTIVE_LEVEL);
}

void actuators_apply(const OutputState *state)
{
    PIN_LED1 = state->led1_temp_high ? LEVEL_ON(LED_ACTIVE_LEVEL) : LEVEL_OFF(LED_ACTIVE_LEVEL);
    PIN_LED2 = state->led2_temp_low ? LEVEL_ON(LED_ACTIVE_LEVEL) : LEVEL_OFF(LED_ACTIVE_LEVEL);
    PIN_LED3 = state->led3_humi_high ? LEVEL_ON(LED_ACTIVE_LEVEL) : LEVEL_OFF(LED_ACTIVE_LEVEL);
    PIN_LED4 = state->led4_humi_low ? LEVEL_ON(LED_ACTIVE_LEVEL) : LEVEL_OFF(LED_ACTIVE_LEVEL);
    PIN_RELAY_PUMP = state->relay_pump ? LEVEL_ON(RELAY_ACTIVE_LEVEL) : LEVEL_OFF(RELAY_ACTIVE_LEVEL);
    PIN_RELAY_FAN = state->relay_fan ? LEVEL_ON(RELAY_ACTIVE_LEVEL) : LEVEL_OFF(RELAY_ACTIVE_LEVEL);
    PIN_RELAY_LIGHT = state->relay_light ? LEVEL_ON(RELAY_ACTIVE_LEVEL) : LEVEL_OFF(RELAY_ACTIVE_LEVEL);
    PIN_BUZZER = state->buzzer ? LEVEL_ON(BUZZER_ACTIVE_LEVEL) : LEVEL_OFF(BUZZER_ACTIVE_LEVEL);
}
