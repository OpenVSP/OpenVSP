//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

#include <filesystem>

#include "Vehicle.h"

#include "AdvLinkMgr.h"
#include "AeroStructMgr.h"
#include "AnalysisMgr.h"
#include "AttributeManager.h"
#include "AuxiliaryGeom.h"
#include "Background3DMgr.h"
#include "BlankGeom.h"
#include "BORGeom.h"
#include "ConformalGeom.h"
#include "CustomGeom.h"
#include "DesignVarMgr.h"
#include "GearGeom.h"
#include "DXFUtil.h"
#include "EllipsoidGeom.h"
#include "FileUtil.h"
#include "FitModelMgr.h"
#include "FuselageGeom.h"
#include "HingeGeom.h"
#include "HumanGeom.h"
#include "GeometryAnalysisMgr.h"
#include "LinkMgr.h"
#include "MeasureMgr.h"
#include "MeshGeom.h"
#include "ModeMgr.h"
#include "NGonMeshGeom.h"
#include "ParasiteDragMgr.h"
#include "ParmMgr.h"
#include "PodGeom.h"
#include "ProjectionMgr.h"
#include "PropGeom.h"
#include "PtCloudGeom.h"
#include "Quat.h"
#include "RoutingGeom.h"
#include "ScriptMgr.h"
#include "StlHelper.h"
#include "StringUtil.h"
#include "StructureMgr.h"
#include "SubSurfaceMgr.h"
#include "SVGUtil.h"
#include "VarPresetMgr.h"
#include "VSPAEROMgr.h"
#include "WingGeom.h"
#include "WireGeom.h"

using namespace vsp;

//==== Constructor ====//
Vehicle::Vehicle()
{
    m_Name = "Vehicle_Constructor";

    SetParmContainerType( vsp::ATTROBJ_VEH );
    AttachAttrCollection();

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
    m_STEPMergeLETE.Init( "MergeLETE", "STEPSettings", this, false, 0, 1 );
    m_STEPExportPropMainSurf.Init( "ExportPropMainSurf", "STEPSettings", this, false, 0, 1 );

    m_STEPLabelID.Init( "LabelID", "STEPSettings", this, true, 0, 1 );
    m_STEPLabelName.Init( "LabelName", "STEPSettings", this, true, 0, 1 );
    m_STEPLabelSurfNo.Init( "LabelSurfNo", "STEPSettings", this, true, 0, 1 );
    m_STEPLabelSplitNo.Init( "LabelSplitNo", "STEPSettings", this, true, 0, 1 );
    m_STEPLabelAirfoilPart.Init( "LabelAirfoilPart", "STEPSettings", this, true, 0, 1 );
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
    m_STEPStructureLabelSplitNo.Init( "StructureLabelSplitNo", "STEPSettings", this, true, 0, 1 );
    m_STEPStructureLabelAirfoilPart.Init( "StructureLabelAirfoilPart", "STEPSettings", this, true, 0, 1 );
    m_STEPStructureLabelDelim.Init( "StructureLabelDelim", "STEPSettings", this, vsp::DELIM_COMMA, vsp::DELIM_COMMA, vsp::DELIM_NUM_TYPES - 1 );

    m_IGESLenUnit.Init( "LenUnit", "IGESSettings", this, vsp::LEN_FT, vsp::LEN_MM, vsp::LEN_FT );
    m_IGESSplitSurfs.Init( "SplitSurfs", "IGESSettings", this, true, 0, 1 );
    m_IGESSplitSubSurfs.Init( "SplitSubSurfs", "IGESSettings", this, false, 0, 1 );
    m_IGESToCubic.Init( "ToCubic", "IGESSettings", this, false, 0, 1 );
    m_IGESToCubic.SetDescript( "Flag to Demote Higher Order Surfaces to Cubic in IGES Export" );
    m_IGESToCubicTol.Init( "ToCubicTol", "IGESSettings", this, 1e-6, 1e-12, 1e12 );
    m_IGESToCubicTol.SetDescript( "Tolerance Used When Demoting Higher Order Surfaces to Cubic" );
    m_IGESTrimTE.Init( "TrimTE", "IGESSettings", this, false, 0, 1 );
    m_IGESMergeLETE.Init( "MergeLETE", "IGESSettings", this, false, 0, 1 );
    m_IGESExportPropMainSurf.Init( "ExportPropMainSurf", "IGESSettings", this, false, 0, 1 );

    m_IGESLabelID.Init( "LabelID", "IGESSettings", this, true, 0, 1 );
    m_IGESLabelName.Init( "LabelName", "IGESSettings", this, true, 0, 1 );
    m_IGESLabelSurfNo.Init( "LabelSurfNo", "IGESSettings", this, true, 0, 1 );
    m_IGESLabelSplitNo.Init( "LabelSplitNo", "IGESSettings", this, true, 0, 1 );
    m_IGESLabelAirfoilPart.Init( "LabelAirfoilPart", "IGESSettings", this, true, 0, 1 );
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
    m_IGESStructureLabelAirfoilPart.Init( "StructureLabelAirfoilPart", "IGESSettings", this, true, 0, 1 );
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
    m_TargetHullFlag.Init( "TargetHullFlag", "Projection", this, false, false, true );
    m_BoundaryType.Init( "BoundaryType", "Projection", this, vsp::NO_BOUNDARY, vsp::NO_BOUNDARY, vsp::NUM_PROJ_BNDY_OPTIONS - 1 );
    m_BoundaryHullFlag.Init( "BoundaryHullFlag", "Projection", this, false, false, true );
    m_DirectionType.Init( "DirectionType", "Projection", this, vsp::X_PROJ, vsp::X_PROJ, vsp::NUM_PROJ_DIR_OPTIONS - 1 );
    m_XComp.Init( "XComp", "Projection", this, 0.0, -1.0, 1.0 );
    m_YComp.Init( "YComp", "Projection", this, 0.0, -1.0, 1.0 );
    m_ZComp.Init( "ZComp", "Projection", this, 0.0, -1.0, 1.0 );

    m_ViewportSizeXValue.Init( "ViewportX", "AdjustView", this, -1, -1, 1.0e12 );
    m_ViewportSizeYValue.Init( "ViewportY", "AdjustView", this, -1, -1, 1.0e12 );
    m_CORXValue.Init( "CORX", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );
    m_CORYValue.Init( "CORY", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );
    m_CORZValue.Init( "CORZ", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );
    m_PanXPosValue.Init( "PanX", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );
    m_PanYPosValue.Init( "PanY", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );
    m_ZoomValue.Init( "Zoom", "AdjustView", this, 0.018, 1e-6, 10 );
    m_XRotationValue.Init( "RotationX", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );
    m_YRotationValue.Init( "RotationY", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );
    m_ZRotationValue.Init( "RotationZ", "AdjustView", this, 0.0, -1.0e12, 1.0e12 );

    m_NewRatioValue.Init( "Ratio", "Screenshot", this, 1.0, 0.0, 1.0e12 );
    m_NewWidthValue.Init( "Width", "Screenshot", this, 1.0, 0.0, 1.0e12 );
    m_NewHeightValue.Init( "Height", "Screenshot", this, 1.0, 0.0, 1.0e12 );
    m_TransparentBGFlag.Init( "TransparentBGFlag", "Screenshot", this, 1, 0, 1 );
    m_AutoCropFlag.Init( "AutoCropFlag", "Screenshot", this, false, false, true );

    m_UserParmVal.Init( "Val", "UserParm", this, 0.0, -1.0e12, 1.0e12 );
    m_UserParmMin.Init( "Min", "UserParm", this, -1.0e5, -1.0e12, 1.0e12 );
    m_UserParmMax.Init( "Max", "UserParm", this, 1.0e5, -1.0e12, 1.0e12 );

    m_AdvLinkGenDefName.Init( "AdvLinkGenDefName", "AdvLinkSettings", this, false, false, true );
    m_AdvLinkDefNameContainer.Init( "AdvLinkDefNameContainer", "AdvLinkSettings", this, true, false, true );
    m_AdvLinkDefNameGroup.Init( "AdvLinkDefNameGroup", "AdvLinkSettings", this, false, false, true );
    m_AdvLinkDefNameAlias.Init( "AdvLinkDefNameAlias", "AdvLinkSettings", this, false, false, true );

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

    m_ScaleIndependentBbXLen.Init( "ScaleIndependentX_Len", "BBox", this, 0, 0, 1e12 );
    m_ScaleIndependentBbXLen.SetDescript( "X length of vehicle scale independent bounding box" );
    m_ScaleIndependentBbYLen.Init( "ScaleIndependentY_Len", "BBox", this, 0, 0, 1e12 );
    m_ScaleIndependentBbYLen.SetDescript( "Y length of vehicle scale independent bounding box" );
    m_ScaleIndependentBbZLen.Init( "ScaleIndependentZ_Len", "BBox", this, 0, 0, 1e12 );
    m_ScaleIndependentBbZLen.SetDescript( "Z length of vehicle scale independent bounding box" );
    m_ScaleIndependentBbXMin.Init( "ScaleIndependentX_Min", "BBox", this, 0, -1e12, 1e12 );
    m_ScaleIndependentBbXMin.SetDescript( "Minimum X coordinate of vehicle scale independent bounding box" );
    m_ScaleIndependentBbYMin.Init( "ScaleIndependentY_Min", "BBox", this, 0, -1e12, 1e12 );
    m_ScaleIndependentBbYMin.SetDescript( "Minimum Y coordinate of vehicle scale independent bounding box" );
    m_ScaleIndependentBbZMin.Init( "ScaleIndependentZ_Min", "BBox", this, 0, -1e12, 1e12 );
    m_ScaleIndependentBbZMin.SetDescript( "Minimum Z coordinate of vehicle scale independent bounding box" );

    m_UseModeCompGeomFlag.Init( "UseModeCompGeomFlag", "CompGeom", this, false, 0, 1 );
    m_UseModeExportFlag.Init( "UseModeExportFlag", "ExportFlag", this, false, 0, 1 );
    m_UseModeDegenGeomFlag.Init( "UseModeDegenGeomFlag", "DegenGeom", this, false, 0, 1 );

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

    m_StructModelUnit.Init( "StructModelUnit", "FeaStructure", this, vsp::LEN_UNITLESS, vsp::LEN_MM, vsp::LEN_UNITLESS );
    m_StructModelUnit.SetDescript( "OpenVSP model unit system" );

    m_StructUnit.Init( "StructUnit", "FeaStructure", this, vsp::BFT_UNIT, vsp::SI_UNIT, vsp::BIN_UNIT );
    m_StructUnit.SetDescript( "Unit System for output FEA Structures" );

    m_NumMassSlices.Init( "NumMassSlices", "MassProperties", this, 20, 10, 200 );
    m_NumMassSlices.SetDescript( "Number of slices used to display mesh" );

    m_MassSliceDir.Init( "MassSliceDir", "MassProperties", this, vsp::X_DIR, vsp::X_DIR, vsp::Z_DIR );
    m_MassSliceDir.SetDescript( "Slicing direction for mass property integration" );

    m_UseModeMassFlag.Init( "UseModeMassFlag", "MassProperties", this, false, 0, 1 );

    m_DrawCgFlag.Init( "DrawCgFlag", "MassProperties", this, true, false, true );
    m_DrawCgFlag.SetDescript( "Adds red center point to mesh" );

    m_NumPlanerSlices.Init( "NumPlanerSlices", "PSlice", this, 10, 1, 100 );
    m_NumPlanerSlices.SetDescript( "Number of planar slices used to display mesh" );

    m_UseModePlanarSlicesFlag.Init( "UseModePlanarSliceFlag", "PSlice", this, false, 0, 1 );

    m_AutoBoundsFlag.Init( "AutoBoundsFlag", "PSlice", this, true, false, true );
    m_AutoBoundsFlag.SetDescript( "Automatically sets Planar Start and End locations" );

    m_PlanarAxisType.Init( "PlanarAxisType", "PSlice", this, vsp::X_DIR, vsp::X_DIR, vsp::Z_DIR );
    m_PlanarAxisType.SetDescript( "Selects from X,Y,Z Axis for Planar Slice" );

    m_PlanarStartLocation.Init( "PlanarStartLocation", "PSlice", this, 0, -1e12, 1e12 );
    m_PlanarStartLocation.SetDescript( "Planar Start Location" );

    m_PlanarEndLocation.Init( "PlanarEndLocation", "PSlice", this, 10, -1e12, 1e12 );
    m_PlanarEndLocation.SetDescript( "Planar End Location" );

    m_PlanarMeasureDuct.Init( "MeasureDuctFlag", "PSlice", this, false, false, true );
    m_PlanarMeasureDuct.SetDescript( "Flag to measure negative area inside positive areas" );

    m_NumUserSets.Init( "NumUserSets", "Sets", this, MIN_NUM_USER, MIN_NUM_USER, MAX_NUM_SETS - SET_FIRST_USER );
    m_NumUserSets.SetDescript( "Number of user sets in this model.\n" );

    // ==== placeholder parms for Attribute Explorer Screen controls ==== //
    m_AttrCaseSensitivity.Init( "CaseSens", "AttributeExplorerGUI", this, 0.0, 0.0, 1.0 );
    m_AttrBoolButtonParm.Init( "AttrBool", "AttributeExplorerGUI", this, 0.0, 0.0, 1.0 );

    // ==== parm to show model notes on file load ==== //
    m_ShowNotesScreenParm.Init( "ShowNotes", "ModelNotesGUI", this, 0.0, 0.0, 1.0 );

    SetupPaths();
    m_VehProjectVec3d.resize( 3 );
    m_ColorCount = 0;

    m_ViewDirty = true;

    // Protect required enum value.
    assert( CUSTOM_GEOM_TYPE == 9 );
}

