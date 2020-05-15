#include "stm32f4xx.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_adc.h"

void USART_Config(void);
void sendBlockDMA(uint8_t *data, uint16_t size);
void schmitt_trigger(uint16_t level, uint16_t hyst);
void TIM2_Config(void);
