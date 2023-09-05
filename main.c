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
#include <stdio.h>
#include <stdint.h>


#include "clog.h"
#include "device.h"
#include "uaio.h"


static ASYNC
startA(struct uaio_task *self) {
    static int seconds;
    static int t = 0;
    static struct uaio_sleep sleep = {2000};
    CORO_START;
    INFO("Initializing...");
    CORO_WAIT(device_init, NULL);

    INFO("Starting...");

    while (1) {
        // delay_s(5);
        CORO_WAIT(sleepA, &sleep);

        seconds = (RTC->TR & RTC_TR_ST) >> RTC_TR_ST_Pos;
        seconds *= 10;
        seconds += RTC->TR & RTC_TR_SU;
        INFO("Ticks: %d, RTC: %d", t++, seconds);
    }

    CORO_FINALLY;
}


static volatile int resume = 1;


int
main(void) {
#ifdef PROD
    clog_verbosity = CLOG_SILENT;
#else
    clog_verbosity = CLOG_DEBUG;
#endif

    return UAIO(startA, NULL, 2);
}
