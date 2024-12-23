/* USER CODE BEGIN Header */

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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "constants.h"
#include "drivers/pins.h"
#include "drivers/uart_blocking_driver.h"
#include "drivers/uart_interrupt_driver.h"
#include "utils/buffer.h"
#include "utils/string_utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define KB_I2C_ADDRESS (0xE2)
#define KB_INPUT_REG (0x0)
#define KB_OUTPUT_REG (0x1)
#define KB_CONFIG_REG (0x3)
#define KB_KEY_DEBOUNCE_TIME (50)

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

struct TransmitionState irq_state = {
	 .is_active = 0
};

struct ReadBuffer read_buffer = {
	.content = (uint8_t*) "\0"
};

float calculate_note_freq(uint8_t note, uint8_t octave) {
	float factor = 1.0 / 16;
	for (int i = 1; i < octave; i++) {
		factor *= 2;
	}
	float note_freq = 0;
	if (note == 1) note_freq = NOTE_DO;
	else if (note == 2) note_freq = NOTE_RE;
	else if (note == 3) note_freq = NOTE_MI;
	else if (note == 4) note_freq = NOTE_FA;
	else if (note == 5) note_freq = NOTE_SO;
	else if (note == 6) note_freq = NOTE_LA;
	else if (note == 7) note_freq = NOTE_SI;
	return factor * note_freq;
}


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



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	receive_uart_interrupt_cb(&read_buffer, &irq_state);
}


// business logic starts here

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

static uint8_t note = 1;
static uint8_t octave = 5;
static uint32_t playtime = 1000;
static uint32_t notes_played_ms = 0;
static uint8_t notes_count = 1;


void play_note() {

	if (note == 0 || octave == 0) {
		htim1.Instance->CCR1 = 0;
	} else {
		htim1.Instance->ARR = 120 * 1000 * 1000 / (calculate_note_freq(note, octave) * htim1.Instance->PSC) - 1;
		htim1.Instance->CCR1 = htim1.Instance->ARR >> 1;
	}
}

void change_note(uint8_t new_note) {
	note = new_note;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM6) {
		notes_played_ms += htim->Instance->PSC + 1;

		if (notes_count == 1) {
			if (notes_played_ms > playtime) {
				change_note(0);
			}
		} else if (notes_count == 7) {
			if (notes_played_ms / playtime >= 7) {
				change_note(0);
			} else {
				change_note(1 + notes_played_ms / playtime);
			}
		}
		play_note();
	}
}

void increase_octave() {
	print("\n");
	if (octave < 9) {
		octave += 1;
		char sprintf_buffer[100];
		sprintf(sprintf_buffer, "Octave number increased to %d.\n", octave);
		print(sprintf_buffer);
	} else {
		print("Octave number cannot be greater than 9.\n");
	}
}

void decrease_octave() {
	print("\n");
	if (octave > 1) {
		octave -= 1;
		char sprintf_buffer[100];
		sprintf(sprintf_buffer, "Octave number decreased to %d.\n", octave);
		print(sprintf_buffer);
	} else {
		print("Octave number cannot be less than 1.\n");
	}
}

void increase_playtime() {
	print("\n");
	char sprintf_buffer[100];
	if (playtime < MAX_TIME) {
		playtime += 100;
		sprintf(sprintf_buffer, "Playback time increased to %d ms.\n", (int)playtime);
	} else {
		sprintf(sprintf_buffer, "Playback time cannot exceed %d ms.\n", MAX_TIME);
	}
	print(sprintf_buffer);
}

void decrease_playtime() {
	print("\n");
	char sprintf_buffer[100];
		if (playtime > MIN_TIME) {
			playtime -= 100;
			sprintf(sprintf_buffer, "Playback time decreased to %d ms.\n", (int)playtime);
		} else {
			sprintf(sprintf_buffer, "Playback time cannot be less than %d ms.\n", MIN_TIME);
		}
		print(sprintf_buffer);
}

void play_notes() {
	print("Playing notes in order\n");
	notes_count = 7;
	notes_played_ms = 0;
}



uint32_t last_pressing_time = 0;
int8_t last_pressed = -1;

int rows[4] = {0, 0, 0, 0};
int cols[3] = {0, 0, 0};

int get_pressed_btn_index(){
	const uint32_t t = HAL_GetTick();
	if (t - last_pressing_time < KB_KEY_DEBOUNCE_TIME) return -1;

	int8_t index = -1;
	uint8_t read_buffer = ~0;
	uint8_t default_config = ~0;
	uint8_t tmp = 0;
	int8_t column = -1;
	int8_t was_pressed_on_row = 0;

	HAL_I2C_Mem_Write(&hi2c1, KB_I2C_ADDRESS, KB_OUTPUT_REG, 1, &tmp, 1, 100);
	HAL_Delay(5);
	for (int row = 0; row < 4; row++) {
		uint8_t buf = ~((uint8_t) (1 << row));
		HAL_I2C_Mem_Write(&hi2c1, KB_I2C_ADDRESS, KB_CONFIG_REG, 1, &buf, 1, 100);
		HAL_Delay(5);
		HAL_I2C_Mem_Read(&hi2c1, KB_I2C_ADDRESS, KB_INPUT_REG, 1, &read_buffer, 1, 100);
		column = -1;
		switch (read_buffer >> 4) {
			case 6: column = 0; break;
			case 5: column = 1; break;
			case 3: column = 2; break;
		}
		if (column != -1) {
			if (!was_pressed_on_row) {
				index = row * 3 + column + 1;
			} else {
				index = -1;
			}
			rows[row] += 1;
			cols[column] += 1;
			was_pressed_on_row += 1;
		}
		HAL_Delay(5);
	}

	if (index == last_pressed) {
		return -1;
	}
	last_pressed = index;

	int s1 = 0; int s2 = 0;
	for (int i = 0; i < 4; i++) {
		s1 += rows[i];
		rows[i] = 0;
	}
	for (int i = 0; i < 3; i++) {
		s2 += cols[i];
		cols[i] = 0;
	}

	if (s1 != 1 && s2 != 1) {
		return -1;
	}
	last_pressing_time = t;

	return index;
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
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART6_UART_Init();
  MX_TIM6_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

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

	  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);


  static uint8_t received_buffer_empty[2] = "\0\0";

  // 0 - print, not execute
  // 1 - print and execute
  int kb_mode = 0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  int command = get_pressed_btn_index();

	  if (command != -1 && kb_mode) {
		char sprintf_buffer[100];
		sprintf(sprintf_buffer, "Pressed button on matrix keyboard: %d\n", command);
		print(sprintf_buffer);
	  }

	  int is_button = is_button_clicked();

	  if (is_button) {
		  kb_mode = !kb_mode;
		  if (kb_mode) {
			  print("Enabled debug mode.\n");
		  } else {
			  print("Disabled debug mode.\n");
		  }
	  }

	  if (kb_mode) continue;

	  	if (command >= 1 && command <= 7) {
			notes_count = 1;
			notes_played_ms = 0;
			change_note(command);

			char sprintf_buffer[100];
			sprintf(sprintf_buffer, "\nPlaying note #%d on octave %d in %dms.\n", note, octave, playtime);
			print(sprintf_buffer);

		} else if (command == 8) {
			increase_octave();
		} else if (command == 9) {
			decrease_octave();
		} else if (command == 10) {
			increase_playtime();
		} else if (command == 11) {
			decrease_playtime();
		} else if (command == 12) {
			play_notes();
		}
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 144;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
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
