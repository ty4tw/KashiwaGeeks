/*
 * Application.cpp
 * 
 *   Created on: 2017/11/25
 *       Author: tomoaki@tomy-tech.com
 *
 */

#include <Application.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <ADB922S.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

using namespace tomyApplication;

/*======================================
 *         Watchdog Timer interval time
 *------------------------------------------------------------------*/
#define WDT_VAL_1S       B01000110
#define WDT_VAL_2S       B01000111
#define WDT_VAL_4S       B01100000
#define WDT_VAL_8S       B01100001

/*=====================================
 Global Instance
 ======================================*/
uint32_t theAlarmTime = 0;
uint8_t theWDTCSR = WDT_VAL_8S;
uint8_t theWdtSecs = 8;

volatile uint32_t theTimeCount;
volatile uint32_t theUTC = 0;
volatile INT_stat_t theIntStat = INT_INIT;
volatile bool theWdtStat = true;

tomyApplication::Application* theApplication = new tomyApplication::Application();
tomyApplication::SerialLog* theLog = new tomyApplication::SerialLog();

extern TaskList_t theTaskList[];

/*=====================================
     Watchdog Tmer functions
 ======================================*/
void setWDT(uint8_t sec)
{
    if ( sec == 1 )
    {
        theWDTCSR = WDT_VAL_1S;
        theWdtSecs = 1;
    }
    else if ( sec == 2 )
    {
        theWDTCSR = WDT_VAL_2S;
        theWdtSecs = 2;
    }
    else if ( sec == 4 )
    {
        theWDTCSR = WDT_VAL_4S;
        theWdtSecs = 4;
    }
    else if ( sec == 8 )
    {
        theWDTCSR = WDT_VAL_8S;
        theWdtSecs = 8;
    }
    else
    {
        theWDTCSR = WDT_VAL_1S;
        theWdtSecs = 1;
    }
}

void startWatchdog(void)
{
    cli();
    wdt_reset();
    MCUSR &= ~_BV(WDRF);
    WDTCSR |= _BV(WDCE) | _BV(WDE);
    WDTCSR =  theWDTCSR;
    sei();
}

void stopWatchdog(void)
{
    cli();
    wdt_reset();
    MCUSR &= ~_BV(WDRF);
    WDTCSR |= _BV(WDCE) | _BV(WDE);
    WDTCSR = 0;
    sei();
}

void setupWatchdogTimeout(uint32_t seconds)
{
    cli();
    theAlarmTime = seconds;
    theTimeCount = 0;
    sei();
}

ISR(WDT_vect)
{
    cli();
    MCUSR &= ~_BV(WDRF);
    theUTC += theWdtSecs;

    if (theAlarmTime > 0)
    {
        theTimeCount += theWdtSecs;
        if (theTimeCount >= theAlarmTime )
        {
            theWdtStat = true;
        }
        else
        {
            theWdtStat = false;
        }
    }
    sei();
}

/*
void DelayMs(unsigned long ms)
{
    uint32_t start = micros();
    stopWatchdog();
    while (ms > 0) {
        yield();
        while ( ms > 0 && (micros() - start) >= 1000) {
            ms--;
            start += 1000;
            theMs ++;
        }
    }
    if ( theMs > 1000 )
    {
        uint32_t sec = theMs / 1000;
        theSec += sec;
        theMs = theMs % 1000;
    }
    startWatchdog();
}
*/

void SetUTC(uint16_t ddmmyy, uint16_t hhmmss)
{
    uint16_t y = ddmmyy % 100;
    uint16_t m = (ddmmyy / 100 ) % 100;

    if ( m < 3 )
    {
        m += 12;
        y--;
    }
    uint32_t days = 365 * ( y + y/4 - y/100 + y/400)  + (306 * (m + 1)) / 10 + ddmmyy % 10000 - 719591;     //  -428 - 719163
    uint32_t newUTC = days * 86400 + hhmmss / 10000 * 3600 + ((hhmmss / 100) % 100) * 60 + hhmmss % 100;

    cli();
    if( newUTC > theUTC )
    {
        theTimeCount += (newUTC - theUTC);
    }
    else
    {
        uint32_t delUTC = theUTC - newUTC;
        if ( delUTC > theTimeCount )
        {
            theTimeCount = 0;
        }
        else
        {
        theTimeCount -= delUTC;
        }
    }
   theUTC = newUTC;
    sei();
}


