#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

/*
*------------------------------------------------------------------------------
* typedefs.h
*
* Include file for processor specific portable data types.
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
* File				: typedefs.h
* Created by		: Sam
* Last changed by	: Sam
* Last changed		: 28/11/2008
*------------------------------------------------------------------------------
*
* Revision 1.0 28/11/2008 Sam
* Demo release
* Revision 0.0 03/10/2008 Sam
* Initial revision
*
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Defines
*------------------------------------------------------------------------------
*/
//NOTE: - All basic data types are taken from MPLAB C18 compiler manual.
/*
*------------------------------------------------------------------------------
*		Basic Type 			New Type	Size 		Minimum 		Maximum
*------------------------------------------------------------------------------
*/

typedef	unsigned char 		BOOL;		//8 bits 	0 				255
typedef	char	 			INT8;		//8 bits 	-128 			127
//signed char 							//8 bits 	-128 			127
typedef	unsigned char 		UINT8;		//8 bits 	0 				255
typedef	int 				INT16;		//16 bits 	-32,768 		32,767
typedef	unsigned int 		UINT16;		//16 bits 	0 				65,535
//short 								//16 bits 	-32,768 		32,767
//unsigned short 						//16 bits 	0 				65,535
typedef	short long 			INT24;		//24 bits 	-8,388,608 		8,388,607
typedef	unsigned short long UINT24;		//24 bits 	0 				16,777,215
typedef	long 				INT32;		//32 bits 	-2,147,483,648 	2,147,483,647
typedef	unsigned long 		UINT32;		//32 bits 	0 				4,294,967,295
typedef float 				FLOAT;		//32 bits 	-126(Exponent)	128(Exponent)
typedef double 				DOUBLE;		//32 bits 	-126(Exponent) 	128(Exponent)


/*
*------------------------------------------------------------------------------
* eNUM
*------------------------------------------------------------------------------
*/


/*
*------------------------------------------------------------------------------
* Public Macros
*------------------------------------------------------------------------------
*/

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

#ifndef SUCCESS
#define SUCCESS			1
#endif

#ifndef FAILURE			
#define FAILURE			0
#endif


#define assert( expr )

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


#endif
/*
*  End of typedefs.h
*/
