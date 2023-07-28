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
#include "clog.h"
#include "device.h"


#ifndef PROD
/* Semihosting debug */
extern void initialise_monitor_handles(void);
#endif


#include "stm32l0xx.h"


#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning \
    "FPU is not initialized, but the project is compiling for an FPU. " \
    "Please initialize the FPU before use."
#endif


/* Variable to store millisecond ticks */
volatile uint32_t ticks_ms = 0;


void
SysTick_Handler(void) {
    if (ticks_ms) {
        ticks_ms--;
    }
}


void
RCC_CRS_IRQHandler(void) {
    /* Check if HSE is ready */
    if ((RCC->CR & RCC_CR_HSERDY) == 0) {
        printf("HSE is not ready ready\n");
        return;
    }

    /* Clear the HSE ready interrupt */
    RCC->CICR |= RCC_CICR_HSERDYC;

    /* Switch SYSCLK to HSE source */
    RCC->CFGR |= RCC_CFGR_SW_HSE;

    /* Update system clock variable */
    system_clock_update();
    INFO("HSE Clock: %luHz\n", system_clock);

    /* SysTick */
    if (SysTick_Config(system_clock / SYSTICKS)) {
        ERROR("SYSTICKS is too large");
    }
}


/**
* This function enables the interrupton HSE ready,
* And start the HSE as external clock with crystal + security.
*/
inline static void
clock_init() {
    /* Enable high periority interrupt on RCC */
    NVIC_EnableIRQ(RCC_CRS_IRQn);
    NVIC_SetPriority(RCC_CRS_IRQn, 0);

    /* Enable interrupt on HSE becomes ready */
    RCC->CIER |= RCC_CIER_HSERDYIE;

    /* Disable PLL */
    RCC->CR &= (uint32_t)(~RCC_CR_PLLON);

    /* Enable HSE without security */
    RCC->CR |= RCC_CR_HSEON;
}


void
delay_ms(uint32_t ms) {
    ticks_ms = ms;
    while (ticks_ms) {}
}


void
delay_s(uint32_t s) {
    while (s--) {
        delay_ms(1000);
    }
}


void
device_init() {
#ifndef PROD
    /* Semihosting debug */
    initialise_monitor_handles();
#endif

    clock_init();
}
