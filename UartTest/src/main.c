/**
  ******************************************************************************
  * @file    ADC_DMA/main.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    11-November-2013
  * @brief   This example describes how to use the ADC and DMA to transfer 
  *          continuously converted data from ADC to memory.
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
#include "main.h"
#include "halFunct.h"


/** @addtogroup STM32F429I_DISCOVERY_Examples
  * @{
  */

/** @addtogroup ADC_DMA
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/ 
  #define FONTSIZE         Font12x12
  #define ADC3_DR_ADDRESS     ((uint32_t)0x4001224C)
  #define BUFFERSIZE	320
#define DAC_DHR12R2_ADDRESS    0x40007414
#define DAC_DHR8R1_ADDRESS     0x40007410
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* You can monitor the converted value by adding the variable "uhADC3ConvertedValue" 
   to the debugger watch window */
__IO uint16_t adcBuffer[BUFFERSIZE]; //Buffer de muestras
uint8_t UsarTxXBuffer[BUFFERSIZE*2]; //Buffer de muestras
__IO uint8_t UsarRxXBuffer[BUFFERSIZE]; //Buffer de muestras

Point 	pointBuffer[BUFFERSIZE];	 //Buffer de puntos para gráficar
pPoint  pPoints=(pPoint)pointBuffer;

__IO uint8_t KeyPressed = SET;
__IO uint8_t intFlag=0, process=0;
DAC_InitTypeDef  DAC_InitStructure;

const uint16_t Sine12bit[128] = {2048,2138,2228,2318,2407,2495,2582,2668,2753,2835,2916,2995,
		3071,3145,3217,3285,3351,3413,3472,3528,3580,3628,3673,3713,3750,3783,3811,3835,3855,
		3870,3881,3888,3890,3888,3881,3870,3855,3835,3811,3783,3750,3713,3673,3628,3580,3528,3472,
		3413,3351,3285,3217,3145,3071,2995,2916,2835,2753,2668,2582,2495,2407,2318,2228,2138,2048,1957,
		1867,1777,1688,1600,1513,1427,1342,1260,1179,1100,1024,950,878,810,744,682,623,567,515,467,422,
		382,345,312,284,260,240,225,214,207,205,207,214,225,240,260,284,312,345,382,422,467,515,567,623,
		682,744,810,878,950,1024,1100,1179,1260,1342,1427,1513,1600,1688,1777,1867,1957}; //Sine wave

/* Private function prototypes -----------------------------------------------*/

#ifdef USE_LCD
static void Display_Init(void);
static void ADC3_CH13_DMA_Config(void);
#endif /* USE_LCD */
/* Private functions ---------------------------------------------------------*/



void TIM6_Config(void)
{
  /* TIM6CLK = HCLK / 4 = SystemCoreClock /4 */

  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
  /* TIM6 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

  /* Time base configuration */
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period = 0x2F;
  TIM_TimeBaseStructure.TIM_Prescaler = TIM_ICPSC_DIV1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV4;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

  /* TIM6 TRGO selection */
  TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);

  /* TIM6 enable counter */
  TIM_Cmd(TIM6, ENABLE);
}


void DAC_Ch2_SineWaveConfig(void)
{
  DMA_InitTypeDef DMA_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* DMA1 clock and GPIOA clock enable (to be used with DAC) */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 | RCC_AHB1Periph_GPIOA, ENABLE);

  /* DAC Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

  /* DAC channel 1 & 2 (DAC_OUT1 = PA.4)(DAC_OUT2 = PA.5) configuration */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* DAC channel2 Configuration */
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_2, &DAC_InitStructure);

  /* DMA1_Stream5 channel7 configuration **************************************/
  DMA_DeInit(DMA1_Stream5);
  DMA_InitStructure.DMA_Channel = DMA_Channel_7;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&DAC->DHR12R2);
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&Sine12bit;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = 128;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream6, &DMA_InitStructure);

  /* Enable DMA1_Stream5 */
  DMA_Cmd(DMA1_Stream6, ENABLE);

  /* Enable DAC Channel2 */
  DAC_Cmd(DAC_Channel_2, ENABLE);

  /* Enable DMA for DAC Channel2 */
  DAC_DMACmd(DAC_Channel_2, ENABLE);
}


/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

static void delay(__IO uint32_t nCount)
{
  __IO uint32_t index = 0;
  for(index = nCount; index != 0; index--)
  {
  }
}

