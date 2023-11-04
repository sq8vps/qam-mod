#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include <stdint.h>

#define SET(reg, val) (reg) |= (val)
#define RESET(reg, val) (reg) &= ~(val)

#define IS_NUMBER(x) (((x) >= '0') && ((x) <= '9'))

int64_t StrToInt(const char *str, uint16_t len);

#endif /* INC_COMMON_H_ */
