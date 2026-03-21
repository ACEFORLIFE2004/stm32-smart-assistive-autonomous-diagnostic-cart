/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

#include "project.h"
#include "system.h"

#include "../Tests/test_project.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
#define FN_MAIN_ID	0x00
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

extern SPI_HandleTypeDef hspi1;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
//  MX_SPI4_Init();
//  MX_ADC1_Init();
//  MX_DAC_Init();
//  MX_I2C1_Init();
  MX_SPI1_Init();
//  MX_TIM11_Init();
//  MX_TIM14_Init();
//  MX_TIM6_Init();
  MX_USART3_UART_Init();
//  MX_USART6_UART_Init();
//  MX_FATFS_Init();
//  MX_TIM2_Init();
  MX_TIM5_Init();
  MX_TIM9_Init();

  /* USER CODE BEGIN 2 */
  HAL_Delay(500);

  LCD_Init();

  // Fill background with Dark Blue (0x0010)
  LCD_DrawRect(0, 0, 320, 240, 0x0010);

  // Draw a "START" button (Green)
  LCD_DrawRect(40, 80, 100, 60, 0x07E0);

  // Draw a "STOP" button (Red)
  LCD_DrawRect(180, 80, 100, 60, 0xF800);

  /* USER CODE END 2 */

  /* Initialize variables to assist with project test */
  test_no = 0;
  event_seq_num = 0;

  /* Clear and Initialize Call Stack for Debugging */
  memset(call_stack, 0xFF, MAX_FUNCTION_CALL_DEPTH);
  call_depth = 0;
  append_stack(FN_MAIN_ID);

  /* Hard code initial global state to SYS_BOOT */
  set_system_boot_state();

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
//	measure_exec_time(DEFAULT_RANGE);									/* Start Test */
	system_update();
//	(void)get_exec_time("system_update", DEFAULT_RANGE, NANO_SECONDS);	/* End Test */
	run_project_test();

//	HAL_Delay(10);
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
