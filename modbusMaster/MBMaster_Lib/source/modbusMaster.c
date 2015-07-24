#include "modbusMaster.h"
#include "utilities.h"

// SimpleModbusMasterV2rev2

// state machine states
#define IDLE 1
#define WAITING_FOR_REPLY 2
#define WAITING_FOR_TURNAROUND 3

#define BUFFER_SIZE 64


#pragma idata MODBUS_DATA
unsigned char state;
unsigned char retry_count;
unsigned char TxEnablePin;

// frame[] is used to receive and transmit packages. 
// The maximum number of bytes in a modbus packet is 256 bytes
// This is limited to the serial buffer of 64 bytes
unsigned char frame[BUFFER_SIZE] = {0}; 
static unsigned char buffer = 0;
long timeout; // timeout interval
long polling; // turnaround delay interval
unsigned int T1_5; // inter character time out in microseconds
unsigned int frameDelay; // frame time out in microseconds
unsigned long delayStart; // init variable for turnaround and timeout delay
unsigned int total_no_of_packets; 
Packet* packetArray; // packet starting address
Packet* packet; // current packet
unsigned int* register_array; // pointer to masters register array
unsigned long systemClock;


MBErrorCode MB_STATUS = MB_TXDONE;
#pragma idata

// function definitions
void idle(void);
void constructPacket(void);
unsigned char construct_F15(void);
unsigned char construct_F16(void);
MBErrorCode waiting_for_reply(void);
MBErrorCode processReply(void);
void waiting_for_turnaround(void);
void process_F1_F2(void);
void process_F3_F4(void);
MBErrorCode process_F5_F6_F15_F16(void);
MBErrorCode processError(void);
void processSuccess(void);
unsigned int calculateCRC(unsigned char bufferSize);
void sendPacket(unsigned char bufferSize);


/************************************
* Modbus task
************************************/

// Modbus Master State Machine
void MB_task() 
{
	switch (state)
	{
		case IDLE:
		{
			if(MB_STATUS == MB_NOERR)
				MB_STATUS = MB_TXDONE;

			idle();
		}
		break;
		case WAITING_FOR_REPLY:
		MB_STATUS = waiting_for_reply();
		break;
		case WAITING_FOR_TURNAROUND:
		{
			waiting_for_turnaround();
			MB_STATUS = MB_NOERR;
		}
		break;
	}
}

void idle(void)
{
  static unsigned int packet_index = 0;	
	
	unsigned int failed_connections = 0;
	
	unsigned char current_connection;
	
	do
	{		
		if (packet_index == total_no_of_packets) // wrap around to the beginning
			packet_index = 0;
				
		// proceed to the next packet
		packet = &packetArray[packet_index];
		
		// get the current connection status
		current_connection = packet->connection;
		
		//if (!current_connection)
		if( current_connection != 1 )
		{			
			// If all the connection attributes are false return
			// immediately to the main sketch
			if (++failed_connections == total_no_of_packets)
				return;
		}
		packet_index++;     
    
	// if a packet has no connection get the next one		
	}while (!current_connection); 
		
	constructPacket();
}
  
void constructPacket()
{	 
	unsigned char frameSize; 
	unsigned int crc16;

	packet->requests++;
	frame[0] = packet->id;
	frame[1] = packet->function;
	frame[2] = packet->address >> 8; // address Hi
	frame[3] = packet->address & 0xFF; // address Lo
		
	// For functions 1 & 2 data is the number of points
	// For function 5 data is either ON (0xFF00) or OFF (0x0000)
	// For function 6 data is exactly that, one register's data
	// For functions 3, 4 & 16 data is the number of registers
	// For function 15 data is the number of coils
		
	// The data attribute needs to be intercepted by F5 & F6 because these requests
	// include their data in the data register and not in the masters array
	if (packet->function == FORCE_SINGLE_COIL || packet->function == PRESET_SINGLE_REGISTER) 
		packet->data = register_array[packet->local_start_address]; // get the data
	
	
	frame[4] = packet->data >> 8; // MSB
	frame[5] = packet->data & 0xFF; // LSB
	
	   
	
  	// construct the frame according to the modbus function  
  	if (packet->function == PRESET_MULTIPLE_REGISTERS) 
		frameSize = construct_F16();
	else if (packet->function == FORCE_MULTIPLE_COILS)
		frameSize = construct_F15();
	else // else functions 1,2,3,4,5 & 6 is assumed. They all share the exact same request format.
    frameSize = 8; // the request is always 8 bytes in size for the above mentioned functions.
		
	crc16 = calculateCRC(frameSize - 2);	
  	frame[frameSize - 2] = crc16 >> 8; // split crc into 2 bytes
  	frame[frameSize - 1] = crc16 & 0xFF;
  	sendPacket(frameSize);

	state = WAITING_FOR_REPLY; // state change
	
	// if broadcast is requested (id == 0) for function 5,6,15 and 16 then override 
  	// the previous state and force a success since the slave wont respond
	if (packet->id == 0)
			processSuccess();
}

