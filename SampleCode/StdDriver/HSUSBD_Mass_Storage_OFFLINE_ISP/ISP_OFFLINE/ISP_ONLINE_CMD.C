#include "stdio.h"
#include "NuMicro.h"
#include "ISP_CMD.h"
#include "ISP_DRIVER.H"
#include <string.h>
extern uint8_t g_u8OutBuff[64];
extern uint8_t g_u8InBuff[64];
io_handle_t  DEV_handle = NULL;


void ONLINE_COMMAND(void)
{
    uint32_t lcmd, pr;
    lcmd = inpw(g_u8OutBuff);
    pr = inpw(g_u8OutBuff + 4);

    if (lcmd == CMD_SET_INTERFACE) //to do auto detect command
    {
        memset(g_u8InBuff, 0, PACKET_SIZE);

        if (pr == 0x01)
        {
            g_u8InBuff[0] = 0xa1;
            io_open(UART_NAME_STRING, &DEV_handle);
        }

        if (pr == 0x02)
        {
            g_u8InBuff[0] = 0xa2;
            io_open(RS485_NAME_STRING, &DEV_handle);
        }

        if (pr == 0x03)
        {
            g_u8InBuff[0] = 0xa3;
            io_open(CAN_NAME_STRING, &DEV_handle);
        }

        if (pr == 0x04)
        {
            g_u8InBuff[0] = 0xa4;
            io_open(I2C_NAME_STRING, &DEV_handle);
        }

        if (pr == 0x05)
        {
            g_u8InBuff[0] = 0xa5;
            io_open(SPI_NAME_STRING, &DEV_handle);
        }


        return ;
    }

    if (lcmd == CMD_UPDATE_APROM) //to do auto detect command
    {
        devtab_entry_t *t = (devtab_entry_t *)DEV_handle;

        if (str_compare(SPI_NAME_STRING, t->name))
        {
            uint32_t out_len = 64;
            io_write(DEV_handle, g_u8OutBuff, &out_len);

            //delay for erase
            for (unsigned int i = 0; i < 5000; i++)
                CLK_SysTickDelay(2000);

            uint32_t in_len = 64;
            io_read(DEV_handle, g_u8InBuff, &in_len);
            return;
        }


        if (str_compare(I2C_NAME_STRING, t->name))
        {
            uint32_t out_len = 64;
            io_write(DEV_handle, g_u8OutBuff, &out_len);

            //delay for erase
            for (int i = 0; i < 2000; i++)
                CLK_SysTickDelay(5000);

            uint32_t in_len = 64;
            io_read(DEV_handle, g_u8InBuff, &in_len);
            return;
        }


    }

    uint32_t out_len = 64;
    io_write(DEV_handle, g_u8OutBuff, &out_len);
    uint32_t in_len = 64;
    io_read(DEV_handle, g_u8InBuff, &in_len);
}

