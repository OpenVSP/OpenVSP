// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "ConstrainedDelaunay2DWindow2.h"
#include <random>

ConstrainedDelaunay2DWindow2::ConstrainedDelaunay2DWindow2(
    Parameters& parameters)
    :
    Window2(parameters),
    mCurrentTriX(-1),
    mCurrentTriY(-1)
{
    // Randomly generated points.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.125f, 0.875f);
    mVertices.resize(256);
    for (auto& v : mVertices)
    {
        v[0] = mXSize*rnd(mte);
        v[1] = mYSize*rnd(mte);
    }

    int32_t numVertices = static_cast<int32_t>(mVertices.size());
    mDelaunay(numVertices, &mVertices[0], 0.001f);

    if (mDelaunay.GetDimension() == 2)
    {
        mDelaunay.GetHull(mHull);
    }
    else
    {
        LogError("Degenerate point set.");
    }

    mInfo.initialTriangle = -1;
    mInfo.finalTriangle = 0;
}

void ConstrainedDelaunay2DWindow2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    uint32_t const white = 0xFFFFFFFF;
    uint32_t const gray = 0xFF808080;
    uint32_t const red = 0xFF0000FF;
    uint32_t const blue = 0xFFFF0000;
    uint32_t const green = 0xFF00FF00;
    uint32_t const black = 0xFF000000;

    int32_t x0{}, y0{}, x1{}, y1{}, x2{}, y2{};
    Vector2<float> v0{}, v1{}, v2{};

    // Draw the triangle mesh.
    std::vector<int32_t> const& indices = mDelaunay.GetIndices();
    size_t i, numTriangles = indices.size() / 3;
    for (i = 0; i < numTriangles; ++i)
    {
        v0 = mVertices[indices[3 * i]];
        x0 = static_cast<int32_t>(std::lrint(v0[0]));
        y0 = static_cast<int32_t>(std::lrint(v0[1]));

        v1 = mVertices[indices[3 * i + 1]];
        x1 = static_cast<int32_t>(std::lrint(v1[0]));
        y1 = static_cast<int32_t>(std::lrint(v1[1]));

        v2 = mVertices[indices[3 * i + 2]];
        x2 = static_cast<int32_t>(std::lrint(v2[0]));
        y2 = static_cast<int32_t>(std::lrint(v2[1]));

        DrawLine(x0, y0, x1, y1, gray);
        DrawLine(x1, y1, x2, y2, gray);
        DrawLine(x2, y2, x0, y0, gray);
    }

    // Draw the hull.
    size_t numEdges = mHull.size() / 2;
    for (i = 0; i < numEdges; ++i)
    {
        v0 = mVertices[mHull[2 * i]];
        x0 = static_cast<int32_t>(std::lrint(v0[0]));
        y0 = static_cast<int32_t>(std::lrint(v0[1]));

        v1 = mVertices[mHull[2 * i + 1]];
        x1 = static_cast<int32_t>(std::lrint(v1[0]));
        y1 = static_cast<int32_t>(std::lrint(v1[1]));

        DrawLine(x0, y0, x1, y1, red);
    }

    // Draw edges <0,5>, <5,9>, and <9,0>.
    v0 = mVertices[0];
    x0 = static_cast<int32_t>(std::lrint(v0[0]));
    y0 = static_cast<int32_t>(std::lrint(v0[1]));
    v1 = mVertices[5];
    x1 = static_cast<int32_t>(std::lrint(v1[0]));
    y1 = static_cast<int32_t>(std::lrint(v1[1]));
    DrawLine(x0, y0, x1, y1, green);

    v0 = mVertices[9];
    x0 = static_cast<int32_t>(std::lrint(v0[0]));
    y0 = static_cast<int32_t>(std::lrint(v0[1]));
    DrawLine(x0, y0, x1, y1, green);

    v1 = mVertices[0];
    x1 = static_cast<int32_t>(std::lrint(v1[0]));
    y1 = static_cast<int32_t>(std::lrint(v1[1]));
    DrawLine(x0, y0, x1, y1, green);

    // Draw the search path.
    if (mInfo.initialTriangle != -1)
    {
        size_t numPath = static_cast<size_t>(mInfo.numPath);
        for (i = 0; i < numPath; ++i)
        {
            size_t index = static_cast<size_t>(mInfo.path[i]);

            v0 = mVertices[indices[3 * index]];
            v1 = mVertices[indices[3 * index + 1]];
            v2 = mVertices[indices[3 * index + 2]];

            Vector2<float> center = (v0 + v1 + v2) / 3.0f;
            int32_t x = static_cast<int32_t>(std::lrint(center[0]));
            int32_t y = static_cast<int32_t>(std::lrint(center[1]));
            if (i + 1 < numPath)
            {
                DrawFloodFill4(x, y, blue, white);
            }
            else
            {
                DrawFloodFill4(x, y, red, white);
            }
        }
    }

    if (mCurrentTriX >= 0)
    {
        // Draw the current triangle.
        DrawFloodFill4(mCurrentTriX, mCurrentTriY, green, red);
    }
    else if (mInfo.initialTriangle != -1)
    {
        // Draw the last edge when the selected point is outside the hull.
        v0 = mVertices[mInfo.finalV[0]];
        x0 = static_cast<int32_t>(std::lrint(v0[0]));
        y0 = static_cast<int32_t>(std::lrint(v0[1]));

        v1 = mVertices[mInfo.finalV[1]];
        x1 = static_cast<int32_t>(std::lrint(v1[0]));
        y1 = static_cast<int32_t>(std::lrint(v1[1]));

        DrawLine(x0, y0, x1, y1, black);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool ConstrainedDelaunay2DWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    std::vector<int32_t> outEdge;
    switch (key)
    {
    case '0':
        mDelaunay.Insert({ 0, 5 }, outEdge);
        mDelaunay.UpdateIndicesAdjacencies();
        OnDisplay();
        return true;
    case '1':
        mDelaunay.Insert({ 5, 9 }, outEdge);
        mDelaunay.UpdateIndicesAdjacencies();
        OnDisplay();
        return true;
    case '2':
        mDelaunay.Insert({ 9, 0 }, outEdge);
        mDelaunay.UpdateIndicesAdjacencies();
        OnDisplay();
        return true;
    }
    return Window2::OnCharPress(key, x, y);
}

bool ConstrainedDelaunay2DWindow2::OnMouseClick(MouseButton button, MouseState state,
    int32_t x, int32_t y, uint32_t)
{
    if (button != MOUSE_LEFT)
    {
        return false;
    }

    if (state == MOUSE_DOWN)
    {
        Vector2<float> pos{ static_cast<float>(x), static_cast<float>(y) };
        mInfo.initialTriangle = mInfo.finalTriangle;
        int32_t i = mDelaunay.GetContainingTriangle(pos, mInfo);
        if (i >= 0)
        {
            mCurrentTriX = x;
            mCurrentTriY = y;
            mInfo.finalTriangle = i;
        }
        else
        {
            mCurrentTriX = -1;
            mCurrentTriY = -1;
        }
        OnDisplay();
    }

    return true;
}
