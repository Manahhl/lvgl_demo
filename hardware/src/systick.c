#include "systick.h"
#include "misc.h"

#define SYSTICK_US_VAL (SystemCoreClock / 8000000ul - 1)
#define SYSTICK_MS_VAL (SystemCoreClock / 8000ul - 1)

void dealy(uint32_t resetVal, uint32_t resetCount)
{
    SysTick->LOAD = resetVal;                 /* set reload register */
    SysTick->VAL  = 0;                        /* set current value register */
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; /* Enable SysTick IRQ and SysTick Timer */
    while (resetCount > 0) {
        while ((SysTick->CTRL & 0x010000) == 0) {}
        resetCount--;
    }
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void delay_ms(uint32_t ms)
{
    dealy(SYSTICK_MS_VAL, ms);
}

void delay_us(uint32_t us)
{
    dealy(SYSTICK_US_VAL, us);
}
