/*
 * ext.c
 *
 *  Created on: Jul 21, 2023
 *      Author: DL
 */

#include "ext.h"

volatile wUSART_t* ext = &wUSART1;
volatile tcs3430_t* sensor = &tcs3430;

uint8_t extRecallCalled = 0;

void extError(extError_t error);
extCmdNum_t	extGetCmd(void);

void extCmd_Serial(void);
void extCmd_LEDs(void);
void extCmd_CALS_Power(void);
void extCmd_CALS_chData(void);
void extCmd_CALS_XYZorIR(void);
void extCmd_CALS_IntegrationTime(void);
void extCmd_CALS_Gain(void);
void extCmd_CALS_StartAtZeroOrPrev(void);
void extCmd_CALS_RunAutozeroAfter(void);
void extCmd_CALS_Interrupt(void);
void extCmd_CALS_LowThreshold(void);
void extCmd_CALS_HighThreshold(void);
void extCmd_CALS_IntAfterOutOfRange(void);
void extCmd_CALS_Wait(void);
void extCmd_CALS_WaitCycles(void);
void extCmd_CALS_SleepAfterInterrupt(void);
void extCmd_GetAllInfo(void);

//--------------------------------------------------------------------------------------------------------------------------------

void handlerExt(void)
{
	if( !ext->ReceiveFlag ) return;
	ext->ReceiveFlag = 0;

	if( ext->rxBuffer[0] != '#' )
	{
		extError(extError_UnknowndCmd);
		return;
	}

	switch( extGetCmd() )
	{
		case extCmdNum_Dummy: break;

		case extCmdNum_Serial: 					extCmd_Serial(); 					break;
		case extCmdNum_LEDs: 					extCmd_LEDs(); 						break;
		case extCmdNum_CALS_Power: 				extCmd_CALS_Power(); 				break;
		case extCmdNum_CALS_chData:				extCmd_CALS_chData(); 				break;
		case extCmdNum_CALS_XYZorIR: 			extCmd_CALS_XYZorIR(); 				break;
		case extCmdNum_CALS_IntegrationTime: 	extCmd_CALS_IntegrationTime();		break;
		case extCmdNum_CALS_Gain: 				extCmd_CALS_Gain(); 				break;
		case extCmdNum_CALS_StartAtZeroOrPrev: 	extCmd_CALS_StartAtZeroOrPrev(); 	break;
		case extCmdNum_CALS_RunAutozeroAfter: 	extCmd_CALS_RunAutozeroAfter();		break;
		case extCmdNum_CALS_Interrupt: 			extCmd_CALS_Interrupt(); 			break;
		case extCmdNum_CALS_LowThreshold: 		extCmd_CALS_LowThreshold(); 		break;
		case extCmdNum_CALS_HighThreshold: 		extCmd_CALS_HighThreshold(); 		break;
		case extCmdNum_CALS_IntAfterOutOfRange: extCmd_CALS_IntAfterOutOfRange(); 	break;
		case extCmdNum_CALS_Wait: 				extCmd_CALS_Wait(); 				break;
		case extCmdNum_CALS_WaitCycles: 		extCmd_CALS_WaitCycles(); 			break;
		case extCmdNum_CALS_SleepAfterInterrupt:extCmd_CALS_SleepAfterInterrupt(); 	break;
		case extCmdNum_AllInfo:					extCmd_GetAllInfo();				break;

		default:
			extError(extError_UnknowndCmd);
			break;
	}

	if( !extRecallCalled )
	{
		ext->ClearRx();
		ext->ClearTx();
	}
}

void extError(extError_t error)
{
	char strUnknown[] = "Unknown command\r\n";
	char strWrongParams[] = "Wrong params\r\n";

	switch( error )
	{
		case extError_UnknowndCmd:
			ext->SendString(strUnknown);
			break;

		case extError_WrongParams:
			ext->SendString(strWrongParams);
			break;

		default:
			break;
	}
}

extCmdNum_t	extGetCmd(void) { return (ext->rxBuffer[3] == '\r' || ext->rxBuffer[3] == '=') ? 10*(ext->rxBuffer[1]-'0') + (ext->rxBuffer[2]-'0') : 0xFF; }

