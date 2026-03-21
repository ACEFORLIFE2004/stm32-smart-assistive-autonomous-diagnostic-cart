/*
 * fonts.h
 *
 *  Created on: Mar 21, 2026
 *      Author: akhan
 */

#ifndef INC_FONTS_H_
#define INC_FONTS_H_

#include "stm32f4xx_hal.h"
#include "string.h"

typedef struct {
	uint8_t FontWidth;    /*!< Font width in pixels */
	uint8_t FontHeight;   /*!< Font height in pixels */
	const uint16_t *data; /*!< Pointer to data font data array */
} FontDef_t;

extern FontDef_t Font_11x18;

#endif /* INC_FONTS_H_ */
