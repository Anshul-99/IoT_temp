/**
 * @file    :   scheduler.c
 * @brief   :   API for event scheduler
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   16 February 2022
 *
 */

#include "stdint.h"
#include "em_core.h"
#include "scheduler.h"
#include "src/i2c.h"
#include "sl_power_manager.h"
#include "src/timers.h"
#include "sl_bt_api.h"
#include "src/ble.h"
#include "gatt_db.h"
#include "lcd.h"
#include "ble_device_type.h"
#include "ble.h"
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>


#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"


//Bits for the event
#define bit_LETIMER0_UF (1)
#define bit_LETIMER_COMP1 (2)
#define bit_I2C_TRANSFER (4)



#define PTR_OUT_OF_BOUND (-1)

// Declare memory for the queue/buffer/fifo, and the write and read pointers
queue_struct_t   my_queue[QUEUE_DEPTH]; // the queue
uint32_t         wptr = 0;              // write pointer
uint32_t         rptr = 0;              // read pointer

static bool isfull = false;
static bool isempty = true;

static uint32_t length = 0;


/* Sets an event when interrupt is triggered
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

  sl_bt_external_signal(event_LETIMER0_UF);   //using BLE's stack to set an event

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

  sl_bt_external_signal(event_LETIMER0_COMP1);   //using BLE's stack to set an event

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

  sl_bt_external_signal(event_I2C_Transfer_Complete);    //using BLE's stack to set an event

  CORE_EXIT_CRITICAL();          //Exit critical section
}


/* Sets an event when interrupt is triggered for button
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void setSchedulerEventExternalPushButton0()
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();          //Enter critical section

  sl_bt_external_signal(event_EXT_BUTTON0_Interrupt);    //using BLE's stack to set an event

  CORE_EXIT_CRITICAL();          //Exit critical section

}

/* Sets an event when interrupt is triggered for button
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void setSchedulerEventExternalPushButton1()
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();          //Enter critical section

  sl_bt_external_signal(event_EXT_BUTTON1_Interrupt);    //using BLE's stack to set an event

  CORE_EXIT_CRITICAL();          //Exit critical section

}

/*
 * State Machine for temperature measurement
 *
 * Parameters:
 *   sl_bt_msg_t event: Gives the current event set from the external signals data structure of the Bluetooth Stack
 *
 * Returns:
 *   None
 */
