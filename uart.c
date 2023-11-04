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


static struct uaio_task *usart2 = NULL;


/** USART initializer
  * baud = 2 X PCLK / USART2DIV
  * div = 2 X PCLK / baud
  */
void
usart_init(struct usart *u) {
    struct reg_usart *reg = u->reg;
    /* Allocate memory for usart2 state */

    if (reg == USART2) {
        dma_setup(DMA1_CH4, DMA_MEM2PERI, (void*)u->send, (void*)&reg->TDR);

        /*
        USART2 pins
        TX,     pin 12, PA2, APB1
        RX,     pin 13, PA3, APB1
        Wakeup, pin 10, PA0
        */
        // const char * msg = "Hello\r\n";

        /* Enable clock for USART2 */
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

        /* TODO: USART2 clock enable during Sleep mode bit */
        // RCC->CCIPR |=
        /* Clock configuration register (RCC_CCIPR)
        USART2SEL: USART2 clock source selection bits
        This bit is set and cleared by software.
        00: APB clock selected as USART2 clock
        01: System clock selected as USART2 clock
        10: HSI16 clock selected as USART2 clock
        11: LSE clock selected as USART2 clock
        */

        /* RCC_APB1SMENR: Bit 17 USART2SMEN */
        GPIOA->MODER &= ~GPIO_MODER_MODE2;

        /* Push/Pull mode for PA2 */
        GPIOA->OTYPER &= ~GPIO_OTYPER_OT_2;

        /* High speed mode for PA2 */
        REG_MODIFY(GPIOA->OSPEEDR, GPIO_OSPEEDR_OSPEED2,
                GPIO_OSPEEDR_OSPEED2_1);

        /* Select alternate function mode for PA2 and PA3 */
        REG_MODIFY(GPIOA->MODER, GPIO_MODER_MODE2, GPIO_MODER_MODE2_1);
        REG_MODIFY(GPIOA->MODER, GPIO_MODER_MODE3, GPIO_MODER_MODE3_1);

        /* Alternate function selection for PA2 */
        REG_MODIFY(GPIOA->AFRL, GPIO_AFRL_AFSEL2,
            GPIOA_AFRL_AFSEL2_AF4_USART2_TX);

        /* Alternate function selection for PA3 */
        REG_MODIFY(GPIOA->AFRL, GPIO_AFRL_AFSEL3,
            GPIOA_AFRL_AFSEL3_AF4_USART2_RX);
    }
    else {
        return;
    }

    /* Disable USART */
    u->sendlen = 0;
    u->recvlen = 0;
    REG_CLEAR(reg->CR1, USART_CR1_UE);

    /* CR1 configuration */
    /* Word length: 00: 1 Start bit, 8 data bits, n stop bits */
    reg->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0);

    /* Oversampling
    Select oversampling by 8 (OVER8=1) to achieve higher speed (up to fCK/8).
    In this case the maximum receiver tolerance to clock deviation is reduced

    Select oversampling by 16 (OVER8=0) to increase the tolerance of the
    receiver to clock deviations. In this case, the maximum speed is limited
    to maximum fCK/16 where fCK is the clock source frequency.
    */
    /* This will cause problem in BAUD rate selection, commented to be dealt
       later */
    // reg->CR1 |= USART_CR1_OVER8;

    /* BRR register configuration */
    uint32_t baud_rate = 115200;
    reg->BRR = (uint32_t) system_clock / baud_rate;

    /* Interrupts */
    REG_SET(u->reg->CR1, USART_CR1_CMIE);
    REG_SET(u->reg->CR1, USART_CR1_RXNEIE);
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_SetPriority(USART2_IRQn, 1);

    /* Enable USART Transmitter */
    REG_SET(reg->CR1, USART_CR1_TE);
    REG_SET(reg->CR1, USART_CR1_RE);

    /* 7 bit char match */
    REG_SET(u->reg->CR2, USART_CR2_ADDM7);
    REG_CLEAR(u->reg->CR1, USART_CR1_RE);
    REG_SET(u->reg->CR2, 13 << USART_CR2_ADD_Pos);

    /* Enable USART */
    REG_SET(reg->CR1, USART_CR1_UE);
}


void
usart_deinit(struct usart *u) {
    /* Disable usart */
    REG_CLEAR(u->reg->CR1, USART_CR1_UE);
}


void
usart_write(struct usart *u, const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    u->sendlen += vsnprintf(u->send + u->sendlen,
            USART2_SENDBUFF_SIZE - u->sendlen, fmt, ap);
    va_end(ap);
}


ASYNC
usart_sendA(struct uaio_task *self, struct usart *u) {
    CORO_START;

    DEBUGN("Sending: %.*s", u->sendlen, u->send);

    /* Enable USART DMA request */
    REG_SET(u->reg->CR3, USART_CR3_DMAT);

    DMA_CH4_WAIT((void*)u->sendlen);
    // DEBUG("Transfer completed");

    /* Disable USART DMA request */
    REG_CLEAR(u->reg->CR3, USART_CR3_DMAT);
    u->sendlen = 0;

    CORO_FINALLY;
}


void
USART2_IRQHandler() {
    char in;
    struct usart *u = usart2->current->state;

    if (REG_GET(USART2->ISR, USART_ISR_RXNE)) {
        in = (char) USART2->RDR;
        u->recv[u->recvlen++] = in;
    }

    if (REG_GET(USART2->ISR, USART_ISR_CMF)) {
        REG_SET(USART2->ICR, USART_ICR_CMCF);
        usart2->status = UAIO_RUNNING;
    }
}


ASYNC
usart_recvA(struct uaio_task *self, struct usart *u) {
    CORO_START;

    DEBUG("Receiving ...");
    u->recvlen = 0;
    usart2 = self;
    UAIO_IWAIT(u->reg->CR1 |= USART_CR1_RE);
    REG_CLEAR(u->reg->CR1, USART_CR1_RE);
    CORO_FINALLY;
}
