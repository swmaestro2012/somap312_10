
#include "stm32f4xx.h"
#include "delay.h"
#include "kernel.h"

void delay_us(volatile unsigned long d){
  long temp = TIM5->CNT-d;
  while((long)(temp-TIM5->CNT) <= 0L);
		//Task_yield();
}

void delay_ms(volatile unsigned long d){
	//unsigned int yhc;
  long temp = TIM5->CNT-(1000L*d);
	//SysTick_Config(SystemCoreClock / 10000);
	//yhc = NVIC_GetActive(SysTick_IRQn);
	//NVIC_EnableIRQ(SysTick_IRQn);
	//NVIC_ClearPendingIRQ(SysTick_IRQn);
	//yhc = NVIC_GetPendingIRQ(SysTick_IRQn);
	//yhc = NVIC_GetActive(SysTick_IRQn);
	//IntRestoreIRQ(0);	
  while((long)(temp-TIM5->CNT) <= 0L);
		//Task_yield();
	//Task_yield();
	
}				 



 
