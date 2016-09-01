//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.cpp: implementation of the Vehicle Class and Vehicle Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "Vehicle.h"
#include "VehicleMgr.h"
#include "ResultsMgr.h"
#include "APIDefines.h"
#include "Util.h"
#include "StlHelper.h"
#include <ctime>

#ifdef WIN32
#include <windows.h>
#endif


//==== Default Results Data ====//
NameValData::NameValData()
{
    Init( "Undefined" );
}

//==== Construtor With Name =====//
NameValData::NameValData( const string & name )
{
    Init( name );
}

//==== Construtors With Name & Data =====//
NameValData::NameValData( const string & name, int i_data )
{
    Init( name, vsp::INT_DATA );
    m_IntData.push_back( i_data );
}
NameValData::NameValData( const string & name, double d_data )
{
    Init( name, vsp::DOUBLE_DATA );
    m_DoubleData.push_back( d_data );
}
NameValData::NameValData( const string & name, const string & s_data )
{
    Init( name, vsp::STRING_DATA );
    m_StringData.push_back( s_data );
}
NameValData::NameValData( const string & name, const vec3d & v_data )
{
    Init( name, vsp::VEC3D_DATA );
    m_Vec3dData.push_back( v_data );
}
NameValData::NameValData( const string & name, vector< int > & i_data )
{
    Init( name, vsp::INT_DATA );
    m_IntData = i_data;
}
NameValData::NameValData( const string & name, vector< double > & d_data )
{
    Init( name, vsp::DOUBLE_DATA );
    m_DoubleData = d_data;
}
NameValData::NameValData( const string & name, vector< string > & s_data )
{
    Init( name, vsp::STRING_DATA );
    m_StringData = s_data;
}
NameValData::NameValData( const string & name, vector< vec3d > & v_data )
{
    Init( name, vsp::VEC3D_DATA );
    m_Vec3dData = v_data;
}

void NameValData::Init( const string & name, int type, int index )
{
    m_Name = name;
    m_Type = type;
}

int NameValData::GetInt( int i ) const
{
    if ( i < ( int )m_IntData.size() )
    {
        return m_IntData[i];
    }
    return 0;
}
double NameValData::GetDouble( int i ) const
{
    if ( i < ( int )m_DoubleData.size() )
    {
        return m_DoubleData[i];
    }
    return 0;
}
string NameValData::GetString( int i ) const
{
    if ( i < ( int )m_StringData.size() )
    {
        return m_StringData[i];
    }
    return string();
}

vec3d NameValData::GetVec3d( int i ) const
{
    if ( i < ( int )m_Vec3dData.size() )
    {
        return m_Vec3dData[i];
    }
    return vec3d();
}


//======================================================================================//
//======================================================================================//
//======================================================================================//


NameValCollection::NameValCollection( const string & name, const string & id )
{
    m_Name = name;
    m_ID = id;
}

//==== Add Data To Results - Can Have Data With The Same Name =====//
void NameValCollection::Add( const NameValData & d )
{
    //==== Find Name ====//
    string name = d.GetName();

    map< string, vector< NameValData > >::iterator iter = m_DataMap.find( name );
    if ( iter !=  m_DataMap.end() )     // Check For Duplicates
    {
        iter->second.push_back( d );
    }
    else
    {
        m_DataMap[name].push_back( d );
    }
}

//==== Get Number of Data Entries For This Name ====//
int NameValCollection::GetNumData( const string & name )
{
    map< string, vector< NameValData > >::iterator iter = m_DataMap.find( name );
    if ( iter ==  m_DataMap.end() )
    {
        return 0;
    }

    return iter->second.size();
}

//==== Get Data Names ====//
vector< string > NameValCollection::GetAllDataNames()
{
    vector< string > name_vec;
    map< string, vector< NameValData > >::iterator iter;

    for ( iter = m_DataMap.begin() ; iter != m_DataMap.end() ; iter++ )
    {
        name_vec.push_back( iter->first );
    }
    return name_vec;
}

//==== Find Res Data Given Name and Index ====//
NameValData NameValCollection::Find( const string & name, int index )
{
    map< string, vector< NameValData > >::iterator iter = m_DataMap.find( name );

    if ( iter !=  m_DataMap.end() )
    {
        if ( index >= 0 && index < ( int )( iter->second.size() ) )
        {
            return iter->second[index];
        }
    }
    return NameValData();
}

