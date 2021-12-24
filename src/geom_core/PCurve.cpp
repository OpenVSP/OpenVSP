//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PCurve.cpp: implementation of the PCurve class
//
//////////////////////////////////////////////////////////////////////


#include "PCurve.h"
#include "ParmMgr.h"
#include <cfloat>

PCurve::PCurve() : ParmContainer()
{
    m_Name = "PCurve"; // ParmContainer name
}

PCurve::~PCurve()
{
    for ( int i = 0; i < m_TParmVec.size(); i++ )
    {
        delete m_TParmVec[i];
    }
    m_TParmVec.clear();

    for ( int i = 0; i < m_ValParmVec.size(); i++ )
    {
        delete m_ValParmVec[i];
    }
    m_ValParmVec.clear();

    for ( int i = 0; i < m_EnforceG1Vec.size(); i++ )
    {
        delete m_EnforceG1Vec[i];
    }
    m_EnforceG1Vec.clear();
}

void PCurve::InitParms()
{
    m_CurveType.Init( "CrvType", m_GroupName, this, vsp::PCHIP, vsp::LINEAR, vsp::CEDIT );
    m_CurveType.SetDescript( "Curve type" );

    m_ConvType.Init( "ConvType", m_GroupName, this, vsp::CEDIT, vsp::LINEAR, vsp::APPROX_CEDIT );
    m_ConvType.SetDescript( "Curve conversion type" );

    m_SplitPt.Init( "SplitPt", m_GroupName, this, 0.5, -1.0e12, 1.0e12 );
    m_SplitPt.SetDescript( "Curve split location" );

    m_SelectPntID = 0;
    m_EnforceG1Next = true;
}

void PCurve::ReservePts( int n )
{
    if ( n < m_TParmVec.size() )
    {
        vector< Parm* > tparms( n );
        vector< Parm* > vparms( n );
        vector< BoolParm* > g1parms( n );

        int i;
        for ( i = 0; i < n; i++ )
        {
            tparms[i] = m_TParmVec[i];
            vparms[i] = m_ValParmVec[i];
            g1parms[i] = m_EnforceG1Vec[i];
        }
        for ( ; i < m_TParmVec.size(); i++ )
        {
            delete m_TParmVec[i];
            delete m_ValParmVec[i];
            delete m_EnforceG1Vec[i];
        }
        m_TParmVec = tparms;
        m_ValParmVec = vparms;
        m_EnforceG1Vec = g1parms;
    }
    else
    {
        while ( m_TParmVec.size() < n )
        {
            AddPt();
        }
    }
}

void PCurve::SetCurveName( const string & name )
{
    m_CurveName = name;
    m_GroupName = name;
}

xmlNodePtr PCurve::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr curve_node = xmlNewChild( node, NULL, BAD_CAST m_CurveName.c_str(), NULL );
    if ( curve_node )
    {
        xmlNodePtr child_node = xmlNewChild( curve_node, NULL, BAD_CAST "PCurve", NULL );
        if ( child_node )
        {
            XmlUtil::AddIntNode( child_node, "NumPts", m_TParmVec.size() );
        }
        ParmContainer::EncodeXml( curve_node );
    }

    return curve_node;
}

xmlNodePtr PCurve::DecodeXml( xmlNodePtr & node )
{
    ClearPtOrder();

    xmlNodePtr curve_node = XmlUtil::GetNode( node, m_CurveName.c_str(), 0 );
    if ( curve_node )
    {
        xmlNodePtr child_node = XmlUtil::GetNode( curve_node, "PCurve", 0 );
        if ( child_node )
        {
            int npt = XmlUtil::FindInt( child_node, "NumPts", m_TParmVec.size() );

            ReservePts( npt );
        }
        ParmContainer::DecodeXml( curve_node );
    }
    return curve_node;
}

