//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CfdMeshSettings.cpp
//
//////////////////////////////////////////////////////////////////////

#include "Geom.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "Util.h"
#include "SubSurfaceMgr.h"
#include "SubSurface.h"
#include "CfdMeshSettings.h"
#include "ParmMgr.h"

#ifdef DEBUG_CFD_MESH
#include <direct.h>
#endif

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

CfdMeshSettings::CfdMeshSettings() : ParmContainer()
{
    m_Name = "CFDMeshSettings";

    m_DrawMeshFlag.Init( "DrawMesh", "DrawCFD", this, true, 0, 1 );
    m_DrawSourceFlag.Init( "DrawSourceWake", "DrawCFD", this, true, 0, 1 );
    m_DrawFarFlag.Init( "DrawFarField", "DrawCFD", this, true, 0, 1 );
    m_DrawFarPreFlag.Init( "DrawFarFieldPreview", "DrawCFD", this, true, 0, 1 );
    m_DrawBadFlag.Init( "DrawBadMeshElements", "DrawCFD", this, true, 0, 1 );
    m_DrawSymmFlag.Init( "DrawSymmetryPlane", "DrawCFD", this, true, 0, 1 );
    m_DrawWakeFlag.Init( "DrawWake", "DrawCFD", this, true, 0, 1 );
    m_ColorTagsFlag.Init( "ColorTagsFlag", "DrawCFD", this, true, 0, 1 );

    m_HalfMeshFlag.Init( "HalfMesh", "FarField", this, false, 0, 1 );
    m_FarCompFlag.Init( "FarComp", "FarField", this, false, 0, 1 );
    m_FarMeshFlag.Init( "FarMesh", "FarField", this, false, 0, 1 );

    //Symmetry Plane Splitting Default Value: OFF
    m_SymSplittingOnFlag.Init( "SymmetrySplitting", "FarField", this, false, 0, 1);

    m_FarManLocFlag.Init( "FarManualLoc", "FarField", this, false, 0, 1 );
    m_FarAbsSizeFlag.Init( "FarAbsSize", "FarField", this, false, 0, 1 );

    m_FarXScale.Init( "FarXScale", "FarField", this, 4.0, 1.0, 1.0e12 );
    m_FarXScale.SetDescript( "Far field X scale" );

    m_FarYScale.Init( "FarYScale", "FarField", this, 4.0, 1.0, 1.0e12 );
    m_FarYScale.SetDescript( "Far field Y scale" );

    m_FarZScale.Init( "FarZScale", "FarField", this, 4.0, 1.0, 1.0e12 );
    m_FarZScale.SetDescript( "Far field Z scale" );

    m_FarLength.Init( "FarLength", "FarField", this, 4.0, 1.1, 1.0e12 );
    m_FarLength.SetDescript( "Far field length" );
    m_FarWidth.Init( "FarWidth", "FarField", this, 4.0, 1.0, 1.0e12 );
    m_FarWidth.SetDescript( "Far field width" );
    m_FarHeight.Init( "FarHeight", "FarField", this, 4.0, 1.1, 1.0e12 );
    m_FarHeight.SetDescript( "Far field height" );

    m_FarXLocation.Init( "FarXLocation", "FarField", this, 0.0, -1.0e12, 1.0e12 );
    m_FarXLocation.SetDescript( "Far field X location" );

    m_FarYLocation.Init( "FarYLocation", "FarField", this, 0.0, -1.0e12, 1.0e12 );
    m_FarYLocation.SetDescript( "Far field Y location" );

    m_FarZLocation.Init( "FarZLocation", "FarField", this, 0.0, -1.0e12, 1.0e12 );
    m_FarZLocation.SetDescript( "Far field Z location" );

    m_WakeScale.Init( "WakeScale", "Wake", this, 2.0, 1.0, 1.0e12 );
    m_WakeScale.SetDescript( "Wake length scale" );

    m_WakeAngle.Init( "WakeAngle", "Wake", this, 0.0, -90.0, 90.0 );
    m_WakeAngle.SetDescript( "Wake angle" );

    m_IntersectSubSurfs.Init( "IntersectSubSurfs", "Global", this, true, 0, 1 );
    m_IntersectSubSurfs.SetDescript( "Flag to intersect subsurfaces" );

    m_SelectedSetIndex.Init( "Set", "Global", this, 0, 0, 12 );
    m_SelectedSetIndex.SetDescript( "Selected set for operation" );

    m_ExportFileFlags[ vsp::CFD_DAT_FILE_NAME ].Init( "DAT_Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::CFD_KEY_FILE_NAME ].Init( "KEY_Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::CFD_OBJ_FILE_NAME ].Init( "OBJ_Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::CFD_POLY_FILE_NAME ].Init( "POLY_Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::CFD_STL_FILE_NAME ].Init( "STL_Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::CFD_TRI_FILE_NAME ].Init( "TRI_Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::CFD_GMSH_FILE_NAME ].Init( "GMSH_Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::CFD_SRF_FILE_NAME ].Init( "SRF_Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::CFD_TKEY_FILE_NAME ].Init( "TKEY_Export", "ExportCFD", this, true, 0, 1 );

    m_XYZIntCurveFlag.Init( "SRF_XYZIntCurve", "ExportCFD", this, false, 0, 1 );
}

