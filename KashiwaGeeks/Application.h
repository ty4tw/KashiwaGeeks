/*
 * Application.h
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
#include <SoftwareSerial.h>

void int0D2(void);
void int1D3(void);
void setupDevice(void);
void sleep(void);
void wakeup(void);
void resetArduino(void);

namespace tomyApplication
{

#define ARDUINO_LED_PIN     13

/*======================================
 MACROs for the Appication
 =======================================*/
#define TASK_LIST   TaskList_t  theTaskList[]
#define TASK(...)         {__VA_ARGS__}
#define END_OF_TASK_LIST  {0, 0, 0}

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
    void systemSleep(void);
    void disableInterrupts(void);
    void enableInterrupts(void);
    void indicator(bool onoff);
    void setWdtInterval(uint8_t second);
    uint32_t getElapseTime(void);
private:
    TaskManager *  _taskMgr;
    bool    _sleepMode;
};

/*============================================
 TaskManager
 ============================================*/
class TaskEvent;

class TaskManager
{
    friend class TaskEvent;
public:
    TaskManager(void);
    ~TaskManager(void);

    void initialize(void);
    void addTask(void (*)(), uint32_t start, uint32_t interval, uint32_t executedTime);
    void execute(void);
    void printTaskEvents(void);
private:
    void startEvent(TaskEvent* event);
    void stopEvent(TaskEvent* event);
    void resetEvent(TaskEvent* event);
    void insertEventHead(TaskEvent* event);
    void insertEvent(TaskEvent* event);
    void eraseEvent(TaskEvent* event);
    bool isExist(TaskEvent* event);

    TaskEvent* _head;
};

/*============================================
 TaskEvent
 ============================================*/
class TaskEvent
{
    friend class TaskManager;
public:
    TaskEvent(TaskManager* mgr);
    ~TaskEvent(void);
    void setTimeValue(uint32_t value);
    void setEventCallback(void (*callback)());
    void setExecutedTime(uint32_t sec);
    void setRemainTime(uint32_t sec);
private:
    uint32_t _remainTime;
    uint32_t _reloadTime;
    uint32_t _executedTime;
    bool _isRunning;
    void (*_callbackPtr)();
    TaskEvent *_next;
    static TaskManager* _taskMgr;
};


} /* tomyApplication */

#endif /* APPLICATION_H_ */
