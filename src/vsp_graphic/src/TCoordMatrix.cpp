#include "TCoordMatrix.h"

namespace VSPGraphic
{
TCoordMatrix::TCoordMatrix()
{
    _tMat = glm::mat4( 1.0 );
}
TCoordMatrix::~TCoordMatrix()
{
}

void TCoordMatrix::translate( float s, float t, float r )
{
    _tMat[3][0] = s;
    _tMat[3][1] = t;
    _tMat[3][2] = r;
}

void TCoordMatrix::scale( float sRatio, float tRatio, float rRatio )
{
    _tMat[0][0] = sRatio;
    _tMat[1][1] = tRatio;
    _tMat[2][2] = rRatio;
}

float TCoordMatrix::getTranslateS()
{
    return _tMat[3][0];
}

float TCoordMatrix::getTranslateT()
{
    return _tMat[3][1];
}

float TCoordMatrix::getTranslateR()
{
    return _tMat[3][2];
}

float TCoordMatrix::getScaleS()
{
    return _tMat[0][0];
}

float TCoordMatrix::getScaleT()
{
    return _tMat[1][1];
}

float TCoordMatrix::getScaleR()
{
    return _tMat[2][2];
}

glm::mat4 TCoordMatrix::getTextureTransformationMatrix()
{
    return _tMat;
}
}