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
 
#include "gpu/CudaModule.hpp"
#include "gpu/Buffer.hpp"
#include "base/Thread.hpp"
#include "base/Timer.hpp"
#include "gpu/CudaCompiler.hpp"

using namespace FW;

//------------------------------------------------------------------------

bool        CudaModule::s_inited        = false;
bool        CudaModule::s_available     = false;
CUdevice    CudaModule::s_device        = 0;
CUcontext   CudaModule::s_context       = NULL;
CUevent     CudaModule::s_startEvent    = NULL;
CUevent     CudaModule::s_endEvent      = NULL;
bool        CudaModule::s_preferL1      = true;

//------------------------------------------------------------------------

CudaModule::CudaModule(const void* cubin)
{
    staticInit();
    checkError("cuModuleLoadData", cuModuleLoadData(&m_module, cubin));
}

//------------------------------------------------------------------------

CudaModule::CudaModule(const String& cubinFile)
{
    staticInit();
    checkError("cuModuleLoad", cuModuleLoad(&m_module, cubinFile.getPtr()));
}

//------------------------------------------------------------------------

CudaModule::~CudaModule(void)
{
    for (int i = 0; i < m_globals.getSize(); i++)
        delete m_globals[i];

    checkError("cuModuleUnload", cuModuleUnload(m_module));
}

//------------------------------------------------------------------------

Buffer& CudaModule::getGlobal(const String& name)
{
    S32* found = m_globalHash.search(name);
    if (found)
        return *m_globals[*found];

    CUdeviceptr ptr;
    CUsize_t size;
    checkError("cuModuleGetGlobal", cuModuleGetGlobal(&ptr, &size, m_module, name.getPtr()));

    Buffer* buffer = new Buffer;
    buffer->wrapCuda(ptr, size);

    m_globalHash.add(name, m_globals.getSize());
    m_globals.add(buffer);
    return *buffer;
}

//------------------------------------------------------------------------

void CudaModule::updateGlobals(bool async, CUstream stream)
{
    for (int i = 0; i < m_globals.getSize(); i++)
        m_globals[i]->setOwner(Buffer::Cuda, true, async, stream);
}

//------------------------------------------------------------------------

CUfunction CudaModule::getKernel(const String& name, int paramSize)
{
    CUfunction kernel = NULL;
    cuModuleGetFunction(&kernel, m_module, name.getPtr());
    if (!kernel)
        cuModuleGetFunction(&kernel, m_module, (String("__globfunc_") + name).getPtr());
    if (kernel)
        checkError("cuParamSetSize", cuParamSetSize(kernel, paramSize));
    return kernel;
}

//------------------------------------------------------------------------

int CudaModule::setParami(CUfunction kernel, int offset, S32 value)
{
    if (kernel)
        checkError("cuParamSeti", cuParamSeti(kernel, offset, value));
    return sizeof(S32);
}

//------------------------------------------------------------------------

int CudaModule::setParamf(CUfunction kernel, int offset, F32 value)
{
    if (kernel)
        checkError("cuParamSetf", cuParamSetf(kernel, offset, value));
    return sizeof(F32);
}

//------------------------------------------------------------------------

int CudaModule::setParamPtr(CUfunction kernel, int offset, CUdeviceptr value)
{
    if (kernel)
        checkError("cuParamSetv", cuParamSetv(kernel, offset, &value, sizeof(CUdeviceptr)));
    return sizeof(CUdeviceptr);
}

//------------------------------------------------------------------------

CUtexref CudaModule::getTexRef(const String& name)
{
    S32* found = m_texRefHash.search(name);
    if (found)
        return m_texRefs[*found];

    CUtexref texRef;
    checkError("cuModuleGetTexRef", cuModuleGetTexRef(&texRef, m_module, name.getPtr()));

    m_texRefHash.add(name, m_texRefs.getSize());
    m_texRefs.add(texRef);
    return texRef;
}

//------------------------------------------------------------------------

void CudaModule::setTexRef(const String& name, Buffer& buf, CUarray_format format, int numComponents)
{
    setTexRef(name, buf.getCudaPtr(), buf.getSize(), format, numComponents);
}

//------------------------------------------------------------------------

void CudaModule::setTexRef(const String& name, CUdeviceptr ptr, S64 size, CUarray_format format, int numComponents)
{
    CUtexref texRef = getTexRef(name);
    checkError("cuTexRefSetFormat", cuTexRefSetFormat(texRef, format, numComponents));
    checkError("cuTexRefSetAddress", cuTexRefSetAddress(NULL, texRef, ptr, (U32)size));
}

//------------------------------------------------------------------------