CfdMeshSettings::~CfdMeshSettings()
{
}

xmlNodePtr CfdMeshSettings::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr cfdsetnode = xmlNewChild( node, NULL, BAD_CAST"CfdSettings", NULL );

    ParmContainer::EncodeXml( cfdsetnode );

    XmlUtil::AddStringNode( cfdsetnode, "FarFieldGeomID", m_FarGeomID );

    return cfdsetnode;
}

xmlNodePtr CfdMeshSettings::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr cfdsetnode = XmlUtil::GetNode( node, "CfdSettings", 0 );
    if ( cfdsetnode )
    {
        ParmContainer::DecodeXml( cfdsetnode );
        m_FarGeomID   = XmlUtil::FindString( cfdsetnode, "FarFieldGeomID", m_FarGeomID );
    }

    return cfdsetnode;
}

void CfdMeshSettings::ReadV2File( xmlNodePtr &root )
{
    m_FarXScale = XmlUtil::FindDouble( root, "CFD_Far_Field_Scale_X", m_FarXScale() );
    m_FarYScale = XmlUtil::FindDouble( root, "CFD_Far_Field_Scale_Y", m_FarYScale() );
    m_FarZScale = XmlUtil::FindDouble( root, "CFD_Far_Field_Scale_Z", m_FarZScale() );

    m_HalfMeshFlag = !! ( XmlUtil::FindInt( root, "CFD_Half_Mesh_Flag", m_HalfMeshFlag() ) );
    m_FarMeshFlag = !! ( XmlUtil::FindInt( root, "CFD_Far_Mesh_Flag", m_FarMeshFlag() ) );
    m_FarAbsSizeFlag = !! ( XmlUtil::FindInt( root, "CFD_Far_Abs_Size_Flag", m_FarAbsSizeFlag() ) );
    m_FarManLocFlag = !! ( XmlUtil::FindInt( root, "CFD_Far_Man_Loc_Flag", m_FarManLocFlag() ) );
    m_FarCompFlag = !! ( XmlUtil::FindInt( root, "CFD_Far_Comp_Flag", m_FarCompFlag() ) );

    string fargeom = XmlUtil::FindString( root, "CFD_Far_Geom_PtrID", m_FarGeomID );
    if ( fargeom != "0" )
    {
        m_FarGeomID = ParmMgr.ForceRemapID( fargeom , 10 );
    }

    m_WakeAngle = XmlUtil::FindDouble( root, "CFD_Wake_Angle", m_WakeAngle() );
    m_WakeScale = XmlUtil::FindDouble( root, "CFD_Wake_Scale", m_WakeScale() );

    SetFileExportFlag( vsp::CFD_STL_FILE_NAME, !!XmlUtil::FindInt( root, "CFD_Stl_File_Flag", GetExportFileFlag( vsp::CFD_STL_FILE_NAME )->Get() ) );
    SetFileExportFlag( vsp::CFD_POLY_FILE_NAME, !!XmlUtil::FindInt( root, "CFD_Poly_File_Flag", GetExportFileFlag( vsp::CFD_POLY_FILE_NAME )->Get() ) );
    SetFileExportFlag( vsp::CFD_TRI_FILE_NAME, !!XmlUtil::FindInt( root, "CFD_Tri_File_Flag", GetExportFileFlag( vsp::CFD_TRI_FILE_NAME )->Get() ) );
    SetFileExportFlag( vsp::CFD_OBJ_FILE_NAME, !!XmlUtil::FindInt( root, "CFD_Obj_File_Flag", GetExportFileFlag( vsp::CFD_OBJ_FILE_NAME )->Get() ) );
    SetFileExportFlag( vsp::CFD_DAT_FILE_NAME, !!XmlUtil::FindInt( root, "CFD_Dat_File_Flag", GetExportFileFlag( vsp::CFD_DAT_FILE_NAME )->Get() ) );
    SetFileExportFlag( vsp::CFD_KEY_FILE_NAME, !!XmlUtil::FindInt( root, "CFD_Key_File_Flag", GetExportFileFlag( vsp::CFD_KEY_FILE_NAME )->Get() ) );
    SetFileExportFlag( vsp::CFD_GMSH_FILE_NAME, !!XmlUtil::FindInt( root, "CFD_Gmsh_File_Flag", GetExportFileFlag( vsp::CFD_GMSH_FILE_NAME )->Get() ) );
    SetFileExportFlag( vsp::CFD_SRF_FILE_NAME, !!XmlUtil::FindInt( root, "CFD_Srf_File_Flag", GetExportFileFlag( vsp::CFD_SRF_FILE_NAME )->Get() ) );
}

