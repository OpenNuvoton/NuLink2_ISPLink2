#include "NuMicro.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ff.h"
#include "diskio.h"
#define dbg_printf printf
//#define dbg_printf(...)
#define DEFINE_FILE "0:\\DEFINE.txt"
FATFS Fs[1];        /* File system object for logical drive */
const char AP_CMD[] = "APROM=1\r\n";
const char DATAFLASH_CMD[] = "DATAFLASH=1\r\n";
const char SN_CMD[] = "SN=1\r\n";
const char WCONFIG_CMD[] = "W_CONFIG=1\r\n";
const char IF0_CMD[] = "UART_BUS\r\n";
const char IF1_CMD[] = "RS485_BUS\r\n";
const char IF2_CMD[] = "CAN_BUS\r\n";
const char IF3_CMD[] = "SPI_BUS\r\n";
const char IF4_CMD[] = "I2C_BUS\r\n";
const char IF5_CMD[] = "UART_1_WIRE_BUS\r\n";
const char IF6_CMD[] = "RS485_A_BUS\r\n";
const char AUTO_DETECT_CMD[] = "AUTO_DETECT=1\r\n";
volatile unsigned char APROM_NAME[128];
volatile unsigned char DATAFLASH_NAME[128];
volatile unsigned char SN_BUFF[128];
volatile unsigned int SN_value;

volatile unsigned int AUTO_DETECT_VALUE;
volatile unsigned int w_Config;
volatile unsigned int w_Config0;
volatile unsigned int w_Config1;
typedef enum
{
    UART_BUS,
    RS485_BUS,
    CAN_BUS,
    SPI_BUS,
    I2C_BUS,
    UART_1_WIRE_BUS,
    RS485_Address_BUS,
} DEVICE_ENUM;
volatile DEVICE_ENUM DEVICE_TYPE;
volatile unsigned int link_state;
volatile unsigned int AP_file_totallen;
volatile unsigned int AP_file_checksum;
volatile unsigned int DATAFLASH_file_totallen;
volatile unsigned int DATAFLASH_file_checksum;
volatile unsigned int user_sn;
#define Check_APROM_Enable 2
#define Read_APROM_BIN_FILE 3
#define Check_DataFlash_Enable 4
#define Read_DataFlash_BIN_FILE 5
#define Check_SN_Enable 6
#define SN 7
#define Read_Interface 8
#define AUTO_DETECT 9
#define WRITE_CONFIG 10
#define CONFIG0_SET 11
#define CONFIG1_SET 12

unsigned int MOUNT_FILE_SYSTEM(void)
{
    FRESULT res;
    res = f_mount(&Fs[0], "0:", 1);
    return res;
}

void UNMOUNT_FILE_SYSTEM(void)
{

    f_mount(0, "0:", 0);

}

void put_rc(FRESULT rc)
{
    const TCHAR *p =
        _T("OK\0DISK_ERR\0INT_ERR\0NOT_READY\0NO_FILE\0NO_PATH\0INVALID_NAME\0")
        _T("DENIED\0EXIST\0INVALID_OBJECT\0WRITE_PROTECTED\0INVALID_DRIVE\0")
        _T("NOT_ENABLED\0NO_FILE_SYSTEM\0MKFS_ABORTED\0TIMEOUT\0LOCKED\0")
        _T("NOT_ENOUGH_CORE\0TOO_MANY_OPEN_FILES\0");

    uint32_t i;

    for (i = 0; (i != (UINT)rc) && *p; i++)
    {
        while (*p++) ;
    }

    dbg_printf(_T("rc=%u FR_%s\n"), (UINT)rc, p);
}


unsigned int GET_FILE_SIZE(void *buf)
{
    FIL file1;
    unsigned int file_size = 0;
    uint8_t *TempChar;

    FRESULT res;
    TempChar = (uint8_t *)buf;
    res = f_open(&file1, (const char *)TempChar, FA_OPEN_EXISTING | FA_READ);

    if (res == FR_OK)
    {
        file_size = file1.fsize;
    }

    f_close(&file1);
    return file_size;
}