//==== Find Res Data Given Name and Index ====//
NameValData* NameValCollection::FindPtr( const string & name, int index )
{
    map< string, vector< NameValData > >::iterator iter = m_DataMap.find( name );

    if ( iter !=  m_DataMap.end() )
    {
        if ( index >= 0 && index < ( int )( iter->second.size() ) )
        {
            return &( iter->second[index] );
        }
    }
    return NULL;
}


//======================================================================================//
//======================================================================================//
//======================================================================================//

Results::Results( const string & name, const string & id ) : NameValCollection( name, id )
{
    SetDateTime();          // Set Time Stamp
}

//===== Find Current Time and Set Stamp =====//
void Results::SetDateTime()
{
    m_Timestamp = time( 0 ); // get time now
    struct tm * now = localtime( &m_Timestamp );

    m_Year = now->tm_year + 1900;
    m_Month = now->tm_mon + 1;
    m_Day = now->tm_mday;
    m_Hour = now->tm_hour;
    m_Min = now->tm_min;
    m_Sec = now->tm_sec;
}

//===== Write A CSV File With Everything =====//
void Results::WriteCSVFile( const string & file_name )
{
    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        WriteCSVFile( fid );
        fclose( fid );          // Close File
    }
}

void Results::WriteCSVFile( FILE* fid )
{
    if ( fid )
    {

        fprintf( fid, "Results_Name,%s\n", m_Name.c_str() );
        fprintf( fid, "Results_Timestamp,%ld\n", m_Timestamp );
        fprintf( fid, "Results_Date,%d,%d,%d\n", m_Month, m_Day, m_Year );
        fprintf( fid, "Results_Time,%d,%d,%d\n", m_Hour, m_Min, m_Sec );

        map< string, vector< NameValData > >::iterator iter;
        for ( iter = m_DataMap.begin() ; iter != m_DataMap.end() ; iter++ )
        {
            for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
            {
                fprintf( fid, "%s", iter->second[i].GetName().c_str() );
                if ( iter->second[i].GetType() == vsp::INT_DATA )
                {
                    for ( int d = 0 ; d < ( int )iter->second[i].GetIntData().size() ; d++ )
                    {
                        fprintf( fid, ",%d", iter->second[i].GetIntData()[d] );
                    }
                }
                else if ( iter->second[i].GetType() == vsp::DOUBLE_DATA )
                {
                    for ( int d = 0 ; d < ( int )iter->second[i].GetDoubleData().size() ; d++ )
                    {
                        fprintf( fid, ",%lf", iter->second[i].GetDoubleData()[d] );
                    }
                }
                else if ( iter->second[i].GetType() == vsp::STRING_DATA )
                {
                    for ( int d = 0 ; d < ( int )iter->second[i].GetStringData().size() ; d++ )
                    {
                        fprintf( fid, ",%s", iter->second[i].GetStringData()[d].c_str() );
                    }
                }
                else if ( iter->second[i].GetType() == vsp::VEC3D_DATA )
                {
                    for ( int d = 0 ; d < ( int )iter->second[i].GetVec3dData().size() ; d++ )
                    {
                        vec3d v = iter->second[i].GetVec3dData()[d];
                        fprintf( fid, ",%lf,%lf,%lf", v.x(), v.y(), v.z() );
                    }
                }
                fprintf( fid, "\n" );
            }
        }
    }
}

