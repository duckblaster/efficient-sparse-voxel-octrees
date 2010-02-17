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
 
#include "gpu/CudaCompiler.hpp"
#include "gpu/CudaModule.hpp"
#include "base/DLLImports.hpp"
#include "io/File.hpp"
#include "gui/Window.hpp"

#include <process.h>
#include <stdio.h>

using namespace FW;

//------------------------------------------------------------------------

bool                    CudaCompiler::s_inited          = false;
Hash<U64, CudaModule*>  CudaCompiler::s_memCache;
U32                     CudaCompiler::s_nvccVersionHash = 0;
String                  CudaCompiler::s_nvccCommand;

//------------------------------------------------------------------------

CudaCompiler::CudaCompiler(void)
:   m_cachePath             ("cudacache"),
    m_sourceFile            ("unspecified.cu"),

    m_sourceFileHash        (0),
    m_optionHash            (0),
    m_defineHash            (0),
    m_memHash               (0),
    m_sourceFileHashValid   (false),
    m_optionHashValid       (false),
    m_defineHashValid       (false),
    m_memHashValid          (false),

    m_window                (NULL)
{
}

//------------------------------------------------------------------------

CudaCompiler::~CudaCompiler(void)
{
}

//------------------------------------------------------------------------

CudaModule* CudaCompiler::compile(bool enablePrints)
{
    staticInit();

    // Cached in memory => done.

    U64 memHash = getMemHash();
    CudaModule** found = s_memCache.search(memHash);
    if (found)
        return *found;

    // Check that the source file exists.
    {
        File file(m_sourceFile, File::Read);
        if (hasError())
            return NULL;
    }

    // Cache directory does not exist => create it.

    createCacheDir();
    if (hasError())
        return NULL;

    // Preprocess.

    writeDefineFile();
    String cubinFile, finalOpts;
    runPreprocessor(cubinFile, finalOpts);
    if (hasError())
        return NULL;

    // CUBIN does not exist => compile.

    if (!fileExists(cubinFile))
    {
        if (enablePrints)
            printf("CudaCompiler: Compiling '%s'...", m_sourceFile.getPtr());
        if (m_window)
            m_window->showModalMessage("Compiling CUDA kernel...");

        runCompiler(cubinFile, finalOpts);

        if (enablePrints)
            printf((hasError()) ? " Failed.\n" : " Done.\n");
        if (hasError())
            return NULL;
    }

    // Load CUBIN.

    File in(cubinFile, File::Read);
    S32 size = (S32)in.getSize();
    Array<U8> buffer(NULL, size + 1);
    in.read(buffer.getPtr(), size);
    buffer[size] = 0;
    CudaModule* module = new CudaModule(buffer.getPtr());

    // Add to memory cache.

    s_memCache.add(memHash, module);
    return module;
}

//------------------------------------------------------------------------

