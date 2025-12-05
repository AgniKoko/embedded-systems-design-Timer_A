#include "timerA_HAL_19.h"

// #include <msp430x14x.h>

int main(void)
{
    /* --------------------------------------------------------------------
     * Example 1: Basic configuration of Timer_A as a simple counter
     * ------------------------------------------------------------------ */

    TimerA_Config basicCfg;

    basicCfg.clockSource          = TIMERA_CLOCK_SMCLK;  /* clock source */
    basicCfg.clockDivider         = TIMERA_DIV_8;        /* prescaler /8 */
    basicCfg.mode                 = TIMERA_MODE_UP;      /* up to TACCR0 */
    basicCfg.period               = 1000u;               /* period = 1000 ticks */
    basicCfg.enableTimerInterrupt = 0u;                  /* no timer interrupt */

    /* Apply configuration (writes TACTL, TACCR0, etc.) */
    TimerA_ApplyConfig(&basicCfg);

    /* Start Timer_A in the mode that we configured above */
    TimerA_Start();

    volatile unsigned long delay;
    uint16_t t1, t2, t3;

    /* Simple software delay */
    for (delay = 0; delay < 10000; ++delay) { }
    t1 = TimerA_GetCounter();      /* read TAR */

    for (delay = 0; delay < 10000; ++delay) { }
    t2 = TimerA_GetCounter();      /* TAR has advanced */

    /* Reset the counter (via TACLR bit) */
    TimerA_ResetCounter();

    for (delay = 0; delay < 10000; ++delay) { }
    t3 = TimerA_GetCounter();      /* after reset it starts again from 0 */

    /* Stop the timer */
    TimerA_Stop();

    /*
     * At this point:
     *  - t1, t2, t3 hold characteristic values of TAR,
     *  - we have demonstrated the use of: ApplyConfig, Start,
     *    GetCounter, Reset, Stop.
     */


    /* --------------------------------------------------------------------
     * Example 2: Configure PWM on CCR1 with 50% duty cycle
     * ------------------------------------------------------------------ */

    TimerA_PWMConfig pwmCfg;

    pwmCfg.channel    = TIMERA_CHANNEL_1;          /* use CCR1 */
    pwmCfg.period     = 1000u;                     /* TACCR0 = 1000 */
    pwmCfg.duty       = 500u;                      /* CCR1 = 500 -> 50% duty */
    pwmCfg.outputMode = TIMERA_OUTMODE_RST_SET;    /* classic PWM mode (reset/set) */

    /*
     * Helper routine that:
     *  - writes TACCR0 (period),
     *  - configures TACCTL1 / TACCR1 for PWM,
     *  - sets MC = up mode in TACTL.
     */
    TimerA_ConfigPWM(&pwmCfg);

    /*
     * Timer_A is already running in up mode after ConfigPWM(),
     * but we can call Start() again for clarity.
     */
    TimerA_Start();

    /*
     * If you connect the CCR1 output pin (e.g. P1.x) on the board,
     * you should see a PWM waveform with 50% duty cycle.
     */


    /* --------------------------------------------------------------------
     * Example 3: Dynamically change the duty cycle
     * ------------------------------------------------------------------ */

    /* Keep 50% duty for a while */
    for (delay = 0; delay < 50000; ++delay) { }

    /* Change to ~75% duty (CCR1 = 750) */
    TimerA_SetDuty(TIMERA_CHANNEL_1, 750u);

    for (delay = 0; delay < 50000; ++delay) { }

    /* Change to ~25% duty (CCR1 = 250) */
    TimerA_SetDuty(TIMERA_CHANNEL_1, 250u);

    uint16_t ccr1_value = TimerA_GetCaptureCompare(TIMERA_CHANNEL_1);
    (void)ccr1_value;  /* avoid "unused variable" warning if not used */

    while (1){}
}
