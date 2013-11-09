#include <public.h>
#include <delay.h>

#include "comm.h"
#include "hardware.h"

extern uint8_t hsvData[][3];

volatile uint32_t ticks = 0;
int speed = 10;

unsigned int holdrand;
void srand (unsigned int seed)
{
	holdrand = (long)seed;
}
int rand ()
{
	return (((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}
int randminmax (int min, int max)
{
	return rand () % (max - min) + min;
}

void myputchar (int c)
{
	USART1->TDR = c;
	while (!(USART1->ISR & USART_ISR_TC));
}

int main ()
{
	SYSCFG->CFGR1 = 0;

	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN | RCC_APB2ENR_SYSCFGEN | RCC_APB2ENR_USART1EN | RCC_APB2ENR_ADC1EN;
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN;
	
	SysTick->LOAD = SysTick->VAL = (F_CPU / 1000) / 8;
	SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;

	_delay_init ();

	ADC1->CR = ADC_CR_ADEN;
	while (!(ADC1->ISR & ADC_ISR_ADRDY));
	ADC->CCR = ADC_CCR_VREFEN;

	uint32_t adcSeed;
	int i, j;
	for (j = 0; j < 100; j++)
	{
		uint32_t tmp = 0;
		for (i = 0; i < 32; i++)
		{
			ADC1->CR = ADC_CR_ADSTART;
			while (!(ADC1->ISR & ADC_ISR_EOC));
			uint32_t d = ADC1->DR;
			tmp <<= 1;
			if (d & 1) tmp |= 1;
		}
		adcSeed ^= tmp;
	}
	if (adcSeed == 0) adcSeed = 1;
	if (adcSeed == 0xffffffff) adcSeed = 1;
	srand (adcSeed);

	uint16_t *mem = (uint16_t*)(0x08000000 + (16 - 1) * 1024);

	if (*mem == 0xffff)
	{
		FLASH->CR |= FLASH_CR_LOCK;
		FLASH->KEYR = 0x45670123;
		FLASH->KEYR = 0xCDEF89AB;
		while (FLASH->CR & FLASH_CR_LOCK);

		FLASH->CR = FLASH_CR_PG;

		mykey = randminmax (0, 360);
		*mem = mykey;
		while (FLASH->SR & FLASH_SR_BSY);
		FLASH->CR |= FLASH_CR_LOCK;
	}
	else
	{
		mykey = *mem;
	}

	ADC1->CR = ADC_CR_ADDIS;

	ticks = rand ();
	speed = randminmax (3, 13);

	IO_ALT_PUSH_PULL(LEDR);
	IO_ALT_PUSH_PULL(LEDG);
	IO_ALT_PUSH_PULL(LEDB);
	IO_ALT_SET(LEDR, 1);
	IO_ALT_SET(LEDG, 1);
	IO_ALT_SET(LEDB, 1);

	TIM3->CNT = 0;
	TIM3->PSC = 512;
	TIM3->ARR = 256;
	TIM3->BDTR = TIM_BDTR_MOE;
	TIM3->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;
	TIM3->CCMR2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1;
	TIM3->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
	TIM3->CR1 = TIM_CR1_CEN | TIM_CR1_ARPE;

	// for(;;)
	// {
	// TIM3->CCR1 = TIM3->CCR2 = TIM3->CCR4 = 0;
	// TIM3->CCR1 = 255;
	// _delay_ms(4000);
	// TIM3->CCR1 = TIM3->CCR2 = TIM3->CCR4 = 0;
	// TIM3->CCR2 = 255;
	// _delay_ms(4000);
	// TIM3->CCR1 = TIM3->CCR2 = TIM3->CCR4 = 0;
	// TIM3->CCR4 = 255;
	// _delay_ms(4000);
	// }

	commInit ();

	for (;;)
	{
		if (!commIsLocked)
			showColor (ticks / speed);

		commProcess ();
	}
}
void showColor (uint16_t u)
{
	uint8_t *val = hsvData[u % 360];
	TIM3->CCR1 = val[0];
	TIM3->CCR2 = val[1];
	TIM3->CCR4 = val[2];
}
void _errorloop ()
{
	IO_PUSH_PULL(LEDR);
	IO_PUSH_PULL(LEDG);
	IO_PUSH_PULL(LEDB);
	IO_LOW(LEDG);
	IO_LOW(LEDB);
	while (1)
	{
		IO_TOGGLE(LEDR);
		_delay_ms (100);
	}
}
void SysTick_Handler ()
{
	ticks++;
}
