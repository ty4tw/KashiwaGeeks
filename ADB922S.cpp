/*
 * ADB922S.cpp
 *
 *   Created on: 2017/11/25
 *       Author: tomoaki@tomy-tech.com
 *
 */

#include <Application.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <stdarg.h>
#include <ADB922S.h>
#include <Payload.h>
#include <AppDefine.h>

using namespace tomyApplication;
extern int getFreeMemory(void);
extern PortList_t thePortList[];

const char* loraTxUnconfirmCmd = "lorawan tx ucnf";
const char* loraTxConfirmCmd = "lorawan tx cnf";

//
//
//     Class ADB922S
//
//

ADB922S::ADB922S(void):
        _baudrate{9600}, _joinStatus{not_joined}, _txTimeoutValue{LoRa_RECEIVE_DELAY2}, _stat{0}, _txOn{false}, _minDR{DR2},
        _minDROn{false}, _adrAckCnt{0}, _adrReqBitOn{false}, _adrAckDelay{ADR_ACK_DELAY}, _adrAckLimit{ADR_ACK_LIMIT}, _noFreeChCnt{0},
        _nbGw{0}, _margin{0}, _chStat{0}, _joinRetryCount{0}
{
    _serialPort = new SoftwareSerial(LoRa_Rx_PIN, LoRa_Tx_PIN);
    _txRetryCount = 1;
    _maxPayloadSize = LoRa_DEFAULT_PAYLOAD_SIZE;
}

ADB922S::~ADB922S(void)
{

}


bool ADB922S::begin(uint32_t baudrate, LoRaDR dr, uint8_t txRetry, uint8_t joinRetry )
{
    pinMode(LoRa_Rx_PIN, INPUT);
    pinMode(LoRa_WAKEUP_PIN, OUTPUT);
    _baudrate = baudrate;
    _txTimeoutValue = LoRa_RECEIVE_DELAY2;
    _serialPort->setTimeout(LoRa_SERIAL_WAIT_TIME);
    _minDR = dr;
    _txRetryCount = txRetry;
    _joinRetryCount = joinRetry;

    for ( uint8_t retry = 0; retry < 3; retry++ )
    {
        if ( checkBaudrate(_baudrate) <  0 )
        {
            char  cmd[24];
           uint32_t  br[] = { 9600, 19200, 57600, 115200};

           for ( uint8_t i = 0; i < 4; i++ )
           {
               if ( checkBaudrate(br[i]) == 0 )
               {
                   LoRaDebug( F("Set Baudrate to %ld\n"), baudrate);
                   String fmt = F("mod set_baudrate %ld");
                   sprintf(cmd, fmt.c_str(),  baudrate);
                   send(cmd, F(""),F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME);
                   _serialPort->begin(_baudrate);
                   if (send(F("mod set_echo off"), F("Ok"),F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME) != LORA_RC_SUCCESS )
                   {
                       return false;
                   }
                   else
                   {
                      goto success_exit;
                   }
               }
           }
        }
        else
        {
            goto success_exit;
        }
    }
    return false;

success_exit:
    setDr(dr);
    setTxRetryCount(_txRetryCount);
    if ( send(F("mod save"), F("Ok"), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME) == LORA_RC_SUCCESS )
    {
          return true;
     }
    else
    {
        return false;
    }
}


void ADB922S::clearCmd(void)
{
    _serialPort->listen();
    send(F("\n"), F(""),F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME);
}


int  ADB922S::checkBaudrate(uint32_t baudrate)
{
    LoRaDebug( F("Baudrate = %ld\n"), baudrate);
    _serialPort->begin(baudrate);
    clearCmd();
    return reset();
}


bool ADB922S::setTxRetryCount(uint8_t retry)
{
    char  cmd[26];
    String fmt = F("lorawan set_txretry %d");
    sprintf(cmd, fmt.c_str(),  retry);
    if ( send(cmd, F("Ok"), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME) == 0 )
    {
        _txRetryCount = retry;
        _txTimeoutValue = (uint32_t)(_txRetryCount + 1) * LoRa_RECEIVE_DELAY2;
        return true;
    }
    return false;
}

bool ADB922S::checkADR(bool confirm)
{
        if ( confirm || _adrReqBitOn )
        {
            return true;
        }
        else
        {
            return false;
        }
}



uint8_t ADB922S::getTxRetryCount(void)
{
    char count[4];
    send( F("lorawan get_txretry"), F(""), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME, count, 3);
    return (uint8_t) atoi(count);
}

