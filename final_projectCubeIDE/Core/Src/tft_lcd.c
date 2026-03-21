/*
 * tft_lcd.c
 *
 *  Created on: Mar 21, 2026
 *      Author: akhan
 */


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
    LCD_WriteData(0x28);    // Landscape

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
