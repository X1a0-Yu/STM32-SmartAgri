#include "key.h"
#include "board.h"
static uint8_t stable,last;static uint16_t held;
void key_init(void){stable=last=3U;held=0;}
KeyEvent key_scan(void){uint8_t raw=(gpio_read(KEY1_PORT,KEY1_PIN)?1U:0U)|(gpio_read(KEY2_PORT,KEY2_PIN)?2U:0U);KeyEvent e=KEY_NONE;if(raw==last){if(held<60000U)held+=20U;}else{last=raw;held=0;return KEY_NONE;}if(held==40U){stable=raw;if(raw==0U)e=KEY_DEC;else if(raw==1U)e=KEY_INC;}if(held==700U&&raw==2U)e=KEY_ENTER_EXIT;if(raw==3U&&stable!=3U){if(stable==2U&&held<700U)e=KEY_NEXT;stable=3U;held=0;}return e;}
