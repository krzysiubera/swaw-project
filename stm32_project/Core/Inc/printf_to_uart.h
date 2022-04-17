/*
 * printf_to_uart.h
 *
 *  Created on: 5 Apr 2022
 *      Author: krzysiu
 */

#ifndef INC_PRINTF_TO_UART_H_
#define INC_PRINTF_TO_UART_H_

#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "usart.h"

int __io_putchar(int ch) {
	if (ch == '\n') {
		uint8_t ch2 = '\r';
		HAL_UART_Transmit(&huart1, &ch2, 1, HAL_MAX_DELAY);
	}
	HAL_UART_Transmit(&huart1, (uint8_t *) &ch, 1, HAL_MAX_DELAY);
	return 1;
}

#endif /* INC_PRINTF_TO_UART_H_ */
