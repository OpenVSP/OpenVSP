//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.cpp: implementation of the Vehicle Class and Vehicle Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "Vehicle.h"
#include "VspUtil.h"
#include "StlHelper.h"

#ifdef WIN32
#include <windows.h>
#endif


//==== Default Results Data ====//
NameValData::NameValData()
{
    Init( "Undefined" );
}

//==== Constructor With Name =====//
NameValData::NameValData( const string & name )
{
    Init( name );
}

//==== Constructors With Name & Data =====//
NameValData::NameValData( const string & name, const int & i_data )
{
    Init( name, vsp::INT_DATA );
    m_IntData.push_back( i_data );
}
NameValData::NameValData( const string & name, const double & d_data )
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
NameValData::NameValData( const string & name, const vector< int > & i_data )
{
    Init( name, vsp::INT_DATA );
    m_IntData = i_data;
}
NameValData::NameValData( const string & name, const vector< double > & d_data )
{
    Init( name, vsp::DOUBLE_DATA );
    m_DoubleData = d_data;
}
NameValData::NameValData( const string & name, const vector< string > & s_data )
{
    Init( name, vsp::STRING_DATA );
    m_StringData = s_data;
}
NameValData::NameValData( const string & name, const vector< vec3d > & v_data )
{
    Init( name, vsp::VEC3D_DATA );
    m_Vec3dData = v_data;
}
NameValData::NameValData( const string &name, const vector< vector< double > > &dmat_data )
{
    Init( name, vsp::DOUBLE_MATRIX_DATA );
    m_DoubleMatData = dmat_data;
}
void NameValData::Init( const string & name, int type, int index )
{
    m_Name = name;
    m_Type = type;
}

int NameValData::GetInt( int i ) const
{
    if ( i >= 0 && i < ( int )m_IntData.size() )
    {
        return m_IntData[i];
    }
    return 0;
}
double NameValData::GetDouble( int i ) const
{
    if ( i >= 0 && i < ( int )m_DoubleData.size() )
    {
        return m_DoubleData[i];
    }
    return 0;
}
double NameValData::GetDouble( int row, int col ) const
{
    if ( row >= 0 && row < ( int )m_DoubleMatData.size() )
    {
        if ( col >= 0 && col < ( int )m_DoubleMatData[row].size() )
        {
            return m_DoubleMatData[row][col];
        }
    }
    return 0;
}
string NameValData::GetString( int i ) const
{
    if ( i >= 0 && i < ( int )m_StringData.size() )
    {
        return m_StringData[i];
    }
    return string();
}

