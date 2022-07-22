//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "Vehicle.h"
#include "PodGeom.h"
#include "FuselageGeom.h"
#include "WingGeom.h"
#include "BlankGeom.h"
#include "BORGeom.h"
#include "MeshGeom.h"
#include "HumanGeom.h"
#include "ConformalGeom.h"
#include "CustomGeom.h"
#include "EllipsoidGeom.h"
#include "PtCloudGeom.h"
#include "PropGeom.h"
#include "HingeGeom.h"
#include "ScriptMgr.h"
#include "StlHelper.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "MeasureMgr.h"
#include "AdvLinkMgr.h"
#include "AnalysisMgr.h"
#include "ParasiteDragMgr.h"
#include "Quat.h"
#include "StringUtil.h"
#include "SubSurfaceMgr.h"
#include "StructureMgr.h"
#include "DesignVarMgr.h"
#include "DXFUtil.h"
#include "SVGUtil.h"
#include "FitModelMgr.h"
#include "FileUtil.h"
#include "VarPresetMgr.h"
#include "VSPAEROMgr.h"
#include "WireGeom.h"

#include "ProjectionMgr.h"

using namespace vsp;

//==== Constructor ====//
Vehicle::Vehicle()
{
    m_STEPLenUnit.Init( "LenUnit", "STEPSettings", this, vsp::LEN_FT, vsp::LEN_MM, vsp::LEN_YD );
    m_STEPTol.Init( "Tolerance", "STEPSettings", this, 1e-6, 1e-12, 1e12 );
    m_STEPSplitSurfs.Init( "SplitSurfs", "STEPSettings", this, true, 0, 1 );
    m_STEPSplitSubSurfs.Init( "SplitSubSurfs", "STEPSettings", this, false, 0, 1 );
    m_STEPMergePoints.Init( "MergePoints", "STEPSettings", this, false, 0, 1 );
    m_STEPToCubic.Init( "ToCubic", "STEPSettings", this, false, 0, 1 );
    m_STEPToCubic.SetDescript( "Flag to Demote Higher Order Surfaces to Cubic in STEP Export" );
    m_STEPToCubicTol.Init( "ToCubicTol", "STEPSettings", this, 1e-6, 1e-12, 1e12 );
    m_STEPToCubicTol.SetDescript( "Tolerance Used When Demoting Higher Order Surfaces to Cubic" );
    m_STEPTrimTE.Init( "TrimTE", "STEPSettings", this, false, 0, 1 );
    m_STEPExportPropMainSurf.Init( "ExportPropMainSurf", "STEPSettings", this, false, 0, 1 );

    m_STEPLabelID.Init( "LabelID", "STEPSettings", this, true, 0, 1 );
    m_STEPLabelName.Init( "LabelName", "STEPSettings", this, true, 0, 1 );
    m_STEPLabelSurfNo.Init( "LabelSurfNo", "STEPSettings", this, true, 0, 1 );
    m_STEPLabelDelim.Init( "LabelDelim", "STEPSettings", this, vsp::DELIM_COMMA, vsp::DELIM_COMMA, vsp::DELIM_NUM_TYPES - 1 );

    m_STEPStructureExportIndex.Init( "StructureExportIndex", "STEPSettings", this, 0, 0, 1000 );
    m_STEPStructureTol.Init( "StructureTolerance", "STEPSettings", this, 1e-6, 1e-12, 1e12 );
    m_STEPStructureSplitSurfs.Init( "StructureSplitSurfs", "STEPSettings", this, true, 0, 1 );
    m_STEPStructureMergePoints.Init( "StructureMergePoints", "STEPSettings", this, false, 0, 1 );
    m_STEPStructureToCubic.Init( "StructureToCubic", "STEPSettings", this, false, 0, 1 );
    m_STEPStructureToCubic.SetDescript( "Flag to Demote Higher Order Surfaces to Cubic in STEP Export" );
    m_STEPStructureToCubicTol.Init( "StructureToCubicTol", "STEPSettings", this, 1e-6, 1e-12, 1e12 );
    m_STEPStructureToCubicTol.SetDescript( "Tolerance Used When Demoting Higher Order Surfaces to Cubic" );

    m_STEPStructureLabelID.Init( "StructureLabelID", "STEPSettings", this, true, 0, 1 );
    m_STEPStructureLabelName.Init( "StructureLabelName", "STEPSettings", this, true, 0, 1 );
    m_STEPStructureLabelSurfNo.Init( "StructureLabelSurfNo", "STEPSettings", this, true, 0, 1 );
    m_STEPStructureLabelDelim.Init( "StructureLabelDelim", "STEPSettings", this, vsp::DELIM_COMMA, vsp::DELIM_COMMA, vsp::DELIM_NUM_TYPES - 1 );

    m_IGESLenUnit.Init( "LenUnit", "IGESSettings", this, vsp::LEN_FT, vsp::LEN_MM, vsp::LEN_FT );
    m_IGESSplitSurfs.Init( "SplitSurfs", "IGESSettings", this, true, 0, 1 );
    m_IGESSplitSubSurfs.Init( "SplitSubSurfs", "IGESSettings", this, false, 0, 1 );
    m_IGESToCubic.Init( "ToCubic", "IGESSettings", this, false, 0, 1 );
    m_IGESToCubic.SetDescript( "Flag to Demote Higher Order Surfaces to Cubic in IGES Export" );
    m_IGESToCubicTol.Init( "ToCubicTol", "IGESSettings", this, 1e-6, 1e-12, 1e12 );
    m_IGESToCubicTol.SetDescript( "Tolerance Used When Demoting Higher Order Surfaces to Cubic" );
    m_IGESTrimTE.Init( "TrimTE", "IGESSettings", this, false, 0, 1 );
    m_IGESExportPropMainSurf.Init( "ExportPropMainSurf", "IGESSettings", this, false, 0, 1 );

    m_IGESLabelID.Init( "LabelID", "IGESSettings", this, true, 0, 1 );
    m_IGESLabelName.Init( "LabelName", "IGESSettings", this, true, 0, 1 );
    m_IGESLabelSurfNo.Init( "LabelSurfNo", "IGESSettings", this, true, 0, 1 );
    m_IGESLabelSplitNo.Init( "LabelSplitNo", "IGESSettings", this, true, 0, 1 );
    m_IGESLabelDelim.Init( "LabelDelim", "IGESSettings", this, vsp::DELIM_COMMA, vsp::DELIM_COMMA, vsp::DELIM_NUM_TYPES - 1 );

    m_IGESStructureExportIndex.Init( "StructureExportIndex", "IGESSettings", this, 0, 0, 1000 );
    m_IGESStructureSplitSurfs.Init( "StructureSplitSurfs", "IGESSettings", this, true, 0, 1 );
    m_IGESStructureToCubic.Init( "StructureToCubic", "IGESSettings", this, false, 0, 1 );
    m_IGESStructureToCubic.SetDescript( "Flag to Demote Higher Order Surfaces to Cubic in IGES Export" );
    m_IGESStructureToCubicTol.Init( "StructureToCubicTol", "IGESSettings", this, 1e-6, 1e-12, 1e12 );
    m_IGESStructureToCubicTol.SetDescript( "Tolerance Used When Demoting Higher Order Surfaces to Cubic" );

    m_IGESStructureLabelID.Init( "StructureLabelID", "IGESSettings", this, true, 0, 1 );
    m_IGESStructureLabelName.Init( "StructureLabelName", "IGESSettings", this, true, 0, 1 );
    m_IGESStructureLabelSurfNo.Init( "StructureLabelSurfNo", "IGESSettings", this, true, 0, 1 );
    m_IGESStructureLabelSplitNo.Init( "StructureLabelSplitNo", "IGESSettings", this, true, 0, 1 );
    m_IGESStructureLabelDelim.Init( "StructureLabelDelim", "IGESSettings", this, vsp::DELIM_COMMA, vsp::DELIM_COMMA, vsp::DELIM_NUM_TYPES - 1 );

    m_DXFLenUnit.Init( "LenUnit", "DXFSettings", this, vsp::LEN_FT, vsp::LEN_MM, vsp::LEN_UNITLESS );
    m_DXFLenUnit.SetDescript( "Sets DXF Header Units; Numeric Values Unchanged" );
    m_DXFProjectionFlag.Init( "DXFProjectionFlag", "DXFSettings", this , false, 0, 1 );
    m_DXFProjectionFlag.SetDescript( "Flag To Export Geom and Vehicle Projection Lines" );
    m_DXFTessFactor.Init( "DXFTessFactor", "DXFSettings", this, 2, 0, 100 );
    m_DXFTessFactor.SetDescript( "DXF Tessellation Multiplier. Caution: May Slow Export" );
    m_DXFAllXSecFlag.Init( "DXFAllXSecFlag", "DXFSettings", this, false, 0, 1 );
    m_DXFAllXSecFlag.SetDescript( "Flag To Export XSec Feature Lines" );
    m_DXFColorFlag.Init( "DXFColorFlag", "DXFSettings", this, false, 0, 1 );
    m_DXFColorFlag.SetDescript( "Flag To Make Each Layer A Different Color" );
    m_DXF2D3DFlag.Init( "DimFlag", "DXFSettings", this , vsp::SET_3D, vsp::SET_3D, vsp::SET_2D );
    m_DXF2DView.Init( "ViewType", "DXFSettings", this, vsp::VIEW_1, vsp::VIEW_1, vsp::VIEW_4 );
    m_DXF2DView.SetDescript( "Sets Number Of 2D Views" );
    m_DXF4View1.Init( "TopLeftView", "DXFSettings", this, vsp::VIEW_TOP, vsp::VIEW_LEFT, vsp::VIEW_NONE );
    m_DXF4View2.Init( "TopRightView", "DXFSettings", this, vsp::VIEW_TOP, vsp::VIEW_LEFT, vsp::VIEW_NONE );
    m_DXF4View3.Init( "BottomLeftView", "DXFSettings", this, vsp::VIEW_TOP, vsp::VIEW_LEFT, vsp::VIEW_NONE );
    m_DXF4View4.Init( "BottomRightView", "DXFSettings", this, vsp::VIEW_TOP, vsp::VIEW_LEFT, vsp::VIEW_NONE );
    m_DXF4View1_rot.Init( "TopLeftRotation", "DXFSettings", this, vsp::ROT_90, vsp::ROT_0, vsp::ROT_270 );
    m_DXF4View2_rot.Init( "TopRightRotation", "DXFSettings", this, vsp::ROT_0, vsp::ROT_0, vsp::ROT_270 );
    m_DXF4View3_rot.Init( "BottomLeftRotation", "DXFSettings", this, vsp::ROT_0, vsp::ROT_0, vsp::ROT_270 );
    m_DXF4View4_rot.Init( "BottomRightRotation", "DXFSettings", this, vsp::ROT_0, vsp::ROT_0, vsp::ROT_270 );

    m_SVGLenUnit.Init( "LenUnit", "SVGSettings", this, vsp::LEN_FT, vsp::LEN_MM, vsp::LEN_UNITLESS );
    m_SVGLenUnit.SetDescript( "Sets Scale Bar Units; Numeric Values Unchanged" );
    m_SVGSet.Init( "SVGSet", "SVGSettings", this, 0, 0, 12 );
    m_Scale.Init( "Scale", "SVGSettings", this, 0, 0, 1e12 );
    m_Scale.SetDescript( "Sets Scale Bar Size" );
    m_SVGProjectionFlag.Init( "SVGProjectionFlag", "SVGSettings", this , false, 0, 1 );
    m_SVGProjectionFlag.SetDescript( "Flag To Export Geom and Vehicle Projection Lines" );
    m_SVGTessFactor.Init( "SVGTessFactor", "SVGSettings", this, 2, 0, 100 );
    m_SVGTessFactor.SetDescript( "SVG Tessellation Multiplier. Caution: May Slow Export" );
    m_SVGAllXSecFlag.Init( "SVGAllXSecFlag", "SVGSettings", this, false, 0, 1 );
    m_SVGAllXSecFlag.SetDescript( "Flag To Export XSec Feature Lines" );
    m_SVGView.Init( "ViewType", "SVGSettings", this, vsp::VIEW_1, vsp::VIEW_1, vsp::VIEW_4 );
    m_SVGView.SetDescript( "Sets Number Of 2D Views" );
    m_SVGView1.Init( "TopLeftView", "SVGSettings", this, vsp::VIEW_TOP, vsp::VIEW_LEFT, vsp::VIEW_NONE );
    m_SVGView2.Init( "TopRightView", "SVGSettings", this, vsp::VIEW_TOP, vsp::VIEW_LEFT, vsp::VIEW_NONE );
    m_SVGView3.Init( "BottomLeftView", "SVGSettings", this, vsp::VIEW_TOP, vsp::VIEW_LEFT, vsp::VIEW_NONE );
    m_SVGView4.Init( "BottomRightView", "SVGSettings", this, vsp::VIEW_TOP, vsp::VIEW_LEFT, vsp::VIEW_NONE );
    m_SVGView1_rot.Init( "TopLeftRotation", "SVGSettings", this, vsp::ROT_90, vsp::ROT_0, vsp::ROT_270 );
    m_SVGView2_rot.Init( "TopRightRotation", "SVGSettings", this, vsp::ROT_0, vsp::ROT_0, vsp::ROT_270 );
    m_SVGView3_rot.Init( "BottomLeftRotation", "SVGSettings", this, vsp::ROT_0, vsp::ROT_0, vsp::ROT_270 );
    m_SVGView4_rot.Init( "BottomRightRotation", "SVGSettings", this, vsp::ROT_0, vsp::ROT_0, vsp::ROT_270 );

    m_WorkingXDDMType.Init( "Working_XDDM_Type", "Design", this, vsp::XDDM_VAR, vsp::XDDM_VAR, vsp::XDDM_CONST );

    m_UType.Init( "U_Type", "FitModel", this, TargetPt::FREE, TargetPt::FIXED, TargetPt::FREE );
    m_UType.SetDescript( "Target U fixed or free" );
    m_UTargetPt.Init( "U_TargetPt", "FitModel", this, 0, 0, 1 );
    m_UTargetPt.SetDescript( "U Coordinate of Fixed Point" );
    m_WType.Init( "W_Type", "FitModel", this, TargetPt::FREE, TargetPt::FIXED, TargetPt::FREE );
    m_WType.SetDescript( "Target W fixed or free" );
    m_WTargetPt.Init( "W_TargetPt", "FitModel", this, 0, 0, 1 );
    m_WTargetPt.SetDescript( "W Coordinate of Fixed Point" );
    m_SelectOneFlag.Init( "Select_One_Flag", "FitModel", this, false, 0, 1 );
    m_SelectBoxFlag.Init( "Select_Box_Flag", "FitModel", this, false, 0, 1 );

    m_TargetType.Init( "TargetType", "Projection", this, vsp::SET_TARGET, vsp::SET_TARGET, vsp::NUM_PROJ_TGT_OPTIONS - 1 );
    m_BoundaryType.Init( "BoundaryType", "Projection", this, vsp::NO_BOUNDARY, vsp::NO_BOUNDARY, vsp::NUM_PROJ_BNDY_OPTIONS - 1 );
    m_DirectionType.Init( "DirectionType", "Projection", this, vsp::X_PROJ, vsp::X_PROJ, vsp::NUM_PROJ_DIR_OPTIONS - 1 );
    m_XComp.Init( "XComp", "Projection", this, 0.0, -1.0, 1.0 );
    m_YComp.Init( "YComp", "Projection", this, 0.0, -1.0, 1.0 );
    m_ZComp.Init( "ZComp", "Projection", this, 0.0, -1.0, 1.0 );

    m_ViewportSizeXValue.Init( "ViewportX", "AdjustView", this, 0.0, 0, 1.0e12 );
    m_ViewportSizeYValue.Init( "ViewportY", "AdjustView", this, 0.0, 0, 1.0e12 );
    m_CORXValue.Init( "CORX", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );
    m_CORYValue.Init( "CORY", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );
    m_CORZValue.Init( "CORZ", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );
    m_PanXPosValue.Init( "PanX", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );
    m_PanYPosValue.Init( "PanY", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );
    m_ZoomValue.Init( "Zoom", "AdjustView", this, 1e-3, 1e-6, 10 );
    m_XRotationValue.Init( "RotationX", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );
    m_YRotationValue.Init( "RotationY", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );
    m_ZRotationValue.Init( "RotationZ", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );

    m_NewRatioValue.Init( "Ratio", "Screenshot", this, 1.0, 0.0, 1.0e12 );
    m_NewWidthValue.Init( "Width", "Screenshot", this, 1.0, 0.0, 1.0e12 );
    m_NewHeightValue.Init( "Height", "Screenshot", this, 1.0, 0.0, 1.0e12 );
    m_TransparentBGFlag.Init( "TransparentBGFlag", "Screenshot", this, 1, 0, 1 );

    m_UserParmVal.Init( "Val", "UserParm", this, 0.0, -1.0e12, 1.0e12 );
    m_UserParmMin.Init( "Min", "UserParm", this, -1.0e5, -1.0e12, 1.0e12 );
    m_UserParmMax.Init( "Max", "UserParm", this, 1.0e5, -1.0e12, 1.0e12 );

    m_BGWidthScaleValue.Init( "WidthScale", "Background", this, 1.0, -1.0e12, 1.0e12 );
    m_BGHeightScaleValue.Init( "HeightScale", "Background", this, 1.0, -1.0e12, 1.0e12 );
    m_BGXOffsetValue.Init( "XOffset", "Background", this, 0.0, -1.0e12, 1.0e12 );
    m_BGYOffsetValue.Init( "YOffset", "Background", this, 0.0, -1.0e12, 1.0e12 );

    m_AFExportType.Init( "AFExportType", "AirfoilExport", this, vsp::BEZIER_AF_EXPORT, vsp::SELIG_AF_EXPORT, vsp::BEZIER_AF_EXPORT );
    m_AFExportType.SetDescript( "Airfoil Representation Written to File" );
    m_AFWTessFactor.Init( "AFWTessFactor", "AirfoilExport", this, 1.0, 0.01, 100 );
    m_AFWTessFactor.SetDescript( "Airfoil W Tessellation Factor" );
    m_AFAppendGeomIDFlag.Init( "AFAppendGeomIDFlag", "AirfoilExport", this, true, false, true );
    m_AFAppendGeomIDFlag.SetDescript( "Airfoil W Tessellation Factor" );

    m_STLMultiSolid.Init( "MultiSolid", "STLSettings", this, false, 0, 1 );
    m_STLExportPropMainSurf.Init( "ExportPropMainSurf", "STLSettings", this, false, 0, 1 );

    m_UpdatingBBox = false;
    m_BbXLen.Init( "X_Len", "BBox", this, 0, 0, 1e12 );
    m_BbXLen.SetDescript( "X length of vehicle bounding box" );
    m_BbYLen.Init( "Y_Len", "BBox", this, 0, 0, 1e12 );
    m_BbYLen.SetDescript( "Y length of vehicle bounding box" );
    m_BbZLen.Init( "Z_Len", "BBox", this, 0, 0, 1e12 );
    m_BbZLen.SetDescript( "Z length of vehicle bounding box" );
    m_BbXMin.Init( "X_Min", "BBox", this, 0, -1e12, 1e12 );
    m_BbXMin.SetDescript( "Minimum X coordinate of vehicle bounding box" );
    m_BbYMin.Init( "Y_Min", "BBox", this, 0, -1e12, 1e12 );
    m_BbYMin.SetDescript( "Minimum Y coordinate of vehicle bounding box" );
    m_BbZMin.Init( "Z_Min", "BBox", this, 0, -1e12, 1e12 );
    m_BbZMin.SetDescript( "Minimum Z coordinate of vehicle bounding box" );

    m_exportCompGeomCsvFile.Init( "CompGeom_CSV_Export", "ExportFlag", this, true, 0, 1 );
    m_exportDegenGeomCsvFile.Init( "DegenGeom_CSV_Export", "ExportFlag", this, true, 0, 1 );
    m_exportDegenGeomMFile.Init( "DegenGeom_M_Export", "ExportFlag", this, true, 0, 1 );

    m_CopySetsWithGeomsFlag.Init("CopySetsWithGeomsFlag", "SetEditor", this, true, false, true);
    m_CopySetsWithGeomsFlag.SetDescript("Flag if Geoms sets get copied with Geoms");

    m_AxisLength.Init( "AxisLength", "Axis", this, 1.0, 1e-12, 1e12 );
    m_AxisLength.SetDescript( "Length of axis icon displayed on screen" );

    m_TextSize.Init( "TextSize", "Text", this, 2.0, 0.0, 100.0 );
    m_TextSize.SetDescript( "Size of text labels displayed on screen" );

    m_MeasureLenUnit.Init( "LenUnit", "Measure", this, vsp::LEN_UNITLESS, vsp::LEN_MM, vsp::LEN_UNITLESS );

    m_StructUnit.Init( "StructUnit", "FeaStructure", this, vsp::BFT_UNIT, vsp::SI_UNIT, vsp::BIN_UNIT );
    m_StructUnit.SetDescript( "Unit System for FEA Structures" );

    m_NumMassSlices.Init( "NumMassSlices", "MassProperties", this, 20, 10, 200 );
    m_NumMassSlices.SetDescript( "Number of slices used to display mesh" );

    m_DrawCgFlag.Init( "DrawCgFlag", "MassProperties", this, true, false, true );
    m_DrawCgFlag.SetDescript( "Adds red center point to mesh" );

    m_NumPlanerSlices.Init( "NumPlanerSlices", "PSlice", this, 10, 1, 100 );
    m_NumPlanerSlices.SetDescript( "Number of planar slices used to display mesh" );

    m_AutoBoundsFlag.Init( "AutoBoundsFlag", "PSlice", this, true, false, true );
    m_AutoBoundsFlag.SetDescript( "Automatically sets Planar Start and End locations" );

    m_PlanarAxisType.Init( "PlanarAxisType", "PSlice", this, vsp::X_DIR, vsp::X_DIR, vsp::Z_DIR );
    m_PlanarAxisType.SetDescript( "Selects from X,Y,Z Axis for Planar Slice" );
    
    m_PlanarStartLocation.Init( "PlanarStartLocation", "PSlice", this, 0, -1e12, 1e12 );
    m_PlanarStartLocation.SetDescript( "Planar Start Location" );

    m_PlanarEndLocation.Init( "PlanarEndLocation", "PSlice", this, 10, -1e12, 1e12 );
    m_PlanarEndLocation.SetDescript( "Planar End Location" );

    SetupPaths();
    m_VehProjectVec3d.resize( 3 );
    m_ColorCount = 0;

    // Protect required enum value.
    assert( CUSTOM_GEOM_TYPE == 9 );
}

//==== Destructor ====//
Vehicle::~Vehicle()
{
    LinkMgr.UnRegisterContainer( this->GetID() );

    for ( int i = 0 ; i < ( int )m_GeomStoreVec.size() ; i++ )
    {
        delete m_GeomStoreVec[i];
    }
}

