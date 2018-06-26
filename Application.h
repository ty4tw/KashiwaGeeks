/*
 * Application.h
 * 
 *   Created on: 2017/11/25
 *       Author: tomoaki@tomy-tech.com
 *
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#if ARDUINO >= 100
#include <Arduino.h>
#include <AppDefine.h>
#else
#if ARDUINO < 100
#include "WProgram.h"
#endif
#endif

#include <inttypes.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <SoftwareSerial.h>

void int0D2(void);
void int1D3(void);
void setupDevice(void);
void sleep(void);
void wakeup(void);
void resetArduino(void);
void EnableInt0(void);
void EnableInt1(void);

void DebugPrint(const __FlashStringHelper *format, ...);
void ConsolePrint(const __FlashStringHelper *format, ...);
void DebugPrint(const  char* format, ...);
void ConsolePrint(const  char* format, ...);
void ConsoleBegin(unsigned long baud);
void ConsoleBegin(unsigned long baud, uint8_t rxpin, uint8_t txpin);
void DisableConsole(void);
void DisableDebug(void);

namespace tomyApplication
{

#define ARDUINO_LED_PIN     13

#define LOG_BUF_LEN 128

/*
 *    Debug define
 */

//#define SHOW_TASK_LIST
//#define LORA_DEBUG

/*
 *    Sleep mode define
 */

#define SLEEP_MODE   SLEEP_MODE_PWR_DOWN
//#define SLEEP_MODE   SLEEP_MODE_STANDBY
//#define SLEEP_MODE   SLEEP_MODE_PWR_SAVE

/*======================================
 MACROs for the Appication
 =======================================*/
#define TASK_LIST   TaskList_t  theTaskList[]
#define TASK(...)         {__VA_ARGS__}
#define END_OF_TASK_LIST  {0, 0, 0}
#define ReRun(...)   theApplication->rerun(__VA_ARGS__)

#define LedOn() theApplication->indicator(true)
#define LedOff() theApplication->indicator(false)
#define disableInterrupt()  theApplication->disableInterrupts()
#define enableInterrupt()  theApplication->enableInterrupts()
#define elapseSeconds()    theApplication->getElapseTime()

/*======================================
 Type definitions
 =======================================*/

/* TaskList type */
typedef struct
{
    void (*callback)(void);
    uint32_t start;
    uint32_t interval;
} TaskList_t;

/* Interrupt Status */
enum INT_stat_t
{
    INT_INIT, INT_INT0, INT_INT1
};


/*========================================
 Class Application
 =======================================*/
class TaskManager;

class Application
{
public:
    Application(void);
    ~Application();

    void initialize( void);
    void run(void);
    void rerun(void (*_callbackPtr)(), uint32_t second);
    void indicator(bool onoff);
    void setWdtInterval(uint8_t second);
    uint32_t getElapseTime(void);
private:
    void systemSleep(void);
    void checkInt(uint8_t pin);
    void disableInterrupts(void);
    void enableInterrupts(void);
    TaskManager *  _taskMgr;
    bool    _sleepMode;
    bool   _int0enable;
    bool   _int1enable;
};

/*============================================
  Class TaskManager
 ============================================*/
class TaskEvent;

class TaskManager
{
    friend class TaskEvent;
public:
    TaskManager(void);
    ~TaskManager(void);

    void initialize(void);
    void addTask(void (*)(), uint32_t start, uint32_t interval);
    void execute(void);
    void printTaskEvents(void);
private:
    void startEvent(TaskEvent* event);
    void insertEventHead(TaskEvent* event);
    void insertEvent(TaskEvent* event);
    void eraseEvent(TaskEvent* event);
    bool isExist(TaskEvent* event);

    TaskEvent* _head;
};

/*============================================
  Class  TaskEvent
 ============================================*/
class TaskEvent
{
    friend class TaskManager;
public:
    TaskEvent(TaskManager* mgr);
    ~TaskEvent(void);
    void setIntervalTime(uint32_t value);
    void setEventCallback(void (*callback)());
    void setRemainTime(uint32_t sec);
    void execute(void);
private:
    uint32_t _remainTime;
    uint32_t _interval;
    uint32_t  _startTime;
    uint32_t _count;
    uint8_t _isRunning;
    void (*_callbackPtr)();
    TaskEvent *_next;
    static TaskManager* _taskMgr;
};

/*============================================
  Class  SerialLog
 ============================================*/
class SerialLog
{
public:
    SerialLog(void);
    ~SerialLog(void);
    void out(bool console,  uint8_t len, const char* fmt, va_list args);
    void out(bool console,  uint8_t len, const __FlashStringHelper* fmt, va_list args);
    void begin(unsigned long baud);
    void begin(unsigned long baud, uint8_t rxpin, uint8_t txpin);
    void disableDebug(void);
    void disableConsole(void);
    void savePower(void);
    void flush(void);
private:
    void print(char* buf);
    SoftwareSerial* _serial;
    bool _serialFlg;
    bool _consoleFlg;
    bool _debugFlg;
};

} /* tomyApplication */

#endif /* APPLICATION_H_ */
