#include "ISP_DRIVER.H"

extern ErrNo ISP_UART_WRITE(io_handle_t handle, const void *buf, uint32 *len);
extern ErrNo ISP_UART_READ(io_handle_t handle, void *buf, uint32 *len);
extern ErrNo UART_Config(void *priv);

CHAR_DEVIO_TABLE(
    UART_Driver,
    ISP_UART_WRITE,
    ISP_UART_READ,
)

extern ErrNo CAN_WRITE(io_handle_t handle, const void *buf, uint32 *len);
extern ErrNo CAN_READ(io_handle_t handle, void *buf, uint32 *len);
extern ErrNo CAN_Config(void *priv);

CHAR_DEVIO_TABLE(
    CAN_Driver,
    CAN_WRITE,
    CAN_READ,
)

extern ErrNo SPI_WRITE(io_handle_t handle, const void *buf, uint32 *len);
extern ErrNo SPI_READ(io_handle_t handle, void *buf, uint32 *len);
extern ErrNo SPI_Config(void *priv);

CHAR_DEVIO_TABLE(
    SPI_Driver,
    SPI_WRITE,
    SPI_READ,
)

extern ErrNo I2C_WRITE(io_handle_t handle, const void *buf, uint32 *len);
extern ErrNo I2C_READ(io_handle_t handle, void *buf, uint32 *len);
extern ErrNo I2C_Config(void *priv);

CHAR_DEVIO_TABLE(
    I2C_Driver,
    I2C_WRITE,
    I2C_READ,
)


extern ErrNo RS485_WRITE(io_handle_t handle, const void *buf, uint32 *len);
extern ErrNo RS485_READ(io_handle_t handle, void *buf, uint32 *len);
extern ErrNo RS485_Config(void *priv);
extern ErrNo RS485_Package(io_handle_t handle, void *buf, uint32 *len);
CHAR_DEVIO_TABLE(
    RS485_Driver,
    RS485_WRITE,
    RS485_READ,
)
devtab_entry_t DevTab[] =
{
    UART_NAME_STRING, &UART_Driver, UART_Config,
    CAN_NAME_STRING, &CAN_Driver, CAN_Config,
    SPI_NAME_STRING, &SPI_Driver, SPI_Config,
    I2C_NAME_STRING, &I2C_Driver, I2C_Config,
    RS485_NAME_STRING, &RS485_Driver, RS485_Config,
};

static int io_compare(const char *n1, const char *n2, const char **ptr)
{
    while (*n1 && *n2)
    {
        if (*n1++ != *n2++)
        {
            return 0;
        }
    }

    if (*n1)
    {
        // See if the devtab name is is a substring
        if (*(n2 - 1) == '/')
        {
            *ptr = n1;
            return 1;
        }
    }

    if (*n1 || *n2)
    {
        return 0;
    }

    *ptr = n1;
    return 1;
}

ErrNo  io_open(const char *dev_name, io_handle_t *io_handle)
{
    devtab_entry_t *p_devtab_entry;
    uint32 devtab_size, i;
    const char  *name_ptr;
    ErrNo re;

    if (dev_name == 0 || io_handle == 0)
    {
        return -EINVAL;
    }

    p_devtab_entry = (devtab_entry_t *)DevTab;

    devtab_size = sizeof(DevTab) / sizeof(devtab_entry_t);

    for (i = 0; i < devtab_size; i++)
    {
        if (io_compare(dev_name, p_devtab_entry->name, &name_ptr))
        {
            if (p_devtab_entry->init)
            {
                re = p_devtab_entry->init((void *)p_devtab_entry);

                if (re != ENOERR)
                    return re;

                *io_handle = (io_handle_t *)p_devtab_entry;
                return re;
            }
        }

        p_devtab_entry++;
    }

    return -ENOENT;
}

ErrNo io_write(io_handle_t handle, const void *buf, uint32 *len)
{
    devtab_entry_t *t = (devtab_entry_t *)handle;

    if (handle == NULL || buf == NULL || len == NULL)
    {
        return -EINVAL;
    }

    // Validate request
    if (!t->handlers->write)
    {
        return -EDEVNOSUPP;
    }

    // Special check.  If length is zero, this just verifies that the
    // 'write' method exists for the given device.
    if (NULL != len && 0 == *len)
    {
        return ENOERR;
    }

    return t->handlers->write(handle, buf, len);
}

//
// 'read' data from a device.
//

ErrNo io_read(io_handle_t handle, void *buf, uint32 *len)
{
    devtab_entry_t *t = (devtab_entry_t *)handle;

    if (handle == NULL || buf == NULL || len == NULL)
    {
        return -EINVAL;
    }

    // Validate request
    if (!t->handlers->read)
    {
        return -EDEVNOSUPP;
    }

    // Special check.  If length is zero, this just verifies that the
    // 'read' method exists for the given device.
    if (NULL != len && 0 == *len)
    {
        return ENOERR;
    }

    return t->handlers->read(handle, buf, len);
}
int str_compare(const char *n1, const char *n2)
{
    while (*n1 && *n2)
    {
        if (*n1++ != *n2++)
        {
            return 0;
        }
    }

    if (*n1)
    {
        // See if the devtab name is is a substring
        if (*(n2 - 1) == '/')
        {
            return 1;
        }
    }

    if (*n1 || *n2)
    {
        return 0;
    }

    return 1;
}


