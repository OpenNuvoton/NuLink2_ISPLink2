#include "stdio.h"
#include "stdint.h"
#include "ISP_DRIVER.H"
#include "NuMicro.h"
#include "hal_api.h"


ErrNo I2C_WRITE(io_handle_t handle, const void *buf, uint32 *len)
{
    UI2C_WriteMultiBytes(UI2C0, 0x60, (uint8_t *)buf, 64);
    return ENOERR;
}

ErrNo I2C_READ(io_handle_t handle, void *buf, uint32 *len)
{
    UI2C_ReadMultiBytes(UI2C0, 0x60, (uint8_t *)buf, 64);
    return ENOERR;
}

ErrNo I2C_Config(void *priv)
{

    return 0;
}

ErrNo I2C_Package(io_handle_t handle, void *buf, uint32 *len)
{
    return ENOERR;

}




