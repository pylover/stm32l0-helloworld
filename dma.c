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
#include "stm32l0xx.h"

#include "clog/clog.h"

#include "dma.h"


static struct uaio_task *channel4 = NULL;


void
dma_init() {
    /* DMA1 clock enable */
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    /* DMA channel 4 selection */
    DMA1_CSELR->CSELR &= ~DMA_CSELR_C4S;
    // TODO: Fix me
    DMA1_CSELR->CSELR |= (0x4U << DMA_CSELR_C4S_Pos);

    /* DMA1 channel 4 to 7 interrupt callback enable */
    NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);
    NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, 0);
}


static void
dma_setup(struct dma *state) {
    struct dma_channel *ch = state->channel;

    /* Disable DMA */
    CLEAR_BIT(ch->CCR, DMA_CCR_EN);

    /* Configure the channel priority to medium level */
    /*
    00: low
    01: medium
    10: high
    11: very high
    */
    ch->CCR &= ~(DMA_CCR_PL_1 | DMA_CCR_PL_0);

    /* Configure data transfer direction, peripheral & memory incremented
       mode, and peripheral & memory data size */

    /* Set data transfer direction: memory -> peripheral */
    /*
    DIR: bit 5
    0: read from peripheral
    1: read from memory

    MEM2MEM: bit 14
    0: disabled
    1: enabled
    */
    if (state->direction == DMA_MEM2MEM) {
        SET_BIT(ch->CCR, DMA_CCR_MEM2MEM);

        /* Enable memory address incrementation by one in both sides */
        SET_BIT(ch->CCR, DMA_CCR_MINC);
        SET_BIT(ch->CCR, DMA_CCR_PINC);

        /* Disable circular mode */
        CLEAR_BIT(ch->CCR, DMA_CCR_CIRC);

        CLEAR_BIT(ch->CCR, DMA_CCR_DIR);
        ch->CPAR = (uint32_t)state->source;
        ch->CMAR = (uint32_t)state->target;
    }
    else {
        CLEAR_BIT(ch->CCR, DMA_CCR_MEM2MEM);
        if (state->direction == DMA_MEM2PERI) {
            SET_BIT(ch->CCR, DMA_CCR_DIR);
            ch->CPAR = (uint32_t)state->target;
            ch->CMAR = (uint32_t)state->source;
        }
        else {
            CLEAR_BIT(ch->CCR, DMA_CCR_DIR);
            ch->CPAR = (uint32_t)state->source;
            ch->CMAR = (uint32_t)state->target;
        }

        /* Set memory address incement by one byte */
        SET_BIT(ch->CCR, DMA_CCR_MINC);

        /* Disable address incrementation on peripheral address */
        CLEAR_BIT(ch->CCR, DMA_CCR_PINC);

        // /* Disable circular mode */
        // CLEAR_BIT(ch->CCR, DMA_CCR_CIRC);
        /* Enable circular mode */
        SET_BIT(ch->CCR, DMA_CCR_CIRC);
    }

    /* Set size of data to be sent */
    ch->CNDTR = state->bytes;

    /* Set the memory and pripheral write chunk size to one byte */
    /*
    00: 8 bits
    01: 16 bits
    10: 32 bits
    11: reserved
    */
    ch->CCR &= ~(DMA_CCR_MSIZE_1 | DMA_CCR_MSIZE_0);
    ch->CCR &= ~(DMA_CCR_PSIZE_1 | DMA_CCR_PSIZE_0);

    /* Enable interrpt after full transfer */
    SET_BIT(ch->CCR, DMA_CCR_TCIE | DMA_CCR_TEIE);
    CLEAR_BIT(ch->CCR, DMA_CCR_HTIE);
}


void
DMA1_Channel4_7_IRQHandler() {
    // if (DMA1->ISR & DMA_ISR_HTIF4) {
    //     INFO("Half transfer");
    //     SET_BIT(DMA1->IFCR, DMA_IFCR_CHTIF4);
    // }

    /* transfer error */
    if (DMA1->ISR & DMA_ISR_TEIF4) {
        DEBUG("DMA ERROR");
        SET_BIT(DMA1->IFCR, DMA_IFCR_CTEIF4);
    }

    /* Full transfer */
    if (DMA1->ISR & DMA_ISR_TCIF4) {
        /* Clear DMA interrupt flags */
        SET_BIT(DMA1->IFCR, DMA_IFCR_CTCIF4);

        /* Disable DMA */
        CLEAR_BIT(DMA1_CH4->CCR, DMA_CCR_EN);

        channel4->status = UAIO_RUNNING;
        channel4 = NULL;
    }
}


ASYNC
dmaA(struct uaio_task *self, struct dma *state) {
    CORO_START;

    if (!state->configured) {
        dma_setup(state);
    }

    if (state->channel == DMA1_CH4) {
        /* clear interrupt flags  */
        SET_BIT(DMA1->IFCR, DMA_IFCR_CTCIF4 | DMA_IFCR_CTEIF4);
        channel4 = self;
    }

    UAIO_IWAIT(state->channel->CCR |= DMA_CCR_EN);

    CORO_FINALLY;
}
