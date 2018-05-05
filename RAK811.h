/*
 * RAK811.h
 *
 *   Created on: 2017/12/15
 *       Author: tomoaki@tomy-tech.com
 *
 */

#ifndef LIBRARIES_KASHIWAGEEKS_RAK811_H_
#define LIBRARIES_KASHIWAGEEKS_RAK811_H_

#include <Application.h>
#include <Payload.h>

namespace tomyApplication
{
extern tomyApplication::SerialLog* theLog;
//
//  LoRaWAN defines
//
#define SOFTCONSOLE
#define LoRa_DEFAULT_PAYLOAD_SIZE      11

// Error Code
#define CODE_ARG_ERR    -1
#define CODE_ARG_NOT_FIND    -2
#define CODE_JOIN_ABP_ERR    -3
#define CODE_JOIN_OTAA_ERR    -4
#define CODE_NOT_JOIN    -5
#define CODE_MAC_BUSY_ERR    -6
#define CODE_TX_ERR    -7
#define CODE_INTER_ERR    -8
#define CODE_WR_CFG_ERR    -11
#define CODE_RD_CFG_ERR    -12
#define CODE_TX_LEN_LIMITE_ERR -13
#define CODE_UNKNOWN_ERR -20

// Event code
#define STATUS_RECV_DATA 0
#define STATUS_TX_COMFIRMED 1
#define STATUS_TX_UNCOMFIRMED 2
#define STATUS_JOINED_SUCCESS 3
#define STATUS_JOINED_FAILED 4
#define STATUS_TX_TIMEOUT 5
#define STATUS_RX2_TIMEOUT 6
#define STATUS_DOWNLINK_REPEATED 7
#define STATUS_WAKE_UP 8
#define STATUS_P2PTX_COMPLETE 9
#define STATUS_UNKNOWN 100

#define LoRa_RC_SUCCESS            0
#define LoRa_RC_DATA_TOO_LONG     -1
#define LoRa_RC_NOT_JOINED        -2
#define LORA_RC_NO_FREE_CH   -3
#define LoRa_RC_BUSY    -4
#define LoRa_RC_ERROR    -5

// Timeout values
#define LoRa_INIT_WAIT_TIME_RAK      2000
#define LoRa_SERIAL_WAIT_TIME_RAK    3000
#define LoRa_RECEIVE_DELAY2_RAK      5000
#define JOIN__WAIT_TIME_RAK         30000

// Debug log
#ifdef LORA_DEBUG
#define LoRaDebug(...)  DebugPrint( __VA_ARGS__)
#define ECHOFLAG  true
#else
#define LoRaDebug(...)
#define ECHOFLAG  false
#endif

#ifdef TEST_ADR
#define ADRDebug(...)  DebugPrint( __VA_ARGS__)
#else
#define ADRDebug(...)
#endif

#define MAX_NO_FREE_CH_CNT  5

#ifndef PORT_LIST
#define PORT_LIST   PortList_t  thePortList[]
#define PORT(...)         {__VA_ARGS__}
#define END_OF_PORT_LIST  {0, 0}


#define ADR_ACK_LIMIT     5
#define ADR_ACK_DELAY   2


typedef enum
{
    DR0, DR1, DR2, DR3, DR4, DR5
}LoRaDR;

typedef enum
{
    CH0, CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8, CH9, CH10, CH11, CH12, CH13, CH14, CH15, CH16
}CHID;

typedef  enum {
    joined, not_joined
}JoineStatus;

typedef struct PortList
{
    uint8_t port;
    void (*callback)(void);
} PortList_t;

#endif

//
//
//   RAK811
//
//
class RAK811
{
public:
    RAK811(void);
    ~RAK811(void);

    bool begin(uint32_t baudrate = 9600, LoRaDR dr = DR2);
    bool isConnect(void);
    bool join(void);

    int sendData(uint8_t port, bool echo, const __FlashStringHelper* format, ...);
    int sendDataConfirm(uint8_t port, bool echo, const __FlashStringHelper* format, ...);
    int sendPayload(uint8_t port, bool echo, Payload* payload);
    int sendPayloadConfirm(uint8_t port, bool echo, Payload* payload);

    uint8_t getDownLinkPort( void);
    Payload* getDownLinkPayload(void);
    String getDownLinkData(void);
    void checkDownLink(void);

    void sleep(void);
    void wakeup(void);

    String getVersion(void);
    uint8_t getMaxPayloadSize(void);

    int setDr(LoRaDR dr);
    int setADR(bool flg);
    bool setADRParams(uint8_t adrAckLimit, uint8_t adrAckDelay);
    bool setLinkCheck(void);     // ToDo:

    int reset(void);

private:
    int setConfig(String param);
    String getConfig(String key);     // RAK811 ONLY
    uint8_t getPwr(void);
    int getLinkCount(uint16_t* upCnt, uint16_t* dwnCnt);
    int setLinkCount(uint16_t upCnt, uint16_t dwnCnt);
    bool isConnected(void);
    int transmitString(bool echo, uint8_t port, bool confirm, const __FlashStringHelper* format, va_list args);
    int transmitBinaryData(bool echo,  uint8_t port, bool confirm, uint8_t* data, uint8_t dataLen);
    int sendCommand(bool echo, String cmd, String param, uint32_t timeout = LoRa_INIT_WAIT_TIME_RAK);
    int sendCommand(bool echo, const __FlashStringHelper* cmd, const __FlashStringHelper* param,  uint32_t timeout = LoRa_INIT_WAIT_TIME_RAK);
    int sendCommand(bool echo, const __FlashStringHelper* cmd, String paramparam,  uint32_t timeout = LoRa_INIT_WAIT_TIME_RAK);
    void recvResponse(uint32_t time);
    int recvEventResponse(uint32_t time = LoRa_SERIAL_WAIT_TIME_RAK);
    uint8_t getDownLinkBinaryData(uint8_t* data);
    uint8_t ctoh(uint8_t ch);

    uint8_t getDr(void);
    void checkRecvData(char* buff, bool conf); // ToDo:
    void controlADR(void);
    void setADRReqBit(void);
    bool setMaxPower(void);  // ToDo
    bool setLowerDr(void);
    bool setChStat(uint8_t ch, bool stat);  // ToDo

    HardwareSerial*  _serialPort;
    uint32_t  _baudrate;
    JoineStatus  _joinStatus;
    uint8_t  _maxPayloadSize;
    uint32_t  _txTimeoutValue;
    String  _resp;
    Payload _payload;
    int  _stat;
    bool _txOn;
    bool _adrOn;
    bool _adrReqBitOn;
    bool _maxPwrOn;
    LoRaDR _minDR;
    bool _minDROn;
    uint16_t _adrAckCnt;
    uint8_t _adrAckLimit;
    uint8_t _adrAckDelay;
    uint8_t _finalDelay;
    uint8_t _noFreeChCnt;
};

}



#endif /* LIBRARIES_KASHIWAGEEKS_RAK811_H_ */
