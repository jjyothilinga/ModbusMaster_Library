#ifndef UTILITIES_H
#define UTILITIES_H

/*
*------------------------------------------------------------------------------
* ssd.h
*
*/

#include "typedefs.h"
#include <delays.h>
#include "modbusMaster.h"


#define GetSystemClock()		(systemClock)      // Hz
#define GetInstructionClock()	(GetSystemClock()/4)
#define GetPeripheralClock()	GetInstructionClock()

#define Delay10us(us)		Delay10TCYx(((GetInstructionClock()/1000000)*(us)))
#define DelayMs(ms)												\
	do																\
	{																\
		unsigned int _iTemp = (ms); 								\
		while(_iTemp--)												\
			Delay1KTCYx((GetInstructionClock()+999999)/1000000);	\
	} while(0)



#endif









