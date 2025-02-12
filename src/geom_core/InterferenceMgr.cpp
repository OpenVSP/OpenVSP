//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// InterferenceMgr.cpp
//
// Rob McDonald
//////////////////////////////////////////////////////////////////////

#include "InterferenceMgr.h"

#include "ModeMgr.h"
#include "StlHelper.h"
#include "Vehicle.h"
#include "ParmMgr.h"
#include "SnapTo.h"

InterferenceCase::InterferenceCase()
{
    string groupname = "InterferenceCase";

    m_PrimarySet.Init( "PrimarySet", groupname, this, DEFAULT_SET, 0, vsp::MAX_NUM_SETS );
    m_PrimarySet.SetDescript( "Selected primary set for operation" );

    m_PrimaryType.Init( "PrimaryType", "Projection", this, vsp::SET_TARGET, vsp::SET_TARGET, vsp::NUM_PROJ_TGT_OPTIONS - 1 );


    m_SecondarySet.Init( "SecondarySet", groupname, this, DEFAULT_SET, 0, vsp::MAX_NUM_SETS );
    m_SecondarySet.SetDescript( "Selected secondary set for operation" );

    m_SecondaryType.Init( "SecondaryType", "Projection", this, vsp::SET_TARGET, vsp::SET_TARGET, vsp::NUM_PROJ_BNDY_OPTIONS - 2 ); // Note - 2, MODE_TARGET not allowed.

    m_IntererenceCheckType.Init( "IntererenceCheckType", groupname, this, vsp::EXTERNAL_INTERFERENCE, vsp::EXTERNAL_INTERFERENCE, vsp::NUM_INTERFERENCE_TYPES - 1 );

    m_LastResultValue.Init( "LastResult", groupname, this, 0.0, -1e12, 1e12 );
}

void InterferenceCase::Update()
{
}

string InterferenceCase::GetPrimaryName() const
{
    if ( m_PrimaryType() == vsp::SET_TARGET )
    {
        Vehicle *veh = VehicleMgr.GetVehicle();
        if ( veh )
        {
            vector < string > setNameVec = veh->GetSetNameVec();
            int pset = m_PrimarySet();
            if ( pset >= 0 && pset < setNameVec.size() )
            {
                return setNameVec[ pset ];
            }
        }
    }
    else if ( m_PrimaryType() == vsp::GEOM_TARGET )
    {
        Vehicle *veh = VehicleMgr.GetVehicle();
        if ( veh )
        {
            Geom *g = veh->FindGeom( m_PrimaryGeomID );
            if ( g )
            {
                return g->GetName();
            }
        }
    }
    else if ( m_PrimaryType() == vsp::MODE_TARGET )
    {
        Mode *m = ModeMgr.GetMode( m_PrimaryModeID );
        if ( m )
        {
            return m->GetName();
        }
    }

    return string();
}

string InterferenceCase::GetSecondaryName() const
{
    if ( m_SecondaryType() == vsp::SET_TARGET )
    {
        Vehicle *veh = VehicleMgr.GetVehicle();
        if ( veh )
        {
            vector < string > setNameVec = veh->GetSetNameVec();
            int pset = m_SecondarySet();
            if ( pset >= 0 && pset < setNameVec.size() )
            {
                return setNameVec[ pset ];
            }
        }
    }
    else if ( m_SecondaryType() == vsp::GEOM_TARGET )
    {
        Vehicle *veh = VehicleMgr.GetVehicle();
        if ( veh )
        {
            Geom *g = veh->FindGeom( m_SecondaryGeomID );
            if ( g )
            {
                return g->GetName();
            }
        }
    }

    return string();
}

