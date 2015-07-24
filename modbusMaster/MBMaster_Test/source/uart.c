
/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/
#include "board.h"
#include "config.h"
#include <delays.h>
#include <string.h>
#include "typedefs.h"
#include "uart.h"
#include <usart.h>


/*
*------------------------------------------------------------------------------
* Private Defines
*------------------------------------------------------------------------------
*/
/*

enum RECEIVER_STATE
{
	UART_RX_STATE_IDLE,
	UART_RX_STATE_RECV_START,
	UART_RX_STATE_WAIT
};
*/
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



#pragma idata UART_DATA
UART uart[ACTIVE_USARTS] = {0,0,0,{0}};

#pragma idata
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

/*
*------------------------------------------------------------------------------
* Private Function Prototypes (static)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Functions
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Functions
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* void  UartReceiveHandler(void)
*
* Summary	: Handle the recepion of data throug UART
*
* Input		: None
*
* Output	: None
*
* Note		: PC Command Fromat: (PC to Device)
*
* Field:	SOF     STATIONID  DATALEN   CMD/STATUS    DATAN           BCC     EOF
*
* offset:	0	    1	       2	     3		       4               N+4     N+5
*
* value:	0xAA	0x00	   N+1	     CMD	       DATA[0..N-1]    BCC     0xBB
*
* Desc:		BCC = STATIONID ^ DATALEN ^ CMD/STATUS ^ DATA[0] ^ DATA[1]...^... DATA[N-1]
*
*------------------------------------------------------------------------------
*/
#pragma code ISR
#pragma interrupt Uart1_ReceiveHandler
void  Uart1_ReceiveHandler(void)
{
  	volatile UINT8 data;
  	// Get the character received from the UART
#if(defined __18F8722_H) ||(defined __18F46K22_H)
  	data = Read1USART();
#else
  	data = ReadUSART();
#endif
    uart[0].rxBuff[uart[0].rxBuffIndex++] = data;

	if( uart[0].rxBuffIndex >= RX_PACKET_SIZE)
	{
		uart[0].rxBuffIndex = 0;
	}
	uart[0].rxDataCount++;

	// Clear the interrupt flag
    PIR1bits.RC1IF = 0;
}

#pragma code



#pragma interrupt Uart2_ReceiveHandler
void  Uart2_ReceiveHandler(void)
{
  	volatile UINT8 data;
  	// Get the character received from the UART
#if(defined __18F8722_H) ||(defined __18F46K22_H)
  	data = Read2USART();
#else
  	data = ReadUSART();
#endif
    uart[1].rxBuff[uart[1].rxBuffIndex++] = data;

	if( uart[1].rxBuffIndex >= RX_PACKET_SIZE)
	{
		uart[1].rxBuffIndex = 0;
	}

	uart[1].rxDataCount++;

	// Clear the interrupt flag
    PIR3bits.RC2IF = 0;
}


/*
*
*
*/

#pragma interrupt Uart1_TransmitHandler
void Uart1_TransmitHandler(void)
{
	volatile UINT8 data ;
	if( uart[0].txDataCount == 0 )	//check whether there is data to be transmitted
	{
#if(defined __18F8722_H) ||(defined __18F46K22_H)
		while(Busy1USART());
#else
		while(BusyUSART());
#endif


#ifdef __RS485__
		TX1_EN = 0;				//disable RS485 control
#endif
		TXSTA1bits.TXEN = 0;		//disable transmission
		return;
	}
	data = uart[0].txBuff[uart[0].txDataIndex];
	TXREG1 = data;	//transmit
	uart[0].txDataIndex++;		//increase the dataIndex to point to the next data to be transmitted
	if( uart[0].txDataIndex >= TX_PACKET_SIZE)		//on rollover
	{
		uart[0].txDataIndex = 0;		//reset
	}
	if( uart[0].txDataCount > 0)
	uart[0].txDataCount--;		//decrement the data count to indicate transmission

}

