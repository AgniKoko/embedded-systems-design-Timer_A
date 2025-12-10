#include "timerA_HAL_19.h"

// Initial state of timer A
static TimerA_Mode g_lastConfiguredMode = MODE_STOP;


/* ======== Internal static helper functions ======== */

// TACCTLx selecting function
static volatile TimerA_TACCTL* TimerA_getTacctlReg(TimerA_Channel channel)
{
    switch (channel)
    {
    case CHANNEL_0: return &TACCTL0;
    case CHANNEL_1: return &TACCTL1;
    case CHANNEL_2: return &TACCTL2;
    default:        return (volatile TimerA_TACCTL*)0;
    }
}

// TACCRx selecting function
static volatile TimerA_TACCR* TimerA_getTaccrReg(TimerA_Channel channel)
{
    switch (channel)
    {
    case CHANNEL_0: return &TACCR0;
    case CHANNEL_1: return &TACCR1;
    case CHANNEL_2: return &TACCR2;
    default:        return (volatile TimerA_TACCR*)0;
    }
}


/* ======== Public API implementation ======== */

void TimerA_ApplyConfig(const TimerA_Config* cfg)
{
    if (!cfg)
        return;
    /* Preparing TACTL to recieve new config */
    TACTL.bits.MC = MODE_STOP; // Upload 00 to bits 5-4 of TACTL (Stop Mode)

    TACTL.bits.TACLR = 1;      // Upload 1 to bit 2 of TACTL (Enable Clear)
 
    TACTL.bits.TAIFG = 0;      // Upload 0 to bit 0 of TACL (Clear Interrupt Flag)
    
    /* Config Uploading */
    TACTL.bits.TASSEL = cfg->clockSource;                // Upload selected ClockSource on TASSEL bits of TACTL bitfield
    
    TACTL.bits.ID     = cfg->clockDivider;               // Upload selected clockDivider on ID bits of TACTL bitfield
    
    TACTL.bits.TAIE = cfg->enableTimerInterrupt ? 1 : 0; // Upload selected enableTimerInterrupt on TAIE bits of TACTL bitfield

    TACCR0 = cfg->period;                                // Upload cfg.period to TACCR0

    g_lastConfiguredMode = cfg->mode;                    // Store last mode so TimerA_Start() can use it
}


void TimerA_ConfigureChannel(const TimerA_ChannelConfig *chCfg)
{
    if (!chCfg)
        return;
    
    // cctl points to the TACCTLx register of the selected channel (CCR0 / CCR1 / CCR2)
    volatile TimerA_TACCTL* cctl = TimerA_getTacctlReg(chCfg->channel);
    // ccr points to the TACCRx register of the selected channel
    volatile TimerA_TACCR* ccr  = TimerA_getTaccrReg(chCfg->channel);

    if (!cctl || !ccr)
        return;

    cctl->bits.CAP = 0; // Forces CAP=0 (Compare Mode)

    // Clear capture-related bits (CM[15:14], CCIS[13:12], SCS[11], SCCI[10])
    // These bits are only used in capture mode, so we reset them in pure compare mode
    cctl->bits.CM   = 0;
    cctl->bits.CCIS = 0;
    cctl->bits.SCS  = 0;
    cctl->bits.SCCI = 0;

    /* Config Uploading to selected channel */
    cctl->bits.OUTMOD = chCfg->outputMode;              // Upload selected outputMode on OUTPUT bits of selected TACCTL bitfield

    cctl->bits.CCIE = chCfg->enableInterrupt ? 1 : 0;   // Upload selected enableInterrupt on CCIE bits of selected TACCTL bitfield
    
    cctl->bits.COV   = 0;                               // Upload 0 on COV bit of selected TACCTL bitfield
    
    cctl->bits.CCIFG = 0;;                              // Upload 0 on CCIFG bit of selected TACCTL bitfield

    *ccr = chCfg->compareValue;                         // Upload selected compareValue on selected TACCR bitfield
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

    // Store last mode so TimerA_Start() can use it later
    g_lastConfiguredMode = MODE_UP;

    TACTL.bits.MC = MODE_UP; // Instantly start Timer by uploading 01 to MC bits of TACTL bitfield (Designing choice)
}


void TimerA_Start(void)
{
    TACTL.bits.MC = g_lastConfiguredMode; // Upload globally saved selected Mode to MC bits of TACTL bitfield
}

// Not neccesary for this application
void TimerA_StartInMode(TimerA_Mode mode)
{
    g_lastConfiguredMode = mode;
    TACTL.bits.MC = mode;
}

void TimerA_Stop(void)
{
    TACTL.bits.MC = MODE_STOP;
}


DOUBLE_BYTE TimerA_GetCounter(void)
{
    return TAR;
}

void TimerA_ResetCounter(void)
{
    // TACLR = 1 -> reset TAR and prescaler
    TACTL.bits.TACLR = 1;
    // Also clear IFG
    TACTL.bits.TAIFG = 0;
}


void TimerA_SetPeriod(DOUBLE_BYTE period)
{
    TACCR0 = period;
}


void TimerA_SetDuty(TimerA_Channel channel, DOUBLE_BYTE dutyValue)
{
    volatile TimerA_TACCR *ccr = TimerA_getTaccrReg(channel);
    if (!ccr)
        return;

    *ccr = dutyValue;
}


DOUBLE_BYTE TimerA_GetCaptureCompare(TimerA_Channel channel)
{
    volatile TimerA_TACCR *ccr = TimerA_getTaccrReg(channel);
    if (!ccr)
        return 0u;

    return *ccr;
}