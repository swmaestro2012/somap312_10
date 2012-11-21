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
//  native_impl.c
//

#include "types.h"
#include "debug.h"
#include "config.h"
#include "error.h"
#include "stm32f4_discovery.h"

#include "vm.h"
#include "nvmfile.h"
#include "native_impl.h"
// #include "ctbot/native.h"
// #include "ctbot/shift.h"
// #include "ctbot/delay.h"
#include "native.h"
#include "shift.h"
#include "delay.h"

#include "stack.h"

#ifdef NVM_USE_STDIO
#include "native_stdio.h"
#endif

#ifdef NVM_USE_MATH
#include "native_math.h"
#endif

#ifdef NVM_USE_FORMATTER
#include "native_formatter.h"
#endif

#include "native_exception.h"
#include "native_thread.h"
// #include "ctbot/native_bot.h"
// #include "ctbot/native_clock.h"
// #include "ctbot/native_display.h"
// #include "ctbot/native_distancesensor.h"
// #include "ctbot/native_edgedetector.h"
// #include "ctbot/native_ldrsensor.h"
// #include "ctbot/native_leds.h"
// #include "ctbot/native_linedetector.h"
// #include "ctbot/native_lightbarrier.h"
// #include "ctbot/native_motor.h"
// #include "ctbot/native_mouse.h"
// #include "ctbot/native_servo.h"
// #include "ctbot/native_shuttersensor.h"
// #include "ctbot/native_wheelencoder.h"
// #include "ctbot/native_irreceiver.h"
#include "native_bot.h"
#include "native_clock.h"
#include "native_display.h"
#include "native_distancesensor.h"
#include "native_edgedetector.h"
#include "native_ldrsensor.h"
#include "native_leds.h"
#include "native_linedetector.h"
#include "native_lightbarrier.h"
#include "native_motor.h"
#include "native_mouse.h"
#include "native_servo.h"
#include "native_shuttersensor.h"
#include "native_wheelencoder.h"
#include "native_irreceiver.h"
#include "adc.h"
#include "uart.h"

void native_java_lang_object_invoke(u08_t mref) {
  if(mref == NATIVE_METHOD_INIT) {
    /* ignore object constructor ... */
    stack_pop();  // pop object reference
  } else 
    error(ERROR_NATIVE_UNKNOWN_METHOD);
}
  
void native_new(u16_t mref) {
	heap_id_t h;
	if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_OBJECT) {		
		// create object with
		h = heap_alloc(TRUE, sizeof(mref));
		stack_push(NVM_TYPE_HEAP | h);
		((nvm_ref_t*)heap_get_addr(h))[0] = mref;
		
	} else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_STRINGBUFFER) {
    // create empty stringbuf object and push reference onto stack
    stack_push(NVM_TYPE_HEAP | heap_alloc(FALSE, 1));
  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_EXCEPTION) {
		// nothing to do for exception
	}	else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_THREAD) {		
		// 여기에 Thread를 생성하는 코드를 삽입함
		native_thread_init(NATIVE_ID2METHOD(mref));
	}	else 
    error(ERROR_NATIVE_UNKNOWN_CLASS);
}

