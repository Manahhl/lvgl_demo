#ifndef __GT911_H__
#define __GT911_H__

#include "stm32f10x.h"

typedef struct
{
    uint8_t isTouched;
    uint16_t x;
    uint16_t y;
} GT911_touch_point;

void gt911_init(void);

GT911_touch_point *gt911_scan(void);

#endif