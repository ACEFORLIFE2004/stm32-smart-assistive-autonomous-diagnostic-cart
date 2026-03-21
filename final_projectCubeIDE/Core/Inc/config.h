/*
 * config.h
 *
 *  Created on: Mar 17, 2026
 *      Author: arnol
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

#include "main.h"

/* Private variables ---------------------------------------------------------*/
extern ADC_HandleTypeDef hadc1;

extern DAC_HandleTypeDef hdac;
extern DMA_HandleTypeDef hdma_dac1;

extern I2C_HandleTypeDef hi2c1;

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi4;
extern DMA_HandleTypeDef hdma_spi4_rx;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim9;
extern TIM_HandleTypeDef htim11;
extern TIM_HandleTypeDef htim14;

extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_SPI4_Init(void);
void MX_ADC1_Init(void);
void MX_DAC_Init(void);
void MX_I2C1_Init(void);
void MX_SPI1_Init(void);
void MX_TIM11_Init(void);
void MX_TIM14_Init(void);
void MX_TIM6_Init(void);
void MX_USART3_UART_Init(void);
void MX_USART6_UART_Init(void);
void MX_TIM2_Init(void);
void MX_TIM5_Init(void);
void MX_TIM9_Init(void);

void CSTM_Core_GPIO_Init(void);
void CSTM_Cart_GPIO_Init(void);

void SYS_Cart_Mode_Init(void);
void SYS_Diagnostic_Mode_Init(void);
void SYS_Core_Peripheral_Init(void);

void SYS_Cart_Mode_DeInit(void);
void SYS_Diagnostic_Mode_DeInit(void);
void SYS_Core_Peripheral_DeInit(void);


#endif /* INC_CONFIG_H_ */
