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

/*! @file       pwm.c
 * @brief       Modul zur Erzeugung der Pwm-Signale und Zeitberechnung
 * @author      Nils Springob (nils@nicai-systems.de)
 * @date        2007-01-05
*/

#include "types.h"
//#include <avr/interrupt.h>
#include <stdlib.h> 
#include <math.h>
//#include "ctbot/pwm.h"
//#include "ctbot/pid.h"
//#include "ctbot/wheelencoder.h"
#include "pwm.h"
#include "pid.h"
#include "wheelencoder.h"
#include "stm32f4_discovery.h"
#include "stm32f4xx_tim.h"

#define PWM_SIGNAL_OC       SIG_OUTPUT_COMPARE0
#define PWM_TIMER_CNT       TCNT0
#define PWM_TIMER_CCR       TCCR0
#define PWM_TIMER_CCR_INIT  _BV(WGM01) | _BV(CS01) | _BV(CS00)
#define PWM_TIMER_OCR       OCR0
#define PWM_TIMER_OCR_INIT  9
#define PWM_TIMER_IMSK      _BV(OCIE0)

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
__IO uint16_t CCR1_Val = 54618;
__IO uint16_t CCR2_Val = 27309;
__IO uint16_t CCR3_Val = 13654;
__IO uint16_t CCR4_Val = 6826;
uint16_t PrescalerValue = 0;
// diese Variablen enthalten die Soll-Werte: 
uint8_t pwm_servo_1;
uint8_t pwm_servo_2;
int16_t pwm_motor_l;
int16_t pwm_motor_r;



typedef struct
{
  uint8_t lo; // zählt bis 25(1ms) in 40us Schritten 
  uint8_t hi; // zählt bis 20(20ms) in 1ms Schritten
} pwm_cnt_t;


// interne Register
pwm_cnt_t pwm_servo_1_cnt;
pwm_cnt_t pwm_servo_2_cnt;
pwm_cnt_t pwm_motor_l_cnt;
pwm_cnt_t pwm_motor_r_cnt;

pwm_cnt_t pwm_timer_sub; // zählt bis 20*25(20ms) in 40us Schritten
uint16_t pwm_timer_ms;   // zählt bis 1000(1s) in 1ms Schritten
uint32_t pwm_timer_sec;  // zählt die Sekunden seit Systemstart
int32_t pwm_value1, pwm_value2, pwm_tick, servo_pwm_tick;
int32_t servo_value[8], servo_flag[8];

void motor_pwm_generate(void)
{
	int abs_pwm;
	
	abs_pwm = abs(pwm_value1);
	
	if(pwm_value1 > 0){
		GPIO_ResetBits(GPIOD, GPIO_Pin_1);
		if(abs_pwm > pwm_tick)
		{
			GPIO_SetBits(GPIOD, GPIO_Pin_0);
		} else
		{
			GPIO_ResetBits(GPIOD, GPIO_Pin_0);
		}
	} else if(pwm_value1 < 0){
		GPIO_SetBits(GPIOD, GPIO_Pin_1);
		if(abs_pwm > pwm_tick)
		{
			GPIO_ResetBits(GPIOD, GPIO_Pin_0);
		} else
		{
			GPIO_SetBits(GPIOD, GPIO_Pin_0);
		}
	}	else
	{
		GPIO_SetBits(GPIOD, GPIO_Pin_0);
		GPIO_SetBits(GPIOD, GPIO_Pin_1);
	}
	
	abs_pwm = abs(pwm_value2);
	
	if(pwm_value2 > 0){
		GPIO_ResetBits(GPIOD, GPIO_Pin_3);
		if(abs_pwm > pwm_tick)
		{
			GPIO_SetBits(GPIOD, GPIO_Pin_2);
		} else
		{
			GPIO_ResetBits(GPIOD, GPIO_Pin_2);
		}
	} else if(pwm_value2 < 0){
		GPIO_SetBits(GPIOD, GPIO_Pin_3);
		if(abs_pwm > pwm_tick)
		{
			GPIO_ResetBits(GPIOD, GPIO_Pin_2);
		} else
		{
			GPIO_SetBits(GPIOD, GPIO_Pin_2);
		}
	}	else
	{
		GPIO_SetBits(GPIOD, GPIO_Pin_2);
		GPIO_SetBits(GPIOD, GPIO_Pin_3);
	}
	
	pwm_tick++;
		
	if(pwm_tick >100)
	{
		pwm_tick=0;
	}
}

