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
 
#include "base/MulticoreLauncher.hpp"
#include "base/Timer.hpp"

using namespace FW;

//------------------------------------------------------------------------

#define MAX_TASK_ARRAY_SLACK    4096

//------------------------------------------------------------------------

Spinlock                        MulticoreLauncher::s_lock;
S32                             MulticoreLauncher::s_numInstances   = 0;
S32                             MulticoreLauncher::s_desiredThreads = -1;

Monitor*                        MulticoreLauncher::s_monitor        = NULL;
Array<MulticoreLauncher::Task>  MulticoreLauncher::s_pending;
S32                             MulticoreLauncher::s_nextPending    = 0;
S32                             MulticoreLauncher::s_numThreads     = 0;

//------------------------------------------------------------------------

MulticoreLauncher::MulticoreLauncher(void)
:   m_numTasks      (0),
    m_nextFinished  (0)
{
    s_lock.enter();

    // First time => query the number of cores.

    if (s_desiredThreads == -1)
        s_desiredThreads = getNumCores();

    // First instance => static init.

    if (s_numInstances++ == 0)
    {
        s_monitor = new Monitor;
        s_nextPending = 0;
    }

    s_lock.leave();
}

//------------------------------------------------------------------------

MulticoreLauncher::~MulticoreLauncher(void)
{
    popAll();
    s_lock.enter();

    // Last instance => static deinit.

    if (--s_numInstances == 0)
    {
        // Kill threads.

        int old = s_desiredThreads;
        s_desiredThreads = 0;
        s_monitor->enter();
        applyNumThreads();
        s_monitor->leave();
        s_desiredThreads = old;

        // Deinit static members.

        delete s_monitor;
        s_monitor = NULL;
        s_pending.reset();
    }

    s_lock.leave();
}

//------------------------------------------------------------------------

MulticoreLauncher& MulticoreLauncher::push(TaskFunc func, void* data, int firstIdx, int numTasks)
{
    FW_ASSERT(func != NULL);
    FW_ASSERT(numTasks >= 0);

    if (numTasks <= 0)
        return *this;

    s_monitor->enter();

    m_numTasks += numTasks;
    Task* tasks = s_pending.add(NULL, numTasks);

    for (int i = 0; i < numTasks; i++)
    {
        tasks[i].launcher = this;
        tasks[i].func     = func;
        tasks[i].data     = data;
        tasks[i].idx      = firstIdx + i;
        tasks[i].result   = NULL;
    }

    applyNumThreads();
    s_monitor->notifyAll();
    s_monitor->leave();
    return *this;
}

//------------------------------------------------------------------------

MulticoreLauncher::Task MulticoreLauncher::pop(void)
{
    FW_ASSERT(getNumTasks());
    s_monitor->enter();

    // Wait for a task to finish.

    while (!getNumFinished())
        s_monitor->wait();

    // Pop from the queue.

    m_numTasks--;
    Task task = m_finished[m_nextFinished++];
    if (m_nextFinished > MAX_TASK_ARRAY_SLACK)
    {
        m_finished.remove(0, m_nextFinished);
        m_nextFinished = 0;
    }

    s_monitor->leave();
    return task;
}

//------------------------------------------------------------------------

int MulticoreLauncher::getNumTasks(void) const
{
    return m_numTasks;
}

//------------------------------------------------------------------------

int MulticoreLauncher::getNumFinished(void) const
{
    return m_finished.getSize() - m_nextFinished;
}

//------------------------------------------------------------------------

void MulticoreLauncher::popAll(const String& progressMessage)
{
    Timer timer;
    F32 progress = 0.0f;
    while (getNumTasks())
    {
        if (timer.getElapsed() > 0.1f)
        {
            printf("\r%s %d%%", progressMessage.getPtr(), (int)progress);
            timer.start();
        }
        pop();
        progress += (100.0f - progress) / (F32)(getNumTasks() + 1);
    }
    printf("\r%s 100%%\n", progressMessage.getPtr());
}

//------------------------------------------------------------------------

int MulticoreLauncher::getNumCores(void)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwNumberOfProcessors;
}

//------------------------------------------------------------------------

void MulticoreLauncher::setNumThreads(int numThreads)
{
    FW_ASSERT(numThreads > 0);
    s_lock.enter();

    s_desiredThreads = numThreads;
    if (s_numThreads != 0)
    {
        s_monitor->enter();
        applyNumThreads();
        s_monitor->leave();
    }

    s_lock.leave();
}

//------------------------------------------------------------------------

void MulticoreLauncher::applyNumThreads(void) // Must have the monitor.
{
    // Start new threads.

    while (s_numThreads < s_desiredThreads)
    {
        (new Thread)->start(threadFunc, NULL);
        s_numThreads++;
    }

    // Kill excess threads.

    if (s_numThreads > s_desiredThreads)
    {
        s_monitor->notifyAll();
        while (s_numThreads > s_desiredThreads)
            s_monitor->wait();
    }
}

//------------------------------------------------------------------------

void MulticoreLauncher::threadFunc(void* param)
{
    FW_UNREF(param);
    Thread::getCurrent()->setPriority(Thread::Priority_Min);
    s_monitor->enter();

    while (s_numThreads <= s_desiredThreads)
    {
        // No pending tasks => wait.

        if (s_nextPending >= s_pending.getSize())
        {
            s_monitor->wait();
            continue;
        }

        // Pick a task.

        Task task = s_pending.get(s_nextPending++);
        MulticoreLauncher* launcher = task.launcher;
        if (s_nextPending > MAX_TASK_ARRAY_SLACK)
        {
            s_pending.remove(0, s_nextPending);
            s_nextPending = 0;
        }

        // Execute.

        s_monitor->leave();
        task.func(task);
        failIfError();
        s_monitor->enter();

        // Mark as finished.

        launcher->m_finished.add(task);
        s_monitor->notifyAll();
    }

    s_numThreads--;
    delete Thread::getCurrent();
    s_monitor->notifyAll();
    s_monitor->leave();
}

//------------------------------------------------------------------------
