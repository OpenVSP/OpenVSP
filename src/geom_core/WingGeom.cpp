//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "WingGeom.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "StlHelper.h"
#include "Cluster.h"

using namespace vsp;


//==== Constructor ====//
WingDriverGroup::WingDriverGroup() : DriverGroup( NUM_WSECT_DRIVER, 3 )
{
    m_CurrChoices[0] = SPAN_WSECT_DRIVER;
    m_CurrChoices[1] = ROOTC_WSECT_DRIVER;
    m_CurrChoices[2] = TIPC_WSECT_DRIVER;
}

void WingDriverGroup::UpdateGroup( vector< string > parmIDs )
{
    Parm* AR = ParmMgr.FindParm( parmIDs[AR_WSECT_DRIVER] );
    Parm* span = ParmMgr.FindParm( parmIDs[SPAN_WSECT_DRIVER] );
    Parm* area = ParmMgr.FindParm( parmIDs[AREA_WSECT_DRIVER] );
    Parm* taper = ParmMgr.FindParm( parmIDs[TAPER_WSECT_DRIVER] );
    Parm* aveC = ParmMgr.FindParm( parmIDs[AVEC_WSECT_DRIVER] );
    Parm* rootC = ParmMgr.FindParm( parmIDs[ROOTC_WSECT_DRIVER] );
    Parm* tipC = ParmMgr.FindParm( parmIDs[TIPC_WSECT_DRIVER] );
    Parm* secsw = ParmMgr.FindParm( parmIDs[SECSWEEP_WSECT_DRIVER] );

    Parm* sweep = ParmMgr.FindParm( parmIDs[SWEEP_WSECT_DRIVER] );
    Parm* sweeploc = ParmMgr.FindParm( parmIDs[SWEEPLOC_WSECT_DRIVER] );
    Parm* secswloc = ParmMgr.FindParm( parmIDs[SECSWEEPLOC_WSECT_DRIVER] );

    vector< bool > uptodate;
    uptodate.resize( m_Nvar );

    for( int i = 0; i < m_Nvar; i++ )
    {
        uptodate[i] = false;
    }
    for( int i = 0; i < m_Nchoice; i++ )
    {
        uptodate[m_CurrChoices[i]] = true;
    }

    taper->SetLowerLimit( 0.0 );
    if( vector_contains_val( m_CurrChoices, ( int ) TAPER_WSECT_DRIVER ) &&
        vector_contains_val( m_CurrChoices, ( int ) TIPC_WSECT_DRIVER ) )
    {
        taper->SetLowerLimit( 1.0e-4 );
    }

    bool parallel = false;
    if( vector_contains_val( m_CurrChoices, ( int ) SECSWEEP_WSECT_DRIVER ) )
    {
        if ( sweep->Get() == secsw->Get() )
        {
            parallel = true;
        }
    }

    if( vector_contains_val( m_CurrChoices, ( int ) TAPER_WSECT_DRIVER ) ){
        if ( taper->Get() == 1.0 )
        {
            parallel = true;
        }
    }

    if( vector_contains_val( m_CurrChoices, ( int ) TIPC_WSECT_DRIVER ) &&
            vector_contains_val( m_CurrChoices, ( int ) ROOTC_WSECT_DRIVER ) )
    {
        if ( tipC->Get() == rootC->Get() )
        {
            parallel = true;
        }
    }

    if( vector_contains_val( m_CurrChoices, ( int ) AVEC_WSECT_DRIVER ) &&
            vector_contains_val( m_CurrChoices, ( int ) ROOTC_WSECT_DRIVER ) )
    {
        if ( aveC->Get() == rootC->Get() )
        {
            parallel = true;
        }
    }

    if( vector_contains_val( m_CurrChoices, ( int ) TIPC_WSECT_DRIVER ) &&
            vector_contains_val( m_CurrChoices, ( int ) AVEC_WSECT_DRIVER ) )
    {
        if ( tipC->Get() == aveC->Get() )
        {
            parallel = true;
        }
    }

    if( parallel )
    {
        taper->Set( 1.0 );
        uptodate[TAPER_WSECT_DRIVER] = true;
        secsw->Set( sweep->Get() );
        uptodate[SECSWEEP_WSECT_DRIVER] = true;

        if( vector_contains_val( m_CurrChoices, ( int ) ROOTC_WSECT_DRIVER ) )
        {
            tipC->Set( rootC->Get() );
            uptodate[TIPC_WSECT_DRIVER] = true;
            aveC->Set( rootC->Get() );
            uptodate[AVEC_WSECT_DRIVER] = true;
        }
        else if( vector_contains_val( m_CurrChoices, ( int ) TIPC_WSECT_DRIVER ) )
        {
            rootC->Set( tipC->Get() );
            uptodate[ROOTC_WSECT_DRIVER] = true;
            aveC->Set( tipC->Get() );
            uptodate[AVEC_WSECT_DRIVER] = true;
        }
        else if( vector_contains_val( m_CurrChoices, ( int ) AVEC_WSECT_DRIVER ) )
        {
            tipC->Set( aveC->Get() );
            uptodate[TIPC_WSECT_DRIVER] = true;
            rootC->Set( aveC->Get() );
            uptodate[ROOTC_WSECT_DRIVER] = true;
        }
    }


    int niter = 0;
    while( vector_contains_val( uptodate, false ) )
    {
        if( !uptodate[AR_WSECT_DRIVER] )
        {
            if( uptodate[AREA_WSECT_DRIVER] && area->Get() == 0.0 )
            {
                AR->Set( 1.0 );
                uptodate[AR_WSECT_DRIVER] = true;
            }
            else if( uptodate[AVEC_WSECT_DRIVER] && aveC->Get() == 0.0 )
            {
                AR->Set( 1.0 );
                uptodate[AR_WSECT_DRIVER] = true;
            }
            else if( uptodate[SPAN_WSECT_DRIVER] && uptodate[AREA_WSECT_DRIVER] && area->Get() != 0.0 )
            {
                AR->Set( span->Get() * span->Get() / area->Get() );
                uptodate[AR_WSECT_DRIVER] = true;
            }
            else if( uptodate[SPAN_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] && aveC->Get() != 0.0 )
            {
                AR->Set( span->Get() / aveC->Get() );
                uptodate[AR_WSECT_DRIVER] = true;
            }
            else if( uptodate[AREA_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] && aveC->Get() != 0.0 )
            {
                AR->Set( area->Get() / ( aveC->Get() * aveC->Get() ) );
                uptodate[AR_WSECT_DRIVER] = true;
            }
            else if( uptodate[SECSWEEP_WSECT_DRIVER] && uptodate[TAPER_WSECT_DRIVER] && !parallel)
            {
                double tan1 = tan( sweep->Get() * DEG_2_RAD );
                double tan2 = tan( secsw->Get() * DEG_2_RAD );
                AR->Set( -2.0 * ( ( 1.0 - taper->Get() ) / ( 1.0 + taper->Get() )) * ( sweeploc->Get() - secswloc->Get() ) / ( tan1 - tan2 ) );
                uptodate[AR_WSECT_DRIVER] = true;
            }
        }

        if( !uptodate[SPAN_WSECT_DRIVER] )
        {
            if( uptodate[AREA_WSECT_DRIVER] && area->Get() == 0.0 )
            {
                span->Set( 0.0 );
                uptodate[SPAN_WSECT_DRIVER] = true;
            }
            else if( uptodate[AVEC_WSECT_DRIVER] && uptodate[AREA_WSECT_DRIVER] && aveC->Get() != 0.0 )
            {
                span->Set( area->Get() / aveC->Get() );
                uptodate[SPAN_WSECT_DRIVER] = true;
            }
            else if( uptodate[AR_WSECT_DRIVER] && uptodate[AREA_WSECT_DRIVER] )
            {
                span->Set( sqrt( AR->Get() * area->Get() ) );
                uptodate[SPAN_WSECT_DRIVER] = true;
            }
            else if( uptodate[AR_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                span->Set( AR->Get() * aveC->Get() );
                uptodate[SPAN_WSECT_DRIVER] = true;
            }
            else if( uptodate[SECSWEEP_WSECT_DRIVER] && uptodate[ROOTC_WSECT_DRIVER] && uptodate[TIPC_WSECT_DRIVER] && !parallel)
            {
                double tan1 = tan( sweep->Get() * DEG_2_RAD );
                double tan2 = tan( secsw->Get() * DEG_2_RAD );
                span->Set( ( rootC->Get() - tipC->Get() ) * ( sweeploc->Get() - secswloc->Get() ) / ( tan2 - tan1 ) );
                uptodate[SPAN_WSECT_DRIVER] = true;
            }
        }


        if( !uptodate[AREA_WSECT_DRIVER] )
        {
            if( uptodate[SPAN_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                area->Set( span->Get() * aveC->Get() );
                uptodate[AREA_WSECT_DRIVER] = true;
            }
            else if( uptodate[SPAN_WSECT_DRIVER] && uptodate[AR_WSECT_DRIVER] && AR->Get() != 0.0 )
            {
                area->Set( span->Get() * span->Get() / AR->Get() );
                uptodate[AREA_WSECT_DRIVER] = true;
            }
            else if( uptodate[AR_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                area->Set( AR->Get() * aveC->Get() * aveC->Get() );
                uptodate[AREA_WSECT_DRIVER] = true;
            }
        }

        if( !uptodate[AVEC_WSECT_DRIVER] )
        {
            if( uptodate[TIPC_WSECT_DRIVER] && uptodate[ROOTC_WSECT_DRIVER] )
            {
                aveC->Set( ( tipC->Get() + rootC->Get() ) / 2.0 );
                uptodate[AVEC_WSECT_DRIVER] = true;
            }
            else if( uptodate[SPAN_WSECT_DRIVER] && uptodate[AREA_WSECT_DRIVER] && span->Get() != 0.0 )
            {
                aveC->Set( area->Get() / span->Get() );
                uptodate[AVEC_WSECT_DRIVER] = true;
            }
            else if( uptodate[SECSWEEP_WSECT_DRIVER] && uptodate[TAPER_WSECT_DRIVER] && uptodate[SPAN_WSECT_DRIVER] && !parallel)
            {
                double tan1 = tan( sweep->Get() * DEG_2_RAD );
                double tan2 = tan( secsw->Get() * DEG_2_RAD );
                aveC->Set( -span->Get() * ( tan1 - tan2 ) * ( 1.0 + taper->Get() ) / ( 2.0 * ( sweeploc->Get() - secswloc->Get() ) * ( 1 - taper->Get() ) ) );
                uptodate[AVEC_WSECT_DRIVER] = true;
            }
        }

        if( !uptodate[TIPC_WSECT_DRIVER] )
        {
            if( uptodate[TAPER_WSECT_DRIVER] && taper->Get() == 0.0 )
            {
                tipC->Set( 0.0 );
                uptodate[TIPC_WSECT_DRIVER] = true;
            }
            else if( uptodate[TAPER_WSECT_DRIVER] && uptodate[ROOTC_WSECT_DRIVER] )
            {
                tipC->Set( taper->Get() * rootC->Get() );
                uptodate[TIPC_WSECT_DRIVER] = true;
            }
            else if( uptodate[TAPER_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                tipC->Set( 2.0 * aveC->Get() / ( 1.0 + ( 1.0 / taper->Get() ) )  );
                uptodate[TIPC_WSECT_DRIVER] = true;
            }
            else if( uptodate[ROOTC_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                tipC->Set( 2.0 * aveC->Get() - rootC->Get() );
                uptodate[TIPC_WSECT_DRIVER] = true;
            }
            else if( uptodate[SECSWEEP_WSECT_DRIVER] && uptodate[ROOTC_WSECT_DRIVER] && uptodate[SPAN_WSECT_DRIVER] )
            {
                double tan1 = tan( sweep->Get() * DEG_2_RAD );
                double tan2 = tan( secsw->Get() * DEG_2_RAD );
                tipC->Set( rootC->Get() + span->Get() * ( tan1 - tan2 ) / ( sweeploc->Get() - secswloc->Get() ) );
                uptodate[TIPC_WSECT_DRIVER] = true;
            }
            else if( uptodate[SECSWEEP_WSECT_DRIVER] && uptodate[ROOTC_WSECT_DRIVER] && uptodate[AREA_WSECT_DRIVER] )
            {
                double tan1 = tan( sweep->Get() * DEG_2_RAD );
                double tan2 = tan( secsw->Get() * DEG_2_RAD );
                double coeff = 2.0 * area->Get() * ( tan1 - tan2 ) / ( sweeploc->Get() - secswloc->Get() );
                tipC->Set( sqrt( -1.0 * ( rootC->Get() * rootC->Get() + coeff ) ) );
                uptodate[TIPC_WSECT_DRIVER] = true;
            }
        }

        if( !uptodate[ROOTC_WSECT_DRIVER] )
        {
            if( uptodate[TAPER_WSECT_DRIVER] && uptodate[TIPC_WSECT_DRIVER] && taper->Get() != 0.0)
            {
                rootC->Set( tipC->Get() / taper->Get() );
                uptodate[ROOTC_WSECT_DRIVER] = true;
            }
            else if( uptodate[TAPER_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                rootC->Set( 2.0 * aveC->Get() / ( 1.0 + taper->Get() )  );
                uptodate[ROOTC_WSECT_DRIVER] = true;
            }
            else if( uptodate[TIPC_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                rootC->Set( 2.0 * aveC->Get() - tipC->Get() );
                uptodate[ROOTC_WSECT_DRIVER] = true;
            }
            else if( uptodate[SECSWEEP_WSECT_DRIVER] && uptodate[TIPC_WSECT_DRIVER] && uptodate[SPAN_WSECT_DRIVER] )
            {
                double tan1 = tan( sweep->Get() * DEG_2_RAD );
                double tan2 = tan( secsw->Get() * DEG_2_RAD );
                rootC->Set( tipC->Get() - span->Get() * ( tan1 - tan2 ) / ( sweeploc->Get() - secswloc->Get() ) );
                uptodate[ROOTC_WSECT_DRIVER] = true;
            }
            else if( uptodate[SECSWEEP_WSECT_DRIVER] && uptodate[TIPC_WSECT_DRIVER] && uptodate[AREA_WSECT_DRIVER] )
            {
                double tan1 = tan( sweep->Get() * DEG_2_RAD );
                double tan2 = tan( secsw->Get() * DEG_2_RAD );
                double coeff = 2.0 * area->Get() * ( tan1 - tan2 ) / ( sweeploc->Get() - secswloc->Get() );
                rootC->Set( sqrt( tipC->Get() * tipC->Get() - coeff ) );
                uptodate[ROOTC_WSECT_DRIVER] = true;
            }
        }

        if( !uptodate[TAPER_WSECT_DRIVER] )
        {
            if( uptodate[TIPC_WSECT_DRIVER] && uptodate[ROOTC_WSECT_DRIVER] )
            {
                if ( rootC->Get() == 0.0 )
                {
                    if ( tipC->Get() == 0.0 )
                    {
                        taper->Set( 1.0 );
                    }
                    else
                    {
                        taper->Set( taper->GetUpperLimit() );
                    }
                }
                else
                {
                    taper->Set( tipC->Get() / rootC->Get() );
                }
                uptodate[TAPER_WSECT_DRIVER] = true;
            }
            else if( uptodate[SECSWEEP_WSECT_DRIVER] && uptodate[AR_WSECT_DRIVER] )
            {
                double tan1 = tan( sweep->Get() * DEG_2_RAD );
                double tan2 = tan( secsw->Get() * DEG_2_RAD );
                double coeff = AR->Get() * ( tan1 - tan2 ) / ( 2.0 * ( sweeploc->Get() - secswloc->Get() ) );
                taper->Set( ( 1.0 + coeff ) / ( 1.0 - coeff ) );
                uptodate[TAPER_WSECT_DRIVER] = true;
            }
        }

        if( !uptodate[SECSWEEP_WSECT_DRIVER] )
        {
            if( uptodate[TAPER_WSECT_DRIVER] && uptodate[AR_WSECT_DRIVER] )
            {
                double tan2 = CalcTanSweepAt( secswloc->Get(), sweep->Get(), sweeploc->Get(), AR->Get(), taper->Get() );
                secsw->Set( atan( tan2 ) * RAD_2_DEG );
                uptodate[SECSWEEP_WSECT_DRIVER] = true;
            }
            else if( uptodate[ROOTC_WSECT_DRIVER] && uptodate[TIPC_WSECT_DRIVER] && uptodate[SPAN_WSECT_DRIVER] )
            {
                double tan1 = tan( sweep->Get() * DEG_2_RAD );
                double tan2 = tan1 + ( rootC->Get() - tipC->Get() ) * ( sweeploc->Get() - secswloc->Get() ) / span->Get();
                secsw->Set( atan( tan2 ) * RAD_2_DEG );
                uptodate[SECSWEEP_WSECT_DRIVER] = true;
            }
        }

        // Each pass through the loop should update at least one variable.
        // With m_Nvar variables and m_Nchoice initially known, all should
        // be updated in ( m_Nvar - m_Nchoice ) iterations.  If not, we're
        // in an infinite loop.
        assert( niter < ( m_Nvar - m_Nchoice ) );
        niter++;
    }
}

bool WingDriverGroup::ValidDrivers( vector< int > choices )
{
    // Check for duplicate selections.
    for( int i = 0; i < (int)choices.size() - 1; i++ )
    {
        for( int j = i + 1; j < (int)choices.size(); j++ )
        {
            if( choices[i] == choices[j] )
            {
                return false;
            }
        }
    }

    // Check for algebraically nonsense selections.
    if( vector_contains_val( choices, ( int ) AR_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) SPAN_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) AREA_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) AR_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) SPAN_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) AVEC_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) AR_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) AREA_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) AVEC_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) AREA_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) SPAN_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) AVEC_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) TAPER_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) ROOTC_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) TIPC_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) AVEC_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) ROOTC_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) TIPC_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) AVEC_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) TAPER_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) TIPC_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) AVEC_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) ROOTC_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) TAPER_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) SECSWEEP_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) AR_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) TAPER_WSECT_DRIVER ) )
    {
        return false;
    }

    // Valid unless sweep and secondary sweep are equal.  In that case,
    // taper = 1 and provides no additional information.
    if( vector_contains_val( choices, ( int ) SECSWEEP_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) TAPER_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) SECSWEEP_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) ROOTC_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) TIPC_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) SECSWEEP_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) AVEC_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) TIPC_WSECT_DRIVER ) )
    {
        return false;
    }

    if( vector_contains_val( choices, ( int ) SECSWEEP_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) ROOTC_WSECT_DRIVER ) &&
            vector_contains_val( choices, ( int ) AVEC_WSECT_DRIVER ) )
    {
        return false;
    }

    return true;
}

