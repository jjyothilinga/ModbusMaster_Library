
/*
*------------------------------------------------------------------------------
* main.c
*
* main application specific module.
*
* (C)2008 Sam's Logic.
*
* The copyright notice above does not evidence any
* actual or intended publication of such source code.
*
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* File				: main.c
* Created by		: Sam
* Last changed by	: Sam
* Last changed		: 07/07/2009
*------------------------------------------------------------------------------
*
* Revision 0.0 07/07/2009 Sam
* Initial revision
*
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/

#include "board.h"
#include <timers.h>			// Timer library functions
#include <delays.h>			// Dely library functions
#include <string.h>			// String library functions
#include "timer.h"			// Timer related functions
#include "uart.h"
#include "modbusMaster.h"
#include "heartbeat.h"
#include "interruptvector.h"
#include "app.h"

/*
*------------------------------------------------------------------------------
* Private Defines
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Processor config bits
*------------------------------------------------------------------------------
*/

#pragma config FOSC 	= INTIO67
#pragma config PLLCFG 	= OFF
#pragma config PRICLKEN = OFF
#pragma config FCMEN 	= OFF
#pragma config IESO 	= OFF
#pragma config PWRTEN   = OFF
#pragma config BOREN 	= OFF
#pragma config BORV 	= 285
#pragma config WDTEN    = OFF
#pragma config WDTPS 	= 512
#pragma config CCP2MX 	= PORTC1
#pragma config PBADEN 	= OFF

#pragma config MCLRE 	= EXTMCLR
#pragma config STVREN 	= OFF
#pragma config XINST 	= OFF
#pragma config LVP      = OFF
#pragma config CP0      = OFF
#pragma config CP1      = OFF
#pragma config CP2      = OFF
#pragma config CP3      = OFF
#pragma config CPB      = OFF
#pragma config CPD      = OFF
#pragma config WRT0     = OFF
#pragma config WRT1     = OFF
#pragma config WRT2     = OFF
#pragma config WRT3     = OFF
#pragma config WRTB     = OFF       
#pragma config WRTC     = OFF
#pragma config WRTD     = OFF
#pragma config EBTR0    = OFF
#pragma config EBTR1    = OFF
#pragma config EBTR2    = OFF
#pragma config EBTR3    = OFF
#pragma config EBTRB    = OFF


/*
*------------------------------------------------------------------------------
* Private Macros
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Data Types
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Variables
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Variables (static)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Constants
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Constants (static)
*------------------------------------------------------------------------------
*/
enum
{
	TIMER0_RELOAD = (65536-20000)
};
/*
*------------------------------------------------------------------------------
* Private Function Prototypes (static)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Functions
*------------------------------------------------------------------------------
*/


/*
*------------------------------------------------------------------------------
* void main(void)

* Summary	: Application specifc main routine. Initializes all port and
*			: pheriperal and put the main task into an infinite loop.
*
* Input		: None
*
* Output	: None
*
*------------------------------------------------------------------------------
*/
void main(void)
{
	UINT8 i;

	BOARD_init();
	TIMER0_init(TIMER0_RELOAD,0);

	APP_init();

	EnableInterrupts();

	//DelayMs(3000);
		

   	while(TRUE)
    {


		if( heartBeatCount >= 100 )
		{
			HB_task();
			heartBeatCount = 0;
		}

		if( comUpdateCount >= 2 )
		{
			MB_task();
			//COM_task();
			comUpdateCount = 0;
		}

		//ClrWdt();

		if( appUpdateCount >= 50 )
		{
			APP_task();
			appUpdateCount = 0;
		}
		//ClrWdt();


		ClrWdt();
				
    }
}




/*
*  End of main.c
*/