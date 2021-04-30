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

#define RX_MAX_LENGTH 10
#define TX_MAX_LENGTH 200

/**
  * @brief Initialize CLI on chosen UART interface.
  * @param huart UART handle.
  * @retval none
  */
void CLI_Init(UART_HandleTypeDef *huart);

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
 
 #endif
