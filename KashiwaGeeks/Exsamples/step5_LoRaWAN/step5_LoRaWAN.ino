#include <KashiwaGeeks.h>

ADB922S LoRa;  // LoRaWANシールドのインスタンスを生成

//================================
//       デバイス初期化関数
//================================
#define BPS_9600     9600
#define BPS_19200   19200
#define BPS_57600   57600
#define BPS_115200 115200

void start()
{
    /*  Setup console */

    Serial.begin(BPS_57600);

    //DisableConsole();
    //DisableDebug();

    ConsolePrint(F("**** Start*****\n"));

    /*  LoRaWANシールドのインスタンスを初期化  */
    if ( LoRa.begin(BPS_19200) == false )
    {
        for(;;)   // 初期化できなければABORT
        {
            LedOn();
            delay(300);
            LedOff();
            delay(300);
        }
    }

    /*  LoRaWANにJOIN  */
    LoRa.reconnect();

    /*  for BME280 initialize  */
     //bme.begin();

    /*  WDTのインターバル設定 1, 2, 4 or 8 秒のいずれか  */
    setWDT(2);    
}

//================================
//   定期的に実行される関数
//================================
float bme_temp = 0;
float bme_humi = 0;
float bme_press = 0;

void task1(void)
{
   ConsolePrint(F("Temperature:  %2d degrees C\n" ), bme_temp);
   ConsolePrint(F("%%RH: %2d%s%%\n"), bme_humi);
   ConsolePrint(F("Pressure: %2d Pa\n"), bme_press);

   disableInterrupt();    //  INT0 & INT1 割込み禁止
   sendTemp();            // 　LoRaWAN送信処理
   enableInterrupt();     //  INT0 & INT1 割込み禁止解除
}

/*-------------------------------------------------------------*/
void task2(void)
{

}


//===============================
//            Execution interval
//    TASK( function, interval by second )
//===============================
TASK_LIST = {
        TASK(task1, 0, 20),
        //TASK(task2, 2, 10),
        END_OF_TASK_LIST
};

/*-------------------------------------------------------------*/

#define LoRa_fPort_TEMP  12        // port 12 = 温度湿度気圧等

//
// 温度湿度気圧を LoRa送信する
//
void sendTemp()
{
  short port = LoRa_fPort_TEMP;    // port 12 = Temp

  int temp = bme_temp * 100;
  unsigned int humi = bme_humi * 100;
  unsigned long press = bme_press * 100;

  // LoRaWANでデータを送信する
  //LoRa.sendData(port, true, F("%04x%04x%06lx"), temp, humi, press);
  LoRa.sendDataConfirm(port, true, F("%04x%04x%06lx"), temp, humi, press);
}

/*   End of Program  */

