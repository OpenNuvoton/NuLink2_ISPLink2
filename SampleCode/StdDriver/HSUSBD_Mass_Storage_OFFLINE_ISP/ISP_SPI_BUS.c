#include <stdio.h>
#include "NuMicro.h"
#include "stdio.h"
#include "stdint.h"
#include "ISP_DRIVER.H"
#include "hal_api.h"

ErrNo SPI_WRITE(io_handle_t handle, const void *buf, uint32 *len)
{
    SPI1_Write((uint32_t *)buf, 16);
    CLK_SysTickDelay(1000);
    return ENOERR;
}

ErrNo SPI_READ(io_handle_t handle, void *buf, uint32 *len)
{
    SPI1_Read((uint32_t *)buf, 16);
    return ENOERR;
}

ErrNo SPI_Config(void *priv)
{

    return ENOERR;
}

ErrNo SPI_Package(io_handle_t handle, void *buf, uint32 *len)
{
    return ENOERR;

}



