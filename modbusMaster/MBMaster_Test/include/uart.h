
#ifndef UART_DRIVER_H
#define UART_DRIVER_H




/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/

#include "typedefs.h"
#include "config.h"


/*
*------------------------------------------------------------------------------
* Public Defines
*------------------------------------------------------------------------------
*/

enum RECEIVER_RESPONSE
{
	IDLE,
	STARTED,
	COMPLETED,
	ERROR
};


/*
*------------------------------------------------------------------------------
* Public Macros
*------------------------------------------------------------------------------
*/
// Enable this macro to enable BCC check in pc communication
//#define 	COMM_CHECK_BCC






/*
*------------------------------------------------------------------------------
* Public Data Types
*------------------------------------------------------------------------------
*/


// the uart structure used to provide access to uart data
typedef struct _UART
{
	UINT8 txBuffIndex;
	UINT8 txDataIndex;  
	INT8  txDataCount;
	UINT8 txBuff[TX_PACKET_SIZE];

	UINT8 rxBuffIndex;
	UINT8 rxDataIndex;
	INT8  rxDataCount;
	UINT8 rxBuff[RX_PACKET_SIZE];
}UART;


/*
*------------------------------------------------------------------------------
* Public Variables (extern)
*------------------------------------------------------------------------------
*/

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

void UART1_init(unsigned long baudRate);

void UART2_init(void);
extern void Uart1_ReceiveHandler(void);
extern void Uart1_TransmitHandler(void);

extern void Uart2_ReceiveHandler(void);
extern void Uart2_TransmitHandler(void);
extern void ResetReceiver(void);
extern void PutrsUART(const rom char *data);
extern void PutrsUARTRam(const UINT8 *data);
extern void PutcUARTRam(const UINT8 data);

extern BOOL UART1_hasData(void);
extern UINT8 UART1_read(void);
extern BOOL UART1_write(UINT8 data);
extern BOOL UART1_transmit(void);

extern BOOL UART2_hasData(void);
extern UINT8 UART2_read(void);
extern BOOL UART2_write(UINT8 data);
extern BOOL UART2_transmit(void);



#endif
/*
*  End of uart_driver.h
*/



