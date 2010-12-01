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
#include "base/Math.hpp"
#include "io/Stream.hpp"

namespace FW
{
//------------------------------------------------------------------------

class Random : public InputStream
{
public:
                    Random          (void)                      { initImpl(); reset(); }
    explicit        Random          (U32 seed)                  { initImpl(); reset(seed); }
                    Random          (const Random& other)       { initImpl(); reset(other); }
                    ~Random         (void)                      { deinitImpl(); }

    void            reset           (void);
    void            reset           (U32 seed);
    void            reset           (const Random& other);

    virtual int     read            (void* ptr, int size);
    Random&         operator=       (const Random& other)       { reset(other); return *this; }

    U32             getU32          (void)                      { return getImpl(); }
    U32             getU32          (U32 hi)                    { return (hi == 0) ? 0 : (getU32() % hi); }
    U32             getU32          (U32 lo, U32 hi)            { return (hi <= lo) ? lo : (getU32() % (hi - lo) + lo); }
    S32             getS32          (void)                      { return getImpl(); }
    S32             getS32          (S32 hi)                    { return (hi <= 0) ? 0 : (getU32() % (U32)hi); }
    S32             getS32          (S32 lo, S32 hi)            { return (hi <= lo) ? lo : (getU32() % (U32)(hi - lo) + lo); }
    F32             getF32          (void)                      { return (F32)getU32() * (1.0f / 4294967296.0f); }
    F32             getF32          (F32 lo, F32 hi)            { return getF32() * (hi - lo) + lo; }
    U64             getU64          (void)                      { return getU32() | ((U64)getU32() << 32); }
    U64             getU64          (U64 hi)                    { return (hi == 0) ? 0 : (getU64() % hi); }
    U64             getU64          (U64 lo, U64 hi)            { return (hi <= lo) ? lo : (getU64() % (hi - lo) + lo); }
    S64             getS64          (S64 hi)                    { return (hi <= 0) ? 0 : (getU64() % (U64)hi); }
    S64             getS64          (S64 lo, S64 hi)            { return (hi <= lo) ? lo : (getU64() % (U64)(hi - lo) + lo); }
    F64             getF64          (void)                      { return (F64)getU64() * (1.0 / 18446744073709551616.0); }
    F64             getF64          (F64 lo, F64 hi)            { return getF64() * (hi - lo) + lo; }

    F32             getF32Exp       (void)                      { return -log(getF32() + (1.0f / 4294967296.0f)); }
    F32             getF32Exp       (F32 deviation)             { return getF32Exp() * deviation; }
    F32             getF32Exp       (F32 mean, F32 deviation)   { return getF32Exp() * deviation + mean; }
    F64             getF64Exp       (void)                      { return -log(getF64() + (1.0 / 18446744073709551616.0)); }
    F64             getF64Exp       (F64 deviation)             { return getF64Exp() * deviation; }
    F64             getF64Exp       (F64 mean, F64 deviation)   { return getF64Exp() * deviation + mean; }

    F32             getF32Normal    (void);
    F32             getF32Normal    (F32 deviation)             { return getF32Normal() * deviation; }
    F32             getF32Normal    (F32 mean, F32 deviation)   { return getF32Normal() * deviation + mean; }
    F64             getF64Normal    (void);
    F64             getF64Normal    (F64 deviation)             { return getF64Normal() * deviation; }
    F64             getF64Normal    (F64 mean, F64 deviation)   { return getF64Normal() * deviation + mean; }

    Vec2f           getVec2f        (void)                      { return Vec2f(getF32(), getF32()); }
    Vec2f           getVec2f        (F32 lo, F32 hi)            { return Vec2f(getF32(lo, hi), getF32(lo, hi)); }
    Vec3f           getVec3f        (void)                      { return Vec3f(getF32(), getF32(), getF32()); }
    Vec3f           getVec3f        (F32 lo, F32 hi)            { return Vec3f(getF32(lo, hi), getF32(lo, hi), getF32(lo, hi)); }
    Vec4f           getVec4f        (void)                      { return Vec4f(getF32(), getF32(), getF32(), getF32()); }
    Vec4f           getVec4f        (F32 lo, F32 hi)            { return Vec4f(getF32(lo, hi), getF32(lo, hi), getF32(lo, hi), getF32(lo, hi)); }

    Vec2d           getVec2d        (void)                      { return Vec2d(getF64(), getF64()); }
    Vec2d           getVec2d        (F64 lo, F64 hi)            { return Vec2d(getF64(lo, hi), getF64(lo, hi)); }
    Vec3d           getVec3d        (void)                      { return Vec3d(getF64(), getF64(), getF64()); }
    Vec3d           getVec3d        (F64 lo, F64 hi)            { return Vec3d(getF64(lo, hi), getF64(lo, hi), getF64(lo, hi)); }
    Vec4d           getVec4d        (void)                      { return Vec4d(getF64(), getF64(), getF64(), getF64()); }
    Vec4d           getVec4d        (F64 lo, F64 hi)            { return Vec4d(getF64(lo, hi), getF64(lo, hi), getF64(lo, hi), getF64(lo, hi)); }

private:
    void            initImpl        (void);
    void            deinitImpl      (void);
    void            resetImpl       (U32 seed);
    void            assignImpl      (const Random& other);
    U32             getImpl         (void);

private:
    void*           m_impl;
    bool            m_normalF32Valid;
    F32             m_normalF32;
    bool            m_normalF64Valid;
    F64             m_normalF64;
};

//------------------------------------------------------------------------
}
