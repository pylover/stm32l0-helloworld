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
#include <unistd.h>

#include "stm32l0xx.h"

#include "uaio.h"
#include "sleep.h"


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY sleep
#define UAIO_ARG1 int
#include "generic.c"


static struct uaio_task *timer1 = NULL;


void
TIM2_IRQHandler() {
    if (!(TIM2->SR & TIM_SR_UIF)) {
        return;
    }

    if (timer1 == NULL) {
        return;
    }

    TIM2->SR = ~TIM_SR_UIF;
    TIM2->CR1 &= ~TIM_CR1_CEN;
    timer1->status = UAIO_RUNNING;
    timer1 = NULL;
}


ASYNC
uaio_sleepA(struct uaio_task *self, int*, int miliseconds) {
    CORO_START;

    if (timer1 != NULL) {
        ERROR("Timer busy");
        CORO_RETURN;
    }

    TIM2->CNT = miliseconds;
    TIM2->ARR = miliseconds;
    TIM2->EGR |= TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;

    timer1 = self;
    UAIO_IWAIT();

    CORO_FINALLY;
}
