/**
 * @file    :   irq.h
 * @brief   :   Header declarations file for LETIMER IRQ
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   28 January 2022
 *
 */

#ifndef IRQ_H
#define IRQ_H

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

#endif      //IRQ_H