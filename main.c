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


#include "clock.h"
#include "stm32l0xx.h"
#include "clog.h"
#include "rtc.h"
#include "device.h"
#include "uart.h"
#include "uaio.h"


void
config_rtc_raw() {
    /* disable rtc before configuration */
    RCC->CSR &= ~RCC_CSR_RTCEN;

	RCC->APB2ENR |= RCC_APB2ENR_DBGMCUEN; // To be able to debug in low power modes

	/* to be able to debug in standby mode */
    DBGMCU->CR |= DBGMCU_CR_DBG_STANDBY;

    /* Enable Interrupt on RTC */
	NVIC_EnableIRQ(RTC_IRQn);
    /* Set priority for RTC */
    NVIC_SetPriority(RTC_IRQn,0);

    /* Enable RTC alarm going through EXTI 20 line to NVIC */
    EXTI->IMR |= EXTI_IMR_IM20;
    EXTI->RTSR |= EXTI_IMR_IM20;

    /* Disable backup write protection */
    PWR->CR |= PWR_CR_DBP;
    /* Clear the WUF flag */
    PWR->CR |= PWR_CR_CWUF;
    /* V_{REFINT} is off in low-power mode */
    PWR->CR |= PWR_CR_ULP;
    PWR->CR |= PWR_CR_PDDS;

    /* reset RTC */
    /* Reset and Clock Control */
    RCC->CSR |= RCC_CSR_RTCRST;
    RCC->CSR &=~ RCC_CSR_RTCRST;

	/* Power interface clock enabled */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    /* Select the RTC clock source: LSE oscillator */
    RCC->CSR |= RCC_CSR_RTCSEL_LSE;
    /* Enable the interal LSE oscillator */
    RCC->CSR |= RCC_CSR_LSEON;

    /* Enable write access for RTC registers */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

	/* RTC configuration */
    /* Disable wake up timer to modify it */
    RTC->CR &=~ RTC_CR_WUTE;
    /* Wait until it is allowed to modify wake up reload value */
    while((RTC->ISR & RTC_ISR_WUTWF) != RTC_ISR_WUTWF) {
    }

    /* Wake up value reload counter [s] */
    RTC->WUTR = 0x5;

    /* OSEL = 0x3 -> RTC_ALARM output = Wake up timer */
    RTC->CR |= RTC_CR_OSEL;
    /* WUCKSEL = 0x4 -> RTC Timer [1s - 18h] */
    RTC->CR |= RTC_CR_WUCKSEL_2;
    // RTC->CR &= ~RTC_CR_WUCKSEL;
    /* Enable wake up counter/interrupt */
    RTC->CR |= RTC_CR_WUTE | RTC_CR_WUTIE;

	/* Disable write access for RTC registers */
    RTC->WPR = 0xFE;
    RTC->WPR = 0x64;

    /* enable rtc clock */
	RCC->CSR |= RCC_CSR_RTCEN;
}


static ASYNC
startA(struct uaio_task *self) {
    static struct uaio_sleep sleep = {2000};
    // static struct usart usart2 = {
    //     .send = "hello\n",
    //     .sendlen = 6,

    // };

    CORO_START;
    INFO("Initializing...");
    CORO_WAIT(device_init, NULL);

    // config_rtc_raw();
    // rtc_autowakup_init();
    com_rtc_autowakup_init();
    INFO("Starting...");

    while (1) {
        CORO_WAIT(sleepA, &sleep);
        print_date(false);
        print_time();

        // CORO_WAIT(usart2_sendA, &usart2);
        device_standby();
    }

    CORO_FINALLY;
}


int
main(void) {
#ifdef PROD
    clog_verbosity = CLOG_SILENT;
#else
    clog_verbosity = CLOG_DEBUG;
#endif

    return UAIO(startA, NULL, 2);
}
