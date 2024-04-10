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
* File Name    : r_cg_serial_user.c
* Version      : CodeGenerator for RL78/G14 V2.04.03.03 [07 Mar 2016]
* Device(s)    : R5F104PJ
* Tool-Chain   : CA78K0R
* Description  : This file implements device driver for Serial module.
* Creation Date: 2024/03/25
***********************************************************************************************************************/

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
#pragma interrupt INTST0 r_uart0_interrupt_send
#pragma interrupt INTSR0 r_uart0_interrupt_receive
#pragma interrupt INTST1 r_uart1_interrupt_send
#pragma interrupt INTSR1 r_uart1_interrupt_receive
#pragma interrupt INTCSI21 r_csi21_interrupt
#pragma interrupt INTIICA0 r_iica0_interrupt
/* Start user code for pragma. Do not edit comment generated here */
#define TEMPERATURE_ADDR 0x90
#define TEMPERATURE_CMD 0x03
#define BUFFER_SIZE 10
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_serial.h"
/* Start user code for include. Do not edit comment generated here */
#include "lcd.h"
#include "RDKRL78_spi.h"
#include <string.h>
#include <stdio.h>
#include "r_cg_it.h"

/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
extern volatile uint8_t * gp_uart0_tx_address;         /* uart0 send buffer address */
extern volatile uint16_t  g_uart0_tx_count;            /* uart0 send data number */
extern volatile uint8_t * gp_uart0_rx_address;         /* uart0 receive buffer address */
extern volatile uint16_t  g_uart0_rx_count;            /* uart0 receive data number */
extern volatile uint16_t  g_uart0_rx_length;           /* uart0 receive data length */
extern volatile uint8_t * gp_uart1_tx_address;         /* uart1 send buffer address */
extern volatile uint16_t  g_uart1_tx_count;            /* uart1 send data number */
extern volatile uint8_t * gp_uart1_rx_address;         /* uart1 receive buffer address */
extern volatile uint16_t  g_uart1_rx_count;            /* uart1 receive data number */
extern volatile uint16_t  g_uart1_rx_length;           /* uart1 receive data length */
extern volatile uint8_t * gp_csi21_rx_address;         /* csi21 receive buffer address */
extern volatile uint16_t  g_csi21_rx_length;           /* csi21 receive data length */
extern volatile uint16_t  g_csi21_rx_count;            /* csi21 receive data count */
extern volatile uint8_t * gp_csi21_tx_address;         /* csi21 send buffer address */
extern volatile uint16_t  g_csi21_send_length;         /* csi21 send data length */
extern volatile uint16_t  g_csi21_tx_count;            /* csi21 send data count */
extern volatile uint8_t   g_iica0_master_status_flag;  /* iica0 master flag */ 
extern volatile uint8_t   g_iica0_slave_status_flag;   /* iica0 slave flag */
extern volatile uint8_t * gp_iica0_rx_address;         /* iica0 receive buffer address */
extern volatile uint16_t  g_iica0_rx_cnt;              /* iica0 receive data length */
extern volatile uint16_t  g_iica0_rx_len;              /* iica0 receive data count */
extern volatile uint8_t * gp_iica0_tx_address;         /* iica0 send buffer address */
extern volatile uint16_t  g_iica0_tx_cnt;              /* iica0 send data count */
/* Start user code for global. Do not edit comment generated here */
volatile uint8_t G_CSI21_SendingData = 0;
volatile uint8_t G_CSI21_ReceivingData = 0;

/*This array is used as a buffer to store data for communication via the I2C (Inter-Integrated Circuit) protocol.*/
uint8_t i2cbuff[10];

/*This variable is of a custom data type.*/
Temperature_Data temperature_Reading;

/*used for storing formatted temperature data for display on an LCD screen.*/
extern int8_t temperature_lcd[];

/*used for storing formatted temperature data for display on an LCD screen.*/
extern int8_t average_temperature_lcd[];

/* This variable is likely used as a flag to indicate the status of a transmission process.*/
uint8_t tx_flag;

/*This variable is commonly used as a loop counter in iterative operations.*/
uint8_t i;

/*This array is used as a circular buffer to store the last 10 temperature readings.*/
uint16_t temperature_buffer[10];

/*********************************************************************************************************************
uint8_t temperature_buffer_index = 0;: This variable keeps track of the current index in the temperature_buffer array. 
It's used to determine where to store the next temperature reading and wraps around when it reaches the end of the buffer.
************************************************************************************************************************/
uint8_t temperature_buffer_index = 0;

