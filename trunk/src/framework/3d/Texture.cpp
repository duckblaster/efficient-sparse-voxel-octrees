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
 
#include "3d/Texture.hpp"

using namespace FW;

//------------------------------------------------------------------------

Hash<String, Texture::Data*>* Texture::s_hash = NULL;

//------------------------------------------------------------------------

Texture::Texture(Image* image, const String& id)
{
    m_data = createData(id);
    m_data->image = image;
}

//------------------------------------------------------------------------

Texture Texture::find(const String& id)
{
    Texture tex;
    tex.m_data = findData(id);
    return tex;
}

//------------------------------------------------------------------------

Texture Texture::import(const String& fileName)
{
    Texture tex;
    tex.m_data = findData(fileName);
    if (!tex.m_data)
    {
        tex.m_data = createData(fileName);
        tex.m_data->image = importImage(fileName);
    }
    return tex;
}

//------------------------------------------------------------------------

GLuint Texture::getGLTexture(ImageFormat::ID desiredFormat, bool generateMipmaps) const
{
    FW_ASSERT(desiredFormat >= 0 && (desiredFormat < ImageFormat::ID_Generic || desiredFormat == ImageFormat::ID_Max));
    if (!m_data)
        return 0;

    if (m_data->glTexture == 0 && m_data->image)
        m_data->glTexture = m_data->image->createGLTexture(desiredFormat, generateMipmaps);
    return m_data->glTexture;
}

//------------------------------------------------------------------------

void Texture::set(const Texture& other)
{
    Data* old = m_data;
    m_data = other.m_data;
    if (m_data)
        referData(m_data);
    if (old)
        unreferData(old);
}

//------------------------------------------------------------------------

Texture::Data* Texture::findData(const String& id)
{
    Data** found = (s_hash) ? s_hash->search(id) : NULL;
    if (!found)
        return NULL;

    referData(*found);
    return *found;
}

//------------------------------------------------------------------------

Texture::Data* Texture::createData(const String& id)
{
    Data* data      = new Data;
    data->id        = id;
    data->refCount  = 1;
    data->isInHash  = false;
    data->image     = NULL;
    data->glTexture = 0;

    // Update hash.

    if (id.getLength())
    {
        if (!s_hash)
            s_hash = new Hash<String, Data*>;

        Data** old = s_hash->search(id);
        if (old)
        {
            s_hash->remove(id);
            (*old)->isInHash = false;
        }

        s_hash->add(id, data);
        data->isInHash = true;
    }
    return data;
}

//------------------------------------------------------------------------

void Texture::unreferData(Data* data)
{
    FW_ASSERT(data);

    // Decrease refcount.

    data->refCount--;
    if (data->refCount)
        return;

    // Remove from hash.

    if (data->isInHash)
    {
        FW_ASSERT(s_hash);
        s_hash->remove(data->id);
        if (!s_hash->getSize())
        {
            delete s_hash;
            s_hash = NULL;
        }
    }

    // Delete data.

    if (data->image)
        delete data->image;
    if (data->glTexture != 0)
        glDeleteTextures(1, &data->glTexture);
    delete data;
}

//------------------------------------------------------------------------