/*=====================================
 Interrupt Handler
 ======================================*/
void interrupt0Handler(void)
{
    theIntStat = INT_INT0;
}

void interrupt1Handler(void)
{
    theIntStat = INT_INT1;
}

/*=====================================
 Global functions
 ======================================*/
void DisableConsole(void)
{
    theLog->disableConsole();
}

void DisableDebug(void)
{
    theLog->disableDebug();
}

void ConsoleBegin(unsigned long baud)
{
    theLog->begin(baud);
}

void ConsoleBegin(unsigned long baud, uint8_t rxpin, uint8_t txpin)
{
    theLog->begin(baud, rxpin, txpin);
}

void ConsolePrint(const char *fmt, ...)
{
     va_list args;
     va_start(args, fmt);
     theLog->out(true, LOG_BUF_LEN, fmt, args);
     va_end(args);
}

void ConsolePrint(const __FlashStringHelper *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    theLog->out(true, LOG_BUF_LEN, fmt, args);
    va_end(args);
}

void DebugPrint(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    theLog->out(false, LOG_BUF_LEN, fmt, args);
    va_end(args);
}

void DebugPrint(const __FlashStringHelper *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    theLog->out(false, LOG_BUF_LEN, fmt, args);
    va_end(args);
}

int getFreeMemory(void)
{
    extern char __bss_end;
    extern char *__brkval;

    int freeMemory;

    if((int)__brkval == 0)
    {
        freeMemory = ((int)&freeMemory) - ((int)&__bss_end);
    }
    else
    {
        freeMemory = ((int)&freeMemory) - ((int)__brkval);
    }
    return freeMemory;
}

uint8_t getPinMode(uint8_t pin)
{
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);

  volatile uint8_t *reg, *out;
  reg = portModeRegister(port);
  out = portOutputRegister(port);

  if (*reg & bit)
  {
    return OUTPUT;
  }
  else if (*out & bit)
  {
    return INPUT_PULLUP;
  }
  else
  {
    return INPUT;
  }
}

void EnableInt0(void)
{
    pinMode(2, INPUT_PULLUP);
}

void EnableInt1(void)
{
    pinMode(3, INPUT_PULLUP);
}

//
//
//   Arduino functions
//
//
void setup(void) __attribute__((weak));
void loop(void) __attribute__((weak));
void int0D2(void) __attribute__((weak));
void int1D3(void) __attribute__((weak));
void start(void) __attribute__((weak));
void sleep(void) __attribute__((weak));
void wakeup(void) __attribute__((weak));
void resetArduino(void) __attribute__((weak));

void setup(void)
{
    start();
    theApplication->initialize();
    theLog->savePower();
    startWatchdog();
}

void loop(void)
{
    theApplication->run();
}

void resetArduino()
{
    asm volatile("jmp 0000");
}

void int0D2(void)
{
    DebugPrint(F("----- INT0 -----\n"));
}

void int1D3(void)
{
    DebugPrint(F("===== INT1 =====\n"));
}

void start(void)
{

}


uint32_t getSysTime(void)
{
    return theUTC;
}


void sleep(void)
{
    DebugPrint(F("\nsleep\n"));
}

void wakeup(void)
{
    DebugPrint(F("wakeup\n\n"));
}


//
//
//    Class Application
//
//
Application::Application(void) : _sleepMode{false}, _int0enable{false}, _int1enable{false}
{
    _taskMgr = new TaskManager();
}

Application::~Application(void)
{
    delete _taskMgr;
}


void Application::initialize(void)
{
    DebugPrint(KGS_VERSION);
    if ( getPinMode(2) == INPUT_PULLUP)
    {
        _int0enable = true;
    }
    if ( getPinMode(3) == INPUT_PULLUP)
    {
        _int1enable = true;
    }

    for (uint8_t i = 0; theTaskList[i].callback != 0; i++)
    {
        theTaskList[i].start = theTaskList[i].start * 60;
        theTaskList[i].interval = theTaskList[i].interval * 60;
        _taskMgr->addTask(theTaskList[i].callback, theTaskList[i].start, theTaskList[i].interval);

        #ifdef SHOW_TASK_LIST
        _taskMgr->printTaskEvents();
        #endif
    }
}


