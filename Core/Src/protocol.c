/*
 * protocol.c
 *
 *  Created on: Jan 28, 2026
 *      Author: filippovm, acostal
 */


#include "gpio.h"
#include "tim.h"
#include <stdio.h>

static uint8_t PREAMBLE = 0x55;
static char MESSAGE[255 + 2];
static int curr_char = 0;
static int curr_bit = 7;
static int transmitting = 0; // false

void init_protocol(void) {

	//gpio and timer set in ioc file
	//set pin high when idle
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

	HAL_TIM_Base_Start_IT(&htim2); // start timer for period elapsed interrupt
	HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_2); // start for output compare
}

void transmit(uint8_t length, char* message) {
	MESSAGE[0] = PREAMBLE;
	MESSAGE[1] = length;

	// message
	for(int i = 0; i < length; ++i) {
		MESSAGE[i+2] = message[i];
	}

	curr_char = 0;
	curr_bit = 7;
	transmitting = (length + 2) * 8 * 2 + 1;

}
//update event, fires at 1 ms, (sr & 1)
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (transmitting > 1) {
		uint8_t c   = MESSAGE[curr_char];
		uint8_t bit = c & (1<<curr_bit);

		if (bit) {
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // set
		}
		else {
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // reset
		}

		curr_bit--;
		transmitting--;

		// move character
		if (curr_bit == -1) {
			curr_bit = 7;
			curr_char++;
		}
    } else if (transmitting == 1) {
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // set idle
    	transmitting--;
    }
}

//CC2, fires at 500 ms, (sr & (1<<2))
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (transmitting > 1) {
		uint8_t c   = MESSAGE[curr_char];
		uint8_t bit = c & (1 << curr_bit);

		if (bit) {
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // reset
		}
		else {
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // set
		}

		transmitting--;
    }
}

