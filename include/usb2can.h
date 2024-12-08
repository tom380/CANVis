#ifndef USB2CAN_H
#define USB2CAN_H

// Error codes

#define CANAL_ERROR_SUCCESS 0               // All is OK.
#define CANAL_ERROR_BAUDRATE 1              // Baudrate error.
#define CANAL_ERROR_BUS_OFF 2               // Bus off error
#define CANAL_ERROR_BUS_PASSIVE 3           // Bus Passive error
#define CANAL_ERROR_BUS_WARNING 4           // Bus warning error
#define CANAL_ERROR_CAN_ID 5                // Invalid CAN ID
#define CANAL_ERROR_CAN_MESSAGE 6           // Invalid CAN message
#define CANAL_ERROR_CHANNEL 7               // Invalid channel
#define CANAL_ERROR_FIFO_EMPTY 8            // Noting available to read. FIFO is empty
#define CANAL_ERROR_FIFO_FULL 9             // FIFO is full
#define CANAL_ERROR_FIFO_SIZE 10            // FIFO size error
#define CANAL_ERROR_FIFO_WAIT 11            //
#define CANAL_ERROR_GENERIC 12              // Generic error
#define CANAL_ERROR_HARDWARE 13             // A hardware related fault.
#define CANAL_ERROR_INIT_FAIL 14            // Initialization failed.
#define CANAL_ERROR_INIT_MISSING 15         //
#define CANAL_ERROR_INIT_READY 16           //
#define CANAL_ERROR_NOT_SUPPORTED 17        // Not supported.
#define CANAL_ERROR_OVERRUN 18              // Overrun.
#define CANAL_ERROR_RCV_EMPTY 19            // Receive buffer empty
#define CANAL_ERROR_REGISTER 20             // Register value error
#define CANAL_ERROR_TRM_FULL 21             //
#define CANAL_ERROR_ERRFRM_STUFF 22         // Errorframe: stuff error detected
#define CANAL_ERROR_ERRFRM_FORM 23          // Errorframe: form error detected
#define CANAL_ERROR_ERRFRM_ACK 24           // Errorframe: acknowledge error
#define CANAL_ERROR_ERRFRM_BIT1 25          // Errorframe: bit 1 error
#define CANAL_ERROR_ERRFRM_BIT0 26          // Errorframe: bit 0 error
#define CANAL_ERROR_ERRFRM_CRC 27           // Errorframe: CRC error
#define CANAL_ERROR_LIBRARY 28              // Unable to load library
#define CANAL_ERROR_PROCADDRESS 29          // Unable get library proc address
#define CANAL_ERROR_ONLY_ONE_INSTANCE 30    // Only one instance allowed
#define CANAL_ERROR_SUB_DRIVER 31           // Problem with sub driver call
#define CANAL_ERROR_TIMEOUT 32              // Blocking call timeout
#define CANAL_ERROR_NOT_OPEN 33             // The device is not open.
#define CANAL_ERROR_PARAMETER 34            // A parameter is invalid.
#define CANAL_ERROR_MEMORY 35               // Memory exhausted.
#define CANAL_ERROR_INTERNAL 36             // Some kind of internal program error
#define CANAL_ERROR_COMMUNICATION 37        // Some kind of communication error

// Flags
#define CANAL_FLAG_LOOPBACK 0x00000001
#define CANAL_FLAG_SILENT 0x00000002
#define CANAL_FLAG_NO_AUTO_RETRANS 0x00000004
#define CANAL_FLAG_STATUS_MSGS 0x00000008

// CANALMSG structure
struct CANALMSG {
    unsigned long flags;
    unsigned long obid;
    unsigned long id;
    unsigned char sizeData;
    unsigned char data[8];
    unsigned long timestamp;
};

// CANALSTATUS structure
struct CANALSTATUS {
    unsigned long channel_status;
};

// CANALSTATISTICS structure
struct CANALSTATISTICS {
    unsigned long cntReceiveFrames;
    unsigned long cntTransmitFrames;
    unsigned long cntReceiveData;
    unsigned long cntTransmitData;
    unsigned long cntOverruns;
    unsigned long cntBusWarnings;
    unsigned long cntBusOff;
};

// Function declarations
#ifdef __cplusplus
extern "C" {
#endif

long CanalOpen(const char* pConfigStr, unsigned long flags);
int CanalClose(long handle);
int CanalSend(long handle, const struct CANALMSG* pCanMsg);
int CanalBlockingSend(long handle, const struct CANALMSG* pCanMsg, unsigned long timeout);
int CanalReceive(long handle, struct CANALMSG* pCanMsg);
int CanalBlockingReceive(long handle, struct CANALMSG* pCanMsg, unsigned long timeout);
int CanalDataAvailable(long handle);
int CanalGetStatus(long handle, struct CANALSTATUS* pCanStatus);
int CanalGetStatistics(long handle, struct CANALSTATISTICS* pCanalStatistics);
int CanalSetFilter(long handle, unsigned long filter);
int CanalSetMask(long handle, unsigned long mask);
int CanalSetBaudrate(long handle, unsigned long baudrate);
unsigned long CanalGetVersion(void);
unsigned long CanalGetDllVersion(void);
const char* CanalGetVendorString(void);

#ifdef __cplusplus
}
#endif

#endif // USB2CAN_H