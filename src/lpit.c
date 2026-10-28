#include "lpit.h"

void LPIT0_init(uint32_t tval)
{
    PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(3) | PCC_PCCn_CGC_MASK; /* FIRCDIV2 48MHz */
    LPIT0->MCR = LPIT_MCR_M_CEN_MASK;
    LPIT0->TMR[0].TVAL = tval;
    LPIT0->MIER = LPIT_MIER_TIE0_MASK;
    LPIT0->TMR[0].TCTRL = LPIT_TMR_TCTRL_T_EN_MASK;
}

void LPIT0_IRQ_enable(void)
{
    S32_NVIC->ISER[(uint32_t)(LPIT0_Ch0_IRQn) >> 5U] =
        (1U << ((uint32_t)(LPIT0_Ch0_IRQn) & 0x1FU));
    S32_NVIC->IP[(uint32_t)(LPIT0_Ch0_IRQn)] = 0U;
}
