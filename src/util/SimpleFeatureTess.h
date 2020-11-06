#ifndef SIMPLEFEATURETESS_H
#define SIMPLEFEATURETESS_H

#include <vector>
using std::vector;

#include "Vec3d.h"
#include "Matrix4d.h"
#include "BndBox.h"

class SimpleFeatureTess
{
public:
    SimpleFeatureTess();
    virtual ~SimpleFeatureTess();

    bool GetFlipNormal() const { return m_FlipNormal; }
    void FlipNormal() { m_FlipNormal = !m_FlipNormal; }
    void ResetFlipNormal( ) { m_FlipNormal = false; }

    void Transform( const Matrix4d & mat );

    bool m_FlipNormal;

    vector < vector < vec3d > > m_ptline;
};

#endif // SIMPLEFEATURETESS_H