vec3d NameValData::GetVec3d( int i ) const
{
    if ( i >= 0 && i < ( int )m_Vec3dData.size() )
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

void NameValCollection::Add(const vector<vector<vec3d> > & d, string prefix)
{
    string names[] = { prefix + "x", prefix + "y", prefix + "z"};
    for ( int dim = 0; dim < 3; dim++ )
    {
        vector< vector< double > > arr;
        arr.reserve( d.size() );
        for ( int i = 0; i < ( int )d.size(); i++ )
        {
            vector<double> row;
            row.reserve( d[i].size() );
            for ( int j = 0; j < ( int )d[i].size(); j++ )
            {
                row.push_back( d[i][j].v[dim] );
            }
            arr.push_back( row );
        }

        Add( NameValData( names[dim], arr ) );
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

    for ( iter = m_DataMap.begin() ; iter != m_DataMap.end() ; ++iter )
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
        fprintf( fid, "Results_Timestamp,%lld\n", m_Timestamp );
        fprintf( fid, "Results_Date,%d,%d,%d\n", m_Month, m_Day, m_Year );
        fprintf( fid, "Results_Time,%d,%d,%d\n", m_Hour, m_Min, m_Sec );

        map< string, vector< NameValData > >::iterator iter;
        for ( iter = m_DataMap.begin() ; iter != m_DataMap.end() ; ++iter )
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
                        fprintf( fid, ",%.*e", DBL_DIG + 3, iter->second[i].GetDoubleData()[d] );
                    }
                }
                else if ( iter->second[i].GetType() == vsp::DOUBLE_MATRIX_DATA )
                {
                    vector< vector< double > > current_double_mat_val = iter->second[i].GetDoubleMatData();
                    for ( unsigned int row = 0; row < current_double_mat_val.size(); row++ )
                    {
                        for ( unsigned int col = 0; col < current_double_mat_val[row].size(); col++ )
                        {
                            fprintf( fid, ",%.*e", DBL_DIG + 3, current_double_mat_val[row][col] );
                        }
                        if ( row < current_double_mat_val.size() - 1 )
                        {
                            fprintf( fid, "\n ");
                        }
                    }
                }
                else if ( iter->second[i].GetType() == vsp::STRING_DATA )
                {
                    // If this is a "ResultsVec" wrapper result replace result UIDs with result names
                    if ( strcmp( iter->second[i].GetName().c_str(), "ResultsVec" ) == 0 )
                    {
                        for ( int d = 0; d < (int)iter->second[i].GetStringData().size(); d++ )
                        {
                            fprintf( fid, ",%s", ResultsMgr.FindResultsPtr(iter->second[i].GetStringData()[d])->GetName().c_str() );
                        }
                    }
                    else
                    {
                        for ( int d = 0; d < (int)iter->second[i].GetStringData().size(); d++ )
                        {
                            fprintf( fid, ",%s", iter->second[i].GetStringData()[d].c_str() );
                        }
                    }
                }
                else if ( iter->second[i].GetType() == vsp::VEC3D_DATA )
                {
                    for ( int d = 0 ; d < ( int )iter->second[i].GetVec3dData().size() ; d++ )
                    {
                        vec3d v = iter->second[i].GetVec3dData()[d];
                        fprintf( fid, ",%.*e,%.*e,%.*e", DBL_DIG + 3, v.x(), DBL_DIG + 3, v.y(), DBL_DIG + 3, v.z() );
                    }
                }
                fprintf( fid, "\n" );
            }
        }

        // Loop again to recursively call WriteCSV() if this result contains a "ResultsVec" wrapper result
        for ( iter = m_DataMap.begin(); iter != m_DataMap.end(); ++iter )
        {
            for ( int i = 0; i < (int)iter->second.size(); i++ )
            {
                if ( (iter->second[i].GetType() == vsp::STRING_DATA) && (strcmp( iter->second[i].GetName().c_str(), "ResultsVec" ) == 0) )
                {
                    for ( int d = 0; d < (int)iter->second[i].GetStringData().size(); d++ )
                    {
                        Results * res = ResultsMgr.FindResultsPtr( iter->second[i].GetStringData()[d] );
                        if ( res )
                        {
                            res->WriteCSVFile( fid );
                        }
                    }
                }
            }
        }   // end recursion

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

            vector < string > remnames = Find( "Meshes_Removed_Names" ).GetStringData();
            for ( int i = 0; i < remnames.size(); i++ )
            {
                fprintf( fid, "     Removed: %s\n", remnames[i].c_str() );
            }
        }

        int num_open_merge = Find( "Num_Open_Meshes_Merged" ).GetInt( 0 );
        if ( num_open_merge != 0 )
        {
            fprintf( fid, "WARNING: %d open meshes merged\n", num_open_merge );

            vector < string > mernames = Find( "Meshes_Merged_Names" ).GetStringData();
            for ( int i = 0; i < mernames.size(); i++ )
            {
                fprintf( fid, "     Merged: %s\n", mernames[i].c_str() );
            }
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

        for ( int i = 0 ; i < num_comps; i++ )
        {
            string comp_Id = Find( "Comp_ID" ).GetString( i );
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

        int num_tag = Find( "Num_Tags" ).GetInt( 0 );
        if ( num_tag > 0 )
        {
            fprintf( fid, "\n" );
            fprintf( fid, "Tag_Theo_Area   Tag_Wet_Area    Tag_Name\n" );

            for ( int i = 0 ; i < num_tag ; i++ )
            {
                fprintf( fid, "    %9.3f      %9.3f    %-15s\n",
                         Find( "Tag_Theo_Area" ).GetDouble( i ), Find( "Tag_Wet_Area" ).GetDouble( i ),
                         Find( "Tag_Name" ).GetString( i ).c_str() );
            }
        }

        if ( Find( "Num_Degen_Tris_Removed" ).GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d degenerate triangle removed\n", Find( "Num_Degen_Tris_Removed" ).GetInt( 0 ) );
        }
        if ( Find( "Num_Open_Meshes_Removed" ).GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d open meshes removed\n", Find( "Num_Open_Meshes_Removed" ).GetInt( 0 ) );

            vector < string > remnames = Find( "Meshes_Removed_Names" ).GetStringData();
            for ( int i = 0; i < remnames.size(); i++ )
            {
                fprintf( fid, "     Removed: %s\n", remnames[i].c_str() );
            }
        }
        if ( Find( "Num_Open_Meshes_Merged" ).GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d open meshes merged\n", Find( "Num_Open_Meshes_Merged" ).GetInt( 0 ) );

            vector < string > mernames = Find( "Meshes_Merged_Names" ).GetStringData();
            for ( int i = 0; i < mernames.size(); i++ )
            {
                fprintf( fid, "     Merged: %s\n", mernames[i].c_str() );
            }
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

        int num_tag = Find( "Num_Tags" ).GetInt( 0 );
        if ( num_tag > 0 )
        {
            fprintf( fid, "\nTag_Name, Tag_Theo_Area, Tag_Wet_Area\n" );
            for ( int i = 0 ; i < num_tag ; i++ )
            {
                fprintf( fid, "%s,%f,%f\n",
                         Find( "Tag_Name" ).GetString( i ).c_str(),
                         Find( "Tag_Theo_Area" ).GetDouble( i ), Find( "Tag_Wet_Area" ).GetDouble( i ) );
            }
        }

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

void Results::WriteParasiteDragFile( const string & file_name )
{
    FILE* file_id = fopen(file_name.c_str(), "w");

    if (file_id)
    {
        fprintf(file_id, "PARASITE DRAG BUILD UP DATA \n\n");

        string AltLabel = Find("Alt_Label").GetString(0);
        string VinfLabel = Find("Vinf_Label").GetString(0);
        string SrefLabel = Find("Sref_Label").GetString(0);
        double mach = Find("FC_Mach").GetDouble(0);
        double hinf = Find("FC_Alt").GetDouble(0);
        double vinf = Find("FC_Vinf").GetDouble(0);
        double sref = Find("FC_Sref").GetDouble(0);
        fprintf(file_id, "Mach, %s, %s, %s\n", AltLabel.c_str(), VinfLabel.c_str(), SrefLabel.c_str());
        fprintf(file_id, "%f, %f, %f, %f \n\n", mach, hinf, vinf, sref);

        string TempLabel = Find("Temp_Label").GetString(0);
        string PresLabel = Find("Pres_Label").GetString(0);
        string RhoLabel = Find("Rho_Label").GetString(0);
        double temp = Find("FC_Temp").GetDouble(0);
        double pres = Find("FC_Pres").GetDouble(0);
        double rho = Find("FC_Rho").GetDouble(0);
        string lamcfeqnname = Find("LamCfEqnName").GetString(0);
        string turbcfeqnname = Find("TurbCfEqnName").GetString(0);

        fprintf(file_id, "%s, %s, %s \n", TempLabel.c_str(), PresLabel.c_str(), RhoLabel.c_str());
        fprintf(file_id, "%f, %f, %f \n\n", temp, pres, rho);
        fprintf(file_id, "Lam Cf Eqn, Turb Cf Eqn \n");
        fprintf(file_id, "%s, %s \n", lamcfeqnname.c_str(), turbcfeqnname.c_str());

        fprintf(file_id, "\n");

        string SwetLabel = Find("Swet_Label").GetString(0);
        string LrefLabel = Find("Lref_Label").GetString(0);
        string fLabel = Find("f_Label").GetString(0);

        fprintf(file_id, "Component Name,%s,%s,t/c or d/l,FF,FF Eqn Type,Re,%% Lam,Cf,Q (Interference Factor),%s,Cd,%% Total \n",
            SwetLabel.c_str(), LrefLabel.c_str(), fLabel.c_str() );

        int num_comp = Find("Num_Comp").GetInt(0);
        for (int i = 0; i < num_comp; i++)
        {
            string label = Find("Comp_Label").GetString(i);
            double swet = Find("Comp_Swet").GetDouble(i);
            double lref = Find("Comp_Lref").GetDouble(i);
            double fineRat = Find("Comp_FineRat").GetDouble(i);
            int FFType = Find("Comp_FFEqn").GetInt(i);
            double FF = 0;
            if (FFType == vsp::FF_B_MANUAL || FFType == vsp::FF_W_MANUAL)
            {
                FF = Find("Comp_FFIn").GetDouble(i);
            }
            else
            {
                FF = Find("Comp_FFOut").GetDouble(i);
            }
            string FFEqnName = Find("Comp_FFEqnName").GetString(i);
            double Re = Find("Comp_Re").GetDouble(i);
            double PercLam = Find("Comp_PercLam").GetDouble(i);
            double Cf = Find("Comp_Cf").GetDouble(i);
            double Q = Find("Comp_Q").GetDouble(i);
            double f = Find("Comp_f").GetDouble(i);
            double CD = Find("Comp_CD").GetDouble(i);
            double PercTotalCD = 100 * Find("Comp_PercTotalCD").GetDouble(i);

            fprintf(file_id, "%s,", label.c_str());
            fprintf(file_id, "%f, %f, %f,", swet, lref, fineRat);
            fprintf(file_id, "%f, %s,", FF, FFEqnName.c_str());
            fprintf(file_id, "%f, %f,", Re, PercLam);
            fprintf(file_id, "%f, %f,", Cf, Q);
            fprintf(file_id, "%f, %f, %f \n", f, CD, PercTotalCD);

        }

        fprintf(file_id, "\n");
        fprintf(file_id, "Excrescences, Type, Input\n");

        int num_excres = Find("Num_Excres").GetInt(0);
        for (int i = 0; i < num_excres; i++)
        {
            string label = Find("Excres_Label").GetString(i);
            string type = Find("Excres_Type").GetString(i);
            double input = Find("Excres_Input").GetDouble(i);
            double f = Find( "Excres_f" ).GetDouble( i );
            double amount = Find("Excres_Amount").GetDouble(i);
            double perctotalcd = 100 * Find("Excres_PercTotalCD").GetDouble(i);

            fprintf(file_id, " %s, %s, %f, , , , , , , ,%f, %f, %f \n",
                label.c_str(), type.c_str(), input, f, amount, perctotalcd);
        }

        double geomftotal = Find("Geom_f_Total").GetDouble(0);
        double geomcdtotal = Find("Geom_CD_Total").GetDouble(0);
        double geomperctotal = 100 * Find("Geom_Perc_Total").GetDouble(0);
        double excresftotal = Find("Excres_f_Total").GetDouble(0);
        double excrescdtotal = Find("Excres_CD_Total").GetDouble(0);
        double excresperctotal = 100 * Find("Excres_Perc_Total").GetDouble(0);
        double totalftotal = Find("Total_f_Total").GetDouble(0);
        double totalcdtotal = Find("Total_CD_Total").GetDouble(0);
        double totalperctotal = 100 * Find("Total_Perc_Total").GetDouble(0);

        fprintf(file_id, "\n");
        fprintf(file_id, " , , , , , , , , , %s, %f, %f, %f \n", "Geometry Sub-Total:", geomftotal, geomcdtotal, geomperctotal);
        fprintf(file_id, " , , , , , , , , , %s, %f, %f, %f \n", "Excrescence Sub-Total:", excresftotal, excrescdtotal, excresperctotal);
        fprintf(file_id, " , , , , , , , , , %s, %f, %f, %f \n", "Totals:", totalftotal, totalcdtotal, totalperctotal);

        fclose(file_id);
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

            vector < string > remnames = Find( "Meshes_Removed_Names" ).GetStringData();
            for ( int i = 0; i < remnames.size(); i++ )
            {
                fprintf( fid, "     Removed: %s\n", remnames[i].c_str() );
            }
        }
        if ( Find( "Num_Open_Meshes_Merged" ).GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d open meshes merged\n", Find( "Num_Open_Meshes_Merged" ).GetInt( 0 ) );

            vector < string > mernames = Find( "Meshes_Merged_Names" ).GetStringData();
            for ( int i = 0; i < mernames.size(); i++ )
            {
                fprintf( fid, "     Merged: %s\n", mernames[i].c_str() );
            }
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

        double Mach = Find( "Mach" ).GetDouble( 0 );
        fprintf( fid, "Mach: %19.8f \n", Mach);

        string Set_Name = Find( "Set_Name" ).GetString( 0 );
        fprintf( fid, "Set Name: %s \n", Set_Name.c_str());

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
        double precone = Find( "Pre_Cone" ).GetDouble( 0 );
        vec3d cen = Find( "Center" ).GetVec3d( 0 );
        vec3d norm = Find( "Normal" ).GetVec3d( 0 );

        fprintf( fid, "Num_Sections: %d\n", num_sect );
        fprintf( fid, "Num_Blade: %d\n", num_blade );
        fprintf( fid, "Diameter: %.8f\n", diam );
        fprintf( fid, "Beta 3/4 (deg): %.8f\n", beta34 );
        fprintf( fid, "Feather (deg): %.8f\n", feather );
        fprintf( fid, "Pre_Cone (deg): %.8f\n", precone );
        fprintf( fid, "Center: %.8f, %.8f, %.8f\n", cen.x(), cen.y(), cen.z() );
        fprintf( fid, "Normal: %.8f, %.8f, %.8f\n", norm.x(), norm.y(), norm.z() );

        vector < double > r_vec = Find( "Radius" ).GetDoubleData();
        vector < double > chord_vec = Find( "Chord" ).GetDoubleData();
        vector < double > twist_vec = Find( "Twist" ).GetDoubleData();
        vector < double > rake_vec = Find( "Rake" ).GetDoubleData();
        vector < double > skew_vec = Find( "Skew" ).GetDoubleData();
        vector < double > sweep_vec = Find( "Sweep" ).GetDoubleData();
        vector < double > thick_vec = Find( "Thick" ).GetDoubleData();
        vector < double > cli_vec = Find( "CLi" ).GetDoubleData();
        vector < double > axial_vec = Find( "Axial" ).GetDoubleData();
        vector < double > tangential_vec = Find( "Tangential" ).GetDoubleData();

        fprintf( fid, "\nRadius/R, Chord/R, Twist (deg), Rake/R, Skew/R, Sweep, t/c, CLi, Axial, Tangential\n" );
        for ( int i = 0; i < num_sect; i++ )
        {
            fprintf( fid, "%.8f, %.8f, %.8f, %.8f, %.8f, %.8f, %.8f, %.8f, %.8f, %.8f\n", r_vec[i], chord_vec[i], twist_vec[i], rake_vec[i], skew_vec[i], sweep_vec[i], thick_vec[i], cli_vec[i], axial_vec[i], tangential_vec[i] );
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

// Copy a NameValData pointer to this Result
void Results::Copy( NameValData* nvd )
{
    switch ( nvd->GetType() )
    {
        case ( vsp::DOUBLE_DATA ):
        {
            Add( ( NameValData( nvd->GetName(), nvd->GetDoubleData() ) ) );
            break;
        }
        case ( vsp::INT_DATA ):
        {
            Add( ( NameValData( nvd->GetName(), nvd->GetIntData() ) ) );
            break;
        }
        case ( vsp::STRING_DATA ):
        {
            Add( ( NameValData( nvd->GetName(), nvd->GetStringData() ) ) );
            break;
        }
        case ( vsp::DOUBLE_MATRIX_DATA ):
        {
            Add( ( NameValData( nvd->GetName(), nvd->GetDoubleMatData() ) ) );
            break;
        }
        case ( vsp::VEC3D_DATA ):
        {
            Add( ( NameValData( nvd->GetName(), nvd->GetVec3dData() ) ) );
            break;
        }
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
    for ( iter = m_ResultsMap.begin() ; iter != m_ResultsMap.end() ; ++iter )
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
    for ( iter = m_NameIDMap.begin() ; iter != m_NameIDMap.end() ; ++iter )
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
    for ( iter = m_NameIDMap.begin() ; iter != m_NameIDMap.end() ; ++iter )
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

void ResultsMgrSingleton::PrintResults( const string &fname, const string &results_id )
{
    FILE *fp;
    fp = fopen( fname.c_str(), "w" );
    if ( fp )
    {
        PrintResults( fp, results_id );
        fclose( fp );
    }
}

void ResultsMgrSingleton::PrintResults( const string &results_id )
{
    PrintResults( stdout, results_id );
}

void ResultsMgrSingleton::PrintResults( FILE * outputStream, const string &results_id )
{
    fprintf( outputStream, "\n\t\t%-20s%s\t%s\t%s\n", "[result_name]", "[type]", "[#]", "[current values-->]" );

    vector<string> results_names = GetAllDataNames( results_id );
    for ( unsigned int i_result_name = 0; i_result_name < results_names.size(); i_result_name++ )
    {
        int current_result_type = GetResultsType( results_id, results_names[i_result_name] );
        unsigned int current_result_num_data = ( unsigned int )GetNumData( results_id, results_names[i_result_name] );
        fprintf( outputStream, "\t\t%-20s%d\t\t%d", results_names[i_result_name].c_str(), current_result_type, current_result_num_data );
        // print out the current value (this needs to handle different types and vector lengths
        fprintf( outputStream, "\t" );
        for ( unsigned int i_val = 0; i_val < current_result_num_data; i_val++ )
        {
            switch( current_result_type )
            {
            case vsp::RES_DATA_TYPE::INT_DATA :
            {
                vector<int> current_int_val = GetIntResults( results_id, results_names[i_result_name], i_val );
                for ( unsigned int j_val = 0; j_val < current_int_val.size(); j_val++ )
                {
                    fprintf( outputStream, "%d ", current_int_val[j_val] );
                }
                break;
            }
            case vsp::RES_DATA_TYPE::DOUBLE_DATA :
            {
                vector<double> current_double_val = GetDoubleResults( results_id, results_names[i_result_name], i_val );
                for ( unsigned int j_val = 0; j_val < current_double_val.size(); j_val++ )
                {
                    fprintf( outputStream, "%.*e ", DBL_DIG + 3, current_double_val[j_val] );
                }
                break;
            }
            case vsp::RES_DATA_TYPE::DOUBLE_MATRIX_DATA :
            {
                vector< vector< double > > current_double_mat_val = GetDoubleMatResults( results_id, results_names[i_result_name], i_val );
                for ( unsigned int row = 0; row < current_double_mat_val.size(); row++ )
                {
                    for ( unsigned int col = 0; col < current_double_mat_val[row].size(); col++ )
                    {
                        fprintf( outputStream, "%.*e ", DBL_DIG + 3, current_double_mat_val[row][col] );
                    }
                    if ( row < current_double_mat_val.size() - 1 )
                    {
                        fprintf( outputStream, "\n\t\t%-20s \t\t \t", "");
                    }
                }
                break;
            }
            case vsp::RES_DATA_TYPE::STRING_DATA :
            {
                vector<string> current_string_val = GetStringResults( results_id, results_names[i_result_name], i_val );
                for ( unsigned int j_val = 0; j_val < current_string_val.size(); j_val++ )
                {
                    fprintf( outputStream, "%s ", current_string_val[j_val].c_str() );
                }
                //Recursive call if results vector found
                if( strcmp( results_names[i_result_name].c_str(), "ResultsVec" ) == 0 )
                {
                    vector < string > res_vec = GetStringResults( results_id, "ResultsVec", 0 );
                    for ( unsigned int i = 0; i < res_vec.size(); i++ )
                    {
                        PrintResults( outputStream, res_vec[i] );
                    }
                }
                break;
            }
            case vsp::RES_DATA_TYPE::VEC3D_DATA :
            {
                vector<vec3d> current_vec3d_val = GetVec3dResults( results_id, results_names[i_result_name], i_val );
                for ( unsigned int j_val = 0; j_val < current_vec3d_val.size(); j_val++ )
                {
                    fprintf( outputStream, "%.*e,%.*e,%.*e ", DBL_DIG + 3, current_vec3d_val[j_val].x(), DBL_DIG + 3, current_vec3d_val[j_val].y(), DBL_DIG + 3, current_vec3d_val[j_val].z() );
                }
                break;
            }
            }    //end switch

        }    // end for

        fprintf( outputStream, "\n" );
    }
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

//==== Get Double Matrix Results Given Results ID and Name of Data and Index (Default 0) ====//
const vector<vector<double>> & ResultsMgrSingleton::GetDoubleMatResults( const string &id, const string &name,
                          int index )
{
    Results* results_ptr = FindResultsPtr( id );
    if ( !results_ptr )
    {
        return m_DefaultDoubleMat;
    }

    NameValData* rd_ptr = results_ptr->FindPtr( name, index );
    if ( !rd_ptr )
    {
        return m_DefaultDoubleMat;
    }

    return rd_ptr->GetDoubleMatData();
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
