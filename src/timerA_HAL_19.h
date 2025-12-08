#ifndef TIMERA_HAL_19_H_
#define TIMERA_HAL_19_H_

typedef unsigned char BYTE;         /* 8-bit unsigned */
typedef unsigned int  DOUBLE_BYTE;  /* 16-bit unsigned */


/* ======== TimerA high-level enums ======== */

/* ---------- TACTL enums ---------- */

typedef enum {
    CLOCK_TACLK = 0,   /* TASSEL = 00: external TACLK */
    CLOCK_ACLK     ,   /* TASSEL = 01: ACLK */
    CLOCK_SMCLK    ,   /* TASSEL = 10: SMCLK */
    CLOCK_INCLK        /* TASSEL = 11: INCLK */
} TimerA_ClockSource;

typedef enum {
    DIV_1 = 0,  /* ID = 00: ClockSource/1 */
    DIV_2    ,  /* ID = 01: ClockSource/2 */
    DIV_4    ,  /* ID = 10: ClockSource/4 */
    DIV_8       /* ID = 11: ClockSource/8 */
} TimerA_ClockDivider;

typedef enum {
    MODE_STOP       = 0,  /* MC = 00: stop */
    MODE_UP            ,  /* MC = 01: up to TACCR0 */
    MODE_CONTINUOUS    ,  /* MC = 10: up to 0xFFFF */
    MODE_UPDOWN           /* MC = 11: up to TACCR0 / down to 0 */
} TimerA_Mode;

/* --------------------------------- */

/* --------- TACCTLx enums --------- */

typedef enum {
    CHANNEL_0 = 0,  /*x = 0: TACCR0 Register*/
    CHANNEL_1    ,  /*x = 1: TACCR1 Register*/
    CHANNEL_2       /*x = 2: TACCR2 Register*/
} TimerA_Channel;

typedef enum {
    OUTMODE_OUT        = 0, /* OUTMOD = 000: use OUT bit value */
    OUTMODE_SET           , /* OUTMOD = 001: set */
    OUTMODE_TOGGLE_RST    , /* OUTMOD = 010: toggle/reset */
    OUTMODE_SET_RST       , /* OUTMOD = 011: set/reset */
    OUTMODE_TOGGLE        , /* OUTMOD = 100: toggle */
    OUTMODE_RESET         , /* OUTMOD = 101: reset */
    OUTMODE_TOGGLE_SET    , /* OUTMOD = 110: toggle/set */
    OUTMODE_RST_SET         /* OUTMOD = 111: reset/set */
} TimerA_OutputMode;

/* --------------------------------- */


/* ======== Bitfield representations of registers ======== */

/* TACTL: Timer_A Control Register */
typedef struct {
    unsigned int TAIFG   : 1; /* bit 0: Timer_A interrupt flag */
    unsigned int TAIE    : 1; /* bit 1: Timer_A interrupt enable */
    unsigned int TACLR   : 1; /* bit 2: Timer_A clear */
    unsigned int         : 1; /* bit 3: unused */
    unsigned int MC      : 2; /* bits 4-5: mode control */
    unsigned int ID      : 2; /* bits 6-7: input divider */
    unsigned int TASSEL  : 2; /* bits 8-9: clock source select */
    unsigned int         : 6; /* bits 10-15: unused */
} TimerA_TACTL_bits;

typedef union {
    unsigned int        reg;
    TimerA_TACTL_bits   bits;
} TimerA_TACTL;

/* TAR: Timer_A Register (count of Timer_A) */
typedef DOUBLE_BYTE TimerA_TAR;

/* TACCTLx: Capture/Compare Control Register */
typedef struct {
    unsigned int CCIFG   : 1; /* bit 0: capture/compare interrupt flag */
    unsigned int COV     : 1; /* bit 1: capture overflow */
    unsigned int OUT     : 1; /* bit 2: output */
    unsigned int CCI     : 1; /* bit 3: capture/compare input */
    unsigned int CCIE    : 1; /* bit 4: capture/compare interrupt enable */
    unsigned int OUTMOD  : 3; /* bits 5-7: output mode */
    unsigned int CAP     : 1; /* bit 8: capture mode (0=compare, 1=capture) */
    unsigned int         : 1; /* bit 9: unused */
    unsigned int SCCI    : 1; /* bit 10: synchronized capture/compare input */
    unsigned int SCS     : 1; /* bit 11: synchronize capture source */
    unsigned int CCIS    : 2; /* bits 12-13: capture/compare input select */
    unsigned int CM      : 2; /* bits 14-15: capture mode */
} TimerA_TACCTL_bits;

