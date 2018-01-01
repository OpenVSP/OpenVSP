//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MeshCommonSettings.cpp
//
//////////////////////////////////////////////////////////////////////

#include "MeshCommonSettings.h"
#include "Vehicle.h"
#include "ParmMgr.h"

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

MeshCommonSettings::MeshCommonSettings() : ParmContainer()
{
    m_Name = "";
}

MeshCommonSettings::~MeshCommonSettings()
{
}

void MeshCommonSettings::InitCommonParms()
{
    m_DrawMeshFlag.Init( "DrawMeshFlag", "DrawMesh", this, true, 0, 1 );
    m_ColorTagsFlag.Init( "ColorTagsFlag", "DrawMesh", this, true, 0, 1 );

    m_DrawBorderFlag.Init( "DrawBorderFlag", "DrawMesh", this, true, 0, 1 );
    m_DrawIsectFlag.Init( "DrawIsectFlag", "DrawMesh", this, true, 0, 1 );
    m_DrawRawFlag.Init( "DrawRawFlag", "DrawMesh", this, false, 0, 1 );
    m_DrawBinAdaptFlag.Init( "DrawBinAdaptFlag", "DrawMesh", this, true, 0, 1 );

    m_DrawCurveFlag.Init( "DrawCurveFlag", "DrawMesh", this, true, 0, 1 );
    m_DrawPntsFlag.Init( "DrawPntsFlag", "DrawMesh", this, true, 0, 1 );

    m_DrawRelCurveTol.Init( "DrawRelCurveTol", "Global", this, 0.01, 1e-6, 1.0 );

    m_IntersectSubSurfs.Init( "IntersectSubSurfs", "Global", this, true, 0, 1 );
    m_IntersectSubSurfs.SetDescript( "Flag to intersect subsurfaces" );

    m_FarCompFlag.Init( "FarComp", "FarField", this, false, 0, 1 );
    m_FarMeshFlag.Init( "FarMesh", "FarField", this, false, 0, 1 );
    m_HalfMeshFlag.Init( "HalfMesh", "FarField", this, false, 0, 1 );

    //Symmetry Plane Splitting Default Value: OFF
    m_SymSplittingOnFlag.Init( "SymmetrySplitting", "FarField", this, false, 0, 1 );
}

//==== Parm Changed ====//
void MeshCommonSettings::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

IntersectSettings::IntersectSettings() : MeshCommonSettings()
{
    m_Name = "SurfaceIntersectSettings";

    m_ExportRelCurveTol.Init( "ExportRelCurveTol", "ExportIntersect", this, 0.01, 1e-6, 1.0 );
    m_ExportRawFlag.Init( "ExportRawFlag", "ExportIntersect", this, false, 0, 1 );

    m_SelectedSetIndex.Init( "Set", "Global", this, 0, 0, 12 );
    m_SelectedSetIndex.SetDescript( "Selected set for operation" );

    m_XYZIntCurveFlag.Init( "SRF_XYZIntCurve", "ExportIntersect", this, false, 0, 1 );

    m_ExportFileFlags[ vsp::INTERSECT_SRF_FILE_NAME ].Init( "SRF_Export", "ExportIntersect", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::INTERSECT_CURV_FILE_NAME ].Init( "CURV_Export", "ExportIntersect", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::INTERSECT_PLOT3D_FILE_NAME ].Init( "PLOT3D_Export", "ExportIntersect", this, true, 0, 1 );

    InitCommonParms();

    SetFarCompFlag( false );
    SetFarMeshFlag( false );
    SetSymSplittingOnFlag( false );
    SetIntersectSubSurfs( true );
}

IntersectSettings::~IntersectSettings()
{
}

xmlNodePtr IntersectSettings::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr structsettingnode = xmlNewChild( node, NULL, BAD_CAST m_Name.c_str(), NULL );

    //for ( size_t i = 0; i < vsp::FEA_NUM_FILE_NAMES; i++ )
    //{
    //    string fname = "FEAExportName_" + std::to_string( i );
    //    XmlUtil::AddStringNode( structsettingnode, fname.c_str(), m_ExportFileNames[i] );
    //}

    MeshCommonSettings::EncodeXml( structsettingnode );

    return structsettingnode;
}