void ADB922S::sleep(void)
{
    digitalWrite(LoRa_WAKEUP_PIN, LOW);
    send( F("mod sleep 1 1 0"), F(""), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME);
}

void ADB922S::wakeup(void)
{
    digitalWrite(LoRa_WAKEUP_PIN, HIGH);
    digitalWrite(LoRa_WAKEUP_PIN, LOW);
    clearCmd();
}

bool ADB922S::isJoin(void)
{
    return connect(false);
}

bool ADB922S::join(void)
{
    for ( uint8_t i = 0; i < _joinRetryCount; i++ )
    {
        DebugPrint(F("try to join... "));
        if ( connect(true) )
        {
            _chStat = 0;
            for ( uint8_t j = 0; j < 16; j++ )
            {
                _chStat = _chStat << 1;
                if ( getChStat(j) )
                {
                   _chStat |= 0x00001;
                }
            }
            DebugPrint(F("accepted.\n"));
            return true;
        }
        for ( uint8_t j = 0; j < 40 && i == _joinRetryCount; j++ )
        {
            delay(1000);  // Duty 1%
        }

    }
    DebugPrint(F("fail to join.\n"));
    return false;
}

bool ADB922S::connect(bool reconnect)
{
    String cmd;
    int rc = 0;

    clearCmd();

    if(!reconnect)
    {
        rc = send(F("lorawan get_join_status"), F("joined"),F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME);
        if ( rc == LORA_RC_SUCCESS )
        {
            _joinStatus = joined;
            return true;
        }
        _joinStatus = not_joined;
    }

    //
    // LoRa module join to Network Server by OTAA
    //
    if ( send(F("lorawan join otaa"), F("accepted"), F(""), ECHOFLAG, JOIN__WAIT_TIME )  != LORA_RC_SUCCESS )
    {
        _joinStatus = not_joined;
        return false;
    }

    _joinStatus = joined;
    return true;
}

//
//
//    Getters
//
//

void ADB922S::getHwModel(char* val, uint8_t valLen)
{
    send(F("mod get_hw_model"), F(""),F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME, val, valLen);
}

void ADB922S::getVersion(char* val, uint8_t valLen)
{
    send(F("mod get_ver"), F(""), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME, val, valLen);
}

void ADB922S::getEUI(char* val, uint8_t valLen)
{
    send(F("lorawan get_deveui"), F(""), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME, val, valLen);
}

uint8_t ADB922S::getMaxPayloadSize(void)
{
    return _maxPayloadSize;
}


