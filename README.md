KashiwaGeeks
=================
LoRaWAN Application framework for Arduino.    
This framwork consists of Application,LoRaWAN devices and Payload Classes.    
These classes provide following functions:    
  1. Power saving with various sleep modes.     
  2. Wake up by watchdog timer.             
  3. Interrupt handling for INT0 and INT1.            
  4. Task execution control.       
  5. LoRaWAN devices control.    
  6. Allow payload to be created bit by bit.   
    

## 1. Application Class
Application Class methods are all wrapped up by the following functions:    
___   
    
**1) #include <KashiwaGeeks.h>**    
This statement declares the usage of The framework.    
___      
      
**2) void start(void)**    
This is the initial setup function used in place of the regular Arduino's setup() function.    
This is executed once only at the beginning of a program.    
___  
   
**3) void ConsoleBegin(uint32_t baudrate)**    
This function sets up console's baudrate in place of Serial.begin().    
___
    
**4) void ConsoleBegin(uint32_t baudrate,uint8_t RxPin, uint8_t TxPin)**    
This function sets up console's baudrate in place of SoftwareSerial.begin().    
___
    
**5) void ConsolePrint(format, ...)**   
This function is used in place of Serial.print() that outputs variable arguments to Serial port specified with the format.     
___
    
**6) void DebugPrint(format, ...)**   
Same as ConsolePrint()    
___    
    
**7) void DisableConsole(void)**   
This function terminates the output of ConsolePrint().   
___    
    
**8) void DisableDebug(void)**   
This function terminates the output of DebugPrint().   
To cut down power consumption while executing DisableDebug() and DisableCosole() at the same time, UART0 power is set off.    
___
    
**9) void LedOn(void), LedOff(void)**     
Turn Arduino's LED light on and off.    
___        
    
**10) void sleep(void)**   
This function is executed right before application goes to sleep. Any processes that are to be executed before sleep are programed in this function.    
___        
    
**11) void wakeup(void)**   
This function is executed right after application comes out of sleep.    
___    
    
**12) void int0D2(void)**   
This function is executed when digital pin2 becomes HIGH. After execution system goes back to sleep.    
___    
    
**13) void int1D3(void)**   
This function is executed when digital pin3 becomes HIGH. After execution system goes back to sleep.            
___    
    
**14) TASL_LIST={TASK(callback, startTime, interval),...,END_OF_TASK_LIST};**   
This statement repeatively executes the callback functions described in the list for a span of execution time(interval) specified. Start time can be set to start at differnt time for the process cycle does not overlap.  
Timer resolution is one minute.     
___    
    
**15) PORT_LIST={PORT(port, callback),...,END_OF_PORT_LIST};**    
This fucntion specifies process execution in accordance with the port from LoRaWAN DownLink data. This is used in CheckDownLink() method of LoRaWAN Device Class.    
___    
    
**16) ReRun(callback, uint32_t startTime)**    
This function executes the specified callback function after the time indicate in the startTime seconds.    
___    
    
    
## 2. ADB922S Class methods    
ADB922S represents an LoRaWAN Arduino Sheild which uses TLM922S device. Cut D2, D3 pins of the shield to be enabled the Interruptions(INT0 & INT1). Connect UNO's D2,D3 pins to the GND with switches.  
  
___    
    
**1) bool begin(uint32_t baudrate,LoRaDR dr, uint8_t retryTx = 1, uint8_t retryJoine = 1 );**    

**Function:** Initialize ADB922S    
**Parameter uint32_t bauderate:** bauderate of a sirial port, valid values are 9600, 19200, 57600, 115200.  
**Parameter LoRaDR dr:** Minimum DataRate.    
**Parameter uint8_t retryTx:** transmission retry count, valid value range from 0 to 255.    
**Parameter uint8_t retryJoin:** retry count of join.    
**Return Value:** Success true, fail false.    
___    
    

**2) bool join(void);**    

**Function:** Connect to the LoRaWAN. Keys ready or not, execute join.    
**Prameter:** None    
**Return value:** If joined once then true. Not able to join after retry to join, then false.   
___    
    
**3) uint8_t setDr(LoRaDR);**    

**Function:** Setup DR rate. DR rate defines FRMPayload length.    
**Parameter:** DR rate, dr0, dr1, dr2, dr3, dr4, dr5    
**Return Value:** FRMPayload length, setup fail the -1.    
___    
    
