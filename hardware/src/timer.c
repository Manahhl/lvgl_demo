#include "timer.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "misc.h"

void timer2_it_init(uint16_t prescaler, uint16_t period)
{

    TIM_TimeBaseInitTypeDef TIM_Attr;
    NVIC_InitTypeDef NVIC_Attr;

    // 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_Attr.TIM_Prescaler     = prescaler; // 分频系数（倍数 = 系数+1）
    TIM_Attr.TIM_Period        = period;
    TIM_Attr.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_Attr.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_Attr);

    // 开启中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    NVIC_Attr.NVIC_IRQChannel                   = TIM2_IRQn;
    NVIC_Attr.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Attr.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_Attr.NVIC_IRQChannelSubPriority        = 0;
    NVIC_Init(&NVIC_Attr);

    // 使能定时器
    TIM_Cmd(TIM2, ENABLE);
}

void timer2_clearFlag()
{
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
}