//=========================================================================================================//
//=========================================================================================================//
//=========================================================================================================//

//==== Constructor ====//
WingSect::WingSect( XSecCurve *xsc ) : XSec( xsc)
{
    m_Type = vsp::XSEC_WING;

    m_ProjectedSpan = 0.0;

    m_XDelta  = m_YDelta  = m_ZDelta  = 0;
    m_XRotate = m_YRotate = m_ZRotate = 0;
    m_XCenterRot = m_YCenterRot = m_ZCenterRot = 0;

    m_Aspect.Init( "Aspect", m_GroupName, this, 1.0, 0.001, 1000.0 );
    m_Aspect.SetDescript( "Aspect Ratio of Wing Section" );
    m_Taper.Init( "Taper", m_GroupName, this, 1.0, 0.0, 1000.0 );
    m_Taper.SetDescript( "Taper Ratio of Wing Section" );
    m_Area.Init( "Area", m_GroupName, this, 1.0, 1e-10, 1.0e12 );
    m_Area.SetDescript( "Area of Wing Section" );
    m_Span.Init( "Span", m_GroupName, this, 1.0, 1e-6, 1000000.0 );
    m_Span.SetDescript( "Span of Wing Section" );
    m_AvgChord.Init( "Avg_Chord", m_GroupName, this, 1.0, 0.0, 1000000.0 );
    m_AvgChord.SetDescript( "Avg Chord of Wing Section" );
    m_TipChord.Init( "Tip_Chord", m_GroupName, this, 1.0, 0.0, 1000000.0 );
    m_TipChord.SetDescript( "Tip Chord of Wing Section" );
    m_RootChord.Init( "Root_Chord", m_GroupName, this, 1.0, 0.0, 1000000.0 );
    m_RootChord.SetDescript( "Root Chord of Wing Section" );
    m_SecSweep.Init( "Sec_Sweep", m_GroupName, this, 0.0, -89.0, 89.0 );
    m_SecSweep.SetDescript( "Secondary Sweep of Wing Section" );

    m_Sweep.Init( "Sweep", m_GroupName, this, 0.0, -89.0, 89.0 );
    m_Sweep.SetDescript( "Sweep of Wing Section" );
    m_SweepLoc.Init( "Sweep_Location", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_SweepLoc.SetDescript( "Location Along Chord That Sweep is Measured For Wing Section" );
    m_SecSweepLoc.Init( "Sec_Sweep_Location", m_GroupName, this, 1.0, 0.0, 1.0 );
    m_SecSweepLoc.SetDescript( "Location Along Chord That Secondary Sweep is Measured For Wing Section" );
    m_SweepLoc.SetOtherParmID( m_SecSweepLoc.GetID(), .001 );
    m_SecSweepLoc.SetOtherParmID( m_SweepLoc.GetID(), .001 );
    m_Twist.Init( "Twist", m_GroupName, this, 0.0, -180.0, 180.0 );
    m_Twist.SetDescript( "Twist of Wing Section" );
    m_TwistLoc.Init( "Twist_Location", m_GroupName, this, 0.25, 0.0, 1.0 );
    m_TwistLoc.SetDescript( "Location Along Chord That Airfoil is Rotated" );
    m_Dihedral.Init( "Dihedral", m_GroupName, this, 0.0, -360.0, 360.0 );
    m_Dihedral.SetDescript( "Dihedral of Wing Section" );

    m_RootCluster.Init( "InCluster", m_GroupName, this, 1.0, 1e-4, 10.0 );
    m_RootCluster.SetDescript( "Inboard Tess Cluster Control" );
    m_TipCluster.Init( "OutCluster", m_GroupName, this, 1.0, 1e-4, 10.0 );
    m_TipCluster.SetDescript( "Outboard Tess Cluster Control" );
}

//==== Set Scale ====//
void WingSect::SetScale( double scale )
{
    XSec::SetScale( scale );
}

//==== Update ====//
void WingSect::UpdateFromWing()
{
    m_LateUpdateFlag = false;

    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();

    // apply the needed transformation to get section into body orientation
    Matrix4d mat;
    xsecsurf->GetBasicTransformation( m_XSCurve->GetWidth(), mat );

    VspCurve baseCurve = GetUntransformedCurve();

    baseCurve.Transform( mat );

    //==== Apply Transform ====//
    m_TransformedCurve = baseCurve;

    Matrix4d tran_mat;
    tran_mat.translatef( m_XDelta, m_YDelta, m_ZDelta );

    Matrix4d rotate_mat;
    rotate_mat.rotateX( m_XRotate );
    rotate_mat.rotateY( m_YRotate );
    rotate_mat.rotateZ( m_ZRotate );

    Matrix4d cent_mat;
    cent_mat.translatef( -m_XCenterRot, -m_YCenterRot, -m_ZCenterRot );

    Matrix4d inv_cent_mat;
    inv_cent_mat.translatef( m_XCenterRot, m_YCenterRot, m_ZCenterRot );

    m_Transform.loadIdentity();

    m_Transform.postMult( tran_mat.data() );
    m_Transform.postMult( cent_mat.data() );
    m_Transform.postMult( rotate_mat.data() );
    m_Transform.postMult( inv_cent_mat.data() );

    m_Transform.postMult( xsecsurf->GetGlobalXForm().data() );

    m_TransformedCurve.Transform( m_Transform );

}

void WingSect::Update()
{
    // Update() is empty because it only needs to be called from WingGeom::UpdateSurf().
    // This empty method is left in place to keep all ohter callers happy.
}

//==== Copy position from base class ====//
void WingSect::CopyBasePos( XSec* xs )
{
    //if ( xs )
    //{
    //    WingSect* sxs = ( WingSect* ) xs;

    //    m_XDelta = sxs->m_XDelta;
    //    m_YDelta = sxs->m_YDelta;
    //    m_ZDelta = sxs->m_ZDelta;

    //    m_XRotate = sxs->m_XRotate;
    //    m_YRotate = sxs->m_YRotate;
    //    m_ZRotate = sxs->m_ZRotate;
    //}
}

//==== Get Driver Parms ====//
vector< string > WingSect::GetDriverParms()
{
    vector< string > parm_ids;
    parm_ids.resize( vsp::NUM_WSECT_DRIVER + 3 );
    parm_ids[ vsp::AR_WSECT_DRIVER ] = m_Aspect.GetID();
    parm_ids[ vsp::SPAN_WSECT_DRIVER ] = m_Span.GetID();
    parm_ids[ vsp::AREA_WSECT_DRIVER ] = m_Area.GetID();
    parm_ids[ vsp::TAPER_WSECT_DRIVER ] = m_Taper.GetID();
    parm_ids[ vsp::AVEC_WSECT_DRIVER ] = m_AvgChord.GetID();
    parm_ids[ vsp::ROOTC_WSECT_DRIVER ] = m_RootChord.GetID();
    parm_ids[ vsp::TIPC_WSECT_DRIVER ] = m_TipChord.GetID();
    parm_ids[ vsp::SECSWEEP_WSECT_DRIVER ] = m_SecSweep.GetID();
    parm_ids[ vsp::SWEEP_WSECT_DRIVER ] = m_Sweep.GetID();
    parm_ids[ vsp::SWEEPLOC_WSECT_DRIVER ] = m_SweepLoc.GetID();
    parm_ids[ vsp::SECSWEEPLOC_WSECT_DRIVER ] = m_SecSweepLoc.GetID();

    return parm_ids;
}

void WingSect::ForceChordVal( double val, bool root_chord_flag )
{
    vector<int> span_rc_tc;
    span_rc_tc.push_back( vsp::SPAN_WSECT_DRIVER );
    span_rc_tc.push_back( vsp::ROOTC_WSECT_DRIVER );
    span_rc_tc.push_back( vsp::TIPC_WSECT_DRIVER );

    vector< int > save_choice_vec = m_DriverGroup.GetChoices();
    m_DriverGroup.SetChoices( span_rc_tc );
    if ( root_chord_flag )
        m_RootChord = val;
    else
        m_TipChord = val;

    m_DriverGroup.UpdateGroup( GetDriverParms() );
    m_DriverGroup.SetChoices( save_choice_vec );
    m_DriverGroup.UpdateGroup( GetDriverParms() );

    Update();
}


void WingSect::ForceSpanRcTc(  double span, double rc, double tc )
{
    vector<int> span_rc_tc;
    span_rc_tc.push_back( vsp::SPAN_WSECT_DRIVER );
    span_rc_tc.push_back( vsp::ROOTC_WSECT_DRIVER );
    span_rc_tc.push_back( vsp::TIPC_WSECT_DRIVER );

    vector< int > save_choice_vec = m_DriverGroup.GetChoices();
    m_DriverGroup.SetChoices( span_rc_tc );

    m_Span = span;
    m_RootChord = rc;
    m_TipChord = tc;

    m_DriverGroup.UpdateGroup( GetDriverParms() );
    m_DriverGroup.SetChoices( save_choice_vec );
    m_DriverGroup.UpdateGroup( GetDriverParms() );

    Update();
}

void WingSect::ForceAspectTaperArea( double aspect, double taper, double area )
{
    vector<int> aspect_taper_area;
    aspect_taper_area.push_back( vsp::AR_WSECT_DRIVER );
    aspect_taper_area.push_back( vsp::TAPER_WSECT_DRIVER );
    aspect_taper_area.push_back( vsp::AREA_WSECT_DRIVER );

    vector< int > save_choice_vec = m_DriverGroup.GetChoices();
    m_DriverGroup.SetChoices( aspect_taper_area );

    m_Aspect = aspect;
    m_Taper  = taper;
    m_Area   = area;

    m_DriverGroup.UpdateGroup( GetDriverParms() );
    m_DriverGroup.SetChoices( save_choice_vec );
    m_DriverGroup.UpdateGroup( GetDriverParms() );

    Update();
}

double CalcTanSweepAt( double loc, double sweep, double baseloc, double aspect, double taper )
{
    double tan_sweep = tan( sweep * DEG_2_RAD );
    double tan_sweep_at = tan_sweep - ( 2.0 / aspect ) * ( loc - baseloc ) * ( 1.0 - taper ) / ( 1.0 + taper ) ;

    return ( tan_sweep_at );
}

double WingSect::GetTanSweepAt( double sweep, double loc  )
{
    return CalcTanSweepAt( loc, sweep, m_SweepLoc.Get(), m_Aspect.Get(), m_Taper.Get() );
}

//==== Encode XML ====//
xmlNodePtr WingSect::EncodeXml(  xmlNodePtr & node  )
{
    ParmContainer::EncodeXml( node );
    xmlNodePtr xsec_node = xmlNewChild( node, NULL, BAD_CAST "XSec", NULL );
    if ( xsec_node )
    {
        XmlUtil::AddIntNode( xsec_node, "Type", m_Type );
        XmlUtil::AddStringNode( xsec_node, "GroupName", m_GroupName );

        m_DriverGroup.EncodeXml( xsec_node );

        xmlNodePtr xscrv_node = xmlNewChild( xsec_node, NULL, BAD_CAST "XSecCurve", NULL );
        if ( xscrv_node )
        {
            m_XSCurve->EncodeXml( xscrv_node );
        }
    }
    return xsec_node;
}

//==== Decode XML ====//
// Called from XSec::DecodeXSec, XSec::CopyFrom, and overridden calls to ParmContainer::DecodeXml --
// i.e. during DecodeXml entire Geom, but also for in-XSecSurf copy/paste/insert.
xmlNodePtr WingSect::DecodeXml(  xmlNodePtr & node  )
{
    ParmContainer::DecodeXml( node );

    xmlNodePtr child_node = XmlUtil::GetNode( node, "XSec", 0 );
    if ( child_node )
    {
        m_GroupName = XmlUtil::FindString( child_node, "GroupName", m_GroupName );

         m_DriverGroup.DecodeXml( child_node );

        xmlNodePtr xscrv_node = XmlUtil::GetNode( child_node, "XSecCurve", 0 );
        if ( xscrv_node )
        {
            m_XSCurve->DecodeXml( xscrv_node );
        }
    }
    return child_node;
}

void WingSect::ReadV2File( xmlNodePtr &sec_node )
{
    vector<int> drivers;

    int v2driver = XmlUtil::FindInt( sec_node, "Driver", -1 );
    switch ( v2driver )
    {
    case V2_AR_TR_A:
        drivers.push_back( AR_WSECT_DRIVER );
        drivers.push_back( TAPER_WSECT_DRIVER );
        drivers.push_back( AREA_WSECT_DRIVER );
        break;
    case V2_AR_TR_S:
        drivers.push_back( AR_WSECT_DRIVER );
        drivers.push_back( TAPER_WSECT_DRIVER );
        drivers.push_back( SPAN_WSECT_DRIVER );
        break;
    case V2_AR_TR_TC:
        drivers.push_back( AR_WSECT_DRIVER );
        drivers.push_back( TAPER_WSECT_DRIVER );
        drivers.push_back( TIPC_WSECT_DRIVER );
        break;
    case V2_AR_TR_RC:
        drivers.push_back( AR_WSECT_DRIVER );
        drivers.push_back( TAPER_WSECT_DRIVER );
        drivers.push_back( ROOTC_WSECT_DRIVER );
        break;
    case V2_S_TC_RC:
        drivers.push_back( SPAN_WSECT_DRIVER );
        drivers.push_back( TIPC_WSECT_DRIVER );
        drivers.push_back( ROOTC_WSECT_DRIVER );
        break;
    case V2_A_TC_RC:
        drivers.push_back( AREA_WSECT_DRIVER );
        drivers.push_back( TIPC_WSECT_DRIVER );
        drivers.push_back( ROOTC_WSECT_DRIVER );
        break;
    case V2_TR_S_A:
        drivers.push_back( TAPER_WSECT_DRIVER );
        drivers.push_back( SPAN_WSECT_DRIVER );
        drivers.push_back( AREA_WSECT_DRIVER );
        break;
    }
    m_DriverGroup.SetChoices( drivers );

// Let these parameters fall out of Span, TC, RC to ensure consistency.
//    m_Aspect = XmlUtil::FindDouble( sec_node, "AR", m_Aspect() );
//    m_Taper = XmlUtil::FindDouble( sec_node, "TR", m_Taper() );
//    m_Area = XmlUtil::FindDouble( sec_node, "Area", m_Area() );

    double span = XmlUtil::FindDouble( sec_node, "Span", m_Span() );
    double tc = XmlUtil::FindDouble( sec_node, "TC", m_TipChord() );
    double rc = XmlUtil::FindDouble( sec_node, "RC", m_RootChord() );

    ForceSpanRcTc( span, rc, tc );

    m_Sweep = XmlUtil::FindDouble( sec_node, "Sweep", m_Sweep() );
    double swloc = XmlUtil::FindDouble( sec_node, "SweepLoc", m_SweepLoc() );

    // Do a jig to avoid conflicting NotEqParm.
    if ( swloc != 1.0 )
    {
        m_SweepLoc.Set( swloc );
    }
    else
    {
        m_SweepLoc.Set( 0.5 );
        m_SecSweepLoc.Set( 0.0 ); // Set secondary to LE.
        m_SweepLoc.Set( 1.0 );
    }

    m_Twist = XmlUtil::FindDouble( sec_node, "Twist", m_Twist() );
    m_TwistLoc = XmlUtil::FindDouble( sec_node, "TwistLoc", m_TwistLoc() );
    m_Dihedral = XmlUtil::FindDouble( sec_node, "Dihedral", m_Dihedral() );

//    ws.dihed_crv1_set( XmlUtil::FindDouble( sec_node, "Dihed_Crv1", ws.dihed_crv1_val() ) );
//    ws.dihed_crv2_set( XmlUtil::FindDouble( sec_node, "Dihed_Crv2", ws.dihed_crv2_val() ) );
//    ws.dihed_crv1_str_set( XmlUtil::FindDouble( sec_node, "Dihed_Crv1_Str", ws.dihed_crv1_str_val() ) );
//    ws.dihed_crv2_str_set( XmlUtil::FindDouble( sec_node, "Dihed_Crv2_Str", ws.dihed_crv2_str_val() ) );

//    ws.dihedRotFlag = XmlUtil::FindInt( sec_node, "DihedRotFlag", ws.dihedRotFlag );
//    ws.smoothBlendFlag = XmlUtil::FindInt( sec_node, "SmoothBlendFlag", ws.smoothBlendFlag );

    m_SectTessU = XmlUtil::FindInt( sec_node, "NumInterpXsecs", m_SectTessU() );
}

//=========================================================================================================//
//=========================================================================================================//
//=========================================================================================================//


//==== Constructor ====//
WingGeom::WingGeom( Vehicle* vehicle_ptr ) : GeomXSec( vehicle_ptr )
{
    m_Name = "WingGeom";
    m_Type.m_Name = "Wing";
    m_Type.m_Type = MS_WING_GEOM_TYPE;

    m_MinActiveXSec = 1;

    m_Closed = false;

    m_XSecSurf.SetParentContainer( GetID() );
    m_XSecSurf.SetBasicOrientation( vsp::Y_DIR, X_DIR, XS_SHIFT_LE, true );

    m_RelativeDihedralFlag.Init("RelativeDihedralFlag", m_Name, this, 0, 0, 1 );
    m_RelativeDihedralFlag.SetDescript( "Relative or Absolute Dihedral" );

    m_RelativeTwistFlag.Init("RelativeTwistFlag", m_Name, this, 0, 0, 1 );
    m_RelativeTwistFlag.SetDescript( "Relative or Absolute Twist" );

    m_RotateAirfoilMatchDiedralFlag.Init("RotateAirfoilMatchDideralFlag", m_Name, this, 0, 0, 1 );
    m_RotateAirfoilMatchDiedralFlag.SetDescript( "Rotate Airfoil To Stay Tangent To Dihedral (or Not)" );

    m_TotalSpan.Init( "TotalSpan", m_Name, this, 1.0, 1e-6, 1000000.0 );
    m_TotalSpan.SetDescript( "Total Planform Span" );

    m_TotalProjSpan.Init( "TotalProjectedSpan", m_Name, this, 1.0, 1e-6, 1000000.0 );
    m_TotalProjSpan.SetDescript( "Total Projected Planform Span" );

    m_TotalChord.Init( "TotalChord", m_Name, this, 1.0, 0.0, 1000000.0 );
    m_TotalChord.SetDescript( "Total Planform Chord" );

    m_TotalArea.Init( "TotalArea", m_Name, this, 1.0, 1e-10, 1.0e12 );
    m_TotalArea.SetDescript( "Total Planform Area" );

    m_LECluster.Init( "LECluster", m_Name, this, 0.25, 1e-4, 10.0 );
    m_LECluster.SetDescript( "LE Tess Cluster Control" );

    m_TECluster.Init( "TECluster", m_Name, this, 0.25, 1e-4, 10.0 );
    m_TECluster.SetDescript( "TE Tess Cluster Control" );

    m_SmallPanelW.Init( "SmallPanelW", m_Name, this, 0.0, 0.0, 1e12 );
    m_SmallPanelW.SetDescript( "Smallest LE/TE panel width");

    m_MaxGrowth.Init( "MaxGrowth", m_Name, this, 1.0, 1.0, 1e12);
    m_MaxGrowth.SetDescript( "Maximum chordwise panel growth ratio" );

    //==== rename capping controls for wing specific terminology ====//
    m_CapUMinOption.SetDescript("Type of End Cap on Wing Root");
    m_CapUMinOption.Parm::Set(FLAT_END_CAP);
    m_CapUMinTess.SetDescript("Number of tessellated curves on Wing Root and Tip");
    m_CapUMaxOption.SetDescript("Type of End Cap on Wing Tip");
    m_CapUMaxOption.Parm::Set(FLAT_END_CAP);

    //==== Init Parms ====//
    m_TessU = 16;
    m_TessW = 31;
    m_ActiveXSec = 1;
    m_ActiveAirfoil = 0;
    m_SymPlanFlag = SYM_XZ;

    //==== Wing XSecs ====//
    m_XSecSurf.SetXSecType( XSEC_WING );
    m_XSecSurf.SetCutMinNumXSecs( 2 );

    m_XSecSurf.AddXSec( vsp::XS_FOUR_SERIES );
    m_XSecSurf.AddXSec( vsp::XS_FOUR_SERIES );

    WingSect* ws;

    ws = ( WingSect* ) m_XSecSurf.FindXSec( 0 );
    ws->SetGroupDisplaySuffix( 0 );

    ws = ( WingSect* ) m_XSecSurf.FindXSec( 1 );
    ws->SetGroupDisplaySuffix( 1 );
    ws->m_Sweep = 30.0;
    ws->m_RootChord = 4.0;
    ws->m_TipChord = 1.0;
    ws->m_Span = 9.0;

    UpdateSurf();
}

//==== Destructor ====//
WingGeom::~WingGeom()
{

}

//==== Change IDs =====//
void WingGeom::ChangeID( string id )
{
    Geom::ChangeID( id );
    m_XSecSurf.SetParentContainer( GetID() );
}

//==== Look Though All Parms and Load Linkable Ones ===//
void WingGeom::AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id )
{
    Geom::AddLinkableParms( linkable_parm_vec );

    m_XSecSurf.AddLinkableParms( linkable_parm_vec, m_ID  );
}

