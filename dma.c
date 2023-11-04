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


void
dma_setup(struct reg_dma_channel *ch, enum dma_direction direction,
        void *source, void *target) {
    /* Disable DMA */
    REG_CLEAR(ch->CCR, DMA_CCR_EN);

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
    if (direction == DMA_MEM2MEM) {
        REG_SET(ch->CCR, DMA_CCR_MEM2MEM);

        /* Enable memory address incrementation by one in both sides */
        REG_SET(ch->CCR, DMA_CCR_MINC);
        REG_SET(ch->CCR, DMA_CCR_PINC);

        /* Disable circular mode */
        REG_CLEAR(ch->CCR, DMA_CCR_CIRC);

        REG_CLEAR(ch->CCR, DMA_CCR_DIR);
        ch->CPAR = (uint32_t)source;
        ch->CMAR = (uint32_t)target;
    }
    else {
        REG_CLEAR(ch->CCR, DMA_CCR_MEM2MEM);
        if (direction == DMA_MEM2PERI) {
            REG_SET(ch->CCR, DMA_CCR_DIR);
            ch->CPAR = (uint32_t)target;
            ch->CMAR = (uint32_t)source;
        }
        else {
            REG_CLEAR(ch->CCR, DMA_CCR_DIR);
            ch->CPAR = (uint32_t)source;
            ch->CMAR = (uint32_t)target;
        }

        /* Set memory address incement by one byte */
        REG_SET(ch->CCR, DMA_CCR_MINC);

        /* Disable address incrementation on peripheral address */
        REG_CLEAR(ch->CCR, DMA_CCR_PINC);

        // /* Disable circular mode */
        // REG_CLEAR(ch->CCR, DMA_CCR_CIRC);
        /* Enable circular mode */
        REG_SET(ch->CCR, DMA_CCR_CIRC);
    }

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
    REG_SET(ch->CCR, DMA_CCR_TCIE | DMA_CCR_TEIE);
    REG_CLEAR(ch->CCR, DMA_CCR_HTIE);
}


void
DMA1_Channel4_7_IRQHandler() {
    // if (DMA1->ISR & DMA_ISR_HTIF4) {
    //     INFO("Half transfer");
    //     REG_SET(DMA1->IFCR, DMA_IFCR_CHTIF4);
    // }

    /* transfer error */
    if (DMA1->ISR & DMA_ISR_TEIF4) {
        REG_SET(DMA1->IFCR, DMA_IFCR_CTEIF4);
        DEBUG("DMA ERROR");
    }

    /* Full transfer */
    if (DMA1->ISR & DMA_ISR_TCIF4) {
        /* Clear DMA interrupt flags */
        REG_SET(DMA1->IFCR, DMA_IFCR_CTCIF4);

        /* Disable DMA */
        REG_CLEAR(DMA1_CH4->CCR, DMA_CCR_EN);

        channel4->status = UAIO_RUNNING;
    }
}


ASYNC
dma_ch4A(struct uaio_task *self, int bytes) {
    CORO_START;

    channel4 = self;
    REG_SET(DMA1->IFCR, DMA_IFCR_CTCIF4 | DMA_IFCR_CTEIF4);

    /* Set size of data to be sent */
    DMA1_CH4->CNDTR = bytes;
    UAIO_IWAIT(DMA1_CH4->CCR |= DMA_CCR_EN);
    CORO_FINALLY;
}
