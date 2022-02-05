/**
 * @file    :   scheduler.h
 * @brief   :   Headers and function definitions for event scheduler
 *
 * @author  :   Khyati Satta [khyati.satta@colorado.edu]
 * @date    :   2 February 2022
 *
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H


//Event enum
typedef enum
{
  event_DEFUALT = 0,
  event_LETIMER0_UF = 1,
}schedulerEvents;


/*
 * Sets an event when interrupt is triggered
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 */
void setSchedulerEventTemp();


/*
 * Returns the current event triggered.
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   uint32_t : Event triggered
 */
uint32_t getCurrentEvent();

#endif  //SCHEDULER_H