void temperature_state_machine(sl_bt_msg_t *evt)
{
  ble_data_struct_t *bleData;
  sl_status_t error_status;

  bleData = getBleDataPtr();

  uint8_t htm_temperature_buffer[5];
  uint8_t *p = &htm_temperature_buffer[1];
  uint32_t htm_temperature_flt;

  temp_state_t currentState;

  static temp_state_t nextState = state0_IDLE;                             //First state is Idle by default

  currentState = nextState;                                           //Update the current state

  switch(currentState)
  {
    case state0_IDLE:
      nextState = state0_IDLE;

      if(bleData->is_connection == true)
        {
          if (evt->data.evt_system_external_signal.extsignals == event_LETIMER0_UF)
            {

              timerWaitUs_irq(80000);                                    //Wait for 80 msec [Setup time]

              nextState = state1_COMP1_POWER_ON;
            }
        }

      else
        displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
      break;

    case state1_COMP1_POWER_ON:
      nextState = state1_COMP1_POWER_ON;

      if((bleData->is_connection == true) && (bleData->is_htm_indication_enabled ==  true))
        {
          if(evt->data.evt_system_external_signal.extsignals == event_LETIMER0_COMP1)                             //Event when the timer delay elapses
            {
              sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);  //While transfer is in progress, put the MCU in EM1 energy mode

              sendI2C_command();                                        //Write temperature measurement sequence to the sensor

              nextState = state2_I2C_TRANSFER_COMPLETE;
            }
        }

      else
        {
          nextState = state0_IDLE;
          displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
        }


      break;

    case state2_I2C_TRANSFER_COMPLETE:
      nextState = state2_I2C_TRANSFER_COMPLETE;

      if((bleData->is_connection == true) && (bleData->is_htm_indication_enabled ==  true))
        {

          if(evt->data.evt_system_external_signal.extsignals == event_I2C_Transfer_Complete)                    //Event when the I2C transfer is completed
            {
              sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);      //Pull MCU out of EM1 mode

              timerWaitUs_irq(10800);                                //I2C sequence time to wait while the sequence is transmitted [10.8 msec]

              nextState = state3_COMP1_I2C_TRANSFER_COMPLETE;
            }

        }

      else
        {
          nextState = state0_IDLE;
          sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
          displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
        }

      break;

    case state3_COMP1_I2C_TRANSFER_COMPLETE:
      nextState = state3_COMP1_I2C_TRANSFER_COMPLETE;


      if((bleData->is_connection == true) && (bleData->is_htm_indication_enabled ==  true))
        {

          if(evt->data.evt_system_external_signal.extsignals == event_LETIMER0_COMP1)                       //Event when the write sequence is written
            {
              sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);   //While transfer is in progress, put the MCU in EM1 energy mode

              receiveI2C_command();                              //Transmit the read command

              nextState = state4_UNDERFLOW_READ;
            }

        }

      else
        {
          nextState = state0_IDLE;
          displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
        }
      break;

    case state4_UNDERFLOW_READ:
      nextState = state4_UNDERFLOW_READ;

      if((bleData->is_connection == true) && (bleData->is_htm_indication_enabled ==  true))
        {
          if(evt->data.evt_system_external_signal.extsignals == event_I2C_Transfer_Complete)             //Event when the I2C transfer is completed
            {
              sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);      //Pull MCU out of EM1 mode

              NVIC_DisableIRQ(I2C0_IRQn);                     //Disable the I2C interrupt
              uint32_t temp_in_C = getTempReadings();                              //Calculate the temperature readings and display on the serial console

              displayPrintf(DISPLAY_ROW_TEMPVALUE, "Temp=%d C", temp_in_C);

              htm_temperature_flt = UINT32_TO_FLOAT(temp_in_C*1000, -3);

              UINT32_TO_BITSTREAM(p, htm_temperature_flt);
              //              LOG_INFO("\r\nConnection handle: %d\r\n", bleData->is_connection);
              //              LOG_INFO("\r\nIndication Enabled: %d\r\n", bleData->is_indication_enabled);
              //              LOG_INFO("\r\nIndication in flight: %d\r\n", bleData->is_indication_in_flight);


              //Only write to local GATT database if there is a connection present, if the indicate button is enabled on the GUI

              error_status = sl_bt_gatt_server_write_attribute_value(gattdb_temperature_measurement,  0,  5,  p);          //Update the local GATT database
              if(error_status != SL_STATUS_OK)
                LOG_ERROR("\r\nUpdating Local Gatt-Database Error\r\n");


              //Only send indication if there is a connection present, if the indicate button is enabled on the GUI and no other indication is in flight
              if((bleData->is_htm_indication_in_flight == false))
                {
                  error_status = sl_bt_gatt_server_send_indication(bleData->connectionSetHandle, gattdb_temperature_measurement, 5, &htm_temperature_buffer[0]);   //Send an indication with the temperature data buffer
                  if(error_status != SL_STATUS_OK)
                    LOG_ERROR("\r\nSending Indication Error: %d\r\n", error_status);
                  else
                    bleData->is_htm_indication_in_flight = true;      //Set the flag to true if indication was sent
                }
              else
                {
                  write_queue(gattdb_temperature_measurement, sizeof(htm_temperature_buffer), htm_temperature_buffer);
                }
            }
          nextState = state0_IDLE;
        }
      else
        {
          nextState = state0_IDLE;
          sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
          displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
        }
      break;

    default:
      break;
  }
}