//==== Destructor ====//
Vehicle::~Vehicle()
{
    LinkMgr.UnRegisterContainer( this->GetID() );

    for ( auto it = m_GeomStoreMap.begin(); it != m_GeomStoreMap.end(); ++it )
    {
        delete it->second;
    }
    m_GeomStoreMap.clear();
}

//=== Init ====//
void Vehicle::Init()
{
    // Reset number of sets to default here so it can be used in this function.
    m_NumUserSets.Set( 20 );

    //==== Init Custom Geom and Script Mgr ====//
    LightMgr.Init();
    CustomGeomMgr.Init();
    ScriptMgr.Init();
    AdvLinkMgr.Init();
    CustomGeomMgr.ReadCustomScripts( this );

    m_Name = "Vehicle";

    SetVSP3FileName( "Unnamed.vsp3" );
    m_FileOpenVersion = -1;

    m_SetNameVec.clear();

    for ( int i = 0; i < m_SetAttrCollVec.size(); i++ )
    {
        AttributeMgr.DeregisterCollID( m_SetAttrCollVec[i]->GetID() );
        delete m_SetAttrCollVec[i];
    }
    m_SetAttrCollVec.clear();

    //==== Load Default Set Names =====//
    m_SetNameVec.push_back( "All" );        // SET_ALL
    m_SetNameVec.push_back( "Shown" );      // SET_SHOWN
    m_SetNameVec.push_back( "Not_Shown" );  // SET_NOT_SHOWN

    for ( int i = 0 ; i < m_NumUserSets(); i++ )
    {
        char str[256];
        snprintf( str, sizeof( str ),  "Set_%d", i );
        m_SetNameVec.push_back( str );

        AttributeCollection* ac_ptr = new AttributeCollection();

        string collname = m_SetNameVec.back() + "_Attributes";
        ac_ptr->SetName( collname );
        ac_ptr->SetCollAttach( GetID(), vsp::ATTROBJ_SET );
        m_SetAttrCollVec.push_back( ac_ptr );

        AttributeMgr.RegisterCollID( ac_ptr->GetID(), ac_ptr );
    }

    //==== Initialize Protected Vehicle Attributes ====//
    AddDefaultAttributes();

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
    m_GeomTypeVec.push_back( GeomType( GEAR_GEOM_TYPE, "GEAR", true ) );
    m_GeomTypeVec.push_back( GeomType( HINGE_GEOM_TYPE, "HINGE", true ) );
    m_GeomTypeVec.push_back( GeomType( CONFORMAL_GEOM_TYPE, "CONFORMAL", true ) );
    m_GeomTypeVec.push_back( GeomType( ROUTING_GEOM_TYPE, "ROUTING", true ) );
    m_GeomTypeVec.push_back( GeomType( AUXILIARY_GEOM_TYPE, "AUXILIARY", true ) );

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
    m_MassSliceDir = vsp::X_DIR;
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
    m_TransparentBGFlag.Set( true );
    m_AutoCropFlag.Set( false );

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

    m_ScaleIndependentBbXLen.Set( 0 );
    m_ScaleIndependentBbYLen.Set( 0 );
    m_ScaleIndependentBbZLen.Set( 0 );
    m_ScaleIndependentBbXMin.Set( 0 );
    m_ScaleIndependentBbYMin.Set( 0 );
    m_ScaleIndependentBbZMin.Set( 0 );

    m_exportCompGeomCsvFile.Set( true );
    m_exportDegenGeomCsvFile.Set( true );
    m_exportDegenGeomMFile.Set( true );

    m_ViewDirty = true;

    AnalysisMgr.Init();
}

void Vehicle::RunTestScripts()
{
    ScriptMgr.RunTestScripts();
}

void Vehicle::AddDefaultAttributes()
{
    int attachType = vsp::ATTROBJ_VEH;
    // for Vehicle attributes, initialize with a few protected attributes

    m_AttrCollection.SetCollAttach( GetID(), attachType );
    string veh_coll_id = m_AttrCollection.GetID();
    bool update_flag = true;

    AttributeMgr.AddAttributeString( veh_coll_id, string("VSP::VehicleNotes"), string(), update_flag, ATTR_VEH_NOTES );
    AttributeMgr.AddAttributeGroup( veh_coll_id, "VSP::WatermarkGroup", update_flag, ATTR_WM_GROUP );
    string wm_group_ac_id = AttributeMgr.GetChildCollection( ATTR_WM_GROUP );

    AttributeMgr.AddAttributeBool(
        wm_group_ac_id,
        string("VSP::ShowWatermark"),
        false,
        update_flag,
        ATTR_WM_SHOW
    );
    AttributeMgr.AddAttributeString(
        wm_group_ac_id,
        string("VSP::Text"),
        string("Watermark"),
        update_flag,
        ATTR_WM_TEXT
    );
    AttributeMgr.AddAttributeDouble(
        wm_group_ac_id,
        string("VSP::TextScale"),
        2.0,
        update_flag,
        ATTR_WM_TEXTSCALE
    );
    AttributeMgr.AddAttributeVec3d(
        wm_group_ac_id,
        string("VSP::TextColor"),
        { vec3d( 1, 1, 1 ) },
        update_flag,
        ATTR_WM_TEXTCOLOR
    );
    AttributeMgr.AddAttributeDouble(
        wm_group_ac_id,
        string("VSP::TextAlpha"),
        1.0,
        update_flag,
        ATTR_WM_TEXTALPHA
    );
    AttributeMgr.AddAttributeVec3d(
        wm_group_ac_id,
        string("VSP::EdgeColor"),
        { vec3d( 0, 0, 0 ) },
        update_flag,
        ATTR_WM_EDGECOLOR
    );
    AttributeMgr.AddAttributeDouble(
        wm_group_ac_id,
        string("VSP::EdgeAlpha"),
        0.0,
        update_flag,
        ATTR_WM_EDGEALPHA
    );
    AttributeMgr.AddAttributeVec3d(
        wm_group_ac_id,
        string("VSP::FillColor"),
        { vec3d( 0.01, 0.5, 0.32 ) },
        update_flag,
        ATTR_WM_FILLCOLOR
    );
    AttributeMgr.AddAttributeDouble(
        wm_group_ac_id,
        string("VSP::FillAlpha"),
        1.0,
        update_flag,
        ATTR_WM_FILLALPHA
    );

    AttributeMgr.SetAttributeDoc( ATTR_WM_GROUP, string("Watermark attributes for text & display control") );
    AttributeMgr.SetAttributeDoc( ATTR_WM_SHOW, string("Toggle watermark display state") );
    AttributeMgr.SetAttributeDoc( ATTR_WM_TEXT, string("Watermark text") );
    AttributeMgr.SetAttributeDoc( ATTR_WM_TEXTSCALE, string("Scale factor for watermark text") );
    AttributeMgr.SetAttributeDoc( ATTR_WM_TEXTCOLOR, string("RGB color for watermark text") );
    AttributeMgr.SetAttributeDoc( ATTR_WM_TEXTALPHA, string("Alpha for watermark text") );
    AttributeMgr.SetAttributeDoc( ATTR_WM_EDGECOLOR, string("RGB Color for watermark box") );
    AttributeMgr.SetAttributeDoc( ATTR_WM_EDGEALPHA, string("Alpha for watermark box") );
    AttributeMgr.SetAttributeDoc( ATTR_WM_FILLCOLOR, string("RGB Color for watermark fill") );
    AttributeMgr.SetAttributeDoc( ATTR_WM_FILLALPHA, string("Alpha for watermark fill") );
    AttributeMgr.SetAttributeDoc( ATTR_VEH_NOTES, string("Vehicle level notes for users") );

    AttributeMgr.SetAttributeEventGroup( ATTR_WM_GROUP, ATTR_GROUP_WATERMARK );
    AttributeMgr.SetAttributeEventGroup( ATTR_WM_SHOW, ATTR_GROUP_WATERMARK );
    AttributeMgr.SetAttributeEventGroup( ATTR_WM_TEXT, ATTR_GROUP_WATERMARK );
    AttributeMgr.SetAttributeEventGroup( ATTR_WM_TEXTSCALE, ATTR_GROUP_WATERMARK );
    AttributeMgr.SetAttributeEventGroup( ATTR_WM_TEXTCOLOR, ATTR_GROUP_WATERMARK );
    AttributeMgr.SetAttributeEventGroup( ATTR_WM_TEXTALPHA, ATTR_GROUP_WATERMARK );
    AttributeMgr.SetAttributeEventGroup( ATTR_WM_EDGECOLOR, ATTR_GROUP_WATERMARK );
    AttributeMgr.SetAttributeEventGroup( ATTR_WM_EDGEALPHA, ATTR_GROUP_WATERMARK );
    AttributeMgr.SetAttributeEventGroup( ATTR_WM_FILLCOLOR, ATTR_GROUP_WATERMARK );
    AttributeMgr.SetAttributeEventGroup( ATTR_WM_FILLALPHA, ATTR_GROUP_WATERMARK );

    AttributeMgr.SetAttributeProtection( ATTR_WM_GROUP );
    AttributeMgr.SetAttributeProtection( ATTR_WM_SHOW );
    AttributeMgr.SetAttributeProtection( ATTR_WM_TEXT );
    AttributeMgr.SetAttributeProtection( ATTR_WM_TEXTSCALE );
    AttributeMgr.SetAttributeProtection( ATTR_WM_TEXTCOLOR );
    AttributeMgr.SetAttributeProtection( ATTR_WM_TEXTALPHA );
    AttributeMgr.SetAttributeProtection( ATTR_WM_EDGECOLOR );
    AttributeMgr.SetAttributeProtection( ATTR_WM_EDGEALPHA );
    AttributeMgr.SetAttributeProtection( ATTR_WM_FILLCOLOR );
    AttributeMgr.SetAttributeProtection( ATTR_WM_FILLALPHA );
    AttributeMgr.SetAttributeProtection( ATTR_VEH_NOTES );
}

