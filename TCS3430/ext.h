/*
 * ext.h
 *
 *  Created on: Jul 21, 2023
 *      Author: DL
 */

#ifndef EXT_H_
#define EXT_H_

#include "wUSART.h"
#include "tcs3430.h"

#define extCALS_Interrupt_IRQn	EXTI4_15_IRQn

#define extCALS_Interrupt(state) (state ? NVIC_EnableIRQ(extCALS_Interrupt_IRQn) : NVIC_DisableIRQ(extCALS_Interrupt_IRQn) )

typedef enum
{
	extCmdNum_Dummy,

	extCmdNum_Serial,
	extCmdNum_CALS_Power,
	extCmdNum_CALS_chData,
	extCmdNum_CALS_XYZorIR,
	extCmdNum_CALS_IntegrationTime,
	extCmdNum_CALS_Gain,
	extCmdNum_CALS_StartAtZeroOrPrev,
	extCmdNum_CALS_RunAutozeroAfter,
	extCmdNum_CALS_Interrupt,
	extCmdNum_CALS_LowThreshold,
	extCmdNum_CALS_HighThreshold,
	extCmdNum_CALS_IntAfterOutOfRange,
	extCmdNum_CALS_Wait,
	extCmdNum_CALS_WaitCycles,
	extCmdNum_CALS_SleepAfterInterrupt,

	extCmdNum_AllInfo = 99,
}extCmdNum_t;

typedef enum
{
	extError_NoError,

	extError_UnknowndCmd,
	extError_WrongParams,
}extError_t;

void handlerExt(void);
void extSensorInterrupt(void);

#endif /* EXT_H_ */
