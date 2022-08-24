//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SubSurfaceMgr.cpp:
// Alex Gary
//
//////////////////////////////////////////////////////////////////////

#include "SubSurfaceMgr.h"
#include "Vehicle.h"

using std::vector;
using std::string;
using std::map;

SubSurfaceMgrSingleton::SubSurfaceMgrSingleton()
{
    m_CurrSurfInd = -1;
    ClearTagMaps();
}

SubSurfaceMgrSingleton::~SubSurfaceMgrSingleton()
{
}


//==== Get the geom pointer matching a given comp_id ====//
Geom* SubSurfaceMgrSingleton::GetGeom( string comp_id )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return NULL;
    }

    return veh->FindGeom( comp_id );
}

void SubSurfaceMgrSingleton::SetCurrSubSurfInd( int index )
{
    m_CurrSurfInd = index;
}

//==== Get copy of sub surfaces from geom by id and surf number ====//
vector< SubSurface*> SubSurfaceMgrSingleton::GetSubSurfs( const string& comp_id, int surfnum )
{
    vector< SubSurface* > ret_vec;

    Geom* geom = GetGeom( comp_id );
    if ( !geom )
    {
        return ret_vec;
    }

    if ( geom->GetType().m_Type == MESH_GEOM_TYPE ||
         geom->GetType().m_Type == HUMAN_GEOM_TYPE )
    {
        return ret_vec;
    }

    int imain = geom->GetMainSurfID( surfnum );

    if ( imain < 0 )
    {
        return ret_vec;
    }

    vector< SubSurface* > all_vec;
    all_vec = geom->GetSubSurfVec();

    for ( int i = 0; i < all_vec.size(); i++ )
    {
        if ( imain == all_vec[i]->m_MainSurfIndx() )
        {
            ret_vec.push_back( all_vec[i] );
        }
    }

    return ret_vec;
}

//==== Get copy of sub surfaces from geom by id ====//
vector< SubSurface*> SubSurfaceMgrSingleton::GetSubSurfs( const string& comp_id )
{
    vector< SubSurface* > ret_vec;

    Geom* geom = GetGeom( comp_id );
    if ( !geom )
    {
        return ret_vec;
    }

    return geom->GetSubSurfVec();
}

//==== Get the sub surfaces from all geoms ====//
vector< SubSurface* > SubSurfaceMgrSingleton::GetSubSurfs()
{
    vector< SubSurface* > ret_vec;
    Vehicle* veh = VehicleMgr.GetVehicle();
    if( !veh )
    {
        return ret_vec;
    }

    vector<Geom*> geoms = veh->FindGeomVec( veh->GetGeomVec() );
    for ( int i = 0 ; i < ( int )geoms.size() ; i++ )
    {
        const vector< SubSurface* > SubSurfVec = geoms[i]->GetSubSurfVec();
        for ( int j = 0 ; j < ( int )SubSurfVec.size() ; j++ )
        {
            ret_vec.push_back( SubSurfVec[j] );
        }
    }

    return ret_vec;
}

//==== Get subsurface pointer from subsurface id ====//
SubSurface* SubSurfaceMgrSingleton::GetSubSurf( const string& subsurf_id )
{
    SubSurface* ret_ptr = NULL;
    Vehicle* veh = VehicleMgr.GetVehicle();
    if( !veh )
    {
        return ret_ptr;
    }

    vector<Geom*> geoms = veh->FindGeomVec( veh->GetGeomVec() );
    for ( int i = 0 ; i < ( int )geoms.size() ; i++ )
    {
        const vector< SubSurface* > SubSurfVec = geoms[i]->GetSubSurfVec();
        for ( int j = 0 ; j < ( int )SubSurfVec.size() ; j++ )
        {
            if( SubSurfVec[j]->GetID() == subsurf_id )
            {
                ret_ptr = SubSurfVec[j];
            }
        }
    }

    return ret_ptr;
}

