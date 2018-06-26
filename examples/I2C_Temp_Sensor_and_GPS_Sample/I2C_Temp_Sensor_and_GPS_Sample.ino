#include <KashiwaGeeks.h>
#include <Wire.h>
#include "KGPS.h"

#define ECHO true

ADB922S LoRa;

KGPS gps;
uint8_t portGPS = 12;
uint8_t portTemp = 16;

//================================
//          Initialize Device Function
//================================
#define BPS_9600       9600
#define BPS_19200     19200
#define BPS_57600     57600
#define BPS_115200   115200


void start()
{
    /*  Setup console */
    ConsoleBegin(BPS_57600);

    //DisableDebug();

    /*
     * Enable Interrupt 0 & 1
     */
    EnableInt0();
    //EnableInt1();  // For ADB922S, CUT the pin3 of the Sheild.


    ConsolePrint(F("\n**** I2C_Temp_Sensor_and_GPS_Sample *****\n"));

    /*  setup Power save Devices */
    power_adc_disable();       // ADC converter
    power_spi_disable();       // SPI

    /*  setup the LoRaWAN device  */
    if ( LoRa.begin(BPS_19200, DR3) == false )
    {
        while(true)
        {
            LedOn();
            delay(300);
            LedOff();
            delay(300);
        }
    }

    /* setup the GPS */
    gps.begin(9600, 8, 9);
    ConsolePrint(F("Initilizing GPS\n"));
    while( !gps.isReady() ){ };

    /* setup I2C */
    Wire.begin();


    /*  join LoRaWAN */
    LoRa.join();
}


//========================================
// Functions to be executed periodically
//========================================
void taskTemp(void)
{
    sendTemp();
}
//========================================
//            Execution interval
//    TASK( function, initial offset, interval by minute )
//========================================
TASK_LIST = {
        TASK(taskTemp, 0, 10),
        END_OF_TASK_LIST
};


//================================
//          INT0 callbaks
//================================
void int0D2(void)
{
    DebugPrint(F("********* INT0 *******\n"));
    sendLocation();
}


//================================
//          Power save functions
//================================
void sleep(void)
{
    LoRa.sleep();
    DebugPrint(F("Sleep.\n"));
}
void wakeup(void)
{
    LoRa.wakeup();
    DebugPrint(F("Wakeup.\n"));
}


//================================
// GPS Function
//================================

void sendLocation(void)
{
    gps.wakeup();
    while( !gps.isReady() ){ };
    Payload* pl = gps.getPayload();
    if (pl)
    {
        LoRa.sendPayload(portGPS, ECHO, pl);
    }
}

//================================
// I2C ADT7410 Sensor Functions
//================================
void sendTemp(void)
{
  Wire.requestFrom(0x48,2);
  uint16_t val = Wire.read() << 8;
  val |= Wire.read();
  val >>= 3;    // convert to 13bit format
  int ival = (int)val;
  if ( val & (0x8000 >> 3) )
  {
    ival -= 8192;
  }
  float temp = (float)ival / 16.0;
  char buf[6];
  DebugPrint(F("Temp=%s [C]\n"), dtostrf(temp,3, 2, buf));

  Payload pl(LoRa.getMaxPayloadSize());
  pl.set_float(temp);
  LoRa.sendPayloadConfirm(portTemp, ECHO, &pl);
}

/*   End of Program  */
