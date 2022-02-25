/*
   gpio.h
  
    Created on: Dec 12, 2018
        Author: Dan Walkes

    Updated by Dave Sluiter Sept 7, 2020. moved #defines from .c to .h file.
    Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_



#define SCL_PIN (10)
#define SDA_PIN (11)
#define SENSOR_ENABLE_PIN (15)
#define SCL_POSITION (14)
#define SDA_POSITION (16)





// Function prototypes
void gpioInit();
void gpioLed0SetOn();
void gpioLed0SetOff();
void gpioLed1SetOn();
void gpioLed1SetOff();
void i2c_gpioInit();
void i2c_gpioDeInit();
void gpioSensorEnSetOn();
void sensorDisable();
void extcomin_enable(bool enable);





#endif /* SRC_GPIO_H_ */
