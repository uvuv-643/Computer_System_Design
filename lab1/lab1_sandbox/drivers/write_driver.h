//
// Created by diana on 18.09.2024.
//

#ifndef WRITE_DRIVER_H
#define WRITE_DRIVER_H


enum Color {
    RED_STM32 = 1,
    GREEN_STM32 = 2,
    YELLOW_STM32 = 3,
    GREEN_FLUSHING_STM32 = 4
};

void write_color(enum Color color);

#endif //WRITE_DRIVER_H