//=== Wype ===//
void Vehicle::Wype()
{
    // Re-initialize everything to default.  Care must be taken with dynamic memory
    // or cases where outside pointers access items being freed.  Otherwise, just
    // clobber it with a default assignment, let the compiler take care of cleaning
    // up the variables on the stack.

    // Remove references to this set up in Init()

    //wype the attributeManager BEFORE removing geoms etc.
    AttributeMgr.Wype();

    LinkMgr.UnRegisterContainer( this->GetID() );

    // Public member variables
    m_IxxIyyIzz = vec3d();
    m_IxyIxzIyz = vec3d();
    m_CG = vec3d();
    m_NumMassSlices = int();
    m_MassSliceDir = vsp::X_DIR;
    m_TotalMass = double();
    m_AttrCollection.Wype();

    m_ViewDirty = true;

    // Private member variables
    m_Name = string();

    m_VSP3FileName = string();

    m_AFFileDir = string();

    m_BEMPropID = string();

    for ( auto it = m_GeomStoreMap.begin(); it != m_GeomStoreMap.end(); ++it )
    {
        delete it->second;
    }
    m_GeomStoreMap.clear();

    m_ActiveGeom.clear();
    m_TopGeom.clear();
    m_ClipBoard.clear();
    m_SetNameVec.clear();

    for ( int i = 0; i < m_SetAttrCollVec.size(); i++ )
    {
        AttributeMgr.DeregisterCollID( m_SetAttrCollVec[i]->GetID() );
        delete m_SetAttrCollVec[i];
    }
    m_SetAttrCollVec.clear();

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
    OldVarPresetMgr.Renew();
    ModeMgr.Renew();
    ParasiteDragMgr.Renew();
    VSPAEROMgr.Renew();
    MeasureMgr.Renew();
    Background3DMgr.Renew();
    StructureMgr.Renew();
    GeometryAnalysisMgr.Renew();

    ResultsMgr.DeleteAllResults();

    // Need to renew FeaMeshMgr to  FeaMeshMgr.CleanMeshMap() however, it is invisible from here.

    LightMgr.Wype();
}

void Vehicle::SetVSP3FileName( const string & f_name )
{
    m_VSP3FileName = f_name;

    std::filesystem::path filepath( m_VSP3FileName );

    if ( m_VSP3FileName != "Unnamed.vsp3" && filepath.is_relative() )
    {
        std::filesystem::path vsppath = std::filesystem::current_path();
        vsppath /= f_name;

        m_VSP3FileName = vsppath.generic_string();
    }

    m_CfdSettings.ResetExportFileNames( m_VSP3FileName );
    m_ISectSettings.ResetExportFileNames( m_VSP3FileName );
    StructureMgr.ResetAllExportFileNames();
    resetExportFileNames();
}

// Make fname into a path relative to m_VSP3ileName.
void Vehicle::MakeRelativePath( string & fname )
{
    if ( m_VSP3FileName != "Unnamed.vsp3" )
    {
        std::filesystem::path vsppath( m_VSP3FileName );
        vsppath.remove_filename();
        const std::filesystem::path inpath( fname );

        fname = std::filesystem::relative( inpath, vsppath ).generic_string();
    }
}

// Make relative path fname into absolute path.
void Vehicle::MakeAbsolutePath( string & fname )
{
    if ( m_VSP3FileName != "Unnamed.vsp3" )
    {
        std::filesystem::path vsppath( m_VSP3FileName );
        vsppath.remove_filename();

        vsppath /= fname; // Append with appropriate path separator.

        fname = vsppath.generic_string();
    }
}

void Vehicle::SetupPaths()
{
    m_ExePath = PathToExe();
    m_HelpPath = m_ExePath + string( "/help/" );
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

    m_VSPAEROFound = false;
    m_VIEWERFound = false;
    m_LOADSFound = false;

    m_CustomScriptDirs.push_back( string( "./CustomScripts/" ) );
    m_CustomScriptDirs.push_back( m_HomePath + string( "/CustomScripts/" ) );
    m_CustomScriptDirs.push_back( m_ExePath + string( "/CustomScripts/" ) );
}

bool Vehicle::CheckForVSPAERO( const string & path )
{
    AeroStructMgr.FindCCX( path );
    AeroStructMgr.FindCGX( path );

    bool ret_val = true;

    if( !CheckForFile( path, m_VSPAEROCmd ) )
    {
        fprintf( stderr, "WARNING %d: VSPAERO Solver Not Found. \n"
            "\tExpected here: %s\n",
            vsp::VSP_FILE_DOES_NOT_EXIST,
            ( path + string("/") + m_VSPAEROCmd ).c_str() );
        ret_val = false;
    }
    else
    {
        m_VSPAEROFound = true;
    }

    if( !CheckForFile( path, m_VIEWERCmd ) )
    {
#ifndef VSP_NO_GRAPHICS
        fprintf( stderr, "WARNING %d: VSPAERO Viewer Not Found. \n"
            "\tExpected here: %s\n",
            vsp::VSP_FILE_DOES_NOT_EXIST,
            ( path + string("/") + m_VIEWERCmd ).c_str() );
        ret_val = false;
#endif
    }
    else
    {
        m_VIEWERFound = true;
    }

    if( !CheckForFile( path, m_LOADSCmd ) )
    {
        fprintf( stderr, "WARNING %d: VSPAERO Loads Not Found. \n"
            "\tExpected here: %s\n",
            vsp::VSP_FILE_DOES_NOT_EXIST,
            ( path + string("/") + m_LOADSCmd ).c_str() );
        ret_val = false;
    }
    else
    {
        m_LOADSFound = true;
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

bool Vehicle::CheckForHelp( const string & path )
{
    string helpfile = "vsp_help";

    if( !CheckForFile( path, helpfile ) )
    {
        fprintf( stderr, "WARNING %d: OpenVSP Help Not Found. \n"
            "\tExpected here: %s\n",
            vsp::VSP_FILE_DOES_NOT_EXIST,
            ( path + string("/") + helpfile ).c_str() );
        return false;
    }

    return true;
}

bool Vehicle::SetHelpPath( const string & path )
{
    if( CheckForHelp( path ) )
    {
        m_HelpPath = path + "/";
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

//==== Parm Changed ====//
void Vehicle::ParmChanged( Parm* parm_ptr, int type )
{
    if ( m_UpdatingBBox )
    {
        return;
    }

    if ( parm_ptr && parm_ptr->GetGroupName() == string( "AdjustView" ) )
    {
        m_ViewDirty = true;
    }

    m_UpdatingBBox = true;
    UpdateBBox();
    m_UpdatingBBox = false;

    if ( parm_ptr == &m_AxisLength )
    {
        ForceUpdate( GeomBase::XFORM );
    }

    if ( parm_ptr == & m_NumUserSets )
    {
        if ( m_SetNameVec.size() != m_NumUserSets() + SET_FIRST_USER )
        {
            SetNumUserSets( m_NumUserSets() );
            Update();
        }
    }

    UpdateGUI();
}

//==== Update All Screens ====//
void Vehicle::UpdateGUI()
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

    UpdateBBox();

    MeasureMgr.Update();
    Background3DMgr.Update();
}

// Update managers that are normally only updated by their
// associated GUI. This enables update from the API
void Vehicle::UpdateManagers()
{
    VSPAEROMgr.Update();
    WaveDragMgr.Update();
    ParasiteDragMgr.Update();
    StructureMgr.Update();
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
    for ( auto it = m_GeomStoreMap.begin(); it != m_GeomStoreMap.end(); ++it )
    {
        Geom* g_ptr = it->second;
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

void Vehicle::ChangeGeomID( const string &oldid, const string &newid )
{
    auto it = m_GeomStoreMap.find( oldid );
    if ( it != m_GeomStoreMap.end() )
    {
        Geom* g_ptr = it->second;
        m_GeomStoreMap.erase( it );
        m_GeomStoreMap[ newid ] = g_ptr;
    }
}

//==== Find Geom Based on GeomID ====//
Geom* Vehicle::FindGeom( const string & geom_id )
{
    if ( geom_id == string( "NONE" ) )
    {
        return nullptr;
    }

    auto it = m_GeomStoreMap.find( geom_id );
    if ( it != m_GeomStoreMap.end() )
    {
        return it->second;
    }
    return nullptr;
}

//==== Find Vector of Geom Ptrs Based on GeomID ====//
vector< Geom* > Vehicle::FindGeomVec( const vector< string > & geom_id_vec )
{
    vector< Geom* > geom_vec;
    for ( int i = 0 ; i < ( int )geom_id_vec.size() ; i++ )
    {
        Geom* gptr = FindGeom( geom_id_vec[i] );
        if ( gptr != nullptr )
        {
            geom_vec.push_back( gptr );
        }
    }
    return geom_vec;
}


//=== Create Geom of Type, Add To Storage and Return ID ====//
string Vehicle::CreateGeom( const GeomType & type )
{
    Geom* new_geom = nullptr;

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
    else if ( type.m_Name == "NGonMesh" || type.m_Name == "NGON" )
    {
        new_geom = new NGonMeshGeom( this );
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
    else if ( type.m_Name == "Routing" || type.m_Name == "ROUTING" )
    {
        new_geom = new RoutingGeom( this );
    }
    else if ( type.m_Name == "Auxiliary" || type.m_Name == "AUXILIARY" )
    {
        new_geom = new AuxiliaryGeom( this );
    }
    else if ( type.m_Name == "Gear" || type.m_Name == "GEAR" )
    {
        new_geom = new GearGeom( this );
    }

    if ( !new_geom )
    {
        printf( "Error: Could not create Geom of type: %s\n", type.m_Name.c_str() );
        return "NONE";
    }

    m_GeomStoreMap[ new_geom->GetID() ] = new_geom;

    Geom* type_geom_ptr = FindGeom( type.m_GeomID );
    if ( type_geom_ptr )
    {
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
                     par->GetType().m_Type == NGON_GEOM_TYPE ||
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
        const string& parent_id = active_vec.back();           // Parent
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
        mesh_geom->Show();    // Ensure Shown Flag is Set
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

vector< TMesh* > Vehicle::CreateTMeshVec( int normal_set )
{
    vector< TMesh* > tmv;
    vector<string> geom_vec = GetGeomVec();

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
                    tmv.push_back( tMeshVec[j] );
                }
            }
        }
    }
    return tmv;
}

vector< TMesh* > Vehicle::CreateTMeshVec( const string &geomid )
{
    vector< TMesh* > tmv;
    Geom *g = FindGeom( geomid );
    if ( g )
    {
        tmv = g->CreateTMeshVec();
    }
    return tmv;
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

vector< Geom* > Vehicle::GetGeomStoreVec()
{
    vector< Geom* > geom_vec;
    geom_vec.reserve( m_GeomStoreMap.size() );
    for ( auto it = m_GeomStoreMap.begin(); it != m_GeomStoreMap.end(); it++ )
    {
        geom_vec.push_back( it->second );
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
            if ( geom->GetMainSurfType(0) == vsp::WING_SURF )
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
        const string& id = del_vec[c];
        DeleteGeom( id );
    }
}

void Vehicle::CutGeomVec( const vector< string > & cut_vec )
{
    RemoveGeomVecFromHierarchy( cut_vec );

    //=== All Geoms To Be Cut ====//
    for ( int c = 0 ; c < ( int )cut_vec.size() ; c++ )
    {
        const string& id = cut_vec[c];
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
        const string& id = cut_vec[c];
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
        const string& id = cut_vec[c];
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
                const string& child_id = child_vec[i];
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

    const string& active_geom_id = active_geom_vec[0];
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

    int index = vector_find_val( id_vec, active_geom_id );
    ReorderVectorIndex( id_vec, index, action );

    if ( !parent_geom )
    {
        m_TopGeom = id_vec;
    }
    else
    {
        parent_geom->SetChildIDVec( id_vec );
    }

}

void Vehicle::ReparentActiveGeom( int action )
{
    //==== Find Active Geom ====//
    vector< string > active_geom_vec = GetActiveGeomVec();
    if ( active_geom_vec.size() != 1 )
    {
        return;
    }

    const string& active_geom_id = active_geom_vec[0];
    Geom* active_geom = FindGeom( active_geom_id );
    if ( !active_geom )
    {
        return;
    }

    //move geom up one level in hierarchy
    //get parent geom's parent, promote to child of current grandparent, if applicable

    if ( action == vsp::REORDER_MOVE_UP )
    {
        // if no parentID, geom already in TopGeom vec and cannot move up
        if ( active_geom->GetParentID() == string("NONE") )
        {
            return;
        }

        // try and see if there is a grandparent geom; if not then promote to vehicle top geom
        int gen = 2;
        string grandparent_id = active_geom->GetAncestorID( gen );
        Geom* grandparent_geom = FindGeom( grandparent_id );

        // get location of parent id in its local child vector
        vector < string > sibling_vector;
        vector < string > ::iterator index_iter;
        string new_sibling_id = string();

        if ( grandparent_geom )
        {
            sibling_vector = grandparent_geom->GetChildIDVec();
        }
        else
        {
            sibling_vector = m_TopGeom;
        }

        index_iter = find( sibling_vector.begin(), sibling_vector.end(), active_geom->GetParentID() );
        if ( index_iter != sibling_vector.end() )
        {
            new_sibling_id = *(index_iter);
        }

        if ( grandparent_geom )
        {
            active_geom->ChangeParentID( grandparent_id, new_sibling_id );
        }
        else
        {
            active_geom->ChangeParentID( GetID(), new_sibling_id );
        }
    }

    else if ( action == vsp::REORDER_MOVE_DOWN )
    {
        // Get geomIDvec from TopGeomVec or geom's parent's ChildVec
        // move into TOP of childVec of next id above the active geom

        string parent_id = active_geom->GetParentID();

        Geom* parent_geom = FindGeom( parent_id );

        // if geom is child of another geom, access the geom neighbor from the parent's childvec
        vector < string > sibling_vector;

        if ( parent_geom )
        {
            sibling_vector = parent_geom->GetChildIDVec();
        }
        // if geom is at top level in vehicle, access neighbor from vehicle TopGeom vector
        else
        {
            sibling_vector = m_TopGeom;
        }

        vector < string > ::iterator index_iter;

        //where the active geom is in the sibling vector
        index_iter = find( sibling_vector.begin(), sibling_vector.end(), active_geom_id );

        if ( index_iter != sibling_vector.begin() )
        {
            string new_parent_id = *(index_iter-1);
            Geom* new_parent_geom = FindGeom( new_parent_id );
            if ( new_parent_geom )
            {
                active_geom->ChangeParentID( new_parent_id );
            }
        }
    }
}

//==== \Delete Active Geom ====//
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
        auto it = m_GeomStoreMap.find( m_ClipBoard[ i ] );
        if ( it != m_GeomStoreMap.end() )
        {
            Geom *gPtr = it->second;
            if ( gPtr )
            {
                m_GeomStoreMap.erase( it );
                delete gPtr;
            }
        }
    }
    m_ClipBoard.clear();
}

void Vehicle::DeleteGeom( const string & geom_id )
{
    auto it = m_GeomStoreMap.find( geom_id );
    if ( it != m_GeomStoreMap.end() )
    {
        Geom *gPtr = it->second;
        if ( gPtr )
        {
            m_GeomStoreMap.erase( it );
            vector_remove_val( m_ActiveGeom, geom_id );
            delete gPtr;
        }
    }
}

void Vehicle::AddTopGeomID( const string & geom_id, const string &insert_after_id )
{
    vector < string > ::iterator index_iter;
    index_iter = find( m_TopGeom.begin(), m_TopGeom.end(), insert_after_id );

    if ( insert_after_id.empty() || index_iter == m_TopGeom.end() )
    {
        m_TopGeom.push_back( geom_id );
    }
    else
    {
        m_TopGeom.insert( index_iter + 1, geom_id );
    }
}

void Vehicle::RemoveTopGeomID( const string & geom_id )
{
    vector_remove_val( m_TopGeom, geom_id );
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

//==== Return bool if id found in vehicle clipboard ====//
bool Vehicle::IDinClipboard( const string & id )
{
    for ( int i = 0; i != m_ClipBoard.size(); ++i )
    {
        if ( m_ClipBoard.at( i ).compare( id ) == 0 )
        {
            return true;
        }
    }
    return false;
}

//==== Copy Geoms In Vec - Create New IDs But Keep Parent/Child ====//
vector< string > Vehicle::CopyGeomVec( const vector< string > & geom_vec )
{
    string lastreset = ParmMgr.ResetRemapID();

    //==== Create New Geoms ====//
    vector< string > created_id_vec;

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
                    for ( int j = SET_FIRST_USER; j < m_NumUserSets() + SET_FIRST_USER; j++ )
                    {
                        toPtr->SetSetFlag( j, false );
                    }

                    toPtr->Show();
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

            if ( par == nullptr )
            {
                geom->SetParentID( "NONE" );
            }

            vector< string > childvec = geom->GetChildIDVec();

            for ( int j = 0; j < static_cast<int>( childvec.size() ); j++ )
            {
                Geom* child = FindGeom( childvec[j] );

                if ( child == nullptr )
                {
                    geom->RemoveChildID( childvec[j] );
                }
            }
        }
    }

    return created_id_vec;
}

//==== Get Draw Objects ====//
void Vehicle::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    //==== Traverse All Active Displayed Geom and Load DrawObjs ====//
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        geom_vec[i]->LoadDrawObjs( draw_obj_vec );
    }
}

void Vehicle::SetNumUserSets( int nuset )
{
    char str[256];

    // Just to ensure consistency.
    m_NumUserSets.Set( nuset );

    int ntotal = m_NumUserSets() + SET_FIRST_USER;

    if ( m_SetNameVec.size() > ntotal )
    {
        m_SetNameVec.resize( ntotal );
    }

    if ( m_SetAttrCollVec.size() > nuset )
    {
        for ( int i = nuset; i < m_SetAttrCollVec.size(); i++ )
        {
            AttributeMgr.DeregisterCollID( m_SetAttrCollVec[i]->GetID() );
            delete m_SetAttrCollVec[i];
        }
        m_SetAttrCollVec.resize( nuset );
    }

    while ( ( int )m_SetNameVec.size() < ntotal )
    {
        snprintf( str, sizeof( str ),  "Set_%d", ( int )m_SetNameVec.size() - SET_FIRST_USER );
        m_SetNameVec.push_back( string( str ) );

        AttributeCollection* ac_ptr = new AttributeCollection();
        string collname = m_SetNameVec.back() + "_Attributes";
        ac_ptr->SetName( collname );
        ac_ptr->SetCollAttach( GetID(), vsp::ATTROBJ_SET );
        m_SetAttrCollVec.push_back( ac_ptr );

        AttributeMgr.RegisterCollID( ac_ptr->GetID(), ac_ptr );
    }
}

void Vehicle::SetSetName( int index, const string& name )
{
    if ( index < 0 || index >= m_SetNameVec.size() )
    {
        return;
    }

    m_SetNameVec[index] = name;

    int coll_index = index - SET_FIRST_USER;

    if ( coll_index > -1 && coll_index <= m_SetAttrCollVec.size() )
    {
        string collname = name + "_Attributes";
        m_SetAttrCollVec[ coll_index ]->SetName( collname );
    }
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
        if ( geom_vec[i]->GetSetFlag( index ) )
        {
            geom_vec[i]->Show();
        }
        else
        {
            geom_vec[i]->NoShow();
        }
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
            geom_vec[i]->NoShow();
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
            geom_vec[i]->Show();
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

string Vehicle::GetGeomSetAttrColl( int index )
{
    string ac_id = string();

    if ( index > -1 && index <= m_SetAttrCollVec.size() )
    {
        ac_id = m_SetAttrCollVec[ index ]->GetID();
    }
    return ac_id;
}

void Vehicle::SetNormalDisplayType( int index )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( index ) )
        {
            for ( size_t j = 0; j < geom_vec[i]->GetNumMainSurfs(); j++ )
            {
                geom_vec[i]->m_GuiDraw.SetDisplayType( vsp::DISPLAY_TYPE::DISPLAY_BEZIER );
            }
            geom_vec[i]->SetDirtyFlag( GeomBase::TESS );
            geom_vec[i]->Update();
        }
    }
}

