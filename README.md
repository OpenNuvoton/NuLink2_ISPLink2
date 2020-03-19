# NuLink2_ISPLink2 
NuLink2_ISPLink2 is an example code, that can do ISP function in NuLink2-Pro offline mode  
### operation steps
1. Program ISPLink2 FW to NuLink2-Pro board
1. pop up a USB DISK, format it
1. Put isp.lua and TEST.BIN into DISK  
1. target board boot from LDROM, and with target code. can be found in BSP, e.g. [M480 BSP link](https://github.com/OpenNuvoton/M480BSP/tree/master/SampleCode/ISP)

1. Connect bus between NuLink2-Pro board and target chip (e.g. UART CON6 PIN1 and PIN2, to TARGET CHIP UART. share the ground
1. press SW1 of NuLink2-Pro, then press RESET button of target board, and it will connect and programming
1. print log from CON4 UART, the progress will show

### For detail, please check "NuLink2-Pro OFFLINE ISP.PPT"

