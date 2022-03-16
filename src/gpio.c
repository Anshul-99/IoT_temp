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
#define LED1_port  (gpioPortF) // Port F for LED 1
#define LED1_pin   (5)         //Pin 5 is connected to LED 1
#define EXTCOMIN_PORT (gpioPortD)
#define EXTCOMIN_PIN (13)
#define EXT_BUTTON_PORT (gpioPortF)
#define EXT_BUTTON_PIN (6)
#define EXT_BUTTON_1_PORT (gpioPortF)
#define EXT_BUTTON_1_PIN (7)


#include "gpio.h"
#include <app.h>


// Set GPIO drive strengths and modes of operation
void gpioInit()
{
  GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);

  GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, false);

  GPIO_DriveStrengthSet(EXTCOMIN_PORT, gpioDriveStrengthStrongAlternateStrong);
  GPIO_PinModeSet(EXTCOMIN_PORT, EXTCOMIN_PIN, gpioModePushPull, false);

  GPIO_PinModeSet(EXT_BUTTON_PORT, EXT_BUTTON_PIN, gpioModeInputPullFilter, 1);

  GPIO_IntDisable(EXT_BUTTON_PIN);

  GPIO_ExtIntConfig(EXT_BUTTON_PORT, EXT_BUTTON_PIN ,EXT_BUTTON_PIN , true, true, true);

  GPIO_PinModeSet(EXT_BUTTON_1_PORT, EXT_BUTTON_1_PIN, gpioModeInputPullFilter, 1);

  GPIO_IntDisable(EXT_BUTTON_1_PIN);

  GPIO_ExtIntConfig(EXT_BUTTON_1_PORT, EXT_BUTTON_1_PIN ,EXT_BUTTON_1_PIN , true, true, true);

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
 * Turns LED1 on
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   None
 */
void gpioLed1SetOn()
{
  GPIO_PinOutSet(LED1_port,LED1_pin);
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
 * Turns LED1 off
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   None
 */
void gpioLed1SetOff()
{
  GPIO_PinOutClear(LED1_port,LED1_pin);
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
void gpioSensorEnSetOn()
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


/*Toggles the EXTCOMIN pin tyied to the LCD to avoid crystal charge accumulation
 *
 * Parameters:
 *  bool enable: Decides whether to set or clear the EXTCOMIN pin
 *
 * Returns:
 *   None
 */
void extcomin_enable(bool enable)
{
  if(enable)
    GPIO_PinOutSet(EXTCOMIN_PORT, EXTCOMIN_PIN);
  else
    GPIO_PinOutClear(EXTCOMIN_PORT, EXTCOMIN_PIN);
}
