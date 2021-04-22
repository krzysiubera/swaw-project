#ifndef _CLI_H_
#define _CLI_H_

#include "usart.h"
#include "string.h"
#include "stdio.h"

#define RX_MAX_LENGTH 200
#define TX_MAX_LENGTH 200

void CLI_Init(UART_HandleTypeDef *huart);

void CLI_Command_Check(uint8_t *rx_buf);

void CLI_Byte_Received_Callback(UART_HandleTypeDef *huart);
 
 #endif
