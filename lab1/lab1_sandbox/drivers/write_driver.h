//
// Created by diana on 18.09.2024.
//

#ifndef WRITE_DRIVER_H
#define WRITE_DRIVER_H


enum Color {
    RED = 1,
    GREEN = 2,
    YELLOW = 3,
    GREEN_FLUSHING = 4
};

void write_color(enum Color color);

#endif //WRITE_DRIVER_H
