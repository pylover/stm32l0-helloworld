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
#ifndef DMA_H_
#define DMA_H_


#include "uaio/uaio.h"


enum dma_direction {
    DMA_MEM2PERI,
    DMA_MEM2MEM,
    DMA_PERI2MEM,
};


typedef struct dma {
    bool configured;
    struct dma_channel *channel;
    enum dma_direction direction;
    void *source;
    void *target;
    uint32_t bytes;
} dma;


void
dma_init();


ASYNC
dmaA(struct uaio_task *self, struct dma *state);


#endif  // DMA_H_

