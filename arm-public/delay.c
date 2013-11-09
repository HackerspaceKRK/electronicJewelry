#ifndef __DELAY_H__
#define __DELAY_H__

#include "public.h"
#include "delay.h"
#include "settings.h"

void _delay_100us (uint16_t d)
{
	DELAY_TIMER->PSC = (F_CPU / 10000) - 1;
	DELAY_TIMER->EGR = TIM_EGR_UG;
	while (!(DELAY_TIMER->SR & TIM_SR_UIF));
	DELAY_TIMER->SR = 0;
	DELAY_TIMER->CNT = 0x10000 - d;
	DELAY_TIMER->CR1 = TIM_CR1_CEN | TIM_CR1_OPM;
	while (!(DELAY_TIMER->SR & TIM_SR_UIF));
	DELAY_TIMER->SR = 0;
}
void _delay_ms (uint16_t d)
{
	DELAY_TIMER->PSC = (F_CPU / 1000) - 1;
	DELAY_TIMER->EGR = TIM_EGR_UG;
	while (!(DELAY_TIMER->SR & TIM_SR_UIF));
	DELAY_TIMER->SR = 0;
	DELAY_TIMER->CNT = d;
	DELAY_TIMER->CR1 = TIM_CR1_CEN | TIM_CR1_OPM | TIM_CR1_DIR;
	while (!(DELAY_TIMER->SR & TIM_SR_UIF));
	DELAY_TIMER->SR = 0;
}
void _delay_us (uint16_t d)
{
	DELAY_TIMER->PSC = (F_CPU / 1000000) - 1;
	DELAY_TIMER->EGR = TIM_EGR_UG;
	while (!(DELAY_TIMER->SR & TIM_SR_UIF));
	DELAY_TIMER->SR = 0;
	DELAY_TIMER->CNT = 0x10000 - d;
	DELAY_TIMER->CR1 = TIM_CR1_CEN | TIM_CR1_OPM;
	while (!(DELAY_TIMER->SR & TIM_SR_UIF));
	DELAY_TIMER->CR1 = 0;
}

#endif
