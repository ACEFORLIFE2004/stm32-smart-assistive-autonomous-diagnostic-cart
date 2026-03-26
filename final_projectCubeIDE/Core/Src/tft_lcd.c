/*
 * tft_lcd.c
 *
 *  Created on: Mar 21, 2026
 *      Author: akhan
 */

#include "tft_lcd.h"

// Sends a single command byte
void LCD_WriteCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET); // DC Low = Command
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET); // CS Low
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);   // CS High
}

// Sends a single data byte
void LCD_WriteData(uint8_t data) {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);   // DC High = Data
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET); // CS Low
    HAL_SPI_Transmit(&hspi1, &data, 1, 10);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);   // CS High
}

void LCD_Init(void) {
    HAL_Delay(150);
    LCD_WriteCommand(0x01); // Software Reset
    HAL_Delay(150);
    LCD_WriteCommand(0x11); // Sleep Out
    HAL_Delay(150);

    LCD_WriteCommand(0x3A); // Pixel Format
    LCD_WriteData(0x55);    // 16-bit

    LCD_WriteCommand(0x36); // Rotation
    LCD_WriteData(0xE8);    // Landscape

    LCD_WriteCommand(0x29); // Display ON
}

void LCD_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    // Set Column Address
    LCD_WriteCommand(0x2A);
    LCD_WriteData(x0 >> 8); LCD_WriteData(x0 & 0xFF);
    LCD_WriteData(x1 >> 8); LCD_WriteData(x1 & 0xFF);

    // Set Page Address
    LCD_WriteCommand(0x2B);
    LCD_WriteData(y0 >> 8); LCD_WriteData(y0 & 0xFF);
    LCD_WriteData(y1 >> 8); LCD_WriteData(y1 & 0xFF);

    LCD_WriteCommand(0x2C); // Ready to write to RAM
}

void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    LCD_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    uint8_t colorData[] = { color >> 8, color & 0xFF };

    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);   // DC High
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET); // CS Low

    for (uint32_t i = 0; i < (w * h); i++) {
        HAL_SPI_Transmit(&hspi1, colorData, 2, 10);
    }

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);   // CS High
}

void LCD_DrawChar(uint16_t x, uint16_t y, char c, FontDef_t font, uint16_t color, uint16_t bg) {
    uint32_t i, b, j;

    // Set the window for the character
    LCD_SetAddressWindow(x, y, x + font.FontWidth - 1, y + font.FontHeight - 1);

    // Loop through each row of the character (18 rows)
    for (i = 0; i < font.FontHeight; i++) {
        // Get the 16-bit row data for the character (ASCII offset is 32)
        b = font.data[(c - 32) * font.FontHeight + i];

        // Loop through each bit (11 bits wide)
        for (j = 0; j < font.FontWidth; j++) {
            // Check if bit is set (starting from the most significant bit)
            if ((b << j) & 0x8000) {
                // Send Color (High Byte, then Low Byte)
                uint8_t colorData[] = { color >> 8, color & 0xFF };
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); // DC High
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET); // CS Low
                HAL_SPI_Transmit(&hspi1, colorData, 2, 10);
            } else {
                // Send Background Color
                uint8_t bgData[] = { bg >> 8, bg & 0xFF };
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); // DC High
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET); // CS Low
                HAL_SPI_Transmit(&hspi1, bgData, 2, 10);
            }
        }
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET); // CS High
}

void LCD_WriteString(uint16_t x, uint16_t y, const char* str, FontDef_t font, uint16_t color, uint16_t bg) {
    while (*str) {
        // Check if we ran off the screen
        if (x + font.FontWidth >= 320) break;

        LCD_DrawChar(x, y, *str, font, color, bg);
        x += font.FontWidth; // Move to the next character position
        str++;
    }
}

void LCD_DrawModeSelect() {

	LCD_DrawRect(0, 0, 320, 240, 0x0000);

	LCD_WriteString(5, 90, "1: Select Cart Mode", Font_11x18, 0xFFFF, 0x0000);
	LCD_WriteString(5, 130, "2: Select Signal Generation Mode", Font_11x18, 0xFFFF, 0x0000);

}

void LCD_DrawCartMain() { /* TO DO: CHECK IT OUT AND PLACE CORRECTLY */

	LCD_DrawRect(0, 0, 320, 240, 0x0000);

	LCD_WriteString(105, 105, "Cart Mode", Font_11x18, 0xFFFF, 0x0000);

}

void LCD_DrawSigMain() {

	LCD_DrawRect(0, 0, 320, 240, 0x0000);

	LCD_DrawRect(5, 5, 310, 160, 0x7BEF);

	LCD_WriteString(5, 180, "3: Select Arbitrary Waveform", Font_11x18, 0xFFFF, 0x0000);
	LCD_WriteString(5, 200, "4: Input a Waveform", Font_11x18, 0xFFFF, 0x0000);
}

void LCD_DrawSigInput() {

	LCD_DrawRect(0, 0, 320, 240, 0x0000);

	// Placeholder
	LCD_WriteString(50, 105, "Waveform Input Screen", Font_11x18, 0xFFFF, 0x0000);

}


