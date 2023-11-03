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
#include <stdlib.h>

#include "stm32l0xx.h"

#include "uart.h"
#include "dma.h"


static struct usart *usart2 = NULL;


/** USART2 initializer
  * baud = 2 X PCLK / USART2DIV
  * div = 2 X PCLK / baud
  */
void
usart2_init() {
    /* Allocate memory for usart2 state */
    usart2 = malloc(sizeof(struct usart));
    if (usart2 == NULL) {
        ERROR("Out of memory when allocating for usart2");
        return;
    }
    usart2->sendlen = 0;
    dma_setup(DMA1_CH4, DMA_MEM2PERI, (void*)usart2->send,
            (void*)&USART2->TDR);

    /*
    USART2 pins
    TX,     pin 12, PA2, APB1
    RX,     pin 13, PA3, APB1
    Wakeup, pin 10, PA0
    */
    // const char * msg = "Hello\r\n";

    /* Enable clock for USART2 */
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

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
    GPIOA->MODER &= ~GPIO_MODER_MODE2;

    /* Push/Pull mode for PA2 */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT_2;

    /* High speed mode for PA2 */
    GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED2;

    /* Select alternate function mode for PA2 and PA3 */
    GPIOA->MODER |= GPIO_MODER_MODE2_1;

    /* Select speed for PA2 */
    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED2_1;

    /* Alternate function selection for PA2 */
    GPIOA->AFRL &= ~GPIO_AFRL_AFSEL2_Msk;
    GPIOA->AFRL |= GPIOA_AFRL_AFSEL2_AF4_USART2_TX;

    /* Disable USART2 */
    CLEAR_BIT(USART2->CR1, USART_CR1_UE);

    /* CR1 configuration */
    /* Word length: 00: 1 Start bit, 8 data bits, n stop bits */
    USART2->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0);

    /* Oversampling
    Select oversampling by 8 (OVER8=1) to achieve higher speed (up to fCK/8).
    In this case the maximum receiver tolerance to clock deviation is reduced

    Select oversampling by 16 (OVER8=0) to increase the tolerance of the
    receiver to clock deviations. In this case, the maximum speed is limited
    to maximum fCK/16 where fCK is the clock source frequency.
    */
    /* This will cause problem in BAUD rate selection, commented to be dealt
       later */
    // USART2->CR1 |= USART_CR1_OVER8;

    /* BRR register configuration */
    uint32_t baud_rate = 115200;
    USART2->BRR = (uint32_t) system_clock / baud_rate;

    /* Enable USART2 Transmitter */
    SET_BIT(USART2->CR1, USART_CR1_TE);

    /* Enable USART2 */
    SET_BIT(USART2->CR1, USART_CR1_UE);
}


void
usart2_deinit() {
    /* Disable USART2 */
    CLEAR_BIT(USART2->CR1, USART_CR1_UE);

    if (usart2) {
        free(usart2);
    }
}


void
usart2_write(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    usart2->sendlen += vsnprintf(usart2->send + usart2->sendlen,
            USART2_SENDBUFF_SIZE - usart2->sendlen, fmt, ap);
    va_end(ap);
}


ASYNC
usart2_sendA(struct uaio_task *self) {
    CORO_START;

    DEBUGN("Sending: %.*s", usart2->sendlen, usart2->send);

    /* Enable USART2 DMA request */
    SET_BIT(USART2->CR3, USART_CR3_DMAT);

    DMA_CH4_WAIT((void*)usart2->sendlen);
    // DEBUG("Transfer completed");

    /* Disable USART2 DMA request */
    CLEAR_BIT(USART2->CR3, USART_CR3_DMAT);
    usart2->sendlen = 0;

    CORO_FINALLY;
}


// ASYNC
// usart2_recvA(struct uaio_task *self) {
//     CORO_START;
//
//     usart2->dma.channel = DMA1_CH4;
//     usart2->dma.direction = DMA_MEM2PERI;
//     usart2->dma.target = (void*)&USART2->TDR;
//     usart2->dma.source = (void*)usart2->send;
//     usart2->dma.bytes = usart2->sendlen;
//
//     DEBUGN("Sending: %.*s", usart2->sendlen, usart2->send);
//
//     /* Enable USART2 DMA request */
//     SET_BIT(USART2->CR3, USART_CR3_DMAT);
//
//     UAIO_AWAIT(dmaA, &(usart2->dma));
//     // DEBUG("Transfer completed");
//
//     /* Disable USART2 DMA request */
//     CLEAR_BIT(USART2->CR3, USART_CR3_DMAT);
//     usart2->sendlen = 0;
//
//     CORO_FINALLY;
// }
