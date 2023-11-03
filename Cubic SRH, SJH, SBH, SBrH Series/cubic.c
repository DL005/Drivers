/*
 * cubic.c
 *
 *  Created on: Jul 10, 2023
 *      Author: DL
 */


#include "cubic.h"
#include "cubic_defines.h"

cubic_t cubic =
{
	0x11,	// Address;
	0,		// DataReady;
	0,		// LastValue;
	20,		// Timeout_ms


	//Config
	{
		cubicGasType_CH4_C3H8_CBrH3,	// GasType
		cubicUnits_ppm,					// Units
		cubicAutoBaselineState_Close,	// AutoBaseLineState
		{0},							// SoftVersion
		0,								// CalibrCycle
		0,								// BaseValue_x100
		0,								// GasConcCalibrValue
		{0},							// InstrNumber
		0.0,							// MeasRange
	},

	//State
	{
		cubicError_NoError, 	// Error
		0,						// Status
		0,						// CorrectCRC
	},

	cubicSendCmd,
	cubicPower,
	initCubic,
};

volatile uint8_t cubicBufferRx[cubicBufferSize] = {0};
uint8_t cubicBufferTx[cubicBufferSize] = {0};

uint16_t cubicCounterRx = 0;
uint8_t	cubicDataReceived;

//--------------------------------------------------------------------------------------------------------------------------------

void initCubic(void)
{
	cubicPower(1);
	sysTickDelay_ms(1500);

	cubic.State.Error = cubicError_NoAnswer;
	for( uint8_t i = 0; i < 3 && (cubic.State.Error != cubicError_NoError); i++, cubicSendCmd(cubicCmdNum_CheckSoftVer) );

	cubic.State.Error = cubicError_NoAnswer;
	for( uint8_t i = 0; i < 3 && (cubic.State.Error != cubicError_NoError); i++, cubicSendCmd(cubicCmdNum_CheckInstrNum) );

	cubic.State.Error = cubicError_NoAnswer;
	for( uint8_t i = 0; i < 3 && (cubic.State.Error != cubicError_NoError); i++, cubicSendCmd(cubicCmdNum_CheckGasMeasProp) );

	cubicPower(0);
}

uint8_t cubicChecksumGet(uint8_t* buffer)
{
	if( buffer != cubicBufferRx && buffer != cubicBufferTx ) return 0;

	return buffer[ buffer[cubicByteRx_NumOfBytes] + 2 ];
}

uint8_t cubicChecksumCalc(uint8_t* buffer)
{
	uint8_t res = 0;

	if( buffer != cubicBufferRx && buffer != cubicBufferTx ) return res;

	for(uint16_t i = 0; i < buffer[cubicByteRx_NumOfBytes] + 2 && i < cubicBufferSize; res += buffer[i], i++);

	return -res;
}

uint8_t cubicChecksumCompare(uint8_t* buffer) { return cubicChecksumCalc(buffer) == cubicChecksumGet(buffer); }

//--------------------------------------------------------------------------------------------------------------------------------

void handlerCubic(void)
{
	uint32_t systickTimeOut = sysTicks + cubic.Timeout_ms;
	while( !cubicDataReceived && sysTicks < systickTimeOut );

	if( !cubicDataReceived )
	{
		cubic.State.Error = cubicError_NoAnswer;
		memset(cubicBufferRx, 0, cubicBufferSize);
		return;
	}

	cubic.State.CorrectCRC = cubicChecksumCompare(cubicBufferRx);

	if( cubicBufferRx[cubicByteRx_Ack] != cubicByteAck )
	{
		cubic.State.Error = ( cubicBufferRx[cubicByteRx_Ack] == cubicByteNak ) ? cubicBufferRx[cubicByteRx_Error] : cubicError_Unknown;
		memset(cubicBufferRx, 0, cubicBufferSize);
		return;
	}

	cubic.State.Error = 0;
	switch( cubicBufferRx[cubicByteRx_CMD] )
	{
		case cubicCmdNum_CheckMeasResult: 	cubicCmdRx_CheckMeasResult();		break;

		case cubicCmdNum_ZeroAdjustment:
		case cubicCmdNum_UserCalibrZero:
		case cubicCmdNum_UserCalibrMiddle:
		case cubicCmdNum_UserCalibrFull: 										break;

		case cubicCmdNum_ResetCalibr: 		cubicCmdRx_ResetCalibr();			break;
		case cubicCmdNum_CheckSoftVer:		cubicCmdRx_CheckSoftVer();			break;
		case cubicCmdNum_CheckInstrNum: 		cubicCmdRx_CheckInstrNum();		break;
		case cubicCmdNum_CheckGasMeasProp: 	cubicCmdRx_CheckGasMeasProp();		break;
		case cubicCmdNum_AutoBaselineCheck:	cubicCmdRx_AutoBaselineCheck();		break;
		case cubicCmdNum_AutoBaselineSet:		cubicCmdRx_AutoBaselineSet();	break;
	}

	memset(cubicBufferRx, 0, cubicBufferSize);
}