void SubSurfaceMgrSingleton::PrepareToSplit()
{
    vector< SubSurface* > sub_surfs = GetSubSurfs();

    for ( int i = 0 ; i < ( int )sub_surfs.size() ; i++ )
    {
        sub_surfs[i]->PrepareSplitVec();
    }
}

void SubSurfaceMgrSingleton::ReSuffixGroupNames( const string& comp_id )
{
    vector< SubSurface* > ss_vec = GetSubSurfs( comp_id );

    map< int, int > t_map;

    for ( int i = 0 ; i < vsp::SS_NUM_TYPES; i++ )
    {
        t_map[i] = 0;
    }

    for ( int i = 0; i < ( int )ss_vec.size() ; i++ )
    {
        t_map[ ss_vec[i]->GetType() ]++;
        ss_vec[i]->SetDisplaySuffix( t_map[ ss_vec[i]->GetType() ] );
    }
}

//==== Manage Tag Maps ====//
void SubSurfaceMgrSingleton::ClearTagMaps()
{

    m_TagCombos.clear();
    m_TagNames.clear();
    m_TagIDs.clear();
    m_SingleTagMap.clear();
    m_CompNames.clear();

    // Add Dummy tag combo for meshes with no tags
    // so there will a draw object for them
    vector<int> dummy_tags;
    dummy_tags.resize( 10, 9999 );
    m_SingleTagMap[dummy_tags] = -1;
}

//==== Set the tag number for each sub-surface of all Geoms ====//
void SubSurfaceMgrSingleton::SetSubSurfTags( int tag_offset )
{
    // tag_offset should be the number of total components in mesh

    vector< SubSurface* > sub_surfs = GetSubSurfs();
    for ( int i = 0 ; i < ( int )sub_surfs.size() ; i++ )
    {
        sub_surfs[i]->m_Tag = tag_offset + i + 1;
        // map tag number to surface name
        m_TagNames[sub_surfs[i]->m_Tag] = sub_surfs[i]->GetName();
        m_TagIDs[sub_surfs[i]->m_Tag] = sub_surfs[i]->GetID();
    }
}

void SubSurfaceMgrSingleton::BuildCompNameMap()
{
    for ( int i = 0 ; i < ( int )m_CompNames.size() ; i++ )
    {
        m_TagNames[i + 1] = m_CompNames[i];
    }

    // Keys 1 - m_CompNames.size() should be empty since SetSubSurfTags
    // should have been called with the appropriate offset
}

void SubSurfaceMgrSingleton::BuildCompIDMap()
{
    for ( int i = 0; i < (int)m_CompIDs.size(); i++ )
    {
        m_TagIDs[i + 1] = m_CompIDs[i];
    }
}

//==== Custom Sorting Function ====//
bool vector_sort( const vector<int>& a, const vector<int>& b )
{
    if ( a.size() == b.size() )
    {
        for ( int i = 0 ; i < ( int )a.size() ; i++ )
        {
            if ( a[i] != b[i] )
            {
                return a[i] < b[i];
            }
        }

        return a[0] < b[0];
    }

    return a.size() < b.size();
}

void SubSurfaceMgrSingleton::BuildSingleTagMap()
{
    m_TagKeys.clear();
    for ( set< vector<int> >::iterator s_iter = m_TagCombos.begin() ; s_iter != m_TagCombos.end() ; ++s_iter )
    {
        m_TagKeys.push_back( *s_iter );
    }

    // Sort tag combos by length then if same size sort by tag order
    sort( m_TagKeys.begin(), m_TagKeys.end(),
          vector_sort );

    // Map tag combos to single tag number
    for ( int i = 0; i < ( int )m_TagKeys.size() ; i++ )
    {
        // If a tag key only has one tag, check to see if the name contains
        // "Wake". If this is true, then this is a wake from CFDMesh and
        // it should be assigned a tag number greater than 100 for
        // CBAero

        if ( m_TagKeys[i].size() == 1 && string::npos != m_TagNames[ m_TagKeys[i][0] ].find( "Wake" ) )
        {
            m_SingleTagMap[m_TagKeys[i]] = i + 1 + 1000;
        }
        else
        {
            m_SingleTagMap[m_TagKeys[i]] = i + 1;
        }
    }
}