unsigned int GET_FILE_CHECKSUM(void *buf)
{
    FIL file1;
    uint8_t *TempChar;
    FRESULT res;
    unsigned int binfile_checksum, s1;
    unsigned int temp_ct;
    unsigned char Buff[512];
    binfile_checksum = 0;
    TempChar = (uint8_t *)buf;
    res = f_open(&file1, (const char *)TempChar, FA_OPEN_EXISTING | FA_READ);

    if (res == FR_OK)
    {
        for (;;)
        {
            res = f_read(&file1, Buff, sizeof(Buff), &s1);

            if (res || s1 == 0) break;

            for (temp_ct = 0; temp_ct < s1; temp_ct++)
            {
                binfile_checksum = binfile_checksum + Buff[temp_ct];
            }
        }


        f_close(&file1);
    }

    return binfile_checksum;
}

unsigned int  f_ReadLine(int LineIDX, void *buf)
{
    FIL file1;
    FRESULT res;
    int CurrentLine = 0;
    uint8_t *TempChar;
    TempChar = (uint8_t *)buf;
    res = f_open(&file1, DEFINE_FILE, FA_OPEN_EXISTING | FA_READ);

    if (res == FR_OK)
    {
        while ((f_eof(&file1) == 0))
        {
            f_gets((char *)TempChar, file1.fsize, &file1);
            CurrentLine++;

            if (CurrentLine == LineIDX)
            {
                break;
            }
        }

        f_close(&file1);
        return 0;
    }
    else
        return 1;


}
unsigned int Read_ARPOM_INFORMATION(void)
{

    unsigned char Buff[256];
    int str_result;
    memset(Buff, '\0', sizeof(Buff));

    if (f_ReadLine(Check_APROM_Enable, Buff) != 0)
        return 1;

    str_result = strcmp((const char *)Buff, AP_CMD);

    if (str_result == 0)
    {
        memset((void *)APROM_NAME, '\0', sizeof(APROM_NAME));
        f_ReadLine(Read_APROM_BIN_FILE, (void *)APROM_NAME);
        printf("%s\n\r", (unsigned char *)APROM_NAME);
        AP_file_totallen = GET_FILE_SIZE((void *)APROM_NAME);
        memset(Buff, '\0', sizeof(Buff));
        sprintf((char *)Buff, "APROM File Size :%d Byte", AP_file_totallen);
        printf("%s\n\r", Buff);

        memset(Buff, '\0', sizeof(Buff));
        AP_file_checksum = GET_FILE_CHECKSUM((void *)APROM_NAME);
        sprintf((char *)Buff, "APROM File Checksum: 0x%x", AP_file_checksum);
        printf("%s\n\r", Buff);
        return 0; //file can access
    }
    else
    {
        return 2; //file can not access
    }

}



