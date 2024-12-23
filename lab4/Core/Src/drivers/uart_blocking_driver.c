/*
 * uart_interrupt_driver.c
 *
 *  Created on: Oct 31, 2024
 *      Author: diana
 */

#include "uart_blocking_driver.h"

#define TIMEOUT_DURATION 100

static uint8_t received_data;
static uint8_t* rn = (uint8_t*) "\n\r";
static uint8_t received_buffer[2] = "\0\0";

void transmit_uart_blocking(const uint8_t *buf, size_t size) {
	HAL_UART_Transmit(&huart6, buf, size, TIMEOUT_DURATION);
}

uint8_t receive_uart_blocking(struct ReadBuffer* buffer) {
  if (HAL_OK == HAL_UART_Receive(&huart6, &received_data, sizeof(uint8_t), TIMEOUT_DURATION)) {
	  received_buffer[0] = received_data;
	  buffer->content = received_buffer;
	  if (received_data == '\r') {
		  buffer->content = rn;
	  }
	  transmit_uart_blocking(buffer->content, sizeof(uint16_t));
	  return 1;
  }
  return 0;
}