void extRecallWithNoParams(void)
{
	extRecallCalled = 1;
	ext->ReceiveFlag = 1;
	ext->rxBuffer[3] = '\r';
	ext->rxBuffer[4] = '\n';
	handlerExt();

	extRecallCalled = 0;
	ext->ClearRx();
	ext->ClearTx();
}

void extSensorInterrupt(void)
{
	extCALS_Interrupt(0);

	char buffer[64] = {0};

	ext->SendString("[INT]");
	if(sensor->Data.StateInterrupt & tcsSTATUS_ASAT) ext->SendString("[ASAT]");
	if(sensor->Data.StateInterrupt & tcsSTATUS_AINT) ext->SendString("[ALS]");
	ext->SendChar('\n');

	sensor->Functions.GetChData();
	if( sensor->Data.ModeIR )
	{
		sprintf(buffer, "IR1:	%d\nIR2:	%d\r\n", sensor->Data.IR[0], sensor->Data.IR[1]);
	}
	else
	{
		sprintf(buffer, "X:	%d\nY:	%d\nZ:	%d\r\n", sensor->Data.XYZ[0], sensor->Data.XYZ[1], sensor->Data.XYZ[2]);
	}

	ext->SendString(buffer);

	extCALS_Interrupt(1);
}

//--------------------------------------------------------------------------------------------------------------------------------

void extCmd_Serial(void)
{
	sprintf(ext->txBuffer, "ID:	%d\nREVID:	%d\r\n", sensor->Data.ID, sensor->Data.ID_Rev);
	ext->SendTxBuffer(0);
}

void extCmd_CALS_Power(void)
{
	if( ext->rxBuffer[3] == '\r' && ext->rxBuffer[4] == '\n' )
	{
		sensor->Functions.UpdateDataFromRegs(tcsENABLE, tcsENABLE);

		sprintf(ext->txBuffer, "Power:	%d\nWith ALS:	%d\r\n", sensor->Data.StatePower, sensor->Data.StateALS );
		ext->SendTxBuffer(0);
		return;
	}

	if(
			ext->rxBuffer[3] != '=' 	|| ext->rxBuffer[5] != ',' 	||
			ext->rxBuffer[7] != '\r' 	|| ext->rxBuffer[8] != '\n' ||

			(ext->rxBuffer[4] != '0' 	&& ext->rxBuffer[4] != '1') ||
			(ext->rxBuffer[6] != '0' 	&& ext->rxBuffer[6] != '1')
	)
	{
		extError(extError_WrongParams);
		return;
	}

	sensor->Functions.Power(ext->rxBuffer[4] - '0', ext->rxBuffer[6] - '0');

	extRecallWithNoParams();
}

void extCmd_CALS_chData(void)
{
	if( ext->rxBuffer[3] != '\r' && ext->rxBuffer[4] != '\n' )
	{
		extError(extError_WrongParams);
		return;
	}

	sensor->Functions.GetChData();
	if( sensor->Data.ModeIR )
	{
		sprintf(ext->txBuffer, "IR1:	%d\nIR2:	%d\r\n", sensor->Data.IR[0], sensor->Data.IR[1]);
	}
	else
	{
		sprintf(ext->txBuffer, "X:	%d\nY:	%d\nZ:	%d\r\n", sensor->Data.XYZ[0], sensor->Data.XYZ[1], sensor->Data.XYZ[2]);
	}

	ext->SendTxBuffer(0);
}

void extCmd_CALS_XYZorIR(void)
{
	if( ext->rxBuffer[3] == '\r' && ext->rxBuffer[4] == '\n' )
	{
		sensor->Functions.UpdateDataFromRegs(tcsCFG1, tcsCFG1);

		sensor->Data.ModeIR ? ext->SendString("Mode:	IR\r\n") : ext->SendString("Mode:	XYZ\r\n");
		return;
	}

	if( ext->rxBuffer[3] != '=' && ext->rxBuffer[4] != '0' && ext->rxBuffer[4] != '1' && ext->rxBuffer[5] != '\r' && ext->rxBuffer[6] != '\n' )
	{
		extError(extError_WrongParams);
		return;
	}

	sensor->Functions.SetModeALS(ext->rxBuffer[4] - '0');

	extRecallWithNoParams();
}

