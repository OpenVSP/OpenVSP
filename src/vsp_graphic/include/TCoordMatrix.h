#ifndef _VSP_GRAPHIC_TEXTURE_TRANSFORM_MATRIX_H
#define _VSP_GRAPHIC_TEXTURE_TRANSFORM_MATRIX_H

#include <vector>

#include "glm/glm.hpp"

namespace VSPGraphic
{
/*!
* Texture Matrix that handles texture transformations.
*/
class TCoordMatrix
{
public:
    /*!
    * Constructor.
    */
    TCoordMatrix();
    /*!
    * Destructor.
    */
    virtual ~TCoordMatrix();

public:
    /*!
    * Translate Texture.
    *
    * s - move along s or x axis.
    * t - move along t or y axis.
    * r - move along r or z axis.
    */
    virtual void translate( float s, float t, float r = 0.f );

    /*!
    * Scale Texture.
    *
    * sRatio - scale along s or x axis.
    * tRatio - scale along t or y axis.
    * rRatio - scale along r or z axis.
    */
    virtual void scale( float sRatio, float tRatio, float rRatio = 1.f );

public:
    /*!
    * Get translate value of s.
    */
    float getTranslateS();
    /*!
    * Get translate value of t.
    */
    float getTranslateT();
    /*!
    * Get translate value of r.
    */
    float getTranslateR();

public:
    /*!
    * Get scale value of s.
    */
    float getScaleS();
    /*!
    * Get scale value of t.
    */
    float getScaleT();
    /*!
    * Get scale value of r.
    */
    float getScaleR();

public:
    /*!
    * Get Texture Transformation Matrix.
    */
    glm::mat4 getTextureTransformationMatrix();

protected:
    glm::mat4 _tMat;
};
}
#endif