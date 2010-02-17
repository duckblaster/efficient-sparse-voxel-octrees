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
 
#include "base/Thread.hpp"

using namespace FW;

//------------------------------------------------------------------------

bool                Thread::s_inited        = false;
Spinlock*           Thread::s_lock          = NULL;
Hash<U32, Thread*>* Thread::s_threads       = NULL;
Thread*             Thread::s_mainThread    = NULL;

//------------------------------------------------------------------------

Spinlock::Spinlock(void)
{
    InitializeCriticalSection(&m_critSect);
}

//------------------------------------------------------------------------

Spinlock::~Spinlock(void)
{
    DeleteCriticalSection(&m_critSect);
}

//------------------------------------------------------------------------

void Spinlock::enter(void)
{
    EnterCriticalSection(&m_critSect);
}

//------------------------------------------------------------------------

void Spinlock::leave(void)
{
    LeaveCriticalSection(&m_critSect);
}

//------------------------------------------------------------------------

Semaphore::Semaphore(int initCount, int maxCount)
{
    m_handle = CreateSemaphore(NULL, initCount, maxCount, NULL);
    if (!m_handle)
        failWin32Error("CreateSemaphore");
}

//------------------------------------------------------------------------

Semaphore::~Semaphore(void)
{
    CloseHandle(m_handle);
}

//------------------------------------------------------------------------

bool Semaphore::acquire(int millis)
{
    DWORD res = WaitForSingleObject(m_handle, (millis >= 0) ? millis : INFINITE);
    if (res == WAIT_FAILED)
        failWin32Error("WaitForSingleObject");
    return (res == WAIT_OBJECT_0);
}

//------------------------------------------------------------------------

void Semaphore::release(void)
{
    if (!ReleaseSemaphore(m_handle, 1, NULL))
        failWin32Error("ReleaseSemaphore");
}

//------------------------------------------------------------------------

Monitor::Monitor(void)
:   m_ownerSem      (1, 1),
    m_waitSem       (0, 1),
    m_notifySem     (0, 1),
    m_ownerThread   (0),
    m_enterCount    (0),
    m_waitCount     (0)
{
}

//------------------------------------------------------------------------

Monitor::~Monitor(void)
{
}

//------------------------------------------------------------------------

void Monitor::enter(void)
{
    U32 currThread = Thread::getID();

    m_lock.enter();
    if (m_ownerThread != currThread || !m_enterCount)
    {
        m_lock.leave();
        m_ownerSem.acquire();
        m_lock.enter();
    }

    m_ownerThread = currThread;
    m_enterCount++;
    m_lock.leave();
}

//------------------------------------------------------------------------

void Monitor::leave(void)
{
    FW_ASSERT(m_ownerThread == Thread::getID() && m_enterCount);
    m_enterCount--;
    if (!m_enterCount)
        m_ownerSem.release();
}

//------------------------------------------------------------------------

void Monitor::wait(void)
{
    FW_ASSERT(m_ownerThread == Thread::getID() && m_enterCount);
    U32 currThread = m_ownerThread;
    int enterCount = m_enterCount;

    m_waitCount++;
    m_enterCount = 0;
    m_ownerSem.release();

    m_waitSem.acquire();
    m_waitCount--;
    m_notifySem.release();

    m_ownerSem.acquire();
    m_lock.enter();
    m_ownerThread = currThread;
    m_enterCount = enterCount;
    m_lock.leave();
}

//------------------------------------------------------------------------

void Monitor::notify(void)
{
    FW_ASSERT(m_ownerThread == Thread::getID() && m_enterCount);
    if (m_waitCount)
    {
        m_waitSem.release();
        m_notifySem.acquire();
    }
}

//------------------------------------------------------------------------

void Monitor::notifyAll(void)
{
    FW_ASSERT(m_ownerThread == Thread::getID() && m_enterCount);
    while (m_waitCount)
    {
        m_waitSem.release();
        m_notifySem.acquire();
    }
}

//------------------------------------------------------------------------

Thread::Thread(void)
:   m_id        (0),
    m_handle    (NULL)
{
    staticInit();
}

//------------------------------------------------------------------------

Thread::~Thread(void)
{
    if (this != s_mainThread)
        join();
    exited();
}

//------------------------------------------------------------------------

void Thread::start(ThreadFunc func, void* param)
{
    join();
    s_lock->enter();

    StartParams params;
    params.thread       = this;
    params.userFunc     = func;
    params.userParam    = param;
    params.parentReady.acquire();
    params.childReady.acquire();

    DWORD id;
    m_handle = CreateThread(NULL, 0, threadProc, &params, 0, &id);
    m_id = id;
    if (!m_handle)
        failWin32Error("CreateThread");

    started();
    s_lock->leave();

    params.parentReady.release();
    params.childReady.acquire();
}