//==== Write The Mass Prop Results ====//
void Results::WriteMassProp( const string & file_name )
{
    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        int num_removed =  Find( "Num_Degen_Triangles_Removed" ).GetInt( 0 );
        if ( num_removed != 0 )
        {
            fprintf( fid, "WARNING: %d degenerate triangle removed\n", num_removed );
        }

        int num_open_remove = Find( "Num_Open_Meshes_Removed" ).GetInt( 0 );
        if ( num_open_remove != 0 )
        {
            fprintf( fid, "WARNING: %d open meshes remove\n", num_open_remove );
        }

        int num_open_merge = Find( "Num_Open_Meshes_Merged" ).GetInt( 0 );
        if ( num_open_merge != 0 )
        {
            fprintf( fid, "WARNING: %d open meshes remove\n", num_open_merge );
        }

        fprintf( fid, "...Mass Properties...\n" );
        int num_comps = Find( "Num_Comps" ).GetInt( 0 );
        fprintf( fid, "%d Num Comps\n", num_comps );
        fprintf( fid, "%d Total Num Meshes\n", Find( "Num_Total_Meshes" ).GetInt( 0 ) );
        fprintf( fid, "%d Total Num Tris\n", Find( "Num_Total_Tris" ).GetInt( 0 ) );
        fprintf( fid, "\n" );
        fprintf( fid, "%f             Total Mass\n", Find( "Total_Mass" ).GetDouble( 0 ) );
        vec3d total_cg = Find( "Total_CG" ).GetVec3d( 0 );
        fprintf( fid, "%f %f %f       Center of Gravity\n", total_cg.x(), total_cg.y(), total_cg.z() );
        double ixx =  Find( "Total_Ixx" ).GetDouble( 0 );
        double iyy =  Find( "Total_Iyy" ).GetDouble( 0 );
        double izz =  Find( "Total_Izz" ).GetDouble( 0 );
        fprintf( fid, "%f %f %f       Ixx, Iyy, Izz\n", ixx, iyy, izz );
        double ixy =  Find( "Total_Ixy" ).GetDouble( 0 );
        double ixz =  Find( "Total_Ixz" ).GetDouble( 0 );
        double iyz =  Find( "Total_Iyz" ).GetDouble( 0 );
        fprintf( fid, "%f %f %f       Ixy, Ixz, Iyz\n", ixy, ixz, iyz );
        fprintf( fid, "%f             Volume\n", Find( "Total_Volume" ).GetDouble( 0 ) );

        fprintf( fid, "\n" );
        fprintf( fid, "Name\tMass\tcgX\tcgY\tcgZ\tIxx\tIyy\tIzz\tIxy\tIxz\tIyz\tVolume\n" );

        for ( int i = 0 ; i < num_comps ; i++ )
        {
            string comp_name = Find( "Comp_Name" ).GetString( i );
            double comp_mass = Find( "Comp_Mass" ).GetDouble( i );
            vec3d comp_cg = Find( "Comp_CG" ).GetVec3d( i );
            double compIxx = Find( "Comp_Ixx" ).GetDouble( i );
            double compIyy = Find( "Comp_Iyy" ).GetDouble( i );
            double compIzz = Find( "Comp_Izz" ).GetDouble( i );
            double compIxy = Find( "Comp_Ixy" ).GetDouble( i );
            double compIyz = Find( "Comp_Iyz" ).GetDouble( i );
            double compIxz = Find( "Comp_Ixz" ).GetDouble( i );
            double compVol = Find( "Comp_Vol" ).GetDouble( i );

            fprintf( fid, "%s\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
                     comp_name.c_str(), comp_mass, comp_cg.x(), comp_cg.y(), comp_cg.z(),
                     compIxx, compIyy, compIzz, compIxy, compIxz, compIyz, compVol );
        }


        fprintf( fid, "Name\tMass\tcgX\tcgY\tcgZ\tIxx\tIyy\tIzz\tIxy\tIxz\tIyz\tVolume\n" );
        fprintf( fid, "%s\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
                 "Totals", Find( "Total_Mass" ).GetDouble( 0 ), total_cg.x(), total_cg.y(), total_cg.z(),
                 ixx, iyy, izz, ixy, ixz, iyz, Find( "Total_Volume" ).GetDouble( 0 ) );

        fclose( fid );          // Close File
    }
}

