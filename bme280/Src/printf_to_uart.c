/*
 * printf_to_uart.c
 *
 *  Created on: May 10, 2021
 *      Author: krzys
 */


#include "usart.h"

int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 1000);
	return ch;
}


