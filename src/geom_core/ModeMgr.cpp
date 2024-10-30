//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ModeMgr.cpp: Mode (Variable Preset and Set) Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "ModeMgr.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "StlHelper.h"
#include "VarPresetMgr.h"

//=========================================================================//
//==========================        Mode       ============================//
//=========================================================================//

Mode::Mode()
{
    m_NormalSet.Init( "NormalSet", "Sets", this, vsp::SET_ALL, vsp::SET_NONE, vsp::MAX_NUM_SETS - 1 );
    m_NormalSet.SetDescript( "Set number used in Mode for normal surface representation." );

    m_DegenSet.Init( "DegenSet", "Sets", this, vsp::SET_NONE, vsp::SET_NONE, vsp::MAX_NUM_SETS - 1 );
    m_DegenSet.SetDescript( "Set number used in Mode for degen surface representation." );
}

Mode::~Mode()
{
}

void Mode::AddSetting( const pair < string, string > &s )
{
    if ( !GroupHasSetting( s.first ) )
    {
        m_GroupSettingVec.push_back( s );
    }
}

pair < string, string > Mode::GetSetting( int indx )
{
    if ( indx >= 0 && indx < m_GroupSettingVec.size() )
    {
        return m_GroupSettingVec[ indx ];
    }
    pair < string, string > none;
    return none;
}

void Mode::RemoveSetting( int indx )
{
    if ( indx >= 0 && indx < m_GroupSettingVec.size() )
    {
        m_GroupSettingVec.erase( m_GroupSettingVec.begin() + indx );
    }
}

void Mode::RemoveAllSettings()
{
    m_GroupSettingVec.clear();
}

bool Mode::GroupHasSetting( const string & gidcheck )
{
    for ( int i = 0; i < m_GroupSettingVec.size(); i++ )
    {
        pair < string, string > set_pair = m_GroupSettingVec[ i ];
        string gid = set_pair.first;
        if ( gidcheck == gid )
        {
            return true;
        }
    }
    return false;
}

void Mode::ApplySettings()
{
    for ( int i = 0; i < m_GroupSettingVec.size(); i++ )
    {
        pair < string, string > set_pair = m_GroupSettingVec[ i ];
        string gid = set_pair.first;
        string sid = set_pair.second;

        SettingGroup * sg = VarPresetMgr.FindSettingGroup( gid );

        sg->ApplySetting( sid );
    }

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        veh->Update();
    }
}

void Mode::ShowOnly()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        veh->SetNormalDisplayType( m_NormalSet() );
        veh->SetDegenDisplayType( m_DegenSet() );

        veh->ShowOnlySet( m_NormalSet() );
        veh->ShowSet( m_DegenSet() );
    }
}

vector < pair < string, string > > Mode::GetAllSettings()
{
    return m_GroupSettingVec;
}

void Mode::SetSettingVec( const vector < pair < string, string > > &sv )
{
    m_GroupSettingVec = sv;
}

xmlNodePtr Mode::EncodeXml( xmlNodePtr &node )
{
    xmlNodePtr parmcontain_node = ParmContainer::EncodeXml( node );

    if ( parmcontain_node )
    {
        xmlNodePtr settings_node = xmlNewChild( parmcontain_node, NULL, BAD_CAST "Settings", NULL );

        if ( settings_node )
        {
            for ( int i = 0; i < m_GroupSettingVec.size(); i++ )
            {
                pair < string, string > set_pair = m_GroupSettingVec[i];

                xmlNodePtr setting_node = xmlNewChild( settings_node, NULL, BAD_CAST "Setting", NULL );
                if ( setting_node )
                {
                    XmlUtil::AddStringNode( setting_node, "SettingGroupID", set_pair.first );
                    XmlUtil::AddStringNode( setting_node, "SettingID", set_pair.second );
                }
            }
        }
    }

    return parmcontain_node;
}

xmlNodePtr Mode::DecodeXml( xmlNodePtr &node )
{
    xmlNodePtr parmcontain_node = ParmContainer::DecodeXml( node );

    if ( parmcontain_node )
    {
        xmlNodePtr settings_node = XmlUtil::GetNode( parmcontain_node, "Settings", 0 );

        if ( settings_node )
        {
            int num_set = XmlUtil::GetNumNames( settings_node, "Setting" );

            for ( int i = 0; i < num_set; i++ )
            {
                xmlNodePtr setting_node = XmlUtil::GetNode( settings_node, "Setting", i );

                if ( setting_node )
                {
                    pair < string, string > set_pair;
                    set_pair.first = ParmMgr.RemapID( XmlUtil::FindString( setting_node, "SettingGroupID", set_pair.first ) );
                    set_pair.second = ParmMgr.RemapID( XmlUtil::FindString( setting_node, "SettingID", set_pair.second ) );
                    m_GroupSettingVec.push_back( set_pair );
                }
            }
        }
    }

    return parmcontain_node;
}

