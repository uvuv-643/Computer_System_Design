/*
 * buffer.c
 *
 *  Created on: Oct 31, 2024
 *      Author: diana
 */

#ifndef SRC_UTILS_BUFFER_C_
#define SRC_UTILS_BUFFER_C_

#define QUEUE_SIZE 100
#include "buffer.h"

static uint8_t queue[QUEUE_SIZE];
static int insertion_index = 0;
static int queue_size = 0;

void insert_in_queue(uint8_t x) {
	if (queue_size >= QUEUE_SIZE) return;
	queue_size += 1;
	queue[insertion_index] = x;
	insertion_index = (insertion_index + 1) % QUEUE_SIZE;
}

uint8_t take_element_from_queue() {
	if (queue_size <= 0) return 0;
	int dequeue_index = (insertion_index - queue_size + QUEUE_SIZE) % QUEUE_SIZE;
	queue_size -= 1;
	return queue[dequeue_index];
}

uint8_t is_queue_empty() {
	return queue_size == 0;
}

uint8_t get_queue_size() {
	return queue_size;
}

uint8_t get_read_buffer(struct ReadBuffer* buffer) {
	uint8_t value = buffer->content[0];
	buffer->content = "\0\0";
	return value;
}


#endif /* SRC_UTILS_BUFFER_C_ */