void PCurve::AddPt()
{
    Parm* p = ParmMgr.CreateParm( vsp::PARM_DOUBLE_TYPE );
    if ( p )
    {
        int i = m_TParmVec.size();
        char str[255];
        sprintf( str, "%s_%d", m_XParmName.c_str(), i );
        p->Init( string( str ), m_GroupName, this, 0.0, -1.0e12, 1.0e12 );
        p->SetDescript( "Curve point parameter" );
        m_TParmVec.push_back( p );
    }


    p = ParmMgr.CreateParm( vsp::PARM_DOUBLE_TYPE );
    if ( p )
    {
        int i = m_ValParmVec.size();
        char str[255];
        sprintf( str, "%s_%d", m_YParmName.c_str(),  i );
        p->Init( string( str ), m_GroupName, this, 0.0, -1.0e12, 1.0e12 );
        p->SetDescript( "Curve point value" );
        m_ValParmVec.push_back( p );
    }

    BoolParm* bp = dynamic_cast<BoolParm*>( ParmMgr.CreateParm( vsp::PARM_BOOL_TYPE ) );
    if ( bp )
    {
        int i = (int)m_EnforceG1Vec.size();
        char str[15];
        sprintf( str, "G1_%d", i );
        bp->Init( string( str ), m_GroupName, this, false, false, true );
        bp->SetDescript( "G1 Enforcement Flag" );
        m_EnforceG1Vec.push_back( bp );
    }
}

void PCurve::DeletePt()
{
    DeletePt( m_SelectPntID );
}

void PCurve::DeletePt( int indx )
{
    if ( indx > 0 && indx < ( m_TParmVec.size() - 1 ) )
    {
        if ( m_CurveType() == vsp::CEDIT )
        {
            if ( ( indx % 3 ) == 0 )
            {
                delete m_TParmVec[indx-1];
                delete m_TParmVec[indx];
                delete m_TParmVec[indx+1];

                m_TParmVec.erase( m_TParmVec.begin() + indx - 1, m_TParmVec.begin() + indx + 2 );

                delete m_ValParmVec[indx-1];
                delete m_ValParmVec[indx];
                delete m_ValParmVec[indx+1];
                m_ValParmVec.erase( m_ValParmVec.begin() + indx - 1, m_ValParmVec.begin() + indx + 2 );

                delete m_EnforceG1Vec[indx - 1];
                delete m_EnforceG1Vec[indx];
                delete m_EnforceG1Vec[indx + 1];
                m_EnforceG1Vec.erase( m_EnforceG1Vec.begin() + indx - 1, m_EnforceG1Vec.begin() + indx + 2 );

                ValidateCEDIT();
            }
        }
        else
        {
            delete m_TParmVec[indx];
            m_TParmVec.erase( m_TParmVec.begin() + indx );

            delete m_ValParmVec[indx];
            m_ValParmVec.erase( m_ValParmVec.begin() + indx );

            delete m_EnforceG1Vec[indx];
            m_EnforceG1Vec.erase( m_EnforceG1Vec.begin() + indx );

            EnforcePtOrder();
        }

        RenameParms();

        m_SelectPntID = 0;

        m_LateUpdateFlag = true;
        ParmChanged( NULL, Parm::SET_FROM_DEVICE ); // Force update.
    }
}

