#ifndef TIMERA_HAL_19_H_
#define TIMERA_HAL_19_H_

typedef unsigned char  BYTE;            /* 8-bit unsigned */
typedef unsigned int   DOUBLE_BYTE;     /* 16-bit unsigned */


/* ======== TimerA high-level enums ======== */

/* ---------- TACTL enums ---------- */

typedef enum {
    CLOCK_TACLK = 0,   /* TASSELx = 00: external TACLK */
    CLOCK_ACLK     ,   /* TASSELx = 01: ACLK */
    CLOCK_SMCLK    ,   /* TASSELx = 10: SMCLK */
    CLOCK_INCLK        /* TASSELx = 11: INCLK */
} TimerA_ClockSource;

typedef enum {
    DIV_1 = 0,  /* IDx = 00: ClockSource/1 */
    DIV_2    ,  /* IDx = 01: ClockSource/2 */
    DIV_4    ,  /* IDx = 10: ClockSource/4 */
    DIV_8       /* IDx = 11: ClockSource/8 */
} TimerA_ClockDivider;

typedef enum {
    MODE_STOP       = 0,  /* MCx = 00: stop */
    MODE_UP            ,  /* MCx = 01: up to TACCR0 */
    MODE_CONTINUOUS    ,  /* MCx = 10: up to 0xFFFF */
    MODE_UPDOWN           /* MCx = 11: up to TACCR0 / down to 0 */
} TimerA_Mode;

/* --------------------------------- */

/* --------- TACCTLx enums --------- */

typedef enum {
    CC_REGISTER_0 = 0,  /*x = 0: TACCR0 Register*/
    CC_REGISTER_1    ,  /*x = 1: TACCR1 Register*/
    CC_REGISTER_2       /*x = 2: TACCR2 Register*/
} TimerA_CCRegister;

typedef enum {
    NO_CAPTURE           = 0, /* CMx = 00: no capture */
    CAPTURE_RISING_EDGE     , /* CMx = 01: capture on rising edge */
    CAPTURE_FALLING_EDGE    , /* CMx = 10: capture on falling edge */
    CAPTURE_BOTH_EDGES        /* CMx = 11: capture on both edges */
} TimerA_CaptureMode;

typedef enum {
    INPUT_CCIxA = 0, /* CCISx = 00: CCIxA */
    INPUT_CCIxB    , /* CCISx = 01: CCIxB */
    INPUT_GND      , /* CCISx = 10: GND */
    INPUT_VCC        /* CCISx = 11: VCC */
} TimerA_CCInput;

typedef enum {
    OUTMODE_OUT        = 0, /* OUTMODx = 000: use OUT bit value */
    OUTMODE_SET           , /* OUTMODx = 001: set */
    OUTMODE_TOGGLE_RST    , /* OUTMODx = 010: toggle/reset */
    OUTMODE_SET_RST       , /* OUTMODx = 011: set/reset */
    OUTMODE_TOGGLE        , /* OUTMODx = 100: toggle */
    OUTMODE_RESET         , /* OUTMODx = 101: reset */
    OUTMODE_TOGGLE_SET    , /* OUTMODx = 110: toggle/set */
    OUTMODE_RST_SET         /* OUTMODx = 111: reset/set */
} TimerA_OutputMode;

/* --------------------------------- */


/* ======== bitfield representations of registers ======== */

/* TACTL ? Timer_A Control Register */
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
} TimerA_TACTL_t;

/* TACCTLx ? Capture/Compare Control Register */
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
} TimerA_TACCTL_t;

/* TAR / TACCRx interpreted as 16-bit values */
typedef DOUBLE_BYTE TimerA_TAR_t;
typedef DOUBLE_BYTE TimerA_TACCR_t;

/* ======== High-level configuration structs ======== */

typedef struct {
    TimerA_ClockSource  clockSource;
    TimerA_ClockDivider clockDivider;
    TimerA_Mode         mode;
    DOUBLE_BYTE         period;               /* TACCR0 */
    BYTE                enableTimerInterrupt; /* 0/1 -> TAIE */
} TimerA_Config;

typedef struct {
    TimerA_CCRegister   channel;         /* 0,1,2 -> TACCR0/1/2 */
    TimerA_OutputMode   outputMode;      /* OUTMOD bits */
    DOUBLE_BYTE         compareValue;    /* TACCRx */
    BYTE                enableInterrupt; /* 0/1 -> CCIE */
} TimerA_ChannelConfig;

