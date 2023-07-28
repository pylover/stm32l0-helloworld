/**
 * This file provides two functions and one global variable to be called from
 * user application:
 *
 * system_init(): This function is called at startup just after reset and
 *                before branch to main program. This call is made inside
 *                the "rollup/stm32l0xx.s" file.
 *
 * system_clock variable: Contains the core clock (HCLK), it can be used
 *                        by the user application to setup the SysTick
 *                        timer or configure other parameters.
 *
 * system_clock_update(): Updates the variable system_clock and must
 *                        be called whenever the core clock is changed
 *                        during program execution.
 */


#include "device.h"
#include "stm32l0xx.h"


#ifndef HSE_VALUE
  /* Value of the external oscillator in Hz */
  #define HSE_VALUE    ((uint32_t)8000000U)
#endif


#ifndef MSI_VALUE
  /* Value of the multi-speed internal oscillator in Hz */
  #define MSI_VALUE    ((uint32_t)2097152U)
#endif


#ifndef HSI_VALUE
  /* Value of the high-speed internal oscillator in Hz */
  #define HSI_VALUE    ((uint32_t)16000000U)
#endif


/* Note: Following vector table addresses must be defined in line with linker
         configuration. */
/* Uncomment the following line if you need to relocate the vector table
   anywhere in Flash or Sram, else the vector table is kept at the automatic
   remap of boot address selected */
/* #define USER_VECT_TAB_ADDRESS */

#if defined(USER_VECT_TAB_ADDRESS)
/* Uncomment the following line if you need to relocate your vector Table in
   Sram else user remap will be done in Flash. */
/* #define VECT_TAB_SRAM */
#if defined(VECT_TAB_SRAM)
    /* Vector Table base address field.
       This value must be a multiple of 0x200. */
    #define VECT_TAB_BASE_ADDRESS   SRAM_BASE
    #define VECT_TAB_OFFSET         0x00000000U
#else
    /* Vector Table base address field.
       This value must be a multiple of 0x200. */
    #define VECT_TAB_BASE_ADDRESS   FLASH_BASE

    /* Vector Table base offset field.
       This value must be a multiple of 0x200. */
    #define VECT_TAB_OFFSET         0x00000000U
#endif /* VECT_TAB_SRAM */
#endif /* USER_VECT_TAB_ADDRESS */


/** This variable is updated in three ways:
1) by calling CMSIS function system_clock_update()
2) by calling HAL API function HAL_RCC_GetHCLKFreq()
3) each time HAL_RCC_ClockConfig() is called to configure the system clock
   frequency.
   Note: If you use this function to configure the system clock; then there
         is no need to call the 2 first functions listed above, since
         system_clock variable is updated automatically.
*/
uint32_t system_clock = 2097152U; /* 32.768 kHz * 2^6 */
const uint8_t AHBPrescTable[16] =
    {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U, 6U, 7U, 8U, 9U};
const uint8_t APBPrescTable[8] = {0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U};
const uint8_t PLLMulTable[9] = {3U, 4U, 6U, 8U, 12U, 16U, 24U, 32U, 48U};


/** Setup the microcontroller system.
  */
void system_init (void) {
  /* Configure the Vector Table location add offset address */
#if defined (USER_VECT_TAB_ADDRESS)
  /* Vector Table Relocation in Internal SRAM */
  SCB->VTOR = VECT_TAB_BASE_ADDRESS | VECT_TAB_OFFSET;
#endif /* USER_VECT_TAB_ADDRESS */
}


/** Update system_clock variable according to Clock Register Values.
 * The system_clock variable contains the core clock (HCLK), it can
 * be used by the user application to setup the SysTick timer or configure
 * other parameters.
 *
 * Each time the core clock (HCLK) changes, this function must be called
 * to update system_clock variable value. Otherwise, any configuration
 * based on this variable will be incorrect.
 *
 * - The system frequency computed by this function is not the real
 *   frequency in the chip. It is calculated based on the predefined
 *   constant and the selected clock source:
 *
 * - If SYSCLK source is MSI, system_clock will contain the MSI
 *   value as defined by the MSI range.
 *
 * - If SYSCLK source is HSI, system_clock will contain the HSI_VALUE(*)
 *
 * - If SYSCLK source is HSE, system_clock will contain the HSE_VALUE(**)
 *
 * - If SYSCLK source is PLL, system_clock will contain the HSE_VALUE(**)
 *   or HSI_VALUE(*) multiplied/divided by the PLL factors.
 *
 * (*) HSI_VALUE is a constant defined in stm32l0xx_hal.h file (default value
 *     16 MHz) but the real value may vary depending on the variations
 *     in voltage and temperature.
 *
 * (**) HSE_VALUE is a constant defined in stm32l0xx_hal.h file (default value
 *      8 MHz), user has to ensure that HSE_VALUE is same as the real
 *      frequency of the crystal used. Otherwise, this function may
 *      have wrong result.
 *
 * - The result of this function could be not correct when using fractional
 *   value for HSE crystal.
 */
void
system_clock_update(void) {
  uint32_t tmp = 0U, pllmul = 0U, plldiv = 0U, pllsource = 0U, msirange = 0U;

  /* Get SYSCLK source ----------------------------------------------------*/
  tmp = RCC->CFGR & RCC_CFGR_SWS;

  switch (tmp) {
    case 0x00U:  /* MSI used as system clock */
      msirange = (RCC->ICSCR & RCC_ICSCR_MSIRANGE) >> RCC_ICSCR_MSIRANGE_Pos;
      system_clock = (32768U * (1U << (msirange + 1U)));
      break;
    case 0x04U:  /* HSI used as system clock */
      if ((RCC->CR & RCC_CR_HSIDIVF) != 0U)
      {
        system_clock = HSI_VALUE / 4U;
      }
      else
      {
        system_clock = HSI_VALUE;
      }
      break;
    case 0x08U:  /* HSE used as system clock */
      system_clock = HSE_VALUE;
      break;
    default:  /* PLL used as system clock */
      /* Get PLL clock source and multiplication factor -------------------*/
      pllmul = RCC->CFGR & RCC_CFGR_PLLMUL;
      plldiv = RCC->CFGR & RCC_CFGR_PLLDIV;
      pllmul = PLLMulTable[(pllmul >> RCC_CFGR_PLLMUL_Pos)];
      plldiv = (plldiv >> RCC_CFGR_PLLDIV_Pos) + 1U;

      pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;

      if (pllsource == 0x00U)
      {
        /* HSI oscillator clock selected as PLL clock entry */
        if ((RCC->CR & RCC_CR_HSIDIVF) != 0U)
        {
          system_clock = (((HSI_VALUE / 4U) * pllmul) / plldiv);
        }
        else
        {
          system_clock = (((HSI_VALUE) * pllmul) / plldiv);
        }
      }
      else
      {
        /* HSE selected as PLL clock entry */
        system_clock = (((HSE_VALUE) * pllmul) / plldiv);
      }
      break;
  }
  /* Compute HCLK clock frequency */
  /* Get HCLK prescaler */
  tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos)];
  /* HCLK clock frequency */
  system_clock >>= tmp;
}
