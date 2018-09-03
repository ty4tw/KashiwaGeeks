/****************************************************************
 *
 *                The MIT License
 *
 *   Copyright (c) 2017-2018  tomoaki@tomy-tech.com
 *
 ***************************************************************/
#include <KashiwaGeeks.h>

void start(void)
{
    ConsoleBegin(9600);
    EnableInt0();

  //DisableConsole();
  //DisableDebug();
}


//================================
//          Intrrupt callbaks
//================================
void int0D2(void)
{
    ConsolePrint(F("********* INT0 *******\n"));
}


//================================
//          Tasks
//================================

int cnt = 0;

void func1()
{
  ConsolePrint("func1:  Count=%d\n", cnt++);
  DebugPrint(F("func1:  Debug Count=%d\n"), cnt++);
}


void func2()
{
  ConsolePrint("func2:  Count=%d\n", cnt++);
  DebugPrint(F("func2:  Debug Count=%d\n"), cnt++);
}


//===============================
//            Execution interval
//     TASK( function, initial offset, interval by minute )
//===============================
TASK_LIST = {
  TASK(func1, 0, 1),
  TASK(func2, 1, 2),
  END_OF_TASK_LIST
};




