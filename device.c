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
#include "stm32l0xx.h"


#ifndef PROD
/* Semihosting debug */
extern void initialise_monitor_handles(void);
#endif


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


/* Initialize RTC clock. */
inline static void
rtc_init() {
    /* Disable the RTC register write protection. */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

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
    RTC->ISR = ~RTC_ISR_INIT_Msk;

    /* Enable RTC register write protection. */
    RTC->WPR = 0xFE;
    RTC->WPR = 0x64;
}


void
RCC_CRS_IRQHandler(void) {
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
    }

    if ((RCC->CSR & RCC_CSR_LSERDY) == RCC_CSR_LSERDY) {
        printf("LSE is ready\n");

        /* Clear the LSE ready interrupt */
        RCC->CICR |= RCC_CICR_LSERDYC;

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
        RCC->CSR &= ~RCC_CSR_RTCSEL_Msk;
        RCC->CSR |= RCC_CSR_RTCSEL_0;

        /* Enable the RTC clock by programming the RTCEN bit in the RCC_CSR
           register. */
        RCC->CSR |= RCC_CSR_RTCEN;

        rtc_init();
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
inline static void
clock_init() {
    /* Enable high periority interrupt on RCC */
    NVIC_EnableIRQ(RCC_CRS_IRQn);
    NVIC_SetPriority(RCC_CRS_IRQn, 0);

    /* LSE */
    /* Enable the power interface clock by setting the PWREN bits in the
       RCC_APB1ENR */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    /* Set the DBP(Disable backup write protection) bit in the PWR_CR register
       (see Section 6.4.1). */
    PWR->CR |= PWR_CR_DBP;

    /* Enable interrupt on LSE becomes ready */
    RCC->CIER |= RCC_CIER_LSERDYIE;

    /* Not bypass LSE */
    RCC->CSR &= ~RCC_CSR_LSEBYP;

    /* Enable LSE */
    /* 32.768Khz crystal is connected to pins: OSC32IN, OSC32OUT */
    RCC->CSR &= ~RCC_CSR_LSEON;
    RCC->CSR |= RCC_CSR_LSEON;

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
    RCC->CSR |= RCC_CSR_LSEDRV_0 | RCC_CSR_LSEDRV_1;

    /* HSE */
    /* Enable interrupt on HSE becomes ready */
    RCC->CIER |= RCC_CIER_HSERDYIE;

    /* Disable PLL */
    RCC->CR &= ~RCC_CR_PLLON;
    RCC->CFGR &= ~RCC_CFGR_PLLDIV;

    /* Enable HSE without security */
    RCC->CR |= RCC_CR_HSEON;

    /* AHB */
    RCC->CFGR &= ~RCC_CFGR_HPRE_Msk;
    RCC->CFGR |= RCC_CFGR_HPRE_DIV4;
    RCC->AHBENR &= ~RCC_AHBENR_CRYPEN;
    RCC->AHBENR &= ~RCC_AHBENR_CRCEN;
    RCC->AHBENR |= RCC_AHBENR_DMAEN;
}


void
dma_memory_to_peripheral_circular(volatile uint32_t *peripheral,
        const char *data, uint32_t count) {
    /* Set USART1 TX data register address into DMA Channel 4 */
    DMA1_CH3->CPAR = (uint32_t)peripheral;

    /* Set pointer to data to be sent */
    DMA1_CH3->CMAR = (uint32_t)data;

    /* Set size of data to be sent */
    DMA1_CH3->CNDTR = count;

    /* Configure the channel priority to medium level */
    /*
    00: low
    01: medium
    10: high
    11: very high
    */
    DMA1_CH3->CCR &= ~(DMA_CCR_PL_0 | DMA_CCR_PL_1);

    /* Configure data transfer direction, peripheral & memory incremented
       mode, and peripheral & memory data size */

    /* Set data transfer direction: memory -> peripheral */
    /*
    0: read from peripheral
    1: read from memory
    */
    DMA1_CH3->CCR |= DMA_CCR_DIR;

    /* Set the memory and pripheral write chunk size to one byte */
    /*
    00: 8 bits
    01: 16 bits
    10: 32 bits
    11: reserved
    */
    DMA1_CH3->CCR &= ~(DMA_CCR_MSIZE_0 | DMA_CCR_MSIZE_0);
    DMA1_CH3->CCR &= ~(DMA_CCR_PSIZE_0 | DMA_CCR_PSIZE_0);

    /* Set memory address incement by one byte */
    DMA1_CH3->CCR |= DMA_CCR_MINC;

    /* Disable address incrementation on peripheral address */
    DMA1_CH3->CCR &= ~DMA_CCR_PINC;

    /* Enable circular mode */
    DMA1_CH3->CCR |= DMA_CCR_CIRC;

    /* Enable interrpt after full transfer */
    DMA1_CH3->CCR |= DMA_CCR_TCIE;
}


/** USART2 initializer
  * baud = 2 X PCLK / USART2DIV
  * div = 2 X PCLK / baud
  */
static void
usart2_init() {
    /*
    USART2 pins
    RX,     pin 12, PA2, APB1
    TX,     pin 13, PA3, APB1
    Wakeup, pin 10, PA0
    */
    // const char * msg = "Hello\r\n";

    /* Enable clock for GPIOA and USART1 */
    RCC->IOPENR |= RCC_IOPENR_IOPAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* Oversampling

    Select oversampling by 8 (OVER8=1) to achieve higher speed (up to fCK/8).
    In this case the maximum receiver tolerance to clock deviation is reduced

    Select oversampling by 16 (OVER8=0) to increase the tolerance of the
    receiver to clock deviations. In this case, the maximum speed is limited
    to maximum fCK/16 where fCK is the clock source frequency.
    */
    USART2->CR1 |= USART_CR1_OVER8;

    /* Clock configuration register (RCC_CCIPR)
    USART2SEL: USART2 clock source selection bits
    This bit is set and cleared by software.
    00: APB clock selected as USART2 clock
    01: System clock selected as USART2 clock
    10: HSI16 clock selected as USART2 clock
    11: LSE clock selected as USART2 clock
    */
    // RCC->CCIPR |=

    /* TODO: USART2 clock enable during Sleep mode bit */
    /* RCC_APB1SMENR: Bit 17 USART2SMEN */

    /* Reset mode configuration bits for PA2 and PA3 */
    GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);

    /* Select alternate function mode for PA2 and PA3 */
    GPIOA->MODER |= GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1;

    /* Select alternate functions of PA2 and PA3 */
    GPIOA->AFRL &= ~(GPIO_AFRL_AFSEL2_Msk | GPIO_AFRL_AFSEL3_Msk);
    GPIOA->AFRL |=
        GPIOA_AFRL_AFSEL2_AF2_USART2_TX |
        GPIOA_AFRL_AFSEL3_AF2_USART2_RX;

    /* Word length: 00: 1 Start bit, 8 data bits, n stop bits */
    USART2->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0);

    /* Enable the USART2 */
    USART2->CR1 = USART_CR1_UE;

    // dma_memory_to_peripheral_circular(&USART1->TDR, msg, strlen(msg));
    // // TODO:
    // /* Enable the channel */
    // //DMA1_CH3->CCR |= DMA_CCR_EN;

    // /* Enable DMA mode for transmitter */
    // //USART1->CR3 |= USART_CR3_DMAT;
}


void
device_init() {
#ifndef PROD
    /* Semihosting debug */
    initialise_monitor_handles();
#endif

    clock_init();
    usart2_init();
}
