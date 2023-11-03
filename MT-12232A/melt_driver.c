/*
 * melt_driver.h
 *
 *  Created on: Mar 9, 2023
 *      Author: DL
 */

#include "melt_driver.h"

#include "image.h"
#include "font.h"

void 	meltShowImageFullScreen(uint8_t* image);
uint8_t meltPrintLocalImage(uint8_t* image, uint16_t X, uint16_t Y, uint16_t width, uint16_t height, meltAlign_t align);
void 	meltFillAllScreen(uint8_t byte);
void 	meltClearScreenZone(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd);
uint8_t meltPrintDigit(uint16_t X, uint16_t Y, uint8_t digit, meltAlign_t align);
uint8_t meltPrintInteger(uint16_t X, uint16_t Y, int64_t number, meltAlign_t align);
uint8_t meltPrintFixedPoint(uint16_t X, uint16_t Y, int64_t number, uint8_t fixedPoint, meltAlign_t align);
void 	meltLEDSetState(int8_t state);
uint8_t meltLEDGetState(void);
void 	meltFillPageZone(uint8_t byte, uint8_t page, uint8_t xStart, uint8_t xEnd);

enum
{
	meltMaskDB0 = 0x01,
	meltMaskDB1 = 0x02,
	meltMaskDB2 = 0x04,
	meltMaskDB3 = 0x08,
	meltMaskDB4 = 0x10,
	meltMaskDB5 = 0x20,
	meltMaskDB6 = 0x40,
	meltMaskDB7 = 0x80,

	meltMaskA0 	= 0x200,
	meltMaskE 	= 0x400,
	meltMaskRES = 0x800,
	meltMaskCS 	= 0x1000,
}meltPins_t;

lcd_t lcd =
{
	meltShowImageFullScreen,// PrintScreen
	meltPrintLocalImage, 	// PrintImage

	meltFillAllScreen, 		// FillScreen
	meltFillPageZone,		// FillPageZone
	meltClearScreenZone, 	// ClearScreenZone

	meltPrintDigit, 		// PrintDigit
	meltPrintInteger, 		// PrintInteger
	meltPrintFixedPoint, 	// PrintFixedPoint

	meltLEDSetState,		// LEDGetState
	meltLEDGetState,		// LEDSetState
};

lcd_t* melt = &lcd;

typedef struct
{
	uint8_t DB[8];
	uint8_t RW;
	uint8_t A0;
	uint8_t CS;
	uint8_t E;
	uint8_t RES;
}LCD_Data_MT12232A_t;

LCD_Data_MT12232A_t lcdDataMT12232A =
{
	{0}, // DB[8];
	0, // RW;
	0, // A0;
	0, // CS;
	0, // E;
	0, // RES;
};

uint8_t miscIntLen(int64_t x)
{
	if(x < 0) x *= -1;

    if (x >= 1000000000000000000) 	return 19;
    if (x >= 100000000000000000)  	return 18;
    if (x >= 10000000000000000)   	return 17;
    if (x >= 1000000000000000)    	return 16;
    if (x >= 100000000000000)     	return 15;
    if (x >= 10000000000000)      	return 14;
    if (x >= 1000000000000)       	return 13;
    if (x >= 100000000000)        	return 12;
    if (x >= 10000000000)         	return 11;
    if (x >= 1000000000) 			return 10;
    if (x >= 100000000)  			return 9;
    if (x >= 10000000)   			return 8;
    if (x >= 1000000)    			return 7;
    if (x >= 100000)     			return 6;
    if (x >= 10000)      			return 5;
    if (x >= 1000)       			return 4;
    if (x >= 100)        			return 3;
    if (x >= 10)         			return 2;
    return 1;
}

