#include <usart.h>
#include <stddef.h>
#include <stdint.h>
#include "../utils/buffer.h"

#ifndef SRC_DRIVERS_UART_INTERRUPT_DRIVER_H_
#define SRC_DRIVERS_UART_INTERRUPT_DRIVER_H_



struct TransmitionState {
	uint8_t is_active;
};

void transmit_uart_interrupt(const uint8_t *buf, size_t size, struct TransmitionState* state);

void receive_uart_interrupt();

void receive_uart_interrupt_cb(struct ReadBuffer* buffer, struct TransmitionState* state);

#endif /* SRC_DRIVERS_UART_INTERRUPT_DRIVER_H_ */


