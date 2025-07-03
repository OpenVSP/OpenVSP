// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/GraphicsObject.h>
#include <cstdint>
#include <vector>

namespace gte
{
    class Resource : public GraphicsObject
    {
    public:
        // Abstract base class.
        virtual ~Resource();
    protected:
        // The default usage is Resource::Usage::IMMUTABLE.
        Resource(uint32_t numElements, size_t elementSize, bool createStorage = true);

    public:
        // Create or destroy the system-memory storage associated with the
        // resource.  A resource does not necessarily require system memory
        // if it is intended only to provide information for GPU-resource
        // creation.
        void CreateStorage();
        void DestroyStorage();

        // Basic member access.
        inline uint32_t GetNumElements() const
        {
            return mNumElements;
        }

        inline uint32_t GetElementSize() const
        {
            return mElementSize;
        }

        inline uint32_t GetNumBytes() const
        {
            return mNumBytes;
        }

        // The resource usage.  These control how the GPU versions are
        // created.  You must set the usage type before binding the resource
        // to an engine.
        enum Usage
        {
            IMMUTABLE,
            DYNAMIC_UPDATE,
            SHADER_OUTPUT
        };

        inline void SetUsage(Usage usage)
        {
            mUsage = usage;
        }

        inline Usage GetUsage() const
        {
            return mUsage;
        }

        // Internal staging buffer generation.  These control creation of
        // staging buffers to support copies between CPU, staging buffers, and
        // GPU.  You must set the copy type before binding the resource to an
        // engine.
        enum Copy
        {
            NONE,
            CPU_TO_STAGING,
            STAGING_TO_CPU,
            BIDIRECTIONAL
        };

        inline void SetCopy(Copy copy)
        {
            mCopy = copy;
        }

        inline Copy GetCopy() const
        {
            return mCopy;
        }

        // Member access to the raw data of the resource.  The ResetData call
        // sets mData to the storage (or to nullptr if storage does not
        // exist.)  The caller of SetData is responsible for ensuring 'data'
        // has at least mNumBytes elements.  The template member functions are
        // a convenience for accessing the raw data as a specified type.
        inline void ResetData()
        {
            mData = (mStorage.size() > 0 ? mStorage.data() : nullptr);
        }

        inline void SetData(char* data)
        {
            mData = data;
        }

        inline char const* GetData() const
        {
            return mData;
        }

        inline char* GetData()
        {
            return mData;
        }

        template <typename T>
        inline void Set(T* data)
        {
            mData = reinterpret_cast<char*>(data);
        }

        template <typename T>
        inline T const* Get() const
        {
            return reinterpret_cast<T const*>(mData);
        }

        template <typename T>
        inline T* Get()
        {
            return reinterpret_cast<T*>(mData);
        }

        inline void Reset()
        {
            mData = (mStorage.size() > 0 ? mStorage.data() : nullptr);
        }

        // Specify a contiguous block of active elements in the resource.  An
        // element occupies mElementSize bytes.  Treating the data array as an
        // array of elements, mOffset is the index of the first active
        // element; that is, the first element has address
        //   mData + mOffset * mElementSize
        // The offset must satisfy
        //   mOffset < mNumElements
        // The number of active elements is mNumActiveElements.  It must
        // satisfy
        //   mNumActiveElements <= mNumElements - mOffset
        // If you plan on modifying both mOffset and mNumActiveElements for
        // the same object, call SetOffset() before SetNumActiveElements()
        // because of the dependency of the num-active constraint on offset.
        void SetOffset(uint32_t offset);

        inline uint32_t GetOffset() const
        {
            return mOffset;
        }

        void SetNumActiveElements(uint32_t numActiveElements);

        inline uint32_t GetNumActiveElements() const
        {
            return mNumActiveElements;
        }

        inline uint32_t GetNumActiveBytes() const
        {
            return mNumActiveElements * mElementSize;
        }

    protected:
        uint32_t mNumElements;          // default: 0
        uint32_t mElementSize;          // default: 0
        uint32_t mNumBytes;             // default: 0
        Usage mUsage;                   // default: IMMUTABLE
        Copy mCopy;                     // default: NONE
        uint32_t mOffset;               // default: 0
        uint32_t mNumActiveElements;    // default: 0
        std::vector<char> mStorage;     // default: empty
        char* mData;                    // default: nullptr
    };
}
