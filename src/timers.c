/**
 * @file    :   timers.c
 * @brief   :   API for LETIMER peripheral
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   10 February 2022
 *
 */

#include <app.h>
#include "em_cmu.h"
#include "em_letimer.h"
#include "src/timers.h"
#include "stdint.h"
#include "em_core.h"

// Include logging specifically for this .c file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

//Macros for calculating values to load in LETIMER
#define PRESCALED_FREQ (CMU_ClockFreqGet(cmuClock_LFA) / PRESCALAR_VALUE)
#define VALUE_TO_LOAD ((LETIMER_PERIOD_MS * PRESCALED_FREQ) / 1000)

//Interrupt enable bits sequence
#define LETIMER_INT_BITS (6)

/*
 * Initializes the LETIMER0
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void letimer_init()
{
  const LETIMER_Init_TypeDef letimer0_init =
      {
          .debugRun = true,
          .topValue = VALUE_TO_LOAD,
          .enable = false,
          .bufTop = false,
          .comp0Top = false,
          .out0Pol = 0,
          .out1Pol = 0,
          .repMode = letimerRepeatFree,
      };

  LETIMER_Init(LETIMER0, &letimer0_init);              //Initialize this instance of LETIMER0 as initialized above

  LETIMER_Enable(LETIMER0, true);                     //Enable LETIMER0

}

static uint32_t get_current_tick()
{
  return LETIMER_CounterGet(LETIMER0);
}

/*
 * Hard spin time delay loop using LETIMER ticks as reference
 *
 * Parameters:
 *   uint32_t time_us: Time in micro-seconds
 *
 * Returns:
 *   None
 */
void timerWaitUs_polled(uint32_t time_us)
{
  volatile uint32_t current_time = 0;

  uint32_t time_ticks = 0;

  uint32_t extra_ticks = 0;

  int main_loop_val = 1;             //Loop for running the entire LETIMER counter

  if(time_us >= 1000000)
    time_ticks = ((time_us / 1000000) * PRESCALED_FREQ);

  else
    time_ticks = ((time_us * PRESCALED_FREQ) / 1000000);

  uint32_t temp_ticks = time_ticks;


  /***************************Section for cases where time is greater than the top value of LETIMER0*****************/
  if(temp_ticks > VALUE_TO_LOAD)
    {
      time_ticks = VALUE_TO_LOAD;
      if((temp_ticks % VALUE_TO_LOAD == 0) || ((temp_ticks / VALUE_TO_LOAD) > 1))         //Check if value greater than 6 seconds[Increment the loop counter]
        {
          main_loop_val = (temp_ticks / VALUE_TO_LOAD);
        }
      extra_ticks = temp_ticks % VALUE_TO_LOAD;             //Get additional ticks

      //Run blocking function for the additional ticks
      if(extra_ticks > 0)
        {
          current_time = get_current_tick();

          if(current_time < extra_ticks)
            {
              extra_ticks = VALUE_TO_LOAD - (extra_ticks - current_time);

              while(LETIMER_CounterGet(LETIMER0) != 0);

              while(LETIMER_CounterGet(LETIMER0) != extra_ticks);
            }
          else
            while(current_time - LETIMER_CounterGet(LETIMER0) < extra_ticks);
        }
    }
  /*****************************************************************************************************************/

  current_time = get_current_tick();

  //Run blocking function for the number of ticks
  for (int i= 0; i < main_loop_val ; i++)
    {
      if(current_time < time_ticks)
        {
          if(time_ticks == VALUE_TO_LOAD)
            temp_ticks = current_time + 1;

          else
            temp_ticks = VALUE_TO_LOAD - (time_ticks - current_time);

          while(LETIMER_CounterGet(LETIMER0) != 0);

          while(LETIMER_CounterGet(LETIMER0) != temp_ticks);
        }
      else
        while(current_time - LETIMER_CounterGet(LETIMER0) < time_ticks);
    }
}


/*
 * Interrupt-based time delay using LETIMER Comp1 interrupts
 *
 * Parameters:
 *   uint32_t time_us: Time in micro-seconds
 *
 * Returns:
 *   None
 */
void timerWaitUs_irq(uint32_t time_us)
{
  uint32_t time_ticks = 0;

  if(time_us >= 1000000)
    time_ticks = ((time_us / 1000000) * PRESCALED_FREQ);

  else
    time_ticks = ((time_us * PRESCALED_FREQ) / 1000000);

  if(time_ticks == 0 || time_ticks > VALUE_TO_LOAD || time_us < 122)
    {
      LOG_ERROR("\r\nInvalid range- Value should be less than 3 seconds and greater than 122 us\r\n");
    }

  if(LETIMER_CounterGet(LETIMER0) < time_ticks)
    LETIMER_CompareSet(LETIMER0, 1, VALUE_TO_LOAD - (time_ticks - LETIMER_CounterGet(LETIMER0)));

  else
    LETIMER_CompareSet(LETIMER0, 1, LETIMER_CounterGet(LETIMER0) - time_ticks);


  LETIMER_IntClear(LETIMER0, LETIMER_IFC_COMP1);

  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);            //Enable the COMP1 flag for the LETIMER0 peripheral


  int int_bit = LETIMER0-> IEN;

  if(int_bit != LETIMER_INT_BITS)                           //Check for if the interrupt bit got set
    {
      LOG_ERROR("\r\nComp1 interrupt not enabled\r\n");
      LETIMER0 -> IEN |= LETIMER_INT_BITS;

      if(LETIMER0 -> IEN != LETIMER_INT_BITS)
        {
          LOG_ERROR("\r\nASSERT: Interrupt not enabled\r\n");
//          __BKPT(0);
        }
    }
}
