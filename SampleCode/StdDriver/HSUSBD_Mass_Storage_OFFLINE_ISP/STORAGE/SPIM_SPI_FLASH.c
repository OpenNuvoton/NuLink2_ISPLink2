/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * @brief    This sample demonstrates how to issue SPI flash erase, program, and read commands under SPIM I/O mode.
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "NuMicro.h"

#define DRVSPIFLASH_PAGE_SIZE       256
#define DRVSPIFLASH_SECTOR_SIZE     4096

uint32_t g_sectorBuf[1024]; /* 4096 bytes */
#define USE_4_BYTES_MODE            0            /* W25Q20 does not support 4-bytes address mode. */

//porting define
void SpiInit(void)
{

    uint8_t     idBuf[3];

    /* Enable SPIM module clock */
    CLK_EnableModuleClock(SPIM_MODULE);

    /* Init SPIM multi-function pins, MOSI(PC.0), MISO(PC.1), CLK(PC.2), SS(PC.3), D3(PC.4), and D2(PC.5) */
    SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC0MFP_Msk | SYS_GPC_MFPL_PC1MFP_Msk | SYS_GPC_MFPL_PC2MFP_Msk |
                       SYS_GPC_MFPL_PC3MFP_Msk | SYS_GPC_MFPL_PC4MFP_Msk | SYS_GPC_MFPL_PC5MFP_Msk);
    SYS->GPC_MFPL |= SYS_GPC_MFPL_PC0MFP_SPIM_MOSI | SYS_GPC_MFPL_PC1MFP_SPIM_MISO |
                     SYS_GPC_MFPL_PC2MFP_SPIM_CLK | SYS_GPC_MFPL_PC3MFP_SPIM_SS |
                     SYS_GPC_MFPL_PC4MFP_SPIM_D3 | SYS_GPC_MFPL_PC5MFP_SPIM_D2;
    PC->SMTEN |= GPIO_SMTEN_SMTEN2_Msk;

    /* Set SPIM I/O pins as high slew rate up to 80 MHz. */
    PC->SLEWCTL = (PC->SLEWCTL & 0xFFFFF000) |
                  (0x1 << GPIO_SLEWCTL_HSREN0_Pos) | (0x1 << GPIO_SLEWCTL_HSREN1_Pos) |
                  (0x1 << GPIO_SLEWCTL_HSREN2_Pos) | (0x1 << GPIO_SLEWCTL_HSREN3_Pos) |
                  (0x1 << GPIO_SLEWCTL_HSREN4_Pos) | (0x1 << GPIO_SLEWCTL_HSREN5_Pos);
    SPIM_SET_CLOCK_DIVIDER(3);        /* Set SPIM clock as HCLK divided by 2 */

    SPIM_SET_RXCLKDLY_RDDLYSEL(0);    /* Insert 0 delay cycle. Adjust the sampling clock of received data to latch the correct data. */
    SPIM_SET_RXCLKDLY_RDEDGE();       /* Use SPI input clock rising edge to sample received data. */

    SPIM_SET_DCNUM(8);                /* Set 8 dummy cycle. */

    if (SPIM_InitFlash(1) != 0)        /* Initialized SPI flash */
    {
        printf("SPIM flash initialize failed!\n");
    }

    SPIM_ReadJedecId(idBuf, sizeof(idBuf), 1);
    printf("SPIM get JEDEC ID=0x%02X, 0x%02X, 0x%02X\n", idBuf[0], idBuf[1], idBuf[2]);

    SPIM_Enable_4Bytes_Mode(USE_4_BYTES_MODE, 1);
}

void WINBOND25X16A_EraseSector(uint32_t u32StartSector)
{
    uint32_t u32StartAddr;
    u32StartAddr = u32StartSector & 0x0FFFF000;
    SPIM_EraseBlock(u32StartAddr, USE_4_BYTES_MODE, OPCODE_SE_4K, 1, 1);
}

