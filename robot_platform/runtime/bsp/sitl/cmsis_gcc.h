#ifndef __CMSIS_GCC_H
#define __CMSIS_GCC_H

/* SITL Mock for ARM CMSIS GCC intrinsics */

#include <stdint.h>

#ifndef __ASM
#define __ASM            __asm
#endif

#ifndef __INLINE
#define __INLINE         inline
#endif

#ifndef __STATIC_INLINE
#define __STATIC_INLINE  static inline
#endif

__STATIC_INLINE uint32_t __get_IPSR(void)
{
  // SITL doesn't use ARM interrupts; return 0 for Thread Mode
  return 0; 
}

__STATIC_INLINE void __disable_irq(void) {}
__STATIC_INLINE void __enable_irq(void) {}
__STATIC_INLINE void __set_PRIMASK(uint32_t priMask) { (void)priMask; }
__STATIC_INLINE uint32_t __get_PRIMASK(void) { return 0; }

__STATIC_INLINE void __DSB(void) {}
__STATIC_INLINE void __ISB(void) {}
__STATIC_INLINE void __DMB(void) {}

#endif /* __CMSIS_GCC_H */
