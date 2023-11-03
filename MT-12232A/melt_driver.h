/*
 * melt_driver.h
 *
 *  Created on: Mar 9, 2023
 *      Author: DL
 */

#ifndef MT_12232A_MELT_DRIVER_H_
#define MT_12232A_MELT_DRIVER_H_

#include "main.h"
#include "misc.h"

#define meltSinglePortForDisplay	1

#if meltSinglePortForDisplay
	#define meltGPIO	GPIOx
#endif

#define pinLCD_LedControl_Pin 
#define pinLCD_LedControl_GPIO_Port 
#define pinLCD_DB0_Pin 
#define pinLCD_DB0_GPIO_Port 
#define pinLCD_DB1_Pin 
#define pinLCD_DB1_GPIO_Port 
#define pinLCD_DB2_Pin 
#define pinLCD_DB2_GPIO_Port 
#define pinLCD_DB3_Pin 
#define pinLCD_DB3_GPIO_Port 
#define pinLCD_DB4_Pin 
#define pinLCD_DB4_GPIO_Port 
#define pinLCD_DB5_Pin 
#define pinLCD_DB5_GPIO_Port 
#define pinLCD_DB6_Pin 
#define pinLCD_DB6_GPIO_Port 
#define pinLCD_DB7_Pin 
#define pinLCD_DB7_GPIO_Port 
#define pinLCD_A0_Pin 
#define pinLCD_A0_GPIO_Port 
#define pinLCD_E_Pin 
#define pinLCD_E_GPIO_Port 
#define pinLCD_RES_Pin 
#define pinLCD_RES_GPIO_Port 
#define pinLCD_CS_Pin 
#define pinLCD_CS_GPIO_Port 

typedef enum
{
	alignTL,
	alignTC,
	alignTR,

	alignCL,
	alignCC,
	alignCR,

	alignDL,
	alignDC,
	alignDR
}meltAlign_t;

typedef struct
{
	void (*PrintScreen)(uint8_t* image);
	uint8_t (*PrintImage)(uint8_t* image, uint16_t X, uint16_t Y, uint16_t width, uint16_t height, meltAlign_t align);

	void (*FillScreen)(uint8_t byte);
	void (*FillPageZone)(uint8_t byte, uint8_t page, uint8_t xStart, uint8_t xEnd);
	void (*ClearScreenZone)(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd);

	uint8_t (*PrintDigit)(uint16_t X, uint16_t Y, uint8_t digit, meltAlign_t align);
	uint8_t (*PrintInteger)(uint16_t X, uint16_t Y, int64_t number, meltAlign_t align);
	uint8_t (*PrintFixedPoint)(uint16_t X, uint16_t Y, int64_t number, uint8_t fixedPoint, meltAlign_t align);

	void (*LEDSetState) (int8_t state);
	uint8_t (*LEDGetState) (void);
}lcd_t;

extern lcd_t* melt;

void initLCD(void);

#endif /* MT_12232A_MELT_DRIVER_H_ */
