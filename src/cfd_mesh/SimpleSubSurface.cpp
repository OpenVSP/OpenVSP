//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SimpleSubSurface.cpp
//
// SimpleSubSurface.cpp contains the SimpleSubSurface class, a simplified version of SubSurfaces used
//  to limit reliance on SubSurface pointers and data transfer between cfd_mesh and geom_core. 
//
//////////////////////////////////////////////////////////////////////

#include "SimpleSubSurface.h"

//////////////////////////////////////////////////////
//=============== SimpleSubSurface =================//
//////////////////////////////////////////////////////

SimpleSubSurface::SimpleSubSurface()
{
    m_Tag = 0;
    m_PolyPntsReadyFlag = false;
    m_FirstSplit = true;
    m_PolyFlag = true;
    m_TestType = vsp::INSIDE;
    m_MainSurfIndx = 0;
    m_IncludedElements = 0;
    m_Type = vsp::SS_LINE;
    m_FeaPropertyIndex = 0;
    m_CapFeaPropertyIndex = 0;
    m_FeaOrientationType = vsp::FEA_ORIENT_PART_U;
}

SimpleSubSurface::~SimpleSubSurface()
{
}

void SimpleSubSurface::CopyFrom( SubSurface* ss )
{
    if ( ss )
    {
        m_SSID = ss->GetID();
        m_CompID = ss->GetCompID();
        m_Name = ss->GetName();
        m_Type = ss->GetType();
        m_Tag = ss->m_Tag;
        m_TestType = ss->m_TestType.Get();
        m_MainSurfIndx = ss->m_MainSurfIndx.Get();
        m_IncludedElements = ss->m_IncludedElements.Get();
        m_FeaPropertyIndex = ss->m_FeaPropertyIndex();
        m_CapFeaPropertyIndex = ss->m_CapFeaPropertyIndex();
        m_FeaOrientationType = ss->m_FeaOrientationType();
        m_FeaOrientationVec = ss->m_FeaOrientationVec;

        ss->PrepareSplitVec();
        m_SplitLVec = ss->GetSplitSegs();
        m_LVec = ss->GetLVec();
        m_FirstSplit = true;

        ss->UpdatePolygonPnts();
        m_PolyPntsVec = ss->GetPolyPntsVec();
        m_PolyPntsReadyFlag = true;
        m_PolyFlag = ss->GetPolyFlag();
    }
}

void SimpleSubSurface::SplitSegsU( const double & u )
{
    for ( int i = 0; i < m_SplitLVec.size(); i++ )
    {
        SplitSegsU( u, m_SplitLVec[i] );
    }
}

void SimpleSubSurface::SplitSegsW( const double & w )
{
    for ( int i = 0; i < m_SplitLVec.size(); i++ )
    {
        SplitSegsW( w, m_SplitLVec[i] );
    }
}

void SimpleSubSurface::SplitSegsU( const double & u, vector<SSLineSeg> &splitvec )
{
    double tol = 1e-10;
    int num_l_segs = splitvec.size();
    int num_splits = 0;
    bool reorder = false;
    vector<SSLineSeg> new_lsegs;
    vector<int> inds;
    for ( int i = 0; i < num_l_segs; i++ )
    {
        SSLineSeg& seg = splitvec[i];
        vec3d p0 = seg.GetP0();
        vec3d p1 = seg.GetP1();

        double t = ( u - p0.x() ) / ( p1.x() - p0.x() );

        if ( t < 1 - tol && t > 0 + tol )
        {
            if ( m_FirstSplit )
            {
                m_FirstSplit = false;
                reorder = true;
            }
            // Split the segments
            vec3d int_pnt = point_on_line( p0, p1, t );
            SSLineSeg split_seg = SSLineSeg( seg );

            seg.SetP1( int_pnt );
            split_seg.SetP0( int_pnt );
            inds.push_back( i + num_splits + 1 );
            new_lsegs.push_back( split_seg );
            num_splits++;
        }
    }

    for ( int i = 0; i < (int)inds.size(); i++ )
    {
        splitvec.insert( splitvec.begin() + inds[i], new_lsegs[i] );
    }

    if ( reorder )
    {
        ReorderSplitSegs( inds[0], splitvec );
    }
}

void SimpleSubSurface::SplitSegsW( const double & w, vector<SSLineSeg> &splitvec )
{
    double tol = 1e-10;
    int num_l_segs = splitvec.size();
    int num_splits = 0;
    bool reorder = false;
    vector<SSLineSeg> new_lsegs;
    vector<int> inds;
    for ( int i = 0; i < num_l_segs; i++ )
    {
        SSLineSeg& seg = splitvec[i];
        vec3d p0 = seg.GetP0();
        vec3d p1 = seg.GetP1();

        double t = ( w - p0.y() ) / ( p1.y() - p0.y() );

        if ( t < 1 - tol && t > 0 + tol )
        {
            if ( m_FirstSplit )
            {
                m_FirstSplit = false;
                reorder = true;
            }
            // Split the segments
            vec3d int_pnt = point_on_line( p0, p1, t );
            SSLineSeg split_seg = SSLineSeg( seg );

            seg.SetP1( int_pnt );
            split_seg.SetP0( int_pnt );
            inds.push_back( i + num_splits + 1 );
            new_lsegs.push_back( split_seg );
            num_splits++;
        }
    }

    for ( int i = 0; i < (int)inds.size(); i++ )
    {
        splitvec.insert( splitvec.begin() + inds[i], new_lsegs[i] );
    }

    if ( reorder )
    {
        ReorderSplitSegs( inds[0], splitvec );
    }
}

void SimpleSubSurface::ReorderSplitSegs( int ind, vector<SSLineSeg> &splitvec )
{
    if ( ind < 0 || ind >( int )splitvec.size() - 1 )
    {
        return;
    }

    vector<SSLineSeg> ret_vec;
    ret_vec.resize( splitvec.size() );

    int cnt = 0;
    for ( int i = ind; i < (int)splitvec.size(); i++ )
    {
        ret_vec[cnt] = splitvec[i];
        cnt++;
    }
    for ( int i = 0; i < ind; i++ )
    {
        ret_vec[cnt] = splitvec[i];
        cnt++;
    }

    splitvec = ret_vec;
}

bool SimpleSubSurface::Subtag( const vec3d & center )
{
    if ( m_Type == vsp::SS_LINE )
    {
        return m_LVec[0].Subtag( center );
    }
    else
    {
        if ( m_TestType == vsp::NONE )
        {
            return false;
        }

        for ( int p = 0; p < (int)m_PolyPntsVec.size(); p++ )
        {
            bool inPoly = PointInPolygon( vec2d( center.x(), center.y() ), m_PolyPntsVec[p] );

            if ( inPoly && m_TestType == vsp::INSIDE )
            {
                return true;
            }
            else if ( inPoly && m_TestType == vsp::OUTSIDE )
            {
                return false;
            }
        }

        if ( m_TestType == vsp::OUTSIDE )
        {
            return true;
        }

        return false;
    }
}