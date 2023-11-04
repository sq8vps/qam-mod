#ifndef TERMINAL_H_
#define TERMINAL_H_

#include "uart.h"
#include <stdint.h>

/**
 * @brief Send data to all available ports
 * @param mode Output mode/data type
 * @param *data Data buffer
 * @param size Data size
 */
void TermSendToAll(enum UartMode mode, uint8_t *data, uint16_t size);

/**
 * @brief Send signed number to all available ports
 * @param mode Output mode/data type
 * @param n Number to send
 */
void TermSendNumberToAll(enum UartMode mode, int32_t n);


/**
 * @brief Handle "special" terminal cases like backspace or local echo
 * @param *u UART structure
 * @attention Must be called for every received data
 */
void TermHandleSpecial(Uart *u);

/**
 * @brief Parse and process received data
 * @param *src UART structure
 */
void TermParse(Uart *src);

#endif /* DEBUG_H_ */
