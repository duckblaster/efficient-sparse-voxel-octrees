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
 
//------------------------------------------------------------------------
// CUDA Driver API
//------------------------------------------------------------------------

// CUDA 2.1

FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuInit,                                 (unsigned int Flags), (Flags))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuDeviceGet,                            (CUdevice* device, int ordinal), (device, ordinal))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuDeviceGetCount,                       (int* count), (count))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuDeviceGetName,                        (char* name, int len, CUdevice dev), (name, len, dev))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuDeviceComputeCapability,              (int* major, int* minor, CUdevice dev), (major, minor, dev))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuDeviceGetProperties,                  (CUdevprop* prop, CUdevice dev), (prop, dev))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuDeviceGetAttribute,                   (int* pi, CUdevice_attribute attrib, CUdevice dev), (pi, attrib, dev))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuCtxDestroy,                           (CUcontext ctx), (ctx))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuCtxAttach,                            (CUcontext* pctx, unsigned int flags), (pctx, flags))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuCtxDetach,                            (CUcontext ctx), (ctx))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuCtxPushCurrent,                       (CUcontext ctx), (ctx))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuCtxPopCurrent,                        (CUcontext* pctx), (pctx))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuCtxGetDevice,                         (CUdevice* device), (device))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuCtxSynchronize,                       (void), ())
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuModuleLoad,                           (CUmodule* module, const char* fname), (module, fname))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuModuleLoadData,                       (CUmodule* module, const void* image), (module, image))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuModuleLoadDataEx,                     (CUmodule* module, const void* image, unsigned int numOptions, CUjit_option* options, void** optionValues), (module, image, numOptions, options, optionValues))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuModuleLoadFatBinary,                  (CUmodule* module, const void* fatCubin), (module, fatCubin))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuModuleUnload,                         (CUmodule hmod), (hmod))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuModuleGetFunction,                    (CUfunction* hfunc, CUmodule hmod, const char* name), (hfunc, hmod, name))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuModuleGetTexRef,                      (CUtexref* pTexRef, CUmodule hmod, const char* name), (pTexRef, hmod, name))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemFreeHost,                          (void* p), (p))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuFuncSetBlockShape,                    (CUfunction hfunc, int x, int y, int z), (hfunc, x, y, z))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuFuncSetSharedSize,                    (CUfunction hfunc, unsigned int bytes), (hfunc, bytes))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuArrayDestroy,                         (CUarray hArray), (hArray))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuTexRefCreate,                         (CUtexref* pTexRef), (pTexRef))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuTexRefDestroy,                        (CUtexref hTexRef), (hTexRef))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuTexRefSetArray,                       (CUtexref hTexRef, CUarray hArray, unsigned int Flags), (hTexRef, hArray, Flags))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuTexRefSetFormat,                      (CUtexref hTexRef, CUarray_format fmt, int NumPackedComponents), (hTexRef, fmt, NumPackedComponents))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuTexRefSetAddressMode,                 (CUtexref hTexRef, int dim, CUaddress_mode am), (hTexRef, dim, am))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuTexRefSetFilterMode,                  (CUtexref hTexRef, CUfilter_mode fm), (hTexRef, fm))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuTexRefSetFlags,                       (CUtexref hTexRef, unsigned int Flags), (hTexRef, Flags))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuTexRefGetArray,                       (CUarray* phArray, CUtexref hTexRef), (phArray, hTexRef))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuTexRefGetAddressMode,                 (CUaddress_mode* pam, CUtexref hTexRef, int dim), (pam, hTexRef, dim))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuTexRefGetFilterMode,                  (CUfilter_mode* pfm, CUtexref hTexRef), (pfm, hTexRef))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuTexRefGetFormat,                      (CUarray_format* pFormat, int* pNumChannels, CUtexref hTexRef), (pFormat, pNumChannels, hTexRef))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuTexRefGetFlags,                       (unsigned int* pFlags, CUtexref hTexRef), (pFlags, hTexRef))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuParamSetSize,                         (CUfunction hfunc, unsigned int numbytes), (hfunc, numbytes))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuParamSeti,                            (CUfunction hfunc, int offset, unsigned int value), (hfunc, offset, value))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuParamSetf,                            (CUfunction hfunc, int offset, float value), (hfunc, offset, value))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuParamSetv,                            (CUfunction hfunc, int offset, void* ptr, unsigned int numbytes), (hfunc, offset, ptr, numbytes))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuParamSetTexRef,                       (CUfunction hfunc, int texunit, CUtexref hTexRef), (hfunc, texunit, hTexRef))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuLaunch,                               (CUfunction f), (f))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuLaunchGrid,                           (CUfunction f, int grid_width, int grid_height), (f, grid_width, grid_height))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuLaunchGridAsync,                      (CUfunction f, int grid_width, int grid_height, CUstream hStream), (f, grid_width, grid_height, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuEventCreate,                          (CUevent* phEvent, unsigned int Flags), (phEvent, Flags))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuEventRecord,                          (CUevent hEvent, CUstream hStream), (hEvent, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuEventQuery,                           (CUevent hEvent), (hEvent))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuEventSynchronize,                     (CUevent hEvent), (hEvent))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuEventDestroy,                         (CUevent hEvent), (hEvent))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuEventElapsedTime,                     (float* pMilliseconds, CUevent hStart, CUevent hEnd), (pMilliseconds, hStart, hEnd))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuStreamCreate,                         (CUstream* phStream, unsigned int Flags), (phStream, Flags))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuStreamQuery,                          (CUstream hStream), (hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuStreamSynchronize,                    (CUstream hStream), (hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuStreamDestroy,                        (CUstream hStream), (hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGLInit,                               (void), ())
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGLRegisterBufferObject,               (GLuint bufferobj), (bufferobj))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGLUnmapBufferObject,                  (GLuint bufferobj), (bufferobj))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGLUnregisterBufferObject,             (GLuint bufferobj), (bufferobj))

