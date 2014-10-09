/**
 * @file: 	main.c
 * @brief:	LED test
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

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <timers.h>
#include <led.h>
#include <comm.h>
#include <keys.h>
#include <hmc5883l.h>
#include <hd44780.h>

#define SYSTICK_FREQ 1000 ///< Frequency of the SysTick set at 1kHz.
#define COMM_BAUD_RATE 115200UL ///< Baud rate for communication with PC

void softTimerCallback(void);

#define DEBUG

#ifdef DEBUG
#define print(str, args...) printf("MAIN--> "str"%s",##args,"\r")
#define println(str, args...) printf("MAIN--> "str"%s",##args,"\r\n")
#else
#define print(str, args...) (void)0
#define println(str, args...) (void)0
#endif


int main(void) {
	
  COMM_Init(COMM_BAUD_RATE);
  println("Starting program"); // Print a string to terminal

	TIMER_Init(SYSTICK_FREQ); // Initialize timer

	// Add a soft timer with callback running every 1000ms
	int8_t timerID = TIMER_AddSoftTimer(1000, softTimerCallback);
	TIMER_StartSoftTimer(timerID);

	LED_Init(LED0); // Add an LED
	LED_Init(LED1); // Add an LED
	LED_Init(LED2); // Add an LED
	LED_Init(LED3); // Add an LED
	LED_Init(LED5); // Add nonexising LED for test
	LED_ChangeState(LED5, LED_ON);

	KEYS_Init(); // initialize matrix keyboard
	HMC5883L_Init();

  uint8_t regVal = HMC5883L_Read(0x0a);
  println("Id A %02x", regVal);

  regVal = HMC5883L_Read(0x0b);
  println("Id B %02x", regVal);

  regVal = HMC5883L_Read(0x0c);
  println("Id C %02x", regVal);

  HMC5883L_Write(0x02, 0x00); // Continuous mode
  LCD_Init();
  LCD_Clear();

  uint8_t buf[255];
  uint8_t len;

  uint32_t softTimer = TIMER_GetTime(); // get start time for delay

	while (1) {

	  if (TIMER_DelayTimer(1000, softTimer)) {
	    LED_Toggle(LED3);
	    softTimer = TIMER_GetTime(); // get start time for delay
	  }

	  // check for new frames from PC
	  if (!COMM_GetFrame(buf, &len)) {
	    println("Got frame of length %d: %s", (int)len, (char*)buf);

	    // control LED0 from terminal
	    if (!strcmp((char*)buf, ":LED0 ON")) {
	      LED_ChangeState(LED0, LED_ON);
	    }
	    if (!strcmp((char*)buf, ":LED0 OFF")) {
	      LED_ChangeState(LED0, LED_OFF);
	    }
	  }

		TIMER_SoftTimersUpdate(); // run timers
		KEYS_Update(); // run keyboard
		LCD_Update();
	}
}
/**
 * @brief Callback function called on every soft timer overflow
 */
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

  int16_t x_s = (int16_t) x;
  int16_t y_s = (int16_t) y;
//  int16_t z_s = (int16_t) z;

//  printf("x = %d ",x_s);
//  printf("y = %d ",y_s);
//  printf("z = %d\r\n",z_s);


  double direction;

  if (y_s > 0) {
    direction = 90.0 - atan((double)x_s/(double)y_s)*180.0/M_PI;
  } else if (y_s < 0){
    direction = 270.0 - atan((double)x_s/(double)y_s)*180.0/M_PI;
  } else if (y_s == 0 && x_s < 0) {
    direction = 180.0;
  } else {
    direction = 0.0;
  }
  println("%.2f", direction);

  char buf[20];

  sprintf(buf, "%.2f", direction);
  LCD_Clear();
  LCD_Position(0,0);
  LCD_Puts("Dir: ");
  LCD_Puts(buf);
  LCD_Position(0,1);

  if (direction < 45 || direction >= 315) {
    LCD_Puts("North");
  } else if (direction >= 45 && direction < 135) {
    LCD_Puts("East");
  } else if (direction >= 135 && direction < 225) {
    LCD_Puts("South");
  } else if (direction >= 225 && direction < 315) {
    LCD_Puts("West");
  }

}