//==== Write the CompGeom Txt File =====//
void Results::WriteCompGeomTxtFile( const string & file_name )
{
    FILE* fid = fopen( file_name.c_str(), "w" );

    if ( fid )
    {
        fprintf( fid, "...Comp Geom...\n" );
        fprintf( fid, "%d Num Comps\n",        Find( "Num_Comps" ).GetInt( 0 ) );
        fprintf( fid, "%d Total Num Meshes\n", Find( "Total_Num_Meshes" ).GetInt( 0 ) );
        fprintf( fid, "%d Total Num Tris\n",   Find( "Total_Num_Tris" ).GetInt( 0 ) );
        fprintf( fid, "\n" );
        fprintf( fid, "Theo_Area   Wet_Area   Theo_Vol    Wet_Vol  Name\n" );

        int num_mesh = Find( "Num_Meshes" ).GetInt( 0 );
        for ( int i = 0 ; i < num_mesh ; i++ )
        {
            fprintf( fid, "%9.3f  %9.3f  %9.3f  %9.3f  %-15s\n",
                     Find( "Theo_Area" ).GetDouble( i ), Find( "Wet_Area" ).GetDouble( i ),
                     Find( "Theo_Vol" ).GetDouble( i ),  Find( "Wet_Vol" ).GetDouble( i ),
                     Find( "Comp_Name" ).GetString( i ).c_str() );
        }

        fprintf( fid, "-------------------------------------------------\n" );
        fprintf( fid, "%9.3f  %9.3f  %9.3f  %9.3f  %-15s\n",
                 Find( "Total_Theo_Area" ).GetDouble( 0 ),   Find( "Total_Wet_Area" ).GetDouble( 0 ),
                 Find( "Total_Theo_Vol" ).GetDouble( 0 ),    Find( "Total_Wet_Vol" ).GetDouble( 0 ), "Totals" );


        if ( Find( "Num_Degen_Tris_Removed" ).GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d degenerate triangle removed\n", Find( "Num_Degen_Tris_Removed" ).GetInt( 0 ) );
        }
        if ( Find( "Num_Open_Meshes_Removed" ).GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d open meshes removed\n", Find( "Num_Open_Meshes_Removed" ).GetInt( 0 ) );
        }
        if ( Find( "Num_Open_Meshes_Merged" ).GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d open meshes merged\n", Find( "Num_Open_Meshes_Merged" ).GetInt( 0 ) );
        }

        fclose( fid );      // Close File
    }
}

//==== Write The CompGeom CSV File ====//
void Results::WriteCompGeomCsvFile( const string & file_name )
{
    FILE* fid = fopen( file_name.c_str(), "w" );

    if ( fid )
    {
        fprintf( fid, "Name, Theo_Area, Wet_Area, Theo_Vol, Wet_Vol\n" );
        int num_mesh = Find( "Num_Meshes" ).GetInt( 0 );
        for ( int i = 0 ; i < num_mesh ; i++ )
        {
            fprintf( fid, "%s,%f,%f,%f,%f\n",
                     Find( "Comp_Name" ).GetString( i ).c_str(),
                     Find( "Theo_Area" ).GetDouble( i ), Find( "Wet_Area" ).GetDouble( i ),
                     Find( "Theo_Vol" ).GetDouble( i ),  Find( "Wet_Vol" ).GetDouble( i ) );
        }
        fprintf( fid, "%s,%f,%f,%f,%f\n", "Totals",
                 Find( "Total_Theo_Area" ).GetDouble( 0 ),   Find( "Total_Wet_Area" ).GetDouble( 0 ),
                 Find( "Total_Theo_Vol" ).GetDouble( 0 ),    Find( "Total_Wet_Vol" ).GetDouble( 0 ) );

        fclose( fid );
    }
}

//==== Write The CompGeom Drag BuildUp File ====//
void Results::WriteDragBuildFile( const string & file_name )
{

    FILE* fid = fopen( file_name.c_str(), "w" );

    if ( fid )
    {
        fprintf( fid, "Name\tTheo_Area\tWet_Area\tTheo_Vol\tWet_Vol\tMin_Chord\tAve_Chord\tMax_Chord\t" );
        fprintf( fid, "Min_TC_Ratio\tAvg_TC_Ratio\tMax_TC_Ratio\tAve_Sweep\tLength\tMax_Xsec_Area\tLen_Dia_Ratio\n" );

        int num_mesh = Find( "Num_Meshes" ).GetInt( 0 );
        for ( int i = 0 ; i < num_mesh ; i++ )
        {
            fprintf( fid, "%s\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
                     Find( "Comp_Name" ).GetString( i ).c_str(),
                     Find( "Theo_Area" ).GetDouble( i ), Find( "Wet_Area" ).GetDouble( i ),
                     Find( "Theo_Vol" ).GetDouble( i ),  Find( "Wet_Vol" ).GetDouble( i ),
                     Find( "Min_Chord" ).GetDouble( i ), Find( "Avg_Chord" ).GetDouble( i ), Find( "Max_Chord" ).GetDouble( i ),
                     Find( "Min_TC" ).GetDouble( i ),    Find( "Avg_TC" ).GetDouble( i ),    Find( "Max_TC" ).GetDouble( i ),
                     Find( "Avg_Sweep" ).GetDouble( i ),
                     Find( "Length" ).GetDouble( i ),
                     Find( "Max_Area" ).GetDouble( i ),
                     Find( "Length_Dia" ).GetDouble( i ) );
        }

        fclose( fid );
    }
}