typedef struct {
    TimerA_CCRegister   channel;   /* CCR channel used for PWM */
    DOUBLE_BYTE         period;    /* TACCR0 (PWM period) */
    DOUBLE_BYTE         duty;      /* TACCRx (PWM duty) */
    TimerA_OutputMode   outputMode;
} TimerA_PWMConfig;


/* ======== Public API prototypes ======== */

/**
 * @brief          : writes on a specific bit of the PxOUT of a target port
 * @param port     : the port whose PxOUT should be modified
 * @param pinMask  : the mask to select the bit to be written
 * @param state    : the logic state to write to the selected bit of the PxOUT
 * @return         : N/A
 */
void TimerA_ApplyConfig(const TimerA_Config *cfg);

/**
 * @brief          : writes on a specific bit of the PxOUT of a target port
 * @param port     : the port whose PxOUT should be modified
 * @param pinMask  : the mask to select the bit to be written
 * @param state    : the logic state to write to the selected bit of the PxOUT
 * @return         : N/A
 */
void TimerA_ConfigureChannel(const TimerA_ChannelConfig *chCfg);

/**
 * @brief          : writes on a specific bit of the PxOUT of a target port
 * @param port     : the port whose PxOUT should be modified
 * @param pinMask  : the mask to select the bit to be written
 * @param state    : the logic state to write to the selected bit of the PxOUT
 * @return         : N/A
 */
void TimerA_ConfigPWM(const TimerA_PWMConfig *pwmCfg);

/**
 * @brief          : writes on a specific bit of the PxOUT of a target port
 * @param port     : the port whose PxOUT should be modified
 * @param pinMask  : the mask to select the bit to be written
 * @param state    : the logic state to write to the selected bit of the PxOUT
 * @return         : N/A
 */
void TimerA_Start(void);

/**
 * @brief          : writes on a specific bit of the PxOUT of a target port
 * @param port     : the port whose PxOUT should be modified
 * @param pinMask  : the mask to select the bit to be written
 * @param state    : the logic state to write to the selected bit of the PxOUT
 * @return         : N/A
 */
void TimerA_StartInMode(TimerA_Mode mode);

/**
 * @brief          : writes on a specific bit of the PxOUT of a target port
 * @param port     : the port whose PxOUT should be modified
 * @param pinMask  : the mask to select the bit to be written
 * @param state    : the logic state to write to the selected bit of the PxOUT
 * @return         : N/A
 */
void TimerA_Stop(void);

/**
 * @brief          : writes on a specific bit of the PxOUT of a target port
 * @param port     : the port whose PxOUT should be modified
 * @param pinMask  : the mask to select the bit to be written
 * @param state    : the logic state to write to the selected bit of the PxOUT
 * @return         : N/A
 */
DOUBLE_BYTE TimerA_GetCounter(void);

/**
 * @brief          : writes on a specific bit of the PxOUT of a target port
 * @param port     : the port whose PxOUT should be modified
 * @param pinMask  : the mask to select the bit to be written
 * @param state    : the logic state to write to the selected bit of the PxOUT
 * @return         : N/A
 */
void TimerA_ResetCounter(void);

/**
 * @brief          : writes on a specific bit of the PxOUT of a target port
 * @param port     : the port whose PxOUT should be modified
 * @param pinMask  : the mask to select the bit to be written
 * @param state    : the logic state to write to the selected bit of the PxOUT
 * @return         : N/A
 */
void TimerA_SetPeriod(DOUBLE_BYTE period);

/**
 * @brief          : writes on a specific bit of the PxOUT of a target port
 * @param port     : the port whose PxOUT should be modified
 * @param pinMask  : the mask to select the bit to be written
 * @param state    : the logic state to write to the selected bit of the PxOUT
 * @return         : N/A
 */
void TimerA_SetDuty(TimerA_CCRegister channel, DOUBLE_BYTE dutyValue);

/**
 * @brief          : writes on a specific bit of the PxOUT of a target port
 * @param port     : the port whose PxOUT should be modified
 * @param pinMask  : the mask to select the bit to be written
 * @param state    : the logic state to write to the selected bit of the PxOUT
 * @return         : N/A
 */
DOUBLE_BYTE TimerA_GetCaptureCompare(TimerA_CCRegister channel);

#endif /* TIMERA_HAL_19_H_ */