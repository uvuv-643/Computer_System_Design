/* USER CODE BEGIN Header */

#define COMMAND_MAX_SIZE 953

#include "constants.h"
#include "drivers/write_driver.h"
#include "drivers/read_driver.h"
#include "drivers/uart_blocking_driver.h"
#include "drivers/uart_interrupt_driver.h"
#include "utils/buffer.h"
#include "utils/string_utils.h"
#include "states.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static uint8_t received_buffer_empty[2] = "\0\0";


enum LightState state = GREEN;
static char* enumStrings[] = {
		"GREEN",
		"YELLOW",
		"RED",
		"GREEN BLINKING",
		"GREEN",
		"YELLOW",
		"RED",
		"GREEN BLINKING",
};

int button_tick = 0;


void switch_to_button_with_timer_flush(enum LightState new_state) {
    state = new_state;
    button_tick = HAL_GetTick();
}

void switch_to_button(enum LightState new_state) {
    state = new_state;
}

struct TransmitionState irq_state = {
	 .is_active = 0
};

struct ReadBuffer read_buffer = {
	.content = (uint8_t*) "\0"
};


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  uint8_t temp_buffer[1000];
  int string_length = 0;
  while (!is_queue_empty()) {
	  temp_buffer[string_length++] = take_element_from_queue();
  }
  if (string_length > 0) {
	  HAL_UART_Transmit_IT(&huart6, temp_buffer, string_length);
  } else {
	  irq_state.is_active = 0;
  }
}

char last_entered_command[COMMAND_MAX_SIZE];
char str_buffer[COMMAND_MAX_SIZE];
size_t command_tail_index = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	receive_uart_interrupt_cb(&read_buffer, &irq_state);
}


// business logic starts here

/*
 * set mode 1 или set mode 2 – установить режим работы светофора, когда
 * обрабатываются или игнорируются нажатия кнопки;
 */
uint8_t button_mode = 1;

/*
 * set timeout X – установить тайм-аут (X – длина периода в секундах);
 */
uint32_t timeout = RED_TIME / 1000;

/*
 * set interrupts on или set interrupts off – включить или выключить прерывания.
 */
uint8_t interrupts_mode = 1;

void print(char* content) {
	size_t length = strlen(content);
	if (interrupts_mode) {
		transmit_uart_interrupt((uint8_t *) content, length, &irq_state);
	} else {
		transmit_uart_blocking((uint8_t *) content, length);
	}
}

uint8_t read() {
	if (interrupts_mode) {
		receive_uart_interrupt();
		return 1;
	} else {
		return receive_uart_blocking(&read_buffer);
	}
}

void debug_command_info(char* command) {
	print("command `");
	print(command);
	print("` executed. \n");
}

void execute_command(char* command) {

	if (strcmp(command, "?") == 0) {
		debug_command_info(command);
		char buffer[10];
		print("State: ");
		print(enumStrings[state]);
		print("\n");
		print("Mode: ");
		sprintf(buffer, "%d", button_mode);
		print(buffer);
		print("\n");
		print("Timeout: ");
		sprintf(buffer, "%d", (int)timeout);
		print(buffer);
		print("\n");
		print("Interrupt / Polling: ");
		print(interrupts_mode ? "I" : "P");
		print("\n");
	} else if (strcmp(command, "set interrupts on") == 0) {
		debug_command_info(command);
		interrupts_mode = 1;
	} else if (strcmp(command, "set interrupts off") == 0) {
		debug_command_info(command);
		interrupts_mode = 0;
	} else if (strcmp(command, "set mode 1") == 0) {
		debug_command_info(command);
		button_mode = 1;
	} else if (strcmp(command, "set mode 2") == 0) {
		debug_command_info(command);
		button_mode = 2;
	} else if (prefix("set timeout ", command)) {
		char timeout_number_as_str[10];
		strncpy(timeout_number_as_str, &command[12], 10);
		for (size_t i = 0; i < strlen(timeout_number_as_str); i++) {
			if (timeout_number_as_str[i] != '\0' && (timeout_number_as_str[i] < '0' || timeout_number_as_str[i] > '9')) {
				print("timeout must be a positive number less than 1000 seconds");
				print("\n");
				return;
			}
		}
		int timeout_number = atoi(timeout_number_as_str);
		if (timeout_number >= 0 && timeout_number < 1000) {
			debug_command_info(command);
			timeout = timeout_number;
		} else {
			print("timeout must be a positive number less than 1000 seconds");
			print("\n");
		}
	} else {
		print("cannot recognize this command: ");
		print(command);
		print("\n");
	}
}

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
            return timeout * 1000;
        case RED_SHORT:
            return timeout * 1000 / 4.0;
    }
    return timeout * 1000;
}

int ready_to_switch() {
    return (HAL_GetTick() - button_tick) > get_time_of_color(state);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */


  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

	  GPIO_InitStruct.Pin = GPIO_PIN_15;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	__disable_irq();
	uint8_t received = read();
	if (received) {
		if (read_buffer.content[0] != '\0') {
			if (strcmp((char*)read_buffer.content, "\n\r") == 0) {
				last_entered_command[command_tail_index] = '\0';
				strcpy(str_buffer, last_entered_command);
				execute_command(str_buffer);
				last_entered_command[0] = '\0';
				command_tail_index = 0;
			} else {
				if (read_buffer.content[0] == 127) {
					if (command_tail_index > 0) {
						command_tail_index--;
					}
				} else {
					last_entered_command[command_tail_index++] = read_buffer.content[0];
				}
			}
			read_buffer.content = received_buffer_empty;
		}
	}

	__enable_irq();

	int is_button_pressed = read_button() && button_mode == 1;

	switch (state) {
	 case GREEN:
		 if (ready_to_switch()) switch_to_button_with_timer_flush(GREEN_FLUSHING);
		 break;
	 case GREEN_FLUSHING:
		 if (is_button_pressed) switch_to_button(GREEN_FLUSHING_SHORT);
		 else if (ready_to_switch()) switch_to_button_with_timer_flush(YELLOW);
		 break;
	 case RED:
		 if (is_button_pressed) switch_to_button(RED_SHORT);
		 else if (ready_to_switch()) switch_to_button_with_timer_flush(GREEN);
		 break;
	 case YELLOW:
		 if (is_button_pressed) switch_to_button(YELLOW_SHORT);
		 else if (ready_to_switch()) switch_to_button_with_timer_flush(RED);
		 break;
	 case GREEN_SHORT:
		 if (ready_to_switch()) switch_to_button_with_timer_flush(GREEN_FLUSHING_SHORT);
		 break;
	 case GREEN_FLUSHING_SHORT:
		 if (ready_to_switch()) switch_to_button_with_timer_flush(YELLOW_SHORT);
		 break;
	 case RED_SHORT:
		 if (ready_to_switch()) switch_to_button_with_timer_flush(GREEN);
		 break;
	 case YELLOW_SHORT:
		 if (ready_to_switch()) switch_to_button_with_timer_flush(RED_SHORT);
		 break;
	}

	write_color(state);


  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
