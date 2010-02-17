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
 
#include "base/Defs.hpp"
#include "base/String.hpp"
#include "base/Thread.hpp"
#include "base/Timer.hpp"
#include "gui/Window.hpp"
#include "io/File.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>

using namespace FW;

//------------------------------------------------------------------------

#define FW_MEM_DEBUG    0

//------------------------------------------------------------------------

struct AllocHeader
{
    AllocHeader*    prev;
    AllocHeader*    next;
    size_t          size;
    const char*     ownerID;
};

//------------------------------------------------------------------------

struct ProfileTimer
{
    String          id;
    Timer           timer;
    S32             parent;
    Array<S32>      children;
};

//------------------------------------------------------------------------

static Spinlock                         s_lock;
static size_t                           s_memoryUsed        = 0;
static bool                             s_hasFailed         = false;
static String                           s_error;
static bool                             s_discardEvents     = false;

static Array<File*>                     s_logFiles;
static Array<BufferedOutputStream*>     s_logStreams;

#if FW_MEM_DEBUG
static bool                             s_memPushingOwner   = false;
static AllocHeader                      s_memAllocs         = { &s_memAllocs, &s_memAllocs, 0, NULL };
static Hash<U32, Array<const char*> >   s_memOwnerStacks;
#endif

static bool                             s_profileStarted    = false;
static Hash<const char*, S32>           s_profilePointerToToken;
static Hash<String, S32>                s_profileStringToToken;
static Hash<Vec2i, S32>                 s_profileTimerHash;         // (parentTimer, childToken) => childTimer
static Array<ProfileTimer>              s_profileTimers;
static Array<S32>                       s_profileStack;

//------------------------------------------------------------------------

void* FW::malloc(size_t size)
{
    FW_ASSERT(size >= 0);

#if FW_MEM_DEBUG
    s_lock.enter();

    AllocHeader* alloc = (AllocHeader*)::malloc(sizeof(AllocHeader) + size);
    if (!alloc)
        fail("Out of memory!");

    void* ptr           = alloc + 1;
    alloc->prev         = s_memAllocs.prev;
    alloc->next         = &s_memAllocs;
    alloc->prev->next   = alloc;
    alloc->next->prev   = alloc;
    alloc->size         = size;
    alloc->ownerID      = "Uncategorized";

    if (!s_memPushingOwner)
    {
        U32 threadID = Thread::getID();
        if (s_memOwnerStacks.contains(threadID) && s_memOwnerStacks[threadID].getSize())
               alloc->ownerID = s_memOwnerStacks[threadID].getLast();
    }

    s_lock.leave();

#else
    void* ptr = ::malloc(size);
    if (!ptr)
        fail("Out of memory!");
#endif

    s_memoryUsed += size;
    return ptr;
}

//------------------------------------------------------------------------

void FW::free(void* ptr)
{
    if (!ptr)
        return;

#if FW_MEM_DEBUG
    s_lock.enter();

    AllocHeader* alloc = (AllocHeader*)ptr - 1;
    alloc->prev->next = alloc->next;
    alloc->next->prev = alloc->prev;
    s_memoryUsed -= alloc->size;
    ::free(alloc);

    s_lock.leave();

#else
    s_memoryUsed -= _msize(ptr);
    ::free(ptr);
#endif
}

//------------------------------------------------------------------------

void* FW::realloc(void* ptr, size_t size)
{
    FW_ASSERT(size >= 0);

    if (!ptr)
        return FW::malloc(size);

    if (!size)
    {
        FW::free(ptr);
        return NULL;
    }

#if FW_MEM_DEBUG
    size_t oldSize = ((AllocHeader*)ptr - 1)->size;
    void* newPtr = FW::malloc(size);
    memcpy(newPtr, ptr, min(size, oldSize));
    FW::free(ptr);

#else
    size_t oldSize = _msize(ptr);
    void* newPtr = ::realloc(ptr, size);
    if (!newPtr)
        fail("Out of memory!");
#endif

    s_memoryUsed += size - oldSize;
    return newPtr;
}

//------------------------------------------------------------------------

void* FW::memset(void* dst, int value, size_t size)
{
    FW_ASSERT(size >= 0);
    FW_ASSERT(dst || !size);
    return ::memset(dst, value, size);
}

//------------------------------------------------------------------------

void* FW::memcpy(void* dst, const void* src, size_t size)
{
    FW_ASSERT(size >= 0);
    FW_ASSERT((dst && src) || !size);
    return ::memcpy(dst, src, size);
}

//------------------------------------------------------------------------

void* FW::memmove(void* dst, const void* src, size_t size)
{
    FW_ASSERT(size >= 0);
    FW_ASSERT((dst && src) || !size);
    return ::memmove(dst, src, size);
}

//------------------------------------------------------------------------

int FW::memcmp(const void* srcA, const void* srcB, size_t size)
{
    FW_ASSERT(size >= 0);
    FW_ASSERT((srcA && srcB) || !size);
    return ::memcmp(srcA, srcB, size);
}

//------------------------------------------------------------------------

