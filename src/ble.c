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
#include "ble_device_type.h"
#include <math.h>
#include "src/scheduler.h"
#include "src/gpio.h"


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

#define PASSIVE_SCAN (0)
#define SCAN_INTERVAL (80)
#define SCAN_WINDOW (40)
#define CLIENT_LATENCY (4)
#define CLIENT_MAX_CE_LENGTH (4)
#define CLIENT_TIMEOUT (83)
#define CONFIRM_BONDING (1)
#define CONFIRM_PASSKEY (1)
#define BONDING_FLAG (0x2F)
#define READ_CHAR_ERROR_CODE (0x110F)

//Data structure instance
ble_data_struct_t ble_data ;

#if (DEVICE_IS_BLE_SERVER == 0)
// -----------------------------------------------
// Private function, original from Dan Walkes. I fixed a sign extension bug.
// We'll need this for Client A7 assignment to convert health thermometer
// indications back to an integer. Convert IEEE-11073 32-bit float to signed integer.
// -----------------------------------------------
static int32_t FLOAT_TO_INT32(const uint8_t *value_start_little_endian)
{
  uint8_t signByte = 0;
  int32_t mantissa;
  // input data format is:
  // [0] = flags byte
  // [3][2][1] = mantissa (2's complement)
  // [4] = exponent (2's complement)
  // BT value_start_little_endian[0] has the flags byte
  int8_t exponent = (int8_t)value_start_little_endian[4];
  // sign extend the mantissa value if the mantissa is negative
  if (value_start_little_endian[3] & 0x80) { // msb of [3] is the sign of the mantissa
      signByte = 0xFF;
  }
  mantissa = (int32_t) (value_start_little_endian[1] << 0) |
      (value_start_little_endian[2] << 8) |
      (value_start_little_endian[3] << 16) |
      (signByte << 24) ;
  // value = 10^exponent * mantissa, pow() returns a double type
  return (int32_t) (pow(10, exponent) * mantissa);
} // FLOAT_TO_INT32

#endif

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
  static bool button0_pressed = false;

#if(DEVICE_IS_BLE_SERVER == 1)
  uint8_t button_pressed = 0x01;
  uint8_t button_released = 0x00;
#endif

#if(DEVICE_IS_BLE_SERVER == 0)
  static bool button1_pressed = false;
  static uint8_t cnt = 1;
