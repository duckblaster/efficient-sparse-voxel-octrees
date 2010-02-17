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

    void                    clearOptions    (void)                                          { m_options = ""; m_optionHashValid = false; }
    void                    addOptions      (const String& options)                         { m_options += options + " "; m_optionHashValid = false; m_memHashValid = false; }
    void                    include         (const String& path)                            { addOptions(sprintf("-I\"%s\"", path.getPtr())); }

    void                    clearDefines    (void)                                          { m_defines.clear(); m_memHashValid = false; }
    void                    undef           (const String& key)                             { if (m_defines.contains(key)) { m_defines.remove(key); m_defineHashValid = false; m_memHashValid = false; } }
    void                    define          (const String& key, const String& value = "")   { undef(key); m_defines.add(key, value); m_defineHashValid = false; m_memHashValid = false; }
    void                    define          (const String& key, int value)                  { define(key, sprintf("%d", value)); }

    void                    setMessageWindow(Window* window)                                { m_window = window; }
    CudaModule*             compile         (bool enablePrints = true);

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

    U64                     getMemHash      (void);
    void                    createCacheDir  (void);
    void                    writeDefineFile (void);
    void                    initLogFile     (const String& name, const String& firstLine);

    void                    runPreprocessor (String& cubinFile, String& finalOpts);
    void                    runCompiler     (const String& cubinFile, const String& finalOpts);

    void                    setLoggedError  (const String& description, const String& logFile);

private:
    static bool             s_inited;
    static Hash<U64, CudaModule*> s_memCache;
    static U32              s_nvccVersionHash;
    static String           s_nvccCommand;

    String                  m_cachePath;
    String                  m_sourceFile;
    String                  m_options;
    Hash<String, String>    m_defines;
    Array<String>           m_defineValues;

    U32                     m_sourceFileHash;
    U32                     m_optionHash;
    U64                     m_defineHash;
    U64                     m_memHash;
    bool                    m_sourceFileHashValid;
    bool                    m_optionHashValid;
    bool                    m_defineHashValid;
    bool                    m_memHashValid;

    Window*                 m_window;
};

//------------------------------------------------------------------------
}
