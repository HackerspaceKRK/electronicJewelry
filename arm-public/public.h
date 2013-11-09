#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#if defined(STM32F10X_MD_VL) || defined(STM32F10X_MD)
#include <public_f1.h>
#elif defined(STM32L1XX_MD)
#include <public_l1.h>
#elif defined(STM32F0XX_MD) || defined(STM32F0XX_LD)
#include <public_f0.h>
#elif defined(STM32F40_41xxx)
#include <public_f4.h>
#else
#error NO_PART
#endif

#endif
