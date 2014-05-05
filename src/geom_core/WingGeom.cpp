//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "WingGeom.h"
#include "ParmMgr.h"
#include "VspSurf.h"
#include "Vehicle.h"
#include "StlHelper.h"


//==== Constructor ====//
WingDriverGroup::WingDriverGroup() : DriverGroup( NUM_WSECT_DRIVER, 3 )
{
    m_CurrChoices[0] = AR_WSECT_DRIVER;
    m_CurrChoices[1] = SPAN_WSECT_DRIVER;
    m_CurrChoices[2] = TAPER_WSECT_DRIVER;
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

    int niter = 0;
    while( vector_contains_val( uptodate, false ) )
    {
        if( !uptodate[AR_WSECT_DRIVER] )
        {
            if( uptodate[SPAN_WSECT_DRIVER] && uptodate[AREA_WSECT_DRIVER] )
            {
                AR->Set( span->Get() * span->Get() / area->Get() );
                uptodate[AR_WSECT_DRIVER] = true;
            }
            else if( uptodate[SPAN_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                AR->Set( span->Get() / aveC->Get() );
                uptodate[AR_WSECT_DRIVER] = true;
            }
            else if( uptodate[AREA_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                AR->Set( area->Get() / ( aveC->Get() * aveC->Get() ) );
                uptodate[AR_WSECT_DRIVER] = true;
            }
        }

        if( !uptodate[SPAN_WSECT_DRIVER] )
        {
            if( uptodate[AVEC_WSECT_DRIVER] && uptodate[AREA_WSECT_DRIVER] )
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
        }


        if( !uptodate[AREA_WSECT_DRIVER] )
        {
            if( uptodate[SPAN_WSECT_DRIVER] && uptodate[AVEC_WSECT_DRIVER] )
            {
                area->Set( span->Get() * aveC->Get() );
                uptodate[AREA_WSECT_DRIVER] = true;
            }
            else if( uptodate[SPAN_WSECT_DRIVER] && uptodate[AR_WSECT_DRIVER] )
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
            else if( uptodate[SPAN_WSECT_DRIVER] && uptodate[AREA_WSECT_DRIVER] )
            {
                aveC->Set( area->Get() / span->Get() );
                uptodate[AVEC_WSECT_DRIVER] = true;
            }
        }

        if( !uptodate[TIPC_WSECT_DRIVER] )
        {
            if( uptodate[TAPER_WSECT_DRIVER] && uptodate[ROOTC_WSECT_DRIVER] )
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
        }

        if( !uptodate[ROOTC_WSECT_DRIVER] )
        {
            if( uptodate[TAPER_WSECT_DRIVER] && uptodate[TIPC_WSECT_DRIVER] )
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
        }

        if( !uptodate[TAPER_WSECT_DRIVER] )
        {
            if( uptodate[TIPC_WSECT_DRIVER] && uptodate[ROOTC_WSECT_DRIVER] )
            {
                taper->Set( tipC->Get() / rootC->Get() );
                uptodate[TAPER_WSECT_DRIVER] = true;
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

    return true;
}



//==== Constructor ====//
WingGeom::WingGeom( Vehicle* vehicle_ptr ) : FuselageGeom( vehicle_ptr )
{
    m_Name = "WingGeom";
    m_Type.m_Name = "Wing";
    m_Type.m_Type = MS_WING_GEOM_TYPE;
}

