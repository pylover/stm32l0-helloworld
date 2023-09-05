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
#include <stdlib.h>

#include <clog.h>

#include "uaio.h"
#include "device.h"


#define TASKPOOL_ISFULL(self) ((self)->count == (self)->size)
#define TASKPOOL_ISEMPTY(self) ((self)->count == 0)


static struct uaio_taskpool _tasks;


static int
taskpool_init(struct uaio_taskpool *self, size_t size) {
    self->pool = calloc(size, sizeof(struct uaio_task*));
    if (self->pool == NULL) {
        return -1;
    }
    memset(self->pool, 0, size * sizeof(struct uaio_task*));
    self->count = 0;
    self->size = size;
    return 0;
}


static void
taskpool_deinit(struct uaio_taskpool *self) {
    if (self->pool == NULL) {
        return;
    }
    free(self->pool);
}


static int
taskpool_append(struct uaio_taskpool *self, struct uaio_task *item) {
    int i;

    if (item == NULL) {
        return -1;
    }

    if (TASKPOOL_ISFULL(self)) {
        return -1;
    }

    for (i = 0; i < self->size; i++) {
        if (self->pool[i] == NULL) {
            goto found;
        }
    }

    return -1;

found:
    self->pool[i] = item;
    self->count++;
    return i;
}


static int
taskpool_delete(struct uaio_taskpool *self, unsigned int index) {
    if (self->size <= index) {
        return -1;
    }

    self->pool[index] = NULL;
    return 0;
}


static struct uaio_task*
taskpool_get(struct uaio_taskpool *self, unsigned int index) {
    if (self->size <= index) {
        return NULL;
    }

    return self->pool[index];
}


static void
taskpool_vacuum(struct uaio_taskpool *self) {
    int i;
    int shift = 0;

    for (i = 0; i < self->count; i++) {
        if (self->pool[i] == NULL) {
            shift++;
            continue;
        }

        if (!shift) {
            continue;
        }

        self->pool[i - shift] = self->pool[i];
        self->pool[i - shift]->index = i - shift;
        self->pool[i] = NULL;
    }

    self->count -= shift;
}


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
sleepA(struct uaio_task *self, struct uaio_sleep *state) {
    CORO_START;

    if (timer1 != NULL) {
        CORO_REJECT("Timer busy");
    }

    TIM2->CNT = state->miliseconds;
    TIM2->ARR = state->miliseconds;
    TIM2->EGR |= TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;

    timer1 = self;
    CORO_WAITI();

    CORO_FINALLY;
}


int
uaio_init(size_t maxtasks) {
    /* Initialize task pool */
    if (taskpool_init(&_tasks, maxtasks)) {
        goto onerror;
    }

    return 0;

onerror:
    uaio_deinit();
    return -1;
}


void
uaio_deinit() {
    taskpool_deinit(&_tasks);
}


static void
_uaio_task_dispose(struct uaio_task *task) {
    if (task == NULL) {
        return;
    }
    taskpool_delete(&_tasks, task->index);
    free(task);
}


int
uaio_task_new(uaio_coro coro, void *state) {
    struct uaio_task *task;

    if (TASKPOOL_ISFULL(&_tasks)) {
        return -1;
    }

    task = malloc(sizeof(struct uaio_task));
    if (task == NULL) {
        return -1;
    }

    /* Register task */
    task->index = taskpool_append(&_tasks, task);
    if (task->index == -1) {
        free(task);
        return -1;
    }
    task->current = NULL;

    /* Update the task->current */
    if (uaio_call_new(task, coro, state)) {
        _uaio_task_dispose(task);
        return -1;
    }

    return 0;
}


void
uaio_task_killall() {
    int taskindex;
    struct uaio_task *task;

    for (taskindex = 0; taskindex < _tasks.count; taskindex++) {
        task = taskpool_get(&_tasks, taskindex);
        if (task == NULL) {
            continue;
        }

        task->status = UAIO_TERMINATING;
    }
}


int
uaio_call_new(struct uaio_task *task, uaio_coro coro, void *state) {
    struct uaio_call *parent = task->current;
    struct uaio_call *call = malloc(sizeof(struct uaio_call));
    if (call == NULL) {
        return -1;
    }

    if (parent == NULL) {
        call->parent = NULL;
    }
    else {
        call->parent = parent;
    }

    call->coro = coro;
    call->state = state;
    call->line = 0;

    task->status = UAIO_RUNNING;
    task->current = call;
    return 0;
}


bool
uaio_task_step(struct uaio_task *task) {
    struct uaio_call *call = task->current;

start:

    /* Pre execution */
    switch (task->status) {
        case UAIO_TERMINATING:
            /* Tell coroutine to jump to the CORO_FINALLY label */
            call->line = -1;
            break;

        case UAIO_SLEEPING:
            /* Ignore if task is waiting for IO events */
            return false;

        case UAIO_TERMINATED:
        case UAIO_YIELDING:
        case UAIO_RUNNING:
            /* Nothing to do. */
            break;
    }

    call->coro(task, call->state);
    /* Post execution */
    switch (task->status) {
        case UAIO_TERMINATING:
            goto start;
        case UAIO_YIELDING:
            if (call->parent == NULL) {
                task->status = UAIO_RUNNING;
                break;
            }
        case UAIO_TERMINATED:
            task->current = call->parent;
            free(call);
            if (task->current != NULL) {
                task->status = UAIO_RUNNING;
            }
            break;

        case UAIO_SLEEPING:
        case UAIO_RUNNING:
            /* Nothing to do. */
            break;
    }

    if (task->current == NULL) {
        _uaio_task_dispose(task);
        return true;
    }

    return false;
}


int
uaio_start() {
    int taskindex;
    struct uaio_task *task = NULL;
    bool vacuum_needed;

    while (_tasks.count) {
        vacuum_needed = false;

        for (taskindex = 0; taskindex < _tasks.count; taskindex++) {
            task = taskpool_get(&_tasks, taskindex);
            if (task == NULL) {
                continue;
            }

            vacuum_needed |= uaio_task_step(task);
        }

        if (vacuum_needed) {
            taskpool_vacuum(&_tasks);
        }
    }

    return 0;
}


int
uaio_forever() {
    if (uaio_start()) {
        goto onerror;
    }

    uaio_deinit();
    return 0;

onerror:
    uaio_deinit();
    return -1;
}


int
uaio(uaio_coro coro, void *state, size_t maxtasks) {
    if (uaio_init(maxtasks)) {
        return -1;
    }

    if (uaio_task_new(coro, state)) {
        goto failure;
    }

    if (uaio_start()) {
        goto failure;
    }

    uaio_deinit();
    return 0;

failure:
    uaio_deinit();
    return -1;
}
