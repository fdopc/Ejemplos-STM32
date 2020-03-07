/**
  ******************************************************************************
  * @file    ADC3_DMA/stm32f4xx_it.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
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
#include "stm32f4xx_it.h"
#include "CustomDefines.h"


/** @addtogroup STM32F4_Discovery_Peripheral_Examples
  * @{
  */

/** @addtogroup ADC_ADC3_DMA
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */

#define ThresVal   1024
extern __IO uint8_t ADC3ConvertedValue[BUFFER_SIZE*2];
void DMA2_Stream0_IRQHandler(void)
{
	//if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0)==SET)
//	{
	    ADC_DMACmd(ADC3, DISABLE);
	    ADC_Cmd(ADC3, DISABLE);
		ADC_DMARequestAfterLastTransferCmd(ADC3, DISABLE);

		SendBlock();

		ADC_Cmd(ADC3, ENABLE);
		ADC_SoftwareStartConv(ADC3);
		while(ADC_GetSoftwareStartConvStatus(ADC3)==SET);
	//	while((ADC3->DR)<ThresVal);
		//while((ADC3->DR)>ThresVal+100);
	//	while((ADC3->DR)>ThresVal+300);

		ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);
		ADC_DMACmd(ADC3, ENABLE);

		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
//	}
}

void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE)==SET)
	{
	//	USART_SendData(USART1, USART1->DR);
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}

void SendBlock(void)
{
	uint16_t i;
	//uint8_t *Data = (uint8_t *)ADC3ConvertedValue[0];
//	uint16_t HighByte, LowByte;
	for(i=24;i<BUFFER_SIZE*2;i++)
	{
	//	LowByte = (ADC3ConvertedValue[i]&0xFF);
	//	HighByte = (ADC3ConvertedValue[i]>>8);
	//	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)==RESET);
	//	USART_SendData(USART1, LowByte);
	//	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)==RESET);
	//	USART_SendData(USART1, HighByte );
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)==RESET);
		USART_SendData(USART1, ADC3ConvertedValue[i]);
	}
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)==RESET);
	USART_SendData(USART1, (uint16_t)('\r'));
	//
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)==RESET);
	USART_SendData(USART1, (uint16_t)('\n'));
	//while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET);
}
