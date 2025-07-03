// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Applications/GTApplicationsPCH.h>
#include <Applications/GLX/Console.h>
using namespace gte;

Console::Parameters::Parameters()
    :
    display(nullptr),
    window(0),
    deviceCreationFlags(0)
{
}

Console::Parameters::Parameters(std::wstring const& inTitle)
    :
    ConsoleApplication::Parameters(inTitle),
    display(nullptr),
    window(0),
    deviceCreationFlags(0)
{
}

Console::~Console()
{
}

Console::Console(Parameters& parameters)
    :
    ConsoleApplication(parameters),
    mEngine(std::static_pointer_cast<GraphicsEngine>(mBaseEngine))
{
}
