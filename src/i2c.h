/**
 * @file    :   i2c.h
 * @brief   :   Headers and function definitions for I2C protocol for temperature sensor
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   10 February 2022
 *
 */

#ifndef I2Q_H
#define I2Q_H

#include "stdbool.h"


typedef enum
{
  SUCCESS = 0,
  I2C_INIT_ERROR = -1,
  I2C_WRITE_ERROR = -2,
  I2C_READ_ERROR = -3,
  I2C_ERROR = -4
}error_t;


/*
 * Initializes the I2C0 peripheral
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   Error code
 */
error_t i2c_temp_init();


/*
 * I2C read command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void receiveI2C_command();


/*
 * I2C write command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void sendI2C_command();


/*
 * Enables/Disables the I2C0 peripheral
 *
 * Parameters:
 *   bool val: true: Enables the I2C0 peripheral
 *             false: Disables the I2C0 peripheral
 *
 * Returns:
 *   None
 */
void loadpowerTempSensor(bool val);


/*
 * Enables/Disables the I2C0 peripheral
 *
 * Parameters:
 *   bool val: true: Enables the I2C0 peripheral
 *             false: Disables the I2C0 peripheral
 *
 * Returns:
 *   None
 */
uint32_t getTempReadings();

#endif  //I2Q_H
