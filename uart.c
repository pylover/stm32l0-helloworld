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

#include "uart.h"


/** USART2 initializer
  * baud = 2 X PCLK / USART2DIV
  * div = 2 X PCLK / baud
  */
void
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
