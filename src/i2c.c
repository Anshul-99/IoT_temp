/**
 * @file    :   i2c.c
 * @brief   :   API for I2C protocol for temperature sensor
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   10 February 2022
 *
 */

#include <stdbool.h>
#include "sl_i2cspm.h"
#include "src/i2c.h"
#include "src/gpio.h"
#include "src/timers.h"
#include "src/scheduler.h"
#include "src/ble.h"
#include "gatt_db.h"


#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

//Device address for temperature sensor
#define SI7021_TEMP_SENSOR_ADDR (0x40)

//Temperature measurement sequence
#define SI7021_TEMP_SENSOR_SEQ (0xF3)

typedef uint8_t temp_data_t;

//Global buffer for I2C read
temp_data_t read_data[2];

//Global variable for sending command over I2C
temp_data_t cmd_data;

I2C_TransferSeq_TypeDef i2c_temp_sensor_transfer;


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
 *   None
 *
 * Returns:
 *   None
 */
void sendI2C_command()
{
  I2C_TransferReturn_TypeDef i2c_temp_sensor_transfer_result;

  if(SUCCESS != i2c_temp_init())
    LOG_ERROR("\r\nI2C Init Failed\r\n");

  cmd_data = SI7021_TEMP_SENSOR_SEQ;
  i2c_temp_sensor_transfer.addr = (SI7021_TEMP_SENSOR_ADDR << 1);
  i2c_temp_sensor_transfer.flags = I2C_FLAG_WRITE;
  i2c_temp_sensor_transfer.buf[0].data = &cmd_data;
  i2c_temp_sensor_transfer.buf[0].len = sizeof(cmd_data);

  NVIC_EnableIRQ(I2C0_IRQn);                                                                       //Enable I2C0 interrupts

  i2c_temp_sensor_transfer_result = I2C_TransferInit(I2C0, &i2c_temp_sensor_transfer);             //Initialize I2C0 Transfer

  if (i2c_temp_sensor_transfer_result < 0)
    LOG_ERROR("\r\nI2C_TransferInit() Write error = %d\r\n", i2c_temp_sensor_transfer_result);

}


/*
 * I2C read command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void receiveI2C_command()
{
  I2C_TransferReturn_TypeDef i2c_temp_sensor_transfer_result;

  cmd_data = SI7021_TEMP_SENSOR_SEQ;
  i2c_temp_sensor_transfer.addr = (SI7021_TEMP_SENSOR_ADDR << 1);
  i2c_temp_sensor_transfer.flags = I2C_FLAG_READ;
  i2c_temp_sensor_transfer.buf[0].data = read_data;
  i2c_temp_sensor_transfer.buf[0].len = sizeof(read_data);

  NVIC_EnableIRQ(I2C0_IRQn);                                                                  //Enable I2C0 interrupts

  i2c_temp_sensor_transfer_result = I2C_TransferInit(I2C0, &i2c_temp_sensor_transfer);        //Initialize I2C0 Transfer

  if (i2c_temp_sensor_transfer_result < 0)
    LOG_ERROR("I2C_TransferInit() Read error = %d\r\n", i2c_temp_sensor_transfer_result);
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
uint32_t getTempReadings()
{

  uint32_t final_temp_read = 0;

  uint32_t temp_total = (read_data[0] << 8) | (read_data[1]);            //Concatenate the temperature data into one 16 bit variable

  final_temp_read = (((175.72 * temp_total)/65536) - 46.85);             //Get actual temperature from raw data

  LOG_INFO("\r\nTemperature in degC: %d\r\n", (int)final_temp_read);   //Output the temperature data along with time-stamp on serial console

  return (uint32_t)(final_temp_read);
}