void CudaModule::setTexRef(const String& name, CUarray cudaArray, bool wrap, bool bilinear, bool normalizedCoords, bool readAsInt)
{
    CUaddress_mode addressMode = (wrap) ? CU_TR_ADDRESS_MODE_WRAP : CU_TR_ADDRESS_MODE_CLAMP;
    CUfilter_mode filterMode = (bilinear) ? CU_TR_FILTER_MODE_LINEAR : CU_TR_FILTER_MODE_POINT;

    U32 flags = 0;
    if (normalizedCoords)
        flags |= CU_TRSF_NORMALIZED_COORDINATES;
    if (readAsInt)
        flags |= CU_TRSF_READ_AS_INTEGER;

    CUtexref texRef = getTexRef(name);
    for (int dim = 0; dim < 3; dim++)
        checkError("cuTexRefSetAddressMode", cuTexRefSetAddressMode(texRef, dim, addressMode));
    checkError("cuTexRefSetFilterMode", cuTexRefSetFilterMode(texRef, filterMode));
    checkError("cuTexRefSetFlags", cuTexRefSetFlags(texRef, flags));
    checkError("cuTexRefSetArray", cuTexRefSetArray(texRef, cudaArray, CU_TRSA_OVERRIDE_FORMAT));
}

//------------------------------------------------------------------------

void CudaModule::unsetTexRef(const String& name)
{
    CUtexref texRef = getTexRef(name);
    checkError("cuTexRefSetAddress", cuTexRefSetAddress(NULL, texRef, NULL, 0));
}

//------------------------------------------------------------------------

void CudaModule::updateTexRefs(CUfunction kernel)
{
    for (int i = 0; i < m_texRefs.getSize(); i++)
        checkError("cuParamSetTexRef", cuParamSetTexRef(kernel, CU_PARAM_TR_DEFAULT, m_texRefs[i]));
}

//------------------------------------------------------------------------

void CudaModule::launchKernel(CUfunction kernel, const Vec2i& blockSize, const Vec2i& gridSize, bool async, CUstream stream)
{
    if (!kernel)
        fail("CudaModule: No kernel specified!");

#if (CUDA_VERSION >= 3000)
    if (isAvailable_cuFuncSetCacheConfig())
        checkError("cuFuncSetCacheConfig", cuFuncSetCacheConfig(kernel,
            (s_preferL1) ? CU_FUNC_CACHE_PREFER_L1 : CU_FUNC_CACHE_PREFER_SHARED));
#endif

    updateGlobals();
    updateTexRefs(kernel);
    checkError("cuFuncSetBlockShape", cuFuncSetBlockShape(kernel, blockSize.x, blockSize.y, 1));
    if (async && isAvailable_cuLaunchGridAsync())
        checkError("cuLaunchGridAsync", cuLaunchGridAsync(kernel, gridSize.x, gridSize.y, stream));
    else
        checkError("cuLaunchGrid", cuLaunchGrid(kernel, gridSize.x, gridSize.y));
}

//------------------------------------------------------------------------

F32 CudaModule::launchKernelTimed(CUfunction kernel, const Vec2i& blockSize, const Vec2i& gridSize, bool async, CUstream stream, bool yield)
{
    // Update globals before timing.

    updateGlobals();
    updateTexRefs(kernel);
    sync(false);

    // Events not supported => use CPU-based timer.

    if (!s_startEvent)
    {
        Timer timer(true);
        launchKernel(kernel, blockSize, gridSize, async, stream);
        sync(false); // spin for more accurate timing
        return timer.getElapsed();
    }

    // Use events.

    checkError("cuEventRecord", cuEventRecord(s_startEvent, NULL));
    launchKernel(kernel, blockSize, gridSize, async, stream);
    checkError("cuEventRecord", cuEventRecord(s_endEvent, NULL));
    sync(yield);

    F32 time;
    checkError("cuEventElapsedTime", cuEventElapsedTime(&time, s_startEvent, s_endEvent));
    return time * 1.0e-3f;
}

//------------------------------------------------------------------------

