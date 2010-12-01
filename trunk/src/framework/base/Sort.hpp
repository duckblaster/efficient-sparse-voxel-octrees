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
#include "base/Defs.hpp"

namespace FW
{
//------------------------------------------------------------------------

typedef int     (*SortCompareFunc)  (void* data, int idxA, int idxB);
typedef void    (*SortSwapFunc)     (void* data, int idxA, int idxB);

//------------------------------------------------------------------------

void sort       (int start, int end, void* data, SortCompareFunc compareFunc, SortSwapFunc swapFunc); // [start, end[
void sortMulticore  (int start, int end, void* data, SortCompareFunc compareFunc, SortSwapFunc swapFunc); // [start, end[

int  compareS32 (void* data, int idxA, int idxB);
void swapS32    (void* data, int idxA, int idxB);

int  compareF32 (void* data, int idxA, int idxB);
void swapF32    (void* data, int idxA, int idxB);

//------------------------------------------------------------------------
}
