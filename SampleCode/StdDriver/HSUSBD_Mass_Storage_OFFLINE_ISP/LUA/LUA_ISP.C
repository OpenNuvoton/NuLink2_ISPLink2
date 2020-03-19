#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "M480.h"
#include "lauxlib.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luaconf.h"
#include "ISP_CMD.h"
#include "DRV_TFTLCD.h"
#include "GUI.h"
#include "diskio.h"
#include "ff.h"
volatile unsigned char FONE_SIZE = 32;
int ISP_CmdSyncPackno(lua_State *L)
{
    int argc, n ;
    argc    = lua_gettop(L); //get number count
    // printf("ISP_CmdSyncPackno argc:%d\n\r", argc);
    n = (int)lua_tonumber(L, 1); //get top data

    // printf("argc:%d\n\r", n);
    if (CmdSyncPackno() == 1)
    {
        lua_pushnumber(L, (int)1);
    }
    else
    {
        lua_pushnumber(L, (int)0);
    }

    return 1;
}


int ISP_CmdUpdateAprom(lua_State *L)
{
    int argc, n ;
    const char *ttt;
    unsigned char APROM_NAME[128];
    argc    = lua_gettop(L);
    ttt = lua_tostring(L, 1);
    memset(APROM_NAME, '\0', sizeof(APROM_NAME));
    memcpy((unsigned char *)APROM_NAME, ttt, lua_rawlen(L, 1));

    if (CmdUpdateAprom((void *)APROM_NAME) == 1)
    {
        lua_pushnumber(L, (int)1);
    }
    else
    {
        lua_pushnumber(L, (int)0);
    }

    return 1;
}

int ISP_CmdFWVersion(lua_State *L)
{
    unsigned int local_temp;

    if (CmdFWVersion(&local_temp) == 1)
    {
        lua_newtable(L);
        lua_pushnumber(L, (lua_Number) - 1); //??????
        lua_rawseti(L, -2, 0);
        lua_pushnumber(L, (lua_Number)((local_temp >> 24) & 0xff)); //???
        lua_rawseti(L, -2, 1);
        lua_pushnumber(L, (lua_Number)((local_temp >> 16) & 0xff)); //???
        lua_rawseti(L, -2, 2);
        lua_pushnumber(L, (lua_Number)((local_temp >> 8) & 0xff)); //???
        lua_rawseti(L, -2, 3);
        lua_pushnumber(L, (lua_Number)((local_temp >> 0) & 0xff)); //???
        lua_rawseti(L, -2, 4);
    }

    return 1;
}


int ISP_CmdGetFlashMode(lua_State *L)
{
    unsigned int local_temp;

    if (CmdGetFlashMode(&local_temp) == 1)
    {
        lua_pushnumber(L, local_temp);
    }
    else
    {
        lua_pushnumber(L, (unsigned int)0xffffffff);
    }

    return 1;
}

int ISP_CmdGetDeviceID(lua_State *L)
{
    unsigned int local_temp;

    if (CmdGetDeviceID(&local_temp) == 1)
    {
        lua_pushnumber(L, local_temp);
    }
    else
    {
        lua_pushnumber(L, (unsigned int)0xffffffff);
    }

    return 1;
}

int ISP_KEY_INPUT(lua_State *L)
{
    unsigned int local_temp;

    if (KEY1 == 0)
    {
        while (KEY1 == 0);

        lua_pushnumber(L, (unsigned int)1);
        return 1;
    }

    if (KEY3 == 0)
    {
        while (KEY3 == 0);

        lua_pushnumber(L, (unsigned int)2);
        return 1;
    }

    if (KEY2 == 0)
    {
        while (KEY2 == 0);

        lua_pushnumber(L, (unsigned int)3);
        return 1;
    }

    if (KEY4 == 0)
    {
        while (KEY4 == 0);

        lua_pushnumber(L, (unsigned int)4);
        return 1;
    }

    if ((KEY1 == 1) & (KEY2 == 1) & (KEY3 == 1) & (KEY4 == 1))
    {
        lua_pushnumber(L, (unsigned int)0);
        return 1;
    }


}


