#include "timerA_HAL_19.h"
#include <msp430x14x.h>

// Initial state of timer A
static TimerA_Mode g_lastConfiguredMode = MODE_STOP;

// TACTL bit masks
#define TACTL_TASSEL_MASK   (0x0300u)  /* bits 9-8 */
#define TACTL_ID_MASK       (0x00C0u)  /* bits 7-6 */
#define TACTL_MC_MASK       (0x0030u)  /* bits 5-4 */
#define TACTL_TACLR         (0x0004u)  /* bit 2  */
#define TACTL_TAIE          (0x0002u)  /* bit 1  */
#define TACTL_TAIFG         (0x0001u)  /* bit 0  */

// TACCTLx bit masks
#define TACCTL_OUTMOD_MASK  (0x00E0u)  /* bits 7-5 */
#define TACCTL_CAP          (0x0100u)  /* bit 8  */
#define TACCTL_CCIE         (0x0010u)  /* bit 4  */
#define TACCTL_COV          (0x0002u)  /* bit 1  */
#define TACCTL_CCIFG        (0x0001u)  /* bit 0  */
#define TACCTL_CM_MASK      (0xC000u)  /* bits 15-14 */
#define TACCTL_CCIS_MASK    (0x3000u)  /* bits 13-12 */
#define TACCTL_SCS          (0x0800u)  /* bit 11 */
#define TACCTL_SCCI         (0x0400u)  /* bit 10 */


/* ======== Internal static helper functions ======== */

// TACCTLx selecting function
static volatile unsigned int* TimerA_getTacctlReg(TimerA_CCRegister channel)
{
    switch (channel)
    {
    case CC_REGISTER_0: return &TACCTL0;
    case CC_REGISTER_1: return &TACCTL1;
    case CC_REGISTER_2: return &TACCTL2;
    default:            return (volatile unsigned int*)0;
    }
}

// TACCRx selecting function
static volatile TimerA_TACCR_t* TimerA_getTaccrReg(TimerA_CCRegister channel)
{
    switch (channel)
    {
    case CC_REGISTER_0: return &TACCR0;
    case CC_REGISTER_1: return &TACCR1;
    case CC_REGISTER_2: return &TACCR2;
    default:            return (volatile TimerA_TACCR_t*)0;
    }
}


/* ======== Public API implementation ======== */

void TimerA_ApplyConfig(const TimerA_Config *cfg)
{
    if (!cfg)
        return;
    
    TACTL &= ~TACTL_MC_MASK; // bit-wise AND with reverse of the MC mask --> forces bits 5-4 of TACTL to be 0 (Stop Mode)

    TACTL |= TACTL_TACLR;    // bit-wise OR with TACLR mask --> forces bit 2 of TACTo be 1 (Enable Clear)

    TACTL &= ~TACTL_TAIFG;   // bit-wise AND with TAIFG --> forces bit 0 of TACTL to be 0 (Clear Interrupt Flag)

    TACTL &= ~(TACTL_TASSEL_MASK | TACTL_ID_MASK); // bit-wise AND with reverse of the (TASSEL_mask || ID_mask) --> forces clock source and divider to 0
    
    // Upload config settings to TACTL
    TACTL |= (((DOUBLE_BYTE)cfg->clockSource  & 0x3u) << 8)        // put cfg.Clocksource on TACTL bits 9-8 by sliding them by 8
          |  (((DOUBLE_BYTE)cfg->clockDivider & 0x3u) << 6)        // put cfg.clockDivider on TACTL bits 7-6 by sliding them by 6
          |  (((DOUBLE_BYTE)cfg->mode         & 0x3u) << 4);       // put cfg.clockDivider on TACTL bits 5-4 by sliding them by 4

    TACTL &= ~TACTL_TAIE; // Clear TAIE 
    TACTL |= (((DOUBLE_BYTE)cfg->enableTimerInterrupt & 0x1u) << 1); // put cfg.enableTimerInterrupt on TACTL bit 1 by sliding them by 1

    // Upload cfg.period to TACCR0
    TACCR0 = cfg->period;

    // Store last mode so TimerA_Start() can reuse it
    g_lastConfiguredMode = cfg->mode;
}


