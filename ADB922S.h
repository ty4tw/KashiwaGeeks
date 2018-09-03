/*
 * ADB922S.h
 *
 *                       The MIT License
 *
 *   Copyright (c) 2017-2018     Tomoaki Yamaguchi    tomoaki@tomy-tech.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef ADB922S_H_
#define ADB922S_H_
#include <Application.h>
#include <LoRaWANDef.h>
#include <Payload.h>
#include <SoftwareSerial.h>
namespace tomyApplication
{

class ADB922S
{
public:
    ADB922S(void);
    ~ADB922S(void);

    bool begin(uint32_t baudrate, LoRaDR dr = DR2, uint8_t retryTx = 1 , uint8_t retryJoin = 3 );
    bool join(void);
    bool isJoin(void);

    int sendData(uint8_t port, bool echo, const __FlashStringHelper* format, ...);
    int sendDataConfirm(uint8_t port, bool echo, const __FlashStringHelper* format, ...);
    int sendPayload(uint8_t port, bool echo, Payload* payload);
    int sendPayloadConfirm(uint8_t port, bool echo, Payload* payload);

    uint8_t getDownLinkPort( void);
    Payload* getDownLinkPayload(void);
    String getDownLinkData(void);
    void checkDownLink(void);

    bool setADR(bool onOff);
    bool setADRParams(uint8_t adrAckLimit, uint8_t adrAckDelay);
    bool setLinkCheck(void);
    bool setTxRetryCount(uint8_t retry);
    int setDr(LoRaDR dr);

    void getVersion(char* version, uint8_t length);
    uint8_t getMaxPayloadSize(void);
    void getEUI(char* eui, uint8_t length);     //  ADB922S ONLY
    void getHwModel(char* model, uint8_t length);      //  ADB922S ONLY
    uint8_t getTxRetryCount(void);
    uint8_t getMargin(void);
    uint8_t getNbGw(void);

    int reset(void);
    void sleep(void);
    void wakeup(void);

private:
    uint8_t getDr(void);
    uint8_t getPwrIndex(void);
    bool isAdrOn(void);
    int  getChPara(CHID chId);
    bool getChStat(CHID chId);
    int getDcBand(CHID bandId);
    uint16_t getUpcnt(void);
    uint16_t getDowncnt(void);
    bool saveConfig(void);
    int sendBinary(uint8_t port, bool echo, uint8_t* data, uint8_t dataLen);
    uint8_t getDownLinkBinaryData(uint8_t* data);
    void clearCmd(void);
    int  checkBaudrate(uint32_t baudrate);
    bool connect(bool reconnect);
    bool isConnected(void);
    int transmitString(uint8_t port, bool echo, bool ack, const __FlashStringHelper* format, va_list args);
    int transmitBinaryData( uint8_t port, bool echo, bool ack, uint8_t* data, uint8_t dataLen);
    int send(String cmd, String resp1, String resp2, bool echo = false, uint32_t timeout = LoRa_INIT_WAIT_TIME, char* returnValue = 0, uint8_t len = 0);
    int send(const __FlashStringHelper* cmd, const __FlashStringHelper* resp1, const __FlashStringHelper* resp2, bool echo = false , uint32_t = 0, char* returnValue = 0, uint8_t len = 0);
    uint8_t ctoh(uint8_t ch);
    void checkRecvData(char* buff, bool conf);
    void controlADR(void);
    bool checkADR(bool conf);
    bool setMaxPower(void);
    bool setLowerDr(void);
    bool setChStat(uint8_t ch, bool stat);

    SoftwareSerial*  _serialPort;
    uint32_t  _baudrate;
    JoineStatus  _joinStatus;
    uint8_t  _txRetryCount;
    uint8_t _joinRetryCount;
    uint8_t  _maxPayloadSize;
    uint32_t  _txTimeoutValue;
    String  _downLinkData;
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
    uint8_t _nbGw;
    uint8_t _margin;
    uint16_t _chStat;
};

}
#endif /* ADB922S_H_ */
