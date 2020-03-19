local status, err = pcall(function()
File_name="0:\\test.bin" --bin file for isp aprom
temp=string.format("strage=0x%x",ISP_SHOW_STORAGE())
print(temp)

--ISP_AP_FILE_CHECKSUM for read file size
filesize=ISP_AP_FILE_SIZE(File_name)
if (filesize==0)
then
print("no file exit")
return
end
temp=string.format("APROM SIZE=%d",filesize)
print(temp)

--ISP_AP_FILE_CHECKSUM for read file size checksum
filechecksum=ISP_AP_FILE_CHECKSUM(File_name)
temp=string.format("APROM checksum=0x%x",filechecksum)
print(temp)
--initial interface 0 uart.
--initial interface 1 rs485
ISP_INTERFACE_INIT(0)
print("sync:")

--sync isp protocol package
if(ISP_CmdSyncPackno()==1)
then
print("sync false")
return
end
print("sync pass")

--read device id
devid=ISP_CmdGetDeviceID()
temp=string.format("devce id=0x%x",devid)
print(temp)

--read target chip isp fw version
tb=ISP_CmdFWVersion()
temp=(tb[1]<<24)|(tb[2]<<16)|(tb[3]<<8)|tb[4]
print(string.format("fw version=0x%x",temp))

--read config bit
tb=ISP_CmdGetConfig()
local config0=(tb[1]<<24)|(tb[2]<<16)|(tb[3]<<8)|tb[4]
print(string.format("config0=0x%x",config0))
config1=(tb[5]<<24)|(tb[6]<<16)|(tb[7]<<8)|tb[8]
print(string.format("config0=0x%x",config1))

--read target chip boot mode
if(ISP_CmdGetFlashMode()==2)
then
print("MCU in LDROM")
else
print("MCU in APROM")
end

--set program
ISP_SET_PFILE(File_name)

--do isp program flow
if(ISP_PROGRAM(0,filesize)==0)
then
print("program pass")
end

end)
if not status then
print(err);
end
