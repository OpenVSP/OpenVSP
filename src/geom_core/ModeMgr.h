//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ModeMgr.h: interface for the Mode Singleton.
//
//////////////////////////////////////////////////////////////////////

#if !defined(MODEMGR__INCLUDED_)
#define MODEMGR__INCLUDED_

#include "Parm.h"
#include "APIDefines.h"
#include "ParmContainer.h"

#include <vector>
#include <string>

using std::string;
using std::vector;


class Mode : public ParmContainer
{
public:
    Mode();
    ~Mode();


    void AddSetting( const pair < string, string > &s );
    pair < string, string > GetSetting( int indx );
    void RemoveSetting( int indx );
    void RemoveAllSettings();

    bool GroupHasSetting( const string & gidcheck );

    void ApplySettings();
    void ShowOnly();

    vector < pair < string, string > > GetAllSettings();
    void SetSettingVec( const vector < pair < string, string > > &sv );

    xmlNodePtr EncodeXml( xmlNodePtr &node );
    xmlNodePtr DecodeXml( xmlNodePtr &node );

    IntParm m_NormalSet;
    IntParm m_DegenSet;

protected:

    vector < pair < string, string > > m_GroupSettingVec;
};

class ModeMgrSingleton
{
public:
    static ModeMgrSingleton& getInstance()
    {
        static ModeMgrSingleton instance;
        return instance;
    }

    virtual ~ModeMgrSingleton();

    void Renew();
    void Wype();

    Mode * CreateAndAddMode();
    string CreateAndAddMode( const string & name );

    void DelAllModes();

    Mode *  GetMode( const string &id );
    Mode *  GetMode( int index );
    int GetNumModes() const;
    vector < string > GetAllModes();
    void DelMode( const int &i );
    void DelMode( const string &id );


    xmlNodePtr EncodeXml( xmlNodePtr &node ) const;
    xmlNodePtr DecodeXml( xmlNodePtr &node );

private:
    ModeMgrSingleton();
    ModeMgrSingleton( ModeMgrSingleton const& copy );             // Not Implemented
    ModeMgrSingleton& operator=( ModeMgrSingleton const& copy );  // Not Implemented

    vector < Mode* > m_ModeVec;

};

#define ModeMgr ModeMgrSingleton::getInstance()

#endif
