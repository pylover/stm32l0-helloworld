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
#ifndef GPIO_H_
#define GPIO_H_


#define GPIO_SET_OUTPUT(port, num) \
    (port)->MODER &= ~(3 << ((num) * 2)); \
    (port)->MODER |= (1 << ((num) * 2))


#define GPIO_SET_OPENDRAIN(port, num) \
    (port)->OTYPER |= (1 << (num))


#define GPIO_SET_LOWSPEED(port, num) \
    (port)->OSPEEDR &= ~(3 << ((num) * 2));


#define GPIO_SET_NOPUPD(port, num) \
    (port)->PUPDR &= ~(3 << ((num) * 2));


#define GPIO_SET(port, num) \
    (port)->ODR |= (1 << (num))


#define GPIO_CLEAR(port, num) \
    (port)->ODR &= ~(1 << (num))


#define GPIO_TOGGLE(port, num) \
    if (((port)->ODR & (1 << (num)))) { \
        GPIO_CLEAR(port, num); \
    } \
    else { \
        GPIO_SET(port, num); \
    }


void
gpio_init();


#endif  // GPIO_H_
