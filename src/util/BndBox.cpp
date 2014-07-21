//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Bounding Box  BndBox.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#include "BndBox.h"
#include <assert.h>


//===== Constructor =====//
BndBox::BndBox()
{
    m_Min.set_xyz( 1.0e12, 1.0e12, 1.0e12 );
    m_Max.set_xyz( -1.0e12, -1.0e12, -1.0e12 );
}

//===== Constructor:   =====//
BndBox::BndBox( const vec3d& min_pnt, const vec3d& max_pnt )
{
    m_Min = min_pnt;
    m_Max = max_pnt;
}

//==== Reset Box ====//
void BndBox::Reset()
{
    m_Min.set_xyz( 1.0e12, 1.0e12, 1.0e12 );
    m_Max.set_xyz( -1.0e12, -1.0e12, -1.0e12 );
}

//====== Max[i] = value ======//
void BndBox::SetMax( int ind, double val )
{
    assert( ind >= 0 && ind < 3 );
    m_Max[ind] = val;
}

//====== Min[i] = value ======//
void BndBox::SetMin( int ind, double val )
{
    assert( ind >= 0 && ind < 3 );
    m_Min[ind] = val;
}

//====== f = max[i] ======//
double BndBox::GetMax( int ind ) const
{
    assert( ind >= 0 && ind < 3 );
    return m_Max[ind];
}

//====== f = min[i] ======//
double BndBox::GetMin( int ind ) const
{
    assert( ind >= 0 && ind < 3 );
    return m_Min[ind];
}

//==== Update Bounding Box - vec3d ====//
void BndBox::Update( const vec3d& pnt )
{
    if ( pnt.x() < m_Min[0] )
    {
        m_Min[0] = pnt.x();
    }
    if ( pnt.x() > m_Max[0] )
    {
        m_Max[0] = pnt.x();
    }

    if ( pnt.y() < m_Min[1] )
    {
        m_Min[1] = pnt.y();
    }
    if ( pnt.y() > m_Max[1] )
    {
        m_Max[1] = pnt.y();
    }

    if ( pnt.z() < m_Min[2] )
    {
        m_Min[2] = pnt.z();
    }
    if ( pnt.z() > m_Max[2] )
    {
        m_Max[2] = pnt.z();
    }
}

//==== Update Bounding Box - Bbox ====//
void BndBox::Update( const BndBox& bb )
{
    if ( bb.m_Min[0] < m_Min[0] )
    {
        m_Min[0] = bb.m_Min[0];
    }
    if ( bb.m_Min[1] < m_Min[1] )
    {
        m_Min[1] = bb.m_Min[1];
    }
    if ( bb.m_Min[2] < m_Min[2] )
    {
        m_Min[2] = bb.m_Min[2];
    }

    if ( bb.m_Max[0] > m_Max[0] )
    {
        m_Max[0] = bb.m_Max[0];
    }
    if ( bb.m_Max[1] > m_Max[1] )
    {
        m_Max[1] = bb.m_Max[1];
    }
    if ( bb.m_Max[2] > m_Max[2] )
    {
        m_Max[2] = bb.m_Max[2];
    }
}

//==== Get Diagonal Dist ====//
double BndBox::DiagDist() const
{
    return ( dist( m_Min, m_Max ) );
}

//==== Get Largest Dimension ====//
double BndBox::GetLargestDist() const
{
    double del_x = m_Max[0] - m_Min[0];
    double del_y = m_Max[1] - m_Min[1];
    double del_z = m_Max[2] - m_Min[2];

    if ( del_x > del_y && del_x > del_z )
    {
        return( del_x );
    }
    else if ( del_y > del_z )
    {
        return( del_y );
    }
    else
    {
        return( del_z );
    }
}

//==== Get Estimated Area ====//
double BndBox::GetEstArea() const
{
    double del_x = m_Max[0] - m_Min[0];
    double del_y = m_Max[1] - m_Min[1];
    double del_z = m_Max[2] - m_Min[2];

    if ( del_x >= del_z && del_y >= del_z )
    {
        return( del_x * del_y );
    }
    else if ( del_x >= del_y && del_z >= del_y )
    {
        return( del_x * del_z );
    }
    else
    {
        return( del_y * del_z );
    }
}


//==== Get Center f Bounding Box ====//
vec3d BndBox::GetCenter() const
{
    return ( ( m_Max + m_Min ) * 0.5 );
}