xmlNodePtr IntersectSettings::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr structsettingnode = XmlUtil::GetNode( node, m_Name.c_str(), 0 );
    if ( structsettingnode )
    {
        //for ( size_t i = 0; i < vsp::FEA_NUM_FILE_NAMES; i++ )
        //{
        //    string fname = "FEAExportName_" + std::to_string( i );
        //    m_ExportFileNames[i] = XmlUtil::FindString( structsettingnode, fname.c_str(), 0 );
        //}

        MeshCommonSettings::DecodeXml( structsettingnode );
    }

    return structsettingnode;
}

string IntersectSettings::GetExportFileName( int type )
{
    if ( type >= 0 && type < vsp::INTERSECT_NUM_FILE_NAMES )
    {
        return m_ExportFileNames[type];
    }

    return string();
}

void IntersectSettings::SetExportFileName( const string &fn, int type )
{
    if ( type >= 0 && type < vsp::INTERSECT_NUM_FILE_NAMES )
    {
        m_ExportFileNames[type] = fn;
    }
}

void IntersectSettings::ResetExportFileNames()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        ResetExportFileNames( veh->GetVSP3FileName() );
    }
}

void IntersectSettings::ResetExportFileNames( string basename )
{
    int pos;
    const char *suffix[] = {".srf", ".curv", ".p3d" };

    for ( int i = 0 ; i < vsp::INTERSECT_NUM_FILE_NAMES; i++ )
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

BoolParm* IntersectSettings::GetExportFileFlag( int type )
{
    assert( type >= 0 && type < vsp::INTERSECT_NUM_FILE_NAMES );

    return &m_ExportFileFlags[type];
}

void IntersectSettings::SetAllFileExportFlags( bool flag )
{
    for ( int i = 0 ; i < vsp::INTERSECT_NUM_FILE_NAMES; i++ )
    {
        m_ExportFileFlags[i] = flag;
    }
}

void IntersectSettings::SetFileExportFlag( int type, bool flag )
{
    if ( type >= 0 && type < vsp::INTERSECT_NUM_FILE_NAMES )
    {
        m_ExportFileFlags[type] = flag;
    }
}

vector < string > IntersectSettings::GetExportFileNames()
{
    vector < string > ret_vec;

    for ( size_t i = 0; i < vsp::INTERSECT_NUM_FILE_NAMES; i++ )
    {
        ret_vec.push_back( m_ExportFileNames[i] );
    }
    return ret_vec;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

CfdMeshSettings::CfdMeshSettings() : MeshCommonSettings()
{
    m_Name = "CFDMeshSettings";

    m_DrawSourceFlag.Init( "DrawSourceWake", "DrawMesh", this, true, 0, 1 );
    m_DrawFarFlag.Init( "DrawFarField", "DrawMesh", this, true, 0, 1 );
    m_DrawFarPreFlag.Init( "DrawFarFieldPreview", "DrawMesh", this, true, 0, 1 );
    m_DrawSymmFlag.Init( "DrawSymmetryPlane", "DrawMesh", this, true, 0, 1 );
    m_DrawBadFlag.Init( "DrawBadElementsFlag", "DrawMesh", this, true, 0, 1 );
    m_DrawWakeFlag.Init( "DrawWake", "DrawMesh", this, true, 0, 1 );

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
    m_ExportFileFlags[ vsp::CFD_FACET_FILE_NAME ].Init( "FACET_Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::CFD_CURV_FILE_NAME ].Init( "CURV_Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::CFD_PLOT3D_FILE_NAME ].Init( "PLOT3D_Export", "ExportCFD", this, true, 0, 1 );

    m_XYZIntCurveFlag.Init( "SRF_XYZIntCurve", "ExportCFD", this, false, 0, 1 );

    m_ExportRelCurveTol.Init( "ExportRelCurveTol", "ExportCFD", this, 0.01, 1e-6, 1.0 );
    m_ExportRawFlag.Init( "ExportRawFlag", "ExportCFD", this, false, 0, 1 );

    InitCommonParms();
    m_DrawBorderFlag = false;
    m_DrawIsectFlag = false;

}

CfdMeshSettings::~CfdMeshSettings()
{
}

xmlNodePtr CfdMeshSettings::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr cfdsettingnode = xmlNewChild( node, NULL, BAD_CAST m_Name.c_str(), NULL );

    XmlUtil::AddStringNode( cfdsettingnode, "FarGeomID", m_FarGeomID );

    MeshCommonSettings::EncodeXml( cfdsettingnode );

    return cfdsettingnode;
}

xmlNodePtr CfdMeshSettings::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr cfdsettingnode = XmlUtil::GetNode( node, m_Name.c_str(), 0 );
    if ( cfdsettingnode )
    {
        m_FarGeomID = XmlUtil::FindString( cfdsettingnode, "FarGeomID", m_FarGeomID );

        MeshCommonSettings::DecodeXml( cfdsettingnode );
    }

    return cfdsettingnode;
}

