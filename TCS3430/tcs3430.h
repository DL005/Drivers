/*
 * tcs3430.h
 *
 *  Created on: Apr 19, 2023
 *      Author: DL
 */

#ifndef TCS3430_H_
#define TCS3430_H_

#include "main.h"
#include "string.h"
#include "i2c.h"

#define tcs3430_hi2c	hi2c1

typedef enum
{
	tcsENABLE,
	tcsATIME,
	tcsWTIME,
	tcsAILTL,
	tcsAILTH,
	tcsAIHTL,
	tcsAIHTH,
	tcsPERS,
	tcsCFG0,
	tcsCFG1,
	tcsREVID,
	tcsID,
	tcsSTATUS,
	tcsCH0DATAL,
	tcsCH0DATAH,
	tcsCH1DATAL,
	tcsCH1DATAH,
	tcsCH2DATAL,
	tcsCH2DATAH,
	tcsCH3DATAL,
	tcsCH3DATAH,
	tcsCFG2,
	tcsCFG3,
	tcsAZ_CONFIG,
	tcsINTENAB,
}tcs3430Registers_t;

typedef enum
{
	tcsSTATUS_ASAT = 0x80,
	tcsSTATUS_AINT = 0x10,
}tcs3430RegMask_status_t;

typedef struct
{
	uint8_t ID;
	uint8_t ID_Rev;

	uint16_t Threshold[2];
	uint16_t XYZ[3];
	uint16_t IR[2];

	uint8_t StatePower;
	uint8_t StateWait;
	uint8_t StateALS;
	uint8_t StateInterrupt;

	uint16_t MaxValue;
	uint32_t IntegrationTimeIn_us;
	uint32_t WaitTimeIn_us;

	uint8_t WaitLong;

	uint8_t ModeIR;

	uint8_t GainALS;

	uint8_t ClearStatusAfterRead;
	uint8_t SleepAfterInterrupt;
	uint8_t AlwaysStartAtPrev;

	uint8_t IntWhenConsValuesOutOfRange;
	uint8_t IntASAT;
	uint8_t IntALS;

	uint8_t RunAutozeroAfter;
}tcs3430Data_t;

typedef struct
{
	void (*UpdateDataFromRegs)(tcs3430Registers_t regStart, tcs3430Registers_t regStop);
	void (*Power)(uint8_t state, uint8_t withALS);
	void (*SetWait)(uint8_t state);
	uint8_t (*SetIntegrationTime)(uint16_t numOf2780usIntervals);
	uint8_t (*SetWaitCyclesBetweenALS)(uint16_t waitCycles);
	uint8_t (*SetInterruptMode)(uint8_t numOfConsValuesOutOfRange);
	void (*WaitLong)(uint8_t state);
	void (*SetModeALS)(uint8_t xyzOrIR);
	uint8_t (*SetGainALS)(uint8_t gain);
	uint8_t (*GetID)(void);
	uint8_t (*GetREVID)(void);
	uint8_t (*GetStatusInterrupt)(uint8_t clear);
	void (*GetChData)(void);
	void (*SetStatusClearAfterRead)(uint8_t state);
	void (*SetSAI)(uint8_t state);
	void (*ClearStatus)(void);
	void (*AlwaysStartAt)(uint8_t zeroOrPrev);
	void (*RunAutozeroAfter)(uint8_t numOfIterations);
	void (*SetStateInterruptASAT)(uint8_t state);
	void (*SetStateInterruptALS)(uint8_t state);
	void (*SetLowThreshold)(uint16_t value);
	void (*SetHighThreshold)(uint16_t value);
	uint16_t (*GetMaximumValueALS)(void);
}tcs3430Functions_t;

typedef struct
{
	tcs3430Data_t Data;
	tcs3430Functions_t Functions;
}tcs3430_t;

extern tcs3430_t tcs3430;

void initTCS3430(void);
void cbTCS3430(void);	// Call me from EXTI IRQ

//void tcsTest(void);

#endif /* TCS3430_H_ */