vec3d  BndBox::GetCornerPnt( int ind ) const
{
    switch ( ind )
    {
    case 0:
        return ( m_Min );
    case 1:
        return ( vec3d ( m_Max[0], m_Min[1], m_Min[2] ) );
    case 2:
        return ( vec3d ( m_Min[0], m_Max[1], m_Min[2] ) );
    case 3:
        return ( vec3d ( m_Max[0], m_Max[1], m_Min[2] ) );
    case 4:
        return ( vec3d ( m_Min[0], m_Min[1], m_Max[2] ) );
    case 5:
        return ( vec3d ( m_Max[0], m_Min[1], m_Max[2] ) );
    case 6:
        return ( vec3d ( m_Min[0], m_Max[1], m_Max[2] ) );

    default:
        return ( m_Max );
    }
    return m_Max;
}

std::vector< vec3d > BndBox::GetCornerPnts() const
{
    std::vector< vec3d > pnt_vec;
    pnt_vec.push_back( vec3d( m_Min.x(), m_Min.y(), m_Min.z() ) );
    pnt_vec.push_back( vec3d( m_Min.x(), m_Max.y(), m_Min.z() ) );
    pnt_vec.push_back( vec3d( m_Max.x(), m_Max.y(), m_Min.z() ) );
    pnt_vec.push_back( vec3d( m_Max.x(), m_Min.y(), m_Min.z() ) );
    pnt_vec.push_back( vec3d( m_Min.x(), m_Min.y(), m_Max.z() ) );
    pnt_vec.push_back( vec3d( m_Min.x(), m_Max.y(), m_Max.z() ) );
    pnt_vec.push_back( vec3d( m_Max.x(), m_Max.y(), m_Max.z() ) );
    pnt_vec.push_back( vec3d( m_Max.x(), m_Min.y(), m_Max.z() ) );
    return pnt_vec;
}

//==== Expland Bounding Box ====//
void BndBox::Expand( double val )
{
    m_Min = m_Min - vec3d( val, val, val );
    m_Max = m_Max + vec3d( val, val, val );
}

//==== Scale Bounding Box ====//
void BndBox::Scale( const vec3d & scale_xyz )
{
    vec3d center = GetCenter();
    for ( int i = 0 ; i < 3 ; i++ )
    {
        m_Min[i] = center[i] + ( m_Min[i] - center[i] ) * scale_xyz[i];
        m_Max[i] = center[i] + ( m_Max[i] - center[i] ) * scale_xyz[i];
    }
}

//==== Compare Bounding Box ====//
bool BndBox::CheckPnt( double x, double y, double z ) const
{
    if ( x < m_Min[0] )
    {
        return false;
    }
    if ( x > m_Max[0] )
    {
        return false;
    }
    if ( y < m_Min[1] )
    {
        return false;
    }
    if ( y > m_Max[1] )
    {
        return false;
    }
    if ( z < m_Min[2] )
    {
        return false;
    }
    if ( z > m_Max[2] )
    {
        return false;
    }
    return true;
}

bool BndBox::CheckPnt( const vec3d & pnt ) const
{
    return CheckPnt( pnt[0], pnt[1], pnt[2] );
}

//==== Compare Bounding Box ====//
bool Compare( const BndBox& bb1, const BndBox& bb2, double tol )
{
    if ( ( bb2.m_Min[0] - bb1.m_Max[0] ) > tol )
    {
        return false;
    }
    if ( ( bb1.m_Min[0] - bb2.m_Max[0] ) > tol )
    {
        return false;
    }
    if ( ( bb2.m_Min[1] - bb1.m_Max[1] ) > tol )
    {
        return false;
    }
    if ( ( bb1.m_Min[1] - bb2.m_Max[1] ) > tol )
    {
        return false;
    }
    if ( ( bb2.m_Min[2] - bb1.m_Max[2] ) > tol )
    {
        return false;
    }
    if ( ( bb1.m_Min[2] - bb2.m_Max[2] ) > tol )
    {
        return false;
    }
    return true;
}

//==== Assemble Boundbox Draw Lines ====//
std::vector< vec3d > BndBox::GetBBoxDrawLines() const
{
    static int index[24] = {0, 1, 0, 2, 1, 3, 2, 3, 0, 4, 1, 5,
                            2, 6, 3, 7, 4, 5, 4, 6, 5, 7, 6, 7
                           };

    std::vector< vec3d > lines;

    for ( int i = 0 ; i < 24 ; i++ )
    {
        lines.push_back( GetCornerPnt( index[i] ) );
    }
    return lines;
}

