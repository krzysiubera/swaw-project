/*
 * cli.c
 *
 *  Created on: 8 kwi 2021
 *      Authors: Krzysztof Bera, Jan Kuliga, Michał Tomacha
 */

#include "cli.h"

UART_HandleTypeDef *CLI_huart;
uint8_t byte, rx_buf[RX_MAX_LENGTH], tx_buf[TX_MAX_LENGTH];
uint8_t ready_sign = '>';
uint16_t rx_len = 0, tx_len = 0;

char ip[]="\r\n192.168.1.1\r\n";
char mac[]="\r\n1f:22:d4:12:54:12\r\n";
char help[]="\r\nshow ip\r\nshow mac\r\ndiode on\r\ndiode off\r\nget tick\r\n"; //help message

void CLI_Init(UART_HandleTypeDef *huart)
{
	CLI_huart = huart;
	HAL_UART_Receive_IT(CLI_huart, &byte, 1); //Starting UART in interrupt mode
}

void CLI_Command_Check(uint8_t *rx_buf, uint16_t rx_len)
{
	if(rx_len >= RX_MAX_LENGTH) //check for buffer overflow
	{
		tx_len = sprintf((char*)tx_buf, "\r\ncommand too long\r\n");
	}
	else if(strcmp((const char*)rx_buf, "\\show ip") == 0) //execute command
	{
		tx_len = sprintf((char*)tx_buf, ip);
	}
	else if(strcmp((const char*)rx_buf, "\\show mac") == 0)
	{
		tx_len = sprintf((char*)tx_buf, mac);
	}
	else if(strcmp((const char*)rx_buf, "\\get tick") == 0)
	{
		tx_len = sprintf((char*)tx_buf, "\r\n%lu us\r\n", HAL_GetTick());
	}
	else if(strcmp((const char*)rx_buf, "\\diode on") == 0)
	{
		tx_len = sprintf((char*)tx_buf, "\r\ndiode is on\r\n");
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
	}
	else if(strcmp((const char*)rx_buf, "\\diode off") == 0)
	{
		tx_len = sprintf((char*)tx_buf, "\r\ndiode is off\r\n");
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
	}
	else if(strcmp((const char*)rx_buf, "\\help") == 0)
	{
		tx_len = sprintf((char*)tx_buf, help);
	}
	else //command is not implemented
	{
		tx_len = sprintf((char*)tx_buf, "\r\nwrong command\r\n");
	}
	HAL_UART_Transmit(CLI_huart, tx_buf, tx_len, 100); //send response
	HAL_UART_Transmit(CLI_huart, &ready_sign, 1, 100); //dislpay ready sign
}

void CLI_Byte_Received_Callback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == CLI_huart->Instance)
	{
		if(byte == (char)13) //if enter key is pressed
		{
			CLI_Command_Check(rx_buf, rx_len);
			rx_len = 0;
			memset(rx_buf, '\0', RX_MAX_LENGTH); //clearing receive buffer
		}
		else if(byte == (char)127) //if backspace key is pressed
		{
			if(rx_len != 0)
			{
				rx_buf[--rx_len] = '\0';
			}
		}
		else //save byte to buffer
		{
			if(rx_len < RX_MAX_LENGTH)
			{
				rx_buf[rx_len] = byte;
			}
			rx_len++;
		}
		HAL_UART_Transmit(CLI_huart, &byte, 1, 100); //echo
		HAL_UART_Receive_IT(CLI_huart, &byte, 1); //enable UART to receive next byte
	}
}