void cubicCmdRx_CheckMeasResult(void)
{
	cubic.LastValue = (cubicBufferRx[3]<<8) | cubicBufferRx[4];
	cubic.State.Status = cubicBufferRx[5];
}

void cubicCmdRx_ResetCalibr(void) { cubic.State.CorrectCRC = ( cubicBufferRx[0]==0x16&&cubicBufferRx[1]==0x01&&cubicBufferRx[2]==0x4D&&cubicBufferRx[3]==0x9C ) ? 1 : 0; }

void cubicCmdRx_CheckSoftVer(void) 	{ memcpy(cubic.Config.SoftVersion, cubicBufferRx + 3, cubicBufferRx[cubicByteRx_NumOfBytes]-1); }
void cubicCmdRx_CheckInstrNum(void) 	{ memcpy(cubic.Config.InstrNumber, cubicBufferRx + 3, 10); }
void cubicCmdRx_CheckGasMeasProp(void)
{
	cubic.Config.MeasRange = (float)((cubicBufferRx[3]<<8) | cubicBufferRx[4]);
	for(uint8_t deg = cubicBufferRx[5]; deg; cubic.Config.MeasRange /= 10.0, deg--);

	cubic.Config.GasType = cubicBufferRx[6];
	cubic.Config.Units = cubicBufferRx[7];
}
void cubicCmdRx_AutoBaselineCheck(void)
{
	cubic.Config.AutoBaseLineState = cubicBufferRx[4];
	cubic.Config.CalibrCycle = cubicBufferRx[5];
	cubic.Config.BaseValue_x100 = ((uint16_t)cubicBufferRx[5]<<8) | cubicBufferRx[6];
}

void cubicCmdRx_AutoBaselineSet(void) { cubic.State.CorrectCRC = ( cubicBufferRx[0]==0x16&&cubicBufferRx[1]==0x01&&cubicBufferRx[2]==0x10&&cubicBufferRx[3]==0xD9 ) ? 1 : 0; }

//--------------------------------------------------------------------------------------------------------------------------------

uint8_t cubicSendCmd(cubicCmdNum_t cmd)
{
	cubicDataReceived = 0;
	cubic.DataReady = 0;

	switch( cmd )
	{
		case cubicCmdNum_CheckMeasResult: 	cubicCmdTx_CheckMeasResult();		break;

		case cubicCmdNum_ZeroAdjustment:		cubicCmdTx_ZeroAdjustment();	break;
		case cubicCmdNum_UserCalibrZero:		cubicCmdTx_UserCalibrZero();	break;
		case cubicCmdNum_UserCalibrMiddle:	cubicCmdTx_UserCalibrMiddle();		break;
		case cubicCmdNum_UserCalibrFull: 		cubicCmdTx_UserCalibrFull();	break;

		case cubicCmdNum_ResetCalibr: 		cubicCmdTx_ResetCalibr();			break;
		case cubicCmdNum_CheckSoftVer:		cubicCmdTx_CheckSoftVer();			break;
		case cubicCmdNum_CheckInstrNum: 		cubicCmdTx_CheckInstrNum();		break;
		case cubicCmdNum_CheckGasMeasProp: 	cubicCmdTx_CheckGasMeasProp();		break;
		case cubicCmdNum_AutoBaselineCheck:	cubicCmdTx_AutoBaselineCheck();		break;
		case cubicCmdNum_AutoBaselineSet:		cubicCmdTx_AutoBaselineSet();	break;
	}

	cubicSendTx();

	handlercubic();
	cubic.DataReady = 1;

	memset(cubicBufferTx, 0, cubicBufferSize);
	return cubic.DataReady;
}

