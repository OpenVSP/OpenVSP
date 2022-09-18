//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MeshAnalysis.cpp
// Gary Gray, ESAero, 10/24/20
//
// Collection of analyses for the cfd_mesh project, similar to the AnalysisMgr in geom_core
//
//////////////////////////////////////////////////////////////////////

#include "MeshAnalysis.h"
#include "StructureMgr.h"
#include "FeaMeshMgr.h"


//======================================================================================//
//================================= Cfd Mesh Analysis ==================================//
//======================================================================================//

void CfdMeshAnalysis::SetDefaults()
{
    m_Inputs.Clear();
    Vehicle* veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        m_Inputs.Add( NameValData( "BaseLen", veh->GetCfdGridDensityPtr()->m_BaseLen() ) );
        m_Inputs.Add( NameValData( "MinLen", veh->GetCfdGridDensityPtr()->m_MinLen() ) );
        m_Inputs.Add( NameValData( "MaxGap", veh->GetCfdGridDensityPtr()->m_MaxGap() ) );
        m_Inputs.Add( NameValData( "NCircSeg", veh->GetCfdGridDensityPtr()->m_NCircSeg() ) );
        m_Inputs.Add( NameValData( "GrowthRatio", veh->GetCfdGridDensityPtr()->m_GrowRatio() ) );
        m_Inputs.Add( NameValData( "RelCurveTol", veh->GetCfdSettingsPtr()->m_RelCurveTol() ) );

        m_Inputs.Add( NameValData( "RigorLimit", veh->GetCfdGridDensityPtr()->m_RigorLimit() ) );
        m_Inputs.Add( NameValData( "IntersectSubSurfs", veh->GetCfdSettingsPtr()->m_IntersectSubSurfs() ) );
        m_Inputs.Add( NameValData( "TaggedMultiSolid", veh->m_STLMultiSolid() ) );
        m_Inputs.Add( NameValData( "XYZIntCurveFlag", veh->GetCfdSettingsPtr()->m_XYZIntCurveFlag() ) );
        m_Inputs.Add( NameValData( "ExportRawFlag", veh->GetCfdSettingsPtr()->m_ExportRawFlag() ) );
        m_Inputs.Add( NameValData( "GenerateHalfMesh", veh->GetCfdSettingsPtr()->m_HalfMeshFlag() ) );

        m_Inputs.Add( NameValData( "SelectedSetIndex", veh->GetCfdSettingsPtr()->m_SelectedSetIndex() ) );

        // File Outputs
        m_Inputs.Add( NameValData( "STLFileFlag", veh->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_STL_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "STLFileName", veh->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_STL_FILE_NAME ) ) );
        m_Inputs.Add( NameValData( "POLYFileFlag", veh->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_POLY_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "POLYFileName", veh->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_POLY_FILE_NAME ) ) );
        m_Inputs.Add( NameValData( "TRIFileFlag", veh->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_TRI_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "TRIFileName", veh->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_TRI_FILE_NAME ) ) );
        m_Inputs.Add( NameValData( "FACETFileFlag", veh->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_FACET_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "FACETFileName", veh->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_FACET_FILE_NAME ) ) );
        m_Inputs.Add( NameValData( "OBJFileFlag", veh->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_OBJ_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "OBJFileName", veh->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_OBJ_FILE_NAME ) ) );
        m_Inputs.Add( NameValData( "GMSHFileFlag", veh->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_GMSH_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "GMSHFileName", veh->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_GMSH_FILE_NAME ) ) );
    }
    else
    {
        printf( "ERROR - trying to set defaults without a vehicle: void CfdMeshAnalysis::SetDefaults()\n" );
    }
}

