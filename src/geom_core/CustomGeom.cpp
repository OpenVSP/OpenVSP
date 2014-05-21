//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CustomGeom.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "VspSurf.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "VSP_Geom_API.h"
#include "ScriptMgr.h"
#include "FileUtil.h"

#include "APIDefines.h"
using namespace vsp;

//==== Constructor ====//
CustomGeomMgrSingleton::CustomGeomMgrSingleton()
{
//    m_ScriptDir = "./CustomScripts/";
    m_ScriptDir = "../../../CustomScripts/";
}

//==== Scan Custom Directory And Return All Possible Types ====//
void CustomGeomMgrSingleton::ReadCustomScripts()
{
    m_CustomTypeVec.clear();
    vector< string > file_vec = ScanFolder( m_ScriptDir.c_str() );

    for ( int i = 0 ; i < ( int )file_vec.size() ; i++ )
    {
        if ( file_vec[i].compare( file_vec[i].size() - 3, 3, ".as" ) == 0 )
        {
            string sub = file_vec[i].substr( 0, file_vec[i].size() - 3 );
            m_CustomTypeVec.push_back( GeomType( CUSTOM_GEOM_TYPE, sub, false ) );

            //jrg CHheck for errors
            string file_name = m_ScriptDir;
            file_name.append( file_vec[i] );
            ScriptMgr.ReadScript( sub.c_str(), file_name.c_str()  );
        }
    }
}

//==== Init Custom Geom ====//
void CustomGeomMgrSingleton::InitGeom( const string& id )
{
    Vehicle* veh = VehicleMgr::getInstance().GetVehicle();
    Geom* gptr = veh->FindGeom( id );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        m_CurrGeom = id;
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );

        string module_name = gptr->GetType().m_Name;

        custom_geom->InitGeom( module_name );
    }
}

//==== Add Parm To Current Custom Geom ====//
string CustomGeomMgrSingleton::AddParm( int type, const string & name, const string & group )
{
    Vehicle* veh = VehicleMgr::getInstance().GetVehicle();
    Geom* gptr = veh->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );

        return custom_geom->AddParm( type, name, group );
    }

    return string();
}

string CustomGeomMgrSingleton::GetCustomParm( int index )
{
    Geom* gptr = VehicleMgr::getInstance().GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        return custom_geom->FindParmID( index );
    }
    return string();
}


//==== Add Gui Device Build Data For Custom Geom ====//
int CustomGeomMgrSingleton::AddGui( int type, const string & label )
{
   Geom* gptr = VehicleMgr::getInstance().GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );

        GuiDef gd;
        gd.m_Type = type;
        gd.m_Label = label;
        return custom_geom->AddGui( gd );
    }
    return -1;
}

//==== Update Gui ====//
void CustomGeomMgrSingleton::AddUpdateGui( int gui_id, const string & parm_id )
{
    Geom* gptr = VehicleMgr::getInstance().GetVehicle()->FindGeom( m_CurrGeom );
    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );

        GuiUpdate gu;
        gu.m_GuiID = gui_id;
        gu.m_ParmID = parm_id;

        custom_geom->AddUpdateGui( gu );
    }
}

//==== Get GuiDef =====//
vector< GuiDef > CustomGeomMgrSingleton::GetGuiDefVec( const string & geom_id )
{
    SetCurrCustomGeom( geom_id );
    Geom* gptr = VehicleMgr::getInstance().GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );

        return custom_geom->GetGuiDefVec();
    }
    vector< GuiDef > defvec;
    return defvec;
}


//==== Build Update Gui Instruction Vector ====//
vector< GuiUpdate > CustomGeomMgrSingleton::GetGuiUpdateVec()
{
    Geom* gptr = VehicleMgr::getInstance().GetVehicle()->FindGeom( m_CurrGeom );
    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );

        return custom_geom->GetGuiUpdateVec();
    }
    vector< GuiUpdate > defvec;
    return defvec;
}

//==== Add XSec Surface To Current Geom - Return ID =====//
string CustomGeomMgrSingleton::AddXSecSurf()
{
    Geom* gptr = VehicleMgr::getInstance().GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        return custom_geom->AddXSecSurf();
    }
    return string();
}

//==== Skin XSec Surf =====//
void CustomGeomMgrSingleton::SkinXSecSurf()
{
    Geom* gptr = VehicleMgr::getInstance().GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        custom_geom->SkinXSecSurf();
    }
}

//==== Custom XSecs Functions =====//
void CustomGeomMgrSingleton::SetCustomXSecLoc( const string & xsec_id, const vec3d & loc )
{
    ParmContainer* pc = ParmMgr.FindParmContainer( xsec_id );

    if ( !pc )
        return;

    CustomXSec* cxs = dynamic_cast<CustomXSec*>( pc );
    if ( !cxs )
        return;

    cxs->SetLoc( loc );
}


//==================================================================================================//
//==================================================================================================//
//==================================================================================================//
//==================================================================================================//
//==== Constructor ====//
CustomXSec::CustomXSec( XSecCurve *xsc, bool use_left ) : XSec( xsc, use_left)
{
    m_Type = vsp::XSEC_CUSTOM;
};


