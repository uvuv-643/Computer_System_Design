#include "read_driver.h"

int read_button() {
    GPIO_PinState pin_info = !HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);
    return pin_info;
}
