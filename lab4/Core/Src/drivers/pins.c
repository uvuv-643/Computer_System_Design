/*
 * pins.c
 *
 *  Created on: 20 дек. 2024 г.
 *      Author: diana
 */
#include "stdio.h"
#include "pins.h"

#define BUTTON_CLICK_DELAY 200

static uint32_t prevClick = 0;
static GPIO_PinState prevState = GPIO_PIN_SET;

uint8_t is_button_clicked(void)
{
	uint32_t currentTick = HAL_GetTick();

	if (currentTick - prevClick < BUTTON_CLICK_DELAY) {
		return 0;
	}

	GPIO_PinState currentState = HAL_GPIO_ReadPin(GPIOC, BUTTON_PIN);

	if (currentState == GPIO_PIN_RESET && prevState == GPIO_PIN_SET)
	{
		prevClick = currentTick;
		prevState == GPIO_PIN_RESET;
		return 1;
	}

	if (currentState == GPIO_PIN_SET) {
		prevClick = currentTick;
		prevState == GPIO_PIN_SET;
	}

	return 0;

}