//==== Scale ====//
void WingGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();

    if( abs( 1.0 - currentScale ) > 1e-6 )
    {
        //==== Adjust Sections Area ====//
        vector< WingSect* > ws_vec = GetWingSectVec();
        for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
        {
            WingSect* ws = ws_vec[i];
            if ( ws )
            {
                double area = ws->m_Area()*currentScale*currentScale;
                ws->ForceAspectTaperArea( ws->m_Aspect(), ws->m_Taper(), area );
            }
        }

        m_LastScale = m_Scale();
    }
}

void WingGeom::AddDefaultSources( double base_len )
{
    vector< WingSect* > ws_vec = GetWingSectVec();
    double nseg = ( ( int ) ws_vec.size() ) - 1;
    double ustart = 0.0;

    if ( m_CapUMinOption() != NO_END_CAP )
    {
        nseg = nseg + 1.0;
        ustart = ustart  + 1.0;
    }

    if ( m_CapUMaxOption() != NO_END_CAP )
    {
        nseg = nseg + 1.0;;
    }

    char str[256];

    LineSource* lsource;

    for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
    {
        WingSect* ws = ws_vec[i];
        if ( ws )
        {
            double cr = ws->m_RootChord();
            double ct = ws->m_TipChord();

            lsource = new LineSource();
            sprintf( str, "Def_TE_LS_%d", i );
            lsource->SetName( str );
            lsource->m_Len = 0.01 * cr;
            lsource->m_Len2 = 0.01 * ct;
            lsource->m_Rad = 0.2 * cr;
            lsource->m_Rad2 = 0.2 * ct;
            lsource->m_ULoc1 = (i+ustart-1)/nseg;
            lsource->m_WLoc1 = 0.0;
            lsource->m_ULoc2 = (i+ustart)/nseg;
            lsource->m_WLoc2 = 0.0;
            AddCfdMeshSource( lsource );

            lsource = new LineSource();
            sprintf( str, "Def_LE_LS_%d", i );
            lsource->SetName( str );
            lsource->m_Len = 0.01 * cr;
            lsource->m_Len2 = 0.01 * ct;
            lsource->m_Rad = 0.2 * cr;
            lsource->m_Rad2 = 0.2 * ct;
            lsource->m_ULoc1 = (i+ustart-1)/nseg;
            lsource->m_WLoc1 = 0.5;
            lsource->m_ULoc2 = (i+ustart)/nseg;
            lsource->m_WLoc2 = 0.5;
            AddCfdMeshSource( lsource );

            if ( i == ( ( int ) ws_vec.size() - 1 ) )
            {
                lsource = new LineSource();
                lsource->SetName( "Def_Tip_LS" );
                lsource->m_Len = 0.01 * ct;
                lsource->m_Len2 = 0.01 * ct;
                lsource->m_Rad = 0.2 * ct;
                lsource->m_Rad2 = 0.2 * ct;
                lsource->m_ULoc1 = (i+ustart)/nseg;
                lsource->m_WLoc1 = 0.0;
                lsource->m_ULoc2 = (i+ustart)/nseg;
                lsource->m_WLoc2 = 0.5;
                AddCfdMeshSource( lsource );
            }
        }
    }
}