#endif

  switch (SL_BT_MSG_ID(evt->header))                                                                           //Switching data on the header information
  {
#if (DEVICE_IS_BLE_SERVER == 1)

    case sl_bt_evt_system_boot_id:
      displayInit();

      displayPrintf(DISPLAY_ROW_NAME, BLE_DEVICE_TYPE_STRING);
      displayPrintf(DISPLAY_ROW_ASSIGNMENT, "A9");

      error_status =  sl_bt_system_get_identity_address(&ble_data.myAddress, &ble_data.myAddressType);        //On booting event, get the device address
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Booting Error\r\n");

      displayPrintf(DISPLAY_ROW_BTADDR,"%02x:%02x:%02x:%02x:%02x:%02x",ble_data.myAddress.addr[5], ble_data.myAddress.addr[4], ble_data.myAddress.addr[3], ble_data.myAddress.addr[2] , ble_data.myAddress.addr[1], ble_data.myAddress.addr[0]);

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

      uint8_t button_default = 0;

      error_status = sl_bt_gatt_server_write_attribute_value(gattdb_button_state, 0, 1, &button_default);
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nButton Attribute Value Write Error\r\n");

      error_status = sl_bt_sm_delete_bondings();
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nError Deleting bonding\r\n");
      else
        ble_data.is_bonded = false;

      //      error_status = sl_bt_sm_configure(BONDING_FLAG, sm_io_capability_displayyesno);
      //      if(error_status != SL_STATUS_OK)
      //        LOG_ERROR("\r\nBonding Error\r\n");

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

      displayPrintf(DISPLAY_ROW_9, " ");

      error_status = sl_bt_advertiser_stop(ble_data.advertisingSetHandle);                    //Stop the advertising since a new connection is found
      ble_data.is_connection = true;
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Advertising Stop Error\r\n");


      error_status = sl_bt_connection_set_parameters(ble_data.connectionSetHandle, MIN_CONNECTION_TIME , MAX_CONNECTION_TIME, SLAVE_LATENCY, CONNECTION_TIMEOUT, MIN_CE_LENGTH, MAX_CE_LENGTH);      //Set timing parameters for the new connection
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Advertising Connection Setup Error\r\n");

      error_status = sl_bt_sm_configure(BONDING_FLAG, sm_io_capability_displayyesno);
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBonding Error\r\n");

      error_status = sl_bt_sm_delete_bondings();
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nError Deleting bonding\r\n");
      else
        ble_data.is_bonded = false;

      error_status = sl_bt_system_set_soft_timer(TICKS_PER_125_MS , CB_TIMER_HANDLE , REPEATING_BUFFER);     //1 Hz timer, 1 second timer, generates event sl_bt_system_set_soft_timer_id
      if (error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nSoft Timer Error\r\n");

      break;

    case sl_bt_evt_connection_closed_id:
      error_status = sl_bt_advertiser_start(ble_data.advertisingSetHandle , sl_bt_advertiser_general_discoverable, sl_bt_advertiser_connectable_scannable);    //When connection is closed, start advertising again
      //      LOG_INFO("\r\nConnection: %d closed due to: %d\r\n", evt->data.evt_connection_closed.connection, evt->data.evt_connection_closed.reason);
      ble_data.is_connection = false;
      ble_data.is_htm_indication_enabled = false;
      ble_data.is_custom_indication_enabled = false;
      ble_data.is_htm_indication_in_flight = false;
      displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Advertising Start Error\r\n");

      error_status = sl_bt_sm_delete_bondings();
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nError Deleting bonding\r\n");
      else
        ble_data.is_bonded = false;

      displayPrintf(DISPLAY_ROW_9, " ");
      displayPrintf(DISPLAY_ROW_PASSKEY, " ");
      displayPrintf(DISPLAY_ROW_ACTION, " ");

      gpioLed0SetOff();
      gpioLed1SetOff();
      break;

      //Event to clear the LCD charge every second in repeat mode
    case sl_bt_evt_system_soft_timer_id:

      if(evt->data.evt_system_soft_timer.handle == CB_TIMER_HANDLE)
        {
          uint16_t char_handle;
          uint8_t data[5];
          size_t data_len;
          if((get_queue_depth() != 0) && (ble_data.is_htm_indication_in_flight == false))
            {
              read_queue(&char_handle, &data_len, data);
              //              if(char_handle == gattdb_temperature_measurement)
              //                {
              //                  error_status = sl_bt_gatt_server_send_indication(ble_data.connectionSetHandle, char_handle , data_len , &data[0]);
              //                  if(error_status != SL_STATUS_OK)
              //                    LOG_ERROR("\r\nError sending indication\r\n");
              //                  else
              //                    ble_data.is_htm_indication_in_flight = true;
              //                }
              //              else if(char_handle == gattdb_button_state)
              //                {
              //                  error_status = sl_bt_gatt_server_send_indication(ble_data.connectionSetHandle, char_handle , data_len , &data[0]);
              //                  if(error_status != SL_STATUS_OK)
              //                    LOG_ERROR("\r\nError sending indication\r\n");
              //                  else
              //                    ble_data.is_htm_indication_in_flight = true;
              //                }
              error_status = sl_bt_gatt_server_send_indication(ble_data.connectionSetHandle, char_handle , data_len , data);
              if(error_status != SL_STATUS_OK)
                LOG_ERROR("\r\nError sending indication\r\n");
              else
                ble_data.is_htm_indication_in_flight = true;
            }
        }

      if(evt->data.evt_system_soft_timer.handle == LCD_TIMER_HANDLE)
        {
          displayUpdate();
        }


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

      if(evt->data.evt_system_external_signal.extsignals == event_EXT_BUTTON0_Interrupt)
        {
          button0_pressed = !(button0_pressed);

          if((button0_pressed == true) && (ble_data.is_connection == true))
            {
              displayPrintf(DISPLAY_ROW_9, "Button Pressed");
              ble_data.button_state = button_pressed;
              error_status = sl_bt_gatt_server_write_attribute_value(gattdb_button_state, 0, 1, &button_pressed);
              if(error_status != SL_STATUS_OK)
                LOG_ERROR("\r\nButton press attribute write error\r\n");

              if ((ble_data.is_bonded == false) && (ble_data.is_connection == true))
                {
                  error_status = sl_bt_sm_passkey_confirm(ble_data.connectionSetHandle , 1);
                  if(error_status != SL_STATUS_OK)
                    LOG_ERROR("\r\nError confirming passkey\r\n");
                }
            }
          else if (button0_pressed == false)
            {
              displayPrintf(DISPLAY_ROW_9, "Button Released");
              ble_data.button_state = button_released;
              error_status = sl_bt_gatt_server_write_attribute_value(gattdb_button_state, 0, 1, &button_released);
              if(error_status != SL_STATUS_OK)
                LOG_ERROR("\r\nButton release attribute write error: %d\r\n", error_status);
            }

          if((ble_data.is_bonded == true) && (ble_data.is_connection == true) && (ble_data.is_custom_indication_enabled == true) && (ble_data.is_htm_indication_in_flight == false))
            {
              error_status = sl_bt_gatt_server_send_indication(ble_data.connectionSetHandle, gattdb_button_state, 1, &ble_data.button_state);
              if(error_status != SL_STATUS_OK)
                LOG_ERROR("\r\nButton indication error: %d\r\n", error_status);
              else
                ble_data.is_htm_indication_in_flight = true;
            }
          else if((ble_data.is_htm_indication_in_flight == true) && (ble_data.is_bonded == true))
            {
              write_queue(gattdb_button_state, 1, &ble_data.button_state);
              //              write_queue(gattdb_button_state, 2, &ble_data.button_state);
            }
        }

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
        {
          ble_data.is_htm_indication_enabled = true;
          gpioLed0SetOn();
        }
      else if((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement) && (evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_server_disable))
        {
          ble_data.is_htm_indication_enabled = false;
          gpioLed0SetOff();
        }


      if ((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement) && (evt->data.evt_gatt_server_characteristic_status.status_flags == sl_bt_gatt_server_confirmation))
        {
          if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_server_confirmation)
            ble_data.is_htm_indication_in_flight = false;                                                                          //If the config flag is 0x02, set the indication in flight to false
        }

      if((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_button_state) && (evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_server_indication)) //Indication enabled flag
        {
          ble_data.is_custom_indication_enabled = true;
          gpioLed1SetOn();
        }

      else if ((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_button_state) && (evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_server_disable))
        {
          ble_data.is_custom_indication_enabled = false;
          gpioLed1SetOff();
        }

      if ((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_button_state) && (evt->data.evt_gatt_server_characteristic_status.status_flags == sl_bt_gatt_server_confirmation))
        {
          if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_server_confirmation)
            ble_data.is_htm_indication_in_flight = false;                                                                          //If the config flag is 0x02, set the indication in flight to false
        }
      break;

    case sl_bt_evt_gatt_server_indication_timeout_id:
      //      LOG_INFO("\r\nBluetooth Client Indication Acknowledgement Time-out\r\n");
      if(ble_data.is_htm_indication_in_flight == true)
        ble_data.is_htm_indication_in_flight = false;

      ble_data.is_connection = false;
      break;

    case sl_bt_evt_sm_confirm_bonding_id:
      error_status = sl_bt_sm_bonding_confirm(ble_data.connectionSetHandle , CONFIRM_BONDING);
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nError setting up default parameters for the connection\r\n");
      break;

    case sl_bt_evt_sm_bonded_id:
      displayPrintf(DISPLAY_ROW_CONNECTION, "Bonded");
      displayPrintf(DISPLAY_ROW_PASSKEY, " ");
      displayPrintf(DISPLAY_ROW_ACTION, " ");
      ble_data.is_bonded = true;
      break;

    case sl_bt_evt_sm_confirm_passkey_id:
      displayPrintf(DISPLAY_ROW_PASSKEY, "%d", evt->data.evt_sm_confirm_passkey.passkey);
      displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");
      break;

    case sl_bt_evt_sm_bonding_failed_id:
      ble_data.is_bonded = false;
      LOG_ERROR("\r\nError bonding\r\n");
      break;

    default:
      break;

