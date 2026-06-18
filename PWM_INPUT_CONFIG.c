/*****************************************************************************
 * File Name          : PWM_INPUT_CONFIG.c
 * Author             : User Customizable
 * Version            : V1.0.0
 * Date               : 2024
 * Description        : PWM Input Configuration for TIM2 Channel 1 (INPUT pin)
 *                      Edit this file to change PWM input parameters
 *****************************************************************************/

#include "ch32v20x.h"
#include "ch32v20x_gpio.h"
#include "ch32v20x_rcc.h"
#include "ch32v20x_tim.h"

// =============================================================================
// EDITABLE PARAMETERS - MODIFY HERE FOR YOUR PWM INPUT SETTINGS
// =============================================================================

// ===== 1. TIMER CONFIGURATION =====
#define PWM_TIMER                TIM2                    // Timer to use (TIM1-TIM5)
#define PWM_TIMER_PERIOD         0xFFFF                  // Timer period (0-65535)
#define PWM_TIMER_PRESCALER      48                      // Prescaler value (affects time resolution)
                                                         // Resolution = (Prescaler+1) / Timer_Clock
                                                         // For 48MHz: 48 = 1us per tick

// ===== 2. INPUT CAPTURE CHANNEL CONFIGURATION =====
#define PWM_INPUT_CHANNEL        TIM_Channel_1           // TIM2 Channel 1 = INPUT pin
                                                         // Options: TIM_Channel_1, TIM_Channel_2, TIM_Channel_3, TIM_Channel_4

// ===== 3. EDGE DETECTION CONFIGURATION =====
#define PWM_POLARITY             TIM_ICPolarity_Rising   // Which edge to detect
                                                         // Options: 
                                                         // - TIM_ICPolarity_Rising (default, standard PWM)
                                                         // - TIM_ICPolarity_Falling
                                                         // - TIM_ICPolarity_BothEdge (measures both edges)

// ===== 4. INPUT SELECTION =====
#define PWM_INPUT_SELECTION      TIM_ICSelection_DirectTI // How to connect input
                                                           // Options:
                                                           // - TIM_ICSelection_DirectTI (direct connection)
                                                           // - TIM_ICSelection_IndirectTI (indirect)
                                                           // - TIM_ICSelection_TRC (trigger input)

// ===== 5. SAMPLING/PRESCALER CONFIGURATION =====
#define PWM_PRESCALER            TIM_ICPSC_DIV1          // How often to sample
                                                         // Options:
                                                         // - TIM_ICPSC_DIV1 (every edge - most accurate)
                                                         // - TIM_ICPSC_DIV2 (every 2 edges)
                                                         // - TIM_ICPSC_DIV4 (every 4 edges)
                                                         // - TIM_ICPSC_DIV8 (every 8 edges - for noise filtering)

// ===== 6. NOISE FILTER CONFIGURATION =====
#define PWM_FILTER_VALUE         0x00                    // Input capture filter (0-15)
                                                         // 0x00 = no filter (fastest response)
                                                         // 0x01-0x03 = light filtering
                                                         // 0x04-0x07 = medium filtering
                                                         // 0x08-0x0F = heavy filtering (best noise rejection)

// ===== 7. INTERRUPT CONFIGURATION =====
#define PWM_ENABLE_INTERRUPT     1                       // Enable capture interrupt: 0=disabled, 1=enabled
#define PWM_INTERRUPT_PRIORITY   1                       // Interrupt priority (0-3, lower = higher priority)

// =============================================================================
// INITIALIZATION FUNCTION - CALL THIS IN YOUR MAIN SETUP
// =============================================================================

void PWM_Input_Init(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseInitStruct;
    TIM_ICInitTypeDef        TIM_ICInitStruct;
    GPIO_InitTypeDef         GPIO_InitStruct;
    NVIC_InitTypeDef         NVIC_InitStruct;

    // ===== Step 1: Enable Clocks =====
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // ===== Step 2: Configure GPIO Pin (PA0 for TIM2 CH1) =====
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;              // INPUT pin
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;  // Input floating
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // ===== Step 3: Configure Timer Base =====
    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
    TIM_TimeBaseInitStruct.TIM_Period = PWM_TIMER_PERIOD;
    TIM_TimeBaseInitStruct.TIM_Prescaler = PWM_TIMER_PRESCALER;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(PWM_TIMER, &TIM_TimeBaseInitStruct);

    // ===== Step 4: Configure Input Capture =====
    TIM_ICStructInit(&TIM_ICInitStruct);
    TIM_ICInitStruct.TIM_Channel = PWM_INPUT_CHANNEL;
    TIM_ICInitStruct.TIM_ICPolarity = PWM_POLARITY;
    TIM_ICInitStruct.TIM_ICSelection = PWM_INPUT_SELECTION;
    TIM_ICInitStruct.TIM_ICPrescaler = PWM_PRESCALER;
    TIM_ICInitStruct.TIM_ICFilter = PWM_FILTER_VALUE;
    TIM_ICInit(PWM_TIMER, &TIM_ICInitStruct);

    // ===== Step 5: Enable Interrupt (Optional) =====
    if(PWM_ENABLE_INTERRUPT)
    {
        TIM_ITConfig(PWM_TIMER, TIM_IT_CC1, ENABLE);
        
        // Configure NVIC
        NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = PWM_INTERRUPT_PRIORITY;
        NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStruct);
    }

    // ===== Step 6: Enable Timer =====
    TIM_Cmd(PWM_TIMER, ENABLE);
}

