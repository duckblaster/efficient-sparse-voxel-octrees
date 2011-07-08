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

class TextureAtlas
{
private:
    struct Item
    {
        Texture             texture;
        S32                 border;
        bool                wrap;
        Vec2i               size;
        Vec2i               pos;
    };

public:
                            TextureAtlas    (const ImageFormat& format = ImageFormat::ABGR_8888);
                            ~TextureAtlas   (void);

    void                    clear           (void);
    bool                    addTexture      (const Texture& tex, int border = 1, bool wrap = true);

    Vec2i                   getAtlasSize    (void)  { validate(); return m_atlasSize; }
    Vec2i                   getTexturePos   (const Texture& tex);
    const Texture&          getAtlasTexture (void)  { validate(); return m_atlasTexture; }

private:
                            TextureAtlas    (const TextureAtlas&); // forbidden
    TextureAtlas&           operator=       (const TextureAtlas&); // forbidden

private:
    void                    validate        (void);
    void                    layoutItems     (void);
    void                    createAtlas     (void);

    static int              compareVec2i   (void* data, int idxA, int idxB);
    static void             swapVec2i      (void* data, int idxA, int idxB);

private:
    ImageFormat             m_format;
    Array<Item>             m_items;
    Hash<const Image*, S32> m_itemHash;

    Vec2i                   m_atlasSize;
    Texture                 m_atlasTexture;
};

//------------------------------------------------------------------------
}
