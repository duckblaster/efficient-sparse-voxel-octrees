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
#include "base/Hash.hpp"

namespace FW
{
//------------------------------------------------------------------------

class CudaModule;
class Window;

//------------------------------------------------------------------------

class CudaCompiler
{
public:
                            CudaCompiler    (void);
                            ~CudaCompiler   (void);

    void                    setCachePath    (const String& path)                            { m_cachePath = path; }
    void                    setSourceFile   (const String& path)                            { m_sourceFile = path; m_sourceFileHashValid = false; m_memHashValid = false; }
    void                    overrideSMArch  (int arch)                                      { m_overriddenSMArch = arch; }

    void                    clearOptions    (void)                                          { m_options = ""; m_optionHashValid = false; m_memHashValid = false; }
    void                    addOptions      (const String& options)                         { m_options += options + " "; m_optionHashValid = false; m_memHashValid = false; }
    void                    include         (const String& path)                            { addOptions(sprintf("-I\"%s\"", path.getPtr())); }

    void                    clearDefines    (void)                                          { m_defines.clear(); m_defineHashValid = false; m_memHashValid = false; }
    void                    undef           (const String& key)                             { if (m_defines.contains(key)) { m_defines.remove(key); m_defineHashValid = false; m_memHashValid = false; } }
    void                    define          (const String& key, const String& value = "")   { undef(key); m_defines.add(key, value); m_defineHashValid = false; m_memHashValid = false; }
    void                    define          (const String& key, int value)                  { define(key, sprintf("%d", value)); }

    void                    clearPreamble   (void)                                          { m_preamble = ""; m_preambleHashValid = false; m_memHashValid = false; }
    void                    addPreamble     (const String& preamble)                        { m_preamble += preamble + "\n"; m_preambleHashValid = false; m_memHashValid = false; }

    void                    setMessageWindow(Window* window)                                { m_window = window; }
    CudaModule*             compile         (bool enablePrints = true);
    const Array<U8>*        compileCubin    (bool enablePrints = true); // returns data in cubin file, padded with a zero
    String                  compileCubinFile(bool enablePrints = true); // returns file name, empty string on error

    static void             setStaticCudaBinPath(const String& path)                        { FW_ASSERT(!s_inited); s_staticCudaBinPath = path; }
    static void             setStaticOptions(const String& options)                         { FW_ASSERT(!s_inited); s_staticOptions = options; }
    static void             setStaticPreamble(const String& preamble)                       { FW_ASSERT(!s_inited); s_staticPreamble = preamble; } // e.g. "#include \"myheader.h\""
    static void             setStaticBinaryFormat(const String& format)                     { FW_ASSERT(!s_inited); s_staticBinaryFormat = format; } // e.g. "-ptx"

    static void             staticInit      (void);
    static void             staticDeinit    (void);
    static void             flushMemCache   (void);

private:
                            CudaCompiler    (const CudaCompiler&); // forbidden
    CudaCompiler&           operator=       (const CudaCompiler&); // forbidden

private:
    static String           queryEnv        (const String& name);
    static void             splitPathList   (Array<String>& res, const String& value);
    static bool             fileExists      (const String& name);
    static String           removeOption    (const String& opts, const String& tag, bool hasParam);

    U64                     getMemHash      (void);
    void                    createCacheDir  (void);
    void                    writeDefineFile (void);
    void                    initLogFile     (const String& name, const String& firstLine);

    void                    runPreprocessor (String& cubinFile, String& finalOpts);
    void                    runCompiler     (const String& cubinFile, const String& finalOpts);

    void                    setLoggedError  (const String& description, const String& logFile);

private:
    static String           s_staticCudaBinPath;
    static String           s_staticOptions;
    static String           s_staticPreamble;
    static String           s_staticBinaryFormat;

    static bool             s_inited;
    static Hash<U64, Array<U8>*> s_cubinCache;
    static Hash<U64, CudaModule*> s_moduleCache;
    static U32              s_nvccVersionHash;
    static String           s_nvccCommand;

    String                  m_cachePath;
    String                  m_sourceFile;
    S32                     m_overriddenSMArch;

    String                  m_options;
    Hash<String, String>    m_defines;
    String                  m_preamble;

    U32                     m_sourceFileHash;
    U32                     m_optionHash;
    U64                     m_defineHash;
    U32                     m_preambleHash;
    U64                     m_memHash;
    bool                    m_sourceFileHashValid;
    bool                    m_optionHashValid;
    bool                    m_defineHashValid;
    bool                    m_preambleHashValid;
    bool                    m_memHashValid;

    Window*                 m_window;
};

//------------------------------------------------------------------------
}