int main(void)
{
#ifdef USE_LCD 
  /* LCD Display init  */
  Display_Init();
#endif /* USE_LCD */
  TIM6_Config();

  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);
  DAC_Ch2_SineWaveConfig();
  USART_Config();
  /* ADC3 configuration *******************************************************/
  /*  - Enable peripheral clocks                                              */
  /*  - DMA2_Stream0 channel2 configuration                                   */
  /*  - Configure ADC Channel13 pin as analog input                           */
  /*  - Configure ADC3 Channel13                                              */

  uint16_t i;
for(i=0;i<BUFFERSIZE;i++){
	pPoints++->Y=i;  //eje de tiempo en la Y, dimension mas larga de la pantalla
}
pPoints=(pPoint)pointBuffer;
ADC3_CH13_DMA_Config();
TIM2_Config();

while (1)
  {
	// Schmitt trigger
	ADC_ContinuousModeCmd(ADC3, ENABLE);
	ADC_SoftwareStartConv(ADC3);
	schmitt_trigger(2000, 50);
	ADC_ContinuousModeCmd(ADC3, DISABLE);
	delay(10000);
	ADC_DMACmd(ADC3, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
	intFlag=0;
    while(intFlag==0){} //Esperar interrupcion del DMA
	LCD_SetTextColor(LCD_COLOR_BLACK); //Seleccionar pincel negr
	LCD_PolyLine(pPoints, BUFFERSIZE); //Borrar curva (dibujar en negro
	for(i=0;i<BUFFERSIZE;i++){
  			pPoints++->X=(adcBuffer[i]/18);//Asignar el valor del ADC/18 a la curva en pantall
  			UsarTxXBuffer[i*2+1]=adcBuffer[i]&0xFF;
  			UsarTxXBuffer[i*2]=adcBuffer[i]>>8;
	} 
	pPoints=(pPoint)pointBuffer; //Apuntar al primer par X,  /		/* Display ADCs converted values on LCD *
	LCD_SetTextColor(LCD_COLOR_YELLOW); //seleccionar pincel amarill
	LCD_PolyLine((pPoint)pointBuffer, BUFFERSIZE); //graficar cu
	sendBlockDMA(UsarTxXBuffer, BUFFERSIZE*2);
  }
}

/**
  * @brief  Display Init (LCD)
  * @param  None
  * @retval None
  */
static void Display_Init(void)
{
  /* Initialize the LCD */
  LCD_Init();
  LCD_LayerInit();
  /* Enable the LTDC */
  LTDC_Cmd(ENABLE);

  /* Set LCD Background Layer  */
  LCD_SetLayer(LCD_BACKGROUND_LAYER);

  /* Clear the Background Layer */
  LCD_Clear(LCD_COLOR_BLACK);

  /* Configure the transparency for background */
  LCD_SetTransparency(0);

  /* Set LCD Foreground Layer  */
  LCD_SetLayer(LCD_FOREGROUND_LAYER);

  /* Configure the transparency for foreground */
  LCD_SetTransparency(255);

  /* Clear the Foreground Layer */
  LCD_Clear(LCD_COLOR_BLACK);

  /* Set the LCD Back Color and Text Color*/
  LCD_SetBackColor(LCD_COLOR_BLUE);
  LCD_SetTextColor(LCD_COLOR_WHITE);

    /* Set the LCD Text size */
  LCD_SetFont(&FONTSIZE);

  /* Set the LCD Back Color and Text Color*/
  LCD_SetBackColor(LCD_COLOR_WHITE);
  LCD_SetTextColor(LCD_COLOR_YELLOW);
  LCD_Clear(LCD_COLOR_BLACK);
}

/**
  * @brief  ADC3 channel13 with DMA configuration
  * @param  None
  * @retval None
  */
static void ADC3_CH13_DMA_Config(void)
{
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;

  /* Enable ADC3, DMA2 and GPIO clocks ****************************************/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

  /* DMA2 Stream0 channel2 configuration **************************************/
  DMA_InitStructure.DMA_Channel = DMA_Channel_2;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&ADC3->DR);
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)adcBuffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = BUFFERSIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);
  DMA_Cmd(DMA2_Stream0, ENABLE);

  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);

  /* Configure ADC3 Channel13 pin as analog input ******************************/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC3 Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC3, &ADC_InitStructure);

  /* ADC3 regular channel13 configuration *************************************/
  ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 1, ADC_SampleTime_3Cycles);

 /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);

  /* Enable ADC3 DMA */
  ADC_DMACmd(ADC3, ENABLE);

  /* Enable ADC3 */
  ADC_Cmd(ADC3, ENABLE);

  /* Configure Interrupt */
  /* Enable and set DMA Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