void Vehicle::SetDegenDisplayType( int index )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( index ) )
        {
            for ( size_t j = 0; j < geom_vec[i]->GetNumMainSurfs(); j++ )
            {
                int surf_type = geom_vec[i]->GetMainSurfType(j);

                if ( surf_type == vsp::DISK_SURF )
                {
                    geom_vec[i]->m_GuiDraw.SetDisplayType( vsp::DISPLAY_TYPE::DISPLAY_DEGEN_SURF );
                }
                else if ( surf_type == vsp::WING_SURF )
                {
                    geom_vec[i]->m_GuiDraw.SetDisplayType( vsp::DISPLAY_TYPE::DISPLAY_DEGEN_CAMBER );
                }
                else
                {
                    geom_vec[i]->m_GuiDraw.SetDisplayType( vsp::DISPLAY_TYPE::DISPLAY_DEGEN_PLATE );
                }
            }
            geom_vec[i]->SetDirtyFlag( GeomBase::TESS );
            geom_vec[i]->Update();
        }
    }
}


void Vehicle::HideAllExcept( const string& id )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        Geom* geom_ptr = geom_vec[i];

        if ( geom_ptr )
        {
            if ( geom_ptr->GetID() != id )
            {
                // No Show All Other Components
                geom_ptr->NoShow();
            }
        }
    }
}