#if (CUDA_VERSION < 3020)
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuDeviceTotalMem,                       (unsigned int* bytes, CUdevice dev), (bytes, dev))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuCtxCreate,                            (CUcontext* pctx, unsigned int flags, CUdevice dev), (pctx, flags, dev))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuModuleGetGlobal,                      (CUdeviceptr* dptr, unsigned int* bytes, CUmodule hmod, const char* name), (dptr, bytes, hmod, name))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemGetInfo,                           (unsigned int* free, unsigned int* total), (free, total))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemAlloc,                             (CUdeviceptr* dptr, unsigned int bytesize), (dptr, bytesize))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemAllocPitch,                        (CUdeviceptr* dptr, unsigned int* pPitch, unsigned int WidthInBytes, unsigned int Height, unsigned int ElementSizeBytes), (dptr, pPitch, WidthInBytes, Height, ElementSizeBytes))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemFree,                              (CUdeviceptr dptr), (dptr))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemGetAddressRange,                   (CUdeviceptr* pbase, unsigned int* psize, CUdeviceptr dptr), (pbase, psize, dptr))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemAllocHost,                         (void** pp, unsigned int bytesize), (pp, bytesize))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpyHtoD,                           (CUdeviceptr dstDevice, const void* srcHost, unsigned int ByteCount), (dstDevice, srcHost, ByteCount))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpyDtoH,                           (void* dstHost, CUdeviceptr srcDevice, unsigned int ByteCount), (dstHost, srcDevice, ByteCount))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpyDtoD,                           (CUdeviceptr dstDevice, CUdeviceptr srcDevice, unsigned int ByteCount), (dstDevice, srcDevice, ByteCount))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpyDtoA,                           (CUarray dstArray, unsigned int dstIndex, CUdeviceptr srcDevice, unsigned int ByteCount), (dstArray, dstIndex, srcDevice, ByteCount))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpyAtoD,                           (CUdeviceptr dstDevice, CUarray hSrc, unsigned int SrcIndex, unsigned int ByteCount), (dstDevice, hSrc, SrcIndex, ByteCount))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpyHtoA,                           (CUarray dstArray, unsigned int dstIndex, const void* pSrc, unsigned int ByteCount), (dstArray, dstIndex, pSrc, ByteCount))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpyAtoH,                           (void* dstHost, CUarray srcArray, unsigned int srcIndex, unsigned int ByteCount), (dstHost, srcArray, srcIndex, ByteCount))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpyAtoA,                           (CUarray dstArray, unsigned int dstIndex, CUarray srcArray, unsigned int srcIndex, unsigned int ByteCount), (dstArray, dstIndex, srcArray, srcIndex, ByteCount))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpyHtoAAsync,                      (CUarray dstArray, unsigned int dstIndex, const void* pSrc, unsigned int ByteCount, CUstream hStream), (dstArray, dstIndex, pSrc, ByteCount, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpyAtoHAsync,                      (void* dstHost, CUarray srcArray, unsigned int srcIndex, unsigned int ByteCount, CUstream hStream), (dstHost, srcArray, srcIndex, ByteCount, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpy2D,                             (const CUDA_MEMCPY2D* pCopy), (pCopy))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpy2DUnaligned,                    (const CUDA_MEMCPY2D* pCopy), (pCopy))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpy3D,                             (const CUDA_MEMCPY3D* pCopy), (pCopy))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpyHtoDAsync,                      (CUdeviceptr dstDevice, const void* srcHost, unsigned int ByteCount, CUstream hStream), (dstDevice, srcHost, ByteCount, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpyDtoHAsync,                      (void* dstHost, CUdeviceptr srcDevice, unsigned int ByteCount, CUstream hStream), (dstHost, srcDevice, ByteCount, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpy2DAsync,                        (const CUDA_MEMCPY2D* pCopy, CUstream hStream), (pCopy, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpy3DAsync,                        (const CUDA_MEMCPY3D* pCopy, CUstream hStream), (pCopy, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemsetD8,                             (CUdeviceptr dstDevice, unsigned char uc, unsigned int N), (dstDevice, uc, N))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemsetD16,                            (CUdeviceptr dstDevice, unsigned short us, unsigned int N), (dstDevice, us, N))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemsetD32,                            (CUdeviceptr dstDevice, unsigned int ui, unsigned int N), (dstDevice, ui, N))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemsetD2D8,                           (CUdeviceptr dstDevice, unsigned int dstPitch, unsigned char uc, unsigned int Width, unsigned int Height), (dstDevice, dstPitch, uc, Width, Height))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemsetD2D16,                          (CUdeviceptr dstDevice, unsigned int dstPitch, unsigned short us, unsigned int Width, unsigned int Height), (dstDevice, dstPitch, us, Width, Height))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemsetD2D32,                          (CUdeviceptr dstDevice, unsigned int dstPitch, unsigned int ui, unsigned int Width, unsigned int Height), (dstDevice, dstPitch, ui, Width, Height))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuArrayCreate,                          (CUarray* pHandle, const CUDA_ARRAY_DESCRIPTOR* pAllocateArray), (pHandle, pAllocateArray))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuArrayGetDescriptor,                   (CUDA_ARRAY_DESCRIPTOR* pArrayDescriptor, CUarray hArray), (pArrayDescriptor, hArray))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuArray3DCreate,                        (CUarray* pHandle, const CUDA_ARRAY3D_DESCRIPTOR* pAllocateArray), (pHandle, pAllocateArray))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuArray3DGetDescriptor,                 (CUDA_ARRAY3D_DESCRIPTOR* pArrayDescriptor, CUarray hArray), (pArrayDescriptor, hArray))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuTexRefSetAddress,                     (unsigned int* ByteOffset, CUtexref hTexRef, CUdeviceptr dptr, unsigned int bytes), (ByteOffset, hTexRef, dptr, bytes))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuTexRefGetAddress,                     (CUdeviceptr* pdptr, CUtexref hTexRef), (pdptr, hTexRef))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGLCtxCreate,                          (CUcontext* pCtx, unsigned int Flags, CUdevice device), (pCtx, Flags, device))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGLMapBufferObject,                    (CUdeviceptr* dptr, unsigned int* size,  GLuint bufferobj), (dptr, size, bufferobj))
#endif

