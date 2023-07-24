#include <stdio.h>
#include <stdint.h>

extern void initialise_monitor_handles(void);

#include "stm32l0xx.h"


#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif


/* delay counter */
#define DELAY_MAX       50

void
RCC_CRS_IRQHandler(void) {
    if ((RCC->CIFR & RCC_CIFR_HSERDYF) != 0) {
        RCC->CICR |= RCC_CICR_HSERDYC; /* (2) */
        RCC->CFGR = ((RCC->CFGR & (~RCC_CFGR_SW)) | RCC_CFGR_SW_HSE); /* (3) */
    }
    else {
        /* TODO: Manage error */
    }
}


void
uart_init() {
    /* Program the number of stop bits in USART_CR2. */
    /* This bit field can only be written when the USART is disabled: UE=0. */
    USART2->CR2 |= USART_CR2_STOP;

    /* Program the M bit in USART_CR1 to define the word length. */
    USART2->CR1 |= USART_CR1_M;

    /* Enable the USART by writing the UE bit in USART_CR1 register to 1. */
    USART2->CR1 |= USART_CR1_UE;

// Select DMA to enable (DMAT) in USART_CR3 if Multi buffer Communication is to take place. Configure the DMA register as explained in multi-buffer communication.
// Select the desired baud rate using the USART_BRR register.
// Set the TE bit in USART_CR1 to send an idle frame as the first transmission.
// Write the data to send in the USART_DR register (this clears the TXE bit). Repeat this for each data to be transmitted in case of a single buffer.
// After writing the last data into the USART_DR register, wait until TC=1. This indicates that the transmission of the last frame is complete. This is required for instance when the USART is disabled or enters the Halt mode to avoid corrupting the last transmission.

    DMA1_CSELR->CSELR |= DMA_CSELR_C3S;
    DMA1_CSELR->CSELR |= DMA_CSELR_C4S;
}


void
clock_init() {
    printf("Clock: %lu\n", SystemCoreClock);
}


int
main(void) {
    initialise_monitor_handles();

    clock_init();
    uart_init();

    printf("Starting...\n");

    // NVIC_EnableIRQ(RCC_CRS_IRQn);
    // NVIC_SetPriority(RCC_CRS_IRQn,0);
    // RCC->CIER |= RCC_CIER_HSERDYIE;
    // RCC->CR |= RCC_CR_CSSHSEON | RCC_CR_HSEBYP | RCC_CR_HSEON;
    // RCC->CR |= RCC_CR_HSERDY;
    // /* turn on clock on GPIOA */
    // RCC->AHBENR |= RCC_AHBENR_CRCEN;

    // /* set PA5 to output mode */
    // GPIOA->MODER &= ~GPIO_MODER_MODE5_1;
    // GPIOA->MODER |=  GPIO_MODER_MODE5_0;

    // while(1) {
    //   /* set HIGH value on pin PA5 */
    //   GPIOA->BSRR |= GPIO_BSRR_BS_5;
    //   for(uint32_t i=DELAY_MAX; i--;) {}

    //   /* set LOW value on pin PA5 */
    //   GPIOA->BSRR |= GPIO_BSRR_BR_5;
    //   for(uint32_t i=DELAY_MAX; i--;) {}
    // }
}