void PCurve::ParmChanged( Parm* parm_ptr, int type )
{
    for ( size_t i = 0; i < m_EnforceG1Vec.size(); i++ )
    {
        Parm* g1_parm = dynamic_cast<Parm*> ( m_EnforceG1Vec[i] );

        if ( g1_parm == parm_ptr )
        {
            EnforceG1( i );
            break;
        }
    }

    if ( m_CurveType() == vsp::CEDIT )
    {
        for ( size_t i = 0; i < m_TParmVec.size(); i++ )
        {
            Parm* t_parm = dynamic_cast<Parm*> ( m_TParmVec[i] );
            Parm* v_parm = dynamic_cast<Parm*> ( m_ValParmVec[i] );

            if ( parm_ptr == t_parm || parm_ptr == v_parm )
            {
                if ( i % 3 == 1 )
                {
                    m_EnforceG1Next = true;
                }
                else if ( i % 3 == 2 )
                {
                    m_EnforceG1Next = false;
                }
                else if ( i % 3 == 0 && m_EnforceG1Vec[i]->Get() && parm_ptr == v_parm && type == Parm::SET_FROM_DEVICE )
                {
                    // Adjust the value of the neighboring control points if GUI slider is adjusted (not used for a click-and-drag event)
                    double dv = v_parm->Get() - v_parm->GetLastVal();

                    Parm* vprev = NULL;
                    if ( i > 0 )
                    {
                        vprev = m_ValParmVec[i - 1];
                        vprev->Set( vprev->Get() + dv );
                    }
                    Parm* vnext = NULL;
                    if ( i < m_ValParmVec.size() - 1 )
                    {
                        vnext = m_ValParmVec[i + 1];
                        vnext->Set( vnext->Get() + dv );
                    }
                }

                break;
            }
        }
    }

    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;

        //==== Notify Parent Container (XSecSurf) ====//
        ParmContainer* pc = GetParentContainerPtr();
        if ( pc )
        {
            pc->ParmChanged( parm_ptr, type );
        }

        return;
    }

    Update();

    //==== Notify Parent Container (XSecSurf) ====//
    ParmContainer* pc = GetParentContainerPtr();
    if ( pc )
    {
        pc->ParmChanged( parm_ptr, type );
    }
}


void PCurve::Update()
{
    EnforceG1();

    vector < double > tvec = GetTVec();
    vector < double > valvec = GetValVec();

    switch( this->m_CurveType() )
    {
    case vsp::LINEAR:
        m_Curve.InterpolateLinear( valvec, tvec, false );
        break;
    case vsp::PCHIP:
        if ( tvec.size() >= 3 )
        {
            m_Curve.InterpolatePCHIP( valvec, tvec, false );
        }
        else
        {
            m_Curve.InterpolateLinear( valvec, tvec, false );
        }
        break;
    case vsp::CEDIT:
        m_Curve.SetCubicControlPoints( valvec, tvec, false );
        break;
    }

    UpdateG1Parms();

    m_LateUpdateFlag = false;
}

void PCurve::ValidateCEDIT()
{
    vector < double > tvec = GetTVec();

    ValidateCEDIT( tvec );

    SetTVec( tvec );

}

void PCurve::ValidateCEDIT( vector < double > &tvec )
{
    int nseg = ( tvec.size() - 1 ) / 3;

    for ( int i = 0; i < nseg; i++ )
    {
        int istart = 3 * i;
        int iend = 3 * ( i + 1 );

        double dt = tvec[iend] - tvec[istart];

        tvec[iend - 1] = tvec[iend] - dt/3.0;
        tvec[istart + 1] = tvec[istart] + dt/3.0;
    }
}

double PCurve::GetRFirst()
{
    return m_TParmVec[0]->Get();
}

double PCurve::GetRLast()
{
    return m_TParmVec.back()->Get();
}

void PCurve::EnforcePtOrder( )
{
    EnforcePtOrder( GetRFirst(), GetRLast() );
}

