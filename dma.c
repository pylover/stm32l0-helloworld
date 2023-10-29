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

#include "clog.h"
#include "dma.h"


void
dma_init() {
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    
    /* DMA channel 4 selection */
    DMA1_CSELR->CSELR &= ~DMA_CSELR_C4S;
    DMA1_CSELR->CSELR |= (0x4U << DMA_CSELR_C4S_Pos);
    
    NVIC_SetPriority(DMA1_Channel4_5_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel4_5_IRQn);
}


void
DMA1_Channel4_5_IRQHandler() {
	// if ((DMA1->ISR)&(1<<22))  // If the Half Transfer Complete Interrupt is set
	// {
	// 	memcpy (&MainBuf[indx], &RxBuf[0], RXSIZE/2);
	// 	DMA1->IFCR |= (1<<22);
	// 	indx = indx+(RXSIZE/2);
	// 	if (indx>49) indx=0;
	// }

    DEBUG("Transfer completed");

    /* Full transfer */
	if (DMA1->ISR & DMA_ISR_TCIF4) {
		DMA1->IFCR |= DMA_ISR_TCIF4;
	}

}

void
dma_memory_to_peripheral_circular(volatile uint32_t *peripheral,
        const char *data, uint32_t count) {
    /* Disable DMA */
    DMA1_CH4->CCR &= ~DMA_CCR_EN;

    /* Configure the channel priority to medium level */
    /*
    00: low
    01: medium
    10: high
    11: very high
    */
    DMA1_CH4->CCR &= ~(DMA_CCR_PL_1 | DMA_CCR_PL_0);

    /* Configure data transfer direction, peripheral & memory incremented
       mode, and peripheral & memory data size */

    /* Set data transfer direction: memory -> peripheral */
    /*
    0: read from peripheral
    1: read from memory
    */
    DMA1_CH4->CCR &= ~DMA_CCR_MEM2MEM;
    DMA1_CH4->CCR |= DMA_CCR_DIR;

    /* Set the memory and pripheral write chunk size to one byte */
    /*
    00: 8 bits
    01: 16 bits
    10: 32 bits
    11: reserved
    */
    DMA1_CH4->CCR &= ~(DMA_CCR_MSIZE_1 | DMA_CCR_MSIZE_0);
    DMA1_CH4->CCR &= ~(DMA_CCR_PSIZE_1 | DMA_CCR_PSIZE_0);

    /* Set memory address incement by one byte */
    DMA1_CH4->CCR |= DMA_CCR_MINC;

    /* Disable address incrementation on peripheral address */
    DMA1_CH4->CCR &= ~DMA_CCR_PINC;

    /* Enable circular mode */
    DMA1_CH4->CCR |= DMA_CCR_CIRC;

    /* Enable interrpt after full transfer */
    DMA1_CH4->CCR |= DMA_CCR_TCIE | DMA_CCR_TEIE | DMA_CCR_HTIE;

    /* clear interrupt flags  */
    DMA1->IFCR |= (DMA_IFCR_CHTIF4 | DMA_IFCR_CGIF4 | DMA_IFCR_CTCIF4);

    /* Set USART2 TX data register address into DMA Channel 4 */
    DMA1_CH4->CPAR = (uint32_t)peripheral;

    /* Set pointer to data to be sent */
    DMA1_CH4->CMAR = (uint32_t)data;

    /* Set size of data to be sent */
    DMA1_CH4->CNDTR = count;

    /* Enable DMA channel 4 */
    DMA1_CH4->CCR |= DMA_CCR_EN;

    /* Enable USART2 to request from DMA*/
    USART2->CR1 |= USART_CR1_UE;

    DEBUG("Sending: %.*s", count, data);
}
