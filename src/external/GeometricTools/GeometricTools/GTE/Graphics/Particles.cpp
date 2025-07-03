// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Particles.h>
#include <Mathematics/Logger.h>
#include <Mathematics/Vector2.h>
using namespace gte;

Particles::Particles(std::vector<Vector4<float>> const& positionSize,
    float sizeAdjust, VertexFormat const& vformat)
    :
    mPositionSize(positionSize),
    mSizeAdjust(sizeAdjust),
    mNumActive(static_cast<uint32_t>(positionSize.size()))
{
    uint32_t texOffset = IsValid(vformat);
    if (texOffset == std::numeric_limits<uint32_t>::max())
    {
        // IsValid(...) will generate logging messages depending on the
        // specific failure condition.
        return;
    }

    uint32_t numParticles = mNumActive;
    uint32_t numVertices = 4 * numParticles;
    mVBuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    mVBuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    uint32_t vertexSize = vformat.GetVertexSize();
    int32_t index = vformat.GetIndex(VASemantic::TEXCOORD, 0);
    uint32_t offset = vformat.GetOffset(index);
    char* tcoords = mVBuffer->GetData() + offset;
    std::array<Vector2<float>, 4> commonTCD =
    {
        Vector2<float>{ 0.0f, 0.0f },
        Vector2<float>{ 1.0f, 0.0f },
        Vector2<float>{ 1.0f, 1.0f },
        Vector2<float>{ 0.0f, 1.0f }
    };
    for (uint32_t i = 0; i < numParticles; ++i)
    {
        for (uint32_t j = 0; j < 4; ++j)
        {
            Vector2<float>& tcoord = *reinterpret_cast<Vector2<float>*>(tcoords);
            tcoord = commonTCD[j];
            tcoords += vertexSize;
        }
    }

    mIBuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2 * numParticles, sizeof(uint32_t));
    auto* indices = mIBuffer->Get<uint32_t>();
    for (uint32_t i = 0; i < numParticles; ++i)
    {
        uint32_t iFI = 4 * i;
        uint32_t iFIp1 = iFI + 1;
        uint32_t iFIp2 = iFI + 2;
        uint32_t iFIp3 = iFI + 3;
        *indices++ = iFI;
        *indices++ = iFIp1;
        *indices++ = iFIp2;
        *indices++ = iFI;
        *indices++ = iFIp2;
        *indices++ = iFIp3;
    }

    UpdateModelBound();
}

void Particles::SetSizeAdjust(float sizeAdjust)
{
    LogAssert(sizeAdjust > 0.0f, "Invalid size-adjust parameter.");
    mSizeAdjust = sizeAdjust;
}

void Particles::SetNumActive(uint32_t numActive)
{
    uint32_t numParticles = static_cast<uint32_t>(mPositionSize.size());
    if (numActive <= numParticles)
    {
        mNumActive = numActive;
    }
    else
    {
        mNumActive = numParticles;
    }

    mVBuffer->SetNumActiveElements(4 * mNumActive);
    mIBuffer->SetNumActiveElements(6 * mNumActive);
}

void Particles::GenerateParticles(std::shared_ptr<Camera> const& camera)
{
    // Get access to the positions.
    VertexFormat vformat = mVBuffer->GetFormat();
    uint32_t vertexSize = vformat.GetVertexSize();
    char* vertices = mVBuffer->GetData();

    // Get camera axis directions in model space of particles.
    Matrix4x4<float> inverse = worldTransform.GetHInverse();
    Vector4<float> UpR = inverse * (camera->GetUVector() + camera->GetRVector());
    Vector4<float> UmR = inverse * (camera->GetUVector() - camera->GetRVector());

    // Generate quadrilaterals as pairs of triangles.
    for (uint32_t i = 0; i < mNumActive; ++i)
    {
        Vector4<float> posSize = mPositionSize[i];
        Vector3<float> position{ posSize[0], posSize[1], posSize[2] };
        float trueSize = mSizeAdjust * posSize[3];
        Vector3<float> scaledUpR = HProject(trueSize * UpR);
        Vector3<float> scaledUmR = HProject(trueSize * UmR);

        Vector3<float>& pos0 = *reinterpret_cast<Vector3<float>*>(vertices);
        pos0 = position - scaledUpR;
        vertices += vertexSize;

        Vector3<float>& pos1 = *reinterpret_cast<Vector3<float>*>(vertices);
        pos1 = position - scaledUmR;
        vertices += vertexSize;

        Vector3<float>& pos2 = *reinterpret_cast<Vector3<float>*>(vertices);
        pos2 = position + scaledUpR;
        vertices += vertexSize;

        Vector3<float>& pos3 = *reinterpret_cast<Vector3<float>*>(vertices);
        pos3 = position + scaledUmR;
        vertices += vertexSize;
    }

    UpdateModelBound();
}

uint32_t Particles::IsValid(VertexFormat const& vformat) const
{
    // Validate the vertex position.
    int32_t index = vformat.GetIndex(VASemantic::POSITION, 0);
    if (index < 0)
    {
#if defined(GTE_THROW_ON_PARTICLES_INVALID)
        LogError("Vertex format does not have VASemantic::POSITION.");
#else
        return std::numeric_limits<uint32_t>::max();
#endif
    }

    uint32_t posType = vformat.GetType(index);
    if (posType != DF_R32G32B32_FLOAT && posType != DF_R32G32B32A32_FLOAT)
    {
#if defined(GTE_THROW_ON_PARTICLES_INVALID)
        LogError("Invalid position type.");
#else
        return std::numeric_limits<uint32_t>::max();
#endif
    }

    uint32_t offset = vformat.GetOffset(index);
    if (offset != 0)
    {
#if defined(GTE_THROW_ON_PARTICLES_INVALID)
        LogError("Position offset must be 0.");
#else
        return std::numeric_limits<uint32_t>::max();
#endif
    }

    // Validate the vertex texture coordinate that is used for drawing the
    // billboards.
    index = vformat.GetIndex(VASemantic::TEXCOORD, 0);
    if (index < 0)
    {
#if defined(GTE_THROW_ON_PARTICLES_INVALID)
        LogError("Vertex format does not have VASemantic::TEXCOORD.");
#else
        return std::numeric_limits<uint32_t>::max();
#endif
    }

    uint32_t texType = vformat.GetType(index);
    if (texType != DF_R32G32_FLOAT)
    {
#if defined(GTE_THROW_ON_PARTICLES_INVALID)
        LogError("Invalid texture coordinate type.");
#else
        return std::numeric_limits<uint32_t>::max();
#endif
    }

    uint32_t texOffset = (posType == DF_R32G32B32_FLOAT ? 3 * sizeof(float) : 4 * sizeof(float));
    offset = vformat.GetOffset(index);
    if (offset != texOffset)
    {
#if defined(GTE_THROW_ON_PARTICLES_INVALID)
        LogError("Texture coordinate must immediately follow position.");
#else
        return std::numeric_limits<uint32_t>::max();
#endif
    }

    return texOffset;
}