void PCurve::EnforcePtOrder( double rfirst, double rlast )
{
    double offset = 1e-4;

    ClearPtOrder();

    if ( m_CurveType() == vsp::CEDIT )
    {
        int npt = m_TParmVec.size();
        int nseg = ( npt - 1 ) / 3;

        Parm * pprev = NULL;
        for ( int i = 0; i < nseg; i++ )
        {
            int istart = i * 3;
            int iend = ( i + 1 ) * 3;

            Parm *pstart = m_TParmVec[istart];
            Parm *pend = m_TParmVec[iend];

            if ( i == 0 && pstart )
            {
                pstart->Set( rfirst );
                pstart->Deactivate();
                pstart->SetLowerLimit( rfirst );
                pstart->SetUpperLimit( rfirst );
            }

            if ( i == nseg - 1 && pend )
            {
                pend->Set( rlast );
                pend->Deactivate();
                pend->SetUpperLimit( rlast );
                pend->SetLowerLimit( rlast );
            }

            if ( pstart && pend )
            {
                pstart->SetUpperLimit( pend->Get() - offset );
                pend->SetLowerLimit( pstart->Get() + offset );
            }

            // Keep intermediate points valid.
            double dt = pend->Get() - pstart->Get();
            m_TParmVec[iend - 1]->Set( pend->Get() - dt/3.0 );
            m_TParmVec[istart + 1]->Set( pstart->Get() + dt/3.0 );
        }

        // Deactivate interior control points without setting limits.
        for ( int i = 0; i < npt; i++ )
        {
            int m = i % 3;
            if ( m == 1 || m == 2 )
            {
                Parm *p = m_TParmVec[i];
                if ( p )
                {
                    p->Deactivate();
                }
            }
        }
    }
    else  // LINEAR or PCHIP
    {
        Parm *pprev = NULL;
        for ( int j = 0; j < m_TParmVec.size(); j++ )
        {
            Parm *p = m_TParmVec[j];
            Parm *pnxt = NULL;
            if ( j < m_TParmVec.size() - 1 )
            {
                pnxt = m_TParmVec[j + 1];
            }
            if ( p )
            {
                if ( j == 0 )
                {
                    p->Set( rfirst );
                    p->Deactivate();
                    p->SetLowerLimit( rfirst );
                    p->SetUpperLimit( rfirst );
                }
                else if ( j == m_TParmVec.size() - 1 )
                {
                    p->Set( rlast );
                    p->Deactivate();
                    p->SetUpperLimit( rlast );
                    p->SetLowerLimit( rlast );
                }
                else
                {
                    p->Activate();
                    if ( pprev )
                    {
                        p->SetLowerLimit( pprev->Get() + offset );
                    }
                    if ( pnxt )
                    {
                        p->SetUpperLimit( pnxt->Get() - offset );
                    }
                }
            }
            pprev = p;
        }
    }

    m_SplitPt.SetLowerLimit( rfirst );
    m_SplitPt.SetUpperLimit( rlast );
}

void PCurve::ClearPtOrder()
{
    for ( int j = 0; j < m_TParmVec.size(); j++ )
    {
        Parm *p = m_TParmVec[j];

        if ( p )
        {
            p->SetLowerLimit( -1.0e12 );
            p->SetUpperLimit( 1.0e12 );
            p->Activate();
        }
    }

    m_SplitPt.SetLowerLimit( -1.0e12 );
    m_SplitPt.SetUpperLimit( 1.0e12 );
}

double PCurve::Comp( double t )
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_Curve.CompPnt( t );
}

double PCurve::Compdt( double t )
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_Curve.CompTan( t );
}

void PCurve::Tessellate( vector < double > & t, vector < double > & v )
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    m_Curve.TessAdapt( v, t, 1e-2, 8 );
}

vector < double > PCurve::GetTVec()
{
    unsigned int n = m_TParmVec.size();

    vector < double > retvec( n );

    for ( int i = 0; i < n; ++i )
    {
        Parm* p = m_TParmVec[i];
        if ( p )
        {
            retvec[i] = p->Get();
        }
    }

    return retvec;
}

vector < double > PCurve::GetValVec()
{
    unsigned int n = m_ValParmVec.size();

    vector < double > retvec( n );

    for ( int i = 0; i < n; ++i )
    {
        Parm* p = m_ValParmVec[i];
        if ( p )
        {
            retvec[i] = p->Get();
        }
    }

    return retvec;
}

vector < bool > PCurve::GetG1Vec()
{
    vector < bool > retvec( m_EnforceG1Vec.size() );

    for ( size_t i = 0; i < m_EnforceG1Vec.size(); ++i )
    {
        BoolParm* p = m_EnforceG1Vec[i];
        if ( p )
        {
            retvec[i] = p->Get();
        }
    }

    return retvec;
}


void PCurve::SetTVec( const vector < double > & vec )
{
    ClearPtOrder();

    int n = vec.size();
    ReservePts( n );
    for ( int i = 0; i < n; ++i )
    {
        Parm* p = m_TParmVec[i];
        if ( p )
        {
            p->Set( vec[i] );
        }
    }

    EnforcePtOrder( vec.front(), vec.back() );

    m_LateUpdateFlag = true;
}

