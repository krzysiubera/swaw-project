/*
 * utils.c
 *
 *  Created on: Apr 17, 2022
 *      Author: krzysiu
 */

#include "utils.h"
#include "stdbool.h"
#include "string.h"

bool string_starts_with(const char* cli_command, const char* received_command_uart) {
    size_t len_cli_command = strlen(cli_command);
    size_t len_received_command_uart = strlen(received_command_uart);
    if (len_received_command_uart < len_cli_command) {
        return false;
    }
    return memcmp(cli_command, received_command_uart, len_cli_command) == 0;
}