//==== Drag Parameters ====//
void WingGeom::LoadDragFactors( DragFactors& drag_factors )
{


}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr WingGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );
    xmlNodePtr wing_node = xmlNewChild( node, NULL, BAD_CAST "WingGeom", NULL );
    if ( wing_node )
    {
        m_XSecSurf.EncodeXml( wing_node );
    }
    return wing_node;
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr WingGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr wing_node = XmlUtil::GetNode( node, "WingGeom", 0 );
    if ( wing_node )
    {
        m_XSecSurf.DecodeXml( wing_node );
    }

    return wing_node;
}

//==== Compute Rotation Center ====//
void WingGeom::ComputeCenter()
{
    m_Center = vec3d(0,0,0);

    WingSect* ws = ( WingSect* ) m_XSecSurf.FindXSec( 1 );
    if ( ws )
    {
        double len = ws->m_RootChord();
        m_Center.set_x( len*m_Origin() );
    }
}

//==== Set Index For Active Airfoil ====//
void WingGeom::SetActiveAirfoilIndex( int index )
{
    index = Clamp<int>( index, 0, m_XSecSurf.NumXSec() - 1 );
    m_ActiveAirfoil = index;
}

//==== Set Active XSec Type ====//
void WingGeom::SetActiveAirfoilType( int type )
{
    XSec* xs = GetXSec( m_ActiveAirfoil );

    if ( !xs )
    {
        return;
    }

    if ( type == xs->GetXSecCurve()->GetType() )
    {
        return;
    }

    m_XSecSurf.ChangeXSecShape( m_ActiveAirfoil, type );

    Update();
}