//=== Init ====//
void Vehicle::Init()
{
    //==== Init Custom Geom and Script Mgr ====//
    LightMgr.Init();
    CustomGeomMgr.Init();
    ScriptMgr.Init();
    AdvLinkMgr.Init();
    CustomGeomMgr.ReadCustomScripts( this );

    m_Name = "Vehicle";

    SetVSP3FileName( "Unnamed.vsp3" );
    m_FileOpenVersion = -1;

    //==== Load Default Set Names =====//
    m_SetNameVec.push_back( "All" );        // SET_ALL
    m_SetNameVec.push_back( "Shown" );      // SET_SHOWN
    m_SetNameVec.push_back( "Not_Shown" );  // SET_NOT_SHOWN
    for ( int i = 0 ; i < NUM_SETS; i++ )
    {
        char str[256];
        sprintf( str, "Set_%d", i );
        m_SetNameVec.push_back( str );
    }

    //==== Load Geom Types =====//
    m_GeomTypeVec.push_back( GeomType( POD_GEOM_TYPE, "POD", true ) );
    m_GeomTypeVec.push_back( GeomType( FUSELAGE_GEOM_TYPE, "FUSELAGE", true ) );
    m_GeomTypeVec.push_back( GeomType( MS_WING_GEOM_TYPE, "WING", true ) );
    m_GeomTypeVec.push_back( GeomType( STACK_GEOM_TYPE, "STACK", true ) );
    m_GeomTypeVec.push_back( GeomType( BLANK_GEOM_TYPE, "BLANK", true ) );
    m_GeomTypeVec.push_back( GeomType( ELLIPSOID_GEOM_TYPE, "ELLIPSOID", true ) );
    m_GeomTypeVec.push_back( GeomType( BOR_GEOM_TYPE, "BODYOFREVOLUTION", true ) );
    m_GeomTypeVec.push_back( GeomType( HUMAN_GEOM_TYPE, "HUMAN", true ) );
    m_GeomTypeVec.push_back( GeomType( PROP_GEOM_TYPE, "PROP", true ) );
    m_GeomTypeVec.push_back( GeomType( HINGE_GEOM_TYPE, "HINGE", true ) );
    m_GeomTypeVec.push_back( GeomType( CONFORMAL_GEOM_TYPE, "CONFORMAL", true ) );

    //==== Get Custom Geom Types =====//
    vector< GeomType > custom_types = CustomGeomMgr.GetCustomTypes();
    for ( int i = 0 ; i < ( int ) custom_types.size() ; i++ )
    {
        m_GeomTypeVec.push_back( custom_types[i] );
    }

    LinkMgr.RegisterContainer( this->GetID() );
    LinkMgr.RegisterContainer( m_CfdSettings.GetID() );
    LinkMgr.RegisterContainer( m_ISectSettings.GetID() );
    LinkMgr.RegisterContainer( m_CfdGridDensity.GetID() );
    LinkMgr.RegisterContainer( VSPAEROMgr.GetID() );
    LinkMgr.RegisterContainer( WaveDragMgr.GetID() );
    LinkMgr.RegisterContainer( ParasiteDragMgr.GetID() );

    m_IxxIyyIzz = vec3d( 0, 0, 0 );
    m_IxyIxzIyz = vec3d( 0, 0, 0 );
    m_CG = vec3d( 0, 0, 0 );
    m_NumMassSlices = 20;
    m_TotalMass = 0;

    m_STEPLenUnit.Set( vsp::LEN_FT );
    m_STEPTol.Set( 1e-6 );
    m_STEPSplitSurfs.Set( true );
    m_STEPSplitSubSurfs.Set( false );
    m_STEPMergePoints.Set( false );
    m_STEPToCubic.Set( false );
    m_STEPToCubicTol.Set( 1e-6 );
    m_STEPTrimTE.Set( false );

    m_IGESLenUnit.Set( vsp::LEN_FT );
    m_IGESSplitSurfs.Set( true );
    m_IGESSplitSubSurfs.Set( false );
    m_IGESToCubic.Set( false );
    m_IGESToCubicTol.Set( 1e-6 );
    m_IGESTrimTE.Set( false );

    //=== DXF Initial Conditions ===//
    m_DXFLenUnit.Set( vsp::LEN_FT );
    m_DXF2DView.Set( vsp::VIEW_4 );
    m_DXF2D3DFlag.Set( vsp::SET_3D );
    m_DXF4View1.Set( vsp::VIEW_TOP );
    m_DXF4View2.Set( vsp::VIEW_NONE );
    m_DXF4View3.Set( vsp::VIEW_FRONT );
    m_DXF4View4.Set( vsp::VIEW_LEFT );
    m_DXF4View1_rot.Set( vsp::ROT_90 );
    m_DXF4View2_rot.Set( vsp::ROT_0 );
    m_DXF4View3_rot.Set( vsp::ROT_0 );
    m_DXF4View4_rot.Set( vsp::ROT_0 );

    //=== SVG Initial Conditions ===//
    m_SVGLenUnit.Set( vsp::LEN_FT );
    m_SVGView.Set( vsp::VIEW_4 );
    m_SVGView1.Set( vsp::VIEW_TOP );
    m_SVGView2.Set( vsp::VIEW_NONE );
    m_SVGView3.Set( vsp::VIEW_FRONT );
    m_SVGView4.Set( vsp::VIEW_LEFT );
    m_SVGView1_rot.Set( vsp::ROT_90 );
    m_SVGView2_rot.Set( vsp::ROT_0 );
    m_SVGView3_rot.Set( vsp::ROT_0 );
    m_SVGView4_rot.Set( vsp::ROT_0 );

    m_WorkingXDDMType.Set( vsp::XDDM_VAR );

    m_UType.Set( TargetPt::FREE );
    m_UTargetPt.Set( 0 );
    m_WType.Set( TargetPt::FREE );
    m_WTargetPt.Set( 0 );
    m_SelectOneFlag.Set( false );
    m_SelectBoxFlag.Set( false );

    m_TargetType.Set( vsp::SET_TARGET );
    m_BoundaryType.Set( vsp::NO_BOUNDARY );
    m_DirectionType.Set( vsp::X_PROJ );
    m_XComp.Set( 0.0 );
    m_YComp.Set( 0.0 );
    m_ZComp.Set( 0.0 );

    m_NewRatioValue.Set( 1.0 );
    m_NewWidthValue.Set( 1.0 );
    m_NewHeightValue.Set( 1.0 );
    m_TransparentBGFlag.Set( 1 );

    m_STLMultiSolid.Set( false );
    m_STLExportPropMainSurf.Set( false );

    m_BEMPropID = string();

    m_AFExportType.Set( vsp::BEZIER_AF_EXPORT );
    m_AFWTessFactor.Set( 1.0 );
    m_AFAppendGeomIDFlag.Set( true );
    m_AFFileDir = string();

    m_UpdatingBBox = false;
    m_BbXLen.Set( 0 );
    m_BbYLen.Set( 0 );
    m_BbZLen.Set( 0 );
    m_BbXMin.Set( 0 );
    m_BbYMin.Set( 0 );
    m_BbZMin.Set( 0 );

    m_exportCompGeomCsvFile.Set( true );
    m_exportDegenGeomCsvFile.Set( true );
    m_exportDegenGeomMFile.Set( true );

    AnalysisMgr.Init();
}

void Vehicle::RunTestScripts()
{
    ScriptMgr.RunTestScripts();
}

//=== Wype ===//
void Vehicle::Wype()
{
    // Re-initialize everything to default.  Care must be taken with dynamic memory
    // or cases where outside pointers access items being freed.  Otherwise, just
    // clobber it with a default assignment, let the compiler take care of cleaning
    // up the variables on the stack.

    // Remove references to this set up in Init()
    LinkMgr.UnRegisterContainer( this->GetID() );

    // Public member variables
    m_IxxIyyIzz = vec3d();
    m_IxyIxzIyz = vec3d();
    m_CG = vec3d();
    m_NumMassSlices = int();
    m_TotalMass = double();


    // Private member variables
    m_Name = string();

    m_VSP3FileName = string();

    m_AFFileDir = string();

    m_BEMPropID = string();

    for ( int i = 0 ; i < ( int )m_GeomStoreVec.size() ; i++ )
    {
        delete m_GeomStoreVec[i];
    }

    m_GeomStoreVec.clear();

    m_ActiveGeom.clear();
    m_TopGeom.clear();
    m_ClipBoard.clear();
    m_SetNameVec.clear();

//jrg should we clear types????
    m_GeomTypeVec.clear();

    m_BBox = BndBox();

    m_ExportFileNames.clear();

    // Clear out various managers...
    LinkMgr.Renew();
    AdvLinkMgr.Renew();
    DesignVarMgr.Renew();
    FitModelMgr.Renew();
    AnalysisMgr.Renew();
    VarPresetMgr.Renew();
    ParasiteDragMgr.Renew();
    VSPAEROMgr.Renew();
    MeasureMgr.Renew();
    StructureMgr.Renew();

    LightMgr.Wype();
}

void Vehicle::SetVSP3FileName( const string & f_name )
{
    m_VSP3FileName = f_name;

    m_CfdSettings.ResetExportFileNames( m_VSP3FileName );
    m_ISectSettings.ResetExportFileNames( m_VSP3FileName );
    resetExportFileNames();
}

void Vehicle::SetupPaths()
{
    m_ExePath = PathToExe();
    m_HomePath = PathToHome();
    
    // Initialize VSPAERO directory as VSP executable directory. 
    // This may be overwritten from the API, where the VSP executable
    // path and VSPAERO executable path may differ
    m_VSPAEROPath = m_ExePath;

#ifdef WIN32
    m_VSPAEROCmd = string( "vspaero.exe" );
    m_VIEWERCmd = string( "vspviewer.exe" );
    m_LOADSCmd = string( "vsploads.exe" );
#else
    m_VSPAEROCmd = string( "vspaero" );
    m_VIEWERCmd = string( "vspviewer" );
    m_LOADSCmd = string( "vsploads" );
#endif

    if( !CheckForFile( m_ExePath, m_VSPAEROCmd ) )
    {
        printf("VSPAERO solver not found in %s.\n", m_ExePath.c_str());
    }
    if( !CheckForFile( m_ExePath, m_VIEWERCmd ) )
    {
        printf("VSPAERO viewer not found in %s.\n", m_ExePath.c_str());
    }
    if ( !CheckForFile( m_ExePath, m_LOADSCmd ) )
    {
        printf( "VSPAERO loads not found in %s.\n", m_ExePath.c_str());
    }

    m_CustomScriptDirs.push_back( string( "./CustomScripts/" ) );
    m_CustomScriptDirs.push_back( m_HomePath + string( "/CustomScripts/" ) );
    m_CustomScriptDirs.push_back( m_ExePath + string( "/CustomScripts/" ) );
}

bool Vehicle::CheckForVSPAERO( const string & path )
{
    bool ret_val = true;
    string path_file, vspaero_exe, viewer_exe, loads_exe;

#ifdef WIN32
    vspaero_exe = string( "vspaero.exe" );
    viewer_exe = string( "vspviewer.exe" );
    loads_exe = string( "vsploads.exe" );
#else
    vspaero_exe = string( "vspaero" );
    viewer_exe = string( "vspviewer" );
    loads_exe = string( "vsploads" );
#endif

    path_file = path + string( "/" ) + vspaero_exe;

    if( !FileExist( path_file ) )
    {
        fprintf( stderr, "ERROR %d: VSPAERO Solver Not Found. \n"
            "\tExpected here: %s\n"
            "\tFile: %s \tLine: %d\n",
            vsp::VSP_FILE_DOES_NOT_EXIST,
            path_file.c_str(),
            __FILE__, __LINE__ );
        ret_val = false;
    }
    else
    {
        // Save VSPAERO executable
        m_VSPAEROCmd = vspaero_exe;
    }

    path_file = path + string( "/" ) + viewer_exe;

    if( !FileExist( path_file ) )
    {
        fprintf( stderr, "ERROR %d: VSPAERO Viewer Not Found. \n"
            "\tExpected here: %s\n"
            "\tFile: %s \tLine: %d\n",
            vsp::VSP_FILE_DOES_NOT_EXIST,
            path_file.c_str(),
            __FILE__, __LINE__ );
        ret_val = false;
    }
    else
    {
        // Save Viewer executable
        m_VIEWERCmd = viewer_exe;
    }

    path_file = path + string( "/" ) + loads_exe;

    if( !FileExist( path_file ) )
    {
        fprintf( stderr, "ERROR %d: VSPAERO Loads Not Found. \n"
            "\tExpected here: %s\n"
            "\tFile: %s \tLine: %d\n",
            vsp::VSP_FILE_DOES_NOT_EXIST,
            path_file.c_str(),
            __FILE__, __LINE__ );
        ret_val = false;
    }
    else
    {
        // Save Slicer executable
        m_LOADSCmd = loads_exe;
    }

    return ret_val;
}

bool Vehicle::SetVSPAEROPath( const string & path )
{
    if( CheckForVSPAERO( path ) )
    {
        m_VSPAEROPath = path;
        return true;
    }
    return false;
}

//=== NewFile ===//
// Reset VSP state to nearly that of startup.  Leave clipboard contents intact.
void Vehicle::Renew()
{
    Wype();
    Init();
}

void Vehicle::GenAPIDocs( const string & file_name )
{
    ScriptMgr.GenAPIDocs( file_name );
}

//==== Parm Changed ====//
void Vehicle::ParmChanged( Parm* parm_ptr, int type )
{
    if ( m_UpdatingBBox )
    {
        return;
    }

    m_UpdatingBBox = true;
    UpdateBBox();
    m_UpdatingBBox = false;

    if ( parm_ptr == &m_AxisLength )
    {
        ForceUpdate( GeomBase::XFORM );
    }

    UpdateGui();
}

//==== Update All Screens ====//
void Vehicle::UpdateGui()
{
    MessageMgr::getInstance().Send( "ScreenMgr", "UpdateAllScreens" );
}

//==== Undo Last Parameter Change ====//
void Vehicle::UnDo()
{
    ParmMgr.UnDo();
}

//===== Update All Geometry ====//
void Vehicle::Update( bool fullupdate )
{
    for ( int i = 0 ; i < ( int )m_TopGeom.size() ; i++ )
    {
        Geom* g_ptr = FindGeom( m_TopGeom[i] );
        if ( g_ptr )
        {
            g_ptr->Update( fullupdate );
        }
    }

    MeasureMgr.Update();
}

// Update managers that are normally only updated by their 
// associated GUI. This enables update from the API
void Vehicle::UpdateManagers()
{
    VSPAEROMgr.Update();
    WaveDragMgr.Update();
    ParasiteDragMgr.Update();
    // MeasureMgr already updated in Update()
}

void Vehicle::UpdateGeom( const string &geom_id )
{
    Geom* g_ptr = FindGeom( geom_id );
    if ( g_ptr )
    {
        g_ptr->Update( );
    }
}

void Vehicle::ForceUpdate( int dirtyflag )
{
    for ( int i = 0 ; i < ( int )m_GeomStoreVec.size() ; i++ )
    {
        Geom* g_ptr = m_GeomStoreVec[i];
        if ( g_ptr )
        {
            g_ptr->SetLateUpdateFlag( true );
            if ( dirtyflag != GeomBase::NONE )
            {
                g_ptr->SetDirtyFlag( dirtyflag );
            }
        }
    }
    Update();
}

//===== Run Script ====//
int Vehicle::RunScript( const string & file_name, const string & function_name )
{
    return ScriptMgr.ReadExecuteScriptFile( file_name, function_name );
}


//==== Find Geom Based on GeomID ====//
Geom* Vehicle::FindGeom( const string & geom_id )
{
    if ( geom_id == string( "NONE" ) )
    {
        return NULL;
    }
    for ( int i = 0 ; i < ( int )m_GeomStoreVec.size() ; i++ )
    {
        if ( m_GeomStoreVec[i]->IsMatch( geom_id ) )
        {
            return m_GeomStoreVec[i];
        }
    }
    return NULL;
}

//==== Find Vector of Geom Ptrs Based on GeomID ====//
vector< Geom* > Vehicle::FindGeomVec( const vector< string > & geom_id_vec )
{
    vector< Geom* > geom_vec;
    for ( int i = 0 ; i < ( int )geom_id_vec.size() ; i++ )
    {
        Geom* gptr = FindGeom( geom_id_vec[i] );
        if ( gptr != NULL )
        {
            geom_vec.push_back( gptr );
        }
    }
    return geom_vec;
}


//=== Create Geom of Type, Add To Storage and Return ID ====//
string Vehicle::CreateGeom( const GeomType & type )
{
    Geom* new_geom = NULL;

    if ( type.m_Type == CUSTOM_GEOM_TYPE )     // Match Custom on number
    {
        new_geom = new CustomGeom( this );
    }
    else if ( type.m_Name == "Pod" || type.m_Name == "POD" )   // Match all others on name
    {
        new_geom = new PodGeom( this );
    }
    else if ( type.m_Name == "Fuselage" || type.m_Name == "FUSELAGE" )
    {
        new_geom = new FuselageGeom( this );
    }
    else if ( type.m_Name == "Wing" || type.m_Name == "WING" )
    {
        new_geom = new WingGeom( this );
    }
    else if ( type.m_Name == "Blank" || type.m_Name == "BLANK" )
    {
        new_geom = new BlankGeom( this );
    }
    else if ( type.m_Name == "Mesh" || type.m_Name == "MESH" )
    {
        new_geom = new MeshGeom( this );
    }
    else if ( type.m_Name == "Stack" || type.m_Name == "STACK" )
    {
        new_geom = new StackGeom( this );
    }
    else if ( type.m_Name == "PtCloud" || type.m_Name == "PTS" )
    {
        new_geom = new PtCloudGeom( this );
    }
    else if ( type.m_Name == "Propeller" || type.m_Name == "PROP" )
    {
        new_geom = new PropGeom( this );
    }
    else if ( type.m_Name == "Hinge" || type.m_Name == "HINGE" )
    {
        new_geom = new HingeGeom( this );
    }
    else if ( type.m_Name == "Conformal" || type.m_Name == "CONFORMAL" )
    {
        new_geom = new ConformalGeom( this );
    }
    else if ( type.m_Name == "Ellipsoid" || type.m_Name == "ELLIPSOID" )
    {
        new_geom = new EllipsoidGeom( this );
    }
    else if ( type.m_Name == "BodyOfRevolution" || type.m_Name == "BODYOFREVOLUTION" )
    {
        new_geom = new BORGeom( this );
    }
    else if ( type.m_Name == "WireFrame" || type.m_Name == "WIREFRAME" )
    {
        new_geom = new WireGeom( this );
    }
    else if ( type.m_Name == "Human" || type.m_Name == "HUMAN" )
    {
        new_geom = new HumanGeom( this );
    }

    if ( !new_geom )
    {
        printf( "Error: Could not create Geom of type: %s\n", type.m_Name.c_str() );
        return "NONE";
    }

    m_GeomStoreVec.push_back( new_geom );

    Geom* type_geom_ptr = FindGeom( type.m_GeomID );
    if ( type_geom_ptr )
    {
        string id = new_geom->GetID();
        new_geom->CopyFrom( type_geom_ptr );
        new_geom->SetName( type.m_Name );
    }

    return new_geom->GetID();
}

//=== Create Geom and Set Up Parent/Child ====//
string Vehicle::AddGeom( const GeomType & type )
{
    string add_id = CreateGeom( type );
    Geom* add_geom = FindGeom( add_id );

    string geom_id =  AddGeom( add_geom );

    if ( add_geom )
    {
        if ( type.m_Type == CUSTOM_GEOM_TYPE )
        {
            add_geom->SetType( type );
            CustomGeomMgr.InitGeom( geom_id, type.m_ModuleName, type.m_DisplayName );
        }
        //==== Update Conformal After Attachment to Parent ====//
        else if ( type.m_Type == CONFORMAL_GEOM_TYPE )
        {
            string parID = add_geom->GetParentID();
            Geom* par = FindGeom( parID );

            if ( par )
            {
                if ( par->GetType().m_Type == BLANK_GEOM_TYPE ||
                     par->GetType().m_Type == MESH_GEOM_TYPE ||
                     par->GetType().m_Type == HUMAN_GEOM_TYPE ||
                     par->GetType().m_Type == PT_CLOUD_GEOM_TYPE ||
                     par->GetType().m_Type == HINGE_GEOM_TYPE ||
                     par->GetType().m_Type == CONFORMAL_GEOM_TYPE ||
                     par->GetType().m_Type == WIRE_FRAME_GEOM_TYPE )
                {
                    MessageData errMsgData;
                    errMsgData.m_String = "Error";
                    errMsgData.m_IntVec.push_back( vsp::VSP_CONFORMAL_PARENT_UNSUPPORTED );
                    errMsgData.m_StringVec.push_back( string( "Error:  Conformal component not supported for this parent type." ) );

                    MessageMgr::getInstance().SendAll( errMsgData );

                    DeleteGeom( geom_id );
                    return "NONE";
                }
            }
            else
            {
                MessageData errMsgData;
                errMsgData.m_String = "Error";
                errMsgData.m_IntVec.push_back( vsp::VSP_CONFORMAL_PARENT_UNSUPPORTED );
                errMsgData.m_StringVec.push_back( string( "Error:  Conformal component not supported for this parent type." ) );

                MessageMgr::getInstance().SendAll( errMsgData );

                DeleteGeom( geom_id );
                return "NONE";
            }

        }

        add_geom->Update();
    }
    return geom_id;
}


//=== Create Geom and Set Up Parent/Child ====//
string Vehicle::AddGeom( Geom* add_geom )
{
    if ( !add_geom )
    {
        return string( "NONE" );
    }

    string add_id = add_geom->GetID();

    //==== Set Parent/Child ====//
    vector< string > active_vec = GetActiveGeomVec();
    if ( active_vec.size()  )
    {
        string parent_id = active_vec.back();           // Parent
        Geom* parent_geom = FindGeom( parent_id );
        if ( parent_geom )
        {
            add_geom->SetParentID( parent_geom->GetID() );      // Set Parent ID
            parent_geom->AddChildID( add_id );                  // Add Child ID
        }
    }
    else
    {
        m_TopGeom.push_back( add_id );
    }

    UpdateBBox();
    return add_id;
}

string Vehicle::AddMeshGeom( int normal_set, int degen_set, bool suppressdisks )
{
    ClearActiveGeom();

    vector<string> geom_vec = GetGeomVec(); // Get geom vec before mesh is added

    GeomType type = GeomType( MESH_GEOM_TYPE, "MESH", true );
    string id = AddGeom( type );
    Geom* geom_ptr = FindGeom( id );
    if ( !geom_ptr )
    {
        return string( "NONE" );
    }

    MeshGeom* mesh_geom = ( MeshGeom* )( geom_ptr );

    if ( normal_set >= SET_FIRST_USER )
    {
        mesh_geom->SetSetFlag( normal_set, true );
    }
    else
    {
        mesh_geom->SetSetFlag( SET_SHOWN, true );    // Ensure Shown Flag is Set
    }

    // Create TMeshVec
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        Geom* g_ptr = FindGeom( geom_vec[i] );
        if ( g_ptr )
        {
            if ( g_ptr->GetSetFlag( normal_set ) )
            {
                vector< TMesh* > tMeshVec = g_ptr->CreateTMeshVec();
                for ( int j = 0 ; j < ( int )tMeshVec.size() ; j++ )
                {
                    if ( suppressdisks && ( tMeshVec[j]->m_SurfType == vsp::DISK_SURF ) )
                    {
                        // Skip actuator disk.
                    }
                    else
                    {
                        mesh_geom->m_TMeshVec.push_back( tMeshVec[j] );
                    }
                }
            }

            if ( g_ptr->GetSetFlag( degen_set ) )
            {
                if( g_ptr->GetType().m_Type != BLANK_GEOM_TYPE )
                {
                    vector< DegenGeom > DegenGeomVec; // Vector of geom in degenerate representation

                    g_ptr->CreateDegenGeom( DegenGeomVec, true );

                    vector< TMesh* > tMeshVec;
                    for ( int j = 0; j < DegenGeomVec.size(); j++ )
                    {
                        // Flip normals because surfaces are based on 'bottom' surface and we'd prefer normals face up.
                        DegenGeomVec[j].setFlipNormal( ! DegenGeomVec[j].getFlipNormal() );
                        // Create MeshGeom from DegenGeom
                        // Camber surfaces for wings & props, plates for bodies.
                        DegenGeomVec[j].createTMeshVec( g_ptr, tMeshVec );
                    }

                    // Do not combine these loops.  tMeshVec.size() != DegenGeomVec.size()
                    for ( int j = 0 ; j < ( int )tMeshVec.size() ; j++ )
                    {
                        if ( suppressdisks && ( tMeshVec[j]->m_SurfType == vsp::DISK_SURF ) )
                        {
                            // Skip actuator disk.
                        }
                        else
                        {
                            mesh_geom->m_TMeshVec.push_back( tMeshVec[j] );
                        }
                    }
                }
            }
        }
    }

    mesh_geom->m_SurfDirty = true;

    SetActiveGeom( id );
    return id;
}

//==== Traverse Top Geoms And Get All Geoms - Check Display Flag if True ====//
vector< string > Vehicle::GetGeomVec( bool check_display_flag )
{
    vector< string > geom_vec;
    for ( int i = 0 ; i < ( int )m_TopGeom.size() ; i++ )
    {
        Geom* g_ptr = FindGeom( m_TopGeom[i] );
        if ( g_ptr )
        {
            g_ptr->LoadIDAndChildren( geom_vec, check_display_flag );
        }
    }
    return geom_vec;
}

