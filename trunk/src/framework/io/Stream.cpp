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
 
#include "io/Stream.hpp"

#include <stdio.h>

using namespace FW;

//------------------------------------------------------------------------

void InputStream::readFully(void* ptr, int size)
{
    S32 numRead = read(ptr, size);
    if (numRead != size)
    {
        FW_ASSERT(numRead >= 0 && numRead <= size);
        memset((U8*)ptr + numRead, 0, size - numRead);
        setError("Unexpected end of stream!");
    }
}

//------------------------------------------------------------------------

BufferedInputStream::BufferedInputStream(InputStream& stream, int bufferSize)
:   m_stream        (stream),
    m_numRead       (0),
    m_numConsumed   (0),
    m_buffer        (NULL, bufferSize)
{
    FW_ASSERT(bufferSize > 0);
}

//------------------------------------------------------------------------

BufferedInputStream::~BufferedInputStream(void)
{
}

//------------------------------------------------------------------------

int BufferedInputStream::read(void* ptr, int size)
{
    if (!size)
        return 0;

    FW_ASSERT(ptr && size > 0);
    int ofs = 0;
    while (ofs < size)
    {
        fillBuffer(1);
        int num = min(size - ofs, getBufferSize());
        if (!num)
            break;

        memcpy((U8*)ptr + ofs, getBufferPtr(), num);
        consumeBuffer(num);
        ofs += num;
    }
    return ofs;
}

//------------------------------------------------------------------------

char* BufferedInputStream::readLine(bool combineWithBackslash, bool normalizeWhitespace)
{
    if (!getBufferSize() && !fillBuffer(1))
        return NULL;

    U8* ptr = getBufferPtr();
    int size = getBufferSize();
    int inPos = 0;
    int outPos = 0;
    bool pendingBackslash = false;

    for (;;)
    {
        U8 chr = ptr[inPos++];
        if (chr >= 32 && chr != '\\' && !pendingBackslash)
            ptr[outPos++] = chr;
        else if (chr == '\n')
        {
            if (!pendingBackslash)
                break;
            pendingBackslash = false;
        }
        else if (chr != '\r')
        {
            if (pendingBackslash)
            {
                ptr[outPos++] = '\\';
                pendingBackslash = false;
            }
            if (chr == '\t' && normalizeWhitespace)
                ptr[outPos++] = ' ';
            else if (chr == '\\' && combineWithBackslash)
                pendingBackslash = true;
            else
                ptr[outPos++] = chr;
        }

        if (inPos == size)
        {
            fillBuffer(inPos + 1);
            ptr = getBufferPtr();
            size = getBufferSize();
            if (inPos == size)
            {
                if (pendingBackslash)
                    ptr[outPos++] = '\\';
                break;
            }
        }
    }

    ptr[outPos] = '\0';
    char* line = (char*)ptr;
    consumeBuffer(inPos);
    return line;
}

//------------------------------------------------------------------------

bool BufferedInputStream::fillBuffer(int size)
{
    FW_ASSERT(size >= 0);

    // Already have the data => done.

    if (m_numRead - size >= m_numConsumed)
        return true;

    // Buffer is full => grow or shift.

    if (m_numRead == m_buffer.getSize())
    {
        if (!m_numConsumed)
            m_buffer.resize(m_buffer.getSize() * 2);
        else
        {
            memcpy(m_buffer.getPtr(), m_buffer.getPtr(m_numConsumed), m_numRead - m_numConsumed);
            m_numRead -= m_numConsumed;
            m_numConsumed = 0;
        }
    }

    // Read more data.

    m_numRead += m_stream.read(m_buffer.getPtr(m_numRead), m_buffer.getSize() - m_numRead);
    return (m_numRead - size >= m_numConsumed);
}

//------------------------------------------------------------------------

void BufferedInputStream::consumeBuffer(int num)
{
    FW_ASSERT(num >= 0);
    int numLeft = num;
    while (numLeft)
    {
        fillBuffer(1);
        int tmp = min(numLeft, m_numRead - m_numConsumed);
        numLeft -= tmp;
        m_numConsumed += tmp;
    }
}

//------------------------------------------------------------------------

BufferedOutputStream::BufferedOutputStream(OutputStream& stream, int bufferSize, bool writeOnLF, bool emulateCR)
:   m_stream        (stream),
    m_writeOnLF     (writeOnLF),
    m_emulateCR     (emulateCR),

    m_buffer        (NULL, bufferSize),
    m_numValid      (0),
    m_lineStart     (0),
    m_currOfs       (0)
{
    FW_ASSERT(bufferSize > 0);
}

