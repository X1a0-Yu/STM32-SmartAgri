#ifndef KEY_H
#define KEY_H

#include "common.h"

typedef enum {
    KEY_EVENT_NONE = 0,
    KEY_EVENT_MODE,
    KEY_EVENT_SELECT,
    KEY_EVENT_UP,
    KEY_EVENT_DOWN
} KeyEvent;

void key_init(void);
KeyEvent key_scan(void);

#endif
