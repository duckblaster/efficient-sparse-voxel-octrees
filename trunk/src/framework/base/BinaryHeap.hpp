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
#include "base/Array.hpp"

namespace FW
{
//------------------------------------------------------------------------

template <class T> class BinaryHeap
{
private:
    struct Item
    {
        T           value;
        S32         slot;       // -1 if the item does not exist
    };

public:
                    BinaryHeap  (void)                          : m_hasBeenBuilt(false) {}
                    BinaryHeap  (const BinaryHeap<T>& other)    : m_hasBeenBuilt(false) { set(other); }
                    ~BinaryHeap (void)                          {}

    int             numIndices  (void) const                    { return m_items.getSize(); }
    int             numItems    (void) const                    { return m_slots.getSize(); }
    bool            isEmpty     (void) const                    { return (numItems() == 0); }
    bool            contains    (int idx) const                 { return (idx >= 0 && idx < m_items.getSize() && m_items[idx].slot != -1); }
    const T&        get         (int idx) const                 { FW_ASSERT(contains(idx)); return m_items[idx].value; }

    void            clear       (void)                          { m_items.clear(); m_slots.clear(); m_hasBeenBuilt = false; }
    void            reset       (void)                          { m_items.reset(); m_slots.reset(); m_hasBeenBuilt = false; }
    void            set         (const BinaryHeap<T>& other)    { m_items = other.m_items; m_slots = other.m_slots; m_hasBeenBuilt = other.m_hasBeenBuilt; }
    void            add         (int idx, const T& value);
    T               remove      (int idx);

    int             getMinIndex (void);
    const T&        getMin      (void)                          { return get(getMinIndex()); }
    T               removeMin   (void)                          { return remove(getMinIndex()); }

    const T&        operator[]  (int idx) const                 { return get(idx); }
    BinaryHeap<T>&  operator=   (const BinaryHeap<T>& other)    { set(other); return *this; }

private:
    bool            heapify     (int parentSlot);
    void            adjust      (int idx, bool increased);

private:
    Array<Item>     m_items;
    Array<S32>      m_slots;
    bool            m_hasBeenBuilt;
};

//------------------------------------------------------------------------

template <class T> void BinaryHeap<T>::add(int idx, const T& value)
{
    FW_ASSERT(idx >= 0);

    // Ensure that the index exists.

    while (idx >= m_items.getSize())
        m_items.add().slot = -1;

    // Replace an existing item or add a new item in the last slot.

    bool increased = false;
    if (m_items[idx].slot != -1)
        increased = (m_items[idx].value < value);
    else
    {
        m_items[idx].slot = m_slots.getSize();
        m_slots.add(idx);
    }
    m_items[idx].value = value;

    // Adjust the slot.

    if (m_hasBeenBuilt)
        adjust(idx, increased);
}

//------------------------------------------------------------------------

template <class T> T BinaryHeap<T>::remove(int idx)
{
    // Not in the heap => ignore.

    if (!contains(idx))
        return T();

    // Will have less than two slots => no need to maintain heap property.

    if (m_slots.getSize() <= 2)
        m_hasBeenBuilt = false;

    // Remove.

    Item item = m_items[idx];
    m_items[idx].slot = -1;

    // Move the last item into the slot.

    int last = m_slots.removeLast();
    if (last != idx)
    {
        m_items[last].slot = item.slot;
        m_slots[item.slot] = last;

        // Adjust the slot.

        if (m_hasBeenBuilt)
            adjust(last, (item.value < m_items[last].value));
    }
    return item.value;
}

//------------------------------------------------------------------------

template <class T> int BinaryHeap<T>::getMinIndex(void)
{
    // Empty => ignore.

    if (isEmpty())
        return -1;

    // Not built and has at least two slots => build now.

    if (!m_hasBeenBuilt && m_slots.getSize() >= 2)
    {
        for (int i = (m_slots.getSize() - 2) >> 1; i >= 0; i--)
        {
            int idx = m_slots[i];
            while (heapify(m_items[idx].slot));
        }
        m_hasBeenBuilt = true;
    }

    // Return the root.

    return m_slots[0];
}

//------------------------------------------------------------------------

template <class T> bool BinaryHeap<T>::heapify(int parentSlot)
{
    FW_ASSERT(parentSlot >= 0 && parentSlot < m_slots.getSize());

    // Find the left-hand child.
    // No children => done.

    int childSlot = (parentSlot << 1) + 1;
    if (childSlot >= m_slots.getSize())
        return false;

    int child = m_slots[childSlot];

    // Right-hand child has a smaller value => use it instead.

    if (childSlot + 1 < m_slots.getSize())
    {
        int other = m_slots[childSlot + 1];
        if (m_items[other].value < m_items[child].value)
        {
            childSlot++;
            child = other;
        }
    }

    // The parent has the smallest value => done.

    int parent = m_slots[parentSlot];
    if (!(m_items[child].value < m_items[parent].value))
        return false;

    /* Swap the parent and child slots. */

    m_items[child].slot = parentSlot;
    m_items[parent].slot = childSlot;
    m_slots[parentSlot] = child;
    m_slots[childSlot] = parent;
    return true;
}

//------------------------------------------------------------------------

template <class T> void BinaryHeap<T>::adjust(int idx, bool increased)
{
    FW_ASSERT(contains(idx));
    if (increased)
        while (heapify(m_items[idx].slot));
    else
        while (m_items[idx].slot != 0 && heapify((m_items[idx].slot - 1) >> 1));
}

//------------------------------------------------------------------------
}
