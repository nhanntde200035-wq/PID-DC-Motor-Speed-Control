#include "S32K144.h"
#include "lpit.h"
#include "ftm_pwm.h"

/* ====================================================================
 * MO RONG LAB 2 - CUM DIEU KHIEN XE MAY (board MaaZEDU)
 *   - SW0 (PTC12) bat/tat xi-nhan TRAI  -> LED do  (PTD15) nhay
 *   - SW1 (PTC13) bat/tat xi-nhan PHAI  -> LED xanh la (PTD16) nhay
 *   - BTN_UP   (PTD10, J8-18) -> tang toc do dong co
 *   - BTN_DOWN (PTD9 , J8-20) -> giam toc do dong co
 *     PWM ra FTM2_CH4 (PTD13) -> ENA driver L298N
 *   Clock: FIRC 48MHz noi bo (khong can thach anh ngoai)
 * ================================================================== */

#define IN1_PIN      12U
#define IN2_PIN      11U
#define LED_L_PIN    15U
#define LED_R_PIN    16U
#define BTN_UP_PIN   10U
#define BTN_DOWN_PIN  9U
#define RAMP_DIV      2U
#define BLINK_TICKS  40U

volatile uint8_t  g_left_on  = 0U;
volatile uint8_t  g_right_on = 0U;
volatile uint8_t  g_blink    = 0U;
volatile uint16_t g_tick     = 0U;
volatile uint8_t  g_speed    = 0U;
volatile uint8_t  g_sw0_prev = 1U;
volatile uint8_t  g_sw1_prev = 1U;
volatile uint8_t  g_ramp_cnt = 0U;

void GPIO_init(void)
{
    PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;
    PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;

    PORTD->PCR[LED_L_PIN] = PORT_PCR_MUX(1);
    PORTD->PCR[LED_R_PIN] = PORT_PCR_MUX(1);
    PTD->PDDR |= (1U << LED_L_PIN) | (1U << LED_R_PIN);
    PTD->PSOR  = (1U << LED_L_PIN) | (1U << LED_R_PIN);

    PORTD->PCR[IN1_PIN] = PORT_PCR_MUX(1);
    PORTD->PCR[IN2_PIN] = PORT_PCR_MUX(1);
    PTD->PDDR |= (1U << IN1_PIN) | (1U << IN2_PIN);
    PTD->PSOR  = (1U << IN1_PIN);
    PTD->PCOR  = (1U << IN2_PIN);

    PORTC->PCR[12] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PORTC->PCR[13] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PTC->PDDR &= ~((1U << 12) | (1U << 13));

    PORTD->PCR[BTN_UP_PIN]   = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PORTD->PCR[BTN_DOWN_PIN] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PTD->PDDR &= ~((1U << BTN_UP_PIN) | (1U << BTN_DOWN_PIN));
}

void LPIT0_Ch0_IRQHandler(void)
{
    uint8_t sw0, sw1, up, down;

    LPIT0->MSR = LPIT_MSR_TIF0_MASK;

    sw0  = (uint8_t)((PTC->PDIR >> 12) & 1U);
    sw1  = (uint8_t)((PTC->PDIR >> 13) & 1U);
    up   = (uint8_t)((PTD->PDIR >> BTN_UP_PIN)   & 1U);
    down = (uint8_t)((PTD->PDIR >> BTN_DOWN_PIN) & 1U);

    if ((sw0 == 0U) && (g_sw0_prev == 1U)) { g_left_on  ^= 1U; g_right_on = 0U; }
    if ((sw1 == 0U) && (g_sw1_prev == 1U)) { g_right_on ^= 1U; g_left_on  = 0U; }
    g_sw0_prev = sw0;
    g_sw1_prev = sw1;

    if (++g_ramp_cnt >= RAMP_DIV)
    {
        g_ramp_cnt = 0U;
        if (up == 0U)        { if (g_speed < 100U) g_speed++; }
        else if (down == 0U) { if (g_speed > 0U)   g_speed--; }
    }

    Motor_set_speed(g_speed);

    if (++g_tick >= BLINK_TICKS) { g_tick = 0U; g_blink ^= 1U; }

    if (g_left_on  && g_blink) PTD->PCOR = (1U << LED_L_PIN);
    else                       PTD->PSOR = (1U << LED_L_PIN);

    if (g_right_on && g_blink) PTD->PCOR = (1U << LED_R_PIN);
    else                       PTD->PSOR = (1U << LED_R_PIN);
}

int main(void)
{
    /* Clock: bat FIRCDIV1 va FIRCDIV2 = 48MHz (FIRC noi bo, mac dinh sau reset) */
    SCG->FIRCDIV = SCG_FIRCDIV_FIRCDIV1(1) | SCG_FIRCDIV_FIRCDIV2(1);

    GPIO_init();
    Motor_PWM_init();
    LPIT0_init(LPIT_10MS_TVAL);
    LPIT0_IRQ_enable();

    for (;;) {}
}
