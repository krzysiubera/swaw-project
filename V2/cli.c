#include "cli.h"

UART_HandleTypeDef *CLI_huart;
uint8_t byte, rx_buf[RX_MAX_LENGTH], rx_len = 0, tx_buf[TX_MAX_LENGTH], tx_len = 0;

char ip[]="\r\n192.168.1.1\r\n";
char mac[]="\r\n1f:22:d4:12:54\r\n";
char help[]="\r\nshow ip\r\nshow mac\r\ndiode on\r\ndiode off\r\nget tick\n\r";

void CLI_Init(UART_HandleTypeDef *huart)
{
	CLI_huart = huart;
	HAL_UART_Receive_IT(CLI_huart, &byte, 1);
}

void CLI_Command_Check(uint8_t *rx_buf)
{
	if(strcmp((const char*)rx_buf, "show ip") == 0)
	{
		tx_len = sprintf((char*)tx_buf, ip);
	}
	else if(strcmp((const char*)rx_buf, "show mac") == 0)
	{
		tx_len = sprintf((char*)tx_buf, mac);
	}
	else if(strcmp((const char*)rx_buf, "get tick") == 0)
	{
		tx_len = sprintf((char*)tx_buf, "\r\n%lu us\r\n", HAL_GetTick());
	}
	else if(strcmp((const char*)rx_buf, "diode on") == 0)
	{
		tx_len = sprintf((char*)tx_buf, "\r\ndiode is on\r\n");
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
	}
	else if(strcmp((const char*)rx_buf, "diode off") == 0)
	{
		tx_len = sprintf((char*)tx_buf, "\r\ndiode is off\r\n");
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
	}
	else if(strcmp((const char*)rx_buf, "help") == 0)
	{
		tx_len = sprintf((char*)tx_buf, help);
	}
	else
	{
		tx_len = sprintf((char*)tx_buf, "\r\nwrong command\r\n");
	}
	HAL_UART_Transmit(CLI_huart, tx_buf, tx_len, 100);
}

void CLI_Byte_Received_Callback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == CLI_huart->Instance)
	{
		if(byte == (char)13)
		{
			CLI_Command_Check(rx_buf);
			rx_len = 0;
			memset(rx_buf, '\0',50);
		}
		else if(byte == (char)127)
		{
			rx_buf[--rx_len] = '\0';
		}
		else
		{
			rx_buf[rx_len++] = byte;
		}
		HAL_UART_Transmit(CLI_huart, &byte, 1, 100);
		HAL_UART_Receive_IT(CLI_huart, &byte, 1);
	}
}
