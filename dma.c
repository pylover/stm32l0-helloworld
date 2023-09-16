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
#include "stm32l0xx.h"

#include "dma.h"


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


