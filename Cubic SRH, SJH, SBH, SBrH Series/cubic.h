/*
 * srh.h
 *
 *  Created on: Jul 10, 2023
 *      Author: DL
 */

#ifndef INC_CUBIC_H_
#define INC_CUBIC_H_

#include "stdint.h"
#include "string.h"

#include "misc.h"
#include "stm32l1xx_rcc.h"
#include "stm32l1xx_gpio.h"
#include "stm32l1xx_usart.h"

#include "systick.h"

#define cubicBufferSize	16

#define cubicUSART_BaudRate 	9600
#define cubicUSART_ByteSize 	USART_WordLength_8b
#define cubicUSART_StopBits 	USART_StopBits_1
#define cubicUSART_Parity    	USART_Parity_No

#define	cubicUSART				USART2
#define	cubicUSART_IRQ			USART2_IRQn
#define cubicUSART_AF			GPIO_AF_USART2
#define cubicUSART_Port_RCC   	RCC_APB1Periph_USART2

#define cubicUSART_GPIOPort_RCC   RCC_AHBPeriph_GPIOA
#define cubicUSART_GPIOPort       GPIOA
#define cubicUSART_Pin_Tx_s       GPIO_PinSource2
#define cubicUSART_Pin_Rx_s       GPIO_PinSource3
#define cubicUSART_Pin_Tx         GPIO_Pin_2
#define cubicUSART_Pin_Rx         GPIO_Pin_3

#define cubicVCC_GPIOPort_RCC	RCC_AHBPeriph_GPIOC
#define cubicVCC_GPIOPort		GPIOC
#define cubicVCC_Pin			GPIO_Pin_3

typedef enum
{
	cubicError_NoError					= 0,
	cubicError_WrongLenOrCantResolve 	= 1,
	cubicError_IncorrectCmd 			= 2,
	cubicError_CantImplementUnderStatus = 3,

	cubicError_Unknown					= 0xFE,
	cubicError_NoAnswer					= 0xFF,
}cubicError_t;

typedef enum
{
	cubicGasType_CH4_C3H8_CBrH3 	= 0x00,
	cubicGasType_CO2				= 0x01,
}cubicGasType_t;

typedef enum
{
	cubicUnits_ppm		= 0x00,
	cubicUnits_Perc1	= 0x01,
	cubicUnits_Perc2	= 0x02,
	cubicUnits_Perc3	= 0x03,
}cubicUnits_t;

typedef enum
{
	cubicStatus_WarmingUp		= 0b00000001,
	cubicStatus_Malfunction		= 0b00000010,
	cubicStatus_Outrange		= 0b00000100,

	cubicStatus_NoCalibr		= 0b00010000,
	cubicStatus_HighHumi		= 0b00100000,
	cubicStatus_RefOverLim		= 0b01000000,
	cubicStatus_MeasOverLim		= 0b10000000,
}cubicStatus_t;

typedef enum
{
	cubicAutoBaselineState_Open1 = 0x00,
	cubicAutoBaselineState_Open2 = 0x01,
	cubicAutoBaselineState_Close = 0x02,
}cubicAutoBaselineState_t;

typedef enum
{
	cubicCmdNum_CheckMeasResult = 0x01,
	cubicCmdNum_ZeroAdjustment 	= 0x03,

	cubicCmdNum_UserCalibrZero 		= 0x4B,
	cubicCmdNum_UserCalibrMiddle 	= 0x4E,
	cubicCmdNum_UserCalibrFull 		= 0x4C,

	cubicCmdNum_ResetCalibr 		= 0x4D,
	cubicCmdNum_CheckSoftVer 		= 0x1E,
	cubicCmdNum_CheckInstrNum 		= 0x1F,
	cubicCmdNum_CheckGasMeasProp 	= 0x0D,
	cubicCmdNum_AutoBaselineCheck	= 0x0F,
	cubicCmdNum_AutoBaselineSet		= 0x10,
}cubicCmdNum_t;

typedef struct
{
	cubicError_t 	Error;
	uint8_t			Status;
	uint8_t			CorrectCRC;
}cubicState_t;

typedef struct
{
	cubicGasType_t				GasType;
	cubicUnits_t				Units;
	cubicAutoBaselineState_t	AutoBaseLineState;
	uint8_t						SoftVersion[10];
	uint8_t						CalibrCycle;
	uint16_t					BaseValue_x100;
	uint16_t					GasConcCalibrValue;
	uint16_t					InstrNumber[5];
	float						MeasRange;
}cubicConfig_t;

typedef struct
{
	const uint8_t	Address;
	uint8_t			DataReady;
	uint16_t		LastValue;
	uint32_t		Timeout_ms;

	cubicConfig_t 	Config;
	cubicState_t 	State;

	uint8_t (*SendCmd)(cubicCmdNum_t cmd);
	uint8_t (*Power)(int8_t state);
	void (*Init)(void);
}cubic_t;

extern cubic_t cubic;

void USART2_IRQHandler(void);

#endif /* INC_CUBIC_H_ */
