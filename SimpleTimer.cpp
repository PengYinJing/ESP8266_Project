/*
 * SimpleTimer.cpp
 *
 * SimpleTimer - A timer library for Arduino.
 * Author: mromani@ottotecnica.com
 * Copyright (c) 2010 OTTOTECNICA Italy
 *
 * This library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser
 * General Public License as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser
 * General Public License along with this library; if not,
 * write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */


#include "SimpleTimer.h"

// Select time function:
static inline long elapsed() { return millis(); }

SimpleTimer::SimpleTimer() {
    long current_millis = elapsed();

    for (int i = 0; i < MAX_TIMERS; i++) {
        enabled[i] = false;
        callbacks[i] = 0;                   // if the callback pointer is zero, the slot is free, i.e. doesn't "contain" any timer
        prev_millis[i] = current_millis;
        numRuns[i] = 0;
    }

    numTimers = 0;
}


void SimpleTimer::run() {
    int i;
    long current_millis = elapsed();
    
    for (i = 0; i < MAX_TIMERS; i++)
    {
        toBeCalled[i] = DEFCALL_DONTRUN;
        if (callbacks[i])
        {
            if (current_millis - prev_millis[i] >= delays[i])
            {
                prev_millis[i] = current_millis;
                //prev_millis[i] += delays[i];

                if (enabled[i])
                {
                    if (maxNumRuns[i] == RUN_FOREVER)
                        toBeCalled[i] = DEFCALL_RUNONLY;
                    else if (numRuns[i] < maxNumRuns[i])
                    {
                        toBeCalled[i] = DEFCALL_RUNONLY;
                        numRuns[i]++;

                        // after the last run, delete the timer
                        if (numRuns[i] >= maxNumRuns[i])
                            toBeCalled[i] = DEFCALL_RUNANDDEL;
                    }
                }
            }
        }
    }

    for (i = 0; i < MAX_TIMERS; i++)
    {
        switch (toBeCalled[i])
        {
        case DEFCALL_DONTRUN:
            break;

        case DEFCALL_RUNONLY:
            (*callbacks[i])();
            break;

        case DEFCALL_RUNANDDEL:
            (*callbacks[i])();
            deleteTimer(i);
            break;
        }
    }
}


// find the first available slot
// return -1 if none found
int SimpleTimer::findFirstFreeSlot() {
    int i;

    // all slots are used
    if (numTimers >= MAX_TIMERS)
        return -1;

    // return the first slot with no callback (i.e. free)
    for (i = 0; i < MAX_TIMERS; i++)
    {
        if (callbacks[i] == 0)
            return i;
    }

    // no free slots found
    return -1;
}


int SimpleTimer::setTimer(long initial_delay, long interval_ms, timer_callback func, int n) {
    int freeTimer;

    freeTimer = findFirstFreeSlot();
    if (freeTimer < 0)
        return -1;

    if (func == NULL)
        return -1;

    delays[freeTimer] = interval_ms;
    callbacks[freeTimer] = func;
    maxNumRuns[freeTimer] = n;
    enabled[freeTimer] = true;
    prev_millis[freeTimer] = elapsed() + initial_delay;

    numTimers++;

    return freeTimer;
}

int SimpleTimer::setInterval(long initial_delay, long interval_ms, timer_callback func)
{
    return setTimer(initial_delay, interval_ms, func, RUN_FOREVER);
}


int SimpleTimer::setTimeout(long interval_ms, timer_callback func)
{
    return setTimer(0, interval_ms, func, RUN_ONCE);
}


void SimpleTimer::deleteTimer(int timerId)
{
    if (timerId >= MAX_TIMERS)
        return;

    // nothing to delete if no timers are in use
    if (numTimers == 0)
        return;

    // don't decrease the number of timers if the
    // specified slot is already empty
    if (callbacks[timerId] != NULL)
    {
        callbacks[timerId] = 0;
        enabled[timerId] = false;
        toBeCalled[timerId] = DEFCALL_DONTRUN;
        delays[timerId] = 0;
        numRuns[timerId] = 0;

        // update number of timers
        numTimers--;
    }
}


// function contributed by code@rowansimms.com
void SimpleTimer::restartTimer(int numTimer)
{
    if (numTimer >= MAX_TIMERS)
        return;
    prev_millis[numTimer] = elapsed();
}


boolean SimpleTimer::isEnabled(int numTimer)
{
    if (numTimer >= MAX_TIMERS)
        return false;
    return enabled[numTimer];
}


void SimpleTimer::enable(int numTimer)
{
    if (numTimer >= MAX_TIMERS)
        return;
    enabled[numTimer] = true;
}


void SimpleTimer::disable(int numTimer)
{
    if (numTimer >= MAX_TIMERS)
        return;
    enabled[numTimer] = false;
}


void SimpleTimer::toggle(int numTimer)
{
    if (numTimer >= MAX_TIMERS)
        return;
    enabled[numTimer] = !enabled[numTimer];
}


int SimpleTimer::getNumTimers()
{
    return numTimers;
}

