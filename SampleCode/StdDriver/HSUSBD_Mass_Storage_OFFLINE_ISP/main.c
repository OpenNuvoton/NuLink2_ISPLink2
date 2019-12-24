/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * @brief    Implement a mass storage class sample to demonstrate how to
 *           receive an USB short packet.
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"
#include "massstorage.h"
#include "isp_bridge.h"
#include "ISP_CMD.h"
#include "ISP_DRIVER.H"
#include "hal_api.h"
#include "Voltage.h"
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
io_handle_t  DEV_handle = NULL;
extern struct sISP_COMMAND ISP_COMMAND;
uint32_t fwversion, flash_boot;
extern volatile unsigned int AP_file_totallen;
uint32_t devid, config[2];
extern void SpiInit(void);

#define ISP_LED_ON PC6=0
#define ISP_LED_OFF PC6=1
#define BUSY PB9
#define PASS PB8

#if 1
void check_error(char d, char *l, ErrNo ret_in)
{
    if (ret_in)
    {
        printf("%d\n\r", d);
        printf("%s\n\r", l);
        printf("this is error code %d\n\r", ret_in);
        //while(1);
    }
}
#else
void check_error(char d, char *l, ErrNo ret_in)
{

}
#endif
/*--------------------------------------------------------------------------*/
void SYS_Init(void)
{
    uint32_t volatile i;

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Set XT1_OUT(PF.2) and XT1_IN(PF.3) to input mode */
    PF->MODE &= ~(GPIO_MODE_MODE2_Msk | GPIO_MODE_MODE3_Msk);

    /* Enable External XTAL (4~24 MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);

    /* Waiting for 12MHz clock ready */
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* Switch HCLK clock source to HXT */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HXT, CLK_CLKDIV0_HCLK(1));

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(FREQ_192MHZ);

    /* Set both PCLK0 and PCLK1 as HCLK/2 */
    CLK->PCLKDIV = CLK_PCLKDIV_PCLK0DIV2 | CLK_PCLKDIV_PCLK1DIV2;

    SYS->USBPHY &= ~SYS_USBPHY_HSUSBROLE_Msk;    /* select HSUSBD */
    /* Enable USB PHY */
    SYS->USBPHY = (SYS->USBPHY & ~(SYS_USBPHY_HSUSBROLE_Msk | SYS_USBPHY_HSUSBACT_Msk)) | SYS_USBPHY_HSUSBEN_Msk;

    for (i = 0; i < 0x1000; i++);  // delay > 10 us

    SYS->USBPHY |= SYS_USBPHY_HSUSBACT_Msk;

    /* Enable IP clock */
    CLK_EnableModuleClock(HSUSBD_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HXT, CLK_CLKDIV0_UART0(1));

    /* Enable IP clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Set GPB multi-function pins for UART0 RXD and TXD */
    //SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk);
    //SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);
    SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk);
    SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA0MFP_UART0_RXD | SYS_GPA_MFPL_PA1MFP_UART0_TXD);

}
extern unsigned int Read_ARPOM_INFORMATION(void);
extern void Read_DATAFLASH_INFORMATION(void);
extern void Read_SN_INFORMATION(void);
extern void Read_INTERFACE(void);
extern void Read_Define_File(void);
extern void Read_CONFIG_SDCARD(void);
extern unsigned int MOUNT_FILE_SYSTEM(void) ;
extern void UNMOUNT_FILE_SYSTEM(void);
extern volatile DEVICE_ENUM DEVICE_TYPE;
extern volatile unsigned int AUTO_DETECT_VALUE;
unsigned int programming_count;


