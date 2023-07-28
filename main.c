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
#include <stdint.h>


#ifdef DEBUG
/* Semihosting debug */
extern void initialise_monitor_handles(void);
#endif


#include "stm32l0xx.h"


#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning \
    "FPU is not initialized, but the project is compiling for an FPU. " \
    "Please initialize the FPU before use."
#endif


/* delay counter */
#define DELAY_MAX       50


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
    printf("HSE Clock: %luHz\n", system_clock);
}


/**
* This function enables the interrupton HSE ready,
* And start the HSE as external clock with crystal + security.
*/
static __INLINE void
clock_init() {
    /* Enable high periority interrupt on RCC */
    NVIC_EnableIRQ(RCC_CRS_IRQn);
    NVIC_SetPriority(RCC_CRS_IRQn, 0);

    /* Enable interrupt on HSE becomes ready */
    RCC->CIER |= RCC_CIER_HSERDYIE;

    /* Disable PLL */
    RCC->CR &= RCC->CR & (~RCC_CR_PLLON);

    /* Enable HSE without security */
    RCC->CR |= RCC_CR_HSEON;
}


int
main(void) {
#ifdef DEBUG
    /* Semihosting debug */
    initialise_monitor_handles();
#endif

    clock_init();
    // uart_init();

    printf("Starting...\n");

    while (1) {}
}
