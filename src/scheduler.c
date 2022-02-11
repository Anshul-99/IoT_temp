/**
 * @file    :   scheduler.c
 * @brief   :   API for event scheduler
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   10 February 2022
 *
 */

#include "stdint.h"
#include "em_core.h"
#include "scheduler.h"
#include "src/i2c.h"
#include "sl_power_manager.h"
#include "src/timers.h"


//Global variable for storing the events
static uint32_t getEvent = 0;


//Bits for the event
#define bit_LETIMER0_UF (1)
#define bit_LETIMER_COMP1 (2)
#define bit_I2C_TRANSFER (4)


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
 * Sets an event when the interrupt-base time delay is hit [COMP1 interrupt]
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void setSchedulerEventDelay()
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();          //Enter critical section

  getEvent |= event_LETIMER0_COMP1;           //Set temperature event

  CORE_EXIT_CRITICAL();          //Exit critical section
}

/*
 * Sets an event when I2C transfer interrupt is triggered
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void setSchedulerEventTransferComplete()
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();          //Enter critical section

  getEvent |= event_I2C_Transfer_Complete;           //Set temperature event

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

  if(getEvent & bit_LETIMER0_UF)          //Check for the event set
    {
      setEvent |= event_LETIMER0_UF;
      getEvent &= ~(bit_LETIMER0_UF);        //Clear the event set
    }

  if(getEvent & bit_LETIMER_COMP1)          //Check for the event set
    {
      setEvent = event_LETIMER0_COMP1;
      getEvent &= ~(bit_LETIMER_COMP1);        //Clear the event set
    }

  if(getEvent & bit_I2C_TRANSFER)          //Check for the event set
    {
      setEvent = event_I2C_Transfer_Complete;
      getEvent &= ~(bit_I2C_TRANSFER);        //Clear the event set
    }

  CORE_EXIT_CRITICAL();              //Enter critical section

  return setEvent;
}

/*
 * State Machine for temperature measurement
 *
 * Parameters:
 *   uint32_t event: Gives the current event set
 *
 * Returns:
 *   None
 */
void temperature_state_machine(uint32_t event)
{
  state_t currentState;

  static state_t nextState = state0_IDLE;                             //First state is Idle by default

  currentState = nextState;                                           //Update the current state

  switch(currentState)
  {
    case state0_IDLE:
      nextState = state0_IDLE;

      if (event == event_LETIMER0_UF)
        {
          loadpowerTempSensor(true);                                 //Power ON the temperature sensor

          timerWaitUs_irq(80000);                                    //Wait for 80 msec [Setup time]

          nextState = state1_COMP1_POWER_ON;
        }
      break;

    case state1_COMP1_POWER_ON:
      nextState = state1_COMP1_POWER_ON;

      if(event == event_LETIMER0_COMP1)                             //Event when the timer delay elapses
        {
          sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);  //While transfer is in progress, put the MCU in EM1 energy mode

          sendI2C_command();                                        //Write temperature measurement sequence to the sensor

          nextState = state2_I2C_TRANSFER_COMPLETE;
        }
      break;

    case state2_I2C_TRANSFER_COMPLETE:
      nextState = state2_I2C_TRANSFER_COMPLETE;

      if(event == event_I2C_Transfer_Complete)                    //Event when the I2C transfer is completed
        {
          sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);      //Pull MCU out of EM1 mode

          timerWaitUs_irq(10800);                                //I2C sequence time to wait while the sequence is transmitted [10.8 msec]

          nextState = state3_COMP1_I2C_TRANSFER_COMPLETE;
        }
      break;

    case state3_COMP1_I2C_TRANSFER_COMPLETE:
      nextState = state3_COMP1_I2C_TRANSFER_COMPLETE;

      if(event == event_LETIMER0_COMP1)                       //Event when the write sequence is written
        {
          sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);   //While transfer is in progress, put the MCU in EM1 energy mode

          receiveI2C_command();                              //Transmit the read command

          nextState = state4_UNDERFLOW_READ;
        }
      break;

    case state4_UNDERFLOW_READ:
      nextState = state4_UNDERFLOW_READ;

      if(event == event_I2C_Transfer_Complete)             //Event when the I2C transfer is completed
        {
          sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);      //Pull MCU out of EM1 mode

          loadpowerTempSensor(false);                     //Power OFF the sensor
          NVIC_DisableIRQ(I2C0_IRQn);                     //Disable the I2C interrupt
          getTempReadings();                              //Calculate the temperature readings and display on the serial console

          nextState = state0_IDLE;
        }
      break;

    default:
      break;
  }
}