//==== Update ====//
void CustomXSec::Update()
{

    m_Type = XSEC_CUSTOM;

    m_LateUpdateFlag = false;

    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();

    // apply the needed transformation to get section into body orientation
    Matrix4d mat;
    xsecsurf->GetBasicTransformation( m_XSCurve->GetWidth(), mat );

    VspCurve baseCurve = GetUntransformedCurve();

    baseCurve.Transform( mat );

    //==== Apply Transform ====//
    m_TransformedCurve = baseCurve;

    Matrix4d tran_mat;
    tran_mat.translatef( m_Loc.x(),  m_Loc.y(),  m_Loc.z() );

    Matrix4d rotate_mat;
    rotate_mat.rotateX( m_Rot.x() );
    rotate_mat.rotateY( m_Rot.y() );
    rotate_mat.rotateZ( m_Rot.z() );

    Matrix4d cent_mat;
    cent_mat.translatef( -m_CenterRot.x(), -m_CenterRot.y(), -m_CenterRot.z() );

    Matrix4d inv_cent_mat;
    inv_cent_mat.translatef( m_CenterRot.x(), m_CenterRot.y(), m_CenterRot.z() );

    m_Transform.loadIdentity();

    m_Transform.postMult( tran_mat.data() );
    m_Transform.postMult( cent_mat.data() );
    m_Transform.postMult( rotate_mat.data() );
    m_Transform.postMult( inv_cent_mat.data() );

    m_Transform.postMult( xsecsurf->GetGlobalXForm().data() );

    m_TransformedCurve.Transform( m_Transform );
}

//==== Copy position from base class ====//
void CustomXSec::CopyBasePos( XSec* xs )
{
    if ( xs )
    {
        CustomXSec* cxs = dynamic_cast< CustomXSec* > (xs );
        if ( cxs )
        {
            m_Loc = cxs->m_Loc;
            m_Rot = cxs->m_Rot;
            m_CenterRot = cxs->m_CenterRot;
        }
    }
}

//==== Set XSec Location - Not Using Parms To Avoid Exposing Dependant Vars ====//
void CustomXSec::SetLoc( const vec3d & loc )
{
    m_Loc = loc;
    m_LateUpdateFlag = true;
}

//==== Set XSec Rotation - Not Using Parms To Avoid Exposing Dependant Vars ====//
void CustomXSec::SetRot( const vec3d & rot )
{
    m_Rot = rot;
    m_LateUpdateFlag = true;
}

//==== Set XSec Center Rotation - Not Using Parms To Avoid Exposing Dependant Vars ====//
void CustomXSec::SetCenterRot( const vec3d & cent )
{
    m_CenterRot = cent;
    m_LateUpdateFlag = true;
}



//==================================================================================================//
//==================================================================================================//
//==================================================================================================//
//==================================================================================================//


//==== Constructor ====//
CustomGeom::CustomGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_InitGeomFlag = false;
    m_Name = "CustomGeom";
    m_Type.m_Name = "Custom";
    m_Type.m_Type = CUSTOM_GEOM_TYPE;



}

//==== Destructor ====//
CustomGeom::~CustomGeom()
{
    Clear();
}

void CustomGeom::Clear()
{
    //==== Clear Parms ====//
    for ( int i = 0 ; i < (int)m_ParmVec.size() ; i++ )
    {
        delete m_ParmVec[i];
    }
    m_ParmVec.clear();

    //==== Clear XSecs ====//
    for ( int i = 0 ; i < (int)m_XSecSurfVec.size() ; i++ )
    {
        delete m_XSecSurfVec[i];
    }
    m_XSecSurfVec.clear();
}

//==== Init Geometry ====//
void CustomGeom::InitGeom( const string & module_name )
{
    Clear();
    m_InitGeomFlag = true;
    SetScriptModuleName( module_name.c_str() );
    ScriptMgr.ExecuteScript( GetScriptModuleName().c_str(), "void Init()" );
    ScriptMgr.ExecuteScript( GetScriptModuleName().c_str(), "void InitGui()" );
    SetName( module_name );
    Update();

}

//==== Add Gui Definition ====//
int CustomGeom::AddGui( const GuiDef & gd )
{
    m_GuiDefVec.push_back( gd );
    return m_GuiDefVec.size() - 1;
}

//==== Add Parm->Gui Match ====//
void CustomGeom::AddUpdateGui( const GuiUpdate & gu )
{
    m_UpdateGuiVec.push_back( gu );
}

//==== Match Parm to GUI by Execting UpdateGui Script ====//
vector< GuiUpdate > CustomGeom::GetGuiUpdateVec()
{
    m_UpdateGuiVec.clear();

    //==== Call Script ====//
    ScriptMgr.ExecuteScript( GetScriptModuleName().c_str(), "void UpdateGui()" );

    return m_UpdateGuiVec;
}


