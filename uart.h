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
#ifndef UART_H_
#define UART_H_


#include "uaio/uaio.h"

#include "dma.h"


#define USART2_SENDBUFF_SIZE    1024
#define USART_SEND(u) UAIO_AWAIT(usart_sendA, u)


struct usart {
    char send[USART2_SENDBUFF_SIZE];
    int sendlen;
    struct reg_usart *reg;
};


void
usart_deinit(struct usart *u);


void
usart_init(struct usart *u);


void
usart_write(struct usart *u, const char *fmt, ...);


ASYNC
usart_sendA(struct uaio_task *self, struct usart *u);


#endif  // UART_H_
