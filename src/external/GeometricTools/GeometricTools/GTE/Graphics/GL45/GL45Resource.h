// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/Resource.h>
#include <Graphics/GL45/GL45GraphicsObject.h>

namespace gte
{
    class GL45Resource : public GL45GraphicsObject
    {
    public:
        // Abstract base class.
        virtual ~GL45Resource() = default;
    protected:
        GL45Resource(Resource const* gtResource);

    public:
        // Member access.
        inline Resource* GetResource() const
        {
            return static_cast<Resource*>(mGTObject);
        }

        // TODO: This is a tentative interface; modify as needed.  Make these
        // pure virtual latter (if relevant).
        void* MapForWrite(GLenum target);
        void Unmap(GLenum target);

        virtual bool Update()
        {
            return false;
        }

        virtual bool CopyCpuToGpu()
        {
            return false;
        }

        virtual bool CopyGpuToCpu()
        {
            return false;
        }

        virtual void CopyGpuToGpu(GL45Resource* target)
        {
            (void)target;
            throw std::logic_error(std::string(__FILE__) + "(" + std::string(__FUNCTION__) + "," + std::to_string(__LINE__) + "): Not yet implemented.\n");
        }

    protected:
        // Support for copying between CPU and GPU.
        bool PreparedForCopy(GLenum access) const;
    };
}