void Vehicle::HideAll()
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        Geom* geom_ptr = geom_vec[i];

        if ( geom_ptr )
        {
            // No Show All Other Components
            geom_ptr->NoShow();
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

xmlNodePtr Vehicle::EncodeXml( xmlNodePtr & node, int set )
{
    xmlNodePtr vehicle_node = xmlNewChild( node, nullptr, BAD_CAST"Vehicle", nullptr );

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
    ModeMgr.EncodeXml( node );
    m_CfdSettings.EncodeXml( node );
    m_ISectSettings.EncodeXml( node );
    m_CfdGridDensity.EncodeXml( node );
    StructureMgr.EncodeXml( node );
    GeometryAnalysisMgr.EncodeXml( node );
    m_ClippingMgr.EncodeXml( node );
    WaveDragMgr.EncodeXml( node );
    ParasiteDragMgr.EncodeXml( node );
    AeroStructMgr.EncodeXml( node );
    Background3DMgr.EncodeXml( node );

    xmlNodePtr setnamenode = xmlNewChild( node, nullptr, BAD_CAST"SetNames", nullptr );
    if ( setnamenode )
    {
        for ( int i = 0; i < m_SetNameVec.size(); i++ )
        {
            XmlUtil::AddStringNode( setnamenode, "Set", m_SetNameVec[i] );
        }
    }

    xmlNodePtr set_attr_node = xmlNewChild( node, nullptr, BAD_CAST"SetAttrs", nullptr );
    if ( set_attr_node )
    {
        for ( int i = 0; i < m_SetAttrCollVec.size(); i++ )
        {
            int j = i + SET_FIRST_USER;

            if ( m_SetAttrCollVec[i]->GetAttrDataFlag() )
            {
                xmlNodePtr dnode = xmlNewChild( set_attr_node, nullptr, BAD_CAST"Set", nullptr );
                if ( dnode )
                {
                    XmlUtil::SetStringProp( dnode, "SetName", m_SetNameVec[j] );
                    XmlUtil::SetIntProp( dnode, "SetIndex", i );
                    m_SetAttrCollVec[i]->EncodeXml( dnode );
                }
            }
        }
    }

    return vehicle_node;
}

xmlNodePtr Vehicle::DecodeXml( xmlNodePtr & node )
{
    // Preserve viewport size.
    double viewX = m_ViewportSizeXValue();
    double viewY = m_ViewportSizeYValue();
    xmlNodePtr vehicle_node = XmlUtil::GetNode( node, "Vehicle", 0 );
    if ( vehicle_node )
    {
        ParmContainer::DecodeXml( vehicle_node );

        // Decode lighting information.
        LightMgr.DecodeXml( vehicle_node );

    }
    // Maintain viewport size
    m_ViewportSizeXValue = viewX;    m_ViewportSizeYValue = viewY;
    // Reset view to default.
    m_CORXValue = 0;    m_CORYValue = 0;    m_CORZValue = 0;
    m_PanXPosValue = 0;    m_PanYPosValue = 0;
    m_ZoomValue = 0.018;
    m_XRotationValue = 0;    m_YRotationValue = 0;    m_ZRotationValue = 0;

    // 'GeomsOnly' is a euphamism for those entities we want to read when 'inserting' a file.
    // It is mostly the Geoms, but also materials, presets, links, and advanced links.
    DecodeXmlGeomsOnly( node );

    VSPAEROMgr.DecodeXml( node );
    m_CfdSettings.DecodeXml( node );
    m_ISectSettings.DecodeXml( node );
    m_CfdGridDensity.DecodeXml( node );
    m_ClippingMgr.DecodeXml( node );
    WaveDragMgr.DecodeXml( node );
    ParasiteDragMgr.DecodeXml( node );
    AeroStructMgr.DecodeXml( node );
    Background3DMgr.DecodeXml( node );

    ParasiteDragMgr.CorrectTurbEquation();

    xmlNodePtr setnamenode = XmlUtil::GetNode( node, "SetNames", 0 );
    if ( setnamenode )
    {
        int num = XmlUtil::GetNumNames( setnamenode, "Set" );

        SetNumUserSets( num - SET_FIRST_USER );

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

    for ( int i = 0; i < m_SetAttrCollVec.size(); i++ )
    {
        m_SetAttrCollVec[i]->SetCollAttach( GetID(), vsp::ATTROBJ_SET );
    }

    xmlNodePtr set_attr_node = XmlUtil::GetNode( node, "SetAttrs", 0 );
    if ( set_attr_node )
    {
        int def_int = -1;
        string def_str = string();

        int num_set_colls = XmlUtil::GetNumNames( set_attr_node, "Set" );
        for ( int i = 0; i < num_set_colls; i++ )
        {
            xmlNodePtr set_node = XmlUtil::GetNode( set_attr_node, "Set", i );
            if ( set_node )
            {
                string set_name = XmlUtil::FindStringProp( set_node, "SetName", def_str );
                int set_index = XmlUtil::FindIntProp( set_node, "SetIndex", def_int );
                m_SetAttrCollVec[set_index]->DecodeXml( set_node );
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
        // Decode label information.
        MeasureMgr.DecodeXml( vehicle_node );

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
    OldVarPresetMgr.DecodeXml( node );
    VarPresetMgr.ConvertOldToNew();
    ModeMgr.DecodeXml( node );
    StructureMgr.DecodeXml( node );
    GeometryAnalysisMgr.DecodeXml( node );
    Background3DMgr.EncodeXml( node );

    return vehicle_node;
}

//==== Write File ====//
bool Vehicle::WriteXMLFile( const string & file_name, int set )
{
    xmlDocPtr doc = xmlNewDoc( ( const xmlChar * )"1.0" );

    xmlNodePtr root = xmlNewNode( nullptr, ( const xmlChar * )"Vsp_Geometry" );
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
    doc = xmlReadFile( file_name.c_str(), nullptr, XML_PARSE_HUGE );
    if ( doc == nullptr )
    {
        fprintf( stderr, "could not parse XML document\n" );
        return 1;
    }

    xmlNodePtr root = xmlDocGetRootElement( doc );
    if ( root == nullptr )
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
    doc = xmlReadFile( file_name.c_str(), nullptr, XML_PARSE_HUGE );
    if ( doc == nullptr )
    {
        fprintf( stderr, "could not parse XML document\n" );
        return 1;
    }

    xmlNodePtr root = xmlDocGetRootElement( doc );
    if ( root == nullptr )
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
void Vehicle::WriteXSecFile( const string & file_name, int write_set, bool useMode, const string& modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

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
void Vehicle::WritePLOT3DFile( const string & file_name, int write_set, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

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
string Vehicle::WriteSTLFile( const string & file_name, int write_set, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

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
string Vehicle::WriteTaggedMSSTLFile( const string & file_name, int write_set, int subsFlag, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

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
                mg->SubTagTris( subsFlag );
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
            mg->BuildIndexedMesh();
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
string Vehicle::WriteFacetFile( const string & file_name, int write_set, int subsFlag, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

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
                mg->SubTagTris( subsFlag );
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
                mg->BuildIndexedMesh();
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
string Vehicle::WriteTRIFile( const string & file_name, int write_set, int subsFlag, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

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
                mg->SubTagTris( subsFlag );
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
            mg->BuildIndexedMesh();
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
string Vehicle::WriteOBJFile( const string & file_name, int write_set, int subsFlag, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

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
                mg->SubTagTris( subsFlag );
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
            mg->BuildIndexedMesh();
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
# vspgeom v2                 // Header marking file version number -- added with v2.
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
p1 t1 u11 v11 u12 v12...u1n v1n // Part number then tag number for face 1 followed by multi-valued nodal data -- currently parametric UV coordinate.
p2 t2 u21 v21 u22 v22...u2n v2n // Also any other face-centered data that we desire to add.
...
pnface tnface un1 vn2...unn vnn // Last part then tag, multi-valued nodal and face-centered data
nwake                        // Number of wake lines
n1 i11 i12 i13 i14...i1n     // Number of points in wake line, indices in chain-order.  Typically line wrapped at ten indices per line.
n2 i21 i22 i13 i24...i2n
...
nnwake in1 in2 in3 in4...inn // Last wake line
f1 n1 i11 i12 i13...i1n      // Alternate triangulation of faces.
f2 n2 i21 i22 i23...i2n      // Face number, number of triangles, node-list of triangles for face
...
nface nn in1 in2 in3...inn   // Last alternate triangulation line
*/

//==== Write VSPGeom File ====//
string Vehicle::WriteVSPGeomFile( const string &file_name, int write_set, int degen_set, int subsFlag, bool useMode, const string &modeID, bool half_flag, bool hideset, bool suppressdisks )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            degen_set = m->m_DegenSet();
        }
    }

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

                if ( half_flag )
                {
                    // This check is to ensure any triangles remaining from the positive bodies on the symmetry plane are removed.
                    // Absolute tolerance here, would be perhaps better as a fraction of the triangle's edge lengths.  Comparison
                    // based on triangle center location, so it should be reliable.
                    mg->IgnoreYLessThan( -1e-5 );

                    // Purge ignored tris.
                    mg->FlattenTMeshVec();
                }

                mg->SubTagTris( subsFlag );
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

    fprintf( file_id, "# vspgeom v2\n" );

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
            mg->BuildIndexedMesh();
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
            mg->IdentifyWakes();
            offset = mg->WriteVSPGeomWakes( file_id, offset );

            mg->m_SurfDirty = true;
            mg->Update();
        }
    }

    offset = 0;
    int tcount = 1;
    //==== Dump alternate Tris ====//
    for ( i = 0; i < ( int ) geom_vec.size(); i++ )
    {
        if ( ( geom_vec[i]->GetSetFlag( write_set ) || geom_vec[i]->GetSetFlag( degen_set ) ) &&
             geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
        {
            MeshGeom *mg = ( MeshGeom * ) geom_vec[i];            // Cast
            offset = mg->WriteVSPGeomAlternateTris( file_id, offset, tcount );
        }
    }
    fclose( file_id );


    int ntagfile = 0;

    std::vector < int > partvec;
    SubSurfaceMgr.MakePartList( partvec );
    vector < SubSurface* > ssurfs = SubSurfaceMgr.GetSubSurfs();

    for ( int ipart = 0; ipart < partvec.size(); ipart++ )
    {
        int part = partvec[ ipart ];

        for ( int iss = 0; iss < ssurfs.size(); iss++ )
        {
            SubSurface *ssurf = ssurfs[iss];
            int tag = ssurf->m_Tag;

            if ( SubSurfaceMgr.ExistPartAndTag( part, tag ) )
            {
                ntagfile++;
            }
        }
    }

    if ( ntagfile > 0 )
    {
        string base_name = GetBasename( file_name );

        string base_path, base_fname;
        GetPathFile( base_name, base_path, base_fname );

        string taglist_name = base_name + ".taglist";

        FILE* taglist_fid = fopen( taglist_name.c_str(), "w" );
        if ( taglist_fid )
        {

            fprintf( taglist_fid, "%d\n", ntagfile );

            for ( int ipart = 0; ipart < partvec.size(); ipart++ )
            {
                int part = partvec[ ipart ];

                for ( int iss = 0; iss < ssurfs.size(); iss++ )
                {
                    SubSurface *ssurf = ssurfs[iss];
                    int tag = ssurf->m_Tag;

                    if ( SubSurfaceMgr.ExistPartAndTag( part, tag ) )
                    {
                        vector < int > parttag;
                        parttag.push_back( part );
                        parttag.push_back( tag );

                        string ptagname = SubSurfaceMgr.GetTagNames( parttag );

                        string tagfile_name = base_name + ptagname + ".tag";
                        string tagfile_localname = base_fname + ptagname + ".tag";

                        fprintf( taglist_fid, "%s\n", tagfile_localname.c_str() );

                        FILE* fid = fopen( tagfile_name.c_str(), "w" );
                        if ( fid )
                        {
                            int tri_offset = 0;
                            for ( i = 0; i < ( int ) geom_vec.size(); i++ )
                            {
                                if ( ( geom_vec[i]->GetSetFlag( write_set ) || geom_vec[i]->GetSetFlag( degen_set ) ) &&
                                     geom_vec[i]->GetType().m_Type == MESH_GEOM_TYPE )
                                {
                                    MeshGeom *mg = ( MeshGeom * ) geom_vec[i];            // Cast
                                    tri_offset = mg->WriteVSPGeomPartTagTris( fid, tri_offset, part, tag );
                                }
                            }

                            fclose( fid );
                        }
                    }
                }
            }

            fclose( taglist_fid );
        }
    }



    //==== Write Out tag key file ====//

    SubSurfaceMgr.WriteVSPGEOMKeyFile( file_name );

    vector < string > gidvec;
    vector < int > partvec2;
    vector < int > surfvec;
    SubSurfaceMgr.GetPartData( gidvec, partvec2, surfvec );


    WriteControlSurfaceFile( file_name, gidvec, partvec2, surfvec );

    return mesh_id;

}


//==== Write Nascart Files ====//
string Vehicle::WriteNascartFiles( const string & file_name, int write_set, int subsFlag, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

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
                mg->SubTagTris( subsFlag );
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
            mg->BuildIndexedMesh();
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
    std::string::size_type loc = key_name.find_last_of( '.' );
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

string Vehicle::WriteGmshFile( const string & file_name, int write_set, int subsFlag, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

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
                mg->SubTagTris( subsFlag );
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
            mg->BuildIndexedMesh();
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

void Vehicle::WriteX3DFile( const string & file_name, int write_set, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    if ( !geom_vec[0] )
    {
        return;
    }

    xmlDocPtr doc = xmlNewDoc( BAD_CAST "1.0" );

    xmlNodePtr root = xmlNewNode( nullptr, BAD_CAST "X3D" );
    xmlDocSetRootElement( doc, root );

    xmlNodePtr scene_node = xmlNewChild( root, nullptr, BAD_CAST "Scene", nullptr );

    WriteX3DViewpoints( scene_node );

    //==== All Geometry ====//
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) && geom_vec[i]->GetType().m_Type != BLANK_GEOM_TYPE && geom_vec[i]->GetType().m_Type != HINGE_GEOM_TYPE )
        {
            xmlNodePtr shape_node = xmlNewChild( scene_node, nullptr, BAD_CAST "Shape", nullptr );

            xmlNodePtr app_node = xmlNewChild( shape_node, nullptr, BAD_CAST "Appearance", nullptr );

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
        string orients, cents, posits, sfov;

        snprintf( numstr, sizeof( numstr ),  format4.c_str(),  rot_axis.x(), rot_axis.y(), rot_axis.z(), angle );
        orients = numstr;

        snprintf( numstr, sizeof( numstr ),  format3.c_str(), center.x(), center.y(), center.z() );
        cents = numstr;

        snprintf( numstr, sizeof( numstr ),  format3.c_str(), position.x(), position.y(), position.z() );
        posits = numstr;

        snprintf( numstr, sizeof( numstr ),  "%lf", fov );
        sfov = numstr;

        // write first viewpoint twice so viewpoint buttons will work correctly //
        if ( i == 0 )
        {
            xmlNodePtr first_view_node = xmlNewChild( node, nullptr, BAD_CAST "Viewpoint", BAD_CAST " " );
            WriteX3DViewpointProps( first_view_node, orients, cents, posits, sfov, string( "first" ) );
        }

        // write each viewpoint node's properties //
        xmlNodePtr viewpoint_node = xmlNewChild( node, nullptr, BAD_CAST "Viewpoint", BAD_CAST " " );
        WriteX3DViewpointProps( viewpoint_node, orients, cents, posits, sfov, x3d_views[i].c_str() );
    }
}

void Vehicle::WriteX3DViewpointProps( xmlNodePtr node, const string &orients, const string &cents, const string &posits, const string &sfov, const string &name )
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
    xmlNodePtr mat_node = xmlNewChild( node, nullptr, BAD_CAST "Material", BAD_CAST " " );

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
    snprintf( numstr, sizeof( numstr ),  "%lf", 1.0-alpha );
    xmlSetProp( mat_node, BAD_CAST "transparency", BAD_CAST numstr );

    double shine;
    material->GetShininess(shine);
    snprintf( numstr, sizeof( numstr ),  "%lf", shine );
    xmlSetProp( mat_node, BAD_CAST "shininess", BAD_CAST numstr );

    material->GetAmbient(amb);
    double ambd = 0.0;

    for( int i = 0; i < 3; i++ )
    {
        ambd += amb[i] / dif[i];
    }
    ambd = ambd / 3.0;



    snprintf( numstr, sizeof( numstr ),  "%lf", ambd );
    xmlSetProp( mat_node, BAD_CAST "ambientIntensity", BAD_CAST numstr );
}

void Vehicle::WritePovRayFile( const string & file_name, int write_set, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

    UpdateBBox();
    int i;

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    if ( !geom_vec[0] )
    {
        return;
    }

    //==== Figure Out Basename ====//
    string base_name = GetBasename( file_name );

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
            StringUtil::change_space_to_underscore( name );
            fprintf( pov_file, "mesh { %s_%d texture {darkgreymetal} } \n", name.c_str(), comp_num );
            comp_num++;
        }
    }

    fclose( pov_file );
}

void Vehicle::FetchXFerSurfs(int normal_set, int degen_set, vector< XferSurf > &xfersurfs )
{
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );

    int icomp = 0;
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        bool innormalset = geom_vec[i]->GetSetFlag(normal_set );
        bool indegenset = geom_vec[i]->GetSetFlag( degen_set );
        if( innormalset || indegenset )
        {
            int num_surf = geom_vec[i]->GetNumTotalSurfs();

            vector<VspSurf> surf_vec;
            const vector<VspSurf> *surf_vec_ptr = nullptr;

            if ( innormalset )
            {
                surf_vec_ptr = geom_vec[i]->GetSurfVecPtr();
            }
            else // indegenset
            {
                surf_vec = geom_vec[i]->GetDegenSurfVec();
                surf_vec_ptr = &surf_vec;
            }

            for ( int j = 0; j < num_surf; j++ )
            {
                (*surf_vec_ptr)[j].FetchXFerSurf( geom_vec[i]->GetID(), geom_vec[i]->GetName(), geom_vec[i]->GetMainSurfID( j ), icomp, j, xfersurfs );
                icomp++;
            }
        }
    }
}

void Vehicle::WriteSTEPFile( const string & file_name, int write_set, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

    WriteSTEPFile( file_name, write_set, m_STEPLabelID(), m_STEPLabelName(), m_STEPLabelSurfNo(), m_STEPLabelSplitNo(), m_STEPLabelAirfoilPart(), m_STEPLabelDelim() );
}

void Vehicle::WriteSTEPFile( const string & file_name, int write_set, bool labelID,
                             bool labelName, bool labelSurfNo, bool labelSplitNo, bool labelAirfoilPart, int delimType )
{
    string delim = StringUtil::get_delim( delimType );

    STEPutil step( m_STEPLenUnit(), m_STEPTol() );

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if( geom_vec[i]->GetSetFlag( write_set ) )
        {
            int num_surf = geom_vec[i]->GetNumTotalSurfs();

            for ( int j = 0; j < num_surf; j++ )
            {
                int mainid = geom_vec[i]->GetMainSurfID( j );
                const VspSurf *surf = geom_vec[i]->GetSurfPtr(j);

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
                            if( sub->m_MainSurfIndx() == -1 || sub->m_MainSurfIndx() == mainid )
                            {
                                if( sub->GetType() == vsp::SS_LINE )
                                {
                                    SSLine *subline = (SSLine*) sub;

                                    if( subline->m_ConstType() == vsp::CONST_U )
                                    {
                                        usplit.push_back( subline->m_ConstVal() * surf->GetUMax() );
                                    }
                                    else
                                    {
                                        wsplit.push_back( subline->m_ConstVal() * surf->GetWMax() );
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
                surf->ToSTEP_BSpline_Quilt( &step, surfs, prefix, m_STEPSplitSurfs(), m_STEPMergePoints(),
                                            m_STEPToCubic(), m_STEPToCubicTol(), m_STEPTrimTE(), m_STEPMergeLETE(), usplit, wsplit,
                                            labelSplitNo, labelAirfoilPart, delim );

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
            surf_vec[ j ].ToSTEP_BSpline_Quilt( &step, surfs, prefix, m_STEPStructureSplitSurfs(),
                                                m_STEPStructureMergePoints(), m_STEPStructureToCubic(),
                                                m_STEPStructureToCubicTol(), false, false, usplit, wsplit,
                                                m_STEPStructureLabelSplitNo(), m_STEPStructureLabelAirfoilPart(),
                                                delim );

            step.RepresentUntrimmedSurfs( surfs );
        }
    }

    step.WriteFile( file_name );
}

void Vehicle::WriteIGESFile( const string & file_name, int write_set, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

    WriteIGESFile( file_name, write_set, m_IGESLenUnit(), m_IGESSplitSubSurfs(), m_IGESSplitSurfs(), m_IGESToCubic(),
                   m_IGESToCubicTol(), m_IGESTrimTE(), m_IGESMergeLETE(), m_IGESLabelID(), m_IGESLabelName(), m_IGESLabelSurfNo(),
                   m_IGESLabelSplitNo(), m_IGESLabelAirfoilPart(), m_IGESLabelDelim());
}

void Vehicle::WriteIGESFile( const string &file_name, int write_set, int lenUnit, bool splitSubSurfs, bool splitSurfs,
                             bool toCubic, double toCubicTol, bool trimTE, bool mergeLETE, bool labelID, bool labelName,
                             bool labelSurfNo, bool labelSplitNo, bool labelAirfoilPart, int delimType )
{
    string delim = StringUtil::get_delim( delimType );

    IGESutil iges( lenUnit );

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if( geom_vec[i]->GetSetFlag( write_set ) )
        {
            int num_surf = geom_vec[i]->GetNumTotalSurfs();

            for ( int j = 0; j < num_surf; j++ )
            {
                const VspSurf *surf = geom_vec[i]->GetSurfPtr(j);
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
                            if( sub->m_MainSurfIndx() == -1 || sub->m_MainSurfIndx() == mainid )
                            {
                                if( sub->GetType() == vsp::SS_LINE )
                                {
                                    SSLine *subline = (SSLine*) sub;

                                    if( subline->m_ConstType() == vsp::CONST_U )
                                    {
                                        usplit.push_back( subline->m_ConstVal() * surf->GetUMax() );
                                    }
                                    else
                                    {
                                        wsplit.push_back( subline->m_ConstVal() * surf->GetWMax()  );
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

                surf->ToIGES( &iges, splitSurfs, toCubic, toCubicTol, trimTE, mergeLETE, usplit, wsplit, prefix,
                              labelSplitNo, labelAirfoilPart, delim );
            }
        }
    }

    iges.WriteFile( file_name.c_str(), true );
}

void Vehicle::WriteStructureIGESFile( const string & file_name )
{
    WriteStructureIGESFile( file_name, m_IGESStructureExportIndex(), m_IGESStructureSplitSurfs(), m_IGESStructureToCubic(),
                   m_IGESStructureToCubicTol(), m_IGESStructureLabelID(), m_IGESStructureLabelName(), m_IGESStructureLabelSurfNo(),
                   m_IGESStructureLabelSplitNo(), m_IGESStructureLabelAirfoilPart(), m_IGESStructureLabelDelim() );
}

void Vehicle::WriteStructureIGESFile( const string & file_name, int feaMeshStructIndex,
                             bool splitSurfs, bool toCubic, double toCubicTol, bool labelID,
                             bool labelName, bool labelSurfNo, bool labelSplitNo, bool labelAirfoilPart, int delimType )
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

            surf_vec[ j ].ToIGES( &iges, splitSurfs, toCubic, toCubicTol, false, false, usplit, wsplit, prefix,
                                  labelSplitNo, labelAirfoilPart, delim );
        }
    }

    iges.WriteFile( file_name.c_str(), true );
}

void Vehicle::WriteBEMFile( const string &file_name, int write_set, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            // write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

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

void Vehicle::WriteAirfoilFile( const string &file_name, int write_set, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

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
    }
    else
    {
        m_AFFileDir = PathToCWD();
    }
    fprintf( meta_fid, "Airfoil File Directory, %s\n\n", m_AFFileDir.c_str() );

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

void Vehicle::WriteDXFFile( const string & file_name, int write_set, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

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

void Vehicle::WriteSVGFile( const string & file_name, int write_set, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );

    if ( geom_vec.size() == 0 )
    {
        return;
    }

    xmlDocPtr doc = xmlNewDoc( ( const xmlChar * )"1.0" );

    xmlNodePtr root = xmlNewNode( nullptr, ( const xmlChar * )"svg" );
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

void Vehicle::WritePMARCFile( const string & file_name, int write_set, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            write_set = m->m_NormalSet();
            // degenset = m->m_DegenSet();
        }
    }

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
    vector < int > wstart( ntotal );
    vector < int > wend( ntotal );

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
            geom_vec[i]->WritePMARCGeomFile(fp, ipatch, idpat, wstart, wend);
        }
    }

    ipatch = 0;
    //==== Write surface boundary points ====//
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) )
        {
            geom_vec[i]->WritePMARCWakeFile(fp, ipatch, idpat, wstart, wend);
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

void Vehicle::WriteControlSurfaceFile( const string & file_name, const vector < string > &gidvec, const vector < int > &partvec, const vector < int > &surfvec )
{
    string base_name = GetBasename( file_name );
    string csf_name = base_name + ".csf";

    FILE* csf_file = fopen( csf_name.c_str(), "w" );

    if ( csf_file )
    {
        int ncsurf = 0;

        for ( int ipart = 0; ipart < ( int )gidvec.size(); ipart++ ) // Loop over all geoms.
        {
            string gid = gidvec[ipart];
            int part = partvec[ipart];
            int isurf = surfvec[ipart];

            vector< SubSurface* > ssvec = SubSurfaceMgr.GetSubSurfs( gid, isurf );

            int nss = ssvec.size();

            for ( int issurf = 0; issurf < nss; issurf++ ) // Loop over geom's subsurfaces.
            {
                SSControlSurf* cs = dynamic_cast< SSControlSurf* > ( ssvec[ issurf ] );
                if ( cs )                                  // Restrict to control surface subsurf.
                {
                    ncsurf++;
                }
            }
        }
        fprintf( csf_file, "%d Control Surfaces\n", ncsurf );

        for ( int ipart = 0; ipart < ( int )gidvec.size(); ipart++ ) // Loop over all geoms.
        {
            string gid = gidvec[ipart];
            int part = partvec[ipart];
            int isurf = surfvec[ipart];

            vector< SubSurface* > ssvec = SubSurfaceMgr.GetSubSurfs( gid, isurf );
            int nss = ssvec.size();


            Geom *g = FindGeom( gid );
            if ( g )
            {
                for ( int issurf = 0; issurf < nss; issurf++ ) // Loop over subsurfaces.
                {
                    SSControlSurf* cs = dynamic_cast< SSControlSurf* > ( ssvec[ issurf ] );
                    if ( cs )                                  // Restrict to control surface subsurf.
                    {
                        string gName = g->GetName();
                        double umax = g->GetUMax( isurf );
                        double wmax = g->GetWMax( isurf );

                        fprintf( csf_file, "CSurf ID %s, %s\n", cs->GetID().c_str(), cs->GetName().c_str() );
                        fprintf( csf_file, "Geom  ID %s, %s\n", g->GetID().c_str(), g->GetName().c_str() );
                        fprintf( csf_file, "Surface # %d\n", isurf );
                        fprintf( csf_file, "Part # %d\n", part );

                        if ( cs->m_SurfType() == SSControlSurf::UPPER_SURF )
                        {
                            fprintf( csf_file, "1 Upper\n" );
                        }
                        else if ( cs->m_SurfType() == SSControlSurf::LOWER_SURF )
                        {
                            fprintf( csf_file, "1 Lower\n" );
                        }
                        else
                        {
                            fprintf( csf_file, "2 Both\n" );
                        }

                        std::vector< std::vector< vec2d > > ppvec = cs->GetPolyPntsVec();

                        int nhinge = cs->m_UWStart.size();
                        int nbound = ppvec.size();

                        if ( nhinge != nbound )
                        {
                            printf( "Mismatch number of control surfaces\n" );
                        }

                        for ( int ihinge = 0; ihinge < nhinge; ihinge++ )
                        {
                            fprintf( csf_file, "2 Hinge UV\n" );
                            fprintf( csf_file, "%16.10g %16.10g\n", cs->m_UWStart[ihinge].x(), cs->m_UWStart[ihinge].y() );
                            fprintf( csf_file, "%16.10g %16.10g\n", cs->m_UWEnd[ihinge].x(), cs->m_UWEnd[ihinge].y() );

                            int nbndpt = ppvec[ihinge].size();

                            fprintf( csf_file, "%d Boundary UV\n", nbndpt );
                            for ( int j = 0; j < nbndpt; j++ )
                            {
                                fprintf( csf_file, "%16.10g %16.10g\n", ppvec[ ihinge ][ j ].x(), ppvec[ ihinge ][ j ].y() );
                            }
                        }

                        for ( int ihinge = 0; ihinge < nhinge; ihinge++ )
                        {
                            vec3d xStart = g->CompPnt01( isurf, clamp( cs->m_UWStart[ihinge].x(), 0.0, umax ) / umax, clamp( cs->m_UWStart[ihinge].y(), 0.0, wmax ) / wmax );
                            vec3d xEnd = g->CompPnt01( isurf, clamp( cs->m_UWEnd[ihinge].x(), 0.0, umax ) / umax, clamp( cs->m_UWEnd[ihinge].y(), 0.0, wmax ) / wmax );

                            fprintf( csf_file, "2 Hinge XYZ\n" );
                            fprintf( csf_file, "%16.10g %16.10g %16.10g\n", xStart.x(), xStart.y(), xStart.z() );
                            fprintf( csf_file, "%16.10g %16.10g %16.10g\n", xEnd.x(), xEnd.y(), xEnd.z() );

                            int nbndpt = ppvec[ihinge].size();

                            fprintf( csf_file, "%d Boundary XYZ\n", nbndpt );
                            for ( int j = 0; j < nbndpt; j++ )
                            {
                                vec3d x = g->CompPnt01( isurf, clamp( ppvec[ ihinge ][ j ].x(), 0.0, umax ) / umax, clamp( ppvec[ ihinge ][ j ].y(), 0.0, wmax ) / wmax );

                                fprintf( csf_file, "%16.10g %16.10g %16.10g\n", x.x(), x.y(), x.z() );
                            }
                        }

                    }
                }

            }
        }
    }

    fclose( csf_file );
}

vector< vector < vec3d > > Vehicle::GetVehProjectionLines( int view, const vec3d &offset )
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
    GeometryAnalysisMgr.AddLinkableContainers( linkable_container_vec );
}

void Vehicle::UpdateBBox()
{
    BndBox new_box, scale_independent_box;
    int ngeom;
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    ngeom = (int) geom_vec.size();
    for ( int i = 0 ; i < ngeom ; i++ )
    {
        new_box.Update( geom_vec[i]->GetBndBox() );
        scale_independent_box.Update( geom_vec[i]->GetScaleIndependentBndBox() ); // Without scale-dependent stuff
    }

    if( ngeom > 0 && ( new_box != m_BBox ) )
    {
        m_BBox = new_box;

        m_BbXLen = new_box.GetMax( 0 ) - new_box.GetMin( 0 );
        m_BbYLen = new_box.GetMax( 1 ) - new_box.GetMin( 1 );
        m_BbZLen = new_box.GetMax( 2 ) - new_box.GetMin( 2 );

        m_BbXMin = new_box.GetMin( 0 );
        m_BbYMin = new_box.GetMin( 1 );
        m_BbZMin = new_box.GetMin( 2 );
    }

    if( ngeom > 0 && ( scale_independent_box != m_ScaleIndependentBBox ) )
    {
        m_ScaleIndependentBBox = scale_independent_box;

        m_ScaleIndependentBbXLen = scale_independent_box.GetMax( 0 ) - scale_independent_box.GetMin( 0 );
        m_ScaleIndependentBbYLen = scale_independent_box.GetMax( 1 ) - scale_independent_box.GetMin( 1 );
        m_ScaleIndependentBbZLen = scale_independent_box.GetMax( 2 ) - scale_independent_box.GetMin( 2 );

        m_ScaleIndependentBbXMin = scale_independent_box.GetMin( 0 );
        m_ScaleIndependentBbYMin = scale_independent_box.GetMin( 1 );
        m_ScaleIndependentBbZMin = scale_independent_box.GetMin( 2 );

        for ( int i = 0 ; i < ngeom ; i++ )
        {
            // If so, loop through all Geoms, asking if they are sensitive to overall model scale.
            if ( geom_vec[i]->IsModelScaleSensitive() )
            {
                // If yes, then set dirty surface flag & trigger update.
                geom_vec[i]->SetDirtyFlag( GeomBase::SURF );
                geom_vec[i]->SetDirtyFlag( GeomBase::GLOBAL_SCALE );

                geom_vec[i]->Update();
            }
        }
    }
}

bool Vehicle::GetVisibleBndBox( BndBox &b )
{
    return GetBndBoxSet( vsp::SET_SHOWN, b );
}

bool Vehicle::GetBndBoxSet( int set, BndBox &b )
{
    b.Reset();

    int ngeom;
    vector< Geom* > geom_vec = FindGeomVec( GetGeomVec() );
    ngeom = (int) geom_vec.size();

    bool sethasmembers = false;

    for ( int i = 0 ; i < ngeom ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( set ) )
        {
            b.Update( geom_vec[i]->GetBndBox() );
            sethasmembers = true;
        }
    }

    return sethasmembers;
}

bool Vehicle::GetBndBoxSet( int set, double & xmin_out, double & ymin_out, double & zmin_out, double & xlen_out, double & ylen_out, double & zlen_out )
{
    BndBox bbox;
    bool sethasmembers = GetBndBoxSet( set, bbox );

    xlen_out = bbox.GetMax( 0 ) - bbox.GetMin( 0 );
    ylen_out = bbox.GetMax( 1 ) - bbox.GetMin( 1 );
    zlen_out = bbox.GetMax( 2 ) - bbox.GetMin( 2 );

    xmin_out = bbox.GetMin( 0 );
    ymin_out = bbox.GetMin( 1 );
    zmin_out = bbox.GetMin( 2 );

    return sethasmembers;
}

// As m_BBox, but without model scale dependent surfaces included.
bool Vehicle::GetScaleIndependentBBoxSet( int set, BndBox & bbox )
{
    vector<string> geom_vec = GetGeomVec();

    bool sethasmembers = false;
    for ( int i = 0 ; i < geom_vec.size() ; i++ )
    {
        Geom* g_ptr = FindGeom( geom_vec[i] );
        if ( g_ptr )
        {
            if ( g_ptr->GetSetFlag( set ) )
            {
                bbox.Update( g_ptr->GetScaleIndependentBndBox() );
                sethasmembers = true;
            }
        }
    }

    return sethasmembers;
}

bool Vehicle::GetScaleIndependentBBoxSet( int set, double & xmin_out, double & ymin_out, double & zmin_out, double & xlen_out, double & ylen_out, double & zlen_out )
{
    BndBox bbox;
    bool sethasmembers = GetScaleIndependentBBoxSet( set, bbox );

    xlen_out = bbox.GetMax( 0 ) - bbox.GetMin( 0 );
    ylen_out = bbox.GetMax( 1 ) - bbox.GetMin( 1 );
    zlen_out = bbox.GetMax( 2 ) - bbox.GetMin( 2 );

    xmin_out = bbox.GetMin( 0 );
    ymin_out = bbox.GetMin( 1 );
    zmin_out = bbox.GetMin( 2 );

    return sethasmembers;
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

void Vehicle::setExportFileName( int type, const string &f_name )
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

    string fname = m_VSP3FileName;
    int pos = fname.find( ".vsp3" );
    if ( pos >= 0 )
    {
        fname.erase( pos, fname.length() - 1 );
    }

    for( int i = 0; i < ntype; i++ )
    {
        m_ExportFileNames[types[i]] = fname;
        m_ExportFileNames[types[i]].append( suffix[i] );
    }
}

string Vehicle::CompGeom( int set, int degenset, int halfFlag, int intSubsFlag, bool hideset, bool suppressdisks, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            set = m->m_NormalSet();
            degenset = m->m_DegenSet();
        }
    }

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
    if ( mesh_ptr == nullptr )
    {
        return id;
    }

    if ( mesh_ptr->m_TMeshVec.size() )
    {
        vector< DegenGeom > dg;
        mesh_ptr->IntersectTrim( dg, false, intSubsFlag, halfFlag );
    }
    else
    {
        // Not sure if this is the best way to delete a mesh geom with no tMeshVec
        CutActiveGeomVec();
        DeleteClipBoard();
        id = "NONE";
    }

    return id;
}

