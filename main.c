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
#include "rtc.h"
#include "device.h"
#include "uart.h"
#include "gpio.h"

#include "clog/clog.h"
#include "uaio/uaio.h"
#include "uaio/sleep.h"
#include "lwjson/lwjson.h"


/* Uncomment for JSON example */
// static lwjson_token_t tokens[128];
// static lwjson_t lwjson;
//
//
// static void
// json_example(void) {
//     lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));
//     if (lwjson_parse(&lwjson, "{\"foo\": \"bar\"}") == lwjsonOK) {
//         const lwjson_token_t* t;
//         printf("JSON parsed..\r\n");
//
//         /* Find custom key in JSON */
//         if ((t = lwjson_find(&lwjson, "foo")) != NULL) {
//             printf("Key found %.*s: %.*s\r\n",
//                     t->token_name_len,
//                     t->token_name,
//                     t->u.str.token_value_len,
//                     t->u.str.token_value);
//         }
//
//         /* Call this when not used anymore */
//         lwjson_free(&lwjson);
//     }
// }


static ASYNC
startA(struct uaio_task *self) {
    static int c = 0;

    CORO_START;
    INFO("Initializing...");
    UAIO_AWAIT(device_init, NULL);

    rtc_autowakup_init();
    INFO("Starting...");

    while (1) {
        // /* JSON example */
        // json_example();

        // /* RTC Date & time */
        // print_date(false);
        // print_time();

        /* GPIO Toggle */
        GPIO_TOGGLE(GPIOB, 15);
        // CORO_SLEEP(1000);
        // GPIO_TOGGLE(GPIOB, 15);

        // /* GPIO set/clear */
        // GPIO_CLEAR(GPIOB, 15);
        // CORO_SLEEP(1000);
        // GPIO_SET(GPIOB, 15);

        // /* USART receive using DMA */
        // UAIO_AWAIT(usart2_recvA, &usart2);

        /* USART send using DMA */
        usart2_write("Hello");
        usart2_write(", Packet: %d\r\n", c++);
        USART2_SEND();
        // USART2_RECV();

        // /* device_standby commented for now to test uart dma */
        // device_standby();

        // /* sleep example */
        // CORO_SLEEP(1000);
    }

    CORO_FINALLY;
}


int
main(void) {
    int status;
#ifdef PROD
    clog_verbosity = CLOG_SILENT;
#else
    clog_verbosity = CLOG_DEBUG;
#endif

    status = UAIO_FOREVER(startA, NULL, 3);

    device_deinit();
    return status;
}
