//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XSec.cpp: implementation of the XSec class.
//
//////////////////////////////////////////////////////////////////////

#include "Airfoil.h"
#include "Geom.h"
#include "Parm.h"
#include "ParmContainer.h"
#include "VehicleMgr.h"
#include "ParmMgr.h"
#include "StringUtil.h"
#include "APIDefines.h"
#include <float.h>

using std::string;
using namespace vsp;

//==== Default Constructor ====//
Airfoil::Airfoil( ) : XSecCurve( )
{
    m_Invert.Init( "Invert", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_Chord.Init( "Chord", m_GroupName, this, 1.0, 0.0, 1.0e12 );
    m_ThickChord.Init( "ThickChord", m_GroupName, this, 0.1, 0.0, 10 );
}

//==== Update ====//
void Airfoil::Update()
{
    // invert airfoil if needed
    if ( m_Invert() )
    {
        // reflect about xz-plane
        m_Curve.ReflectXZ();

        // reverse parameterization
        m_Curve.Reverse();
    }

    XSecCurve::Update();
}

//==== Get Width ====//
double Airfoil::GetWidth()
{
    return m_Chord();
}

//==== Get Height ====//
double Airfoil::GetHeight()
{
    return m_Chord() * m_ThickChord();
}

//==== Set Width and Height ====//
void Airfoil::SetWidthHeight( double w, double h )
{
    m_Chord = w;
    m_ThickChord = 0.0;

    if ( w > DBL_EPSILON )
    {
        m_ThickChord = h / w;
    }
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
FourSeries::FourSeries( ) : Airfoil( )
{
    m_Type = XS_FOUR_SERIES;
    m_Camber.Init( "Camber", m_GroupName, this, 0.0, 0.0, 0.5 );
    m_CamberLoc.Init( "CamberLoc", m_GroupName, this, 0.2, 0.0, 1.0 );

    m_Creator.set_sharp_trailing_edge(true);
}

//==== Update ====//
void FourSeries::Update()
{
    piecewise_curve_type c;

    m_Creator.set_thickness( m_ThickChord() * 100.0f );
    m_Creator.set_camber( m_Camber() * 100.0f, m_CamberLoc() * 10.0f );

    m_Creator.create( c );

    m_Curve.SetCurve( c );

    Matrix4d mat;
    mat.loadIdentity();
    mat.scale( m_Chord() );

    m_Curve.Transform( mat );

    Airfoil::Update();
}

//===== Load Name And Number of 4 Series =====//
string FourSeries::GetAirfoilName()
{
    int icam     = int( m_Camber() * 100.0f + 0.5f );
    int icam_loc = int( m_CamberLoc() * 10.0f + 0.5f );
    int ithick   = int( m_ThickChord() * 100.0f + 0.5f );

    if ( icam == 0 )
    {
        icam_loc = 0;
    }

    char str[255];
    if ( ithick < 10 )
    {
        sprintf( str, "  NACA %d%d0%d", icam, icam_loc, ithick );
    }
    else
    {
        sprintf( str, "  NACA %d%d%d", icam, icam_loc, ithick );
    }

    return string( str );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
SixSeries::SixSeries( ) : Airfoil( )
{
    m_Type = XS_SIX_SERIES;

    m_Series.Init( "Series", m_GroupName, this, 0, 0, NUM_SERIES );
    m_IdealCl.Init( "IdealCl", m_GroupName, this, 0.0, 0.0, 1.0 );
    m_A.Init( "A", m_GroupName, this, 0.0, 0.0, 1.0 );

}

//==== Update ====//
void SixSeries::Update()
{
#if 0
    //==== Run Six Series Fortran Generation Code ====//
    float cli = ( float )m_IdealCl();
    float ta = ( float )m_A();
    float toc = ( float )m_ThickChord();
    int sixser = 63;
    if ( m_Series() <= SERIES_67 )
    {
        sixser = m_Series() + 63;
    }
    else if ( m_Series() < NUM_SERIES )
    {
        sixser = -( m_Series() + 58 );
    }

    //==== Generate Airfoil ====//
    sixseries_( &sixser, &toc, &cli, &ta );

    int num_pnts_upper = sixpnts_.nmu;
    int num_pnts_lower = sixpnts_.nml;

    //==== Load Points ====//
    vector< vec3d > pnts;
    for ( int i = num_pnts_upper - 1 ; i >= 0 ; i-- )
    {
        pnts.push_back( vec3d( 0.0, sixpnts_.yyu[i], sixpnts_.xxu[i] ) );
    }
    for ( int i = 1 ; i < num_pnts_lower ; i++ )
    {
        pnts.push_back( vec3d( 0.0, sixpnts_.yyl[i], sixpnts_.xxl[i] ) );
    }

    pnts = ScaleCheckInvert( pnts );
    m_Curve.Interpolate( pnts, true );
    m_Curve.UniformInterpolate( m_NumBasePnts, true );

    Airfoil::Update();
#endif
}

//===== Load Name And Number of 4 Series =====//
string SixSeries::GetAirfoilName()
{
    int ithick   = int( m_ThickChord() * 100.0 + 0.5f );
    int icl = int( m_IdealCl() * 10.0 + 0.5f );
    int ia = int( m_A() * 10.0 + 0.5f );

    int sixser = 63;
    if ( m_Series() < SERIES_67 )
    {
        sixser = m_Series() + 63;
    }
    else if ( m_Series() < NUM_SERIES )
    {
        sixser = -( m_Series() + 58 );
    }

    char str[255];
    if ( sixser > 0 )
    {
        if ( ithick < 10 )
        {
            sprintf( str, "  NACA %d%d0%d  a=0.%d", sixser, icl, ithick, ia );
        }
        else
        {
            sprintf( str, "  NACA %d%d%d  a=0.%d",  sixser, icl, ithick, ia );
        }
    }
    else
    {
        if ( ithick < 10 )
        {
            sprintf( str, "  NACA %dA%d0%d  a=0.%d", -sixser, icl, ithick, ia );
        }
        else
        {
            sprintf( str, "  NACA %dA%d%d  a=0.%d",  -sixser, icl, ithick, ia );
        }
    }
    return string( str );
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
Biconvex::Biconvex( ) : Airfoil( )
{
    m_Type = XS_BICONVEX;
}

//==== Update ====//
void Biconvex::Update()
{
#if 0
    double x, xu, zu;
    //==== Initialize Array For Points ====//
    vector< vec3d > pnts( m_NumBasePnts );
    int half_pnts = m_NumBasePnts / 2;

    //==== Generate Airfoil ====//
    for ( int i = 1 ; i < half_pnts ; i++ )
    {
        //==== More Points At Leading Edge
        x = ( double )i / ( double )half_pnts;
        x = x * sqrt( x );

        //==== Compute Upper Surface Points ====//
        xu = x;
        zu = 2.0 * m_ThickChord() * x * ( 1.0 - x );
        pnts[half_pnts - i] = vec3d( 0.0, zu, xu );

        //==== Compute Lower Surface Points ====//
        pnts[half_pnts + i] = vec3d( 0.0, -zu, xu );
    }

    pnts = ScaleCheckInvert( pnts );
    m_Curve.Interpolate( pnts, true );
    m_Curve.UniformInterpolate( m_NumBasePnts, true );

    Airfoil::Update();
#endif
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
Wedge::Wedge( ) : Airfoil( )
{
    m_Type = XS_WEDGE;
    m_ThickLoc.Init( "ThickLoc", m_GroupName, this, 0.5, 0.0, 1.0 );

}

//==== Update ====//
void Wedge::Update()
{
    vector<vec3d> pt( 4 );
    vector<double> u( 5 );
    double x_apex( m_ThickLoc()*m_Chord() ), y_apex( m_ThickChord()*m_Chord() / 2 );

    pt[0].set_xyz( m_Chord(), 0, 0 );
    pt[1].set_xyz( x_apex, -y_apex, 0 );
    pt[2].set_xyz( 0, 0, 0 );
    pt[3].set_xyz( x_apex, y_apex, 0 );

    u[0] = 0;
    u[1] = 1;
    u[2] = 2;
    u[3] = 3;
    u[4] = 4;

    // build the wedge
    m_Curve.InterpolateLinear( pt, u, true );

    Airfoil::Update();
}

//==========================================================================//
//==========================================================================//
//==========================================================================//

//==== Constructor ====//
FileAirfoil::FileAirfoil( ) : Airfoil( )
{
#if 0
    m_NumBasePnts = 21;
    m_Type = XS_FILE_AIRFOIL;
    m_UpperPnts.resize( m_NumBasePnts, vec3d( 0, 0, 0 ) );
    m_LowerPnts.resize( m_NumBasePnts, vec3d( 0, 0, 0 ) );
#endif
}

//==== Update ====//
void FileAirfoil::Update()
{
#if 0
    //==== Load Points ====//
    vector< vec3d > pnts;
    for ( int i = ( int )m_UpperPnts.size() - 1 ; i >= 0 ; i-- )
    {
        pnts.push_back( m_UpperPnts[i] );
    }
    for ( int i = 1 ; i < ( int )m_LowerPnts.size() ; i++ )
    {
        pnts.push_back( m_LowerPnts[i] );
    }

    pnts = ScaleCheckInvert( pnts );
    m_Curve.Interpolate( pnts, true );
    m_Curve.UniformInterpolate( m_NumBasePnts, true );

    Airfoil::Update();
#endif
}

//==== Encode XML ====//
xmlNodePtr FileAirfoil::EncodeXml( xmlNodePtr & node )
{
    Airfoil::EncodeXml( node );

    xmlNodePtr child_node = xmlNewChild( node, NULL, BAD_CAST "FileAirfoil", NULL );
    if ( child_node )
    {
        XmlUtil::AddStringNode( child_node, "AirfoilName", m_AirfoilName );

        //==== Encode Pnts ====//
        XmlUtil::AddVectorVec3dNode( child_node, "UpperPnts", m_UpperPnts );
        XmlUtil::AddVectorVec3dNode( child_node, "LowerPnts", m_LowerPnts );
    }
    return child_node;
}

//==== Decode XML ====//
xmlNodePtr FileAirfoil::DecodeXml( xmlNodePtr & node )
{
    Airfoil::DecodeXml( node );

    xmlNodePtr child_node = XmlUtil::GetNode( node, "FileAirfoil", 0 );
    if ( child_node )
    {
        m_AirfoilName = XmlUtil::FindString( child_node, "AirfoilName", m_AirfoilName );
        m_UpperPnts = XmlUtil::ExtractVectorVec3dNode( child_node, "UpperPnts" );
        m_LowerPnts = XmlUtil::ExtractVectorVec3dNode( child_node, "LowerPnts" );
    }
    return child_node;
}

////==== Set GroupName ====//
//void FileAirfoil::SetGroupName( string group_name )
//{
//
//}

//==== Read Airfoil File ====//
bool FileAirfoil::ReadFile( string file_name )
{
    //==== Open File ====//
    FILE* file_id =  fopen( file_name.c_str(), "r" );
    if ( file_id == ( FILE * )NULL )
    {
        return false;
    }

    char buff[256];
    string line;

    fgets( buff, 255, file_id );
    line.assign( buff );

    size_t found = line.find( "AIRFOIL FILE" );

    bool valid_file = false;
    if ( found == string::npos )
    {
        valid_file = ReadSeligAirfoil( file_id );           // Check If Selig Format

        if ( !valid_file )
        {
            valid_file = ReadLednicerAirfoil( file_id );    // Check If Lednicer Format
        }
    }
    else
    {
        fgets( buff, 255, file_id );                        // Name
        valid_file = ReadVspAirfoil( file_id );             // VSP Format
    }

    if ( !valid_file )
    {
        fclose( file_id );
        return false;
    }

    //==== Extract Name ====//
    m_AirfoilName.assign( buff );
    StringUtil::change_from_to( m_AirfoilName, '\n', ' ' );
    StringUtil::change_from_to( m_AirfoilName, '\r', ' ' );
    StringUtil::remove_trailing( m_AirfoilName, ' ' );

    fclose( file_id );

    return valid_file;

}


//==== Read Selig Airfoil File ====//
bool FileAirfoil::ReadSeligAirfoil( FILE* file_id )
{
    int i;
    char buff[256];
    float x, y;

    vector< float > xvec;
    vector< float > yvec;

    int more_data_flag = 1;
    while ( more_data_flag )
    {
        more_data_flag = 0;
        if ( fgets( buff, 255, file_id ) )
        {
            more_data_flag = 1;
        }

        if ( more_data_flag )
        {
            x = y = 100000.0;
            sscanf( buff, "%f %f", &x, &y );
            if ( x >= 0.0 && x <= 1.0 && y >= -1.0 && y <= 1.0 )
            {
                xvec.push_back( x );
                yvec.push_back( y );
            }
            else
            {
                more_data_flag = 0;
            }
        }
    }

    int totalPnts = xvec.size();

    if ( totalPnts < 5 )
    {
        return false;
    }

    //==== Find Leading Edge Index ====//
    int leInd = 0;
    float smallestX = 1.0e06;
    for ( i = 0 ; i < totalPnts ; i++ )
    {
        if ( xvec[i] < smallestX )
        {
            smallestX = xvec[i];
            leInd = i;
        }
    }

    //==== Not Enough Pnts ====//
    int num_pnts_lower = totalPnts - leInd;
    int num_pnts_upper = leInd + 1;

    if ( num_pnts_lower < 3 || num_pnts_upper < 3 )
    {
        return false;
    }

    m_UpperPnts.clear();
    m_LowerPnts.clear();

    //==== Load Em Up ====//
    for ( i = leInd ; i >= 0 ; i-- )
    {
        m_UpperPnts.push_back( vec3d( xvec[i], yvec[i], 0.0 ) );
    }

    for ( i = leInd ; i < totalPnts ; i++ )
    {
        m_LowerPnts.push_back( vec3d( xvec[i], yvec[i], 0.0 ) );
    }
    //==== Close Trailing Edge - Set Last Points ====//
    vec3d last_pnt = m_UpperPnts.back() + m_LowerPnts.back();
    m_UpperPnts.back() = last_pnt * 0.5;
    m_LowerPnts.back() = last_pnt * 0.5;

    return true;
}

//==== Read Lednicer Airfoil File ====//
bool FileAirfoil::ReadLednicerAirfoil( FILE* file_id )
{
    char buff[256];
    float x, y;

    rewind( file_id );

    fgets( buff, 255, file_id );
    fgets( buff, 255, file_id );
    sscanf( buff, "%f %f", &x, &y );
    int num_pnts_upper = ( int )( x + 0.5 );
    int num_pnts_lower = ( int )( y + 0.5 );

    if ( num_pnts_upper < 3 || num_pnts_lower < 3 )
    {
        return false;
    }

    m_UpperPnts.clear();
    m_LowerPnts.clear();

    fgets( buff, 255, file_id );
    for ( int i = 0 ; i < num_pnts_upper ; i++ )
    {
        fgets( buff, 255, file_id );
        sscanf( buff, "%f %f", &x, &y );
        m_UpperPnts.push_back( vec3d( x, y, 0.0 ) );
    }
    fgets( buff, 255, file_id );
    for ( int i = 0 ; i < num_pnts_lower ; i++ )
    {
        fgets( buff, 255, file_id );
        sscanf( buff, "%f %f", &x, &y );
        m_LowerPnts.push_back( vec3d( x, y, 0.0 ) );
    }

    //==== Close Trailing Edge - Set Last Points ====//
    vec3d last_pnt = m_UpperPnts.back() + m_LowerPnts.back();
    m_UpperPnts.back() = last_pnt * 0.5;
    m_LowerPnts.back() = last_pnt * 0.5;

    return true;
}

//==== Read VSP Airfoil File ====//
bool FileAirfoil::ReadVspAirfoil( FILE* file_id )
{
    int i;
    char buff[256];

    int sym_flag;
    int num_pnts_upper, num_pnts_lower;
    float x, y;

    fgets( buff, 255, file_id );
    sscanf( buff, "%d", &sym_flag );

    fgets( buff, 255, file_id );
    sscanf( buff, "%d", &num_pnts_upper );

    if ( sym_flag )
    {
        num_pnts_lower = num_pnts_upper;
    }
    else
    {
        fgets( buff, 255, file_id );
        sscanf( buff, "%d", &num_pnts_lower );
    }

    m_UpperPnts.clear();
    m_LowerPnts.clear();
    for ( i = 0 ; i < num_pnts_upper ; i++ )
    {
        fgets( buff, 255, file_id );
        sscanf( buff, "%f %f", &x, &y );

        m_UpperPnts.push_back( vec3d( x, y, 0.0 ) );

        if ( sym_flag )
        {
            m_LowerPnts.push_back( vec3d( x, -y, 0.0 ) );
        }
    }
    fgets( buff, 255, file_id );

    if ( !sym_flag )
    {
        for ( i = 0 ; i < num_pnts_lower ; i++ )
        {
            fgets( buff, 255, file_id );
            sscanf( buff, "%f %f", &x, &y );
            m_LowerPnts.push_back( vec3d( x, y, 0.0 ) );
        }
    }

    //==== Close Trailing Edge - Set Last Points ====//
    vec3d last_pnt = m_UpperPnts.back() + m_LowerPnts.back();
    m_UpperPnts.back() = last_pnt * 0.5;
    m_LowerPnts.back() = last_pnt * 0.5;

    return true;
}

