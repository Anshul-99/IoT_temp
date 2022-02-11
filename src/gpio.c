/*
  gpio.c

   Created on: Dec 12, 2018
       Author: Dan Walkes
   Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

   March 17
   Dave Sluiter: Use this file to define functions that set up or control GPIOs.

 */
// *****************************************************************************
// Students:
// We will be creating additional functions that configure and manipulate GPIOs.
// For any new GPIO function you create, place that function in this file.
// *****************************************************************************

#include <stdbool.h>
#include "em_gpio.h"
#include <string.h>


#define LED0_port  (gpioPortF) // Port F for LED 0
#define LED0_pin   (4)         //Pin 4 is connected to LED 0


#include "gpio.h"
#include <app.h>


// Set GPIO drive strengths and modes of operation
void gpioInit()
{
  GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);

} // gpioInit()

/*
 * Turns LED0 on
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   None
 */
void gpioLed0SetOn()
{
  GPIO_PinOutSet(LED0_port,LED0_pin);
}

/*
 * Turns LED0 off
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   None
 */
void gpioLed0SetOff()
{
  GPIO_PinOutClear(LED0_port,LED0_pin);
}

/*
 * Initializes the GPIO for I2C0 peripheral
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   None
 */
void i2c_gpioInit()
{
  GPIO_PinModeSet(gpioPortD, SENSOR_ENABLE_PIN, gpioModePushPull, 1);             //Sensor enable pin[Push Pull mode]
  GPIO_DriveStrengthSet(gpioPortD, gpioDriveStrengthWeakAlternateWeak);
}


/*
 * Deinitializes the GPIO for I2C0 peripheral
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   None
 */
void i2c_gpioDeInit()
{
  GPIO_PinModeSet(gpioPortC, SCL_PIN, gpioModeDisabled, 1);
  GPIO_PinModeSet(gpioPortC, SDA_PIN, gpioModeDisabled, 1);
}


/*
 * Enables the temperature sensor
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   None
 */
void sensorEnable()
{
  GPIO_PinOutSet(gpioPortD, SENSOR_ENABLE_PIN);
}


/*
 * Enables the temperature sensor
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   None
 */
void sensorDisable()
{
  GPIO_PinOutClear(gpioPortD, SENSOR_ENABLE_PIN);
}



