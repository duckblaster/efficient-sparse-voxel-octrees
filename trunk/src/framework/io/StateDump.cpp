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
 
#include "io/StateDump.hpp"

using namespace FW;

//------------------------------------------------------------------------

void StateDump::readFromStream(InputStream& s)
{
    clear();
    S32 num;
    s >> num;
    for (int i = 0; i < num; i++)
    {
        String id;
        Array<U8>* data = new Array<U8>;
        s >> id >> *data;
        setInternal(data, id);
    }
}

//------------------------------------------------------------------------

void StateDump::writeToStream(OutputStream& s) const
{
    s << m_values.getSize();
    for (int i = m_values.firstSlot(); i != -1; i = m_values.nextSlot(i))
        s << m_values.getSlot(i).key << *m_values.getSlot(i).value;
}

//------------------------------------------------------------------------

void StateDump::clear(void)
{
    for (int i = m_values.firstSlot(); i != -1; i = m_values.nextSlot(i))
        delete m_values.getSlot(i).value;
    m_values.clear();
}

//------------------------------------------------------------------------

void StateDump::add(const StateDump& other)
{
    if (&other != this)
        for (int i = other.m_values.firstSlot(); i != -1; i = other.m_values.nextSlot(i))
            setInternal(new Array<U8>(*other.m_values.getSlot(i).value), other.m_values.getSlot(i).key);
}

//------------------------------------------------------------------------

const Array<U8>* StateDump::get(const String& id) const
{
    Array<U8>* const* data = m_values.search(xlateId(id));
    return (data) ? *data : NULL;
}

//------------------------------------------------------------------------

bool StateDump::get(void* ptr, int size, const String& id)
{
    FW_ASSERT(size >= 0);
    FW_ASSERT(ptr || !size);

    Array<U8>* const* data = m_values.search(xlateId(id));
    if (!data)
        return false;

    FW_ASSERT((*data)->getNumBytes() == size);
    memcpy(ptr, (*data)->getPtr(), size);
    return true;
}

//------------------------------------------------------------------------

void StateDump::set(const void* ptr, int size, const String& id)
{
    setInternal(new Array<U8>((const U8*)ptr, size), xlateId(id));
}

//------------------------------------------------------------------------

void StateDump::setInternal(Array<U8>* data, const String& id)
{
    if (m_values.contains(id))
        delete m_values.remove(id);
    if (data)
        m_values.add(id, data);
}

//------------------------------------------------------------------------