vector < int > Vehicle::GetDegenGeomTypeVec( int set_index )
{
    vector < string > all_geom_vec = GetGeomSet( set_index );

    vector < int > degen_type_vec( all_geom_vec.size() );

    for ( size_t i = 0; i < all_geom_vec.size(); i++ )
    {
        Geom* geom = FindGeom( all_geom_vec[i] );
        if ( geom )
        {
            // Identify the DegenGeom type
            int surftype = DegenGeom::BODY_TYPE;
            if ( geom->GetMainSurfType(0) == vsp::WING_SURF || geom->GetMainSurfType(0) == vsp::PROP_SURF )
            {
                surftype = DegenGeom::SURFACE_TYPE;
            }
            else if ( geom->GetMainSurfType(0) == vsp::DISK_SURF )
            {
                surftype = DegenGeom::DISK_TYPE;
            }
            else if ( geom->GetType().m_Type == MESH_GEOM_TYPE || geom->GetType().m_Type == HUMAN_GEOM_TYPE )
            {
                surftype = DegenGeom::MESH_TYPE;
            }

            degen_type_vec[i] = surftype;
        }
    }

    return degen_type_vec;
}

//==== Add ID to Active Geom Vec ====//
void Vehicle::AddActiveGeom( const string & id )
{
    if ( FindGeom( id ) )
    {
        m_ActiveGeom.push_back( id );
    }
}

//==== Add ID to Cleard Active Geom Vec ====//
void Vehicle::SetActiveGeom( const string & id )
{
    if ( FindGeom( id ) )
    {
        m_ActiveGeom.clear();
        m_ActiveGeom.push_back( id );
    }
}

//==== Is This Geom Active? ====//
bool Vehicle::IsGeomActive( const string & geom_id )
{
    return vector_contains_val( m_ActiveGeom, geom_id );
}

void Vehicle::DeleteGeomVec( const vector< string > & del_vec )
{
    RemoveGeomVecFromHierarchy( del_vec );

    for ( int c = 0 ; c < ( int )del_vec.size() ; c++ )
    {
        string id = del_vec[c];
        DeleteGeom( id );
    }
}

void Vehicle::CutGeomVec( const vector< string > & cut_vec )
{
    RemoveGeomVecFromHierarchy( cut_vec );

    //=== All Geoms To Be Cut ====//
    for ( int c = 0 ; c < ( int )cut_vec.size() ; c++ )
    {
        string id = cut_vec[c];
        Geom* gPtr = FindGeom( id );
        if ( gPtr )
        {
            m_ClipBoard.push_back( id );
        }
    }

    //==== Make Sure Destructor is Called On Cut Geoms ===//
    vector< string > stored = CopyGeomVec( m_ClipBoard );
    DeleteClipBoard();
    m_ClipBoard = stored;
}

void Vehicle::RemoveGeomVecFromHierarchy( const vector< string > & cut_vec )
{
    //=== Build Ancestor ID vector before changing hierarchy ===//
    vector< string > ancest_vec;
    ancest_vec.resize( cut_vec.size() );

    //=== All Geoms To Be Cut ====//
    for ( int c = 0 ; c < ( int )cut_vec.size() ; c++ )
    {
        string id = cut_vec[c];
        Geom* gPtr = FindGeom( id );
        if ( gPtr )
        {
            //==== Check For Parent ====//
            Geom* ancestPtr = FindGeom( gPtr->GetParentID() );
            string ancestID;

            if ( ancestPtr )
            {
                ancestID = ancestPtr->GetID();
            }
            while ( ancestPtr && vector_contains_val( cut_vec, ancestID ) ) // Loop until ancestor out of cut list.
            {
                ancestID = ancestPtr->GetParentID();
                ancestPtr = FindGeom( ancestID );
            }
            ancest_vec[c] = ancestID;
        }
    }

    //=== All Geoms To Be Cut ====//
    for ( int c = 0 ; c < ( int )cut_vec.size() ; c++ )
    {
        string id = cut_vec[c];
        Geom* gPtr = FindGeom( id );
        if ( gPtr )
        {
            //==== Check For Parent ====//
            Geom* parentPtr = FindGeom( gPtr->GetParentID() );
            if ( parentPtr )
            {
                //=== Check If Parent is In List Of IDs to Cut ====//
                if ( !vector_contains_val( cut_vec, parentPtr->GetID() ) )  // Parent is outside cut list.
                {
                    gPtr->SetParentID( "NONE" );            // Reassign Parent
                    parentPtr->RemoveChildID( id );         // Remove From Parents Child Vec
                }
            }

            //==== Children ====//
            vector< string > child_vec = gPtr->GetChildIDVec();
            for ( int i = 0 ; i < ( int )child_vec.size() ; i++ )
            {
                string child_id = child_vec[i];
                if ( !vector_contains_val( cut_vec, child_id ) )        // Child is outside cut list.
                {
                    Geom* childPtr = FindGeom( child_id );
                    Geom* ancestPtr = FindGeom( ancest_vec[c] );
                    if ( childPtr && ancestPtr )                        // Ancestor and child exist
                    {
                        ancestPtr->AddChildID( child_id );
                        childPtr->SetParentID( ancestPtr->GetID() );
                    }
                    else                                                // No ancestor
                    {
                        if ( childPtr )
                        {
                            childPtr->SetParentID( "NONE" );
                            m_TopGeom.push_back( child_id );
                        }
                    }
                    gPtr->RemoveChildID( child_id );
                }
            }
        }

        //==== Remove From Top Geom Deque ====//
        vector_remove_val( m_TopGeom, id );
    }

    Update();
}

//==== Reorder Active Geom  ====//
void Vehicle::ReorderActiveGeom( int action )
{
    //==== Find Active Geom ====//
    vector< string > active_geom_vec = GetActiveGeomVec();
    if ( active_geom_vec.size() != 1 )
    {
        return;
    }

    string active_geom_id = active_geom_vec[0];
    Geom* active_geom = FindGeom( active_geom_id );
    if ( !active_geom )
    {
        return;
    }

    vector< string > id_vec;
    string parent_id = active_geom->GetParentID();
    Geom* parent_geom = FindGeom( parent_id );
    if ( !parent_geom )
    {
        id_vec = m_TopGeom;
    }
    else
    {
        id_vec = parent_geom->GetChildIDVec();
    }

    vector< string > new_id_vec;
    if ( action == REORDER_MOVE_TOP || action == REORDER_MOVE_BOTTOM )
    {
        if ( action == REORDER_MOVE_TOP )
        {
            new_id_vec.push_back( active_geom_id );
        }

        for ( int i = 0 ; i < ( int )id_vec.size() ; i++ )
            if ( id_vec[i] != active_geom_id )
            {
                new_id_vec.push_back( id_vec[i] );
            }

        if ( action == REORDER_MOVE_BOTTOM )
        {
            new_id_vec.push_back( active_geom_id );
        }
    }
    else if ( action == REORDER_MOVE_UP || action == REORDER_MOVE_DOWN )
    {
        for ( int i = 0 ; i < ( int )id_vec.size() ; i++ )
        {
            if ( i < ( int )( id_vec.size() - 1 ) &&
                    ( ( action == REORDER_MOVE_DOWN && id_vec[i] == active_geom_id ) ||
                      ( action == REORDER_MOVE_UP   && id_vec[i + 1] == active_geom_id ) ) )
            {
                new_id_vec.push_back( id_vec[i + 1] );
                new_id_vec.push_back( id_vec[i] );
                i++;
            }
            else
            {
                new_id_vec.push_back( id_vec[i] );
            }
        }
    }

    if ( !parent_geom )
    {
        m_TopGeom = new_id_vec;
    }
    else
    {
        parent_geom->SetChildIDVec( new_id_vec );
    }

}

//==== Delete Active Geom ====//
void Vehicle::DeleteActiveGeomVec()
{
    vector< string > sel_vec = GetActiveGeomVec();
    if ( sel_vec.size() == 0 )
    {
        return;
    }

    DeleteGeomVec( sel_vec );

    ClearActiveGeom();
}

//==== Cut Active Geom and Place in Clipboard ====//
void Vehicle::CutActiveGeomVec()
{
    vector< string > sel_vec = GetActiveGeomVec();
    if ( sel_vec.size() == 0 )
    {
        return;
    }

    DeleteClipBoard();
    CutGeomVec( sel_vec );

    ClearActiveGeom();
}
//==== Copy Active Geom and Place in Clipboard ====//
void Vehicle::CopyActiveGeomVec()
{
    vector< string > sel_vec = GetActiveGeomVec();
    if ( sel_vec.size() == 0 )
    {
        return;
    }

    DeleteClipBoard();

    m_ClipBoard = CopyGeomVec( sel_vec );
}

//==== Delete All Geoms in Clipboard ====//
void Vehicle::DeleteClipBoard()
{
    for ( int i = 0 ; i < ( int )m_ClipBoard.size() ; i++ )
    {
        Geom* gPtr = FindGeom( m_ClipBoard[i] );
        if ( gPtr )
        {
            vector_remove_val( m_GeomStoreVec, gPtr );
            delete gPtr;
        }
    }
    m_ClipBoard.clear();
}

void Vehicle::DeleteGeom( const string & geom_id )
{
    Geom* gPtr = FindGeom( geom_id );
    if ( gPtr )
    {
        vector_remove_val( m_GeomStoreVec, gPtr );
        vector_remove_val( m_ActiveGeom, geom_id );
        delete gPtr;
    }

}

//==== Paste All Geoms in Clipboard ====//
vector< string > Vehicle::PasteClipboard()
{
    //==== Find Current Parent ====//
    string parent_id = "NONE";
    vector< string > sel_vec = GetActiveGeomVec();
    if ( sel_vec.size() )
    {
        parent_id = sel_vec.back();
    }

    //==== Copy Clipboard ====//
    vector< string > copy_geom = CopyGeomVec( m_ClipBoard );

    //==== Find First Unattached Geom In ClipBoard ====//
    Geom* parentGeom = FindGeom( parent_id );
    for ( int i = 0 ; i < ( int )m_ClipBoard.size() ; i++ )
    {
        Geom* gPtr = FindGeom( m_ClipBoard[i] );
        if ( gPtr && gPtr->GetParentID() == "NONE" )
        {
            if ( parentGeom )
            {
                gPtr->SetParentID( parent_id );
                parentGeom->AddChildID( gPtr->GetID() );

                //==== Update gPtr and all children  ====//
                if ( parentGeom->GetType().m_Type != HINGE_GEOM_TYPE )
                {
                    gPtr->SetIgnoreAbsFlag( true );
                }
            }

            gPtr->Update();

            if ( parentGeom )
            {
                if ( parentGeom->GetType().m_Type != HINGE_GEOM_TYPE )
                {
                    gPtr->SetIgnoreAbsFlag( false );
                }
            }
        }
    }

    //==== Add To Active Geoms ====//
    for ( int i = 0 ; i < ( int )m_ClipBoard.size() ; i++ )
    {
        Geom* gPtr = FindGeom( m_ClipBoard[i] );
        if ( gPtr && gPtr->GetParentID() == "NONE" )
        {
            m_TopGeom.push_back(  m_ClipBoard[i] );
        }
    }

    //==== Store ids of pasted geoms ====//
    vector<string> pasted_ids = m_ClipBoard;

    //==== Move Copied Geoms into ClipBoard ====//
    m_ClipBoard.clear();
    for  ( int i = 0 ; i < ( int )copy_geom.size() ; i++ )
    {
        m_ClipBoard.push_back( copy_geom[i] );
    }

    return pasted_ids;
}

//==== Copy Geoms In Vec - Create New IDs But Keep Parent/Child ====//
vector< string > Vehicle::CopyGeomVec( const vector< string > & geom_vec )
{
    string lastreset = ParmMgr.ResetRemapID();

    //==== Create New Geoms ====//
    vector< string > created_id_vec;
    vector< Geom* > created_geom_vec;

    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        Geom* fromPtr = FindGeom( geom_vec[i] );
        if ( fromPtr )
        {
            GeomType t = fromPtr->GetType();
            string id = CreateGeom( t );
            Geom* toPtr = FindGeom( id );
            if ( toPtr )
            {
                toPtr->CopyFrom( fromPtr );
                if (m_CopySetsWithGeomsFlag.Get() == false)
                {
                    // New geom is only in SET_SHOWN
                    for ( int i = SET_FIRST_USER; i < NUM_SETS + 2; i++ )
                    {
                        toPtr->SetSetFlag( i, false );
                    }

                    toPtr->SetSetFlag( SET_SHOWN, true );
                }

                id = toPtr->GetID();
                created_id_vec.push_back( id );
            }
        }
    }

    ParmMgr.ResetRemapID( lastreset );

    // Scan through and look for parents & children outside copied vector
    // These have nonexistant ID's because the remap created a new unique
    // ID, but no geom was ever created and changed to that ID.
    for ( int i = 0 ; i < ( int )created_id_vec.size() ; i++ )
    {
        Geom* geom = FindGeom( created_id_vec[i] );
        if ( geom )
        {
            Geom* par = FindGeom( geom->GetParentID() );

            if ( par == NULL )
            {
                geom->SetParentID( "NONE" );
            }

            vector< string > childvec = geom->GetChildIDVec();

            for ( int j = 0; j < static_cast<int>( childvec.size() ); j++ )
            {
                Geom* child = FindGeom( childvec[j] );

                if ( child == NULL )
                {
                    geom->RemoveChildID( childvec[j] );
                }
            }
        }
    }

    return created_id_vec;
}

//==== Get Draw Objects ====//
vector< DrawObj* > Vehicle::GetDrawObjs()
{
    vector< DrawObj* > draw_obj_vec;

    //==== Traverse All Active Displayed Geom and Load DrawObjs ====//
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        geom_vec[i]->LoadDrawObjs( draw_obj_vec );
    }

    return draw_obj_vec;
}

void Vehicle::ResetDrawObjsGeomChangedFlags()
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        geom_vec[i]->ResetGeomChangedFlag();
    }
}

void Vehicle::SetSetName( int index, const string& name )
{
    char str[256];

    if ( index < 0 || index > 511 )
    {
        return;
    }

    while ( ( int )m_SetNameVec.size() <= index )
    {
        sprintf( str, "Set_%d", ( int )m_SetNameVec.size() );
        m_SetNameVec.push_back( string( str ) );
    }
    m_SetNameVec[index] = name;
}

vector< string > Vehicle::GetSetNameVec( bool includeNone )
{
    if ( includeNone )
    {
        vector < string > ret;
        ret.reserve( m_SetNameVec.size() + 1 );
        ret.push_back( "None" );
        ret.insert( ret.end(), m_SetNameVec.begin(), m_SetNameVec.end() );
        return ret;
    }
    return m_SetNameVec;
}

//=== Set 'Show' set to specified index set ===//
void Vehicle::ShowOnlySet( int index )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        bool f = geom_vec[i]->GetSetFlag( index );

        geom_vec[i]->SetSetFlag( SET_SHOWN, f );
        geom_vec[i]->SetSetFlag( SET_NOT_SHOWN, !f );
    }
}

void Vehicle::NoShowSet( int index )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        bool f = geom_vec[i]->GetSetFlag( index );

        if ( f )
        {
            geom_vec[i]->SetSetFlag( SET_SHOWN, false );
            geom_vec[i]->SetSetFlag( SET_NOT_SHOWN, true );
        }
    }
}

void Vehicle::ShowSet( int index )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        bool f = geom_vec[i]->GetSetFlag( index );

        if ( f )
        {
            geom_vec[i]->SetSetFlag( SET_SHOWN, true );
            geom_vec[i]->SetSetFlag( SET_NOT_SHOWN, false );
        }
    }
}

//=== Get Vector of Geom IDs That Are In Set Index ===//
vector< string > Vehicle::GetGeomSet( int index )
{
    vector< string > geom_id_vec;

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( index ) )
        {
            geom_id_vec.push_back( geom_vec[i]->GetID() );
        }
    }
    return geom_id_vec;
}

void Vehicle::HideAllExcept( const string& id )
{
    vector< string > geom_id_vec;

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        Geom* geom_ptr = geom_vec[i];

        if ( geom_ptr )
        {
            if ( geom_ptr->GetID() != id )
            {
                // No Show All Other Components
                geom_ptr->SetSetFlag( SET_SHOWN, false ); //remove from shown
                geom_ptr->SetSetFlag( SET_NOT_SHOWN, true ); //add to no show
            }
        }
    }
}

void Vehicle::HideAll()
{
    vector< string > geom_id_vec;

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        Geom* geom_ptr = geom_vec[i];

        if ( geom_ptr )
        {
            // No Show All Other Components
            geom_ptr->SetSetFlag( SET_SHOWN, false ); //remove from shown
            geom_ptr->SetSetFlag( SET_NOT_SHOWN, true ); //add to no show
        }
    }
}

void Vehicle::CopyPasteSet(int copyIndex, int pasteIndex)
{
    vector< string > geom_id_vec = this->GetGeomVec();
    for (int i = 0; i < (int)geom_id_vec.size(); i++)
    {
        Geom* gptr = this->FindGeom(geom_id_vec[i]);

        if (gptr->GetSetFlag(copyIndex))
        {
            gptr->SetSetFlag(pasteIndex, true);
        }
        else
        {
            gptr->SetSetFlag(pasteIndex, false);
        }
    }
}

//==== Get Number of Fixed Geometry Types ====//
int Vehicle::GetNumFixedGeomTypes()
{
    int num = 0;
    for ( int i = 0 ; i < ( int )m_GeomTypeVec.size() ; i++ )
    {
        if ( m_GeomTypeVec[i].m_FixedFlag )
        {
            num++;
        }
    }
    return num;
}

//==== Get Geom Type ====//
GeomType Vehicle::GetGeomType( int index )
{
    if ( index >= 0 &&  index < (int)m_GeomTypeVec.size() )
        return m_GeomTypeVec[index];

    return GeomType( 0, "" );
}

//==== Set Geom Type ====//
void Vehicle::SetGeomType( int index, const GeomType & type )
{
    if ( index >= 0 && index < (int)m_GeomTypeVec.size() )
    {
        m_GeomTypeVec[index] = type;
    }
}

//==== Add Type From Geometry ====//
void Vehicle::AddType( const string & geom_id )
{
    Geom* gptr = FindGeom( geom_id );
    if ( gptr && gptr->GetType().m_Type != CUSTOM_GEOM_TYPE )
    {
        GeomType type( gptr->GetType().m_Type, gptr->GetName(), false, gptr->GetType().m_ModuleName, gptr->GetType().m_DisplayName );

        //===== Create Geom ====//
        GeomType t = gptr->GetType();
        string id = CreateGeom( t );
        Geom* toPtr = FindGeom( id );
        if ( toPtr )
        {
            toPtr->CopyFrom( gptr );
            toPtr->Update();
        }

        type.m_GeomID = id;
        m_GeomTypeVec.push_back( type );
    }

}

//==== Get Vector of Geom IDs That Are Valid For Types ====//
vector< string > Vehicle::GetValidTypeGeoms()
{
    vector< string > geom_id_vec;
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetType().m_Type != CUSTOM_GEOM_TYPE )
        {
           geom_id_vec.push_back( geom_vec[i]->GetID() );
        }
    }
    return geom_id_vec;
}


//==== Get All Geometry Types That Are Editable ====//
vector< GeomType > Vehicle::GetEditableGeomTypes()
{
    vector< GeomType > type_vec;
    for ( int i = 0 ; i < (int)m_GeomTypeVec.size() ; i++ )
    {
        if ( !m_GeomTypeVec[i].m_FixedFlag && m_GeomTypeVec[i].m_Type != CUSTOM_GEOM_TYPE )
        {
            type_vec.push_back( m_GeomTypeVec[i] );
        }
    }
    return type_vec;
}


//==== Delete Type ====//
void Vehicle::DeleteType( int index )
{
     if ( index < 0 || index >= (int)m_GeomTypeVec.size() )
        return;

    GeomType type = GetGeomType( index );

    if ( type.m_FixedFlag )
    {
        return;
    }

    Geom* gPtr = FindGeom( type.m_GeomID );
    if ( gPtr )
    {
        vector_remove_val( m_GeomStoreVec, gPtr );
        delete gPtr;
    }

    m_GeomTypeVec.erase( m_GeomTypeVec.begin() + index );
}


xmlNodePtr Vehicle::EncodeXml( xmlNodePtr & node, int set )
{
    xmlNodePtr vehicle_node = xmlNewChild( node, NULL, BAD_CAST"Vehicle", NULL );

    ParmContainer::EncodeXml( vehicle_node );

    // Encode lighting information.
    LightMgr.EncodeXml( vehicle_node );

    // Encode label information.
    MeasureMgr.EncodeXml( vehicle_node );

    MaterialMgr.EncodeXml( node );

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( set ) )
        {
            geom_vec[i]->EncodeGeom( vehicle_node );
        }
    }

    LinkMgr.EncodeXml( node );
    AdvLinkMgr.EncodeXml( node );
    VSPAEROMgr.EncodeXml( node );
    VarPresetMgr.EncodeXml( node );
    m_CfdSettings.EncodeXml( node );
    m_ISectSettings.EncodeXml( node );
    m_CfdGridDensity.EncodeXml( node );
    StructureMgr.EncodeXml( node );
    m_ClippingMgr.EncodeXml( node );
    WaveDragMgr.EncodeXml( node );
    ParasiteDragMgr.EncodeXml( node );

    xmlNodePtr setnamenode = xmlNewChild( node, NULL, BAD_CAST"SetNames", NULL );
    if ( setnamenode )
    {
        for ( int i = 0; i < m_SetNameVec.size(); i++ )
        {
            XmlUtil::AddStringNode( setnamenode, "Set", m_SetNameVec[i] );
        }
    }

    return vehicle_node;
}

xmlNodePtr Vehicle::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr vehicle_node = XmlUtil::GetNode( node, "Vehicle", 0 );
    if ( vehicle_node )
    {
        ParmContainer::DecodeXml( vehicle_node );

        // Decode lighting information.
        LightMgr.DecodeXml( vehicle_node );

        // Decode label information.
        MeasureMgr.DecodeXml( vehicle_node );

    }

    // 'GeomsOnly' is a euphamism for those entities we want to read when 'inserting' a file.
    // It is mostly the Geoms, but also materials, presets, links, and advanced links.
    DecodeXmlGeomsOnly( node );

    VSPAEROMgr.DecodeXml( node );
    m_CfdSettings.DecodeXml( node );
    m_ISectSettings.DecodeXml( node );
    m_CfdGridDensity.DecodeXml( node );
    StructureMgr.DecodeXml( node );
    m_ClippingMgr.DecodeXml( node );
    WaveDragMgr.DecodeXml( node );
    ParasiteDragMgr.DecodeXml( node );

    ParasiteDragMgr.CorrectTurbEquation();

    xmlNodePtr setnamenode = XmlUtil::GetNode( node, "SetNames", 0 );
    if ( setnamenode )
    {
        int num = XmlUtil::GetNumNames( setnamenode, "Set" );

        for ( int i = 0; i < num; i++ )
        {
            xmlNodePtr namenode = XmlUtil::GetNode( setnamenode, "Set", i );
            if ( namenode )
            {
                string name = XmlUtil::ExtractString( namenode );
                SetSetName( i, name );
            }
        }
    }

    return vehicle_node;
}

