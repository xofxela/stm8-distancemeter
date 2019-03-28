/**
  ******************************************************************************
  * @file    Project/main.c
  * @author  MCD Application Team
  * @version V2.3.0
  * @date    16-June-2017
  * @brief   Main program body
   ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
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
#include "stm8s.h"

#define LED_Port        GPIOA
#define LED_Pin         GPIO_PIN_1
#define BTN_PORT        GPIOC
#define BTN             GPIO_PIN_3

#define TIM4_PERIOD              250

/* Counter Delta = count_right - count_left */
/* TIM1 Period = 32767=0x00007FFF */

#define TIM1_COUNT_LEFT         (0x3FF1)
#define TIM1_COUNT_RIGHT        (0x3FF9)
#define TIM1_COUNT              (0x3FF5)

void clock_setup(void);
void GPIO_setup(void);
void TIM1_setup(void);
void TIM4_setup(void);
void EXTI_setup(void);
volatile uint16_t count, count_halt;



/* Private defines -----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void delay(uint32_t t)
{
    while(t--);
}

void main(void)
{
//  //Comment FLASH functions after programming device
//  FLASH_Unlock (FLASH_MEMTYPE_DATA);
//
//   //Activate AFR0 (Timer 1) and AFR3(TLI)
//  FLASH_ProgramOptionByte(0x4803, 0x09);
//
//  FLASH_Lock (FLASH_MEMTYPE_DATA);


  clock_setup();
  GPIO_setup();
  TIM1_setup();
  TIM4_setup();
  EXTI_setup();

  enableInterrupts();
  wfi();

  /* Infinite loop */
  while (1)
  {
    if(count_halt >= 50000)
    {
       TIM4_Cmd(DISABLE);
       TIM1_Cmd(DISABLE);
       count_halt = 0;
       CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1, DISABLE);
//       CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2, DISABLE);
       CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4, DISABLE);
       GPIO_Init(GPIOC, (GPIO_Pin_TypeDef) (GPIO_PIN_6 | GPIO_PIN_7), GPIO_MODE_OUT_OD_LOW_SLOW);
       halt();
    }
    wfi();
  }
}

void clock_setup(void)
{
  CLK_DeInit();
  CLK_HSECmd(DISABLE);
  CLK_LSICmd(ENABLE);
  CLK_HSICmd(DISABLE);
  while(CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == FALSE);
  CLK_ClockSwitchCmd(ENABLE);
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
  CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);
  CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_LSI, DISABLE,
                        CLK_CURRENTCLOCKSTATE_ENABLE);
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C, DISABLE);
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1, ENABLE);
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_SPI, DISABLE);
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, DISABLE);
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_AWU, DISABLE);
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART1, DISABLE);
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2, DISABLE);
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4, ENABLE);
}

void GPIO_setup(void)
{
  GPIO_DeInit(GPIOC);
  GPIO_Init(GPIOC, (GPIO_Pin_TypeDef) (GPIO_PIN_6 | GPIO_PIN_7), GPIO_MODE_IN_PU_NO_IT);
  GPIO_DeInit(GPIOA);
  GPIO_Init(LED_Port, LED_Pin, GPIO_MODE_OUT_OD_HIZ_FAST);
  GPIO_DeInit(GPIOD);
//  //Buzzer OFF
//  GPIO_WriteHigh(GPIOD, (GPIO_Pin_TypeDef)(GPIO_PIN_4));
//  GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_SLOW);
  GPIO_WriteHigh(GPIOD, (GPIO_Pin_TypeDef)(GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_5| GPIO_PIN_6));
  GPIO_Init(GPIOD, (GPIO_Pin_TypeDef)(GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_5| GPIO_PIN_6), GPIO_MODE_OUT_OD_HIZ_SLOW);

  //Pull Down Pins
  GPIO_Init(GPIOA, (GPIO_Pin_TypeDef)(GPIO_PIN_2 | GPIO_PIN_3), GPIO_MODE_OUT_OD_LOW_SLOW);
  GPIO_DeInit(GPIOB);
  GPIO_Init(GPIOB, (GPIO_Pin_TypeDef)(GPIO_PIN_4 | GPIO_PIN_5), GPIO_MODE_OUT_OD_LOW_SLOW);
  GPIO_Init(GPIOC, (GPIO_Pin_TypeDef)(GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5), GPIO_MODE_OUT_OD_LOW_SLOW);
  //Buzzer OFF
  GPIO_Init(GPIOD, (GPIO_Pin_TypeDef)(GPIO_PIN_4), GPIO_MODE_OUT_OD_LOW_SLOW);
  //User Button
  GPIO_DeInit(BTN_PORT);
  GPIO_Init(BTN_PORT, BTN, GPIO_MODE_IN_PU_IT);
}

void TIM1_setup(void)
{
  TIM1_DeInit();
  TIM1_TimeBaseInit(1, TIM1_COUNTERMODE_UP, 0xFFFF, 1);
  TIM1_EncoderInterfaceConfig(TIM1_ENCODERMODE_TI12,
                            TIM1_ICPOLARITY_RISING,
                            TIM1_ICPOLARITY_RISING);

  TIM1_CCxCmd(TIM1_CHANNEL_3, ENABLE);
  TIM1_CCxCmd(TIM1_CHANNEL_4, ENABLE);
  TIM1_ITConfig(TIM1_IT_CC3, ENABLE);
  TIM1_ITConfig(TIM1_IT_CC4, ENABLE);
  TIM1_SetCounter(TIM1_COUNT);
  TIM1_SetCompare3(TIM1_COUNT_LEFT);
  TIM1_SetCompare4(TIM1_COUNT_RIGHT);
  TIM1_Cmd(ENABLE);
}

void TIM4_setup(void)
{
  TIM4_TimeBaseInit(TIM4_PRESCALER_128, TIM4_PERIOD);
  TIM4_ARRPreloadConfig(ENABLE);
  TIM4->PSCR = 0xFF;
  TIM4->ARR = 249;
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
  TIM4_Cmd(ENABLE);
}

void EXTI_setup(void)
{
  EXTI_DeInit();
  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOC, EXTI_SENSITIVITY_FALL_LOW);
  EXTI_SetTLISensitivity(EXTI_TLISENSITIVITY_FALL_ONLY);
}
#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(u8* file, u32 line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
