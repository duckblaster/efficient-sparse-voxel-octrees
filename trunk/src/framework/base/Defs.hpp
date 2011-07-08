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

#pragma warning(disable:4530) // C++ exception handler used, but unwind semantics are not enabled.
#include <new>
#include <string.h>

namespace FW
{
//------------------------------------------------------------------------

#ifndef NULL
#   define NULL 0
#endif

#ifdef _DEBUG
#   define FW_DEBUG 1
#else
#   define FW_DEBUG 0
#endif

#ifdef _M_X64
#   define FW_64    1
#else
#   define FW_64    0
#endif

#ifdef __CUDACC__
#   define FW_CUDA 1
#else
#   define FW_CUDA 0
#endif

#if (FW_DEBUG || defined(FW_ENABLE_ASSERT)) && !FW_CUDA
#   define FW_ASSERT(X) ((X) ? ((void)0) : FW::fail("Assertion failed!\n%s:%d\n%s", __FILE__, __LINE__, #X))
#else
#   define FW_ASSERT(X) ((void)0)
#endif

#if FW_CUDA
#   define FW_CUDA_FUNC     __device__ __inline__
#   define FW_CUDA_CONST    __constant__
#else
#   define FW_CUDA_FUNC     inline
#   define FW_CUDA_CONST    static const
#endif

#define FW_UNREF(X)         ((void)(X))
#define FW_ARRAY_SIZE(X)    (sizeof(X) / sizeof((X)[0]))

//------------------------------------------------------------------------

typedef unsigned char       U8;
typedef unsigned short      U16;
typedef unsigned int        U32;
typedef signed char         S8;
typedef signed short        S16;
typedef signed int          S32;
typedef float               F32;
typedef double              F64;
typedef void                (*FuncPtr)(void);

#if FW_CUDA
typedef unsigned long long  U64;
typedef signed long long    S64;
#else
typedef unsigned __int64    U64;
typedef signed __int64      S64;
#endif

#if FW_64
typedef S64                 SPTR;
typedef U64                 UPTR;
#else
typedef __w64 S32           SPTR;
typedef __w64 U32           UPTR;
#endif

//------------------------------------------------------------------------

#define FW_U32_MAX          (0xFFFFFFFFu)
#define FW_S32_MIN          (~0x7FFFFFFF)
#define FW_S32_MAX          (0x7FFFFFFF)
#define FW_U64_MAX          ((U64)(S64)-1)
#define FW_S64_MIN          ((S64)-1 << 63)
#define FW_S64_MAX          (~((S64)-1 << 63))
#define FW_F32_MIN          (1.175494351e-38f)
#define FW_F32_MAX          (3.402823466e+38f)
#define FW_F64_MIN          (2.2250738585072014e-308)
#define FW_F64_MAX          (1.7976931348623158e+308)
#define FW_PI               (3.14159265358979323846f)

//------------------------------------------------------------------------

#if !FW_CUDA

class String;

// Common functionality.

void*           malloc          (size_t size);
void            free            (void* ptr);
void*           realloc         (void* ptr, size_t size);

void            printf          (const char* fmt, ...);
String          sprintf         (const char* fmt, ...);

// Error handling.

void            setError        (const char* fmt, ...);
String          clearError      (void);
bool            restoreError    (const String& old);
bool            hasError        (void);
const String&   getError        (void);

void            fail            (const char* fmt, ...);
void            failWin32Error  (const char* funcName);
void            failIfError     (void);

bool            setDiscardEvents(bool discard);
bool            getDiscardEvents(void);

// Logging.

void            pushLogFile     (const String& name, bool append = true);
void            popLogFile      (void);
bool            hasLogFile      (void);

// Memory profiling.

size_t          getMemoryUsed   (void);
void            pushMemOwner    (const char* id);
void            popMemOwner     (void);
void            printMemStats   (void);

// Performance profiling.

void            profileStart    (void);
void            profilePush     (const char* id);
void            profilePop      (void);
void            profileEnd      (bool printResults = true);

#endif

//------------------------------------------------------------------------

template <class T> FW_CUDA_FUNC void        swap    (T& a, T& b)                            { T t = a; a = b; b = t; }

#define FW_SPECIALIZE_MINMAX(TEMPLATE, T, MIN, MAX) \
    TEMPLATE FW_CUDA_FUNC T min(T a, T b) { return MIN; } \
    TEMPLATE FW_CUDA_FUNC T max(T a, T b) { return MAX; } \
    TEMPLATE FW_CUDA_FUNC T min(T a, T b, T c) { return min(min(a, b), c); } \
    TEMPLATE FW_CUDA_FUNC T max(T a, T b, T c) { return max(max(a, b), c); } \
    TEMPLATE FW_CUDA_FUNC T min(T a, T b, T c, T d) { return min(min(min(a, b), c), d); } \
    TEMPLATE FW_CUDA_FUNC T max(T a, T b, T c, T d) { return max(max(max(a, b), c), d); } \
    TEMPLATE FW_CUDA_FUNC T min(T a, T b, T c, T d, T e) { return min(min(min(min(a, b), c), d), e); } \
    TEMPLATE FW_CUDA_FUNC T max(T a, T b, T c, T d, T e) { return max(max(max(max(a, b), c), d), e); } \
    TEMPLATE FW_CUDA_FUNC T min(T a, T b, T c, T d, T e, T f) { return min(min(min(min(min(a, b), c), d), e), f); } \
    TEMPLATE FW_CUDA_FUNC T max(T a, T b, T c, T d, T e, T f) { return max(max(max(max(max(a, b), c), d), e), f); } \
    TEMPLATE FW_CUDA_FUNC T min(T a, T b, T c, T d, T e, T f, T g) { return min(min(min(min(min(min(a, b), c), d), e), f), g); } \
    TEMPLATE FW_CUDA_FUNC T max(T a, T b, T c, T d, T e, T f, T g) { return max(max(max(max(max(max(a, b), c), d), e), f), g); } \
    TEMPLATE FW_CUDA_FUNC T min(T a, T b, T c, T d, T e, T f, T g, T h) { return min(min(min(min(min(min(min(a, b), c), d), e), f), g), h); } \
    TEMPLATE FW_CUDA_FUNC T max(T a, T b, T c, T d, T e, T f, T g, T h) { return max(max(max(max(max(max(max(a, b), c), d), e), f), g), h); } \
    TEMPLATE FW_CUDA_FUNC T clamp(T v, T lo, T hi) { return min(max(v, lo), hi); }

FW_SPECIALIZE_MINMAX(template <class T>, T&, (a < b) ? a : b, (a > b) ? a : b)
FW_SPECIALIZE_MINMAX(template <class T>, const T&, (a < b) ? a : b, (a > b) ? a : b)

#if FW_CUDA
FW_SPECIALIZE_MINMAX(, U32, ::min(a, b), ::max(a, b))
FW_SPECIALIZE_MINMAX(, S32, ::min(a, b), ::max(a, b))
FW_SPECIALIZE_MINMAX(, U64, ::min(a, b), ::max(a, b))
FW_SPECIALIZE_MINMAX(, S64, ::min(a, b), ::max(a, b))
FW_SPECIALIZE_MINMAX(, F32, ::fminf(a, b), ::fmaxf(a, b))
FW_SPECIALIZE_MINMAX(, F64, ::fmin(a, b), ::fmax(a, b))
#endif

//------------------------------------------------------------------------
}

#if !FW_CUDA

inline void*    operator new        (size_t size)       { return FW::malloc(size); }
inline void*    operator new[]      (size_t size)       { return FW::malloc(size); }
inline void     operator delete     (void* ptr)         { return FW::free(ptr); }
inline void     operator delete[]   (void* ptr)         { return FW::free(ptr); }

#endif

//------------------------------------------------------------------------