// CUDA 2.2

#if (CUDA_VERSION >= 2020)
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuDriverGetVersion,                     (int *driverVersion), (driverVersion))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemHostAlloc,                         (void **pp, size_t bytesize, unsigned int Flags), (pp, bytesize, Flags))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuFuncGetAttribute,                     (int* pi, CUfunction_attribute attrib, CUfunction hfunc), (pi, attrib, hfunc))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuWGLGetDevice,                         (CUdevice* pDevice, HGPUNV hGpu), (pDevice, hGpu))
#endif

#if (CUDA_VERSION >= 2020 && CUDA_VERSION < 3020)
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemHostGetDevicePointer,              (CUdeviceptr *pdptr, void *p, unsigned int Flags), (pdptr, p, Flags))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuTexRefSetAddress2D,                   (CUtexref hTexRef, const CUDA_ARRAY_DESCRIPTOR *desc, CUdeviceptr dptr, unsigned int Pitch), (hTexRef, desc, dptr, Pitch))
#endif

// CUDA 2.3

#if (CUDA_VERSION >= 2030)
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemHostGetFlags,                      (unsigned int *pFlags, void *p), (pFlags, p))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGLSetBufferObjectMapFlags,            (GLuint buffer, unsigned int Flags), (buffer, Flags))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGLUnmapBufferObjectAsync,             (GLuint buffer, CUstream hStream), (buffer, hStream))
#endif

