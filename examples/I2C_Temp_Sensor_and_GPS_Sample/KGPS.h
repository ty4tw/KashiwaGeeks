/*
 * KGPS.h
 * 
 * COPYRIGHT(c) 2018  tomoaki  < tomoaki@tomy-tech.com >
 *
 *  Created on: 2018/02/04
 *      Author: tomoaki
 */

#ifndef KGPS_H_
#define KGPS_H_
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

class KGPS
{
public:
    KGPS(void);
    ~KGPS(void);
    void begin(uint32_t baudrate, uint8_t rxPin, uint8_t txPin);
    bool isReady(void);
    void sleep(void);
    void wakeup(void);
    Payload* getPayload(void);
private:
    TinyGPSPlus _gps;
    SoftwareSerial*  _serial;
    Payload*  _payload;
};


#endif /* KGPS_H_ */
