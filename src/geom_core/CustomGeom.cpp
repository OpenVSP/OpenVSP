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

//==== Constructor ====//
CustomGeomMgrSingleton::CustomGeomMgrSingleton()
{
    m_ScriptDir = "./CustomScripts/";
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

}

//==== Init Geometry ====//
void CustomGeom::InitGeom( const string & module_name )
{
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
                crv_vec.push_back( xs->GetCurve() );
            }
        }

//        vector< VspPntData > tandata;
//        tandata.resize( crv_vec.size(), VspPntData( VspPntData::ZERO ) );

//      m_SurfVec[i].Interpolate( crv_vec, tandata, false );

        m_SurfVec[i].InterpolateLinear( crv_vec, false );
        m_SurfVec[i].SwapUWDirections();
    }
}
