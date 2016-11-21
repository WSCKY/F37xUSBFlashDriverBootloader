/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Mass Storage demo main file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "hw_config.h" 
#include "usb_lib.h"
#include "usb_pwr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Flash memory address from where user application will be loaded */
#define ApplicationAddress 0x08005200

typedef  void (*pFunction)(void);
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
uint32_t TimeOut = 0;

pFunction Jump_To_Application;
uint32_t JumpAddress;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : main.
* Description    : Main routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int main(void)
{
	MAL_Config();
  Set_System();
  Set_USBClock();
  Led_Config();
  STM_EVAL_LEDOn(LED2);
  USB_Interrupts_Config();
  USB_Init();
  while((bDeviceState != CONFIGURED) && (TimeOut < 0x1FFFFF))
	{
//		TimeOut ++;
	}
if(TimeOut < 0x1FFFFF)
  USB_Configured_LED();
//else
//{
//	/* Test if user code is programmed starting from address 0x08005200 */
//    if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000)
//    { /* Jump to user application */

//      JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
//      Jump_To_Application = (pFunction) JumpAddress;
//      /* Initialize user application's Stack Pointer */
//      __set_MSP(*(__IO uint32_t*) ApplicationAddress);
//      Jump_To_Application();
//    }
//}

  while (1)
  {}
}

#ifdef USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
