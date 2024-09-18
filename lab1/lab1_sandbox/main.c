#include <stdio.h>
#include "constants.h"
#include "drivers/write_driver.h"
#include <Windows.h>
#include "drivers/read_driver.h"



/*

 FINITE STATE MACHINE FOR TRAFFIC LIGHT ON STM32

       +------+            +------+
       |      |            |      |
   +---+  G   +------------>  GF  +---+
   |   |      <------+     |      |   |
   |   +--^---+      |     +---+--+   |
   |      |          |         |      |
   |      |          |         |      |
   |   +--+---+      |     +---v--+   |
   |   |      |      |     |      |   |
+--+---+  Y   |      |     |  R   +---+--+
|  |   |      |<-----+-----+      |   |  |
|  |   +------+      |     +------+   |  |
|  |                 |                |  |
|  |                 |                |  |
|  |                 |                |  |
|  |   +------+      |     +------+   |  |
|  |   |      |      |     |      |   |  |
|  |   |  GS  |      |     | GFS  |   |  |
|  +--->      |      |     |      <---+  |
|      +------+      |     +------+      |
|                    |                   |
|                    |                   |
|      +------+      |     +------+      |
|      |      |      |     |      |      |
+------>  YS  |      +-----+  RS  |<-----+
       |      |            |      |
       +------+            +------+

 */

enum LightState {
    GREEN,
    YELLOW,
    RED,
    GREEN_FLUSHING,
    GREEN_SHORT,
    YELLOW_SHORT,
    RED_SHORT,
    GREEN_FLUSHING_SHORT
};


enum LightState state = GREEN;

int tick = 0;
int button_tick = 0;

int get_time_of_color(enum LightState current_state) {
    switch (current_state) {
        case YELLOW_SHORT:
        case YELLOW:
            return YELLOW_TIME;
        case GREEN:
        case GREEN_SHORT:
            return GREEN_TIME;
        case GREEN_FLUSHING:
        case GREEN_FLUSHING_SHORT:
            return GREEN_FLUSHING_TIME;
        case RED:
            return RED_TIME;
        case RED_SHORT:
            return RED_TIME / 4.0;
    }
}

int ready_to_switch() {
    return (tick - button_tick) > get_time_of_color(state);
}

void switch_to_button_with_timer_flush(enum LightState new_state) {
    state = new_state;
    button_tick = tick;
}

void switch_to_button(enum LightState new_state) {
    state = new_state;
}

int main() {

    while (1) {
        Sleep(10);
        write_color(GREEN_FLUSHING_STM32);
        tick += 1;
        switch (state) {
            case GREEN:
                if (read_button() == 1) switch_to_button(GREEN_SHORT);
                else if (ready_to_switch()) switch_to_button_with_timer_flush(GREEN_FLUSHING_SHORT);
                break;
            case GREEN_FLUSHING:
                if (read_button() == 1) switch_to_button(GREEN_FLUSHING_SHORT);
                else if (ready_to_switch()) switch_to_button_with_timer_flush(RED);
                break;
            case RED:
                if (read_button() == 1) switch_to_button(RED_SHORT);
                else if (ready_to_switch()) switch_to_button_with_timer_flush(YELLOW);
                break;
            case YELLOW:
                if (read_button() == 1) switch_to_button(YELLOW_SHORT);
                else if (ready_to_switch()) switch_to_button_with_timer_flush(GREEN);
                break;
            case GREEN_SHORT:
                if (ready_to_switch()) switch_to_button_with_timer_flush(GREEN_FLUSHING_SHORT);
                break;
            case GREEN_FLUSHING_SHORT:
                if (ready_to_switch()) switch_to_button_with_timer_flush(RED_SHORT);
                break;
            case RED_SHORT:
                if (ready_to_switch()) switch_to_button_with_timer_flush(GREEN);
                break;
            case YELLOW_SHORT:
                if (ready_to_switch()) switch_to_button_with_timer_flush(GREEN_SHORT);
                break;
        }
    }

    return 0;

}
