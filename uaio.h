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
#ifndef UAIO_H_
#define UAIO_H_


#include <clog.h>


#define ASYNC void


#define CORO_START \
    switch ((self)->current->line) { \
        case 0:


#define CORO_FINALLY \
        case -1:; } \
    (self)->status = UAIO_TERMINATED;


#define CORO_YIELD(v) \
    do { \
        (self)->current->line = __LINE__; \
        (self)->status = UAIO_YIELDING; \
        (self)->value = v; \
        return; \
        case __LINE__:; \
    } while (0)


#define CORO_YIELDFROM(coro, state, v, t) \
    do { \
        (self)->current->line = __LINE__; \
        if (uaio_call_new(self, (uaio_coro)coro, (void *)state)) { \
            (self)->status = UAIO_TERMINATING; \
        } \
        else { \
            (self)->status = UAIO_RUNNING; \
        } \
        return; \
        case __LINE__:; \
        v = (t)(self)->value; \
    } while (0)


#define CORO_WAIT(coro, state) \
    do { \
        (self)->current->line = __LINE__; \
        if (uaio_call_new(self, (uaio_coro)coro, (void *)state)) { \
            (self)->status = UAIO_TERMINATING; \
        } \
        return; \
        case __LINE__:; \
    } while (0)


#define CORO_WAITI() \
    do { \
        (self)->current->line = __LINE__; \
        (self)->status = UAIO_SLEEPING; \
        return; \
        case __LINE__:; \
    } while (0)


#define CORO_REJECT(fmt, ...) \
    if (fmt) { \
        ERROR(fmt, ## __VA_ARGS__); \
    } \
    (self)->status = UAIO_TERMINATING; \
    return;


#define UAIO(coro, state, maxtasks) \
    uaio((uaio_coro)(coro), (void*)(state), maxtasks)


#define UAIO_RUN(coro, state) \
    uaio_task_new((uaio_coro)coro, (void *)(state))


enum uaio_taskstatus {
    UAIO_RUNNING,
    UAIO_YIELDING,
    UAIO_SLEEPING,
    UAIO_TERMINATING,
    UAIO_TERMINATED,
};


struct uaio_task;
typedef void (*uaio_coro) (struct uaio_task *self, void *state);


struct uaio_call {
    uaio_coro coro;
    int line;
    struct uaio_call *parent;
    void *state;
};


struct uaio_task {
    int index;
    enum uaio_taskstatus status;
    struct uaio_call *current;
    int value;
};


struct uaio_taskpool {
    struct uaio_task **pool;
    size_t size;
    size_t count;
};


struct uaio_sleep {
    unsigned int miliseconds;
};


int
uaio(uaio_coro coro, void *state, size_t maxtasks);


int
uaio_forever();


int
uaio_init(size_t maxtasks);


void
uaio_deinit();


int
uaio_start();


int
uaio_task_new(uaio_coro coro, void *state);


int
uaio_call_new(struct uaio_task *task, uaio_coro coro, void *state);


void
uaio_task_killall();


ASYNC
sleepA(struct uaio_task *self, struct uaio_sleep *state);


#endif  // UAIO_H_
