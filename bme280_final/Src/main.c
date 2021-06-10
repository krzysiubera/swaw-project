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
#include "BMPXX80.h"
#include "delays.h"
#include "ring_buffer.h"
#include "cli.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define RINGBUF_SIZE 180
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
RingBuffer ringbuf;
struct bme280_pkt buffer[RINGBUF_SIZE];
bool print_enable = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void GetTemp(void);
void GetHum(void);
void GetPress(void);
void GetAll(void);
void GetAvgTemp(void);
void GetAvgHum(void);
void GetAvgPress(void);
void GetAvgAll(void);
void GetAllContinuous(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	struct bme280_pkt tmp;
		if (htim->Instance == TIM11) {
			BME280_ReadTemperatureAndPressureAndHuminidity(&tmp.temp, &tmp.pres, &tmp.hum);
			RingBuffer_PutPkt(&ringbuf, tmp);

			if (print_enable) {
				printf("\r\nTemp: %.2f deg C, Hum: %.2f%%, Press: %.2f hPa", tmp.temp, tmp.hum, tmp.pres/((float)100.00));
			}
		}
}

//cli commands
cli_command cli_commands[] = {
	{ .command = "\\get cont all", .callback = GetAllContinuous},
	{ .command = "\\get temp", .callback = GetTemp},
	{ .command = "\\get hum", .callback = GetHum},
	{ .command = "\\get press", .callback = GetPress},
	{ .command = "\\get all", .callback = GetAll},
	{ .command = "\\get avg temp", .callback = GetAvgTemp},
	{ .command = "\\get avg hum", .callback = GetAvgHum},
	{ .command = "\\get avg press", .callback = GetAvgPress},
	{ .command = "\\get avg all", .callback = GetAvgAll},
	CLI_CMD_LIST_END
};

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)//byte received callback
{
	if(!print_enable)
	{
		CLI_Byte_Received_Callback(huart); //CLI callback
	}
	else if(print_enable)
	{
		print_enable = false;
		CLI_Init(&huart1, cli_commands);
		printf("\r\n");
	}
}
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
    if (RingBuffer_Init(&ringbuf, buffer, RINGBUF_SIZE) == false)
  	  return -1;
    CLI_Init(&huart1, cli_commands); // initialization of CLI using UART2
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

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
void GetTemp(void)
{
	float Temperature;
	Temperature = BME280_ReadTemperature();
	printf("\r\nTemperature: %.2f deg C\r\n", Temperature);
}
void GetHum(void)
{
	float Humidity;
	Humidity = BME280_ReadHuminidity();
	printf("\r\nHumidity: %.2f%%\r\n", Humidity);
}
void GetPress(void)
{
	uint32_t Pressure;
	Pressure = BME280_ReadPressure();
	printf("\r\nPressure: %.2f hPa\r\n", Pressure/(float) 100.0);
}
void GetAll(void)
{
	struct bme280_pkt meas;
	BME280_ReadTemperatureAndPressureAndHuminidity(&meas.temp, &meas.pres, &meas.hum);
	printf("\r\nTemp: %.2f deg C, Hum: %.2f%%, Press: %.2f hPa\r\n", meas.temp, meas.hum, meas.pres/(float) 100.0);

}
void GetAvgTemp(void)
{
	struct bme280_pkt meas;
	uint16_t count = 0;
	float Temperature = 0.0;

	while((RingBuffer_GetPkt(&ringbuf, &meas)) && (count < ringbuf.count))
	{
		count++;
		Temperature += meas.temp;
	}

	printf("\r\nAvg. Temperature: %.2f deg C\r\n", Temperature/count);
}
void GetAvgHum(void)
{
	struct bme280_pkt meas;
	uint16_t count = 0;
	float Humidity = 0.0;
	//__disable_irq();
	while((RingBuffer_GetPkt(&ringbuf, &meas)) && (count < ringbuf.count))
	{
		count++;
		Humidity += meas.hum;
	}
	//__enable_irq();
	printf("\r\nAvg. Humidity: %.2f%%\r\n", Humidity/count);
}
void GetAvgPress(void)
{
	struct bme280_pkt meas;
	uint16_t count = 0;
	uint32_t Pressure = 0.0;

	while((RingBuffer_GetPkt(&ringbuf, &meas)) && (count < ringbuf.count))
	{
		count++;
		Pressure += meas.pres;
	}

	printf("\r\nAvg. Pressure: %.2f hPa\r\n", Pressure/(((float) 100.0)*count));
}
void GetAvgAll(void)
{
	struct bme280_pkt meas;
	uint16_t count = 0;
	uint32_t Pressure = 0.0;
	float Humidity = 0.0;
	float Temperature = 0.0;

	while((RingBuffer_GetPkt(&ringbuf, &meas)) && (count < ringbuf.count))
	{
			count++;
			Pressure += meas.pres;
			Humidity += meas.hum;
			Temperature += meas.temp;
	}

	printf("\r\nTemp: %.2f deg C, Hum: %.2f%%, Press: %.2f hPa\r\n", Temperature/count, Humidity/count, Pressure/(((float) 100.0)*count));
}
void GetAllContinuous(void)
{
	print_enable = true;
}
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
