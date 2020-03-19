# NuLink2_ISPLink2 
NuLink2_ISPLink2 is an example code, that can do ISP function in NuLink2-Pro offline mode  
### operation steps
1. Program ISPLink2 FW to NuLink2-Pro board
1. Pop up a USB disk, format it
1. Put isp.lua and test.bin into the USB disk
1. Configure target chip to boot from LDROM with LDROM ISP code. LDROM code can be found in BSP, e.g. [M480 BSP link](https://github.com/OpenNuvoton/M480BSP/tree/master/SampleCode/ISP)

1. Connect bus between NuLink2-Pro and target board (e.g. UART CON6 pin1 and pin2 to target board UART, share the ground)
1. Press SW1 button of NuLink2-Pro, then press RESET button of target board, and it will connect and programming
1. Print log from CON4 UART, the progress will show

### For detail, please check "NuLink2-Pro OFFLINE ISP.PPT"

