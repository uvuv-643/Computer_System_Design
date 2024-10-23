#include "read_driver.h"

int read_button() {

    return (GPIOC->IDR & GPIO_PIN_15) == (uint32_t)GPIO_PIN_RESET;

}
