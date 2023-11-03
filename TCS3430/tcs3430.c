/*
 * tcs3430.c
 *
 *  Created on: Apr 19, 2023
 *      Author: DL
 */

#include "tcs3430.h"
#include "tcs3430_defines.h"

uint8_t tcsBufferRx[25] = {0};
uint8_t tcsBufferTx[25] = {0};

tcs3430_t tcs3430 =
{
	{
		0b110111,	//ID;
		1, 			//ID_Rev;

		{0, 0},//Threshold[2];

		{0, 0, 0}, 	// XYZ[3];
		{0, 0},		// IR[2];

		0, // StatePower;
		0, // StateWait;
		0, // StateALS;
		0, // StateInterrupt;

		2780, // IntegrationTimeIn_us;
		2780, // WaitTimeIn_us;
		1024, // MaxValue

		0,	// WaitLong;

		0,	// ModeIR;

		1,	// GainALS;

		0,	// ClearStatusAfterRead;
		0,	// SleepAfterInterrupt;
		0,	// AlwaysStartAtPrev;

		0,	// IntWhenConsValuesOutOfRange;
		0,	// IntASAT;
		0,	// IntALS;

		0x7F,	//	RunAutozeroAfter
	},

	{
		tcsUpdateDataFromRegs,
		tcsPower,
		tcsSetWait,
		tcsSetIntegrationTime,
		tcsSetWaitCyclesBetweenALS,
		tcsSetInterruptMode,
		tcsWaitLong,
		tcsSetModeALS,
		tcsSetGainALS,
		tcsGetID,
		tcsGetREVID,
		tcsGetStatus,
		tcsGetChData,
		tcsSetStatusClearAfterRead,
		tcsSetSAI,
		tcsClearStatus,
		tcsAlwaysStartAt,
		tcsRunAutozeroAfter,
		tcsSetStateInterruptASAT,
		tcsSetStateInterruptALS,
		tcsSetLowThreshold,
		tcsSetHighThreshold,
		tcsGetMaximumValueALS,
	}
};

//----------------------------------------------------------------

void initTCS3430(void)
{
	tcs3430.Functions.Power(1, 1);
//	HAL_Delay(10);

	tcs3430.Functions.UpdateDataFromRegs(tcsCFG0, tcsCFG1);
	tcs3430.Functions.UpdateDataFromRegs(tcsCFG2, tcsCFG3);
	tcs3430.Functions.UpdateDataFromRegs(tcsENABLE, tcsINTENAB);

	tcs3430.Functions.SetStatusClearAfterRead(1);

	tcs3430.Functions.GetChData();
}

// Call me from EXTI IRQ
void cbTCS3430(void)
{
	tcs3430.Data.StateInterrupt = tcsGetStatus( tcs3430.Data.ClearStatusAfterRead );
}

//----------------------------------------------------------------
// Pet me
//----------------------------------------------------------------

void tcsReadRegs(tcs3430Registers_t regStart, uint8_t numOfRegs)
{
	if( regStart + numOfRegs > 25) return;

	for(tcs3430Registers_t reg = regStart; reg < regStart + numOfRegs; reg++)
	{
		HAL_I2C_Mem_Read(&tcs3430_hi2c, tcs3430Addr<<1, tcsRegAddr[reg], I2C_MEMADD_SIZE_8BIT, &tcsBufferRx[reg], 1, 10);
	}
}

void tcsWriteRegs(tcs3430Registers_t regStart, uint8_t numOfRegs)
{
	if( regStart + numOfRegs > 25 ) return;
	if( (regStart >= tcsREVID) && (regStart <= tcsCH3DATAH) ) return;
	if( (regStart + numOfRegs - 1 >= tcsREVID) && (regStart + numOfRegs <= tcsCH3DATAH) ) return;

	for(tcs3430Registers_t reg = regStart; reg < regStart + numOfRegs; reg++)
	{
		HAL_I2C_Mem_Write(&tcs3430_hi2c, tcs3430Addr<<1, tcsRegAddr[reg], I2C_MEMADD_SIZE_8BIT, &tcsBufferTx[reg], 1, 10);
	}
}

//----------------------------------------------------------------

