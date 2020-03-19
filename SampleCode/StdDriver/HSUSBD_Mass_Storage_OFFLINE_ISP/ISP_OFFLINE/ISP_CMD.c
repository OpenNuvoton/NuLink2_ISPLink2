#include "stdio.h"
#include "string.h"
#include "NuMicro.h"
#include "ISP_DRIVER.H"
#include "ISP_CMD.h"
#include "ff.h"
#define PACKET_SIZE 64
__align(4) uint8_t rcvbuf[PACKET_SIZE];
__align(4) uint8_t sendbuf[PACKET_SIZE];
__align(4) uint8_t file_buffer[512];
extern volatile unsigned char APROM_NAME[128];
unsigned int g_packno = 1;
unsigned short gcksum;
extern  unsigned int AP_file_totallen;
extern  unsigned int AP_file_checksum;
struct sISP_COMMAND ISP_COMMAND;
#define BOOL  uint8_t
#define PAGE_SIZE                      0x00000200     /* Page size */
#define dbg_printf printf
//#define dbg_printf(...)
extern io_handle_t  DEV_handle;
void WordsCpy(void *dest, void *src, int32_t size)
{
    uint8_t *pu8Src, *pu8Dest;
    int32_t i;

    pu8Dest = (uint8_t *)dest;
    pu8Src = (uint8_t *)src;

    for (i = 0; i < size; i++)
        pu8Dest[i] = pu8Src[i];
}

uint16_t Checksum(unsigned char *buf, int len)
{
    int i;
    uint16_t c;

    for (c = 0, i = 0; i < len; i++)
    {
        c += buf[i];
    }

    return (c);
}
#define BUSY PB9
#define PASS PB8

void SendData(void)
{
    BUSY = 1;
    uint32_t len = 64;
    gcksum = Checksum(sendbuf, PACKET_SIZE);

    io_write(DEV_handle, sendbuf, &len);
}

ErrNo RcvData(void)
{
    BUSY = 0;
    ErrNo Result;
    unsigned short lcksum;
    uint8_t *pBuf;
    uint32_t len = 64;
    Result = io_read(DEV_handle, rcvbuf, &len);

    if (Result != ENOERR)
    {
        dbg_printf("t0\n\r");
        return Result;
    }

    pBuf = rcvbuf;
    WordsCpy(&lcksum, pBuf, 2);
    pBuf += 4;
    // dbg_printf("gcksum=0x%x lcksum=0x%x\n", gcksum, lcksum);
#if 1

    if (inpw(pBuf) != g_packno)
    {
        dbg_printf("g_packno=%d rcv %d\n", g_packno, inpw(pBuf));
        BUSY = 0;
        PASS = 1;
        Result = EPACKAGENO;
    }
    else
    {
        if (lcksum != gcksum)
        {
            dbg_printf("gcksum=%x lcksum=%x\n", gcksum, lcksum);
            BUSY = 0;
            PASS = 1;
            Result = ECHECKSUM;
        }



    }

#endif
    g_packno++;
    return Result;
}


ErrNo CmdSyncPackno(void)
{
    ErrNo Result;
    unsigned long cmdData;
    g_packno = 1;
    //sync send&recv packno
    memset(sendbuf, 0, PACKET_SIZE);
    memset(sendbuf, 0, PACKET_SIZE);
    cmdData = CMD_SYNC_PACKNO;//CMD_UPDATE_APROM
    WordsCpy(sendbuf + 0, &cmdData, 4);
    WordsCpy(sendbuf + 4, &g_packno, 4);
    WordsCpy(sendbuf + 8, &g_packno, 4);
    g_packno++;

    SendData();
    devtab_entry_t *t = (devtab_entry_t *)DEV_handle;

    if (str_compare(SPI_NAME_STRING, t->name))
    {
        CLK_SysTickDelay(50000);
    }

    Result = RcvData();

    return Result;
}

void CmdRunLDROM(void)
{
    int i;
    unsigned long cmdData;
    //sync send&recv packno
    memset(sendbuf, 0, PACKET_SIZE);
    memset(sendbuf, 0, PACKET_SIZE);
    cmdData = CMD_RUN_LDROM;//CMD_UPDATE_APROM
    WordsCpy(sendbuf + 0, &cmdData, 4);
    WordsCpy(sendbuf + 4, &g_packno, 4);
    g_packno = 0;

    SendData();

    //delay for reset
    for (i = 0; i < 200; i++)
        CLK_SysTickDelay(1000);
}