void Results::WriteSliceFile( const string & file_name )
{
    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        if ( Find( "Num_Degen_Tris_Removed" ).GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d degenerate triangle removed\n", Find( "Num_Degen_Tris_Removed" ).GetInt( 0 ) );
        }
        if ( Find( "Num_Open_Meshes_Removed" ).GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d open meshes removed\n", Find( "Num_Open_Meshes_Removed" ).GetInt( 0 ) );
        }
        if ( Find( "Num_Open_Meshes_Merged" ).GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d open meshes merged\n", Find( "Num_Open_Meshes_Merged" ).GetInt( 0 ) );
        }

        fprintf( fid, "...Slice...\n" );
        fprintf( fid, "%d Num Comps\n", Find( "Num_Comps" ).GetInt( 0 ) );
        fprintf( fid, "%d Total Num Meshes\n", Find( "Num_Meshes" ).GetInt( 0 ) );
        fprintf( fid, "%d Total Num Tris\n", Find( "Num_Tris" ).GetInt( 0 ) );
        vec3d norm_axis = Find( "Axis_Vector" ).GetVec3d( 0 );
        fprintf( fid, "%1.5f %1.5f %1.5f Axis Vector\n", norm_axis.x(), norm_axis.y(), norm_axis.z() );

        fprintf( fid, "\n" );
        fprintf( fid, "    Loc    XCenter  YCenter  ZCenter         Area\n" );
        for ( int s = 0 ; s <  Find( "Num_Slices" ).GetInt( 0 ) ; s++ )
        {
            vec3d area_center = Find( "Slice_Area_Center" ).GetVec3d( s );
            fprintf( fid, "%9.3f %9.3f %9.3f %9.3f %9.3f\n", Find( "Slice_Loc" ).GetDouble( s ), area_center[0],
                    area_center[1], area_center[2], Find( "Slice_Area" ).GetDouble( s ) );
        }

        fclose( fid );
    }

}

void Results::WriteWaveDragFile( const string & file_name )
{
    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        fprintf( fid, "...Wave Drag Slice...\n" );

        fprintf( fid, "Inlet Area: %f\n", Find( "Inlet_Area" ).GetDouble( 0 ) );
        fprintf( fid, "Exit Area: %f\n", Find( "Exit_Area" ).GetDouble( 0 ) );

        int num_cone_sections = Find( "Num_Cone_Sections" ).GetInt( 0 );
        int num_slices = Find( "Num_Slices" ).GetInt( 0 );

        fprintf( fid, "\n" );
        for ( int i = 0; i < num_cone_sections ; i++ )
        {
            fprintf( fid, "Theta: %6.2f, Start: %6.2f, End: %6.2f\n", Find( "Theta" ).GetDouble( i ), Find( "Start_X" ).GetDouble( i ), Find( "End_X" ).GetDouble( i ) );

            for ( int s = 0 ; s < num_slices ; s++ )
            {
                fprintf( fid, "%19.8f, ", Find( "X_Norm" ).GetDouble( s ) );
                fprintf( fid, "%19.8f", Find( "Slice_Area", i ).GetDouble( s ) );
                fprintf( fid, "\n" );
            }
            fprintf( fid, "\n" );
        }

        double CD0w = Find( "CDWave" ).GetDouble( 0 );
        fprintf( fid, "CDWave: %19.8f \n", CD0w );

        fclose( fid );
    }

}

