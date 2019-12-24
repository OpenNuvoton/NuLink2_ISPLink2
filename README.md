# NuLink2_ISPLink2
### operation steps
1. Program ISPLink2 FW to Nu-Link2 board
1. pop up a USB DISK, format it
1. Put DEFINE.TXT and TEST1.BIN into DISK  
1. target board boot from LDROM, and with target code. can be found in BSP, e.g. [M480 BSP link](https://github.com/OpenNuvoton/M480BSP/tree/master/SampleCode/ISP)

1. Connect bus between Nu-Link2 board and target chip (e.g. UART CON8 PIN1 and PIN2, to TARGET CHIP UART. share the ground.
1. press SW1 of Nu-Link2, then press RESET button of target board, and it will connect and programming.
1. the LED will blink while programming
1. print log from UART. connect CON4 (PIN 9) to Nu-Link2 (GND), CON4 (PIN 10) to UART RX
 

### DEFINE.TXT: configuration file 
#### Set bin file to target ROM
#### besides UART_BUS, there are I2C_BUS/SPI_BUS/CAN_BUS/RS485_BUS

    START    
    APROM=1    
    0:\\test1.bin    
    DATAFLASH=0    
    0:\\test2.bin    
    UART_BUS
    END    
