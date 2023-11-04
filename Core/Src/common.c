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
		else
			return 0;
	}
	return tmp;
}
