/****************************************************************
 *
 *                The MIT License
 *
 *   Copyright (c) 2017-2018  tomoaki@tomy-tech.com
 *
 *            I M P O R T A N T    N O T I C E
 *
 *    Uncomment the line17 of AppDefine.h in the library
 *    Before compling to see a detail log of this sketch.
 *
 *          !!!!! DO NOT FORGET TO  PUT IT BACK !!!!!
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
    EnableInt0();
    //EnableInt1();; // For ADB922S, CUT the pin3 of the Sheild.


    ConsolePrint(F("\n**** ADR_Sample *****\n"));

    /*  setup Power save Devices */
    //power_adc_disable();          // ADC converter
    //power_spi_disable();          // SPI
    //power_timer1_disable();   // Timer1
    //power_timer2_disable();   // Timer2, tone()
    //power_twi_disable();         // I2C

    /*  setup ADB922S  */
    if ( LoRa.begin(BPS_9600) == false )
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

    ConsolePrint(F("\n      Press Interrupt Button to proceed.!\n"));

    LoRa.setDr(DR5);
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
//          INT0 callbaks
//================================
void int0D2(void)
{
  ConsolePrint(F("\nINT0 !!!\n"));
  task1();
}

void int1D3(void)
{
  ConsolePrint(F("\nINT1 !!!\n"));
}


//================================
//    Functions to be executed periodically
//================================

void task1(void)
{
    int16_t temp = 1020;
    uint16_t humi = 2020;
    uint32_t press = 5005;
    char s[16];

    LoRa.sendData(12, ECHO, F("%04X%04X%08X"), temp, humi, press);
}


//===============================
//            Execution interval
//     TASK( function, initial offset, interval by minute )
//===============================

TASK_LIST = {
        END_OF_TASK_LIST
};

/*   End of Program  */