#pragma interrupt Uart2_TransmitHandler	
void Uart2_TransmitHandler(void)
{
	volatile UINT8 data ;
	if( uart[1].txDataCount == 0 )	//check whether there is data to be transmitted
	{
#if(defined __18F8722_H) ||(defined __18F46K22_H)
		while(Busy2USART());
#else
		while(BusyUSART());
#endif

#ifdef __RS485__
		TX2_EN = 0;				//disable RS485 control
#endif
		TXSTA2bits.TXEN = 0;		//disable transmission
		return;
	}
	data = uart[1].txBuff[uart[1].txDataIndex];
	TXREG2 = data;	//transmit
	uart[1].txDataIndex++;		//increase the dataIndex to point to the next data to be transmitted
	if( uart[1].txDataIndex >= TX_PACKET_SIZE)		//on rollover
	{
		uart[1].txDataIndex = 0;		//reset
	}
	if( uart[1].txDataCount > 0)
	uart[1].txDataCount--;		//decrement the data count to indicate transmission

}


/*
*------------------------------------------------------------------------------
* void InitializeUART(void)
*
* Summary	: Initialize Uart for communication with PC
*			  char conunters
*				Transimission and Reception interrupts are enabled
*				
* 				Refer PIC 18F4520 datasheet Table 18-3.
*
* Input		: None
*
* Output	: None
*------------------------------------------------------------------------------
*/
void UART1_init(unsigned long baudRate)
{
	UINT8 uartConfig= USART_TX_INT_OFF &
				USART_RX_INT_ON &
				USART_ASYNCH_MODE &
				USART_EIGHT_BIT &
				USART_CONT_RX &
				USART_BRGH_HIGH;

	baudRate += (unsigned long)1;
	baudRate *= (unsigned long)16;
	baudRate = GetSystemClock() / 	baudRate;

	// configure USART
	Open1USART( uartConfig , baudRate);	

// Enable interrupt priority
// 	RCONbits.IPEN = 1;

	TXSTAbits.TXEN = 1;	//enable transmission
	TXSTAbits.CSRC = 0;
	//PIE1bits.RC1IE = 1;

//	IPR1bits.TXIP = 1;	//make transmit interrupt high priority


 	
}


void UART2_init(void)
{
	UINT8 uartConfig= USART_TX_INT_OFF &
				USART_RX_INT_ON &
				USART_ASYNCH_MODE &
				USART_EIGHT_BIT &
				USART_CONT_RX &
				USART_BRGH_HIGH;

	Open2USART( uartConfig ,
				//77);
				//129 );
				//10 );
			    //  25 );
				//51 );
				103);
// Enable interrupt priority
  	RCONbits.IPEN = 1;
	RCSTA2bits.SPEN = 1;
	TXSTA2bits.TXEN = 1;	//enable transmission
	TXSTA2bits.CSRC = 0;
	PIE3bits.RC2IE = 1;

//	IPR1bits.TXIP = 1;	//make transmit interrupt high priority



  	// Make receive interrupt high priority
  	IPR3bits.RC2IP = 1;

}

/*
*------------------------------------------------------------------------------
* BOOL UART1_write(UINT8 data)
*
* Summary	: Fill the transmit buffer of the uart with data
* Input		: data to be transmitted which is stored in the txBuff
*
* Output	: Boolean indicating success or failure
* 
*------------------------------------------------------------------------------
*/
BOOL UART1_write(UINT8 data)
{

#if(defined __18F8722_H) ||(defined __18F46K22_H)
	while(Busy1USART());					//wait for current transmission if any
#else
	while(BusyUSART());					//wait for current transmission if any
#endif
	//DISABLE_UART_TX_INTERRUPT();		//disable the transmit interrupt
	uart[0].txBuff[uart[0].txBuffIndex++] = data;	//store the data in the tx buffer
	uart[0].txDataCount++;		//increment the data count
	if(uart[0].txBuffIndex >= TX_PACKET_SIZE)	//check for overflow
	{
		uart[0].txBuffIndex = 0;
	}

	//ENABLE_UART_TX_INTERRUPT();
		
	return TRUE;
}