void Application::run(void)
{
    if (theIntStat == INT_INT0)
    {
        checkInt(2);
    }
    else if (theIntStat == INT_INT1)
    {
        checkInt(3);
    }

    if ( theWdtStat )
    {
        if ( _sleepMode )
        {
            wakeup();
            _sleepMode = false;
        }
        _taskMgr->execute();
        theWdtStat = false;
    }
    systemSleep();
}


void Application::rerun(void (*_callbackPtr)(), uint32_t second)
{
    _taskMgr->addTask(_callbackPtr, second, 0);
}


void Application::systemSleep(void)
{
    if ( !_sleepMode )
    {
        sleep();   // set device low power mode
        _sleepMode = true;
    }
    theLog->flush();
    set_sleep_mode(SLEEP_MODE);
    sleep_enable();
    enableInterrupts();
    sleep_mode();   // sleep
    sleep_disable(); // wakeup by WDT & INT0 or INT1
    disableInterrupts();
}

void Application::checkInt(uint8_t pin)
{
    indicator(true);
    delay(30);
    while ( true )
    {
        if ( digitalRead(pin) == HIGH )
        {
            break;
        }
    }
    indicator(false);

    if ( _sleepMode )
    {
        wakeup();
        _sleepMode = false;
    }

    if ( pin == 2 )
    {
        int0D2();
    }
    else
    {
        int1D3();
    }
    theIntStat = INT_INIT;
}

void Application::disableInterrupts(void)
{
    if ( _int0enable)
    {
        detachInterrupt(0);
    }
    if ( _int1enable)
    {
        detachInterrupt(1);
    }
}


void Application::enableInterrupts(void)
{
    if ( _int0enable)
    {
        attachInterrupt(0, interrupt0Handler, LOW);
    }
    if ( _int1enable)
    {
        attachInterrupt(1, interrupt1Handler, LOW);
    }
}


void Application::indicator(bool onoff)
{
    if (onoff)
    {
        digitalWrite(ARDUINO_LED_PIN, 1);
    }
    else
    {
        digitalWrite(ARDUINO_LED_PIN, 0);
    }
}



//
//
//    Class TaskManager
//
//
TaskManager::TaskManager(void) :
        _head { 0 }
{

}

TaskManager::~TaskManager(void)
{
    while (_head != 0)
    {
        eraseEvent(_head);
    }
}

void TaskManager::addTask(void (*task)(), uint32_t start, uint32_t interval)
{
    TaskEvent* event = new TaskEvent(this);
    event->setEventCallback(task);
    event->setRemainTime(start);
    event->setIntervalTime(interval);
    startEvent(event);
}

void TaskManager::execute(void)
{
    TaskEvent* taskEv = 0;
    TaskEvent* execList = 0;
    TaskEvent* prevTaskEv = 0;
    uint32_t elapsetime = 0;

    if ( _head == 0 )
    {
        return;
    }

    _head->_remainTime = 0;
    taskEv = _head;

    while ((_head != 0) && (_head->_remainTime == 0) )
    {
        taskEv = _head;
        _head = _head->_next;

        if (taskEv->_callbackPtr)
        {
            taskEv->execute();
        }

        // Add the executed Event to the list.
        if ( taskEv->_interval > 0 )
        {
            taskEv->_remainTime = taskEv-> _interval * taskEv->_count + taskEv->_startTime - getSysTime();
            taskEv->_isRunning = false;
            taskEv->_next = 0;

            if (execList == 0)
            {
                execList = taskEv;
                prevTaskEv = taskEv;
            }
            else
            {
                prevTaskEv->_next = taskEv;
                prevTaskEv = taskEv;
            }
        }
    }


    // start the next TimerListHead if it exists
    if (_head != NULL)
    {
        if (_head->_isRunning == false)
        {
            _head->_isRunning = true;
            setupWatchdogTimeout(_head->_remainTime);   // elapseTime of WDT starts at this point.
        }
    }
    // Restart Task
    while(execList)
    {
        taskEv = execList;
        execList = execList->_next;
        taskEv->_next = 0;
        startEvent(taskEv);

#ifdef SHOW_TASK_LIST
        printTaskEvents();
#endif

    }
}