/*
 * State Machine for client discovery
 *
 * Parameters:
 *   sl_bt_msg_t event: Gives the current event set from the external signals data structure of the Bluetooth Stack
 *
 * Returns:
 *   None
 */
void discovery_state_machine(sl_bt_msg_t *evt)
{
  ble_data_struct_t *bleData;
  sl_status_t error_status;

  bleData = getBleDataPtr();

  client_state_t currentState;

  static client_state_t nextState = state0_NO_CONNECTION;                             //First state is Idle by default

  currentState = nextState;                                           //Update the current state

  switch(currentState)
  {
    case state0_NO_CONNECTION:
      nextState = state0_NO_CONNECTION;

      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_opened_id)
        {
          //          nextState = state0_BUTTON_SERVICE_DISCOVERY;
          nextState = state1_TEMP_SERVICE_DISCOVERED;
          displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");

          displayPrintf(DISPLAY_ROW_BTADDR2,"%02x:%02x:%02x:%02x:%02x:%02x",SERVER_BT_ADDRESS.addr[5], SERVER_BT_ADDRESS.addr[4], SERVER_BT_ADDRESS.addr[3], SERVER_BT_ADDRESS.addr[2] , SERVER_BT_ADDRESS.addr[1], SERVER_BT_ADDRESS.addr[0]);
          error_status = sl_bt_gatt_discover_primary_services_by_uuid(bleData->connectionSetHandle, HTM_SERVICE_UUID_LEN , HTM_SERVICE_UUID);
          if(error_status != SL_STATUS_OK)
            LOG_ERROR("\r\nError discovering a service - HTM\r\n");

        }
      else
        {
          if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_closed_id)
            {
              nextState = state0_NO_CONNECTION;
              error_status = sl_bt_scanner_start(PHYSICAL_LAYER_1M, sl_bt_scanner_discover_observation);
              if(error_status != SL_STATUS_OK)
                LOG_ERROR("\r\nError starting connection scanning\r\n");
              displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
              displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
              displayPrintf(DISPLAY_ROW_BTADDR2, " ");
            }
        }
      break;

    case state1_TEMP_SERVICE_DISCOVERED:
      nextState = state1_TEMP_SERVICE_DISCOVERED;

      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)
        {
          //          nextState = state1_BUTTON_SERVICE_DISCOVERED;
          nextState = state2_TEMP_MEASUREMENT_CHAR_ENABLED;

          error_status = sl_bt_gatt_discover_characteristics_by_uuid(bleData->connectionSetHandle, bleData->htmServiceHandle, sizeof(HTM_CHAR_UUID), HTM_CHAR_UUID );
          if(error_status != SL_STATUS_OK)
            LOG_ERROR("\r\nError discovering a characteristic - TEMPERATURE MEASUREMENT\r\n");
        }
      else
        {
          if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_closed_id)
            {
              nextState = state0_NO_CONNECTION;

              error_status = sl_bt_scanner_start(PHYSICAL_LAYER_1M, sl_bt_scanner_discover_observation);
              if(error_status != SL_STATUS_OK)
                LOG_ERROR("\r\nError starting connection scanning\r\n");
              displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
              displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
              displayPrintf(DISPLAY_ROW_BTADDR2, " ");
            }
        }

      break;

    case state2_TEMP_MEASUREMENT_CHAR_ENABLED:
      nextState = state2_TEMP_MEASUREMENT_CHAR_ENABLED;

      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)
        {
          //          nextState = state2_BUTTON_MEASUREMENT_CHAR_ENABLED;
          nextState = state0_BUTTON_SERVICE_DISCOVERY;

          error_status = sl_bt_gatt_set_characteristic_notification(bleData->connectionSetHandle, bleData->htmCharacteristicHandle, sl_bt_gatt_indication);
          if(error_status != SL_STATUS_OK)
            LOG_ERROR("\r\nError setting up characteristic notification\r\n");
          else
            displayPrintf(DISPLAY_ROW_CONNECTION, "Handling Indications");
        }
      else
        {
          if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_closed_id)
            {
              nextState = state0_NO_CONNECTION;
              error_status = sl_bt_scanner_start(PHYSICAL_LAYER_1M, sl_bt_scanner_discover_observation);
              if(error_status != SL_STATUS_OK)
                LOG_ERROR("\r\nError starting connection scanning\r\n");
              displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
              displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
              displayPrintf(DISPLAY_ROW_BTADDR2, " ");
            }
        }

      break;

    case state0_BUTTON_SERVICE_DISCOVERY:
      nextState = state0_BUTTON_SERVICE_DISCOVERY;
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)
        {
          //          nextState = state1_TEMP_SERVICE_DISCOVERED;
          nextState = state1_BUTTON_SERVICE_DISCOVERED;

          error_status = sl_bt_gatt_discover_primary_services_by_uuid(bleData->connectionSetHandle, BUTTON_SERVICE_UUID_LEN, BUTTON_SERVICE_UUID);
          if(error_status != SL_STATUS_OK)
            LOG_ERROR("\r\nError discovering a service - BUTTON\r\n");
        }

      else
        {
          if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_closed_id)
            {
              nextState = state0_NO_CONNECTION;
              error_status = sl_bt_scanner_start(PHYSICAL_LAYER_1M, sl_bt_scanner_discover_observation);
              if(error_status != SL_STATUS_OK)
                LOG_ERROR("\r\nError starting connection scanning\r\n");
              displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
              displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
              displayPrintf(DISPLAY_ROW_BTADDR2, " ");
            }
        }

      break;



    case state1_BUTTON_SERVICE_DISCOVERED:
      nextState = state1_BUTTON_SERVICE_DISCOVERED;

      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)
        {
          //          nextState = state2_TEMP_MEASUREMENT_CHAR_ENABLED;
          nextState = state2_BUTTON_MEASUREMENT_CHAR_ENABLED;

          error_status = sl_bt_gatt_discover_characteristics_by_uuid(bleData->connectionSetHandle, bleData->buttonServiceHandle, sizeof(BUTTON_CHAR_UUID), BUTTON_CHAR_UUID);
          if(error_status != SL_STATUS_OK)
            LOG_ERROR("\r\nError discovering a characteristic - BUTTON\r\n");
        }
      else
        {
          if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_closed_id)
            {
              nextState = state0_NO_CONNECTION;
              error_status = sl_bt_scanner_start(PHYSICAL_LAYER_1M, sl_bt_scanner_discover_observation);
              if(error_status != SL_STATUS_OK)
                LOG_ERROR("\r\nError starting connection scanning\r\n");
              displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
              displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
              displayPrintf(DISPLAY_ROW_BTADDR2, " ");
            }
        }
      break;




    case state2_BUTTON_MEASUREMENT_CHAR_ENABLED:
      nextState = state2_BUTTON_MEASUREMENT_CHAR_ENABLED;

      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)
        {
          nextState = state3_INDICATION_ENABLED;
          error_status = sl_bt_gatt_set_characteristic_notification(bleData->connectionSetHandle, bleData->buttonCharacteristicHandle, sl_bt_gatt_indication);
          if(error_status != SL_STATUS_OK)
            LOG_ERROR("\r\nError setting up characteristic notification\r\n");
          else
            displayPrintf(DISPLAY_ROW_CONNECTION, "Handling Indications");
        }
      else
        {
          if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_closed_id)
            {
              nextState = state0_NO_CONNECTION;
              error_status = sl_bt_scanner_start(PHYSICAL_LAYER_1M, sl_bt_scanner_discover_observation);
              if(error_status != SL_STATUS_OK)
                LOG_ERROR("\r\nError starting connection scanning\r\n");
              displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
              displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
              displayPrintf(DISPLAY_ROW_BTADDR2, " ");
            }
        }
      break;


    case state3_INDICATION_ENABLED:
      nextState = state3_INDICATION_ENABLED;

      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_closed_id)
        {
          nextState = state0_NO_CONNECTION;
          error_status = sl_bt_scanner_start(PHYSICAL_LAYER_1M, sl_bt_scanner_discover_observation);
          if(error_status != SL_STATUS_OK)
            LOG_ERROR("\r\nError starting connection scanning\r\n");
          displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
          displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
          displayPrintf(DISPLAY_ROW_BTADDR2, " ");
        }
      break;

    default:
      break;
  }
}

