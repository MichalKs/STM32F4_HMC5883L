/**
 * @file: 	main.c
 * @brief:	HD44780 LCD test project.
 * redirected to USART.
 * @date: 	9 kwi 2014
 * @author: Michal Ksiezopolski
 * 
 *
 * @verbatim
 * Copyright (c) 2014 Michal Ksiezopolski.
 * All rights reserved. This program and the 
 * accompanying materials are made available 
 * under the terms of the GNU Public License 
 * v3.0 which accompanies this distribution, 
 * and is available at 
 * http://www.gnu.org/licenses/gpl.html
 * @endverbatim
 */

#include <stm32f4xx.h>
#include <stdio.h>

#include "timers.h"
#include "led.h"
#include "uart.h"
#include "hmc5883l.h"

#define SYSTICK_FREQ 1000 ///< Frequency of the SysTick.

void softTimerCallback(void);

/**
 * Main function
 * @return Nothing
 */
int main(void) {

	UART2_Init(); // Initialize USART2 (for printf)
	TIMER_Init(SYSTICK_FREQ); // Initialize timer

	LED_TypeDef led;
	led.nr=LED0;
	led.gpio=GPIOD;
	led.pin=12;
	led.clk=RCC_AHB1Periph_GPIOD;

	LED_Add(&led); // Add an LED

	printf("Starting program\r\n"); // Print a string to UART2

	int8_t timerID = TIMER_AddSoftTimer(1000,softTimerCallback);
	TIMER_StartSoftTimer(timerID);

	HMC5883L_Init();

	uint8_t regVal = HMC5883L_Read(0x0a);
	printf("Id A %02x\r\n", regVal);

	regVal = HMC5883L_Read(0x0b);
	printf("Id B %02x\r\n", regVal);

	regVal = HMC5883L_Read(0x0c);
	printf("Id C %02x\r\n", regVal);

	HMC5883L_Write(0x02, 0x00); // Continuous mode


	while (1) {

		TIMER_SoftTimersUpdate();

	}

	return 0;
}

void softTimerCallback(void) {

	uint16_t x, y, z;

	LED_Toggle(LED0); // Toggle LED
	//printf("Test string sent from STM32F4!!!\r\n"); // Print test string
	x = HMC5883L_Read(0x03) << 8;
	x |= HMC5883L_Read(0x04);

	y = HMC5883L_Read(0x07) << 8;
	y |= HMC5883L_Read(0x08);

	z = HMC5883L_Read(0x05) << 8;
	z |= HMC5883L_Read(0x06);

	int8_t x_s = (int8_t) x;
	int8_t y_s = (int8_t) y;
	int8_t z_s = (int8_t) z;

	printf("x = %d\r\n",x_s);
	printf("y = %d\r\n",y_s);
	printf("z = %d\r\n",z_s);
}