void extCmd_CALS_IntegrationTime(void)
{
	if( ext->rxBuffer[3] == '\r' && ext->rxBuffer[4] == '\n' )
	{
		sensor->Functions.UpdateDataFromRegs(tcsATIME, tcsATIME);

		sprintf(ext->txBuffer, "Int time:	%d	us\nMax value:	%d\r\n", sensor->Data.IntegrationTimeIn_us, sensor->Data.MaxValue);
		ext->SendTxBuffer(0);

		return;
	}

	for(uint16_t i = 4; i < usart1RxBufferSize && ext->rxBuffer[i] != '\r'; i++)
	{
		if( ext->rxBuffer[i] < '0' || ext->rxBuffer[i] > '9')
		{
			extError(extError_WrongParams);
			return;
		}
	}

	uint32_t intTime = strtoul( ext->rxBuffer + 4, NULL, 10 );
	uint16_t intCycles = intTime / 2780;

	intCycles = (intCycles == 0) 	? 1 	:
				(intCycles > 256) 	? 256 	: intCycles;

	if( intCycles != 256 )
	{
		if( (intCycles + 1)*2780 - intTime >= intTime - intCycles*2780 ) intCycles++;
	}

	sensor->Functions.SetIntegrationTime(intCycles);

	extRecallWithNoParams();
}

void extCmd_CALS_Gain(void)
{
	if( ext->rxBuffer[3] == '\r' && ext->rxBuffer[4] == '\n' )
	{
		sensor->Functions.UpdateDataFromRegs(tcsCFG1, tcsCFG1);
		sensor->Functions.UpdateDataFromRegs(tcsCFG2, tcsCFG2);

		sprintf(ext->txBuffer, "Gain:	x%d\r\n", sensor->Data.GainALS);
		ext->SendTxBuffer(0);

		return;
	}

	uint32_t gain = strtoul(ext->rxBuffer + 4, NULL, 10);

	switch(gain)
	{
		case 1:
		case 4:
		case 16:
		case 64:
		case 128:
			break;

		default:
			extError(extError_WrongParams);
			return;
	}

	sensor->Functions.SetGainALS(gain);

	extRecallWithNoParams();
}

void extCmd_CALS_StartAtZeroOrPrev(void)
{
	if( ext->rxBuffer[3] == '\r' && ext->rxBuffer[4] == '\n' )
	{
		sensor->Functions.UpdateDataFromRegs(tcsAZ_CONFIG, tcsAZ_CONFIG);

		sensor->Data.AlwaysStartAtPrev ? ext->SendString("Start at:	previous\r\n") : ext->SendString("Start at:	zero\r\n");
		return;
	}

	if( ext->rxBuffer[3] != '=' || (ext->rxBuffer[4] != '0' && ext->rxBuffer[4] != '1') || ext->rxBuffer[5] != '\r' || ext->rxBuffer[6] != '\n' )
	{
		extError(extError_WrongParams);
		return;
	}

	sensor->Functions.AlwaysStartAt(ext->rxBuffer[4] - '0');

	extRecallWithNoParams();
}

void extCmd_CALS_RunAutozeroAfter(void)
{
	if( ext->rxBuffer[3] == '\r' && ext->rxBuffer[4] == '\n' )
	{
		sensor->Functions.UpdateDataFromRegs(tcsAZ_CONFIG, tcsAZ_CONFIG);

		if		( sensor->Data.RunAutozeroAfter == 0 ) 		{ ext->SendString("Autozero:	never\r\n"); }
		else if	( sensor->Data.RunAutozeroAfter >= 0x7F ) 	{ ext->SendString("Autozero:	only at first\r\n"); }
		else
		{
			sprintf(ext->txBuffer, "Autozero:	every %d cycles\r\n", sensor->Data.RunAutozeroAfter);
			ext->SendTxBuffer(0);
		}

		return;
	}

	char* strEnd;
	uint32_t value = strtoul(ext->rxBuffer + 4, &strEnd, 10);

	if( *strEnd != '\r' || value > 0x7F )
	{
		extError(extError_WrongParams);
		return;
	}

	sensor->Functions.RunAutozeroAfter(value);

	extRecallWithNoParams();
}