void ISP_FLOW(void)
{
    ErrNo ret;
    BUSY = 0;

    if (MOUNT_FILE_SYSTEM() == 0)
    {
        printf("fatfs ok\n\r");
    }
    else
        return ;

    //read aprom information
    if (Read_ARPOM_INFORMATION() == 0)
    {

        //read dataflash information
        Read_DATAFLASH_INFORMATION();

        //read serinal number
        Read_SN_INFORMATION();

        //read interface
        Read_INTERFACE();

        Read_CONFIG_SDCARD();

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



        init_ISP_command();

        if (AUTO_DETECT_VALUE == 1)
        {
            Auto_Detect_Connect(&ISP_COMMAND);
        }
        if (DEVICE_TYPE == RS485_BUS)
            AUTO_DETECT_VALUE = 1;


        if (DEVICE_TYPE == UART_BUS)
            AUTO_DETECT_VALUE = 1;
        ret = SyncPackno(&ISP_COMMAND);
        check_error(__LINE__, __FILE__, ret);
        if (DEVICE_TYPE == UART_BUS)
            AUTO_DETECT_VALUE = 0;
				if (DEVICE_TYPE == RS485_BUS)
            AUTO_DETECT_VALUE = 0;
        if (ret!=0)
            goto EXIT;

        ret = FWVersion(&ISP_COMMAND, &fwversion);
        check_error(__LINE__, __FILE__, ret);
			
        printf("fw version:0x%x\n\r", fwversion);
        ret = GetDeviceID(&ISP_COMMAND, &devid);
        check_error(__LINE__, __FILE__, ret);

        printf("device id:0x%x\n\r", devid);
        ret = GetConfig(&ISP_COMMAND, config);
        check_error(__LINE__, __FILE__, ret);
        printf("config0: 0x%x\n\r", config[0]);
        printf("config1: 0x%x\n\r", config[1]);
        ret = GetFlashMode(&ISP_COMMAND, &flash_boot);
        check_error(__LINE__, __FILE__, ret);
        printf("device id:0x%x\n\r", devid);

        if (flash_boot != LDROM_MODE)
        {
            printf("boot in APROM\n\r");
        }
        else
        {
            printf("boot in LDROM\n\r");
        }

        if (AP_file_totallen != 0)
        {
            ret = Updated_Target_Flash(&ISP_COMMAND, 0, AP_file_totallen);
            check_error(__LINE__, __FILE__, ret);
            programming_count++;
            printf("%d\n\r", programming_count);
        }

    }

EXIT:
    UNMOUNT_FILE_SYSTEM();

}

void GPC_IRQHandler(void)
{
    /* To check if PC.5 interrupt occurred */
    if (GPIO_GET_INT_FLAG(PC, BIT7))
    {
        GPIO_CLR_INT_FLAG(PC, BIT7);
    }
}

int32_t main(void)
{
    SYS_Init();
    Voltage_Init();
    Voltage_SupplyTargetPower(1, 3300);

    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);
    SpiInit();
    ISP_Bridge_Init();

    HSUSBD_Open(&gsHSInfo, MSC_ClassRequest, NULL);

    /* Endpoint configuration */
    MSC_Init();
    GPIO_SET_DEBOUNCE_TIME(GPIO_DBCTL_DBCLKSRC_LIRC, GPIO_DBCTL_DBCLKSEL_1024);
    GPIO_ENABLE_DEBOUNCE(PC, BIT7);
    /* Enable USBD interrupt */
    NVIC_EnableIRQ(USBD20_IRQn);
    GPIO_SetMode(PC, BIT7, GPIO_MODE_INPUT);
    GPIO_EnableInt(PC, 7, GPIO_INT_FALLING);
    NVIC_EnableIRQ(GPC_IRQn);
    /* Start transaction */

    GPIO_SetMode(PC, BIT6, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PB, BIT9, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PB, BIT8, GPIO_MODE_OUTPUT);
    ISP_LED_ON;

    while (1)
    {
        //  SYS_UnlockReg();
        CLK_Idle();

        if (PC7 == 0)
        {
            NVIC_DisableIRQ(USBD20_IRQn);
            ISP_FLOW();
            NVIC_EnableIRQ(USBD20_IRQn);
        }

        if (HSUSBD_IS_ATTACHED())
        {
            HSUSBD_Start();
            break;
        }
    }

    while (1)
    {
        CLK_Idle();

        if (g_hsusbd_Configured)
            MSC_ProcessCmd();

        if (PC7 == 0)
        {
            NVIC_DisableIRQ(USBD20_IRQn);
            ISP_FLOW();
            NVIC_EnableIRQ(USBD20_IRQn);

        }
    }
}



/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/

