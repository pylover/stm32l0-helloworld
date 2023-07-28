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
#ifndef DEVICE_H_
#define DEVICE_H_


#include <stdint.h>


extern volatile uint32_t ticks_ms;


/* Value of the external crystal oscillator in Hz */
#define HSE_VALUE ((uint32_t)12000000U)


/* SysTick devider */
#define SYSTICKS ((uint32_t)1000U)


void
device_init();


void
delay_ms(uint32_t ms);


void
delay_s(uint32_t s);


#endif  // DEVICE_H_