void CudaModule::staticInit(void)
{
    if (s_inited)
        return;
    s_inited = true;
    s_available = false;

    if (!isAvailable_cuInit())
        return;

    CUresult res = cuInit(0);
    if (res != CUDA_SUCCESS)
    {
        if (res != CUDA_ERROR_NO_DEVICE)
            checkError("cuInit", res);
        return;
    }

    s_available = true;
    s_device = selectDevice();
    printDeviceInfo(s_device);

    U32 flags = 0;
    flags |= CU_CTX_SCHED_SPIN; // use sync() if you want to yield
#if (CUDA_VERSION >= 2030)
    if (getDriverVersion() >= 23)
        flags |= CU_CTX_LMEM_RESIZE_TO_MAX; // reduce launch overhead with large localmem
#endif

    if (!isAvailable_cuGLCtxCreate())
        checkError("cuCtxCreate", cuCtxCreate(&s_context, flags, s_device));
    else
    {
        GLContext::staticInit();
        checkError("cuGLCtxCreate", cuGLCtxCreate(&s_context, flags, s_device));
    }

    if (isAvailable_cuEventCreate())
    {
        checkError("cuEventCreate", cuEventCreate(&s_startEvent, 0));
        checkError("cuEventCreate", cuEventCreate(&s_endEvent, 0));
    }
}

//------------------------------------------------------------------------

void CudaModule::staticDeinit(void)
{
    if (!s_inited)
        return;
    s_inited = false;

    if (s_startEvent)
        checkError("cuEventDestroy", cuEventDestroy(s_startEvent));
    s_startEvent = NULL;

    if (s_endEvent)
        checkError("cuEventDestroy", cuEventDestroy(s_endEvent));
    s_endEvent = NULL;

    if (s_context)
        checkError("cuCtxDestroy", cuCtxDestroy(s_context));
    s_context = NULL;

    s_device = NULL;
}

//------------------------------------------------------------------------

S64 CudaModule::getMemoryUsed(void)
{
    staticInit();
    if (!s_available)
        return 0;

    CUsize_t free = 0;
    CUsize_t total = 0;
    cuMemGetInfo(&free, &total);
    return total - free;
}

//------------------------------------------------------------------------

void CudaModule::sync(bool yield)
{
    if (!s_inited)
        return;

    if (!yield || !s_endEvent)
    {
            checkError("cuCtxSynchronize", cuCtxSynchronize());
        return;
    }

    checkError("cuEventRecord", cuEventRecord(s_endEvent, NULL));
    for (;;)
    {
        CUresult res = cuEventQuery(s_endEvent);
        if (res != CUDA_ERROR_NOT_READY)
        {
            checkError("cuEventQuery", res);
            break;
        }
        Thread::yield();
    }
}

//------------------------------------------------------------------------

const char* CudaModule::decodeError(CUresult res)
{
    const char* error;
    switch (res)
    {
    case CUDA_SUCCESS:                              error = "No error"; break;
    case CUDA_ERROR_INVALID_VALUE:                  error = "Invalid value"; break;
    case CUDA_ERROR_OUT_OF_MEMORY:                  error = "Out of memory"; break;
    case CUDA_ERROR_NOT_INITIALIZED:                error = "Not initialized"; break;
    case CUDA_ERROR_DEINITIALIZED:                  error = "Deinitialized"; break;
    case CUDA_ERROR_NO_DEVICE:                      error = "No device"; break;
    case CUDA_ERROR_INVALID_DEVICE:                 error = "Invalid device"; break;
    case CUDA_ERROR_INVALID_IMAGE:                  error = "Invalid image"; break;
    case CUDA_ERROR_INVALID_CONTEXT:                error = "Invalid context"; break;
    case CUDA_ERROR_CONTEXT_ALREADY_CURRENT:        error = "Context already current"; break;
    case CUDA_ERROR_MAP_FAILED:                     error = "Map failed"; break;
    case CUDA_ERROR_UNMAP_FAILED:                   error = "Unmap failed"; break;
    case CUDA_ERROR_ARRAY_IS_MAPPED:                error = "Array is mapped"; break;
    case CUDA_ERROR_ALREADY_MAPPED:                 error = "Already mapped"; break;
    case CUDA_ERROR_NO_BINARY_FOR_GPU:              error = "No binary for GPU"; break;
    case CUDA_ERROR_ALREADY_ACQUIRED:               error = "Already acquired"; break;
    case CUDA_ERROR_NOT_MAPPED:                     error = "Not mapped"; break;
    case CUDA_ERROR_INVALID_SOURCE:                 error = "Invalid source"; break;
    case CUDA_ERROR_FILE_NOT_FOUND:                 error = "File not found"; break;
    case CUDA_ERROR_INVALID_HANDLE:                 error = "Invalid handle"; break;
    case CUDA_ERROR_NOT_FOUND:                      error = "Not found"; break;
    case CUDA_ERROR_NOT_READY:                      error = "Not ready"; break;
    case CUDA_ERROR_LAUNCH_FAILED:                  error = "Launch failed"; break;
    case CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES:        error = "Launch out of resources"; break;
    case CUDA_ERROR_LAUNCH_TIMEOUT:                 error = "Launch timeout"; break;
    case CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING:  error = "Launch incompatible texturing"; break;
    case CUDA_ERROR_UNKNOWN:                        error = "Unknown error"; break;
    default:                                        error = "Unknown error"; break;
    }
    return error;
}

