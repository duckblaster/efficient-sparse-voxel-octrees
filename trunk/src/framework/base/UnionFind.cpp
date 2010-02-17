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
 
#include "base/UnionFind.hpp"

using namespace FW;

//------------------------------------------------------------------------

int UnionFind::unionSets(int idxA, int idxB)
{
    // Grow the array.

    FW_ASSERT(idxA >= 0 && idxB >= 0);
    int oldSize = m_sets.getSize();
    int newSize = max(idxA, idxB) + 1;
    if (newSize > oldSize)
    {
        m_sets.resize(newSize);
        for (int i = oldSize; i < newSize; i++)
            m_sets[i] = i;
    }

    // Union the sets.

    int root = findSet(idxA);
    m_sets[findSet(idxB)] = root;
    return root;
}

//------------------------------------------------------------------------

int UnionFind::findSet(int idx) const
{
    // Out of the array => isolated.

    if (idx < 0 || idx >= m_sets.getSize())
        return idx;

    // Find the root set.

    int root = idx;
    for (;;)
    {
        int parent = m_sets[root];
        if (parent == root)
            break;
        root = parent;
    }

    // Update parent links to point directly to the root.

    for (;;)
    {
        int parent = m_sets[idx];
        if (parent == root)
            break;
        m_sets[idx] = root;
        idx = parent;
    }
    return root;
}

//------------------------------------------------------------------------