void PCurve::SetValVec( const vector < double > & vec )
{
    int n = vec.size();
    ReservePts( n );
    for ( int i = 0; i < n; ++i )
    {
        Parm* p = m_ValParmVec[i];
        if ( p )
        {
            p->Set( vec[i] );
        }
    }
    m_LateUpdateFlag = true;
}

void PCurve::SetG1Vec( const vector < bool >& vec )
{
    int n = (int)vec.size();
    ReservePts( n );
    for ( int i = 0; i < n; ++i )
    {
        BoolParm* bp = m_EnforceG1Vec[i];
        if ( bp )
        {
            bp->Set( vec[i] );
        }
    }
    m_LateUpdateFlag = true;
}

void PCurve::RenameParms()
{
    for ( int i = 0; i < m_TParmVec.size(); i++ )
    {
        char str[255];
        sprintf( str, "%s_%d", m_XParmName.c_str(), i );
        m_TParmVec[i]->SetName( string( str ) );

        sprintf( str, "%s_%d", m_YParmName.c_str(), i );
        m_ValParmVec[i]->SetName( string( str ) );

        sprintf( str, "G1_%d", i );
        m_EnforceG1Vec[i]->SetName( string( str ) );
    }
}

void PCurve::SetPt( double t, double v, int indx, bool force_update )
{
    Parm *tp = m_TParmVec[ indx ];
    Parm *vp = m_ValParmVec[ indx ];

    if ( tp && vp )
    {
        switch( this->m_CurveType() )
        {
        case vsp::LINEAR:
        case vsp::PCHIP:
            tp->Set( t );
            vp->Set( v );
            break;
        case vsp::CEDIT:
          {
            int m = indx % 3;
            if ( m == 0)  // Changing an end point
            {
                tp->Set( t );

                double dv = v - vp->Get();

                Parm *vprev = NULL;
                if ( indx > 0 )
                {
                    vprev = m_ValParmVec[ indx - 1 ];
                    vprev->Set( vprev->Get() + dv );
                }
                Parm *vnext = NULL;
                if ( indx < m_ValParmVec.size() - 1 )
                {
                    vnext = m_ValParmVec[ indx + 1 ];
                    vnext->Set( vnext->Get() + dv );
                }
            }
            // 1:  // First tangent pt
            // 2:  // Second tangent pt

            vp->Set( v );

            ValidateCEDIT();
            break;
          }
        }
        m_LateUpdateFlag = true;

        if ( force_update )
        {
            ParmChanged( NULL, Parm::SET_FROM_DEVICE );
        }
    }
}

int PCurve::Split()
{
    double tsplit = m_SplitPt();
    return Split( tsplit );
}