string CfdMeshAnalysis::Execute()
{
    string res_id;
    Vehicle* veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        NameValData* nvd = NULL;

        double baseLenOrig = veh->GetCfdGridDensityPtr()->m_BaseLen();
        nvd = m_Inputs.FindPtr( "BaseLen", 0 );
        if( nvd ) veh->GetCfdGridDensityPtr()->m_BaseLen.Set( nvd->GetDouble( 0 ) );

        double minLenOrig = veh->GetCfdGridDensityPtr()->m_MinLen();
        nvd = m_Inputs.FindPtr( "MinLen", 0 );
        if( nvd ) veh->GetCfdGridDensityPtr()->m_MinLen.Set( nvd->GetDouble( 0 ) );

        double maxGapOrig = veh->GetCfdGridDensityPtr()->m_MaxGap();
        nvd = m_Inputs.FindPtr( "MaxGap", 0 );
        if( nvd ) veh->GetCfdGridDensityPtr()->m_MaxGap.Set( nvd->GetDouble( 0 ) );

        double nCircSegOrig = veh->GetCfdGridDensityPtr()->m_NCircSeg();
        nvd = m_Inputs.FindPtr( "NCircSeg", 0 );
        if( nvd ) veh->GetCfdGridDensityPtr()->m_NCircSeg.Set( nvd->GetDouble( 0 ) );

        double growRatioOrig = veh->GetCfdGridDensityPtr()->m_GrowRatio();
        nvd = m_Inputs.FindPtr( "GrowRatio", 0 );
        if( nvd ) veh->GetCfdGridDensityPtr()->m_GrowRatio.Set( nvd->GetDouble( 0 ) );

        double relCurveTolOrig = veh->GetCfdSettingsPtr()->m_RelCurveTol();
        nvd = m_Inputs.FindPtr( "RelCurveTol", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->m_RelCurveTol.Set( nvd->GetDouble( 0 ) );

        bool rigorLimitOrig = veh->GetCfdGridDensityPtr()->m_RigorLimit();
        nvd = m_Inputs.FindPtr( "RigorLimit", 0 );
        if( nvd ) veh->GetCfdGridDensityPtr()->m_RigorLimit.Set( nvd->GetInt( 0 ) );

        bool intersectSubSurfsOrig = veh->GetCfdSettingsPtr()->m_IntersectSubSurfs();
        nvd = m_Inputs.FindPtr( "IntersectSubSurfs", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->m_IntersectSubSurfs.Set( nvd->GetInt( 0 ) );

        bool taggedMultiSolidOrig = veh->m_STLMultiSolid();
        nvd = m_Inputs.FindPtr( "TaggedMultiSolid", 0 );
        if( nvd ) veh->m_STLMultiSolid.Set( nvd->GetInt( 0 ) );

        bool xYZIntCurveOrig = veh->GetCfdSettingsPtr()->m_XYZIntCurveFlag();
        nvd = m_Inputs.FindPtr( "XYZIntCurve", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->m_XYZIntCurveFlag.Set( nvd->GetInt( 0 ) );

        bool exportRawOrig = veh->GetCfdSettingsPtr()->m_ExportRawFlag();
        nvd = m_Inputs.FindPtr( "ExportRawFlag", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->m_ExportRawFlag.Set( nvd->GetInt( 0 ) );

        bool generateHalfMeshOrig = veh->GetCfdSettingsPtr()->m_HalfMeshFlag();
        nvd = m_Inputs.FindPtr( "GenerateHalfMesh", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->m_HalfMeshFlag.Set( nvd->GetInt( 0 ) );

        int selectedSetIndexOrig = veh->GetCfdSettingsPtr()->m_SelectedSetIndex();
        nvd = m_Inputs.FindPtr( "SelectedSetIndex", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->m_SelectedSetIndex.Set( nvd->GetInt( 0 ) );

        // File Outputs
        bool stlFileFlagOrig = veh->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_STL_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "STLFileFlag", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->SetFileExportFlag( vsp::CFD_STL_FILE_NAME, nvd->GetInt( 0 ) );

        string stlFileNameOrig = veh->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_STL_FILE_NAME );
        nvd = m_Inputs.FindPtr( "STLFileName", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::CFD_STL_FILE_NAME );

        bool polyFileFlagOrig = veh->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_POLY_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "POLYFileFlag", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->SetFileExportFlag( vsp::CFD_POLY_FILE_NAME, nvd->GetInt( 0 ) );

        string polyFileNameOrig = veh->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_POLY_FILE_NAME );
        nvd = m_Inputs.FindPtr( "POLYFileName", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::CFD_POLY_FILE_NAME );

        bool triFileFlagOrig = veh->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_TRI_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "TRIFileFlag", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->SetFileExportFlag( vsp::CFD_TRI_FILE_NAME, nvd->GetInt( 0 ) );

        string triFileNameOrig = veh->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_TRI_FILE_NAME );
        nvd = m_Inputs.FindPtr( "TRIFileName", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::CFD_TRI_FILE_NAME );

        bool facetFileFlagOrig = veh->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_FACET_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "FACETFileFlag", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->SetFileExportFlag( vsp::CFD_FACET_FILE_NAME, nvd->GetInt( 0 ) );

        string facetFileNameOrig = veh->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_FACET_FILE_NAME );
        nvd = m_Inputs.FindPtr( "FACETFileName", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::CFD_FACET_FILE_NAME );

        bool objFileFlagOrig = veh->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_OBJ_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "OBJFileFlag", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->SetFileExportFlag( vsp::CFD_OBJ_FILE_NAME, nvd->GetInt( 0 ) );

        string objFileNameOrig = veh->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_OBJ_FILE_NAME );
        nvd = m_Inputs.FindPtr( "OBJFileName", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::CFD_OBJ_FILE_NAME );

        bool gmshFileFlagOrig = veh->GetCfdSettingsPtr()->GetExportFileFlag( vsp::CFD_GMSH_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "GMSHFileFlag", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->SetFileExportFlag( vsp::CFD_GMSH_FILE_NAME, nvd->GetInt( 0 ) );

        string gmshFileNameOrig = veh->GetCfdSettingsPtr()->GetExportFileName( vsp::CFD_GMSH_FILE_NAME );
        nvd = m_Inputs.FindPtr( "GMSHFileName", 0 );
        if( nvd ) veh->GetCfdSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::CFD_GMSH_FILE_NAME );

        // Execute analysis
        CfdMeshMgr.GenerateMesh();

        // ==== Restore original values that were overwritten by analysis inputs ==== //

        veh->GetCfdGridDensityPtr()->m_BaseLen.Set( baseLenOrig );
        veh->GetCfdGridDensityPtr()->m_MinLen.Set( minLenOrig );
        veh->GetCfdGridDensityPtr()->m_MaxGap.Set( maxGapOrig );
        veh->GetCfdGridDensityPtr()->m_NCircSeg.Set( nCircSegOrig );
        veh->GetCfdGridDensityPtr()->m_GrowRatio.Set( growRatioOrig );
        veh->GetCfdSettingsPtr()->m_RelCurveTol.Set( relCurveTolOrig );

        veh->GetCfdGridDensityPtr()->m_RigorLimit.Set( rigorLimitOrig );
        veh->GetCfdSettingsPtr()->m_IntersectSubSurfs.Set( intersectSubSurfsOrig );
        veh->m_STLMultiSolid.Set( taggedMultiSolidOrig );
        veh->GetCfdSettingsPtr()->m_XYZIntCurveFlag.Set( xYZIntCurveOrig );
        veh->GetCfdSettingsPtr()->m_ExportRawFlag.Set( exportRawOrig );
        veh->GetCfdSettingsPtr()->m_HalfMeshFlag.Set( generateHalfMeshOrig );

        veh->GetCfdSettingsPtr()->m_SelectedSetIndex.Set( selectedSetIndexOrig );

        // File Outputs
        veh->GetCfdSettingsPtr()->SetFileExportFlag( vsp::CFD_STL_FILE_NAME, stlFileFlagOrig );
        veh->GetCfdSettingsPtr()->SetExportFileName( stlFileNameOrig, vsp::CFD_STL_FILE_NAME );
        veh->GetCfdSettingsPtr()->SetFileExportFlag( vsp::CFD_POLY_FILE_NAME, polyFileFlagOrig );
        veh->GetCfdSettingsPtr()->SetExportFileName( polyFileNameOrig, vsp::CFD_POLY_FILE_NAME );
        veh->GetCfdSettingsPtr()->SetFileExportFlag( vsp::CFD_TRI_FILE_NAME, triFileFlagOrig );
        veh->GetCfdSettingsPtr()->SetExportFileName( triFileNameOrig, vsp::CFD_TRI_FILE_NAME );
        veh->GetCfdSettingsPtr()->SetFileExportFlag( vsp::CFD_FACET_FILE_NAME, facetFileFlagOrig );
        veh->GetCfdSettingsPtr()->SetExportFileName( facetFileNameOrig, vsp::CFD_FACET_FILE_NAME );
        veh->GetCfdSettingsPtr()->SetFileExportFlag( vsp::CFD_OBJ_FILE_NAME, objFileFlagOrig );
        veh->GetCfdSettingsPtr()->SetExportFileName( objFileNameOrig, vsp::CFD_OBJ_FILE_NAME );
        veh->GetCfdSettingsPtr()->SetFileExportFlag( vsp::CFD_GMSH_FILE_NAME, gmshFileFlagOrig );
        veh->GetCfdSettingsPtr()->SetExportFileName( gmshFileNameOrig, vsp::CFD_GMSH_FILE_NAME );

    }

    return res_id;
}