void extCmd_CALS_Interrupt(void)
{
	if( ext->rxBuffer[3] == '\r' && ext->rxBuffer[4] == '\n' )
	{
		sensor->Functions.UpdateDataFromRegs(tcsINTENAB, tcsINTENAB);

		sprintf(ext->txBuffer, "ASAT:	%d\nALS:	%d\r\n", sensor->Data.IntASAT, sensor->Data.IntALS);
		ext->SendTxBuffer(0);

		return;
	}

	if(
			ext->rxBuffer[3] != '=' 	||	ext->rxBuffer[5] != ',' 	||
			ext->rxBuffer[7] != '\r'	||	ext->rxBuffer[8] != '\n'	||

			( ext->rxBuffer[4] != '0' && ext->rxBuffer[4] != '1' && ext->rxBuffer[4] != 'x' && ext->rxBuffer[4] != 'X' ) ||
			( ext->rxBuffer[6] != '0' && ext->rxBuffer[6] != '1' && ext->rxBuffer[6] != 'x' && ext->rxBuffer[6] != 'X' )
		)
	{
		extError(extError_WrongParams);
		return;
	}

	extCALS_Interrupt(0);

	if( ext->rxBuffer[4] != 'x' && ext->rxBuffer[4] != 'X' ) sensor->Functions.SetStateInterruptASAT( ext->rxBuffer[4] - '0' );
	if( ext->rxBuffer[6] != 'x' && ext->rxBuffer[6] != 'X' ) sensor->Functions.SetStateInterruptALS( ext->rxBuffer[6] - '0' );

	extRecallWithNoParams();

	extCALS_Interrupt(1);
}

void extCmd_CALS_LowThreshold(void)
{
	if( ext->rxBuffer[3] == '\r' && ext->rxBuffer[4] == '\n' )
	{
		sensor->Functions.UpdateDataFromRegs(tcsAILTL, tcsAILTH);

		sprintf(ext->txBuffer, "Low threshold:	%d\r\n", sensor->Data.Threshold[0]);
		ext->SendTxBuffer(0);

		return;
	}

	char* strEnd;
	uint32_t value = strtoul(ext->rxBuffer + 4, &strEnd, 10);

	if( *strEnd != '\r' )
	{
		extError(extError_WrongParams);
		return;
	}

	if(value > 0xFFFF) value = 0xFFFF;
	sensor->Functions.SetLowThreshold(value);

	extRecallWithNoParams();
}

void extCmd_CALS_HighThreshold(void)
{
	if( ext->rxBuffer[3] == '\r' && ext->rxBuffer[4] == '\n' )
	{
		sensor->Functions.UpdateDataFromRegs(tcsAIHTL, tcsAIHTH);

		sprintf(ext->txBuffer, "High threshold:	%d\r\n", sensor->Data.Threshold[1]);
		ext->SendTxBuffer(0);

		return;
	}

	char* strEnd;
	uint32_t value = strtoul(ext->rxBuffer + 4, &strEnd, 10);

	if( *strEnd != '\r' )
	{
		extError(extError_WrongParams);
		return;
	}

	if(value > 0xFFFF) value = 0xFFFF;
	sensor->Functions.SetHighThreshold(value);

	extRecallWithNoParams();
}

void extCmd_CALS_IntAfterOutOfRange(void)
{
	if( ext->rxBuffer[3] == '\r' && ext->rxBuffer[4] == '\n' )
	{
		sensor->Functions.UpdateDataFromRegs(tcsPERS, tcsPERS);

		sprintf(ext->txBuffer, "Interrupt when:	%d	consecutive ALS values out of range\r\n", sensor->Data.IntWhenConsValuesOutOfRange);
		ext->SendTxBuffer(0);

		return;
	}

	char* strEnd;
	uint32_t value = strtoul(ext->rxBuffer + 4, &strEnd, 10);

	if( *strEnd != '\r' || sensor->Functions.SetInterruptMode(value) )
	{
		extError(extError_WrongParams);
		return;
	}

	extRecallWithNoParams();
}

