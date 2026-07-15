#include "key.h"
#include "board_config.h"

typedef struct {
    u8 stable;
    u8 last_raw;
    u8 count;
    u8 reported;
} KeyDebounce;

static xdata KeyDebounce g_mode_key;
static xdata KeyDebounce g_select_key;
static xdata KeyDebounce g_up_key;
static xdata KeyDebounce g_down_key;

static void key_state_init(KeyDebounce *key)
{
    key->stable = 1;
    key->last_raw = 1;
    key->count = 0;
    key->reported = 0;
}

static bit key_update(KeyDebounce *key, u8 raw)
{
    if (raw == key->last_raw) {
        if (key->count < 3) {
            key->count++;
        }
    } else {
        key->count = 0;
        key->last_raw = raw;
    }

    if (key->count >= 2 && key->stable != raw) {
        key->stable = raw;
        if (raw == KEY_ACTIVE_LEVEL) {
            key->reported = 0;
        }
    }

    if (key->stable == KEY_ACTIVE_LEVEL && !key->reported) {
        key->reported = 1;
        return 1;
    }

    if (key->stable != KEY_ACTIVE_LEVEL) {
        key->reported = 0;
    }

    return 0;
}

void key_init(void)
{
    PIN_KEY_MODE = 1;
    PIN_KEY_SELECT = 1;
    PIN_KEY_UP = 1;
    PIN_KEY_DOWN = 1;
    key_state_init(&g_mode_key);
    key_state_init(&g_select_key);
    key_state_init(&g_up_key);
    key_state_init(&g_down_key);
}

KeyEvent key_scan(void)
{
    if (key_update(&g_mode_key, PIN_KEY_MODE)) {
        return KEY_EVENT_MODE;
    }
    if (key_update(&g_select_key, PIN_KEY_SELECT)) {
        return KEY_EVENT_SELECT;
    }
    if (key_update(&g_up_key, PIN_KEY_UP)) {
        return KEY_EVENT_UP;
    }
    if (key_update(&g_down_key, PIN_KEY_DOWN)) {
        return KEY_EVENT_DOWN;
    }
    return KEY_EVENT_NONE;
}
