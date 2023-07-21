#include <stdio.h>
#include <stdint.h>

extern void initialise_monitor_handles(void);

#include <stm32l0xx.h>


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


int
main(void) {
    initialise_monitor_handles();
    printf("Starting...\n");

    while (1);
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