#if (CUDA_VERSION >= 2030 && CUDA_VERSION < 3020)
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGLMapBufferObjectAsync,               (CUdeviceptr *dptr, unsigned int *size,  GLuint buffer, CUstream hStream), (dptr, size, buffer, hStream))
#endif

// CUDA 3.0

#if (CUDA_VERSION >= 3000)
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuFuncSetCacheConfig,                   (CUfunction hfunc, CUfunc_cache config), (hfunc, config))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGraphicsUnregisterResource,           (CUgraphicsResource resource), (resource))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGraphicsSubResourceGetMappedArray,    (CUarray *pArray, CUgraphicsResource resource, unsigned int arrayIndex, unsigned int mipLevel), (pArray, resource, arrayIndex, mipLevel))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGraphicsResourceSetMapFlags,          (CUgraphicsResource resource, unsigned int flags), (resource, flags))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGraphicsMapResources,                 (unsigned int count, CUgraphicsResource* resources, CUstream hStream), (count, resources, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGraphicsUnmapResources,               (unsigned int count, CUgraphicsResource *resources, CUstream hStream), (count, resources, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGetExportTable,                       (const void **ppExportTable, const CUuuid *pExportTableId), (ppExportTable, pExportTableId))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGraphicsGLRegisterBuffer,             (CUgraphicsResource *pCudaResource, GLuint buffer, unsigned int Flags), (pCudaResource, buffer, Flags))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGraphicsGLRegisterImage,              (CUgraphicsResource *pCudaResource, GLuint image, GLenum target, unsigned int Flags), (pCudaResource, image, target, Flags))
#endif

#if (CUDA_VERSION >= 3000 && CUDA_VERSION < 3020)
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemcpyDtoDAsync,                      (CUdeviceptr dstDevice, CUdeviceptr srcDevice, unsigned int ByteCount, CUstream hStream), (dstDevice, srcDevice, ByteCount, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuGraphicsResourceGetMappedPointer,     (CUdeviceptr* pDevPtr, unsigned int* pSize, CUgraphicsResource resource), (pDevPtr, pSize, resource))
#endif

// CUDA 3.1

#if (CUDA_VERSION >= 3010)
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuModuleGetSurfRef,                     (CUsurfref* pSurfRef, CUmodule hmod, const char* name), (pSurfRef, hmod, name))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuSurfRefSetArray,                      (CUsurfref hSurfRef, CUarray hArray, unsigned int Flags), (hSurfRef, hArray, Flags))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuSurfRefGetArray,                      (CUarray* phArray, CUsurfref hSurfRef), (phArray, hSurfRef))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuCtxSetLimit,                          (CUlimit limit, size_t value), (limit, value))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuCtxGetLimit,                          (size_t* pvalue, CUlimit limit), (pvalue, limit))
#endif