void CudaCompiler::staticInit(void)
{
    if (s_inited || hasError())
        return;
    s_inited = true;

    // Query environment variables.

    String pathEnv      = queryEnv("PATH");
    String includeEnv   = queryEnv("INCLUDE");
    String cudaBinEnv   = queryEnv("CUDA_BIN_PATH");
    String cudaIncEnv   = queryEnv("CUDA_INC_PATH");

    // Collect binary path candidates.

    Array<String> binCandidates;
    binCandidates.add(cudaBinEnv);
    splitPathList(binCandidates, pathEnv);

    binCandidates.add("C:\\CUDA\\bin");
    binCandidates.add("C:\\CUDA\\bin64");
    binCandidates.add("D:\\CUDA\\bin");
    binCandidates.add("D:\\CUDA\\bin64");

    binCandidates.add("C:\\Program Files\\Microsoft Visual Studio 8\\VC\\bin");
    binCandidates.add("C:\\Program Files\\Microsoft Visual Studio 9.0\\VC\\bin");
    binCandidates.add("C:\\Program Files (x86)\\Microsoft Visual Studio 8\\VC\\bin");
    binCandidates.add("C:\\Program Files (x86)\\Microsoft Visual Studio 9.0\\VC\\bin");
    binCandidates.add("D:\\Program Files\\Microsoft Visual Studio 8\\VC\\bin");
    binCandidates.add("D:\\Program Files\\Microsoft Visual Studio 9.0\\VC\\bin");
    binCandidates.add("D:\\Program Files (x86)\\Microsoft Visual Studio 8\\VC\\bin");
    binCandidates.add("D:\\Program Files (x86)\\Microsoft Visual Studio 9.0\\VC\\bin");

    // Find CUDA binary path.

    String cudaBinPath;
    for (int i = 0; i < binCandidates.getSize(); i++)
    {
        if (!binCandidates[i].getLength() || !fileExists(binCandidates[i] + "\\nvcc.exe"))
            continue;

        // Execute "nvcc --version".

        FILE* pipe = _popen(sprintf("%s\\nvcc.exe --version 2>nul", binCandidates[i].getPtr()).getPtr(), "rt");
        if (!pipe)
            continue;

        Array<char> output;
        while (!feof(pipe))
            output.add((char)fgetc(pipe));
        fclose(pipe);

        // Invalid response => ignore.

        output.add(0);
        String response(output.getPtr());
        if (!response.startsWith("nvcc: NVIDIA"))
            continue;

        // Hash response.

        cudaBinPath = binCandidates[i];
        s_nvccVersionHash = hash<String>(response);
        break;
    }

    if (!cudaBinPath.getLength())
        fail("Unable to detect CUDA Toolkit binary path!\nPlease set CUDA_BIN_PATH environment variable.");

    // Find Visual Studio binary path.

    String vsBinPath;
    for (int i = 0; i < binCandidates.getSize() && !vsBinPath.getLength(); i++)
        if (binCandidates[i].getLength() && fileExists(binCandidates[i] + "\\vcvars32.bat"))
            vsBinPath = binCandidates[i];

    if (!vsBinPath.getLength())
        fail("Unable to detect Visual Studio binary path!\nPlease run VCVARS32.BAT.");

    // Collect include path candidates.

    Array<String> incCandidates;
    incCandidates.clear();
    incCandidates.add(cudaIncEnv);
    splitPathList(incCandidates, includeEnv);
    incCandidates.add(cudaBinPath + "\\..\\include");
    incCandidates.add(vsBinPath + "\\..\\INCLUDE");

    incCandidates.add("C:\\CUDA\\include");
    incCandidates.add("D:\\CUDA\\include");

    incCandidates.add("C:\\Program Files\\Microsoft Visual Studio 8\\VC\\INCLUDE");
    incCandidates.add("C:\\Program Files\\Microsoft Visual Studio 9.0\\VC\\INCLUDE");
    incCandidates.add("C:\\Program Files (x86)\\Microsoft Visual Studio 8\\VC\\INCLUDE");
    incCandidates.add("C:\\Program Files (x86)\\Microsoft Visual Studio 9.0\\VC\\INCLUDE");
    incCandidates.add("D:\\Program Files\\Microsoft Visual Studio 8\\VC\\INCLUDE");
    incCandidates.add("D:\\Program Files\\Microsoft Visual Studio 9.0\\VC\\INCLUDE");
    incCandidates.add("D:\\Program Files (x86)\\Microsoft Visual Studio 8\\VC\\INCLUDE");
    incCandidates.add("D:\\Program Files (x86)\\Microsoft Visual Studio 9.0\\VC\\INCLUDE");

    // Find CUDA include path.

    String cudaIncPath;
    for (int i = 0; i < incCandidates.getSize() && !cudaIncPath.getLength(); i++)
        if (incCandidates[i].getLength() && fileExists(incCandidates[i] + "\\cuda.h"))
            cudaIncPath = incCandidates[i];

    if (!cudaIncPath.getLength())
        fail("Unable to detect CUDA Toolkit include path!\nPlease set CUDA_INC_PATH environment variable.");

    // Find Visual Studio include path.

    String vsIncPath;
    for (int i = 0; i < incCandidates.getSize() && !vsIncPath.getLength(); i++)
        if (incCandidates[i].getLength() && fileExists(incCandidates[i] + "\\crtdefs.h"))
            vsIncPath = incCandidates[i];

    if (!vsIncPath.getLength())
        fail("Unable to detect Visual Studio include path!\nPlease run VCVARS32.BAT.");

    // Form NVCC command line.

    s_nvccCommand = sprintf("set PATH=%s;%s & nvcc.exe -ccbin \"%s\" -I\"%s\" -I\"%s\" -D_CRT_SECURE_NO_DEPRECATE",
        cudaBinPath.getPtr(),
        pathEnv.getPtr(),
        vsBinPath.getPtr(),
        cudaIncPath.getPtr(),
        vsIncPath.getPtr());
}