int PCurve::Split( const double & tsplit )
{
    // Identify the index of the new point
    m_SelectPntID = 0;

    vector < double > tvec = GetTVec();

    for( size_t i = 0; i < tvec.size(); i++ )
    {
        if( std::abs( tsplit - tvec[i] ) < FLT_EPSILON )
        {
            return m_SelectPntID; // Do not split identical U values
        }
        else if( tsplit < tvec[i] )
        {
            break;
        }
    }

    if ( m_CurveType() == vsp::CEDIT )
    {
        int nseg = ( (int)tvec.size() - 1 ) / 3;

        for ( size_t i = 0; i < nseg; i++ )
        {
            if ( tsplit > tvec[i * 3] && tsplit < tvec[3 * ( i + 1 )] )
            {
                m_SelectPntID = 3 * ( (int)i + 1 );
                break;
            }
        }
    }
    else
    {
        for ( size_t i = 0; i < tvec.size() - 1; i++ )
        {
            if ( tsplit > tvec[i] && tsplit < tvec[i + 1] )
            {
                m_SelectPntID = (int)i + 1;
                break;
            }
        }
    }

    switch( m_CurveType() )
    {
    case vsp::LINEAR:
    case vsp::PCHIP:
      {
          double vsplit = Comp( tsplit );

          tvec = GetTVec();
          vector < double > valvec = GetValVec();
          vector < bool > g1_vec = GetG1Vec();

          vector < double > newtv, newvalv;
          vector < bool > new_g1_vec;
          newtv.reserve( tvec.size() + 1 );
          newvalv.reserve( valvec.size() + 1 );
          new_g1_vec.reserve( g1_vec.size() + 1 );

          if ( tsplit < tvec[0] )
          {
              newtv.push_back( tsplit );
              newvalv.push_back( vsplit );
              new_g1_vec.push_back( false );
          }

          newtv.push_back( tvec[0] );
          newvalv.push_back( valvec[0] );
          new_g1_vec.push_back( g1_vec[0] );

          for ( int i = 1; i < tvec.size(); i++ )
          {
              if ( tsplit >= tvec[i-1] && tsplit < tvec[i] )
              {
                  newtv.push_back( tsplit );
                  newvalv.push_back( vsplit );
                  new_g1_vec.push_back( false );
              }
              newtv.push_back( tvec[i] );
              newvalv.push_back( valvec[i] );
              new_g1_vec.push_back( g1_vec[i] );
          }

          if ( tsplit >= tvec.back() )
          {
              newtv.push_back( tsplit );
              newvalv.push_back( vsplit );
              new_g1_vec.push_back( false );
          }

          InitCurve( newtv, newvalv, new_g1_vec );

      }
        break;
    case vsp::CEDIT:
      {
          vector < bool > prev_g1_vec = GetG1Vec();
          m_Curve.Split( tsplit );

          vector < double > valvec;

          m_Curve.GetCubicControlPoints( valvec, tvec );

          vector < bool > new_g1_vec( valvec.size() );

          for ( size_t i = 0; i < valvec.size(); i++ )
          {
              if ( ( i >= m_SelectPntID - 1 ) && ( i <= m_SelectPntID + 1 ) )
              {
                  new_g1_vec[i] = false;
              }
              else if ( i < m_SelectPntID - 1 )
              {
                  new_g1_vec[i] = prev_g1_vec[i];
              }
              else
              {
                  new_g1_vec[i] = prev_g1_vec[i - 3];
              }
          }

          InitCurve( tvec, valvec, new_g1_vec );

      }
        break;
    }

    RenameParms();

    m_LateUpdateFlag = true;
    ParmChanged( NULL, Parm::SET_FROM_DEVICE ); // Force update.

    return m_SelectPntID;
}

void PCurve::Approximate()
{
    double vmin, vmax;
    m_Curve.GetBoundingBox( vmin, vmax );

    double sf = vmax - vmin;
    if ( sf < 1e-6 )
    {
        sf = 1.0;
    }

    m_Curve.Scale( 1.0/sf );
    m_Curve.ToBinaryCubic( );
    m_Curve.Scale( sf );

    vector < double > tvec;
    vector < double > valvec;

    m_Curve.GetCubicControlPoints( valvec, tvec );

    vector < bool > new_g1_vec( valvec.size(), false );

    m_CurveType = vsp::CEDIT;

    InitCurve( tvec, valvec, new_g1_vec );
}

