/**
 * @file    :   ble.h
 * @brief   :   Headers and function definitions for Bluetooth stack events handling
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   16 February 2022
 *
 */

#ifndef BLE_H
#define BLE_H

#include "sl_bt_api.h"
#include "stdbool.h"


#define UINT8_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); }

#define UINT32_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); *(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24); }

#define UINT32_TO_FLOAT(m, e) (((uint32_t)(m) & 0x00FFFFFFU) | (uint32_t)((int32_t)(e) << 24))


//Private data structure to store the connection attributes
typedef struct
{
  // values that are common to servers and clients
  bd_addr myAddress;
  uint8_t myAddressType;

  // values unique for server
  uint8_t advertisingSetHandle;

  // values unique for client
  uint8_t connectionSetHandle;
  bool is_connection;
  bool is_indication_in_flight;
  bool is_indication_enabled;
} ble_data_struct_t;

/*
 * Gives an instance of the private BLE data structure
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   ble_data_struct_t: Returns an instance of the structure
 */
ble_data_struct_t* getBleDataPtr();


/*
 * Event responder for Bluetooth events
 *
 * Parameters:
 *   sl_bt_msg_t event: Bluetooth events
 *
 * Returns:
 *   None
 */
void handle_ble_event(sl_bt_msg_t *evt);


#endif  //BLE_H