//------------------------------------------------------------------------

void CudaCompiler::staticDeinit(void)
{
    if (!s_inited)
        return;
    s_inited = false;

    flushMemCache();
    s_memCache.reset();
    s_nvccCommand = "";
}

//------------------------------------------------------------------------

void CudaCompiler::flushMemCache(void)
{
    for (int i = s_memCache.firstSlot(); i != -1; i = s_memCache.nextSlot(i))
        delete s_memCache.getSlot(i).value;
    s_memCache.clear();
}

//------------------------------------------------------------------------

String CudaCompiler::queryEnv(const String& name)
{
    char buffer[1024];
    DWORD len = GetEnvironmentVariable(name.getPtr(), buffer, FW_ARRAY_SIZE(buffer));
    if (len > 0 && len < FW_ARRAY_SIZE(buffer) - 1)
        return buffer;
    return "";
}

//------------------------------------------------------------------------

void CudaCompiler::splitPathList(Array<String>& res, const String& value)
{
    for (int startIdx = 0; startIdx < value.getLength();)
    {
        int endIdx = value.indexOf(';', startIdx);
        if (endIdx == -1)
            endIdx = value.getLength();

        String item = value.substring(startIdx, endIdx);
        if (item.getLength() >= 2 && item.startsWith("\"") && item.endsWith("\""))
            item = item.substring(1, item.getLength() - 1);
        res.add(item);

        startIdx = endIdx + 1;
    }
}

//------------------------------------------------------------------------

