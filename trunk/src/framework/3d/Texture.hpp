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
#include "gui/Image.hpp"
#include "base/Hash.hpp"
#include "base/Thread.hpp"

namespace FW
{
//------------------------------------------------------------------------

class Texture
{
private:
    struct Data
    {
        S32         refCount;
        Spinlock    refCountLock;

        String      id;
        bool        isInHash;
        Image*      image;
        GLuint      glTexture;
        CUarray     cudaArray;
        Texture*    nextMip;
    };

public:
                    Texture         (void)                                  : m_data(NULL) {}
                    Texture         (const Texture& other)                  : m_data(NULL) { set(other); }
                    Texture         (Image* image, const String& id = ""); // takes ownership of the image
                    ~Texture        (void)                                  { if (m_data) unreferData(m_data); }

    static Texture  find            (const String& id);
    static Texture  import          (const String& fileName);

    bool            exists          (void) const                            { return (m_data && m_data->image && m_data->image->getSize().min() > 0); }
    String          getID           (void) const                            { return (m_data) ? m_data->id : ""; }
    const Image*    getImage        (void) const                            { return (m_data) ? m_data->image : NULL; }
    Vec2i           getSize         (void) const                            { return (exists()) ? getImage()->getSize() : 0; }

    void            clear           (void)                                  { if (m_data) unreferData(m_data); m_data = NULL; }
    void            set             (const Texture& other);

    GLuint          getGLTexture    (const ImageFormat::ID desiredFormat = ImageFormat::ID_Max, bool generateMipmaps = true) const;
    CUarray         getCudaArray    (const ImageFormat::ID desiredFormat = ImageFormat::ID_Max) const;
    Texture         getMipLevel     (int level) const;

    Texture&        operator=       (const Texture& other)                  { set(other); return *this; }
    bool            operator==      (const Texture& other) const            { return (m_data == other.m_data); }
    bool            operator!=      (const Texture& other) const            { return (m_data != other.m_data); }

private:
    static Data*    findData        (const String& id);
    static Data*    createData      (const String& id);
    static void     referData       (Data* data);
    static void     unreferData     (Data* data);

private:
    static Hash<String, Data*>* s_hash;

    Data*           m_data;
};

//------------------------------------------------------------------------
}
