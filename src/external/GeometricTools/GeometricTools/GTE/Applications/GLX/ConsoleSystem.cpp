// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Applications/GTApplicationsPCH.h>
#include <Applications/GLX/Console.h>
using namespace gte;

// The singleton used to create and destroy consoles for applications.
namespace gte
{
    ConsoleSystem TheConsoleSystem;
}

#if defined(GTE_USE_DIRECTX)
void ConsoleSystem::CreateEngineAndProgramFactory(Console::Parameters& parameters)
{
    auto engine = std::make_shared<DX11Engine>(nullptr, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, parameters.deviceCreationFlags);

    if (engine->GetDevice())
    {
        parameters.engine = engine;
        parameters.factory = std::make_shared<HLSLProgramFactory>();
        parameters.created = true;
    }
    else
    {
        LogError("Cannot create compute engine.");
    }
}
#endif

#if defined(GTE_USE_OPENGL)
#if defined(GTE_USE_MSWINDOWS)
void ConsoleSystem::CreateEngineAndProgramFactory(Console::Parameters& parameters)
{
    bool saveDriverInfo = ((parameters.deviceCreationFlags & 0x00000001) != 0);
    auto engine = std::make_shared<WGLEngine>(false, saveDriverInfo);
    if (!engine->MeetsRequirements())
    {
        LogError("OpenGL 4.5 or later is required.");
    }

    if (engine->GetDevice())
    {
        parameters.engine = engine;
        parameters.factory = std::make_shared<GLSLProgramFactory>();
        parameters.created = true;
    }
    else
    {
        LogError("Cannot create compute engine.");
    }
}
#endif

#if defined(GTE_USE_LINUX)
void ConsoleSystem::CreateEngineAndProgramFactory(Console::Parameters& parameters)
{
    // The construction of GLXEngine requires a depth24-stencil8 buffer
    // in order for X Windows to succeed in the call to glXChooseVisual.
    bool saveDriverInfo = ((parameters.deviceCreationFlags & 0x00000001) != 0);
    auto engine = std::make_shared<GLXEngine>(true, saveDriverInfo);
    if (!engine->MeetsRequirements())
    {
        LogError("OpenGL 4.5 or later is required.");
    }

    if (engine->GetDisplay())
    {
        parameters.display = engine->GetDisplay();
        parameters.engine = engine;
        parameters.factory = std::make_shared<GLSLProgramFactory>();
        parameters.created = true;
    }
    else
    {
        LogError("Cannot create graphics engine.");
        parameters.display = nullptr;
        parameters.window = 0;
        parameters.engine = nullptr;
        parameters.factory = nullptr;
        parameters.created = false;
    }
}
#endif
#endif
