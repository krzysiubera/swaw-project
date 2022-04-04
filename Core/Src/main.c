/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "delays.h"
#include "bme280.h"
#include "ring_buffer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch) {
	if (ch == '\n') {
		uint8_t ch2 = '\r';
		HAL_UART_Transmit(&huart1, &ch2, 1, HAL_MAX_DELAY);
	}
	HAL_UART_Transmit(&huart1, (uint8_t *) &ch, 1, HAL_MAX_DELAY);
	return 1;
}

#define RINGBUF_SIZE 180
RingBuffer ring_buffer;
struct bme280_pkt buffer[RINGBUF_SIZE];

#define LINE_MAX_LENGTH 80
static char line_buffer[LINE_MAX_LENGTH + 1];
static uint32_t line_length;
uint8_t last_received_byte;

void line_append() {
	if (last_received_byte == '\r' || last_received_byte == '\n') {
		if (line_length > 0) {
			line_buffer[line_length] = '\0';

			if (strcmp(line_buffer, "get temp") == 0) {
				printf("%.2f\n", BME280_ReadTemperature());
			} else if (strcmp(line_buffer, "get press") == 0) {
				printf("%.2f\n", BME280_ReadPressure() / (float) 100.0);
			} else if (strcmp(line_buffer, "get hum") == 0) {
				printf("%.2f\n", BME280_ReadHuminidity());
			} else if (strcmp(line_buffer, "get meas temp") == 0) {
				uint32_t count = 0;
				struct bme280_pkt meas;
				while ((RingBuffer_GetPkt(&ring_buffer, &meas)) && (count < ring_buffer.count)) {
					count++;
					printf("%.2f\n", meas.temp);
				}
			} else if (strcmp(line_buffer, "get meas hum") == 0) {
				uint32_t count = 0;
				struct bme280_pkt meas;
				while ((RingBuffer_GetPkt(&ring_buffer, &meas)) && (count < ring_buffer.count)) {
					count++;
					printf("%.2f\n", meas.hum);
				}
			} else if (strcmp(line_buffer, "get meas press") == 0) {
				uint32_t count = 0;
				struct bme280_pkt meas;
				while ((RingBuffer_GetPkt(&ring_buffer, &meas)) && (count < ring_buffer.count)) {
					count++;
					printf("%.2f\n", meas.pres / (float) 100.0);
				}
			}
			else {
				printf("Command not found\n");
			}
			memset(line_buffer, '\0', LINE_MAX_LENGTH);
			line_length = 0;
		}
	} else {
		if (line_length >= LINE_MAX_LENGTH) {
			line_length = 0;
		}
		line_buffer[line_length++] = last_received_byte;
	}
	HAL_UART_Receive_IT(&huart1, &last_received_byte, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	line_append();
}

const uint32_t sample_time_meas_ms = 1000UL;
uint32_t timer_meas = 0UL;


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
  MX_SPI3_Init();
  MX_TIM10_Init();
  MX_USART1_UART_Init();
  MX_TIM11_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim10);
  HAL_TIM_Base_Start_IT(&htim11);
  BME280_Init(&hspi3, BME280_TEMPERATURE_16BIT, BME280_PRESSURE_ULTRALOWPOWER, BME280_HUMINIDITY_STANDARD, BME280_NORMALMODE);
  BME280_SetConfig(BME280_STANDBY_MS_10, BME280_FILTER_OFF);
  RingBuffer_Init(&ring_buffer, buffer, RINGBUF_SIZE);


  HAL_UART_Receive_IT(&huart1, &last_received_byte, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (HAL_GetTick() - timer_meas >= sample_time_meas_ms) {
		  struct bme280_pkt tmp;
		  BME280_ReadTemperatureAndPressureAndHuminidity(&tmp.temp, &tmp.pres, &tmp.hum);
		  RingBuffer_PutPkt(&ring_buffer, tmp);
		  timer_meas = HAL_GetTick();
	  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