**4)  bool setADRParam(uint8_t adrAckLimit, uint8_t adrAckDelay);**
    
**Function:**  Setup ADR parameters and ADR enables        
**Parameter:** uint8_t adrAckLimit:** ADR_ACK_LIMIT.     
**Parameter:** uint8_tadrAckDelay:**  ADR_ACK_DELAY.
**Return Value:** ttrue Setup normal end, false Error   
___

**5) int sendData(uint8_t port, bool echo, const __FlashStringHelper &format, ...);**    

**Function:** Transmit String data. 11) getDownLinkData(void) can be used to check if the DownLink data is received.    
**Parameter uint8_t port:** Data transmitted is sent to the application specified by the port.    
**Parameter bool echo:** If true then data transmitted is outputed on console.   
**Parameter format:** Format of transmitted data. same as format used in printf(). %f is not available.     
**Parameter ... :** Variable arguments.    
**Return Value LORA_RC_SUCCESS:** when the transmition is normaly completed.    
**Retrun Value LORA_RC_DATA_TOO_LONG:** if the data transmitted is too long.    
**Return Value LORA_RC_NOT_JOINED:** if not joined.    
**Return Value LORA_RC_ERROR:** other errors.    
___    
    
**6) int sendDataConfirm(uint8_t port, bool echo, const __FlashStringHelper &format, ...);**    

**Function:** Transmit String data with confirmation request. 11) getDownLinkData(void) can be used to check if the DownLink data is received.   
**Parameter uint8_t port:** Data transmitted is sent to the application specified by the port.    
**Parameter bool echo:** If true then data transmitted is outputed on console.   
**Parameter format:** Format of transmitted data. same as format used in printf(). %f is not available.     
**Parameter ... :** Variable arguments.    
**Return Value LORA_RC_SUCCESS:** when the transmition is normaly completed.    
**Retrun Value LORA_RC_DATA_TOO_LONG:** if the data transmitted is too long.    
**Return Value LORA_RC_NOT_JOINED:** if not joined.    
**Return Value LORA_RC_ERROR:** other errors.    
___    
    
**7)  int sendPayload(uint8_t port, bool echo, &Payload);**    
    
**Function:** Transmit a Payload instance.  11) getDownLinkData(void) can be used to check if the DownLink data is received.     
**Parameter uint8_t port:** Data transmitted is sent to the application specified by the port.    
**Parameter bool echo:** If true then data transmitted is outputed on console.      
**Parameter Payload:** Pointer of a Payload instance.    
**Return Value LORA_RC_SUCCESS:** when the transmition is normaly completed.    
**Retrun Value LORA_RC_DATA_TOO_LONG:** if the data transmitted is too long.    
**Return Value LORA_RC_NOT_JOINED:** if not joined.    
**Return Value LORAa_RC_ERROR:** other errors.  
___
    
**8) int sendPayloadConfirm(uint8_t port, bool echo, &Payload);**    

**Function:** Transmit a Payload instance with confirmation request.  11) getDownLinkData(void) can be used to check if the DownLink data is received.     
**Parameter uint8_t port:** Data transmitted is sent to the application specified by the port.    
**Parameter bool echo:** If true then data transmitted is outputed on console.      
**Parameter Payload:** Pointer of a Payload instance.    
**Return Value LORA_RC_SUCCESS:** when the transmition is normaly completed.    
**Retrun Value LORA_RC_DATA_TOO_LONG:** if the data transmitted is too long.    
**Return Value LORA_RC_NOT_JOINED:** if not joined.    
**Return Value LORA_RC_ERROR:** other errors.    
___    
    
**9)  Payload &getDownLinkPayload(void);**    
    
**Function:** Acquire a Payload instance from the DownLink data previously recieved.    
**Parameter:** None    
**Return Value:** Pointer of the Payload instance.  0, if no DownLink data from the previous received.    
___
    
**10)  uint8_t getDownLinkPort( void);**    
    
**Function:** Acquire the port from previously received DownLink data.    
**Parameter:** None    
**Return Value:** Port(number), 0 if no data.    
___  
    
**11)  String getDownLinkData(void);**    
    
**Function:** Acquire the string data excludeding port(number) from previously received DownLink data.    
**Parameter:** None    
**Return Value:** String excluding Port. Null string if no data.    
___
    
**12) void sleep(void);**    
    