void TaskManager::startEvent(TaskEvent* event)
{
    if (event  == 0)
    {
        return;
    }

    event->_isRunning = false;

#ifdef SHOW_TASK_LIST
    DebugPrint(F("\n  -startEvent() sys:%ld remain:%lu run:%d Interval:%lu  start:%lu cnt:%lu\n"), getSysTime(), event->_remainTime, event->_isRunning, event->_interval, event->_startTime, event->_count);
#endif

    if (_head == 0)
    {
        insertEventHead(event);
    }
    else
    {
        if (event->_remainTime <  _head->_remainTime)
        {
            insertEventHead(event);
        }
        else
        {
            insertEvent(event);
        }
    }
}

void TaskManager::insertEventHead(TaskEvent* event)
{
    if (_head)
    {
        _head->_remainTime = _head->_remainTime - event->_remainTime;
        _head->_isRunning = false;
    }

    event->_next = _head;
    event->_isRunning = true;
    _head = event;
    setupWatchdogTimeout(_head->_remainTime);
}

void TaskManager::insertEvent(TaskEvent* event)
{
    uint32_t aggregatedTimestamp = 0;
    uint32_t aggregatedTimestampNext = 0;

    TaskEvent* prev = _head;
    TaskEvent* cur = _head->_next;

    if (!cur)
    {
        event->_remainTime -= _head->_remainTime;
        _head->_next = event;
        event->_next = 0;
    }
    else
   {
       aggregatedTimestamp = _head->_remainTime;
       aggregatedTimestampNext = cur->_remainTime;

       while (prev)
       {
           if (aggregatedTimestampNext > event->_remainTime - aggregatedTimestamp)
           {
               event->_remainTime -= aggregatedTimestamp;
               if (cur)
               {
                   cur->_remainTime = cur->_remainTime - event->_remainTime;
               }
               prev->_next = event;
               event->_next = cur;
               break;
           }
           else
           {
               prev = cur;
               cur = cur->_next;
               if (!cur)
               {
                   aggregatedTimestamp += aggregatedTimestampNext;
                   event->_remainTime -= aggregatedTimestamp;
                   prev->_next = event;
                   event->_next = 0;
                   break;
               }
               else
               {
                   aggregatedTimestamp += aggregatedTimestampNext;
                   aggregatedTimestampNext = aggregatedTimestampNext + cur->_remainTime;
               }
           }
       }
   }
}

/*
void TaskManager::stopEvent(TaskEvent* event)
{
    uint32_t elapsedTime = 0;
    uint32_t remainingTime = 0;

    if ((_head == 0) || (event == 0))
    {
        return;
    }

    if (_head == event)
    {
        if (_head->_isRunning)
        {
            elapsedTime = _head->elapseTime();
            if (elapsedTime > event->_remainTime)
            {
                elapsedTime = event->_remainTime;
            }

            remainingTime = event->_remainTime - elapsedTime;

            if (_head->_next)
            {
                _head->_isRunning = false;
                _head = _head->_next;
                _head->_isRunning = true;
                setupWatchdogTimeout(_head->_remainTime);
            }
            else
            {
                _head = 0;
            }
        }
        else   // Stop the head before is is started.
        {
            if (_head->_next)
            {
                remainingTime = event->_remainTime;
                _head = _head->_next;
                _head->_remainTime += remainingTime;
            }
            else
            {
                _head = 0;
            }
        }
    }
    else  // Stop an event within the list.
    {
        TaskEvent* prev = _head;
        TaskEvent* cur = _head;

        remainingTime = event->_remainTime;

        while (cur)
        {
            if (cur == event)
            {
                if (cur->_next)
                {
                    cur = cur->_next;
                    prev->_next = cur;
                    cur->_remainTime += remainingTime;
                }
                else
                {
                    cur = 0;
                    prev->_next = cur;
                }
                break;
            }
            else
            {
                prev = cur;
                cur = cur->_next;
            }
        }
    }
}
*/

