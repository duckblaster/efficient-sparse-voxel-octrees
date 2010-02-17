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
#include "io/File.hpp"
#include "gui/Image.hpp"

namespace FW
{

//------------------------------------------------------------------------

class AviExporter
{
public:
                    AviExporter         (const String& fileName, Vec2i size, int fps);
                    ~AviExporter        (void);

    const Image&    getFrame            (void) const    { return m_frame; }
    Image&          getFrame            (void)          { return m_frame; }
    int             getFPS              (void) const    { return m_fps; }

    void            exportFrame         (void);
    void            flush               (void);

private:
    void            setTag              (int ofs, const char* tag);
    void            setS32              (int ofs, S32 value);
    void            writeHeader         (void);

private:
                    AviExporter         (const AviExporter&); // forbidden
    AviExporter&    operator=           (const AviExporter&); // forbidden

private:
    File            m_file;
    Vec2i           m_size;
    Image           m_frame;
    S32             m_fps;

    S32             m_lineBytes;
    S32             m_frameBytes;
    S32             m_numFrames;
    Array<U8>       m_buffer;
};

//------------------------------------------------------------------------
}