bool CudaCompiler::fileExists(const String& name)
{
    return ((GetFileAttributes(name.getPtr()) & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

//------------------------------------------------------------------------

U64 CudaCompiler::getMemHash(void)
{
    if (m_memHashValid)
        return m_memHash;

    if (!m_sourceFileHashValid)
    {
        m_sourceFileHash = hash<String>(m_sourceFile);
        m_sourceFileHashValid = true;
    }

    if (!m_optionHashValid)
    {
        m_optionHash = hash<String>(m_options);
        m_optionHashValid = true;
    }

    if (!m_defineHashValid)
    {
        U32 a = FW_HASH_MAGIC, b = FW_HASH_MAGIC, c = FW_HASH_MAGIC;
        for (int i = m_defines.firstSlot(); i != -1; i = m_defines.nextSlot(i))
        {
            a += hash<String>(m_defines.getSlot(i).key);
            b += hash<String>(m_defines.getSlot(i).value);
            FW_JENKINS_MIX(a, b, c);
        }
        m_defineHash = ((U64)b << 32) | c;
        m_defineHashValid = true;
    }

    U32 a = FW_HASH_MAGIC + m_sourceFileHash;
    U32 b = FW_HASH_MAGIC + m_optionHash;
    U32 c = FW_HASH_MAGIC;
    FW_JENKINS_MIX(a, b, c);
    a += (U32)(m_defineHash >> 32);
    b += (U32)m_defineHash;
    FW_JENKINS_MIX(a, b, c);
    m_memHash = ((U64)b << 32) | c;
    m_memHashValid = true;
    return m_memHash;
}

//------------------------------------------------------------------------

void CudaCompiler::createCacheDir(void)
{
    DWORD res = GetFileAttributes(m_cachePath.getPtr());
    if (res == 0xFFFFFFFF || (res & FILE_ATTRIBUTE_DIRECTORY) == 0)
        if (CreateDirectory(m_cachePath.getPtr(), NULL) == 0)
            fail("Cannot create CudaCompiler cache directory '%s'!", m_cachePath.getPtr());
}

//------------------------------------------------------------------------

void CudaCompiler::writeDefineFile(void)
{
    File file(m_cachePath + "\\defines.inl", File::Create);
    BufferedOutputStream out(file);
    for (int i = m_defines.firstSlot(); i != -1; i = m_defines.nextSlot(i))
        out.writef("#define %s %s\n",
            m_defines.getSlot(i).key.getPtr(),
            m_defines.getSlot(i).value.getPtr());
    out.flush();
}

//------------------------------------------------------------------------

void CudaCompiler::initLogFile(const String& name, const String& firstLine)
{
    File file(name, File::Create);
    BufferedOutputStream out(file);
    out.writef("%s\n", firstLine.getPtr());
    out.flush();
}

//------------------------------------------------------------------------

void CudaCompiler::runPreprocessor(String& cubinFile, String& finalOpts)
{
    // Preprocess.

    String logFile = m_cachePath + "\\preprocess.log";
    String cmd = sprintf("%s -E -o \"%s\\preprocessed.cu\" -include \"%s\\defines.inl\" %s\"%s\" 2>>\"%s\"",
        s_nvccCommand.getPtr(),
        m_cachePath.getPtr(),
        m_cachePath.getPtr(),
        m_options.getPtr(),
        m_sourceFile.getPtr(),
        logFile.getPtr());

    initLogFile(logFile, cmd);
    if (system(cmd.getPtr()) != 0)
    {
        setLoggedError("CudaCompiler: Preprocessing failed!", logFile);
        return;
    }

    // Hash and find inline compiler options.

    String optionPrefix = "// EMIT_NVCC_OPTIONS ";
    File file(m_cachePath + "\\preprocessed.cu", File::Read);
    BufferedInputStream in(file);

    U32 hashA = FW_HASH_MAGIC;
    U32 hashB = FW_HASH_MAGIC;
    U32 hashC = FW_HASH_MAGIC;
    finalOpts = m_options;

    for (int lineIdx = 0;; lineIdx++)
    {
        const char* linePtr = in.readLine();
        if (!linePtr)
            break;

        // Directive or empty => ignore.

        if (*linePtr == '#' || *linePtr == '\0')
            continue;

        // Compiler option directive => record.

        String line(linePtr);
        if (line.startsWith(optionPrefix))
            finalOpts += line.substring(optionPrefix.getLength()) + " ";

        // Otherwise => hash.

        else
        {
            hashA += hash<String>(line);
            FW_JENKINS_MIX(hashA, hashB, hashC);
        }
    }

    // Running on Fermi => force "-arch sm_20".

    if (CudaModule::getComputeCapability() == 20)
    {
        // Remove existing definitions.

        for (int i = 0; i < finalOpts.getLength(); i++)
        {
            if (finalOpts[i] != '-')
                continue;

            String tmp(finalOpts.substring(i));
            if (!tmp.startsWith("-arch ") && !tmp.startsWith("--gpu-architecture"))
                continue;

            int idx = tmp.indexOf(' ', tmp.indexOf(' ') + 1);
            if (idx == -1)
                continue;

            finalOpts = finalOpts.substring(0, i) + tmp.substring(idx + 1);
            i--;
        }

        // Append new definition.

        finalOpts += "-arch sm_20 ";
    }

    // Hash final compiler options and version.

    hashA += hash<String>(finalOpts);
    hashB += s_nvccVersionHash;
    FW_JENKINS_MIX(hashA, hashB, hashC);
    cubinFile = sprintf("%s\\%08x%08x.cubin", m_cachePath.getPtr(), hashB, hashC);
}

//------------------------------------------------------------------------

void CudaCompiler::runCompiler(const String& cubinFile, const String& finalOpts)
{
    String logFile = m_cachePath + "\\compile.log";
    String cmd = sprintf("%s -cubin -o \"%s\" -include \"%s\\defines.inl\" %s\"%s\" 2>>\"%s\"",
        s_nvccCommand.getPtr(),
        cubinFile.getPtr(),
        m_cachePath.getPtr(),
        finalOpts.getPtr(),
        m_sourceFile.getPtr(),
        logFile.getPtr());

    initLogFile(logFile, cmd);
    if (system(cmd.getPtr()) != 0 || !fileExists(cubinFile))
        setLoggedError("CudaCompiler: Compilation failed!", logFile);
}

//------------------------------------------------------------------------

void CudaCompiler::setLoggedError(const String& description, const String& logFile)
{
    String message = description;
    File file(logFile, File::Read);
    BufferedInputStream in(file);
    in.readLine();
    for (;;)
    {
        const char* linePtr = in.readLine();
        if (!linePtr)
            break;
        if (*linePtr)
            message += '\n';
        message += linePtr;
    }
    setError("%s", message.getPtr());
}

//------------------------------------------------------------------------
