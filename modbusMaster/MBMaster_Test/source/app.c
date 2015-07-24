
/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/
#include "config.h"
#include "board.h"
#include "timer.h"
#include "modbusMaster.h"
#include "app.h"
#include "string.h"
#include "eep.h"



//#define SIMULATION

/*
*------------------------------------------------------------------------------
* modbus 
*------------------------------------------------------------------------------
*/

// This is the easiest way to create new packets
// Add as many as you want. TOTAL_NO_OF_PACKETS
// is automatically updated.
enum
{
  PACKET1,
  //PACKET2,
  TOTAL_NO_OF_PACKETS // leave this last entry
};

// Create an array of Packets to be configured
Packet packets[TOTAL_NO_OF_PACKETS];

MBErrorCode PACKET_SENT = MB_TXDONE;
MBErrorCode RETRIES_DONE = MB_TIMEDOUT;

/*
*------------------------------------------------------------------------------
* Structures
*------------------------------------------------------------------------------
*/
																			//Object to store data of raised issue

typedef struct _LOG
{
	UINT8 prevIndex;
	UINT8 writeIndex;
	UINT8 readIndex;
	UINT16 entries[MAX_LOG_ENTRIES][LOG_BUFF_SIZE];
	UINT8 regCount[MAX_LOG_ENTRIES];     // Buffer used to hold the number of 16bits counts in data pack
}LOG;																			//Object to store log entries




/*
*------------------------------------------------------------------------------
* Variables
*------------------------------------------------------------------------------
*/

#pragma idata LOG_DATA
LOG log = {0};
UINT8 buffer[][10] = { 	{"01234567"}, 
						{"ABCDEFGH"}, 
						{"abcdefgh"},
					};
#pragma idata


void updateLog( UINT8 *data );

/*------------------------------------------------------------------------------
* Private Functions
*------------------------------------------------------------------------------
*/




/*
*------------------------------------------------------------------------------
* void APP-init(void)
*------------------------------------------------------------------------------
*/

void APP_init(void)
{

	UINT16 i;
	UINT8 regCount;
	UINT8 ackStatus = 0;
	UINT8 *ptr = buffer[i];
	*ptr++;
	regCount = *ptr;



/*	regCount = strlen(buffer);

	MB_construct(&packets[PACKET1], SLAVE_ID, PRESET_MULTIPLE_REGISTERS, 
							STARTING_ADDRESS, regCount/2, buffer);	
*/		

	//modbus master initialization
	MB_init(SYSTEM_CLOCK, BAUD_RATE, TIMEOUT, POLLING, RETRY_COUNT, packets, TOTAL_NO_OF_PACKETS );

}




/*
*------------------------------------------------------------------------------
* void APP-task(void)
*------------------------------------------------------------------------------
*/
void APP_task(void)
{
	MBErrorCode status;
	UINT8 regCount;
	static UINT8 count;
	static UINT8 i = 0;

	count++;
	
	if( count >= 30 )
	{
		if( i > 2 )
			i = 0;
		updateLog( buffer[i++] );

		count = 0;
	}
	
	status = MB_getStatus();

	if( (status == PACKET_SENT) || (status == RETRIES_DONE) )
	{
		
		//check for log entry, if yes write it to modbus			
		if(log.readIndex != log.writeIndex)
		{			
			MB_write(&packets[PACKET1], SLAVE_ID, PRESET_MULTIPLE_REGISTERS, 
								STARTING_ADDRESS, log.regCount[log.readIndex], log.entries[log.readIndex]);	

			log.readIndex++;
		
			// check for the overflow
			if( log.readIndex >= MAX_LOG_ENTRIES )
				log.readIndex = 0;
								
	
		}
	}
	else
		return;



}





/*---------------------------------------------------------------------------------------------------------------
*	void updateLog(void)
*----------------------------------------------------------------------------------------------------------------
*/
void updateLog( UINT8 *data )
{
	UINT8 i = 0, j = 0;
	UINT8 *ptr = log.entries[log.writeIndex];

	while( *(data+(i*2)) != '\0' )
	{
		*(ptr+((i*2)+1)) = *(data+(i*2));
		i++;
	}

	while( *(data+((j*2)+1)) != '\0' )
	{
		*(ptr+(j*2)) = *(data+((j*2)+1));
		j++;
	}

	if( i >= j )
		log.regCount[log.writeIndex] = i;
	else if ( j >= i )
		log.regCount[log.writeIndex] = j;
	
	log.writeIndex++;
	if( log.writeIndex >= MAX_LOG_ENTRIES)
		log.writeIndex = 0;
}

	

