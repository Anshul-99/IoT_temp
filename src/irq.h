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

uint32_t letimerMilliseconds();

void set_letimer_event();

#endif      //IRQ_H
