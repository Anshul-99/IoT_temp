/**
 * @file    :   irq.c
 * @brief   :   API for LETIMER IRQ
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   2 February 2022
 *
 */

#include "em_letimer.h"
#include "stdint.h"
#include "gpio.h"
#include "scheduler.h"

/*
 * Initializes the IRQ in the NVIC for the LETIMER0 peripheral
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void letimer_irq_init()
{
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);            //Enable the underflow flag for the LETIMER0 peripheral

  NVIC_ClearPendingIRQ(LETIMER0_IRQn);                     //Clear pending LETIMER0 interrupts

  NVIC_EnableIRQ(LETIMER0_IRQn);                           //Enable the LETIMER0 interrupt
}


/*
 * ISR for the LETIMER0 peripheral
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void LETIMER0_IRQHandler(void)
{
  uint32_t int_flags = 0;

  int_flags = LETIMER_IntGet(LETIMER0);                 //Get the raised interrupt flag

  LETIMER_IntClear(LETIMER0, int_flags);                //Clear the interrupt

  if (int_flags & LETIMER_IF_UF)                       //At underflow event, set temperature measurement event
    setSchedulerEventTemp();
}