void tcsUpdateDataFromRegs(tcs3430Registers_t regStart, tcs3430Registers_t regStop)
{
	if( regStop > tcsINTENAB ) regStop = tcsINTENAB;

	tcsReadRegs(regStart, regStop - regStart + 1);

	for(tcs3430Registers_t reg = regStart; reg <= regStop; reg++)
	{
		switch(reg)
		{
			case tcsENABLE:
				tcs3430.Data.StatePower	= (tcsBufferRx[tcsENABLE]&tcsENABLE_PON) != 0;
				tcs3430.Data.StateALS	= (tcsBufferRx[tcsENABLE]&tcsENABLE_AEN) != 0;
				tcs3430.Data.StateWait 	= (tcsBufferRx[tcsENABLE]&tcsENABLE_WEN) != 0;
				break;

			case tcsATIME:
				tcs3430.Data.IntegrationTimeIn_us	= (tcsBufferRx[tcsATIME] + 1)*2780;
				tcs3430.Data.MaxValue = tcsBufferRx[tcsATIME] >= 64 ? 65535 : 1024*(tcsBufferTx[tcsATIME] + 1) - 1;
				break;

			case tcsWTIME:
				tcs3430.Data.WaitTimeIn_us	= (tcsBufferRx[tcsWTIME] + 1)*2780*( tcs3430.Data.WaitLong ? 12 : 1 );
				break;

			case tcsAILTL:
				break;

			case tcsAILTH:
				if( regStart <= tcsAILTL && regStop >= tcsAILTH)
				{
					tcs3430.Data.Threshold[0] = (tcsBufferRx[tcsAILTH] << 8) | tcsBufferRx[tcsAILTL];
				}
				break;

			case tcsAIHTL:
				break;

			case tcsAIHTH:
				if( regStart <= tcsAIHTL && regStop >= tcsAIHTH)
				{
					tcs3430.Data.Threshold[1] = (tcsBufferRx[tcsAIHTH] << 8) | tcsBufferRx[tcsAIHTL];
				}
				break;

			case tcsPERS:
				switch( tcsBufferRx[tcsPERS]&0x0F )
				{
					case tcsPERS_EveryALS: 			tcs3430.Data.IntWhenConsValuesOutOfRange = 0; 	break;
					case tcsPERS_AnyOutOfThreshold: tcs3430.Data.IntWhenConsValuesOutOfRange = 1; 	break;
					case tcsPERS_2ConsOutOfRange: 	tcs3430.Data.IntWhenConsValuesOutOfRange = 2; 	break;
					case tcsPERS_3ConsOutOfRange: 	tcs3430.Data.IntWhenConsValuesOutOfRange = 3; 	break;
					case tcsPERS_5ConsOutOfRange: 	tcs3430.Data.IntWhenConsValuesOutOfRange = 5; 	break;
					case tcsPERS_10ConsOutOfRange: 	tcs3430.Data.IntWhenConsValuesOutOfRange = 10; 	break;
					case tcsPERS_15ConsOutOfRange: 	tcs3430.Data.IntWhenConsValuesOutOfRange = 15; 	break;
					case tcsPERS_20ConsOutOfRange: 	tcs3430.Data.IntWhenConsValuesOutOfRange = 20; 	break;
					case tcsPERS_45ConsOutOfRange: 	tcs3430.Data.IntWhenConsValuesOutOfRange = 45; 	break;
					case tcsPERS_50ConsOutOfRange: 	tcs3430.Data.IntWhenConsValuesOutOfRange = 50; 	break;
					case tcsPERS_55ConsOutOfRange: 	tcs3430.Data.IntWhenConsValuesOutOfRange = 55; 	break;
					case tcsPERS_60ConsOutOfRange: 	tcs3430.Data.IntWhenConsValuesOutOfRange = 60; 	break;

					default: break;
				}
				break;

			case tcsCFG0:
				tcs3430.Data.WaitLong = (tcsBufferRx[tcsCFG0] & tcsCFG0_WLONG) != tcsCFG0_Empty;
				break;

			case tcsCFG1:
				tcs3430.Data.ModeIR = (tcsBufferRx[tcsCFG1] & tcsCFG1_AMUX) != 0;
				switch(tcsBufferRx[tcsCFG1]&0x03)
				{
					case 0b00: tcs3430.Data.GainALS = 1;	break;
					case 0b01: tcs3430.Data.GainALS = 4;	break;
					case 0b10: tcs3430.Data.GainALS = 16;	break;
					case 0b11:
						tcs3430.Data.GainALS = (tcsBufferRx[tcsCFG2] & tcsCFG2_HGAIN) != tcsCFG2_Empty ? 128 : 64;
						break;

					default: break;
				}
				break;

			case tcsREVID:
				tcs3430.Data.ID_Rev = tcsBufferRx[tcsREVID] & tcsREVID_Mask;
				break;

			case tcsID:
				tcs3430.Data.ID = (tcsBufferRx[tcsID]&tcsID_Mask)>>2;
				break;

			case tcsSTATUS:
				break;

			case tcsCH0DATAL:
				break;

			case tcsCH0DATAH:
				if( regStart <= tcsCH0DATAL && regStop >= tcsCH0DATAH)
				{
					if( !tcs3430.Data.ModeIR )
					{
						tcs3430.Data.XYZ[2] = (tcsBufferRx[tcsCH0DATAL] << 8) | tcsBufferRx[tcsCH0DATAH];
					}
				}
				break;

			case tcsCH1DATAL:
				break;

			case tcsCH1DATAH:
				if( regStart <= tcsCH1DATAL && regStop >= tcsCH1DATAH)
				{
					if( !tcs3430.Data.ModeIR )
					{
						tcs3430.Data.XYZ[1] = (tcsBufferRx[tcsCH1DATAH] << 8) | tcsBufferRx[tcsCH1DATAL];
					}
				}
				break;

			case tcsCH2DATAL:
				break;

			case tcsCH2DATAH:
				if( regStart <= tcsCH2DATAL && regStop >= tcsCH2DATAH)
				{
					if( tcs3430.Data.ModeIR )
					{
						tcs3430.Data.IR[0] = (tcsBufferRx[tcsCH2DATAH] << 8) | tcsBufferRx[tcsCH2DATAL];
					}
				}
				break;

			case tcsCH3DATAL:
				break;

			case tcsCH3DATAH:
				if( regStart <= tcsCH3DATAL && regStop >= tcsCH3DATAH)
				{
					if( tcs3430.Data.ModeIR )
					{
						tcs3430.Data.IR[1] = (tcsBufferRx[tcsCH3DATAH] << 8) | tcsBufferRx[tcsCH3DATAL];
					}
					else
					{
						tcs3430.Data.XYZ[0] = (tcsBufferRx[tcsCH3DATAH] << 8) | tcsBufferRx[tcsCH3DATAL];
					}
				}
				break;

			case tcsCFG2:
				if( (tcsBufferRx[tcsCFG2] & tcsCFG2_HGAIN) != tcsCFG2_Empty && (tcsBufferRx[tcsCFG1]&0b11) == 0b11 )
				{
					tcs3430.Data.GainALS = 128;
				}
				break;

			case tcsCFG3:
				tcs3430.Data.ClearStatusAfterRead = (tcsBufferRx[tcsCFG3] & tcsCFG3_INTREADCLEAR) != tcsCFG3_Empty;
				tcs3430.Data.SleepAfterInterrupt = (tcsBufferRx[tcsCFG3] & tcsCFG3_SAI) != tcsCFG3_Empty;
				break;

			case tcsAZ_CONFIG:
				tcs3430.Data.AlwaysStartAtPrev = (tcsBufferRx[tcsAZ_CONFIG] & tcsAZCONFIG_MODE) != 0;
				tcs3430.Data.RunAutozeroAfter = tcsBufferRx[tcsAZ_CONFIG] & tcsAZCONFIG_NthIter_Mask;
				break;

			case tcsINTENAB:
				tcs3430.Data.IntASAT = (tcsBufferRx[tcsINTENAB] & tcsINTENAB_ASIEN) != 0;
				tcs3430.Data.IntALS = (tcsBufferRx[tcsINTENAB] & tcsINTENAB_AIEN) != 0;
				break;
		}
	}
}

