/*
 * image.h
 *
 *  Created on: Mar 9, 2023
 *      Author: DL
 */

#ifndef MT_12232A_IMAGE_H_
#define MT_12232A_IMAGE_H_

typedef struct
{
	unsigned char W;
	unsigned char H;
	unsigned char *Data;
}image_t;

extern image_t imgKilo;

#endif /* MT_12232A_IMAGE_H_ */
