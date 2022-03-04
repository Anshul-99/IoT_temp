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

#include "src/ble.h"
//Event enum
typedef enum
{
  event_DEFUALT = 0,
  event_LETIMER0_UF = 1,
  event_LETIMER0_COMP1 = 2,
  event_I2C_Transfer_Complete = 4
}schedulerEvents;

//Temperature State machine states
typedef enum
{
  state0_IDLE,
  state1_COMP1_POWER_ON,
  state2_I2C_TRANSFER_COMPLETE,
  state3_COMP1_I2C_TRANSFER_COMPLETE,
  state4_UNDERFLOW_READ,
  TEMP_NUM_STATES
}temp_state_t;

//Client Discovery State Machine states
typedef enum
{
  state0_NO_CONNECTION,
  state1_SERVICE_DISCOVERED,
  state2_TEMP_MEASUREMENT_CHAR_ENABLED,
  state3_INDICATION_ENABLED,
  CLIENT_NUM_STATES
}client_state_t;

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
//uint32_t getCurrentEvent();


/*
 * State Machine for temperature measurement
 *
 * Parameters:
 *   uint32_t event: Gives the current event set
 *
 * Returns:
 *   None
 */
void temperature_state_machine(sl_bt_msg_t *evt);

/*
 * State Machine for client discovery
 *
 * Parameters:
 *   sl_bt_msg_t event: Gives the current event set from the external signals data structure of the Bluetooth Stack
 *
 * Returns:
 *   None
 */
void discovery_state_machine(sl_bt_msg_t *evt);

#endif  //SCHEDULER_H

