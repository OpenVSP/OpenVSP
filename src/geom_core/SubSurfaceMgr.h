//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SubSurfaceMgr.h:
//
// SubSurfaceMgr manages adding and deleting sub-surfaces from components. However,
// the actual sub-surface vectors belong to Geom. If a Geom is destroyed, the Geom
// must delete the sub-surface vector and signal the SubSurfaceMgr that it was removed.
//
//
//
// Alex Gary
//
//////////////////////////////////////////////////////////////////////

#ifndef SUBSURFACE_MGR_INCLUDED_
#define SUBSURFACE_MGR_INCLUDED_

#include "SubSurface.h"
#include "Geom.h"
#include <vector>
#include <string>
#include <map>
#include <set>

class SubSurfaceMgrSingleton
{
private:
    SubSurfaceMgrSingleton();
    ~SubSurfaceMgrSingleton();

public:

    int GetCurrSurfInd()
    {
        return m_CurrSurfInd;
    }

    std::vector< SubSurface* > GetSubSurfs( const string& comp_id, int surfnum );
    std::vector< SubSurface* > GetSubSurfs( const string& comp_id );
    static std::vector< SubSurface* > GetSubSurfs();
    static SubSurface* GetSubSurf( const string& subsurf_id );

    void SetCurrSubSurfInd( int index );

    void SetSubSurfTags( int tag_offset );

    void PrepareToSplit();

    void ReSuffixGroupNames( const string& comp_id );

    static SubSurfaceMgrSingleton& GetInstance()
    {
        static SubSurfaceMgrSingleton instance;
        return instance;
    }

    // Manage tag maps
    void ClearTagMaps();
    void BuildCompNameMap();
    void BuildCompIDMap();
    void BuildSingleTagMap();
    int GetTag( const std::vector<int> & tags );
    std::vector< int > GetAllTags();

    // Returns number of tags +1 for dummy tag.
    unsigned int GetNumTags()
    {
        return m_SingleTagMap.size();
    }

    std::map< std::vector<int>, int > GetSingleTagMap() { return m_SingleTagMap; }

    // Write Tag Key File
    void WriteVSPGEOMKeyFile(const string & file_name );
    void WriteTKeyFile(const string & file_name );

    // Write Nascart Key File
    void WriteNascartKeyFile( const string & filename );

    // Get Comma Delimited list of names for a set of tags
    std::string GetTagNames( const std::vector<int> & tags );
    std::string GetTagNames( int indx );

    std::string GetTagIDs( const std::vector<int>& tags );
    std::string GetTagIDs( int indx );

    std::set< std::vector<int> > m_TagCombos;
    std::map< int, std::string > m_TagNames;
    std::map< int, std::string > m_TagIDs;
    std::vector< std::string > m_CompNames;
    std::vector< std::string > m_CompIDs;

protected:
    int m_CurrSurfInd;

    std::vector< std::vector<int> > m_TagKeys;
    std::map< std::vector<int>, int > m_SingleTagMap;

    static Geom* GetGeom( std::string comp_id );
};

#define SubSurfaceMgr SubSurfaceMgrSingleton::GetInstance()

#endif
