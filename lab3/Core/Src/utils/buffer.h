/*
 * buffer.h
 *
 *  Created on: Oct 31, 2024
 *      Author: diana
 */

#ifndef SRC_UTILS_BUFFER_H_
#define SRC_UTILS_BUFFER_H_

#include <stdint.h>

struct ReadBuffer {
	uint8_t* content;
};

void insert_in_queue(uint8_t x);
uint8_t take_element_from_queue();
uint8_t is_queue_empty();

uint8_t get_read_buffer(struct ReadBuffer* buffer);

#endif /* SRC_UTILS_BUFFER_H_ */