void TaskManager::eraseEvent(TaskEvent* event)
{
    if (_head == 0  )
    {
        return;
    }

    if (_head == event)
    {
        if (_head->_next == 0)
        {
            _head = 0;
        }
        else
        {
            _head = event->_next;
        }
        return;
    }

    TaskEvent* cur = 0;
    TaskEvent* prev = 0;

    cur = _head->_next;
    prev = _head;

    while (cur != 0)
    {
        if (cur == event)
        {
            if (event->_next != 0)
            {
                prev->_next = event->_next;
            }
            return;
        }
        prev = cur;
        cur = cur->_next;
    }
}

bool TaskManager::isExist(TaskEvent* event)
{
    TaskEvent* cur = _head;
    while (cur != 0)
    {
        if (cur == event)
        {
            return true;
        }
        cur = cur->_next;
    }
    return false;
}

void TaskManager::printTaskEvents(void)
{
    TaskEvent*  taskEv = _head;

     while( taskEv )
     {
         DebugPrint(F("  -remain:%lu run:%d Interval:%lu start:%lu cnt:%lu\n"), taskEv->_remainTime, taskEv->_isRunning, taskEv->_interval, taskEv->_startTime, taskEv->_count);
         taskEv = taskEv->_next;
     }
     DebugPrint(F("\n"));
}

//
//
//    Class TaskEvent
//
//
TaskManager*  TaskEvent::_taskMgr = 0;

TaskEvent::TaskEvent(TaskManager* mgr) :
        _remainTime{0}, _interval{0}, _count{0}, _startTime{0}, _callbackPtr{0},  _next{0}
{
    _isRunning = false;
    if ( _taskMgr  == 0 )
    {
        _taskMgr = mgr;
    }
}

TaskEvent::~TaskEvent(void)
{
    if ( _taskMgr )
    {
        _taskMgr->eraseEvent(this);
    }
}

void TaskEvent::execute(void)
{
    if ( _count == 0 )
    {
        _startTime = getSysTime();
    }
    _count++;
    _callbackPtr();
}

void TaskEvent::setIntervalTime(uint32_t value)
{
    _interval = value;
}

void TaskEvent::setEventCallback(void (*callback)())
{
    _callbackPtr = callback;
}

void TaskEvent::setRemainTime(uint32_t sec)
{
    _remainTime = sec;
}
//
//
//    Class SerialLog
//
//
SerialLog::SerialLog(void):
        _serial{0}, _serialFlg{false}, _consoleFlg{true}, _debugFlg{true}
{

}

SerialLog::~SerialLog(void)
{

}

void SerialLog::begin(unsigned long baud, uint8_t rxpin, uint8_t txpin)
{
    _serial = new SoftwareSerial( rxpin, txpin);
    _serial->begin(baud);
    _serialFlg = true;
}

void SerialLog::begin(unsigned long baud)
{
    Serial.begin(baud);
    _serialFlg = false;
}

void SerialLog::out(bool console, uint8_t len, const char* fmt, va_list args)
{
    if ( (console && _consoleFlg) || (!console && _debugFlg) )
    {
        char* buf = (char*)malloc(len);

        vsnprintf(buf, len, fmt, args);
        print(buf);
        free(buf);
    }
}

void SerialLog::out(bool console, uint8_t len, const  __FlashStringHelper* fmt, va_list args)
{
    if ( (console && _consoleFlg) || (!console && _debugFlg) )
    {
        char* buf = (char*)malloc(len);

        vsnprintf_P(buf, len, (const char *)fmt, args);
        print(buf);
        free(buf);
    }
}

void SerialLog::print(char* buf)
{
    if ( _serialFlg )
    {
        if ( _serial->isListening() )
        {
            _serial->listen();
        }
        _serial->print(buf);
    }
    else
    {
        Serial.print(buf);
    }
}

void SerialLog::disableDebug(void)
{
    _debugFlg = false;
}

void SerialLog::disableConsole(void)
{
    _consoleFlg = false;
}

void SerialLog::savePower(void)
{
    if (  ! _consoleFlg  && !_debugFlg &&  !_serialFlg)
    {
        power_usart0_disable();       // Serial
    }
}

void SerialLog::flush(void)
{
    if ( !_serialFlg )
    {
        Serial.flush();
    }
}