void set_motor_speed(int channel, int value)
{
	if(channel == 0)
	{
		pwm_value1 = value;
	} else if(channel == 1)
	{
		pwm_value2 = value;
	}
}

void motor_pwm_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

  /* GPIOC and GPIOB clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
  /* GPIOC Configuration: TIM3 CH1 (PC6) and TIM3 CH2 (PC7) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOD, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3);
}

void servo_pwm_generate(void)
{
	int i;
	for(i=0; i<8; i++)
	{
		if(servo_flag[i] == 0 )	continue;
		if(servo_value[i] > servo_pwm_tick)
		{
			GPIO_SetBits(GPIOE, GPIO_Pin_5 << i);
		} else
		{
			GPIO_ResetBits(GPIOE, GPIO_Pin_5 << i);
		}
	}
	
	servo_pwm_tick++;
		
	if(servo_pwm_tick >200)
	{
		servo_pwm_tick=0;
	}
}

void set_servo_position(int channel, int value)
{
	servo_value[channel] = value + 9;
	servo_flag[channel] = 1;
}

void servo_release(int channel)
{
	servo_flag[channel] = 0;
	GPIO_ResetBits(GPIOE, GPIO_Pin_5 << channel);
}

void servo_pwm_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

  /* GPIOC and GPIOB clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE , ENABLE);
  
  /* GPIOC Configuration: TIM3 CH1 (PC6) and TIM3 CH2 (PC7) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOE, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12);
}
void Motor_isr(void)//SIGNAL(PWM_SIGNAL_OC)
//void do_pwm_int()
{
//   if (pwm_timer_sub.lo==25)
//   {
//     // Jede Milisekunde:
//     pwm_timer_sub.lo=0;
//     ++pwm_timer_ms;
//     if (pwm_timer_ms==1000)
//     {
//       pwm_timer_ms=0;
//       ++pwm_timer_sec;
//     }

//     wheelencoder_isr();

//     ++pwm_timer_sub.hi;
//     // Setze die Bits wenn der Startwert erreicht wird:
//     if (pwm_timer_sub.hi==20)
//     {
//       // LEFT MOTOR
//       pwm_timer_sub.hi=0;
//       pid_left_timer_int();
//       pwm_motor_l_cnt.lo = abs(pwm_motor_l)%25;
//       pwm_motor_l_cnt.hi = abs(pwm_motor_l)/25;
//       if (pwm_motor_l_cnt.hi == 20)
//         pwm_motor_l_cnt.hi = 21;
//       set_output_bitval(IO_MOTOR_L_DIR, (pwm_motor_l>0));
//       set_output_bitval(IO_MOTOR_L, (pwm_motor_l!=0));
//     }

//     if (pwm_timer_sub.hi==5)
//     {
//       // SERVO 1
//       pwm_servo_1_cnt.lo = pwm_servo_1%25;
//       pwm_servo_1_cnt.hi = 5+(pwm_servo_1/25)+1;
//       set_output_bitval(IO_SERVO_1, (pwm_servo_1>0));
//     }

//     if (pwm_timer_sub.hi==10)
//     {
//       // RIGHT MOTOR
//       pid_right_timer_int();
//       pwm_motor_r_cnt.lo = abs(pwm_motor_r)%25;
//       pwm_motor_r_cnt.hi = ((250+abs(pwm_motor_r))%500) / 25;
//       if (abs(pwm_motor_r)==500)
//         pwm_motor_r_cnt.hi = 21;
//       set_output_bitval(IO_MOTOR_R_DIR, (pwm_motor_r<0));
//       set_output_bitval(IO_MOTOR_R, (pwm_motor_r!=0));
//     }

//     if (pwm_timer_sub.hi==15)
//     {
//       // SERVO 2
//       pwm_servo_2_cnt.lo = pwm_servo_2%25;
//       pwm_servo_2_cnt.hi = 15+(pwm_servo_2/25)+1;
//       set_output_bitval(IO_SERVO_2, (pwm_servo_2>0));
//     }
//   }

//   uint8_t next = 25 - pwm_timer_sub.lo;
//  
//   if (pwm_motor_l_cnt.hi==pwm_timer_sub.hi)
//   {
//     int8_t diff = pwm_motor_l_cnt.lo-pwm_timer_sub.lo; 
//     if (diff>0)
//     {
//       if (diff<next)
//          next=diff;
//     }
//     else if (diff==0)
//     {
//       clear_output_bit(IO_MOTOR_L);
//     }
//   }

//   if (pwm_motor_r_cnt.hi==pwm_timer_sub.hi)
//   {
//     int8_t diff = pwm_motor_r_cnt.lo-pwm_timer_sub.lo;
//     if (diff>0)
//     {
//       if (diff<next)
//          next=diff;
//     }
//     else if (diff==0)
//     {
//       clear_output_bit(IO_MOTOR_R);
//     }
//   }

//   if (pwm_servo_1_cnt.hi==pwm_timer_sub.hi)
//   {
//     int8_t diff = pwm_servo_1_cnt.lo-pwm_timer_sub.lo;
//     if (diff>0)
//     {
//       if (diff<next)
//          next=diff;
//     }
//     else if (diff==0)
//     {
//       clear_output_bit(IO_SERVO_1);
//     }
//   }

//   if (pwm_servo_2_cnt.hi==pwm_timer_sub.hi)
//   {
//     int8_t diff = pwm_servo_2_cnt.lo-pwm_timer_sub.lo;
//     if (diff>0)
//     {
//       if (diff<next)
//          next=diff;
//     }
//     else if (diff==0)
//     {
//       clear_output_bit(IO_SERVO_2);
//     }
//   }

//   pwm_timer_sub.lo += next;
//   PWM_TIMER_OCR=next*10-1; 
}

void TIM_TimeBaseInit(TIM_TypeDef* TIMx, TIM_TimeBaseInitTypeDef* TIM_TimeBaseInitStruct)
{
  uint16_t tmpcr1 = 0;

  /* Check the parameters */
  assert_param(IS_TIM_ALL_PERIPH(TIMx)); 
  assert_param(IS_TIM_COUNTER_MODE(TIM_TimeBaseInitStruct->TIM_CounterMode));
  assert_param(IS_TIM_CKD_DIV(TIM_TimeBaseInitStruct->TIM_ClockDivision));

  tmpcr1 = TIMx->CR1;  

  if((TIMx == TIM1) || (TIMx == TIM8)||
     (TIMx == TIM2) || (TIMx == TIM3)||
     (TIMx == TIM4) || (TIMx == TIM5)) 
  {
    /* Select the Counter Mode */
    tmpcr1 &= (uint16_t)(~(TIM_CR1_DIR | TIM_CR1_CMS));
    tmpcr1 |= (uint32_t)TIM_TimeBaseInitStruct->TIM_CounterMode;
  }
 
  if((TIMx != TIM6) && (TIMx != TIM7))
  {
    /* Set the clock division */
    tmpcr1 &=  (uint16_t)(~TIM_CR1_CKD);
    tmpcr1 |= (uint32_t)TIM_TimeBaseInitStruct->TIM_ClockDivision;
  }

  TIMx->CR1 = tmpcr1;

  /* Set the Autoreload value */
  TIMx->ARR = TIM_TimeBaseInitStruct->TIM_Period ;
 
  /* Set the Prescaler value */
  TIMx->PSC = TIM_TimeBaseInitStruct->TIM_Prescaler;
    
  if ((TIMx == TIM1) || (TIMx == TIM8))  
  {
    /* Set the Repetition Counter value */
    TIMx->RCR = TIM_TimeBaseInitStruct->TIM_RepetitionCounter;
  }

  /* Generate an update event to reload the Prescaler 
     and the repetition counter(only for TIM1 and TIM8) value immediatly */
  TIMx->EGR = TIM_PSCReloadMode_Immediate;          
}


