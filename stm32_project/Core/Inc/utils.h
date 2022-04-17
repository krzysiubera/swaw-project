/*
 * utils.h
 *
 *  Created on: Apr 17, 2022
 *      Author: krzysiu
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include <stdbool.h>

bool string_starts_with(const char* cli_command, const char* received_command_uart);

#endif /* INC_UTILS_H_ */
