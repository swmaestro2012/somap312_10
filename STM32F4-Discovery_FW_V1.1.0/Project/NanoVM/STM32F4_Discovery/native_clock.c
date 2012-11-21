//
//  c't-Bot-NanoVM, a tiny java VM for the c't-Bot
//
//  Copyright (C) 2007 by Nils Springob <nils@nicai-systems.de>
//  Based on work by Benjamin Benz(c't-Bot) and Till Harbaum(NanoVM)
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
//  native_clock.c
//

#include "types.h"
#include "debug.h"
#include "config.h"
#include "error.h"
#ifdef CTBOT
#include "delay.h"
#include "stack.h"
#include "vm.h"
#include "stm32f4xx.h"

//#include "ctbot/native.h"
//#include "ctbot/native_clock.h"
//#include "ctbot/pwm.h"
#include "native.h"
#include "native_clock.h"
#include "pwm.h"
#include "stm32f4xx_it.h"
//#include <util/delay.h>
//#include <avr/interrupt.h>

#define NATIVE_METHOD_getSeconds 1
#define NATIVE_METHOD_getMilliseconds 2
#define NATIVE_METHOD_delayMilliseconds 3
#define NATIVE_METHOD_delayMicroseconds 4



void TIM_Config(void)// Timer Configuration
{
	uint16_t PrescalerValue;
		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	/* TIM5 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	
	/* Compute the prescaler value */
	PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 1000000) - 1;
	
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 0xffffffff;
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
	
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
	
	TIM5->CNT = 0xffffffff;
	/* TIM5 enable counter */
	TIM_Cmd(TIM5, ENABLE);

} 
// initialize the ctbot hardware
void native_ctbot_clock_init(void) {
  TIM_Config();
	
}

// the ctbot class
void native_ctbot_clock_invoke(u08_t mref) {

  // JAVA: void delayMilliseconds(int msec)
  if(mref == NATIVE_METHOD_delayMilliseconds) { //delay_ms     // getTick
		nvm_int_t time = stack_pop_int();
		delay_ms(time);
		//    nvm_int_t time = stack_pop_int();
			
//    while(time--)
//      _delay_ms(1);
  }

  // JAVA: void delayMicroseconds(int usec)
  else if(mref == NATIVE_METHOD_delayMicroseconds) {//resolve
    nvm_int_t time = stack_pop_int();
		delay_us(time);
  }

  // JAVA: int getSeconds()
  else if(mref == NATIVE_METHOD_getSeconds) { 
//    cli();
		nvm_int_t time = TIM5->CNT;
		stack_push(time>>8);
		stack_push(time&0xff);
//    sei();
  }

  // JAVA: int getMiliseconds()
  else if(mref == NATIVE_METHOD_getMilliseconds) { // delay_us
//    cli();

//    sei();
  }


}

#endif