void CmdRunAPROM(void)
{
    int i;
    unsigned long cmdData;
    //sync send&recv packno
    memset(sendbuf, 0, PACKET_SIZE);
    memset(sendbuf, 0, PACKET_SIZE);
    cmdData = CMD_RUN_APROM;//CMD_UPDATE_APROM
    WordsCpy(sendbuf + 0, &cmdData, 4);
    WordsCpy(sendbuf + 4, &g_packno, 4);
    g_packno = 0;

    SendData();

    //delay for reset
    for (i = 0; i < 200; i++)
        CLK_SysTickDelay(1000);
}

ErrNo CmdFWVersion(unsigned int *fwver)
{
    ErrNo Result;
    unsigned long cmdData;
    unsigned int lfwver;

    //sync send&recv packno
    memset(sendbuf, 0, PACKET_SIZE);
    cmdData = CMD_GET_FWVER;
    WordsCpy(sendbuf + 0, &cmdData, 4);
    WordsCpy(sendbuf + 4, &g_packno, 4);
    g_packno++;

    SendData();
    devtab_entry_t *t = (devtab_entry_t *)DEV_handle;

    if (str_compare(SPI_NAME_STRING, t->name))
    {
        CLK_SysTickDelay(50000);
    }

    Result = RcvData();

    if (Result == 0)
    {
        WordsCpy(&lfwver, rcvbuf + 8, 4);
        *fwver = lfwver;
    }

    return Result;
}



ErrNo CmdGetDeviceID(unsigned int *devid)
{
    ErrNo Result;
    unsigned long cmdData;
    unsigned int ldevid;

    //sync send&recv packno
    memset(sendbuf, 0, PACKET_SIZE);
    cmdData = CMD_GET_DEVICEID;
    WordsCpy(sendbuf + 0, &cmdData, 4);
    WordsCpy(sendbuf + 4, &g_packno, 4);
    g_packno++;

    SendData();
    devtab_entry_t *t = (devtab_entry_t *)DEV_handle;

    if (str_compare(SPI_NAME_STRING, t->name))
    {
        CLK_SysTickDelay(50000);
    }

    Result = RcvData();

    if (Result == 0)
    {
        WordsCpy(&ldevid, rcvbuf + 8, 4);
        *devid = ldevid;
    }

    return Result;
}


ErrNo CmdGetFlashMode(unsigned int *mode)
{
    ErrNo Result;
    unsigned long cmdData;
    unsigned int ldevid;

    //sync send&recv packno
    memset(sendbuf, 0, PACKET_SIZE);
    cmdData = CMD_GET_FLASHMODE;
    WordsCpy(sendbuf + 0, &cmdData, 4);
    WordsCpy(sendbuf + 4, &g_packno, 4);
    g_packno++;

    SendData();
    devtab_entry_t *t = (devtab_entry_t *)DEV_handle;

    if (str_compare(SPI_NAME_STRING, t->name))
    {
        CLK_SysTickDelay(50000);
    }

    Result = RcvData();

    if (Result == 0)
    {
        WordsCpy(&ldevid, rcvbuf + 8, 4);
        *mode = ldevid;
    }

    return Result;
}

ErrNo CmdGetConfig(unsigned int *config)
{
    ErrNo Result;
    unsigned long cmdData;
    unsigned int lconfig[2];

    //sync send&recv packno
    memset(sendbuf, 0, PACKET_SIZE);
    cmdData = CMD_READ_CONFIG;
    WordsCpy(sendbuf + 0, &cmdData, 4);
    WordsCpy(sendbuf + 4, &g_packno, 4);
    g_packno++;

    SendData();
    devtab_entry_t *t = (devtab_entry_t *)DEV_handle;

    if (str_compare(SPI_NAME_STRING, t->name))
    {
        CLK_SysTickDelay(50000);
    }

    Result = RcvData();

    if (Result == 0)
    {
        WordsCpy(&lconfig[0], rcvbuf + 8, 4);
        WordsCpy(&lconfig[1], rcvbuf + 12, 4);
        config[0] = lconfig[0];
        config[1] = lconfig[1];
    }

    return Result;
}

extern volatile unsigned int AUTO_DETECT_VALUE;