void CfdMeshSettings::ReadV2File( xmlNodePtr &root )
{
    // TODO: Complete ReadV2File Functions

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
    SetFileExportFlag( vsp::CFD_FACET_FILE_NAME, !!XmlUtil::FindInt( root, "CFD_Facet_File_Flag", GetExportFileFlag( vsp::CFD_FACET_FILE_NAME )->Get() ) );
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
    const char *suffix[] = {".stl", ".poly", ".tri", ".obj", "_NASCART.dat", "_NASCART.key", ".msh", ".srf", ".tkey", ".facet", ".curv", ".p3d" };

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

vector < string > CfdMeshSettings::GetExportFileNames()
{
    vector < string > ret_vec;

    for ( size_t i = 0; i < vsp::CFD_NUM_FILE_NAMES; i++ )
    {
        ret_vec.push_back( m_ExportFileNames[i] );
    }
    return ret_vec;
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

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

StructSettings::StructSettings() : MeshCommonSettings()
{
    m_Name = "StructSettings";

    InitCommonParms();

    m_ExportFileFlags[ vsp::FEA_MASS_FILE_NAME ].Init( "MASS_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::FEA_NASTRAN_FILE_NAME ].Init( "NASTRAN_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::FEA_CALCULIX_FILE_NAME ].Init( "CALCULIX_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::FEA_STL_FILE_NAME ].Init( "STL_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::FEA_GMSH_FILE_NAME].Init( "GMSH_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::FEA_SRF_FILE_NAME ].Init( "SRF_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::FEA_CURV_FILE_NAME ].Init( "CURV_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::FEA_PLOT3D_FILE_NAME ].Init( "PLOT3D_Export", "ExportFEA", this, true, 0, 1 );

    m_XYZIntCurveFlag.Init( "SRF_XYZIntCurve", "ExportFEA", this, false, 0, 1 );

    m_ExportRelCurveTol.Init( "ExportRelCurveTol", "ExportFEA", this, 0.01, 1e-6, 1.0 );
    m_ExportRawFlag.Init( "ExportRawFlag", "ExportFEA", this, false, 0, 1 );

    m_DrawBorderFlag = false;
    m_DrawIsectFlag = false;

    SetFarCompFlag( false );
    SetFarMeshFlag( false );
    SetSymSplittingOnFlag( false );
    SetIntersectSubSurfs( true );

    m_NumEvenlySpacedPart.Init( "NumEvenlySpacedPart", "StructSettings", this, 10, 0, 1000 );
    m_NumEvenlySpacedPart.SetDescript( "Number of Evenly Spaced FeaParts to Add" );

    m_MultiSliceSpacing.Init( "MultiSliceSpacing", "StructSettings", this, 20, 1e-8, 1e8 );
    m_MultiSliceSpacing.SetDescript( "Spacing Between Multiple Added Slices" );

    m_MultSliceIncludedElements.Init( "MultSliceIncludedElements", "StructSettings", this, vsp::FEA_SHELL, vsp::FEA_SHELL, vsp::FEA_SHELL_AND_BEAM );
    m_MultSliceIncludedElements.SetDescript( "Indicates the FeaElements to be Included for the Added Slices" );

    m_DrawNodesFlag.Init( "DrawNodesFlag", "StructSettings", this, false, false, true );
    m_DrawNodesFlag.SetDescript( "Flag to Draw FeaNodes" );

    m_DrawElementOrientVecFlag.Init( "DrawElementOrientVecFlag", "StructSettings", this, false, false, true );
    m_DrawElementOrientVecFlag.SetDescript( "Flag to Draw FeaElement Orientation Vectors" );

    ResetExportFileNames();
}

StructSettings::~StructSettings()
{
}

xmlNodePtr StructSettings::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr structsettingnode = xmlNewChild( node, NULL, BAD_CAST m_Name.c_str(), NULL );

    //for ( size_t i = 0; i < vsp::FEA_NUM_FILE_NAMES; i++ )
    //{
    //    string fname = "FEAExportName_" + std::to_string( i );
    //    XmlUtil::AddStringNode( structsettingnode, fname.c_str(), m_ExportFileNames[i] );
    //}

    MeshCommonSettings::EncodeXml( structsettingnode );

    return structsettingnode;
}

xmlNodePtr StructSettings::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr structsettingnode = XmlUtil::GetNode( node, m_Name.c_str(), 0 );
    if ( structsettingnode )
    {
        //for ( size_t i = 0; i < vsp::FEA_NUM_FILE_NAMES; i++ )
        //{
        //    string fname = "FEAExportName_" + std::to_string( i );
        //    m_ExportFileNames[i] = XmlUtil::FindString( structsettingnode, fname.c_str(), 0 );
        //}

        MeshCommonSettings::DecodeXml( structsettingnode );
    }

    return structsettingnode;
}

string StructSettings::GetExportFileName( int type )
{
    if ( type >= 0 && type < vsp::FEA_NUM_FILE_NAMES )
    {
        return m_ExportFileNames[type];
    }

    return string();
}

void StructSettings::SetExportFileName( const string &fn, int type )
{
    if ( type >= 0 && type < vsp::FEA_NUM_FILE_NAMES )
    {
        m_ExportFileNames[type] = fn;
    }
}

void StructSettings::ResetExportFileNames()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        ResetExportFileNames( veh->GetVSP3FileName() );
    }
}

