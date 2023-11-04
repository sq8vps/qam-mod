#include "modulator.h"
#include "stm32f1xx.h"
#include "common.h"
#include <stdlib.h>

//16-QAM Gray code mapping
/*
0000    0100   |   1100		1000
			   |
			   |
			   |
0001	0101   |   1101		1001
			   |
----------------------------------
			   |
0011	0111   |   1111		1011
			   |
			   |
			   |
			   |
0010	0110   |   1110		1010
*/
static const uint8_t qam16[16] = {
		0b1001,
		0b0001,
		0b0011,
		0b1011,
		0b1101,
		0b0101,
		0b0111,
		0b1111,
		0b1100,
		0b0100,
		0b0110,
		0b1110,
		0b1000,
		0b0000,
		0b0010,
		0b1010,
};

//4-QAM/QPSK Gray code mapping
//BPSK mapping
/*
00             |			  10
			   |
			   |
			   |
			   |
			   |
----------------------------------
			   |
			   |
			   |
			   |
			   |
			   |
01			   |              11
*/
static const uint8_t qpsk[4] = {
		0b1001,
		0b0011,
		0b1100,
		0b0110,
};

//BPSK mapping
/*
0              |
			   |
			   |
			   |
			   |
			   |
----------------------------------
			   |
			   |
			   |
			   |
			   |
			   |
			   |              1
*/
static const uint8_t bpsk[2] = {
		0b1001,
		0b0110,
};


static enum Modulator mode = QAM16;
static uint8_t *lut = (uint8_t*)qam16;
static uint8_t mask = 0xF;
static uint32_t baudrate = 100;
static uint8_t (*callback)(void) = NULL;

void ModulatorInit(void)
{
	SET(RCC->APB2ENR,  RCC_APB2ENR_IOPAEN);
	SET(RCC->APB2ENR,  RCC_APB2ENR_IOPBEN);
	//PA3
	SET(GPIOA->CRL, GPIO_CRL_MODE3_0);
	RESET(GPIOA->CRL, GPIO_CRL_MODE3_1);
	RESET(GPIOA->CRL, GPIO_CRL_CNF3);
	//PA4
	SET(GPIOA->CRL, GPIO_CRL_MODE4_0);
	RESET(GPIOA->CRL, GPIO_CRL_MODE4_1);
	RESET(GPIOA->CRL, GPIO_CRL_CNF4);
	//PA5
	SET(GPIOA->CRL, GPIO_CRL_MODE5_0);
	RESET(GPIOA->CRL, GPIO_CRL_MODE5_1);
	RESET(GPIOA->CRL, GPIO_CRL_CNF5);
	//PA6
	SET(GPIOA->CRL, GPIO_CRL_MODE6_0);
	RESET(GPIOA->CRL, GPIO_CRL_MODE6_1);
	RESET(GPIOA->CRL, GPIO_CRL_CNF6);
	//PA7
	SET(GPIOA->CRL, GPIO_CRL_MODE7_0);
	RESET(GPIOA->CRL, GPIO_CRL_MODE7_1);
	RESET(GPIOA->CRL, GPIO_CRL_CNF7);
	//PB0
	SET(GPIOB->CRL, GPIO_CRL_MODE0_0);
	RESET(GPIOB->CRL, GPIO_CRL_MODE0_1);
	RESET(GPIOB->CRL, GPIO_CRL_CNF0);
	//PB1
	SET(GPIOB->CRL, GPIO_CRL_MODE1_0);
	RESET(GPIOB->CRL, GPIO_CRL_MODE1_1);
	RESET(GPIOB->CRL, GPIO_CRL_CNF1);
	//PB10
	SET(GPIOB->CRH, GPIO_CRH_MODE10_0);
	RESET(GPIOB->CRH, GPIO_CRH_MODE10_1);
	RESET(GPIOB->CRH, GPIO_CRH_CNF10);

	ModulatorPut(0);


	SET(RCC->APB2ENR, RCC_APB2ENR_TIM1EN);
	//run timer at 1 MHz, minimum baudrate is 15.26 Bd
	TIM1->PSC = 71;
	SET(TIM1->DIER, TIM_DIER_UIE);
	TIM1->ARR = 1000000 / baudrate;
	NVIC_EnableIRQ(TIM1_UP_IRQn);
}

void ModulatorPut(uint8_t value)
{
	value = lut[value & mask]; //map value to constellation point

	if(value & 1)
	{
		GPIOA->BSRR = GPIO_BSRR_BS3;
		GPIOA->BSRR = GPIO_BSRR_BR4;
	}
	else
	{
		GPIOA->BSRR = GPIO_BSRR_BR3;
		GPIOA->BSRR = GPIO_BSRR_BS4;
	}

	//"inner" outputs are actually swapped due to use of Butler matrix without output crossover
	if(value & 4)
	{
		GPIOA->BSRR = GPIO_BSRR_BS5;
		GPIOA->BSRR = GPIO_BSRR_BR6;
	}
	else
	{
		GPIOA->BSRR = GPIO_BSRR_BR5;
		GPIOA->BSRR = GPIO_BSRR_BS6;
	}

	if(value & 2)
	{
		GPIOA->BSRR = GPIO_BSRR_BS7;
		GPIOB->BSRR = GPIO_BSRR_BR0;
	}
	else
	{
		GPIOA->BSRR = GPIO_BSRR_BR7;
		GPIOB->BSRR = GPIO_BSRR_BS0;
	}

	if(value & 8)
	{
		GPIOB->BSRR = GPIO_BSRR_BS1;
		GPIOB->BSRR = GPIO_BSRR_BR10;
	}
	else
	{
		GPIOB->BSRR = GPIO_BSRR_BR1;
		GPIOB->BSRR = GPIO_BSRR_BS10;
	}
}

void ModulatorSetMode(enum Modulator m)
{
	mode = m;
	switch(m)
	{
		case QAM4:
			mask = 0x3;
			lut = (uint8_t*)qpsk;
			break;
		case BPSK:
			mask = 0x1;
			lut = (uint8_t*)bpsk;
			break;
		default:
		case QAM16:
			mask = 0xF;
			lut = (uint8_t*)qam16;
			break;
	}
}

uint32_t ModulatorSetBaudrate(uint32_t baud)
{
	//assume the timer runs at 1 MHz
	uint32_t arr = 1000000 / baud;
	TIM1->ARR = arr;
	baudrate = arr * 1000000;
	return baudrate;
}

void ModulatorStartTimer(uint8_t (*symbolCallback)(void))
{
	if(NULL == symbolCallback)
	{
		RESET(TIM1->CR1, TIM_CR1_CEN);
		return;
	}
	callback = symbolCallback;
	SET(TIM1->CR1, TIM_CR1_CEN);
}

void ModulatorStop(void)
{
	RESET(TIM1->CR1, TIM_CR1_CEN);
}

static uint8_t randomCallback(void)
{
	return rand() % (mask + 1);
}

void ModulatorStartRandom(void)
{
	ModulatorStartTimer(&randomCallback);
}

void TIM1_UP_IRQHandler() __attribute__((interrupt));
void TIM1_UP_IRQHandler()
{
	RESET(TIM1->SR, TIM_SR_UIF);
	if(NULL != callback)
		ModulatorPut(callback());
}