//
//
//  sendCmd( )
//  Return value: LoRa_RV_SUCCESS, LoRa_RV_DATA_TOO_LONG, LoRa_RV_NOT_JOINED, LoRa_RV_ERROR
//
//
int ADB922S::send(String cmd, String resp1, String resp2 , bool echo, uint32_t timeout, char* returnVal, uint8_t len)
{
    uint32_t  tim = 0;
    uint32_t  time = (timeout == 0 ? _txTimeoutValue : timeout );
    String resp;
    int rc = 0;
    String  endCmd;

    _serialPort->listen();

    if ( echo )
    {
        ConsolePrint(F("\nSend  =>%s<=\n"), cmd.c_str());
    }

    if (cmd.indexOf(F("mod reset")) >= 0 )
    {
        endCmd =F("\n\r\n\r> ");
    }
    else
    {
        endCmd = F("\r> ");
    }

    _serialPort->print(cmd);
    _serialPort->print(F("\r"));
    _serialPort->flush();
    delay(200);

    // Recive a response from the module.
    tim = millis() + time + 100;
    while (millis() < tim)
    {
        if (_serialPort->available() > 0)
        {
          char ch = _serialPort->read();
          resp += String(ch);
          LoRaDebug(F("%02x "), ch);
          if ( resp.indexOf(endCmd) >= 0 )
          {
              break;
          }
        }
    }

    if ( echo )
    {
        ConsolePrint(F("\nRecv  =>%s<=\n"), resp.c_str());
    }
    // set the response to a specified area if required.

    if ( returnVal && len )
    {
        int idx1 = 0;
        int idx2 = 0;
        int idx3 = 0;

        if ( ( idx1 = resp.indexOf(F(">>"))) > 0 )
        {
            // check LinkCheck Response because it's longer than payloadLen.
            if ( (idx2 = resp.indexOf(F("DemodMargin") )) > 0 )
            {
                if ( (idx3 = resp.indexOf(F("NbGateways"))) > 0 )
                {
                    _margin = (uint8_t)resp.substring( idx2 + 13, idx3 - 3).toInt();
                    if ( (idx2 = resp.indexOf(F(">>"), idx3)) > idx3 )
                    {
                        _nbGw = (uint8_t)resp.substring( idx3 + 12, idx2 - 2).toInt();
                    }
                }

                //  set  ADR_ACK_CNT  (-1)
                _adrAckCnt = 65535;    // (uint16_t ) 65535  = (int ) -1
                _minDROn = false;
                return LORA_RC_SUCCESS;
            }

            if (  (idx2 = resp.indexOf(F(">>"), idx1 + 2)) > idx1 )
            {
                idx1 = idx2;
            }
            if ( (idx2 = resp.indexOf(F("\r"), idx1)) > idx1 )
            {
                memset(returnVal, 0, len);
                strncpy(returnVal, resp.substring(idx1 + 3, idx2 - 1).c_str(), len - 1);
                LoRaDebug(F("Recv  =>%s<=\n"), returnVal);
            }
        }
    }

    // Check the response with specified response string.

    if ( resp1 == ""  && resp2 == "" )
    {
        rc = LORA_RC_SUCCESS;
    }
    else if (resp.indexOf(resp1) >= 0 ||  ( resp2 != "" && resp.indexOf(resp2) >= 0 ))
    {
        _noFreeChCnt = 0;
        rc = LORA_RC_SUCCESS;


    }
    else if (resp.indexOf(F("not_joined")) >= 0 )
    {
        _joinStatus = not_joined;
        rc = LORA_RC_NOT_JOINED;
        DebugPrint(F("Data too long\n"));
    }
    else if (resp.indexOf(F("no_free_ch")) >= 0 )
    {
       DebugPrint(F("not joined\n"));
       if ( _noFreeChCnt > MAX_NO_FREE_CH_CNT)
       {
           reset();
           LoRaDebug(F(" Reset\n"));
           _joinStatus = not_joined;
           _noFreeChCnt = 0;
           rc = LORA_RC_NOT_JOINED;
       }
       else
       {
           _noFreeChCnt++;
           rc = LORA_RC_NO_FREE_CH;
           DebugPrint(F("no free ch.\n"));
       }
    }
    else if (resp.indexOf(F("invalid_data_length")) >= 0 )
    {
        rc = LORA_RC_DATA_TOO_LONG;
        DebugPrint(F("Data too long\n"));
    }
    else if (resp.indexOf(F("busy")) >= 0 )
    {
        rc = LORA_RC_BUSY;
        DebugPrint(F("busy\n"));
    }
    else
    {
        rc = LORA_RC_ERROR;
        DebugPrint(F("no resp\n"));
    }
#ifdef FREE_MEMORY_CHECK
    ConsolePrint(F("Free RAM is %d bytes\n"), getFreeMemory());
#endif
    return rc;
}

int ADB922S::send(const __FlashStringHelper* cmd, const __FlashStringHelper* resp1, const __FlashStringHelper* resp2, bool echo, uint32_t timeout, char* returnVal, uint8_t len)
{
    String scmd = String(cmd);
    String sresp1 = String(resp1);
    String sresp2 = String(resp2);
    int rc = send(scmd, sresp1, sresp2, echo, timeout, returnVal, len);
    return rc;
}

//
//
//  sendData( )
//  Return value: LoRa_RV_SUCCESS, LoRa_RV_DATA_TOO_LONG, LoRa_RV_NOT_JOINED, LoRa_RV_ERROR
//
//
//#define LORA_BUFF_LEN (20 + _maxPayloadSize+1 +1 )    // last 1 is for checking data length
//#define LORA_BINARY_BUFF_LEN (20 + _maxPayloadSize*2+1 +1 )

int ADB922S::sendData(uint8_t port, bool echo, const __FlashStringHelper* format,  ...)
{
    va_list args;
    va_start(args, format);
    int rc = transmitString(port, echo, false, format, args);
    va_end(args);

    return rc;
}


int ADB922S::sendDataConfirm(uint8_t port, bool echo, const __FlashStringHelper* format,  ...)
{
    va_list args;
    va_start(args, format);
    int rc = transmitString(port, echo, true, format, args);
    va_end(args);

    return rc;
}

int ADB922S::sendPayload(uint8_t port, bool echo, Payload* payload)
{
    return transmitBinaryData(port, echo, false, payload->getRowData(), payload->getLen());
}