/*
 * Computes the next pointer
 *
 * Parameters:
 *   ptr: Current pointer location
 *
 * Returns:
 *   Incremented pointer
 */
static uint32_t nextPtr(uint32_t ptr) {


  if(ptr >= QUEUE_DEPTH)                    //If pointer is greater than the maximum depth, return an error
    return PTR_OUT_OF_BOUND;


  else if((ptr + 1) == QUEUE_DEPTH)         //If the queue is full, wrap the pointer to the front
    return 0;

  else                                     //If the queue is not full, return the incremented pointer
    return ptr + 1;

} // nextPtr()



/*
 *Writes to the
 *
 * Parameters:
 *   charHandle: The characteristic handle
 *   bufferLength: The length of data to be written
 *   indication_data[]: Data buffer to be written in the queue
 *
 * Returns:
 *   If the queue is full(true) or not(false)
 */
bool write_queue (uint16_t charHandle , size_t bufferLength , uint8_t indication_data[])
{

  if (isfull)                           //If full flag is set, return true
    return true;

  my_queue[wptr].charHandle = charHandle;
  my_queue[wptr].bufferLength = bufferLength;
  memcpy(my_queue[wptr].buffer , indication_data , bufferLength);

  length++;                            //Increment length

  isempty = false;                     //Reset empty flag since a successful entry has been made

  if(length == QUEUE_DEPTH)            //Check if queue is full
    isfull = true;

  if (!isfull)                        //If the queue is not full, increment the write pointer
    wptr = nextPtr(wptr);

  return false;

} // write_queue()



