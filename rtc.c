// Copyright 2023 Vahid Mardani
/*
 * This file is part of uaio.
 *  uaio is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  uaio is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with uaio. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Author: Vahid Mardani <vahid.mardani@gmail.com>
 */
#include "rtc.h"

#include "clog.h"
#include "uaio.h"
#include "stm32l0xx.h"


/* Initialize RTC clock. */
void
rtc_init() {
    /* Disable the RTC register write protection. */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    /* Disable the RTC before configure*/
    RCC->CSR &= ~RCC_CSR_RTCEN;

    /* Select the RTC clock source through RTCSEL[1:0] bits in RCC_CSR
     * register.
     *
     * 00: No clock
     * 01: LSE oscillator clock used as RTC clock
     * 10: LSI oscillator clock used as RTC clock
     * 11: HSE oscillator clock divided by a programmable prescaler
     *     (selection through the RTCPRE[1:0] bits in the RCC clock
     *     control register (RCC_CR)) used as the RTC clock
     */
    RCC->CSR &= ~RCC_CSR_RTCSEL;
    RCC->CSR |= RCC_CSR_RTCSEL_0;

    /* Enable the RTC clock by programming the RTCEN bit in the RCC_CSR
       register. */
    RCC->CSR |= RCC_CSR_RTCEN;

    /* Enter to initialization mode */
    RTC->ISR = RTC_ISR_INIT;

    /* Wait to enter initialization mode, 2 RTC clocks */
    while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF) {}

    /* Program the prescaler values */
    RTC->PRER &= ~RTC_PRER_PREDIV_A_Msk;
    RTC->PRER |= 0x7fUL << RTC_PRER_PREDIV_A_Pos;
    RTC->PRER &= ~RTC_PRER_PREDIV_S_Msk;
    RTC->PRER |= 0xffUL << RTC_PRER_PREDIV_S_Pos;

    /* Set initial date and time */
    RTC->TR = 0;
    RTC->DR = 0;

    /* 24 Hours style
     * 0: 24 hour/day format
     * 1: AM/PM hour format
     */
    RTC->CR &= ~RTC_CR_FMT_Msk;

    /* Exit initialization mode */
    RTC->ISR &= ~RTC_ISR_INIT;

    /* Calibration */
    RTC->CR |= RTC_CR_COE;
    RTC->CR &= ~RTC_CR_COSEL;
    while ((RTC->ISR & RTC_ISR_RECALPF) == RTC_ISR_RECALPF) {}
    RTC->CALR |= RTC_CALR_CALP;
    RTC->CALR |= 0xFF;
    // RTC->CALR |= RTC_CALR_CALW8;
    // RTC->CALR |= RTC_CALR_CALW16;
    // RTC->CALR &= ~RTC_CALR_CALM;

    /* Enable RTC register write protection. */
    RTC->WPR = 0xFE;
    RTC->WPR = 0x64;

    /* Enable backup write protection */
    PWR->CR &= ~PWR_CR_DBP;
}


void
print_time() {
    /* To be able to read the RTC calendar register when the APB1 clock
     * frequency is less than seven times the RTC clock frequency (7*RTCLCK),
     * the software must read the calendar time and date registers twice.
     * If the second read of the RTC_TR gives the same result as the first
     * read, this ensures that the data is correct. Otherwise a third read
     * access must be done.
     */

    unsigned long tr = RTC->TR;
    unsigned long tr2 = RTC->TR;

    if (tr != tr2) {
        tr = RTC->TR;
    }

    INFOH("%d%d:%d%d:%d%d",
           (int) (tr & RTC_TR_HT) >> RTC_TR_HT_Pos,
           (int) (tr & RTC_TR_HU) >> RTC_TR_HU_Pos,
           (int) (tr & RTC_TR_MNT) >> RTC_TR_MNT_Pos,
           (int) (tr & RTC_TR_MNU) >> RTC_TR_MNU_Pos,
           (int) (tr & RTC_TR_ST) >> RTC_TR_ST_Pos,
           (int) (tr & RTC_TR_SU) >> RTC_TR_SU_Pos);
}



void
print_date(bool newline) {
    /* To be able to read the RTC calendar register when the APB1 clock
     * frequency is less than seven times the RTC clock frequency (7*RTCLCK),
     * the software must read the calendar time and date registers twice.
     * If the second read of the RTC_TR gives the same result as the first
     * read, this ensures that the data is correct. Otherwise a third read
     * access must be done.
     */

    unsigned long tr = RTC->DR;
    unsigned long tr2 = RTC->DR;

    if (tr != tr2) {
        tr = RTC->DR;
    }

    INFONH("%d%d-%d%d-%d%d%c",
           (int) ((tr & RTC_DR_YT) >> RTC_DR_YT_Pos),
           (int) ((tr & RTC_DR_YU) >> RTC_DR_YU_Pos),
           (int) ((tr & RTC_DR_MT) >> RTC_DR_MT_Pos),
           (int) ((tr & RTC_DR_MU) >> RTC_DR_MU_Pos),
           (int) ((tr & RTC_DR_DT) >> RTC_DR_DT_Pos),
           (int) ((tr & RTC_DR_DU) >> RTC_DR_DU_Pos),
           newline? '\n': ' ');
}
