#ifndef SIMPLETESS_H
#define SIMPLETESS_H

#include <vector>
using std::vector;

#include "Vec3d.h"
#include "Matrix.h"
#include "BndBox.h"

class SimpleTess
{
public:
    SimpleTess();
    virtual ~SimpleTess();

    bool GetFlipNormal() const { return m_FlipNormal; }
    void FlipNormal() { m_FlipNormal = !m_FlipNormal; }
    void ResetFlipNormal( ) { m_FlipNormal = false; }

    void Transform( const Matrix4d & mat );

    void GetBoundingBox( BndBox &bb ) const;

    bool m_FlipNormal;

    vector< vector< vector< vec3d > > > m_pnts;
    vector< vector< vector< vec3d > > > m_norms;
};

#endif // SIMPLETESS_H