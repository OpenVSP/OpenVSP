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
    m_ControlSurfType = -1;
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
        m_FeaPropertyIndex = ss->GetFeaPropertyIndex();
        m_CapFeaPropertyIndex = ss->GetCapFeaPropertyIndex();

        ss->PrepareSplitVec();

        m_SplitLVec = ss->GetSplitSegs();
        m_LVec = m_SplitLVec;

        m_PolyPntsReadyFlag = false;
        m_FirstSplit = true;
        m_PolyFlag = ss->GetPolyFlag();

        if ( m_Type == vsp::SS_CONTROL )
        {
            SSControlSurf* ss_con = dynamic_cast<SSControlSurf*>( ss );
            assert( ss_con );

            m_ControlSurfType = ss_con->m_SurfType.Get();
        }
    }
}

void SimpleSubSurface::SplitSegsU( const double & u )
{
    double tol = 1e-10;
    int num_l_segs = m_SplitLVec.size();
    int num_splits = 0;
    bool reorder = false;
    vector<SSLineSeg> new_lsegs;
    vector<int> inds;
    for ( int i = 0; i < num_l_segs; i++ )
    {
        SSLineSeg& seg = m_SplitLVec[i];
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
        m_SplitLVec.insert( m_SplitLVec.begin() + inds[i], new_lsegs[i] );
    }

    if ( reorder )
    {
        ReorderSplitSegs( inds[0] );
    }
}

void SimpleSubSurface::SplitSegsW( const double & w )
{
    double tol = 1e-10;
    int num_l_segs = m_SplitLVec.size();
    int num_splits = 0;
    bool reorder = false;
    vector<SSLineSeg> new_lsegs;
    vector<int> inds;
    for ( int i = 0; i < num_l_segs; i++ )
    {

        SSLineSeg& seg = m_SplitLVec[i];
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
        m_SplitLVec.insert( m_SplitLVec.begin() + inds[i], new_lsegs[i] );
    }

    if ( reorder )
    {
        ReorderSplitSegs( inds[0] );
    }
}

void SimpleSubSurface::ReorderSplitSegs( int ind )
{
    if ( ind < 0 || ind >( int )m_SplitLVec.size() - 1 )
    {
        return;
    }

    vector<SSLineSeg> ret_vec;
    ret_vec.resize( m_SplitLVec.size() );

    int cnt = 0;
    for ( int i = ind; i < (int)m_SplitLVec.size(); i++ )
    {
        ret_vec[cnt] = m_SplitLVec[i];
        cnt++;
    }
    for ( int i = 0; i < ind; i++ )
    {
        ret_vec[cnt] = m_SplitLVec[i];
        cnt++;
    }

    m_SplitLVec = ret_vec;
}

bool SimpleSubSurface::Subtag( const vec3d & center )
{
    if ( m_Type == vsp::SS_LINE )
    {
        return m_LVec[0].Subtag( center );
    }
    else
    {
        UpdatePolygonPnts(); // Update polygon vector

        if ( m_TestType == vsp::NONE )
        {
            return false;
        }

        for ( int p = 0; p < (int)m_PolyPntsVec.size(); p++ )
        {
            bool inPoly = PointInPolygon( vec2d( center.x(), center.y() ), m_PolyPntsVec[p] );

            if ( inPoly )
            {
                bool test = true;
            }

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

void SimpleSubSurface::UpdatePolygonPnts( bool ss_con_both )
{
    if ( m_PolyPntsReadyFlag )
    {
        return;
    }

    if ( m_Type == vsp::SS_CONTROL && ss_con_both )
    {
        if ( m_ControlSurfType == SSControlSurf::SS_CONTROL_SUBTYPE::UPPER_SURF || m_ControlSurfType == SSControlSurf::SS_CONTROL_SUBTYPE::LOWER_SURF )
        {
            UpdatePolygonPnts( false );

            vec3d pnt = m_LVec[0].GetP0();
            m_PolyPntsVec[0].push_back( vec2d( pnt.x(), pnt.y() ) );
            return;
        }

        m_PolyPntsVec.resize( 2 );

        int last_ind = 0;
        int start_ind = 0;
        for ( int i = 0; i < (int)m_PolyPntsVec.size(); i++ )
        {
            m_PolyPntsVec[i].clear();

            if ( i == 0 )
            {
                last_ind = 3;
            }
            if ( i == 1 )
            {
                last_ind = 6;
            }

            vec3d pnt;
            for ( int ls = start_ind; ls < last_ind; ls++ )
            {
                pnt = m_LVec[ls].GetP0();
                m_PolyPntsVec[i].push_back( vec2d( pnt.x(), pnt.y() ) );
            }
            pnt = m_LVec[last_ind - 1].GetP1();
            m_PolyPntsVec[i].push_back( vec2d( pnt.x(), pnt.y() ) );
            pnt = m_LVec[start_ind].GetP0();
            m_PolyPntsVec[i].push_back( vec2d( pnt.x(), pnt.y() ) );

            start_ind = last_ind;
        }

        m_PolyPntsReadyFlag = true;
    }
    else
    {
        m_PolyPntsVec.resize( 1 );

        m_PolyPntsVec[0].clear();

        int last_ind = m_LVec.size() - 1;
        vec3d pnt;
        for ( int ls = 0; ls < last_ind + 1; ls++ )
        {
            pnt = m_LVec[ls].GetP0();
            m_PolyPntsVec[0].push_back( vec2d( pnt.x(), pnt.y() ) );
        }
        pnt = m_LVec[last_ind].GetP1();
        m_PolyPntsVec[0].push_back( vec2d( pnt.x(), pnt.y() ) );

        m_PolyPntsReadyFlag = true;
    }
}