// DecodeXmlGeomsOnly is a stripped down version of DecodeXml.
//
// It is called directly when we 'insert' instead of 'open' a file.  It skips a lot of the auxiliary information
// contained in the vsp3 file -- instead deferring to that already in the main file.  It attempts to insert
// all the geometry as well as links & advanced links from the file.
//
// To prevent code duplication, it is also called from DecodeXml
//
xmlNodePtr Vehicle::DecodeXmlGeomsOnly( xmlNodePtr & node )
{
    MaterialMgr.DecodeXml( node );

    xmlNodePtr vehicle_node = XmlUtil::GetNode( node, "Vehicle", 0 );
    if ( vehicle_node )
    {

        int num = XmlUtil::GetNumNames( vehicle_node, "Geom" );
        for ( int i = 0 ; i < num ; i++ )
        {
            xmlNodePtr geom_node = XmlUtil::GetNode( vehicle_node, "Geom", i );
            if ( geom_node )
            {
                xmlNodePtr base_node = XmlUtil::GetNode( geom_node, "GeomBase", 0 );

                GeomType type;
                type.m_Name   = XmlUtil::FindString( base_node, "TypeName", type.m_Name );
                type.m_Type   = XmlUtil::FindInt( base_node, "TypeID", type.m_Type );
                type.m_FixedFlag = !!XmlUtil::FindInt( base_node, "TypeFixed", type.m_FixedFlag );

                string id = CreateGeom( type );
                Geom* geom = FindGeom( id );

                if ( geom )
                {
                    geom->DecodeXml( geom_node );

                    if ( geom->GetParentID().compare( "NONE" ) == 0 )
                    {
                        AddGeom( geom );
                    }
                }
            }
        }
    }

    LinkMgr.DecodeXml( node );
    AdvLinkMgr.DecodeXml( node );
    VarPresetMgr.DecodeXml( node );

    return vehicle_node;
}

//==== Write File ====//
bool Vehicle::WriteXMLFile( const string & file_name, int set )
{
    xmlDocPtr doc = xmlNewDoc( ( const xmlChar * )"1.0" );

    xmlNodePtr root = xmlNewNode( NULL, ( const xmlChar * )"Vsp_Geometry" );
    xmlDocSetRootElement( doc, root );
    XmlUtil::AddIntNode( root, "Version", CURRENT_FILE_VER );

    EncodeXml( root, set );

    //===== Save XML Tree and Free Doc =====//
    int err = xmlSaveFormatFile( file_name.c_str(), doc, 1 );
    xmlFreeDoc( doc );

    if( err == -1 )  // Failure occurred
    {
        return false;
    }

    return true;
}

//==== Read File ====//
int Vehicle::ReadXMLFile( const string & file_name )
{
    string lastreset = ParmMgr.ResetRemapID();

    // Disable link updates when until all geoms are loaded
    LinkMgr.SetFreezeUpdateFlag( true );

    //==== Read Xml File ====//
    xmlDocPtr doc;

    LIBXML_TEST_VERSION
    xmlKeepBlanksDefault( 0 );

    //==== Build an XML tree from a the file ====//
    doc = xmlParseFile( file_name.c_str() );
    if ( doc == NULL )
    {
        fprintf( stderr, "could not parse XML document\n" );
        return 1;
    }

    xmlNodePtr root = xmlDocGetRootElement( doc );
    if ( root == NULL )
    {
        fprintf( stderr, "empty document\n" );
        xmlFreeDoc( doc );
        return 2;
    }

    if ( xmlStrcmp( root->name, ( const xmlChar * )"Vsp_Geometry" ) )
    {
        fprintf( stderr, "document of the wrong type, Vsp Geometry not found\n" );
        xmlFreeDoc( doc );
        return 3;
    }

    //==== Find Version Number ====//
    m_FileOpenVersion = XmlUtil::FindInt( root, "Version", 0 );

    if ( m_FileOpenVersion < MIN_FILE_VER )
    {
        fprintf( stderr, "document version not supported \n");
        xmlFreeDoc( doc );
        m_FileOpenVersion = -1;
        return 4;
    }

    //==== Decode Vehicle from document ====//
    DecodeXml( root );

    //===== Free Doc =====//
    xmlFreeDoc( doc );

    ParmMgr.ResetRemapID( lastreset );

    Update();
    AdvLinkMgr.ForceUpdate();

    LinkMgr.SetFreezeUpdateFlag( false );

    m_FileOpenVersion = -1;
    return 0;
}

//==== Read File ====//
int Vehicle::ReadXMLFileGeomsOnly( const string & file_name )
{
    string lastreset = ParmMgr.ResetRemapID();

    //==== Read Xml File ====//
    xmlDocPtr doc;

    LIBXML_TEST_VERSION
    xmlKeepBlanksDefault( 0 );

    //==== Build an XML tree from a the file ====//
    doc = xmlParseFile( file_name.c_str() );
    if ( doc == NULL )
    {
        fprintf( stderr, "could not parse XML document\n" );
        return 1;
    }

    xmlNodePtr root = xmlDocGetRootElement( doc );
    if ( root == NULL )
    {
        fprintf( stderr, "empty document\n" );
        xmlFreeDoc( doc );
        return 2;
    }

    if ( xmlStrcmp( root->name, ( const xmlChar * )"Vsp_Geometry" ) )
    {
        fprintf( stderr, "document of the wrong type, Vsp Geometry not found\n" );
        xmlFreeDoc( doc );
        return 3;
    }

    //==== Find Version Number ====//
    m_FileOpenVersion = XmlUtil::FindInt( root, "Version", 0 );

    if ( m_FileOpenVersion < MIN_FILE_VER )
    {
        fprintf( stderr, "document version not supported \n");
        xmlFreeDoc( doc );
        m_FileOpenVersion = -1;
        return 4;
    }

    //==== Decode Vehicle from document ====//
    DecodeXmlGeomsOnly( root );

    //===== Free Doc =====//
    xmlFreeDoc( doc );

    ParmMgr.ResetRemapID( lastreset );

    Update();

    m_FileOpenVersion = -1;
    return 0;
}

//==== Write Cross Section File ====//
void Vehicle::WriteXSecFile( const string & file_name, int write_set )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );

    int geom_cnt = 0;
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if( geom_vec[i]->GetSetFlag( write_set ) )
        {
            geom_cnt += geom_vec[i]->GetNumTotalHrmSurfs();
        }
    }

    //==== Open file ====//
    FILE* dump_file = fopen( file_name.c_str(), "w" );

    fprintf( dump_file, " HERMITE INPUT FILE\n\n" );
    fprintf( dump_file, " NUMBER OF COMPONENTS = %d\n", geom_cnt );

    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) )
        {
            geom_vec[i]->WriteXSecFile( i, dump_file );
        }
    }

    fclose( dump_file );
}

//==== Write Formatted PLOT3D File ====//
void Vehicle::WritePLOT3DFile( const string & file_name, int write_set )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );

    int geom_cnt = 0;
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if( geom_vec[i]->GetSetFlag( write_set ) )
        {
            geom_cnt += geom_vec[i]->GetNumTotalSurfs();
        }
    }

    //==== Open file ====//
    FILE* dump_file = fopen( file_name.c_str(), "w" );

    //==== Write total number of surfaces ===//
    fprintf( dump_file, " %d\n", geom_cnt );

    //==== Write surface boundary extents ====//
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) )
        {
            geom_vec[i]->WritePLOT3DFileExtents( dump_file );
        }
    }

    //==== Write surface boundary points ====//
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) )
        {
            geom_vec[i]->WritePLOT3DFileXYZ( dump_file );
        }
    }

    fclose( dump_file );
}

//==== Check for an existing mesh in set ====//
bool Vehicle::ExistMesh( int set )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    if ( !geom_vec[0] )
    {
        return false;
    }

    // Check to see if a MeshGeom Exists
    bool existMesh = false;
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            existMesh = true;
        }
    }

    return existMesh;
}

vector < string > Vehicle::GetPtCloudGeoms()
{
    vector < string > ptclouds;

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    if ( !geom_vec[0] )
    {
        return ptclouds;
    }

    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetType().m_Type == PT_CLOUD_GEOM_TYPE )
        {
            ptclouds.push_back( geom_vec[i]->GetID() );
        }
    }

    return ptclouds;
}

//==== Write STL File ====//
string Vehicle::WriteSTLFile( const string & file_name, int write_set )
{
    string mesh_id = string();

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    if ( !geom_vec[0] )
    {
        return mesh_id;
    }

    if ( !ExistMesh( write_set ) )
    {
        mesh_id = AddMeshGeom( write_set );
        if ( mesh_id.compare( "NONE" ) != 0 )
        {
            Geom* gPtr = FindGeom( mesh_id );
            if ( gPtr )
            {
                geom_vec.push_back( gPtr );
                gPtr->Update();
            }
            HideAllExcept( mesh_id );
        }
    }

    // Open File
    FILE* fid = fopen( file_name.c_str(), "w" );
    fprintf( fid, "solid\n" );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            mesh_id = geom_vec[i]->GetID(); // Set ID in case mesh already existed

            geom_vec[i]->WriteStl( fid );
        }
    }

    fprintf( fid, "endsolid\n" );
    fclose( fid );

    return mesh_id;
}

//==== Write STL File ====//
string Vehicle::WriteTaggedMSSTLFile( const string & file_name, int write_set )
{
    string mesh_id = string();

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    if ( !geom_vec[0] )
    {
        return mesh_id;
    }

    if ( !ExistMesh( write_set ) )
    {
        mesh_id = AddMeshGeom( write_set );
        if ( mesh_id.compare( "NONE" ) != 0 )
        {
            Geom* gPtr = FindGeom( mesh_id );
            if ( gPtr )
            {
                MeshGeom* mg = dynamic_cast<MeshGeom*>( gPtr );
                mg->SubTagTris( true );
                geom_vec.push_back( gPtr );
                gPtr->Update();
            }
            HideAllExcept( mesh_id );
        }
    }

    //==== Count Number of Points & Tris ====//
    int num_pnts = 0;
    int num_tris = 0;
    int num_parts = 0;
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            mg->BuildIndexedMesh( num_parts );
            num_parts += mg->GetNumIndexedParts();
            num_pnts += mg->GetNumIndexedPnts();
            num_tris += mg->GetNumIndexedTris();
        }
    }

    FILE* file_id = fopen( file_name.c_str(), "w" );
    if ( file_id )
    {
        std::vector< int > tags = SubSurfaceMgr.GetAllTags();
        for ( int i = 0; i < ( int ) tags.size(); i++ )
        {
            std::string tagname = SubSurfaceMgr.GetTagNames( i );
            fprintf( file_id, "solid %d_%s\n", tags[i], tagname.c_str() );

            for ( int j = 0 ; j < ( int )geom_vec.size() ; j++ )
            {
                if ( geom_vec[j]->GetSetFlag( write_set ) && geom_vec[j]->GetType().m_Type == MESH_GEOM_TYPE )
                {
                    MeshGeom* mg = ( MeshGeom* )geom_vec[j];            // Cast
                    mesh_id = geom_vec[j]->GetID(); // Set ID in case mesh already existed

                    mg->WriteStl( file_id, tags[i] );
                }
            }
            fprintf( file_id, "endsolid %d_%s\n", tags[i], tagname.c_str() );
        }

        fclose( file_id );
    }

    return mesh_id;
}

//==== Write Facet File ====//
string Vehicle::WriteFacetFile( const string & file_name, int write_set )
{
    string mesh_id = string();

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    if ( !geom_vec[0] )
    {
        return mesh_id;
    }

    // Note: If there is already a mesh geometry for the write_set, a new one is not created.
    if ( !ExistMesh( write_set ) )
    {
        mesh_id = AddMeshGeom( write_set );
        if ( mesh_id.compare( "NONE" ) != 0 )
        {
            Geom* gPtr = FindGeom( mesh_id );
            if ( gPtr )
            {
                MeshGeom* mg = dynamic_cast<MeshGeom*>( gPtr );
                mg->SubTagTris( true );
                geom_vec.push_back( gPtr );
                gPtr->Update();
            }
            HideAllExcept( mesh_id );
        }
    }

    // Open File
    FILE* fid = fopen( file_name.c_str(), "w" );

    if ( fid )
    {
        fprintf( fid, "Exported from %s\n", VSPVERSION4 ); // Title/comment line
        fprintf( fid, "1\n" ); // Number of "Big" parts (1 Vehicle broken into small parts by geom and subsurface)

        fprintf( fid, "%s\n", m_Name.c_str() ); // Name of "Big" part: Vehicle name

        // mirror -> i, a b c d
        //     if i = 0 -> no mirror
        //     if i = 1 -> "Big" part is mirrored across plane defined by ax+by+cz-d=0
        fprintf( fid, "0, 0.000 1.000 0.000 0.000\n" );

        //==== Count Number of Points, Tris, and Parts ====//
        int num_pnts = 0;
        int num_parts = 0;

        for ( int i = 0; i < (int)geom_vec.size(); i++ )
        {
            if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
            {
                MeshGeom* mg = (MeshGeom*)geom_vec[i];            // Cast
                mg->BuildIndexedMesh( num_parts );
                num_parts += mg->GetNumIndexedParts();
                num_pnts += mg->GetNumIndexedPnts();
            }
        }

        fprintf( fid, "%d \n", num_pnts ); // # of nodes in "Big" part

        // List all points (nodes) in "Big" part
        for ( int i = 0; i < (int)geom_vec.size(); i++ )
        {
            if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
            {
                MeshGeom* mg = (MeshGeom*)geom_vec[i];
                mesh_id = geom_vec[i]->GetID(); // Set ID in case mesh already existed

                mg->WriteFacetNodes( fid );
            }
        }

        // Define each "Small" part by corresponding nodes for each facet
        int offset = 0;
        int tri_count = 0;
        int part_count = 0;

        // Note: offset, tri_count, and part_count have been implemented in the case of the existence
        //      of multiple meshes. However, tagging is only supported for a single mesh at this time.
        //      A facet export of more than one mesh will lead to tagging and naming errors.

        for ( int i = 0; i < (int)geom_vec.size(); i++ )
        {
            if ( geom_vec[i]->GetSetFlag( write_set ) &&
                 geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
            {
                MeshGeom* mg = (MeshGeom*)geom_vec[i];            // Cast
                mg->WriteFacetTriParts( fid, offset, tri_count, part_count );
            }
        }

        // Note: The mesh geom created during the export is not deleted.
        //      If the mesh geometry is not manually deleted, it will be
        //      used to generate the export regardless of changes to the
        //      vehicle.

        fclose( fid );
    }

    return mesh_id;
}

//==== Write Tri File ====//
string Vehicle::WriteTRIFile( const string & file_name, int write_set )
{
    string mesh_id = string();

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    if ( geom_vec.size()==0 )
    {
        printf("WARNING: No geometry to write \n\tFile: %s \tLine:%d\n",__FILE__,__LINE__);
        return mesh_id;
    }

    // Add a new mesh if one does not exist
    if ( !ExistMesh( write_set ) )
    {
        mesh_id = AddMeshGeom( write_set );
        if ( mesh_id.compare( "NONE" ) != 0 )
        {
            Geom* geom_ptr = FindGeom( mesh_id );
            if ( geom_ptr )
            {
                MeshGeom* mg = dynamic_cast<MeshGeom*>( geom_ptr );
                mg->SubTagTris( true );
                geom_vec.push_back( geom_ptr );
                geom_ptr->Update();
            }
            HideAllExcept( mesh_id );
        }
    }

    //==== Open file ====//
    FILE* file_id = fopen( file_name.c_str(), "w" );

    if ( !file_id )
    {
        return mesh_id;
    }

    //==== Count Number of Points & Tris ====//
    int num_pnts = 0;
    int num_tris = 0;
    int num_parts = 0;
    int i;

    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            mg->BuildIndexedMesh( num_parts );
            num_parts += mg->GetNumIndexedParts();
            num_pnts += mg->GetNumIndexedPnts();
            num_tris += mg->GetNumIndexedTris();
        }
    }

    fprintf( file_id, "%d %d\n", num_pnts, num_tris );

    //==== Dump Points ====//
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) &&
                geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE  )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            mesh_id = geom_vec[i]->GetID(); // Set ID in case mesh already existed

            mg->WriteCart3DPnts( file_id );
        }
    }

    int offset = 0;
    //==== Dump Tris ====//
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) &&
                geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE  )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            offset = mg->WriteCart3DTris( file_id, offset );
        }
    }

    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) &&
                geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            mg->WriteCart3DParts( file_id  );
        }
    }

    fclose( file_id );

    //==== Write Out tag key file ====//

    SubSurfaceMgr.WriteTKeyFile(file_name);

    return mesh_id;
}

//==== Write OBJ File ====//
string Vehicle::WriteOBJFile( const string & file_name, int write_set )
{
    string mesh_id = string();

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    if ( geom_vec.size()==0 )
    {
        printf("WARNING: No geometry to write \n\tFile: %s \tLine:%d\n",__FILE__,__LINE__);
        return mesh_id;
    }

    // Add a new mesh if one does not exist
    if ( !ExistMesh( write_set ) )
    {
        mesh_id = AddMeshGeom( write_set );
        if ( mesh_id.compare( "NONE" ) != 0 )
        {
            Geom* geom_ptr = FindGeom( mesh_id );
            if ( geom_ptr )
            {
                MeshGeom* mg = dynamic_cast<MeshGeom*>( geom_ptr );
                mg->SubTagTris( true );
                geom_vec.push_back( geom_ptr );
                geom_ptr->Update();
            }
            HideAllExcept( mesh_id );
        }
    }

    //==== Open file ====//
    FILE* file_id = fopen( file_name.c_str(), "w" );

    if ( !file_id )
    {
        return mesh_id;
    }

    //==== Count Number of Points & Tris ====//
    int num_pnts = 0;
    int num_tris = 0;
    int num_parts = 0;
    int i;

    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            mg->BuildIndexedMesh( num_parts );
            num_parts += mg->GetNumIndexedParts();
            num_pnts += mg->GetNumIndexedPnts();
            num_tris += mg->GetNumIndexedTris();
        }
    }

    //==== Dump Points ====//
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) &&
             geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE  )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            mesh_id = geom_vec[i]->GetID(); // Set ID in case mesh already existed

            mg->WriteOBJPnts( file_id );
        }
    }

    int offset = 0;
    //==== Dump Tris ====//
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) &&
             geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE  )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast

            fprintf( file_id, "g %s\n", geom_vec[i]->GetName().c_str() );

            offset = mg->WriteOBJTris( file_id, offset );
        }
    }

    fclose( file_id );

    return mesh_id;
}

/*
nnode                        // Number of nodes
x1 y1 z1                     // Nodal position.  Also any future single-valued node-centered data that we desire to add.
x2 y2 z2
...
xnnode ynnode znnode         // Last node
nface                        // Number of faces (currently only tris, but support polygons)
n1 i11 i12 i13...i1n         // Number of points, index 1, 2, ... n for each face, right hand rule ordering for normals facing out.
n2 i21 i22 i23...i2n
...
nnface in1 in2... inn        // Last polygon face
t1 u11 v11 u12 v12...u1n v1n // Tag number for face 1 followed by multi-valued nodal data -- currently parametric UV coordinate.
t2 u21 v21 u22 v22...u2n v2n // Also any other face-centered data that we desire to add.
...
tnface un1 vn2...unn vnn     // Last tag, multi-valued nodal and face-centered data
nwake                        // Number of wake lines
n1 i11 i12 i13 i14...i1n     // Number of points in wake line, indices in chain-order.  Typically line wrapped at ten indices per line.
n2 i21 i22 i13 i24...i2n
...
nnwake in1 in2 in3 in4...inn // Last wake line
*/

//==== Write VSPGeom File ====//
string Vehicle::WriteVSPGeomFile( const string &file_name, int write_set, int degen_set, bool half_flag, bool hideset, bool suppressdisks )
{
    string mesh_id = string();

    vector< Geom * > geom_vec = FindGeomVec( GetGeomVec( false ) );
    if ( geom_vec.size() == 0 || !geom_vec[0] )
    {
        return mesh_id;
    }

    // Add a new mesh if one does not exist in either set
    if ( ( write_set >= 0 && !ExistMesh( write_set ) ) ||
         ( degen_set >= 0 && !ExistMesh( degen_set ) ) )
    {
        mesh_id = AddMeshGeom( write_set, degen_set, suppressdisks );
        if ( mesh_id.compare( "NONE" ) != 0 )
        {
            Geom *geom_ptr = FindGeom( mesh_id );
            if ( geom_ptr )
            {
                MeshGeom *mg = dynamic_cast<MeshGeom *>( geom_ptr );
                mg->SubTagTris( true );
                geom_vec.push_back( geom_ptr );
                geom_ptr->Update();
            }

            if ( hideset)
            {
                HideAllExcept( mesh_id );
            }

            // Below, we are going to write out _all_ MeshGeoms in write_set.
            // AddMeshGeom above will create a MeshGeom in write_set if write_set >= SET_FIRST_USER.
            // Otherwise, the new MeshGeom will be in SET_SHOWN.
            // HideAllExcept should ensure that the new MeshGeom is the only thing in SET_SHOWN.
            // If (for example) write_set is SET_NONE and degen_set contains the geometry, then
            // this will cause trouble.
            // By changing write_set to SET_SHOWN for this code path, we make sure anything created
            // through AddMeshGeom will get written.
            write_set = vsp::SET_SHOWN;
        }
    }

    //==== Open file ====//
    FILE *file_id = fopen( file_name.c_str(), "w" );

    if ( !file_id )
    {
        return mesh_id;
    }

    //==== Count Number of Points & Tris ====//
    int num_pnts = 0;
    int num_tris = 0;
    int num_parts = 0;
    int i;

    for ( i = 0; i < ( int ) geom_vec.size(); i++ )
    {
        if ( ( geom_vec[i]->GetSetFlag( write_set ) || geom_vec[i]->GetSetFlag( degen_set ) )
            && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom *mg = ( MeshGeom * ) geom_vec[i];            // Cast
            mg->BuildIndexedMesh( num_parts, half_flag );
            num_parts += mg->GetNumIndexedParts();
            num_pnts += mg->GetNumIndexedPnts();
            num_tris += mg->GetNumIndexedTris();
        }
    }

    fprintf( file_id, "%d\n", num_pnts );

    //==== Dump Points ====//
    for ( i = 0; i < ( int ) geom_vec.size(); i++ )
    {
        if ( ( geom_vec[i]->GetSetFlag( write_set ) || geom_vec[i]->GetSetFlag( degen_set ) ) &&
             geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom *mg = ( MeshGeom * ) geom_vec[i];            // Cast
            mesh_id = geom_vec[i]->GetID(); // Set ID in case mesh already existed

            mg->WriteVSPGeomPnts( file_id );
        }
    }

    fprintf( file_id, "%d\n", num_tris );

    int offset = 0;
    //==== Dump Tris ====//
    for ( i = 0; i < ( int ) geom_vec.size(); i++ )
    {
        if ( ( geom_vec[i]->GetSetFlag( write_set ) || geom_vec[i]->GetSetFlag( degen_set ) ) &&
             geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom *mg = ( MeshGeom * ) geom_vec[i];            // Cast
            offset = mg->WriteVSPGeomTris( file_id, offset );
        }
    }

    for ( i = 0; i < ( int ) geom_vec.size(); i++ )
    {
        if ( ( geom_vec[i]->GetSetFlag( write_set ) || geom_vec[i]->GetSetFlag( degen_set ) ) &&
             geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom *mg = ( MeshGeom * ) geom_vec[i];            // Cast
            mg->WriteVSPGeomParts( file_id );
        }
    }

    offset = 0;
    // Wake line data.
    for ( i = 0; i < ( int ) geom_vec.size(); i++ )
    {
        if ( ( geom_vec[i]->GetSetFlag( write_set ) || geom_vec[i]->GetSetFlag( degen_set ) ) &&
             geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom *mg = ( MeshGeom * ) geom_vec[i];            // Cast
            offset = mg->WriteVSPGeomWakes( file_id, offset );
        }
    }

    fclose( file_id );

    //==== Write Out tag key file ====//

    SubSurfaceMgr.WriteVSPGEOMKeyFile(file_name);

    return mesh_id;

}


