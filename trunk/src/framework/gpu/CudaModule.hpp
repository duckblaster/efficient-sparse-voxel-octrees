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
#include "gpu/GLContext.hpp"

namespace FW
{
//------------------------------------------------------------------------

class Buffer;

//------------------------------------------------------------------------

class CudaModule
{
private:
    struct Global
    {
        String          name;
        Buffer*         buffer;
    };

public:
                        CudaModule          (const void* cubin);
                        ~CudaModule         (void);

    CUmodule            getHandle           (void) { return m_module; }
    Buffer&             getGlobal           (const String& name);
    void                updateGlobals       (bool async = false, CUstream stream = NULL); // copy to the device if modified

    CUfunction          getKernel           (const String& name, int paramSize = 0);
    void                setKernelParami     (CUfunction kernel, int offset, U32 value);
    void                setKernelParamf     (CUfunction kernel, int offset, F32 value);
    void                setKernelTexRef     (CUfunction kernel, const String& name, Buffer& buf, CUarray_format format, int numComponents);
    void                setKernelTexRef     (CUfunction kernel, const String& name, CUdeviceptr ptr, S64 size, CUarray_format format, int numComponents);
    void                launchKernel        (CUfunction kernel, const Vec2i& blockSize, const Vec2i& gridSize, bool async = false, CUstream stream = NULL);

    static void         staticInit          (void);
    static void         staticDeinit        (void);
    static bool         isAvailable         (void)      { staticInit(); return s_available; }
    static S64          getMemoryUsed       (void);
    static void         sync                (bool yield = true);
    static void         checkError          (const char* funcName, CUresult res);
    static const char*  decodeError         (CUresult res);

    static int          getComputeCapability(void); // e.g. 13 = 1.3
    static int          getDeviceAttribute  (CUdevice_attribute attrib);
    static void         setPreferL1OverShared(bool preferL1) { s_preferL1 = preferL1; }

private:
    static CUdevice     selectDevice        (void);
    static void         printDeviceInfo     (CUdevice device);

private:
                        CudaModule          (const CudaModule&); // forbidden
    CudaModule&         operator=           (const CudaModule&); // forbidden

private:
    static bool         s_inited;
    static bool         s_available;
    static CUdevice     s_device;
    static CUcontext    s_context;
    static CUevent      s_event;
    static bool         s_preferL1;

    CUmodule            m_module;
    Array<Global>       m_globals;
};

//------------------------------------------------------------------------
}
