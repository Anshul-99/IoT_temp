/**
 * @file    :   scheduler.h
 * @brief   :   Headers and function definitions for event scheduler
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   10 February 2022
 *
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H


//Event enum
typedef enum
{
  event_DEFUALT = 0,
  event_LETIMER0_UF = 1,
  event_LETIMER0_COMP1 = 2,
  event_I2C_Transfer_Complete = 4
}schedulerEvents;

//State machine states
typedef enum
{
  state0_IDLE,
  state1_COMP1_POWER_ON,
  state2_I2C_TRANSFER_COMPLETE,
  state3_COMP1_I2C_TRANSFER_COMPLETE,
  state4_UNDERFLOW_READ,
  MY_NUM_STATES
}state_t;


/*
 * Sets an event when interrupt is triggered
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void setSchedulerEventTemp();


/*
 * Sets an event when interrupt is triggered
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void setSchedulerEventDelay();


/*
 * Sets an event when interrupt is triggered
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void setSchedulerEventTransferComplete();


/*
 * Returns the current event triggered.
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   uint32_t : Event triggered
 */
uint32_t getCurrentEvent();


/*
 * State Machine for temperature measurement
 *
 * Parameters:
 *   uint32_t event: Gives the current event set
 *
 * Returns:
 *   None
 */
void temperature_state_machine(uint32_t event);

#endif  //SCHEDULER_H