//==== Parm Changed ====//
void CfdMeshSettings::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

string CfdMeshSettings::GetExportFileName( int type )
{
    if ( type >= 0 && type < vsp::CFD_NUM_FILE_NAMES )
    {
        return m_ExportFileNames[type];
    }

    return string();
}

void CfdMeshSettings::SetExportFileName( const string &fn, int type )
{
    if ( type >= 0 && type < vsp::CFD_NUM_FILE_NAMES )
    {
        m_ExportFileNames[type] = fn;
    }
}

void CfdMeshSettings::ResetExportFileNames()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        ResetExportFileNames( veh->GetVSP3FileName() );
    }
}

void CfdMeshSettings::ResetExportFileNames( string basename )
{
    int pos;
    const char *suffix[] = {".stl", ".poly", ".tri", ".obj", "_NASCART.dat", "_NASCART.key", ".msh", ".srf", ".tkey" };

    for ( int i = 0 ; i < vsp::CFD_NUM_FILE_NAMES ; i++ )
    {
        m_ExportFileNames[i] = basename;
        pos = m_ExportFileNames[i].find( ".vsp3" );
        if ( pos >= 0 )
        {
            m_ExportFileNames[i].erase( pos, m_ExportFileNames[i].length() - 1 );
        }
        m_ExportFileNames[i].append( suffix[i] );
    }
}

BoolParm* CfdMeshSettings::GetExportFileFlag( int type )
{
    assert( type >= 0 && type < vsp::CFD_NUM_FILE_NAMES );

    return &m_ExportFileFlags[type];
}

void CfdMeshSettings::SetAllFileExportFlags( bool flag )
{
    for ( int i = 0 ; i < vsp::CFD_NUM_FILE_NAMES ; i++ )
    {
        m_ExportFileFlags[i] = flag;
    }
}

void CfdMeshSettings::SetFileExportFlag( int type, bool flag )
{
    if ( type >= 0 && type < vsp::CFD_NUM_FILE_NAMES )
        m_ExportFileFlags[type] = flag;
}