int ADB922S::sendPayloadConfirm(uint8_t port, bool echo, Payload* payload)
{
    return transmitBinaryData(port, echo, true, payload->getRowData(), payload->getLen());
}

int ADB922S::transmitString(uint8_t port, bool echo, bool confirm, const __FlashStringHelper* format, va_list args)
{
    char data[(22 + _maxPayloadSize * 2 )  ];
    memset(data, 0, (22 + _maxPayloadSize * 2)  );
    char buffer[(22 + _maxPayloadSize )  ];
    char* pos = 0;
    int len = 0;

    if ( _joinStatus == not_joined )
    {
        LoRaDebug(F("Module is not joined.  try to join.\n"));
        if ( !join() )
        {
            _stat = LORA_RC_NOT_JOINED;
            goto exit;
        }
    }

    // ADR control
    controlADR();
    confirm = checkADR(confirm);

    if (confirm)
    {
        strcpy(data, loraTxConfirmCmd);
         pos = data  + strlen(loraTxConfirmCmd);
    }
    else
    {
        strcpy(data, loraTxUnconfirmCmd);
         pos = data  +strlen(loraTxUnconfirmCmd);
    }

    sprintf(pos, " %d ", port);
    len = strlen(data);
    pos = data + len;

    vsnprintf_P(pos, sizeof(data) - len, (const char*)format, args);
    _stat = send(data, "tx_ok", "rx", echo, _txTimeoutValue, buffer, (20 + _maxPayloadSize+1 +1 )  );
    checkRecvData(buffer, confirm);
exit:
    return _stat;
}



int ADB922S::transmitBinaryData(uint8_t port, bool echo, bool confirm, uint8_t* binaryData, uint8_t dataLen)
{
    uint8_t*  bd = binaryData;
    size_t  len = 0;
    char* pos = 0;
    char data[(22 + _maxPayloadSize*2 )];
    char buffer[(22 + _maxPayloadSize*2 )];

    if ( _joinStatus == not_joined )
    {
        LoRaDebug(F("Module is not joined.  try to join.\n"));
        if ( !join() )
        {
            _stat = LORA_RC_NOT_JOINED;
            goto exit;
        }
    }

    // ADR control
    controlADR();
    confirm = checkADR(confirm);

    memset(data, 0, (22 + _maxPayloadSize*2 ));

    if (confirm)
    {
        strcpy(data, loraTxConfirmCmd);
         len = strlen(loraTxConfirmCmd);
         pos = data  + len;
    }
    else
    {
        strcpy(data, loraTxUnconfirmCmd);
         len = strlen(loraTxUnconfirmCmd);
         pos = data  + len;
    }

    sprintf(pos, " %d ", port);
    len = strlen(data);
    pos = data + len;
    for ( uint8_t i = 0; i < dataLen; i++, bd++ )
    {
        sprintf(pos, "%02x", *bd);
        pos = data + strlen(data);
    }

    _stat = send(data, F("tx_ok"), F("rx"), echo, _txTimeoutValue, buffer, (20 + _maxPayloadSize*2+1 +1 ) );
    checkRecvData(buffer, confirm);
exit:
    return _stat;
}

void ADB922S::checkRecvData(char* buff, bool conf)
{
    if ( _stat == LORA_RC_SUCCESS )
    {
        if ( conf )
        {
            _adrAckCnt = 0;
            _adrReqBitOn = false;
            _minDROn = false;
        }
        else
        {
            if ( strncmp(buff, "DemodMargin =", 13 ) == 0)
           {
                _adrAckCnt = 0;
                _minDROn = false;
           }
            else
            {
                _adrAckCnt++;
            }
        }

        if ( strncmp(buff, "rx", 2) == 0 )
        {
            _downLinkData = String(buff + 3);
           _txOn= true;
           return;
        }
    }
    else if ( _stat == LORA_RC_ERROR )
    {
        _adrAckCnt++;
    }
    _downLinkData = F("");
    _txOn = false;
}

