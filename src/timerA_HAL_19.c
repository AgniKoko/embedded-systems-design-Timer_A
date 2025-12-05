#include "timerA_HAL_19.h"

/*
 * IMPORTANT:
 * Include here the correct MSP430 device header that you use in the lab.
 * For example, in the lab examples you might see:
 *   #include <msp430x14x.h>
 * or
 *   #include <io430x14x.h>
 * Use exactly the same header in this file.
 */
#include <msp430x14x.h>

/* ======== Internal HAL state ======== */

static TimerA_Mode g_lastConfiguredMode = TIMERA_MODE_STOP;


/* ======== Helper macros for TACTL / TACCTLx bit fields ======== */

/* TACTL bits */
#define TACTL_TASSEL_MASK   (0x0300u)  /* bits 9-8 */
#define TACTL_ID_MASK       (0x00C0u)  /* bits 7-6 */
#define TACTL_MC_MASK       (0x0030u)  /* bits 5-4 */
#define TACTL_TACLR         (1u << 2)  /* bit 2  */
#define TACTL_TAIE          (1u << 1)  /* bit 1  */
#define TACTL_TAIFG         (1u << 0)  /* bit 0  */

/* TACCTLx bits */
#define TACCTL_OUTMOD_MASK  (0x00E0u)  /* bits 7-5 */
#define TACCTL_CAP          (1u << 8)  /* bit 8  */
#define TACCTL_CCIE         (1u << 4)  /* bit 4  */
#define TACCTL_COV          (1u << 1)  /* bit 1  */
#define TACCTL_CCIFG        (1u << 0)  /* bit 0  */
#define TACCTL_CM_MASK      (0xC000u)  /* bits 15-14 */
#define TACCTL_CCIS_MASK    (0x3000u)  /* bits 13-12 */
#define TACCTL_SCS          (1u << 11) /* bit 11 */
#define TACCTL_SCCI         (1u << 10) /* bit 10 */


/* ======== Internal static helper functions ======== */

static volatile unsigned int* TimerA_getTacctlReg(TimerA_Channel channel)
{
    switch (channel)
    {
    case TIMERA_CHANNEL_0: return &TACCTL0;
    case TIMERA_CHANNEL_1: return &TACCTL1;
    case TIMERA_CHANNEL_2: return &TACCTL2;
    default:               return (volatile unsigned int*)0;
    }
}

static volatile TimerA_TACCR_t* TimerA_getTaccrReg(TimerA_Channel channel)
{
    switch (channel)
    {
    case TIMERA_CHANNEL_0: return &TACCR0;
    case TIMERA_CHANNEL_1: return &TACCR1;
    case TIMERA_CHANNEL_2: return &TACCR2;
    default:               return (volatile TimerA_TACCR_t*)0;
    }
}


/* ======== Public API implementation ======== */

void TimerA_ApplyConfig(const TimerA_Config *cfg)
{
    if (!cfg)
        return;

    /* 1) Stop the timer before reconfiguring it */
    TACTL &= ~TACTL_MC_MASK;

    /* 2) Clear counter and prescaler (TAR etc.) using TACLR */
    TACTL |= TACTL_TACLR;

    /* 3) Clear the Timer_A IFG flag */
    TACTL &= ~TACTL_TAIFG;

    /* 4) Configure clock source, divider and mode */
    TACTL &= ~(TACTL_TASSEL_MASK | TACTL_ID_MASK | TACTL_MC_MASK); /* clear */
    TACTL |= (((uint16_t)cfg->clockSource  & 0x3u) << 8)
          |  (((uint16_t)cfg->clockDivider & 0x3u) << 6)
          |  (((uint16_t)cfg->mode         & 0x3u) << 4);

    /* 5) Enable / disable Timer_A interrupt (TAIE) */
    if (cfg->enableTimerInterrupt)
        TACTL |= TACTL_TAIE;
    else
        TACTL &= ~TACTL_TAIE;

    /* 6) Configure period (TACCR0) */
    TACCR0 = cfg->period;

    /* Store last mode so TimerA_Start() can reuse it */
    g_lastConfiguredMode = cfg->mode;
}


void TimerA_ConfigureChannel(const TimerA_ChannelConfig *chCfg)
{
    if (!chCfg)
        return;

    volatile unsigned int   *cctl = TimerA_getTacctlReg(chCfg->channel);
    volatile TimerA_TACCR_t *ccr  = TimerA_getTaccrReg(chCfg->channel);

    if (!cctl || !ccr)
        return;

    /* 1) Force compare mode: CAP = 0 */
    *cctl &= ~TACCTL_CAP;

    /* 2) Clear capture-related bits that we do not use in pure compare mode */
    *cctl &= ~(TACCTL_CM_MASK | TACCTL_CCIS_MASK | TACCTL_SCS | TACCTL_SCCI);

    /* 3) Configure OUTMOD bits for output mode */
    *cctl &= ~TACCTL_OUTMOD_MASK;
    *cctl |= (((unsigned int)chCfg->outputMode & 0x7u) << 5);

    /* 4) Enable / disable CC interrupt for this channel */
    if (chCfg->enableInterrupt)
        *cctl |= TACCTL_CCIE;
    else
        *cctl &= ~TACCTL_CCIE;

    /* 5) Clear CC flags */
    *cctl &= ~(TACCTL_COV | TACCTL_CCIFG);

    /* 6) Configure compare value (e.g. PWM duty) */
    *ccr = chCfg->compareValue;
}


void TimerA_ConfigPWM(const TimerA_PWMConfig *pwmCfg)
{
    if (!pwmCfg)
        return;

    /* Configure period in TACCR0 */
    TimerA_SetPeriod(pwmCfg->period);

    /* Configure channel for PWM */
    TimerA_ChannelConfig ch;
    ch.channel         = pwmCfg->channel;
    ch.outputMode      = pwmCfg->outputMode;
    ch.compareValue    = pwmCfg->duty;
    ch.enableInterrupt = 0u;

    TimerA_ConfigureChannel(&ch);

    /* Put Timer_A in up mode for PWM operation */
    g_lastConfiguredMode = TIMERA_MODE_UP;
    TACTL &= ~TACTL_MC_MASK;
    TACTL |= ((unsigned int)TIMERA_MODE_UP << 4);
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


uint16_t TimerA_GetCounter(void)
{
    return TAR;
}

void TimerA_ResetCounter(void)
{
    /* TACLR = 1 -> reset TAR and prescaler */
    TACTL |= TACTL_TACLR;
    /* Also clear IFG */
    TACTL &= ~TACTL_TAIFG;
}


void TimerA_SetPeriod(uint16_t period)
{
    TACCR0 = period;
}


void TimerA_SetDuty(TimerA_Channel channel, uint16_t dutyValue)
{
    volatile TimerA_TACCR_t *ccr = TimerA_getTaccrReg(channel);
    if (!ccr)
        return;

    *ccr = dutyValue;
}


uint16_t TimerA_GetCaptureCompare(TimerA_Channel channel)
{
    volatile TimerA_TACCR_t *ccr = TimerA_getTaccrReg(channel);
    if (!ccr)
        return 0u;

    return *ccr;
}
