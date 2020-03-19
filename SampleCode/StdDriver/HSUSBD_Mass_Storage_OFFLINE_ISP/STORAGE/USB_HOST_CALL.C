#include "NuMicro.h"
//USB HOST
volatile uint32_t  g_tick_cnt;
uint32_t           g_t0;


void TMR0_IRQHandler(void)
{
    /* Clear Timer0 time-out interrupt flag */
    TIMER_ClearIntFlag(TIMER0);
    g_tick_cnt++;
}

uint32_t get_ticks()
{
    return g_tick_cnt;
}


void enable_USB_HOST_tick(int ticks_per_second)
{
    g_tick_cnt = 0;
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 10);
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);
    TIMER_Start(TIMER0);
}
#if  0
void SysTick_Handler(void)
{

}

void enable_sys_tick(int ticks_per_second)
{
    g_tick_cnt = 0;

    if (SysTick_Config(SystemCoreClock / ticks_per_second))
    {
        /* Setup SysTick Timer for 1 second interrupts  */
        printf("Set system tick error!!\n");

        while (1);
    }
}



#endif


//for usb host
void delay_us(int usec)
{
    /*
     *  Configure Timer1, clock source from XTL_12M. Prescale 12
     */
    /* TIMER1 clock from HXT */
    CLK->CLKSEL1 = (CLK->CLKSEL1 & (~CLK_CLKSEL1_TMR1SEL_Msk)) | CLK_CLKSEL1_TMR1SEL_HXT;
    CLK->APBCLK0 |= CLK_APBCLK0_TMR1CKEN_Msk;
    TIMER1->CTL = 0;        /* disable timer */
    TIMER1->INTSTS = 0x3;   /* write 1 to clear for safty */
    TIMER1->CMP = usec;
    TIMER1->CTL = (11 << TIMER_CTL_PSC_Pos) | TIMER_ONESHOT_MODE | TIMER_CTL_CNTEN_Msk;

    while (!TIMER1->INTSTS);
}