bool WingGeom::IsClosed() const
{
    return m_Closed;
}

WingSect* WingGeom::GetWingSect( int index )
{
    XSec* xs = m_XSecSurf.FindXSec( index );
    if ( xs )
    {
        WingSect* ws = dynamic_cast< WingSect* >(xs);
        return ws;
    }
    return NULL;
}

//==== Override Geom Cut/Copy/Insert/Paste ====//
void WingGeom::CutXSec( int index )
{
    SetActiveXSecIndex( index );
    CutWingSect( index );
}
void WingGeom::CopyXSec( int index )
{
    CopyWingSect( index );
}
void WingGeom::PasteXSec( int index )
{
    PasteWingSect( index );
}
void WingGeom::InsertXSec( int index, int type )
{
    if ( index > 0 && index < m_XSecSurf.NumXSec() )
    {
        string ins_id = m_XSecSurf.InsertXSec(type, index);
        SetActiveXSecIndex( index + 1 );
        PasteWingSect( GetActiveXSecIndex() );
    }
}



//==== Insert Wing Sect ====//
void WingGeom::SplitWingSect( int index  )
{
    WingSect* ws = GetWingSect( index );
    if ( ws )
    {
        double span = ws->m_Span();
        double rc = ws->m_RootChord();
        double ac = ws->m_AvgChord();
        double tc = ws->m_TipChord();
        double twist = 0.5*ws->m_Twist();

        int curve_type = ws->GetXSecCurve()->GetType();

        string ins_id = m_XSecSurf.InsertXSec( curve_type, index );

        ws->ForceSpanRcTc( span*0.5, rc, ac );
        ws->m_Twist = twist;
        ws->Update();

        XSec* xs = m_XSecSurf.FindXSec( ins_id );
        if ( xs )
        {
            WingSect* ins_ws = dynamic_cast< WingSect* >(xs);
            ins_ws->CopyFrom( ws );
            ins_ws->ForceSpanRcTc( span*0.5, ac, tc );
            ins_ws->m_Twist = twist;
            ins_ws->Update();
        }
    }

}