typedef union {
    unsigned int         reg;
    TimerA_TACCTL_bits   bits;
} TimerA_TACCTL;

/* TACCR: Timer_A capture/compare Register (capture/compare value) */
typedef unsigned short TimerA_TACCR;


/* ======== High-level configuration structs ======== */

typedef struct {
    TimerA_ClockSource  clockSource;          /* TASSEL */
    TimerA_ClockDivider clockDivider;         /* ID */
    TimerA_Mode         mode;                 /* MC */
    DOUBLE_BYTE         period;               /* TACCR0 value */
    BYTE                enableTimerInterrupt; /* TAIE */
} TimerA_Config;

typedef struct {
    TimerA_Channel    channel;         /* x in TACCRx */
    TimerA_OutputMode outputMode;      /* OUTMOD */
    DOUBLE_BYTE       compareValue;    /* value of TACCRx */
    BYTE              enableInterrupt; /* CCIE */
} TimerA_ChannelConfig;

typedef struct {
    TimerA_Channel    channel;    /* CCR channel used for PWM */
    DOUBLE_BYTE       period;     /* PWM period -> TACCR0 value */
    DOUBLE_BYTE       duty;       /* PWM duty cycle -> TACCRx value */
    TimerA_OutputMode outputMode; /* OUTMOD */
} TimerA_PWMConfig;


/* ======== Public API prototypes ======== */

/**
 * @brief           : applies configurations to TimerA TACTL and load period value to TACCR0
 * @param cfg       : pointer to TimerA configuration struct
 * @return          : N/A
 */
void TimerA_ApplyConfig(const TimerA_Config* cfg);

/**
 * @brief           : applies configurations to a TimerA TACCRx channel 
 * @param chCfg     : pointer to TimerA Channel configuration struct
 * @return          : N/A
 */
void TimerA_ConfigureChannel(const TimerA_ChannelConfig* chCfg);

/**
 * @brief           : applies configurations for PWM using a TACCRx Channel
 * @param pwmCfg    : pointer to PWM configuration struct
 * @return          : N/A
 */
void TimerA_ConfigPWM(const TimerA_PWMConfig* pwmCfg);

/**
 * @brief           : starts Timer_A as configured in TACTL
 * @return          : N/A
 */
void TimerA_Start(void);

/**
 * @brief           : starts Timer_A as configured in TACTL but in a specified mode MC
 * @param mode      : the method MC the timer uses to count
 * @return          : N/A
 */
void TimerA_StartInMode(TimerA_Mode mode);

/**
 * @brief           : halts Timer_A by clearing MC (Stop Mode)
 * @return          : N/A
 */
void TimerA_Stop(void);

/**
 * @brief           : reads the current value of Timer_A counter TAR
 * @return          : the current 16-bit value of TAR
 */
DOUBLE_BYTE TimerA_GetCounter(void);

/**
 * @brief           : resets Timer_A (TAR, ID, count direction) by setting TACLR of TACTL
 * @return          : N/A
 */
void TimerA_ResetCounter(void);

/**
 * @brief           : writes on TACCR0 a new Period value for Timer_A
 * @param period    : the 16-bit period value
 * @return          : N/A
 */
void TimerA_SetPeriod(DOUBLE_BYTE period);

/**
 * @brief           : writes on a TACCRx Channel a new Duty Cycle value for PWM
 * @param channel   : the Channel whose Duty Cycle should be modified
 * @param dutyValue : the new Duty Cycle value of the Channel 
 * @return          : N/A
 */
void TimerA_SetDuty(TimerA_Channel channel, DOUBLE_BYTE dutyValue);

/**
 * @brief           : reads the stored value of a TACCRx Channel
 * @param channel   : the Channel whose value should be read
 * @return          : the 16-bit value of the Channel
 */
DOUBLE_BYTE TimerA_GetCaptureCompare(TimerA_Channel channel);

#endif /* TIMERA_HAL_19_H_ */