#endif

#if (DEVICE_IS_BLE_SERVER ==  0)
    case sl_bt_evt_system_boot_id:
      displayInit();

      displayPrintf(DISPLAY_ROW_NAME, "Client");
      displayPrintf(DISPLAY_ROW_ASSIGNMENT, "A9");
      error_status = sl_bt_scanner_set_mode(PHYSICAL_LAYER_1M , PASSIVE_SCAN);
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nError mode setting scanner mode\r\n");
      error_status = sl_bt_scanner_set_timing(PHYSICAL_LAYER_1M, SCAN_INTERVAL, SCAN_WINDOW);
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nError setting up bluetooth timing parameters\r\n");
      error_status = sl_bt_connection_set_default_parameters(MIN_CONNECTION_TIME, MAX_CONNECTION_TIME, CLIENT_LATENCY , CLIENT_TIMEOUT, MIN_CE_LENGTH, CLIENT_MAX_CE_LENGTH);
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nError setting up default parameters for the connection\r\n");
      error_status = sl_bt_scanner_start(PHYSICAL_LAYER_1M, sl_bt_scanner_discover_observation);
      displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");

      error_status =  sl_bt_system_get_identity_address(&ble_data.myAddress, &ble_data.myAddressType);        //On booting event, get the device address
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBluetooth Booting Error\r\n");

      displayPrintf(DISPLAY_ROW_BTADDR,"%02x:%02x:%02x:%02x:%02x:%02x",ble_data.myAddress.addr[5], ble_data.myAddress.addr[4], ble_data.myAddress.addr[3], ble_data.myAddress.addr[2] , ble_data.myAddress.addr[1], ble_data.myAddress.addr[0]);
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nError starting scanning\r\n");

      error_status = sl_bt_sm_delete_bondings();
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nError Deleting bonding\r\n");
      else
        ble_data.is_bonded = false;
      break;

    case sl_bt_evt_scanner_scan_report_id:
      error_status = sl_bt_scanner_stop();
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nError stopping scanning\r\n");

      error_status = sl_bt_connection_open(SERVER_BT_ADDRESS, 0, PHYSICAL_LAYER_1M, &ble_data.connectionSetHandle);
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nError opening a connection\r\n");

      break;

    case sl_bt_evt_connection_opened_id:

      error_status = sl_bt_sm_configure(BONDING_FLAG, sm_io_capability_displayyesno);
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nBonding Error\r\n");
      break;

    case  sl_bt_evt_gatt_service_id:

      if((evt->data.evt_gatt_service.uuid.data[0] == HTM_SERVICE_UUID[0]) && (evt->data.evt_gatt_service.uuid.len == HTM_SERVICE_UUID_LEN))
        {
          ble_data.htmServiceHandle = evt->data.evt_gatt_service.service;
        }


      else if((evt->data.evt_gatt_service.uuid.data[0] == BUTTON_SERVICE_UUID[0]) && (evt->data.evt_gatt_service.uuid.len == BUTTON_SERVICE_UUID_LEN))
        {
          ble_data.buttonServiceHandle = evt->data.evt_gatt_service.service;
        }

      break;

    case sl_bt_evt_gatt_characteristic_id:
      //      ble_data.htmCharacteristicHandle = evt->data.evt_gatt_characteristic.characteristic;

      if((evt->data.evt_gatt_characteristic.uuid.data[0] == HTM_CHAR_UUID[0]))
        {
          ble_data.htmCharacteristicHandle = evt->data.evt_gatt_characteristic.characteristic;
        }

      else if((evt->data.evt_gatt_characteristic.uuid.data[0] == BUTTON_CHAR_UUID[0]))
        {
          ble_data.buttonCharacteristicHandle = evt->data.evt_gatt_characteristic.characteristic;
        }
      break;

    case sl_bt_evt_system_external_signal_id:

      if(evt->data.evt_system_external_signal.extsignals == event_EXT_BUTTON0_Interrupt)
        {
          button0_pressed = !(button0_pressed);

          if(button0_pressed == true)
            {
              if(ble_data.is_bonded == false)
                {
                  error_status = sl_bt_sm_passkey_confirm(ble_data.connectionSetHandle , 1);
                  if(error_status != SL_STATUS_OK)
                    LOG_ERROR("\r\nError confirming passkey - Client\r\n");
                }
              ble_data.indication_flag = 0x000F;
            }

          else if(button0_pressed == false)
            {
              if(ble_data.indication_flag == 0x0FFF)
                ble_data.indication_flag = 0xFFFF;

              if(((ble_data.indication_flag & 0xFFFF) == 0xFFFF) && (ble_data.is_bonded == true))
                {
                  if(cnt == 1)
                    {
                      error_status = sl_bt_gatt_set_characteristic_notification(ble_data.connectionSetHandle, ble_data.buttonCharacteristicHandle, sl_bt_gatt_disable);
                      if(error_status != SL_STATUS_OK)
                        LOG_ERROR("\r\nError disabling indications: %d\r\n", error_status);
                      cnt = 0;
                    }
                  else
                    {
                      error_status = sl_bt_gatt_set_characteristic_notification(ble_data.connectionSetHandle, ble_data.buttonCharacteristicHandle, sl_bt_gatt_indication);
                      if(error_status != SL_STATUS_OK)
                        LOG_ERROR("\r\nError enabling indications: %d\r\n", error_status);
                      cnt = 1;
                    }
                  ble_data.indication_flag = 0x0000;
                }
            }
        }
      if(evt->data.evt_system_external_signal.extsignals == event_EXT_BUTTON1_Interrupt)
        {
          button1_pressed = !(button1_pressed);

          if(button1_pressed == true)
            {
              if(ble_data.is_bonded == false)
                error_status = sl_bt_gatt_read_characteristic_value(ble_data.connectionSetHandle , ble_data.buttonCharacteristicHandle);

              if((ble_data.is_bonded == true))
                {
                  if(!(ble_data.indication_flag & 0x000F))
                    {
                      error_status = sl_bt_gatt_read_characteristic_value(ble_data.connectionSetHandle  , ble_data.buttonCharacteristicHandle);
                      if(error_status != SL_STATUS_OK)
                        LOG_ERROR("\r\nError reading characteristic value for button state characteristic\r\n");
                    }
                }

              if(ble_data.indication_flag == 0x000F)
                ble_data.indication_flag = 0x00FF;
            }

          else if(button1_pressed == false)
            {
              if(ble_data.indication_flag == 0x00FF)
                ble_data.indication_flag = 0x0FFF;
            }
        }


      break;

      //Event to clear the LCD charge every second in repeat mode
    case sl_bt_evt_system_soft_timer_id:
      if(evt->data.evt_system_soft_timer.handle == LCD_TIMER_HANDLE)
        displayUpdate();
      break;

    case sl_bt_evt_gatt_procedure_completed_id:

      if((evt->data.evt_gatt_procedure_completed.result == SL_STATUS_BT_ATT_INSUFFICIENT_ENCRYPTION))
        {
          error_status = sl_bt_sm_increase_security(ble_data.connectionSetHandle);
          if(error_status != SL_STATUS_OK)
            LOG_ERROR("\r\nError increasing security\r\n");
        }
      break;

    case sl_bt_evt_gatt_characteristic_value_id:

      if((evt->data.evt_gatt_characteristic_value.characteristic == gattdb_button_state))
        {
          if(evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_handle_value_indication)
            {
              error_status = sl_bt_gatt_send_characteristic_confirmation(ble_data.connectionSetHandle);
              if(error_status != SL_STATUS_OK)
                LOG_ERROR("\r\nError fetching characteristic notification confirmation: Button : %d\r\n", error_status);

              if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0)
                displayPrintf(DISPLAY_ROW_9, "Button Released");

              else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 1)
                displayPrintf(DISPLAY_ROW_9, "Button Pressed");

            }

          if(evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_read_response)
            {
              if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0)
                displayPrintf(DISPLAY_ROW_9, "Button Released");

              else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 1)
                displayPrintf(DISPLAY_ROW_9, "Button Pressed");
            }

        }
      if((evt->data.evt_gatt_characteristic_value.characteristic == gattdb_temperature_measurement))
        {
          error_status = sl_bt_gatt_send_characteristic_confirmation(ble_data.connectionSetHandle);
          if(error_status != SL_STATUS_OK)
            LOG_ERROR("\r\nError fetching characteristic notification confirmation: Temperauture: %d\r\n", error_status);

          ble_data.temp_value = FLOAT_TO_INT32(evt->data.evt_gatt_characteristic_value.value.data);
          displayPrintf(DISPLAY_ROW_TEMPVALUE, "Temp=%d C", ble_data.temp_value);
        }
      break;

    case sl_bt_evt_connection_closed_id:

      error_status = sl_bt_sm_delete_bondings();
      if(error_status != SL_STATUS_OK)
        LOG_ERROR("\r\nError Deleting bonding\r\n");
      else
        ble_data.is_bonded = false;
      displayPrintf(DISPLAY_ROW_PASSKEY, " ");
      displayPrintf(DISPLAY_ROW_ACTION, " ");
      break;

    case sl_bt_evt_sm_bonded_id:
      displayPrintf(DISPLAY_ROW_CONNECTION, "Bonded");
      displayPrintf(DISPLAY_ROW_PASSKEY, " ");
      displayPrintf(DISPLAY_ROW_ACTION, " ");
      ble_data.is_bonded = true;
      break;

    case sl_bt_evt_sm_confirm_passkey_id:
      displayPrintf(DISPLAY_ROW_PASSKEY, "%d", evt->data.evt_sm_confirm_passkey.passkey);
      displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");
      break;

    case sl_bt_evt_sm_bonding_failed_id:
      ble_data.is_bonded = false;
      LOG_ERROR("\r\nError bonding\r\n");
      break;

#endif
  }
}