vector< TMesh* > InterferenceCase::GetPrimaryTMeshVec()
{
    vector< TMesh* > tmv;

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        if ( m_PrimaryType() == vsp::SET_TARGET || m_PrimaryType() == vsp::MODE_TARGET )
        {
            int set = vsp::SET_NONE;

            if ( m_PrimaryType() == vsp::SET_TARGET )
            {
                set = m_PrimarySet();
            }
            else
            {
                Mode *m = ModeMgr.GetMode( m_PrimaryModeID );
                if ( m )
                {
                    set = m->m_NormalSet();
                }
            }

            tmv = veh->CreateTMeshVec( set );
        }
        else if ( m_PrimaryType() == vsp::GEOM_TARGET )
        {
            tmv = veh->CreateTMeshVec( m_PrimaryGeomID );
        }
    }

    return tmv;
}

vector< TMesh* > InterferenceCase::GetSecondaryTMeshVec()
{
    vector< TMesh* > tmv;
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        if ( m_SecondaryType() == vsp::SET_TARGET )
        {
            int set = m_SecondarySet();

            tmv = veh->CreateTMeshVec( set );
        }
        else if ( m_SecondaryType() == vsp::GEOM_TARGET )
        {
            tmv = veh->CreateTMeshVec( m_SecondaryGeomID );
        }
    }

    return tmv;
}

xmlNodePtr InterferenceCase::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr icase_node = xmlNewChild( node, NULL, BAD_CAST"InterferenceCase", NULL );

    ParmContainer::EncodeXml( icase_node );
    XmlUtil::AddStringNode( icase_node, "PrimaryModeID", m_PrimaryModeID );
    XmlUtil::AddStringNode( icase_node, "PrimaryGeomID", m_PrimaryGeomID );
    XmlUtil::AddStringNode( icase_node, "SecondaryGeomID", m_SecondaryGeomID );

    return icase_node;
}

xmlNodePtr InterferenceCase::DecodeXml( xmlNodePtr & node )
{
    ParmContainer::DecodeXml( node );
    m_PrimaryModeID = ParmMgr.RemapID( XmlUtil::FindString( node, "PrimaryModeID", m_PrimaryModeID ) );
    m_PrimaryGeomID = ParmMgr.RemapID( XmlUtil::FindString( node, "PrimaryGeomID", m_PrimaryGeomID ) );
    m_SecondaryGeomID = ParmMgr.RemapID( XmlUtil::FindString( node, "SecondaryGeomID", m_SecondaryGeomID ) );

    return node;
}

string InterferenceCase::Evaluate()
{
    m_LastResultValue = 1.0;
    m_LastResult.clear();

    DeleteTMeshVec( m_TMeshVec );
    m_PtsVec.clear();

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        if ( m_PrimaryType() == vsp::MODE_TARGET )
        {
            Mode *m = ModeMgr.GetMode( m_PrimaryModeID );
            if ( m )
            {
                m->ApplySettings();
                // fullupdate = false skips feature lines, subsurfaces, and structured tessellation.
                veh->Update( false );
            }
        }

        vector< TMesh* > primary_tmv = GetPrimaryTMeshVec();
        vector< TMesh* > secondary_tmv = GetSecondaryTMeshVec();

        switch ( m_IntererenceCheckType() )
        {
            case vsp::EXTERNAL_INTERFERENCE:
            {
                m_LastResult = ExteriorInterferenceCheck( primary_tmv, secondary_tmv, m_TMeshVec );
                break;
            }
            case vsp::PACKAGING_INTERFERENCE:
            {
                m_LastResult = PackagingInterferenceCheck( primary_tmv, secondary_tmv, m_TMeshVec );
                break;
            }
        }

        DeleteTMeshVec( primary_tmv );
        DeleteTMeshVec( secondary_tmv );


        m_PtsVec = ResultsMgr.GetVec3dResults( m_LastResult, "Pts", 0 );
        vector < double > resvec = ResultsMgr.GetDoubleResults( m_LastResult, "Result", 0 );
        if ( resvec.size() > 0 )
        {
            m_LastResultValue = resvec[0];
        }

    }

    UpdateDrawObj();

    return m_LastResult;
}