/*These variables are likely used to calculate and store the sum and average of the temperature readings stored in temperature_buffer.*/
float temperature_sum;

/*These variables are likely used to calculate and store the sum and average of the temperature readings stored in temperature_buffer.*/
float temperature_average;

/*These variables are used to store the integer and decimal parts of a temperature value when calculating the average temperature for display. */
int intPart;

/*These variables are used to store the integer and decimal parts of a temperature value when calculating the average temperature for display. */
int decimalPart;

/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: r_uart0_interrupt_receive
* Description  : This function is INTSR0 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void r_uart0_interrupt_receive(void)
{
    volatile uint8_t rx_data;
    volatile uint8_t err_type;
    
    err_type = (uint8_t)(SSR01 & 0x0007U);
    SIR01 = (uint16_t)err_type;

    if (err_type != 0U)
    {
        r_uart0_callback_error(err_type);
    }
    
    rx_data = RXD0;

    if (g_uart0_rx_length > g_uart0_rx_count)
    {
        *gp_uart0_rx_address = rx_data;
        gp_uart0_rx_address++;
        g_uart0_rx_count++;

        if (g_uart0_rx_length == g_uart0_rx_count)
        {
            r_uart0_callback_receiveend();
        }
    }
    else
    {
        r_uart0_callback_softwareoverrun(rx_data);
    }
}

