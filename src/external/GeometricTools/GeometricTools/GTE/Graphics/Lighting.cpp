// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Lighting.h>
#include <Mathematics/Constants.h>
using namespace gte;

Lighting::Lighting()
    :
    ambient{ 1.0f, 1.0f, 1.0f, 1.0f },
    diffuse{ 1.0f, 1.0f, 1.0f, 1.0f },
    specular{ 1.0f, 1.0f, 1.0f, 1.0f },
    spotCutoff{ (float)GTE_C_HALF_PI, 0.0f, 1.0f, 1.0f },
    attenuation{ 1.0f, 0.0f, 0.0f, 1.0f }
{
}
