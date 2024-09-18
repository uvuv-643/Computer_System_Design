#include <stdio.h>
#include "constants.h"
#include "drivers/write_driver.h"
#include <Windows.h>

int main(void) {
    while (1) {
        write_color(GREEN_FLUSHING);
        Sleep(10);
    }

    write_color(GREEN);
    write_color(GREEN);
    write_color(GREEN);
    write_color(RED);
    return 0;
}
