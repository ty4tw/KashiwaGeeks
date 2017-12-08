/*
 * Application.cpp
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

#include <Application.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>
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
uint32_t thePrevUTC = 0;
uint8_t theWDTCSR = WDT_VAL_1S;
uint8_t theWdtSecs = 1;
volatile uint32_t theTimeCount;
volatile uint32_t theUTC = 0;
volatile INT_stat_t theIntStat = INT_INIT;
volatile bool theWdtStat = false;

Application* theApplication = new Application();
bool theConsoleFlag  = true;
bool theDebugFlag  = true;
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
        theWDTCSR = WDT_VAL_2S;
        theWdtSecs = 4;
    }
    else if ( sec == 8 )
    {
        theWDTCSR = WDT_VAL_4S;
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
    thePrevUTC = theUTC;
    sei();
}

uint32_t elapseTime(void)
{
    return theUTC - thePrevUTC;
}

ISR(WDT_vect)
{
    cli();
    MCUSR &= ~_BV(WDRF);
    theUTC += theWdtSecs ;
    if (theAlarmTime > 0)
    {
        theTimeCount += theWdtSecs ;
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
#define BUF_LEN 128

void DisableConsole(void)
{
    theConsoleFlag = false;
}

void DisableDebug(void)
{
    theDebugFlag = false;
}

void ConsolePrint(const char *fmt, ...)
{
    if ( theConsoleFlag )
    {
         char buf[BUF_LEN];
         va_list args;
         va_start(args, fmt);
         vsnprintf(buf, sizeof(buf), fmt, args);
         va_end(args);
         Serial.print(buf);
    }
}

void ConsolePrint(const __FlashStringHelper *fmt, ...)
{
   if ( theConsoleFlag )
   {
       char buf[BUF_LEN];
       va_list args;
        va_start(args, fmt);
        vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args);
        va_end(args);
        Serial.print(buf);
   }
}

void DebugPrint(const char *fmt, ...)
{
    if ( theDebugFlag )
   {
      char buf[BUF_LEN];
      va_list args;
      va_start(args, fmt);
      vsnprintf(buf, sizeof(buf), fmt, args);
      va_end(args);
      Serial.print(buf);
   }
}

void DebugPrint(const __FlashStringHelper *fmt, ...)
{
    if ( theDebugFlag )
   {
       char buf[BUF_LEN];
       va_list args;
        va_start(args, fmt);
        vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args);
        va_end(args);
        Serial.print(buf);
   }
}

#if defined( SHOW_TASK_LIST ) || defined( SHOW_SYSTIME)
void debugout(const char *fmt, ...)
{
  char buf[BUF_LEN];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  Serial.print(buf);
}

void debugout(const __FlashStringHelper *fmt, ...)
{
   char buf[BUF_LEN];
   va_list args;
    va_start(args, fmt);
    vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args);
    va_end(args);
    Serial.print(buf);
}
#else
void debugout(const char *fmt __attribute__ ((unused)), ...)
{

}
void debugout(const __FlashStringHelper *fmt  __attribute__ ((unused)), ...)
{

}
#endif

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
    for ( uint8_t i = 0; i < 20; i++ )
    {
        pinMode(i, OUTPUT);   // To save power
    }
    Serial.begin(9600);
    if (  ! theConsoleFlag  && !theDebugFlag )
    {
        power_usart0_disable();       // Serial
    }
    start();
    theApplication->enableInterrupts();
    theApplication->initialize();
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
    DebugPrint(F("***** INT0 *****\n"));
}

void int1D3(void)
{
    DebugPrint(F("===== INT1 =====\n"));
}

void start(void)
{

}

void sleep(void)
{
    DebugPrint(F("sleep\n"));
}

void wakeup(void)
{
    DebugPrint(F("wakeup\n"));
}

uint32_t getSysTime(void)
{
    return theUTC;
}

//
//
//    Class Application
//
//
Application::Application(void) : _sleepMode{false}
{
    _taskMgr = new TaskManager();
}

Application::~Application(void)
{
    delete _taskMgr;
}

// initialize
void Application::initialize(void)
{
    DebugPrint(F("\n\n_/_/_/ KashiwaGeeksFrame's TaskManager starts. _/_/_/\r\n\n\n"));

    for (uint8_t i = 0; theTaskList[i].callback != 0; i++)
    {
        uint32_t executedTime = theUTC;
        if ( theTaskList[i].start == 0 )
        {
            theTaskList[i].callback();
        }
        _taskMgr->addTask(theTaskList[i].callback, theTaskList[i].start, theTaskList[i].interval, executedTime);

#ifdef SHOW_TASK_LIST
        _taskMgr->printTaskEvents();
#endif
    }
}

//    run
void Application::run(void)
{
    systemSleep();

    if ( theWdtStat )
    {
#if defined( SHOW_TASK_LIST ) || defined( SHOW_SYSTIME)
        ConsolePrint( F("\nSystem Time:%ld elapse:%ld[sec]\n"),theUTC, elapseTime());
#endif
        if ( _sleepMode )
        {
            wakeup();
            _sleepMode = false;
        }
        _taskMgr->execute();
        theWdtStat = false;
    }
    else if (theIntStat == INT_INT0)
    {
        if ( _sleepMode )
        {
            wakeup();
            _sleepMode = false;
        }
        disableInterrupts();
        int0D2();
        enableInterrupts();
    }
    else if (theIntStat == INT_INT1)
    {
        if ( _sleepMode )
        {
            wakeup();
            _sleepMode = false;
        }
        disableInterrupts();
        int1D3();
        enableInterrupts();
    }
}


//    systemSleep
void Application::systemSleep(void)
{
    if ( !_sleepMode )
    {
        sleep();   // set device low power mode
        _sleepMode = true;
    }
    Serial.flush();
    set_sleep_mode(SLEEP_MODE);
    sleep_enable();
    sleep_mode();   // sleep
    sleep_disable(); // wakeup by WDT & INT0 or INT1
}

//    disableInterrupts
void Application::disableInterrupts(void)
{
    detachInterrupt(0);
    detachInterrupt(1);
}

//    enableInterrupts
void Application::enableInterrupts(void)
{
    theIntStat = INT_INIT;
    attachInterrupt(0, interrupt0Handler, RISING);
    attachInterrupt(1, interrupt1Handler, RISING);
}

//    indicator
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

//   getElapseTime
uint32_t Application::getElapseTime(void)
{
    return elapseTime();
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

void TaskManager::addTask(void (*task)(), uint32_t start, uint32_t interval, uint32_t executedTime)
{
    TaskEvent* event = new TaskEvent(this);
    event->setEventCallback(task);
    event->setTimeValue(interval);
    event->setRemainTime(start);
    event->setExecutedTime(executedTime);
    startEvent(event);
}

void TaskManager::execute(void)
{
    TaskEvent* taskEv = 0;
    TaskEvent* execList = 0;
    TaskEvent* prevTaskEv = 0;
    uint32_t elapsetime = elapseTime();

    if (elapsetime >= _head->_remainTime)
    {
        _head->_remainTime = 0;
    }
    else
    {
        _head->_remainTime -= elapsetime;
    }

    taskEv = _head;

    while ((_head != 0) && (_head->_remainTime == 0))
    {
        taskEv = _head;
        _head = _head->_next;

        if (taskEv->_callbackPtr)
        {
            // execute Task
            taskEv->setExecutedTime(theUTC);
            taskEv->_callbackPtr();
        }

        // Add the executed Event to the list.
        uint32_t  elapse = theUTC - taskEv->_executedTime;
        if ( elapse >= taskEv-> _reloadTime)
        {
            taskEv->_remainTime = taskEv-> _reloadTime - elapse;
        }
        else
        {
            taskEv->_remainTime = 0;
        }
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

    if ( event->_remainTime == 0 )
    {
        event->_remainTime = event->_reloadTime;
    }

    event->_isRunning = false;

#ifdef SHOW_TASK_LIST
    debugout( F("---Event start ---- remain:%ld Interval:%ld\n"), event->_remainTime, event->_reloadTime);
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
            elapsedTime = elapseTime();
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

void TaskManager::resetEvent(TaskEvent* event)
{
    startEvent(event);
    stopEvent(event);
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
#ifdef SHOW_TASK_LIST
    TaskEvent*  taskEv = _head;
    debugout(F("---Task Link----\n"));
     while( taskEv )
     {
         debugout(F("remain:%ld run:%d Interval:%ld\n"), taskEv->_remainTime, taskEv->_isRunning, taskEv->_reloadTime);
         taskEv = taskEv->_next;
     }
     debugout(F("\n"));
#endif
}

///
//
//    Class TaskEvent
//
//
TaskManager*  TaskEvent::_taskMgr = 0;

TaskEvent::TaskEvent(TaskManager* mgr) :
        _remainTime{0}, _reloadTime{0}, _executedTime{0}, _callbackPtr{0},  _next{0}
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

void TaskEvent::setTimeValue(uint32_t value)
{
    _remainTime = value;
    _reloadTime = value;
}

void TaskEvent::setEventCallback(void (*callback)())
{
    _callbackPtr = callback;
}

void TaskEvent::setExecutedTime(uint32_t sec)
{
    _executedTime = sec;
}

void TaskEvent::setRemainTime(uint32_t sec)
{
    _remainTime = sec;
}

