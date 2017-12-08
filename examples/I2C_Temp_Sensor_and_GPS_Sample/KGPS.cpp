/*
 * KGPS.cpp
 * 
 * COPYRIGHT(c) 2018  tomoaki  < tomoaki@tomy-tech.com >
 *
 *  Created on: 2018/02/04
 *      Author: tomoaki
 */


#include <KashiwaGeeks.h>
#include "KGPS.h"

using namespace tomyApplication;

KGPS::KGPS(void): _serial{0}
{
    _payload = new Payload(10);
}

KGPS::~KGPS(void)
{

}

/*
 *  Initial setup
 */
void KGPS::begin(uint32_t baudrate, uint8_t rxPin, uint8_t txPin)
{
    pinMode(rxPin, INPUT);
    _serial = new SoftwareSerial(rxPin, txPin);
    _serial->begin(baudrate);
    wakeup();
}

/*
 *  power Off
 */
void KGPS::wakeup(void)
{
    digitalWrite(10, HIGH);
}

/*
 * Power On
 */
void KGPS::sleep(void)
{
    digitalWrite(10, LOW);
}

/*
 * check the GPS ready to read
 */
bool KGPS::isReady(void)
{
    _serial->listen();
    uint32_t  tim = millis() + 3000;

    while (millis() < tim)
    {
        if (_serial->available() > 0)
        {
            char c = _serial->read();

            DebugPrint("%c", c);

            _gps.encode(c);

            if (_gps.location.isUpdated())
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

/*
 * get a location data as a Payload
 *
 * Laitude 3B, Longitude 3B, Altitude 2B,  Number of satellites 1B
 */
Payload* KGPS::getPayload(void)
{
  if ( isReady() )
  {
        uint8_t hdopGps = _gps.hdop.value()/10;
        uint32_t Latitude = ((_gps.location.lat() + 90) / 180.0) * 16777215;
        uint32_t Longitude = ((_gps.location.lng() + 180) / 360.0) * 16777215;
        uint16_t alt = _gps.altitude.meters();

        _payload->clear();
        _payload->set_uint24(Latitude);
        _payload->set_uint24(Longitude);
        _payload->set_uint16(alt);
        _payload->set_uint8(hdopGps);
        DebugPrint(F("Lat=%"PRIu32", Lon=%"PRIu32", Alt=%"PRIu16 ", %d\n"), Latitude, Longitude, alt, hdopGps);
        return _payload;
  }
  return 0;
}

