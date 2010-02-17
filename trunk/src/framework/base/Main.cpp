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
 
#include "base/Main.hpp"
#include "base/DLLImports.hpp"
#include "base/Thread.hpp"
#include "gui/Window.hpp"
#include "gpu/GLContext.hpp"
#include "gpu/CudaModule.hpp"
#include "gpu/CudaCompiler.hpp"

#include <crtdbg.h>
#include <conio.h>

using namespace FW;

//------------------------------------------------------------------------

int     FW::argc        = 0;
char**  FW::argv        = NULL;
int     FW::exitCode    = 0;

//------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    // Store arguments.

    FW::argc = argc;
    FW::argv = argv;

    // Force the main thread to run on a single core.

    SetThreadAffinityMask(GetCurrentThread(), 1);

    // Initialize CRTDBG.

#if FW_DEBUG
    int flag = 0;
    flag |= _CRTDBG_ALLOC_MEM_DF;       // use allocation guards
//  flag |= _CRTDBG_CHECK_ALWAYS_DF;    // check whole memory on each alloc
//  flag |= _CRTDBG_DELAY_FREE_MEM_DF;  // keep freed memory and check that it isn't written
    _CrtSetDbgFlag(flag);

//  _CrtSetBreakAlloc(64);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif

    // Initialize the framework.

    initDLLImports();
    failIfError();

    // Initialize the application.

    FW::init();
    failIfError();

    // Message loop.

    while (Window::getNumOpen())
    {
        Window::realizeAll();

        MSG msg;
        GetMessage(&msg, NULL, 0, 0);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Clean up.

    failIfError();
    CudaCompiler::staticDeinit();
    CudaModule::staticDeinit();
    GLContext::staticDeinit();
    Window::staticDeinit();
    deinitDLLImports();
    profileEnd();
    failIfError();

    while (hasLogFile())
        popLogFile();

    Thread::staticDeinit();

    // Dump memory leaks.

#if FW_DEBUG
    if (_CrtDumpMemoryLeaks())
    {
        printf("Press any key to continue . . . ");
        _getch();
        printf("\n");
    }
#endif

    return exitCode;
}

//------------------------------------------------------------------------
