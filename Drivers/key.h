#ifndef KEY_H
#define KEY_H
#include "common.h"
typedef enum{KEY_NONE,KEY_NEXT,KEY_ENTER_EXIT,KEY_INC,KEY_DEC}KeyEvent;
void key_init(void);
KeyEvent key_scan(void);
#endif
