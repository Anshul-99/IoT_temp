/**
 * @file    :   oscillators.c
 * @brief   :   API for oscillators and clock tree initialization
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   2 February 2022
 *
 */

#include "oscillators.h"
#include "app.h"
#include "em_cmu.h"
#include "src/timers.h"

#define INCLDUE_LOG_DEBUG (1)
#include "src/log.h"

/*
 * Initializes the clock tree and the necessary oscillators for various peripherals
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void oscillator_init()
{
  if (LOWEST_ENERGY_MODE < 3)                                       //If energy modes is EM0, EM1 or EM2, choose LFX0 clock
    {
      CMU_OscillatorEnable(cmuOsc_LFXO  , true , true);
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
    }
  else if (LOWEST_ENERGY_MODE == 3)                                //If energy mode is EM3, choose ULFRC0
    {
      CMU_OscillatorEnable(cmuOsc_ULFRCO  , true , true);
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);
    }
  else
    {
      LOG_ERROR("\r\nInvalid energy mode\r\n");
    }
  CMU_ClockEnable(cmuClock_LFA, true);                            //Enable LFA clock tree

  CMU_ClockDivSet(cmuClock_LETIMER0, PRESCALAR_VALUE);            //Divide the LETIMER clock by the necessary prescalar

  CMU_ClockEnable(cmuClock_LETIMER0, true);                       //Enable the LETIMER0 peripheral clock
}