void Results::WriteBEMFile( const string & file_name )
{
    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        fprintf( fid, "...BEM Propeller...\n" );

        int num_sect = Find( "Num_Sections" ).GetInt( 0 );
        int num_blade = Find( "Num_Blade" ).GetInt( 0 );
        double diam = Find( "Diameter" ).GetDouble( 0 );
        double beta34 = Find( "Beta34" ).GetDouble( 0 );
        double feather = Find( "Feather" ).GetDouble( 0 );
        vec3d cen = Find( "Center" ).GetVec3d( 0 );
        vec3d norm = Find( "Normal" ).GetVec3d( 0 );

        fprintf( fid, "Num_Sections: %d\n", num_sect );
        fprintf( fid, "Num_Blade: %d\n", num_blade );
        fprintf( fid, "Diameter: %.8f\n", diam );
        fprintf( fid, "Beta 3/4 (deg): %.8f\n", beta34 );
        fprintf( fid, "Feather (deg): %.8f\n", feather );
        fprintf( fid, "Center: %.8f, %.8f, %.8f\n", cen.x(), cen.y(), cen.z() );
        fprintf( fid, "Normal: %.8f, %.8f, %.8f\n", norm.x(), norm.y(), norm.z() );

        vector < double > r_vec = Find( "Radius" ).GetDoubleData();
        vector < double > chord_vec = Find( "Chord" ).GetDoubleData();
        vector < double > twist_vec = Find( "Twist" ).GetDoubleData();
        vector < double > rake_vec = Find( "Rake" ).GetDoubleData();
        vector < double > skew_vec = Find( "Skew" ).GetDoubleData();

        fprintf( fid, "\nRadius/R, Chord/R, Twist (deg), Rake/R, Skew/R\n" );
        for ( int i = 0; i < num_sect; i++ )
        {
            fprintf( fid, "%.8f, %.8f, %.8f, %.8f, %.8f\n", r_vec[i], chord_vec[i], twist_vec[i], rake_vec[i], skew_vec[i] );
        }

        for ( int i = 0; i < num_sect; i++ )
        {
            char str[255];
            sprintf( str, "%03d", i );
            vector < double > xpts = Find( "XSection_" + string( str ) ).GetDoubleData();
            vector < double > ypts = Find( "YSection_" + string( str ) ).GetDoubleData();

            fprintf( fid, "\nSection %d X, Y\n", i );

            for ( int j = 0; j < xpts.size(); j++ )
            {
                fprintf( fid, "%.8f, %.8f\n", xpts[j], ypts[j] );
            }
        }

        fclose( fid );
    }
}

//======================================================================================//
//======================================================================================//
//======================================================================================//


//==== Constructor ====//
ResultsMgrSingleton::ResultsMgrSingleton()
{

}
//==== Destructor ====//
ResultsMgrSingleton::~ResultsMgrSingleton()
{
    DeleteAllResults();
}


//==== Create and Add Results Object and Return Ptr ====//
Results* ResultsMgrSingleton::CreateResults( const string & name )
{
    string id = GenerateRandomID( 7 );
    Results* res_ptr = new Results( name, id );     // Create Results

    m_ResultsMap[id] = res_ptr;                     // Map ID to Ptr
    m_NameIDMap[name].push_back( id );              // Map Name to Vector of IDs
    return res_ptr;
}


//==== Create Results And Load With Geometry Data ====//
string ResultsMgrSingleton::CreateGeomResults( const string & geom_id, const string & name )
{
    Results* res_ptr = CreateResults( name );

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return string();
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        return string();
    }

    geom_ptr->CreateGeomResults( res_ptr );

    return res_ptr->GetID();
}


//==== Delete All Results ====//
void ResultsMgrSingleton::DeleteAllResults()
{
    //==== Delete All Created Results =====//
    map< string, Results* >::iterator iter;
    for ( iter = m_ResultsMap.begin() ; iter != m_ResultsMap.end() ; iter++ )
    {
        delete iter->second;
    }
    m_ResultsMap.clear();
    m_NameIDMap.clear();
}

//==== Delete All Results ====//
void ResultsMgrSingleton:: DeleteResult( const string & id )
{
    //==== Find Name Given ID ====//
    map< string, vector< string > >::iterator iter;

    //==== Find Name To Match ID And Remove====//
    for ( iter = m_NameIDMap.begin() ; iter != m_NameIDMap.end() ; iter++ )
    {
        bool found = false;
        for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
        {
            if ( iter->second[i] == id )
            {
                found = true;
            }
        }
        if ( found )
        {
            vector_remove_val( iter->second, id );
            if ( iter->second.size() == 0 )
            {
                m_NameIDMap.erase( iter );
            }

            break;
        }
    }

    //==== Delete All Created Results =====//
    map< string, Results* >::iterator res_iter;

    res_iter = m_ResultsMap.find( id );

    if ( res_iter != m_ResultsMap.end() )
    {
        delete res_iter->second;
        m_ResultsMap.erase( res_iter );
    }
}


//==== Find The Number of Results Given Name ====//
int ResultsMgrSingleton::GetNumResults( const string & name )
{
    map< string, vector< string > >::iterator iter = m_NameIDMap.find( name );

    if ( iter == m_NameIDMap.end() )
    {
        return 0;
    }

    return ( int )( iter->second.size() );
}

