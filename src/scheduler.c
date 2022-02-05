/**
 * @file    :   scheduler.c
 * @brief   :   API for event scheduler
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   2 February 2022
 *
 */

#include "stdint.h"
#include "em_core.h"
#include "scheduler.h"


//Global variable for storing the events
static uint32_t getEvent = 0;


//Bits for the event
#define bit_LETIMER0 (0x01)


/*
 * Sets an event when interrupt is triggered
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void setSchedulerEventTemp()
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();          //Enter critical section

  getEvent |= event_LETIMER0_UF;           //Set temperature event

  CORE_EXIT_CRITICAL();          //Exit critical section
}


/*
 * Returns the current event triggered.
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   uint32_t : Event triggered
 */
uint32_t getCurrentEvent()
{
  uint32_t setEvent = 0;

  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();               //Enter critical section

  if(getEvent & bit_LETIMER0)          //Check for the event set
    setEvent = event_LETIMER0_UF;

  getEvent &= ~(bit_LETIMER0);        //Clear the event set

  CORE_EXIT_CRITICAL();              //Enter critical section

  return setEvent;
}