void TIM_ITConfig(TIM_TypeDef* TIMx, uint16_t TIM_IT, FunctionalState NewState)
{  
  /* Check the parameters */
  assert_param(IS_TIM_ALL_PERIPH(TIMx));
  assert_param(IS_TIM_IT(TIM_IT));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  if (NewState != DISABLE)
  {
    /* Enable the Interrupt sources */
    TIMx->DIER |= TIM_IT;
  }
  else
  {
    /* Disable the Interrupt sources */
    TIMx->DIER &= (uint16_t)~TIM_IT;
  }
}

void TIM_Cmd(TIM_TypeDef* TIMx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_TIM_ALL_PERIPH(TIMx)); 
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  if (NewState != DISABLE)
  {
    /* Enable the TIM Counter */
    TIMx->CR1 |= TIM_CR1_CEN;
  }
  else
  {
    /* Disable the TIM Counter */
    TIMx->CR1 &= (uint16_t)~TIM_CR1_CEN;
  }
}

void TIM_OC1Init(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct)
{
  uint16_t tmpccmrx = 0, tmpccer = 0, tmpcr2 = 0;
   
  /* Check the parameters */
  assert_param(IS_TIM_LIST1_PERIPH(TIMx)); 
  assert_param(IS_TIM_OC_MODE(TIM_OCInitStruct->TIM_OCMode));
  assert_param(IS_TIM_OUTPUT_STATE(TIM_OCInitStruct->TIM_OutputState));
  assert_param(IS_TIM_OC_POLARITY(TIM_OCInitStruct->TIM_OCPolarity));   

  /* Disable the Channel 1: Reset the CC1E Bit */
  TIMx->CCER &= (uint16_t)~TIM_CCER_CC1E;
  
  /* Get the TIMx CCER register value */
  tmpccer = TIMx->CCER;
  /* Get the TIMx CR2 register value */
  tmpcr2 =  TIMx->CR2;
  
  /* Get the TIMx CCMR1 register value */
  tmpccmrx = TIMx->CCMR1;
    
  /* Reset the Output Compare Mode Bits */
  tmpccmrx &= (uint16_t)~TIM_CCMR1_OC1M;
  tmpccmrx &= (uint16_t)~TIM_CCMR1_CC1S;
  /* Select the Output Compare Mode */
  tmpccmrx |= TIM_OCInitStruct->TIM_OCMode;
  
  /* Reset the Output Polarity level */
  tmpccer &= (uint16_t)~TIM_CCER_CC1P;
  /* Set the Output Compare Polarity */
  tmpccer |= TIM_OCInitStruct->TIM_OCPolarity;
  
  /* Set the Output State */
  tmpccer |= TIM_OCInitStruct->TIM_OutputState;
    
  if((TIMx == TIM1) || (TIMx == TIM8))
  {
    assert_param(IS_TIM_OUTPUTN_STATE(TIM_OCInitStruct->TIM_OutputNState));
    assert_param(IS_TIM_OCN_POLARITY(TIM_OCInitStruct->TIM_OCNPolarity));
    assert_param(IS_TIM_OCNIDLE_STATE(TIM_OCInitStruct->TIM_OCNIdleState));
    assert_param(IS_TIM_OCIDLE_STATE(TIM_OCInitStruct->TIM_OCIdleState));
    
    /* Reset the Output N Polarity level */
    tmpccer &= (uint16_t)~TIM_CCER_CC1NP;
    /* Set the Output N Polarity */
    tmpccer |= TIM_OCInitStruct->TIM_OCNPolarity;
    /* Reset the Output N State */
    tmpccer &= (uint16_t)~TIM_CCER_CC1NE;
    
    /* Set the Output N State */
    tmpccer |= TIM_OCInitStruct->TIM_OutputNState;
    /* Reset the Output Compare and Output Compare N IDLE State */
    tmpcr2 &= (uint16_t)~TIM_CR2_OIS1;
    tmpcr2 &= (uint16_t)~TIM_CR2_OIS1N;
    /* Set the Output Idle state */
    tmpcr2 |= TIM_OCInitStruct->TIM_OCIdleState;
    /* Set the Output N Idle state */
    tmpcr2 |= TIM_OCInitStruct->TIM_OCNIdleState;
  }
  /* Write to TIMx CR2 */
  TIMx->CR2 = tmpcr2;
  
  /* Write to TIMx CCMR1 */
  TIMx->CCMR1 = tmpccmrx;
  
  /* Set the Capture Compare Register value */
  TIMx->CCR1 = TIM_OCInitStruct->TIM_Pulse;
  
  /* Write to TIMx CCER */
  TIMx->CCER = tmpccer;
}
void TIM_OC2Init(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct)
{
  uint16_t tmpccmrx = 0, tmpccer = 0, tmpcr2 = 0;
   
  /* Check the parameters */
  assert_param(IS_TIM_LIST2_PERIPH(TIMx)); 
  assert_param(IS_TIM_OC_MODE(TIM_OCInitStruct->TIM_OCMode));
  assert_param(IS_TIM_OUTPUT_STATE(TIM_OCInitStruct->TIM_OutputState));
  assert_param(IS_TIM_OC_POLARITY(TIM_OCInitStruct->TIM_OCPolarity));   

  /* Disable the Channel 2: Reset the CC2E Bit */
  TIMx->CCER &= (uint16_t)~TIM_CCER_CC2E;
  
  /* Get the TIMx CCER register value */  
  tmpccer = TIMx->CCER;
  /* Get the TIMx CR2 register value */
  tmpcr2 =  TIMx->CR2;
  
  /* Get the TIMx CCMR1 register value */
  tmpccmrx = TIMx->CCMR1;
    
  /* Reset the Output Compare mode and Capture/Compare selection Bits */
  tmpccmrx &= (uint16_t)~TIM_CCMR1_OC2M;
  tmpccmrx &= (uint16_t)~TIM_CCMR1_CC2S;
  
  /* Select the Output Compare Mode */
  tmpccmrx |= (uint16_t)(TIM_OCInitStruct->TIM_OCMode << 8);
  
  /* Reset the Output Polarity level */
  tmpccer &= (uint16_t)~TIM_CCER_CC2P;
  /* Set the Output Compare Polarity */
  tmpccer |= (uint16_t)(TIM_OCInitStruct->TIM_OCPolarity << 4);
  
  /* Set the Output State */
  tmpccer |= (uint16_t)(TIM_OCInitStruct->TIM_OutputState << 4);
    
  if((TIMx == TIM1) || (TIMx == TIM8))
  {
    assert_param(IS_TIM_OUTPUTN_STATE(TIM_OCInitStruct->TIM_OutputNState));
    assert_param(IS_TIM_OCN_POLARITY(TIM_OCInitStruct->TIM_OCNPolarity));
    assert_param(IS_TIM_OCNIDLE_STATE(TIM_OCInitStruct->TIM_OCNIdleState));
    assert_param(IS_TIM_OCIDLE_STATE(TIM_OCInitStruct->TIM_OCIdleState));
    
    /* Reset the Output N Polarity level */
    tmpccer &= (uint16_t)~TIM_CCER_CC2NP;
    /* Set the Output N Polarity */
    tmpccer |= (uint16_t)(TIM_OCInitStruct->TIM_OCNPolarity << 4);
    /* Reset the Output N State */
    tmpccer &= (uint16_t)~TIM_CCER_CC2NE;
    
    /* Set the Output N State */
    tmpccer |= (uint16_t)(TIM_OCInitStruct->TIM_OutputNState << 4);
    /* Reset the Output Compare and Output Compare N IDLE State */
    tmpcr2 &= (uint16_t)~TIM_CR2_OIS2;
    tmpcr2 &= (uint16_t)~TIM_CR2_OIS2N;
    /* Set the Output Idle state */
    tmpcr2 |= (uint16_t)(TIM_OCInitStruct->TIM_OCIdleState << 2);
    /* Set the Output N Idle state */
    tmpcr2 |= (uint16_t)(TIM_OCInitStruct->TIM_OCNIdleState << 2);
  }
  /* Write to TIMx CR2 */
  TIMx->CR2 = tmpcr2;
  
  /* Write to TIMx CCMR1 */
  TIMx->CCMR1 = tmpccmrx;
  
  /* Set the Capture Compare Register value */
  TIMx->CCR2 = TIM_OCInitStruct->TIM_Pulse;
  
  /* Write to TIMx CCER */
  TIMx->CCER = tmpccer;
}

