
typedef void *io_handle_t;
typedef unsigned int   uint32;
typedef unsigned char   uint8_t;
typedef signed int ErrNo;
#define NULL 0
typedef struct
{
    ErrNo(*write)(io_handle_t handle,
                  const void *buf,
                  uint32 *len);
    ErrNo(*read)(io_handle_t handle,
                 void *buf,
                 uint32 *len);
} devio_table_t;

typedef struct
{
    const   char        *name;
    const   devio_table_t *handlers;
    ErrNo(*init)(void *);

} devtab_entry_t;

#define CHAR_DEVIO_TABLE(_l,_write,_read,_package)    \
    const devio_table_t _l = { \
                               _write, \
                               _read, \
                             };

#define ENOERR           0     /* No error */
#define EPERM            1     /* Not permitted */
#define ENOENT           2     /* No such entity */
#define EPACKAGENO       3     /* Error package no*/
#define ECHECKSUM        4     /* Error checksum*/
#define EFILECHECKSUM    6       /* File checksum error*/
#define ETO              5     /* TIMEOUT error */
#define EI2CSLW              6     /* I2C SLW error */
#define EI2CSLR              7     /* I2C SLW error */
#define EINVAL           22    /* Invalid argument */
#define EDEVNOSUPP       202   /* Device does not support this operation */
#define CAN_NAME_STRING                            "CAN0"
#define I2C_NAME_STRING                            "I2C0"
#define UART_NAME_STRING                            "UART0"
#define SPI_NAME_STRING                            "SPI0"
#define RS485_NAME_STRING                            "RS4850"
extern ErrNo  io_open(const char *dev_name, io_handle_t *io_handle);
extern ErrNo io_write(io_handle_t handle, const void *buf, uint32 *len);
extern ErrNo io_read(io_handle_t handle, void *buf, uint32 *len);
extern int str_compare(const char *n1, const char *n2);
