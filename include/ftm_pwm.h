#ifndef FTM_PWM_H
#define FTM_PWM_H

#include "S32K144.h"
#include <stdint.h>

/* --------------------------------------------------------------------
 * PWM dieu khien toc do dong co - board MaaZEDU
 *   Chan PTD13 = FTM2_CH4 (ALT2) -> header J8-11 (GPIO3) -> ENA driver L298N
 *   Clock FTM: SOSCDIV1_CLK = 8 MHz, prescaler /1
 *   MOD = 399 -> f_PWM = 8 000 000 / 400 = 20 kHz
 * ------------------------------------------------------------------ */
void Motor_PWM_init(void);
void Motor_set_speed(uint8_t percent);   /* 0 = dung, 100 = full toc */

#endif /* FTM_PWM_H */
