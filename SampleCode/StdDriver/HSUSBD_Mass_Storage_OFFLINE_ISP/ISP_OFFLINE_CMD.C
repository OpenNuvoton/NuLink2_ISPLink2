#include "stdio.h"
#include "NuMicro.h"
#include "ISP_CMD.h"
#include "ISP_DRIVER.H"
extern uint8_t g_u8OutBuff[64];
extern uint8_t g_u8InBuff[64];
io_handle_t  DEV_handle = NULL;
void OFFLINE_COMMAND(void)
{
    uint32_t lcmd;
    lcmd = inpw(u8OutBuff);
    pr = inpw(u8OutBuff + 4);

    if (lcmd == CMD_SET_INTERFACE) //to do auto detect command
    {
        memset(g_u8InBuff, 0, PACKET_SIZE);

        if (pr == 0x01)
        {
            g_u8InBuff[0] = 0xab;
            DEV_handle = io_open(UART_NAME_STRING, &DEV_handle);
        }
    }

}