void Read_INTERFACE(void)
{
    unsigned char Buff[256];
    int str_result;
    memset(Buff, '\0', sizeof(Buff));
    f_ReadLine(Read_Interface, Buff);
    str_result = strcmp((const char *)Buff, IF0_CMD);

    if (str_result == 0)
    {
        DEVICE_TYPE = UART_BUS;
        printf("UART Interface \n\r");
    }

    str_result = strcmp((const char *)Buff, IF1_CMD);

    if (str_result == 0)
    {

        DEVICE_TYPE = RS485_BUS;

        printf("RS485 Interface \n\r");
    }

    str_result = strcmp((const char *)Buff, IF6_CMD);

    //rs485 NMM mode
    if (str_result == 0)
    {

        DEVICE_TYPE = RS485_Address_BUS;


        printf("RS485 Interface \n\r");
    }

    str_result = strcmp((const char *)Buff, IF2_CMD);

    if (str_result == 0)
    {
        DEVICE_TYPE = CAN_BUS;

        printf("CAN Interface \n\r");
    }

    str_result = strcmp((const char *)Buff, IF3_CMD);

    if (str_result == 0)
    {
        DEVICE_TYPE = SPI_BUS;


        printf("SPI Interface \n\r");
    }

    str_result = strcmp((const char *)Buff, IF4_CMD);

    if (str_result == 0)
    {
        DEVICE_TYPE = I2C_BUS;
        printf("I2C Interface\n\r");
    }

    str_result = strcmp((const char *)Buff, IF5_CMD);

    if (str_result == 0)
    {
        DEVICE_TYPE = UART_1_WIRE_BUS;
        printf("UART 1 WIRE Interface\n\r");
    }

    //CHECK SUPPORT UART AUTO DETECT
    memset(Buff, '\0', sizeof(Buff));
    f_ReadLine(AUTO_DETECT, Buff);
    str_result = strcmp((const char *)Buff, AUTO_DETECT_CMD);
    AUTO_DETECT_VALUE = 0;

    if (str_result == 0)
    {
        AUTO_DETECT_VALUE = 1; //SUPPORT AUTO DETECT
        printf(" AUTO DETECT ENABLE\n\r");
    }
}
void Read_DATAFLASH_INFORMATION(void)
{
    unsigned char Buff[256];
    int str_result;
    memset(Buff, '\0', sizeof(Buff));
    f_ReadLine(Check_DataFlash_Enable, Buff);
    str_result = strcmp((const char *)Buff, DATAFLASH_CMD);

    if (str_result == 0)
    {

        memset((void *)DATAFLASH_NAME, '\0', sizeof(DATAFLASH_NAME));
        f_ReadLine(Read_DataFlash_BIN_FILE, (void *)DATAFLASH_NAME);

        printf("%s\n\r", DATAFLASH_NAME);
        DATAFLASH_file_totallen = GET_FILE_SIZE((void *)DATAFLASH_NAME);
        memset(Buff, '\0', sizeof(Buff));
        sprintf((char *)Buff, "DATAFALSH File Size :%d Byte", DATAFLASH_file_totallen);
        printf("%s\n\r", Buff);

        memset(Buff, '\0', sizeof(Buff));
        DATAFLASH_file_checksum = GET_FILE_CHECKSUM((void *)DATAFLASH_NAME);
        sprintf((char *)Buff, "DATAFLASH File Checksum: 0x%x", DATAFLASH_file_checksum);
        printf("%s\n\r", Buff);
    }
}

void Read_SN_INFORMATION(void)
{
    unsigned char Buff[256];
    int str_result;
    char *pEnd;
    memset(Buff, '\0', sizeof(Buff));
    f_ReadLine(Check_SN_Enable, Buff);
    str_result = strcmp((const char *)Buff, SN_CMD);

    if (str_result == 0)
    {
        f_ReadLine(SN, (void *)SN_BUFF);
        SN_value = strtoul((const char *)SN_BUFF, &pEnd, 10);
        sprintf((char *)Buff, "User_SN: %d", SN_value);
        printf("%s\n\r", Buff);
    }
}

void Read_CONFIG_SDCARD(void)
{
    unsigned char Buff[256];
    int str_result;
    char *pEnd;
    memset(Buff, '\0', sizeof(Buff));
    f_ReadLine(WRITE_CONFIG, Buff);
    str_result = strcmp((const char *)Buff, WCONFIG_CMD);

    if (str_result == 0)
    {
        w_Config = 1;
        f_ReadLine(CONFIG0_SET, (void *)SN_BUFF);
        w_Config0 = strtoul((const char *)SN_BUFF, NULL, 0);
        sprintf((char *)Buff, "CONFIG0: 0x%x", w_Config0);
        printf("%s\n\r", Buff);

        f_ReadLine(CONFIG1_SET, (void *)SN_BUFF);
        w_Config1 = strtoul((const char *)SN_BUFF, NULL, 0);
        sprintf((char *)Buff, "CONFIG1: 0x%x", w_Config1);
        printf("%s\n\r", Buff);
    }
}