// =============================================================================
// FUNCTIONS TO READ PWM VALUES
// =============================================================================

/**
 * @fn      Get_PWM_Pulse_Width
 * @brief   Get the PWM pulse width (high time)
 * @return  Pulse width in timer ticks
 */
uint16_t Get_PWM_Pulse_Width(void)
{
    return TIM_GetCapture1(PWM_TIMER);  // Rising edge capture
}

/**
 * @fn      Get_PWM_Period
 * @brief   Get the PWM period (total time)
 * @return  Period in timer ticks
 */
uint16_t Get_PWM_Period(void)
{
    return TIM_GetCapture2(PWM_TIMER);  // Falling edge capture
}

/**
 * @fn      Get_PWM_Frequency
 * @brief   Calculate PWM frequency in Hz
 * @return  Frequency in Hz
 */
uint32_t Get_PWM_Frequency(void)
{
    uint16_t period = Get_PWM_Period();
    if(period == 0) return 0;
    
    // Timer clock = 48MHz / (Prescaler+1)
    uint32_t timer_clock = 48000000 / (PWM_TIMER_PRESCALER + 1);
    
    // Frequency = Timer_Clock / Period
    return timer_clock / period;
}

/**
 * @fn      Get_PWM_Duty_Cycle
 * @brief   Calculate PWM duty cycle as percentage
 * @return  Duty cycle (0-100)
 */
uint8_t Get_PWM_Duty_Cycle(void)
{
    uint16_t pulse = Get_PWM_Pulse_Width();
    uint16_t period = Get_PWM_Period();
    
    if(period == 0) return 0;
    
    return (pulse * 100) / period;
}

// =============================================================================
// INTERRUPT HANDLER (Optional - only if PWM_ENABLE_INTERRUPT = 1)
// =============================================================================

void TIM2_IRQHandler(void)
{
    if(TIM_GetITStatus(PWM_TIMER, TIM_IT_CC1))
    {
        TIM_ClearITPendingBit(PWM_TIMER, TIM_IT_CC1);
        
        // Add your code here when PWM capture occurs
        // Example: Update PWM values, check thresholds, etc.
        
        uint16_t pulse = Get_PWM_Pulse_Width();
        uint16_t period = Get_PWM_Period();
        uint8_t duty = Get_PWM_Duty_Cycle();
        
        // You can use these values here
        // pulse, period, duty are now available
    }
}

// =============================================================================
// CONFIGURATION REFERENCE - DO NOT EDIT BELOW
// =============================================================================

/*
 * TIMER PRESCALER CALCULATION:
 * If your timer clock is 48MHz and you want 1us resolution:
 *   Prescaler = (Clock / Desired_Frequency) - 1
 *   Prescaler = (48MHz / 1MHz) - 1 = 47
 *   Or use 48 for approximately 1us
 *
 * COMMON PRESCALER VALUES:
 *   Prescaler = 47  → ~1 us per tick (48MHz)
 *   Prescaler = 95  → ~2 us per tick
 *   Prescaler = 479 → ~10 us per tick
 *   Prescaler = 4799 → ~100 us per tick
 *
 * ========================================================================
 *
 * PWM FREQUENCY MEASUREMENT RANGES:
 * With Prescaler = 48 (1us per tick) and Period = 65535:
 *   Max Frequency = 48MHz / 1 = 48 MHz
 *   Min Frequency = 48MHz / 65535 = ~733 Hz
 *
 * With Prescaler = 4799 (100us per tick) and Period = 65535:
 *   Max Frequency = 48MHz / 65536 = ~732 Hz
 *   Min Frequency = 48MHz / (65535*4800) = ~0.152 Hz
 *
 * ========================================================================
 *
 * NOISE FILTER VALUES (TIM_ICFilter):
 *   0x0 = fSAMPLING=fCK_INT (no filter, fastest)
 *   0x1 = fSAMPLING=fCK_INT, N=2
 *   0x2 = fSAMPLING=fCK_INT, N=4
 *   0x3 = fSAMPLING=fCK_INT, N=8
 *   0x4 = fSAMPLING=fCK_INT/2, N=6
 *   0x5 = fSAMPLING=fCK_INT/2, N=8
 *   0x6 = fSAMPLING=fCK_INT/4, N=6
 *   0x7 = fSAMPLING=fCK_INT/4, N=8
 *   0x8 = fSAMPLING=fCK_INT/8, N=6
 *   0x9 = fSAMPLING=fCK_INT/8, N=8
 *   0xA = fSAMPLING=fCK_INT/16, N=5
 *   0xB = fSAMPLING=fCK_INT/16, N=6
 *   0xC = fSAMPLING=fCK_INT/16, N=8
 *   0xD = fSAMPLING=fCK_INT/32, N=5
 *   0xE = fSAMPLING=fCK_INT/32, N=6
 *   0xF = fSAMPLING=fCK_INT/32, N=8
 *
 * ========================================================================
 *
 * PIN MAPPINGS FOR TIM2:
 *   TIM2_CH1 = PA0 (INPUT pin in your schematic)
 *   TIM2_CH2 = PA1
 *   TIM2_CH3 = PA2
 *   TIM2_CH4 = PA3
 *
 * ========================================================================
 *
 * TYPICAL PWM RANGES:
 *   RC Servo/Throttle: 1ms - 2ms pulse, 50Hz frequency
 *   DShot300: 3-150µs, 32kHz
 *   DShot600: 1.5-75µs, 32kHz
 *   Standard PWM: Variable depending on application
 *
 */
