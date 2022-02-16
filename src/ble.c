/**
 * @file    :   ble.c
 * @brief   :   API for Bluetooth stack events handling
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   16 February 2022
 *
 */

#include "ble.h"
#include "stdbool.h"

// Include logging specifically for this .c file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#define MAX_ADVERTISING_TIME (400)
#define MIN_ADVERTISING_TIME (400)
#define MAX_CONNECTION_TIME (60)
#define MIN_CONNECTION_TIME (60)
#define SLAVE_LATENCY (300)
#define CONNECTION_TIMEOUT (2260)

ble_data_struct_t ble_data;

ble_data_struct_t* getBleDataPtr()
{
  return (&ble_data);
}


void handle_ble_event(sl_bt_msg_t *evt)
{
  ble_data_struct_t* ble_device = getBleDataPtr();

  sl_status_t ret_val;

  uint32_t event;

  switch (SL_BT_MSG_ID(evt->header))
  {

    case sl_bt_evt_system_boot_id:

      ret_val =  sl_bt_system_get_identity_address(&ble_device->myAddress, &ble_device->myAddressType);
      if(ret_val != 0)
        LOG_ERROR("\r\nBluetooth Booting Error\r\n");

      ret_val = sl_bt_advertiser_create_set(&ble_device->advertisingSetHandle);
      if(ret_val != 0)
        LOG_ERROR("\r\nBluetooth Advertising Handle Setting Error\r\n");


      ret_val = sl_bt_advertiser_set_timing(ble_device->advertisingSetHandle, MAX_ADVERTISING_TIME, MIN_ADVERTISING_TIME , 0 , 0);
      if(ret_val != 0)
        LOG_ERROR("\r\nBluetooth Advertising Handle Setting Timing Parameters Error\r\n");

      ret_val = sl_bt_advertiser_start(ble_device->advertisingSetHandle, sl_bt_advertiser_general_discoverable, sl_bt_advertiser_connectable_scannable);
      if(ret_val != 0)
        LOG_ERROR("\r\nBluetooth Advertising Start Error\r\n");
      else
        ble_device->is_connection = false;
      // handle boot event
      break;

    case sl_bt_evt_connection_opened_id:

      ble_device->connectionSetHandle = evt->data.evt_connection_opened.connection;

      ret_val = sl_bt_advertiser_stop(ble_device->advertisingSetHandle);
      if(ret_val != 0)
        LOG_ERROR("\r\nBluetooth Advertising Stop Error\r\n");
      else
        ble_device->is_connection = true;

      ret_val = sl_bt_connection_set_parameters(ble_device->connectionSetHandle, MIN_CONNECTION_TIME , MAX_CONNECTION_TIME,SLAVE_LATENCY, CONNECTION_TIMEOUT, 0, MAX_CONNECTION_TIME);

      // handle open event
      break;

    case sl_bt_evt_connection_closed_id:

      ret_val = sl_bt_advertiser_start(ble_device->advertisingSetHandle , sl_bt_advertiser_general_discoverable, sl_bt_advertiser_connectable_scannable);
      if(ret_val != 0)
        LOG_ERROR("\r\nBluetooth Advertising Start Error\r\n");
      else
        ble_device->is_connection = false;

      // handle close event
      break;

    case sl_bt_evt_connection_parameters_id:

      LOG_INFO("\r\nThe time-interval is %u", evt->data.evt_connection_parameters.interval);
      LOG_INFO("\r\nThe latency is %u", evt->data.evt_connection_parameters.latency);
      LOG_INFO("\r\nThe timeout is %u", evt->data.evt_connection_parameters.timeout);

      break;

    case sl_bt_evt_system_external_signal_id:
      event = sl_bt_evt_system_external_signal_id;
      evt->data.evt_system_external_signal.extsignals = evt;

      break;

    case sl_bt_evt_gatt_server_characteristic_status_id:
      break;

    case sl_bt_evt_gatt_server_indication_timeout_id:
      break;
  }
}




