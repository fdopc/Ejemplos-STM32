
#include <halFunct.h>

void USART_Config(void)
{
	USART_InitTypeDef USART_InitStructure;
	  GPIO_InitTypeDef GPIO_InitStructure;
	  DMA_InitTypeDef  DMA_InitStructure;
	 // NVIC_InitTypeDef   NVIC_InitStructure;

	  /* Peripheral Clock Enable -------------------------------------------------*/
	  /* Enable GPIO clock */
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	  /* Enable USART clock */
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	  /* Enable the DMA clock */
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	  /* USARTx GPIO configuration -----------------------------------------------*/
	  /* Connect USART pins to AF7 */
	  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	  /* Configure USART Tx and Rx as alternate function push-pull */
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	  GPIO_Init(GPIOA, &GPIO_InitStructure);

	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	  GPIO_Init(GPIOA, &GPIO_InitStructure);

	  /* USARTx configuration ----------------------------------------------------*/
	  /* Enable the USART OverSampling by 8 */
	  USART_OverSampling8Cmd(USART1, ENABLE);

	  USART_InitStructure.USART_BaudRate = 1800000;
	  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	  USART_InitStructure.USART_StopBits = USART_StopBits_1;
	  /* When using Parity the word length must be configured to 9 bits */
	  USART_InitStructure.USART_Parity = USART_Parity_No;
	  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	  USART_Init(USART1, &USART_InitStructure);

	  /* Configure DMA controller to manage USART TX and RX DMA request ----------*/

	  /* Configure DMA Initialization Structure */
	  DMA_InitStructure.DMA_BufferSize = 10 ;
	  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
	  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
	  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
	  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	  DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(USART1->DR)) ;
	  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	  /* Configure TX DMA */
	  DMA_InitStructure.DMA_Channel = DMA_Channel_4 ;
	  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
	//  DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)UsarTxXBuffer;
	  DMA_Init(DMA2_Stream7,&DMA_InitStructure);
	  /* Configure RX DMA */
	  DMA_InitStructure.DMA_Channel = DMA_Channel_4 ;
	  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;
	 // DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)UsarTxXBuffer;
	  DMA_Init(DMA2_Stream5,&DMA_InitStructure);

	  USART_Cmd(USART1, ENABLE);
}



void sendBlockDMA(uint8_t *data, uint16_t size){
USART_SendData(USART1, '#');
	while(!USART_GetFlagStatus(USART1, USART_FLAG_TC));
	USART_SendData(USART1, 's');
	while(!USART_GetFlagStatus(USART1, USART_FLAG_TC));
	USART_SendData(USART1, 't');
	while(!USART_GetFlagStatus(USART1, USART_FLAG_TC));
  /* Enable DMA USART TX Stream */
	DMA_Cmd(DMA2_Stream7,DISABLE);;
	DMA2_Stream7->M0AR=(uint32_t)data;
	DMA2_Stream7->NDTR=size;
    DMA_Cmd(DMA2_Stream7,ENABLE);
	/* Enable USART DMA TX Requsts */
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    /* Waiting the end of Data transfer */
	while (USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);
	while (DMA_GetFlagStatus(DMA2_Stream7,DMA_FLAG_TCIF7)==RESET);

	  /* Clear DMA Transfer Complete Flags */
	DMA_ClearFlag(DMA2_Stream7,DMA_FLAG_TCIF7);
	  /* Clear USART Transfer Complete Flags */
	USART_ClearFlag(USART1,USART_FLAG_TC);
	USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);

	//COnventional polling transmission
	/*for(i=0;i<BUFFERSIZE;i++){
			USART_SendData(USART1, (uint16_t)adcBuffer[i]>>4);
			while(!USART_GetFlagStatus(USART1, USART_FLAG_TC));
	}
	*/
	USART_SendData(USART1, '\r');
	while(!USART_GetFlagStatus(USART1, USART_FLAG_TC));
	USART_SendData(USART1, '\n');
}

void schmitt_trigger(uint16_t level, uint16_t hyst){
uint16_t val=0;
while(val<level+hyst){
	while(ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC) == RESET);
	val=ADC_GetConversionValue(ADC3);
}
while(val>level){
	while(ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC) == RESET);
	val=ADC_GetConversionValue(ADC3);
}
while(val>level-hyst){
	while(ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC) == RESET);
	val=ADC_GetConversionValue(ADC3);
}
}

void TIM2_Config(void)
{
  /* TIM6CLK = HCLK / 4 = SystemCoreClock /4 */

  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
  /* TIM6 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  /* Time base configuration */
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period = 0x2F;
  TIM_TimeBaseStructure.TIM_Prescaler = TIM_ICPSC_DIV1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV4;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  /* TIM6 TRGO selection */
  TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
  TIM_GenerateEvent(TIM2, TIM_EventSource_Trigger);

  /* TIM6 enable counter */
 }

