//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// SubSurface.h
// Alex Gary
//////////////////////////////////////////////////////////////////////

#ifndef SUBSURFACE_INCLUDED_
#define SUBSURFACE_INCLUDED_

#include "VspSurf.h"
#include "TMesh.h"
#include "Vec2d.h"
#include "Vec3d.h"
#include "Parm.h"
#include "ParmContainer.h"
#include "DrawObj.h"

// SubSurface Line Segment
class SSLineSeg
{
public:
    // SubSurface UW Line Type
    SSLineSeg();

    virtual void Update( Geom* geom );

    enum { GT, LT };
    int m_TestType;

    virtual ~SSLineSeg();
    virtual bool Subtag( TTri* tri ) const;
    virtual bool Subtag( const vec3d & center ) const;
    virtual TMesh* CreateTMesh();
    virtual void SetSP0( vec3d pnt )
    {
        m_SP0 = pnt;
    }
    virtual void SetSP1( vec3d pnt )
    {
        m_SP1 = pnt;
    }
    virtual void SetP0( vec3d pnt )
    {
        m_P0 = pnt;
    }
    virtual void SetP1( vec3d pnt )
    {
        m_P1 = pnt;
    }
    virtual vec3d GetP0()
    {
        return m_P0;
    }
    virtual vec3d GetP1()
    {
        return m_P1;
    }
    virtual void UpdateDrawObj( Geom* geom, DrawObj& draw_obj, const int *num_pnts_ptr );
    virtual int CompNumDrawPnts( Geom* geom );

protected:
    vec3d m_SP0; // scaled uw points
    vec3d m_SP1;

    vec3d CompPnt( VspSurf* surf, vec3d uw_pnt ) const;

private:
    vec3d m_P0; // none scaled uw points
    vec3d m_P1;
    vec3d m_line;
};