//----------------------------------------------------------------
void tcsPower(uint8_t state, uint8_t withALS)
{
	tcsReadRegs(tcsENABLE, 1);

	state 	? SET_BIT(tcsBufferRx[tcsENABLE], tcsENABLE_PON) : CLEAR_BIT(tcsBufferRx[tcsENABLE], tcsENABLE_PON);
	withALS ? SET_BIT(tcsBufferRx[tcsENABLE], tcsENABLE_AEN) : CLEAR_BIT(tcsBufferRx[tcsENABLE], tcsENABLE_AEN);
	tcsBufferTx[tcsENABLE] = tcsBufferRx[tcsENABLE];

	tcsWriteRegs(tcsENABLE, 1);

	tcs3430.Data.StatePower = state;
	tcs3430.Data.StateALS = withALS;
}

void tcsSetWait(uint8_t state)
{
	tcsReadRegs(tcsENABLE, 1);

	state 	? SET_BIT(tcsBufferRx[tcsENABLE], tcsENABLE_WEN) : CLEAR_BIT(tcsBufferRx[tcsENABLE], tcsENABLE_WEN);
	tcsBufferTx[tcsENABLE] = tcsBufferRx[tcsENABLE];

	tcsWriteRegs(tcsENABLE, 1);

	tcs3430.Data.StateWait = state;
}

