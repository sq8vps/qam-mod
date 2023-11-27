/*
Copyright 2020-2023 Piotr Wilkon
This file is part of QAM Modulator Controller.

QAM Modulator Controller is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

QAM Modulator Controller is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QAM Modulator Controller.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "common.h"
#include <string.h>

static uint64_t pow10i(uint16_t exp)
{
	if(exp == 0)
		return 1;
	uint64_t n = 1;
	while(exp--)
		n *= 10;
	return n;
}


int64_t StrToInt(const char *str, uint16_t len)
{
	if(len == 0)
		len = strlen(str);

	int64_t tmp = 0;
	for(int32_t i = (len - 1); i >= 0; i--)
	{
		if((i == 0) && (str[0] == '-'))
		{
			return -tmp;
		}
		else if(IS_NUMBER(str[i]))
			tmp += ((str[i] - '0') * pow10i(len - 1 - i));
		else if(' ' == str[i])
			break;
		else
			return 0;
	}
	return tmp;
}
