//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PCurve.cpp: implementation of the PCurve class
//
//////////////////////////////////////////////////////////////////////


#include "PCurve.h"
#include "ParmMgr.h"


PCurve::PCurve() : ParmContainer()
{
    m_Name = "PCurve"; // ParmContainer name
}

void PCurve::InitParms()
{
    m_CurveType.Init( "CrvType", m_GroupName, this, PCHIP, LINEAR, CEDIT );
    m_CurveType.SetDescript( "Curve type" );

    m_ConvType.Init( "ConvType", m_GroupName, this, CEDIT, LINEAR, CEDIT );
    m_ConvType.SetDescript( "Curve conversion type" );

    m_SplitPt.Init( "SplitPt", m_GroupName, this, 0.5, -1.0e12, 1.0e12 );
    m_SplitPt.SetDescript( "Curve split location" );
}

void PCurve::ReservePts( int n )
{
    if ( n < m_TParmVec.size() )
    {
        vector< Parm* > tparms( n );
        vector< Parm* > vparms( n );

        int i;
        for ( i = 0; i < n; i++ )
        {
            tparms[i] = m_TParmVec[i];
            vparms[i] = m_ValParmVec[i];
        }
        for ( ; i < m_TParmVec.size(); i++ )
        {
            delete m_TParmVec[i];
            delete m_ValParmVec[i];
        }
        m_TParmVec = tparms;
        m_ValParmVec = vparms;
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
    Parm* p = ParmMgr.CreateParm( PARM_DOUBLE_TYPE );
    if ( p )
    {
        int i = m_TParmVec.size();
        char str[255];
        sprintf( str, "%s_%d", m_XParmName.c_str(), i );
        p->Init( string( str ), m_GroupName, this, 0.0, -1.0e12, 1.0e12 );
        p->SetDescript( "Curve point parameter" );
        m_TParmVec.push_back( p );
    }


    p = ParmMgr.CreateParm( PARM_DOUBLE_TYPE );
    if ( p )
    {
        int i = m_ValParmVec.size();
        char str[255];
        sprintf( str, "%s_%d", m_YParmName.c_str(),  i );
        p->Init( string( str ), m_GroupName, this, 0.0, -1.0e12, 1.0e12 );
        p->SetDescript( "Curve point value" );
        m_ValParmVec.push_back( p );
    }
}

void PCurve::DeletePt( int indx )
{
    if ( indx > 0 && indx < ( m_TParmVec.size() - 1 ) )
    {
        if ( m_CurveType() == CEDIT )
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

                ValidateCEDIT();
            }
        }
        else
        {
            delete m_TParmVec[indx];
            m_TParmVec.erase( m_TParmVec.begin() + indx );

            delete m_ValParmVec[indx];
            m_ValParmVec.erase( m_ValParmVec.begin() + indx );

            EnforcePtOrder();
        }

        m_LateUpdateFlag = true;
        ParmChanged( NULL, Parm::SET_FROM_DEVICE ); // Force update.
    }
}

