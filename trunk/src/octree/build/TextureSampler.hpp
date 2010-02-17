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
#include "3d/Texture.hpp"

namespace FW
{
//------------------------------------------------------------------------

class TextureSampler
{
public:
    struct Sample
    {
        Vec4f           avg;
        Vec4f           lo;
        Vec4f           hi;
    };

private:
    typedef Vector<U8, 4> Color;

    struct Range
    {
        Color           avg;
        Color           lo;
        Color           hi;
    };

    struct Level
    {
        Vec2i           size;
        S32             numBytes;
        Color*          colors;         // base level
        Range*          ranges;         // mipmaps
    };

public:
                        TextureSampler  (const Texture& tex);
                        ~TextureSampler (void);

    void                samplePoint     (Sample& res, const Vec2f& pos, F32 sizeInTexels) const;
    void                sampleRect      (Sample& res, const Vec2f& lo, const Vec2f& hi) const { samplePoint(res, (lo + hi) * 0.5f, ((hi - lo) * Vec2f(m_size)).max()); }

private:
    static inline void  encode          (Color& dst, const Vec4f& src)      { dst[0] = (U8)clamp((int)src.x, 0x00, 0xFF); dst[1] = (U8)clamp((int)src.y, 0x00, 0xFF); dst[2] = (U8)clamp((int)src.z, 0x00, 0xFF); dst[3] = (U8)clamp((int)src.w, 0x00, 0xFF); }
    static inline void  encode          (Range& dst, const Sample& src)     { encode(dst.avg, src.avg); encode(dst.lo, src.lo); encode(dst.hi, src.hi); }
    static inline void  decode          (Vec4f& dst, const Color& src)      { dst.x = (F32)src[0]; dst.y = (F32)src[1]; dst.z = (F32)src[2]; dst.w = (F32)src[3]; }
    static inline void  decode          (Sample& dst, const Color& src)     { decode(dst.avg, src); dst.lo = dst.avg; dst.hi = dst.avg; }
    static inline void  decode          (Sample& dst, const Range& src)     { decode(dst.avg, src.avg); decode(dst.lo, src.lo); decode(dst.hi, src.hi); }

    static void         lerpMinMax      (Sample& a, const Sample& b, F32 t);
    static void         lerpShrink      (Sample& a, const Sample& b, F32 t, F32 kernelSize);

    void                lookup          (Vec2f& posFrac, Sample& s00, Sample& s10, Sample& s01, Sample& s11, const Vec2f& pos, int lod) const;

private:
                        TextureSampler  (TextureSampler&); // forbidden
    TextureSampler&     operator=       (TextureSampler&); // forbidden

private:
    Vec2i               m_size;
    U8*                 m_data;
    Array<Level>        m_levels;
};

//------------------------------------------------------------------------
}
