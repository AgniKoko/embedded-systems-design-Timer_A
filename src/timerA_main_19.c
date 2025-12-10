#include "timerA_HAL_19.h"


int main(void)
{
    /* ============= IimerA Example: Simple Counting ============= */
    // A struct to keep configuration values
    TimerA_Config basicCfg;

    basicCfg.clockSource          = CLOCK_SMCLK;  /* Clock source SMCLK */
    basicCfg.clockDivider         = DIV_8;        /* Prescaler /8 */
    basicCfg.mode                 = MODE_UP;      /* Counting Up to TACCR0 */
    basicCfg.period               = 1000u;        /* Period = 1000 counts */
    basicCfg.enableTimerInterrupt = 0u;           /* No timer interrupts */

    // Apply TimerA configurations
    TimerA_ApplyConfig(&basicCfg);
     
    volatile unsigned long delay;
    DOUBLE_BYTE t1, t2, t3;

    // Initiate Timer_A in the as configured mode
    TimerA_Start();

    // Read TAR after delays
    for (delay = 0; delay < 5; ++delay);
    t1 = TimerA_GetCounter();      /* read TAR */

    for (delay = 0; delay < 10000; ++delay);
    t2 = TimerA_GetCounter();      /* read TAR (t1<t2) */

    // Reset the counter back to 0    
    TimerA_ResetCounter();

    for (delay = 0; delay < 10000; ++delay);
    t3 = TimerA_GetCounter();      /* read TAR */

    // Stop timer
    TimerA_Stop();


    /* =================== IimerA Example: PWM =================== */

    // A struct to keep PWM configuration values
    TimerA_PWMConfig pwmCfg;

    pwmCfg.channel    = CHANNEL_1;          /* Duty cycle in TACCR1 */
    pwmCfg.period     = 1000u;              /* Period = 1000 counts (in TACCR0)*/
    pwmCfg.duty       = 500u;               /* Duty_Cycle = 500/1000 = 50% */
    pwmCfg.outputMode = OUTMODE_RST_SET;    /* High to Low pulse (reset/set) */

    // Apply TimerA PWM configurations
    TimerA_ConfigPWM(&pwmCfg); /* Immediate timer start after configurations*/

    // PWM for a while with Duty_Cycle = 50%
    for (delay = 0; delay < 50000; ++delay);

    // Change Duty Cycle in TACCR1 from 50% to 750/1000 = 75%
    TimerA_SetDuty(CHANNEL_1, 750u);

    // PWM for a while with Duty_Cycle = 75%
    for (delay = 0; delay < 50000; ++delay);

    // Change Duty Cycle in TACCR1 from 75% to 250/1000 = 25%
    TimerA_SetDuty(CHANNEL_1, 250u);
    
    // Read Duty Cycle value (250) in TACCR1
    DOUBLE_BYTE ccr1_value = TimerA_GetCaptureCompare(CHANNEL_1);
    
    
    
    // !IGNORE!: Is to avoid "unused variable" warnings.
    (void)t1; (void)t2; (void)t3; 
    (void)ccr1_value;

    while(1);
}