void auto_detect_command(void)
{
    unsigned long cmdData;
    memset(sendbuf, 0, PACKET_SIZE);

    devtab_entry_t *t = (devtab_entry_t *)DEV_handle;

    if (str_compare(CAN_NAME_STRING, t->name))
    {
        cmdData = CMD_GET_DEVICEID;

        g_packno = 1;
        WordsCpy(sendbuf + 0, &cmdData, 4);
        WordsCpy(sendbuf + 4, &g_packno, 4);

        while (1)
        {
            SendData();


            RcvData();

            if (sendbuf[4] + 1 == rcvbuf[4])
            {

                break;

            }
        }
    }
    else
    {
        cmdData = CMD_UPDATE_CONNECT;

        g_packno = 1;
        WordsCpy(sendbuf + 0, &cmdData, 4);
        WordsCpy(sendbuf + 4, &g_packno, 4);

        while (1)
        {
            SendData();




            CLK_SysTickDelay(50000);

            RcvData();

            if (sendbuf[4] + 1 == rcvbuf[4] && rcvbuf[0] == 0xaf)
            {

                break;
            }
        }
    }

    AUTO_DETECT_VALUE = 0;
}


ErrNo CmdUpdateConfig(uint32_t *conf)
{
    ErrNo Result;
    unsigned long cmdData;

    //sync send&recv packno
    memset(sendbuf, 0, PACKET_SIZE);
    cmdData = CMD_UPDATE_CONFIG;
    WordsCpy(sendbuf + 0, &cmdData, 4);
    WordsCpy(sendbuf + 4, &g_packno, 4);
    WordsCpy(sendbuf + 8, conf, 8);
    g_packno++;

    SendData();
    devtab_entry_t *t = (devtab_entry_t *)DEV_handle;

    if (str_compare(SPI_NAME_STRING, t->name))
    {
        CLK_SysTickDelay(50000);
    }

    Result = RcvData();

    return Result;
}

ErrNo UpdatedTargetFalsh(uint32_t in_startaddr, uint32_t in_file_totallen)
{
    FIL file1;
    ErrNo Result;
    unsigned int s1;
    unsigned short get_cksum;
    unsigned long cmdData;
    unsigned long startaddr = in_startaddr;
    unsigned long AP_file_totallen = in_file_totallen;
    f_open(&file1, (const char *)APROM_NAME, FA_OPEN_EXISTING | FA_READ);
    memset(sendbuf, 0, PACKET_SIZE);
    cmdData = CMD_UPDATE_APROM;//CMD_UPDATE_APROM
    WordsCpy(sendbuf + 0, &cmdData, 4);
    WordsCpy(sendbuf + 4, &g_packno, 4);
    g_packno++;
    //start address
    startaddr = 0;
    WordsCpy(sendbuf + 8, &startaddr, 4);
    WordsCpy(sendbuf + 12, (void *)&AP_file_totallen, 4);

    f_read(&file1, file_buffer, 48, &s1);
    WordsCpy(sendbuf + 16, file_buffer, 48);
    printf("Erase Starting!\n\r");
    SendData();
    devtab_entry_t *t = (devtab_entry_t *)DEV_handle;

    if (str_compare(SPI_NAME_STRING, t->name))
    {
        //delay for erase
        for (unsigned int i = 0; i < 2000; i++)
            CLK_SysTickDelay(2000);

    }

    //can is different command
#if 0

    if (str_compare(CAN_NAME_STRING, t->name))
    {
        //delay for erase
        for (unsigned int i = 0; i < 5000; i++)
            CLK_SysTickDelay(2000);
    }

#endif

    if (str_compare(I2C_NAME_STRING, t->name))
    {
        //delay for erase
        for (unsigned int i = 0; i < 5000; i++)
            CLK_SysTickDelay(2000);
    }



    Result = RcvData();

    if (Result != ENOERR)
        return Result;

    printf("Erase done!\n\r");

    for (unsigned int i = 48; i < AP_file_totallen; i = i + 56)
    {
        //reaf file cotext to file buffer
        if (((i - 48) % 448) == 0)
            f_read(&file1, file_buffer, 448, &s1);

        printf("Programm: %.0f %%\r", (float)((float)((float)i /(float) AP_file_totallen) * (float)100.0));

        //clear buffer
        memset(sendbuf, 0, PACKET_SIZE);
        //set package number to buffer
        WordsCpy(sendbuf + 4, &g_packno, 4);
        g_packno++;

        if ((AP_file_totallen - i) > 56)
        {

            WordsCpy(sendbuf + 8, &file_buffer[((i - 48) % 448)], 56);

            SendData();

            if (str_compare(CAN_NAME_STRING, t->name))
            {
                //delay for erase
                //for(unsigned int i=0;i<5000;i++)
                CLK_SysTickDelay(50000);
            }

            if (str_compare(I2C_NAME_STRING, t->name))
            {
                //delay for erase
                //for(unsigned int i=0;i<5000;i++)
                CLK_SysTickDelay(20000);
            }

            if (str_compare(SPI_NAME_STRING, t->name))
            {
                //delay for erase
                //for(unsigned int i=0;i<5000;i++)
                CLK_SysTickDelay(50000);
            }

            Result = RcvData();

            if (Result != ENOERR)
            {
                BUSY = 0;
                PASS = 1;
                return Result;
            }
        }
        else
        {


            WordsCpy(sendbuf + 8, &file_buffer[((i - 48) % 448)], AP_file_totallen - i);
            //read target chip checksum
            SendData();

            if (str_compare(CAN_NAME_STRING, t->name))
            {
                //delay for erase
                //for(unsigned int i=0;i<5000;i++)
                CLK_SysTickDelay(50000);
            }

            if (str_compare(SPI_NAME_STRING, t->name))
            {
                //delay for erase
                //for(unsigned int i=0;i<5000;i++)
                CLK_SysTickDelay(50000);
            }

            if (str_compare(I2C_NAME_STRING, t->name))
            {
                //delay for erase
                //for(unsigned int i=0;i<5000;i++)
                CLK_SysTickDelay(50000);
            }

            Result = RcvData();

            if (Result != ENOERR)
            {
                return Result;
            }

            if (str_compare(CAN_NAME_STRING, t->name) == 0)
            {
                WordsCpy(&get_cksum, rcvbuf + 8, 2);

                if ((AP_file_checksum & 0xffff) != get_cksum)
                {
                    BUSY = 0;
                    PASS = 1;
                    return EFILECHECKSUM;
                }
            }

            printf("program finish\n\r");
        }
    }

    f_close(&file1);
    BUSY = 0;
    PASS = 0;
    return ENOERR;



}