// CUDA 3.2

#if (CUDA_VERSION >= 3020)
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuDeviceTotalMem,                       (size_t* bytes, CUdevice dev), (bytes, dev))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuCtxCreate,                            (CUcontext* pctx, unsigned int flags, CUdevice dev), (pctx, flags, dev))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuModuleGetGlobal,                      (CUdeviceptr* dptr, size_t* bytes, CUmodule hmod, const char* name), (dptr, bytes, hmod, name))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemGetInfo,                           (size_t* free, size_t* total), (free, total))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemAlloc,                             (CUdeviceptr* dptr, size_t bytesize), (dptr, bytesize))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemAllocPitch,                        (CUdeviceptr* dptr, size_t* pPitch, size_t WidthInBytes, size_t Height, unsigned int ElementSizeBytes), (dptr, pPitch, WidthInBytes, Height, ElementSizeBytes))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemFree,                              (CUdeviceptr dptr), (dptr))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemGetAddressRange,                   (CUdeviceptr* pbase, size_t* psize, CUdeviceptr dptr), (pbase, psize, dptr))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemAllocHost,                         (void** pp, size_t bytesize), (pp, bytesize))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpyHtoD,                           (CUdeviceptr dstDevice, const void* srcHost, size_t ByteCount), (dstDevice, srcHost, ByteCount))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpyDtoH,                           (void* dstHost, CUdeviceptr srcDevice, size_t ByteCount), (dstHost, srcDevice, ByteCount))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpyDtoD,                           (CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount), (dstDevice, srcDevice, ByteCount))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpyDtoA,                           (CUarray dstArray, size_t dstOffset, CUdeviceptr srcDevice, size_t ByteCount), (dstArray, dstOffset, srcDevice, ByteCount))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpyAtoD,                           (CUdeviceptr dstDevice, CUarray hSrc, size_t srcOffset, size_t ByteCount), (dstDevice, hSrc, srcOffset, ByteCount))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpyHtoA,                           (CUarray dstArray, size_t dstOffset, const void* pSrc, size_t ByteCount), (dstArray, dstOffset, pSrc, ByteCount))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpyAtoH,                           (void* dstHost, CUarray srcArray, size_t srcOffset, size_t ByteCount), (dstHost, srcArray, srcOffset, ByteCount))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpyAtoA,                           (CUarray dstArray, size_t dstOffset, CUarray srcArray, size_t srcOffset, size_t ByteCount), (dstArray, dstOffset, srcArray, srcOffset, ByteCount))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpyHtoAAsync,                      (CUarray dstArray, size_t dstOffset, const void* pSrc, size_t ByteCount, CUstream hStream), (dstArray, dstOffset, pSrc, ByteCount, hStream))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpyAtoHAsync,                      (void* dstHost, CUarray srcArray, size_t srcOffset, size_t ByteCount, CUstream hStream), (dstHost, srcArray, srcOffset, ByteCount, hStream))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpy2D,                             (const CUDA_MEMCPY2D* pCopy), (pCopy))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpy2DUnaligned,                    (const CUDA_MEMCPY2D* pCopy), (pCopy))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpy3D,                             (const CUDA_MEMCPY3D* pCopy), (pCopy))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpyHtoDAsync,                      (CUdeviceptr dstDevice, const void* srcHost, size_t ByteCount, CUstream hStream), (dstDevice, srcHost, ByteCount, hStream))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpyDtoHAsync,                      (void* dstHost, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream), (dstHost, srcDevice, ByteCount, hStream))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpy2DAsync,                        (const CUDA_MEMCPY2D* pCopy, CUstream hStream), (pCopy, hStream))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpy3DAsync,                        (const CUDA_MEMCPY3D* pCopy, CUstream hStream), (pCopy, hStream))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemsetD8,                             (CUdeviceptr dstDevice, unsigned char uc, size_t N), (dstDevice, uc, N))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemsetD16,                            (CUdeviceptr dstDevice, unsigned short us, size_t N), (dstDevice, us, N))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemsetD32,                            (CUdeviceptr dstDevice, unsigned int ui, size_t N), (dstDevice, ui, N))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemsetD2D8,                           (CUdeviceptr dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height), (dstDevice, dstPitch, uc, Width, Height))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemsetD2D16,                          (CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height), (dstDevice, dstPitch, us, Width, Height))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemsetD2D32,                          (CUdeviceptr dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height), (dstDevice, dstPitch, ui, Width, Height))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuArrayCreate,                          (CUarray* pHandle, const CUDA_ARRAY_DESCRIPTOR* pAllocateArray), (pHandle, pAllocateArray))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuArrayGetDescriptor,                   (CUDA_ARRAY_DESCRIPTOR* pArrayDescriptor, CUarray hArray), (pArrayDescriptor, hArray))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuArray3DCreate,                        (CUarray* pHandle, const CUDA_ARRAY3D_DESCRIPTOR* pAllocateArray), (pHandle, pAllocateArray))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuArray3DGetDescriptor,                 (CUDA_ARRAY3D_DESCRIPTOR* pArrayDescriptor, CUarray hArray), (pArrayDescriptor, hArray))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuTexRefSetAddress,                     (size_t* ByteOffset, CUtexref hTexRef, CUdeviceptr dptr, size_t bytes), (ByteOffset, hTexRef, dptr, bytes))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuTexRefGetAddress,                     (CUdeviceptr* pdptr, CUtexref hTexRef), (pdptr, hTexRef))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuGLCtxCreate,                          (CUcontext* pCtx, unsigned int Flags, CUdevice device), (pCtx, Flags, device))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuGLMapBufferObject,                    (CUdeviceptr* dptr, size_t* size,  GLuint bufferobj), (dptr, size, bufferobj))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemHostGetDevicePointer,              (CUdeviceptr* pdptr, void* p, unsigned int Flags), (pdptr, p, Flags))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuTexRefSetAddress2D,                   (CUtexref hTexRef, const CUDA_ARRAY_DESCRIPTOR* desc, CUdeviceptr dptr, size_t Pitch), (hTexRef, desc, dptr, Pitch))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuGLMapBufferObjectAsync,               (CUdeviceptr* dptr, size_t* size,  GLuint buffer, CUstream hStream), (dptr, size, buffer, hStream))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuMemcpyDtoDAsync,                      (CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream), (dstDevice, srcDevice, ByteCount, hStream))
FW_DLL_IMPORT_CUV2( CUresult,   CUDAAPI,    cuGraphicsResourceGetMappedPointer,     (CUdeviceptr* pDevPtr, size_t* pSize, CUgraphicsResource resource), (pDevPtr, pSize, resource))
#endif