void TimerA_ConfigureChannel(const TimerA_ChannelConfig *chCfg)
{
    if (!chCfg)
        return;
    
    // cctl points to the TACCTLx register of the selected channel (CCR0 / CCR1 / CCR2)
    volatile unsigned int   *cctl = TimerA_getTacctlReg(chCfg->channel);
    // ccr points to the TACCRx register of the selected channel
    volatile TimerA_TACCR_t *ccr  = TimerA_getTaccrReg(chCfg->channel);

    if (!cctl || !ccr)
        return;

    *cctl &= ~TACCTL_CAP; // Forces CAP=0 (Compare Mode)

    // Clear capture-related bits (CM[15:14], CCIS[13:12], SCS[11], SCCI[10])
    // These bits are only used in capture mode, so we reset them in pure compare mode
    *cctl &= ~(TACCTL_CM_MASK | TACCTL_CCIS_MASK | TACCTL_SCS | TACCTL_SCCI);

    // bit-wise AND with reverse of OUTMOD_mask -> forces OUTMOD bits (7-5) to 0
    *cctl &= ~TACCTL_OUTMOD_MASK;
    
    // Upload Channel Config to selected TACCTL
    // bit-wise OR with (outputMode << 5) -> uploads chCfg->outputMode into OUTMOD bits (7-5)
    *cctl |= (((unsigned int)chCfg->outputMode & 0x7u) << 5);

    // Enable / disable CC interrupt for this channel:
    // if enableInterrupt != 0, set CCIE bit to 1; else clear CCIE bit
    if (chCfg->enableInterrupt)
        *cctl |= TACCTL_CCIE;   // bit-wise OR with CCIE mask -> forces CCIE (bit 4) to 1
    else
        *cctl &= ~TACCTL_CCIE;  // bit-wise AND with reverse of CCIE mask -> forces bit 4 to 0

    // bit-wise AND with reverse of (COV | CCIFG)
    // --> clears COV (bit 1) and CCIFG (bit 0) flags for this channel
    *cctl &= ~(TACCTL_COV | TACCTL_CCIFG);

    // Upload compare value (e.g. PWM duty or compare threshold) to TACCRx
    *ccr = chCfg->compareValue;
}


void TimerA_ConfigPWM(const TimerA_PWMConfig *pwmCfg)
{
    if (!pwmCfg)
        return;

    // Upload pwmCfg->period to TACCR0 via TimerA_SetPeriod()
    TimerA_SetPeriod(pwmCfg->period);

    // Build a temporary TimerA_ChannelConfig struct "ch"
    // to reuse the generic TimerA_ConfigureChannel() for PWM setup
    TimerA_ChannelConfig ch;
    ch.channel         = pwmCfg->channel;      // select which CCRx is used for PWM (CCR0/1/2)
    ch.outputMode      = pwmCfg->outputMode;   // desired OUTMOD for PWM waveform
    ch.compareValue    = pwmCfg->duty;         // duty cycle value -> loaded into TACCRx
    ch.enableInterrupt = 0u;                   // no CC interrupt for basic PWM operation

    // Configure TACCTLx and TACCRx for the selected channel using the generic helper
    TimerA_ConfigureChannel(&ch);

    // Store last mode so TimerA_Start() can reuse it later
    g_lastConfiguredMode = MODE_UP;

    // bit-wise AND with reverse of MC mask -> forces bits 5-4 of TACTL to 0 (Stop mode)
    TACTL &= ~TACTL_MC_MASK;

    // bit-wise OR with (MODE_UP << 4) -> uploads MODE_UP into MC bits (5-4) of TACTL
    TACTL |= ((unsigned int)MODE_UP << 4);
}


void TimerA_Start(void)
{
    TACTL &= ~TACTL_MC_MASK;
    TACTL |= ((unsigned int)g_lastConfiguredMode & 0x3u) << 4;
}

void TimerA_StartInMode(TimerA_Mode mode)
{
    g_lastConfiguredMode = mode;
    TACTL &= ~TACTL_MC_MASK;
    TACTL |= ((unsigned int)mode & 0x3u) << 4;
}

void TimerA_Stop(void)
{
    TACTL &= ~TACTL_MC_MASK;
}


DOUBLE_BYTE TimerA_GetCounter(void)
{
    return TAR;
}

void TimerA_ResetCounter(void)
{
    // TACLR = 1 -> reset TAR and prescaler
    TACTL |= TACTL_TACLR;
    // Also clear IFG
    TACTL &= ~TACTL_TAIFG;
}


void TimerA_SetPeriod(DOUBLE_BYTE period)
{
    TACCR0 = period;
}


void TimerA_SetDuty(TimerA_CCRegister channel, DOUBLE_BYTE dutyValue)
{
    volatile TimerA_TACCR_t *ccr = TimerA_getTaccrReg(channel);
    if (!ccr)
        return;

    *ccr = dutyValue;
}


DOUBLE_BYTE TimerA_GetCaptureCompare(TimerA_CCRegister channel)
{
    volatile TimerA_TACCR_t *ccr = TimerA_getTaccrReg(channel);
    if (!ccr)
        return 0u;

    return *ccr;
}
