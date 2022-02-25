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
#include "src/lcd.h"

// Include logging specifically for this .c file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"


//Advertising and Connection Timing Parameters
#define MAX_ADVERTISING_TIME (400)
#define MIN_ADVERTISING_TIME (400)
#define MAX_CONNECTION_TIME (60)
#define MIN_CONNECTION_TIME (60)
#define MIN_CE_LENGTH (0)
#define MAX_CE_LENGTH (0xFFFF)
#define SLAVE_LATENCY (3)
#define CONNECTION_TIMEOUT (70)
#define ADVERTISING_DURATION (0)
#define ADVERTISING_MAX_EVENTS (0)

//Data structure instance
ble_data_struct_t ble_data ;


/*
 * Gives an instance of the private BLE data structure
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   ble_data_struct_t: Returns an instance of the structure
 */
ble_data_struct_t* getBleDataPtr()
{
  return (&ble_data);
}


/*
 * Event responder for Bluetooth events
 *
 * Parameters:
 *   sl_bt_msg_t event: Bluetooth events
 *
 * Returns:
 *   None
 */
void handle_ble_event(sl_bt_msg_t *evt)
{
  sl_status_t error_status;

  switch (SL_BT_MSG_ID(evt->header))                                                                           //Switching data on the header information
  {
    case sl_bt_evt_system_boot_id:
      displayInit();

      displayPrintf(DISPLAY_ROW_NAME, "Server");
      displayPrintf(DISPLAY_ROW_ASSIGNMENT, "A6");

      error_status =  sl_bt_system_get_identity_address(&ble_data.myAddress, &ble_data.myAddressType);        //On booting event, get the device address
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Booting Error\r\n");

      displayPrintf(DISPLAY_ROW_BTADDR,"%02x:%02x:%02x:%02x:%02x:%02x",ble_data.myAddress.addr[0], ble_data.myAddress.addr[1], ble_data.myAddress.addr[2], ble_data.myAddress.addr[3] , ble_data.myAddress.addr[4], ble_data.myAddress.addr[5]);

      error_status = sl_bt_advertiser_create_set(&ble_data.advertisingSetHandle);                             //Create an advertising handle
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Advertising Handle Setting Error\r\n");

      error_status = sl_bt_advertiser_set_timing(ble_data.advertisingSetHandle, MIN_ADVERTISING_TIME, MAX_ADVERTISING_TIME , ADVERTISING_DURATION , ADVERTISING_MAX_EVENTS);           //Set timing parameters for the advertising handle
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Advertising Handle Setting Timing Parameters Error\r\n");

      error_status = sl_bt_advertiser_start(ble_data.advertisingSetHandle, sl_bt_advertiser_general_discoverable, sl_bt_advertiser_connectable_scannable);   //Start advertising from the client
      ble_data.is_connection = false;
      displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Advertising Start Error\r\n");



      break;

      //      PACKSTRUCT( struct sl_bt_evt_connection_opened_s
      //      {
      //        bd_addr address;      /**< Remote device address */
      //        uint8_t address_type; /**< Enum @ref sl_bt_gap_address_type_t. Remote device
      //                                   address type. Values:
      //                                     - <b>sl_bt_gap_public_address (0x0):</b> Public
      //                                       device address
      //                                     - <b>sl_bt_gap_static_address (0x1):</b> Static
      //                                       device address
      //                                     - <b>sl_bt_gap_random_resolvable_address
      //                                       (0x2):</b> Resolvable private random address
      //                                     - <b>sl_bt_gap_random_nonresolvable_address
      //                                       (0x3):</b> Non-resolvable private random
      //                                       address */
      //        uint8_t master;       /**< Device role in connection. Values:
      //                                     - <b>0:</b> Peripheral
      //                                     - <b>1:</b> Central */
      //        uint8_t connection;   /**< Handle for new connection */
      //        uint8_t bonding;      /**< Bonding handle. Values:
      //                                     - <b>SL_BT_INVALID_BONDING_HANDLE (0xff):</b> No
      //                                       bonding
      //                                     - <b>Other:</b> Bonding handle */
      //        uint8_t advertiser;   /**< The local advertising set that this connection was
      //                                   opened to. Values:
      //                                     - <b>SL_BT_INVALID_ADVERTISING_SET_HANDLE
      //                                       (0xff):</b> Invalid value or not applicable.
      //                                       Ignore this field
      //                                     - <b>Other:</b> The advertising set handle */
      //      })
    case sl_bt_evt_connection_opened_id:
      ble_data.connectionSetHandle = evt->data.evt_connection_opened.connection;              //Save the connection handle

      displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");

      error_status = sl_bt_advertiser_stop(ble_data.advertisingSetHandle);                    //Stop the advertising since a new connection is found
      ble_data.is_connection = true;
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Advertising Stop Error\r\n");


      error_status = sl_bt_connection_set_parameters(ble_data.connectionSetHandle, MIN_CONNECTION_TIME , MAX_CONNECTION_TIME, SLAVE_LATENCY, CONNECTION_TIMEOUT, MIN_CE_LENGTH, MAX_CE_LENGTH);      //Set timing parameters for the new connection
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Advertising Connection Setup Error\r\n");

      break;

    case sl_bt_evt_connection_closed_id:
      error_status = sl_bt_advertiser_start(ble_data.advertisingSetHandle , sl_bt_advertiser_general_discoverable, sl_bt_advertiser_connectable_scannable);    //When connection is closed, start advertising again
      //      LOG_INFO("\r\nConnection: %d closed due to: %d\r\n", evt->data.evt_connection_closed.connection, evt->data.evt_connection_closed.reason);
      ble_data.is_connection = false;
      ble_data.is_indication_enabled = false;
      ble_data.is_indication_in_flight = false;
      displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Advertising Start Error\r\n");

      break;

      //Event to clear the LCD charge every second in repeat mode
    case sl_bt_evt_system_soft_timer_id:
      displayUpdate();

      break;

      //      PACKSTRUCT( struct sl_bt_evt_connection_parameters_s
      //      {
      //        uint8_t  connection;    /**< Connection handle */
      //        uint16_t interval;      /**< Connection interval. Time = Value x 1.25 ms */
      //        uint16_t latency;       /**< Peripheral latency (how many connection intervals
      //                                     the peripheral can skip) */
      //        uint16_t timeout;       /**< Supervision timeout. Time = Value x 10 ms */
      //        uint8_t  security_mode; /**< Enum @ref sl_bt_connection_security_t. Connection
      //                                     security mode. Values:
      //                                       - <b>sl_bt_connection_mode1_level1 (0x0):</b>
      //                                         No security
      //                                       - <b>sl_bt_connection_mode1_level2 (0x1):</b>
      //                                         Unauthenticated pairing with encryption
      //                                       - <b>sl_bt_connection_mode1_level3 (0x2):</b>
      //                                         Authenticated pairing with encryption
      //                                       - <b>sl_bt_connection_mode1_level4 (0x3):</b>
      //                                         Authenticated Secure Connections pairing with
      //                                         encryption using a 128-bit strength
      //                                         encryption key */
      //        uint16_t txsize;        /**< Maximum Data Channel PDU Payload size that the
      //                                     controller can send in an air packet */
      //      })
    case sl_bt_evt_connection_parameters_id:
      //      LOG_INFO("\r\nThe time-interval is %d milliseconds\r\n", ((evt->data.evt_connection_parameters.interval * 125)/100));          //Log the set parameters
      //      LOG_INFO("\r\nThe latency is %d\r\n", evt->data.evt_connection_parameters.latency);
      //      LOG_INFO("\r\nThe timeout is %d milliseconds\r\n", (evt->data.evt_connection_parameters.timeout * 10));

      break;

    case sl_bt_evt_system_external_signal_id:
      //      ble_data.eventSet =  evt->data.evt_system_external_signal.extsignals;
      //      LOG_INFO("\r\nExternal interrupt triggered\r\n");

      break;

      //      PACKSTRUCT( struct sl_bt_evt_gatt_server_characteristic_status_s
      //      {
      //        uint8_t  connection;          /**< Connection handle */
      //        uint16_t characteristic;      /**< GATT characteristic handle. This value is
      //                                           normally received from the
      //                                           gatt_characteristic event. */
      //        uint8_t  status_flags;        /**< Enum @ref
      //                                           sl_bt_gatt_server_characteristic_status_flag_t.
      //                                           Describes whether Client Characteristic
      //                                           Configuration was changed or if a
      //                                           confirmation was received. Values:
      //                                             - <b>sl_bt_gatt_server_client_config
      //                                               (0x1):</b> Characteristic client
      //                                               configuration has been changed.
      //                                             - <b>sl_bt_gatt_server_confirmation
      //                                               (0x2):</b> Characteristic confirmation
      //                                               has been received. */
      //        uint16_t client_config_flags; /**< Enum @ref
      //                                           sl_bt_gatt_server_client_configuration_t.
      //                                           This field carries the new value of the
      //                                           Client Characteristic Configuration. If the
      //                                           status_flags is 0x2 (confirmation
      //                                           received), the value of this field can be
      //                                           ignored. */
      //        uint16_t client_config;       /**< The handle of client-config descriptor. */
      //      })
    case sl_bt_evt_gatt_server_characteristic_status_id:
      if((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement) && (evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_server_indication)) //Indication enabled flag
        ble_data.is_indication_enabled = true;
      else
        ble_data.is_indication_enabled = false;

      if (evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement && (evt->data.evt_gatt_server_characteristic_status.status_flags == sl_bt_gatt_server_confirmation))
        {
          if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_server_confirmation)
            ble_data.is_indication_in_flight = false;                                                                          //If the config flag is 0x02, set the indication in flight to false
        }
      break;

    case sl_bt_evt_gatt_server_indication_timeout_id:
      //      LOG_INFO("\r\nBluetooth Client Indication Acknowledgement Time-out\r\n");
      if(ble_data.is_indication_in_flight == true)
        ble_data.is_indication_in_flight = false;
      ble_data.is_connection = false;
      break;

    default:
      break;
  }
}