/*
 * Reads the queue
 *
 * Parameters:
 *   *charHandle: The characteristic handle pointer
 *   *bufferLength: The length of data to be written pointer
 *   *indication_data[]: Data buffer to be read from the queue pointer
 *
 * Returns:
 *   If the queue is full or not
 */
bool read_queue (uint16_t *charHandle , size_t *bufferLength , uint8_t *indication_data) {

  if (isempty)                     //If empty flag is set, returns true
    return true;


  *charHandle = my_queue[rptr].charHandle;
  *bufferLength = my_queue[rptr].bufferLength;
  memcpy(indication_data , my_queue[rptr].buffer , my_queue[rptr].bufferLength);

  length--;                      //Decrement the length

  if(isfull)                     //If previously the queue was full, now since one place is available, increment the write pointer
    wptr = nextPtr(wptr);

  isfull = false;                //Reset the full flag since one entry has been popped from the queue

  rptr = nextPtr(rptr);          //Process the read pointer

  if(wptr == rptr)               //Empty condition
    isempty = true;

  return false;

} // read_queue()



/*
 * Gets the queue status
 *
 * Parameters:
 *   *wptr: Write pointer value
 *   *rptr: Read pointer value
 *   *full: Whether full or not
 *   *empty: Whether empty or not
 *
 * Returns:
 *   None
 */
void get_queue_status (uint32_t *_wptr, uint32_t *_rptr, bool *_full, bool *_empty)
{

  *_wptr = wptr;
  *_rptr = rptr;

  *_full = isfull;
  *_empty = isempty;

} // get_queue_status()



/*
 * Returns the queue length
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   Length of the queue
 */
uint32_t get_queue_depth()
{

  return length;          //Returns length

} // get_queue_depth()
