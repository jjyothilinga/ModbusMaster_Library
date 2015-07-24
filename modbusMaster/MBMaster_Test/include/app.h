#ifndef _APP_H_
#define _APP_H_


/*
*----------------------------------------------------------------------------------------------------------------
*	MACROS
*-----------------------------------------------------------------------------------------------------------------
*/

//#define __FACTORY_CONFIGURATION__
#define	COIL_STATUS_CHECK_TIME		10UL

/*
*----------------------------------------------------------------------------------------------------------------
*	Enumerations
*-----------------------------------------------------------------------------------------------------------------
*/
typedef enum _APP_PARAM
{
	MAX_KEYPAD_ENTRIES = 8,
	MAX_ISSUES = 24,
	MAX_DEPARTMENTS = 1,
	MAX_LOG_ENTRIES = 13,
	LOG_BUFF_SIZE = MAX_KEYPAD_ENTRIES+1

}APP_PARAM;

extern void APP_init(void);
extern void APP_task(void);

#endif