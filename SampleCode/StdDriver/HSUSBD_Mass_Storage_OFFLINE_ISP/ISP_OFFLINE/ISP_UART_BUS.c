#include "stdio.h"
#include "NuMicro.h"
#include "stdint.h"
#include "ISP_DRIVER.H"
#include "hal_api.h"
extern __align(4) uint8_t  uart_rcvbuf[64];
extern uint8_t volatile bUartDataReady;
extern uint8_t volatile bufhead;
#define MAX_PKT_SIZE            (64)


void UART_WriteMultiBytes(uint8_t *data)
{
    uint32_t i;
    NVIC_DisableIRQ(UART2_IRQn);
    bUartDataReady = 0;


    for (i = 0; i < MAX_PKT_SIZE; i++)
    {
        while ((UART2->FIFOSTS & UART_FIFOSTS_TXFULL_Msk));

        UART2->DAT = data[i];
    }

    while ((UART2->FIFOSTS & UART_FIFOSTS_TXEMPTYF_Msk) == 0);

    NVIC_EnableIRQ(UART2_IRQn);
}


ErrNo ISP_UART_WRITE(io_handle_t handle, const void *buf, uint32 *len)
{
    UART_WriteMultiBytes((uint8_t *)buf);
    return ENOERR;
}
extern volatile unsigned int AUTO_DETECT_VALUE;
ErrNo ISP_UART_READ(io_handle_t handle, void *buf, uint32 *len)
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
void UART2_IRQHandler(void)
{
    /*----- Determine interrupt source -----*/
    uint32_t u32IntSrc = UART2->INTSTS;

    if (u32IntSrc & 0x11)   //RDA FIFO interrupt & RDA timeout interrupt
    {
        while (((UART2->FIFOSTS & UART_FIFOSTS_RXEMPTY_Msk) == 0) && (bufhead < MAX_PKT_SIZE))      //RX fifo not empty
        {
            uart_rcvbuf[bufhead++] = UART2->DAT;
        }
    }

    if (bufhead == MAX_PKT_SIZE)
    {
        bUartDataReady = TRUE;
        bufhead = 0;

    }
    else if (u32IntSrc & 0x10)
    {
        bufhead = 0;
    }
}

ErrNo UART_Config(void *priv)
{
    /* Enable UART module clock */
    CLK->APBCLK0 |= CLK_APBCLK0_UART2CKEN_Msk;
    /* Select UART module clock source */
    CLK->CLKSEL3 = (CLK->CLKSEL3 & (~CLK_CLKSEL3_UART2SEL_Msk)) | CLK_CLKSEL3_UART2SEL_HXT;
    /* Set GPE multi-function pins for UART2 RXD and TXD */
    SYS->GPE_MFPH = (SYS->GPE_MFPH & ~(SYS_GPE_MFPH_PE8MFP_Msk | SYS_GPE_MFPH_PE9MFP_Msk))
                    | (SYS_GPE_MFPH_PE8MFP_UART2_TXD | SYS_GPE_MFPH_PE9MFP_UART2_RXD);
    /* Select UART function */
    UART2->FUNCSEL = UART_FUNCSEL_UART;
    /* Set UART line configuration */
    UART2->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
    /* Set UART Rx and RTS trigger level */
    UART2->FIFO = UART_FIFO_RFITL_14BYTES | UART_FIFO_RTSTRGLV_14BYTES;
    /* Set UART baud rate */
    UART2->BAUD = (UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HIRC, 115200));
    /* Set time-out interrupt comparator */
    UART2->TOUT = (UART2->TOUT & ~UART_TOUT_TOIC_Msk) | (0x40);
    NVIC_SetPriority(UART2_IRQn, 2);
    NVIC_EnableIRQ(UART2_IRQn);
    /* 0x0811 */
    UART2->INTEN = (UART_INTEN_TOCNTEN_Msk | UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);

    return ENOERR;
}