void InterferenceCase::UpdateDrawObj()
{
    Material mat;
    mat.SetMaterial( "Red Default" );
    mat.m_Diff[3] = 0.25; // Make translucent

    m_MeshResultDO_vec.resize( m_TMeshVec.size(), DrawObj() );

    for ( int i = 0 ; i < ( int )m_TMeshVec.size() ; i++ )
    {
        unsigned int num_tris = m_TMeshVec[i]->m_TVec.size();

        unsigned int pi = 0;
        vector<TTri*>& tris = m_TMeshVec[i]->m_TVec;
        m_MeshResultDO_vec[i].m_PntVec.resize( num_tris * 3 );
        m_MeshResultDO_vec[i].m_NormVec.resize( num_tris * 3 );
        for ( int t = 0 ; t < ( int ) num_tris ; t++ )
        {
            m_MeshResultDO_vec[i].m_PntVec[pi] = tris[t]->m_N0->m_Pnt;
            m_MeshResultDO_vec[i].m_PntVec[pi + 1] = tris[t]->m_N1->m_Pnt;
            m_MeshResultDO_vec[i].m_PntVec[pi + 2] = tris[t]->m_N2->m_Pnt;
            m_MeshResultDO_vec[i].m_NormVec[pi] = tris[t]->m_Norm;
            m_MeshResultDO_vec[i].m_NormVec[pi + 1] = tris[t]->m_Norm;
            m_MeshResultDO_vec[i].m_NormVec[pi + 2] = tris[t]->m_Norm;
            pi += 3;
        }

        // Flag the DrawObjects as changed
        m_MeshResultDO_vec[i].m_GeomChanged = true;

        for ( int j = 0; j < 4; j++ )
        {
            m_MeshResultDO_vec[i].m_MaterialInfo.Ambient[j] = (float)mat.m_Ambi[j];
            m_MeshResultDO_vec[i].m_MaterialInfo.Diffuse[j] = (float)mat.m_Diff[j];
            m_MeshResultDO_vec[i].m_MaterialInfo.Specular[j] = (float)mat.m_Spec[j];
            m_MeshResultDO_vec[i].m_MaterialInfo.Emission[j] = (float)mat.m_Emis[j];
        }
        m_MeshResultDO_vec[i].m_MaterialInfo.Shininess = (float)mat.m_Shininess;

        m_MeshResultDO_vec[i].m_LineColor = DrawObj::Color( DrawObj::RED );

        char str[255];
        snprintf( str, sizeof( str ),  "_%d", i );
        m_MeshResultDO_vec[i].m_GeomID = m_ID + str;
        m_MeshResultDO_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_MeshResultDO_vec[i].m_Type = DrawObj::VSP_WIRE_SHADED_TRIS;
    }

    m_LineResultDO.m_GeomID = m_ID + "Line";
    m_LineResultDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_LineResultDO.m_Type = DrawObj::VSP_LINES;
    m_LineResultDO.m_LineColor = DrawObj::Color( DrawObj::RED );
    m_LineResultDO.m_LineWidth = 3.0;

    m_LineResultDO.m_GeomChanged = true;
    m_LineResultDO.m_PntVec = m_PtsVec;
}


void InterferenceCase::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    for ( int i = 0 ; i < ( int )m_MeshResultDO_vec.size() ; i++ )
    {
        m_MeshResultDO_vec[i].m_Visible = true;
        draw_obj_vec.push_back( &m_MeshResultDO_vec[i] );
    }

    m_LineResultDO.m_Visible = true;
    draw_obj_vec.push_back( &m_LineResultDO );
}

//===============================================================================//
//===============================================================================//
//===============================================================================//


InterferenceMgrSingleton::InterferenceMgrSingleton()
{

}

InterferenceMgrSingleton::~InterferenceMgrSingleton()
{
    Wype();
}

xmlNodePtr InterferenceMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr interferencemgr_node = xmlNewChild( node, NULL, BAD_CAST"InterferenceMgr", NULL );

    if ( interferencemgr_node )
    {
        for ( int i = 0; i < m_ICaseVec.size(); i++ )
        {
            m_ICaseVec[i]->EncodeXml( interferencemgr_node );
        }
    }

    return interferencemgr_node;
}

