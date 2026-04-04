#include "bluetooth.h"
#include "motor_driver.h"
#include "system.h"
#include <math.h>

float current_left = 0, current_right = 0;
float ramped_accel = 0;
volatile int16_t target_left = 0, target_right = 0;
float acceleration_step = 5.0f;
uint8_t rx_buffer[3];
uint32_t last_packet_time = 0;

extern void LCD_UpdateCartSpeed(float speed);
extern float Get_Distance(uint16_t trig_pin);
extern void LCD_UpdateCartStatus(char* status, uint16_t color);

void BT_Init(void) {
    // Reset the timer so the watchdog doesn't kill it immediately
    last_packet_time = HAL_GetTick();

    // We are expecting 3 bytes: [0xFF, Accel, Steer]
    HAL_UART_Receive_IT(&huart6, rx_buffer, 3);
}

void BT_Update_Physics(void) {

    if (__HAL_UART_GET_FLAG(&huart6, UART_FLAG_ORE)) {
        print_msg("!!! UART6 OVERRUN ERROR - Resetting Listener !!!\r\n");
        __HAL_UART_CLEAR_OREFLAG(&huart6);
        HAL_UART_Receive_IT(&huart6, rx_buffer, 3);
    }

    // Safety Guard
    SystemEnv_t* env = get_system_state();
    if (get_global_state() != SYS_RUNNING || env->system_mode != MODE_ASSIST) {
        motor_apply_physics(0, 0);
        current_left = 0;
        current_right = 0;
        return;
    }

	// --- EMERGENCY BRAKE ---
	if (rx_buffer[0] == 0xEE) {
		target_left = 127;
		target_right = 127;
		current_left = 0;
		current_right = 0;
		ramped_accel = 0;

		motor_apply_physics(0, 0);
		cart_brake();

		LCD_UpdateCartStatus("EMERGENCY!", 0xF800);
		return;
	}

    // Watchdog
    if (HAL_GetTick() - last_packet_time > 500) {
        target_left = 0;
        target_right = 0;
    }

    // Convert targets back to their raw components (Accel vs Steer)
    int16_t current_accel = (target_left + target_right) / 2;
    int16_t current_steer = (target_left - target_right) / 2;

    // APPLY RAMPING ONLY TO ACCEL (Forward/Backward)
    if (fabsf(ramped_accel - (float)current_accel) < acceleration_step) {
        ramped_accel = (float)current_accel;
    } else {
        if (ramped_accel < (float)current_accel) ramped_accel += acceleration_step;
        else ramped_accel -= acceleration_step;
    }

    // COMBINE RAMPED ACCEL WITH INSTANT STEER
    current_left = ramped_accel + (float)(current_steer/2);
    current_right = ramped_accel - (float)(current_steer/2);

    // Final Clamp to stay in PWM range
    if (current_left > 127.0f) current_left = 127.0f;
    if (current_left < -127.0f) current_left = -127.0f;
    if (current_right > 127.0f) current_right = 127.0f;
    if (current_right < -127.0f) current_right = -127.0f;

    // Send to Motors
    motor_apply_physics(current_left, current_right);

    // Update LCD (Speed AND Status)
    static uint32_t last_lcd_update = 0;
    static uint8_t is_currently_moving = 0; // Track current UI state

    if (HAL_GetTick() - last_lcd_update > 500) {
        float average_raw = (fabsf(current_left) + fabsf(current_right)) / 2.0f;
        float display_speed = (average_raw / 127.0f) * 1.5f;

        // Switch to DRIVING at 0.1, but only switch back to STANDBY if under 0.02
        if (!is_currently_moving && display_speed > 0.10f) {
            LCD_UpdateCartStatus("DRIVING", 0xF800);
            is_currently_moving = 1;
        }
        else if (is_currently_moving && display_speed < 0.02f) {
            LCD_UpdateCartStatus("STANDBY", 0x07E0);
            is_currently_moving = 0;
        }

        LCD_UpdateCartSpeed(display_speed);
        last_lcd_update = HAL_GetTick();
    }
}

void BT_Process_Packet(uint8_t accel_raw, uint8_t steer_raw) {
	int16_t accel = (int16_t)accel_raw - 127;
	int16_t steer = (int16_t)steer_raw - 127;

	target_left = accel + steer;
	target_right = accel - steer;

	char debug_str[64];
	sprintf(debug_str, "IN: %d, %d | TGT L:%d R:%d\r\n",
			(int)accel_raw, (int)steer_raw, (int)target_left, (int)target_right);
	HAL_UART_Transmit(&huart6, (uint8_t*)debug_str, strlen(debug_str), 10);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART6) {
        last_packet_time = HAL_GetTick();

        if (rx_buffer[0] == 0xEE) {
            // Kill the targets immediately
            target_left = 127;
            target_right = 127;

            // Kill the ramping variables instantly
            current_left = 0;
            current_right = 0;

            // Apply Brake
            cart_brake();
        }
        else if (rx_buffer[0] == 0xFF) {
            BT_Process_Packet(rx_buffer[1], rx_buffer[2]);
        }

        HAL_UART_Receive_IT(&huart6, rx_buffer, 3);
    }
}