void meltWrite(void)
{

#if meltSinglePortForDisplay
	uint32_t gpioPort = 0;

	gpioPort |= lcdDataMT12232A.DB[0] ? meltMaskDB0 : 0;
	gpioPort |= lcdDataMT12232A.DB[1] ? meltMaskDB1 : 0;
	gpioPort |= lcdDataMT12232A.DB[2] ? meltMaskDB2 : 0;
	gpioPort |= lcdDataMT12232A.DB[3] ? meltMaskDB3 : 0;
	gpioPort |= lcdDataMT12232A.DB[4] ? meltMaskDB4 : 0;
	gpioPort |= lcdDataMT12232A.DB[5] ? meltMaskDB5 : 0;
	gpioPort |= lcdDataMT12232A.DB[6] ? meltMaskDB6 : 0;
	gpioPort |= lcdDataMT12232A.DB[7] ? meltMaskDB7 : 0;

	gpioPort |= lcdDataMT12232A.A0 	? meltMaskA0 	: 0;
	gpioPort |= lcdDataMT12232A.CS 	? meltMaskCS 	: 0;
	gpioPort |= lcdDataMT12232A.E 	? meltMaskE 	: 0;
	gpioPort |= lcdDataMT12232A.RES ? meltMaskRES 	: 0;

	LL_GPIO_WriteOutputPort(meltGPIO, gpioPort);
#else
	lcdDataMT12232A.DB[0] ? LL_GPIO_SetOutputPin(pinLCD_DB0_GPIO_Port, pinLCD_DB0_Pin) : LL_GPIO_ResetOutputPin(pinLCD_DB0_GPIO_Port, pinLCD_DB0_Pin);
	lcdDataMT12232A.DB[1] ? LL_GPIO_SetOutputPin(pinLCD_DB1_GPIO_Port, pinLCD_DB1_Pin) : LL_GPIO_ResetOutputPin(pinLCD_DB1_GPIO_Port, pinLCD_DB1_Pin);
	lcdDataMT12232A.DB[2] ? LL_GPIO_SetOutputPin(pinLCD_DB2_GPIO_Port, pinLCD_DB2_Pin) : LL_GPIO_ResetOutputPin(pinLCD_DB2_GPIO_Port, pinLCD_DB2_Pin);
	lcdDataMT12232A.DB[3] ? LL_GPIO_SetOutputPin(pinLCD_DB3_GPIO_Port, pinLCD_DB3_Pin) : LL_GPIO_ResetOutputPin(pinLCD_DB3_GPIO_Port, pinLCD_DB3_Pin);
	lcdDataMT12232A.DB[4] ? LL_GPIO_SetOutputPin(pinLCD_DB4_GPIO_Port, pinLCD_DB4_Pin) : LL_GPIO_ResetOutputPin(pinLCD_DB4_GPIO_Port, pinLCD_DB4_Pin);
	lcdDataMT12232A.DB[5] ? LL_GPIO_SetOutputPin(pinLCD_DB5_GPIO_Port, pinLCD_DB5_Pin) : LL_GPIO_ResetOutputPin(pinLCD_DB5_GPIO_Port, pinLCD_DB5_Pin);
	lcdDataMT12232A.DB[6] ? LL_GPIO_SetOutputPin(pinLCD_DB6_GPIO_Port, pinLCD_DB6_Pin) : LL_GPIO_ResetOutputPin(pinLCD_DB6_GPIO_Port, pinLCD_DB6_Pin);
	lcdDataMT12232A.DB[7] ? LL_GPIO_SetOutputPin(pinLCD_DB7_GPIO_Port, pinLCD_DB7_Pin) : LL_GPIO_ResetOutputPin(pinLCD_DB7_GPIO_Port, pinLCD_DB7_Pin);

	lcdDataMT12232A.A0 	? LL_GPIO_SetOutputPin(pinLCD_A0_GPIO_Port, pinLCD_A0_Pin) 			: LL_GPIO_ResetOutputPin(pinLCD_A0_GPIO_Port, pinLCD_A0_Pin);
	lcdDataMT12232A.CS 	? LL_GPIO_SetOutputPin(pinLCD_CS_GPIO_Port, pinLCD_CS_Pin) 			: LL_GPIO_ResetOutputPin(pinLCD_CS_GPIO_Port, pinLCD_CS_Pin);
	lcdDataMT12232A.E 	? LL_GPIO_SetOutputPin(pinLCD_E_GPIO_Port, pinLCD_E_Pin) 			: LL_GPIO_ResetOutputPin(pinLCD_E_GPIO_Port, pinLCD_E_Pin);
	lcdDataMT12232A.RES ? LL_GPIO_SetOutputPin(pinLCD_RES_GPIO_Port, pinLCD_RES_Pin) 		: LL_GPIO_ResetOutputPin(pinLCD_RES_GPIO_Port, pinLCD_RES_Pin);
#endif

}