//==== Write Nascart Files ====//
string Vehicle::WriteNascartFiles( const string & file_name, int write_set )
{
    string mesh_id = string();

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    if ( !geom_vec[0] )
    {
        return mesh_id;
    }

    // Add a new mesh if one does not exist
    if ( !ExistMesh( write_set ) )
    {
        mesh_id = AddMeshGeom( write_set );
        if ( mesh_id.compare( "NONE" ) != 0 )
        {
            Geom* geom_ptr = FindGeom( mesh_id );
            if ( geom_ptr )
            {
                MeshGeom* mg = dynamic_cast<MeshGeom*>( geom_ptr );
                mg->SubTagTris( true );
                geom_vec.push_back( geom_ptr );
                geom_ptr->Update();
            }
            HideAllExcept( mesh_id );
        }
    }

    FILE* file_id = fopen( file_name.c_str(), "w" );

    if ( !file_id )
    {
        return mesh_id;
    }

    //==== Count Number of Points & Tris ====//
    int i;
    int num_pnts = 0;
    int num_tris = 0;
    int num_parts = 0;
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            mg->BuildIndexedMesh( num_parts );
            num_parts += mg->GetNumIndexedParts();
            num_pnts += mg->GetNumIndexedPnts();
            num_tris += mg->GetNumIndexedTris();
        }
    }
    fprintf( file_id, "%d %d\n", num_pnts, num_tris );

    //==== Dump Points ====//
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            mesh_id = geom_vec[i]->GetID(); // Set ID in case mesh already existed

            mg->WriteNascartPnts( file_id );
        }
    }

    int offset = 0;
    //==== Dump Tris ====//
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            offset = mg->WriteNascartTris( file_id, offset );
        }
    }

    fclose( file_id );

    string key_name = file_name;
    std::string::size_type loc = key_name.find_last_of( "." );
    if ( loc == key_name.npos )
    {
        key_name = string( "bodyin.key" );
    }
    else
    {
        key_name = key_name.substr( 0, loc ).append( ".key" );
    }

    SubSurfaceMgr.WriteNascartKeyFile( key_name );

    SubSurfaceMgr.WriteTKeyFile(file_name);

    return mesh_id;

}

string Vehicle::WriteGmshFile( const string & file_name, int write_set )
{
    string mesh_id = string();

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    if ( !geom_vec[0] )
    {
        return mesh_id;
    }

    // Add a new mesh if one does not exist
    if ( !ExistMesh( write_set ) )
    {
        mesh_id = AddMeshGeom( write_set );
        if ( mesh_id.compare( "NONE" ) != 0 )
        {
            Geom* geom_ptr = FindGeom( mesh_id );
            if ( geom_ptr )
            {
                MeshGeom* mg = dynamic_cast<MeshGeom*>( geom_ptr );
                mg->SubTagTris( true );
                geom_vec.push_back( geom_ptr );
                geom_ptr->Update();
            }
            HideAllExcept( mesh_id );
        }
    }

    FILE* file_id = fopen( file_name.c_str(), "w" );

    if( !file_id )
    {
        return mesh_id;
    }

    //==== Count Number of Points & Tris ====//
    int num_pnts = 0;
    int num_tris = 0;
    int num_parts = 0;
    int i;
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            mg->BuildIndexedMesh( num_parts );
            num_parts += mg->GetNumIndexedParts();
            num_pnts += mg->GetNumIndexedPnts();
            num_tris += mg->GetNumIndexedTris();
        }
    }

    fprintf( file_id, "$MeshFormat\n" );
    fprintf( file_id, "2.2 0 %d\n", ( int )sizeof( double ) );
    fprintf( file_id, "$EndMeshFormat\n" );

    //==== Dump Nodes ====//
    fprintf( file_id, "$Nodes\n" );
    fprintf( file_id, "%d\n", num_pnts );
    int node_offset = 0;
    vector< int > node_offset_vec;
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        node_offset_vec.push_back( node_offset );
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            mesh_id = geom_vec[i]->GetID(); // Set ID in case mesh already existed

            node_offset = mg->WriteGMshNodes( file_id, node_offset );
        }
    }
    fprintf( file_id, "$EndNodes\n" );

    //==== Dump Tris ====//
    fprintf( file_id, "$Elements\n" );
    fprintf( file_id, "%d\n", num_tris );
    int tri_offset = 0;
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom* mg = ( MeshGeom* )geom_vec[i];            // Cast
            tri_offset = mg->WriteGMshTris( file_id, node_offset_vec[i], tri_offset );
        }
    }
    fprintf( file_id, "$EndElements\n" );

    fclose( file_id );

    return mesh_id;
}

void Vehicle::WriteX3DFile( const string & file_name, int write_set )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    if ( !geom_vec[0] )
    {
        return;
    }

    xmlDocPtr doc = xmlNewDoc( BAD_CAST "1.0" );

    xmlNodePtr root = xmlNewNode( NULL, BAD_CAST "X3D" );
    xmlDocSetRootElement( doc, root );

    xmlNodePtr scene_node = xmlNewChild( root, NULL, BAD_CAST "Scene", NULL );

    WriteX3DViewpoints( scene_node );

    //==== All Geometry ====//
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type != BLANK_GEOM_TYPE && geom_vec[i]->GetType().m_Type != HINGE_GEOM_TYPE )
        {
            xmlNodePtr shape_node = xmlNewChild( scene_node, NULL, BAD_CAST "Shape", NULL );

            xmlNodePtr app_node = xmlNewChild( shape_node, NULL, BAD_CAST "Appearance", NULL );

            WriteX3DMaterial( app_node, geom_vec[i]->GetMaterial() ); // Default until materials are implemented in Geom

            geom_vec[i]->WriteX3D( shape_node );
        }
    }

    //===== Save XML Tree and Free Doc =====//
    xmlSaveFormatFile( file_name.c_str(), doc, 1 );
    xmlFreeDoc( doc );
}

void Vehicle::WriteX3DViewpoints( xmlNodePtr node )
{
    char numstr[255];
    string format3;
    string format4;
    format3 = "%lf %lf %lf";
    format4 = format3 + " %lf";

    //==== Update box and get key values ====//
    UpdateBBox();
    vec3d center = m_BBox.GetCenter();
    double len = m_BBox.DiagDist();
    double fov = .4;
    double dist = len / ( 2 * tan( fov / 2 ) );

    // Set the names and vectors to the different viewpoints //
    string x3d_views[] = {"iso", "front", "top", "right"};
    double view_degree[4][4] = { { -1, -1, 1, -PI / 4 } , { -1, 0, 0, -PI / 2}, {0, 0, 1, 0}, {0, -1, 0, 0} };
    vec3d k = vec3d( 0, 0, 1 );

    for( int i = 0; i < sizeof( view_degree ) / sizeof( double[4] ) ; i++ )
    {
        vec3d view_axis = vec3d( view_degree[i][0], view_degree[i][1], view_degree[i][2] );
        view_axis.normalize();

        vec3d rot_axis = cross( k, view_axis );
        double angle = asin( rot_axis.mag() );
        rot_axis.normalize();

        // if rotating view again combine rotations using quaternions //
        if ( view_degree[i][3] != 0 )
        {
            quat rot1 = quat( rot_axis, angle );
            quat rot2 = quat( view_axis, view_degree[i][3] );
            quat combined_rot = hamilton( rot2, rot1 );
            combined_rot.quat2axisangle( rot_axis, angle );
        }

        vec3d position = center + ( view_axis * dist );

        // Convert vectors to strings //
        string orients, cents, posits, name, sfov;

        sprintf( numstr, format4.c_str(),  rot_axis.x(), rot_axis.y(), rot_axis.z(), angle );
        orients = numstr;

        sprintf( numstr, format3.c_str(), center.x(), center.y(), center.z() );
        cents = numstr;

        sprintf( numstr, format3.c_str(), position.x(), position.y(), position.z() );
        posits = numstr;

        sprintf( numstr, "%lf", fov );
        sfov = numstr;

        // write first viewpoint twice so viewpoint buttons will work correctly //
        if ( i == 0 )
        {
            xmlNodePtr first_view_node = xmlNewChild( node, NULL, BAD_CAST "Viewpoint", BAD_CAST " " );
            WriteX3DViewpointProps( first_view_node, orients, cents, posits, sfov, string( "first" ) );
        }

        // write each viewpoint node's properties //
        xmlNodePtr viewpoint_node = xmlNewChild( node, NULL, BAD_CAST "Viewpoint", BAD_CAST " " );
        WriteX3DViewpointProps( viewpoint_node, orients, cents, posits, sfov, x3d_views[i].c_str() );
    }
}

void Vehicle::WriteX3DViewpointProps( xmlNodePtr node, string orients, string cents, string posits, string sfov, string name )
{
    xmlSetProp( node, BAD_CAST "id", BAD_CAST name.c_str() );
    xmlSetProp( node, BAD_CAST "description", BAD_CAST name.c_str() );
    xmlSetProp( node, BAD_CAST "orientation", BAD_CAST orients.c_str() );
    xmlSetProp( node, BAD_CAST "centerOfRotation", BAD_CAST cents.c_str() );
    xmlSetProp( node, BAD_CAST "position", BAD_CAST posits.c_str() );
    xmlSetProp( node, BAD_CAST "fieldOfView", BAD_CAST sfov.c_str() );
}

void Vehicle::WriteX3DMaterial( xmlNodePtr node, Material * material )
{
    if ( !material ) return;

    string diffs, emisss, specs;
    char numstr[255];
    vec3d dif,emi,spec,amb;
    xmlNodePtr mat_node = xmlNewChild( node, NULL, BAD_CAST "Material", BAD_CAST " " );

    material->GetDiffuse(dif);
    dif = dif/255.0;

    diffs = StringUtil::vec3d_to_string(dif,"%lf %lf %lf");
    xmlSetProp( mat_node, BAD_CAST "diffuseColor", BAD_CAST diffs.c_str() );

    material->GetEmissive(emi);
    emi = emi/255.0;
    emisss = StringUtil::vec3d_to_string(emi,"%lf %lf %lf");
    xmlSetProp( mat_node, BAD_CAST "emissiveColor", BAD_CAST emisss.c_str() );

    material->GetSpecular(spec);
    spec = spec/255.0;
    specs = StringUtil::vec3d_to_string(spec,"%lf %lf %lf");
    xmlSetProp( mat_node, BAD_CAST "specularColor", BAD_CAST specs.c_str() );

    double alpha;
    material->GetAlpha(alpha);
    sprintf( numstr, "%lf", 1.0-alpha );
    xmlSetProp( mat_node, BAD_CAST "transparency", BAD_CAST numstr );

    double shine;
    material->GetShininess(shine);
    sprintf( numstr, "%lf", shine );
    xmlSetProp( mat_node, BAD_CAST "shininess", BAD_CAST numstr );

    material->GetAmbient(amb);
    double ambd = 0.0;

    for( int i = 0; i < 3; i++ )
    {
        ambd += amb[i] / dif[i];
    }
    ambd = ambd / 3.0;



    sprintf( numstr, "%lf", ambd );
    xmlSetProp( mat_node, BAD_CAST "ambientIntensity", BAD_CAST numstr );
}

void Vehicle::WritePovRayFile( const string & file_name, int write_set )
{
    UpdateBBox();
    int i;

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    if ( !geom_vec[0] )
    {
        return;
    }

    //==== Figure Out Basename ====//
    string base_name = file_name;
    std::string::size_type loc = base_name.find_last_of( "." );
    if ( loc != base_name.npos )
    {
        base_name = base_name.substr( 0, loc );
    }

    string inc_file_name = base_name;
    inc_file_name.append( ".inc" );

    FILE* inc_out = fopen( inc_file_name.c_str(), "w" );
    int comp_num = 0;
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) )
        {
            geom_vec[i]->WritePovRay( inc_out, comp_num );
            comp_num++;
        }
    }

    //==== Add Some Decent Textures ====//
    fprintf( inc_out, "#declare lightgreymetal = texture {\n  pigment { color rgb < 0.9, 0.9, 0.9 > } \n" );
    fprintf( inc_out, "    normal { granite, 0.01 }\n" );
    fprintf( inc_out, "    finish { ambient 0.2  diffuse 1.0   specular 0.5  roughness 0.1  metallic reflection 0.05 }\n}\n" );

    fprintf( inc_out, "#declare darkgreymetal = texture {\n  pigment { color rgb < 0.7, 0.7, 0.7 > } \n" );
    fprintf( inc_out, "    normal { granite, 0.005 }\n" );
    fprintf( inc_out, "    finish { ambient 0.2  diffuse 1.0   specular 0.8  roughness 0.15   }\n}\n" );

    fprintf( inc_out, "#declare bluegreymetal = texture {\n  pigment { color rgb < 0.4, 0.4, 0.45 > } \n" );
    fprintf( inc_out, "    normal { granite, 0.02 }\n" );
    fprintf( inc_out, "    finish { ambient 0.2  diffuse 1.0   specular 0.8  roughness 0.15 metallic  }\n}\n" );

    fprintf( inc_out, "#declare canopyglass = texture {\n  pigment { color rgb < 0.3, 0.3, 0.3, 0.5 > } \n" );
    fprintf( inc_out, "    finish { ambient 0.1  diffuse 1.0   specular 1.0  roughness 0.01 reflection 0.4  }\n}\n" );

    fclose( inc_out );

    //==== Open POV File  ====//
    string pov_file_name = base_name;
    pov_file_name += ".pov";

    FILE* pov_file = fopen( pov_file_name.c_str(), "w" );

    fprintf( pov_file, "#version 3.6;\n\n" );
    fprintf( pov_file, "#include \"%s\"\n", inc_file_name.c_str() );

    fprintf( pov_file, "#include \"colors.inc\"\n" );
    fprintf( pov_file, "#include \"shapes.inc\"\n" );
    fprintf( pov_file, "#include \"textures.inc\"\n\n" );

    fprintf( pov_file, "global_settings { assumed_gamma 1 }\n\n" );

    BndBox bb = GetBndBox();
    vec3d center = bb.GetCenter();
    double diag  = bb.DiagDist();
    double mult  = 1.3;

    fprintf( pov_file, "camera { location < %6.1f, %6.1f, %6.1f > sky < 0.0, 0.0, -1.0 >  look_at < %6.1f, %6.1f, %6.1f > }\n",
             mult * bb.GetMin( 0 ), mult * bb.GetMin( 1 ), mult * bb.GetMin( 2 ), center.x(), center.z(), center.y() );

    fprintf( pov_file, "light_source { < %6.1f, %6.1f, %6.1f >  color White }\n", center.x(), center.z() + diag, center.y() );

    comp_num = 0;
    for ( i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if (  geom_vec[i]->GetSetFlag( write_set ) )
        {
            string name = geom_vec[i]->GetName();
            StringUtil::chance_space_to_underscore( name );
            fprintf( pov_file, "mesh { %s_%d texture {darkgreymetal} } \n", name.c_str(), comp_num );
            comp_num++;
        }
    }

    fclose( pov_file );
}

void Vehicle::FetchXFerSurfs( int write_set, vector< XferSurf > &xfersurfs )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );

    int icomp = 0;
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if( geom_vec[i]->GetSetFlag( write_set ) )
        {
            vector<VspSurf> surf_vec;
            surf_vec = geom_vec[i]->GetSurfVecConstRef();

            for ( int j = 0; j < ( int )surf_vec.size(); j++ )
            {
                surf_vec[j].FetchXFerSurf( geom_vec[i]->GetID(), geom_vec[i]->GetMainSurfID( j ), icomp, j, xfersurfs );
                icomp++;
            }
        }
    }
}

void Vehicle::WriteSTEPFile( const string & file_name, int write_set )
{
    WriteSTEPFile( file_name, write_set, m_STEPLabelID(), m_STEPLabelName(), m_STEPLabelSurfNo(), m_STEPLabelDelim() );
}

void Vehicle::WriteSTEPFile( const string & file_name, int write_set, bool labelID,
                             bool labelName, bool labelSurfNo, int delimType )
{
    string delim = StringUtil::get_delim( delimType );

    STEPutil step( m_STEPLenUnit(), m_STEPTol() );

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if( geom_vec[i]->GetSetFlag( write_set ) )
        {
            vector<VspSurf> surf_vec;
            surf_vec = geom_vec[i]->GetSurfVecConstRef();

            for ( int j = 0; j < surf_vec.size(); j++ )
            {
                int mainid = geom_vec[i]->GetMainSurfID( j );

                vector < double > usplit;
                vector < double > wsplit;

                vector < SubSurface *> ssvec = geom_vec[i]->GetSubSurfVec();

                if ( m_STEPSplitSubSurfs() )
                {
                    for ( int k = 0; k < ssvec.size(); k++ )
                    {
                        SubSurface *sub = ssvec[ k ];
                        if ( sub )
                        {
                            if( sub->m_MainSurfIndx() == mainid )
                            {
                                if( sub->GetType() == vsp::SS_LINE )
                                {
                                    SSLine *subline = (SSLine*) sub;

                                    if( subline->m_ConstType() == vsp::CONST_U )
                                    {
                                        usplit.push_back( subline->m_ConstVal() * surf_vec[j].GetUMax() );
                                    }
                                    else
                                    {
                                        wsplit.push_back( subline->m_ConstVal() * surf_vec[j].GetWMax() );
                                    }
                                }
                            }
                        }
                    }
                }

                string prefix;

                if ( labelID )
                {
                    prefix = geom_vec[i]->GetID();
                }

                if ( labelName )
                {
                    if ( prefix.size() > 0 )
                    {
                        prefix.append( delim );
                    }
                    prefix.append( geom_vec[i]->GetName() );
                }

                if ( labelSurfNo )
                {
                    if ( prefix.size() > 0 )
                    {
                        prefix.append( delim );
                    }
                    prefix.append( to_string( j ) );
                }

                vector < SdaiB_spline_surface_with_knots* > surfs;
                surf_vec[j].ToSTEP_BSpline_Quilt( &step, surfs, prefix, m_STEPSplitSurfs(), m_STEPMergePoints(), m_STEPToCubic(), m_STEPToCubicTol(), m_STEPTrimTE(), usplit, wsplit );

                step.RepresentUntrimmedSurfs( surfs, prefix );
            }
        }
    }

    step.WriteFile( file_name );
}

void Vehicle::WriteStructureSTEPFile( const string & file_name )
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( m_STEPStructureExportIndex() );
    if ( !fea_struct )
    {
        printf( "ERROR WriteStructureSTEPFile: No FEA Structure Found\n" );
        return;
    }

    int len = UNIT_FOOT;
    switch ( m_StructUnit() )
    {
        case vsp::SI_UNIT:
            len = UNIT_METER;
            break;

        case vsp::CGS_UNIT:
            len =  UNIT_CENTIMETER;
            break;

        case vsp::MPA_UNIT:
            len =  UNIT_MM;
            break;

        case vsp::BFT_UNIT:
            len = UNIT_FOOT;
            break;

        case vsp::BIN_UNIT:
            len =  UNIT_IN;
            break;
    }

    STEPutil step( len, m_STEPStructureTol() );

    string delim = StringUtil::get_delim( m_STEPStructureLabelDelim() );

    vector < double > usplit;
    vector < double > wsplit;


    vector < FeaPart* > fea_part_vec = fea_struct->GetFeaPartVec();

    for ( int i = 0; i < fea_part_vec.size(); i++ )
    {
        FeaPart* part = fea_part_vec[i];
        vector < VspSurf > surf_vec = part->GetFeaPartSurfVec();

        for ( int j = 0; j < surf_vec.size(); j++ )
        {
            string prefix;

            if ( m_STEPStructureLabelID() )
            {
                prefix = fea_struct->GetParentGeomID();
            }

            if ( m_STEPStructureLabelName() )
            {
                if ( prefix.size() > 0 )
                {
                    prefix.append( delim );
                }
                prefix.append( part->GetName() );
            }

            if ( m_STEPStructureLabelSurfNo() )
            {
                if ( prefix.size() > 0 )
                {
                    prefix.append( delim );
                }
                prefix.append( to_string( j ) );
            }

            vector < SdaiB_spline_surface_with_knots* > surfs;
            surf_vec[j].ToSTEP_BSpline_Quilt( &step, surfs, prefix, m_STEPStructureSplitSurfs(), m_STEPStructureMergePoints(), m_STEPStructureToCubic(), m_STEPStructureToCubicTol(), false, usplit, wsplit );

            step.RepresentUntrimmedSurfs( surfs );
        }
    }

    step.WriteFile( file_name );
}

void Vehicle::WriteIGESFile( const string & file_name, int write_set )
{
    WriteIGESFile( file_name, write_set, m_IGESLenUnit(), m_IGESSplitSubSurfs(), m_IGESSplitSurfs(), m_IGESToCubic(),
                   m_IGESToCubicTol(), m_IGESTrimTE(), m_IGESLabelID(), m_IGESLabelName(), m_IGESLabelSurfNo(),
                   m_IGESLabelSplitNo(), m_IGESLabelDelim() );
}

void Vehicle::WriteIGESFile( const string & file_name, int write_set, int lenUnit, bool splitSubSurfs,
                             bool splitSurfs, bool toCubic, double toCubicTol, bool trimTE, bool labelID,
                             bool labelName, bool labelSurfNo, bool labelSplitNo, int delimType )
{
    string delim = StringUtil::get_delim( delimType );

    IGESutil iges( lenUnit );

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if( geom_vec[i]->GetSetFlag( write_set ) )
        {
            vector<VspSurf> surf_vec;
            surf_vec = geom_vec[i]->GetSurfVecConstRef();

            for ( int j = 0; j < surf_vec.size(); j++ )
            {
                int mainid = geom_vec[i]->GetMainSurfID( j );

                vector < double > usplit;
                vector < double > wsplit;

                vector < SubSurface *> ssvec = geom_vec[i]->GetSubSurfVec();

                if ( splitSubSurfs )
                {
                    for ( int k = 0; k < ssvec.size(); k++ )
                    {
                        SubSurface *sub = ssvec[ k ];
                        if ( sub )
                        {
                            if( sub->m_MainSurfIndx() == mainid )
                            {
                                if( sub->GetType() == vsp::SS_LINE )
                                {
                                    SSLine *subline = (SSLine*) sub;

                                    if( subline->m_ConstType() == vsp::CONST_U )
                                    {
                                        usplit.push_back( subline->m_ConstVal() * surf_vec[j].GetUMax() );
                                    }
                                    else
                                    {
                                        wsplit.push_back( subline->m_ConstVal() * surf_vec[j].GetWMax()  );
                                    }
                                }
                            }
                        }
                    }
                }

                string prefix;

                if ( labelID )
                {
                    prefix = geom_vec[i]->GetID();
                }

                if ( labelName )
                {
                    if ( prefix.size() > 0 )
                    {
                        prefix.append( delim );
                    }
                    prefix.append( geom_vec[i]->GetName() );
                }

                if ( labelSurfNo )
                {
                    if ( prefix.size() > 0 )
                    {
                        prefix.append( delim );
                    }
                    prefix.append( to_string( j ) );
                }

                surf_vec[j].ToIGES( &iges, splitSurfs, toCubic, toCubicTol, trimTE, usplit, wsplit, prefix, labelSplitNo, delim );
            }
        }
    }

    iges.WriteFile( file_name.c_str(), true );
}

void Vehicle::WriteStructureIGESFile( const string & file_name )
{
    WriteStructureIGESFile( file_name, m_IGESStructureExportIndex(), m_IGESStructureSplitSurfs(), m_IGESStructureToCubic(),
                   m_IGESStructureToCubicTol(), m_IGESStructureLabelID(), m_IGESStructureLabelName(), m_IGESStructureLabelSurfNo(),
                   m_IGESStructureLabelSplitNo(), m_IGESStructureLabelDelim() );
}

void Vehicle::WriteStructureIGESFile( const string & file_name, int feaMeshStructIndex,
                             bool splitSurfs, bool toCubic, double toCubicTol, bool labelID,
                             bool labelName, bool labelSurfNo, bool labelSplitNo, int delimType )
{
    FeaStructure* fea_struct = StructureMgr.GetFeaStruct( feaMeshStructIndex );
    if ( !fea_struct )
    {
        printf( "ERROR WriteStructureIGESFile: No FEA Structure Found\n" );
        return;
    }

    string delim = StringUtil::get_delim( delimType );

    IGESutil iges( m_StructUnit() );

    vector < double > usplit;
    vector < double > wsplit;


    vector < FeaPart* > fea_part_vec = fea_struct->GetFeaPartVec();

    for ( int i = 0; i < fea_part_vec.size(); i++ )
    {
        FeaPart* part = fea_part_vec[i];
        vector < VspSurf > surf_vec = part->GetFeaPartSurfVec();

        for ( int j = 0; j < surf_vec.size(); j++ )
        {
            string prefix;

            if ( labelID )
            {
                prefix = fea_struct->GetParentGeomID();
            }

            if ( labelName )
            {
                if ( prefix.size() > 0 )
                {
                    prefix.append( delim );
                }
                prefix.append( part->GetName() );
            }

            if ( labelSurfNo )
            {
                if ( prefix.size() > 0 )
                {
                    prefix.append( delim );
                }
                prefix.append( to_string( j ) );
            }

            surf_vec[j].ToIGES( &iges, splitSurfs, toCubic, toCubicTol, false, usplit, wsplit, prefix, labelSplitNo, delim );
        }
    }

    iges.WriteFile( file_name.c_str(), true );
}