unsigned char construct_F15()
{
	// function 15 coil information is packed LSB first until the first 16 bits are completed
  	// It is received the same way..
  	unsigned char no_of_registers = packet->data / 16;
  	unsigned char no_of_bytes = no_of_registers * 2; 

  	unsigned char bytes_processed = 0;
 	unsigned char index = 7; // user data starts at index 7
	unsigned int temp;	

	unsigned char frameSize;
	unsigned char i;

  	// if the number of points dont fit in even 2byte amounts (one register) then use another register and pad 
  	if (packet->data % 16 > 0) 
  	{
    	no_of_registers++;
    	no_of_bytes++;
  	}
	
  	frame[6] = no_of_bytes;

	
  	for (i = 0; i < no_of_registers; i++)
  	{
    	temp = register_array[packet->local_start_address + i]; // get the data
    	frame[index] = temp & 0xFF; 
    	bytes_processed++;
     
    	if (bytes_processed < no_of_bytes)
    	{
      		frame[index + 1] = temp >> 8;
      		bytes_processed++;
      		index += 2;
    	}
  }
	frameSize = (9 + no_of_bytes); // first 7 bytes of the array + 2 bytes CRC + noOfBytes 
	return frameSize;
}

unsigned char construct_F16()
{
	unsigned char no_of_bytes = packet->data * 2; 
    
  	unsigned char index = 7; // user data starts at index 7
	unsigned char no_of_registers = packet->data;
	unsigned int temp;
	unsigned char i;
	unsigned char frameSize;

	// first 6 bytes of the array + no_of_bytes + 2 bytes CRC 
  	frame[6] = no_of_bytes; // number of bytes
		
  	for (i = 0; i < no_of_registers; i++)
  	{
    	//temp = register_array[packet->local_start_address + i]; // get the data
		temp = (UINT16)register_array[i];
//		frame[index] = temp;
//		index++;
    	frame[index] = temp >> 8;
    	index++;
    	frame[index] = temp & 0xFF;
    	index++;
  	}

	frameSize = (9 + no_of_bytes); // first 7 bytes of the array + 2 bytes CRC + noOfBytes 
	return frameSize;
}

void waiting_for_turnaround()
{
  if ((GetAppTime() - delayStart) > polling)
		state = IDLE;
}

// get the serial data from the buffer
MBErrorCode waiting_for_reply(void)
{
	MBErrorCode status = MB_SENDING;

#if defined (__18f4520)
	if(UART_hasData())
#else
	if(UART1_hasData())
#endif
	{
		unsigned char overflowFlag = 0;
#if defined (__18f4520)
		while(UART_hasData())
#else
		while(UART1_hasData())
#endif
		{
		  // The maximum number of bytes is limited to the serial buffer size 
	      // of BUFFER_SIZE. If more bytes is received than the BUFFER_SIZE the 
	      // overflow flag will be set and the serial buffer will be read until
	      // all the data is cleared from the receive buffer, while the slave is 
	      // still responding.
	
			if (buffer == BUFFER_SIZE)
				overflowFlag = 1;
		
#if defined (__18f4520)			
			frame[buffer] = UART_read();
#else
			frame[buffer] = UART1_read();
#endif

			buffer++;
			
			// This is not 100% correct but it will suffice.
			// worst case scenario is if more than one character time expires
			// while reading from the buffer then the buffer is most likely empty
			// If there are more bytes after such a delay it is not supposed to
			// be received and thus will force a frame_error.
			//delayMicroseconds(T1_5); // inter character time out
			Delay10us(T1_5/10);
		}
		
			
		// The minimum buffer size from a slave can be an exception response of
    	// 5 bytes. If the buffer was partially filled set a frame_error.
		// The maximum number of bytes in a modbus packet is 256 bytes.
		// The serial buffer limits this to 64 bytes.
	
		if ((buffer < 5) || overflowFlag)
			processError();       
      
		// Modbus over serial line datasheet states that if an unexpected slave 
    	// responded the master must do nothing and continue with the time out.
		// This seems silly cause if an incorrect slave responded you would want to
    	// have a quick turnaround and poll the right one again. If an unexpected 
   		// slave responded it will most likely be a frame error in any event
		else if (frame[0] != packet->id) // check id returned
		{
			processError();
			buffer = 0;
		}
		else
		{
			processReply();
			buffer = 0;
		}
	}
	else if ((GetAppTime() - delayStart) > timeout) // check timeout
	{
		status = processError();
		buffer = 0;
		state = IDLE; //state change, override processError() state
	}

	return status;
}

