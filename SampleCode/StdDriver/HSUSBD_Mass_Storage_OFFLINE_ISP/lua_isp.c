#include <stdio.h>
#include "NuMicro.h"
#include "massstorage.h"
#include "isp_bridge.h"
#include "ISP_CMD.h"
#include "ISP_DRIVER.H"
#include "hal_api.h"
#include "usbh_lib.h"
#include "Voltage.h"
#include "ff.h"
#include <string.h>
#include "lauxlib.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luaconf.h"
typedef enum
{
    UART_BUS,
    CAN_BUS,
    SPI_BUS,
    I2C_BUS,
    RS485_BUS,
    USBH_HID_BUS,
} DEVICE_ENUM;
extern unsigned int Read_ARPOM_INFORMATION(void);
extern void Read_DATAFLASH_INFORMATION(void);
extern void Read_SN_INFORMATION(void);
extern void Read_INTERFACE(void);

extern void Read_Define_File(void);
extern void Read_CONFIG_SDCARD(void);
extern unsigned int MOUNT_FILE_SYSTEM(void) ;
extern void UNMOUNT_FILE_SYSTEM(void);
extern void CmdRunAPROM(void);
extern void CmdRunLDROM(void);
extern volatile DEVICE_ENUM DEVICE_TYPE;
extern volatile unsigned int AUTO_DETECT_VALUE;
extern unsigned int GET_FILE_CHECKSUM(void *buf);
extern unsigned int GET_FILE_SIZE(void *buf);
io_handle_t  DEV_handle = NULL;
extern struct sISP_COMMAND ISP_COMMAND;
extern volatile unsigned int AP_file_totallen;
extern  unsigned int AP_file_checksum;
extern volatile unsigned int DATAFLASH_file_totallen;
extern volatile unsigned int DATAFLASH_file_checksum;
extern unsigned char storage;
uint32_t config[2];


static int SHOW_STORAGE(lua_State *L)
{
    lua_pushnumber(L, (unsigned int)storage);
#if 0

    if ((storage & 0x01) == 0x01)
    {
        printf("the SPI flash ready\n\r");
    }

    if ((storage & 0x02) == 0x02)
    {
        printf("the SDCARD ready\n\r");
    }

    if ((storage & 0x04) == 0x04)
    {
        printf("the USB DISK ready\n\r");
    }

#endif
    return 1;
}


int ISP_PROGRAM(lua_State *L)
{
    unsigned int program_size;
    unsigned int start_address;
    int argc;
    ErrNo ret;
    argc    = lua_gettop(L); //get number count
    start_address = (unsigned int)lua_tonumber(L, 1); //get top data
    program_size = (unsigned int)lua_tonumber(L, 2);

    if (argc != 2)
        return 1;

    ret = Updated_Target_Flash(&ISP_COMMAND, start_address, program_size);

    lua_pushnumber(L, (int)ret); //0 pass, 1 false

    return 1;
}

int ISP_INTERFACE_INIT(lua_State *L)
{

    int argc;
    ErrNo ret;
    argc    = lua_gettop(L); //get number count
    DEVICE_TYPE = (DEVICE_ENUM)lua_tonumber(L, 1); //get top data

    if (DEVICE_TYPE == I2C_BUS)
        ret = io_open(I2C_NAME_STRING, &DEV_handle);

    if (DEVICE_TYPE == RS485_BUS)
        ret = io_open(RS485_NAME_STRING, &DEV_handle);

    if (DEVICE_TYPE == SPI_BUS)
        ret = io_open(SPI_NAME_STRING, &DEV_handle);

    if (DEVICE_TYPE == UART_BUS)
        ret = io_open(UART_NAME_STRING, &DEV_handle);

    if (DEVICE_TYPE == CAN_BUS)
        ret = io_open(CAN_NAME_STRING, &DEV_handle);

    if (DEVICE_TYPE == USBH_HID_BUS)
        ret = io_open(USBH_HID_NAME_STRING, &DEV_handle);

    init_ISP_command();
    //if  (ret ==0 )


    return 1;
}

