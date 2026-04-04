/*
 * sensor.c
 *
 *  Created on: Mar 31, 2026
 *      Author: akhan
 */

#include "main.h"
#include "timer_system.h"

volatile uint32_t echo_start = 0;
volatile uint32_t echo_end = 0;
volatile float latest_dist = -1.0f;

uint8_t obstacle_detected = 0;

// This function is called automatically by the HAL_GPIO_EXTI_IRQHandler

void delay_us(uint32_t us) {
    uint32_t startTick = DWT->CYCCNT;
    uint32_t delayTicks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - startTick) < delayTicks);
}

float Get_Distance(uint16_t trig_pin) {
    // 1. Trigger the sensor (10us pulse)
    HAL_GPIO_WritePin(GPIOD, trig_pin, GPIO_PIN_SET);

    // 3. Start your TIM5 measurement
    measure_exec_time(LOW_RANGE);

    delay_us(10);
    HAL_GPIO_WritePin(GPIOD, trig_pin, GPIO_PIN_RESET);

    // 2. Wait for Echo to start
//    uint32_t timeout = 100000;
//    while(HAL_GPIO_ReadPin(GPIOD, SENSOR_ECHO_Pin) == GPIO_PIN_RESET) {
//        if (timeout-- == 0) return -1.0f;
//    }
//    uint32_t = GetTick();
    while(!obstacle_detected){
    	print_msg("  Still waiting\r\n");
    }

    obstacle_detected = 0;

    // 4. Wait for Echo to end (The 5-10ms "hang" you're okay with)
//    timeout = 100000;
//    while(HAL_GPIO_ReadPin(GPIOD, SENSOR_ECHO_Pin) == GPIO_PIN_SET) {
//        if (timeout-- == 0) break;
//    }

    // 5. Get the time in microseconds
    // Note: You might want a version of get_exec_time that doesn't
    // print to UART every time, or it will flood your terminal!
    uint32_t pulse_us = get_exec_time("HC-SR04", LOW_RANGE, MICRO_SECONDS);

    // 6. Math: (Time in us * Speed of Sound in ft/us) / 2
    // Speed of sound is approx 0.001125 ft/us
    return pulse_us;
}