BOOL UART2_write(UINT8 data)
{

#if(defined __18F8722_H) ||(defined __18F46K22_H)
	while(Busy2USART());					//wait for current transmission if any
#else
	while(BusyUSART());					//wait for current transmission if any
#endif
	//DISABLE_UART_TX_INTERRUPT();		//disable the transmit interrupt
	uart[1].txBuff[uart[1].txBuffIndex++] = data;	//store the data in the tx buffer
	uart[1].txDataCount++;		//increment the data count
	if(uart[1].txBuffIndex >= TX_PACKET_SIZE)	//check for overflow
	{
		uart[1].txBuffIndex = 0;
	}

	//ENABLE_UART_TX_INTERRUPT();
		
	return TRUE;
}



/*
*------------------------------------------------------------------------------
* BOOL UART1_transmit(void)
*
* Summary	: Fill the transmit buffer of the uart with data
* Input		: data to be transmitted which is stored in the txBuff
*
* Output	: Boolean indicating success or failure
* 
*------------------------------------------------------------------------------
*/
BOOL UART1_transmit(void)
{
/*
	DISABLE_UART_TX_INTERRUPT();		//disable the transmit interrupt
	if( TXSTA1bits.TXEN == 0 )		//data count ==1 requires initiation of transmission
	{
		while(BusyUSART());			//wait for current transmission to complete 
#ifdef __RS485__
		TX1_EN = 1;					//enable RS485 transmission
#endif
		TXSTA2bits.TXEN = 1;		//enable uart transmission
		ENABLE_UART_TX_INTERRUPT();

		return TRUE;
	}
		
	ENABLE_UART_TX_INTERRUPT();
	return FALSE;
*/

	UINT8 i,data;
#ifdef __RS485__
	TX1_EN = 1;	//enable the tx control for rs485 communication
#endif
	DelayMs(1);
	while(uart[0].txDataCount > 0 )
	{
		
		
		data = uart[0].txBuff[uart[0].txDataIndex];
#if(defined __18F8722_H) ||(defined __18F46K22_H)
		while (!TXSTA1bits.TRMT);	//check whether ready to transmit
		TXREG1=data;					//load new data for transmission
	
		while (!TXSTA1bits.TRMT);	//check whether ready to transmit

#else

		while (!TXSTAbits.TRMT);	//check whether ready to transmit
		TXREG=data;					//load new data for transmission
	
		while (!TXSTAbits.TRMT);	//check whether ready to transmit

#endif
		uart[0].txDataIndex++;		//increase the dataIndex to point to the next data to be transmitted
		if( uart[0].txDataIndex >= TX_PACKET_SIZE)		//on rollover
		{
			uart[0].txDataIndex = 0;		//reset
		}
		if( uart[0].txDataCount > 0)
			uart[0].txDataCount--;		//decrement the data count to indicate transmission
		
	}

#ifdef __RS485__
	TX1_EN = 0;	//disable the tx control for rs485 communication
#endif

	DelayMs(1);
	

}
	


BOOL UART2_transmit(void)
{
/*
	DISABLE_UART_TX_INTERRUPT();		//disable the transmit interrupt
	if( TXSTA2bits.TXEN == 0 )		//data count ==1 requires initiation of transmission
	{
		while(BusyUSART());			//wait for current transmission to complete 
		TX2_EN = 1;					//enable RS485 transmission
		TXSTA2bits.TXEN = 1;		//enable uart transmission
		ENABLE_UART_TX_INTERRUPT();

		return TRUE;
	}
		
	ENABLE_UART_TX_INTERRUPT();
	return FALSE;
*/

	UINT8 i,data;

#ifdef __RS485__
	TX2_EN = 1;	//enable the tx control for rs485 communication
#endif

	DelayMs(1);
	while(uart[1].txDataCount > 0 )
	{
		
		
		data = uart[1].txBuff[uart[1].txDataIndex];
#if(defined __18F8722_H) ||(defined __18F46K22_H)
		while (!TXSTA2bits.TRMT);	//check whether ready to transmit
		TXREG2=data;					//load new data for transmission
	
		while (!TXSTA2bits.TRMT);	//check whether ready to transmit

#else

		while (!TXSTAbits.TRMT);	//check whether ready to transmit
		TXREG=data;					//load new data for transmission
	
		while (!TXSTAbits.TRMT);	//check whether ready to transmit

#endif
		uart[1].txDataIndex++;		//increase the dataIndex to point to the next data to be transmitted
		if( uart[1].txDataIndex >= TX_PACKET_SIZE)		//on rollover
		{
			uart[1].txDataIndex = 0;		//reset
		}
		if( uart[1].txDataCount > 0)
		uart[1].txDataCount--;		//decrement the data count to indicate transmission
		
	}
#ifdef __RS485__
	TX2_EN = 0;	//disable the tx control for rs485 communication
#endif

	DelayMs(1);

}