void native_invoke(u16_t mref) {
  // check for native classes/methods
  if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_OBJECT) {
    native_java_lang_object_invoke(NATIVE_ID2METHOD(mref));

#ifdef NVM_USE_STDIO
  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_PRINTSTREAM) {
    native_java_io_printstream_invoke(NATIVE_ID2METHOD(mref));
  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_INPUTSTREAM) {
    native_java_io_inputstream_invoke(NATIVE_ID2METHOD(mref));
  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_STRINGBUFFER) {
    native_java_lang_stringbuffer_invoke(NATIVE_ID2METHOD(mref));
#endif

#ifdef NVM_USE_MATH
  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_MATH) {
    native_math_invoke(NATIVE_ID2METHOD(mref));
#endif
	} else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_EXCEPTION) {
		native_exception_invoke(NATIVE_ID2METHOD(mref));
	} else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_THREAD) {
		native_thread_invoke(NATIVE_ID2METHOD(mref));
#ifdef NVM_USE_FORMATTER
    // the formatter class
  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_FORMATTER) {
    native_formatter_invoke(NATIVE_ID2METHOD(mref));
#endif

   // the c't-Bot specific classes

  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_CTBOT_BOT) {
    native_ctbot_bot_invoke(NATIVE_ID2METHOD(mref));

  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_CTBOT_CLOCK) {
    native_ctbot_clock_invoke(NATIVE_ID2METHOD(mref));

  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_CTBOT_DISPLAY) {
    native_ctbot_display_invoke(NATIVE_ID2METHOD(mref));

  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_CTBOT_DISTANCESENSOR) {
    native_ctbot_distsensor_invoke(NATIVE_ID2METHOD(mref));

  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_CTBOT_EDGEDETECTOR) {
    native_ctbot_edgedetector_invoke(NATIVE_ID2METHOD(mref));

  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_CTBOT_IRRECEIVER) {
    native_ctbot_irreceiver_invoke(NATIVE_ID2METHOD(mref));

  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_CTBOT_LDRSENSOR) {
    native_ctbot_ldrsensor_invoke(NATIVE_ID2METHOD(mref));

  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_CTBOT_LEDS) {
    native_ctbot_leds_invoke(NATIVE_ID2METHOD(mref));

  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_CTBOT_LINEDETECTOR) {
    native_ctbot_linedetector_invoke(NATIVE_ID2METHOD(mref));

  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_CTBOT_LIGHTBARRIER) {
    native_ctbot_lightbarrier_invoke(NATIVE_ID2METHOD(mref));

  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_CTBOT_MOTOR) {
    native_ctbot_motor_invoke(NATIVE_ID2METHOD(mref));

  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_CTBOT_MOUSE) {
    native_ctbot_mouse_invoke(NATIVE_ID2METHOD(mref));

  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_CTBOT_SERVO) {
    native_ctbot_servo_invoke(NATIVE_ID2METHOD(mref));

  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_CTBOT_SHUTTERSENSOR) {
    native_ctbot_shuttersensor_invoke(NATIVE_ID2METHOD(mref));

  } else if(NATIVE_ID2CLASS(mref) == NATIVE_CLASS_CTBOT_WHEELENCODER) {
    native_ctbot_wheelencoder_invoke(NATIVE_ID2METHOD(mref));

  } else {
    error(ERROR_NATIVE_UNKNOWN_CLASS);
  }
}

extern TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
extern TIM_OCInitTypeDef  TIM_OCInitStructure;
extern NVIC_InitTypeDef NVIC_InitStructure;
void TIM_Init(void)
{
	uint16_t PrescalerValue = 0;
	
	 /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* Enable the TIM3 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	
	/* Compute the prescaler value */
  PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 1000000) - 1;

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 10000;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	/* Prescaler configuration */
  TIM_PrescalerConfig(TIM3, PrescalerValue, TIM_PSCReloadMode_Immediate);

  /* Output Compare Timing Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(TIM3, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);
	
	TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
 
}
void native_init(void)
{
    native_ctbot_clock_init();
   // native_ctbot_leds_init();
    //native_ctbot_display_init();
	  SysTick_Config(SystemCoreClock / 10000);
		//TIM_Init();
		adc_init();
		uart_init();
    // here the other inits will start:
    native_ctbot_bot_init();
    native_ctbot_distsensor_init();
    native_ctbot_edgedetector_init();
    native_ctbot_irreceiver_init();
    native_ctbot_ldrsensor_init();
    native_ctbot_lightbarrier_init();
    native_ctbot_linedetector_init();
    native_ctbot_motor_init();
    native_ctbot_mouse_init();
    native_ctbot_servo_init();
    native_ctbot_shuttersensor_init();
    native_ctbot_wheelencoder_init();

    // show init complete:
    shift_data(0x03, SHIFT_REGISTER_LED);
    delay_ms(10);
    shift_data(0x00, SHIFT_REGISTER_LED);
}

