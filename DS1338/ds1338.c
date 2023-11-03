/*
 * ds1338.c
 *
 *  Created on: Apr 10, 2023
 *      Author: DL
 */


#include "ds1338.h"

#define	i2cAddressDS1338	0x68
#define i2cTimeOut			1000

void dsGetTime(void);
void dsGetDate(void);

void dsSetTime(void);
void dsSetDate(void);

uint8_t dsBufferRx[8] = {0};
uint8_t dsBufferTx[8] = {0};
//uint8_t dsBufferTx[9] = {0};

uint8_t dsDataBuffer[8] = {0};

ds1338_t ds1338 =
{
	dsDataBuffer,

	dsGetTime,
	dsGetDate,
	dsSetTime,
	dsSetDate,
};

void dsWrite(uint8_t address, uint8_t* data, uint8_t numOfBytes)
{
	i2cSendData(i2cAddressDS1338, address, data, numOfBytes);
}

void dsRead(uint8_t address, uint8_t numOfBytes)
{
	if( address + numOfBytes > 8) return;

	i2cReceiveData(i2cAddressDS1338, address, dsBufferRx + address, numOfBytes);
}

void dsGetTime(void)
{
	dsRead(dsSeconds, 3);

	dsDataBuffer[dsSeconds] = 10*((dsBufferRx[dsSeconds]&0x70)>>4) + (dsBufferRx[dsSeconds]&0x0F);
	dsDataBuffer[dsMinutes] = 10*((dsBufferRx[dsMinutes]&0x70)>>4) + (dsBufferRx[dsMinutes]&0x0F);

	dsDataBuffer[dsHours] = dsBufferRx[dsHours]&0x0F;
	if( dsBufferRx[dsHours]&0x40 )
	{
		// 12 hours
		dsDataBuffer[dsHours] += 10*( (dsBufferRx[dsHours]&0x10) != 0 ) + 12*( (dsBufferRx[dsHours]&0x20) != 0 );
	}
	else
	{
		// 24 hours
		dsDataBuffer[dsHours] += 10*( (dsBufferRx[dsHours]&0x30)>>4 );
	}
}

void dsGetDate(void)
{
	dsRead(dsWeekday, 4);

	dsDataBuffer[dsWeekday] = dsBufferRx[dsWeekday]	&0x07;
	dsDataBuffer[dsDay] 	= 10*((dsBufferRx[dsDay]	&0x30)>>4) 	+ (dsBufferRx[dsDay]	&0x0F);
	dsDataBuffer[dsMonth] 	= 10*((dsBufferRx[dsMonth]	&0x10)!= 0)	+ (dsBufferRx[dsMonth]	&0x0F);
	dsDataBuffer[dsYear] 	= 10*((dsBufferRx[dsYear]	&0xF0)>>4) 	+ (dsBufferRx[dsYear]	&0x0F);
}

void dsSetTime(void)
{
	dsBufferTx[dsSeconds] 	= (((dsDataBuffer[dsSeconds]/10)<<4 )&0x70) | ((dsDataBuffer[dsSeconds]	%10)&0x0F);
	dsBufferTx[dsMinutes] 	= (((dsDataBuffer[dsMinutes]/10)<<4 )&0x70) | ((dsDataBuffer[dsMinutes]	%10)&0x0F);
	dsBufferTx[dsHours]		= (((dsDataBuffer[dsHours]	/10)<<4 )&0x70) | ((dsDataBuffer[dsHours]	%10)&0x0F);

	dsWrite(dsSeconds, dsBufferTx + dsSeconds, 3);
}

void dsSetDate(void)
{
	dsBufferTx[dsWeekday] 	= dsDataBuffer[dsWeekday]&0x07;
	dsBufferTx[dsDay] 		= (((dsDataBuffer[dsDay]	/10)<<4 )&0x30) | ((dsDataBuffer[dsDay]		%10)&0x0F);
	dsBufferTx[dsMonth] 	= (((dsDataBuffer[dsMonth]	/10)<<4 )&0x10) | ((dsDataBuffer[dsMonth]	%10)&0x0F);
	dsBufferTx[dsYear] 		= (((dsDataBuffer[dsYear]	/10)<<4 )&0xF0) | ((dsDataBuffer[dsYear]	%10)&0x0F);

	dsWrite(dsWeekday, dsBufferTx + dsWeekday, 4);
}
