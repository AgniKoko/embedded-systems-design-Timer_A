# Timer_A HAL for MSP430x1xx

This repository contains the implementation of a Hardware Abstraction Layer (HAL) for
the **Timer_A** peripheral of the MSP430x1xx microcontroller family, developed as
part of the course *Embedded Systems Design*.

The goal of the HAL is to hide low-level register manipulation behind a small,
type-safe API, so that Timer_A can be configured and used as a simple software
component (counter / PWM generator) in application code. The design follows the
same philosophy as the GPIO HAL used in the lab exercises and is based on
chapter 11 of the official *MSP430x1xx User’s Guide* (Timer_A module description
and control registers).

<img width="693" height="690" alt="image" src="https://github.com/user-attachments/assets/83c7dd3b-e349-4076-b17a-ac765326a2ac" />


## File overview

- `timerA_HAL_19.h`  
  Public interface of the Timer_A HAL:
  - basic typedefs (`BYTE`, `DOUBLE_BYTE`)
  - high-level enums for TACTL and TACCTLx fields (clock source, divider, mode,
    output mode, etc.)
  - bitfield representations of TACTL and TACCTLx
  - configuration structs:
    - `TimerA_Config` – global timer configuration (clock source/divider, mode,
      period, timer interrupt enable)
    - `TimerA_ChannelConfig` – per-channel compare/PWM configuration
    - `TimerA_PWMConfig` – convenience struct for PWM setup
  - public API prototypes (`TimerA_ApplyConfig`, `TimerA_ConfigureChannel`,
    `TimerA_ConfigPWM`, `TimerA_Start`, `TimerA_Stop`, etc.).

- `timerA_HAL_19.c`  
  Implementation of the HAL on top of the memory-mapped Timer_A control
  registers:
  - bit masks for `TACTL` and `TACCTLx`
  - static helpers to select the correct `TACCTLx` / `TACCRx` register based on
    channel (`CC_REGISTER_0/1/2`)
  - implementation of:
    - `TimerA_ApplyConfig()` – writes TACTL and TACCR0 according to a
      `TimerA_Config`
    - `TimerA_ConfigureChannel()` – configures TACCTLx/TACCRx for compare or
      PWM use
    - `TimerA_ConfigPWM()` – high-level PWM setup based on `TimerA_PWMConfig`
    - basic control / utility calls: `TimerA_Start`,
      `TimerA_StartInMode`, `TimerA_Stop`,
      `TimerA_GetCounter`, `TimerA_ResetCounter`,
      `TimerA_SetPeriod`, `TimerA_SetDuty`,
      `TimerA_GetCaptureCompare`. 

- `timerA_main_19.c`  
  Example `main()` demonstrating how to use the HAL:
  1. **Example 1 – Basic counter**  
     Configures Timer_A in up mode, reads TAR multiple times, resets the
     counter and stops the timer.
  2. **Example 2 – PWM on CCR1**  
     Uses `TimerA_ConfigPWM()` to generate a 50 % duty-cycle PWM signal on
     CCR1.
  3. **Example 3 – Dynamic duty-cycle change**  
     Changes the CCR1 duty cycle at run-time (50 % → 75 % → 25 %) using
     `TimerA_SetDuty()` and reads back the CCR1 value with
     `TimerA_GetCaptureCompare()`. 

## Target platform & dependencies

- **MCU family:** MSP430x1xx  
- **Timer peripheral:** Timer_A  
- The implementation assumes that the vendor device header  
  (e.g. `msp430x14x.h`) is available in the include path so that the
  memory-mapped registers (`TACTL`, `TAR`, `TACCTL0–2`, `TACCR0–2`, …) are
  defined.  
- Typical toolchains: IAR Embedded Workbench, Code Composer Studio, or other
  MSP430-compatible compilers.

## Quick usage example

Basic Timer_A configuration as a simple counter:

```c
#include "timerA_HAL_19.h"

void example_counter(void)
{
    TimerA_Config cfg;

    cfg.clockSource          = CLOCK_SMCLK;
    cfg.clockDivider         = DIV_8;
    cfg.mode                 = MODE_UP;
    cfg.period               = 1000u;
    cfg.enableTimerInterrupt = 0u;

    TimerA_ApplyConfig(&cfg);
    TimerA_Start();

    DOUBLE_BYTE t1 = TimerA_GetCounter();
    // ...
    TimerA_Stop();
}
```

Basic PWM configuration on CCR1:

```c
void example_pwm(void)
{
    TimerA_PWMConfig pwm;

    pwm.channel    = CC_REGISTER_1;
    pwm.period     = 1000u;  // TACCR0
    pwm.duty       = 500u;   // TACCR1 -> 50% duty
    pwm.outputMode = OUTMODE_RST_SET;

    TimerA_ConfigPWM(&pwm);
    TimerA_Start();          // already running after ConfigPWM(), but safe
}
```

## Documentation

The full course report describing the theoretical background, design choices
and mapping to the MSP430x1xx Timer_A control registers is available under:

- `docs/timerA_report.pdf`

An HTML/Markdown summary is also provided in:

- `docs/index.md`
