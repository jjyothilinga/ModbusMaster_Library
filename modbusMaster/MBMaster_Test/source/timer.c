#include "timer.h"
#include "config.h"

typedef struct _TMR
{
	unsigned int reload;
	void (*func)(void);
}TMR;


#pragma idata TIMER_DATA
TMR tmr[3] = { 0 , 0 };


UINT16 count = 0;
UINT16 heartBeatCount  =0 ;
UINT16 keypadUpdateCount  =0 ;
UINT16 comUpdateCount = 0;
UINT16 appUpdateCount = 0;
UINT16 uiUpdateCount = 0;
INT16 timeStampUpdateCount = TIMESTAMP_DURATION;
UINT32 AppTimestamp = 0;

/*
*------------------------------------------------------------------------------
* void TMR0_ISR(void)
*
* Summary	: Timer0 ISR for periodi tick for schedular base
*
* Input		: None
*
* Output	: None
*
*------------------------------------------------------------------------------
*/
#pragma code ISR
#pragma interrupt TIMER0_ISR
void TIMER0_ISR(void)
{
  	/*
   	* Clears the TMR0 interrupt flag to stop the program from processing the
   	* same interrupt multiple times.
   	*/
  	INTCONbits.TMR0IF = 0;

	++heartBeatCount;
	++keypadUpdateCount;
	++comUpdateCount;
	++appUpdateCount;
	++uiUpdateCount;

	--timeStampUpdateCount;

	if( timeStampUpdateCount <= 0 )
	{
		AppTimestamp++;
		timeStampUpdateCount = TIMESTAMP_DURATION;
	}

	if(tmr[0].func != 0 )
		(*(tmr[0].func))();


	// Reload value for 1ms overflow
	WriteTimer0(tmr[0].reload);
}


#pragma code





/*
*------------------------------------------------------------------------------
* void TMR0_init(void)

* Summary	: Initialize timer0 for schedular base
*
* Input		: None
*
* Output	: None
*
*------------------------------------------------------------------------------
*/
void TIMER0_init(UINT16 reload , void (*func)(void))
{
	
   	OpenTimer0(TIMER_INT_ON & T0_SOURCE_INT & T0_16BIT & 
		T0_PS_1_1);	// Enable the TMR0 interrupt,16bit counter with internal clock,No prescalar.

	
	tmr[0].reload = reload;
	tmr[0].func = func;

	INTCON2bits.TMR0IP = 0;		//MAKE TIMER0 interrupt low priority

	WriteTimer0(reload);	// Reload 
}

UINT32 GetAppTime(void)
{	
	UINT32 temp;

	DISABLE_TMR0_INTERRUPT();
	temp  = AppTimestamp;
	ENABLE_TMR0_INTERRUPT();
	return temp;
}

void ResetAppTime(void)
{
	ENTER_CRITICAL_SECTION();
	AppTimestamp = 0;
	EXIT_CRITICAL_SECTION();
}