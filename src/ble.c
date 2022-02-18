/**
 * @file    :   ble.c
 * @brief   :   API for Bluetooth stack events handling
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   16 February 2022
 *
 */

#include "src/ble.h"
#include "stdbool.h"
#include "gatt_db.h"

// Include logging specifically for this .c file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#define MAX_ADVERTISING_TIME (0x190)
#define MIN_ADVERTISING_TIME (0x190)
#define MAX_CONNECTION_TIME (0x3C)
#define MIN_CONNECTION_TIME (0x3C)
#define SLAVE_LATENCY (3)
#define CONNECTION_TIMEOUT (70)

ble_data_struct_t ble_data;

ble_data_struct_t* getBleDataPtr()
{
  return (&ble_data);
}

void handle_ble_event(sl_bt_msg_t *evt)
{
  sl_status_t error_status;

  getBleDataPtr();

  switch (SL_BT_MSG_ID(evt->header))
  {
    case sl_bt_evt_system_boot_id:
      error_status =  sl_bt_system_get_identity_address(&ble_data.myAddress, &ble_data.myAddressType);
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Booting Error\r\n");

      error_status = sl_bt_advertiser_create_set(&ble_data.advertisingSetHandle);
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Advertising Handle Setting Error\r\n");

      error_status = sl_bt_advertiser_set_timing(ble_data.advertisingSetHandle, MIN_ADVERTISING_TIME, MAX_ADVERTISING_TIME , 0 , 0);
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Advertising Handle Setting Timing Parameters Error\r\n");

      //TODO: Check about the parameters
      error_status = sl_bt_advertiser_start(ble_data.advertisingSetHandle, sl_bt_advertiser_general_discoverable, sl_bt_advertiser_connectable_scannable);
      ble_data.is_connection = false;
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Advertising Start Error\r\n");

      // handle boot event
      break;

    case sl_bt_evt_connection_opened_id:
      ble_data.connectionSetHandle = evt->data.evt_connection_opened.connection;

      error_status = sl_bt_advertiser_stop(ble_data.advertisingSetHandle);
      ble_data.is_connection = true;
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Advertising Stop Error\r\n");


      error_status = sl_bt_connection_set_parameters(ble_data.connectionSetHandle, MIN_CONNECTION_TIME , MAX_CONNECTION_TIME, SLAVE_LATENCY, CONNECTION_TIMEOUT, 0, 0xFFFF);
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Advertising Connection Setup Error\r\n");

      // handle open event
      break;

    case sl_bt_evt_connection_closed_id:
      error_status = sl_bt_advertiser_start(ble_data.advertisingSetHandle , sl_bt_advertiser_general_discoverable, sl_bt_advertiser_connectable_scannable);
      ble_data.is_connection = false;
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Advertising Start Error\r\n");

      // handle close event
      break;

    case sl_bt_evt_connection_parameters_id:
      LOG_INFO("\r\nThe time-interval is %d\r\n", evt->data.evt_connection_parameters.interval);
      LOG_INFO("\r\nThe latency is %d\r\n", evt->data.evt_connection_parameters.latency);
      LOG_INFO("\r\nThe timeout is %d\r\n", evt->data.evt_connection_parameters.timeout);

      break;

    case sl_bt_evt_system_external_signal_id:
      //      ble_data.eventSet =  evt->data.evt_system_external_signal.extsignals;
      //      LOG_INFO("\r\nExternal interrupt triggered\r\n");

      break;

    case sl_bt_evt_gatt_server_characteristic_status_id:
      if((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement) && (evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_server_indication))
        ble_data.is_indication_enabled = true;
      else
        ble_data.is_indication_enabled = false;

      if (evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement && (evt->data.evt_gatt_server_characteristic_status.status_flags == sl_bt_gatt_server_confirmation))
        {
          if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_server_confirmation)
            {
              ble_data.is_indication_in_flight = false;
            }
        }
      break;

    case sl_bt_evt_gatt_server_indication_timeout_id:
      LOG_ERROR("\r\nBluetooth Server Time-out error\r\n");
      ble_data.is_indication_in_flight = true;
      break;
  }
}