void FW::printf(const char* fmt, ...)
{
    s_lock.enter();
    va_list args;
    va_start(args, fmt);

    vprintf(fmt, args);
    for (int i = 0; i < s_logFiles.getSize(); i++)
        s_logStreams[i]->writefv(fmt, args);

    va_end(args);
    s_lock.leave();
}

//------------------------------------------------------------------------

String FW::sprintf(const char* fmt, ...)
{
    String str;
    va_list args;
    va_start(args, fmt);
    str.setfv(fmt, args);
    va_end(args);
    return str;
}

//------------------------------------------------------------------------

void FW::setError(const char* fmt, ...)
{
    if (!Thread::isMain())
        fail("setError() must be called from the main thread!");

    if (hasError())
        return;

    va_list args;
    va_start(args, fmt);
    s_error.setfv(fmt, args);
    va_end(args);

    FW_ASSERT(hasError());
}

//------------------------------------------------------------------------

String FW::clearError(void)
{
    if (!Thread::isMain())
        fail("clearError() must be called from the main thread!");

    String old = s_error;
    s_error.reset();
    return old;
}

//------------------------------------------------------------------------

bool FW::restoreError(const String& old)
{
    if (!Thread::isMain())
        fail("restoreError() must be called from the main thread!");

    bool had = hasError();
    s_error = old;
    return had;
}

//------------------------------------------------------------------------

bool FW::hasError(void)
{
    if (!Thread::isMain())
        fail("hasError() must be called from the main thread!");
    return (s_error.getLength() != 0);
}

//------------------------------------------------------------------------

const String& FW::getError(void)
{
    if (!Thread::isMain())
        fail("getError() must be called from the main thread!");
    return s_error;
}

//------------------------------------------------------------------------

void FW::fail(const char* fmt, ...)
{
    s_lock.enter();
    bool alreadyFailed = s_hasFailed;
    s_hasFailed = true;
    setDiscardEvents(true);
    s_lock.leave();

    if (alreadyFailed)
        return;

    String tmp;
    va_list args;
    va_start(args, fmt);
    tmp.setfv(fmt, args);
    va_end(args);

    printf("%s\n", tmp.getPtr());
    MessageBox(NULL, tmp.getPtr(), "Fatal error", MB_OK);

    DebugBreak();
    exit(1);
}

//------------------------------------------------------------------------

void FW::failWin32Error(const char* funcName)
{
    DWORD err = GetLastError();
    LPTSTR msgBuf = NULL;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, (LPTSTR)&msgBuf, 0, NULL);
    String msg(msgBuf);
    LocalFree(msgBuf);

    if (msg.getLength())
        fail("%s() failed!\n%s", funcName, msg.getPtr());
    else
        fail("%s() failed!\nError %d\n", funcName, err);
}

//------------------------------------------------------------------------

void FW::failIfError(void)
{
    if (!Thread::isMain())
        fail("failIfError() must be called from the main thread!");
    if (hasError())
        fail("%s", s_error.getPtr());
}

//------------------------------------------------------------------------

bool FW::setDiscardEvents(bool discard)
{
    bool old = s_discardEvents;
    s_discardEvents = discard;
    return old;
}

//------------------------------------------------------------------------

bool FW::getDiscardEvents(void)
{
    return s_discardEvents;
}

//------------------------------------------------------------------------

void FW::pushLogFile(const String& name, bool append)
{
    File* file = new File(name, (append) ? File::Modify : File::Create);
    file->seek(file->getSize());
    s_logFiles.add(file);
    s_logStreams.add(new BufferedOutputStream(*file, 1024, true, true));
}

//------------------------------------------------------------------------

void FW::popLogFile(void)
{
    if (!s_logFiles.getSize())
        return;

    s_logStreams.getLast()->flush();
    delete s_logFiles.removeLast();
    delete s_logStreams.removeLast();

    if (!s_logFiles.getSize())
    {
        s_logFiles.reset();
        s_logStreams.reset();
    }
}

//------------------------------------------------------------------------

bool FW::hasLogFile(void)
{
    return (s_logFiles.getSize() != 0);
}

//------------------------------------------------------------------------

size_t FW::getMemoryUsed(void)
{
    return s_memoryUsed;
}

//------------------------------------------------------------------------

void FW::pushMemOwner(const char* id)
{
#if !FW_MEM_DEBUG
    FW_UNREF(id);
#else
    s_lock.enter();
    s_memPushingOwner = true;

    U32 threadID = Thread::getID();
    Array<const char*>* stack = s_memOwnerStacks.search(threadID);
    if (!stack)
    {
        stack = &s_memOwnerStacks.add(threadID);
        stack->clear();
    }
    stack->add(id);

    s_memPushingOwner = false;
    s_lock.leave();
#endif
}

//------------------------------------------------------------------------

void FW::popMemOwner(void)
{
#if FW_MEM_DEBUG
    U32 threadID = Thread::getID();
    Array<const char*>* stack = s_memOwnerStacks.search(threadID);
    if (stack)
    {
        stack->removeLast();
        if (!stack->getSize())
        {
            s_memOwnerStacks.remove(threadID);
            if (!s_memOwnerStacks.getSize())
                s_memOwnerStacks.reset();
        }
    }
#endif
}

