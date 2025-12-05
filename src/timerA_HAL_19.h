#ifndef TIMERA_HAL_19_H_
#define TIMERA_HAL_19_H_

/* Basic fixed-width type aliases for MSP430 (no stdint.h in this IAR setup). */
typedef unsigned char  uint8_t;   /* 8-bit unsigned */
typedef unsigned int   uint16_t;  /* 16-bit unsigned */

/* ======== TimerA high-level enums ======== */

typedef enum {
    TIMERA_CLOCK_TACLK = 0u,   /* TASSEL = 00: external TACLK */
    TIMERA_CLOCK_ACLK  = 1u,   /* TASSEL = 01: ACLK */
    TIMERA_CLOCK_SMCLK = 2u,   /* TASSEL = 10: SMCLK */
    TIMERA_CLOCK_INCLK = 3u    /* TASSEL = 11: INCLK */
} TimerA_ClockSource;

typedef enum {
    TIMERA_DIV_1 = 0u,  /* ID = 00: /1 */
    TIMERA_DIV_2 = 1u,  /* ID = 01: /2 */
    TIMERA_DIV_4 = 2u,  /* ID = 10: /4 */
    TIMERA_DIV_8 = 3u   /* ID = 11: /8 */
} TimerA_ClockDivider;

typedef enum {
    TIMERA_MODE_STOP       = 0u,  /* MC = 00: stop */
    TIMERA_MODE_UP         = 1u,  /* MC = 01: up to TACCR0 */
    TIMERA_MODE_CONTINUOUS = 2u,  /* MC = 10: up to 0xFFFF */
    TIMERA_MODE_UPDOWN     = 3u   /* MC = 11: up/down */
} TimerA_Mode;

/* CCR channels of Timer_A */
typedef enum {
    TIMERA_CHANNEL_0 = 0u,
    TIMERA_CHANNEL_1 = 1u,
    TIMERA_CHANNEL_2 = 2u
} TimerA_Channel;

/* OUTMOD bits – output mode */
typedef enum {
    TIMERA_OUTMODE_OUT        = 0u, /* 000: use OUT bit value */
    TIMERA_OUTMODE_SET        = 1u, /* 001: set */
    TIMERA_OUTMODE_TOGGLE_RST = 2u, /* 010: toggle/reset */
    TIMERA_OUTMODE_SET_RST    = 3u, /* 011: set/reset */
    TIMERA_OUTMODE_TOGGLE     = 4u, /* 100: toggle */
    TIMERA_OUTMODE_RESET      = 5u, /* 101: reset */
    TIMERA_OUTMODE_TOGGLE_SET = 6u, /* 110: toggle/set */
    TIMERA_OUTMODE_RST_SET    = 7u  /* 111: reset/set */
} TimerA_OutputMode;

/* ======== Bitfield representations of registers ======== */

/* TACTL – Timer_A Control Register */
typedef struct {
    unsigned int TAIFG   : 1; /* bit 0: Timer_A interrupt flag */
    unsigned int TAIE    : 1; /* bit 1: Timer_A interrupt enable */
    unsigned int TACLR   : 1; /* bit 2: Timer_A clear */
    unsigned int UNUSED0 : 1; /* bit 3: unused */
    unsigned int MC      : 2; /* bits 4-5: mode control */
    unsigned int ID      : 2; /* bits 6-7: input divider */
    unsigned int TASSEL  : 2; /* bits 8-9: clock source select */
    unsigned int UNUSED1 : 6; /* bits 10-15: unused */
} TimerA_TACTL_bits;

typedef union {
    unsigned int        reg;
    TimerA_TACTL_bits   bits;
} TimerA_TACTL_t;

/* TACCTLx – Capture/Compare Control Register */
typedef struct {
    unsigned int CCIFG   : 1; /* bit 0: interrupt flag */
    unsigned int COV     : 1; /* bit 1: capture overflow */
    unsigned int OUT     : 1; /* bit 2: output */
    unsigned int CCI     : 1; /* bit 3: capture/compare input */
    unsigned int CCIE    : 1; /* bit 4: interrupt enable */
    unsigned int OUTMOD  : 3; /* bits 5-7: output mode */
    unsigned int CAP     : 1; /* bit 8: 0=compare, 1=capture */
    unsigned int UNUSED0 : 1; /* bit 9: unused */
    unsigned int SCCI    : 1; /* bit 10: synchronized CCI */
    unsigned int SCS     : 1; /* bit 11: synchronize capture source */
    unsigned int CCIS    : 2; /* bits 12-13: input select */
    unsigned int CM      : 2; /* bits 14-15: capture mode */
} TimerA_TACCTL_bits;

typedef union {
    unsigned int         reg;
    TimerA_TACCTL_bits   bits;
} TimerA_TACCTL_t;

/* TAR / TACCRx interpreted as 16-bit values */
typedef uint16_t TimerA_TAR_t;
typedef uint16_t TimerA_TACCR_t;

/* ======== High-level configuration structs ======== */

typedef struct {
    TimerA_ClockSource  clockSource;
    TimerA_ClockDivider clockDivider;
    TimerA_Mode         mode;
    uint16_t            period;               /* TACCR0 */
    uint8_t             enableTimerInterrupt; /* 0/1 -> TAIE */
} TimerA_Config;

typedef struct {
    TimerA_Channel    channel;         /* 0,1,2 -> TACCR0/1/2 */
    TimerA_OutputMode outputMode;      /* OUTMOD bits */
    uint16_t          compareValue;    /* TACCRx */
    uint8_t           enableInterrupt; /* 0/1 -> CCIE */
} TimerA_ChannelConfig;

typedef struct {
    TimerA_Channel    channel;   /* CCR channel used for PWM */
    uint16_t          period;    /* TACCR0 (PWM period) */
    uint16_t          duty;      /* TACCRx (PWM duty) */
    TimerA_OutputMode outputMode;
} TimerA_PWMConfig;

/* ======== Public API prototypes ======== */

void     TimerA_ApplyConfig(const TimerA_Config *cfg);
void     TimerA_ConfigureChannel(const TimerA_ChannelConfig *chCfg);
void     TimerA_ConfigPWM(const TimerA_PWMConfig *pwmCfg);

void     TimerA_Start(void);
void     TimerA_StartInMode(TimerA_Mode mode);
void     TimerA_Stop(void);

uint16_t TimerA_GetCounter(void);
void     TimerA_ResetCounter(void);

void     TimerA_SetPeriod(uint16_t period);
void     TimerA_SetDuty(TimerA_Channel channel, uint16_t dutyValue);

uint16_t TimerA_GetCaptureCompare(TimerA_Channel channel);

#endif /* TIMERA_HAL_19_H_ */