void Vehicle::WriteBEMFile( const string &file_name, int write_set )
{
    Geom* geom = FindGeom( m_BEMPropID );

    PropGeom* pgeom = dynamic_cast < PropGeom* > ( geom );
    if ( pgeom )
    {
        string rid = pgeom->BuildBEMResults();

        Results* resptr = ResultsMgr.FindResultsPtr( rid );
        if( resptr )
        {
            resptr->WriteBEMFile( file_name );
        }
    }
}

void Vehicle::WriteAirfoilFile( const string &file_name, int write_set )
{
    FILE* meta_fid = fopen( file_name.c_str(), "w" );
    if ( !meta_fid )
    {
        return;
    }

    fprintf( meta_fid, "# AIRFOIL METADATA CSV FILE\n\n" );

    // Determine directory to place all airfoil files (same directory as metadata file)
    size_t last_index = file_name.find_last_of( "/\\" );
    if ( last_index > 0 && last_index != std::string::npos )
    {
        m_AFFileDir = file_name.substr( 0, ( last_index + 1 ) );
        fprintf( meta_fid, "Airfoil File Directory, %s\n\n", m_AFFileDir.c_str() );
    }
    else if ( m_ExePath.find_last_of( "/\\" ) > 0 && m_ExePath.find_last_of( "/\\" ) != std::string::npos )
    {
        m_AFFileDir = m_ExePath.substr( 0, ( m_ExePath.find_last_of( "/\\" ) + 1 ) ); // place all airfoil files in executable directory
        fprintf( meta_fid, "Airfoil File Directory, %s\n\n", m_AFFileDir.c_str() );
    }
    else
    {
        m_AFFileDir = string();
    }

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );

    for ( int i = 0; i < (int)geom_vec.size(); i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && ( geom_vec[i]->GetType().m_Type == MS_WING_GEOM_TYPE || geom_vec[i]->GetType().m_Type == PROP_GEOM_TYPE ) )
        {
            geom_vec[i]->WriteAirfoilFiles( meta_fid );
        }
    }

    fclose( meta_fid );
}

void Vehicle::WriteDXFFile( const string & file_name, int write_set )
{
    FILE* dxf_file = fopen( file_name.c_str(), "w" );

    if ( dxf_file )
    {
        vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );

        if ( geom_vec.size() == 0 )
        {
            fclose( dxf_file );
            return;
        }

        BndBox dxfbox;

        // Clear Vehicle Projection Line Vec and Reset Color Count
        m_ColorCount = 0;
        m_VehProjectVec3d.clear();
        m_VehProjectVec3d.resize( 3 );

        // Tessellation adjustment
        // Tessellation must be an integer -- something rational should be done with either
        // tessfactor or the places where it is used.
        double tessfactor = m_DXFTessFactor.Get();

        for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
        {
            if ( geom_vec[i]->GetSetFlag( write_set ) )
            {
                // Get Vehicle Bounding Box
                dxfbox.Update( geom_vec[i]->GetBndBox() );

                // Clear Geom Projection Vec
                geom_vec[i]->ClearGeomProjectVec3d();

                if ( ( m_DXFProjectionFlag() && m_DXFTessFactor.Get() != 1.0 ) )
                {
                    // Increase tellelation:
                    geom_vec[i]->m_TessW.Set( (int)( geom_vec[i]->m_TessW() * tessfactor ) );

                    int num_xsec_surf = geom_vec[i]->GetNumXSecSurfs();

                    if ( num_xsec_surf > 0 ) // Increase U Tessellation by section for segmented geoms
                    {
                        for ( unsigned int j = 0; j < num_xsec_surf; j++ )
                        {
                            XSecSurf* xsecsurf = geom_vec[i]->GetXSecSurf( j );

                            if ( xsecsurf )
                            {
                                int num_xsec = xsecsurf->NumXSec();

                                for ( unsigned int k = 0; k < num_xsec; k++ )
                                {
                                    XSec* curr_xsec = xsecsurf->FindXSec( k );

                                    if ( curr_xsec )
                                    {
                                        curr_xsec->m_SectTessU.Set( (int)( curr_xsec->m_SectTessU() * tessfactor ) );
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        geom_vec[i]->m_TessU.Set((int)( geom_vec[i]->m_TessU() * tessfactor ) );
                    }
                }

                if ( m_DXFAllXSecFlag() )
                {
                    // Force XSec Feature Line Visibility:
                    geom_vec[i]->SetForceXSecFlag( true );
                }

                if ( ( m_DXFProjectionFlag() && tessfactor != 1.0 ) || m_DXFAllXSecFlag() )
                {
                    // Update Geom:
                    geom_vec[i]->Update( true );
                }
            }
        }

        // Write DXF Header
        WriteDXFHeader( dxf_file, m_DXFLenUnit.Get() );

        if ( m_DXFProjectionFlag() )
        {
            // Generate Mesh for Projections:
            vector < TMesh* > TotalProjectMeshVec;

            for ( int i = 0; i < (int)geom_vec.size(); i++ )
            {
                if ( geom_vec[i]->GetSetFlag( write_set ) )
                {
                    vector< TMesh* > tMeshVec = geom_vec[i]->CreateTMeshVec();
                    for ( int j = 0; j < (int)tMeshVec.size(); j++ )
                    {
                        TotalProjectMeshVec.push_back( tMeshVec[j] );
                    }
                }
            }
            // Generate Geom and Vehicle Projection Line Vectors:
            ProjectionMgr.ExportProjectLines( TotalProjectMeshVec );

            // Delete TMesh Pointers
            for ( unsigned int i = 0; i < TotalProjectMeshVec.size(); i++ )
            {
                delete TotalProjectMeshVec[i];
            }
            TotalProjectMeshVec.clear();
        }

        for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
        {
            if ( geom_vec[i]->GetSetFlag( write_set ) )
            {
                // Write Geom Projection Lines:
                if ( m_DXFProjectionFlag() )
                {
                    geom_vec[i]->WriteProjectionLinesDXF( dxf_file, dxfbox );
                }

                // Write Feature Lines:
                geom_vec[i]->WriteFeatureLinesDXF( dxf_file, dxfbox );
            }
        }

        if ( m_DXFProjectionFlag() )
        {
            // Write Total Projection Lines:
            WriteVehProjectionLinesDXF( dxf_file, dxfbox );
        }

        for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
        {
            if ( geom_vec[i]->GetSetFlag( write_set ) )
            {
                // Clear Geom Projection Vec
                geom_vec[i]->ClearGeomProjectVec3d();

                if ( ( m_DXFProjectionFlag() && tessfactor != 1.0 ) )
                {
                    // Restore tellelation and update:
                    geom_vec[i]->m_TessW.Set( (int)( geom_vec[i]->m_TessW.GetLastVal() ) );

                    int num_xsec_surf = geom_vec[i]->GetNumXSecSurfs();

                    if ( num_xsec_surf > 0 ) // Restore U Tessellation by section for segmented geoms
                    {
                        for ( unsigned int j = 0; j < num_xsec_surf; j++ )
                        {
                            XSecSurf* xsecsurf = geom_vec[i]->GetXSecSurf( j );

                            if ( xsecsurf )
                            {
                                int num_xsec = xsecsurf->NumXSec();

                                for ( unsigned int k = 0; k < num_xsec; k++ )
                                {
                                    XSec* curr_xsec = xsecsurf->FindXSec( k );

                                    if ( curr_xsec )
                                    {
                                        curr_xsec->m_SectTessU.Set( (int)( curr_xsec->m_SectTessU.GetLastVal() ) );
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        geom_vec[i]->m_TessU.Set( (int)( geom_vec[i]->m_TessU.GetLastVal() ) );
                    }
                }

                if ( m_DXFAllXSecFlag() )
                {
                    // Restore Feature Line Visibility:
                    geom_vec[i]->SetForceXSecFlag( false );
                }

                if ( ( m_DXFProjectionFlag() && tessfactor != 1.0 ) || m_DXFAllXSecFlag() )
                {
                    // Update Geom:
                    geom_vec[i]->Update( true );
                }
            }
        }

        WriteDXFClose( dxf_file );

        fclose( dxf_file );

        // Clear Projection Line Vec:
        m_VehProjectVec3d.clear(); 

        // Restore Color Counter:
        m_ColorCount = 0;
    }
    else
    {
        fprintf( stderr, "Error: File export failed\nFile: %s\n", file_name.c_str() );
    }
}

void Vehicle::WriteSVGFile( const string & file_name, int write_set )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );

    if ( geom_vec.size() == 0 )
    {
        return;
    }

    xmlDocPtr doc = xmlNewDoc( ( const xmlChar * )"1.0" );

    xmlNodePtr root = xmlNewNode( NULL, ( const xmlChar * )"svg" );
    doc->standalone=0;
    
    xmlDocSetRootElement( doc, root );

    // Tessellation adjustment
    double tessfactor = m_SVGTessFactor.Get();

    BndBox svgbox;

    // Clear Vehicle Projection Line Vec
    m_VehProjectVec3d.clear();
    m_VehProjectVec3d.resize( 3 );

    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) )
        {
            // Get Vehicle Bounding Box
            svgbox.Update( geom_vec[i]->GetBndBox() );

            // Clear Geom Projection Vec
            geom_vec[i]->ClearGeomProjectVec3d();

            if ( ( m_SVGProjectionFlag() && tessfactor != 1.0 ) )
            {
                // Increase tellelation:
                geom_vec[i]->m_TessW.Set( (int)( geom_vec[i]->m_TessW() * tessfactor ) );

                int num_xsec_surf = geom_vec[i]->GetNumXSecSurfs();

                if ( num_xsec_surf > 0 ) // Increase U Tessellation by section for segmented geoms
                {
                    for ( unsigned int j = 0; j < num_xsec_surf; j++ )
                    {
                        XSecSurf* xsecsurf = geom_vec[i]->GetXSecSurf( j );

                        if ( xsecsurf )
                        {
                            int num_xsec = xsecsurf->NumXSec();

                            for ( unsigned int k = 0; k < num_xsec; k++ )
                            {
                                XSec* curr_xsec = xsecsurf->FindXSec( k );

                                if ( curr_xsec )
                                {
                                    curr_xsec->m_SectTessU.Set( (int)( curr_xsec->m_SectTessU() * tessfactor ) );
                                }
                            }
                        }
                    }
                }
                else
                {
                    geom_vec[i]->m_TessU.Set( (int)( geom_vec[i]->m_TessU() * tessfactor ) );
                }
            }

            if ( m_SVGAllXSecFlag() )
            {
                // Force XSec Feature Line Visibility:
                geom_vec[i]->SetForceXSecFlag( true );
            }

            if ( ( m_SVGProjectionFlag() && tessfactor != 1.0 ) || m_SVGAllXSecFlag() )
            {
                // Update Geom:
                geom_vec[i]->Update( true );
            }
        }
    }

    // Write SVG Header:
    WriteSVGHeader( root, svgbox );

    if ( m_SVGProjectionFlag() )
    {
        // Generate Mesh for Projections:
        vector < TMesh* > TotalProjectMeshVec;

        for ( int i = 0; i < (int)geom_vec.size(); i++ )
        {
            if ( geom_vec[i]->GetSetFlag( write_set ) )
            {
                vector< TMesh* > tMeshVec = geom_vec[i]->CreateTMeshVec();
                for ( int j = 0; j < (int)tMeshVec.size(); j++ )
                {
                    TotalProjectMeshVec.push_back( tMeshVec[j] );
                }
            }
        }

        // Generate Geom and Vehicle Projection Line Vectors:
        ProjectionMgr.ExportProjectLines( TotalProjectMeshVec );

        // Delete TMesh Pointers 
        for ( unsigned int i = 0; i < TotalProjectMeshVec.size(); i++ )
        {
            delete TotalProjectMeshVec[i];
        }
        TotalProjectMeshVec.clear();
    }

    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) )
        {
            if ( m_SVGProjectionFlag() )
            {
                // Write Geom Projection Lines
                geom_vec[i]->WriteProjectionLinesSVG( root, svgbox );
            }

            // Write Feature Lines
            geom_vec[i]->WriteFeatureLinesSVG( root, svgbox );
        }
    }

    if ( m_SVGProjectionFlag() )
    {
        // Write Vehicle Projection Lines
        WriteVehProjectionLinesSVG( root, svgbox );
    }

    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) )
        {
            // Clear Geom Projection Line Vec
            geom_vec[i]->ClearGeomProjectVec3d();

            if ( ( m_SVGProjectionFlag() && tessfactor != 1.0 ) )
            {
                // Restore tellelation and update:
                geom_vec[i]->m_TessW.Set( (int)( geom_vec[i]->m_TessW.GetLastVal() ) );

                int num_xsec_surf = geom_vec[i]->GetNumXSecSurfs();

                if ( num_xsec_surf > 0 ) // Restore U Tessellation by section for segmented geoms
                {
                    for ( unsigned int j = 0; j < num_xsec_surf; j++ )
                    {
                        XSecSurf* xsecsurf = geom_vec[i]->GetXSecSurf( j );

                        if ( xsecsurf )
                        {
                            int num_xsec = xsecsurf->NumXSec();

                            for ( unsigned int k = 0; k < num_xsec; k++ )
                            {
                                XSec* curr_xsec = xsecsurf->FindXSec( k );

                                if ( curr_xsec )
                                {
                                    curr_xsec->m_SectTessU.Set( (int)( curr_xsec->m_SectTessU.GetLastVal() ) );
                                }
                            }
                        }
                    }
                }
                else
                {
                    geom_vec[i]->m_TessU.Set( (int)( geom_vec[i]->m_TessU.GetLastVal() ) );
                }
            }

            if ( m_SVGAllXSecFlag() )
            {
                // Restore Feature Line Visibility:
                geom_vec[i]->SetForceXSecFlag( false );
            }

            if ( ( m_SVGProjectionFlag() && tessfactor != 1.0 ) || m_SVGAllXSecFlag() )
            {
                // Update Geom:
                geom_vec[i]->Update( true );
            }
        }
    }

    // Add Scale Bar:
    if ( m_SVGLenUnit() != vsp::LEN_UNITLESS )
    {
        WriteSVGScaleBar( root, m_SVGView.Get(), svgbox, m_SVGLenUnit.Get(), m_Scale.Get() );
    }

    //===== Save XML Tree and Free Doc =====//
    int err = xmlSaveFormatFile( file_name.c_str(), doc, 1 );
    xmlFreeDoc( doc );

    // Clear Vehicle Projection Line Vec:
    m_VehProjectVec3d.clear();

    if( err == -1 )  // Failure occurred
    {
        fprintf( stderr, "Error: File export failed\nFile: %s\n", file_name.c_str() );
    }
}

void Vehicle::WritePMARCFile( const string & file_name, int write_set )
{

    int ntstep = 10;
    double dtstep = 0.5;
    double alpha = 10.0;
    double beta = 0.0;

    double cbar = 1.0;
    double Sref = 1.0;
    double b = 1.0;

    //==== Open file ====//
    FILE* fp = fopen( file_name.c_str(), "w" );

    // PMARC header.

    // Case title.
    fprintf(fp," OpenVSP_PMARC_Export\n");
    // Program run/output verbosity options.
    fprintf(fp," &BINP2   LSTINP=2,    LSTOUT=0,    LSTFRQ=0,    LENRUN=0,    LPLTYP=1,     &END\n");
    // Detailed printout information (not used with LSTOUT=0).
    fprintf(fp," &BINP3   LSTGEO=0,    LSTNAB=0,    LSTWAK=0,    LSTCPV=0,                  &END\n");
    // Matrix solver parameters.
    fprintf(fp," &BINP4   MAXIT=200,   SOLRES=0.0005, &END\n");
    // Time stepped wake parameters.
    fprintf(fp," &BINP5   NTSTPS=%d,   DTSTEP=%10.2f,    &END\n", ntstep, dtstep);
    //  Global symmetry and solver parameters.  Note that PMARC is globally in asymmetrical mode.
    fprintf(fp," &BINP6   RSYM=1.0,    RGPR=0.0,    RFF=5.0,  RCORES=0.050,  RCOREW=0.050,  &END\n");
    // Freestream conditions.  1.0 for nondimensional velocity.
    fprintf(fp," &BINP7   VINF=1.0,    VSOUND=1116.0, &END\n");
    // Orientation angle and rotation rates
    fprintf(fp," &BINP8   ALDEG=%6.2f,   YAWDEG=%6.2f,  PHIDOT=0.0,  THEDOT=0.0, PSIDOT=0.0, &END\n", alpha, beta);
    // Oscillatory motion magnitude and frequencies.
    fprintf(fp," &BINP8A  PHIMAX= 0.0, THEMAX=0.0,  PSIMAX=0.0,\n          WRX=0.0,     WRY=0.0,   WRZ=0.0,   &END\n");
    fprintf(fp," &BINP8B  DXMAX=0.0,   DYMAX=0.0,   DZMAX=0.0,\n          WTX=0.0,     WTY=0.0,   WTZ=0.000, &END\n");

    // Reference conditions.
    fprintf(fp," &BINP9   CBAR=%6.2f,  SREF= %6.2f, SSPAN= %6.2f,\n          RMPX=0.0,    RMPY=0.00, RMPZ=0.00, &END\n", cbar, Sref, b/2.0);

    // Special options.  Panel neighbor changes, boundary condition changes, internal flow problems.
    fprintf(fp," &BINP10  NORSET=0,    NBCHGE=0,    NCZONE=0,\n          NCZPCH=0,    CZDUB=0.0, VREF=00.0, &END\n");
    // Normal velocity specification
    fprintf(fp," &BINP11  NORPCH=0,    NORF=0,      NORL=0,\n          NOCF=0,      NOCL=0,    VNORM=0.0, &END\n");
    // Panel neighbor change
    fprintf(fp," &BINP12  KPAN(1)=0,   KSIDE(1)=0,  NEWNAB(1)=0,    NEWSID(1)=0, &END\n");
    // Boundary layer calculation.  Requires onbody streamlines
    fprintf(fp," &BINP13  NBLIT = 0,   &END\n");

    // Geometry section.
    // Assembly coordinate system position, scale, and rotation.
    fprintf(fp," &ASEM1   ASEMX=    0.0000, ASEMY=    0.0000, ASEMZ=    0.0000,\n");
    fprintf(fp,"          ASCAL=    1.0000, ATHET=   0.0,     NODEA=   5,        &END\n");
    // Assembly coordinate system arbitrary rotation axis.
    fprintf(fp," &ASEM2   APXX=0.00,        APYY=0.00,        APZZ=0.00,\n");
    fprintf(fp,"          AHXX=0.00,        AHYY=1.00,        AHZZ=0.00,         &END\n");
    // Component coordinate system position, scale, and rotation.
    fprintf(fp," &COMP1   COMPX=    0.0000, COMPY=    0.0000, COMPZ=    0.0000,\n");
    fprintf(fp,"          CSCAL=    1.0000, CTHET=   0.0,     NODEC=   5,        &END\n");
    // Component coordinate system arbitrary rotation axis.
    fprintf(fp," &COMP2   CPXX= 0.0000,     CPYY= 0.0000,     CPZZ= 0.0000,\n");
    fprintf(fp,"          CHXX= 0.0000,     CHYY=  1.000,     CHZZ= 0.0000,      &END\n");


    unsigned int ntotal = 0;

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    //==== Write surface boundary points ====//
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) )
        {
            ntotal += geom_vec[i]->GetNumTotalSurfs();
        }
    }

    vector < int > idpat( ntotal );

    int ipatch = 0;
    //==== Write surface boundary points ====//
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) )
        {
            geom_vec[i]->SetupPMARCFile( ipatch, idpat );
        }
    }

    ipatch = 0;
    //==== Write surface boundary points ====//
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) )
        {
            geom_vec[i]->WritePMARCGeomFile(fp, ipatch, idpat);
        }
    }

    ipatch = 0;
    //==== Write surface boundary points ====//
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) )
        {
            geom_vec[i]->WritePMARCWakeFile(fp, ipatch, idpat);
        }
    }

    // Other inputs (minimal namelists to disable features).

    // On body streamline inputs.
    fprintf(fp," &ONSTRM  NONSL =0,             &END\n");
    // Off body velocity scan.
    fprintf(fp," &VS1     NVOLR= 0,  NVOLC= 0,  &END\n");
    // Off body streamline inputs.
    fprintf(fp," &SLIN1   NSTLIN=0,             &END\n");

    fclose(fp);
}

void Vehicle::WriteVehProjectionLinesDXF( FILE * file_name, const BndBox &dxfbox )
{
    bool color = m_DXFColorFlag.Get();

    // Bounding box diagonal, used to separate multi-view drawings
    vec3d shiftvec = dxfbox.GetMax() - dxfbox.GetMin();

    // Shift the vehicle bounding box to align with the +x, +y, +z axes at the orgin
    vec3d to_orgin = GetVecToOrgin( dxfbox );

    string vehiclelayer = "VehicleProjection";

    if ( m_DXF2D3DFlag() == vsp::SET_2D )
    {
        if ( m_DXF2DView() == vsp::VIEW_1 )
        {
            vector < vector < vec3d > > projectionvec = GetVehProjectionLines( m_DXF4View1(), to_orgin );
            FeatureLinesManipulate( projectionvec, m_DXF4View1(), m_DXF4View1_rot(), shiftvec );
            WriteDXFPolylines2D( file_name, projectionvec, vehiclelayer, color, m_ColorCount );
            m_ColorCount++;
        }
        else if ( m_DXF2DView() == vsp::VIEW_2HOR )
        {
            vector < vector < vec3d > > projectionvec1 = GetVehProjectionLines( m_DXF4View1(), to_orgin );

            if ( projectionvec1.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec1, m_DXF4View1(), m_DXF4View1_rot(), shiftvec );
                FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::LEFT, m_DXF4View1_rot(), 0 );
                string vehiclelayer_v1 = vehiclelayer + "_v1";

                WriteDXFPolylines2D( file_name, projectionvec1, vehiclelayer_v1, color, m_ColorCount );
                m_ColorCount++;
            }

            vector < vector < vec3d > > projectionvec2 = GetVehProjectionLines( m_DXF4View2(), to_orgin );

            if ( projectionvec2.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec2, m_DXF4View2(), m_DXF4View2_rot(), shiftvec );
                FeatureLinesShift( projectionvec2, shiftvec, VIEW_SHIFT::RIGHT, m_DXF4View2_rot(), 0 );
                string vehiclelayer_v2 = vehiclelayer + "_v2";

                WriteDXFPolylines2D( file_name, projectionvec2, vehiclelayer_v2, color, m_ColorCount );
                m_ColorCount++;
            }
        }
        else if ( m_DXF2DView() == vsp::VIEW_2VER )
        {
            vector < vector < vec3d > > projectionvec1 = GetVehProjectionLines( m_DXF4View1(), to_orgin );

            if ( projectionvec1.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec1, m_DXF4View1(), m_DXF4View1_rot(), shiftvec );
                FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::UP, m_DXF4View1_rot(), 0 );
                string vehiclelayer_v1 = vehiclelayer + "_v1";

                WriteDXFPolylines2D( file_name, projectionvec1, vehiclelayer_v1, color, m_ColorCount );
                m_ColorCount++;
            }

            vector < vector < vec3d > > projectionvec3 = GetVehProjectionLines( m_DXF4View3(), to_orgin );

            if ( projectionvec3.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec3, m_DXF4View3(), m_DXF4View3_rot(), shiftvec );
                FeatureLinesShift( projectionvec3, shiftvec, VIEW_SHIFT::DOWN, m_DXF4View3_rot(), 0 );
                string vehiclelayer_v2 = vehiclelayer + "_v2";

                WriteDXFPolylines2D( file_name, projectionvec3, vehiclelayer_v2, color, m_ColorCount );
                m_ColorCount++;
            }
        }
        else if ( m_DXF2DView() == vsp::VIEW_4 )
        {
            vector < vector < vec3d > > projectionvec1 = GetVehProjectionLines( m_DXF4View1(), to_orgin );

            if ( projectionvec1.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec1, m_DXF4View1(), m_DXF4View1_rot(), shiftvec );
                FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::UP, m_DXF4View1_rot(), m_DXF4View2_rot() );
                FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::LEFT, m_DXF4View1_rot(), m_DXF4View3_rot() );
                string vehiclelayer_v1 = vehiclelayer + "_v1";

                WriteDXFPolylines2D( file_name, projectionvec1, vehiclelayer_v1, color, m_ColorCount );
                m_ColorCount++;
            }

            vector < vector < vec3d > > projectionvec2 = GetVehProjectionLines( m_DXF4View2(), to_orgin );

            if ( projectionvec2.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec2, m_DXF4View2(), m_DXF4View2_rot(), shiftvec );
                FeatureLinesShift( projectionvec2, shiftvec, VIEW_SHIFT::UP, m_DXF4View2_rot(), m_DXF4View1_rot() );
                FeatureLinesShift( projectionvec2, shiftvec, VIEW_SHIFT::RIGHT, m_DXF4View2_rot(), m_DXF4View4_rot() );
                string vehiclelayer_v2 = vehiclelayer + "_v2";

                WriteDXFPolylines2D( file_name, projectionvec2, vehiclelayer_v2, color, m_ColorCount );
                m_ColorCount++;
            }

            vector < vector < vec3d > > projectionvec3 = GetVehProjectionLines( m_DXF4View3(), to_orgin );

            if ( projectionvec3.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec3, m_DXF4View3(), m_DXF4View3_rot(), shiftvec );
                FeatureLinesShift( projectionvec3, shiftvec, VIEW_SHIFT::DOWN, m_DXF4View3_rot(), m_DXF4View4_rot() );
                FeatureLinesShift( projectionvec3, shiftvec, VIEW_SHIFT::LEFT, m_DXF4View3_rot(), m_DXF4View1_rot() );
                string vehiclelayer_v3 = vehiclelayer + "_v3";

                WriteDXFPolylines2D( file_name, projectionvec3, vehiclelayer_v3, color, m_ColorCount );
                m_ColorCount++;
            }

            vector < vector < vec3d > > projectionvec4 = GetVehProjectionLines( m_DXF4View4(), to_orgin );

            if ( projectionvec4.size() > 0 )
            {
                FeatureLinesManipulate( projectionvec4, m_DXF4View4(), m_DXF4View4_rot(), shiftvec );
                FeatureLinesShift( projectionvec4, shiftvec, VIEW_SHIFT::DOWN, m_DXF4View4_rot(), m_DXF4View3_rot() );
                FeatureLinesShift( projectionvec4, shiftvec, VIEW_SHIFT::RIGHT, m_DXF4View4_rot(), m_DXF4View2_rot() );
                string vehiclelayer_v4 = vehiclelayer + "_v4";

                WriteDXFPolylines2D( file_name, projectionvec4, vehiclelayer_v4, color, m_ColorCount );
                m_ColorCount++;
            }
        }
    }
}