void meltWriteToDB(uint8_t byte)
{
	lcdDataMT12232A.DB[0] = ( (byte&0x01) != 0 );
	lcdDataMT12232A.DB[1] = ( (byte&0x02) != 0 );
	lcdDataMT12232A.DB[2] = ( (byte&0x04) != 0 );
	lcdDataMT12232A.DB[3] = ( (byte&0x08) != 0 );
	lcdDataMT12232A.DB[4] = ( (byte&0x10) != 0 );
	lcdDataMT12232A.DB[5] = ( (byte&0x20) != 0 );
	lcdDataMT12232A.DB[6] = ( (byte&0x40) != 0 );
	lcdDataMT12232A.DB[7] = ( (byte&0x80) != 0 );
}

void meltClearStruct(void)
{
	memset(lcdDataMT12232A.DB, 0, 8);

	lcdDataMT12232A.RW = 0;
	lcdDataMT12232A.A0 = 0;
	lcdDataMT12232A.CS = 0;
	lcdDataMT12232A.E = 0;
	lcdDataMT12232A.RES = 0;
}

//Процедура выдачи байта в индикатор
void WriteByte(uint8_t b, uint8_t cd, uint8_t lr) {
//При необходимости настроить здесь шину данных на вывод
	lcdDataMT12232A.RW=0; lcdDataMT12232A.A0=cd;	//Выдача байта в индикатор как данных или команды
	lcdDataMT12232A.CS=lr; meltWriteToDB(b); //Выбрать кристалл индикатора и выдать байт на шину данных индикатора
	meltWrite();

//	LL_mDelay(1);
	wDWT.Delay(1); // >40ns Это время предустановки адреса (tAW)

	lcdDataMT12232A.E=0;
	meltWrite();
//	LL_mDelay(1);
	wDWT.Delay(3); // >160ns Длительность сигнала E=0 (время предустановки данных попало сюда (tDS))

	lcdDataMT12232A.E=1;		//Сбросить сигнал E индикатору
	meltWrite();
//	LL_mDelay(1);
	wDWT.Delay( wDWT.ToTicks(2, dwt_us) ); // >(2000ns-40ns-160ns) Минимально допустимый интервал между сигналами E=0
}

void WriteCodeL(uint8_t b) { WriteByte(b,0,1); }//Команду в левый кристалл индикатора
void WriteCodeR(uint8_t b) { WriteByte(b,0,0); }//Команду в правый кристалл индикатора

void WriteDataL(uint8_t b) { WriteByte(b,1,1); }//Данные в левую половину индикатора
void WriteDataR(uint8_t b) { WriteByte(b,1,0); }//Данные в правую половину индикатора

//Процедура программной инициализации индикатора
void initLCD(void)
{
	lcdDataMT12232A.E=1;//Начальное значение сигнала индикатору
	lcdDataMT12232A.RES=0;//Выдать сигнал RES=0 индикатору
	meltWrite();
	wDWT.Delay( wDWT.ToTicks(11, dwt_us) ); // Задержка на время больше 10 мкс
	lcdDataMT12232A.RES=1;//Снять сигнал RES
	meltWrite();
	wDWT.Delay( wDWT.ToTicks(1100, dwt_us) ); // Задержка на время больше 1 мс
	WriteCodeL(0xE2);//Reset
	WriteCodeR(0xE2);//Reset
	WriteCodeL(0xEE);//ReadModifyWrite off
	WriteCodeR(0xEE);//ReadModifyWrite off
	WriteCodeL(0xA4);//Включить обычный режим
	WriteCodeR(0xA4);//Включить обычный режим
	WriteCodeL(0xA9);//Мультиплекс 1/32
	WriteCodeR(0xA9);//Мультиплекс 1/32
	WriteCodeL(0xC0);//Верхнюю строку на 0
	WriteCodeR(0xC0);//Верхнюю строку на 0
	WriteCodeL(0xA1);//Invert scan RAM
	WriteCodeR(0xA0);//NonInvert scan RAM
	WriteCodeL(0xAF);//Display on
	WriteCodeR(0xAF);//Display on
}