uint8_t tcsSetIntegrationTime(uint16_t numOf2780usIntervals)
{
	if(numOf2780usIntervals == 0 || numOf2780usIntervals > 256) return 1;

	tcsBufferTx[tcsATIME] = (uint8_t)(numOf2780usIntervals - 1);
	tcsWriteRegs(tcsATIME, 1);

	tcs3430.Data.IntegrationTimeIn_us = numOf2780usIntervals * 2780;

	return 0;
}

uint8_t tcsSetWaitCyclesBetweenALS(uint16_t waitCycles)
{
	if(waitCycles == 0 || waitCycles > 256) return 1;

	tcsBufferTx[tcsWTIME] = (uint8_t)(waitCycles - 1);
	tcsWriteRegs(tcsWTIME, 1);

	tcs3430.Data.WaitTimeIn_us = waitCycles*2780;

	return 0;
}

uint8_t tcsSetInterruptMode(uint8_t numOfConsValuesOutOfRange)
{
	switch(numOfConsValuesOutOfRange)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			tcsBufferTx[tcsPERS] = numOfConsValuesOutOfRange&0x03;
			break;

		case 5: tcsBufferTx[tcsPERS] = tcsPERS_5ConsOutOfRange; 	break;
		case 10: tcsBufferTx[tcsPERS] = tcsPERS_10ConsOutOfRange; 	break;
		case 15: tcsBufferTx[tcsPERS] = tcsPERS_15ConsOutOfRange; 	break;
		case 20: tcsBufferTx[tcsPERS] = tcsPERS_20ConsOutOfRange; 	break;
		case 45: tcsBufferTx[tcsPERS] = tcsPERS_45ConsOutOfRange; 	break;
		case 50: tcsBufferTx[tcsPERS] = tcsPERS_50ConsOutOfRange; 	break;
		case 55: tcsBufferTx[tcsPERS] = tcsPERS_55ConsOutOfRange; 	break;
		case 60: tcsBufferTx[tcsPERS] = tcsPERS_60ConsOutOfRange; 	break;

		default:
			return 1;
	}

	tcsWriteRegs(tcsPERS, 1);

	tcs3430.Data.IntWhenConsValuesOutOfRange = numOfConsValuesOutOfRange;

	return 0;
}

void tcsWaitLong(uint8_t state)
{
	tcsBufferTx[tcsCFG0] = state ? tcsCFG0_WLONG : tcsCFG0_Empty;
	tcsWriteRegs(tcsCFG0, 1);

	tcs3430.Data.WaitLong = state;
}

void tcsSetModeALS(uint8_t xyzOrIR)
{
	tcsReadRegs(tcsCFG1, 1);

	xyzOrIR ? SET_BIT(tcsBufferRx[tcsCFG1], tcsCFG1_AMUX) : CLEAR_BIT(tcsBufferRx[tcsCFG1], tcsCFG1_AMUX);
	tcsBufferTx[tcsCFG1] = tcsBufferRx[tcsCFG1];

	tcsWriteRegs(tcsCFG1, 1);

	tcs3430.Data.ModeIR = xyzOrIR;
}

