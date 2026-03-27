/*
 * tft_lcd.h
 *
 *  Created on: Mar 21, 2026
 *      Author: akhan
 */

#ifndef INC_TFT_LCD_H_
#define INC_TFT_LCD_H_

#include "project.h"
#include "fonts.h"

void LCD_WriteCommand(uint8_t cmd);

void LCD_WriteData(uint8_t data);

void LCD_Init(void);

void LCD_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

void LCD_DrawChar(uint16_t x, uint16_t y, char c, FontDef_t font, uint16_t color, uint16_t bg);

void LCD_WriteString(uint16_t x, uint16_t y, const char* str, FontDef_t font, uint16_t color, uint16_t bg);

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

void LCD_DrawSigMain();

void LCD_DrawCartMain();

void LCD_DrawModeSelect();

void LCD_DrawArbInput();

void LCD_DrawFuncInput();

#endif /* INC_TFT_LCD_H_ */
