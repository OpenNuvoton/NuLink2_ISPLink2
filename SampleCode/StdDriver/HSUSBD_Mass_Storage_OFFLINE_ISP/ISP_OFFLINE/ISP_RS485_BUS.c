#include "stdio.h"
#include "NuMicro.h"
#include "stdint.h"
#include "ISP_DRIVER.H"
#include "hal_api.h"
extern __align(4) uint8_t  uart_rcvbuf[64];
extern uint8_t volatile bUartDataReady;
ErrNo RS485_WRITE(io_handle_t handle, const void *buf, uint32 *len)
{
    RS485_WriteMultiBytes((uint8_t *)buf);
    return ENOERR;
}
extern volatile unsigned int AUTO_DETECT_VALUE;
ErrNo RS485_READ(io_handle_t handle, void *buf, uint32 *len)
{
    uint8_t i;
    uint8_t *pRxData;
    uint32 rxlen = *len;
    pRxData = (uint8_t *)buf;


    if (AUTO_DETECT_VALUE == 0)
    {
        while (bUartDataReady == 0);
    }
    else
    {
        SysTick->LOAD = 20000 * CyclesPerUs;
        SysTick->VAL  = 0x0UL;
        SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

        while (bUartDataReady == 0)
        {
            /* Waiting for down-count to zero */
            if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0UL)
            {
                break;
            }


        }

        /* Disable SysTick counter */
        SysTick->CTRL = 0UL;
    }


    for (i = 0; i < rxlen; i++)
    {
        pRxData[i] = uart_rcvbuf[i];
    }

    return ENOERR;
}



ErrNo RS485_Config(void *priv)
{

    return ENOERR;
}

ErrNo RS485_Package(io_handle_t handle, void *buf, uint32 *len)
{
    return ENOERR;

}




