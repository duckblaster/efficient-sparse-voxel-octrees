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

class Image;
class InputStream;
class OutputStream;

//------------------------------------------------------------------------

Image*  importBinaryImage   (InputStream& stream);
void    exportBinaryImage   (OutputStream& stream, const Image* image);

//------------------------------------------------------------------------
/*

Binary image file format v1
---------------------------

- the basic units of data are 32-bit little-endian ints and floats

BinaryImage
    0       7       struct  ImageHeader
    3       n*6     struct  array of ImageChannel (ImageHeader.numChannels)
    ?       ?       struct  image data (ImageHeader.width * ImageHeader.height * ImageHeader.bpp)
    ?

ImageHeader
    0       2       bytes   formatID (must be "BinImage")
    2       1       int     formatVersion (must be 1)
    3       1       int     width
    4       1       int     height
    5       1       int     bpp
    6       1       int     numChannels
    7

ImageChannel
    0       1       int     type (see ImageFormat::ChannelType)
    1       1       int     format (see ImageFormat::ChannelFormat)
    2       1       int     wordOfs
    3       1       int     wordSize
    4       1       int     fieldOfs
    5       1       int     fieldSize
    6

*/
//------------------------------------------------------------------------
}