void Vehicle::WriteVehProjectionLinesSVG( xmlNodePtr root, const BndBox &svgbox )
{
    // Bounding box diagonal, used to separate multi-view drawings
    vec3d shiftvec = svgbox.GetMax() - svgbox.GetMin();

    // Shift the vehicle bounding box to align with the +x, +y, +z axes at the orgin
    vec3d to_orgin = GetVecToOrgin( svgbox );

    if ( m_SVGView() == vsp::VIEW_1 )
    {
        vector < vector < vec3d > > projectionvec = GetVehProjectionLines( m_SVGView1(), to_orgin );
        FeatureLinesManipulate( projectionvec, m_SVGView1(), m_SVGView1_rot(), shiftvec );
        WriteSVGPolylines2D( root, projectionvec, svgbox );
    }
    else if ( m_SVGView() == vsp::VIEW_2HOR )
    {
        vector < vector < vec3d > > projectionvec1 = GetVehProjectionLines( m_SVGView1(), to_orgin );
        FeatureLinesManipulate( projectionvec1, m_SVGView1(), m_SVGView1_rot(), shiftvec );
        FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::LEFT,m_SVGView1_rot(), 0 );
        WriteSVGPolylines2D( root, projectionvec1, svgbox );

        vector < vector < vec3d > > projectionvec2 = GetVehProjectionLines( m_SVGView2(), to_orgin );
        FeatureLinesManipulate( projectionvec2, m_SVGView2(), m_SVGView2_rot(), shiftvec );
        FeatureLinesShift( projectionvec2, shiftvec, VIEW_SHIFT::RIGHT, m_SVGView2_rot(), 0 );
        WriteSVGPolylines2D( root, projectionvec2, svgbox );
    }
    else if ( m_SVGView() == vsp::VIEW_NUM::VIEW_2VER )
    {
        vector < vector < vec3d > > projectionvec1 = GetVehProjectionLines( m_SVGView1(), to_orgin );
        FeatureLinesManipulate( projectionvec1, m_SVGView1(), m_SVGView1_rot(), shiftvec );
        FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::UP, m_SVGView1_rot(), 0 );
        WriteSVGPolylines2D( root, projectionvec1, svgbox );

        vector < vector < vec3d > > projectionvec3 = GetVehProjectionLines( m_SVGView3(), to_orgin );
        FeatureLinesManipulate( projectionvec3, m_SVGView3(), m_SVGView3_rot(), shiftvec );
        FeatureLinesShift( projectionvec3, shiftvec, VIEW_SHIFT::DOWN, m_SVGView3_rot(), 0 );
        WriteSVGPolylines2D( root, projectionvec3, svgbox );
    }
    else if ( m_SVGView() == vsp::VIEW_NUM::VIEW_4 )
    {
        vector < vector < vec3d > > projectionvec1 = GetVehProjectionLines( m_SVGView1(), to_orgin );
        FeatureLinesManipulate( projectionvec1, m_SVGView1(), m_SVGView1_rot(), shiftvec );
        FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::UP, m_SVGView1_rot(), m_SVGView2_rot() );
        FeatureLinesShift( projectionvec1, shiftvec, VIEW_SHIFT::LEFT,m_SVGView1_rot(), m_SVGView3_rot() );
        WriteSVGPolylines2D( root, projectionvec1, svgbox );

        vector < vector < vec3d > > projectionvec2 = GetVehProjectionLines( m_SVGView2(), to_orgin );
        FeatureLinesManipulate( projectionvec2, m_SVGView2(), m_SVGView2_rot(), shiftvec );
        FeatureLinesShift( projectionvec2, shiftvec, VIEW_SHIFT::UP, m_SVGView2_rot(), m_SVGView1_rot() );
        FeatureLinesShift( projectionvec2, shiftvec, VIEW_SHIFT::RIGHT, m_SVGView2_rot(), m_SVGView4_rot() );
        WriteSVGPolylines2D( root, projectionvec2, svgbox );

        vector < vector < vec3d > > projectionvec3 = GetVehProjectionLines( m_SVGView3(), to_orgin );
        FeatureLinesManipulate( projectionvec3, m_SVGView3(), m_SVGView3_rot(), shiftvec );
        FeatureLinesShift( projectionvec3, shiftvec, VIEW_SHIFT::DOWN, m_SVGView3_rot(), m_SVGView4_rot() );
        FeatureLinesShift( projectionvec3, shiftvec, VIEW_SHIFT::LEFT, m_SVGView3_rot(), m_SVGView1_rot() );
        WriteSVGPolylines2D( root, projectionvec3, svgbox );

        vector < vector < vec3d > > projectionvec4 = GetVehProjectionLines( m_SVGView4(), to_orgin );
        FeatureLinesManipulate( projectionvec4, m_SVGView4(), m_SVGView4_rot(), shiftvec );
        FeatureLinesShift( projectionvec4, shiftvec, VIEW_SHIFT::DOWN, m_SVGView4_rot(), m_SVGView3_rot() );
        FeatureLinesShift( projectionvec4, shiftvec, VIEW_SHIFT::RIGHT, m_SVGView4_rot(), m_SVGView2_rot() );
        WriteSVGPolylines2D( root, projectionvec4, svgbox );
    }
}

vector< vector < vec3d > > Vehicle::GetVehProjectionLines( int view, vec3d offset )
{
    vector < vector < vec3d > > PathVec;

    if ( view == vsp::VIEW_LEFT || view == vsp::VIEW_RIGHT )
    {
        PathVec = m_VehProjectVec3d[vsp::Y_DIR];
    }
    else if ( view == vsp::VIEW_FRONT || view == vsp::VIEW_REAR )
    {
        PathVec = m_VehProjectVec3d[vsp::X_DIR];
    }
    else if ( view == vsp::VIEW_TOP || view == vsp::VIEW_BOTTOM )
    {
        PathVec = m_VehProjectVec3d[vsp::Z_DIR];
    }

    for ( int j = 0; j < PathVec.size(); j++ )
    {
        // Shift Projection Lines back near the orgin:
        for ( unsigned int k = 0; k < PathVec[j].size(); k++ )
        {
            PathVec[j][k].offset_x( -offset.x() );
            PathVec[j][k].offset_y( -offset.y() );
            PathVec[j][k].offset_z( -offset.z() );
        }
    }

    return PathVec;
}

void Vehicle::AddLinkableContainers( vector< string > & linkable_container_vec )
{
    ParmContainer::AddLinkableContainers( linkable_container_vec );

    //==== Add Geom Containers ====//
    vector< string > geom_id_vec = GetGeomVec();
    vector< Geom* > geom_vec = FindGeomVec( geom_id_vec );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        geom_vec[i]->AddLinkableContainers( linkable_container_vec );
    }

    m_ClippingMgr.AddLinkableContainers( linkable_container_vec );
    StructureMgr.AddLinkableContainers( linkable_container_vec );
}

void Vehicle::UpdateBBox()
{
    BndBox new_box;
    int ngeom;
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    ngeom = (int) geom_vec.size();
    for ( int i = 0 ; i < ngeom ; i++ )
    {
        new_box.Update( geom_vec[i]->GetBndBox() );
    }

    if( ngeom > 0 )
    {
        m_BbXLen = new_box.GetMax( 0 ) - new_box.GetMin( 0 );
        m_BbYLen = new_box.GetMax( 1 ) - new_box.GetMin( 1 );
        m_BbZLen = new_box.GetMax( 2 ) - new_box.GetMin( 2 );

        m_BbXMin = new_box.GetMin( 0 );
        m_BbYMin = new_box.GetMin( 1 );
        m_BbZMin = new_box.GetMin( 2 );
    }

    m_BBox = new_box;
}

bool Vehicle::GetVisibleBndBox( BndBox &b )
{
    b.Reset();

    int ngeom;
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    ngeom = (int) geom_vec.size();

    bool anyvisible = false;

    for ( int i = 0 ; i < ngeom ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( vsp::SET_SHOWN ) )
        {
            b.Update( geom_vec[i]->GetBndBox() );
            anyvisible = true;
        }
    }

    return anyvisible;
}

string Vehicle::getExportFileName( int type )
{
    bool doreturn = false;
    if ( type == COMP_GEOM_TXT_TYPE )
    {
        doreturn = true;
    }
    else if ( type == COMP_GEOM_CSV_TYPE )
    {
        doreturn = true;
    }
    else if ( type == MASS_PROP_TXT_TYPE )
    {
        doreturn = true;
    }
    else if ( type == SLICE_TXT_TYPE )
    {
        doreturn = true;
    }
    else if ( type == DEGEN_GEOM_CSV_TYPE )
    {
        doreturn = true;
    }
    else if ( type == DEGEN_GEOM_M_TYPE )
    {
        doreturn = true;
    }
    else if ( type == PROJ_AREA_CSV_TYPE )
    {
        doreturn = true;
    }
    else if ( type == WAVE_DRAG_TXT_TYPE )
    {
        doreturn = true;
    }
    else if ( type == VSPAERO_PANEL_TRI_TYPE )
    {
        doreturn = true;
    }
    else if ( type == DRAG_BUILD_CSV_TYPE )
    {
        doreturn = true;
    }
    else if ( type == VSPAERO_VSPGEOM_TYPE )
    {
        doreturn = true;
    }

    if( doreturn )
    {
        return m_ExportFileNames[ type ];
    }
    else
    {
        return string( "default_export.txt" );
    }
}

void Vehicle::setExportFileName( int type, string f_name )
{
    if ( f_name.compare( "" ) == 0 || f_name.compare( "/" ) == 0 )
    {
        return;
    }

    bool doset = false;

    if ( type == COMP_GEOM_TXT_TYPE )
    {
        doset = true;
    }
    else if ( type == COMP_GEOM_CSV_TYPE )
    {
        doset = true;
    }
    else if ( type == MASS_PROP_TXT_TYPE )
    {
        doset = true;
    }
    else if ( type == SLICE_TXT_TYPE )
    {
        doset = true;
    }
    else if ( type == DEGEN_GEOM_CSV_TYPE )
    {
        doset = true;
    }
    else if ( type == DEGEN_GEOM_M_TYPE )
    {
        doset = true;
    }
    else if ( type == PROJ_AREA_CSV_TYPE )
    {
        doset = true;
    }
    else if ( type == WAVE_DRAG_TXT_TYPE )
    {
        doset = true;
    }
    else if ( type == VSPAERO_PANEL_TRI_TYPE )
    {
        doset = true;
    }
    else if ( type == DRAG_BUILD_CSV_TYPE )
    {
        doset = true;
    }
    else if ( type == VSPAERO_VSPGEOM_TYPE )
    {
        doset = true;
    }

    if( doset )
    {
        m_ExportFileNames[ type ] = f_name;
    }
}

void Vehicle::resetExportFileNames()
{
    const char *suffix[] = {"_CompGeom.txt", "_CompGeom.csv", "_Slice.txt", "_MassProps.txt", "_DegenGeom.csv", "_DegenGeom.m", "_ProjArea.csv", "_WaveDrag.txt", ".tri", "_ParasiteBuildUp.csv", "_VSPGeom.vspgeom" };
    const int types[] = { COMP_GEOM_TXT_TYPE, COMP_GEOM_CSV_TYPE, SLICE_TXT_TYPE, MASS_PROP_TXT_TYPE, DEGEN_GEOM_CSV_TYPE, DEGEN_GEOM_M_TYPE, PROJ_AREA_CSV_TYPE, WAVE_DRAG_TXT_TYPE, VSPAERO_PANEL_TRI_TYPE, DRAG_BUILD_CSV_TYPE, VSPAERO_VSPGEOM_TYPE };
    const int ntype = ( sizeof(types) / sizeof(types[0]) );
    int pos;

    for( int i = 0; i < ntype; i++ )
    {
        string fname = m_VSP3FileName;
        pos = fname.find( ".vsp3" );
        if ( pos >= 0 )
        {
            fname.erase( pos, fname.length() - 1 );
        }
        fname.append( suffix[i] );
        m_ExportFileNames[types[i]] = fname;
    }
}

string Vehicle::CompGeom( int set, int degenset, int halfFlag, int intSubsFlag, bool hideset, bool suppressdisks )
{

    string id = AddMeshGeom( set, degenset, suppressdisks );
    if ( id.compare( "NONE" ) == 0 )
    {
        return id;
    }

    if ( hideset )
    {
        HideAllExcept( id );
    }

    MeshGeom* mesh_ptr = dynamic_cast<MeshGeom*> ( FindGeom( id ) );
    if ( mesh_ptr == NULL )
    {
        return id;
    }

    if ( halfFlag )
    {
        mesh_ptr->AddHalfBox( "NEGATIVE_HALF" );
    }

    if ( mesh_ptr->m_TMeshVec.size() )
    {
        vector< DegenGeom > dg;
        mesh_ptr->IntersectTrim( dg, false, intSubsFlag );
    }
    else
    {
        // Not sure if this is the best way to delete a mesh geom with no tMeshVec
        CutActiveGeomVec();
        DeleteClipBoard();
        id = "NONE";
    }

    if ( halfFlag )
    {
        mesh_ptr->GetMeshByID( "NEGATIVE_HALF" )->m_DeleteMeFlag = true;
        mesh_ptr->DeleteMarkedMeshes();
    }

    return id;
}

string Vehicle::CompGeomAndFlatten( int set, int halfFlag, int intSubsFlag, int degenset, bool hideset, bool suppressdisks )
{
    string id = CompGeom( set, degenset, halfFlag, intSubsFlag, hideset, suppressdisks );
    Geom* geom = FindGeom( id );
    if ( !geom )
    {
        return string( "NONE" );
    }
    MeshGeom* mesh_ptr = ( MeshGeom* )geom;
    mesh_ptr->FlattenTMeshVec();
    mesh_ptr->m_SurfDirty = true;
    mesh_ptr->Update();
    return id;
}

string Vehicle::MassProps( int set, int numSlices, bool hidegeom, bool writefile )
{
    string id = AddMeshGeom( set );
    if ( id.compare( "NONE" ) == 0 )
    {
        return id;
    }

    MeshGeom* mesh_ptr = ( MeshGeom* )FindGeom( id );
    if ( mesh_ptr == NULL )
    {
        return id;
    }

    //==== Load Point Mass Properties From Blank Geom ====//
    vector<string> geom_vec = GetGeomVec();
 
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i].compare( id ) != 0 )
        {
  
            Geom* geom_ptr = FindGeom( geom_vec[i] );
            if ( geom_ptr )
            {
                if ( geom_ptr->GetSetFlag( set ) && geom_ptr->GetType().m_Type == BLANK_GEOM_TYPE )
                {
                    BlankGeom* BGeom = ( BlankGeom* ) geom_ptr;

                    if ( BGeom->m_PointMassFlag() )
                    {
                        TetraMassProp* pm = new TetraMassProp(); // Deleted by mesh_ptr
                        pm->SetPointMass( BGeom->m_PointMass(), BGeom->getModelMatrix().getTranslation() );
                        pm->m_CompId = BGeom->GetID();
                        mesh_ptr->AddPointMass( pm );
                        
                    }
                }
            }
        }
    }

    if( hidegeom )
    {
        HideAllExcept( id );
    }

    if ( mesh_ptr->m_TMeshVec.size() || mesh_ptr->m_PointMassVec.size() )
    {
        mesh_ptr->MassSliceX( numSlices, writefile );
        m_TotalMass = mesh_ptr->m_TotalMass;
        m_IxxIyyIzz = vec3d( mesh_ptr->m_TotalIxx, mesh_ptr->m_TotalIyy, mesh_ptr->m_TotalIzz );
        m_IxyIxzIyz = vec3d( mesh_ptr->m_TotalIxy, mesh_ptr->m_TotalIxz, mesh_ptr->m_TotalIyz );
        m_CG = mesh_ptr->m_CenterOfGrav;

    }
    else
    {
        CutActiveGeomVec();
        DeleteClipBoard();
        id = "NONE";
    }

    return id;
}

string Vehicle::MassPropsAndFlatten( int set, int numSlices, bool hidegeom, bool writefile )
{
    string id = MassProps( set, numSlices, hidegeom, writefile );
    Geom* geom = FindGeom( id );
    if ( !geom )
    {
        return string( "NONE" );
    }
    MeshGeom* mesh_ptr = ( MeshGeom* )geom;
    mesh_ptr->FlattenTMeshVec();
    mesh_ptr->FlattenSliceVec();
    mesh_ptr->m_SurfDirty = true;
    mesh_ptr->Update();
    return id;
}

string Vehicle::PSlice( int set, int numSlices, vec3d axis, bool autoBoundsFlag, double start, double end )
{

    string id = AddMeshGeom( set );
    if ( id.compare( "NONE" ) == 0 )
    {
        return id;
    }

    HideAllExcept( id );

    MeshGeom* mesh_ptr = ( MeshGeom* )FindGeom( id );
    if ( mesh_ptr == NULL )
    {
        return id;
    }

    if ( mesh_ptr->m_TMeshVec.size() )
    {
        mesh_ptr->AreaSlice( numSlices, axis, autoBoundsFlag, start, end );
    }
    else
    {
        CutActiveGeomVec();
        DeleteClipBoard();
        id = "NONE";
    }

    return id;
}

string Vehicle::PSliceAndFlatten( int set, int numSlices, vec3d axis, bool autoBoundsFlag, double start, double end )
{
    string id = PSlice( set, numSlices, axis, autoBoundsFlag, start, end );
    Geom* geom = FindGeom( id );
    if ( !geom )
    {
        return string( "NONE" );
    }
    MeshGeom* mesh_ptr = ( MeshGeom* )geom;
    mesh_ptr->FlattenTMeshVec();
    mesh_ptr->FlattenSliceVec();
    mesh_ptr->m_SurfDirty = true;
    mesh_ptr->Update();
    return id;
}

//==== Import File Methods ====//
string Vehicle::ImportFile( const string & file_name, int file_type )
{
    string id;

    if ( file_type == IMPORT_PTS )
    {
        GeomType type = GeomType( PT_CLOUD_GEOM_TYPE, "PTS", true );
        id = AddGeom( type );
        if ( !id.compare( "NONE" ) )
        {
            return id;
        }

        PtCloudGeom* new_geom = ( PtCloudGeom* )FindGeom( id );
        if ( new_geom )
        {
            int validFlag = new_geom->ReadPTS( file_name.c_str() );

            if ( !validFlag )
            {
                DeleteGeom( id );
                id = "NONE";
            }
            else
            {
                SetActiveGeom( id );
                new_geom->SetDirtyFlag( GeomBase::SURF );
                new_geom->Update();
            }
        }
    }
    else if ( file_type == IMPORT_V2 )
    {
        return ImportV2File( file_name );
    }
    else if ( file_type == IMPORT_BEM )
    {
        GeomType type = GeomType( PROP_GEOM_TYPE, "PROP", true );
        id = AddGeom( type );
        if ( !id.compare( "NONE" ) )
        {
            return id;
        }

        Geom* new_geom = FindGeom( id );
        if ( new_geom )
        {
            PropGeom* prop = dynamic_cast < PropGeom* > (new_geom );
            if ( prop )
            {
                int validFlag = prop->ReadBEM( file_name.c_str() );

                if ( !validFlag )
                {
                    DeleteGeom( id );
                    id = "NONE";
                }
                else
                {
                    SetActiveGeom( id );
                    prop->SetDirtyFlag( GeomBase::SURF );
                    prop->Update();
                }
            }
        }
    }
    else if ( file_type == IMPORT_XSEC_WIRE )
    {
        FILE *fp;
        char str[256] = {};

        //==== Make Sure File Exists ====//
        if ( ( fp = fopen( file_name.c_str(), "r" ) ) == ( FILE * )NULL )
        {
            return id;
        }

        //==== Read first Line of file and compare against expected header ====//
        fscanf( fp, "%s INPUT FILE\n\n", str );
        if ( strcmp( "HERMITE", str ) != 0 )
        {
            fclose ( fp );
            return id;
        }
        //==== Read in number of components ====//
        int num_comps;
        fscanf( fp, " NUMBER OF COMPONENTS = %d\n", &num_comps );

        if ( num_comps <= 0 )
        {
            fclose ( fp );
            return id;
        }

        // Make sure blank gets added to top level.
        // Consider removing this to make blank added as child of active.
        ClearActiveGeom();

        GeomType type = GeomType( BLANK_GEOM_TYPE, "BLANK", true );
        id = AddGeom( type );
        if ( !id.compare( "NONE" ) )
        {
            fclose( fp );
            return id;
        }

        // Make blank active so components will be children of it.
        SetActiveGeom( id );

        for ( int c = 0 ; c < num_comps ; c++ )
        {
            type = GeomType( WIRE_FRAME_GEOM_TYPE, "WIREFRAME", true );
            string cid = AddGeom( type );
            if ( !cid.compare( "NONE" ) )
            {
                return id;
            }

            WireGeom* new_geom = ( WireGeom* )FindGeom( cid );
            if ( new_geom )
            {
                new_geom->ReadXSec( fp );
                new_geom->SetDirtyFlag( GeomBase::SURF );
            }
        }
        fclose( fp );

        return id;
    }
    else if ( file_type == IMPORT_P3D_WIRE )
    {
        FILE *fp;

        //==== Make Sure File Exists ====//
        if ( ( fp = fopen( file_name.c_str(), "r" ) ) == ( FILE * )NULL )
        {
            return id;
        }

        //==== Read in number of blockks ====//
        int num_comps;
        fscanf( fp, "%d\n", &num_comps );

        if ( num_comps <= 0 )
        {
            fclose ( fp );
            return id;
        }

        vector <int> ni( num_comps, 0 );
        vector <int> nj( num_comps, 0 );
        vector <int> nk( num_comps, 0 );

        for ( int c = 0 ; c < num_comps ; c++ )
        {
            fscanf( fp, "%d %d %d\n", &ni[c], &nj[c], &nk[c] );
        }

        // Make sure blank gets added to top level.
        // Consider removing this to make blank added as child of active.
        ClearActiveGeom();

        GeomType type = GeomType( BLANK_GEOM_TYPE, "BLANK", true );
        id = AddGeom( type );
        if ( !id.compare( "NONE" ) )
        {
            fclose( fp );
            return id;
        }

        // Make blank active so components will be children of it.
        SetActiveGeom( id );

        for ( int c = 0 ; c < num_comps ; c++ )
        {
            type = GeomType( WIRE_FRAME_GEOM_TYPE, "WIREFRAME", true );
            string cid = AddGeom( type );
            if ( !cid.compare( "NONE" ) )
            {
                return id;
            }

            WireGeom* new_geom = ( WireGeom* )FindGeom( cid );
            if ( new_geom )
            {
                new_geom->ReadP3D( fp, ni[c], nj[c], nk[c] );
                new_geom->SetDirtyFlag( GeomBase::SURF );
            }
        }
        fclose( fp );

        return id;
    }
    else
    {
        GeomType type = GeomType( MESH_GEOM_TYPE, "MESH", true );
        id = AddGeom( type );
        if ( !id.compare( "NONE" ) )
        {
            return id;
        }

        MeshGeom* new_geom = ( MeshGeom* )FindGeom( id );
        if ( new_geom )
        {
            int validFlag;
            if ( file_type == IMPORT_STL )
            {
                validFlag = new_geom->ReadSTL( file_name.c_str() );
            }
            else if ( file_type == IMPORT_NASCART )
            {
                validFlag = new_geom->ReadNascart( file_name.c_str() );
            }
            else if ( file_type == IMPORT_CART3D_TRI )
            {
                validFlag = new_geom->ReadTriFile( file_name.c_str() );
            }
            else if ( file_type == IMPORT_XSEC_MESH )
            {
                validFlag = new_geom->ReadXSec( file_name.c_str() );
            }
            else
            {
                validFlag = 0;
            }

            if ( !validFlag )
            {
                DeleteGeom( id );
                id = "NONE";
            }
            else
            {
                SetActiveGeom( id );
                new_geom->SetDirtyFlag( GeomBase::SURF ); // m_TMeshVec has been updated
                new_geom->Update();
            }
        }
    }
    return id;
}