uint8_t tcsSetGainALS(uint8_t gain)
{
	tcsReadRegs(tcsCFG1, 1);
	CLEAR_BIT(tcsBufferRx[tcsCFG1], 0x03);

	switch(gain)
	{
		case 1:
			tcsBufferTx[tcsCFG1] = tcsBufferRx[tcsCFG1] | tcsCFG1_AGAINx1;
			tcsBufferTx[tcsCFG2] = tcsCFG2_Empty;
			break;

		case 4:
			tcsBufferTx[tcsCFG1] = tcsBufferRx[tcsCFG1] | tcsCFG1_AGAINx4;
			tcsBufferTx[tcsCFG2] = tcsCFG2_Empty;
			break;

		case 16:
			tcsBufferTx[tcsCFG1] = tcsBufferRx[tcsCFG1] | tcsCFG1_AGAINx16;
			tcsBufferTx[tcsCFG2] = tcsCFG2_Empty;
			break;

		case 64:
			tcsBufferTx[tcsCFG1] = tcsBufferRx[tcsCFG1] | tcsCFG1_AGAINx64;
			tcsBufferTx[tcsCFG2] = tcsCFG2_Empty;
			break;

		case 128:
			tcsBufferTx[tcsCFG1] = tcsBufferRx[tcsCFG1] | tcsCFG1_AGAINx64;
			tcsBufferTx[tcsCFG2] = tcsCFG2_HGAIN;
			break;

		default:
			return 1;
	}

	if( gain == 128)
	{
		tcsWriteRegs(tcsCFG1, 1);
		tcsWriteRegs(tcsCFG2, 1);
	}
	else
	{
		tcsWriteRegs(tcsCFG2, 1);
		tcsWriteRegs(tcsCFG1, 1);
	}

	tcs3430.Data.GainALS = gain;

	return 0;
}

uint8_t tcsGetID(void)
{
	tcsReadRegs(tcsID, 1);
	tcs3430.Data.ID = (tcsBufferRx[tcsID]&tcsID_Mask)>>2;

	return tcs3430.Data.ID;
}

uint8_t tcsGetREVID(void)
{
	tcsReadRegs(tcsREVID, 1);
	tcs3430.Data.ID_Rev = tcsBufferRx[tcsREVID]&tcsREVID_Mask;

	return tcs3430.Data.ID_Rev;
}

uint8_t tcsGetStatus(uint8_t clear)
{
	tcsReadRegs(tcsSTATUS, 1);

	if(clear)	tcsClearStatus();

	return tcsBufferRx[tcsSTATUS];
}

void tcsGetChData(void)
{
	tcsReadRegs(tcsCFG1, 1);
	tcsReadRegs(tcsCH0DATAL, 8);

	if(tcs3430.Data.ModeIR)
	{
		tcs3430.Data.IR[0] = (tcsBufferRx[tcsCH2DATAH]<<8) | tcsBufferRx[tcsCH2DATAL];
		tcs3430.Data.IR[1] = (tcsBufferRx[tcsCH3DATAH]<<8) | tcsBufferRx[tcsCH3DATAL];
	}
	else
	{
		tcs3430.Data.XYZ[0] = (tcsBufferRx[tcsCH3DATAH]<<8) | tcsBufferRx[tcsCH3DATAL];
		tcs3430.Data.XYZ[1] = (tcsBufferRx[tcsCH1DATAH]<<8) | tcsBufferRx[tcsCH1DATAL];
		tcs3430.Data.XYZ[2] = (tcsBufferRx[tcsCH0DATAH]<<8) | tcsBufferRx[tcsCH0DATAL];
	}
}

void tcsSetStatusClearAfterRead(uint8_t state)
{
	tcsReadRegs(tcsCFG3, 1);

	state ? SET_BIT(tcsBufferRx[tcsCFG3], tcsCFG3_INTREADCLEAR) : CLEAR_BIT(tcsBufferRx[tcsCFG3], tcsCFG3_INTREADCLEAR);
	tcsBufferTx[tcsCFG3] = tcsBufferRx[tcsCFG3];

	tcsWriteRegs(tcsCFG3, 1);

	tcs3430.Data.ClearStatusAfterRead = state;
}

void tcsSetSAI(uint8_t state)
{
	tcsReadRegs(tcsCFG3, 1);

	state ? SET_BIT(tcsBufferRx[tcsCFG3], tcsCFG3_SAI) : CLEAR_BIT(tcsBufferRx[tcsCFG3], tcsCFG3_SAI);
	SET_BIT(tcsBufferRx[tcsCFG3], tcsCFG3_Empty);

	tcsBufferTx[tcsCFG3] = tcsBufferRx[tcsCFG3];

	tcsWriteRegs(tcsCFG3, 1);

	tcs3430.Data.SleepAfterInterrupt = state;
}