void PCurve::ParmChanged( Parm* parm_ptr, int type )
{
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
    vector < double > tvec = GetTVec();
    vector < double > valvec = GetValVec();

    switch( this->m_CurveType() )
    {
    case LINEAR:
        m_Curve.InterpolateLinear( valvec, tvec, false );
        break;
    case PCHIP:
        if ( tvec.size() >= 3 )
        {
            m_Curve.InterpolatePCHIP( valvec, tvec, false );
        }
        else
        {
            m_Curve.InterpolateLinear( valvec, tvec, false );
        }
        break;
    case CEDIT:
        m_Curve.SetCubicControlPoints( valvec, tvec, false );
        break;
    }

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

void PCurve::EnforcePtOrder( )
{
    double rfirst = m_TParmVec[0]->Get();
    double rlast = m_TParmVec.back()->Get();

    EnforcePtOrder( rfirst, rlast );
}

void PCurve::EnforcePtOrder( double rfirst, double rlast )
{
    double offset = 1e-4;

    if ( m_CurveType() == CEDIT )
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
                pstart->SetLowerLimit( 0.0 );
            }

            if ( i == nseg - 1 && pend )
            {
                pend->Set( rlast );
                pend->Deactivate();
                pend->SetUpperLimit( 1.0 );
            }

            if ( pstart && pend )
            {
                pstart->SetUpperLimit( pend->Get() - offset );
                pend->SetLowerLimit( pstart->Get() + offset );
            }
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
            Parm *pnxt = m_TParmVec[j + 1];
            if ( p )
            {
                if ( j == 0 )
                {
                    p->Set( rfirst );
                    p->Deactivate();
                    p->SetLowerLimit( 0.0 );
                    if ( pnxt )
                    {
                        p->SetUpperLimit( pnxt->Get() - offset );
                    }
                }
                else if ( j == m_TParmVec.size() - 1 )
                {
                    p->Set( rlast );
                    p->Deactivate();
                    p->SetUpperLimit( 1.0 );
                    if ( pprev )
                    {
                        p->SetLowerLimit( pprev->Get() + offset );
                    }
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
    int n = m_TParmVec.size();

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
    int n = m_ValParmVec.size();

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

void PCurve::RenameParms()
{
    for ( int i = 0; i < m_TParmVec.size(); i++ )
    {
        char str[255];
        sprintf( str, "%s_%d", m_XParmName.c_str(), i );
        m_TParmVec[i]->SetName( string( str ) );

        sprintf( str, "%s_%d", m_YParmName.c_str(), i );
        m_ValParmVec[i]->SetName( string( str ) );
    }
}

void PCurve::SetPt( double t, double v, int indx )
{
    Parm *tp = m_TParmVec[ indx ];
    Parm *vp = m_ValParmVec[ indx ];

    if ( tp && vp )
    {
        switch( this->m_CurveType() )
        {
        case LINEAR:
        case PCHIP:
            tp->Set( t );
            vp->Set( v );
            break;
        case CEDIT:
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
        ParmChanged( NULL, Parm::SET_FROM_DEVICE ); // Force update.

    }
}

void PCurve::Split()
{

    double tsplit = m_SplitPt();

    switch( m_CurveType() )
    {
    case LINEAR:
    case PCHIP:
      {
          double vsplit = Comp( tsplit );

          vector < double > tvec = GetTVec();
          vector < double > valvec = GetValVec();

          vector < double > newtv, newvalv;
          newtv.reserve( tvec.size() + 1 );
          newvalv.reserve( valvec.size() + 1 );

          if ( tsplit < tvec[0] )
          {
              newtv.push_back( tsplit );
              newvalv.push_back( vsplit );
          }

          newtv.push_back( tvec[0] );
          newvalv.push_back( valvec[0] );

          for ( int i = 1; i < tvec.size(); i++ )
          {
              if ( tsplit >= tvec[i-1] && tsplit < tvec[i] )
              {
                  newtv.push_back( tsplit );
                  newvalv.push_back( vsplit );
              }
              newtv.push_back( tvec[i] );
              newvalv.push_back( valvec[i] );
          }

          if ( tsplit >= tvec.back() )
          {
              newtv.push_back( tsplit );
              newvalv.push_back( vsplit );
          }

          InitCurve( newtv, newvalv );

      }
        break;
    case CEDIT:
      {
          m_Curve.Split( tsplit );

          vector < double > tvec;
          vector < double > valvec;

          m_Curve.GetCubicControlPoints( valvec, tvec );

          InitCurve( tvec, valvec );

      }
        break;
    }

    m_LateUpdateFlag = true;
    ParmChanged( NULL, Parm::SET_FROM_DEVICE ); // Force update.
}

void PCurve::ConvertTo( int newtype )
{
    switch( m_CurveType() )
    {
    case LINEAR:
        {
            switch( newtype )
            {
            case LINEAR:
                return;
                break;
            case PCHIP:
                m_CurveType = PCHIP;
                break;
            case CEDIT:
                m_Curve.ToCubic();

                vector < double > tvec;
                vector < double > valvec;

                m_Curve.GetCubicControlPoints( valvec, tvec );

                m_CurveType = CEDIT;

                InitCurve( tvec, valvec );

                break;
            }
        }
        break;
    case PCHIP:
        {
            switch( newtype )
            {
            case LINEAR:
                m_CurveType = LINEAR;
                break;
            case PCHIP:
                return;
                break;
            case CEDIT:

                vector < double > tvec;
                vector < double > valvec;

                m_Curve.GetCubicControlPoints( valvec, tvec );

                m_CurveType = CEDIT;
                InitCurve( tvec, valvec );

                break;
            }
        }
        break;
    case CEDIT:
        {
            switch( newtype )
            {
            case LINEAR:
            case PCHIP:
                {
                    vector < double > tvec = GetTVec();
                    vector < double > valvec = GetValVec();

                    vector < double > newtvec;
                    vector < double > newvalvec;

                    int npt = tvec.size();
                    int nseg = ( npt - 1 ) / 3;
                    for ( int i = 0; i < nseg + 1; i++ )
                    {
                        int ipt = 3 * i;
                        newtvec.push_back( tvec[ipt] );
                        newvalvec.push_back( valvec[ipt] );
                    }

                    m_CurveType = newtype;

                    InitCurve( newtvec, newvalvec );
                }
                break;
            case CEDIT:
                return;
                break;
            }
        }
        break;
    }

    m_LateUpdateFlag = true;
    ParmChanged( NULL, Parm::SET_FROM_DEVICE ); // Force update.
}

void PCurve::InitCurve( const vector < double > & tvec, const vector < double > & valvec )
{
    SetTVec( tvec );
    SetValVec( valvec );
}

void PCurve::SetDispNames( const string & xname, const string & yname )
{
    m_XDispName = xname;
    m_YDispName = yname;
}

void PCurve::SetParmNames( const string & xname, const string & yname )
{
    m_XParmName = xname;
    m_YParmName = yname;
}

double PCurve::IntegrateAF( double r0 )
{
    if ( m_LateUpdateFlag )
    {
        Update();
    }

    return m_Curve.IntegrateAF( r0 );
}

void PCurve::BinCubicTMap( vector < double > &tmap, vector < double > &tdisc )
{
    m_Curve.BinCubicTMap( tmap, tdisc );
}
