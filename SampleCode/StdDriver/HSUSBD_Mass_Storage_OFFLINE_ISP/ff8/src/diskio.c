/*-----------------------------------------------------------------------*/
/* Low level disk control module for Win32              (C)ChaN, 2007    */
/*-----------------------------------------------------------------------*/


#include <stdio.h>

#include <string.h>
#include <stdint.h>
#include "diskio.h"
#include "NuMicro.h"
#include "usbh_lib.h"
extern void SpiRead(uint32_t addr, uint32_t size, uint32_t buffer);	
extern void SpiWrite(uint32_t addr, uint32_t size, uint32_t buffer);
#define Sector_Size 128 //512byte
uint32_t Tmp_Buffer[Sector_Size];

#define SPI_FLASH_DRIVE      0        // for SPI FLASH
#define SDH1_DRIVE      1        // for SD0 
#define USB_DISK      2        // for USB_DISK 
void RoughDelay(unsigned int t)
{
    volatile int delay;

    delay = t;

    while(delay-- >= 0);
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv		/* Physical drive nmuber */
)
{
		DSTATUS sta;
	if (drv == SPI_FLASH_DRIVE)
    {	
	   sta = RES_OK;
		}
		else if (drv == SDH1_DRIVE)
    {
        if (SDH_GET_CARD_CAPACITY(SDH1) == 0)
            return STA_NOINIT;
				sta = RES_OK;
    }
		else if (drv == USB_DISK)
		{
		    usbh_pooling_hubs();
    if (usbh_umas_disk_status(drv) == UMAS_ERR_NO_DEVICE)
        return STA_NODISK;
		sta = RES_OK;
		}
		
	return sta;
	

}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0) */
)
{   
	DSTATUS sta=STA_OK;
	 if  (drv == SPI_FLASH_DRIVE)
	 {
	  sta = RES_OK;
	 }
	 else if (drv == SDH1_DRIVE)
    {
        if (SDH_GET_CARD_CAPACITY(SDH1) == 0)
            return STA_NOINIT;				
				sta = RES_OK;
    }
		else if (drv == USB_DISK)
		{
		usbh_pooling_hubs();
    if (usbh_umas_disk_status(drv) == UMAS_ERR_NO_DEVICE)
        return STA_NODISK;
		sta = RES_OK;
		}
		
	return sta;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
extern unsigned char  SD_Type;

DRESULT disk_read (
	BYTE drv,			/* Physical drive nmuber (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	DRESULT res;
	  uint32_t size,address;

    uint32_t shift_buf_flag = 0;
    uint32_t tmp_StartBufAddr;
	if (drv==SPI_FLASH_DRIVE) {

	    address= sector*512;
	    size=count*512;
		  SpiRead(address, size, (uint32_t)buff);			

		res =RES_OK;	/* Clear STA_NOINIT */;
	}
	
	else if (drv==SDH1_DRIVE) {
	if(shift_buf_flag == 1)
        {
            if(count == 1)
            {
                res = (DRESULT) SDH_Read(SDH1, (uint8_t*)(&Tmp_Buffer), sector, count);
                memcpy(buff, (&Tmp_Buffer), count*SD1.sectorSize);
            }
            else
            {
                tmp_StartBufAddr = (((uint32_t)buff/4 + 1) * 4);
                res = (DRESULT) SDH_Read(SDH1, ((uint8_t*)tmp_StartBufAddr), sector, (count -1));
                memcpy(buff, (void*)tmp_StartBufAddr, (SD1.sectorSize*(count-1)) );
                res = (DRESULT) SDH_Read(SDH1, (uint8_t*)(&Tmp_Buffer), (sector+count-1), 1);
                memcpy( (buff+(SD1.sectorSize*(count-1))), (void*)Tmp_Buffer, SD1.sectorSize);
            }
        }
        else
            res = (DRESULT) SDH_Read(SDH1, buff, sector, count);		
	}
	
    else if (drv == USB_DISK)
		{
			 int       ret;
		ret = usbh_umas_read(drv, sector, count, buff);
    if (ret != UMAS_OK)
    {
        usbh_umas_reset_disk(drv);
        ret = usbh_umas_read(drv, sector, count, buff);
    }

    if (ret == UMAS_OK)
        return RES_OK;

    if (ret == UMAS_ERR_NO_DEVICE)
        return RES_NOTRDY;

    if (ret == UMAS_ERR_IO)
        return RES_ERROR;
		
		}	
	
	return res;
}

