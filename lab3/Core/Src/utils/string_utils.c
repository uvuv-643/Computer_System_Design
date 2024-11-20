/*
 * string_utils.c
 *
 *  Created on: 8 нояб. 2024 г.
 *      Author: diana
 */

#include "./string_utils.h"

uint8_t prefix(const char *pre, const char *str)
{
	 return strncmp(pre, str, strlen(pre)) == 0;
}