void ADB922S::controlADR(void)
{
    ADRDebug(F(" \n-----ADR Info-----\nADR_ACK_CNT=%d\n"), _adrAckCnt);

    if ( _adrOn  && _adrAckCnt >= _adrAckLimit )
    {
        _adrReqBitOn = true;

        if ( _adrAckCnt  < _adrAckLimit + _adrAckDelay )
       {
            _maxPwrOn = false;
       }

        if ( _adrAckCnt >= _adrAckLimit )
        {
            _adrReqBitOn = true;
        }

set_DR_Lower:
        if ( (_adrAckCnt == _adrAckLimit + 5 * _adrAckDelay  && !_minDROn) ||
              (_adrAckCnt == _adrAckLimit + 4 * _adrAckDelay  && !_minDROn) ||
              (_adrAckCnt == _adrAckLimit + 3 * _adrAckDelay  && !_minDROn) ||
              (_adrAckCnt == _adrAckLimit + 2 * _adrAckDelay  && !_minDROn)  )
        {
            _minDROn = setLowerDr();
            if ( _minDROn )
            {
                _finalDelay = 0;
            }
        }

        if ( _minDROn )
        {
            if ( _finalDelay < _adrAckDelay )
            {
                _finalDelay++;
            }
            else
            {
                ADRDebug(F("Enable Channels\n"));

                if ( _adrReqBitOn && _finalDelay == _adrAckDelay )
                {
                    _finalDelay++;
                    uint16_t stats = _chStat;
                    for ( uint8_t  ch  = 0; ch < 16; ch++ )
                    {
                        bool  chs = stats & 0x0001;
                        stats = stats >> 1;
                        ADRDebug(F("Ch%02d:%s\n"), 15 - ch, (chs ? "ON": "OFF"));
                        setChStat(15 - ch, chs);   // ToDo:   uncomment
                    }
                }
                _adrReqBitOn = false;
            }
        }

        if ( _adrAckCnt == _adrAckLimit + _adrAckDelay  && !_maxPwrOn)
        {
            if ( getPwrIndex() == 0 )
            {
                ADRDebug(F("Power is max\n"));
                _adrAckCnt += _adrAckDelay;
                _maxPwrOn = true;
                goto set_DR_Lower;
            }
            else
            {
                _maxPwrOn = setMaxPower();
            }
        }

        if ( _adrReqBitOn )
        {
            ADRDebug(F("Set ADRReqBit ON\n"));
        }
    }
    ADRDebug(F("-------------------\n"));
}

Payload* ADB922S::getDownLinkPayload(void)
{
    _payload.create(_maxPayloadSize);
    getDownLinkBinaryData(_payload.getRowData());
    return &_payload;
}

uint8_t ADB922S::getDownLinkPort( void)
{
    int epos = _downLinkData.lastIndexOf(F(" "));
    if ( epos >= 0 )
    {
        String port =  _downLinkData.substring(0, epos);
        return atoi(port.c_str());
    }
    return 0;
}


uint8_t ADB922S::getDownLinkBinaryData(uint8_t* data)
{
    int bpos = _downLinkData.indexOf(F(" "));
       if ( bpos >= 0 )
       {
           uint8_t i = 0;
           for (bpos++ ; bpos < (uint8_t)_downLinkData.length(); bpos += 2 )
           {
               data[i] = ctoh(_downLinkData[bpos]);
               data[i] = data[i] << 4;
               data[i++]  |= ctoh(_downLinkData[bpos + 1]);
           }
           return i;
       }
       return 0;
}

String ADB922S::getDownLinkData(void)
{
    int bpos = _downLinkData.indexOf(F(" "));
    if ( bpos >= 0 )
    {
        return _downLinkData.substring(bpos + 1);
    }
    return F("");
}

uint8_t ADB922S::ctoh(uint8_t ch)
{
    if ( ch >= 0x30 && ch <=0x39 )
    {
        return ch - 0x30;
    }
    else if ( ch >= 0x41 && ch <= 0x46 )
    {
        return ch - 0x37;
    }
    return 0;
}


int ADB922S::setDr(LoRaDR dr)
{
    char  cmd[18];
    String fmt = F("lorawan set_dr %d");
    sprintf(cmd, fmt.c_str(),  dr);
    if ( send(cmd, F("Ok"), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME) == 0 )
    {
        uint8_t pll[] = { 0, 0, 11, 53, 125, 242 };
        _maxPayloadSize = pll[dr];
        return _maxPayloadSize;
    }
    return -1;
}

bool ADB922S::setLowerDr(void )
{
    uint8_t dr = getDr();

    if (dr <= (uint8_t) _minDR )
    {
        ADRDebug(F("DR is min %d\n"),dr);
        return true;
    }
    else
    {
        setDr((LoRaDR)--dr);
        ADRDebug(F("Set DR=%d\n"), dr);
        return false;
    }
}

bool ADB922S::setMaxPower(void)
{
    char  cmd[20];
    ADRDebug(F("Set Power Max\n"));
    String cdata = F("lorawan set_power 5");
    sprintf(cmd, cdata.c_str());
    if ( send(cmd, F("Ok"), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME) == 0 )
    {
        return true;
    }
    _maxPwrOn = false;
}

