#include "ftm_pwm.h"

#define MOTOR_MOD  2399U   /* 48MHz / 2400 = 20kHz */

void Motor_PWM_init(void)
{
    PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;
    PORTD->PCR[13] = PORT_PCR_MUX(2);   /* PTD13 = FTM2_CH4 ALT2 */

    PCC->PCCn[PCC_FTM2_INDEX] &= ~PCC_PCCn_CGC_MASK;
    PCC->PCCn[PCC_FTM2_INDEX] |=  PCC_PCCn_PCS(3) | PCC_PCCn_CGC_MASK;

    FTM2->MODE = FTM_MODE_WPDIS_MASK;
    FTM2->SC   = FTM_SC_PWMEN4_MASK | FTM_SC_PS(0);
    FTM2->MOD  = MOTOR_MOD;
    FTM2->CNT  = 0U;
    FTM2->CONTROLS[4].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
    FTM2->CONTROLS[4].CnV  = 0U;
    FTM2->SC  |= FTM_SC_CLKS(1);
}

void Motor_set_speed(uint8_t percent)
{
    if (percent > 100U) percent = 100U;
    FTM2->CONTROLS[4].CnV =
        (uint32_t)(MOTOR_MOD + 1U) * (uint32_t)percent / 100U;
}