//==== Cut Wing Sect ====//
void WingGeom::CutWingSect( int index  )
{
    if ( index > 0 && index < m_XSecSurf.NumXSec() )
    {
        m_XSecSurf.CutXSec( index );

        //==== Reset Active Indeices ====//
        SetActiveXSecIndex( GetActiveXSecIndex() );
        SetActiveAirfoilIndex( GetActiveAirfoilIndex() );
    }
}


//==== Copy Wing Sect ====//
void WingGeom::CopyWingSect( int index  )
{
    if ( index > 0 && index < m_XSecSurf.NumXSec() )
    {
        m_XSecSurf.CopyXSec(index);
    }
}

//==== Paste Wing Sect ====//
void WingGeom::PasteWingSect( int index  )
{
    if ( index > 0 && index < m_XSecSurf.NumXSec() )
    {
        m_XSecSurf.PasteXSec(index);
        XSec* xs = m_XSecSurf.FindXSec( index );
        if ( xs )
            xs->SetLateUpdateFlag( true );
        Update();
    }
}

//==== Copy Wing Sect ====//
void WingGeom::CopyAirfoil( int index  )
{
    if ( index >= 0 && index < m_XSecSurf.NumXSec() )
    {
        m_XSecSurf.CopyXSecCurve( index );
    }
}

//==== Paste Wing Sect ====//
void WingGeom::PasteAirfoil( int index  )
{
    if ( index >= 0 && index < m_XSecSurf.NumXSec() )
    {
        m_XSecSurf.PasteXSecCurve(index);
        XSec* xs = m_XSecSurf.FindXSec( index );
        if ( xs )
            xs->SetLateUpdateFlag( true );
        Update();
    }
}

//==== Insert Wing Sect ====//
void WingGeom::InsertWingSect( int index  )
{
    if ( index > 0 && index < m_XSecSurf.NumXSec() )
    {
        XSec* xs = m_XSecSurf.FindXSec( index );
        int type = xs->GetXSecCurve()->GetType();

        string ins_id = m_XSecSurf.InsertXSec(type, index);
        SetActiveXSecIndex( index + 1 );
        PasteWingSect( GetActiveXSecIndex() );
    }
}

//==== Update Wing And Cross Section Placement ====//
void WingGeom::UpdateSurf()
{
    //==== Check If Total Span/Chord/Area Has Changed ====//
    bool total_change_flag = false;
    if ( UpdatedParm( m_TotalSpan.GetID() ) )
    {
        UpdateTotalSpan();
        total_change_flag = true;
    }

    if ( UpdatedParm( m_TotalProjSpan.GetID() ) )
    {
        UpdateTotalProjSpan();
        total_change_flag = true;
    }

    if ( UpdatedParm( m_TotalChord.GetID() ) )
    {
        UpdateTotalChord();
        total_change_flag = true;
    }

    if ( UpdatedParm( m_TotalArea.GetID() ) )
    {
        UpdateTotalArea();
        total_change_flag = true;
    }

    int active_sect = GetActiveXSecIndex();     // Save Active Section

    //==== Set Temp Active XSec Based On Updated Parms ====//
    if ( total_change_flag )
        SetActiveXSecIndex(1);
    else
        SetTempActiveXSec();

    //==== Make Sure Chord Match For Adjacent Wing Sections ====//
    MatchWingSections();

    SetActiveXSecIndex(active_sect);            // Restore Active Section


    // clear the u tessellation vector
    m_TessUVec.clear();
    m_RootClusterVec.clear();
    m_TipClusterVec.clear();

    vector< VspCurve > crv_vec( m_XSecSurf.NumXSec() );

    //==== Compute Parameters For Each Section ====//
    double total_span = 0.0;
    double total_sweep_offset = 0.0;
    double total_dihed_offset = 0.0;
    double total_twist = 0.0;

    //==== Load End Points for Each Section ====//
    for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
    {
        WingSect* ws = ( WingSect* ) m_XSecSurf.FindXSec( i );
        if ( ws )
        {
            //==== Reset Group Names ====//
            ws->SetGroupDisplaySuffix( i );

            double rad = ws->m_Span();
            double ty = rad*cos(GetSumDihedral(i)*DEG_2_RAD);
            double tz = rad*sin(GetSumDihedral(i)*DEG_2_RAD);

            double tan_le  = ws->GetTanSweepAt( ws->m_Sweep(), 0.0 );
            double toff    = tan_le*rad;                    // Tip X Offset

            if ( i == 0 )
            {
                ty = 0;
                tz = 0;
                toff = 0;
            }

            total_dihed_offset += tz;
            total_span += ty;
            total_sweep_offset += toff;

            if ( m_RelativeTwistFlag() )
                total_twist += ws->m_Twist();
            else
                total_twist = ws->m_Twist();

            ws->SetProjectedSpan( ty );

            //==== Find Width Parm ====//
            string width_id = ws->GetXSecCurve()->GetWidthParmID();
            Parm* width_parm = ParmMgr.FindParm( width_id );

            if ( width_parm )
            {
                width_parm->Deactivate();
                width_parm->Set( ws->m_TipChord() );
            }

            double dihead_rot = 0.0;
            if ( m_RotateAirfoilMatchDiedralFlag() )
            {
                if ( i == 0 )
                {
                    dihead_rot = GetSumDihedral( i+1 );
                }
                else if ( i ==  m_XSecSurf.NumXSec()-1 )
                {
                    dihead_rot = GetSumDihedral( i );
                }
                else
                {
                    dihead_rot = 0.5*( GetSumDihedral( i ) + GetSumDihedral( i+1 ) );
                }
            }

            //==== Load Transformations =====//
            ws->m_YDelta = total_span;
            ws->m_XDelta = total_sweep_offset;
            ws->m_ZDelta = total_dihed_offset;

            ws->m_YRotate = total_twist;
            ws->m_XRotate = dihead_rot;

            ws->m_XCenterRot = ws->m_XDelta + ws->m_TwistLoc()*ws->m_TipChord();
            ws->m_YCenterRot = ws->m_YDelta;
            ws->m_ZCenterRot = ws->m_ZDelta;

            // Force update to wing section.
            ws->UpdateFromWing();

            crv_vec[i] =  ws->GetCurve();

            if ( i > 0 )
            {
                m_TessUVec.push_back( ws->m_SectTessU() );
                m_RootClusterVec.push_back( ws->m_RootCluster() );
                m_TipClusterVec.push_back( ws->m_TipCluster() );
            }
        }
    }

    m_MainSurfVec[0].SkinC0( crv_vec, false );
    if ( m_XSecSurf.GetFlipUD() )
    {
        m_MainSurfVec[0].FlipNormal();
    }
    m_MainSurfVec[0].SetSurfType( vsp::WING_SURF );
    m_MainSurfVec[0].SetMagicVParm( true );

    m_MainSurfVec[0].SetClustering( m_LECluster(), m_TECluster() );

    //==== Load Totals ====//
    m_TotalSpan = ComputeTotalSpan();
    m_TotalProjSpan = ComputeTotalProjSpan();
    m_TotalChord = ComputeTotalChord();
    m_TotalArea = ComputeTotalArea();

    CalculateMeshMetrics();
}

void WingGeom::CalculateMeshMetrics()
{
    // Check dimensional LE/TE first panel width at all hard airfoil sections.

    int nu = m_MainSurfVec[0].GetNumSectU();

    std::vector<double> vcheck( 8 );

    double vmin, vmax, vle, vlelow, vleup, vtruemax;

    vmin = 0.0;
    vmax = m_MainSurfVec[0].GetWMax();
    vtruemax = vmax;

    vle = ( vmin + vmax ) * 0.5;

    vmin += TMAGIC;
    vmax -= TMAGIC;

    vlelow = vle - TMAGIC;
    vleup = vle + TMAGIC;

    double dj = 2.0 / ( m_TessW() - 1 );

    // Calculate lower surface tessellation check points.
    vcheck[0] = ( vmin );
    vcheck[1] = ( vmin + ( vlelow - vmin ) * Cluster( dj, m_TECluster(), m_LECluster() ) );
    vcheck[2] = ( vmin + ( vlelow - vmin ) * Cluster( 1.0 - dj, m_TECluster(), m_LECluster() ) );
    vcheck[3] = ( vlelow );

    // Upper surface constructed as:  vupper = m_Surface.get_vmax() - vlower;
    vcheck[4] = vtruemax - vcheck[0];
    vcheck[5] = vtruemax - vcheck[1];
    vcheck[6] = vtruemax - vcheck[2];
    vcheck[7] = vtruemax - vcheck[3];

    // Loop over points checking for minimum panel width.
    double mind = std::numeric_limits < double >::max();
    for ( int i = 0; i < vcheck.size() - 1; i += 2 )
    {
        double v1 = vcheck[ i ];
        double v2 = vcheck[ i + 1 ];

        for ( int j = 0; j <= nu; j++ )
        {
            double u = 1.0 * j;

            double d = dist( m_MainSurfVec[0].CompPnt( u, v1 ), m_MainSurfVec[0].CompPnt( u, v2 ) );
            mind = min( mind, d );
        }
    }
    m_SmallPanelW = mind;

    // Check theoretical growth ratio assuming arclength parameterization is correct.  No need to check actual
    // actual realized surface.  Also, no need to check all airfoil sections.
    double maxrat = 1.0;

    int jle = ( m_TessW() - 1 ) / 2;
    int j = 0;

    double t0 = Cluster( static_cast<double>( j ) / jle, m_TECluster(), m_LECluster() );
    j++;
    double t1 = Cluster( static_cast<double>( j ) / jle, m_TECluster(), m_LECluster() );
    double dt1 = t1 - t0;
    j++;
    double t2;
    for ( ; j <= jle; ++j )
    {
        t2 = Cluster( static_cast<double>( j ) / jle, m_TECluster(), m_LECluster() );

        double dt2 = t2 - t1;

        maxrat = max( maxrat, dt1 / dt2 );
        maxrat = max( maxrat, dt2 / dt1 );

        t0 = t1;
        t1 = t2;
        dt1 = dt2;

    }
    m_MaxGrowth = maxrat;
}

