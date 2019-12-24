#define CMD_UPDATE_CONNECT  0x000000AE
#define CMD_UPDATE_APROM    0x000000A0
#define CMD_UPDATE_CONFIG   0x000000A1
#define CMD_READ_CONFIG     0x000000A2
#define CMD_ERASE_ALL       0x000000A3
#define CMD_SYNC_PACKNO     0x000000A4
#define CMD_GET_FWVER       0x000000A6
#define CMD_APROM_SIZE      0x000000AA
#define CMD_RUN_APROM       0x000000AB
#define CMD_RUN_LDROM       0x000000AC
#define CMD_RESET           0x000000AD

#define CMD_GET_DEVICEID    0x000000B1
#define CMD_SET_INTERFACE 0x000000BA
#define CMD_PROGRAM_WOERASE     0x000000C2
#define CMD_PROGRAM_WERASE      0x000000C3
#define CMD_READ_CHECKSUM       0x000000C8
#define CMD_WRITE_CHECKSUM      0x000000C9
#define CMD_GET_FLASHMODE       0x000000CA
#define CMD_CHIP_RESET    0x000000AD
#define APROM_MODE  1
#define LDROM_MODE  2

typedef signed int ErrNo;
#define PAGE_SIZE                      0x00000200     /* Page size */

#define PACKET_SIZE 64//32
#define FILE_BUFFER 128
struct sISP_COMMAND
{
    void (*ISPauto_detect_command)(void);
    ErrNo(*ISPSyncPackno)(void);
    ErrNo(*ISPFWVersion)(uint32_t *fwver);
    ErrNo(*ISPGetFlashMode)(uint32_t *mode);
    ErrNo(*ISPGetDeviceID)(uint32_t *devid);
    ErrNo(*ISPGetConfig)(uint32_t *config);
    ErrNo(*ISPUpdateConfig)(uint32_t *config);
    void (*ISPCmdRunLDROM)(void);
    void (*ISPCmdRunAPROM)(void);
    ErrNo(*ISPUpdateFlash)(uint32_t in_startaddr, uint32_t in_file_totallen);
};


extern void init_ISP_command(void);
extern int SyncPackno(struct sISP_COMMAND *ISP_COMMAND);
extern void Auto_Detect_Connect(struct sISP_COMMAND *gISP_COMMAND);
extern int FWVersion(struct sISP_COMMAND *gISP_COMMAND, uint32_t *buff);
extern int GetDeviceID(struct sISP_COMMAND *gISP_COMMAND, uint32_t *buff);
extern int GetConfig(struct sISP_COMMAND *gISP_COMMAND, uint32_t *buff);
extern int GetFlashMode(struct sISP_COMMAND *gISP_COMMAND, uint32_t *buff);
extern void RunAPROM(struct sISP_COMMAND *gISP_COMMAND);
extern void RunLDROM(struct sISP_COMMAND *gISP_COMMAND);
extern int Updated_Target_Flash(struct sISP_COMMAND *gISP_COMMAND, uint32_t in_startaddr, uint32_t in_file_totallen);