void extCmd_CALS_Wait(void)
{
	if( ext->rxBuffer[3] == '\r' && ext->rxBuffer[4] == '\n' )
	{
		sensor->Functions.UpdateDataFromRegs(tcsENABLE, tcsENABLE);

		sensor->Data.StateWait ? ext->SendString("Wait:	1\r\n") : ext->SendString("Wait:	0\r\n") ;

		return;
	}

	if( ext->rxBuffer[3] != '=' || (ext->rxBuffer[4] != '0' && ext->rxBuffer[4] != '1') || ext->rxBuffer[5] != '\r' || ext->rxBuffer[6] != '\n' )
	{
		extError(extError_WrongParams);
		return;
	}

	sensor->Functions.SetWait( ext->rxBuffer[4] - '0' );

	extRecallWithNoParams();
}

void extCmd_CALS_WaitCycles(void)
{
	if( ext->rxBuffer[3] == '\r' && ext->rxBuffer[4] == '\n' )
	{
		sensor->Functions.UpdateDataFromRegs(tcsCFG0, tcsCFG0);
		sensor->Functions.UpdateDataFromRegs(tcsWTIME, tcsWTIME);

		if( sensor->Data.WaitTimeIn_us < 1000000 )
		{
			sprintf( ext->txBuffer, "Wait time:	%d	us\r\n", sensor->Data.WaitTimeIn_us);
		}
		else
		{
			sprintf( ext->txBuffer, "Wait time:	%d	ms\r\n", sensor->Data.WaitTimeIn_us/1000);
		}

		ext->SendTxBuffer(0);
		return;
	}

	char* strEnd;
	uint32_t value = strtoul(ext->rxBuffer + 4, &strEnd, 10);
	uint32_t waitCycles = 0;

	if( *strEnd != '\r' )
	{
		extError(extError_WrongParams);
		return;
	}

	value = value < 2780 	? 2780 		:
			value > 8540160 ? 8540160	: value;

	if( value <= 711680 )
	{
		waitCycles = value/2780;
		if( (waitCycles + 1)*2780 - value <= value - waitCycles*2780 ) waitCycles++;

		sensor->Functions.WaitLong(0);
	}
	else
	{
		waitCycles = value/(2780*12);
		if( value != 8540160 && ( (waitCycles + 1)*2780*12 - value <= value - waitCycles*2780*12 )) waitCycles++;

		sensor->Functions.WaitLong(1);
	}

	sensor->Functions.SetWaitCyclesBetweenALS(waitCycles);

	extRecallWithNoParams();
}

void extCmd_CALS_SleepAfterInterrupt(void)
{
	if( ext->rxBuffer[3] == '\r' && ext->rxBuffer[4] == '\n' )
	{
		sensor->Functions.UpdateDataFromRegs(tcsCFG3, tcsCFG3);

		sensor->Data.SleepAfterInterrupt ? ext->SendString("SAI:	1\r\n") : ext->SendString("SAI:	0\r\n") ;

		return;
	}

	if( ext->rxBuffer[3] != '=' || (ext->rxBuffer[4] != '0' && ext->rxBuffer[4] != '1') || ext->rxBuffer[5] != '\r' || ext->rxBuffer[6] != '\n' )
	{
		extError(extError_WrongParams);
		return;
	}

	sensor->Functions.SetSAI( ext->rxBuffer[4] - '0' );

	extRecallWithNoParams();
}

void extCmd_GetAllInfo(void)
{
	if( ext->rxBuffer[3] != '\r' || ext->rxBuffer[4] != '\n' )
	{
		extError(extError_WrongParams);
		return;
	}

	for(extCmdNum_t cmd = extCmdNum_Serial; cmd <= extCmdNum_CALS_SleepAfterInterrupt; cmd++)
	{
		sprintf(ext->rxBuffer, "#%02d\r\n", cmd);
		ext->ReceiveFlag = 1;

		handlerExt();
	}
}