/***********************************************************************************************************************
* Function Name: r_uart0_interrupt_send
* Description  : This function is INTST0 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void r_uart0_interrupt_send(void)
{
    if (g_uart0_tx_count > 0U)
    {
        TXD0 = *gp_uart0_tx_address;
        gp_uart0_tx_address++;
        g_uart0_tx_count--;
    }
    else
    {
        r_uart0_callback_sendend();
    }
}

/***********************************************************************************************************************
* Function Name: r_uart0_callback_receiveend
* Description  : This function is a callback function when UART0 finishes reception.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_uart0_callback_receiveend(void)
{
    /* Start user code. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_uart0_callback_softwareoverrun
* Description  : This function is a callback function when UART0 receives an overflow data.
* Arguments    : rx_data -
*                    receive data
* Return Value : None
***********************************************************************************************************************/
static void r_uart0_callback_softwareoverrun(uint16_t rx_data)
{
    /* Start user code. Do not edit comment generated here */

    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_uart0_callback_sendend
* Description  : This function is a callback function when UART0 finishes transmission.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_uart0_callback_sendend(void)
{
    /* Start user code. Do not edit comment generated here */

    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_uart0_callback_error
* Description  : This function is a callback function when UART0 reception error occurs.
* Arguments    : err_type -
*                    error type value
* Return Value : None
***********************************************************************************************************************/
static void r_uart0_callback_error(uint8_t err_type)
{
    /* Start user code. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_uart1_interrupt_receive
* Description  : This function is INTSR1 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void r_uart1_interrupt_receive(void)
{
    volatile uint8_t rx_data;
    volatile uint8_t err_type;
    
    err_type = (uint8_t)(SSR03 & 0x0007U);
    SIR03 = (uint16_t)err_type;

    if (err_type != 0U)
    {
        r_uart1_callback_error(err_type);
    }
    
    rx_data = RXD1;

    if (g_uart1_rx_length > g_uart1_rx_count)
    {
        *gp_uart1_rx_address = rx_data;
        gp_uart1_rx_address++;
        g_uart1_rx_count++;

        if (g_uart1_rx_length == g_uart1_rx_count)
        {
            r_uart1_callback_receiveend();
        }
    }
    else
    {
        r_uart1_callback_softwareoverrun(rx_data);
    }
}

/***********************************************************************************************************************
* Function Name: r_uart1_interrupt_send
* Description  : This function is INTST1 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void r_uart1_interrupt_send(void)
{
    if (g_uart1_tx_count > 0U)
    {
        TXD1 = *gp_uart1_tx_address;
        gp_uart1_tx_address++;
        g_uart1_tx_count--;
    }
    else
    {
        r_uart1_callback_sendend();
    }
}

/***********************************************************************************************************************
* Function Name: r_uart1_callback_receiveend
* Description  : This function is a callback function when UART1 finishes reception.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_uart1_callback_receiveend(void)
{
    /* Start user code. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_uart1_callback_softwareoverrun
* Description  : This function is a callback function when UART1 receives an overflow data.
* Arguments    : rx_data -
*                    receive data
* Return Value : None
***********************************************************************************************************************/
static void r_uart1_callback_softwareoverrun(uint16_t rx_data)
{
    /* Start user code. Do not edit comment generated here */

    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_uart1_callback_sendend
* Description  : This function is a callback function when UART1 finishes transmission.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_uart1_callback_sendend(void)
{
    /* Start user code. Do not edit comment generated here */
    tx_flag = 0;
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_uart1_callback_error
* Description  : This function is a callback function when UART1 reception error occurs.
* Arguments    : err_type -
*                    error type value
* Return Value : None
***********************************************************************************************************************/
static void r_uart1_callback_error(uint8_t err_type)
{
    /* Start user code. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_csi21_interrupt
* Description  : This function is INTCSI21 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void r_csi21_interrupt(void)
{
    volatile uint8_t err_type;
    volatile uint8_t sio_dummy;

    err_type = (uint8_t)(SSR11 & _0001_SAU_OVERRUN_ERROR);
    SIR11 = (uint16_t)err_type;

    if (1U == err_type)
    {
        r_csi21_callback_error(err_type);    /* overrun error occurs */
    }
    else
    {
        if (g_csi21_tx_count > 0U)
        {
            if (0U != gp_csi21_rx_address)
            {
                *gp_csi21_rx_address = SIO21;
                gp_csi21_rx_address++;
            }
            else
            {
                sio_dummy = SIO21;
            }

            if (0U != gp_csi21_tx_address)
            {
                SIO21 = *gp_csi21_tx_address;
                gp_csi21_tx_address++;
            }
            else
            {
                SIO21 = 0xFFU;
            }

            g_csi21_tx_count--;
        }
        else 
        {
            if (0U == g_csi21_tx_count)
            {
                if (0U != gp_csi21_rx_address)
                {
                    *gp_csi21_rx_address = SIO21;
                }
                else
                {
                    sio_dummy = SIO21;
                }
            }

            r_csi21_callback_sendend();    /* complete send */
            r_csi21_callback_receiveend();    /* complete receive */
        }
    }
}

/***********************************************************************************************************************
* Function Name: r_csi21_callback_receiveend
* Description  : This function is a callback function when CSI21 finishes reception.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_csi21_callback_receiveend(void)
{
    /* Start user code. Do not edit comment generated here */
    G_CSI21_ReceivingData = 0;
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_csi21_callback_error
* Description  : This function is a callback function when CSI21 reception error occurs.
* Arguments    : err_type -
*                    error type value
* Return Value : None
***********************************************************************************************************************/
static void r_csi21_callback_error(uint8_t err_type)
{
    /* Start user code. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_csi21_callback_sendend
* Description  : This function is a callback function when CSI21 finishes transmission.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_csi21_callback_sendend(void)
{
    /* Start user code. Do not edit comment generated here */
    G_CSI21_SendingData = 0;
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_iica0_interrupt
* Description  : This function is INTIICA0 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void r_iica0_interrupt(void)
{
    if ((IICS0 & _80_IICA_STATUS_MASTER) == 0x80U)
    {
        iica0_master_handler();
    }
}

/***********************************************************************************************************************
* Function Name: iica0_master_handler
* Description  : This function is IICA0 master handler.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void iica0_master_handler(void)
{
    /* Control for communication */
    if ((0U == IICBSY0) && (g_iica0_tx_cnt != 0U))
    {
        r_iica0_callback_master_error(MD_SPT);
    }
    /* Control for sended address */
    else
    {
        if ((g_iica0_master_status_flag & _80_IICA_ADDRESS_COMPLETE) == 0U)
        {
            if (1U == ACKD0)
            {
                g_iica0_master_status_flag |= _80_IICA_ADDRESS_COMPLETE;
                
                if (1U == TRC0)
                {
                    WTIM0 = 1U;
                    
                    if (g_iica0_tx_cnt > 0U)
                    {
                        IICA0 = *gp_iica0_tx_address;
                        gp_iica0_tx_address++;
                        g_iica0_tx_cnt--;
                    }
                    else
                    {
                        r_iica0_callback_master_sendend();
                    }
                }
                else
                {
                    ACKE0 = 1U;
                    WTIM0 = 0U;
                    WREL0 = 1U;
                }
            }
            else
            {
                r_iica0_callback_master_error(MD_NACK);
            }
        }
        else
        {
            /* Master send control */
            if (1U == TRC0)
            {
                if ((0U == ACKD0) && (g_iica0_tx_cnt != 0U))
                {
                    r_iica0_callback_master_error(MD_NACK);
                }
                else
                {
                    if (g_iica0_tx_cnt > 0U)
                    {
                        IICA0 = *gp_iica0_tx_address;
                        gp_iica0_tx_address++;
                        g_iica0_tx_cnt--;
                    }
                    else
                    {
                        r_iica0_callback_master_sendend();
                    }
                }
            }
            /* Master receive control */
            else
            {
                if (g_iica0_rx_cnt < g_iica0_rx_len)
                {
                    *gp_iica0_rx_address = IICA0;
                    gp_iica0_rx_address++;
                    g_iica0_rx_cnt++;
                    
                    if (g_iica0_rx_cnt == g_iica0_rx_len)
                    {
                        ACKE0 = 0U;
                        WREL0 = 1U;
                        WTIM0 = 1U;
                    }
                    else
                    {
                        WREL0 = 1U;
                    }
                }
                else
                {
                    r_iica0_callback_master_receiveend();
                }
            }
        }
    }
}

/***********************************************************************************************************************
* Function Name: r_iica0_callback_master_error
* Description  : This function is a callback function when IICA0 master error occurs.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_iica0_callback_master_error(MD_STATUS flag)
{
    /* Start user code. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_iica0_callback_master_receiveend
* Description  : This function is a callback function when IICA0 finishes master reception.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_iica0_callback_master_receiveend(void)
{
    SPT0 = 1U;
    /* Start user code. Do not edit comment generated here */
    
    /*This line extracts the received temperature datafrom the i2cbuff array and stores it in the temperature_Reading structure*/
    temperature_Reading.temperature = (uint16_t)i2cbuff[0] | ((uint16_t)i2cbuff[1] << 8);
    
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_iica0_callback_master_sendend
* Description  : This function is a callback function when IICA0 finishes master transmission.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_iica0_callback_master_sendend(void)
{
    SPT0 = 1U;
    /* Start user code. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: TemperatureSensor_Init
* Description  : This function initializes the temperature sensor.
* Arguments    : None
* Return Value : MD_STATUS - status of the initialization process (MD_OK if successful)
***********************************************************************************************************************/
MD_STATUS TemperatureSensor_Init()
{
    MD_STATUS ret = MD_OK;
    
    /* Sets the first element of the i2cbuff array to 0x03.This value is likely a command or configuration data that needs to be sent to the temperature sensor for initialization.*/
    i2cbuff[0] = 0x03;
    
    /*Initiates a master-send operation via IICA0 to send the data stored in i2cbuff to the temperature sensor at the address specified by TEMPERATURE_ADDR. It sends 1 byte of data (1U), which is the size of the i2cbuff array, and the last argument 0 may be some additional configuration or control parameter.*/
    ret = R_IICA0_Master_Send(TEMPERATURE_ADDR, i2cbuff, 1U, 0);
    
    return ret;
}
/***********************************************************************************************************************
* Function Name: Temperature_Get
* Description  : This function retrieves the temperature from the sensor.
* Arguments    : data - pointer to a structure where the temperature data will be stored
* Return Value : MD_STATUS - status of the retrieval process (MD_OK if successful)
***********************************************************************************************************************/
MD_STATUS Temperature_Get(Temperature_Data *const data)
{

    /*Initializes a variable ret of type MD_STATUS with an initial value of MD_OK, indicating that the retrieval process starts with the assumption that it will succeed.*/
    MD_STATUS ret = MD_OK;
    
    /*Initiates a master-receive operation via IICA0 to receive temperature data from the sensor at the address specified by TEMPERATURE_ADDR. It reads 1 byte of data (1U) and stores it in the i2cbuff array.The last argument 0 may be some additional configuration or control parameter.*/
    ret = R_IICA0_Master_Receive(TEMPERATURE_ADDR, i2cbuff, 1U, 0);
    
    return ret;
}
/***********************************************************************************************************************
* Function Name: Temperature_Display
* Description  : This function displays the temperature on an LCD screen.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void Temperature_Display(void)
{
    /*Calls the Temperature_Get() function to retrieve temperature data from the sensor and store it in the temperature_Reading variable.*/
    Temperature_Get(&temperature_Reading);
    
    /*Assigns the unit digit character of the temperature (computed from the remainder when divided by 10) to the 3rd position of the temperature_lcd array. */
    temperature_lcd[3] = (int8_t)((temperature_Reading.temperature % 10) + '0');
    temperature_lcd[2] = (int8_t)(((temperature_Reading.temperature / 10) % 10) + '0');
    temperature_lcd[1] = (int8_t)(((temperature_Reading.temperature / 100) % 10) + '0');
    
    /*Assigns the '+' character (indicating positive) to the first position of the temperature_lcd array, representing the sign of the temperature.*/
    temperature_lcd[0] = (int8_t)('+');
    
   /*Checks if the temperature is negative.*/
   if (temperature_Reading.temperature < 0)
    {
	/*Adjusts the temperature value for negative temperatures.*/
        temperature_Reading.temperature = temperature_Reading.temperature - 512;
	
	/*Updates the unit digit character of the temperature after adjustment.*/
        temperature_lcd[3] = (int8_t)((temperature_Reading.temperature % 10) + '0');
        temperature_lcd[2] = (int8_t)(((temperature_Reading.temperature / 10) % 10) + '0');
        temperature_lcd[1] = (int8_t)(((temperature_Reading.temperature / 100) % 10) + '0');
	
	/*Assigns the '-' character (indicating negative) to the first position of the temperature_lcd array for negative temperatures.*/
        temperature_lcd[0] = (int8_t)('-');
    }
    /*Displays the temperature value converted to a character string on the LCD, starting from line 3 column 2.*/
    DisplayLCD(LCD_LINE3 + 2, temperature_lcd);
    /*Displays the "*C" (degrees Celsius) characters on the LCD, starting from line 3 column 7.*/
    DisplayLCD(LCD_LINE3 + 7, "*C");
}
/***********************************************************************************************************************
* Function Name: Average_10tGetData
* Description  : This function calculates the average temperature from 10 measurements.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void Average_10tGetData(void)
{
    /*Stores the current temperature reading in the temperature_buffer array at the index specified by temperature_buffer_index.*/
    temperature_buffer[temperature_buffer_index] = temperature_Reading.temperature;
    
    /*Updates the index for the next temperature reading. It cycles back to 0 when it reaches 10, allowing for a circular buffer of size 10.*/
    temperature_buffer_index = (temperature_buffer_index + 1) % BUFFER_SIZE;
    
    /*Resets the variable temperature_sum to zero before calculating the sum of temperatures.*/
    temperature_sum = 0;
    
    /*Calculates the sum of temperatures stored in the temperature_buffer array.*/
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        temperature_sum += temperature_buffer[i];
    }
    /*Computes the average temperature by dividing the sum of temperatures (temperature_sum) by 10*/
    temperature_average = temperature_sum / 10;

    /*Checks if the buffer index has reached 0, indicating that the buffer is full and it's time to calculate the average.*/
    if (temperature_buffer_index == 0)
    {
	/*Extracts the integer part of the average temperature*/
        intPart = (int)temperature_average;
	
	/*Computes the decimal part of the average temperature by subtracting the integer part and multiplying the result by 10.*/
        decimalPart = (int)(((temperature_average - intPart) * 10));
	
	/*Sets the fifth character of the LCD display buffer to a space.*/
	average_temperature_lcd[5] = ' ';
	/*Converts the decimal part to a character and stores it in the fourth position of the LCD display buffer.*/
        average_temperature_lcd[4] = (int8_t)decimalPart + '0';
	/*Sets the third character of the LCD display buffer to a period (decimal point).*/
        average_temperature_lcd[3] = '.';
	/*Extracts and converts the units digit of the integer part to a character and stores it in the second position of the LCD display buffer.*/
        average_temperature_lcd[2] = (int8_t)(intPart % 10) + '0';
	/* Extracts and converts the tens digit of the integer part to a character and stores it in the first position of the LCD display buffer.*/
        average_temperature_lcd[1] = (int8_t)((intPart / 10) % 10) + '0';
	/*Extracts and converts the hundreds digit of the integer part to a character and stores it in the zeroth position of the LCD display buffer.*/
        average_temperature_lcd[0] = (int8_t)((intPart / 100) % 10) + '0';
    }
}
/* End user code. Do not edit comment generated here */
