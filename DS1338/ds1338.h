/*
 * ds1338.h
 *
 *  Created on: Apr 10, 2023
 *      Author: DL
 */

#ifndef DS1338_H_
#define DS1338_H_

#include <i2cSoft.h>
#include "main.h"

enum
{
	dsSeconds,
	dsMinutes,
	dsHours,

	dsWeekday,
	dsDay,
	dsMonth,
	dsYear,

	dsControl,
};

typedef struct
{
	uint8_t* Data;

	void (*GetTime)(void);
	void (*GetDate)(void);

	void (*SetTime)(void);
	void (*SetDate)(void);
}ds1338_t;

extern ds1338_t ds1338;

#endif /* DS1338_H_ */
