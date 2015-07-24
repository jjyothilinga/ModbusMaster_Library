#include "board.h"
#include "timer.h"
#include "uart.h"

/*
 * For PIC18xxxx devices, the low interrupt vector is found at 000000018h.
 * Change the default code section to the absolute code section named
 * low_vector located at address 0x18.
 */


#pragma code high_vector = 0x08
void high_interrupt (void)
{
	/*
   	* Inline assembly that will jump to the ISR.
   	*/
#if(defined __18F8722_H) ||(defined __18F46K22_H)
	if(PIR1bits.RC1IF == 1)
	{
		_asm GOTO Uart1_ReceiveHandler _endasm
	}

#else

	if(PIR1bits.RCIF == 1)
	{
		_asm GOTO Uart1_ReceiveHandler _endasm
	}
#endif


/*
	if(INTCONbits.TMR0IF == 1)
	{
  		_asm GOTO TIMER0_ISR _endasm
	}
*/













}



#pragma code low_vector=0x18
void low_interrupt (void)
{
	if(INTCONbits.TMR0IF == 1)
	{
  		_asm GOTO TIMER0_ISR _endasm
	}
}




/*
*------------------------------------------------------------------------------
* void EnableInterrupts(void)

* Summary	: Enable interrupts and related priorities
*
* Input		: None
*
* Output	: None
*
*------------------------------------------------------------------------------
*/
void EnableInterrupts(void)
{
	// Enable interrupt priority
  	RCONbits.IPEN = 1;

// Make serial receive interrupt high priority
  	IPR1bits.RC1IP = 1;
 	// Enable all high priority interrupts
  	INTCONbits.GIEH = 1;
	INTCONbits.PEIE = 1;	//enable low priority interrupts
}