//======================================================================================//
//================================= Fea Mesh Analysis ==================================//
//======================================================================================//

void FeaMeshAnalysis::SetDefaults()
{
    m_Inputs.Clear();

    FeaStructure* curr_struct = StructureMgr.GetFeaStruct( FeaMeshMgr.GetFeaMeshStructID() );

    StructSettings temp_settings;
    FeaGridDensity temp_density;

    if( curr_struct )
    {
        m_Inputs.Add( NameValData( "BaseLen", curr_struct->GetFeaGridDensityPtr()->m_BaseLen() ) );
        m_Inputs.Add( NameValData( "MinLen", curr_struct->GetFeaGridDensityPtr()->m_MinLen() ) );
        m_Inputs.Add( NameValData( "MaxGap", curr_struct->GetFeaGridDensityPtr()->m_MaxGap() ) );
        m_Inputs.Add( NameValData( "NCircSeg", curr_struct->GetFeaGridDensityPtr()->m_NCircSeg() ) );
        m_Inputs.Add( NameValData( "GrowthRatio", curr_struct->GetFeaGridDensityPtr()->m_GrowRatio() ) );
        m_Inputs.Add( NameValData( "RelCurveTol", curr_struct->GetStructSettingsPtr()->m_RelCurveTol() ) );
        m_Inputs.Add( NameValData( "STEPTol", curr_struct->GetStructSettingsPtr()->m_STEPTol() ) );

        m_Inputs.Add( NameValData( "RigorLimit", curr_struct->GetFeaGridDensityPtr()->m_RigorLimit() ) );
        m_Inputs.Add( NameValData( "ExportRawFlag", curr_struct->GetStructSettingsPtr()->m_ExportRawFlag() ) );
        m_Inputs.Add( NameValData( "HalfMeshFlag", curr_struct->GetStructSettingsPtr()->m_HalfMeshFlag() ) );
        m_Inputs.Add( NameValData( "XYZIntCurveFlag", curr_struct->GetStructSettingsPtr()->m_XYZIntCurveFlag() ) );
        m_Inputs.Add( NameValData( "CADLabelID", curr_struct->GetStructSettingsPtr()->m_CADLabelID() ) );
        m_Inputs.Add( NameValData( "CADLabelName", curr_struct->GetStructSettingsPtr()->m_CADLabelName() ) );
        m_Inputs.Add( NameValData( "CADLabelSurfNo", curr_struct->GetStructSettingsPtr()->m_CADLabelSurfNo() ) );
        m_Inputs.Add( NameValData( "CADLabelSplitNo", curr_struct->GetStructSettingsPtr()->m_CADLabelSplitNo() ) );
        m_Inputs.Add( NameValData( "STEPRepresentation", curr_struct->GetStructSettingsPtr()->m_STEPRepresentation() ) );

        m_Inputs.Add( NameValData( "CADLenUnit", curr_struct->GetStructSettingsPtr()->m_CADLenUnit() ) );
        m_Inputs.Add( NameValData( "CADLabelDelim", curr_struct->GetStructSettingsPtr()->m_CADLabelDelim() ) );

        // File Outputs
        m_Inputs.Add( NameValData( "STLFileFlag", curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_STL_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "STLFileName", curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_STL_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "GMSHFileFlag", curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_GMSH_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "GMSHFileName", curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_GMSH_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "MASSFileFlag", curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_MASS_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "MASSFileName", curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_MASS_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "NASTRANFileFlag", curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_NASTRAN_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "NASTRANFileName", curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_NASTRAN_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "NKEYFileFlag", curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_NKEY_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "NKEYFileName", curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_NKEY_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "CALCULIXFileFlag", curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_CALCULIX_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "CALCULIXFileName", curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_CALCULIX_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "CURVFileFlag", curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_CURV_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "CURVFileName", curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_CURV_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "P3DFileFlag", curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_PLOT3D_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "P3DFileName", curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_PLOT3D_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "SRFFileFlag", curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_SRF_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "SRFFileName", curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_SRF_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "IGESFileFlag", curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_IGES_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "IGESFileName", curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_IGES_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "STEPFileFlag", curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_STEP_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "STEPFileName", curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_STEP_FILE_NAME ) ) );

    }
    else
    {
        m_Inputs.Add( NameValData( "BaseLen", temp_density.m_BaseLen() ) );
        m_Inputs.Add( NameValData( "MinLen", temp_density.m_MinLen() ) );
        m_Inputs.Add( NameValData( "MaxGap", temp_density.m_MaxGap() ) );
        m_Inputs.Add( NameValData( "NCircSeg", temp_density.m_NCircSeg() ) );
        m_Inputs.Add( NameValData( "GrowthRatio", temp_density.m_GrowRatio() ) );
        m_Inputs.Add( NameValData( "RelCurveTol", temp_settings.m_RelCurveTol() ) );
        m_Inputs.Add( NameValData( "STEPTol", temp_settings.m_STEPTol() ) );

        m_Inputs.Add( NameValData( "RigorLimit", temp_density.m_RigorLimit() ) );
        m_Inputs.Add( NameValData( "ExportRawFlag", temp_settings.m_ExportRawFlag() ) );
        m_Inputs.Add( NameValData( "HalfMeshFlag", temp_settings.m_HalfMeshFlag() ) );
        m_Inputs.Add( NameValData( "XYZIntCurveFlag", temp_settings.m_XYZIntCurveFlag() ) );
        m_Inputs.Add( NameValData( "CADLabelID", temp_settings.m_CADLabelID() ) );
        m_Inputs.Add( NameValData( "CADLabelName", temp_settings.m_CADLabelName() ) );
        m_Inputs.Add( NameValData( "CADLabelSurfNo", temp_settings.m_CADLabelSurfNo() ) );
        m_Inputs.Add( NameValData( "CADLabelSplitNo", temp_settings.m_CADLabelSplitNo() ) );
        m_Inputs.Add( NameValData( "STEPRepresentation", temp_settings.m_STEPRepresentation() ) );

        m_Inputs.Add( NameValData( "CADLenUnit", temp_settings.m_CADLenUnit() ) );
        m_Inputs.Add( NameValData( "CADLabelDelim", temp_settings.m_CADLabelDelim() ) );

        // File Outputs
        m_Inputs.Add( NameValData( "STLFileFlag", temp_settings.GetExportFileFlag( vsp::FEA_STL_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "STLFileName", temp_settings.GetExportFileName( vsp::FEA_STL_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "GMSHFileFlag", temp_settings.GetExportFileFlag( vsp::FEA_GMSH_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "GMSHFileName", temp_settings.GetExportFileName( vsp::FEA_GMSH_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "MASSFileFlag", temp_settings.GetExportFileFlag( vsp::FEA_MASS_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "MASSFileName", temp_settings.GetExportFileName( vsp::FEA_MASS_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "NASTRANFileFlag", temp_settings.GetExportFileFlag( vsp::FEA_NASTRAN_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "NASTRANFileName", temp_settings.GetExportFileName( vsp::FEA_NASTRAN_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "NKEYFileFlag", temp_settings.GetExportFileFlag( vsp::FEA_NKEY_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "NKEYFileName", temp_settings.GetExportFileName( vsp::FEA_NKEY_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "CALCULIXFileFlag", temp_settings.GetExportFileFlag( vsp::FEA_CALCULIX_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "CALCULIXFileName", temp_settings.GetExportFileName( vsp::FEA_CALCULIX_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "CURVFileFlag", temp_settings.GetExportFileFlag( vsp::FEA_CURV_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "CURVFileName", temp_settings.GetExportFileName( vsp::FEA_CURV_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "P3DFileFlag", temp_settings.GetExportFileFlag( vsp::FEA_PLOT3D_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "P3DFileName", temp_settings.GetExportFileName( vsp::FEA_PLOT3D_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "SRFFileFlag", temp_settings.GetExportFileFlag( vsp::FEA_SRF_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "SRFFileName", temp_settings.GetExportFileName( vsp::FEA_SRF_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "IGESFileFlag", temp_settings.GetExportFileFlag( vsp::FEA_IGES_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "IGESFileName", temp_settings.GetExportFileName( vsp::FEA_IGES_FILE_NAME ) ) );

        m_Inputs.Add( NameValData( "STEPFileFlag", temp_settings.GetExportFileFlag( vsp::FEA_STEP_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "STEPFileName", temp_settings.GetExportFileName( vsp::FEA_STEP_FILE_NAME ) ) );

    }
}

string FeaMeshAnalysis::Execute()
{
    string res_id;

    FeaStructure* curr_struct = StructureMgr.GetFeaStruct( FeaMeshMgr.GetFeaMeshStructID() );

    if( curr_struct )
    {
        NameValData* nvd = NULL;

        double baseLenOrig = curr_struct->GetFeaGridDensityPtr()->m_BaseLen();
        nvd = m_Inputs.FindPtr( "BaseLen", 0 );
        if( nvd ) curr_struct->GetFeaGridDensityPtr()->m_BaseLen.Set( nvd->GetDouble( 0 ) );

        double minLenOrig = curr_struct->GetFeaGridDensityPtr()->m_MinLen();
        nvd = m_Inputs.FindPtr( "MinLen", 0 );
        if( nvd ) curr_struct->GetFeaGridDensityPtr()->m_MinLen.Set( nvd->GetDouble( 0 ) );

        double maxGapOrig = curr_struct->GetFeaGridDensityPtr()->m_MaxGap();
        nvd = m_Inputs.FindPtr( "MaxGap", 0 );
        if( nvd ) curr_struct->GetFeaGridDensityPtr()->m_MaxGap.Set( nvd->GetDouble( 0 ) );

        double nCircSegOrig = curr_struct->GetFeaGridDensityPtr()->m_NCircSeg();
        nvd = m_Inputs.FindPtr( "NCircSeg", 0 );
        if( nvd ) curr_struct->GetFeaGridDensityPtr()->m_NCircSeg.Set( nvd->GetDouble( 0 ) );

        double growRatioOrig = curr_struct->GetFeaGridDensityPtr()->m_GrowRatio();
        nvd = m_Inputs.FindPtr( "GrowRatio", 0 );
        if( nvd ) curr_struct->GetFeaGridDensityPtr()->m_GrowRatio.Set( nvd->GetDouble( 0 ) );

        double relCurveTolOrig = curr_struct->GetStructSettingsPtr()->m_RelCurveTol();
        nvd = m_Inputs.FindPtr( "RelCurveTol", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->m_RelCurveTol.Set( nvd->GetDouble( 0 ) );

        double sTEPTolOrig = curr_struct->GetStructSettingsPtr()->m_STEPTol();
        nvd = m_Inputs.FindPtr( "STEPTol", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->m_STEPTol.Set( nvd->GetDouble( 0 ) );

        bool rigorLimitOrig = curr_struct->GetFeaGridDensityPtr()->m_RigorLimit();
        nvd = m_Inputs.FindPtr( "RigorLimit", 0 );
        if( nvd ) curr_struct->GetFeaGridDensityPtr()->m_RigorLimit.Set( nvd->GetInt( 0 ) );

        bool exportRawFlagOrig = curr_struct->GetStructSettingsPtr()->m_ExportRawFlag();
        nvd = m_Inputs.FindPtr( "ExportRawFlag", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->m_ExportRawFlag.Set( nvd->GetInt( 0 ) );

        bool halfMeshFlagOrig = curr_struct->GetStructSettingsPtr()->m_HalfMeshFlag();
        nvd = m_Inputs.FindPtr( "HalfMeshFlag", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->m_HalfMeshFlag.Set( nvd->GetInt( 0 ) );

        bool xYZIntCurveFlagOrig = curr_struct->GetStructSettingsPtr()->m_XYZIntCurveFlag();
        nvd = m_Inputs.FindPtr( "XYZIntCurveFlag", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->m_XYZIntCurveFlag.Set( nvd->GetInt( 0 ) );

        bool cADLabelIDOrig = curr_struct->GetStructSettingsPtr()->m_CADLabelID();
        nvd = m_Inputs.FindPtr( "CADLabelID", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->m_CADLabelID.Set( nvd->GetInt( 0 ) );

        bool cADLabelNameOrig = curr_struct->GetStructSettingsPtr()->m_CADLabelName();
        nvd = m_Inputs.FindPtr( "CADLabelName", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->m_CADLabelName.Set( nvd->GetInt( 0 ) );

        bool cADLabelSurfNoOrig = curr_struct->GetStructSettingsPtr()->m_CADLabelSurfNo();
        nvd = m_Inputs.FindPtr( "CADLabelSurfNo", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->m_CADLabelSurfNo.Set( nvd->GetInt( 0 ) );

        bool cADLabelSplitNoOrig = curr_struct->GetStructSettingsPtr()->m_CADLabelSplitNo();
        nvd = m_Inputs.FindPtr( "CADLabelSplitNo", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->m_CADLabelSplitNo.Set( nvd->GetInt( 0 ) );

        bool sTEPRepGroupOrig = curr_struct->GetStructSettingsPtr()->m_STEPRepresentation();
        nvd = m_Inputs.FindPtr( "STEPRepresentation", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->m_STEPRepresentation.Set( nvd->GetInt( 0 ) );

        int lenUnitChoiceOrig = curr_struct->GetStructSettingsPtr()->m_CADLenUnit();
        nvd = m_Inputs.FindPtr( "CADLenUnit", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->m_CADLenUnit.Set( nvd->GetInt( 0 ) );

        int labelDelimChoiceOrig = curr_struct->GetStructSettingsPtr()->m_CADLabelDelim();
        nvd = m_Inputs.FindPtr( "CADLabelDelim", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->m_CADLabelDelim.Set( nvd->GetInt( 0 ) );

        // File Outputs
        bool stlFileFlagOrig = curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_STL_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "STLFileFlag", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_STL_FILE_NAME, nvd->GetInt( 0 ) );

        string stlFileNameOrig = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_STL_FILE_NAME );
        nvd = m_Inputs.FindPtr( "STLFileName", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::FEA_STL_FILE_NAME );

        bool gMSHFileFlagOrig = curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_GMSH_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "GMSHFileFlag", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_GMSH_FILE_NAME, nvd->GetInt( 0 ) );

        string gMSHFileNameOrig = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_GMSH_FILE_NAME );
        nvd = m_Inputs.FindPtr( "GMSHFileName", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::FEA_GMSH_FILE_NAME );

        bool mASSFileFlagOrig = curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_MASS_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "MASSFileFlag", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_MASS_FILE_NAME, nvd->GetInt( 0 ) );

        string mASSFileNameOrig = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_MASS_FILE_NAME );
        nvd = m_Inputs.FindPtr( "MASSFileName", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::FEA_MASS_FILE_NAME );

        bool nASTRANFileFlagOrig = curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_NASTRAN_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "NASTRANFileFlag", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_NASTRAN_FILE_NAME, nvd->GetInt( 0 ) );

        string nASTRANFileNameOrig = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_NASTRAN_FILE_NAME );
        nvd = m_Inputs.FindPtr( "NASTRANFileName", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::FEA_NASTRAN_FILE_NAME );

        bool nKEYFileFlagOrig = curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_NKEY_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "NKEYFileFlag", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_NKEY_FILE_NAME, nvd->GetInt( 0 ) );

        string nKEYFileNameOrig = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_NKEY_FILE_NAME );
        nvd = m_Inputs.FindPtr( "NKEYFileName", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::FEA_NKEY_FILE_NAME );

        bool cALCULIXFileFlagOrig = curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_CALCULIX_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "CALCULIXFileFlag", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_CALCULIX_FILE_NAME, nvd->GetInt( 0 ) );

        string cALCULIXFileNameOrig = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_CALCULIX_FILE_NAME );
        nvd = m_Inputs.FindPtr( "CALCULIXFileName", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::FEA_CALCULIX_FILE_NAME );

        bool cURVFileFlagOrig = curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_CURV_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "CURVFileFlag", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_CURV_FILE_NAME, nvd->GetInt( 0 ) );

        string cURVFileNameOrig = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_CURV_FILE_NAME );
        nvd = m_Inputs.FindPtr( "CURVFileName", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::FEA_CURV_FILE_NAME );

        bool p3DFileFlagOrig = curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_PLOT3D_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "P3DFileFlag", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_PLOT3D_FILE_NAME, nvd->GetInt( 0 ) );

        string p3DFileNameOrig = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_PLOT3D_FILE_NAME );
        nvd = m_Inputs.FindPtr( "P3DFileName", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::FEA_PLOT3D_FILE_NAME );

        bool sRFFileFlagOrig = curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_SRF_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "SRFFileFlag", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_SRF_FILE_NAME, nvd->GetInt( 0 ) );

        string sRFFileNameOrig = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_SRF_FILE_NAME );
        nvd = m_Inputs.FindPtr( "SRFFileName", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::FEA_SRF_FILE_NAME );

        bool iGESFileFlagOrig = curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_IGES_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "IGESFileFlag", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_IGES_FILE_NAME, nvd->GetInt( 0 ) );

        string iGESFileNameOrig = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_IGES_FILE_NAME );
        nvd = m_Inputs.FindPtr( "IGESFileName", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::FEA_IGES_FILE_NAME );

        bool sTEPFileFlagOrig = curr_struct->GetStructSettingsPtr()->GetExportFileFlag( vsp::FEA_STEP_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "STEPFileFlag", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_STEP_FILE_NAME, nvd->GetInt( 0 ) );

        string sTEPFileNameOrig = curr_struct->GetStructSettingsPtr()->GetExportFileName( vsp::FEA_STEP_FILE_NAME );
        nvd = m_Inputs.FindPtr( "STEPFileName", 0 );
        if( nvd ) curr_struct->GetStructSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::FEA_STEP_FILE_NAME );

        // Execute analysis
        FeaMeshMgr.GenerateFeaMesh();

        // ==== Restore original values that were overwritten by analysis inputs ==== //

        //Input Sliders
        curr_struct->GetFeaGridDensityPtr()->m_BaseLen.Set( baseLenOrig );
        curr_struct->GetFeaGridDensityPtr()->m_MinLen.Set( minLenOrig );
        curr_struct->GetFeaGridDensityPtr()->m_MaxGap.Set( maxGapOrig );
        curr_struct->GetFeaGridDensityPtr()->m_NCircSeg.Set( nCircSegOrig );
        curr_struct->GetFeaGridDensityPtr()->m_GrowRatio.Set( growRatioOrig );
        curr_struct->GetStructSettingsPtr()->m_RelCurveTol.Set( relCurveTolOrig );
        curr_struct->GetStructSettingsPtr()->m_STEPTol.Set( sTEPTolOrig );

        //Input Triggers
        curr_struct->GetFeaGridDensityPtr()->m_RigorLimit.Set( rigorLimitOrig );
        curr_struct->GetStructSettingsPtr()->m_ExportRawFlag.Set( exportRawFlagOrig );
        curr_struct->GetStructSettingsPtr()->m_HalfMeshFlag.Set( halfMeshFlagOrig );
        curr_struct->GetStructSettingsPtr()->m_XYZIntCurveFlag.Set( xYZIntCurveFlagOrig );
        curr_struct->GetStructSettingsPtr()->m_CADLabelID.Set( cADLabelIDOrig );
        curr_struct->GetStructSettingsPtr()->m_CADLabelName.Set( cADLabelNameOrig );
        curr_struct->GetStructSettingsPtr()->m_CADLabelSurfNo.Set( cADLabelSurfNoOrig );
        curr_struct->GetStructSettingsPtr()->m_CADLabelSplitNo.Set( cADLabelSplitNoOrig );
        curr_struct->GetStructSettingsPtr()->m_STEPRepresentation.Set( sTEPRepGroupOrig );

        //Input DropDowns
        curr_struct->GetStructSettingsPtr()->m_CADLenUnit.Set( lenUnitChoiceOrig );
        curr_struct->GetStructSettingsPtr()->m_CADLabelDelim.Set( labelDelimChoiceOrig );

        // File Outputs
        curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_STL_FILE_NAME, stlFileFlagOrig );
        curr_struct->GetStructSettingsPtr()->SetExportFileName( stlFileNameOrig, vsp::FEA_STL_FILE_NAME );

        curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_GMSH_FILE_NAME, gMSHFileFlagOrig );
        curr_struct->GetStructSettingsPtr()->SetExportFileName( gMSHFileNameOrig, vsp::FEA_GMSH_FILE_NAME );

        curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_MASS_FILE_NAME, mASSFileFlagOrig );
        curr_struct->GetStructSettingsPtr()->SetExportFileName( mASSFileNameOrig, vsp::FEA_MASS_FILE_NAME );

        curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_NASTRAN_FILE_NAME, nASTRANFileFlagOrig );
        curr_struct->GetStructSettingsPtr()->SetExportFileName( nASTRANFileNameOrig, vsp::FEA_NASTRAN_FILE_NAME );

        curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_NKEY_FILE_NAME, nKEYFileFlagOrig );
        curr_struct->GetStructSettingsPtr()->SetExportFileName( nKEYFileNameOrig, vsp::FEA_NKEY_FILE_NAME );

        curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_CALCULIX_FILE_NAME, cALCULIXFileFlagOrig );
        curr_struct->GetStructSettingsPtr()->SetExportFileName( cALCULIXFileNameOrig, vsp::FEA_CALCULIX_FILE_NAME );

        curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_CURV_FILE_NAME, cURVFileFlagOrig );
        curr_struct->GetStructSettingsPtr()->SetExportFileName( cURVFileNameOrig, vsp::FEA_CURV_FILE_NAME );

        curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_PLOT3D_FILE_NAME, p3DFileFlagOrig );
        curr_struct->GetStructSettingsPtr()->SetExportFileName( p3DFileNameOrig, vsp::FEA_PLOT3D_FILE_NAME );

        curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_SRF_FILE_NAME, sRFFileFlagOrig );
        curr_struct->GetStructSettingsPtr()->SetExportFileName( sRFFileNameOrig, vsp::FEA_SRF_FILE_NAME );

        curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_IGES_FILE_NAME, iGESFileFlagOrig );
        curr_struct->GetStructSettingsPtr()->SetExportFileName( iGESFileNameOrig, vsp::FEA_IGES_FILE_NAME );

        curr_struct->GetStructSettingsPtr()->SetFileExportFlag( vsp::FEA_STEP_FILE_NAME, sTEPFileFlagOrig );
        curr_struct->GetStructSettingsPtr()->SetExportFileName( sTEPFileNameOrig, vsp::FEA_STEP_FILE_NAME );

    }
    else
    {
        printf( " Error - Cannot find FEA Structure. See SetFeaMeshStructIndex API Function \n " );
    }

    return res_id;
}

