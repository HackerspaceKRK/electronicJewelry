extern void main ();
void _errorloop () __attribute__ ((noreturn));

void Reset_Handler ();
extern void NMI_Handle ();
extern void HardFault_Handler ();
extern void SVCall_Handler ();
extern void PendSV_Handler ();
extern void SysTick_Handler ();

extern void WWDG_Handler ();
extern void RTC_Handler ();
extern void FLASH_Handler ();
extern void RCC_Handler ();
extern void EXTI0_1_Handler ();
extern void EXTI2_3_Handler ();
extern void EXTI4_15_Handler ();
extern void DMA1_Channel1_Handler ();
extern void DMA1_Channel2_3_Handler ();
extern void DMA1_Channel4_5_Handler ();
extern void ADC_Handler ();
extern void TIM1_BRK_UP_TRG_COM_Handler ();
extern void TIM1_CC_Handler ();
extern void TIM3_Handler ();
extern void TIM14_Handler ();
extern void TIM15_Handler ();
extern void TIM16_Handler ();
extern void TIM17_Handler ();
extern void I2C1_Handler ();
extern void I2C2_Handler ();
extern void SPI1_Handler ();
extern void SPI2_Handler ();
extern void USART1_Handler ();
extern void USART2_Handler ();

typedef void (*pfnISR)(void);

__attribute__ ((section(".isr_vector")))
pfnISR VectorTable[] =  
{ 
  (pfnISR)(0x20000000 + RAMSIZE), // The initial stack pointer is the top of SRAM 
  Reset_Handler,
  NMI_Handle, 
  HardFault_Handler,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  SVCall_Handler,
  0,
  0,
  PendSV_Handler,
  SysTick_Handler,

	/*  0 */ 0, // WWDG_Handler,
	/*  1 */ 0, // Reserved,
	/*  2 */ 0, // RTC_Handler,
	/*  3 */ 0, // FLASH_Handler,
	/*  4 */ 0, // RCC_Handler,
	/*  5 */ 0, // EXTI0_1_Handler,
	/*  6 */ 0, // EXTI2_3_Handler,
	/*  7 */ 0, // EXTI4_15_Handler,
	/*  8 */ 0, // Reserved,
	/*  9 */ 0, // DMA1_Channel1_Handler,
	/* 10 */ 0, // DMA1_Channel2_3_Handler,
	/* 11 */ 0, // DMA1_Channel4_5_Handler,
	/* 12 */ 0, // ADC_Handler,
	/* 13 */ 0, // TIM1_BRK_UP_TRG_COM_Handler,
	/* 14 */ 0, // TIM1_CC_Handler,
	/* 15 */ 0, // Reserved,
	/* 16 */ 0, // TIM3_Handler,
	/* 17 */ 0, // Reserved,
	/* 18 */ 0, // Reserved,
	/* 19 */ 0, // TIM14_Handler,
	/* 20 */ 0, // TIM15_Handler,
	/* 21 */ 0, // TIM16_Handler,
	/* 22 */ 0, // TIM17_Handler,
	/* 23 */ 0, // I2C1_Handler,
	/* 24 */ 0, // I2C2_Handler,
	/* 25 */ 0, // SPI1_Handler,
	/* 26 */ 0, // SPI2_Handler,
	/* 27 */ USART1_Handler,
	/* 28 */ 0, // USART2_Handler,
	/* 29 */ 0, // Reserved,
	/* 30 */ 0, // Reserved,
	/* 31 */ 0, // Reserved,
};

extern unsigned long _eisr_vector;
extern unsigned long _text;
extern unsigned long _etext; 
extern unsigned long _data; 
extern unsigned long _edata; 
extern unsigned long _bss; 
extern unsigned long _ebss; 
extern unsigned long _sidata; 
extern unsigned long _sdata; 

void Reset_Handler () 
{ 
  unsigned long *src, *dst; 

  src = &_sidata;
  dst = &_sdata;
  while (dst < &_edata)
    *dst++ = *src++;

	dst = &_bss;
	while (dst < &_ebss)
    *dst++ = 0; 

  main ();
} 
void NMI_Handle () { _errorloop (); }
void HardFault_Handler () { _errorloop (); }
void SVCall_Handler () { _errorloop (); }
void PendSV_Handler () { _errorloop (); }

