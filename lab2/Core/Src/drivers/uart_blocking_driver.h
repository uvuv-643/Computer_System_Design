#include <usart.h>
#include <stddef.h>
#include "../utils/buffer.h"

#ifndef SRC_DRIVERS_UART_BLOCKING_DRIVER_H_
#define SRC_DRIVERS_UART_BLOCKING_DRIVER_H_

void transmit_uart_blocking(const uint8_t *buf, size_t size);

uint8_t receive_uart_blocking(struct ReadBuffer* buffer);

#endif /* SRC_DRIVERS_UART_BLOCKING_DRIVER_H_ */


