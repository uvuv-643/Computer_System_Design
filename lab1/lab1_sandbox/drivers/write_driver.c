//
// Created by diana on 18.09.2024.
//

#include "write_driver.h"

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

int tick = 0;

void write_color(const enum Color color) {
    tick += 1;
    system("cls");
    switch (color) {
        case RED:
            printf("R");
            break;
        case GREEN:
            printf("1G");
            break;
        case YELLOW:
            printf("1Y");
            break;
        case GREEN_FLUSHING:
            if ((tick / 50) % 2 == 0) {
                printf("G");
            } else {
                printf("X");
            }
            break;
    }
}