void WingGeom::UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, bool degen )
{
    vector < int > tessvec;
    vector < double > rootc;
    vector < double > tipc;
    if (m_CapUMinOption()!=NO_END_CAP && m_CapUMinSuccess[ m_SurfIndxVec[indx] ] )
    {
        tessvec.push_back( m_CapUMinTess() );
        rootc.push_back( 1.0 );
        tipc.push_back( 1.0 );
    }

    for ( int i = 0; i < m_TessUVec.size(); i++ )
    {
        tessvec.push_back( m_TessUVec[i] );
        rootc.push_back( m_RootClusterVec[i] );
        tipc.push_back( m_TipClusterVec[i] );
    }

    if (m_CapUMaxOption()!=NO_END_CAP && m_CapUMaxSuccess[ m_SurfIndxVec[indx] ] )
    {
        tessvec.push_back( m_CapUMinTess() );
        rootc.push_back( 1.0 );
        tipc.push_back( 1.0 );
    }

    m_SurfVec[indx].SetRootTipClustering( rootc, tipc );
    m_SurfVec[indx].Tesselate( tessvec, m_TessW(), pnts, norms, uw_pnts, m_CapUMinTess(), degen );
}

void WingGeom::UpdateSplitTesselate( int indx, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms )
{
    vector < int > tessvec;
    vector < double > rootc;
    vector < double > tipc;

    if (m_CapUMinOption()!=NO_END_CAP && m_CapUMinSuccess[ m_SurfIndxVec[indx] ] )
    {
        tessvec.push_back( m_CapUMinTess() );
        rootc.push_back( 1.0 );
        tipc.push_back( 1.0 );
    }

    for ( int i = 0; i < m_TessUVec.size(); i++ )
    {
        tessvec.push_back( m_TessUVec[i] );
        rootc.push_back( m_RootClusterVec[i] );
        tipc.push_back( m_TipClusterVec[i] );
    }

    if (m_CapUMaxOption()!=NO_END_CAP && m_CapUMaxSuccess[ m_SurfIndxVec[indx] ] )
    {
        tessvec.push_back( m_CapUMinTess() );
        rootc.push_back( 1.0 );
        tipc.push_back( 1.0 );
    }

    m_SurfVec[indx].SetRootTipClustering( rootc, tipc );
    m_SurfVec[indx].SplitTesselate( tessvec, m_TessW(), pnts, norms, m_CapUMinTess() );
}

void WingGeom::UpdateDrawObj()
{
    Geom::UpdateDrawObj();

    Matrix4d attachMat;
    Matrix4d relTrans;
    attachMat = ComposeAttachMatrix();
    relTrans = attachMat;
    relTrans.affineInverse();
    relTrans.matMult( m_ModelMatrix.data() );
    relTrans.postMult( attachMat.data() );

    int nxsec = m_XSecSurf.NumXSec();
    m_XSecDrawObj_vec.resize( nxsec, DrawObj() );

    //==== Tesselate Surface ====//
    for ( int i = 0 ; i < nxsec ; i++ )
    {
        m_XSecDrawObj_vec[i].m_PntVec = m_XSecSurf.FindXSec( i )->GetDrawLines( relTrans );
        m_XSecDrawObj_vec[i].m_GeomChanged = true;
    }

    m_HighlightXSecDrawObj.m_PntVec = m_XSecSurf.FindXSec( m_ActiveAirfoil )->GetDrawLines( relTrans );
    m_HighlightXSecDrawObj.m_GeomChanged = true;

    double w = m_XSecSurf.FindXSec( m_ActiveAirfoil )->GetXSecCurve()->GetWidth();

    Matrix4d mat;
    m_XSecSurf.GetBasicTransformation( Z_DIR, X_DIR, XS_SHIFT_MID, false, 1.0, mat );
    mat.scale( 1.0/w );

    VspCurve crv = m_XSecSurf.FindXSec( m_ActiveAirfoil )->GetUntransformedCurve();
    crv.Transform( mat );

    vector< vec3d > pts;
    crv.TessAdapt( pts, 1e-2, 10 );

    m_CurrentXSecDrawObj.m_PntVec = pts;
    m_CurrentXSecDrawObj.m_LineWidth = 1.5;
    m_CurrentXSecDrawObj.m_LineColor = vec3d( 0.0, 0.0, 0.0 );
    m_CurrentXSecDrawObj.m_Type = DrawObj::VSP_LINES;
    m_CurrentXSecDrawObj.m_GeomChanged = true;


    VspCurve inbd = m_XSecSurf.FindXSec( m_ActiveXSec - 1 )->GetCurve();
    inbd.Transform( relTrans );

    VspCurve outbd = m_XSecSurf.FindXSec( m_ActiveXSec )->GetCurve();
    outbd.Transform( relTrans );

    BndBox iBBox, oBBox;
    inbd.GetBoundingBox( iBBox );
    outbd.GetBoundingBox( oBBox );
    oBBox.Update( iBBox );

    m_HighlightWingSecDrawObj.m_PntVec = oBBox.GetBBoxDrawLines();

}

void WingGeom::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    GeomXSec::LoadDrawObjs( draw_obj_vec );

    if ( m_Vehicle->IsGeomActive( m_ID ) )
    {
        m_HighlightWingSecDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_HighlightWingSecDrawObj.m_GeomID = BBOXHEADER + m_ID + "ACTIVE_SECT";
        m_HighlightWingSecDrawObj.m_LineWidth = 2.0;
        m_HighlightWingSecDrawObj.m_LineColor = vec3d( 1.0, 0.0, 0.0 );
        m_HighlightWingSecDrawObj.m_Type = DrawObj::VSP_LINES;
        draw_obj_vec.push_back( &m_HighlightWingSecDrawObj );
    }
}


//==== Get All WingSections ====//
vector< WingSect* > WingGeom::GetWingSectVec()
{
    vector< WingSect* > ws_vec;
    for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
    {
        WingSect* ws = ( WingSect* ) m_XSecSurf.FindXSec( i );
        if ( ws )
        {
            ws_vec.push_back( ws );
        }
    }
    return ws_vec;
}

//==== Compute Total Span ====//
double WingGeom::ComputeTotalSpan()
{
    double ts = 0.0;
    vector< WingSect* > ws_vec = GetWingSectVec();
    for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
    {
            ts += ws_vec[i]->m_Span();
    }

    if ( GetSymFlag() != 0 )
    {
        ts *= 2.0;
    }

    return ts;
}



//==== Compute Total Proj Span ====//
double WingGeom::ComputeTotalProjSpan()
{
    double ts = 0.0;
    vector< WingSect* > ws_vec = GetWingSectVec();
    for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
    {
            ts += ws_vec[i]->m_Span()*cos( ws_vec[i]->m_Dihedral() * DEG_2_RAD);
    }

    if ( GetSymFlag() != 0 )
    {
        ts *= 2.0;
    }

    return ts;
}

//==== Compute Total Chord ====//
double WingGeom::ComputeTotalChord()
{
    double tc = 0.0;
    vector< WingSect* > ws_vec = GetWingSectVec();
    for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
    {
        tc += ws_vec[i]->m_AvgChord();
    }
    tc = tc/(double)(ws_vec.size()-1);
    return tc;

}

//==== Compute Total Chord ====//
double WingGeom::ComputeTotalArea()
{
    double ta = 0.0;
    vector< WingSect* > ws_vec = GetWingSectVec();
    for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
    {
        ta += ws_vec[i]->m_Area();
    }

    if ( GetSymFlag() != 0 )
    {
        ta *= 2.0;
    }

    return ta;
}

