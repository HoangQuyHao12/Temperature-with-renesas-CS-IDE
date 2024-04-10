/***********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
 * No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIESREGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
 * LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
 * ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability
 * of this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2011, 2016 Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * File Name    : r_main.c
 * Version      : CodeGenerator for RL78/G14 V2.04.03.03 [07 Mar 2016]
 * Device(s)    : R5F104PJ
 * Tool-Chain   : CA78K0R
 * Description  : This file implements main function.
 * Creation Date: 2024/03/25
 ***********************************************************************************************************************/

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_cgc.h"
#include "r_cg_port.h"
#include "r_cg_serial.h"
#include "r_cg_rtc.h"
#include "r_cg_it.h"
/* Start user code for include. Do not edit comment generated here */
#include "lcd.h"
#include "RDKRL78_spi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define CR 0x0D
#define LF 0x0A
#define DELAY_VALUE1 87
#define DELAY_VALUE2 0

#define LCD_SIZE 7
#define AVERAGE_LCD_SIZE 7
#define TX_BUFFER_SIZE 7
#define AVERAGE_TX_BUFFER_SIZE 16
#define TX_STRING_SIZE 7

#define MAX_TEMPERATURE 10
#define CRITICAL_TEMPERATURE 10

/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */

/*The status flag of UART data transmission.*/
extern uint8_t tx_flag;

/*Array to store temperature data for LCD display.*/
int8_t temperature_lcd[LCD_SIZE];

/*Similar to temperature_lcd, for average temperature data.*/
int8_t average_temperature_lcd[AVERAGE_LCD_SIZE];

/*Initialized variable to 0, possibly used for counting or tracking in temperature measurement for PC Connect.*/
int tempt = 0;

/*Variable for delay time, likely determining program delay*/
int delay_time = 100;

/*Variable initialized with 10, function not specified.*/
uint8_t a = 10;

/*Static array for storing UART transmission data.*/
static uint8_t tx_buffer[TX_BUFFER_SIZE];

/*Static array for storing average data for UART transmission.*/
static uint8_t average_tx_buffer[AVERAGE_TX_BUFFER_SIZE];

/*Static array for storing the character string "AVERAGE", likely for UART transmission or display.*/
static uint8_t tx_string[TX_STRING_SIZE] = "AVERAGE";

/*switch flag for Switch_TimeChange function.*/
unsigned char sw1_prev = 0;

/***********************************************************************************************************************
 * Function Name: Switch_TimeChange
 * Description: This function checks the status of a switch (P7.6) and changes the delay time accordingly.
 *              If the switch is pressed and released, it toggles between two predefined delay values.
 * Arguments: None
 * Return Value: None
 ***********************************************************************************************************************/
void Switch_TimeChange()
{
	if (P7 .6 == 0)
	{
		if (sw1_prev == 0)
		{
			/* Change delay_time based on its current value. */
			delay_time = (delay_time == DELAY_VALUE2) ? DELAY_VALUE1 : DELAY_VALUE2;
		}
		/* Update switch state. */
		sw1_prev = 1;
	}
	else
	{
		/* Reset switch state. */
		sw1_prev = 0;
	}
}
/* End user code. Do not edit comment generated here */
void R_MAIN_UserInit(void);

/***********************************************************************************************************************
 * Function Name: main
 * Description  : This function implements main function.
 * Arguments    : None
 * Return Value : None
 ***********************************************************************************************************************/
void main(void)
{
	R_MAIN_UserInit();

	/* SPI initialization for peripheral communication. */
	SPI_Init();

	/* LCD initialization for temperature display. */
	InitialiseLCD();

	/* UART communication initialization. */
	R_UART1_Start();

	/* Temperature sensor initialization. */
	TemperatureSensor_Init();

	/* Display "TEMPERATURE:" on LCD. */
	DisplayLCD(LCD_LINE1, "TEMPERATURE:");

	/* Display content of tx_string on LCD. */
	DisplayLCD(LCD_LINE5 + 2, &tx_string);

	while (1U)
	{
		/* Display current temperature on LCD. */
		Temperature_Display();

		/* Calculate and process average temperature. */
		Average_10tGetData();

		/* Increment tempt counter. */
		tempt += 1;

		/* Copy temperature data to tx_buffer for transmission. */
		strncpy((char *)tx_buffer, (const char *)temperature_lcd, sizeof(tx_buffer) - 2);

		/* Set carriage return and line feed characters. */
		tx_buffer[sizeof(tx_buffer) - 2] = CR;
		tx_buffer[sizeof(tx_buffer) - 1] = LF;
		average_tx_buffer[sizeof(average_tx_buffer) - 2] = CR;
		average_tx_buffer[sizeof(average_tx_buffer) - 1] = LF;

		/* Copy average temperature data to average_tx_buffer. */
		strncpy((char *)average_tx_buffer, (const char *)average_temperature_lcd, sizeof(average_tx_buffer) - 2);

		/* Concatenate tx_string to average_tx_buffer. */
		strcat((char *)average_tx_buffer, (const char *)tx_string);

		/* Send data if flag is clear and tempt is within range. */
		if (tx_flag == 0 && tempt < MAX_TEMPERATURE)
		{
			R_UART1_Send(&tx_buffer[0], sizeof(tx_buffer));
			tx_flag = 1;
		}

		/* Send average data if flag is clear and tempt reaches 12. */
		else if (tx_flag == 0 && tempt == CRITICAL_TEMPERATURE)
		{
			
		
			R_UART1_Send(&average_tx_buffer[0], sizeof(average_tx_buffer));
			tx_flag = 1;
			tempt = 0;
		}

		/* Display average temperature on LCD. */
		DisplayLCD(LCD_LINE7 + 3, &average_temperature_lcd[0]);

		/* Introduce a delay. */
		delay_ms(delay_time);
		P4.1 = ~ P4.1;

		/* Perform time change related action on switch. */
		Switch_TimeChange();
		
		
	}
	/* End user code. Do not edit comment generated here */
}
/***********************************************************************************************************************
 * Function Name: R_MAIN_UserInit
 * Description  : This function adds user code before implementing main function.
 * Arguments    : None
 * Return Value : None
 ***********************************************************************************************************************/
void R_MAIN_UserInit(void)
{
	/* Start user code. Do not edit comment generated here */
	EI();
	/* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