//==== Write Key File ====//
void SubSurfaceMgrSingleton::WriteVSPGEOMKeyFile( const string & file_name )
{
    // figure out basename
    string base_name = file_name;
    std::string::size_type loc = base_name.find_last_of( "." );
    if ( loc != base_name.npos )
    {
        base_name = base_name.substr( 0, loc );
    }
    string key_name = base_name + ".vkey";

    FILE* fid = fopen( key_name.c_str(), "w" );
    if ( !fid )
    {
        return;
    }

    // Write Out Header Information
    fprintf( fid, "# VSPGEOM Tag Key File\n" );
    fprintf( fid, "%s\n", file_name.c_str() ); // Write out the file that this key information is for
    fprintf( fid, "%lu\n", m_SingleTagMap.size() - 1 ); // Total number of tags ( the minus 1 is from the dummy tags )
    fprintf( fid, "\n" );
    fprintf( fid, "# tag#,geom#,surf#,gname,gid,ssname1,ssname2,...,ssid1,ssid2,...\n" );

    // Build GeomID set to have unique integer index instead of GeomID.
    std::set< string, greater< string > > gids;
    for ( int i = 0 ; i < ( int )m_TagKeys.size() ; i++ )
    {
        string id_list = GetTagIDs( m_TagKeys[i] );
        int pos = id_list.find( "_Surf" );
        string gid = id_list.substr( 0, pos );
        gids.insert( gid );
    }

    for ( int i = 0 ; i < ( int )m_TagKeys.size() ; i++ )
    {
        int tag = GetTag( m_TagKeys[i] );

        string comp_list = GetTagNames( m_TagKeys[i] );

        // Find position of token _Surf
        int spos = comp_list.find( "_Surf" );

        string gname = comp_list.substr( 0, spos );

        string snum, ssnames, ssids;

        // Find position of first comma
        int cpos = comp_list.find( "," );
        if ( cpos != std::string::npos )
        {
            snum = comp_list.substr( spos + 5, cpos - ( spos + 5 ) );
            ssnames = comp_list.substr( cpos );
        }
        else
        {
            snum = comp_list.substr( spos + 5 );
        }

        string id_list = GetTagIDs( m_TagKeys[i] );

        // Find position of token _Surf
        spos = id_list.find( "_Surf" );
        string gid = id_list.substr( 0, spos );

        // Find position of first comma
        cpos = id_list.find( "," );
        if ( cpos != std::string::npos )
        {
            ssids = id_list.substr( cpos );
        }

        // Lookup Geom number
        int gnum = distance( gids.begin(), gids.find( gid ) );

        // Write tag number and surface list to file
        fprintf( fid, "%d,%d,%s,%s,%s", tag , gnum, snum.c_str(), gname.c_str(), gid.c_str() );

        // Write subsurface information if there is any
        if( !ssnames.empty() )
        {
            fprintf( fid, "%s%s\n", ssnames.c_str(), ssids.c_str() );
        }
        else
        {
            fprintf( fid, "\n" );
        }
    }
    fclose( fid );
}

//==== Write Key File ====//
void SubSurfaceMgrSingleton::WriteTKeyFile(const string & file_name )
{
    // figure out basename
    string base_name = file_name;
    std::string::size_type loc = base_name.find_last_of( "." );
    if ( loc != base_name.npos )
    {
        base_name = base_name.substr( 0, loc );
    }
    string key_name = base_name + ".tkey";

    FILE* fid = fopen( key_name.c_str(), "w" );
    if ( !fid )
    {
        return;
    }

    // Write Out Header Information
    fprintf( fid, "# VSP Tag Key File\n" );
    fprintf( fid, "%s\n", file_name.c_str() ); // Write out the file that this key information is for
    fprintf( fid, "%lu\n", m_SingleTagMap.size() - 1 ); // Total number of tags ( the minus 1 is from the dummy tags )
    fprintf( fid, "\n" );

    map< vector<int>, int >::iterator ii;
    map< int, string >::iterator si;

    for ( int i = 0 ; i < ( int )m_TagKeys.size() ; i++ )
    {
        string comp_list = GetTagNames( m_TagKeys[i] );
        int tag = GetTag( m_TagKeys[i] );

        // Write tag number and surface list to file
        fprintf( fid, "%d,%s\n", tag , comp_list.c_str() );

    }

    fclose( fid );

}

