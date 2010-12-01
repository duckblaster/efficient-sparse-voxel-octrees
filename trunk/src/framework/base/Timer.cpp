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
 
#include "base/Timer.hpp"
#include "base/Thread.hpp"

using namespace FW;

//------------------------------------------------------------------------

F64 Timer::s_ticksToSecsCoef    = -1.0;
S64 Timer::s_prevTicks          = 0;

//------------------------------------------------------------------------

F32 Timer::end(void)
{
    S64 elapsed = getElapsedTicks();
    m_startTicks += elapsed;
    m_totalTicks += elapsed;
    return ticksToSecs(elapsed);
}

//------------------------------------------------------------------------

S64 Timer::queryTicks(void)
{
    if (!Thread::isMain())
        fail("Timers can only be used in the main thread!");

    LARGE_INTEGER ticks;
    if (!QueryPerformanceCounter(&ticks))
        failWin32Error("QueryPerformanceCounter");

    s_prevTicks = max(s_prevTicks, ticks.QuadPart);
    return s_prevTicks;
}

//------------------------------------------------------------------------

F32 Timer::ticksToSecs(S64 ticks)
{
    if (s_ticksToSecsCoef == -1.0)
    {
        LARGE_INTEGER freq;
        if (!QueryPerformanceFrequency(&freq))
            failWin32Error("QueryPerformanceFrequency");
        s_ticksToSecsCoef = max(1.0 / (F64)freq.QuadPart, 0.0);
    }

    return (F32)(ticks * s_ticksToSecsCoef);
}

//------------------------------------------------------------------------

S64 Timer::getElapsedTicks(void)
{
    S64 curr = queryTicks();
    if (m_startTicks == -1)
        m_startTicks = curr;
    return curr - m_startTicks;
}

//------------------------------------------------------------------------