void cubicCmdTx_CheckMeasResult(void)
{
	cubicBufferTx[cubicByteTx_Addr]			=cubic.Address;
	cubicBufferTx[cubicByteTx_NumOfBytes]	=0x01;
	cubicBufferTx[cubicByteTx_CMD]			=cubicCmdNum_CheckMeasResult;
	cubicBufferTx[3]						=0xED;
}
void cubicCmdTx_ZeroAdjustment(void)
{
	cubicBufferTx[cubicByteTx_Addr]			=cubic.Address;
	cubicBufferTx[cubicByteTx_NumOfBytes]	=0x01;
	cubicBufferTx[cubicByteTx_CMD]			=cubicCmdNum_ZeroAdjustment;
	cubicBufferTx[3]						=0xEB;
}
void cubicCmdTx_UserCalibrZero(void)
{
	cubicBufferTx[cubicByteTx_Addr]			=cubic.Address;
	cubicBufferTx[cubicByteTx_NumOfBytes]	=0x04;
	cubicBufferTx[cubicByteTx_CMD]			=cubicCmdNum_UserCalibrZero;

	cubicBufferTx[3] = cubic.Config.GasType;
	cubicBufferTx[4] = (uint8_t)(cubic.Config.GasConcCalibrValue>>8);
	cubicBufferTx[5] = (uint8_t)(cubic.Config.GasConcCalibrValue&0xFF);

	cubicBufferTx[6] = cubicChecksumCalc(cubicBufferTx);
}
void cubicCmdTx_UserCalibrMiddle(void)
{
	cubicBufferTx[cubicByteTx_Addr]			=cubic.Address;
	cubicBufferTx[cubicByteTx_NumOfBytes]	=0x04;
	cubicBufferTx[cubicByteTx_CMD]			=cubicCmdNum_UserCalibrMiddle;

	cubicBufferTx[3] = cubic.Config.GasType;
	cubicBufferTx[4] = (uint8_t)(cubic.Config.GasConcCalibrValue>>8);
	cubicBufferTx[5] = (uint8_t)(cubic.Config.GasConcCalibrValue&0xFF);

	cubicBufferTx[6] = cubicChecksumCalc(cubicBufferTx);
}
void cubicCmdTx_UserCalibrFull(void)
{
	cubicBufferTx[cubicByteTx_Addr]			=cubic.Address;
	cubicBufferTx[cubicByteTx_NumOfBytes]	=0x04;
	cubicBufferTx[cubicByteTx_CMD]			=cubicCmdNum_UserCalibrFull;

	cubicBufferTx[3] = cubic.Config.GasType;
	cubicBufferTx[4] = (uint8_t)(cubic.Config.GasConcCalibrValue>>8);
	cubicBufferTx[5] = (uint8_t)(cubic.Config.GasConcCalibrValue&0xFF);

	cubicBufferTx[6] = cubicChecksumCalc(cubicBufferTx);
}
void cubicCmdTx_ResetCalibr(void)
{
	cubicBufferTx[cubicByteTx_Addr]			=cubic.Address;
	cubicBufferTx[cubicByteTx_NumOfBytes]	=0x02;
	cubicBufferTx[cubicByteTx_CMD]			=cubicCmdNum_ResetCalibr;

	cubicBufferTx[3] = cubic.Config.GasType;

	cubicBufferTx[4] = cubicChecksumCalc(cubicBufferTx);
}
void cubicCmdTx_CheckSoftVer(void)
{
	cubicBufferTx[cubicByteTx_Addr]			=cubic.Address;
	cubicBufferTx[cubicByteTx_NumOfBytes]	=0x01;
	cubicBufferTx[cubicByteTx_CMD]			=cubicCmdNum_CheckSoftVer;
	cubicBufferTx[3]						=0xD0;
}
void cubicCmdTx_CheckInstrNum(void)
{
	cubicBufferTx[cubicByteTx_Addr]			=cubic.Address;
	cubicBufferTx[cubicByteTx_NumOfBytes]	=0x01;
	cubicBufferTx[cubicByteTx_CMD]			=cubicCmdNum_CheckInstrNum;
	cubicBufferTx[3]						=0xCF;
}
void cubicCmdTx_CheckGasMeasProp(void)
{
	cubicBufferTx[cubicByteTx_Addr]			=cubic.Address;
	cubicBufferTx[cubicByteTx_NumOfBytes]	=0x01;
	cubicBufferTx[cubicByteTx_CMD]			=cubicCmdNum_CheckGasMeasProp;
	cubicBufferTx[3]						=0xE1;
}
void cubicCmdTx_AutoBaselineCheck(void)
{
	cubicBufferTx[cubicByteTx_Addr]			=cubic.Address;
	cubicBufferTx[cubicByteTx_NumOfBytes]	=0x01;
	cubicBufferTx[cubicByteTx_CMD]			=cubicCmdNum_AutoBaselineCheck;
	cubicBufferTx[3]						=0xDF;
}
void cubicCmdTx_AutoBaselineSet(void)
{
	cubicBufferTx[cubicByteTx_Addr]			=cubic.Address;
	cubicBufferTx[cubicByteTx_NumOfBytes]	=0x07;
	cubicBufferTx[cubicByteTx_CMD]			=cubicCmdNum_AutoBaselineSet;

	cubicBufferTx[3] = 0x00;
	cubicBufferTx[4] = cubic.Config.AutoBaseLineState;
	cubicBufferTx[5] = cubic.Config.CalibrCycle;
	cubicBufferTx[6] = cubic.Config.BaseValue_x100>>8;
	cubicBufferTx[7] = cubic.Config.BaseValue_x100&0xFF;
	cubicBufferTx[8] = 0x00;

	cubicBufferTx[9] = cubicChecksumCalc(cubicBufferTx);
}