void meltShowImageFullScreen(uint8_t* image)
{
	uint8_t	p;//Номер текущей страницы индикатора
	uint8_t	c;//Позиция по горизонтали выводимого байта

	for(p=0; p<4; p++)
	{//Цикл по всем 4-м страницам индикатора
		WriteCodeL(p|0xB8);//Установка текущей страницы для левого кристалла индикатора
		WriteCodeL(0x13);//Установка текущего адреса для записи данных в левую отображаемую позицию левой половины индикатора
		for(c=0; c<61; c++)
		{//Цикл вывода данных в левую половину индикатора
			WriteDataL(image[p*122+c]);//Вывод очередного байта в индикатор
		}

		WriteCodeR(p|0xB8);//Установка текущей страницы для правого кристалла индикатора
		WriteCodeR(0x00);//Установка текущего адреса для записи данных в левую отображаемую позицию правой половины индикатора
		for(c=61; c<122; c++)
		{//Цикл вывода данных в правую половину индикатора
			WriteDataR(image[p*122+c]);//Вывод очередного байта в индикатор
		}
	}
}

uint32_t meltGetImageColumn(uint8_t* image, uint16_t width, uint16_t height, uint16_t column)
{
	uint32_t columnImage = 0;

	columnImage = image[column] ;
	if( (height-1) / 8)  columnImage |= image[column + width]<<0x08;
	if( (height-1) / 16) columnImage |= image[column + 2*width]<<0x10;
	if( (height-1) / 24) columnImage |= image[column + 3*width]<<0x18;

	return columnImage;
}

uint8_t meltPrintLocalImage(uint8_t* image, uint16_t X, uint16_t Y, uint16_t width, uint16_t height, meltAlign_t align)
{
	int x = X, y = Y;

	uint8_t	page;//Номер текущей страницы индикатора
	uint8_t pageLast;

	uint8_t	column;//Позиция по горизонтали выводимого байта

	x -= (width>>1)*(align%3);
	y -= (height>>1)*(align/3);

	if(x < 0 || y < 0) return 1;
	if(x + width > lcdMax_X || y + height > lcdMax_Y) return 2;

	pageLast = (y+height)>>3;
	pageLast += (y+height) - (pageLast<<3) ? 1 : 0;

	uint32_t columnData = 0;

	for(page = y/8; page < pageLast; page++)
	{//Цикл по всем 4-м страницам индикатора
		WriteCodeL(page|0xB8);//Установка текущей страницы для левого кристалла индикатора
		WriteCodeL(0x13 + x);//Установка текущего адреса для записи данных в левую отображаемую позицию левой половины индикатора
		for(column=x; column < x + width && column<61; column++)
		{//Цикл вывода данных в левую половину индикатора
			WriteDataL(((meltGetImageColumn(image, width, height, column - x) << y) >> (page<<3)) & 0xFF);
		}

		if(x < 61 && x + width > 61)
		{
			WriteCodeR(page|0xB8);//Установка текущей страницы для правого кристалла индикатора
			WriteCodeR(0x00);//Установка текущего адреса для записи данных в левую отображаемую позицию правой половины индикатора
			for(column = 61; column < x + width; column++)
			{//Цикл вывода данных в правую половину индикатора
				WriteDataR(((meltGetImageColumn(image, width, height, column - x) << y) >> (page<<3)) & 0xFF);
			}
		}
		else if(x >= 61)
		{
			WriteCodeR(page|0xB8);//Установка текущей страницы для правого кристалла индикатора
			WriteCodeR(x - 61);//Установка текущего адреса для записи данных в левую отображаемую позицию правой половины индикатора
			for(column = x; column < x + width; column++)
			{//Цикл вывода данных в правую половину индикатора
				WriteDataR(((meltGetImageColumn(image, width, height, column - x) << y) >> (page<<3)) & 0xFF);
			}
		}
	}

	return 0;
}