string Vehicle::CompGeomAndFlatten( int set, int halfFlag, int intSubsFlag, int degenset, bool hideset, bool suppressdisks, bool useMode, const string &modeID )
{
    string id = CompGeom( set, degenset, halfFlag, intSubsFlag, hideset, suppressdisks, useMode, modeID );
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

string Vehicle::MassProps( int set, int degen_set, int numSlices, int idir, bool hidegeom, bool writefile, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            set = m->m_NormalSet();
            degen_set = m->m_DegenSet();
        }
    }

    string id = AddMeshGeom( set, degen_set );
    if ( id.compare( "NONE" ) == 0 )
    {
        return id;
    }

    MeshGeom* mesh_ptr = ( MeshGeom* )FindGeom( id );
    if ( mesh_ptr == nullptr )
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
                if ( geom_ptr->GetSetFlag( set ) )
                {
                    if ( geom_ptr->m_PointMass() != 0.0 )
                    {
                        vector <Matrix4d> tmv = geom_ptr->GetTransMatVec();

                        for ( int j = 0; j < tmv.size(); j++ )
                        {
                            TetraMassProp *pm = new TetraMassProp(); // Deleted by mesh_ptr

                            pm->SetDistributedMass( geom_ptr->m_PointMass(),
                                                    vec3d( geom_ptr->m_CGx(), geom_ptr->m_CGy(), geom_ptr->m_CGz()),
                                                    geom_ptr->m_Ixx(), geom_ptr->m_Iyy(), geom_ptr->m_Izz(),
                                                    geom_ptr->m_Ixy(), geom_ptr->m_Ixz(), geom_ptr->m_Iyz(), tmv[ j ] );
                            pm->m_CompId = geom_ptr->GetID();
                            pm->m_Name = geom_ptr->GetName() + "_pm";
                            mesh_ptr->AddPointMass( pm );
                        }

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
        vector <DegenGeom> dg;
        mesh_ptr->MassSlice( dg, false, numSlices, idir, writefile );
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

string Vehicle::MassPropsAndFlatten( int set, int degen_set, int numSlices, int idir, bool hidegeom, bool writefile, bool useMode, const string &modeID )
{
    DeleteGeom( m_LastMassMeshID );
    m_LastMassMeshID = MassProps( set, degen_set, numSlices, idir, hidegeom, writefile, useMode, modeID );
    Geom* geom = FindGeom( m_LastMassMeshID );
    if ( !geom )
    {
        m_LastMassMeshID = "NONE";
        return m_LastMassMeshID;
    }
    MeshGeom* mesh_ptr = ( MeshGeom* )geom;
    mesh_ptr->FlattenTMeshVec();
    mesh_ptr->FlattenSliceVec();
    mesh_ptr->m_SurfDirty = true;
    mesh_ptr->Update();
    return m_LastMassMeshID;
}

string Vehicle::PSlice( int set, int numSlices, const vec3d &axis, bool autoBoundsFlag, double start, double end, bool measureduct, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            set = m->m_NormalSet();
        }
    }

    string id = AddMeshGeom( set );
    if ( id.compare( "NONE" ) == 0 )
    {
        return id;
    }

    HideAllExcept( id );

    MeshGeom* mesh_ptr = ( MeshGeom* )FindGeom( id );
    if ( mesh_ptr == nullptr )
    {
        return id;
    }

    if ( mesh_ptr->m_TMeshVec.size() )
    {
        mesh_ptr->AreaSlice( numSlices, axis, autoBoundsFlag, start, end, measureduct );
    }
    else
    {
        CutActiveGeomVec();
        DeleteClipBoard();
        id = "NONE";
    }

    return id;
}

