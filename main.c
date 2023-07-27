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


/* Semihosting debug */
extern void initialise_monitor_handles(void);


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
    if ((RCC->CR & RCC_CR_HSERDY) == 0) {
        printf("HSE is not ready ready\n");
        return;
    }

    RCC->CICR |= RCC_CICR_HSERDYC;
    RCC->CFGR |= RCC_CFGR_SW_HSE;
    SystemCoreClockUpdate();
    printf("HSE Clock: %luHz\n", SystemCoreClock);
}


/**
* This function enables the interrupton HSE ready,
* And start the HSE as external clock with crystal + security.
*/
static __INLINE void
clock_init() {
    SystemCoreClockUpdate();
    printf("Clock before hse: %luHz\n", SystemCoreClock);

    /* Enable high periority interrupt on RCC */
    NVIC_EnableIRQ(RCC_CRS_IRQn);
    NVIC_SetPriority(RCC_CRS_IRQn, 0);

    /* Enable interrupt on HSE becomes ready */
    RCC->CIER |= RCC_CIER_HSERDYIE;

    /* Enable HSE with security */
    RCC->CR |= RCC_CR_HSEON;

    // // SYSCLK
    // // AHB, APB1, APB2
}


int
main(void) {
    /* Semihosting debug */
    initialise_monitor_handles();

    clock_init();
    // uart_init();

    printf("Starting...\n");

    // NVIC_EnableIRQ(RCC_CRS_IRQn);
    // NVIC_SetPriority(RCC_CRS_IRQn,0);
    // RCC->CIER |= RCC_CIER_HSERDYIE;
    // RCC->CR |= RCC_CR_HSERDY;
    // /* turn on clock on GPIOA */
    // RCC->AHBENR |= RCC_AHBENR_CRCEN;

    // /* set PA5 to output mode */
    // GPIOA->MODER &= ~GPIO_MODER_MODE5_1;
    // GPIOA->MODER |=  GPIO_MODER_MODE5_0;

    // while(1) {
    //   /* set HIGH value on pin PA5 */
    //   GPIOA->BSRR |= GPIO_BSRR_BS_5;
    //   for(uint32_t i=DELAY_MAX; i--;) {}

    //   /* set LOW value on pin PA5 */
    //   GPIOA->BSRR |= GPIO_BSRR_BR_5;
    //   for(uint32_t i=DELAY_MAX; i--;) {}
    // }
}
