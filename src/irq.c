/**
 * @file    :   irq.c
 * @brief   :   API for LETIMER IRQ
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   2 February 2022
 *
 */

#include "em_letimer.h"
#include "em_i2c.h"
#include "stdint.h"
#include "gpio.h"
#include "scheduler.h"
#include "stdint.h"
#include "app.h"

static uint32_t log_time = 0;

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#define UNDERFLOW_INT_BIT (4)
/*
 * Initializes the IRQ in the NVIC for the LETIMER0 peripheral
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void letimer_irq_init()
{
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);            //Enable the underflow flag for the LETIMER0 peripheral
  int int_bit = LETIMER0-> IEN;

   if(int_bit != UNDERFLOW_INT_BIT)                                       //Check for if the interrupt bit got set
     {
       LOG_ERROR("\r\nUnderflow interrupt not enabled\r\n");
       LETIMER0 -> IEN |= UNDERFLOW_INT_BIT;

       if(LETIMER0 -> IEN != UNDERFLOW_INT_BIT)
         {
           LOG_ERROR("\r\nASSERT: Interrupt not enabled\r\n");
//           __BKPT(0);
         }
     }

  NVIC_ClearPendingIRQ(LETIMER0_IRQn);                     //Clear pending LETIMER0 interrupts

  NVIC_EnableIRQ(LETIMER0_IRQn);                           //Enable the LETIMER0 interrupt
}

/*
 * ISR for the LETIMER0 peripheral
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void LETIMER0_IRQHandler(void)
{
  uint32_t int_flags = 0;
  int int_bit = 0;;

  int_flags = LETIMER_IntGet(LETIMER0);                 //Get the raised interrupt flag

  if (int_flags & LETIMER_IFC_UF)                       //At underflow event, set temperature measurement event
    {
      LETIMER_IntClear(LETIMER0, LETIMER_IFC_UF);      //Clear the set interrupt flag
      int_bit = LETIMER0-> IFC;
      if(int_bit != 0)                                //Check if the bit got cleared
           {
             LOG_ERROR("\r\nUnderflow interrupt not cleared\r\n");
             LETIMER0 -> IFC = LETIMER_IFC_UF;

             if(LETIMER0 -> IFC != 0)
               {
                 LOG_ERROR("\r\nASSERT: Interrupt not cleared\r\n");
//                 __BKPT(0);
               }
           }
      setSchedulerEventTemp();
      log_time += LETIMER_PERIOD_MS;
    }

  else if (int_flags & LETIMER_IFC_COMP1)
    {
      LETIMER_IntClear(LETIMER0, LETIMER_IFC_COMP1);   //Clear the set interrupt flag
      int_bit = LETIMER0-> IFC;
            if(int_bit != 0)                          //Check if the bit got cleared
                 {
                   LOG_ERROR("\r\nComp1 interrupt not cleared\r\n");
                   LETIMER0 -> IFC = LETIMER_IFC_COMP1;

                   if(LETIMER0 -> IFC != 0)
                     {
                       LOG_ERROR("\r\nASSERT: Interrupt not cleared\r\n");
//                       __BKPT(0);
                     }
                 }
      setSchedulerEventDelay();                           //At COMP1 event, set time delay complete event
      LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);    //Disable the COMP1 interrupt till the time_delay function is not called again
    }

}


/*
 * ISR for the I2C0 peripheral
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void I2C0_IRQHandler()
{
  I2C_TransferReturn_TypeDef i2c_temp_sensor_transfer_result;

  i2c_temp_sensor_transfer_result = I2C_Transfer(I2C0);                //Transfer the I2C sequence

  if (i2c_temp_sensor_transfer_result == i2cTransferDone)             //If the transfer is successful, set the transfer complete event
    setSchedulerEventTransferComplete();

  if (i2c_temp_sensor_transfer_result < 0)
    LOG_ERROR("\r\n%d\r\n", i2c_temp_sensor_transfer_result);
}


/*
 * Returns time elapsed in milliseconds
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   uint32_t time_elapsed: Time elapsed in milliseconds with a resolution of 3000 milliseconds
 */
uint32_t letimerMilliseconds()
{
   return log_time;
}