//------------------------------------------------------------------------

BufferedOutputStream::~BufferedOutputStream(void)
{
}

//------------------------------------------------------------------------

void BufferedOutputStream::write(const void* ptr, int size)
{
    if (size <= 0)
        return;

    int ofs = 0;
    for (;;)
    {
        int num = min(size - ofs, m_buffer.getSize() - m_numValid);
        memcpy(m_buffer.getPtr(m_numValid), (const U8*)ptr + ofs, num);
        addValid(num);

        ofs += num;
        if (ofs >= size)
            break;

        flushInternal();
    }
}

//------------------------------------------------------------------------

void BufferedOutputStream::writef(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    writefv(fmt, args);
    va_end(args);
}

//------------------------------------------------------------------------

void BufferedOutputStream::writefv(const char* fmt, va_list args)
{
    int space = m_buffer.getSize() - m_numValid;
    int size = vsnprintf_s((char*)m_buffer.getPtr(m_numValid), space, space - 1, fmt, args);
    if (size >= 0)
    {
        addValid(size);
        return;
    }

    flushInternal();
    size = _vscprintf(fmt, args);
    if (size < m_buffer.getSize())
        addValid(vsprintf_s((char*)m_buffer.getPtr(), m_buffer.getSize(), fmt, args));
    else
    {
        char* tmp = new char[size + 1];
        vsprintf_s(tmp, size + 1, fmt, args);
        m_stream.write(tmp, size);
        delete[] tmp;
    }
}

//------------------------------------------------------------------------

void BufferedOutputStream::flush(void)
{
    m_lineStart = 0;
    flushInternal();
    m_stream.flush();
}

//------------------------------------------------------------------------

void BufferedOutputStream::addValid(int size)
{
    FW_ASSERT(size >= 0);
    if (!size)
        return;

    // Increase valid size.

    int old = m_numValid;
    m_numValid += size;
    if (!m_writeOnLF && !m_emulateCR)
        return;

    // Write on LF => find the last LF.

    if (!m_emulateCR)
    {
        for (int i = m_numValid - 1; i >= old; i--)
        {
            if (m_buffer[i] == '\n')
            {
                m_lineStart = i + 1;
                flushInternal();
                break;
            }
        }
        return;
    }

    // Emulate CR => scan through the new bytes.

    int lineEnd = old;
    for (int i = old; i < m_numValid; i++)
    {
        U8 v = m_buffer[i];
        if (v == '\r')
            m_currOfs = m_lineStart;
        else if (v == '\n')
        {
            m_currOfs = lineEnd;
            m_buffer[m_currOfs++] = v;
            m_lineStart = m_currOfs;
            lineEnd = m_currOfs;
        }
        else
        {
            m_buffer[m_currOfs++] = v;
            lineEnd = max(lineEnd, m_currOfs);
        }
    }

    m_numValid = lineEnd;
    if (m_writeOnLF && m_lineStart)
        flushInternal();
}

//------------------------------------------------------------------------

void BufferedOutputStream::flushInternal(void)
{
    int size = (m_lineStart) ? m_lineStart : m_numValid;
    if (!size)
        return;

    m_stream.write(m_buffer.getPtr(), size);
    m_numValid -= size;
    memmove(m_buffer.getPtr(), m_buffer.getPtr(size), m_numValid);
    m_lineStart = max(m_lineStart - size, 0);
    m_currOfs = max(m_currOfs - size, 0);
}

//------------------------------------------------------------------------

MemoryInputStream::~MemoryInputStream(void)
{
}

//------------------------------------------------------------------------

int MemoryInputStream::read(void* ptr, int size)
{
    int numRead = min(size, m_size - m_ofs);
    memcpy(ptr, m_ptr + m_ofs, numRead);
    m_ofs += numRead;
    return numRead;
}

//------------------------------------------------------------------------

MemoryOutputStream::~MemoryOutputStream(void)
{
}

//------------------------------------------------------------------------

void MemoryOutputStream::write(const void* ptr, int size)
{
    m_data.add((const U8*)ptr, size);
}

//------------------------------------------------------------------------

void MemoryOutputStream::flush(void)
{
}

//------------------------------------------------------------------------