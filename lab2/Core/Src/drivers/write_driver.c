#include "write_driver.h"

#include <stdio.h>
#include <stdlib.h>

void HAL_GPIO_WritePin_Impl(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
  assert_param(IS_GPIO_PIN(GPIO_Pin));
  assert_param(IS_GPIO_PIN_ACTION(PinState));

  if(PinState != GPIO_PIN_RESET)
  {
    GPIOx->BSRR = GPIO_Pin;
  }
  else
  {
    GPIOx->BSRR = (uint32_t)GPIO_Pin << 16U;
  }
}


void write_color(const enum LightState color) {

	HAL_GPIO_WritePin_Impl(GPIOD, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_RESET);

    switch (color) {
        case RED:
        case RED_SHORT:
        	HAL_GPIO_WritePin_Impl(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
            break;
        case GREEN:
        case GREEN_SHORT:
        	HAL_GPIO_WritePin_Impl(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
            break;
        case YELLOW:
        case YELLOW_SHORT:
        	HAL_GPIO_WritePin_Impl(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
            break;
        case GREEN_FLUSHING:
        case GREEN_FLUSHING_SHORT:
            if ((HAL_GetTick() / 300) % 2 == 0) {
            	HAL_GPIO_WritePin_Impl(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
            } else {
            	HAL_GPIO_WritePin_Impl(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
            }
            break;
    }
}