//only rs485, uart support auto detect command
int ISP_AUTO_DETECT(lua_State *L)
{
    ErrNo ret;
    int argc;
    argc    = lua_gettop(L); //get number count
    AUTO_DETECT_VALUE = (int)lua_tonumber(L, 1); //get top data
    AUTO_DETECT_VALUE = 1    ;

    if (AUTO_DETECT_VALUE == 1)
    {
        Auto_Detect_Connect(&ISP_COMMAND);
    }

    if (DEVICE_TYPE == RS485_BUS)
        AUTO_DETECT_VALUE = 1;


    if (DEVICE_TYPE == UART_BUS)
        AUTO_DETECT_VALUE = 1;

    ret =  SyncPackno(&ISP_COMMAND); //return 0 =>pass
    lua_pushnumber(L, (int)ret);

    if (DEVICE_TYPE == UART_BUS)
        AUTO_DETECT_VALUE = 0;

    if (DEVICE_TYPE == RS485_BUS)
        AUTO_DETECT_VALUE = 0;

    return 1;
}


int ISP_CmdSyncPackno(lua_State *L)
{
    int argc, n ;
    ErrNo ret;
    argc    = lua_gettop(L); //get number count
    // printf("ISP_CmdSyncPackno argc:%d\n\r", argc);
    n = (int)lua_tonumber(L, 1); //get top data
    AUTO_DETECT_VALUE = 0;
    ret = SyncPackno(&ISP_COMMAND);
    lua_pushnumber(L, (int)ret);   //return 0, pass
    return 1;
}


int ISP_AP_FILE_SIZE(lua_State *L)
{
    unsigned char FILE_NAME[128];
    int argc;

    const char *ttt;
    argc    = lua_gettop(L);
    ttt = lua_tostring(L, 1);
    memset(FILE_NAME, '\0', sizeof(FILE_NAME));
    memcpy((unsigned char *)FILE_NAME, ttt, lua_rawlen(L, 1));

    AP_file_totallen = GET_FILE_SIZE((void *)FILE_NAME);
    //printf("APROM file size =0x%x\n\r",AP_file_totallen);
    lua_pushnumber(L, AP_file_totallen);
    return 1;
}

int ISP_AP_FILE_CHECKSUM(lua_State *L)
{
    unsigned char FILE_NAME[128];
    int argc;

    const char *ttt;
    argc    = lua_gettop(L);
    ttt = lua_tostring(L, 1);
    memset(FILE_NAME, '\0', sizeof(FILE_NAME));
    memcpy((unsigned char *)FILE_NAME, ttt, lua_rawlen(L, 1));
#if USBHOST_DISK
    usbh_pooling_hubs();
#endif
    AP_file_checksum = GET_FILE_CHECKSUM((void *)FILE_NAME);
    //printf("APROM file checksum =0x%x\n\r",AP_file_checksum);
    lua_pushnumber(L, AP_file_checksum);
    return 1;
}


int ISP_DATAFLASH_FILE_SIZE(lua_State *L)
{
    unsigned char FILE_NAME[128];
    int argc;

    const char *ttt;
    argc    = lua_gettop(L);
    ttt = lua_tostring(L, 1);
    memset(FILE_NAME, '\0', sizeof(FILE_NAME));
    memcpy((unsigned char *)FILE_NAME, ttt, lua_rawlen(L, 1));
#if USBHOST_DISK
    usbh_pooling_hubs();
#endif
    DATAFLASH_file_totallen = GET_FILE_SIZE((void *)FILE_NAME);
    //printf("dataflash file size =0x%x\n\r",AP_file_totallen);
    lua_pushnumber(L, DATAFLASH_file_totallen);
    return 1;
}

int ISP_DATA_FILE_CHECKSUM(lua_State *L)
{

    unsigned char FILE_NAME[128];
    int argc;

    const char *ttt;
    argc    = lua_gettop(L);
    ttt = lua_tostring(L, 1);
    memset(FILE_NAME, '\0', sizeof(FILE_NAME));
    memcpy((unsigned char *)FILE_NAME, ttt, lua_rawlen(L, 1));
#if USBHOST_DISK
    usbh_pooling_hubs();
#endif
    DATAFLASH_file_checksum = GET_FILE_CHECKSUM((void *)FILE_NAME);
    //printf("dataflash file checksum =0x%x\n\r",AP_file_checksum);
    lua_pushnumber(L, DATAFLASH_file_checksum);
    return 1;
}

