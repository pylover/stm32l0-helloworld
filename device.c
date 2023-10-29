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
#include "dma.h"
#include "uart.h"
#include "uaio.h"
#include "device.h"
#include "rtc.h"


#ifndef PROD
/* Semihosting debug */
extern void initialise_monitor_handles(void);
#endif


#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning \
    "FPU is not initialized, but the project is compiling for an FPU. " \
    "Please initialize the FPU before use."
#endif


ASYNC
device_init(struct uaio_task *self) {
    CORO_START;
#ifndef PROD
    /* Semihosting debug */
    initialise_monitor_handles();
#endif

    CORO_WAIT(clock_init, NULL);

    DEBUG("Init DMA");
    dma_init();

    DEBUG("Init USART2");
    usart2_init();

    CORO_FINALLY;
}


void
device_standby() {
    /*
     * WFI (Wait for Interrupt) or WFE (Wait for Event) while:
     * – SLEEPDEEP = 1 in Cortex®-M0+ System Control register
     * – PDDS = 1 bit in Power Control register (PWR_CR)
     * – No interrupt (for WFI) or event (for WFE) is pending.
     * – WUF = 0 bit in Power Control/Status register (PWR_CSR)
     * – the RTC flag corresponding to the chosen wakeup source (RTC Alarm A,
     * RTC Alarm B, RTC wakeup, Tamper or Time-stamp flags) is cleared
     */

    DEBUG("Entering standby");

    // DBGMCU->CR |= DBGMCU_CR_DBG_SLEEP;

    /* Enable debuging in standby mode */
    DBGMCU->CR |= DBGMCU_CR_DBG_STANDBY;

    // SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    PWR->CR |= PWR_CR_PDDS;
    PWR->CR |= PWR_CR_CWUF;
    while ((PWR->CSR & PWR_CSR_WUF) != 0) {}
    RTC->ISR &= ~RTC_ISR_WUTF;
    __WFI();
}
