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
 
#include "base/Math.hpp"

using namespace FW;

//------------------------------------------------------------------------

Vec4f Vec4f::fromABGR(U32 abgr)
{
    return Vec4f(
        (F32)(abgr & 0xFF) * (1.0f / 255.0f),
        (F32)((abgr >> 8) & 0xFF) * (1.0f / 255.0f),
        (F32)((abgr >> 16) & 0xFF) * (1.0f / 255.0f),
        (F32)(abgr >> 24) * (1.0f / 255.0f));
}

//------------------------------------------------------------------------

U32 Vec4f::toABGR(void) const
{
    return FW::clamp((int)(x * 255.0f + 0.5f), 0x00, 0xFF) |
           (FW::clamp((int)(y * 255.0f + 0.5f), 0x00, 0xFF) << 8) |
           (FW::clamp((int)(z * 255.0f + 0.5f), 0x00, 0xFF) << 16) |
           (FW::clamp((int)(w * 255.0f + 0.5f), 0x00, 0xFF) << 24);
}

//------------------------------------------------------------------------

Mat3f Mat4f::getXYZ(void) const
{
    Mat3f r;
    for (int i = 0; i < 3; i++)
        r.col(i) = Vec4f(col(i)).getXYZ();
    return r;
}

//------------------------------------------------------------------------

Mat4f Mat4f::fitToView(const Vec2f& pos, const Vec2f& size, const Vec2f& viewSize)
{
    FW_ASSERT(size.x != 0.0f && size.y != 0.0f);
    FW_ASSERT(viewSize.x != 0.0f && viewSize.y != 0.0f);

    return Mat4f()
        .preXlate(Vec3f(-pos - size * 0.5f, 0.0f))
        .preScale(Vec3f((viewSize / size).min(), 1.0f))
        .preScale(Vec3f(Vec2f(2.0f) / viewSize, 1.0f));
}

//------------------------------------------------------------------------

Mat4f Mat4f::perspective(F32 fov, F32 near, F32 far)
{
    F32 f = rcp(tan(fov * FW_PI / 360.0f));
    F32 d = rcp(near - far);

    Mat4f r;
    r.setRow(0, Vec4f(  f,      0.0f,   0.0f,               0.0f                    ));
    r.setRow(1, Vec4f(  0.0f,   f,      0.0f,               0.0f                    ));
    r.setRow(2, Vec4f(  0.0f,   0.0f,   (near + far) * d,   2.0f * near * far * d   ));
    r.setRow(3, Vec4f(  0.0f,   0.0f,   -1.0f,              0.0f                    ));
    return r;
}

//------------------------------------------------------------------------