bool ADB922S::setADR(bool onOff)
{
    char  cmd[20];
    const char* stat = (onOff ? "on" : "off");
    String fmt = F("lorawan set_adr %s");
    sprintf(cmd, fmt.c_str(),  stat);
    if ( send(cmd, F("Ok"), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME) == 0 )
    {
        _adrOn = true;
        _adrAckCnt = 0;
        _finalDelay = 0;
        return true;
    }
    _adrOn = false;
    _adrAckCnt = 0;
    return false;
}

bool ADB922S::setADRParams(uint8_t limit, uint8_t delay)
{
    _adrAckLimit = limit;
    _adrAckDelay = delay;
    setADR(true);
}

bool ADB922S::setChStat(uint8_t ch, bool onOff)
{
    char  cmd[30];
    const char* stat = (onOff ? "on" : "off");
    String fmt = F("lorawan set_ch_status %d %s");
    sprintf(cmd, fmt.c_str(),  ch, stat);
    if ( send(cmd, F("Ok"), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME) == 0 )
    {
        return true;
    }
    return false;
}

bool ADB922S::setLinkCheck(void)
{
    _margin = _nbGw = 0;
    if ( send(F("lorawan set_linkchk"), F("Ok"), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME) == 0 )
    {
        return true;
    }
    return false;
}

bool ADB922S::saveConfig(void)
{
    if ( send(F("lorawan save"), F("Ok"), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME) == 0 )
    {
        return true;
    }
    return false;
}

uint8_t ADB922S::getDr(void)
{
    char dr[3];
    send( F("lorawan get_dr"), F(""), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME, dr, 2);
    return (uint8_t) atoi(dr);
}

uint8_t ADB922S::getPwrIndex(void)
{
    char pwr[3];
     send( F("lorawan get_pwr"), F(""), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME, pwr, 2);
     return (uint8_t) atoi(pwr);
}

uint8_t ADB922S::getMargin(void)
{
    return _margin;
}

 uint8_t ADB922S::getNbGw(void)
 {
     return _nbGw;
 }

bool ADB922S::isAdrOn(void)
{
    char stat[5];
     send( F("lorawan get_adr"), F(""), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME, stat, 4);
     if ( strcmp(stat, "on") == 0 )
     {
         _adrOn = true;
         return true;
     }
     _adrOn = false;
     return false;
}

int ADB922S::getChPara(CHID chId)
{
    char  cmd[24];
    String fmt = F("lorawan get_ch_para %d");
    sprintf(cmd, fmt.c_str(),  chId);
    send(cmd, F("Ok"), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME);
    return 1;
}

bool ADB922S::getChStat(CHID chId)
{
    char stat[5];
    char  cmd[26];
    String fmt = F("lorawan get_ch_status %d");
    sprintf(cmd, fmt.c_str(),  chId);
    send( cmd, F(""), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME, stat, 4);
     if ( strcmp(stat, "on") == 0 )
     {
         return true;
     }
     return false;
}

int ADB922S::getDcBand(CHID bandId)
{
    char  cmd[24];
    String fmt = F("lorawan get_dc_band %d");
    sprintf(cmd, fmt.c_str(),  bandId);
    send( cmd, F(""), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME);
     return 0;
}


/* get a uplink counter that will be used at next transmission.*/
uint16_t ADB922S::getUpcnt(void)
{
    char cnt[6];
    send( F("lorawan get_upcnt"), F(""), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME, cnt, 5);
    return (uint8_t) atoi(cnt);
}

uint16_t ADB922S::getDowncnt(void)
{
    char cnt[6];
    send( F("lorawan get_downcnt"), F(""), F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME, cnt, 5);
    return (uint8_t) atoi(cnt);
}


void ADB922S::checkDownLink(void)
{
    if ( _stat == LORA_RC_SUCCESS && _txOn == true )
    {
        uint8_t port = getDownLinkPort();
        if ( port )
        {
            for ( uint8_t i = 0; thePortList[i].port != 0;  i++ )
            {
                if ( port == thePortList[i].port  )
                {
                    thePortList[i].callback();
                    break;
                }
            }
        }
    }
    _txOn = false;
}

int ADB922S::reset(void)
{
    return send(F("mod reset"), F("TLM922S"),F(""), ECHOFLAG, LoRa_INIT_WAIT_TIME);
}



