#include "bluetooth.h"
#include "motor_driver.h"

// 1. Declare the current and target states for BOTH wheels
float current_left = 0, current_right = 0;
int16_t target_left = 0, target_right = 0;

float acceleration_step = 0.5; // Smoothness factor
uint8_t rx_buffer[3];

void BT_Init(void) {
    HAL_UART_Receive_IT(&huart6, rx_buffer, 3);
}

void BT_Update_Physics(void) {
    // Ramp Left Wheel
    if (current_left < target_left) current_left += acceleration_step;
    else if (current_left > target_left) current_left -= acceleration_step;

    // Ramp Right Wheel
    if (current_right < target_right) current_right += acceleration_step;
    else if (current_right > target_right) current_right -= acceleration_step;

    // 2. Apply to Motors (Example logic to handle Direction)
    // need to check if current_left is positive (forward) or negative (back)
    // and call drive_forward or a custom drive_backward.
}

void BT_Process_Packet(uint8_t accel_raw, uint8_t steer_raw) {
    int16_t accel = (int16_t)accel_raw - 127;
    int16_t steer = (int16_t)steer_raw - 127;

    // Mixing for differential drive
    target_left = accel + steer;
    target_right = accel - steer;

    // Caps based on the 0-100 range
    if (target_left > 100) target_left = 100;
    if (target_left < -100) target_left = -100;
    if (target_right > 100) target_right = 100;
    if (target_right < -100) target_right = -100;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART6) {
        if (rx_buffer[0] == 0xAA) {
            // 3. Use the mixing function we just polished
            BT_Process_Packet(rx_buffer[1], rx_buffer[2]);
        }
        HAL_UART_Receive_IT(&huart6, rx_buffer, 3);
    }
}