int ISP_CmdGetConfig(lua_State *L)
{
    unsigned int local_temp[2];

    //int argc,n ;
    //argc  = lua_gettop(L); //get number count
    //printf("ISP_CmdSyncPackno argc:%d\n\r", argc);
    //n = (int)lua_tonumber(L, 1); //get top data
    //printf("argc:%d\n\r", n);
    if (CmdGetConfig(local_temp) == 1)
    {
        lua_newtable(L);
        lua_pushnumber(L, (lua_Number) - 1); //??????
        lua_rawseti(L, -2, 0);
#if 1
        //config 0
        lua_pushnumber(L, (lua_Number)((local_temp[0] >> 24) & 0xff)); //???
        lua_rawseti(L, -2, 1);
        lua_pushnumber(L, (lua_Number)((local_temp[0] >> 16) & 0xff)); //???
        lua_rawseti(L, -2, 2);
        lua_pushnumber(L, (lua_Number)((local_temp[0] >> 8) & 0xff)); //???
        lua_rawseti(L, -2, 3);
        lua_pushnumber(L, (lua_Number)((local_temp[0] >> 0) & 0xff)); //???
        lua_rawseti(L, -2, 4);
        //config1
        lua_pushnumber(L, (lua_Number)((local_temp[1] >> 24) & 0xff)); //???
        lua_rawseti(L, -2, 5);
        lua_pushnumber(L, (lua_Number)((local_temp[1] >> 16) & 0xff)); //???
        lua_rawseti(L, -2, 6);
        lua_pushnumber(L, (lua_Number)((local_temp[1] >> 8) & 0xff)); //???
        lua_rawseti(L, -2, 7);
        lua_pushnumber(L, (lua_Number)((local_temp[1] >> 0) & 0xff)); //???
        lua_rawseti(L, -2, 8);
#endif

    }

    return 1;
}


int ISP_CmdJumpAPROM(lua_State *L)
{
    if (CmdJumpAPROM() == 1)
    {
        lua_pushnumber(L, (unsigned int)0xffffffff);
    }

    return 1;
}
int ISP_CmdJumpLDROM(lua_State *L)
{
    if (CmdJumpLDROM() == 1)
    {
        lua_pushnumber(L, (unsigned int)0xffffffff);
    }

    return 1;
}

int ISP_INTERFACE_INIT(lua_State *L)
{

    int argc;
    argc    = lua_gettop(L); //get number count
    DEVICE_TYPE = (int)lua_tonumber(L, 1); //get top data
    INTERFACE_INIT();
    return 1;
}

int ISP_LCD_Show_String(lua_State *L)
{
    int argc;

    const char *ttt;
    unsigned int x, y, f, b;
    argc  = lua_gettop(L);
    ttt = lua_tostring(L, 1);
    x = atoi(ttt);
    ttt = lua_tostring(L, 2);
    y = atoi(ttt);
    ttt = lua_tostring(L, 4);
    f = atoi(ttt);
    ttt = lua_tostring(L, 5);
    b = atoi(ttt);
    ttt = lua_tostring(L, 3);

    GUI_SetBkColor(b);
    GUI_SetColor(f);

    GUI_DispStringAt((uint8_t *)ttt, x * FONE_SIZE, y * FONE_SIZE);
    return 1;
}

int LOAD_FILE_SIZE(lua_State *L)
{
    unsigned int local_temp;
    unsigned char APROM_NAME[128];
    int argc;

    const char *ttt;
    argc    = lua_gettop(L);
    ttt = lua_tostring(L, 1);
    memset(APROM_NAME, '\0', sizeof(APROM_NAME));
    memcpy((unsigned char *)APROM_NAME, ttt, lua_rawlen(L, 1));
    local_temp = GET_FILE_SIZE((void *)APROM_NAME);
    lua_pushnumber(L, local_temp);
    return 1;
}

int LOAD_FILE_CHECKSUM(lua_State *L)
{
    unsigned int local_temp;
    unsigned char APROM_NAME[128];
    int argc;

    const char *ttt;
    argc    = lua_gettop(L);
    ttt = lua_tostring(L, 1);
    memset(APROM_NAME, '\0', sizeof(APROM_NAME));
    memcpy((unsigned char *)APROM_NAME, ttt, lua_rawlen(L, 1));
    local_temp = GET_FILE_CHECKSUM((void *)APROM_NAME);
    lua_pushnumber(L, local_temp);
    return 1;
}

extern uint32_t volatile g1Sec;
int GET_SEC(lua_State *L)
{
    unsigned int local_temp;
    local_temp = g1Sec;
    lua_pushnumber(L, local_temp);
    return 1;
}