//==== Update Surf By Executing Update Surf Script ====//
void CustomGeom::UpdateSurf()
{
    if ( !m_InitGeomFlag )
    {
        return;
    }

    CustomGeomMgr.SetCurrCustomGeom( GetID() );

    //==== Call Script ====//
    ScriptMgr.ExecuteScript( GetScriptModuleName().c_str(), "void UpdateSurf()" );

}

//==== Create Parm and Add To Vector Of Parms ====//
string CustomGeom::AddParm( int type, const string & name, const string & group )
{
    Parm* p = NULL;
    if ( type == PARM_DOUBLE_TYPE )
    {
        p = new Parm();
    }
    else if ( type == PARM_INT_TYPE )
    {
        p = new IntParm();
    }
    else if ( type == PARM_BOOL_TYPE )
    {
        p = new BoolParm();
    }
    else if ( type == PARM_FRACTION_TYPE )
    {
        p = new FractionParm();
    }

    if ( p )
    {
        p->Init( name, group, this, 0.0, -1.0e6, 1.0e6, true );
        p->SetDescript( "Custom Descript" );
        m_ParmVec.push_back( p );
        return p->GetID();
    }
    return string();
}

//==== Find Parm String Based On Index ===//
string CustomGeom::FindParmID( int index )
{
    if ( index >= 0 && index < ( int )m_ParmVec.size() )
    {
        return m_ParmVec[index]->GetID();

    }
    return string();
}


//==== Add XSec Surface Return ID =====//
string CustomGeom::AddXSecSurf()
{
    XSecSurf* xsec_surf = new XSecSurf();

    xsec_surf->SetXSecType( XSEC_CUSTOM );

    xsec_surf->SetBasicOrientation( X_DIR, Y_DIR, XS_SHIFT_MID, false );

    xsec_surf->SetParentContainer( GetID() );

    m_XSecSurfVec.push_back( xsec_surf );

    return xsec_surf->GetID();
}

//==== Get XSecSurf At Index =====//
XSecSurf* CustomGeom::GetXSecSurf( int index )
{
    if ( index < 0 || index >= ( int )m_XSecSurfVec.size() )
    {
        return NULL;
    }
    return m_XSecSurfVec[index];
}


//==== Skin XSec Surfs ====//
void CustomGeom::SkinXSecSurf()
{
    m_SurfVec.resize( m_XSecSurfVec.size() );
    assert( m_XSecSurfVec.size() == m_SurfVec.size() );

    for ( int i = 0 ; i < ( int )m_XSecSurfVec.size() ; i++ )
    {
        vector< VspCurve > crv_vec;

        //==== Update XSec Location/Rotation ====//
       for ( int j = 0 ; j < m_XSecSurfVec[i]->NumXSec() ; j++ )
        {
            XSec* xs = m_XSecSurfVec[i]->FindXSec( j );
            if ( xs )
            {
                xs->SetLateUpdateFlag( true );

                VspCurve crv = xs->GetCurve();
                //==== Check If Curve Exactly Matches Procedding Curve ====//
                if ( crv_vec.size() )
                {
                    if ( !crv_vec.back().IsEqual( crv ) )
                        crv_vec.push_back(crv );
                }
                else
                {
                    crv_vec.push_back( crv );
                }
            }
        }
        if ( crv_vec.size() >= 2 )
        {
            m_SurfVec[i].InterpolateLinear( crv_vec, false );
            m_SurfVec[i].SwapUWDirections();
        }
    }
}


//==== Encode Data Into XML Data Struct ====//
xmlNodePtr CustomGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );
    xmlNodePtr custom_node = xmlNewChild( node, NULL, BAD_CAST "CustomGeom", NULL );
    if ( custom_node )
    {
        string file_contents;
        string file_name = ScriptMgr.FindModuleFileName( GetScriptModuleName() );
        FILE* fp = fopen( file_name.c_str(), "r" );
        if ( fp )
        {
            char buff[256];
            while ( fgets( buff, 256, fp ) )
            {
                file_contents.append( buff );
            }
            file_contents.append( "\0" );
            fclose( fp );
        }

        for ( int i = 0 ; i < (int)m_ParmVec.size() ; i++ )
        {
            m_ParmVec[i]->EncodeXml( custom_node );
        }

        XmlUtil::AddStringNode( custom_node, "ScriptFileModule", GetScriptModuleName()  );
        XmlUtil::AddStringNode( custom_node, "ScriptFileName", file_name );
        XmlUtil::AddStringNode( custom_node, "ScriptFileContents", file_contents );
    }

    return custom_node;
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr CustomGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr custom_node = XmlUtil::GetNode( node, "CustomGeom", 0 );
    if ( custom_node )
    {
        string module_name = XmlUtil::FindString( custom_node, "ScriptFileModule", GetScriptModuleName() );
        string file_name   = XmlUtil::FindString( custom_node, "ScriptFileName", string() );
        string file_contents = XmlUtil::FindString( custom_node, "ScriptFileContents", string() );

         CustomGeomMgr.InitGeom( GetID() );

        for ( int i = 0 ; i < (int)m_ParmVec.size() ; i++ )
        {
            m_ParmVec[i]->DecodeXml( custom_node );
        }


    }

    return custom_node;
}
