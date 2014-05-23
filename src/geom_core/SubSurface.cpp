//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// SubSurface.cpp
// Alex Gary
//////////////////////////////////////////////////////////////////////

#include "SubSurface.h"
#include "Geom.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "VspSurf.h"
#include "ParmMgr.h"
#include "Util.h"
#include "Matrix.h"
#include "Defines.h"

//////////////////////////////////////////////////////
//=================== SSLineSeg =====================//
//////////////////////////////////////////////////////

SSLineSeg::SSLineSeg()
{
    m_TestType = GT;
}

SSLineSeg::~SSLineSeg()
{
}

bool SSLineSeg::Subtag( const vec3d & center ) const
{
    // Compute cross product of line and first point to center
    vec3d v0c = center - m_P0;
    vec3d c_prod = cross( m_line, v0c );

    if ( m_TestType == GT && c_prod.z() > 0 )
    {
        return true;
    }
    if ( m_TestType == LT && c_prod.z() < 0 )
    {
        return true;
    }

    return false;
}

bool SSLineSeg::Subtag( TTri* tri ) const
{
    vec3d center = tri->ComputeCenterUW();

    return Subtag( center );
}

void SSLineSeg::Update( Geom* geom )
{
    VspSurf* surf = geom->GetSurfPtr();
    if ( !surf )
    {
        return;
    }

    int num_u = surf->GetNumSectU();
    int num_w = surf->GetNumSectW();
    // Update none scaled points
    m_P0.set_xyz( m_SP0[0]*num_u, m_SP0[1]*num_w, 0 );
    m_P1.set_xyz( m_SP1[0]*num_u, m_SP1[1]*num_w, 0 );

    // Update line
    m_line = m_P1 - m_P0;
}

int SSLineSeg::CompNumDrawPnts( Geom* geom )
{
    VspSurf* surf = geom->GetSurfPtr();
    if ( !surf )
    {
        return 0;
    }
    double avg_num_secs = ( double )( surf->GetNumSectU() + surf->GetNumSectW() ) / 2.0;
    double avg_tess = ( double )( geom->m_TessU() + geom->m_TessW() ) / 2.0;

    return ( int )avg_num_secs * ( avg_tess - 1 );
}

void SSLineSeg::UpdateDrawObj( Geom* geom, DrawObj& draw_obj, const int *num_pnts_ptr )
{
    int num_pnts;
    if ( num_pnts_ptr )
    {
        num_pnts = *num_pnts_ptr;
    }
    else
    {
        num_pnts = CompNumDrawPnts( geom );
    }

    draw_obj.m_PntVec.resize( num_pnts * 2 );

    VspSurf* surf = geom->GetSurfPtr();
    draw_obj.m_PntVec[0] = CompPnt( surf, m_P0 );
    int pi = 1;
    for ( int i = 1 ; i < num_pnts ; i ++ )
    {
        vec3d uw = ( m_P0 + m_line * ( ( double )i / num_pnts ) );
        draw_obj.m_PntVec[pi] = CompPnt( surf, uw );
        draw_obj.m_PntVec[pi + 1] = draw_obj.m_PntVec[pi];
        pi += 2;
    }

    draw_obj.m_PntVec[pi] = CompPnt( surf, m_P1 );

    draw_obj.m_LineWidth = 3.0;
    draw_obj.m_LineColor = vec3d( 177.0 / 255, 1, 58.0 / 255 );
    draw_obj.m_Type = DrawObj::VSP_LINES;
    draw_obj.m_GeomChanged = true;
}

vec3d SSLineSeg::CompPnt( VspSurf* surf, vec3d uw_pnt ) const
{
    if ( !surf )
    {
        return vec3d();
    }

    int num_u = surf->GetNumSectU();
    int num_w = surf->GetNumSectW();

    if ( uw_pnt.x() < 0 )
    {
        uw_pnt.set_x( 0 );
    }
    else if ( uw_pnt.x() > num_u )
    {
        uw_pnt.set_x( num_u );
    }

    if ( uw_pnt.y() < 0 )
    {
        uw_pnt.set_y( 0 );
    }
    else if ( uw_pnt.y() > num_w )
    {
        uw_pnt.set_y( num_w );
    }

    return surf->CompPnt( uw_pnt.x(), uw_pnt.y() );
}

TMesh* SSLineSeg::CreateTMesh()
{
    int num_cut_lines = 0;
    int num_z_lines = 0;

    TMesh* tmesh = new TMesh();

    vec3d dc = m_line / ( num_cut_lines + 1.0 );
    vec3d dz = vec3d( 0, 0, 2.0 ) / ( num_z_lines + 1 );
    vec3d start = m_P0 + vec3d( 0, 0, -1 );

    int c, cz;

    vector< vector< vec3d > > pnt_mesh;
    pnt_mesh.resize( num_cut_lines + 2 );
    for ( int i = 0; i < ( int )pnt_mesh.size(); i++ )
    {
        pnt_mesh[i].resize( num_z_lines + 2 );
    }

    // Build plane
    for ( c = 0 ; c < num_cut_lines + 2 ; c++ )
    {
        for ( cz = 0 ; cz < num_z_lines + 2 ; cz++ )
        {
            pnt_mesh[c][cz] = start + dc * c + dz * cz;
        }
    }

    // Build triangles on that plane

    for ( c = 0 ; c < ( int )pnt_mesh.size() - 1 ; c++ )
    {
        for ( cz = 0 ; cz < ( int )pnt_mesh[c].size() - 1 ; cz ++ )
        {
            vec3d v0, v1, v2, v3, d01, d21, d20, d03, d23, norm;

            v0 = pnt_mesh[c][cz];
            v1 = pnt_mesh[c + 1][cz];
            v2 = pnt_mesh[c + 1][cz + 1];
            v3 = pnt_mesh[c][cz + 1];

            d21 = v2 - v1;
            d01 = v0 - v1;
            d20 = v2 - v0;

            if ( d21.mag() > 0.000001 && d01.mag() > 0.000001 && d20.mag() > 0.000001 )
            {
                norm = cross( d21, d01 );
                norm.normalize();
                tmesh->AddUWTri( v0, v1, v2, norm );
            }

            d03 = v0 - v3;
            d23 = v2 - v3;
            if ( d03.mag() > 0.000001 && d23.mag() > 0.000001 && d20.mag() > 0.000001 )
            {
                norm = cross( d03, d23 );
                norm.normalize();
                tmesh->AddUWTri( v0, v2, v3, norm );
            }
        }
    }
    return tmesh;
}

