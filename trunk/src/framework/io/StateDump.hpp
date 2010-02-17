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
#include "base/Hash.hpp"
#include "io/Stream.hpp"

namespace FW
{

//------------------------------------------------------------------------

class StateDump : public Serializable
{
public:
                                StateDump       (void)                          {}
                                StateDump       (const StateDump& other)        { add(other); }
    virtual                     ~StateDump      (void)                          { clear(); }

    virtual void                readFromStream  (InputStream& s);
    virtual void                writeToStream   (OutputStream& s) const;

    void                        clear           (void);
    void                        add             (const StateDump& other);
    void                        set             (const StateDump& other)        { if (&other != this) { clear(); add(other); } }

    void                        pushOwner       (const String& id)              { m_owners.add(xlateId(id + "::")); }
    void                        popOwner        (void)                          { m_owners.removeLast(); }

    bool                        has             (const String& id) const        { return m_values.contains(xlateId(id)); }
    const Array<U8>*            get             (const String& id) const;
    bool                        get             (void* ptr, int size, const String& id);
    template <class T> bool     get             (T& value, const String& id) const;
    template <class T> bool     get             (T& value, const String& id, const T& defValue) const;
    template <class T> T        get             (const String& id, const T& defValue) const;

    void                        set             (const void* ptr, int size, const String& id);
    template <class T> void     set             (const T& value, const String& id);
    void                        unset           (const String& id)              { setInternal(NULL, xlateId(id)); }

    StateDump&                  operator=       (const StateDump& other)        { set(other); return *this; }

private:
    String                      xlateId         (const String& id) const        { return (m_owners.getSize()) ? m_owners.getLast() + id : id; }
    void                        setInternal     (Array<U8>* data, const String& id); // takes ownership of data

private:
    Hash<String, Array<U8>* >   m_values;
    Array<String>               m_owners;

    mutable MemoryInputStream   m_memIn;
    MemoryOutputStream          m_memOut;
};

//------------------------------------------------------------------------

template <class T> bool StateDump::get(T& value, const String& id) const
{
    const Array<U8>* data = get(id);
    if (!data)
        return false;

    m_memIn.reset(*data);
    m_memIn >> value;
    FW_ASSERT(m_memIn.getOffset() == data->getSize());
    return true;
}

//------------------------------------------------------------------------

template <class T> bool StateDump::get(T& value, const String& id, const T& defValue) const
{
    if (get(value, id))
        return true;
    value = defValue;
    return false;
}

//------------------------------------------------------------------------

template <class T> T StateDump::get(const String& id, const T& defValue) const
{
    T value;
    get(value, id, defValue);
    return value;
}

//------------------------------------------------------------------------

template <class T> void StateDump::set(const T& value, const String& id)
{
    m_memOut.clear();
    m_memOut << value;
    Array<U8>& data = m_memOut.getData();
    set(data.getPtr(), data.getNumBytes(), id);
}

//------------------------------------------------------------------------
}
