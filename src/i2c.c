/**
 * @file    :   i2c.c
 * @brief   :   API for I2C protocol for temperature sensor
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   2 February 2022
 *
 */

#include <stdbool.h>
#include "sl_i2cspm.h"
#include "src/i2c.h"
#include "src/gpio.h"
#include "src/timers.h"

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

//Device address for temperature sensor
#define SI7021_TEMP_SENSOR_ADDR (0x40)

//Temperature measurement sequence
#define SI7021_TEMP_SENSOR_SEQ (0xF3)


typedef uint8_t temp_data_t;

//Global buffer for I2C read
temp_data_t read_data[2];

I2C_TransferSeq_TypeDef i2c_temp_sensor_transfer;
I2C_TransferReturn_TypeDef i2c_temp_sensor_transfer_result;


/*
 * Initializes the I2C0 peripheral
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   Error code
 */
error_t i2c_temp_init()
{
  error_t ret = SUCCESS;

  I2CSPM_Init_TypeDef i2c_temp_sensor =
      {
          .port = I2C0,
          .sclPort = gpioPortC,
          .sclPin = SCL_PIN,
          .sdaPort = gpioPortC,
          .sdaPin = SDA_PIN,
          .portLocationScl = SCL_POSITION,
          .portLocationSda = SDA_POSITION,
          .i2cRefFreq  = 0,
          .i2cMaxFreq = I2C_FREQ_STANDARD_MAX,
          .i2cClhr = i2cClockHLRStandard
      };
  I2CSPM_Init(&i2c_temp_sensor);    //Initializing I2C0 peripheral

  I2C_Enable(I2C0, true);           //Enable I2C0 peripheral

  return ret;
}


/*
 * I2C write command
 *
 * Parameters:
 *   uint8_t write_data: Data/Command to be sent
 *
 * Returns:
 *   Error code
 */
error_t i2cWrite(uint8_t write_data)
{
  if(SUCCESS != i2c_temp_init())
    return I2C_ERROR;

  i2c_temp_sensor_transfer.addr = (SI7021_TEMP_SENSOR_ADDR << 1);
  i2c_temp_sensor_transfer.flags = I2C_FLAG_WRITE;
  i2c_temp_sensor_transfer.buf[0].data = &write_data;
  i2c_temp_sensor_transfer.buf[0].len = sizeof(write_data);

  i2c_temp_sensor_transfer_result =  I2CSPM_Transfer(I2C0, &i2c_temp_sensor_transfer);

  if(i2c_temp_sensor_transfer_result != i2cTransferDone)
    {
      LOG_ERROR("I2CSPM_Transfer: I2C bus write failed");
      return I2C_WRITE_ERROR;
    }


  else
      return SUCCESS;
}


/*
 * I2C read command
 *
 * Parameters:
 *   int no_bytes: Number of bytes to be read
 *
 * Returns:
 *   Error code
 */
error_t i2cRead(int no_bytes)
{
  i2c_temp_sensor_transfer.addr = (SI7021_TEMP_SENSOR_ADDR << 1);
  i2c_temp_sensor_transfer.flags = I2C_FLAG_READ;
  i2c_temp_sensor_transfer.buf[0].data = read_data ;
  i2c_temp_sensor_transfer.buf[0].len = no_bytes;

  i2c_temp_sensor_transfer_result =  I2CSPM_Transfer(I2C0, &i2c_temp_sensor_transfer);

  if(i2c_temp_sensor_transfer_result != i2cTransferDone)
    {
      LOG_ERROR("I2CSPM_Transfer: I2C bus read failed");
      return I2C_READ_ERROR;
    }
  else
      return SUCCESS;
}


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
void loadpowerTempSensor(bool val)
{
  if(val == true)
    {
      i2c_gpioInit();          //Initialize the sensor enable pin

      sensorEnable();          //Enable the sensor

      time_delay(80000);       //Wait for 80 ms [Setup time]
    }

  if (val == false)
    {
      i2c_gpioDeInit();        //Disable the GPIO for I2C0

      sensorDisable();         //Clear the sensor enable pin
    }
}

/*
 * Temperature measurement
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void getTempReadings()
{
  float final_temp_read = 0;

  loadpowerTempSensor(true);                                              //Power ON the sensor

  if (SUCCESS != i2cWrite(SI7021_TEMP_SENSOR_SEQ))                        //Send temperature measurement command to sensor
    LOG_ERROR("\r\nI2C write command error: Temperature measurement command sequence\r\n");

  time_delay(10000);                                                      //Wait for 10 ms for master to write to the slaves

  if (SUCCESS != i2cRead(2))                                              //Read 2 bytes of temperature data from the sensor
    LOG_ERROR("\r\nI2C read command error: Temperature measurement command sequence\r\n");

  loadpowerTempSensor(false);                                            //Power OFF the sensor

  uint16_t temp_total = (read_data[0] << 8) | (read_data[1]);            //Concatenate the temperature data into one 16 bit variable

  final_temp_read = (((175.72 * temp_total)/65536) - 46.85);             //Get actual temperature from raw data

  LOG_INFO("\r\nTemperature in degree Celsius is %d\r\n", (int)final_temp_read);

}

