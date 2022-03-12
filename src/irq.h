/**
 * @file    :   irq.h
 * @brief   :   Header declarations file for LETIMER IRQ
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   10 February 2022
 *
 */

#ifndef IRQ_H
#define IRQ_H

#include "stdint.h"

/*
 * Initializes the IRQ in the NVIC for the LETIMER0 peripheral
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void letimer_irq_init();


/*
 * Returns time elapsed in milliseconds
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   uint32_t time_elapsed: Time elapsed in milliseconds with a resolution of 3000 milliseconds
 */
uint32_t letimerMilliseconds();

void set_letimer_event();


/*
 * Initializes the IRQ in the NVIC for the external button peripheral
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void gpio_ext_init();

#endif      //IRQ_H