//==== Update Total Span ====//
void WingGeom::UpdateTotalSpan()
{
     //==== Compute Totals ====//
    double ts = ComputeTotalSpan();

    double fract = 1.0;
    if ( ts > 1.0e-08 )
        fract = m_TotalSpan()/ts;

    //==== Adjust Sections Total Span ====//
    if ( fract > 1.0e-08 )
    {
        vector< WingSect* > ws_vec = GetWingSectVec();
        for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
        {
            double span = ws_vec[i]->m_Span()*fract;
            ws_vec[i]->ForceSpanRcTc( span, ws_vec[i]->m_RootChord(), ws_vec[i]->m_TipChord() );
        }
    }
}
//==== Update Total Proj Span ====//
void WingGeom::UpdateTotalProjSpan()
{
     //==== Compute Totals ====//
    double ts = ComputeTotalProjSpan();

    double fract = 1.0;
    if ( ts > 1.0e-08 )
        fract = m_TotalProjSpan()/ts;

    //==== Adjust Sections Total Span ====//
    if ( fract > 1.0e-08 )
    {
        vector< WingSect* > ws_vec = GetWingSectVec();
        for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
        {
            double span = ws_vec[i]->m_Span()*fract;
            ws_vec[i]->ForceSpanRcTc( span, ws_vec[i]->m_RootChord(), ws_vec[i]->m_TipChord() );
        }
    }
}

//==== Update Total Chord ====//
void WingGeom::UpdateTotalChord()
{
    double totc = ComputeTotalChord();

    double fract = 1.0;
    if ( totc > 1.0e-08 )
        fract = m_TotalChord()/totc;

    //==== Adjust Sections Chord ====//
    if ( fract > 1.0e-08 )
    {
        vector< WingSect* > ws_vec = GetWingSectVec();

        vector< double > new_tc_vals;
        for ( int i = 0 ; i < (int)ws_vec.size() ; i++ )
        {
            new_tc_vals.push_back( ws_vec[i]->m_TipChord()*fract );
        }

        for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
        {
            double rc = new_tc_vals[i-1];
            double tc = new_tc_vals[i];
            ws_vec[i]->ForceSpanRcTc( ws_vec[i]->m_Span(), rc, tc );
        }
    }
}

//==== Update Total Area ====//
void WingGeom::UpdateTotalArea()
{
    double ta = ComputeTotalArea();

    double fract = 1.0;
    if ( ta > 1.0e-08 )
        fract = m_TotalArea()/ta;

    //==== Adjust Sections Area ====//
    if ( fract > 1.0e-08 )
    {
        vector< WingSect* > ws_vec = GetWingSectVec();
        for ( int i = 1 ; i < (int)ws_vec.size() ; i++ )
        {
            double area = ws_vec[i]->m_Area()*fract;
            ws_vec[i]->ForceAspectTaperArea( ws_vec[i]->m_Aspect(), ws_vec[i]->m_Taper(), area );
        }
    }
}


//==== Match Wing Sections =====//
void WingGeom::MatchWingSections()
{
    //==== Match Section Root/Tip ====//
    WingSect* active_ws = ( WingSect* ) m_XSecSurf.FindXSec( m_ActiveXSec );
    if ( active_ws )
    {
        active_ws->m_DriverGroup.UpdateGroup( active_ws->GetDriverParms() );
        double active_rc = active_ws->m_RootChord();
        double active_tc = active_ws->m_TipChord();

        if ( m_ActiveXSec > 0 )
        {
            WingSect* inboard_ws = ( WingSect* ) m_XSecSurf.FindXSec( m_ActiveXSec-1 );
            inboard_ws->ForceChordVal( active_rc, false );
        }
        if ( m_ActiveXSec < m_XSecSurf.NumXSec()-1 )
        {
            WingSect* outboard_ws = ( WingSect* ) m_XSecSurf.FindXSec( m_ActiveXSec+1 );
            outboard_ws->ForceChordVal( active_tc, true );
        }
    }
}

//==== Set Temp Active XSec ====//
void WingGeom::SetTempActiveXSec()
{
    int active_sect_index = -1;
    int largest_change_cnt = 0;

    vector< WingSect* > wsvec = GetWingSectVec();
    for ( int i = 0 ; i < (int)wsvec.size() ; i++ )
    {
        int c = wsvec[i]->GetLatestChangeCnt();
        if ( c > largest_change_cnt )
        {
            active_sect_index = i;
            largest_change_cnt = c;
        }
    }

    if ( active_sect_index >= 0 )
        SetActiveXSecIndex( active_sect_index );
}

//==== Get Sum Dihedral ====//
double WingGeom::GetSumDihedral( int sect_id )
{
    if ( sect_id < 1 || sect_id >= (int)m_XSecSurf.NumXSec() )
        return 0.0;

    if ( m_RelativeDihedralFlag() == false )
    {
        WingSect* ws = ( WingSect* ) m_XSecSurf.FindXSec( sect_id );
        return ws->m_Dihedral();
    }

    double sum_dihedral = 0.0;
    for ( int i = 1 ; i <= sect_id ; i++ )
    {
        WingSect* ws = ( WingSect* ) m_XSecSurf.FindXSec( i );
        sum_dihedral += ws->m_Dihedral();
    }

    return sum_dihedral;
}

void WingGeom::ReadV2File( xmlNodePtr &root )
{
    int i;
    xmlNodePtr node;

    m_XSecSurf.DeleteAllXSecs();

    //===== Read General Parameters =====//
    node = XmlUtil::GetNode( root, "General_Parms", 0 );
    if ( node )
    {
        Geom::ReadV2File( node );
    }

    double sweep_off = 0.0;

    //===== Read Wing Parameters =====//
    node = XmlUtil::GetNode( root, "Mswing_Parms", 0 );
    if ( node )
    {
// Don't set these.  Let them fall out of the wing buildup.
//        m_TotalArea = XmlUtil::FindDouble( node, "Total_Area", m_TotalArea() );
//        m_TotalSpan = XmlUtil::FindDouble( node, "Total_Span", m_TotalSpan() );
//        m_TotalProjSpan = XmlUtil::FindDouble( node, "Total_Proj_Span", m_TotalProjSpan() );
//        m_TotalChord  = XmlUtil::FindDouble( node, "Avg_Chord", m_TotalChord() );

        sweep_off  = XmlUtil::FindDouble( node, "Sweep_Off", sweep_off );

//        deg_per_seg = XmlUtil::FindInt( node, "Deg_Per_Seg", (int)(deg_per_seg()+0.5) );
//        max_num_segs = XmlUtil::FindInt( node, "Max_Num_Seg", (int)(max_num_segs()+0.5) );

        m_RelativeDihedralFlag = XmlUtil::FindInt( node, "Rel_Dihedral_Flag", m_RelativeDihedralFlag() )!= 0;
        m_RelativeTwistFlag = XmlUtil::FindInt( node, "Rel_Twist_Flag", m_RelativeTwistFlag() )!= 0;

        int round_end_cap_flag = XmlUtil::FindInt( node, "Round_End_Cap_Flag", 0 )!= 0;
        if ( round_end_cap_flag )
        {
            m_CapUMaxOption = ROUND_END_CAP;
        }
    }

    //==== Read Airfoils ====//
    xmlNodePtr af_list_node = XmlUtil::GetNode( root, "Airfoil_List", 0 );
    xmlNodePtr sec_list_node = XmlUtil::GetNode( root, "Section_List", 0 );

    if ( af_list_node && sec_list_node )
    {
        int num_af = XmlUtil::GetNumNames( af_list_node, "Airfoil" );
        int num_sec =  XmlUtil::GetNumNames( sec_list_node, "Section" );

        assert( num_sec + 1 == num_af );


        for ( i = 0 ; i < num_af ; i++ )
        {
            xmlNodePtr af_node = NULL;
            af_node = XmlUtil::GetNode( af_list_node, "Airfoil", i );

            xmlNodePtr sec_node = NULL;
            if ( i > 0 )
            {
                sec_node = XmlUtil::GetNode( sec_list_node, "Section", i - 1 );
            }
            else
            {
                sec_node = XmlUtil::GetNode( sec_list_node, "Section", i );
            }

            if ( af_node )
            {
                int af_type = XmlUtil::FindInt( af_node, "Type", 0 );

                XSec* xsec_ptr = NULL;

                switch ( af_type )
                {
                case V2_NACA_4_SERIES:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_FOUR_SERIES ) );
                    break;
                case V2_BICONVEX:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_BICONVEX ) );
                    break;
                case V2_WEDGE:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_WEDGE ) );
                    break;
                case V2_AIRFOIL_FILE:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_FILE_AIRFOIL ) );
                    break;
                case V2_NACA_6_SERIES:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_SIX_SERIES ) );
                    break;
                }

                if ( xsec_ptr )
                {
                    WingSect* wing_xsec_ptr = dynamic_cast < WingSect* > (xsec_ptr);

                    if ( wing_xsec_ptr )
                    {
                        Airfoil *af_ptr = dynamic_cast< Airfoil* > ( wing_xsec_ptr->GetXSecCurve() );
                        if ( af_ptr )
                        {
                            af_ptr->ReadV2File( af_node );
                        }

                        if ( i > 0 )
                        {
                            wing_xsec_ptr->ReadV2File( sec_node );
                            wing_xsec_ptr->m_Sweep = wing_xsec_ptr->m_Sweep() + sweep_off;
                        }
                        else // Special handling of 'zeroth' wing section.
                        {
                            double rc = XmlUtil::FindDouble( sec_node, "RC", 1.0 );
                            wing_xsec_ptr->m_TipChord = rc;
                        }
                    }
                }
            }
        }
    }
}
