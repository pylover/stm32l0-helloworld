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


/* delay counter */
#define DELAY_MAX       50


int
main(void) {
#ifdef PROD
    clog_verbosity = CLOG_SILENT;
#else
    clog_verbosity = CLOG_DEBUG;
#endif

    device_init();
    // uart_init();

    INFO("Starting...");
    int t = 0;

    while (1) {
        delay_s(5);
        INFO("Ticks: %d", t++);
    }
}
