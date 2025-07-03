// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Resource.h>
#include <Mathematics/Logger.h>
using namespace gte;

Resource::~Resource()
{
    DestroyStorage();
}

Resource::Resource(uint32_t numElements, size_t elementSize, bool createStorage)
    :
    mNumElements(numElements),
    mElementSize(static_cast<uint32_t>(elementSize)),
    mUsage(Usage::IMMUTABLE),
    mCopy(Copy::NONE),
    mOffset(0),
    mData(nullptr)
{
    mType = GT_RESOURCE;

    if (mNumElements > 0)
    {
        if (mElementSize > 0)
        {
            mNumBytes = mNumElements * mElementSize;
            mNumActiveElements = mNumElements;
            if (createStorage)
            {
                CreateStorage();
            }
        }
        else
        {
            // The VertexBuffer constructor that takes only the number of
            // vertices has been called.  The vertex shader code is maintained
            // completely in the HLSL.
            mNumBytes = 0;
            mNumActiveElements = mNumElements;
        }
    }
    else
    {
        // No assertion may occur here.  The VertexBuffer constructor with a
        // VertexFormat of zero attributes (used for vertex-ID-based drawing)
        // and the IndexBuffer constructor for which no indices are provided
        // will lead to this path.
        mNumBytes = 0;
        mElementSize = 0;
        mNumActiveElements = 0;
    }
}

void Resource::CreateStorage()
{
    if (mStorage.size() == 0)
    {
        mStorage.resize(mNumBytes);
        if (!mData)
        {
            mData = mStorage.data();
        }
    }
}

void Resource::DestroyStorage()
{
    // The intent of DestroyStorage is to free up CPU memory that is not
    // required when the resource GPU memory is all that is required.
    // The 'clear' call sets the size to 0, but the capacity remains the
    // same; that is, the memory is not freed.  The 'shrink_to_fit' call
    // is required to free the memory.
    if (mStorage.size() > 0 && mData == mStorage.data())
    {
        mData = nullptr;
        mStorage.clear();
        mStorage.shrink_to_fit();
    }
}

void Resource::SetOffset(uint32_t offset)
{
    if (offset < mNumElements)
    {
        mOffset = offset;
    }
    else
    {
        mOffset = 0;
        std::string message = "Invalid offset (" + std::to_string(offset) + ") for " +
            mName + "; total elements = " + std::to_string(mNumElements) + ".";
        LogError(message);
    }
}

void Resource::SetNumActiveElements(uint32_t numActiveElements)
{
    if (numActiveElements + mOffset <= mNumElements)
    {
        mNumActiveElements = numActiveElements;
    }
    else
    {
        mNumActiveElements = static_cast<uint32_t>(mNumElements - mOffset);
        std::string message = "Invalid number of active elements (" +
            std::to_string(numActiveElements) + ") for " + mName +
            "; offset = " + std::to_string(mOffset) + ", total elements = " +
            std::to_string(mNumElements) + ".";
        LogError(message);
    }
}
