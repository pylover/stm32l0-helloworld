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
#ifndef UAIO_SLEEP_H_
#define UAIO_SLEEP_H_


#include "uaio.h"


typedef int sleep_t;


#undef UAIO_ARG1
#undef UAIO_ARG2
#undef UAIO_ENTITY
#define UAIO_ENTITY sleep
#define UAIO_ARG1 int
#include "generic.h"


#define CORO_SLEEP(milisec) AWAIT(sleep, uaio_sleepA, NULL, milisec)


ASYNC
uaio_sleepA(struct uaio_task *self, int *, int seconds);


#endif  // UAIO_SLEEP_H_
