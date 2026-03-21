/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MicroSD_SCK_Pin GPIO_PIN_2
#define MicroSD_SCK_GPIO_Port GPIOE
#define MicroSD_DET1_Pin GPIO_PIN_3
#define MicroSD_DET1_GPIO_Port GPIOE
#define MicroSD_NSS_Pin GPIO_PIN_4
#define MicroSD_NSS_GPIO_Port GPIOE
#define MicroSD_MISO_Pin GPIO_PIN_5
#define MicroSD_MISO_GPIO_Port GPIOE
#define MicroSD_MOSI_Pin GPIO_PIN_6
#define MicroSD_MOSI_GPIO_Port GPIOE
#define MOTOR_L1_Pin GPIO_PIN_7
#define MOTOR_L1_GPIO_Port GPIOF
#define MOTOR_L2_Pin GPIO_PIN_9
#define MOTOR_L2_GPIO_Port GPIOF
#define ROW3_Pin GPIO_PIN_0
#define ROW3_GPIO_Port GPIOA
#define DAC_OUT_Pin GPIO_PIN_4
#define DAC_OUT_GPIO_Port GPIOA
#define TFT_LCD_SCK_Pin GPIO_PIN_5
#define TFT_LCD_SCK_GPIO_Port GPIOA
#define TFT_LCD_MISO_Pin GPIO_PIN_6
#define TFT_LCD_MISO_GPIO_Port GPIOA
#define TFT_LCD_MOSI_Pin GPIO_PIN_7
#define TFT_LCD_MOSI_GPIO_Port GPIOA
#define ROW2_Pin GPIO_PIN_0
#define ROW2_GPIO_Port GPIOB
#define ADC_Potentiometer_Pin GPIO_PIN_1
#define ADC_Potentiometer_GPIO_Port GPIOB
#define ROW4_Pin GPIO_PIN_2
#define ROW4_GPIO_Port GPIOB
#define BT_STATE_Pin GPIO_PIN_8
#define BT_STATE_GPIO_Port GPIOE
#define BT_STATE_EXTI_IRQn EXTI9_5_IRQn
#define MOTOR_R1_Pin GPIO_PIN_10
#define MOTOR_R1_GPIO_Port GPIOB
#define MOTOR_R2_Pin GPIO_PIN_11
#define MOTOR_R2_GPIO_Port GPIOB
#define STLK_RX_Pin GPIO_PIN_8
#define STLK_RX_GPIO_Port GPIOD
#define STLK_TX_Pin GPIO_PIN_9
#define STLK_TX_GPIO_Port GPIOD
#define COL1_Pin GPIO_PIN_11
#define COL1_GPIO_Port GPIOD
#define COL1_EXTI_IRQn EXTI15_10_IRQn
#define COL2_Pin GPIO_PIN_12
#define COL2_GPIO_Port GPIOD
#define COL2_EXTI_IRQn EXTI15_10_IRQn
#define COL3_Pin GPIO_PIN_13
#define COL3_GPIO_Port GPIOD
#define COL3_EXTI_IRQn EXTI15_10_IRQn
#define TFT_LCD_DC_Pin GPIO_PIN_14
#define TFT_LCD_DC_GPIO_Port GPIOD
#define CTPM_IRQ_Pin GPIO_PIN_6
#define CTPM_IRQ_GPIO_Port GPIOC
#define CTPM_IRQ_EXTI_IRQn EXTI9_5_IRQn
#define TFT_LCD_NSS_Pin GPIO_PIN_15
#define TFT_LCD_NSS_GPIO_Port GPIOA
#define IR_RX_LEFT_Pin GPIO_PIN_10
#define IR_RX_LEFT_GPIO_Port GPIOC
#define IR_RX_LEFT_EXTI_IRQn EXTI15_10_IRQn
#define IR_RX_CENTER_Pin GPIO_PIN_11
#define IR_RX_CENTER_GPIO_Port GPIOC
#define IR_RX_CENTER_EXTI_IRQn EXTI15_10_IRQn
#define IR_RX_RIGHT_Pin GPIO_PIN_12
#define IR_RX_RIGHT_GPIO_Port GPIOC
#define IR_RX_RIGHT_EXTI_IRQn EXTI15_10_IRQn
#define SENSOR_ECHO_Pin GPIO_PIN_4
#define SENSOR_ECHO_GPIO_Port GPIOD
#define SENSOR_ECHO_EXTI_IRQn EXTI4_IRQn
#define SENSOR_TRIG_RIGHT_Pin GPIO_PIN_5
#define SENSOR_TRIG_RIGHT_GPIO_Port GPIOD
#define SENSOR_TRIG_CENTER_Pin GPIO_PIN_6
#define SENSOR_TRIG_CENTER_GPIO_Port GPIOD
#define SENSOR_TRIG_LEFT_Pin GPIO_PIN_7
#define SENSOR_TRIG_LEFT_GPIO_Port GPIOD
#define BT_TX_Pin GPIO_PIN_9
#define BT_TX_GPIO_Port GPIOG
#define BT_RX_Pin GPIO_PIN_14
#define BT_RX_GPIO_Port GPIOG
#define ROW1_Pin GPIO_PIN_0
#define ROW1_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