/*---------------------------------------------------------*/
/* User Provided RTC Function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support an RTC.                     */
/* This function is not required in read-only cfg.         */
unsigned long get_fattime (void)
{
	unsigned long tmr;

    tmr=0x00000;

	return tmr;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{

	DRESULT  res;	
      uint32_t size;
	uint32_t address;
    uint32_t shift_buf_flag = 0;
	    uint32_t tmp_StartBufAddr;
	   uint32_t volatile i;
 	if (drv==SPI_FLASH_DRIVE) {

	    size=count*512;

		address= sector*512;
		SpiWrite(address, size,(uint32_t)buff);
	    res = RES_OK;
}
	else if (drv==SDH1_DRIVE) {
		if(shift_buf_flag == 1)
        {
            if(count == 1)
            {
                memcpy((&Tmp_Buffer), buff, count*SD1.sectorSize);
                res = (DRESULT) SDH_Write(SDH1, (uint8_t*)(&Tmp_Buffer), sector, count);
            }
            else
            {
                tmp_StartBufAddr = (((uint32_t)buff/4 + 1) * 4);
                memcpy((void*)Tmp_Buffer, (buff+(SD1.sectorSize*(count-1))), SD1.sectorSize);

                for(i = (SD1.sectorSize*(count-1)); i > 0; i--)
                {
                    memcpy((void *)(tmp_StartBufAddr + i - 1), (buff + i -1), 1);
                }

                res = (DRESULT) SDH_Write(SDH1, ((uint8_t*)tmp_StartBufAddr), sector, (count -1));
                res = (DRESULT) SDH_Write(SDH1, (uint8_t*)(&Tmp_Buffer), (sector+count-1), 1);
            }
        }
        else
            res = (DRESULT) SDH_Write(SDH1, (uint8_t *)buff, sector, count);
		
		
	}
	else if (drv == USB_DISK)
		{
		 int       ret;
		ret = usbh_umas_write(drv, sector, count, (uint8_t *)buff);
    if (ret != UMAS_OK)
    {
        usbh_umas_reset_disk(drv);
        ret = usbh_umas_write(drv, sector, count, (uint8_t *)buff);
    }

    if (ret == UMAS_OK)
        return RES_OK;

    if (ret == UMAS_ERR_NO_DEVICE)
        return RES_NOTRDY;

    if (ret == UMAS_ERR_IO)
        return RES_ERROR;
		res=RES_PARERR;
		}
	
	return res;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
	DRESULT res;
	if (drv==SPI_FLASH_DRIVE)
	{
	switch (ctrl) {
	case CTRL_SYNC :		/* Make sure that no pending write process */
		res = RES_OK;
		break;

	case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
		//DrvSDCARD_GetCardSize(buff);
	 *(DWORD*)buff = (8*1024*1024)/512;
		res = RES_OK;
		break;

	case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
		*(DWORD*)buff = 512;	//512;
		res = RES_OK;
		break;

	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
		*(DWORD*)buff = 1;
		res = RES_OK;
		break;


	default:
		res = RES_PARERR;
	}
}
	else if(drv==SDH1_DRIVE)
{
 switch(ctrl)
    {
    case CTRL_SYNC:
			  res = RES_OK;
        break;
    case GET_SECTOR_COUNT:
        *(DWORD*)buff = SD0.totalSectorN;
				res = RES_OK;
        break;
    case GET_SECTOR_SIZE:
        *(WORD*)buff = SD0.sectorSize;
				res = RES_OK;
        break;
    default:
        res = RES_PARERR;
        break;
    }


}
else if (drv == USB_DISK)
		{
		int  ret;

    ret = usbh_umas_ioctl(drv, ctrl, buff);

    if (ret == UMAS_OK)
        return RES_OK;

    if (ret == UMAS_ERR_IVALID_PARM)
        return RES_PARERR;

    if (ret == UMAS_ERR_NO_DEVICE)
        return RES_NOTRDY;
		res=RES_PARERR;
		}
	


	return res;
}




