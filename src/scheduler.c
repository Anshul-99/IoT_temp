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

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"


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

  state_t currentState;

  static state_t nextState = state0_IDLE;                             //First state is Idle by default

  currentState = nextState;                                           //Update the current state

  switch(currentState)
  {
    case state0_IDLE:
      nextState = state0_IDLE;

      if((bleData->is_connection == true) && (bleData->is_indication_enabled ==  true))
        {
          if (evt->data.evt_system_external_signal.extsignals == event_LETIMER0_UF)
            {
              loadpowerTempSensor(true);                                 //Power ON the temperature sensor

              timerWaitUs_irq(80000);                                    //Wait for 80 msec [Setup time]

              nextState = state1_COMP1_POWER_ON;
            }
        }
      break;

    case state1_COMP1_POWER_ON:
      nextState = state1_COMP1_POWER_ON;

      if((bleData->is_connection == true) && (bleData->is_indication_enabled ==  true))
        {
          if(evt->data.evt_system_external_signal.extsignals == event_LETIMER0_COMP1)                             //Event when the timer delay elapses
            {
              sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);  //While transfer is in progress, put the MCU in EM1 energy mode

              sendI2C_command();                                        //Write temperature measurement sequence to the sensor

              nextState = state2_I2C_TRANSFER_COMPLETE;
            }
        }
      else
        nextState = state0_IDLE;

      break;

    case state2_I2C_TRANSFER_COMPLETE:
      nextState = state2_I2C_TRANSFER_COMPLETE;

      if((bleData->is_connection == true) && (bleData->is_indication_enabled ==  true))
        {
          if(evt->data.evt_system_external_signal.extsignals == event_I2C_Transfer_Complete)                    //Event when the I2C transfer is completed
            {
              sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);      //Pull MCU out of EM1 mode

              timerWaitUs_irq(10800);                                //I2C sequence time to wait while the sequence is transmitted [10.8 msec]

              nextState = state3_COMP1_I2C_TRANSFER_COMPLETE;
            }
        }
      else
        nextState = state0_IDLE;
      break;

    case state3_COMP1_I2C_TRANSFER_COMPLETE:
      nextState = state3_COMP1_I2C_TRANSFER_COMPLETE;

      if((bleData->is_connection == true) && (bleData->is_indication_enabled ==  true))
        {
          if(evt->data.evt_system_external_signal.extsignals == event_LETIMER0_COMP1)                       //Event when the write sequence is written
            {
              sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);   //While transfer is in progress, put the MCU in EM1 energy mode

              receiveI2C_command();                              //Transmit the read command

              nextState = state4_UNDERFLOW_READ;
            }
        }
      else
        nextState = state0_IDLE;
      break;

    case state4_UNDERFLOW_READ:
      nextState = state4_UNDERFLOW_READ;

      if((bleData->is_connection == true) && (bleData->is_indication_enabled ==  true))
        {
          if(evt->data.evt_system_external_signal.extsignals == event_I2C_Transfer_Complete)             //Event when the I2C transfer is completed
            {
              sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);      //Pull MCU out of EM1 mode

              loadpowerTempSensor(false);                     //Power OFF the sensor
              NVIC_DisableIRQ(I2C0_IRQn);                     //Disable the I2C interrupt
              uint32_t temp_in_C = getTempReadings();                              //Calculate the temperature readings and display on the serial console

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
              if((bleData->is_connection == true) && (bleData->is_indication_enabled ==  true) && (bleData->is_indication_in_flight == false))
                {
                  error_status = sl_bt_gatt_server_send_indication(bleData->connectionSetHandle, gattdb_temperature_measurement, 5, &htm_temperature_buffer[0]);   //Send an indication with the temperature data buffer
                  if(error_status != SL_STATUS_OK)
                    LOG_ERROR("\r\nSending Indication Error\r\n");
                  else
                    bleData->is_indication_in_flight = true;      //Set the flag to true if indication was sent
                }
            }
          nextState = state0_IDLE;
        }
      else
        nextState = state0_IDLE;
      break;

    default:
      break;
  }
}
