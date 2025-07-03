// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11TextureSingle.h>
using namespace gte;

DX11TextureSingle::DX11TextureSingle(TextureSingle const* gtTextureSingle)
    :
    DX11Texture(gtTextureSingle)
{
}