//Пример программы для вывода картинки на индикаторы MT-12232A, MT-12232C, MT-12232D
void meltFillAllScreen(uint8_t byte)
{
	uint8_t	p;//Номер текущей страницы индикатора
	uint8_t	c;//Позиция по горизонтали выводимого байта

	for(p=0; p<4; p++) 
	{//Цикл по всем 4-м страницам индикатора
		WriteCodeL(p|0xB8);//Установка текущей страницы для левого кристалла индикатора
		WriteCodeL(0x13);//Установка текущего адреса для записи данных в левую отображаемую позицию левой половины индикатора
		for(c=0; c<61; c++) { WriteDataL(byte); }

		WriteCodeR(p|0xB8);//Установка текущей страницы для правого кристалла индикатора
		WriteCodeR(0x00);//Установка текущего адреса для записи данных в левую отображаемую позицию правой половины индикатора
		for(c=61; c<122; c++) { WriteDataR(byte); }
	}
}

void meltClearScreenZone(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd)
{
	for(uint8_t page = yStart/8; page <= yEnd/8; page++)
	{//Цикл по всем 4-м страницам индикатора
		WriteCodeL(page|0xB8);//Установка текущей страницы для левого кристалла индикатора
		WriteCodeL(0x13 + xStart);//Установка текущего адреса для записи данных в левую отображаемую позицию левой половины индикатора
		for(uint16_t column=xStart; column <= xEnd && column<61; column++) { WriteDataL(0x00); }

		if(xStart < 61 && xEnd >= 61)
		{
			WriteCodeR(page|0xB8);//Установка текущей страницы для правого кристалла индикатора
			WriteCodeR(0x00);//Установка текущего адреса для записи данных в левую отображаемую позицию правой половины индикатора
			for(uint16_t column = 61; column <= xEnd; column++) { WriteDataR(0x00); }
		}
		else if(xEnd >= 61)
		{
			WriteCodeR(page|0xB8);//Установка текущей страницы для правого кристалла индикатора
			WriteCodeR(xStart - 61);//Установка текущего адреса для записи данных в левую отображаемую позицию правой половины индикатора
			for(uint16_t column = xStart; column <= xEnd; column++) { WriteDataR(0x00); }
		}

	}
}

uint8_t meltPrintDigit(uint16_t X, uint16_t Y, uint8_t digit, meltAlign_t align)
{
	if(digit > 9) return 1;

	return meltPrintLocalImage(fontDigits_10x16[digit], X, Y, 10, 16, align);
}

uint8_t meltPrintInteger(uint16_t X, uint16_t Y, int64_t number, meltAlign_t align)
{
	uint8_t digits[20] = {0};

	uint8_t numOfDigits = 1;
	uint8_t isNegative = number < 0 ? 1 : 0;

	int x = X, y = Y;

	if( isNegative ){ number *= -1; }

	if( number )
	{
		for(numOfDigits = 0; number > 0; numOfDigits++)
		{
			digits[numOfDigits] = number % 10;
			number /= 10;
		}
	}

	x -= ( (fontW>>1)*(align%3) )*(int)(numOfDigits);
	y -= (fontH>>1)*(align/3);

	if(isNegative)
	{
		x -= (align % 3) * fontW/2;

		meltPrintLocalImage(fontDigits_10x16[13], x, y, fontW, fontH, alignTL);

		x += fontW;
	}

	for(uint8_t i = 0; i < numOfDigits; i++)
	{
		meltPrintDigit(x + i*fontW, y, digits[numOfDigits - i - 1], alignTL);
	}

	return 0;
}

