/*
 * tft_lcd.h
 *
 *  Created on: Mar 21, 2026
 *      Author: akhan
 */

#ifndef INC_TFT_LCD_H_
#define INC_TFT_LCD_H_

#include "project.h"

void LCD_WriteCommand(uint8_t cmd);

void LCD_WriteData(uint8_t data);

void LCD_Init(void);

void LCD_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

#endif /* INC_TFT_LCD_H_ */
