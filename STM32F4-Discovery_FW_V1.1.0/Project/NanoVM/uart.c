//
//  NanoVM, a tiny java VM for the Atmel AVR family
//  Copyright (C) 2005 by Till Harbaum <Till@Harbaum.org>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

//
//  uart.c
//

#include "types.h"
#include "config.h"
#include "debug.h"
#include "stm32f4xx_usart.h"
#include "stm324xg_eval.h"
#include "uart.h"
//#include "delay.h"
#include <stdio.h>
#define CR1_CLEAR_MASK            ((uint16_t)(USART_CR1_M | USART_CR1_PCE | \
                                              USART_CR1_PS | USART_CR1_TE | \
                                              USART_CR1_RE))
#define CR3_CLEAR_MASK            ((uint16_t)(USART_CR3_RTSE | USART_CR3_CTSE))

USART_InitTypeDef USART_InitStructure;
USART_TypeDef* COM_USART[COMn] = {EVAL_COM0,EVAL_COM1}; 

GPIO_TypeDef* COM_TX_PORT[COMn] = {EVAL_COM0_TX_GPIO_PORT,EVAL_COM1_TX_GPIO_PORT};
 
GPIO_TypeDef* COM_RX_PORT[COMn] = {EVAL_COM0_RX_GPIO_PORT, EVAL_COM1_RX_GPIO_PORT};

const uint32_t COM_USART_CLK[COMn] = {EVAL_COM0_CLK,EVAL_COM1_CLK};

const uint32_t COM_TX_PORT_CLK[COMn] = {EVAL_COM0_TX_GPIO_CLK,EVAL_COM1_TX_GPIO_CLK};
 
const uint32_t COM_RX_PORT_CLK[COMn] = {EVAL_COM0_RX_GPIO_CLK,EVAL_COM1_RX_GPIO_CLK};

const uint16_t COM_TX_PIN[COMn] = {EVAL_COM0_TX_PIN,EVAL_COM1_TX_PIN};

const uint16_t COM_RX_PIN[COMn] = {EVAL_COM0_RX_PIN,EVAL_COM1_RX_PIN};
 
const uint16_t COM_TX_PIN_SOURCE[COMn] = {EVAL_COM0_TX_SOURCE,EVAL_COM1_TX_SOURCE};

const uint16_t COM_RX_PIN_SOURCE[COMn] = {EVAL_COM0_RX_SOURCE,EVAL_COM1_RX_SOURCE};
 
const uint16_t COM_TX_AF[COMn] = {EVAL_COM0_TX_AF,EVAL_COM1_TX_AF};
 
const uint16_t COM_RX_AF[COMn] = {EVAL_COM0_RX_AF,EVAL_COM1_RX_AF};

void STM_EVAL_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  if (COM == COM1 || COM == COM0)
  {
  /* Enable GPIO clock */
  RCC_AHB1PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM], ENABLE);

    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
   

  /* Connect PXx to USARTx_Tx*/
  GPIO_PinAFConfig(COM_TX_PORT[COM], COM_TX_PIN_SOURCE[COM], COM_TX_AF[COM]);

  /* Connect PXx to USARTx_Rx*/
  GPIO_PinAFConfig(COM_RX_PORT[COM], COM_RX_PIN_SOURCE[COM], COM_RX_AF[COM]);

  /* Configure USART Tx as alternate function  */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);

  /* Configure USART Rx as alternate function  */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
  GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);

	
  /* USART configuration */
  USART_Init(COM_USART[COM], USART_InitStruct);
  
  
  /* Enable USART */
  USART_Cmd(COM_USART[COM], ENABLE);
	}
}
int test_count;
int test_index;
int idex;
char test;
void uart_init(void)
{
	  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  STM_EVAL_COMInit(COM1, &USART_InitStructure);
		  USART_InitStructure.USART_BaudRate = 9600;

	STM_EVAL_COMInit(COM0, &USART_InitStructure);
	GPIO_WriteBit(GPIOD, GPIO_Pin_10, Bit_RESET);
	GPIO_WriteBit(GPIOD, GPIO_Pin_10, Bit_SET);
  /* Output a message on Hyperterminal using printf function */
	
	/*while(1)
	{
		USART_SendData(EVAL_COM0, 'a');
	}*/
	
	//bluetooth test code
  /*while (1)
  {
		if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE))
		{
			test = USART_ReceiveData(USART2);
			while(!USART_GetFlagStatus(USART2, USART_FLAG_TXE));
			USART_SendData(USART2, test);
			while(!USART_GetFlagStatus(USART3, USART_FLAG_TXE));
			USART_SendData(USART3, test);
		}
		
		if(USART_GetFlagStatus(USART3, USART_FLAG_RXNE))
		{
			test = USART_ReceiveData(USART3);
			while(!USART_GetFlagStatus(USART2, USART_FLAG_TXE));
			USART_SendData(USART2, test);
		}
		
  }*/
	
}


u08_t uart_available(void)
{
	return 0;
}
void uart_write_byte(u08_t byte) {
	
}
u08_t uart_read_byte(void) {
	return 0;
}

void uart_putc(u08_t byte) {
	while(!USART_GetFlagStatus(USART2, USART_FLAG_TXE));
	
  USART_SendData(USART2, byte);
	
}

void PrintString(u08_t *str_p)
{
	int i=0;
	
	while(str_p[i]){
		uart_putc(str_p[i]);
		i++;
	}
}


