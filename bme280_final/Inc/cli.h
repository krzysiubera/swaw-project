/*
 * cli.h
 *
 *  Created on: 8 kwi 2021
 *      Authors: Krzysztof Bera, Jan Kuliga, Michał Tomacha
 */

#ifndef _CLI_H_
#define _CLI_H_

#include "usart.h"
#include "string.h"
#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"

#define RX_MAX_LENGTH 20
#define CLI_CMD_LIST_END       {NULL, NULL}

typedef void (*Callback)(void);

struct _cli_command {
        const char * command;
        Callback callback;
};

typedef struct _cli_command cli_command;

/**
  * @brief Initialize CLI on chosen UART interface.
  * @param huart UART handle.
  * @param commands table of commands.
  * @retval none
  */
void CLI_Init(UART_HandleTypeDef *huart, cli_command *commands);

/**
  * @brief Executes chosen command.
  * @note   When command is not implemented or too long displays adequate message
  * @param rx_buf Pointer to data buffer.
  * @param rx_len Length of received data.
  * @retval none
  */
void CLI_Command_Check(uint8_t *rx_buf, uint16_t rx_len);

/**
  * @brief Receive byte of data and save it to buffer.
  * @note   When When backspace is pressed removes last value form buffer. In this case enter button
  * 		is pressed calls CLI_Command_Check function.
  * @param huart UART handle.
  * @retval none
  */
void CLI_Byte_Received_Callback(UART_HandleTypeDef *huart);

/**
  * @brief Constructs help message.
  * @param commands Table of commands.
  * @retval none
  */
void CLI_Help_Construct(cli_command *commands);
 #endif
