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
#include <unordered_map>
#include <set>

struct PairGreaterLess
{
    bool operator() ( const std::pair< string, int >& a, const std::pair< string, int >& b ) const
    {
        if ( a.first != b.first )
        {
            return a.first > b.first;
        }
        return a.second < b.second;
    }
};

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
    void PartialClearTagMaps();
    void BuildCompNameMap();
    void BuildCompIDMap();
    void BuildSingleTagMap();
    int GetTag( const std::vector<int> & tags );
    int GetPart( const std::vector<int> & tags );
    int GetPart( int tag );
    std::vector< int > GetAllTags();

    // Returns number of tags +1 for dummy tag.
    unsigned int GetNumTags()
    {
        return m_SingleTagMap.size();
    }

    std::map< std::vector<int>, int > GetSingleTagMap() { return m_SingleTagMap; }
    std::vector< std::vector<int> > GetTagKeys() { return m_TagKeys; }

    int FindGNum( const string &gid );
    int FindGCNum( const string &gid, int s );

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

    bool MatchPartAndTag( const vector < int > & tags, int part, int tag );
    bool MatchPartAndTag( int singletag, int part, int tag );
    bool ExistPartAndTag( int part, int tag );
    void MakePartList( std::vector < int > & partvec );

    void GetPartData( vector < string > &gidvec, vector < int > &partvec, vector < int > &surfvec );

    std::set< std::vector<int> > m_TagCombos;
    std::unordered_map< int, std::string > m_TagNames;
    std::unordered_map< int, std::string > m_TagIDs;
    std::vector< std::string > m_CompNames;
    std::vector< std::string > m_CompIDs;
    std::vector< int > m_CompTypes;
    std::vector< int > m_CompPlate;
    std::vector< int > m_CompCopyIndex;
    std::vector< double > m_CompWmin;
    std::vector< double > m_CompUscale;
    std::vector< double > m_CompWscale;
    std::vector< int > m_CompThick;

protected:
    int m_CurrSurfInd;

    std::vector< std::vector<int> > m_TagKeys;
    std::map< std::vector<int>, int > m_SingleTagMap;

    std::set< string, greater< string > > m_GeomIDs;
    std::set < std::pair< string, int >, PairGreaterLess >  m_GeomCopySet;

    static Geom* GetGeom( const std::string &comp_id );
};

#define SubSurfaceMgr SubSurfaceMgrSingleton::GetInstance()

#endif