void PCurve::ConvertTo( int newtype )
{
    if (newtype == vsp::APPROX_CEDIT)
    {
        Approximate();
    }
    else
    {
        switch( m_CurveType() )
        {
        case vsp::LINEAR:
            {
                switch( newtype )
                {
                case vsp::LINEAR:
                    return;
                    break;
                case vsp::PCHIP:
                    m_CurveType = vsp::PCHIP;
                    break;
                case vsp::CEDIT:
                    vector < bool > prev_g1_vec = GetG1Vec();

                    m_Curve.ToCubic();

                    vector < double > tvec;
                    vector < double > valvec;

                    m_Curve.GetCubicControlPoints( valvec, tvec );

                    vector < bool > new_g1_vec( valvec.size() );

                    for ( size_t i = 0; i < valvec.size(); i++ )
                    {
                        if ( i % 3 == 0 )
                        {
                            new_g1_vec[i] = prev_g1_vec[i / 3];
                        }
                        else
                        {
                            new_g1_vec[i] = false;
                        }
                    }

                    m_CurveType = vsp::CEDIT;

                    InitCurve( tvec, valvec, new_g1_vec );

                    break;
                }
            }
            break;
        case vsp::PCHIP:
            {
                switch( newtype )
                {
                case vsp::LINEAR:
                    m_CurveType = vsp::LINEAR;
                    break;
                case vsp::PCHIP:
                    return;
                    break;
                case vsp::CEDIT:

                    vector < bool > prev_g1_vec = GetG1Vec();
                    vector < double > tvec;
                    vector < double > valvec;

                    m_Curve.GetCubicControlPoints( valvec, tvec );

                    vector < bool > new_g1_vec( valvec.size() );

                    for ( size_t i = 0; i < valvec.size(); i++ )
                    {
                        if ( i % 3 == 0 )
                        {
                            new_g1_vec[i] = prev_g1_vec[i / 3];
                        }
                        else
                        {
                            new_g1_vec[i] = false;
                        }
                    }

                    m_CurveType = vsp::CEDIT;
                    InitCurve( tvec, valvec, new_g1_vec );

                    break;
                }
            }
            break;
        case vsp::CEDIT:
            {
                switch( newtype )
                {
                case vsp::LINEAR:
                case vsp::PCHIP:
                    {
                        vector < double > tvec = GetTVec();
                        vector < double > valvec = GetValVec();
                        vector < bool > g1_vec = GetG1Vec();

                        vector < double > newtvec, newvalvec;
                        vector < bool > new_g1_vec;

                        int npt = tvec.size();
                        int nseg = ( npt - 1 ) / 3;
                        for ( int i = 0; i < nseg + 1; i++ )
                        {
                            int ipt = 3 * i;
                            newtvec.push_back( tvec[ipt] );
                            newvalvec.push_back( valvec[ipt] );
                            new_g1_vec.push_back( g1_vec[ipt] );
                        }

                        m_CurveType = newtype;

                        InitCurve( newtvec, newvalvec, new_g1_vec );
                    }
                    break;
                case vsp::CEDIT:
                    return;
                    break;
                }
            }
            break;
        }
    }

    RenameParms();

    // Reset selected control point (due to potentially added/removed points)
    m_SelectPntID = 0;

    m_LateUpdateFlag = true;
    ParmChanged( NULL, Parm::SET_FROM_DEVICE ); // Force update.
}

void PCurve::InitCurve( const vector < double > & tvec, const vector < double > & valvec, vector < bool > g1vec )
{
    SetTVec( tvec );
    SetValVec( valvec );

    if ( g1vec.size() != tvec.size() )
    {
        g1vec = vector < bool >( tvec.size(), false );
    }

    SetG1Vec( g1vec );
}

void PCurve::SetDispNames( const string & xname, const string & yname )
{
    m_XDispName = xname;
    m_YDispName = yname;
}

void PCurve::SetCurve( const vector < double > & tvec, const vector < double > & valvec, int newtype, vector < bool > g1vec )
{
    m_CurveType = newtype;
    InitCurve( tvec, valvec, g1vec );
    RenameParms();
}

void PCurve::SetParmNames( const string & xname, const string & yname )
{
    m_XParmName = xname;
    m_YParmName = yname;
}

double PCurve::IntegrateCrv( )
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_Curve.IntegrateCrv( );
}

double PCurve::IntegrateCrv( double r0 )
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_Curve.IntegrateCrv( r0 );
}

double PCurve::IntegrateCrv_r( )
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_Curve.IntegrateCrv_r( );
}

double PCurve::IntegrateCrv_r( double r0 )
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_Curve.IntegrateCrv_r( r0 );
}

double PCurve::IntegrateCrv_rsq( )
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_Curve.IntegrateCrv_rsq( );
}

double PCurve::IntegrateCrv_rsq( double r0 )
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_Curve.IntegrateCrv_rsq( r0 );
}

double PCurve::IntegrateCrv_rcub( )
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_Curve.IntegrateCrv_rcub( );
}

double PCurve::IntegrateCrv_rcub( double r0 )
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_Curve.IntegrateCrv_rcub( r0 );
}