/*
*------------------------------------------------------------------------------
BOOL UART_hasData(void)
*
* Summary	: indicates whether data has been received from the uart
* Input		: None
*
* Output	: the data from the rxBuff at index = rxDataIndex
* 
*------------------------------------------------------------------------------
*/
BOOL UART2_hasData(void)
{
	BOOL result = FALSE;
#if(defined __18F8722_H) ||(defined __18F46K22_H)
	DISABLE_UART2_RX_INTERRUPT();
	if(uart[1].rxDataCount > 0)
		result = TRUE; 
	ENABLE_UART2_RX_INTERRUPT();
#else

	DISABLE_UART_RX_INTERRUPT();
	if(uart.rxDataCount > 0)
		result = TRUE; 
	ENABLE_UART_RX_INTERRUPT();
#endif	
		
	return result;
}

BOOL UART1_hasData(void)
{
	BOOL result = FALSE;

#if(defined __18F8722_H) ||(defined __18F46K22_H)

	DISABLE_UART1_RX_INTERRUPT();
	if(uart[0].rxDataCount > 0)
		result = TRUE; 
	ENABLE_UART1_RX_INTERRUPT();

#else

	DISABLE_UART_RX_INTERRUPT();
	if(uart.rxDataCount > 0)
		result = TRUE; 
	ENABLE_UART_RX_INTERRUPT();
#endif	
		
	return result;
}




/*
*------------------------------------------------------------------------------
UINT8 UART_read(void)
*
* Summary	: return the data from the rxBuff 
* Input		: None
*
* Output	: the data from the rxBuff at index = rxDataIndex
* 
*------------------------------------------------------------------------------
*/
UINT8 UART1_read(void)
{
	UINT8 data;

#if (defined  __18F8722_H )|| (defined  __18F46K22_H)
	DISABLE_UART1_RX_INTERRUPT();
	data = uart[0].rxBuff[uart[0].rxDataIndex++];
	if( uart[0].rxDataIndex >= RX_PACKET_SIZE)
	{
		uart[0].rxDataIndex = 0;
	}
	if( uart[0].rxDataCount > 0 )
		--uart[0].rxDataCount;
	ENABLE_UART1_RX_INTERRUPT();

#else

	DISABLE_UART_RX_INTERRUPT();
	data = uart.rxBuff[uart.rxDataIndex++];
	if( uart.rxDataIndex >= RX_PACKET_SIZE)
	{
		uart.rxDataIndex = 0;
	}
	if( uart.rxDataCount > 0 )
		--uart.rxDataCount;
	ENABLE_UART_RX_INTERRUPT();
#endif

	return data;
}


UINT8 UART2_read(void)
{
	UINT8 data;

#if(defined __18F8722_H) ||(defined __18F46K22_H)
	DISABLE_UART2_RX_INTERRUPT();
	data = uart[1].rxBuff[uart[1].rxDataIndex++];
	if( uart[1].rxDataIndex >= RX_PACKET_SIZE)
	{
		uart[1].rxDataIndex = 0;
	}
	if( uart[1].rxDataCount > 0 )
		--uart[1].rxDataCount;
	ENABLE_UART1_RX_INTERRUPT();

#else

	DISABLE_UART_RX_INTERRUPT();
	data = uart.rxBuff[uart.rxDataIndex++];
	if( uart.rxDataIndex >= RX_PACKET_SIZE)
	{
		uart.rxDataIndex = 0;
	}
	if( uart.rxDataCount > 0 )
		--uart.rxDataCount;
	ENABLE_UART_RX_INTERRUPT();
#endif

	return data;
}



/*
*  End of uart_driver.c
*/
