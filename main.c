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


#include "clock.h"
#include "stm32l0xx.h"
#include "clog.h"
#include "rtc.h"
#include "device.h"
#include "uart.h"
#include "uaio.h"


static ASYNC
startA(struct uaio_task *self) {
    static struct uaio_sleep sleep = {2000};
    // static struct usart usart2 = {
    //     .send = "hello\r\n",
    //     .sendlen = 7,
    // };

    CORO_START;
    INFO("Initializing...");
    CORO_WAIT(device_init, NULL);

    rtc_autowakup_init();
    INFO("Starting...");

    while (1) {
        /* sleep example */
        CORO_WAIT(sleepA, &sleep);

        /* RTC Date & time */
        print_date(false);
        print_time();

        // /* USART send using DMA */
        // CORO_WAIT(usart2_sendA, &usart2);

        // /* device_standby commented for now to test uart dma */
        // device_standby();
    }

    CORO_FINALLY;
}


int
main(void) {
#ifdef PROD
    clog_verbosity = CLOG_SILENT;
#else
    clog_verbosity = CLOG_DEBUG;
#endif

    return UAIO(startA, NULL, 2);
}