void SubSurfaceMgrSingleton::WriteNascartKeyFile( const string & filename )
{
    // Writes a Nascart Key File that is consistent between both CompGeom and CFDMesh

    if ( filename.size() == 0 ) { return; }

    FILE* fp = fopen( filename.c_str(), "w" );

    if ( fp )
    {
        fprintf( fp, "Color Name            BCType\n" );

        for ( int i = 0 ; i < ( int )m_TagKeys.size() ; i++ )
        {
            string comp_list = GetTagNames( m_TagKeys[i] );
            int tag = GetTag( m_TagKeys[i] );
            fprintf( fp, "%d.0  %s  0\n", tag, comp_list.c_str() );
        }

        fclose( fp );
    }

}

string SubSurfaceMgrSingleton::GetTagNames( const vector<int> & tags )
{
    string comp_list;
    map< int, string >::iterator si;

    int tag = GetTag( tags );

    if ( tag == -1 )
    {
        comp_list = "Error_Tag";
        return comp_list;
    }
    else
    {
        for ( int i = 0 ; i < ( int )tags.size() ; i++ )
        {
            si = m_TagNames.find( tags[i] );

            if ( si == m_TagNames.end() )
            {
                comp_list += ",Error_SubSurf";
            }
            else if ( si != m_TagNames.end() )
            {
                comp_list += "," + si->second ;
            }

            // Remove leading comma on first loop
            if ( i == 0 )
            { comp_list.erase( comp_list.begin(), comp_list.begin() + 1 ); }
        }
    }

    return comp_list;
}

string SubSurfaceMgrSingleton::GetTagNames( int indx )
{
    if ( indx < m_TagKeys.size() && indx >= 0 )
    {
        return GetTagNames( m_TagKeys[indx] );
    }
    return string( "Error_Tag" );
}

string SubSurfaceMgrSingleton::GetTagIDs( const vector<int>& tags )
{
    string comp_list;
    map< int, string >::iterator si;

    int tag = GetTag( tags );

    if ( tag == -1 )
    {
        comp_list = "Error_Tag";
        return comp_list;
    }
    else
    {
        for ( int i = 0; i < (int)tags.size(); i++ )
        {
            si = m_TagIDs.find( tags[i] );

            if ( si == m_TagIDs.end() )
            {
                comp_list += ",Error_SubSurf";
            }
            else if ( si != m_TagIDs.end() )
            {
                comp_list += "," + si->second;
            }

            // Remove leading comma on first loop
            if ( i == 0 )
            {
                comp_list.erase( comp_list.begin(), comp_list.begin() + 1 );
            }
        }
    }

    return comp_list;
}

string SubSurfaceMgrSingleton::GetTagIDs( int indx )
{
    if ( indx < m_TagKeys.size() && indx >= 0 )
    {
        return GetTagIDs( m_TagKeys[indx] );
    }
    return string( "Error_Tag" );
}

int SubSurfaceMgrSingleton::GetTag( const vector<int> & tags )
{
    map< vector<int>, int >::iterator mi;
    mi = m_SingleTagMap.find( tags );

    if ( mi != m_SingleTagMap.end() )
    {
        return mi->second;
    }
    else
    {
        return -1;
    }
}

std::vector< int > SubSurfaceMgrSingleton::GetAllTags()
{
    std::vector< int > ret;

    for ( int i = 0 ; i < ( int )m_TagKeys.size() ; i++ )
    {
        int tag = GetTag( m_TagKeys[i] );
        ret.push_back(tag);
    }
    return ret;
}
