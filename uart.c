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


void
uart_init() {
    /* Program the number of stop bits in USART_CR2. */
    /* This bit field can only be written when the USART is disabled: UE=0. */
    USART2->CR2 |= USART_CR2_STOP;

    /* Program the M bit in USART_CR1 to define the word length. */
    USART2->CR1 |= USART_CR1_M;

    /* Enable the USART by writing the UE bit in USART_CR1 register to 1. */
    USART2->CR1 |= USART_CR1_UE;

    /*
    - Select DMA to enable (DMAT) in USART_CR3 if Multi buffer Communication
      is to take place. Configure the DMA register as explained in
      multi-buffer communication.
    - Select the desired baud rate using the USART_BRR register.
    - Set the TE bit in USART_CR1 to send an idle frame as the first
      transmission.
    - Write the data to send in the USART_DR register (this clears the TXE
      bit). Repeat this for each data to be transmitted in case of a single
      buffer.
    - After writing the last data into the USART_DR register, wait until
      TC=1. This indicates that the transmission of the last frame is
      complete. This is required for instance when the USART is disabled or
      enters the Halt mode to avoid corrupting the last transmission.
    */

    DMA1_CSELR->CSELR |= DMA_CSELR_C3S;
    DMA1_CSELR->CSELR |= DMA_CSELR_C4S;
}


