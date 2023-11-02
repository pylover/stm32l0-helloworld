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

#include "gpio.h"


void
gpio_init() {
    /* Enable clock for GPIO ports a, b and USART2 */
    RCC->IOPENR |= RCC_IOPENR_IOPAEN | RCC_IOPENR_IOPBEN;

    /* Set GPIOB#15 as output, opendrain, lowspeed and no pull up/down. */
    GPIO_SET_OUTPUT(GPIOB, 15);
    GPIO_SET_OPENDRAIN(GPIOB, 15);
    GPIO_SET_LOWSPEED(GPIOB, 15);
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD15_1 | GPIO_PUPDR_PUPD15_0);
}
