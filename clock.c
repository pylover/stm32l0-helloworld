// Copyright 2023 Vahid Mardani
/*
 * This file is part of stm32l0-helloworld.
 *  stm32l0-helloworld is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  stm32l0-helloworld is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with stm32l0-helloworld. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Author: Vahid Mardani <vahid.mardani@gmail.com>
 */
#include <stdio.h>

#include "clock.h"
#include "stm32l0xx.h"
#include "clog.h"
// #include "uart.h"
#include "uaio.h"
// #include "device.h"
#include "rtc.h"


/* Variable to store millisecond ticks */
volatile uint32_t ticks_ms = 0;


void
SysTick_Handler(void) {
    if (ticks_ms) {
        ticks_ms--;
    }
}


// void
// delay_ms(uint32_t ms) {
//     ticks_ms = ms;
//     while (ticks_ms) {}
// }
//
//
// void
// delay_s(uint32_t s) {
//     while (s--) {
//         delay_ms(1000);
//     }
// }


inline static void
timers_init() {
    /* Enable TIM2 clock */
    __disable_irq();
    TIM2->CR1 &= ~TIM_CR1_CEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* Disable slave mode */
    TIM2->SMCR &= ~TIM_SMCR_SMS;

    /* Edge-aligned count-down mode */
    TIM2->CR1 &= ~TIM_CR1_CMS;
    // TIM2->CR1 |= TIM_CR1_OPM;
    TIM2->CR1 |= TIM_CR1_DIR;

    /* Only counter overflow/underflow generates an update interrupt */
    TIM2->CR1 |= TIM_CR1_URS;

    /* Enable update interrupts */
    TIM2->DIER |= TIM_DIER_UIE;

    /* Prescaler, 1 milisecond */
    TIM2->PSC = (system_clock / 1000) - 1;

    /* Disable the auto-reload/update */
    TIM2->CR1 &= ~TIM_CR1_ARPE;
    TIM2->CR1 &= ~TIM_CR1_UDIS;

    NVIC_EnableIRQ(TIM2_IRQn);
    __enable_irq();

    // /* Enable counter */
    // TIM2->CR1 |= TIM_CR1_CEN;
}


static struct uaio_task *inittask = NULL;


void
RCC_CRS_IRQHandler(void) {
    static int counter = 0;

    /* Check if HSE is ready */
    if ((RCC->CR & RCC_CR_HSERDY) == RCC_CR_HSERDY) {
        printf("HSE is ready\n");

        /* Clear the HSE ready interrupt */
        RCC->CICR |= RCC_CICR_HSERDYC;

        /* Switch SYSCLK to HSE source */
        RCC->CFGR &= ~RCC_CFGR_SW_Msk;
        RCC->CFGR |= RCC_CFGR_SW_HSE;

        /* Update system clock variable */
        system_clock_update();
        INFO("HSE Clock: %luHz\n", system_clock);

        /* SysTick */
        if (SysTick_Config(system_clock / SYSTICKS)) {
            ERROR("SYSTICKS is too large");
        }
        counter++;
    }

    if ((RCC->CSR & RCC_CSR_LSERDY) == RCC_CSR_LSERDY) {
        printf("LSE is ready\n");

        /* Clear the LSE ready interrupt */
        RCC->CICR |= RCC_CICR_LSERDYC;

        counter++;
    }

    if (counter == 2) {
        rtc_init();
        timers_init();

        if (inittask != NULL) {
            inittask->status = UAIO_RUNNING;
            inittask = NULL;
        }
    }
}


/** System clock initialization
 * HSE (12MHz) crystal is selected as main system clock,
 * AHB prescaler: TODO
 * APB prescaler: TODO
 */