int32_t WINBOND25X16A_ProgramPage(uint32_t u32StartPage, uint8_t *pu8Data)
{
    uint32_t u32StartAddr;
    u32StartAddr = u32StartPage & 0x0FFFFF00;
    SPIM_IO_Write(u32StartAddr, USE_4_BYTES_MODE, 256, pu8Data, OPCODE_PP, 1, 1, 1);
    return 0;
}


int32_t WINBOND25X16A_ReadPage(
    uint8_t u8ReadMode,
    uint32_t u32StartPage,
    uint8_t *pu8Data
)
{
    uint32_t u32StartAddr;
    u32StartAddr = u32StartPage & 0x0FFFFF00;
    SPIM_IO_Read(u32StartAddr, USE_4_BYTES_MODE, 256, pu8Data, OPCODE_FAST_READ, 1, 1, 1, 1);
    return 0;

}
void SpiRead(uint32_t addr, uint32_t size, uint32_t buffer)
{
    /* This is low level read function of USB Mass Storage */
    int32_t len;

    len = (int32_t)size;

    while (len >= DRVSPIFLASH_PAGE_SIZE)
    {
        WINBOND25X16A_ReadPage(0, addr, (uint8_t *)buffer);
        addr   += DRVSPIFLASH_PAGE_SIZE;
        buffer += DRVSPIFLASH_PAGE_SIZE;
        len    -= DRVSPIFLASH_PAGE_SIZE;
    }
}
void SpiWrite(uint32_t addr, uint32_t size, uint32_t buffer)
{
    /* This is low level write function of USB Mass Storage */
    int32_t len, i, offset;
    uint32_t *pu32;
    uint32_t alignAddr;

    len = (int32_t)size;

    if (len == DRVSPIFLASH_SECTOR_SIZE && ((addr & (DRVSPIFLASH_SECTOR_SIZE - 1)) == 0))
    {
        /* one-sector length & the start address is sector-alignment */
        WINBOND25X16A_EraseSector(addr);

        //WINBOND25X16A_EnableWrite(g_SpiPort, g_SlaveSel, TRUE);

        while (len >= DRVSPIFLASH_PAGE_SIZE)
        {
            WINBOND25X16A_ProgramPage(addr, (uint8_t *)buffer);
            len    -= DRVSPIFLASH_PAGE_SIZE;
            buffer += DRVSPIFLASH_PAGE_SIZE;
            addr   += DRVSPIFLASH_PAGE_SIZE;
        }
    }
    else
    {
        do
        {
            /* alignAddr: sector address */
            alignAddr = addr & 0x1FFFF000;

            /* Get the sector offset*/
            offset = (addr & (DRVSPIFLASH_SECTOR_SIZE - 1));

            if (offset || (size < DRVSPIFLASH_SECTOR_SIZE))
            {
                /* if the start address is not sector-alignment or the size is less than one sector, */
                /* read back the data of the destination sector to g_sectorBuf[].                    */
                SpiRead(alignAddr, DRVSPIFLASH_SECTOR_SIZE, (uint32_t)&g_sectorBuf[0]);
            }

            /* Update the data */
            pu32 = (uint32_t *)buffer;
            len = DRVSPIFLASH_SECTOR_SIZE - offset; /* len: the byte count between the start address and the end of a sector. */

            if (size < len) /* check if the range of data arrive at the end of a sector. */
                len = size; /* Not arrive at the end of a sector. "len" indicate the actual byte count of data. */

            for (i = 0; i < len / 4; i++)
            {
                g_sectorBuf[offset / 4 + i] = pu32[i];
            }

            WINBOND25X16A_EraseSector(alignAddr);


            //WINBOND25X16A_EnableWrite(g_SpiPort, g_SlaveSel, TRUE);

            for (i = 0; i < 16; i++) /* one sector (16 pages) */
            {
                WINBOND25X16A_ProgramPage(alignAddr + (i << 8), (uint8_t *)g_sectorBuf + (i << 8));
            }

            size -= len;
            addr += len;
            buffer += len;

        } while (size > 0);
    }
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