**Function:** Set to infinite deep sleep. ( D7Pin is used to wakeup)   
**Parameter:** None    
**Return value:** None    
___
    
**13) void wakeup(void);**    
     
**Function:** Wake up from infinte deep sleep.    
**Parameter:** None    
**Return value:** None    
___
    
**14)  void getHwModel(char &model, uint8_t length);**    
    
**Function:** Acquire the Model name of TLM922S.    
**Parameter char &model:** Specified  return address fro the Model name.    
**Parameter uint8_t length:** Maxmum length to obtain the Model name.    
**Return Value:** String of Model name in the length specified returned in model argument.    
___
    
**15)  void getVersion(char &version, uint8_t length);**    
      
**Function:** Acquire the version of TLM922S.   
**Parameter char &version:** Specified  return address fro the version.    
**Parameter uint8_t length:** Maxmum length to obtain the version.    
**Return Value:** String of version in the length specified returned in version argment.     
___
    
**16)  void getEUI(char &eui, uint8_t length);**    
    
**Function:** Acquire the DevEUI.    
**Parameter char &version:** Specified  return address fro the devEUI.    
**Parameter uint8_t length:** Maxmum length to obtain the devEUI.    
**Return Value:** String of version in the length specified returned in eui argment. 
___
    
**17)  uint8_t getMaxPayloadSize(void);**    
    
**Function:** Acquire the maximum transmittable payloadd length setten by setDR().    
**Parameter:** None    
**Return Value:** Transmittable Payload length.    
___
    
**18)  bool setTxRetryCount(uint8_t retry);**
    
**Function:** Set the transmission retry count. Valid value, 0 to 255.    
**Parameter:** retry counts.    
**Return Value:** True if normal complete. otherwise, false.    
____
    
**19)  uint8_t getTxRetryCount(void);**    
    
**Function:** Acquire the value of transmission retry count.    
**Parameter:** None    
**Return Value:** the value of transmission retry count 
___
    
**20)  void checkDownLink(void);**    
    
**Function:**  Excecutes the callback function that is linked to the specified port by PORT_LINK if DownLink data is received.    
**Parameter:** None    
**Return Value:** None    
___

    
    
## 3. Payload Calss methods    
Payload Class represents FRMPayload. Maximum FRMPayload is 242 bytes.     
To extend distance of transmission, Payload should be minimize as possible. (using Low DR rate)    
This Class efficiently creates the one-bit-bool and 4-bits, 1-byte, 2-bytes, 4-bytes integer and unsinged integer in a way that eliminate the byte boundary of bits stream.    
![image](https://user-images.githubusercontent.com/7830788/34324083-220a170c-e8a7-11e7-89d2-0ccbe2e26c2f.png)    

___
    
**1) Payload(uint8_t bytes);**
    
**Function:** Creates Payload.    
**Parameter uint8_t bytes:** length of the payload in byte.    
**Return Value:** Payload instance.    
____
    
**2) setters**    
```
// methods sample 
Payload pl(50);
pl.set_bool(true);
pl.set_int4((int8_t)-4);  // -8 ~ 7
pl.set_int8((int8_t) 129);
pl.set_uint8((uint8_t) 250);
pl.set_uint4((uint8_t) 15);
```
    
**3) getters** 
```
bool b1 = pl.get_bool();
int8_t i41 = pl.get_int4();
int8_t i81 = pl.get_int8();
uint8_t u81 = pl.get_uint8();
uint8_t u41 = pl.getuint4();
```

To retreive data, Payload data is retrived in storing order.    
For string up to 15 characters, set_string(String), get_string() can be used.    
For string longer than 15 characters, Payload can not be used.

```
void set_bool(bool);
void  set_int4(int8_t);
void set_int8(int8_t);
void set_int16(int16_t);
void set_int32(int32_t);
void set_float(float);
void set_uint4(uint8_t);
void set_uint8(uint8_t);
void set_uint16(uint16_t);
void set_uint24(uint32_t);
void set_uint32(uint32_t);
void set_string(String);

bool get_bool(void);
int8_t get_int4(void);
int8_t get_int8(void);
int16_t get_int16(void);
int32_t get_int32(void);
float get_float(void);
uint8_t get_uint4(void);
uint8_t get_uint8(void);
uint16_t get_uint16(void);
uint32_t get_uint24(void);
uint32_t get_uint32(void);
String get_string(void);
```


