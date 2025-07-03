// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GLSLComputeProgram.h>
using namespace gte;

GLSLComputeProgram::~GLSLComputeProgram()
{
    if (glIsProgram(mProgramHandle))
    {
        if (glIsShader(mComputeShaderHandle))
        {
            glDetachShader(mProgramHandle, mComputeShaderHandle);
            glDeleteShader(mComputeShaderHandle);
        }

        glDeleteProgram(mProgramHandle);
    }
}

GLSLComputeProgram::GLSLComputeProgram(GLuint programHandle, GLuint computeShaderHandle)
    :
    mProgramHandle(programHandle),
    mComputeShaderHandle(computeShaderHandle),
    mReflector(programHandle)
{
}
