/*
 *  Copyright 2009-2010 NVIDIA Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
 
#pragma once
#include "base/Defs.hpp"

namespace FW
{
//------------------------------------------------------------------------

class Timer
{
public:
    explicit        Timer           (bool started = false)  : m_startTicks(-1), m_totalTicks(0) { if (started) start(); }
                    Timer           (const Timer& other)    { operator=(other); }
                    ~Timer          (void)                  {}

    Timer&          operator=       (const Timer& other)    { m_startTicks = other.m_startTicks; m_totalTicks = other.m_totalTicks; return *this; }

    void            start           (void)                  { m_startTicks = queryTicks(); }
    void            unstart         (void)                  { m_startTicks = -1; }
    F32             getElapsed      (void)                  { return ticksToSecs(getElapsedTicks()); }

    F32             end             (void);                 // return elapsed, total += elapsed, restart
    F32             getTotal        (void) const            { return ticksToSecs(m_totalTicks); }
    void            clearTotal      (void)                  { m_totalTicks = 0; }

private:
    static S64      queryTicks      (void);
    static F32      ticksToSecs     (S64 ticks);
    S64             getElapsedTicks (void);                 // return time since start, start if unstarted

private:
    static F64      s_ticksToSecsCoef;
    static S64      s_prevTicks;

    S64             m_startTicks;
    S64             m_totalTicks;
};

//------------------------------------------------------------------------
}
