#include <stdio.h>
#include "NuMicro.h"
#include "hid_transfer.h"
#include "isp_bridge.h"
#include "ISP_CMD.h"
#include "ISP_DRIVER.H"
#include "hal_api.h"
io_handle_t  DEV_handle = NULL;
extern struct sISP_COMMAND ISP_COMMAND;
uint32_t fwversion,flash_boot;
uint32_t devid, config[2];


#if 1
void check_error(char d, char *l,ErrNo ret_in)
{
if(ret_in)
{
printf("%d\n\r",d);
printf("%s\n\r",l);
printf("this is error code %d\n\r",ret_in);
while(1);
}
}
#else
void check_error(char d, char *l,ErrNo ret_in)
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
    CLK->PWRCTL |= CLK_PWRCTL_HIRCEN_Msk;
    CLK->CLKSEL0 = (CLK->CLKSEL0 & (~CLK_CLKSEL0_HCLKSEL_Msk)) | CLK_CLKSEL0_HCLKSEL_HIRC;

    // Waiting for clock switching ok
    while (CLK->STATUS & CLK_STATUS_CLKSFAIL_Msk);

    CLK->PLLCTL = CLK_PLLCTL_PD_Msk; // Disable PLL
    CLK->PLLCTL = 0x8842E;           // Enable PLL & set frequency 192MHz

    while (!(CLK->STATUS & CLK_STATUS_PLLSTB_Msk));

    /* Enable External XTAL (4~24 MHz) */
    CLK->PWRCTL |= CLK_PWRCTL_HXTEN_Msk;

    while ((CLK->STATUS & CLK_STATUS_PLLSTB_Msk) != CLK_STATUS_PLLSTB_Msk);

    CLK->CLKDIV0 = CLK->CLKDIV0 & (~CLK_CLKDIV0_HCLKDIV_Msk);   /* PLL/1 */
    CLK->CLKSEL0 = (CLK->CLKSEL0 & (~CLK_CLKSEL0_HCLKSEL_Msk)) | CLK_CLKSEL0_HCLKSEL_PLL;
    /* Set both PCLK0 and PCLK1 as HCLK/2 */
    CLK->PCLKDIV = CLK_PCLKDIV_PCLK0DIV2 | CLK_PCLKDIV_PCLK1DIV2;
    SYS->USBPHY &= ~SYS_USBPHY_HSUSBROLE_Msk;    /* select HSUSBD */
    /* Enable USB PHY */
    SYS->USBPHY = (SYS->USBPHY & ~(SYS_USBPHY_HSUSBROLE_Msk | SYS_USBPHY_HSUSBACT_Msk)) | SYS_USBPHY_HSUSBEN_Msk;

    for (i = 0; i < 0x1000; i++);  // delay > 10 us

    SYS->USBPHY |= SYS_USBPHY_HSUSBACT_Msk;
    /* Enable IP clock */
    CLK->AHBCLK |= CLK_AHBCLK_HSUSBDCKEN_Msk;   /* USBD20 */
    PllClock        = FREQ_192MHZ;
    SystemCoreClock = FREQ_192MHZ;
    CyclesPerUs     = (SystemCoreClock + 500000UL) / 1000000UL; // For SYS_SysTickDelay()
}

extern uint8_t bUsbDataReady;
void USBD20_IRQHandler(void);

int32_t main(void)
{
		 ErrNo ret;
    SYS_Init();
    HSUSBD_ENABLE_PHY();

    /* wait PHY clock ready */
    while (1) {
        HSUSBD->EP[EPA].EPMPS = 0x20ul;

        if (HSUSBD->EP[EPA].EPMPS == 0x20ul) {
            break;
        }
    }

    /* Force SE0, and then clear it to connect*/
    HSUSBD_SET_SE0();
    /* Endpoint configuration */
    HID_Init();
    /* Enable USBD interrupt */
    NVIC_EnableIRQ(USBD20_IRQn);
    /* Start transaction */
    HSUSBD_CLR_SE0();
    ISP_Bridge_Init();		
				
		//ret=io_open(I2C_NAME_STRING, &DEV_handle);
		//ret=io_open(RS485_NAME_STRING, &DEV_handle);
		ret=io_open(SPI_NAME_STRING, &DEV_handle); //NO STABLE
		//ret=io_open(UART_NAME_STRING, &DEV_handle); //NO STABLE
		//ret=io_open(CAN_NAME_STRING, &DEV_handle); //NO STABLE
		init_ISP_command();
	 ret=FWVersion(&ISP_COMMAND,&fwversion);
	check_error(__LINE__, __FILE__, ret);
	printf("fw version:0x%x\n\r",fwversion);
	ret=GetDeviceID(&ISP_COMMAND,&devid);
	check_error(__LINE__, __FILE__, ret);
	printf("device id:0x%x\n\r",devid);
	ret=GetConfig(&ISP_COMMAND,config);
	check_error(__LINE__, __FILE__, ret);
	printf("config0: 0x%x\n\r", config[0]);
	printf("config1: 0x%x\n\r", config[1]);
	ret=GetFlashMode(&ISP_COMMAND,&flash_boot);
	check_error(__LINE__, __FILE__, ret);
	printf("device id:0x%x\n\r",devid);
	if (flash_boot != LDROM_MODE)
	{
		printf("boot in APROM\n\r");
	}
	else
	{
		printf("boot in LDROM\n\r");
	}
	
	ret=Updated_Target_Flash(&ISP_COMMAND,0,512);
	check_error(__LINE__, __FILE__, ret);
	
    while (1) {
       // ISP_Bridge_Main();
    }
}