uint8_t meltPrintFixedPoint(uint16_t X, uint16_t Y, int64_t number, uint8_t fixedPoint, meltAlign_t align)
{
	if(!fixedPoint) return meltPrintInteger(X, Y, number, align);

	uint8_t isNegative = number < 0 ? 1 : 0;
	uint8_t unsignificantZeros = 0;
	uint16_t dx = 0, dy = 0;
	int64_t integer, tail, tens = 1;

	int x = X, y = Y;

	if(isNegative)	{ number *= -1; }

	for(uint8_t cursor = 0; cursor < fixedPoint; cursor++) { tens *= 10; }

	tail = number%tens;
	integer = number/tens;

	while( !(tail%10) )
	{
		tail /= 10;
		unsignificantZeros++;

		if( !tail ) break;
	};

	dx = (fontW*(miscIntLen(integer) + fixedPoint + isNegative) + (fixedPoint ? 3 : 0) )>>1;
	dy = fontH>>1;

	x -= dx*(align % 3);
	y -= dy*(align / 3);
	meltPrintInteger(x, y, isNegative ? -1*integer : integer, alignTL);

	x += fontW*(miscIntLen(integer) + isNegative);
	meltPrintLocalImage( fontDigits_10x16[10], x, y, fontW, fontH, alignTL);

	x += 3;
	for(uint8_t i = 0; i < fixedPoint - miscIntLen(tail) - unsignificantZeros; i++)
	{
		meltPrintInteger(x, y, 0, alignTL);
		x += fontW;
	}
	meltPrintInteger(x, y, tail, alignTL);
	for(tail; tail; tail /= 10) { x += fontW; }
	for(uint8_t i = 0; i < unsignificantZeros; i++)
	{
		meltPrintInteger(x, y, 0, alignTL);
		x += fontW;
	}

	return 0;
}

void meltLEDSetState(int8_t state)
{
	if		( state < 0 ) 	{ LL_GPIO_TogglePin(pinLCD_LedControl_GPIO_Port, pinLCD_LedControl_Pin); }
	else if	( state == 0) 	{ LL_GPIO_ResetOutputPin(pinLCD_LedControl_GPIO_Port, pinLCD_LedControl_Pin); }
	else if	( state > 0 ) 	{ LL_GPIO_SetOutputPin(pinLCD_LedControl_GPIO_Port, pinLCD_LedControl_Pin); }
}

uint8_t meltLEDGetState(void)
{
	return LL_GPIO_IsOutputPinSet(pinLCD_LedControl_GPIO_Port, pinLCD_LedControl_Pin) != 0;
}

void meltFillPageZone(uint8_t byte, uint8_t page, uint8_t xStart, uint8_t xEnd)
{
	if( xStart >= lcdMax_X || xEnd >= lcdMax_X || page > 3) return;

	if(xStart >= 61)
	{
		WriteCodeR(page|0xB8);//Установка текущей страницы для правого кристалла индикатора
		WriteCodeR(xStart - 61);//Установка текущего адреса для записи данных в левую отображаемую позицию правой половины индикатора
		for(uint8_t column = xStart; column <= xEnd; column++) { WriteDataR(byte); }

		return;
	}

	WriteCodeL(page|0xB8);//Установка текущей страницы для левого кристалла индикатора
	WriteCodeL(0x13 + xStart);//Установка текущего адреса для записи данных в левую отображаемую позицию левой половины индикатора
	for(uint8_t column = xStart; column <= xEnd && column < 61; column++) { WriteDataL(byte); }

	if(xEnd >= 61)
	{
		WriteCodeR(page|0xB8);//Установка текущей страницы для правого кристалла индикатора
		WriteCodeR(0x00);//Установка текущего адреса для записи данных в левую отображаемую позицию правой половины индикатора
		for(uint8_t column = 61; column <= xEnd; column++) { WriteDataR(byte); }
	}
}

void meltTst(void)
{
	int64_t number = 10120;

	meltFillAllScreen(0x00);
	for(uint8_t align = 0; align <= alignDR; align++)
	{
		meltPrintFixedPoint((lcdMax_X/2 - 1)*(align%3), (lcdMax_Y/2 - 1)*(align/3), number, 3, align);
		meltFillAllScreen(0x00);
	}
}
