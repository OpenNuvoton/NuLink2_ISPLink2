#include <string.h>
#include "NuMicro.h"
#include "stdio.h"
#include "stdint.h"
#include "ISP_DRIVER.H"
#include "hal_api.h"
#include "usbh_lib.h"
#include "usbh_hid.h"
volatile  unsigned char  has_out_data = 0;
HID_DEV_T   *g_hid_list[CONFIG_HID_MAX_DEV];
HID_DEV_T    *hdev, *hdev_list;
extern void enable_USB_HOST_tick(int ticks_per_second);
void  dump_buff_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;

    while (nBytes > 0)
    {
        printf("0x%04X  ", nIdx);

        for (i = 0; (i < 16) && (nBytes > 0); i++)
        {
            printf("%02x ", pucBuff[nIdx + i]);
            nBytes--;
        }

        nIdx += 16;
        printf("\n");
    }

    printf("\n");
}
unsigned char USB_HOST_recbuf[64];
void  int_read_callback(HID_DEV_T *hdev, uint16_t ep_addr, int status, uint8_t *rdata, uint32_t data_len)
{
    int i;
    //   memcpy((uint8_t *)rcvbuf, rdata, 64);
    has_out_data = 1;

    for (i = 0; i < 64; i++)
        USB_HOST_recbuf[i] = rdata[i];
}

unsigned char USB_HOST_sendbuf[64];
void  int_write_callback(HID_DEV_T *hdev, uint16_t ep_addr, int status, uint8_t *wbuff, uint32_t *buff_size)
{
    int i;

    //copy data to send buffer
    for (i = 0; i < 64; i++)
        wbuff[i] = USB_HOST_sendbuf[i];

    //wbuff = &USB_HOST_sendbuf[0];
    *buff_size = 64;
}

uint8_t  g_buff_pool[1024] __attribute__((aligned(32)));
int  init_hid_device(HID_DEV_T *hdev)
{
    uint8_t   *data_buff;
    int       i, ret;

    data_buff = (uint8_t *)((uint32_t)g_buff_pool);

    printf("\n\n==================================\n");
    printf("  Init HID device : 0x%x\n", (int)hdev);
    printf("  VID: 0x%x, PID: 0x%x\n\n", hdev->idVendor, hdev->idProduct);

    ret = usbh_hid_get_report_descriptor(hdev, data_buff, 1024);

    if (ret > 0)
    {
        printf("\nDump report descriptor =>\n");
        dump_buff_hex(data_buff, ret);
    }

#if 0
    /*
     *  Example: GET_PROTOCOL request.
     */
    ret = usbh_hid_get_protocol(hdev, data_buff);
    printf("[GET_PROTOCOL] ret = %d, protocol = %d\n", ret, data_buff[0]);

    /*
     *  Example: SET_PROTOCOL request.
     */
    ret = usbh_hid_set_protocol(hdev, data_buff[0]);
    printf("[SET_PROTOCOL] ret = %d, protocol = %d\n", ret, data_buff[0]);

    /*
     *  Example: GET_REPORT request on report ID 0x1, report type FEATURE.
     */
    ret = usbh_hid_get_report(hdev, RT_FEATURE, 0x1, data_buff, 64);

    if (ret > 0)
    {
        printf("[GET_REPORT] Data => ");

        for (i = 0; i < ret; i++)
            printf("%02x ", data_buff[i]);

        printf("\n");
    }

#endif
#if 1
    printf("\nUSBH_HidStartIntReadPipe...\n");
    ret = usbh_hid_start_int_read(hdev, 0x81, int_read_callback);

    if (ret != HID_RET_OK)
        printf("usbh_hid_start_int_read failed! %d\n", ret);
    else
        printf("Interrupt in transfer started...\n");

#endif
    return 0;
}
void update_hid_device_list(HID_DEV_T *hdev)
{
    int  i = 0;
    memset(g_hid_list, 0, sizeof(g_hid_list));

    while ((i < CONFIG_HID_MAX_DEV) && (hdev != NULL))
    {
        g_hid_list[i++] = hdev;
        hdev = hdev->next;
    }
}

ErrNo USBH_HID_WRITE(io_handle_t handle, const void *buf, uint32 *len)
{
    usbh_pooling_hubs();
    uint8_t *pSrc;
    pSrc = (uint8_t *)buf;
    int i = 0;
    has_out_data = 0;

    for (i = 0; i < 64; i++)
        USB_HOST_sendbuf[i] = pSrc[i];

   // printf("write:%d\n\r", usbh_hid_start_int_write(hdev, 0x2, int_write_callback));

    if (usbh_hid_start_int_write(hdev, 0x2, int_write_callback) != HID_RET_OK)
    {
        printf("\n[FAIL] ==> Interrupt out transfer started...\n");

     }


    return ENOERR;
}

ErrNo USBH_HID_READ(io_handle_t handle, void *buf, uint32 *len)
{
    usbh_pooling_hubs();
    uint8_t *pSrc;
    int i;
    pSrc = (uint8_t *)buf;

    int        ret;
    //ret = usbh_hid_start_int_read(hdev, 0x81, int_read_callback);
    usbh_pooling_hubs();
    // if (ret != HID_RET_OK)
    //    printf("usbh_hid_start_int_read failed!\n");

    while (has_out_data == 0)
    {
        usbh_pooling_hubs();
    }

    usbh_hid_stop_int_write(hdev, 0x02) ;
    //usbh_hid_stop_int_read(hdev, 0x02) ;
    usbh_pooling_hubs();

    ///CLK_SysTickDelay(50000);
    for (i = 0; i < 64; i++)
        pSrc[i] = USB_HOST_recbuf[i];

    return ENOERR;
}

ErrNo USBH_HID_Config(void *priv)
{
    enable_USB_HOST_tick(100);
    usbh_core_init();
    usbh_hid_init();
    usbh_memory_used();

    memset(g_hid_list, 0, sizeof(g_hid_list));

    while (1)
    {
        if (usbh_pooling_hubs())             /* USB Host port detect polling and management */
        {
            printf("\n Has hub events.\n");
            hdev = usbh_hid_get_device_list();

            if (hdev != NULL)
            {
                init_hid_device(hdev);
                update_hid_device_list(hdev_list);
                usbh_memory_used();
                break;
            }


        }
    }

    return ENOERR;
}