/**
  * @brief  Initializes the TIMx Channel3 according to the specified parameters
  *         in the TIM_OCInitStruct.
  * @param  TIMx: where x can be 1, 2, 3, 4, 5 or 8 to select the TIM peripheral.
  * @param  TIM_OCInitStruct: pointer to a TIM_OCInitTypeDef structure that contains
  *         the configuration information for the specified TIM peripheral.
  * @retval None
  */
void TIM_OC3Init(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct)
{
  uint16_t tmpccmrx = 0, tmpccer = 0, tmpcr2 = 0;
   
  /* Check the parameters */
  assert_param(IS_TIM_LIST3_PERIPH(TIMx)); 
  assert_param(IS_TIM_OC_MODE(TIM_OCInitStruct->TIM_OCMode));
  assert_param(IS_TIM_OUTPUT_STATE(TIM_OCInitStruct->TIM_OutputState));
  assert_param(IS_TIM_OC_POLARITY(TIM_OCInitStruct->TIM_OCPolarity));   

  /* Disable the Channel 3: Reset the CC2E Bit */
  TIMx->CCER &= (uint16_t)~TIM_CCER_CC3E;
  
  /* Get the TIMx CCER register value */
  tmpccer = TIMx->CCER;
  /* Get the TIMx CR2 register value */
  tmpcr2 =  TIMx->CR2;
  
  /* Get the TIMx CCMR2 register value */
  tmpccmrx = TIMx->CCMR2;
    
  /* Reset the Output Compare mode and Capture/Compare selection Bits */
  tmpccmrx &= (uint16_t)~TIM_CCMR2_OC3M;
  tmpccmrx &= (uint16_t)~TIM_CCMR2_CC3S;  
  /* Select the Output Compare Mode */
  tmpccmrx |= TIM_OCInitStruct->TIM_OCMode;
  
  /* Reset the Output Polarity level */
  tmpccer &= (uint16_t)~TIM_CCER_CC3P;
  /* Set the Output Compare Polarity */
  tmpccer |= (uint16_t)(TIM_OCInitStruct->TIM_OCPolarity << 8);
  
  /* Set the Output State */
  tmpccer |= (uint16_t)(TIM_OCInitStruct->TIM_OutputState << 8);
    
  if((TIMx == TIM1) || (TIMx == TIM8))
  {
    assert_param(IS_TIM_OUTPUTN_STATE(TIM_OCInitStruct->TIM_OutputNState));
    assert_param(IS_TIM_OCN_POLARITY(TIM_OCInitStruct->TIM_OCNPolarity));
    assert_param(IS_TIM_OCNIDLE_STATE(TIM_OCInitStruct->TIM_OCNIdleState));
    assert_param(IS_TIM_OCIDLE_STATE(TIM_OCInitStruct->TIM_OCIdleState));
    
    /* Reset the Output N Polarity level */
    tmpccer &= (uint16_t)~TIM_CCER_CC3NP;
    /* Set the Output N Polarity */
    tmpccer |= (uint16_t)(TIM_OCInitStruct->TIM_OCNPolarity << 8);
    /* Reset the Output N State */
    tmpccer &= (uint16_t)~TIM_CCER_CC3NE;
    
    /* Set the Output N State */
    tmpccer |= (uint16_t)(TIM_OCInitStruct->TIM_OutputNState << 8);
    /* Reset the Output Compare and Output Compare N IDLE State */
    tmpcr2 &= (uint16_t)~TIM_CR2_OIS3;
    tmpcr2 &= (uint16_t)~TIM_CR2_OIS3N;
    /* Set the Output Idle state */
    tmpcr2 |= (uint16_t)(TIM_OCInitStruct->TIM_OCIdleState << 4);
    /* Set the Output N Idle state */
    tmpcr2 |= (uint16_t)(TIM_OCInitStruct->TIM_OCNIdleState << 4);
  }
  /* Write to TIMx CR2 */
  TIMx->CR2 = tmpcr2;
  
  /* Write to TIMx CCMR2 */
  TIMx->CCMR2 = tmpccmrx;
  
  /* Set the Capture Compare Register value */
  TIMx->CCR3 = TIM_OCInitStruct->TIM_Pulse;
  
  /* Write to TIMx CCER */
  TIMx->CCER = tmpccer;
}
void TIM_OC4Init(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct)
{
  uint16_t tmpccmrx = 0, tmpccer = 0, tmpcr2 = 0;
   
  /* Check the parameters */
  assert_param(IS_TIM_LIST3_PERIPH(TIMx)); 
  assert_param(IS_TIM_OC_MODE(TIM_OCInitStruct->TIM_OCMode));
  assert_param(IS_TIM_OUTPUT_STATE(TIM_OCInitStruct->TIM_OutputState));
  assert_param(IS_TIM_OC_POLARITY(TIM_OCInitStruct->TIM_OCPolarity));   

  /* Disable the Channel 4: Reset the CC4E Bit */
  TIMx->CCER &= (uint16_t)~TIM_CCER_CC4E;
  
  /* Get the TIMx CCER register value */
  tmpccer = TIMx->CCER;
  /* Get the TIMx CR2 register value */
  tmpcr2 =  TIMx->CR2;
  
  /* Get the TIMx CCMR2 register value */
  tmpccmrx = TIMx->CCMR2;
    
  /* Reset the Output Compare mode and Capture/Compare selection Bits */
  tmpccmrx &= (uint16_t)~TIM_CCMR2_OC4M;
  tmpccmrx &= (uint16_t)~TIM_CCMR2_CC4S;
  
  /* Select the Output Compare Mode */
  tmpccmrx |= (uint16_t)(TIM_OCInitStruct->TIM_OCMode << 8);
  
  /* Reset the Output Polarity level */
  tmpccer &= (uint16_t)~TIM_CCER_CC4P;
  /* Set the Output Compare Polarity */
  tmpccer |= (uint16_t)(TIM_OCInitStruct->TIM_OCPolarity << 12);
  
  /* Set the Output State */
  tmpccer |= (uint16_t)(TIM_OCInitStruct->TIM_OutputState << 12);
  
  if((TIMx == TIM1) || (TIMx == TIM8))
  {
    assert_param(IS_TIM_OCIDLE_STATE(TIM_OCInitStruct->TIM_OCIdleState));
    /* Reset the Output Compare IDLE State */
    tmpcr2 &=(uint16_t) ~TIM_CR2_OIS4;
    /* Set the Output Idle state */
    tmpcr2 |= (uint16_t)(TIM_OCInitStruct->TIM_OCIdleState << 6);
  }
  /* Write to TIMx CR2 */
  TIMx->CR2 = tmpcr2;
  
  /* Write to TIMx CCMR2 */  
  TIMx->CCMR2 = tmpccmrx;
    
  /* Set the Capture Compare Register value */
  TIMx->CCR4 = TIM_OCInitStruct->TIM_Pulse;
  
  /* Write to TIMx CCER */
  TIMx->CCER = tmpccer;
}