xmlNodePtr InterferenceMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr interferencemgr_node = XmlUtil::GetNode( node, "InterferenceMgr", 0 );

    if ( interferencemgr_node )
    {


        int num_icase = XmlUtil::GetNumNames( interferencemgr_node, "InterferenceCase" );
        for ( int i = 0; i < num_icase; i++ )
        {
            xmlNodePtr icasenode = XmlUtil::GetNode( interferencemgr_node, "InterferenceCase", i );

            if ( icasenode )
            {
                string id = AddInterferenceCase();
                InterferenceCase* icase = GetInterferenceCase( id );

                if ( icase )
                {
                    icase->DecodeXml( icasenode );
                }
            }
        }
    }

    return node;
}

void InterferenceMgrSingleton::Renew()
{
    Wype();
}

void InterferenceMgrSingleton::Wype()
{
    for ( int i = 0; i < (int)m_ICaseVec.size(); i++ )
    {
        delete m_ICaseVec[i];
    }
    m_ICaseVec.clear();
}

void InterferenceMgrSingleton::Update()
{
    for ( int i = 0; i < (int)m_ICaseVec.size(); i++ )
    {
        m_ICaseVec[i]->Update();
    }

}

string InterferenceMgrSingleton::EvaluateAll()
{
    std::vector <string> res_id_vector;

    for ( int i = 0; i < (int)m_ICaseVec.size(); i++ )
    {
        string rid = m_ICaseVec[i]->Evaluate();
        res_id_vector.push_back( rid );
    }

    Results *res = ResultsMgr.CreateResults( "InterferenceCheckAll", "All interference check results for model." );
    if( res )
    {
        res->Add( new NameValData( "ResultsVec", res_id_vector, "ID's of interference check analysis results." ) );
        return res->GetID();
    }
    return string();
}

void InterferenceMgrSingleton::AddLinkableContainers( vector< string > & linkable_container_vec )
{
    for ( int i = 0; i < (int)m_ICaseVec.size(); i++ )
    {
        if ( m_ICaseVec[i] )
        {
            m_ICaseVec[i]->AddLinkableContainers( linkable_container_vec );
        }
    }
}

string InterferenceMgrSingleton::AddInterferenceCase()
{
    InterferenceCase* ic = new InterferenceCase();

    m_ICaseVec.push_back( ic );
    return ic->GetID();
}

void InterferenceMgrSingleton::DeleteInterferenceCase( const string &id )
{
    int indx = GetInterferenceCaseIndex( id );
    DeleteInterferenceCase( indx );
}

void InterferenceMgrSingleton::DeleteInterferenceCase( int indx )
{
    if ( indx >= 0 && indx < m_ICaseVec.size() )
    {
        delete m_ICaseVec[ indx ];

        m_ICaseVec.erase( m_ICaseVec.begin() + indx );
    }
}

void InterferenceMgrSingleton::DeleteAllInterferenceCases()
{
    for ( int i = 0; i < (int)m_ICaseVec.size(); i++ )
    {
        delete m_ICaseVec[i];
    }
    m_ICaseVec.clear();
}

InterferenceCase * InterferenceMgrSingleton::GetInterferenceCase( int indx ) const
{
    if ( indx >= 0 && indx < m_ICaseVec.size() )
    {
        return m_ICaseVec[ indx ];
    }
    return nullptr;
}

InterferenceCase * InterferenceMgrSingleton::GetInterferenceCase( const string &id ) const
{
    for ( int i = 0; i < (int)m_ICaseVec.size(); i++ )
    {
        if ( m_ICaseVec[i]->GetID() == id )
        {
            return m_ICaseVec[i];
        }
    }

    return nullptr;
}

int InterferenceMgrSingleton::GetInterferenceCaseIndex( const string &id ) const
{
    for ( int i = 0; i < (int)m_ICaseVec.size(); i++ )
    {
        if ( m_ICaseVec[i]->GetID() == id )
        {
            return i;
        }
    }

    return -1;
}