#if (CUDA_VERSION >= 3020)
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemsetD8Async,                        (CUdeviceptr dstDevice, unsigned char uc, size_t N, CUstream hStream), (dstDevice, uc, N, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemsetD16Async,                       (CUdeviceptr dstDevice, unsigned short us, size_t N, CUstream hStream), (dstDevice, us, N, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemsetD32Async,                       (CUdeviceptr dstDevice, unsigned int ui, size_t N, CUstream hStream), (dstDevice, ui, N, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemsetD2D8Async,                      (CUdeviceptr dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height, CUstream hStream), (dstDevice, dstPitch, uc, Width, Height, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemsetD2D16Async,                     (CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height, CUstream hStream), (dstDevice, dstPitch, us, Width, Height, hStream))
FW_DLL_IMPORT_RETV( CUresult,   CUDAAPI,    cuMemsetD2D32Async,                     (CUdeviceptr dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height, CUstream hStream), (dstDevice, dstPitch, ui, Width, Height, hStream))
#endif

//------------------------------------------------------------------------
// OpenGL
//------------------------------------------------------------------------

#if !FW_USE_GLEW

FW_DLL_DECLARE_VOID(void,       APIENTRY,   glActiveTexture,                        (GLenum texture), (texture))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glAttachShader,                         (GLuint program, GLuint shader), (program, shader))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glBindBuffer,                           (GLenum target, GLuint buffer), (target, buffer))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glBindFramebuffer,                      (GLenum target, GLuint framebuffer), (target, framebuffer))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glBindRenderbuffer,                     (GLenum target, GLuint renderbuffer), (target, renderbuffer))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glBlendEquation,                        (GLenum mode), (mode))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glBufferData,                           (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage), (target, size, data, usage))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glBufferSubData,                        (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data), (target, offset, size, data))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glCompileShader,                        (GLuint shader), (shader))
FW_DLL_DECLARE_RETV(GLuint,     APIENTRY,   glCreateProgram,                        (void), ())
FW_DLL_DECLARE_RETV(GLuint,     APIENTRY,   glCreateShader,                         (GLenum type), (type))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glDeleteBuffers,                        (GLsizei n, const GLuint* buffers), (n, buffers))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glDeleteFramebuffers,                   (GLsizei n, const GLuint* framebuffers), (n, framebuffers))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glDeleteProgram,                        (GLuint program), (program))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glDeleteRenderbuffers,                  (GLsizei n, const GLuint* renderbuffers), (n, renderbuffers))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glDeleteShader,                         (GLuint shader), (shader))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glDisableVertexAttribArray,             (GLuint v), (v))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glDrawBuffers,                          (GLsizei n, const GLenum* bufs), (n, bufs))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glEnableVertexAttribArray,              (GLuint v), (v))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glFramebufferRenderbuffer,              (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer), (target, attachment, renderbuffertarget, renderbuffer))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glFramebufferTexture2D,                 (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level), (target, attachment, textarget, texture, level))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glGenBuffers,                           (GLsizei n, GLuint* buffers), (n, buffers))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glGenFramebuffers,                      (GLsizei n, GLuint* framebuffers), (n, framebuffers))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glGenRenderbuffers,                     (GLsizei n, GLuint* renderbuffers), (n, renderbuffers))
FW_DLL_DECLARE_RETV(GLint,      APIENTRY,   glGetAttribLocation,                    (GLuint program, const GLchar* name), (program, name))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glGetBufferParameteriv,                 (GLenum target, GLenum pname, GLint* params), (target, pname, params))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glGetBufferSubData,                     (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data), (target, offset, size, data))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glGetProgramInfoLog,                    (GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog), (program, bufSize, length, infoLog))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glGetProgramiv,                         (GLuint program, GLenum pname, GLint* param), (program, pname, param))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glGetShaderInfoLog,                     (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog), (shader, bufSize, length, infoLog))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glGetShaderiv,                          (GLuint shader, GLenum pname, GLint* param), (shader, pname, param))
FW_DLL_DECLARE_RETV(GLint,      APIENTRY,   glGetUniformLocation,                   (GLuint program, const GLchar* name), (program, name))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glLinkProgram,                          (GLhandleARB programObj), (programObj))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glProgramParameteriARB,                 (GLuint program, GLenum pname, GLint value), (program, pname, value))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glRenderbufferStorage,                  (GLenum target, GLenum internalformat, GLsizei width, GLsizei height), (target, internalformat, width, height))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glShaderSource,                         (GLuint shader, GLsizei count, const GLchar** strings, const GLint* lengths), (shader, count, strings, lengths))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glTexImage3D,                           (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels), (target, level, internalFormat, width, height, depth, border, format, type, pixels))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glUniform1f,                            (GLint location, GLfloat v0), (location, v0))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glUniform1fv,                           (GLint location, GLsizei count, const GLfloat* value), (location, count, value))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glUniform1i,                            (GLint location, GLint v0), (location, v0))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glUniform2f,                            (GLint location, GLfloat v0, GLfloat v1), (location, v0, v1))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glUniform2fv,                           (GLint location, GLsizei count, const GLfloat* value), (location, count, value))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glUniform3f,                            (GLint location, GLfloat v0, GLfloat v1, GLfloat v2), (location, v0, v1, v2))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glUniform4f,                            (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3), (location, v0, v1, v2, v3))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glUniform4fv,                           (GLint location, GLsizei count, const GLfloat* value), (location, count, value))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glUniformMatrix2fv,                     (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value), (location, count, transpose, value))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glUniformMatrix3fv,                     (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value), (location, count, transpose, value))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glUniformMatrix4fv,                     (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value), (location, count, transpose, value))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glUseProgram,                           (GLuint program), (program))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glVertexAttrib2f,                       (GLuint index, GLfloat x, GLfloat y), (index, x, y))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glVertexAttrib3f,                       (GLuint index, GLfloat x, GLfloat y, GLfloat z), (index, x, y, z))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glVertexAttrib4f,                       (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w), (index, x, y, z, w))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glVertexAttribPointer,                  (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer), (index, size, type, normalized, stride, pointer))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glWindowPos2i,                          (GLint x, GLint y), (x, y))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glBindFragDataLocationEXT,              (GLuint program, GLuint color, const GLchar* name), (program, color, name))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glBlitFramebuffer,                      (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter), (srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter))
FW_DLL_DECLARE_VOID(void,       APIENTRY,   glRenderbufferStorageMultisample,       (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height), (target, samples, internalformat, width, height))