void TIM_OC1PreloadConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPreload)
{
  uint16_t tmpccmr1 = 0;

  /* Check the parameters */
  assert_param(IS_TIM_LIST1_PERIPH(TIMx));
  assert_param(IS_TIM_OCPRELOAD_STATE(TIM_OCPreload));

  tmpccmr1 = TIMx->CCMR1;

  /* Reset the OC1PE Bit */
  tmpccmr1 &= (uint16_t)(~TIM_CCMR1_OC1PE);

  /* Enable or Disable the Output Compare Preload feature */
  tmpccmr1 |= TIM_OCPreload;

  /* Write to TIMx CCMR1 register */
  TIMx->CCMR1 = tmpccmr1;
}

/**
  * @brief  Enables or disables the TIMx peripheral Preload register on CCR2.
  * @param  TIMx: where x can be  1, 2, 3, 4, 5, 8, 9 or 12 to select the TIM 
  *         peripheral.
  * @param  TIM_OCPreload: new state of the TIMx peripheral Preload register
  *          This parameter can be one of the following values:
  *            @arg TIM_OCPreload_Enable
  *            @arg TIM_OCPreload_Disable
  * @retval None
  */
void TIM_OC2PreloadConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPreload)
{
  uint16_t tmpccmr1 = 0;

  /* Check the parameters */
  assert_param(IS_TIM_LIST2_PERIPH(TIMx));
  assert_param(IS_TIM_OCPRELOAD_STATE(TIM_OCPreload));

  tmpccmr1 = TIMx->CCMR1;

  /* Reset the OC2PE Bit */
  tmpccmr1 &= (uint16_t)(~TIM_CCMR1_OC2PE);

  /* Enable or Disable the Output Compare Preload feature */
  tmpccmr1 |= (uint16_t)(TIM_OCPreload << 8);

  /* Write to TIMx CCMR1 register */
  TIMx->CCMR1 = tmpccmr1;
}