int ISP_CmdFWVersion(lua_State *L)
{
    uint32_t fwversion;
    ErrNo ret;
    ret = FWVersion(&ISP_COMMAND, &fwversion);

    if (ret == 0)
    {
        lua_newtable(L);
        lua_pushnumber(L, (lua_Number) - 1);
        lua_rawseti(L, -2, 0);
        lua_pushnumber(L, (lua_Number)((fwversion >> 24) & 0xff));
        lua_rawseti(L, -2, 1);
        lua_pushnumber(L, (lua_Number)((fwversion >> 16) & 0xff));
        lua_rawseti(L, -2, 2);
        lua_pushnumber(L, (lua_Number)((fwversion >> 8) & 0xff));
        lua_rawseti(L, -2, 3);
        lua_pushnumber(L, (lua_Number)((fwversion >> 0) & 0xff));
        lua_rawseti(L, -2, 4);
    }

    return 1;
}

int ISP_CmdGetDeviceID(lua_State *L)
{
    uint32_t devid;
    ErrNo ret;
    ret = GetDeviceID(&ISP_COMMAND, &devid);

    if (ret == 0) //pass
    {
        lua_pushnumber(L, devid);
    }
    else
    {
        lua_pushnumber(L, (unsigned int)0xffffffff);
    }

    return 1;
}

int ISP_CmdGetFlashMode(lua_State *L)
{
    uint32_t flash_boot;
    ErrNo ret;
    ret = GetFlashMode(&ISP_COMMAND, &flash_boot);

    if (ret == 0)
    {
        lua_pushnumber(L, flash_boot);
    }
    else
    {
        lua_pushnumber(L, (unsigned int)0xffffffff);
    }

    return 1;
}


int ISP_CmdGetConfig(lua_State *L)
{
    unsigned int config[2];
    ErrNo ret;
    //int argc,n ;
    //argc  = lua_gettop(L); //get number count
    //printf("ISP_CmdSyncPackno argc:%d\n\r", argc);
    //n = (int)lua_tonumber(L, 1); //get top data
    //printf("argc:%d\n\r", n);
    ret = GetConfig(&ISP_COMMAND, config);

    if (ret == 0)
    {
        lua_newtable(L);
        lua_pushnumber(L, (lua_Number) - 1); //??????
        lua_rawseti(L, -2, 0);
#if 1
        //config 0
        lua_pushnumber(L, (lua_Number)((config[0] >> 24) & 0xff)); //???
        lua_rawseti(L, -2, 1);
        lua_pushnumber(L, (lua_Number)((config[0] >> 16) & 0xff)); //???
        lua_rawseti(L, -2, 2);
        lua_pushnumber(L, (lua_Number)((config[0] >> 8) & 0xff)); //???
        lua_rawseti(L, -2, 3);
        lua_pushnumber(L, (lua_Number)((config[0] >> 0) & 0xff)); //???
        lua_rawseti(L, -2, 4);
        //config1
        lua_pushnumber(L, (lua_Number)((config[1] >> 24) & 0xff)); //???
        lua_rawseti(L, -2, 5);
        lua_pushnumber(L, (lua_Number)((config[1] >> 16) & 0xff)); //???
        lua_rawseti(L, -2, 6);
        lua_pushnumber(L, (lua_Number)((config[1] >> 8) & 0xff)); //???
        lua_rawseti(L, -2, 7);
        lua_pushnumber(L, (lua_Number)((config[1] >> 0) & 0xff)); //???
        lua_rawseti(L, -2, 8);
#endif

    }

    return 1;
}


int ISP_CmdJumpAPROM(lua_State *L)
{
    CmdRunAPROM();


    return 1;
}
int ISP_CmdJumpLDROM(lua_State *L)
{
    CmdRunLDROM();

    return 1;
}

