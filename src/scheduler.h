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
  event_I2C_Transfer_Complete = 4,
  event_EXT_BUTTON_Interrupt = 8
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

#define QUEUE_DEPTH      (16)
#define USE_ALL_ENTRIES  (1)

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

/* Sets an event when interrupt is triggered for button
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void setSchedulerEventExternalPushButton();


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



// This is the number of entries in the queue. Please leave
// this value set to 16.

// Student edit:
//   define this to 1 if your design uses all array entries
//   define this to 0 if your design leaves 1 array entry empty




// Modern C (circa 2021 does it this way)
// typedef <name> is referred to as an anonymous struct definition
// This is the structure of 1 queue/buffer entry
typedef struct {

  uint16_t charHandle; // Char handle from gatt_db.h
  size_t bufferLength; // Length of buffer in bytes to send
  uint8_t buffer[5]; // The actual data buffer for the indication. Need space for HTM (5 bytes) and button_state (2 bytes) indications, array [0] holds the flags byte.

} queue_struct_t;


// function prototypes
bool     write_queue (uint16_t charHandle , size_t bufferLength , uint8_t indication_data[]);
bool     read_queue (uint16_t *charHandle , size_t *bufferLength , uint8_t *indication_data);
void     get_queue_status (uint32_t *_wptr, uint32_t *_rptr, bool *_full, bool *_empty);
uint32_t get_queue_depth (void);

#endif
