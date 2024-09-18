#include "write_driver.h"

#include <stdio.h>
#include <stdlib.h>

int tick = 0;

void write_color(const enum Color color) {
    tick += 1;

    system("cls");

    // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
    // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);

    switch (color) {
        case RED:
            printf("R");
        // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
            break;
        case GREEN:
            printf("G");
        // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
            break;
        case YELLOW:
            printf("Y");
        // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
            break;
        case GREEN_FLUSHING:
            if ((tick / 50) % 2 == 0) {
                printf("G");
                // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
            } else {
                printf("X");
                // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
            }
            break;
    }
}
