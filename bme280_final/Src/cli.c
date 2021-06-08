/*
 * cli.c
 *
 *  Created on: 8 kwi 2021
 *      Authors: Krzysztof Bera, Jan Kuliga, Michał Tomacha
 */

#include "cli.h"

UART_HandleTypeDef *CLI_huart;
uint8_t byte, rx_buf[RX_MAX_LENGTH];
uint16_t rx_len = 0;
cli_command *cmdlist;
const char *help;

int __io_putchar(int ch)
{
	HAL_UART_Transmit(CLI_huart, (uint8_t*)&ch, 1, 100);
	return ch;
}

void CLI_Init(UART_HandleTypeDef *huart, cli_command *commands)
{
	CLI_huart = huart;
	cmdlist = commands;
	CLI_Help_Construct(commands);
	HAL_UART_Receive_IT(CLI_huart, &byte, 1); //Starting UART in interrupt mode
}

void CLI_Command_Check(uint8_t *rx_buf, uint16_t rx_len)
{
	cli_command *commandlist = cmdlist;
	bool command_not_found = true;

	if(rx_len >= RX_MAX_LENGTH) //check for buffer overflow
	{
		printf("\r\ncommand too long\r\n");
		command_not_found = false;
	}
	else if(strcmp((const char*)rx_buf, "\\help") == 0)
	{
		printf("\r\n%s", help);
		command_not_found = false;
	}
	else
	{
		while(commandlist->command != NULL)
		{
			if(strcmp((const char*)rx_buf, commandlist->command) == 0)
			{
				commandlist->callback();
				command_not_found = false;
			}
			commandlist++;
		}
	}

	if(command_not_found) //command is not implemented
	{
		printf("\r\nwrong command\r\n");
	}

	command_not_found = true;
}

void CLI_Byte_Received_Callback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == CLI_huart->Instance)
	{
		// było (char)13
		if(byte == '\n') //if enter key is pressed
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

void CLI_Help_Construct(cli_command *commands)
{
	uint8_t len = 0;
	len += strlen("Help:\r\n");
	cli_command *commandlist = commands;
	while(commandlist->command != NULL)
	{
		len += strlen(commandlist->command);
		len += strlen("\r\n");
		commandlist++;
	}

	char* temp_help = (char*) malloc(len);
	strcpy(temp_help, "Help:\r\n");
	commandlist = commands;
	while(commandlist->command != NULL)
	{
		strcat(temp_help, commandlist->command);
		strcat(temp_help, "\r\n");
		commandlist++;
	}
	help = temp_help;
}
