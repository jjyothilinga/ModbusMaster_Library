#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

/*
*------------------------------------------------------------------------------
* timer_driver.h
*
* Include file for timer_driver module.
*/
/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/

#include "board.h"
#include <timers.h>

/*
*------------------------------------------------------------------------------
* Public Defines
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Macros
*------------------------------------------------------------------------------
*/






/*
*------------------------------------------------------------------------------
* Public Data Types
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Variables (extern)
*------------------------------------------------------------------------------
*/
extern UINT16 heartBeatCount  ;
extern UINT16 keypadUpdateCount  ;
extern UINT16 comUpdateCount ;
extern UINT16 appUpdateCount ;
extern UINT16 uiUpdateCount ;
/*
*------------------------------------------------------------------------------
* Public Constants (extern)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Function Prototypes (extern)
*------------------------------------------------------------------------------
*/

extern void TIMER0_init(unsigned int reload , void (*func)());
extern void TIMER1_init(unsigned int reload , void (*func)());
void TIMER1_ISR(void);
void TIMER0_ISR(void);
void TIMER3_init(unsigned int reload , void (*func)());
UINT32 GetAppTime(void);
void ResetAppTime(void);

#endif
/*
*  End of timer_driver.h
*/