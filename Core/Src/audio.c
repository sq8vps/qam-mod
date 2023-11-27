/*
Copyright 2023 Piotr Wilkon
This file is part of QAM Modulator Controller.

QAM Modulator Controller is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

QAM Modulator Controller is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QAM Modulator Controller.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "audio.h"
#include "stm32f1xx.h"
#include "common.h"





#define AUDIO_DMA_BUFFER_SIZE 32
static uint16_t dmaBuffer[AUDIO_DMA_BUFFER_SIZE];
static int16_t buffer[AUDIO_DMA_BUFFER_SIZE];

static void (*callback)(int16_t*, uint32_t) = NULL;

//#define FILTER_TAPS 7
//
//struct Filter
//{
//	int16_t coeffs[FILTER_TAPS];
//	int16_t samples[FILTER_TAPS];
//	uint8_t gainShift;
//
//} *lpf = NULL;
//
//static struct Filter fil22050 = {
//		.coeffs = {
//				-736,
//		        -1692,
//		         8983,
//		        19660,
//		         8983,
//		        -1692,
//			},
//		.gainShift = 15,
//};
//
//static struct Filter fil11025 = {
//		.coeffs = {
//		         1004,
//		         5542,
//		        15314,
//		        21813,
//		        15314,
//		         5542,
//		         1004
//			},
//		.gainShift = 16,
//};

//static enum AudioSampleRate sr = AUDIO_44100;
//
//static int16_t filter(int16_t sample)
//{
//	if(NULL == lpf)
//		return sample;
//
//    int32_t out = 0;
//
//    for(uint8_t n = FILTER_TAPS - 1; n > 0; n--)
//       lpf->samples[n] = lpf->samples[n - 1];
//
//    lpf->samples[0] = sample;
//    for(uint8_t n = 0; n < FILTER_TAPS; n++)
//        out += lpf->coeffs[n] * lpf->samples[n];
//
//    return out >> lpf->gainShift;
//}

void AudioInit(void)
{
	SET(RCC->APB2ENR,RCC_APB2ENR_IOPAEN);
	SET(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
	SET(RCC->AHBENR, RCC_AHBENR_DMA1EN);

	/* ADC input: PA2 */
	RESET(GPIOA->CRL, GPIO_CRL_CNF2);
	RESET(GPIOA->CRL, GPIO_CRL_MODE2);
	/*/6 prescaler */
	SET(RCC->CFGR, RCC_CFGR_ADCPRE_1);
	RESET(RCC->CFGR, RCC_CFGR_ADCPRE_0);
	SET(ADC1->CR2, ADC_CR2_CONT);
	SET(ADC1->CR2, ADC_CR2_EXTSEL);
	RESET(ADC1->SQR1, ADC_SQR1_L);
	/* 41.5 cycle sampling */
	SET(ADC1->SMPR2, ADC_SMPR2_SMP0_2);
	/* Channel 2 */
	RESET(ADC1->SQR3, ADC_SQR3_SQ1);
	SET(ADC1->SQR3, ADC_SQR3_SQ1_1);

	SET(ADC1->CR2, ADC_CR2_ADON);
	/* calibrate */
	SET(ADC1->CR2, ADC_CR2_RSTCAL);
	while(ADC1->CR2 & ADC_CR2_RSTCAL)
		;
	SET(ADC1->CR2, ADC_CR2_CAL);
	while(ADC1->CR2 & ADC_CR2_CAL)
		;
	SET(ADC1->CR2, ADC_CR2_EXTTRIG);
	SET(ADC1->CR2, ADC_CR2_SWSTART);


	/* 16 bit memory region */
	SET(DMA1_Channel2->CCR, DMA_CCR_MSIZE_0);
	RESET(DMA1_Channel2->CCR, DMA_CCR_MSIZE_1);
	SET(DMA1_Channel2->CCR, DMA_CCR_PSIZE_0);
	RESET(DMA1_Channel2->CCR, DMA_CCR_PSIZE_1);
	/* enable memory pointer increment, circular mode and interrupt generation */
	SET(DMA1_Channel2->CCR, DMA_CCR_MINC | DMA_CCR_CIRC| DMA_CCR_TCIE);
	DMA1_Channel2->CNDTR = AUDIO_DMA_BUFFER_SIZE;
	DMA1_Channel2->CPAR = (uintptr_t)&(ADC1->DR);
	DMA1_Channel2->CMAR = (uintptr_t)dmaBuffer;
	SET(DMA1_Channel2->CCR, DMA_CCR_EN);

	/* 72 / 4 = 23 MHz */
	TIM2->PSC = 3;
	/* 23 MHz / 408 = 44118 Hz, which is probably the closest we can get */
	TIM2->ARR = 407;
	/* enable DMA call instead of standard interrupt */
	SET(TIM2->DIER, TIM_DIER_UDE);

	NVIC_EnableIRQ(DMA1_Channel2_IRQn);
}

void AudioStart(void (*sampleCallback)(int16_t*, uint32_t))
{
	callback = sampleCallback;
	SET(TIM2->CR1, TIM_CR1_CEN);
}

void AudioStop(void)
{
	RESET(TIM2->CR1, TIM_CR1_CEN);
}

void DMA1_Channel2_IRQHandler() __attribute__ ((interrupt));
void DMA1_Channel2_IRQHandler()
{
	for(uint16_t i = 0; i < AUDIO_DMA_BUFFER_SIZE; i++)
		buffer[i] = (int16_t)dmaBuffer[i] - 2047;
	if(NULL != callback)
		callback(buffer, AUDIO_DMA_BUFFER_SIZE);
	SET(DMA1->IFCR, DMA_IFCR_CTCIF2);
}