//==== Find Results ID Given Name and Optional Index =====//
string ResultsMgrSingleton::FindResultsID( const string & name, int index )
{
    map< string, vector< string > >::iterator iter = m_NameIDMap.find( name );
    if ( iter == m_NameIDMap.end() )
    {
        return string();
    }

    if ( index < 0 || index >= ( int )( iter->second.size() ) )
    {
        return string();
    }

    return iter->second[index];
}

//==== Find The Latest Results ID For the Given Name
string ResultsMgrSingleton::FindLatestResultsID( const string & name )
{
    map< string, vector< string > >::iterator iter = m_NameIDMap.find( name );
    if ( iter == m_NameIDMap.end() )
    {
        return string();
    }

    string latest_id;
    time_t latest_stamp = 0;
    for ( int i = 0 ; i < ( int )( iter->second.size() ) ; i++ )
    {
        string id = iter->second[i];
        Results* res_ptr = FindResultsPtr( id );

        if ( res_ptr && res_ptr->GetTimestamp() >= latest_stamp )
        {
            latest_stamp = res_ptr->GetTimestamp();
            latest_id = id;
        }
    }
    return latest_id;
}


//==== Find Results Ptr Given ID =====//
Results* ResultsMgrSingleton::FindResultsPtr( const string & id )
{
    map< string, Results* >::iterator id_iter = m_ResultsMap.find( id );

    if ( id_iter ==  m_ResultsMap.end() )
    {
        return NULL;
    }

    return id_iter->second;
}

//==== Find Results Ptr Given Name and Optional Index =====//
Results* ResultsMgrSingleton::FindResults( const string & name, int index )
{
    string id = FindResultsID( name, index );
    return FindResultsPtr( id );
}


//==== Get Results TimeStamp Given ID ====//
time_t ResultsMgrSingleton::GetResultsTimestamp( const string & results_id )
{
    map< string, Results* >::iterator iter = m_ResultsMap.find( results_id );

    if ( iter ==  m_ResultsMap.end() )
    {
        return vsp::INT_DATA;
    }

    return iter->second->GetTimestamp();
}


//==== Get Results TimeStamp Given ID ====//
int ResultsMgrSingleton::GetNumData( const string & results_id, const string & data_name )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return 0;
    }

    return results_ptr->GetNumData( data_name );
}

int ResultsMgrSingleton::GetResultsType( const string & results_id, const string & data_name )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return vsp::INVALID_TYPE;
    }

    NameValData* rd_ptr = results_ptr->FindPtr( data_name );
    if ( !rd_ptr )
    {
        return vsp::INVALID_TYPE;
    }

    return rd_ptr->GetType();
}

//==== Get The Names of All Results ====//
vector< string > ResultsMgrSingleton::GetAllResultsNames()
{
    vector< string > name_vec;
    map< string, vector< string > >::iterator iter;
    for ( iter = m_NameIDMap.begin() ; iter != m_NameIDMap.end() ; iter++ )
    {
        name_vec.push_back( iter->first );
    }
    return name_vec;
}

//==== Get The Names of All Data for A Givent Result ====//
vector< string > ResultsMgrSingleton::GetAllDataNames( const string & results_id )
{
    vector< string > name_vec;
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return name_vec;
    }

    return results_ptr->GetAllDataNames();
}

//==== Get Int Results Given Results ID and Name of Data and Index (Default 0) ====//
const vector<int> & ResultsMgrSingleton::GetIntResults( const string & results_id, const string & name, int index )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return m_DefaultIntVec;
    }

    NameValData* rd_ptr = results_ptr->FindPtr( name, index );
    if ( !rd_ptr )
    {
        return m_DefaultIntVec;
    }

    return rd_ptr->GetIntData();
}

//==== Get Double Results Given Results ID and Name of Data and Index (Default 0) ====//
const vector<double> & ResultsMgrSingleton::GetDoubleResults( const string & results_id, const string & name, int index )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return m_DefaultDoubleVec;
    }

    NameValData* rd_ptr = results_ptr->FindPtr( name, index );
    if ( !rd_ptr )
    {
        return m_DefaultDoubleVec;
    }

    return rd_ptr->GetDoubleData();
}

//==== Get string Results Given Results ID and Name of Data and Index (Default 0) ====//
const vector<string> & ResultsMgrSingleton::GetStringResults( const string & results_id, const string & name, int index )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return m_DefaultStringVec;
    }

    NameValData* rd_ptr = results_ptr->FindPtr( name, index );
    if ( !rd_ptr )
    {
        return m_DefaultStringVec;
    }

    return rd_ptr->GetStringData();
}