void tcsClearStatus(void)
{
	tcsBufferTx[tcsSTATUS] = tcsSTATUS_ASAT | tcsSTATUS_AINT;
	tcsWriteRegs(tcsSTATUS, 1);
}

void tcsAlwaysStartAt(uint8_t zeroOrPrev)
{
	tcsReadRegs(tcsAZ_CONFIG, 1);

	zeroOrPrev ? SET_BIT(tcsBufferRx[tcsAZ_CONFIG], tcsAZCONFIG_MODE) : CLEAR_BIT(tcsBufferRx[tcsAZ_CONFIG], tcsAZCONFIG_MODE);
	tcsBufferTx[tcsAZ_CONFIG] = tcsBufferRx[tcsAZ_CONFIG];

	tcsWriteRegs(tcsAZ_CONFIG, 1);

	tcs3430.Data.AlwaysStartAtPrev = zeroOrPrev;
}

void tcsRunAutozeroAfter(uint8_t numOfIterations)
{
	if(numOfIterations > tcsAZCONFIG_NthIter_Mask) numOfIterations = tcsAZCONFIG_NthIter_Mask;

	tcsReadRegs(tcsAZ_CONFIG, 1);

	CLEAR_BIT(tcsBufferRx[tcsAZ_CONFIG], tcsAZCONFIG_NthIter_Mask);
	tcsBufferTx[tcsAZ_CONFIG] = tcsBufferRx[tcsAZ_CONFIG] | (numOfIterations&tcsAZCONFIG_NthIter_Mask);

	tcsWriteRegs(tcsAZ_CONFIG, 1);
}

void tcsSetStateInterruptASAT(uint8_t state)
{
	tcsReadRegs(tcsINTENAB, 1);

	state ? SET_BIT(tcsBufferRx[tcsINTENAB], tcsINTENAB_ASIEN) : CLEAR_BIT(tcsBufferRx[tcsINTENAB], tcsINTENAB_ASIEN);
	tcsBufferTx[tcsINTENAB] = tcsBufferRx[tcsINTENAB];

	tcsWriteRegs(tcsINTENAB, 1);

	tcs3430.Data.IntASAT = state;
}

void tcsSetStateInterruptALS(uint8_t state)
{
	tcsReadRegs(tcsINTENAB, 1);

	state ? SET_BIT(tcsBufferRx[tcsINTENAB], tcsINTENAB_AIEN) : CLEAR_BIT(tcsBufferRx[tcsINTENAB], tcsINTENAB_AIEN);
	tcsBufferTx[tcsINTENAB] = tcsBufferRx[tcsINTENAB];

	tcsWriteRegs(tcsINTENAB, 1);

	tcs3430.Data.IntALS = state;
}

void tcsSetLowThreshold(uint16_t value)
{
	tcsBufferTx[tcsAILTL] = value & 0xFF;
	tcsBufferTx[tcsAILTH] = (value >> 8) & 0xFF;

	tcsWriteRegs(tcsAILTL, 2);

	tcs3430.Data.Threshold[0] = value;
}

void tcsSetHighThreshold(uint16_t value)
{
	tcsBufferTx[tcsAIHTL] = value & 0xFF;
	tcsBufferTx[tcsAIHTH] = (value >> 8) & 0xFF;

	tcsWriteRegs(tcsAIHTL, 2);

	tcs3430.Data.Threshold[1] = value;
}

uint16_t tcsGetMaximumValueALS(void)
{
	tcsUpdateDataFromRegs(tcsATIME, 1);

	tcs3430.Data.MaxValue = tcsBufferRx[tcsATIME] >= 64 ? 65535 : 1024*(tcsBufferTx[tcsATIME] + 1) - 1;

	return tcs3430.Data.MaxValue;
}


void tcsTest(void)
{
	tcsSetWait(0);
	tcsSetIntegrationTime(65);
	tcsSetInterruptMode(55);
	tcsWaitLong(1);
	tcsSetModeALS(0);
	tcsSetGainALS(16);
	tcsSetStatusClearAfterRead(1);
	tcsSetSAI(0);
	tcsAlwaysStartAt(1);
	tcsRunAutozeroAfter(0);
	tcsSetStateInterruptASAT(0);
	tcsSetStateInterruptALS(0);
	tcsSetLowThreshold(1000);
	tcsSetHighThreshold(2000);

	tcsUpdateDataFromRegs(tcsENABLE, tcsINTENAB);

	return;
}
