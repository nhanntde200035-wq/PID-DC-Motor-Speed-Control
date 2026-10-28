#ifndef LPIT_H
#define LPIT_H

#include "S32K144.h"
#include <stdint.h>

/* LPIT0 clock: FIRCDIV2_CLK = 48MHz (FIRC noi bo) */
#define LPIT_CLOCK_HZ    48000000UL
#define LPIT_10MS_TVAL   (LPIT_CLOCK_HZ / 100U - 1U)   /* 10 ms */

void LPIT0_init(uint32_t tval);
void LPIT0_IRQ_enable(void);

#endif /* LPIT_H */
