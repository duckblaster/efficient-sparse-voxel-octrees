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
#include "base/Thread.hpp"
#include "base/Deque.hpp"

namespace FW
{
//------------------------------------------------------------------------
// Launch a batch of tasks and wait:
//   MulticoreLauncher().push(myTaskFunc, &myTaskData, 0, numTasks);
//
// Display progress indicator:
//   MulticoreLauncher().push(myTaskFunc, &myTaskData, 0, numTasks).popAll("Processing...");
//
// Grab results:
// {
//     MulticoreLauncher launcher;
//     launcher.push(...);
//     while (launcher.getNumTasks())
//         doSomething(launcher.pop().result);
// }
//
// Asynchronous operation:
// {
//     MulticoreLauncher launcher;
//     launcher.push(...);
//     while (launcher.getNumTasks())
//     {
//         spendSomeIdleTime();
//         while (launcher.getNumFinished())
//             doSomething(launcher.pop().result);
//     }
// }
//
// Create dependent tasks dynamically:
//
// void myTaskFunc(MulticoreLauncher::Task& task)
// {
//     ...
//     task.launcher->push(...);
//     ...
// }
//
// Per-thread temporary data:
//
// void myDeinitFunc(void* threadData)
// {
//     ...
// }
//
// void myTaskFunc(MulticoreLauncher::Task& task)
// {
//     void* threadData = Thread::getCurrent()->getUserData("myID");
//     if (!threadData)
//     {
//         threadData = ...;
//         Thread::getCurrent()->setUserData("myID", threadData, myDeinitFunc);
//     }
//     ...
// }
//------------------------------------------------------------------------

class MulticoreLauncher
{
public:
    struct Task;
    typedef void (*TaskFunc)(Task& task);

    struct Task
    {
        MulticoreLauncher*  launcher;
        TaskFunc            func;
        void*               data;
        int                 idx;
        void*               result; // Potentially written by TaskFunc.
    };

public:
                            MulticoreLauncher   (void);
                            ~MulticoreLauncher  (void);

    MulticoreLauncher&      push                (TaskFunc func, void* data, int firstIdx = 0, int numTasks = 1);
    Task                    pop                 (void);         // Blocks until at least one task has finished.

    int                     getNumTasks         (void) const;   // Tasks that have been pushed but not popped.
    int                     getNumFinished      (void) const;   // Tasks that can be popped without blocking.

    void                    popAll              (void)          { while (getNumTasks()) pop(); }
    void                    popAll              (const String& progressMessage);

    static int              getNumCores         (void);
    static void             setNumThreads       (int numThreads);

private:
    static void             applyNumThreads     (void);
    static void             threadFunc          (void* param);

private:
                            MulticoreLauncher   (const MulticoreLauncher&); // forbidden
    MulticoreLauncher&      operator=           (const MulticoreLauncher&); // forbidden

private:
    static Spinlock         s_lock;
    static S32              s_numInstances;
    static S32              s_desiredThreads;

    static Monitor*         s_monitor;
    static Deque<Task>      s_pending;
    static S32              s_numThreads;

    S32                     m_numTasks;
    Deque<Task>             m_finished;
};

//------------------------------------------------------------------------
}