MBErrorCode processReply(void)
{
	MBErrorCode status;

	UINT16 temp = (UINT16)(frame[buffer - 2]);
  	UINT16 calculated_crc = calculateCRC(buffer - 2);

	// combine the crc Low & High bytes
  	UINT16 received_crc = temp << 8; //(UINT16)(frame[buffer - 2] << 8); 
	received_crc |= (UINT16)frame[buffer - 1]; 
	
	if (calculated_crc == received_crc) // verify checksum
	{
		// To indicate an exception response a slave will 'OR' 
		// the requested function with 0x80 
		if ((frame[1] & 0x80) == 0x80) // extract 0x80
		{
			packet->exception_errors++;
			processError();
			status = MB_EXCEPTION;
		}
		else
		{
			switch (frame[1]) // check function returned
      		{
        		case READ_COIL_STATUS:
        		case READ_INPUT_STATUS:
        			process_F1_F2();
        		break;
        		case READ_INPUT_REGISTERS:
        		case READ_HOLDING_REGISTERS:
        			process_F3_F4();
        		break;
				case FORCE_SINGLE_COIL:
				case PRESET_SINGLE_REGISTER:
        		case FORCE_MULTIPLE_COILS:
        		case PRESET_MULTIPLE_REGISTERS:
        			status = process_F5_F6_F15_F16();
        		break;
        		default: // illegal function returned
        			status = processError();
        		break;
      		}
		}
	} 
	else // checksum failed
	{
		status = processError();
	}

	return status;
}

void process_F1_F2()
{
	// packet->data for function 1 & 2 is actually the number of boolean points
  	unsigned char no_of_registers = packet->data / 16;
  	unsigned char number_of_bytes = no_of_registers * 2; 
    unsigned char bytes_processed = 0;
    unsigned char index = 3; // start at the 4th element in the frame and combine the Lo byte  
    unsigned int temp;
	unsigned char i;
       
 	 // if the number of points dont fit in even 2byte amounts (one register) then use another register and pad 
  	if (packet->data % 16 > 0) 
  	{
    	no_of_registers++;
    	number_of_bytes++;
  	}
             
  	if (frame[2] == number_of_bytes) // check number of bytes returned
  	{ 
    	for (i = 0; i < no_of_registers; i++)
    	{
      		temp = frame[index]; 
      		bytes_processed++;
      		if (bytes_processed < number_of_bytes)
      		{
				temp = (frame[index + 1] << 8) | temp;
        		bytes_processed++;
        		index += 2;
      		}
      		register_array[i] = temp;
    	}

    	processSuccess(); 
  }
  else // incorrect number of bytes returned 
    processError();
}

void process_F3_F4()
{
	unsigned char index = 3;
	unsigned char i;

	// check number of bytes returned - unsigned int == 2 bytes
  	// data for function 3 & 4 is the number of registers
  	if (frame[2] == (packet->data * 2)) 
  	{
    	for (i = 0; i < packet->data; i++)
    	{
      		// start at the 4th element in the frame and combine the Lo byte 
      		register_array[packet->local_start_address + i] = (frame[index] << 8) | frame[index + 1]; 
      		index += 2;
    	}
    	processSuccess(); 
  }
  else // incorrect number of bytes returned  
    processError();  
}

MBErrorCode process_F5_F6_F15_F16(void)
{
	MBErrorCode status = MB_TXDONE;

	unsigned int received_address = 0;
	unsigned int received_data = 0;
	
	// The repsonse of functions 5,6,15 & 16 are just an echo of the query
  	received_address = frame[2];
	received_address <<= 8;
	received_address |= frame[3];

	received_data = frame[4];
	received_data <<= 8;
	received_data = frame[5];

		
	if ((received_address == packet->address) && (received_data == packet->data))
		processSuccess();
	else
		status = processError();

	return status;
}