//--------------------------------------------------------------------------------------------------------------------------------

void cubicInitUSART(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure; 
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHBPeriphClockCmd(cubicVCC_GPIOPort_RCC, ENABLE);

	GPIO_InitStructure.GPIO_Pin   = cubicVCC_Pin;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_Init(cubicVCC_GPIOPort, &GPIO_InitStructure);

	GPIO_ResetBits(cubicVCC_GPIOPort, cubicVCC_Pin);

	RCC_AHBPeriphClockCmd(cubicVCC_GPIOPort_RCC, ENABLE);

	GPIO_PinAFConfig(cubicUSART_GPIOPort, cubicUSART_Pin_Tx_s, cubicUSART_AF);
	GPIO_PinAFConfig(cubicUSART_GPIOPort, cubicUSART_Pin_Rx_s, cubicUSART_AF);

	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin   = cubicUSART_Pin_Tx;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_Init(cubicUSART_GPIOPort, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin   = cubicUSART_Pin_Rx;
	GPIO_Init(cubicUSART_GPIOPort, &GPIO_InitStructure);

	RCC_APB1PeriphClockCmd(cubicUSART_Port_RCC, ENABLE);
	USART_InitStructure.USART_BaudRate            = cubicUSART_BaudRate;
	USART_InitStructure.USART_WordLength          = cubicUSART_ByteSize;
	USART_InitStructure.USART_StopBits            = cubicUSART_StopBits;
	USART_InitStructure.USART_Parity              = cubicUSART_Parity;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(cubicUSART, &USART_InitStructure);


	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 

	NVIC_InitStructure.NVIC_IRQChannel = cubicUSART_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_Cmd(cubicUSART, ENABLE);
	USART_ITConfig(cubicUSART, USART_IT_RXNE, ENABLE);
}

void cubicSendTx(void)
{
	for (uint8_t i = 0; i < cubicBufferTx[cubicByteRx_NumOfBytes] + 3; i++)
	{
		while(USART_GetFlagStatus(cubicUSART,USART_FLAG_TXE)==RESET){}
		USART_SendData(cubicUSART,cubicBufferTx[i]);
	}
}

uint8_t cubicPower(int8_t state)
{
	if( state > 0)
	{
		GPIO_ResetBits(cubicVCC_GPIOPort, cubicVCC_Pin);
		cubicInitUSART();
	}
	else if( state == 0)
	{
		USART_DeInit(cubicUSART);
		GPIO_SetBits(cubicVCC_GPIOPort, cubicVCC_Pin);
	}

	return !GPIO_ReadOutputDataBit(cubicVCC_GPIOPort, cubicVCC_Pin);
}

//--------------------------------------------------------------------------------------------------------------------------------

void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(cubicUSART, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(cubicUSART, USART_IT_RXNE);

		cubicBufferRx[cubicCounterRx] = (uint8_t)USART_ReceiveData(cubicUSART);

		if( cubicCounterRx > cubicByteRx_NumOfBytes && cubicCounterRx >= cubicBufferRx[cubicByteRx_NumOfBytes] + 2 )
		{
			cubicDataReceived = 1;
			cubicCounterRx = 0;
		}
		else
		{
			cubicCounterRx++;
		}
	}
}
