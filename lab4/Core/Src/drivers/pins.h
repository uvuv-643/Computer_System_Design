/*
 * pins.h
 *
 *  Created on: 20 дек. 2024 г.
 *      Author: diana
 */

#ifndef SRC_DRIVERS_PINS_H_
#define SRC_DRIVERS_PINS_H_

#define BUTTON_PIN GPIO_PIN_15

#include "stm32f4xx_hal.h"

uint8_t is_button_clicked(void);

#endif /* SRC_DRIVERS_PINS_H_ */