/**
  * @brief  Enables or disables the TIMx peripheral Preload register on CCR3.
  * @param  TIMx: where x can be  1, 2, 3, 4, 5 or 8 to select the TIM peripheral.
  * @param  TIM_OCPreload: new state of the TIMx peripheral Preload register
  *          This parameter can be one of the following values:
  *            @arg TIM_OCPreload_Enable
  *            @arg TIM_OCPreload_Disable
  * @retval None
  */
void TIM_OC3PreloadConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPreload)
{
  uint16_t tmpccmr2 = 0;

  /* Check the parameters */
  assert_param(IS_TIM_LIST3_PERIPH(TIMx));
  assert_param(IS_TIM_OCPRELOAD_STATE(TIM_OCPreload));

  tmpccmr2 = TIMx->CCMR2;

  /* Reset the OC3PE Bit */
  tmpccmr2 &= (uint16_t)(~TIM_CCMR2_OC3PE);

  /* Enable or Disable the Output Compare Preload feature */
  tmpccmr2 |= TIM_OCPreload;

  /* Write to TIMx CCMR2 register */
  TIMx->CCMR2 = tmpccmr2;
}

/**
  * @brief  Enables or disables the TIMx peripheral Preload register on CCR4.
  * @param  TIMx: where x can be  1, 2, 3, 4, 5 or 8 to select the TIM peripheral.
  * @param  TIM_OCPreload: new state of the TIMx peripheral Preload register
  *          This parameter can be one of the following values:
  *            @arg TIM_OCPreload_Enable
  *            @arg TIM_OCPreload_Disable
  * @retval None
  */
void TIM_OC4PreloadConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPreload)
{
  uint16_t tmpccmr2 = 0;

  /* Check the parameters */
  assert_param(IS_TIM_LIST3_PERIPH(TIMx));
  assert_param(IS_TIM_OCPRELOAD_STATE(TIM_OCPreload));

  tmpccmr2 = TIMx->CCMR2;

  /* Reset the OC4PE Bit */
  tmpccmr2 &= (uint16_t)(~TIM_CCMR2_OC4PE);

  /* Enable or Disable the Output Compare Preload feature */
  tmpccmr2 |= (uint16_t)(TIM_OCPreload << 8);

  /* Write to TIMx CCMR2 register */
  TIMx->CCMR2 = tmpccmr2;
}

void TIM_PrescalerConfig(TIM_TypeDef* TIMx, uint16_t Prescaler, uint16_t TIM_PSCReloadMode)
{
  /* Check the parameters */
  assert_param(IS_TIM_ALL_PERIPH(TIMx));
  assert_param(IS_TIM_PRESCALER_RELOAD(TIM_PSCReloadMode));
  /* Set the Prescaler value */
  TIMx->PSC = Prescaler;
  /* Set or reset the UG Bit */
  TIMx->EGR = TIM_PSCReloadMode;
}