//------------------------------------------------------------------------

void FW::printMemStats(void)
{
#if FW_MEM_DEBUG
    // Create snapshot of the alloc list.

    s_lock.enter();
    AllocHeader* first = NULL;
    for (AllocHeader* src = s_memAllocs.next; src != &s_memAllocs; src = src->next)
    {
        AllocHeader* alloc = (AllocHeader*)::malloc(sizeof(AllocHeader));
        *alloc = *src;
        alloc->next = first;
        first = alloc;
    }
    s_lock.leave();

    // Calculate total size per owner.

    Hash<String, S64> owners;
    for (AllocHeader* alloc = first; alloc;)
    {
        if (!owners.contains(alloc->ownerID))
            owners.add(alloc->ownerID, 0);
        owners[alloc->ownerID] += alloc->size;

        AllocHeader* next = alloc->next;
        ::free(alloc);
        alloc = next;
    }

    // Print.

    printf("\n");
    printf("%-32s%.2f\n", "Memory usage / megs", (F32)s_memoryUsed * exp2(-20));
    for (int slot = owners.firstSlot(); slot != -1; slot = owners.nextSlot(slot))
    {
        const HashEntry<String, S64>& entry = owners.getSlot(slot);
        printf("  %-30s%-12.2f%.0f%%\n",
            entry.key.getPtr(),
            (F32)entry.value * exp2(-20),
            (F32)entry.value / (F32)s_memoryUsed * 100.0f);
    }
    printf("\n");
#endif
}

//------------------------------------------------------------------------

void FW::profileStart(void)
{
    if (!Thread::isMain())
        fail("profileStart() must be called from the main thread!");
    if (s_profileStarted)
        return;

    s_profileStarted = true;
    profilePush("Total time spent");
}

//------------------------------------------------------------------------

void FW::profilePush(const char* id)
{
    if (!s_profileStarted)
        return;
    if (!Thread::isMain())
        fail("profilePush() must be called from the main thread!");

    // Find or create token.

    S32 token;
    S32* found = s_profilePointerToToken.search(id);
    if (found)
        token = *found;
    else
    {
        found = s_profileStringToToken.search(id);
        if (found)
            token = *found;
        else
        {
            token = s_profileStringToToken.getSize();
            s_profileStringToToken.add(id, token);
        }
        s_profilePointerToToken.add(id, token);
    }

    // Find or create timer.

    Vec2i timerKey(-1, token);
    if (s_profileStack.getSize())
        timerKey.x = s_profileStack.getLast();

    S32 timerIdx;
    found = s_profileTimerHash.search(timerKey);
    if (found)
        timerIdx = *found;
    else
    {
        timerIdx = s_profileTimers.getSize();
        s_profileTimerHash.add(timerKey, timerIdx);
        ProfileTimer& timer = s_profileTimers.add();
        timer.id = id;
        timer.parent = timerKey.x;
        if (timerKey.x != -1)
            s_profileTimers[timerKey.x].children.add(timerIdx);
    }

    // Push timer.

    if (s_profileStack.getSize() == 1)
        s_profileTimers[s_profileStack[0]].timer.start();
    s_profileStack.add(timerIdx);
    if (s_profileStack.getSize() > 1)
        s_profileTimers[timerIdx].timer.start();
}

//------------------------------------------------------------------------

void FW::profilePop(void)
{
    if (!s_profileStarted)
        return;
    if (!Thread::isMain())
        fail("profilePop() must be called from the main thread!");

    if (s_profileStack.getSize() > 1)
        s_profileTimers[s_profileStack.getLast()].timer.end();
    s_profileStack.removeLast();
    if (s_profileStack.getSize() == 1)
        s_profileTimers[s_profileStack.getLast()].timer.end();
}

//------------------------------------------------------------------------

void FW::profileEnd(void)
{
    if (!Thread::isMain())
        fail("profileEnd() must be called from the main thread!");
    if (!s_profileStarted)
        return;

    // Pop remaining timers.

    while (s_profileStack.getSize())
        profilePop();

    // Recurse and print.

    printf("\n");
    Array<Vec2i> stack(Vec2i(0, 0));
    while (stack.getSize())
    {
        Vec2i entry = stack.removeLast();
        const ProfileTimer& timer = s_profileTimers[entry.x];
        for (int i = timer.children.getSize() - 1; i >= 0; i--)
            stack.add(Vec2i(timer.children[i], entry.y + 2));

        printf("%*s%-*s%-8.3f",
            entry.y, "",
            32 - entry.y, timer.id.getPtr(),
            timer.timer.getTotal());

        printf("%.0f%%\n", timer.timer.getTotal() / s_profileTimers[0].timer.getTotal() * 100.0f);
    }
    printf("\n");

    // Clean up.

    s_profileStarted = false;
    s_profilePointerToToken.reset();
    s_profileStringToToken.reset();
    s_profileTimerHash.reset();
    s_profileTimers.reset();
    s_profileStack.reset();
}

//------------------------------------------------------------------------