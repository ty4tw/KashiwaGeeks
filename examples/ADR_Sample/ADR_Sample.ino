/****************************************************************
 *
 *            I M P O R T A N T    N O T I C E
 *
 * 1. Uncomment the line17 of AppDefine.h in the library
 * 2. Change the line 33 of Application.cpp as follows
 *
 *     uint8_t theWdtSecs = 8;    ==>  uint8_t theWdtSecs = 30;
 *
 * Before compling to see a detail log of this sketch.
 *
 * Those changes help you to see what's going on in a short time.
 *
 ****************************************************************/

#include <KashiwaGeeks.h>

#define ECHO true

ADB922S LoRa;


//================================
//          Initialize Device Function
//================================
#define BPS_9600       9600
#define BPS_19200     19200
#define BPS_57600     57600
#define BPS_115200   115200

void start()
{
    ConsoleBegin(BPS_57600);
    //DisableConsole();
    //DisableDebug();

    /*
     * Enable Interrupt 0 & 1  Uncomment the following two  lines.
     */
    //pinMode(2, INPUT_PULLUP);
    //pinMode(3, INPUT_PULLUP); // For ADB922S, CUT the pin3 of the Sheild.

    ConsolePrint(F("**** ADR_Sample *****\n"));

    /*  setup Power save Devices */
    //power_adc_disable();          // ADC converter
    //power_spi_disable();          // SPI
    //power_timer1_disable();   // Timer1
    //power_timer2_disable();   // Timer2, tone()
    //power_twi_disable();         // I2C

    /*  setup ADB922S  */
    if ( LoRa.begin(BPS_19200, DR3) == false )     //  !!!!!!!   Set Default DR to DR3
    {
        while(true)
        {
            LedOn();
            delay(300);
            LedOff();
            delay(300);
        }
    }

    /* Set ADR ON */
    LoRa.setADRParams(3, 2);   // ADR_ACK_LIMIT, ADR_ACK_DELAY

    /*  join LoRaWAN */
    LoRa.join();

}

//================================
//          Power save functions
//================================
void sleep(void)
{
    LoRa.sleep();
    DebugPrint(F("LoRa sleep.\n"));

    //
    //  ToDo: Set Device to Power saving mode
     //
}

void wakeup(void)
{
    LoRa.wakeup();
    DebugPrint(F("LoRa wakeup.\n"));
   //
   //  ToDo: Set Device to Power On mode
   //
}

//================================
//          INT0, INT2 callbaks
//================================
void int0D2(void)
{
  ConsolePrint(F("\nINT0 !!!\n"));
  task1();
}

void int1D3(void)
{
  ConsolePrint(F("\nINT1 !!!\n"));
  task2();
}


//================================
//    Functions to be executed periodically
//================================
#define LoRa_Port_NORM  12
#define LoRa_Port_COMP  13

float bme_temp = 10.2;
float bme_humi = 20.2;
float bme_press = 50.05;


/*-------------------------------------------------------------*/
void task1(void)
{
    int16_t temp = bme_temp * 100;
    uint16_t humi = bme_humi * 100;
    uint32_t press = bme_press * 100;
    char s[16];
    ConsolePrint(F("\n  Task1 invoked\n\n"));
    ConsolePrint(F("Temperature:  %d degrees C\n"), temp);
    ConsolePrint(F("%%RH: %d %%\n"), humi);
    ConsolePrint(F("Pressure: %d Pa\n"), press);

    int rc = LoRa.sendData(LoRa_Port_NORM, ECHO, F("%04X%04X%08X"), temp, humi, press);

    if ( rc == LoRa_RC_DATA_TOO_LONG )
    {
        ConsolePrint(F("\n !!!DATA_TOO_LONG\n"));
    }
    else if ( rc == LORA_RC_NO_FREE_CH )
    {
        ConsolePrint(F("\n !!!No free CH\n"));
    }
    else if ( rc == LoRa_RC_BUSY )
    {
        ConsolePrint(F("\n !!!Busy\n"));
    }
    else if ( rc == LoRa_RC_ERROR )
    {
        ConsolePrint(F("\n !!!ERROR\n"));
    }
}

/*-------------------------------------------------------------*/
void task2(void)
{
    char buf[16];
    ConsolePrint(F("\n  Task2 invoked\n\n"));
    ConsolePrint(F("Temperature:  %s degrees C\n"), dtostrf(bme_temp, 6, 2, buf));
    ConsolePrint(F("%%RH: %s %%\n"), dtostrf(bme_humi, 6, 2, buf));
    ConsolePrint(F("Pressure: %s Pa\n"), dtostrf(bme_press, 6, 2, buf));

    Payload pl(LoRa.getMaxPayloadSize());
    pl.set_float(bme_temp);
    pl.set_float(bme_humi);
    pl.set_float(bme_press);

    int rc = LoRa.sendPayloadConfirm(LoRa_Port_COMP, ECHO, &pl);

    if ( rc == LoRa_RC_DATA_TOO_LONG )
    {
        ConsolePrint(F("\n !!!DATA_TOO_LONG\n"));
    }
    else if ( rc == LORA_RC_NO_FREE_CH )
    {
        ConsolePrint(F("\n !!!No free CH\n"));
    }
    else if ( rc == LoRa_RC_BUSY )
    {
        ConsolePrint(F("\n !!!Busy\n"));
    }
    else if ( rc == LoRa_RC_ERROR )
    {
        ConsolePrint(F("\n !!!ERROR\n"));
    }
}

//===============================
//            Execution interval
//     TASK( function, initial offset, interval by minute )
//===============================

TASK_LIST = {
        TASK(task1, 0, 1),
        TASK(task2, 15, 15),
        END_OF_TASK_LIST
};


/*   End of Program  */