//======================================================================================//
//========================== Surface Intersection Analysis =============================//
//======================================================================================//

void SurfaceIntersectionAnalysis::SetDefaults()
{
    m_Inputs.Clear();
    Vehicle* veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        m_Inputs.Add( NameValData( "ExportRawFlag", veh->GetISectSettingsPtr()->m_ExportRawFlag() ) );
        m_Inputs.Add( NameValData( "IntersectSubSurfs", veh->GetISectSettingsPtr()->m_IntersectSubSurfs() ) );
        m_Inputs.Add( NameValData( "RelCurveTol", veh->GetISectSettingsPtr()->m_RelCurveTol() ) );
        m_Inputs.Add( NameValData( "SelectedSetIndex", veh->GetISectSettingsPtr()->m_SelectedSetIndex() ) );

        // CAD Export
        m_Inputs.Add( NameValData( "CADLabelDelim", veh->GetISectSettingsPtr()->m_CADLabelDelim() ) );
        m_Inputs.Add( NameValData( "CADLabelID", veh->GetISectSettingsPtr()->m_CADLabelID() ) );
        m_Inputs.Add( NameValData( "CADLabelName", veh->GetISectSettingsPtr()->m_CADLabelName() ) );
        m_Inputs.Add( NameValData( "CADLabelSplitNo", veh->GetISectSettingsPtr()->m_CADLabelSplitNo() ) );
        m_Inputs.Add( NameValData( "CADLabelSurfNo", veh->GetISectSettingsPtr()->m_CADLabelSurfNo() ) );
        m_Inputs.Add( NameValData( "CADLenUnit", veh->GetISectSettingsPtr()->m_CADLenUnit() ) );
        //m_Inputs.Add( NameValData( "STEPMergePoints", veh->GetISectSettingsPtr()->m_STEPMergePoints() ) );
        m_Inputs.Add( NameValData( "STEPRepresentation", veh->GetISectSettingsPtr()->m_STEPRepresentation() ) );
        m_Inputs.Add( NameValData( "STEPTol", veh->GetISectSettingsPtr()->m_STEPTol() ) );

        // File Outputs
        m_Inputs.Add( NameValData( "CURVFileFlag", veh->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_CURV_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "CURVFileName", veh->GetISectSettingsPtr()->GetExportFileName( vsp::INTERSECT_CURV_FILE_NAME ) ) );
        m_Inputs.Add( NameValData( "SRFFileFlag", veh->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_SRF_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "SRFFileName", veh->GetISectSettingsPtr()->GetExportFileName( vsp::INTERSECT_SRF_FILE_NAME ) ) );
        m_Inputs.Add( NameValData( "P3DFileFlag", veh->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_PLOT3D_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "P3DFileName", veh->GetISectSettingsPtr()->GetExportFileName( vsp::INTERSECT_PLOT3D_FILE_NAME ) ) );
        m_Inputs.Add( NameValData( "IGESFileFlag", veh->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_IGES_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "IGESFileName", veh->GetISectSettingsPtr()->GetExportFileName( vsp::INTERSECT_IGES_FILE_NAME ) ) );
        m_Inputs.Add( NameValData( "STEPFileFlag", veh->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_STEP_FILE_NAME )->Get() ) );
        m_Inputs.Add( NameValData( "STEPFileName", veh->GetISectSettingsPtr()->GetExportFileName( vsp::INTERSECT_STEP_FILE_NAME ) ) );
    }
    else
    {
        printf( "ERROR - trying to set defaults without a vehicle: void SurfaceIntersectionAnalysis::SetDefaults()\n" );
    }
}