extern volatile unsigned char APROM_NAME[128];
int ISP_SET_PFILE(lua_State *L)
{
    unsigned int local_temp;
    int argc;

    const char *ttt;
    argc    = lua_gettop(L);
    ttt = lua_tostring(L, 1);
    memset((void *)APROM_NAME, '\0', sizeof(APROM_NAME));
    memcpy((unsigned char *)APROM_NAME, ttt, lua_rawlen(L, 1));
    return 1;
}

static const struct luaL_Reg mylib[] =
{

    {"ISP_CmdFWVersion", ISP_CmdFWVersion},
    {"ISP_CmdJumpAPROM", ISP_CmdJumpAPROM},
    {"ISP_CmdJumpLDROM", ISP_CmdJumpLDROM},
    {"ISP_CmdGetConfig", ISP_CmdGetConfig},
    {"ISP_CmdGetFlashMode", ISP_CmdGetFlashMode},
    {"ISP_CmdGetDeviceID", ISP_CmdGetDeviceID},
    {"ISP_INTERFACE_INIT", ISP_INTERFACE_INIT},
    {"ISP_AUTO_DETECT", ISP_AUTO_DETECT},
    {"ISP_CmdSyncPackno", ISP_CmdSyncPackno},
    {"ISP_AP_FILE_SIZE", ISP_AP_FILE_SIZE},
    {"ISP_AP_FILE_CHECKSUM", ISP_AP_FILE_CHECKSUM},
    {"ISP_DATAFLASH_FILE_SIZE", ISP_DATAFLASH_FILE_SIZE},
    {"ISP_DATA_FILE_CHECKSUM", ISP_DATA_FILE_CHECKSUM},
    {"ISP_SET_PFILE", ISP_SET_PFILE},
    {"ISP_PROGRAM", ISP_PROGRAM},
    {"ISP_SHOW_STORAGE", SHOW_STORAGE},
    {NULL, NULL}

};

int luaopen_mylib(lua_State *L)
{
    luaL_setfuncs(L, mylib, 0);
    return 1;
}

char LUA_SCRIPT_GLOBAL[4096];

#if 0
    char LUA_SCRIPT_GLOBAL[] = "\
    File_name=\"0:\\\\test.bin\"\
    temp=string.format(\"strage=0x%X\",ISP_SHOW_STORAGE())\
    print(temp)\
    filesize=ISP_AP_FILE_SIZE(File_name)\
    if (filesize==0)\
    then\
    print(\"no file exit\")\
    return\
    end\
    temp=string.format(\"APROM SIZE=%d\",filesize)\
    print(temp)\
    filechecksum=ISP_AP_FILE_CHECKSUM(File_name)\
    temp=string.format(\"APROM checksum=0x%x\",filechecksum)\
    print(temp)\
    ISP_INTERFACE_INIT(0)\
    print(\"sync:\")\
    if(ISP_CmdSyncPackno()==1)\
    then\
    print(\"sync false\")\
    return\
    end\
    print(\"sync pass\")\
    devid=ISP_CmdGetDeviceID()\
    temp=string.format(\"devce id=0x%x\",devid)\
    print(temp)\
    tb=ISP_CmdFWVersion()\
    temp=(tb[1]<<24)|(tb[2]<<16)|(tb[3]<<8)|tb[4]\
    print(string.format(\"fw version=0x%x\",temp))\
    tb=ISP_CmdGetConfig()\
    local config0=(tb[1]<<24)|(tb[2]<<16)|(tb[3]<<8)|tb[4]\
    print(string.format(\"config0=0x%x\",config0))\
    config1=(tb[5]<<24)|(tb[6]<<16)|(tb[7]<<8)|tb[8]\
    print(string.format(\"config0=0x%x\",config1))\
    if(ISP_CmdGetFlashMode()==2)\
    then\
    print(\"MCU in LDROM\")\
    else\
    print(\"MCU in APROM\")\
    end\
    ISP_SET_PFILE(File_name)\
    if(ISP_PROGRAM(0,filesize)==0)\
    then\
    print(\"program pass\")\
    end\
    ";
#endif

