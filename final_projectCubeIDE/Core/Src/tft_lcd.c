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

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= 320 || y >= 240) return;

    LCD_SetAddressWindow(x, y, x, y);

    // Send 16-bit color as two 8-bit data packets
    LCD_WriteData(color >> 8);   // High byte
    LCD_WriteData(color & 0xFF); // Low byte
}

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    int dx = (x1 < x2) ? (x2 - x1) : (x1 - x2);
    int dy = (y1 < y2) ? (y2 - y1) : (y1 - y2);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        LCD_DrawPixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void LCD_DrawModeSelect() {
    // Clear Screen
    LCD_DrawRect(0, 0, 320, 240, 0x0000);

    // Header with double-line underline
    LCD_WriteString(60, 20, "SYSTEM MAIN MENU", Font_11x18, 0x07E0, 0x0000);
    LCD_DrawRect(50, 42, 220, 2, 0xFFFF);
    LCD_DrawRect(50, 46, 220, 1, 0xFFFF);

    // Option 1 Box
//    LCD_DrawRect(15, 70, 290, 60, 0xFFFF); // White Border
    LCD_WriteString(35, 92, "1: CART CONTROL", Font_11x18, 0xFFFF, 0x0000);

    // Option 2 Box
//    LCD_DrawRect(15, 145, 290, 60, 0xFFFF); // White Border
    LCD_WriteString(35, 167, "2: SIGNAL GENERATOR", Font_11x18, 0xFFFF, 0x0000);
}

void LCD_DrawCartMain() {
    LCD_DrawRect(0, 0, 320, 240, 0x0000);

    // Sidebar Indicator
    LCD_DrawRect(0, 0, 10, 240, 0x07E0); // Green side-strip for "Active"

    // Title
    LCD_WriteString(25, 20, "SMART UTILITY CART", Font_11x18, 0xFFFF, 0x0000);
    LCD_DrawRect(25, 42, 270, 2, 0x7BEF);

    // Status Rows
    LCD_WriteString(25, 80, "STATUS: ", Font_11x18, 0xFFFF, 0x0000);
    LCD_WriteString(120, 80, "STANDBY", Font_11x18, 0x07E0, 0x0000);

    LCD_WriteString(25, 120, "SPEED:  ", Font_11x18, 0xFFFF, 0x0000);
    LCD_WriteString(120, 120, "0.0 m/s", Font_11x18, 0xFFFF, 0x0000);

    // Footer
    LCD_DrawRect(0, 210, 320, 30, 0x3186); // Gray footer bar
    LCD_WriteString(45, 216, "PRESS [#] TO EXIT", Font_11x18, 0xFFFF, 0x3186);
}

void LCD_DrawSigMain() {
    LCD_DrawRect(0, 0, 320, 240, 0x0000);

    // Waveform Viewport (Top Box)
    LCD_DrawRect(5, 5, 310, 140, 0x7BEF); // Gray border for the grid
    LCD_WriteString(85, 65, "[WAVEFORM]", Font_11x18, 0x7BEF, 0x0000); // Placeholder

    // Control Label
    LCD_DrawRect(0, 150, 320, 2, 0xFFFF);
    LCD_WriteString(20, 160, "SELECT INPUT TYPE:", Font_11x18, 0x07E0, 0x0000);

    // Menu Options
    LCD_WriteString(20, 190, "3: MANUAL INPUT", Font_11x18, 0xFFFF, 0x0000);
    LCD_WriteString(20, 215, "4: PRESET WAVES", Font_11x18, 0xFFFF, 0x0000);
}

void LCD_DrawArbInput() {
    LCD_DrawRect(0, 0, 320, 240, 0x0000);

    // Header
    LCD_WriteString(10, 10, "ARBITRARY GENERATOR", Font_11x18, 0xF81F, 0x0000); // Magenta
    LCD_DrawLine(0, 35, 320, 35, 0xFFFF);

    // Row 1: File/Source Status
    LCD_WriteString(10, 50, "SOURCE: SD CARD", Font_11x18, 0xFFFF, 0x0000);

    // Row 2: Digital Filter
    LCD_WriteString(10, 90, "1. FILTER:", Font_11x18, 0xFFFF, 0x0000);

    // Row 3: Output Mode (FFT or Wave)
    LCD_WriteString(10, 130, "2. OUTPUT:", Font_11x18, 0xFFFF, 0x0000);

    // Row 4: Sample Size
    LCD_WriteString(10, 170, "SAMPLES:", Font_11x18, 0xFFFF, 0x0000);

    // Footer
    LCD_WriteString(10, 215, "[*] Run  [#] Back", Font_11x18, 0xF800, 0x0000);
}

void LCD_DrawFuncInput() {
    LCD_DrawRect(0, 0, 320, 240, 0x0000); // Clear Screen Black

    // Header
    LCD_WriteString(10, 10, "FUNCTION GENERATOR", Font_11x18, 0x07FF, 0x0000); // Cyan
    LCD_DrawLine(0, 35, 320, 35, 0xFFFF); // Divider line

    // Row 1: Wave Shape
    LCD_WriteString(10, 50, "1. SHAPE:", Font_11x18, 0xFFFF, 0x0000);
    // Logic later: Display "Sine", "Square", etc. based on wav_function

    // Row 2: Frequency
    LCD_WriteString(10, 90, "2. FREQ (Hz):", Font_11x18, 0xFFFF, 0x0000);
    // Display current freq (e.g., "1000")

    // Row 3: Amplitude
    LCD_WriteString(10, 130, "3. AMPL (V):", Font_11x18, 0xFFFF, 0x0000);
    // Display current amplitude (e.g., "1.20")

    // Row 4: Filter (NOT DOING)
//    LCD_WriteString(10, 170, "4. FILTER:", Font_11x18, 0xFFFF, 0x0000);

    // Footer
    LCD_WriteString(10, 215, "[*] Run  [#] Back", Font_11x18, 0xF800, 0x0000); // Red hint
}


