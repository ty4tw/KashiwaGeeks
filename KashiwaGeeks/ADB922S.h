/*
 * ADB922S.h
 * 
 *                 MIT License
 *      Copyright (c) 2017 Tomoaki Yamaguchi
 *
 *   This software is released under the MIT License.
 *   http://opensource.org/licenses/mit-license.php
 *
 *   Created on: 2017/11/25
 *       Author: tomoaki@tomy-tech.com
 *
 */

#ifndef ADB922S_H_
#define ADB922S_H_
#include <AppDefine.h>
#include <SoftwareSerial.h>
namespace tomyApplication
{
//
//  LoRaWAN defines
//
#ifndef LoRa_MAX_PAYLOAD_SIZE
#define LoRa_MAX_PAYLOAD_SIZE    64
#endif

#define LoRa_INIT_WAIT_TIME          1000
#define LoRa_SERIAL_WAIT_TIME    2000
#define LoRa_RECEIVE_DELAY2        5000
#define JOIN__WAIT_TIME              30000

#define LoRa_RC_SUCCESS                       0
#define LoRa_RC_DATA_TOO_LONG     -1
#define LoRa_RC_NOT_JOINED              -2
#define LoRa_RC_ERROR                         -3
#define LoRa_Rx_PIN                              11
#define LoRa_Tx_PIN                              12
#define LoRa_WAKEUP_PIN                   7

#ifdef SHOW_LORA_TRANSACTION
#define LoRaDebug(...)  DebugPrint(__VA_ARGS__)
#define ECHOFLAG  true
#else
#define LoRaDebug(...)
#define ECHOFLAG  false
#endif

#define PORT_LIST   PortList_t  thePortList[]
#define PORT(...)         {__VA_ARGS__}
#define END_OF_PORT_LIST  {0, 0}

typedef enum
{
    dr0, dr1, dr2, dr3, dr4, dr5, dr6
}LoRaDR;

typedef enum
{
    ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7, ch8, ch9, ch10, ch11, ch12, ch13, ch14, ch15, ch16
}CHID;

typedef  enum {
    joined, not_joined
}JoineStatus;

typedef struct PortList
{
    uint8_t port;
    void (*callback)(void);
} PortList_t;


class ADB922S
{
public:
    ADB922S(void);
    ~ADB922S(void);

    bool begin(uint32_t baudrate = 9600, uint8_t retryTx = 1 );
    bool connect(void);
    bool reconnect(void);
    int sendData(uint8_t port, bool echo, const __FlashStringHelper* format, ...);
    int sendBinary(uint8_t port, bool echo, uint8_t* data, uint8_t dataLen);
    int sendDataConfirm(uint8_t port, bool echo, const __FlashStringHelper* format, ...);
    int sendBinaryConfirm(uint8_t port, bool echo, uint8_t* data, uint8_t dataLen);
    uint8_t getDownLinkPort( void);
    String getDownLinkPayload(void);
    uint8_t getDownLinkBinaryData(uint8_t* data);
    String getDownLinkData(void);
    void checkDownLink(void);
    void sleep(void);
    void wakeup(void);
    void getHwModel(char* model, uint8_t length);
    void getVersion(char* version, uint8_t length);
    void getEUI(char* eui, uint8_t length);
    uint8_t getMaxPayloadSize(void);
    bool setTxRetryCount(uint8_t retry);
    uint8_t getTxRetryCount(void);

    bool setDr(LoRaDR dr);
    bool setAdr(bool onOff);
    bool setLinkCheck(void);
    bool saveConfig(void);

    uint8_t getDr(void);
    uint8_t getPwr(void);
    bool isAdrOn(void);
    int  getChPara(CHID chId);
    bool getChStat(CHID chId);
    int getDcBand(CHID bandId);
    uint16_t getUpcnt(void);
    uint16_t getDowncnt(void);

private:
    void clearCmd(void);
    int  checkBaudrate(uint32_t baudrate);
    bool connect(bool reconnect);
    bool isConnected(void);
    int transmitData(uint8_t port, bool echo, bool ack, const __FlashStringHelper* format, va_list args);
    int transmitBinaryData( uint8_t port, bool echo, bool ack, uint8_t* data, uint8_t dataLen);
    int send(String cmd, String resp1, String resp2, bool echo = false, uint32_t timeout = LoRa_INIT_WAIT_TIME, char* returnValue = 0, uint8_t len = 0);
    int send(const __FlashStringHelper* cmd, const __FlashStringHelper* resp1, const __FlashStringHelper* resp2, bool echo = false , uint32_t = 0, char* returnValue = 0, uint8_t len = 0);
    uint8_t ctoh(uint8_t ch);
    void addPort(uint8_t port, void (*)());

    SoftwareSerial*  _serialPort;
    uint32_t               _baudrate;
    JoineStatus         _joinStatus;
    uint8_t                 _txRetryCount;
    uint8_t                _maxPayloadSize;
    uint32_t              _txTimeoutValue;
    String                 _downLinkData;
    int                     _stat;
};

}
#endif /* ADB922S_H_ */