//------------------------------------------------------------------------

void CudaModule::checkError(const char* funcName, CUresult res)
{
    if (res != CUDA_SUCCESS)
        fail("%s() failed: %s!", funcName, decodeError(res));
}

//------------------------------------------------------------------------

int CudaModule::getDriverVersion(void)
{
    int version = 2010;
#if (CUDA_VERSION >= 2020)
    if (isAvailable_cuDriverGetVersion())
        cuDriverGetVersion(&version);
#endif
    version /= 10;
    return version / 10 + version % 10;
}

//------------------------------------------------------------------------

int CudaModule::getComputeCapability(void)
{
    staticInit();
    if (!s_available)
        return 0;

    int major;
    int minor;
    checkError("cuDeviceComputeCapability", cuDeviceComputeCapability(&major, &minor, s_device));
    return major * 10 + minor;
}

//------------------------------------------------------------------------

int CudaModule::getDeviceAttribute(CUdevice_attribute attrib)
{
    staticInit();
    if (!s_available)
        return 0;

    int value;
    checkError("cuDeviceGetAttribute", cuDeviceGetAttribute(&value, attrib, s_device));
    return value;
}

//------------------------------------------------------------------------

CUdevice CudaModule::selectDevice(void)
{
    int numDevices;
    CUdevice device = 0;
    S32 bestScore = FW_S32_MIN;
    checkError("cuDeviceGetCount", cuDeviceGetCount(&numDevices));

    for (int i = 0; i < numDevices; i++)
    {
        CUdevice dev;
        checkError("cuDeviceGet", cuDeviceGet(&dev, i));

        int clockRate;
        int numProcessors;
        checkError("cuDeviceGetAttribute", cuDeviceGetAttribute(&clockRate, CU_DEVICE_ATTRIBUTE_CLOCK_RATE, dev));
        checkError("cuDeviceGetAttribute", cuDeviceGetAttribute(&numProcessors, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, dev));

        S32 score = clockRate * numProcessors;
        if (score > bestScore)
        {
            device = dev;
            bestScore = score;
        }
    }

    if (bestScore == FW_S32_MIN)
        fail("No appropriate CUDA device found!");
    return device;
}

//------------------------------------------------------------------------

void CudaModule::printDeviceInfo(CUdevice device)
{
    static const struct
    {
        CUdevice_attribute  attrib;
        const char*         name;
    } attribs[] =
    {
#define A(NAME) { CU_DEVICE_ATTRIBUTE_ ## NAME, #NAME }
        A(MAX_THREADS_PER_BLOCK),
        A(MAX_BLOCK_DIM_X),
        A(MAX_BLOCK_DIM_Y),
        A(MAX_BLOCK_DIM_Z),
        A(MAX_GRID_DIM_X),
        A(MAX_GRID_DIM_Y),
        A(MAX_GRID_DIM_Z),
        A(SHARED_MEMORY_PER_BLOCK),
        A(TOTAL_CONSTANT_MEMORY),
        A(WARP_SIZE),
        A(MAX_PITCH),
        A(REGISTERS_PER_BLOCK),
        A(CLOCK_RATE),
        A(TEXTURE_ALIGNMENT),
        A(GPU_OVERLAP),
        A(MULTIPROCESSOR_COUNT),
//        A(KERNEL_EXEC_TIMEOUT),
#undef A
    };

    char name[256];
    int major;
    int minor;
    CUsize_t memory;

    checkError("cuDeviceGetName", cuDeviceGetName(name, FW_ARRAY_SIZE(name) - 1, device));
    checkError("cuDeviceComputeCapability", cuDeviceComputeCapability(&major, &minor, device));
    checkError("cuDeviceTotalMem", cuDeviceTotalMem(&memory, device));
    name[FW_ARRAY_SIZE(name) - 1] = '\0';

    printf("\n");
    printf("%-24s%s\n", "CUDA device", name);
    printf("%-24s%d.%d\n", "Compute capability", major, minor);
    printf("%-24s%d megs\n", "Total memory", memory >> 20);

    for (int i = 0; i < (int)FW_ARRAY_SIZE(attribs); i++)
    {
        int value;
        checkError("cuDeviceGetAttribute", cuDeviceGetAttribute(&value, attribs[i].attrib, device));
        printf("%-24s%d\n", attribs[i].name, value);
    }
    printf("\n");
}

//------------------------------------------------------------------------
