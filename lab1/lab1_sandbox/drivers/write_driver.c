#include "write_driver.h"

#include <stdio.h>
#include <stdlib.h>

void write_color(const enum LightState color) {

    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);

    switch (color) {
        case RED:
        case RED_SHORT:
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
        break;
        case GREEN:
        case GREEN_SHORT:
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        break;
        case YELLOW:
        case YELLOW_SHORT:
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
        break;
        case GREEN_FLUSHING:
        case GREEN_FLUSHING_SHORT:
            if ((HAL_GetTick() / 300) % 2 == 0) {
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
            } else {
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
            }
        break;
    }
}
