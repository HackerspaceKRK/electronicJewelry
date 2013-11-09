#ifndef __PUBLIC_L_H__
#define __PUBLIC_L_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stm32f0xx.h"

extern volatile uint32_t ticks;

void _errorloop () __attribute__ ((noreturn));

#define SWAP16(x) (((x & 0x00ff) << 8) | ((x & 0xff00) >> 8))

#define BITBAND_SYSTEM(addr,bit) (uint32_t*)(0x42000000 + ((uint32_t)(addr) - 0x40000000) * 32 + (bit) * 4)

// IO
#define PORTCR_FROM_PORTPIN(port,pin) *(uint32_t*)((uint32_t)port + 0x04 * (pin / 8))

#define IO_SPEED_400kHZ 0b00
#define IO_SPEED_2MHZ   0b01
#define IO_SPEED_10MHZ  0b10
#define IO_SPEED_50MHZ  0b11
static void IO_SET_MODE_TYPE_PP (GPIO_TypeDef* port, int pin, uint32_t mode, uint32_t type, uint32_t pp)
{
	port->MODER &= ~(0b11 << ((pin) * 2));
	port->MODER |= mode << ((pin) * 2);

	port->OTYPER &= ~(0b1 << pin);
	port->OTYPER |= type << pin;

	port->PUPDR &= ~(0b11 << ((pin) * 2));
	port->PUPDR |= pp << ((pin) * 2);

	port->OSPEEDR &= ~(0b11 << ((pin) * 2));
	port->OSPEEDR |= IO_SPEED_400kHZ << ((pin) * 2);
}
static void IO_SET_MODE (GPIO_TypeDef* port, int pin, uint32_t mode)
{
	port->MODER &= ~(0b11 << ((pin) * 2));
	port->MODER |= mode << ((pin) * 2);
}

static inline void IO_PULLUP (GPIO_TypeDef* port, int pin)
{
	port->PUPDR &= ~(0b11 << ((pin) * 2));
	port->PUPDR |= 0b01 << ((pin) * 2);
}
static inline void IO_PULLDOWN (GPIO_TypeDef* port, int pin)
{
	port->PUPDR &= ~(0b11 << ((pin) * 2));
	port->PUPDR |= 0b10 << ((pin) * 2);
}
static inline void IO_PULLNONE(GPIO_TypeDef* port, int pin)
{
	port->PUPDR &= ~(0b11 << ((pin) * 2));
}
static inline void IO_PUSH_PULL (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b01, 0, 0b00);
}
static inline void IO_PUSH_PULL_MODE (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE (port, pin, 0b01);
}
static inline void IO_OPEN_DRAIN (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b01, 1, 0b00);
}
static inline void IO_ALT_PUSH_PULL (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b10, 0, 0b00);
}
static inline void IO_ALT_PUSH_PULL_MODE (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE (port, pin, 0b10);
}
static inline void IO_ALT_OPEN_DRAIN (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b10, 1, 0b00);
}
static inline void IO_ALT_SET (GPIO_TypeDef* port, int pin, uint32_t AF)
{
	port->AFR[pin / 8] &= ~((0b1111) << ((pin % 8) * 4));
	port->AFR[pin / 8] |= AF << ((pin % 8) * 4);
}
static inline void IO_ANALOG (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b11, 0, 0b00);
}
static inline void IO_INPUT (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b00, 0, 0b00);
}
static inline void IO_INPUT_PU (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b00, 0, 0b01);
}
static inline void IO_INPUT_PD (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b00, 0, 0b10);
}

static inline void IO_HIGH (GPIO_TypeDef* port, int pin)
{
	// port->ODR |= (1 << pin);
	
	//volatile uint32_t *addr = (uint32_t*)(0x42000000 + ((uint32_t)&port->ODR - 0x40000000) * 32 + pin * 4);
	//*addr = 1;
	
	port->BSRR = 1 << pin;
}
static inline void IO_LOW (GPIO_TypeDef* port, int pin)
{
	// port->ODR &= ~(1 << pin);
	
	//volatile uint32_t *addr = (uint32_t*)(0x42000000 + ((uint32_t)&port->ODR - 0x40000000) * 32 + pin * 4);
	//*addr = 0;
	
	port->BSRR = 1 << (pin + 16);
}
static inline void IO_TOGGLE (GPIO_TypeDef* port, int pin)
{
	port->ODR ^= (1 << pin);

	// volatile uint32_t *addr = (volatile uint32_t*)(0x42000000 + ((uint32_t)&port->ODR - 0x40000000) * 32 + pin * 4);
	// *addr = *addr ^ 1;
}

static inline uint8_t IO_IS_HIGH (GPIO_TypeDef* port, int pin)
{
	return (port->IDR & (1 << pin)) ? 1 : 0;
	// uint32_t *addr = (uint32_t*)(0x42000000 + ((uint32_t)&port->IDR - 0x40000000) * 32 + pin * 4);
	// return *addr == 0 ? 0 : 1;
}
static inline uint8_t IO_IS_LOW (GPIO_TypeDef* port, int pin)
{
	return (port->IDR & (1 << pin)) == 0;
	// uint32_t *addr = (uint32_t*)(0x42000000 + ((uint32_t)&port->IDR - 0x40000000) * 32 + pin * 4);
	// return *addr == 0 ? 1 : 0;
}

// Interrupts
#define ENABLE_INTERRUPT(x) NVIC->ISER[(x) / 32] = 1 << ((x) % 32)
#define DISABLE_INTERRUPT(x) NVIC->ICER[(x) / 32] = 1 << ((x) % 32)

// USART
#define USART_BRR(x) (((F_CPU/(16*(x))) << 4) | \
	(int)((((float)F_CPU/(float)(16*(x))) - (int)((float)F_CPU/(float)(16*(x)))) * 16.0f + 0.5f))
#define USART_BRR_FCPU(fcpu,x) (((fcpu/(16*(x))) << 4) | \
	(int)((((float)fcpu/(float)(16*(x))) - (int)((float)fcpu/(float)(16*(x)))) * 16.0f + 0.5f))


#define M_PI 3.141592f
#define M_SQ2_2 0.70710678f

static float d2r (float v) { return v * M_PI / 180.0f; }
static float r2d (float v) { return v * 180.0f / M_PI; }

static void st_delay_ms (uint32_t ms)
{
	uint32_t end = ticks + ms;
	while (ticks < end);
}

#define EXTI_PORTA 0b0000
#define EXTI_PORTB 0b0001
#define EXTI_PORTC 0b0010
#define EXTI_PORTD 0b0011
#define EXTI_PORTE 0b0100
#define EXTI_PORTF 0b0110
#define EXTI_PORTG 0b0111
#define EXTI_PORTH 0b0101

#define SET_EXTICR(irq,port) { \
	SYSCFG->EXTICR[(irq) / 4] &= ~(0b1111 << (((irq) % 4) * 4)); \
	SYSCFG->EXTICR[(irq) / 4] |= (port) << (((irq) % 4) * 4); }

#endif
