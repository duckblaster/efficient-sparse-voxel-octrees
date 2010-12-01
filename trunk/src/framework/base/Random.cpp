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
 
#include "base/Random.hpp"
#include "base/DLLImports.hpp"

using namespace FW;

//------------------------------------------------------------------------

#define FW_USE_MERSENNE_TWISTER 0

//------------------------------------------------------------------------
// Mersenne Twister
//------------------------------------------------------------------------

#if FW_USE_MERSENNE_TWISTER
extern "C"
{
#include "3rdparty/mt19937ar/mt19937ar_ctx.h"
}
#endif

//------------------------------------------------------------------------
// RANROT-A
//------------------------------------------------------------------------

class RanrotA
{
private:
    S32     p1;
    S32     p2;
    U32     buffer[11];

public:
    void reset(U32 seed)
    {
        if (seed == 0)
            seed--;

        for (int i = 0; i < (int)FW_ARRAY_SIZE(buffer); i++) 
        {
            seed ^= seed << 13; 
            seed ^= seed >> 17; 
            seed ^= seed << 5;
            buffer[i] = seed;
        }

        p1 = 0;  
        p2 = 7;

        for (int i = 0; i < (int)FW_ARRAY_SIZE(buffer); i++) 
            get();
    }

    U32 get(void)
    {
        U32 x = buffer[p1] + buffer[p2];
        x = (x << 13) | (x >> 19);
        buffer[p1] = x;

        p1--;
        p1 += (p1 >> 31) & FW_ARRAY_SIZE(buffer);
        p2--;
        p2 += (p2 >> 31) & FW_ARRAY_SIZE(buffer);
        return x;
    }
};

//------------------------------------------------------------------------
// Common functionality.
//------------------------------------------------------------------------

void Random::reset(void)
{
    LARGE_INTEGER ticks;
    if (!QueryPerformanceCounter(&ticks))
        failWin32Error("QueryPerformanceCounter");
    reset(ticks.LowPart);
}

//------------------------------------------------------------------------

void Random::reset(U32 seed)
{
    resetImpl(seed);

    m_normalF32Valid    = false;
    m_normalF32         = 0.0f;
    m_normalF64Valid    = false;
    m_normalF64         = 0.0;
}

//------------------------------------------------------------------------

void Random::reset(const Random& other)
{
    assignImpl(other);

    m_normalF32Valid    = other.m_normalF32Valid;
    m_normalF32         = other.m_normalF32;
    m_normalF64Valid    = other.m_normalF64Valid;
    m_normalF64         = other.m_normalF64;
}

//------------------------------------------------------------------------

int Random::read(void* ptr, int size)
{
    for (int i = 0; i < size; i++)
        ((U8*)ptr)[i] = (U8)getU32();
    return max(size, 0);
}

//------------------------------------------------------------------------

F32 Random::getF32Normal(void)
{
    m_normalF32Valid = (!m_normalF32Valid);
    if (!m_normalF32Valid)
        return m_normalF32;

    F32 a, b, c;
    do
    {
        a = (F32)getU32() * (2.0f / 4294967296.0f) + (1.0f / 4294967296.0f - 1.0f);
        b = (F32)getU32() * (2.0f / 4294967296.0f) + (1.0f / 4294967296.0f - 1.0f);
        c = a * a + b * b;
    }
    while (c >= 1.0f);

    c = sqrt(-2.0f * log(c) / c);
    m_normalF32 = b * c;
    return a * c;
}

//------------------------------------------------------------------------

F64 Random::getF64Normal(void)
{
    m_normalF64Valid = (!m_normalF64Valid);
    if (!m_normalF64Valid)
        return m_normalF64;

    F64 a, b, c;
    do
    {
        a = (F64)getU64() * (2.0 / 18446744073709551616.0) + (1.0 / 18446744073709551616.0 - 1.0);
        b = (F64)getU64() * (2.0 / 18446744073709551616.0) + (1.0 / 18446744073709551616.0 - 1.0);
        c = a * a + b * b;
    }
    while (c >= 1.0);

    c = sqrt(-2.0 * log(c) / c);
    m_normalF64 = b * c;
    return a * c;
}

//------------------------------------------------------------------------
// Implementation wrappers.
//------------------------------------------------------------------------

void Random::initImpl(void)
{
#if FW_USE_MERSENNE_TWISTER
    m_impl = (void*)new genrand;
#else
    m_impl = (void*)new RanrotA;
#endif
}

//------------------------------------------------------------------------

void Random::deinitImpl(void)
{
#if FW_USE_MERSENNE_TWISTER
    delete (genrand*)m_impl;
#else
    delete (RanrotA*)m_impl;
#endif
}

//------------------------------------------------------------------------

void Random::resetImpl(U32 seed)
{
#if FW_USE_MERSENNE_TWISTER
    init_genrand((genrand*)m_impl, seed);
#else
    ((RanrotA*)m_impl)->reset(seed);
#endif
}

//------------------------------------------------------------------------

void Random::assignImpl(const Random& other)
{
#if FW_USE_MERSENNE_TWISTER
    *(genrand*)m_impl = *(const genrand*)other.m_impl;
#else
    *(RanrotA*)m_impl = *(const RanrotA*)other.m_impl;
#endif
}

//------------------------------------------------------------------------

U32 Random::getImpl(void)
{
#if FW_USE_MERSENNE_TWISTER
    return genrand_int32((genrand*)m_impl);
#else
    return ((RanrotA*)m_impl)->get();
#endif
}

//------------------------------------------------------------------------