/**
* This function enables the interrupton HSE ready,
* And start the HSE as external clock with crystal + security(TODO).
*/
ASYNC
clock_init(struct uaio_task *self) {
    CORO_START;
    inittask = self;

    /* Enable high periority interrupt on RCC */
    NVIC_EnableIRQ(RCC_CRS_IRQn);
    NVIC_SetPriority(RCC_CRS_IRQn, 0);

    /* LSE */
    /* Enable the power interface clock by setting the PWREN bits in the
       RCC_APB1ENR */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    
    /* To be able to debug in low power modes */
    RCC->APB2ENR |= RCC_APB2ENR_DBGMCUEN;
    
    /* Set the DBP(Disable backup write protection) bit in the PWR_CR register
       (see Section 6.4.1). */
    PWR->CR |= PWR_CR_DBP;

    /* Enable LSE */
    /* 32.768Khz crystal is connected to pins: OSC32IN, OSC32OUT */
    // RCC->CSR &= ~RCC_CSR_LSEON;
    RCC->CSR |= RCC_CSR_LSEON;

    /* Enable interrupt on LSE becomes ready */
    RCC->CIER |= RCC_CIER_LSERDYIE;

    /* Disable LSE security */
    RCC->CSR &= ~RCC_CSR_LSECSSON;
    // /* Enable LSE security */
    // RCC->CSR |= RCC_CSR_LSECSSON;

    /* Not bypass LSE */
    RCC->CSR &= ~RCC_CSR_LSEBYP;

    /* LSEDRV; LSE oscillator Driving capability bits
     * These bits are set by software to select the driving capability of the
     * LSE oscillator.
     * They are cleared by a power-on reset or an RTC reset. Once “00” has
     * been written, the content of LSEDRV cannot be changed by software.
     * 00: Lowest drive
     * 01: Medium low drive
     * 10: Medium high drive
     * 11: Highest drive
     */
    RCC->CSR &= ~RCC_CSR_LSEDRV;
    // RCC->CSR |= RCC_CSR_LSEDRV_0 | RCC_CSR_LSEDRV_1;
    RCC->CSR |= RCC_CSR_LSEDRV_1;

    /* HSE */
    /* Enable interrupt on HSE becomes ready */
    RCC->CIER |= RCC_CIER_HSERDYIE;

    /* Disable PLL */
    RCC->CR &= ~RCC_CR_PLLON;
    RCC->CFGR &= ~RCC_CFGR_PLLDIV;

    /* Enable HSE without security */
    RCC->CR |= RCC_CR_HSEON;

    /* AHB */
    RCC->AHBENR &= ~RCC_AHBENR_CRYPEN;
    RCC->AHBENR &= ~RCC_AHBENR_CRCEN;

    /* Bits 7:4 HPRE[3:0]: AHB prescaler
     * 0xxx: SYSCLK not divided
     * 1000: SYSCLK divided by 2
     * 1001: SYSCLK divided by 4
     * 1010: SYSCLK divided by 8
     * 1011: SYSCLK divided by 16
     * 1100: SYSCLK divided by 64
     * 1101: SYSCLK divided by 128
     * 1110: SYSCLK divided by 256
     * 1111: SYSCLK divided by 512
     */
    RCC->CFGR &= ~RCC_CFGR_HPRE_Msk;
    RCC->CFGR |= RCC_CFGR_HPRE_DIV4;

    /* APB */
    /* APB2 */
    /* Bits 13:11 PPRE2[2:0]: APB high-speed prescaler (APB2)
     * These bits are set and cleared by software to control the division
     * factor of the APB high-speed clock (PCLK2).
     * 0xx: HCLK not divided
     * 100: HCLK divided by 2
     * 101: HCLK divided by 4
     * 110: HCLK divided by 8
     * 111: HCLK divided by 16
     */
    RCC->CFGR &= ~RCC_CFGR_PPRE2;
    // RCC->CFGR |= ~RCC_CFGR_PPRE2_DIV2;

    /* APB1 */
    /* Bits 10:8 PPRE1[2:0]: APB low-speed prescaler (APB1)
     * These bits are set and cleared by software to control the division
     * factor of the APB low-speed clock (PCLK1).
     * 0xx: HCLK not divided
     * 100: HCLK divided by 2
     * 101: HCLK divided by 4
     * 110: HCLK divided by 8
     * 111: HCLK divided by 16
     */
    RCC->CFGR &= ~RCC_CFGR_PPRE1;
    // RCC->CFGR |= ~RCC_CFGR_PPRE1_DIV2;

    CORO_WAITI();
    CORO_FINALLY;
}
