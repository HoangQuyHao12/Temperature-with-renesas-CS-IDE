/*******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only 
* intended for use with Renesas products. No other uses are authorized. This 
* software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS 
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE 
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer *
* Copyright (C) 2012 Renesas Electronics Corporation. All rights reserved.    
*******************************************************************************/
/*******************************************************************************
* Project Name  : ADC_repeat
* Version       : 1.0
* Device(s)     : R5F104PJ
* Tool-Chain    : Cubesuite+ v2.0 or higher
* OS            : None
* H/W Platform  : YRDKRL78G14
* Description   : This sample demonstrates use of the  A/D converter.
*                 The ADC is set up to read channel ANI8 every 50 ms.  ANI8 is
*                 connected to the potentiometer VR1 on the YRDKRL78G14 board.
*                 The 50 ms delay is configured using the 12 bit interval timer.
*                 The ADC reading is recorded and displayed on the LCD.
*
* Operation     : 1. Compile the sample code  and download to the RDK by clicking on the 
* 		  "Debug > Build & Download" button on the "Debug" toolbar. 					
*
*                 2. Click the 'Go' button to start program execution. 
*         
*                 3. The debug LCD will show the name of the sample program along with
*                    instructions directing you to adjust pot VR1. 
*
*                 4. The current ADC value, in hex is displayed.
*
* 		  5. Press SW1 to switch to 8 bit conversion mode.  Press SW3 to
* 		     switch back to 10 bit conversion mode.
*                      
*******************************************************************************/         
/*******************************************************************************
* History : DD.MM.YYYY     Version     Description
*         : 08.08.2013     1.00        First release
*******************************************************************************/
