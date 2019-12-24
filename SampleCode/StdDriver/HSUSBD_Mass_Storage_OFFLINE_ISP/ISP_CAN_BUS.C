#include <stdio.h>
#include "NuMicro.h"
#include "stdint.h"
#include "ISP_DRIVER.H"
#include "hal_api.h"
#include "ISP_CMD.h"
extern volatile uint8_t u8CAN_PackageFlag;
#define CAN_CMD_READ_CONFIG                   0xA2000000
#define CAN_CMD_RUN_APROM                     0xAB000000
#define CAN_CMD_GET_DEVICEID                  0xB1000000
extern void CAN_Transmit_ISP(uint32_t cmd, uint32_t data);
extern STR_CANMSG_T rrMsg;
unsigned char loca_buffer[64];
unsigned char response_buff[64];
uint16_t Checksum_up(unsigned char *buf, int len)
{
    int i;
    uint16_t c;

    for (c = 0, i = 0 ; i < len; i++)
    {
        c += buf[i];
    }

    return (c);
}
volatile uint32_t StartAddress, TotalLen;
volatile      uint32_t glcmd;
ErrNo CAN_WRITE(io_handle_t handle, const void *buf, uint32 *len)
{
    int i;
    uint8_t *pSrc;
    uint32_t txlen = *len;
    uint32_t g_packno;
    uint8_t *response;
    uint32_t lcmd, outdata;
    uint16_t lcksum;
    uint8_t srclen;
    response = response_buff;
    pSrc = (uint8_t *)buf;
    srclen = txlen;

    for (i = 0; i < txlen; i++)
    {
        loca_buffer[i] = pSrc[i];
    }

    lcmd = inpw(pSrc);
    g_packno = inpw(pSrc + 4);


    lcksum = Checksum_up(pSrc, txlen);
    outps(response, lcksum);
    outpw(response + 4, ++g_packno);

    if (glcmd == CMD_UPDATE_APROM)
    {
        srclen = 64 - 8;

        if (TotalLen < srclen)
        {
            srclen = TotalLen;//prevent last package from over writing
            TotalLen = 0;
            glcmd = 0;
        }
        else
        {
            TotalLen = TotalLen - srclen;
        }

        for (i = 0; i < srclen; i = i + 4)
        {
            outdata = inpw(pSrc + i + 8);
            //printf("0x%x\n\r",outdata);
            CAN_Transmit_ISP(StartAddress, outdata);

            while (u8CAN_PackageFlag == 0);

            if (outdata != *(unsigned int *)&rrMsg.Data[4])
            {
                outps(response, 0);
                outpw(response + 4, 0);

                goto CAN_ERROR;
            }

            StartAddress = StartAddress + 4;
        }

    }

    if (lcmd == CMD_UPDATE_APROM)
    {
        StartAddress = inpw(pSrc + 8);
        TotalLen = inpw(pSrc + 12);
        srclen = 64 - 16;

        if (TotalLen < srclen)
        {
            srclen = TotalLen;//prevent last package from over writing
        }
        else
        {
            glcmd = CMD_UPDATE_APROM;
            TotalLen = TotalLen - srclen;
        }

        for (i = 0; i < srclen; i = i + 4)
        {
            //printf("0x%x\n\r",StartAddress);
            outdata = inpw(pSrc + i + 16);
            // printf("0x%x\n\r",outdata);
            CAN_Transmit_ISP(StartAddress, outdata);

            while (u8CAN_PackageFlag == 0);

            if (outdata != *(unsigned int *)&rrMsg.Data[4])
            {
                outps(response, 0);
                outpw(response + 4, 0);

                goto CAN_ERROR;
            }

            StartAddress = StartAddress + 4;
        }

    }


#if 1

    if (lcmd == CMD_GET_DEVICEID)
    {
        CAN_Transmit_ISP(CAN_CMD_GET_DEVICEID, 0);

        while (u8CAN_PackageFlag == 0);

        //printf("0x%x",*(unsigned int*)&rrMsg.Data[4]); //conver data to uint32
        outpw(response + 8, *(unsigned int *)&rrMsg.Data[4]);
    }


    if (lcmd == CMD_RUN_APROM)
    {
        CAN_Transmit_ISP(CAN_CMD_RUN_APROM, 0);
    }

    if (lcmd == CMD_GET_FWVER)
    {
        outpw(response + 8, 0xffffff5a);
    }

    if (lcmd == CMD_GET_FLASHMODE)
    {
        outpw(response + 8, 0X02);
    }

    if (lcmd == CMD_READ_CONFIG)
    {
        CAN_Transmit_ISP(CAN_CMD_READ_CONFIG, 0x00300000);//CONFIG0

        while (u8CAN_PackageFlag == 0);

        outpw(response + 8, *(unsigned int *)&rrMsg.Data[4]);

        CAN_Transmit_ISP(CAN_CMD_READ_CONFIG, 0x00300004);//CONFIG1

        while (u8CAN_PackageFlag == 0);

        outpw(response + 12, *(unsigned int *)&rrMsg.Data[4]);

        CAN_Transmit_ISP(CAN_CMD_READ_CONFIG, 0x00300008);//CONFIG2

        while (u8CAN_PackageFlag == 0);

        outpw(response + 16, *(unsigned int *)&rrMsg.Data[4]);

        CAN_Transmit_ISP(CAN_CMD_READ_CONFIG, 0x0030000C);//CONFIG2

        while (u8CAN_PackageFlag == 0);

        outpw(response + 20, *(unsigned int *)&rrMsg.Data[4]);
    }

#endif
CAN_ERROR:
    return ENOERR;
}

ErrNo CAN_READ(io_handle_t handle, void *buf, uint32 *len)
{
    uint8_t i;
    uint8_t *pRxData;
    uint32 rxlen = *len;
    pRxData = (uint8_t *)buf;

    for (i = 0; i < rxlen; i++)
    {
        pRxData[i] = response_buff[i];
    }

    return ENOERR;
}

ErrNo CAN_Config(void *priv)
{

    return ENOERR;
}
ErrNo CAN_Package(io_handle_t handle, void *buf, uint32 *len)
{
    return ENOERR;
}

