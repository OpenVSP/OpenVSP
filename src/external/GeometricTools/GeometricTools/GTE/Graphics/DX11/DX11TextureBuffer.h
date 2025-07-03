// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/TextureBuffer.h>
#include <Graphics/DX11/DX11Buffer.h>

namespace gte
{
    class DX11TextureBuffer : public DX11Buffer
    {
    public:
        // Construction and destruction.
        virtual ~DX11TextureBuffer();
        DX11TextureBuffer(ID3D11Device* device, TextureBuffer const* tbuffer);
        static std::shared_ptr<GEObject> Create(void* device, GraphicsObject const* object);

        // Member access.
        inline TextureBuffer* GetTextureBuffer() const
        {
            return static_cast<TextureBuffer*>(mGTObject);
        }

        inline ID3D11ShaderResourceView* GetSRView() const
        {
            return mSRView;
        }

        // Support for the DX11 debug layer; see comments in the file
        // DX11GraphicsObject.h about usage.
        virtual void SetName(std::string const& name);

    private:
        // Support for construction.
        void CreateSRView(ID3D11Device* device);

        ID3D11ShaderResourceView* mSRView;
    };
}
