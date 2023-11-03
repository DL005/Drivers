/*
 * tcs3430_defines.h
 *
 *  Created on: Apr 19, 2023
 *      Author: DL
 */

#ifndef TCS3430_DEFINES_H_
#define TCS3430_DEFINES_H_

#define tcs3430Addr		0x39

typedef enum
{
	tcsENABLE_WEN = 0x08,
	tcsENABLE_AEN = 0x02,
	tcsENABLE_PON = 0x01,
}tcs3430RegMask_enable_t;

typedef enum
{
	tcsPERS_EveryALS = 00,
	tcsPERS_AnyOutOfThreshold = 01,
	tcsPERS_2ConsOutOfRange = 02,
	tcsPERS_3ConsOutOfRange = 03,
	tcsPERS_5ConsOutOfRange = 04,
	tcsPERS_10ConsOutOfRange = 05,
	tcsPERS_15ConsOutOfRange = 06,
	tcsPERS_20ConsOutOfRange = 07,
	tcsPERS_45ConsOutOfRange = 12,
	tcsPERS_50ConsOutOfRange = 13,
	tcsPERS_55ConsOutOfRange = 14,
	tcsPERS_60ConsOutOfRange = 15,
}tcs3430RegMask_pers_t;

typedef enum
{
	tcsCFG0_WLONG = 0x84,
	tcsCFG0_Empty = 0x80,
}tcs3430RegMask_cfg0_t;

typedef enum
{
	tcsCFG1_AMUX = 0x08,
	tcsCFG1_AGAINx64 = 0x03,
	tcsCFG1_AGAINx16 = 0x02,
	tcsCFG1_AGAINx4 = 0x01,
	tcsCFG1_AGAINx1 = 0x00,
}tcs3430RegMask_cfg1_t;

typedef enum
{
	tcsREVID_Mask = 0x07,
}tcs3430RegMask_revid_t;

typedef enum
{
	tcsID_Mask = 0xFC,
}tcs3430RegMask_id_t;

typedef enum
{
	tcsCFG2_HGAIN = 0x14,
	tcsCFG2_Empty = 0x04,
}tcs3430RegMask_cfg2_t;

typedef enum
{
	tcsCFG3_INTREADCLEAR = 0x8C,
	tcsCFG3_SAI = 0x1C,
	tcsCFG3_Empty = 0x0C,
}tcs3430RegMask_cfg3_t;

typedef enum
{
	tcsAZCONFIG_MODE = 0x80,
	tcsAZCONFIG_NthIter_Mask = 0x7F,
}tcs3430RegMask_azconfig_t;

typedef enum
{
	tcsINTENAB_ASIEN = 0x80,
	tcsINTENAB_AIEN = 0x10,
}tcs3430RegMask_intenab_t;

const uint8_t tcsRegAddr[] =
{
	0x80,	//ENABLE
	0x81,	//ATIME
	0x83,	//WTIME
	0x84,	//AILTL
	0x85,	//AILTH
	0x86,	//AIHTL
	0x87,	//AIHTH
	0x8C,	//PERS
	0x8D,	//CFG0
	0x90,	//CFG1
	0x91,	//REVID
	0x92,	//ID
	0x93,	//STATUS
	0x94,	//CH0DATAL
	0x95,	//CH0DATAH
	0x96,	//CH1DATAL
	0x97,	//CH1DATAH
	0x98,	//CH2DATAL
	0x99,	//CH2DATAH
	0x9A,	//CH3DATAL
	0x9B,	//CH3DATAH
	0x9F,	//CFG2
	0xAB,	//CFG3
	0xD6,	//AZ_CONFIG
	0xDD,	//INTENAB
};

void tcsUpdateDataFromRegs(tcs3430Registers_t regStart, tcs3430Registers_t regStop);
void tcsPower(uint8_t state, uint8_t withALS);
void tcsSetWait(uint8_t state);
uint8_t tcsSetIntegrationTime(uint16_t numOf2780usIntervals);
uint8_t tcsSetWaitCyclesBetweenALS(uint16_t waitCycles);
uint8_t tcsSetInterruptMode(uint8_t numOfConsValuesOutOfRange);
void tcsWaitLong(uint8_t state);
void tcsSetModeALS(uint8_t xyzOrIR);
uint8_t tcsSetGainALS(uint8_t gain);
uint8_t tcsGetID(void);
uint8_t tcsGetREVID(void);
uint8_t tcsGetStatusSaturation(void);
uint8_t tcsGetStatusInterrupt(void);
uint8_t tcsGetStatus(uint8_t clear);
void tcsGetChData(void);
void tcsSetStatusClearAfterRead(uint8_t state);
void tcsSetSAI(uint8_t state);
void tcsClearStatus(void);
void tcsAlwaysStartAt(uint8_t zeroOrPrev);
void tcsRunAutozeroAfter(uint8_t numOfIterations);
void tcsSetStateInterruptASAT(uint8_t state);
void tcsSetStateInterruptALS(uint8_t state);
void tcsSetLowThreshold(uint16_t value);
void tcsSetHighThreshold(uint16_t value);
uint16_t tcsGetMaximumValueALS(void);

#endif /* TCS3430_DEFINES_H_ */
