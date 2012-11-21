/**
  ******************************************************************************
  * @file    IO_Toggle/main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************  
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4_discovery.h"

/** @addtogroup STM32F4_Discovery_Peripheral_Examples
  * @{
  */

/** @addtogroup IO_Toggle
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Delay(__IO uint32_t nCount);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
#include "kernel.h"
#include "types.h"
#include "config.h"
#include "debug.h"
#include "stm32f4xx_adc.h"
#include "loader.h"
#include "uart.h"
#include "nvmfile.h"
#include "vm.h"
#include "native_impl.h"




int Maintread_invoke(void *args)
{
//	Task_yield();
#ifdef NATIVE_INIT
  NATIVE_INIT;
#endif
//  debug_enable(FALSE);  
	
  nvmfile_init();
	
  vm_init();
	
  nvmfile_call_main();
	
  for(;;);  // reset wdt if in use	
	
	return 0;
}

void kernel_main(void)
{
	int i;
	//printf("\nStart SM RTOS kernel...............\n",cpu_id);    

	Init_scheduler();
	Init_idle_task();
	for(i=0;i<15;i++)
		MutexInit(&mutex_key[i],CONFIG_MAX_PRIORITY - 1);
}

int main(void)
{  
	kernel_main();

	Task_init(&MainThread, Maintread_invoke, 0, CONFIG_MAX_PRIORITY - 1,MainThread_stack, sizeof(MainThread_stack));
	Task_start(&MainThread);
//	Task_yield();
	for(;;);  // reset wdt if in use
}

/**
  * @brief  Delay Function.
  * @param  nCount:specifies the Delay time length.
  * @retval None
  */
void Delay(__IO uint32_t nCount)
{
  while(nCount--)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