string Vehicle::PSliceAndFlatten( int set, int numSlices, const vec3d &axis, bool autoBoundsFlag, double start, double end, bool measureduct, bool useMode, const string &modeID )
{
    string id = PSlice( set, numSlices, axis, autoBoundsFlag, start, end, measureduct, useMode, modeID );
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
        if ( ( fp = fopen( file_name.c_str(), "r" ) ) == ( FILE * )nullptr )
        {
            return id;
        }

        //==== Read first Line of file and compare against expected header ====//
        fscanf( fp, "%255s INPUT FILE\n\n", str );
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
        if ( ( fp = fopen( file_name.c_str(), "r" ) ) == ( FILE * )nullptr )
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
    doc = xmlReadFile( file_name.c_str(), nullptr, XML_PARSE_HUGE );
    if ( doc == nullptr ) return 0;

    xmlNodePtr root = xmlDocGetRootElement( doc );
    if ( root == nullptr )
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
string Vehicle::ExportFile( const string & file_name, int write_set, int degen_set, int subsFlag, int file_type, bool useMode, const string &modeID )
{
    string mesh_id = string();

    if ( file_type == EXPORT_XSEC )
    {
        WriteXSecFile( file_name, write_set, useMode, modeID );
    }
    else if ( file_type == EXPORT_PLOT3D )
    {
        WritePLOT3DFile( file_name, write_set, useMode, modeID );
    }
    else if ( file_type == EXPORT_STL )
    {
        if ( m_STLExportPropMainSurf() )
        {
            SetExportPropMainSurf( true );
        }

        if ( !m_STLMultiSolid() )
        {
            mesh_id = WriteSTLFile( file_name, write_set, useMode, modeID );
        }
        else
        {
            mesh_id = WriteTaggedMSSTLFile( file_name, write_set, subsFlag, useMode, modeID );
        }

        if ( m_STLExportPropMainSurf() )
        {
            SetExportPropMainSurf( false );
        }
    }
    else if ( file_type == EXPORT_CART3D )
    {
        mesh_id = WriteTRIFile( file_name, write_set, subsFlag, useMode, modeID );
    }
    else if ( file_type == EXPORT_OBJ )
    {
        mesh_id = WriteOBJFile( file_name, write_set, subsFlag, useMode, modeID );
    }
    else if ( file_type == EXPORT_VSPGEOM )
    {
        mesh_id = WriteVSPGeomFile( file_name, write_set, degen_set, subsFlag, useMode, modeID );
    }
    else if ( file_type == EXPORT_NASCART )
    {
        mesh_id = WriteNascartFiles( file_name, write_set, subsFlag, useMode, modeID );
    }
    else if ( file_type == EXPORT_GMSH )
    {
        mesh_id = WriteGmshFile( file_name, write_set, subsFlag, useMode, modeID );
    }
    else if ( file_type == EXPORT_POVRAY )
    {
        WritePovRayFile( file_name, write_set, useMode, modeID );
    }
    else if ( file_type == EXPORT_X3D )
    {
        WriteX3DFile( file_name, write_set, useMode, modeID );
    }
    else if ( file_type == EXPORT_STEP )
    {
        if ( m_STEPExportPropMainSurf() )
        {
            SetExportPropMainSurf( true );
        }

        WriteSTEPFile( file_name, write_set, useMode, modeID );

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

        WriteIGESFile( file_name, write_set, useMode, modeID );

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
        WriteBEMFile( file_name, write_set, useMode, modeID );
    }
    else if ( file_type == EXPORT_DXF )
    {
        WriteDXFFile( file_name, write_set, useMode, modeID );
    }
    else if ( file_type == EXPORT_SVG )
    {
        WriteSVGFile( file_name, write_set, useMode, modeID );
    }
    else if ( file_type == EXPORT_FACET )
    {
        mesh_id = WriteFacetFile(file_name, write_set, subsFlag, useMode, modeID );
    }
    else if ( file_type == EXPORT_PMARC )
    {
        WritePMARCFile(file_name, write_set, useMode, modeID );
    }
    else if ( file_type == EXPORT_SELIG_AIRFOIL )
    {
        m_AFExportType.Set( vsp::SELIG_AF_EXPORT );
        WriteAirfoilFile( file_name, write_set, useMode, modeID );
    }
    else if ( file_type == EXPORT_BEZIER_AIRFOIL )
    {
        m_AFExportType.Set( vsp::BEZIER_AF_EXPORT );
        WriteAirfoilFile( file_name, write_set, useMode, modeID );
    }

    return mesh_id;
}

void Vehicle::CreateDegenGeom( int set, bool useMode, const string &modeID )
{
    if ( useMode )
    {
        Mode *m = ModeMgr.GetMode( modeID );
        if ( m )
        {
            m->ApplySettings();
            set = m->m_NormalSet();
        }
    }

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
                if( g->m_PointMass() != 0.0 )
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
                vector < DegenGeom > dgs;
                geom_vec[i]->CreateDegenGeom( dgs );
                // m_DegenGeomVec accumulates here.
                m_DegenGeomVec.insert( m_DegenGeomVec.end(), dgs.begin(), dgs.end() );
            }
        }
    }

    vector< string > active_vec_store = GetActiveGeomVec();

    string id = AddMeshGeom( set );
    if ( id.compare( "NONE" ) != 0 )
    {
        MeshGeom* mesh_ptr = dynamic_cast<MeshGeom*> ( FindGeom( id ) );
        if ( mesh_ptr != nullptr )
        {
            mesh_ptr->IntersectTrim( m_DegenGeomVec, true, 0, false );
            DeleteGeom( id );
        }
    }


    id = AddMeshGeom( set );
    if ( id.compare( "NONE" ) != 0 )
    {
        MeshGeom* mesh_ptr = dynamic_cast<MeshGeom*> ( FindGeom( id ) );
        if ( mesh_ptr != nullptr )
        {
            mesh_ptr->MassSlice( m_DegenGeomVec, true, 25, vsp::X_DIR, false );
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

    geomCnt = m_DegenGeomVec.size();

    blankCnt = m_DegenPtMassVec.size();

    char geomCntStr[255];
    snprintf( geomCntStr, sizeof( geomCntStr ), "%d components and %d", geomCnt, blankCnt);
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
    Results *res = ResultsMgr.CreateResults( "DegenGeom", "Vehicle level degen geom results." );
    vector < string > degen_results_ids;
    vector < string > blank_degen_result_ids;

    if ( blankCnt > 0 )
    {
        for ( int i = 0; i < ( int ) m_DegenPtMassVec.size(); i++ )
        {
            Results *blnk_res = ResultsMgr.CreateResults( "Degen_BlankGeom", "Blank component degen geom results." );
            blank_degen_result_ids.push_back( blnk_res->GetID() );

            blnk_res->Add( new NameValData( "name", m_DegenPtMassVec[i].name, "Name." ) );
            blnk_res->Add( new NameValData( "geom_id", m_DegenPtMassVec[i].geom_id, "GeomID." ) );
            blnk_res->Add( new NameValData( "X", m_DegenPtMassVec[i].x, "Coordinate." ) );
            blnk_res->Add( new NameValData( "mass", m_DegenPtMassVec[i].mass, "Mass." ) );
        }
    }

    for ( int i = 0; i < ( int )m_DegenGeomVec.size(); i++ )
    {
        m_DegenGeomVec[i].write_degenGeomResultsManager( degen_results_ids );
    }

    res->Add( new NameValData( "Degen_BlankGeoms", blank_degen_result_ids, "ID's of degen blanks." ) );
    res->Add( new NameValData( "Degen_DegenGeoms", degen_results_ids, "Believed unused." ) );  // TODO: check for removal.
    return outStr;
}

// Method to add pnts and normals to results managers for all surfaces
// in the selected set
string Vehicle::ExportSurfacePatches( int set )
{
    vector< string > geom_vec = GetGeomVec();

    Results* veh_surfaces = ResultsMgr.CreateResults( "VehicleSurfaces", "Vehicle level surface patch results." );
    vector< string > components;

    for ( int i = 0; i < (int)geom_vec.size(); i++ )
    {
        Geom* geom = FindGeom( geom_vec[i] );

        if ( geom )
        {
            if ( geom->GetSetFlag( set ) )
            {
                // Loop over all surfaces adding points to the results manager
                Results* res = ResultsMgr.CreateResults( "ComponentSurfaces", "Geom group for surface patch results." );
                res->Add( new NameValData( "name", geom->GetName(), "Geom name." ) );
                res->Add( new NameValData( "id", geom->GetID(), "GeomID." ) );

                vector< string > surfaces;
                geom->ExportSurfacePatches( surfaces  );

                res->Add( new NameValData( "surfaces", surfaces, "ID's of surface results.") );

                components.push_back( res->GetID() );
            }
        }
    }

    veh_surfaces->Add( new NameValData( "components", components, "ID's of component results." ) );
    return veh_surfaces->GetID();
}

double Vehicle::ComputeStructuresScaleFactor()
{
    if ( m_StructModelUnit() == vsp::LEN_UNITLESS )
    {
        return 1.0;
    }

    int to_unit = -1;
    switch ( m_StructUnit() )
    {
        case vsp::SI_UNIT:
            to_unit = vsp::LEN_M;
            break;

        case vsp::CGS_UNIT:
            to_unit = vsp::LEN_CM;
            break;

        case vsp::MPA_UNIT:
            to_unit = vsp::LEN_MM;
            break;

        case vsp::BFT_UNIT:
            to_unit = vsp::LEN_FT;
            break;

        case vsp::BIN_UNIT:
            to_unit = vsp::LEN_IN;
            break;

        default:
            return 1.0;
            break;
    }

    return ConvertLength( 1.0, m_StructModelUnit(), to_unit );
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