string Vehicle::ImportV2File( const string & file_name )
{
    string lastreset = ParmMgr.ResetRemapID();

    //==== Read Xml File ====//
    xmlDocPtr doc;
    xmlNodePtr node;

    LIBXML_TEST_VERSION
    xmlKeepBlanksDefault( 0 );

    //==== Build an XML tree from a the file ====//
    doc = xmlParseFile( file_name.c_str() );
    if ( doc == NULL ) return 0;

    xmlNodePtr root = xmlDocGetRootElement( doc );
    if ( root == NULL )
    {
        fprintf( stderr, "Empty document\n" );
        xmlFreeDoc( doc );

        return string();
    }

    if ( xmlStrcmp( root->name, (const xmlChar *)"Vsp_Geometry" ) &&
         xmlStrcmp( root->name, (const xmlChar *)"Ram_Geometry" ) )
    {
        fprintf( stderr, "Document of the wrong type, OpenVSP v2 Geometry not found\n" );
        xmlFreeDoc( doc );

        return string();
    }

    //==== Find Version Number ====//
    int version = XmlUtil::FindInt( root, "Version", 0 );

    //==== Find Name ====//
    string nameStr = XmlUtil::FindString( root, "Name", string() );

    vector< string > add_geoms;

    //==== Read Components ====//
    node = XmlUtil::GetNode( root, "Component_List", 0 );
    if ( node  )
    {

        xmlNodePtr comp_list_node = node;
        int num_comps = XmlUtil::GetNumNames( comp_list_node, "Component" );

        for ( int i = 0 ; i < num_comps ; i++ )
        {
            xmlNodePtr comp_node = XmlUtil::GetNode( comp_list_node, "Component", i );

            node = XmlUtil::GetNode( comp_node, "Type", 0 );

            if ( node )
            {
                string typeStr = XmlUtil::ExtractString( node );

                string id;

                if ( typeStr == "Pod" )
                {
                    id = CreateGeom( GeomType( POD_GEOM_TYPE, "Pod", true ) );
                }
                else if ( typeStr == "External" )
                {
                    printf("Found External component.  Not yet supported.\n");
                }
                else if ( typeStr == "Havoc")
                {
                    printf("Found Havoc component.  Not yet supported.\n");
                }
                else if ( typeStr == "Fuselage" )
                {
                    id = CreateGeom( GeomType( FUSELAGE_GEOM_TYPE, "Fuselage", true ) );
                }
                else if ( typeStr == "Fuselage2" )
                {
                    id = CreateGeom( GeomType( FUSELAGE_GEOM_TYPE, "Fuselage", true ) );
                }
                else if ( typeStr == "Mwing" )
                {
                    printf("Found Mwing component.  Open file in OpenVSP v2 and save to convert to MS_Wing.\n");
                }
                else if ( typeStr == "Mswing" )
                {
                    id = CreateGeom( GeomType( MS_WING_GEOM_TYPE, "Wing", true ) );
                }
                else if ( typeStr == "Hwb" )
                {
                    id = CreateGeom( GeomType( MS_WING_GEOM_TYPE, "Wing", true ) );
                    printf("Found Hwb component.  Not yet supported, importing as Wing.\n");
                }
                else if ( typeStr == "Blank" )
                {
                    id = CreateGeom( GeomType( BLANK_GEOM_TYPE, "Blank", true ) );
                }
                else if ( typeStr == "Duct" )
                {
                    printf("Found Duct component.  Not yet supported.\n");
                }
                else if ( typeStr == "Prop" )
                {
                    printf("Found Prop component.  Not yet supported.\n");
                }
                else if ( typeStr == "Engine" )
                {
                    printf("Found Engine component.  Not yet supported.\n");
                }
                else if ( typeStr == "Mesh" )
                {
                    printf("Found Mesh component.  Not yet supported.\n");
                }
                else if ( typeStr == "Cabin_Layout" )
                {
                    printf("Found Cabin_Layout component.  Not yet supported.\n");
                }
                else if ( typeStr == "User" )
                {
                    // Since v2 parameter links can't be transferred to v3, no point in
                    // importing user parameter values.
                }
                else if ( typeStr == "XSecGeom" )
                {
                    printf("Found XSecGeom component.  Not yet supported.\n");
                }

                // Common code to import and insert into tree.
                Geom* geom = FindGeom( id );
                if ( geom )
                {
                    add_geoms.push_back( id );
                    geom->ReadV2File( comp_node );
                    geom->SetDirtyFlag( GeomBase::SURF );

                    if ( geom->GetParentID().compare( "NONE" ) == 0 )
                    {
                        AddGeom( geom );
                    }
                }

            }
        }
    }

    m_CfdSettings.ReadV2File( root );
    m_CfdGridDensity.ReadV2File( root );

    //===== Free Doc =====//
    xmlFreeDoc( doc );

    ParmMgr.ResetRemapID( lastreset );

    // The import routine has set the appropriate coordinate system values and
    // rel/abs flags. Therefore, the ignore absolute coordinate flag should
    // not be applied
    SetApplyAbsIgnoreFlag( add_geoms, false );

    // Update all of the geoms
    Update();

    // Turn the apply abs ignore flag back on, so attachments work properly
    // when parent geoms are moved
    SetApplyAbsIgnoreFlag( add_geoms, true );

    return string();
}

void Vehicle::SetApplyAbsIgnoreFlag( const vector< string > &g_vec, bool val )
{
    for ( unsigned int i = 0; i < g_vec.size() ; i++ )
    {
        Geom* g = FindGeom( g_vec[i] );
        if ( g )
        {
            g->SetApplyAbsIgnoreFlag( val );
        }
    }
}

//==== Import File Methods ====//
string Vehicle::ExportFile( const string & file_name, int write_set, int degen_set, int file_type )
{
    string mesh_id = string();

    if ( file_type == EXPORT_XSEC )
    {
        WriteXSecFile( file_name, write_set );
    }
    else if ( file_type == EXPORT_PLOT3D )
    {
        WritePLOT3DFile( file_name, write_set  );
    }
    else if ( file_type == EXPORT_STL )
    {
        if ( m_STLExportPropMainSurf() )
        {
            SetExportPropMainSurf( true );
        }

        if ( !m_STLMultiSolid() )
        {
            mesh_id = WriteSTLFile( file_name, write_set );
        }
        else
        {
            mesh_id = WriteTaggedMSSTLFile( file_name, write_set );
        }

        if ( m_STLExportPropMainSurf() )
        {
            SetExportPropMainSurf( false );
        }
    }
    else if ( file_type == EXPORT_CART3D )
    {
        mesh_id = WriteTRIFile( file_name, write_set );
    }
    else if ( file_type == EXPORT_OBJ )
    {
        mesh_id = WriteOBJFile( file_name, write_set );
    }
    else if ( file_type == EXPORT_VSPGEOM )
    {
        mesh_id = WriteVSPGeomFile( file_name, write_set, degen_set );
    }
    else if ( file_type == EXPORT_NASCART )
    {
        mesh_id = WriteNascartFiles( file_name, write_set );
    }
    else if ( file_type == EXPORT_GMSH )
    {
        mesh_id = WriteGmshFile( file_name, write_set );
    }
    else if ( file_type == EXPORT_POVRAY )
    {
        WritePovRayFile( file_name, write_set );
    }
    else if ( file_type == EXPORT_X3D )
    {
        WriteX3DFile( file_name, write_set );
    }
    else if ( file_type == EXPORT_STEP )
    {
        if ( m_STEPExportPropMainSurf() )
        {
            SetExportPropMainSurf( true );
        }

        WriteSTEPFile( file_name, write_set );

        if ( m_STEPExportPropMainSurf() )
        {
            SetExportPropMainSurf( false );
        }
    }
    else if ( file_type == EXPORT_STEP_STRUCTURE )
    {
        WriteStructureSTEPFile( file_name );
    }
    else if ( file_type == EXPORT_IGES )
    {
        if ( m_IGESExportPropMainSurf() )
        {
            SetExportPropMainSurf( true );
        }

        WriteIGESFile( file_name, write_set );

        if ( m_IGESExportPropMainSurf() )
        {
            SetExportPropMainSurf( false );
        }
    }
    else if ( file_type == EXPORT_IGES_STRUCTURE )
    {
        WriteStructureIGESFile( file_name );
    }
    else if ( file_type == EXPORT_BEM )
    {
        WriteBEMFile( file_name, write_set );
    }
    else if ( file_type == EXPORT_DXF )
    {
        WriteDXFFile( file_name, write_set  );
    }
    else if ( file_type == EXPORT_SVG )
    {
        WriteSVGFile( file_name, write_set  );
    }
    else if ( file_type == EXPORT_FACET )
    {
        mesh_id = WriteFacetFile(file_name, write_set);
    }
    else if ( file_type == EXPORT_PMARC )
    {
        WritePMARCFile(file_name, write_set);
    }
    else if ( file_type == EXPORT_SELIG_AIRFOIL )
    {
        m_AFExportType.Set( vsp::SELIG_AF_EXPORT );
        WriteAirfoilFile( file_name, write_set );
    }
    else if ( file_type == EXPORT_BEZIER_AIRFOIL )
    {
        m_AFExportType.Set( vsp::BEZIER_AF_EXPORT );
        WriteAirfoilFile( file_name, write_set );
    }

    return mesh_id;
}

void Vehicle::CreateDegenGeom( int set )
{
    vector< string > geom_id_vec;
    m_DegenGeomVec.clear();
    m_DegenPtMassVec.clear();

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( set ) )
        {
            if( geom_vec[i]->GetType().m_Type == BLANK_GEOM_TYPE )
            {
                BlankGeom *g = (BlankGeom*) geom_vec[i];
                if( g->m_PointMassFlag() )
                {
                    DegenPtMass pm;
                    pm.name = g->GetName();
                    pm.mass = g->m_PointMass();
                    pm.x = g->getModelMatrix().getTranslation();
                    pm.geom_id = g->GetID();
                    m_DegenPtMassVec.push_back( pm );
                }
            }
            else
            {
                geom_vec[i]->CreateDegenGeom( m_DegenGeomVec );
            }
        }
    }

    vector< string > active_vec_store = GetActiveGeomVec();

    string id = AddMeshGeom( set );
    if ( id.compare( "NONE" ) != 0 )
    {
        MeshGeom* mesh_ptr = dynamic_cast<MeshGeom*> ( FindGeom( id ) );
        if ( mesh_ptr != NULL )
        {
            mesh_ptr->IntersectTrim( m_DegenGeomVec, true, 0 );
            DeleteGeom( id );
        }
    }


    id = AddMeshGeom( set );
    if ( id.compare( "NONE" ) != 0 )
    {
        MeshGeom* mesh_ptr = dynamic_cast<MeshGeom*> ( FindGeom( id ) );
        if ( mesh_ptr != NULL )
        {
            mesh_ptr->degenGeomMassSliceX(m_DegenGeomVec);
            DeleteGeom( id );
        }
    }

    SetActiveGeomVec( active_vec_store );
}

//==== Write Degen Geom File ====//
string Vehicle::WriteDegenGeomFile()
{
    int geomCnt = 0, blankCnt = 0;
    string outStr = "\n";

    vector<BlankGeom*> blankGeom;

    geomCnt = m_DegenGeomVec.size();

    blankCnt = m_DegenPtMassVec.size();

    char geomCntStr[255];
    sprintf(geomCntStr,"%d components and %d", geomCnt, blankCnt);
    outStr += "Wrote ";
    outStr += geomCntStr;
    outStr += " blank geoms\nto the following files:\n\n";

    if ( getExportDegenGeomCsvFile() )
    {
        string file_name = getExportFileName( DEGEN_GEOM_CSV_TYPE );
        FILE* file_id = fopen(file_name.c_str(), "w");

        if ( !file_id ) // Check if the file was successfully opened
        {
            outStr += "\tFAILED TO OPEN: ";
            outStr += file_name;
            outStr += "\n";
        }
        else
        {
            fprintf(file_id, "# DEGENERATE GEOMETRY CSV FILE\n\n");
            fprintf(file_id, "# NUMBER OF COMPONENTS\n%d\n", geomCnt);

            if ( m_DegenPtMassVec.size() > 0 )
            {
                fprintf(file_id, "BLANK_GEOMS,%d\n", blankCnt);
                fprintf(file_id, "# Name, xLoc, yLoc, zLoc, Mass, GeomID");

                for ( int i = 0; i < (int)m_DegenPtMassVec.size(); i++ )
                {
                    // Blank geom translated location
                    fprintf(file_id, "\n%s,%f,%f,%f,%f,%s", m_DegenPtMassVec[i].name.c_str(), \
                                                         m_DegenPtMassVec[i].x.v[0], \
                                                         m_DegenPtMassVec[i].x.v[1], \
                                                         m_DegenPtMassVec[i].x.v[2], \
                                                         m_DegenPtMassVec[i].mass, \
                                                         m_DegenPtMassVec[i].geom_id.c_str() );
                }
            }

            for ( int i = 0; i < (int)m_DegenGeomVec.size(); i++ )
            {
                m_DegenGeomVec[i].write_degenGeomCsv_file( file_id );
            }

            fclose(file_id);

            outStr += "\t";
            outStr += file_name;
            outStr += "\n";
        }
    }

    if ( getExportDegenGeomMFile() )
    {
        string file_name = getExportFileName( DEGEN_GEOM_M_TYPE );
        FILE* file_id = fopen(file_name.c_str(), "w");
        if ( !file_id )
        {
            outStr += "\tFAILED TO OPEN: ";
            outStr += file_name;
            outStr += "\n";
        }
        else
        {
            fprintf( file_id, "%%-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-%%\n" );
            fprintf( file_id, "%%-=-=-=-=-=-= DEGENERATE GEOMETRY M FILE =-=-=-=-=-=-=%%\n" );
            fprintf( file_id, "%%-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-%%\n\n" );

            if ( blankCnt > 0)
            {
                fprintf( file_id, "blankGeom = [];" );

                for ( int i = 0; i < (int)m_DegenPtMassVec.size(); i++ )
                {
                    fprintf( file_id, "\nblankGeom(end+1).name = '%s';", \
                                     m_DegenPtMassVec[i].name.c_str() );
                    fprintf( file_id, "\nblankGeom(end).geom_id = '%s';", m_DegenPtMassVec[i].geom_id.c_str() );

                    fprintf( file_id, "\nblankGeom(end).X = [%f, %f, %f];", m_DegenPtMassVec[i].x.v[0],\
                                                                            m_DegenPtMassVec[i].x.v[1],\
                                                                            m_DegenPtMassVec[i].x.v[2] );
                    fprintf( file_id, "\nblankGeom(end).mass = %f;", m_DegenPtMassVec[i].mass );
                }
                fprintf( file_id, "\n\n" );
            }

            fprintf(file_id, "degenGeom = [];");

            for ( int i = 0, propIdx = 1; i < (int)m_DegenGeomVec.size(); i++, propIdx++ )
            {
                m_DegenGeomVec[i].write_degenGeomM_file(file_id);
            }

            fclose(file_id);

            outStr += "\t";
            outStr += file_name;
            outStr += "\n";
        }
    }

    // Create results object to contain the ids of all of the results associated
    // with degen geoms
    Results *res = ResultsMgr.CreateResults( "DegenGeom" );
    vector < string > degen_results_ids;
    vector < string > blank_degen_result_ids;

    if ( blankCnt > 0 )
    {
        for ( int i = 0; i < ( int ) m_DegenPtMassVec.size(); i++ )
        {
            Results *blnk_res = ResultsMgr.CreateResults( "Degen_BlankGeom" );
            blank_degen_result_ids.push_back( blnk_res->GetID() );

            blnk_res->Add( NameValData( "name", m_DegenPtMassVec[i].name ) );
            blnk_res->Add( NameValData( "geom_id", m_DegenPtMassVec[i].geom_id ) );
            blnk_res->Add( NameValData( "X", m_DegenPtMassVec[i].x ) );
            blnk_res->Add( NameValData( "mass", m_DegenPtMassVec[i].mass ) );
        }
    }

    for ( int i = 0; i < ( int )m_DegenGeomVec.size(); i++ )
    {
        m_DegenGeomVec[i].write_degenGeomResultsManager( degen_results_ids );
    }

    res->Add( NameValData( "Degen_BlankGeoms", blank_degen_result_ids ) );
    res->Add( NameValData( "Degen_DegenGeoms", degen_results_ids ) );
    return outStr;
}

vec3d Vehicle::CompPnt01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w)
{
    Geom* geom_ptr = FindGeom( geom_id );
    vec3d ret;
    if ( geom_ptr )
    {
        if ( surf_indx >= 0 && surf_indx < geom_ptr->GetNumTotalSurfs() )
        {
            ret = geom_ptr->CompPnt01(surf_indx, u, w);
        }
    }

    return ret;
}

vec3d Vehicle::CompNorm01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w)
{
    Geom* geom_ptr = FindGeom( geom_id );
    vec3d ret;
    if ( geom_ptr )
    {
        if ( surf_indx >= 0 && surf_indx < geom_ptr->GetNumTotalSurfs() )
        {
            VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );
            ret = surf->CompNorm01( u, w );
        }
    }

    return ret;
}

void Vehicle::CompCurvature01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w, double &k1, double &k2, double &ka, double &kg)
{
    Geom* geom_ptr = FindGeom( geom_id );

    k1 = 0.0;
    k2 = 0.0;
    ka = 0.0;
    kg = 0.0;

    if ( geom_ptr )
    {
        if ( surf_indx >= 0 && surf_indx < geom_ptr->GetNumTotalSurfs() )
        {
            VspSurf *surf = geom_ptr->GetSurfPtr( surf_indx );
            if ( surf )
            {
                surf->CompCurvature01( u, w, k1, k2, ka, kg );
            }
        }
    }
}

double Vehicle::ProjPnt01I(const std::string &geom_id, const vec3d & pt, int &surf_indx, double &u, double &w)
{
    double tol = 1e-12;

    double dmin = std::numeric_limits<double>::max();

    Geom * geom = FindGeom( geom_id );

    if ( geom )
    {
        int nsurf = geom->GetNumTotalSurfs();
        for ( int i = 0; i < nsurf; i++ )
        {
            double utest, wtest;

            double d = geom->GetSurfPtr(i)->FindNearest01( utest, wtest, pt );

            if ( d < dmin )
            {
                dmin = d;
                u = utest;
                w = wtest;
                surf_indx = i;

                if ( d < tol )
                {
                    break;
                }
            }
        }
    }
    return dmin;
}

double Vehicle::AxisProjPnt01I(const std::string &geom_id, const int &iaxis, const vec3d &pt, int &surf_indx_out, double &u_out, double &w_out, vec3d &p_out )
{
    double idmin = std::numeric_limits<double>::max();

    bool converged = false;

    Geom * geom = FindGeom( geom_id );

    if ( geom )
    {
        int nsurf = geom->GetNumTotalSurfs();
        for ( int i = 0; i < nsurf; i++ )
        {
            double utest, wtest;
            vec3d ptest;

            double id = geom->GetSurfPtr( i )->ProjectPt01( pt, iaxis, utest, wtest, ptest );

            if ( id >= 0 && id < idmin )
            {
                idmin = id;
                u_out = utest;
                w_out = wtest;
                p_out = ptest;
                surf_indx_out = i;
                converged = true;
            }
        }
    }

    if ( converged )
    {
        return idmin;
    }

    u_out = -1;
    w_out = -1;
    p_out = pt;
    surf_indx_out = -1;
    idmin = -1;

    return idmin;
}

vec3d Vehicle::CompPntRST( const std::string &geom_id, const int &surf_indx, const double &r, const double &s, const double &t )
{
    Geom* geom_ptr = FindGeom( geom_id );
    vec3d ret;
    if ( geom_ptr )
    {
        if ( surf_indx >= 0 && surf_indx < geom_ptr->GetNumTotalSurfs() )
        {
            ret = geom_ptr->CompPntRST( surf_indx, r, s, t );
        }
    }

    return ret;
}

// Method to add pnts and normals to results managers for all surfaces
// in the selected set
string Vehicle::ExportSurfacePatches( int set )
{
    vector< string > geom_vec = GetGeomVec();

    Results* veh_surfaces = ResultsMgr.CreateResults( "VehicleSurfaces" );
    vector< string > components;

    for ( int i = 0; i < (int)geom_vec.size(); i++ )
    {
        Geom* geom = FindGeom( geom_vec[i] );

        if ( geom )
        {
            if ( geom->GetSetFlag( set ) )
            {
                // Loop over all surfaces adding points to the results manager
                Results* res = ResultsMgr.CreateResults( "ComponentSurfaces" );
                res->Add( NameValData( "name", geom->GetName() ) );
                res->Add( NameValData( "id", geom->GetID() ) );

                vector< string > surfaces;
                geom->ExportSurfacePatches( surfaces  );

                res->Add( NameValData( "surfaces", surfaces) );

                components.push_back( res->GetID() );
            }
        }
    }

    veh_surfaces->Add( NameValData( "components", components ) );
    return veh_surfaces->GetID();
}

void Vehicle::SetExportPropMainSurf( bool b )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0; i < (int) geom_vec.size(); i++ )
    {
        PropGeom *pg = dynamic_cast< PropGeom * > ( geom_vec[i] );
        if ( pg )
        {
            pg->SetExportMainSurf( b );
        }
    }
}
