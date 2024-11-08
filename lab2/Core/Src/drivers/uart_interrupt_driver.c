/*
 * uart_interrupt_driver.c
 *
 *  Created on: Oct 31, 2024
 *      Author: diana
 */
#include "../utils/buffer.h"
#include "./uart_interrupt_driver.h"
#include "./write_driver.h"


static uint8_t received_data;
static uint8_t* rn = (uint8_t*) "\n\r";
static uint8_t received_buffer[2] = "\0\0";

void transmit_uart_interrupt(const uint8_t *buf, size_t size, struct TransmitionState* state) {
	if (state->is_active) {
		for (size_t i = 0; i < size / sizeof(uint8_t); i++) {
			insert_in_queue(buf[i]);
		}
	} else {
		HAL_UART_Transmit_IT(&huart6, buf, size);
		state->is_active = 1;
	}
}

void receive_uart_interrupt() {
	HAL_UART_Receive_IT(&huart6, &received_data, sizeof(uint8_t));
}

void receive_uart_interrupt_cb(struct ReadBuffer* buffer, struct TransmitionState* state) {
	received_buffer[0] = received_data;
	buffer->content = received_buffer;
	if (received_data == '\r') {
		buffer->content = rn;
	}
	transmit_uart_interrupt(buffer->content, sizeof(uint16_t), state);
}