void wr_log(void *name, void *log)
{

    FIL file1;
    FRESULT res;
    unsigned int s1;
    unsigned int file_size;
    unsigned int l = 0;
    unsigned char buffer[256];
    uint8_t *TempChar;
    uint8_t *logdata;
    TempChar = (uint8_t *)name;
    logdata = (uint8_t *)log;
    res = f_open(&file1, TempChar, FA_WRITE);

    if (res == FR_OK)
    {
        while (*logdata)
        {
            if (*logdata < 0x80)
            {
                buffer[l] = *logdata;
                logdata++;
                l++;
            }

        }

        //for next line
        buffer[l] = 0x0d;
        l++;
        buffer[l] = 0x0a;
        l++;
        file_size = f_size(&file1);
        res = f_lseek(&file1, file_size);//to file end
        res = f_write(&file1, buffer, l, &s1);

        if (res == FR_OK)
            f_close(&file1);
    }

}


int WRITE_LOG(lua_State *L)
{
    int argc;

    const char *f_name;
    const char *string;

    argc  = lua_gettop(L);

    f_name = lua_tostring(L, 1);
    string = lua_tostring(L, 2);
    printf("%s\n\r", f_name);
    printf("%s\n\r", string);

    wr_log((void *)f_name, (void *)string);
    return 1;
}


int ISP_Release_Data(lua_State *L)
{
    lua_pushstring(L, "2018/01/19");
    return 1;
}

int ISP_Release_Version(lua_State *L)
{
    lua_pushstring(L, "V0.001");
    return 1;
}

int ISP_Release_Owner(lua_State *L)
{
    lua_pushstring(L, "JCliu");
    return 1;
}

int ISP_LIST_INTERFACE(lua_State *L)
{


    lua_newtable(L);
    lua_pushstring(L, ""); //??????
    lua_rawseti(L, -2, 0);
    lua_pushstring(L, "UART=>0"); //??????
    lua_rawseti(L, -2, 1);
    lua_pushstring(L, "USB HOST=>7"); //??????
    lua_rawseti(L, -2, 2);


    return 1;
}
int LCD_TFT_FONT(lua_State *L)
{
    int argc;

    argc    = lua_gettop(L); //get number count
    FONE_SIZE = (int)lua_tonumber(L, 1); //get top data

    if (FONE_SIZE == 16)
        GUI_SetFont(&GUI_Font16_ASCII);

    if (FONE_SIZE == 24)
        GUI_SetFont(&GUI_Font24_ASCII);

    if (FONE_SIZE == 32)
        GUI_SetFont(&GUI_Font32_ASCII);

    return 1;
}

int LCD_TFT_CLEAR(lua_State *L)
{
    GUI_Clear();
    return 1;
}


static const struct luaL_Reg ISP_COMMAND[] =
{
    {"ISP_CmdSyncPackno", ISP_CmdSyncPackno},
    {"ISP_CmdFWVersion", ISP_CmdFWVersion},
    {"ISP_CmdGetDeviceID", ISP_CmdGetDeviceID},
    {"ISP_CmdGetConfig", ISP_CmdGetConfig},
    {"ISP_CmdJumpLDROM", ISP_CmdJumpLDROM},
    {"ISP_CmdJumpAPROM", ISP_CmdJumpAPROM},
    {"ISP_CmdGetFlashMode", ISP_CmdGetFlashMode},
    {"ISP_INTERFACE_INIT", ISP_INTERFACE_INIT},
    {"ISP_LCD_Show_String", ISP_LCD_Show_String},
    {"ISP_CmdUpdateAprom", ISP_CmdUpdateAprom},
    {"LOAD_FILE_SIZE", LOAD_FILE_SIZE},
    {"LOAD_FILE_CHECKSUM", LOAD_FILE_CHECKSUM},
    {"WRITE_LOG", WRITE_LOG},
    {"ISP_KEY_INPUT", ISP_KEY_INPUT},
    {"GET_SEC", GET_SEC},
    {"ISP_Release_Owner", ISP_Release_Owner},
    {"ISP_Release_Data", ISP_Release_Data},
    {"ISP_Release_Version", ISP_Release_Version},
    {"ISP_LIST_INTERFACE", ISP_LIST_INTERFACE},
    {"LCD_TFT_FONT", LCD_TFT_FONT},
    {"LCD_TFT_CLEAR", LCD_TFT_CLEAR},
    {NULL, NULL}
};

int luaopen_ISP(lua_State *L)
{
    luaL_setfuncs(L, ISP_COMMAND, 0);
    return 1;
}
