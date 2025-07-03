// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/TextureCube.h>
#include <Graphics/DX11/DX11TextureArray.h>

namespace gte
{
    class DX11TextureCube : public DX11TextureArray
    {
    public:
        // Construction and destruction.
        virtual ~DX11TextureCube() = default;
        DX11TextureCube(ID3D11Device* device, TextureCube const* textureCube);
        static std::shared_ptr<GEObject> Create(void* device, GraphicsObject const* object);

        // Member access.
        inline TextureCube* GetTexture() const
        {
            return static_cast<TextureCube*>(mGTObject);
        }

        inline ID3D11Texture2D* GetDXTexture() const
        {
            return static_cast<ID3D11Texture2D*>(mDXObject);
        }

    private:
        // Support for construction.
        void CreateStaging(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
        void CreateSRView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
        void CreateUAView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
    };
}