#endif

//------------------------------------------------------------------------
// WGL
//------------------------------------------------------------------------

#if !FW_USE_GLEW

FW_DLL_DECLARE_RETV(BOOL,       WINAPI,     wglChoosePixelFormatARB,                (HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats), (hdc, piAttribIList, pfAttribFList, nMaxFormats, piFormats, nNumFormats))
FW_DLL_DECLARE_RETV(BOOL,       WINAPI,     wglSwapIntervalEXT,                     (int interval), (interval))
FW_DLL_DECLARE_RETV(BOOL,       WINAPI,     wglGetPixelFormatAttribivARB,           (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int* piAttributes, int* piValues), (hdc, iPixelFormat, iLayerPlane, nAttributes, piAttributes, piValues))

#endif

//------------------------------------------------------------------------
// WinMM
//------------------------------------------------------------------------

FW_DLL_IMPORT_RETV( MMRESULT,   WINAPI,     waveOutOpen,                            (OUT LPHWAVEOUT phwo, IN UINT uDeviceID, IN LPCWAVEFORMATEX pwfx, IN DWORD_PTR dwCallback, IN DWORD_PTR dwInstance, IN DWORD fdwOpen), (phwo, uDeviceID, pwfx, dwCallback, dwInstance, fdwOpen))
FW_DLL_IMPORT_RETV( MMRESULT,   WINAPI,     waveOutClose,                           (IN OUT HWAVEOUT hwo), (hwo))
FW_DLL_IMPORT_RETV( MMRESULT,   WINAPI,     waveOutPrepareHeader,                   (IN HWAVEOUT hwo, IN OUT LPWAVEHDR pwh, IN UINT cbwh), (hwo, pwh, cbwh))
FW_DLL_IMPORT_RETV( MMRESULT,   WINAPI,     waveOutUnprepareHeader,                 (IN HWAVEOUT hwo, IN OUT LPWAVEHDR pwh, IN UINT cbwh), (hwo, pwh, cbwh))
FW_DLL_IMPORT_RETV( MMRESULT,   WINAPI,     waveOutWrite,                           (IN HWAVEOUT hwo, IN OUT LPWAVEHDR pwh, IN UINT cbwh), (hwo, pwh, cbwh))
FW_DLL_IMPORT_RETV( MMRESULT,   WINAPI,     waveOutReset,                           (IN HWAVEOUT hwo), (hwo))

//------------------------------------------------------------------------
// ShLwAPI
//------------------------------------------------------------------------

FW_DLL_IMPORT_RETV( BOOL,   STDAPICALLTYPE, PathRelativePathToA,                    (LPSTR pszPath, LPCSTR pszFrom, DWORD dwAttrFrom, LPCSTR pszTo, DWORD dwAttrTo), (pszPath, pszFrom, dwAttrFrom, pszTo, dwAttrTo))

//------------------------------------------------------------------------
