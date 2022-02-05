/**
 * @file    :   i2c.h
 * @brief   :   Headers and function definitions for I2C protocol for temperature sensor
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   2 February 2022
 *
 */

#ifndef I2Q_H
#define I2Q_H

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
 * I2C write command
 *
 * Parameters:
 *   uint8_t write_data: Data/Command to be sent
 *
 * Returns:
 *   Error code
 */
error_t i2cWrite(uint8_t write_data);


/*
 * I2C read command
 *
 * Parameters:
 *   int no_bytes: Number of bytes to be read
 *
 * Returns:
 *   Error code
 */
error_t i2cRead(int no_bytes);


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
void getTempReadings();

#endif  //I2Q_H
