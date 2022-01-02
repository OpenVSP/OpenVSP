//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SupperEllipse.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////


#include "SuperEllipse.h"

using std::vector;

//====================== Constructor ======================//
SuperEllipse::SuperEllipse()
{

}

SuperEllipse::~SuperEllipse()
{

}

//==== Build Super Ellipse ====//
vector< vec3d > SuperEllipse::Build( int num_pnts, double a, double b )
{
    if ( num_pnts < 3 )
    {
        num_pnts = 3;
    }

    BuildCosSinTables( num_pnts );

    double z, y;
    vector< vec3d > ret_pnts( num_pnts );
    for ( int i = 0 ; i < num_pnts - 1 ; i++ )
    {
        z = m_CosU[i] * a;
        y = m_SinU[i] * b;
        ret_pnts[i].set_xyz( 0.0, y, z );
    }
    ret_pnts.back() = ret_pnts[0];

    return ret_pnts;
}


//==== Build Super Ellipse
vector< vec3d > SuperEllipse::Build( int num_pnts, double a, double b, double m, double n )
{
    if ( num_pnts < 3 )
    {
        num_pnts = 3;
    }

    BuildCosSinTables( num_pnts );

    double z, y, a_sign, b_sign;
    vector< vec3d > ret_pnts( num_pnts );
    for ( int i = 0 ; i < num_pnts - 1 ; i++ )
    {
        a_sign = a;
        b_sign = b;
        if ( m_CosU[i] < 0 )
        {
            a_sign = -a;
        }
        if ( m_SinU[i] < 0 )
        {
            b_sign = -b;
        }

        z = pow( std::abs( m_CosU[i] ), 1.0 / m ) * a_sign;
        y = pow( std::abs( m_SinU[i] ), 1.0 / n ) * b_sign;
        ret_pnts[i].set_xyz( 0.0, y, z );
    }
    ret_pnts.back() = ret_pnts[0];

    return ret_pnts;
}

void SuperEllipse::BuildCosSinTables( int num_pnts )
{
    if ( num_pnts != ( int )m_CosU.size() || num_pnts != ( int )m_SinU.size() )
    {
        m_CosU.resize( num_pnts );
        m_SinU.resize( num_pnts );
        for ( int i = 0 ; i < num_pnts ; i++ )
        {
            double u = ( double )i / ( double )( num_pnts - 1 );
            m_CosU[i] = cos( u * 2.0 * PI );
            m_SinU[i] = sin( u * 2.0 * PI );
        }
    }
}
