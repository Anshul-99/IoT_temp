/**
 * @file    :   timers.h
 * @brief   :   Header declarations file for LETIMER peripheral
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   10 February 2022
 *
 */
#ifndef TIMERS_H
#define TIMERS_H

#define PRESCALAR_VALUE (4)

/*
 * Initializes the LETIMER0
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void letimer_init();


/*
 * Hard spin time delay loop using LETIMER ticks as reference
 *
 * Parameters:
 *   uint32_t time_us: Time in micro-seconds
 *
 * Returns:
 *   None
 */
void timerWaitUs_polled(uint32_t time_us);


/*
 * Interrupt-based time delay using LETIMER Comp1 interrupts
 *
 * Parameters:
 *   uint32_t time_us: Time in micro-seconds
 *
 * Returns:
 *   None
 */
void timerWaitUs_irq(uint32_t time_us);


#endif     //TIMERS_H
