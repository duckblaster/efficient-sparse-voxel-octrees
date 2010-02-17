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
#include "gpu/Buffer.hpp"

namespace FW
{
//------------------------------------------------------------------------

class MemoryManager
{
public:

    //------------------------------------------------------------------------

    enum
    {
        MaxCpuBytes     = 512 << 20,
        MaxTextureBytes = 512 << 20,
        CudaMemReserve  = 32 << 20,
        CudaMemMaxPct   = 90,
        CompactMinCopy  = 8 << 20,
        CompactMaxTemp  = 16 << 20
    };

    enum Mode
    {
        Mode_CPU,
        Mode_Cuda,
    };

    //------------------------------------------------------------------------

    class Relocation
    {
    public:
                            Relocation              (void)                      : m_initStep(1) {}
                            Relocation              (const Relocation& other)   { operator=(other); }
                            ~Relocation             (void)                      {}

        bool                isEmpty                 (void) const                { return (m_oldOfs.getSize() == 0); }
        S64                 getByteDelta            (S64 oldOfs) const;
        S64                 getDWordDelta           (S64 oldOfs) const          { return getByteDelta(oldOfs << 2) >> 2; }

        void                clear                   (void)                      { m_initStep = 1; m_oldOfs.clear(); m_delta.clear(); }
        void                add                     (S64 oldOfs, S64 delta);
        Relocation&         operator=               (const Relocation& other);

    private:
        S32                 m_initStep;
        Array<S64>          m_oldOfs;
        Array<S64>          m_delta;
    };

    //------------------------------------------------------------------------

private:
    struct FreeRange // tracks allocation status of pages
    {
        FreeRange*          prev;
        FreeRange*          next;

        S64                 startPage;              // inclusive
        S64                 endPage;                // exclusive
    };

public:
                            MemoryManager           (Mode mode, int align);
                            ~MemoryManager          (void);

    Buffer&                 getBuffer               (void)                      { return m_buffer; }
    Mode                    getMode                 (void) const                { return m_mode; }
    S64                     getTotalBytes           (void) const                { return m_numPages << m_pageBytesLog; }
    S64                     getFreeBytes            (void) const                { return m_numFreePages << m_pageBytesLog; }

    void                    clear                   (void);
    S64                     alloc                   (S64 numBytes, Relocation* reloc = NULL); // -1 if out of memory
    void                    free                    (S64 ofs, S64 numBytes);

private:
    FreeRange*              addFreeRange            (FreeRange* prev, S64 startPage, S64 endPage);
    void                    removeFreeRange         (FreeRange* range);
    void                    compact                 (FreeRange* startRange, FreeRange* endRange, Relocation* reloc);

    void                    allocMaximalCudaBuffer  (void);

private:
                            MemoryManager           (MemoryManager&); // forbidden
    MemoryManager&          operator=               (MemoryManager&); // forbidden

private:
    Mode                    m_mode;
    S32                     m_pageBytes;
    S32                     m_pageBytesLog;

    Buffer                  m_buffer;
    Array<CUdeviceptr>      m_cudaBlocks;
    S64                     m_numPages;
    S64                     m_numFreePages;
    FreeRange               m_freeRanges;

    Buffer                  m_compactTemp;
};

//------------------------------------------------------------------------
}