//=========================================================================//
//====================        ModeMgrSingleton       ======================//
//=========================================================================//

ModeMgrSingleton::ModeMgrSingleton()
{
}

ModeMgrSingleton::~ModeMgrSingleton()
{
    Wype();
}

void ModeMgrSingleton::Renew()
{
    Wype();
}

void ModeMgrSingleton::Wype()
{
    for( int i = 0; i < ( int )m_ModeVec.size(); i++ )
    {
        delete m_ModeVec[i];
    }
    m_ModeVec.clear();
}

Mode * ModeMgrSingleton::CreateAndAddMode()
{
    Mode *mod = new Mode();
    m_ModeVec.push_back( mod );
    return mod;
}

string ModeMgrSingleton::CreateAndAddMode( const string & name )
{
    Mode *mod = CreateAndAddMode();
    mod->SetName( name );
    return mod->GetID();
}

void ModeMgrSingleton::DelAllModes()
{
    for( int i = 0; i < ( int )m_ModeVec.size(); i++ )
    {
        delete m_ModeVec[i];
    }
    m_ModeVec.clear();
}

Mode * ModeMgrSingleton::GetMode( const string &id )
{
    for( int i = 0; i < ( int )m_ModeVec.size(); i++ )
    {
        if ( m_ModeVec[i]->GetID() == id )
        {
            return m_ModeVec[i];
        }
    }

    return NULL;
}

Mode * ModeMgrSingleton::GetMode( int index )
{
    if ( index >= 0 && index < ( int )m_ModeVec.size() )
    {
        return m_ModeVec[ index ];
    }
    return nullptr;
}

int ModeMgrSingleton::GetNumModes() const
{
    return m_ModeVec.size();
}

vector < string > ModeMgrSingleton::GetAllModes()
{
    vector < string > ModeList( m_ModeVec.size() );

    for( int i = 0; i < ( int )m_ModeVec.size(); i++ )
    {
        ModeList[i] = m_ModeVec[i]->GetID();
    }

    return ModeList;
}

void ModeMgrSingleton::DelMode( const int & i )
{
    if ( i < 0 || i >= ( int )m_ModeVec.size() )
    {
        return;
    }

    Mode* Mode = m_ModeVec[i];

    m_ModeVec.erase( m_ModeVec.begin() +  i );

    delete Mode;
}

void ModeMgrSingleton::DelMode( const string &id )
{
    int idel = -1;
    for( int i = 0; i < ( int )m_ModeVec.size(); i++ )
    {
        if ( m_ModeVec[i]->GetID() == id )
        {
            idel = i;
            break;
        }
    }

    DelMode( idel );
}


xmlNodePtr ModeMgrSingleton::EncodeXml( xmlNodePtr &node ) const
{
    xmlNodePtr ModeMgr_node = xmlNewChild( node, NULL, BAD_CAST"Modes", NULL );

    for ( int i = 0; i < m_ModeVec.size(); i++ )
    {
        xmlNodePtr mode_node = xmlNewChild( ModeMgr_node, NULL, BAD_CAST "Mode", NULL );

        if ( mode_node )
        {
            m_ModeVec[i]->EncodeXml( mode_node );
        }
    }

    return ModeMgr_node;
}

xmlNodePtr ModeMgrSingleton::DecodeXml( xmlNodePtr &node )
{
    xmlNodePtr ModeMgr_node = XmlUtil::GetNode( node, "Modes", 0 );

    if ( ModeMgr_node )
    {
        int num_modes = XmlUtil::GetNumNames( ModeMgr_node, "Mode" );

        for ( unsigned int i = 0; i < num_modes; i++ )
        {
            xmlNodePtr mode_node = XmlUtil::GetNode( ModeMgr_node, "Mode", i );

            if ( mode_node )
            {
                Mode* mod = CreateAndAddMode();

                if ( mod )
                {
                    mod->DecodeXml( mode_node );
                }
            }
        }
    }

    return node;
}