//------------------------------------------------------------------------

Thread* Thread::getCurrent(void)
{
    staticInit();
    s_lock->enter();
    Thread** found = s_threads->search(getID());
    Thread* thread = (found) ? *found : NULL;
    s_lock->leave();

    if (!thread)
    {
        thread = new Thread;
        thread->setCurrent();
    }
    return thread;
}

//------------------------------------------------------------------------

Thread* Thread::getMain(void)
{
    staticInit();
    FW_ASSERT(s_mainThread);
    return s_mainThread;
}

//------------------------------------------------------------------------

bool Thread::isMain(void)
{
    staticInit();
    return (!s_mainThread || getID() == s_mainThread->m_id);
}

//------------------------------------------------------------------------

U32 Thread::getID(void)
{
    return GetCurrentThreadId();
}

//------------------------------------------------------------------------

void Thread::sleep(int millis)
{
    Sleep(millis);
}

//------------------------------------------------------------------------

void Thread::yield(void)
{
    SwitchToThread();
}

//------------------------------------------------------------------------

Thread::Priority Thread::getPriority(void) const
{
    HANDLE handle = m_handle;
    if (!handle)
        return Priority_Normal;

    int res = GetThreadPriority(handle);
    if (res == THREAD_PRIORITY_ERROR_RETURN)
        failWin32Error("GetThreadPriority");
    return (Priority)res;
}

//------------------------------------------------------------------------

void Thread::setPriority(Priority priority)
{
    HANDLE handle = m_handle;
    if (handle && !SetThreadPriority(handle, priority))
        failWin32Error("SetThreadPriority");
}

//------------------------------------------------------------------------

bool Thread::isAlive(void)
{
    HANDLE handle = m_handle;
    if (!handle)
        return false;

    DWORD exitCode;
    if (!GetExitCodeThread(handle, &exitCode))
        failWin32Error("GetExitCodeThread");
    if (exitCode == STILL_ACTIVE)
        return true;

    s_lock->enter();
    exited();
    s_lock->leave();
    return false;
}

//------------------------------------------------------------------------

void Thread::join(void)
{
    FW_ASSERT(this != s_mainThread);
    FW_ASSERT(this != getCurrent());

    HANDLE handle = m_handle;
    if (!handle)
        return;

    if (WaitForSingleObject(handle, INFINITE) == WAIT_FAILED)
        failWin32Error("WaitForSingleObject");

    s_lock->enter();
    exited();
    s_lock->leave();
}

//------------------------------------------------------------------------

void Thread::staticInit(void)
{
    if (s_inited)
        return;
    s_inited = true;

    FW_ASSERT(!s_lock && !s_threads && !s_mainThread);
    s_lock = new Spinlock();
    s_threads = new Hash<U32, Thread*>;
    s_mainThread = new Thread;
    s_mainThread->setCurrent();
}

//------------------------------------------------------------------------

void Thread::staticDeinit(void)
{
    if (!s_inited)
        return;
    s_inited = false;

    delete s_mainThread;
    s_mainThread = NULL;

    delete s_lock;
    s_lock = NULL;

    delete s_threads;
    s_threads = NULL;
}

//------------------------------------------------------------------------

void Thread::setCurrent(void)
{
    m_id = getID();
    m_handle = OpenThread(THREAD_ALL_ACCESS, FALSE, m_id);
    if (!m_handle)
        failWin32Error("OpenThread");

    s_lock->enter();
    started();
    s_lock->leave();
}

//------------------------------------------------------------------------

void Thread::started(void)
{
    if (!s_threads->contains(m_id))
        s_threads->add(m_id, this);
}

//------------------------------------------------------------------------

void Thread::exited(void)
{
    if (!m_handle)
        return;

    if (s_threads->contains(m_id))
        s_threads->remove(m_id);
    m_id = 0;

    if (m_handle)
        CloseHandle(m_handle);
    m_handle = NULL;
}

//------------------------------------------------------------------------

DWORD WINAPI Thread::threadProc(LPVOID lpParameter)
{
    StartParams*    params      = (StartParams*)lpParameter;
    Thread*         thread      = params->thread;
    ThreadFunc      userFunc    = params->userFunc;
    void*           userParam   = params->userParam;

    params->parentReady.acquire();
    params->childReady.release();
    userFunc(userParam);

    s_lock->enter();
    thread->exited();
    s_lock->leave();
    return 0;
}

//------------------------------------------------------------------------
