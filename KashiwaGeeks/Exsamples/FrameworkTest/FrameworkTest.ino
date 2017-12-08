/*
 *   FrameworkTest.ino
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

#include <KashiwaGeeks.h>

#define BPS_9600            9600
#define BPS_19200       19200
#define BPS_57600       57600
#define BPS_115200   115200

void setupDevice()
{
    Serial.begin(BPS_57600);
    //DisableConsole();
    //DisableDebug();


    //power_adc_disable();     // ADC converter
    //power_uart0_disable();   // Serial
    //power_spi_disable();     // SPI
    //power_timer1_disable();  // Timer1
    //power_timer2_disable();  // Timer2, tone()
    //power_twi_disable();     // I2C

    /* seetup WDT interval to 1, 2, 4 or 8 seconds */
    setWDT(2);    // set to 2 seconds
}

//================================
//          Power save functions
//================================
void sleep(void)
{
    //
    //  ToDo: Set Device to Power saving mode
    //
}

void wakeup(void)
{
   //
   //  ToDo: Set Device to Power On mode
   //
}

//================================
//     RISING interrupt callbaks for Pin2, 3
//================================
void int0D2(void)
{
    ConsolePrint(F("INT0 executed\n\r"));
}

void int1D3(void)
{
    ConsolePrint(F("INT1 executed\n\r"));
}

//===========================================
//    Functions to be executed periodically
//===========================================

void task1(void)
{
    ConsolePrint(F("task1 executed\n\r"));
    DebugPrint( F("--- Debug Print task1\n\r "));
}

/*-------------------------------------------------------------*/
void task2(void)
{
    ConsolePrint(F("task2 executed\n\r"));
    DebugPrint( F("--- Debug Print task2\n\r "));
}

/*-------------------------------------------------------------*/
void task3(void)
{
    ConsolePrint(F("task3 executed\n\r"));
    DebugPrint( F("--- Debug Print task3\n\r "));
}

//===============================
//            Execution interval
//    TASK( function, interval by second )
//===============================
TASK_LIST = {
        TASK(task1, 0, 10),
        TASK(task2, 5, 10),
        TASK(task3, 7, 10),
        END_OF_TASK_LIST
};

/*   End of Program  */