void init_ISP_command(void)
{
    ISP_COMMAND.ISPSyncPackno = CmdSyncPackno;
    ISP_COMMAND.ISPFWVersion = CmdFWVersion;
    ISP_COMMAND.ISPGetDeviceID = CmdGetDeviceID;
    ISP_COMMAND.ISPGetFlashMode = CmdGetFlashMode;
    ISP_COMMAND.ISPGetConfig = CmdGetConfig;
    ISP_COMMAND.ISPUpdateConfig = CmdUpdateConfig;
    ISP_COMMAND.ISPCmdRunLDROM = CmdRunLDROM;
    ISP_COMMAND.ISPCmdRunAPROM = CmdRunAPROM;
    ISP_COMMAND.ISPauto_detect_command = auto_detect_command;
    ISP_COMMAND.ISPUpdateFlash = UpdatedTargetFalsh;
}

int SyncPackno(struct sISP_COMMAND *gISP_COMMAND)
{
    return gISP_COMMAND->ISPSyncPackno();
}

void Auto_Detect_Connect(struct sISP_COMMAND *gISP_COMMAND)
{
    gISP_COMMAND->ISPauto_detect_command();
}


int FWVersion(struct sISP_COMMAND *gISP_COMMAND, uint32_t *buff)
{
    return gISP_COMMAND->ISPFWVersion(buff);
}


int GetDeviceID(struct sISP_COMMAND *gISP_COMMAND, uint32_t *buff)
{
    return gISP_COMMAND->ISPGetDeviceID(buff);
}

int GetConfig(struct sISP_COMMAND *gISP_COMMAND, uint32_t *buff)
{

    return gISP_COMMAND->ISPGetConfig(buff);
}

int GetFlashMode(struct sISP_COMMAND *gISP_COMMAND, uint32_t *buff)
{
    return gISP_COMMAND->ISPGetFlashMode(buff);
}
void RunLDROM(struct sISP_COMMAND *gISP_COMMAND)
{
    gISP_COMMAND->ISPCmdRunLDROM();
}

void RunAPROM(struct sISP_COMMAND *gISP_COMMAND)
{
    gISP_COMMAND->ISPCmdRunAPROM();
}

int Updated_Target_Flash(struct sISP_COMMAND *gISP_COMMAND, uint32_t in_startaddr, uint32_t in_file_totallen)
{
    return gISP_COMMAND->ISPUpdateFlash(in_startaddr, in_file_totallen);
}