//==== Get Vec3d Results Given Results ID and Name of Data and Index (Default 0) ====//
const vector<vec3d> & ResultsMgrSingleton::GetVec3dResults( const string & results_id, const string & name, int index )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return m_DefaultVec3dVec;
    }

    NameValData* rd_ptr = results_ptr->FindPtr( name, index );
    if ( !rd_ptr )
    {
        return m_DefaultVec3dVec;
    }

    return rd_ptr->GetVec3dData();
}

//==== Check If Results ID is Valid ====//
bool ResultsMgrSingleton::ValidResultsID( const string & results_id )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return false;
    }
    return true;
}

//==== Check If Data Name and Index is Valid ====//
bool ResultsMgrSingleton::ValidDataNameIndex( const string & results_id, const string & name, int index )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return false;
    }

    NameValData* rd_ptr = results_ptr->FindPtr( name, index );
    if ( !rd_ptr )
    {
        return false;
    }

    return true;
}




//==== Write Some Test Results ====//
void ResultsMgrSingleton::WriteTestResults()
{
    //==== Write Two Sets of Results ====//
    for ( int s = 0 ; s < 2 ; s++ )
    {
        Results* res = ResultsMgr.CreateResults( "Test_Results" );

//      printf( "Timestamp = %d \n", res->GetTimestamp() );

        res->Add( NameValData( "Test_Int", s + 1 ) );
        res->Add( NameValData( "Test_Int", s + 2 ) );
        res->Add( NameValData( "Test_Double", ( s + 1 ) * 0.1 ) );
        res->Add( NameValData( "Test_String", "This Is A Test" ) );
        res->Add( NameValData( "Test_Vec3d", vec3d( s, s * 2, s * 4 ) ) );

        vector< double > dvec;
        for ( int i = 0 ; i < 5 ; i++ )
        {
            dvec.push_back( i * ( s + 1 ) );
        }
        res->Add( NameValData( "Test_Double_Vec", dvec ) );
    }


}


//==== Test Speed ====//
void ResultsMgrSingleton::TestSpeed()
{
//  vector< vec3d > pnt_vec;
//  for ( int i = 0 ; i < 10000000 ; i++ )
//      pnt_vec.push_back( vec3d( i, 2*i, 3*i ) );
//
//  Results* res = CreateResults( "TestVec" );
//  res->Add( ResData( "Pnt_Vec", pnt_vec)  );
//
//  int start_tics = GetTickCount();
//
//  //==== Sum Vectors ====//
//  vec3d sum;
//  for ( int i = 0 ; i < (int)pnt_vec.size() ; i++ )
//      sum = sum + pnt_vec[i];
//
//  int tics1 = GetTickCount();
//
//  Results* test_res = FindLatestResult( "TestVec" );
//  ResData pnt_vec_result = test_res->Find( "Pnt_Vec", 0 );
//
////    const vector< vec3d > & res_pnt_vec = pnt_vec_result.GetVec3dData();
//  vector< vec3d > res_pnt_vec = pnt_vec_result.GetVec3dData();
//
//  //==== Sum Vectors ====//
//  vec3d sum1;
//  for ( int i = 0 ; i < (int)res_pnt_vec.size() ; i++ )
//      sum1 = sum1 + res_pnt_vec[i];

//  int tics2 = GetTickCount();
//
//  if ( dist( sum, sum1 ) > 0.000001 )
//      printf("Not Good\n" );
//
//  int del_t0 = tics1 - start_tics;
//  int del_t1 = tics2 - tics1;
//  printf(" Results Speed %d %d \n", del_t0, del_t1 );
}


int ResultsMgrSingleton::WriteCSVFile( const string & file_name, const vector < string > &resids )
{
    FILE* fid = fopen( file_name.c_str(), "w" );
    if( fid )
    {
        for( unsigned int iRes=0; iRes<resids.size(); iRes++ )
        {
            Results* resptr = ResultsMgr.FindResultsPtr( resids[iRes] );
            if( resptr )
            {
                // TODO print the name of the result to the file as a header for each result
                resptr->WriteCSVFile( fid );    //append this result to the csv file
            }
        }
        fclose( fid );          // Close File
        return vsp::VSP_OK;
    }
    else
    {
        return vsp::VSP_FILE_WRITE_FAILURE;
    }
}