MBErrorCode processError(void)
{
	MBErrorCode status = MB_SENDING;

	packet->retries++;
	packet->failed_requests++;
	
	// if the number of retries have reached the max number of retries 
  	// allowable, stop requesting the specific packet
  	if (packet->retries >= retry_count)
	{
    	packet->connection = 0;
		packet->retries = 0;
		status = MB_TIMEDOUT;
	}
	else 
		status = MB_SENDING;

	state = WAITING_FOR_TURNAROUND;
	delayStart = GetAppTime(); // start the turnaround delay

	return status;
}

void processSuccess()
{
	packet->successful_requests++; // transaction sent successfully

	//added for testing
	packet->connection = 0;
	packet->retries = 0; // if a request was successful reset the retry counter
	state = WAITING_FOR_TURNAROUND;
	delayStart = GetAppTime(); // start the turnaround delay
}
  
void MB_init( unsigned long system_clock,								
											long baud,
											long _timeout, 
											long _polling, 
											unsigned char _retry_count,  
											Packet* _packets, 
											unsigned int _total_no_of_packets)
{ 
	// Modbus states that a baud rate higher than 19200 must use a fixed 750 us 
  	// for inter character time out and 1.75 ms for a frame delay for baud rates
  	// below 19200 the timing is more critical and has to be calculated.
  	// E.g. 9600 baud in a 11 bit packet is 9600/11 = 872 characters per second
  	// In GetAppTime()econds this will be 872 characters per 1000ms. So for 1 character
  	// 1000ms/872 characters is 1.14583ms per character and finally modbus states
  	// an inter-character must be 1.5T or 1.5 times longer than a character. Thus
  	// 1.5T = 1.14583ms * 1.5 = 1.71875ms. A frame delay is 3.5T.
	// Thus the formula is T1.5(us) = (1000ms * 1000(us) * 1.5 * 11bits)/baud
	// 1000ms * 1000(us) * 1.5 * 11bits = 16500000 can be calculated as a constant
	
	if (baud > 19200)
		T1_5 = 750; 
	else 
		T1_5 = 16500000/baud; // 1T * 1.5 = T1.5
		
	/* The modbus definition of a frame delay is a waiting period of 3.5 character times
		 between packets. This is not quite the same as the frameDelay implemented in
		 this library but does benifit from it.
		 The frameDelay variable is mainly used to ensure that the last character is 
		 transmitted without truncation. A value of 2 character times is chosen which
		 should suffice without holding the bus line high for too long.*/
		 
	frameDelay = T1_5 * 2; 
	
	// initialize
	state = IDLE;
  	timeout = _timeout;
  	polling = _polling;
	retry_count = _retry_count;
	total_no_of_packets = _total_no_of_packets;
	packetArray = _packets;
	
	//initialize uart with the required baud rate
#if defined (__18f4520)
	UART_init(baud);
#else
	UART1_init(baud);
#endif

	//store the system clock
	systemClock = system_clock;


} 

void MB_write(far Packet *_packet, 
											unsigned char id, 
											unsigned char function, 
											unsigned int address, 
											unsigned int dataCount,
											unsigned int *dataBuffer
											)
{
	_packet->id = id;
  	_packet->function = function;
  	_packet->address = address;
  	_packet->data = dataCount;
	//_packet->local_start_address = local_start_address;
	_packet->connection = 1;
	register_array = dataBuffer;
}

unsigned int calculateCRC(unsigned char bufferSize) 
{
  	unsigned int temp, temp2, flag;
	unsigned char i, j;

  	temp = 0xFFFF;
  	for (i = 0; i < bufferSize; i++)
  	{
    	temp = temp ^ frame[i];
    	for (j = 1; j <= 8; j++)
    	{
      		flag = temp & 0x0001;
      		temp >>= 1;
      		if (flag)
        		temp ^= 0xA001;
    	}
  	}

  	// Reverse byte order. 
  	temp2 = temp >> 8;
  	temp = (temp << 8) | temp2;
  	temp &= 0xFFFF;
  	// the returned value is already swapped
  	// crcLo byte is first & crcHi byte is last
  	return temp; 
}

void sendPacket(unsigned char bufferSize)
{
	unsigned char i;

	for (i = 0; i < bufferSize; i++)
#if defined (__18f4520)
		UART_write(frame[i]);
#else
		UART1_write(frame[i]);
#endif

#if defined (__18f4520)
	UART_transmit();
#else
	UART1_transmit();
#endif
	
	
	//delayMicroseconds(frameDelay);
	Delay10us(frameDelay/10);
	
	delayStart = GetAppTime(); // start the timeout delay	
}


MBErrorCode MB_getStatus(void)
{
	return MB_STATUS;
}