void StructSettings::ResetExportFileNames( string basename )
{
    int pos;
    const char *suffix[] = {"_mass.txt", "_NASTRAN.dat", "_calculix.dat", ".stl", ".msh", ".srf", ".curv", ".p3d" };

    for ( int i = 0 ; i < vsp::FEA_NUM_FILE_NAMES; i++ )
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

BoolParm* StructSettings::GetExportFileFlag( int type )
{
    assert( type >= 0 && type < vsp::FEA_NUM_FILE_NAMES );

    return &m_ExportFileFlags[type];
}

void StructSettings::SetAllFileExportFlags( bool flag )
{
    for ( int i = 0 ; i < vsp::FEA_NUM_FILE_NAMES; i++ )
    {
        m_ExportFileFlags[i] = flag;
    }
}

void StructSettings::SetFileExportFlag( int type, bool flag )
{
    if ( type >= 0 && type < vsp::FEA_NUM_FILE_NAMES )
        m_ExportFileFlags[type] = flag;
}

vector < string > StructSettings::GetExportFileNames()
{
    vector < string > ret_vec;

    for ( size_t i = 0; i < vsp::FEA_NUM_FILE_NAMES; i++ )
    {
        ret_vec.push_back( m_ExportFileNames[i] );
    }
    return ret_vec;
}