void PCurve::BinCubicTMap( vector < double > &tmap, vector < double > &tdisc )
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    m_Curve.BinCubicTMap( tmap, tdisc );
}

void PCurve::GetTMap( vector < double > &tmap, vector < double > &tdisc )
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    m_Curve.GetTMap( tmap, tdisc );
}

void PCurve::SetSelectPntID( int id )
{
    if ( id < 0 || id > m_TParmVec.size() - 1 )
    {
        return;
    }

    if ( m_CurveType() == vsp::CEDIT )
    {
        if ( id % 3 == 1 )
        {
            m_EnforceG1Next = true;
        }
        else if ( id % 3 == 2 )
        {
            m_EnforceG1Next = false;
        }
    }

    m_SelectPntID = id;
}

void PCurve::UpdateG1Parms()
{
    if ( m_CurveType() == vsp::CEDIT )
    {
        for ( int i = 1; i < m_EnforceG1Vec.size() - 1; i++ )
        {
            BoolParm* p = m_EnforceG1Vec[i];

            if ( p )
            {
                if ( i % 3 != 0 ) // Deactivate G1 for interior control points
                {
                    p->Set( false );
                    p->Deactivate();
                }
                else
                {
                    p->Activate();
                }
            }
        }
    }
    else
    {
        for ( int j = 1; j < m_EnforceG1Vec.size() - 1; j++ )
        {
            m_EnforceG1Vec[j]->Set( false );
            m_EnforceG1Vec[j]->Deactivate(); // No G1 enforcement for linear or PCHIP
        }
    }

    m_EnforceG1Vec[0]->Set( false );
    m_EnforceG1Vec[m_EnforceG1Vec.size() - 1]->Set( false );
    m_EnforceG1Vec[0]->Deactivate();
    m_EnforceG1Vec[m_EnforceG1Vec.size() - 1]->Deactivate();
}

void PCurve::EnforceG1( int new_index )
{
    if ( m_CurveType() != vsp::CEDIT )
    {
        return;
    }

    for ( size_t i = 0; i < m_EnforceG1Vec.size(); i++ )
    {
        if ( m_EnforceG1Vec[i]->Get() )
        {
            int prev_ind = i - 1;
            int next_ind = i + 1;

            vec3d prev_pnt = vec3d( m_TParmVec[prev_ind]->Get(), m_ValParmVec[prev_ind]->Get(), 0.0 );
            vec3d curr_pnt = vec3d( m_TParmVec[i]->Get(), m_ValParmVec[i]->Get(), 0.0 );
            vec3d next_pnt = vec3d( m_TParmVec[next_ind]->Get(), m_ValParmVec[next_ind]->Get(), 0.0 );

            // Identify average slope
            double prev_m = ( curr_pnt.y() - prev_pnt.y() ) / ( curr_pnt.x() - prev_pnt.x() );
            double next_m = ( next_pnt.y() - curr_pnt.y() ) / ( next_pnt.x() - curr_pnt.x() );

            if ( new_index == i )
            {
                double avg_m = ( prev_m + next_m ) / 2;

                // Anforce average tangent slope on left and right side
                double prev_val = curr_pnt.y() - avg_m * ( curr_pnt.x() - prev_pnt.x() );
                m_ValParmVec[prev_ind]->Set( prev_val );

                double next_val = curr_pnt.y() + avg_m * ( next_pnt.x() - curr_pnt.x() );
                m_ValParmVec[next_ind]->Set( next_val );
            }
            else if ( std::abs( prev_m - next_m ) > FLT_EPSILON )
            {
                if ( m_EnforceG1Next )
                {
                    // Enforce next point tangent slope on previous point
                    double prev_val = curr_pnt.y() - next_m * ( curr_pnt.x() - prev_pnt.x() );
                    m_ValParmVec[prev_ind]->Set( prev_val );
                }
                else
                {
                    // Enforce previous point tangent slope on next point
                    double next_val = curr_pnt.y() + prev_m * ( next_pnt.x() - curr_pnt.x() );
                    m_ValParmVec[next_ind]->Set( next_val );
                }
            }
        }
    }
}