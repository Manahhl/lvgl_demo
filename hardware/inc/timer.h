#ifndef __TIMER_H__
#define __TIMER_H__

#include "stm32f10x.h"

/**
 * 定时器2初始化
 * @param prescaler     分频系数 (倍数 = 系数 + 1)
 * @param period        计数重装载值
 *
 */
void timer2_it_init(uint16_t prescaler, uint16_t period);

void timer2_clearFlag(void);

void timer3_pwm_init(uint16_t prescaler, uint16_t period, uint16_t compareValue);

#endif