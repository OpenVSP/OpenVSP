#ifndef SIMPLETESS_H
#define SIMPLETESS_H

#include <vector>
using std::vector;

#include "Vec3d.h"
#include "Matrix4d.h"
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

    void CalcTexCoords();

    bool m_FlipNormal;

    int m_nufeat;
    int m_nvfeat;

    vector< vector< vector< vec3d > > > m_pnts;
    vector< vector< vector< vec3d > > > m_norms;

    vector< vector< vector< double > > > m_utex;
    vector< vector< vector< double > > > m_vtex;
};

#endif // SIMPLETESS_H