string SurfaceIntersectionAnalysis::Execute()
{
    string res_id;
    Vehicle* veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        NameValData* nvd = NULL;

        bool exportRawFlagOrig = veh->GetISectSettingsPtr()->m_ExportRawFlag();
        nvd = m_Inputs.FindPtr( "ExportRawFlag", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->m_ExportRawFlag.Set( nvd->GetInt( 0 ) );

        bool intersectSubSurfsOrig = veh->GetISectSettingsPtr()->m_IntersectSubSurfs();
        nvd = m_Inputs.FindPtr( "IntersectSubSurfs", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->m_IntersectSubSurfs.Set( nvd->GetInt( 0 ) );

        double relCurveTolOrig = veh->GetISectSettingsPtr()->m_RelCurveTol();
        nvd = m_Inputs.FindPtr( "RelCurveTol", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->m_RelCurveTol.Set( nvd->GetDouble( 0 ) );

        int selectedSetIndexOrig = veh->GetISectSettingsPtr()->m_SelectedSetIndex();
        nvd = m_Inputs.FindPtr( "SelectedSetIndex", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->m_SelectedSetIndex.Set( nvd->GetInt( 0 ) );

        // CAD Export
        int cadLabelDelimOrig = veh->GetISectSettingsPtr()->m_CADLabelDelim();
        nvd = m_Inputs.FindPtr( "CADLabelDelim", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->m_CADLabelDelim.Set( nvd->GetInt( 0 ) );

        bool cadLabelIDOrig = veh->GetISectSettingsPtr()->m_CADLabelID();
        nvd = m_Inputs.FindPtr( "CADLabelID", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->m_CADLabelID.Set( nvd->GetInt( 0 ) );

        bool cadLabelNameOrig = veh->GetISectSettingsPtr()->m_CADLabelName();
        nvd = m_Inputs.FindPtr( "CADLabelName", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->m_CADLabelName.Set( nvd->GetInt( 0 ) );

        bool cadLabelSplitNoOrig = veh->GetISectSettingsPtr()->m_CADLabelSplitNo();
        nvd = m_Inputs.FindPtr( "CADLabelSplitNo", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->m_CADLabelSplitNo.Set( nvd->GetInt( 0 ) );

        bool cadLabelSurfNoOrig = veh->GetISectSettingsPtr()->m_CADLabelSurfNo();
        nvd = m_Inputs.FindPtr( "CADLabelSurfNo", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->m_CADLabelSurfNo.Set( nvd->GetInt( 0 ) );

        int cadLenUnitOrig = veh->GetISectSettingsPtr()->m_CADLenUnit();
        nvd = m_Inputs.FindPtr( "CADLenUnit", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->m_CADLenUnit.Set( nvd->GetInt( 0 ) );

        //bool stepMergePointsOrig = veh->GetISectSettingsPtr()->m_STEPMergePoints();
        //nvd = m_Inputs.FindPtr( "STEPMergePoints", 0 );
        //if ( nvd ) veh->GetISectSettingsPtr()->m_STEPMergePoints.Set( nvd->GetInt( 0 ) );

        int stepRepresentationOrig = veh->GetISectSettingsPtr()->m_STEPRepresentation();
        nvd = m_Inputs.FindPtr( "STEPRepresentation", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->m_STEPRepresentation.Set( nvd->GetInt( 0 ) );

        double stepTolOrig = veh->GetISectSettingsPtr()->m_STEPTol();
        nvd = m_Inputs.FindPtr( "STEPTol", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->m_STEPTol.Set( nvd->GetDouble( 0 ) );

        // File Outputs
        bool curvFileFlagOrig = veh->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_CURV_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "CURVFileFlag", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->SetFileExportFlag( vsp::INTERSECT_CURV_FILE_NAME, nvd->GetInt( 0 ) );

        string curvFileNameOrig = veh->GetISectSettingsPtr()->GetExportFileName( vsp::INTERSECT_CURV_FILE_NAME );
        nvd = m_Inputs.FindPtr( "CURVFileName", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::INTERSECT_CURV_FILE_NAME );

        bool srfFileFlagOrig = veh->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_SRF_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "SRFFileFlag", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->SetFileExportFlag( vsp::INTERSECT_SRF_FILE_NAME, nvd->GetInt( 0 ) );

        string srfFileNameOrig = veh->GetISectSettingsPtr()->GetExportFileName( vsp::INTERSECT_SRF_FILE_NAME );
        nvd = m_Inputs.FindPtr( "SRFFileName", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::INTERSECT_SRF_FILE_NAME );

        bool p3dFileFlagOrig = veh->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_PLOT3D_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "P3DFileFlag", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->SetFileExportFlag( vsp::INTERSECT_PLOT3D_FILE_NAME, nvd->GetInt( 0 ) );

        string p3dFileNameOrig = veh->GetISectSettingsPtr()->GetExportFileName( vsp::INTERSECT_PLOT3D_FILE_NAME );
        nvd = m_Inputs.FindPtr( "P3DFileName", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::INTERSECT_PLOT3D_FILE_NAME );

        bool igesFileFlagOrig = veh->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_IGES_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "IGESFileFlag", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->SetFileExportFlag( vsp::INTERSECT_IGES_FILE_NAME, nvd->GetInt( 0 ) );

        string igesFileNameOrig = veh->GetISectSettingsPtr()->GetExportFileName( vsp::INTERSECT_IGES_FILE_NAME );
        nvd = m_Inputs.FindPtr( "IGESFileName", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::INTERSECT_IGES_FILE_NAME );

        bool stepFileFlagOrig = veh->GetISectSettingsPtr()->GetExportFileFlag( vsp::INTERSECT_STEP_FILE_NAME )->Get();
        nvd = m_Inputs.FindPtr( "STEPFileFlag", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->SetFileExportFlag( vsp::INTERSECT_STEP_FILE_NAME, nvd->GetInt( 0 ) );

        string stepFileNameOrig = veh->GetISectSettingsPtr()->GetExportFileName( vsp::INTERSECT_STEP_FILE_NAME );
        nvd = m_Inputs.FindPtr( "STEPFileName", 0 );
        if( nvd ) veh->GetISectSettingsPtr()->SetExportFileName( nvd->GetString( 0 ), vsp::INTERSECT_STEP_FILE_NAME );

        // Execute analysis
        SurfaceIntersectionMgr.IntersectSurfaces(); // TODO: Add results

        // ==== Restore original values that were overwritten by analysis inputs ==== //
        veh->GetISectSettingsPtr()->m_ExportRawFlag.Set( exportRawFlagOrig );
        veh->GetISectSettingsPtr()->m_IntersectSubSurfs.Set( intersectSubSurfsOrig );
        veh->GetISectSettingsPtr()->m_RelCurveTol.Set( relCurveTolOrig );
        veh->GetISectSettingsPtr()->m_SelectedSetIndex.Set( selectedSetIndexOrig );

        // CAD Export
        veh->GetISectSettingsPtr()->m_CADLabelDelim.Set( cadLabelDelimOrig );
        veh->GetISectSettingsPtr()->m_CADLabelID.Set( cadLabelIDOrig );
        veh->GetISectSettingsPtr()->m_CADLabelName.Set( cadLabelNameOrig );
        veh->GetISectSettingsPtr()->m_CADLabelSplitNo.Set( cadLabelSplitNoOrig );
        veh->GetISectSettingsPtr()->m_CADLabelSurfNo.Set( cadLabelSurfNoOrig );
        veh->GetISectSettingsPtr()->m_CADLenUnit.Set( cadLenUnitOrig );
        //veh->GetISectSettingsPtr()->m_STEPMergePoints.Set( stepMergePointsOrig );
        veh->GetISectSettingsPtr()->m_STEPRepresentation.Set( stepRepresentationOrig );
        veh->GetISectSettingsPtr()->m_STEPTol.Set( stepTolOrig );

        // File Outputs
        veh->GetISectSettingsPtr()->SetFileExportFlag( vsp::INTERSECT_CURV_FILE_NAME, curvFileFlagOrig );
        veh->GetISectSettingsPtr()->SetExportFileName( curvFileNameOrig, vsp::INTERSECT_CURV_FILE_NAME );
        veh->GetISectSettingsPtr()->SetFileExportFlag( vsp::INTERSECT_SRF_FILE_NAME, srfFileFlagOrig );
        veh->GetISectSettingsPtr()->SetExportFileName( srfFileNameOrig, vsp::INTERSECT_SRF_FILE_NAME );
        veh->GetISectSettingsPtr()->SetFileExportFlag( vsp::INTERSECT_PLOT3D_FILE_NAME, p3dFileFlagOrig );
        veh->GetISectSettingsPtr()->SetExportFileName( p3dFileNameOrig, vsp::INTERSECT_PLOT3D_FILE_NAME );
        veh->GetISectSettingsPtr()->SetFileExportFlag( vsp::INTERSECT_IGES_FILE_NAME, igesFileFlagOrig );
        veh->GetISectSettingsPtr()->SetExportFileName( igesFileNameOrig, vsp::INTERSECT_IGES_FILE_NAME );
        veh->GetISectSettingsPtr()->SetFileExportFlag( vsp::INTERSECT_STEP_FILE_NAME, stepFileFlagOrig );
        veh->GetISectSettingsPtr()->SetExportFileName( stepFileNameOrig, vsp::INTERSECT_STEP_FILE_NAME );

    }

    return res_id;
}
