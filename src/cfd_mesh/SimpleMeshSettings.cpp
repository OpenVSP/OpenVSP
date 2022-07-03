//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SimpleMeshSettings.cpp
//
// SimpleMeshSettings.cpp contains the simplified versions of MeshCommonSettings, StructSettings, CfdMeshSettings,
//  FeaGridDensity, and CfdGridDensity in an effort to reduce pointers and data transfer between cfd_mesh and geom_core. 
//
//////////////////////////////////////////////////////////////////////

#include "SimpleMeshSettings.h"

//////////////////////////////////////////////////////
//=========== SimpleMeshCommonSettings =============//
//////////////////////////////////////////////////////

SimpleMeshCommonSettings::SimpleMeshCommonSettings()
{
    m_SymSplittingOnFlag = false;

    m_DrawMeshFlag = false;
    m_ColorTagsFlag = false;

    m_DrawSourceWakeFlag = false;

    m_DrawBorderFlag = false;
    m_DrawIsectFlag = false;
    m_DrawRawFlag = false;
    m_DrawBinAdaptFlag = false;
    m_DrawCurveFlag = false;
    m_DrawPntsFlag = false;

    m_RelCurveTol = 1e-6;
    m_ExportRawFlag = false;
    m_ExportRelCurveTol = 1e-6;

    m_IntersectSubSurfs = true;

    m_DemoteSurfsCubicFlag = false;
    m_CubicSurfTolerance = 1e-6;

    m_ConvertToQuadsFlag = false;
    m_HighOrderElementFlag = false;

    m_FarMeshFlag = false;
    m_FarCompFlag = false;
    m_HalfMeshFlag = false;

    m_SelectedSetIndex = 0;

    m_XYZIntCurveFlag = false;

    m_STEPTol = 1e-6;
    m_STEPMergePoints = false;
    m_STEPRepresentation = 0;

    m_CADLenUnit = 0;
    m_CADLabelID = false;
    m_CADLabelName = false;
    m_CADLabelSurfNo = false;
    m_CADLabelDelim = 0;
    m_CADLabelSplitNo = false;
}

SimpleMeshCommonSettings::~SimpleMeshCommonSettings()
{

}

void SimpleMeshCommonSettings::CopyFrom( MeshCommonSettings* settings )
{
    m_SymSplittingOnFlag = settings->m_SymSplittingOnFlag.Get();

    m_DrawMeshFlag = settings->m_DrawMeshFlag.Get();
    m_ColorTagsFlag = settings->m_ColorTagsFlag.Get();

    m_DrawSourceWakeFlag = settings->m_DrawSourceWakeFlag.Get();

    m_DrawBorderFlag = settings->m_DrawBorderFlag.Get();
    m_DrawIsectFlag = settings->m_DrawIsectFlag.Get();
    m_DrawRawFlag = settings->m_DrawRawFlag.Get();
    m_DrawBinAdaptFlag = settings->m_DrawBinAdaptFlag.Get();
    m_DrawCurveFlag = settings->m_DrawCurveFlag.Get();
    m_DrawPntsFlag = settings->m_DrawPntsFlag.Get();

    m_RelCurveTol = settings->m_RelCurveTol.Get();

    m_ExportRawFlag = settings->m_ExportRawFlag.Get();

    m_IntersectSubSurfs = settings->m_IntersectSubSurfs.Get();

    m_DemoteSurfsCubicFlag = settings->m_DemoteSurfsCubicFlag.Get();
    m_CubicSurfTolerance = settings->m_CubicSurfTolerance.Get();

    m_ConvertToQuadsFlag = settings->m_ConvertToQuadsFlag.Get();
    m_HighOrderElementFlag = settings->m_HighOrderElementFlag.Get();

    m_FarMeshFlag = settings->m_FarMeshFlag.Get();
    m_FarCompFlag = settings->m_FarCompFlag.Get();
    m_HalfMeshFlag = settings->m_HalfMeshFlag.Get();
}

//////////////////////////////////////////////////////
//============ SimpleIntersectSettings ===============//
//////////////////////////////////////////////////////

SimpleIntersectSettings::SimpleIntersectSettings()
{

}

SimpleIntersectSettings::~SimpleIntersectSettings()
{

}

void SimpleIntersectSettings::CopyFrom( IntersectSettings* settings )
{

    m_ExportFileFlags.clear();
    m_ExportFileFlags.resize( vsp::INTERSECT_NUM_FILE_NAMES );

    for ( size_t i = 0; i < vsp::INTERSECT_NUM_FILE_NAMES; i++ )
    {
        m_ExportFileFlags[i] = settings->m_ExportFileFlags[i].Get();
    }

    m_ExportFileNames = settings->GetExportFileNames();

    m_SelectedSetIndex = settings->m_SelectedSetIndex.Get();

    m_STEPTol = settings->m_STEPTol.Get();
    m_STEPMergePoints = settings->m_STEPMergePoints.Get();
    m_STEPRepresentation = settings->m_STEPRepresentation.Get();

    m_CADLenUnit = settings->m_CADLenUnit.Get();
    m_CADLabelID = settings->m_CADLabelID.Get();
    m_CADLabelName = settings->m_CADLabelName.Get();
    m_CADLabelSurfNo = settings->m_CADLabelSurfNo.Get();
    m_CADLabelDelim = settings->m_CADLabelDelim.Get();
    m_CADLabelSplitNo = settings->m_CADLabelSplitNo.Get();

    SimpleMeshCommonSettings::CopyFrom( settings );
}

string SimpleIntersectSettings::GetExportFileName( int type )
{
    if ( type >= 0 && type < vsp::INTERSECT_NUM_FILE_NAMES )
    {
        return m_ExportFileNames[type];
    }

    return string();
}

bool SimpleIntersectSettings::GetExportFileFlag( int type )
{
    assert( type >= 0 && type < vsp::INTERSECT_NUM_FILE_NAMES );

    return m_ExportFileFlags[type];
}

//////////////////////////////////////////////////////
//============ SimpleCfdMeshSettings ===============//
//////////////////////////////////////////////////////

SimpleCfdMeshSettings::SimpleCfdMeshSettings()
{
    m_FarManLocFlag = false;
    m_FarAbsSizeFlag = false;

    m_FarGeomID = "";

    m_FarXScale = 1;
    m_FarYScale = 1;
    m_FarZScale = 1;

    m_FarLength = 1;
    m_FarWidth = 1;
    m_FarHeight = 1;

    m_FarXLocation = 0;
    m_FarYLocation = 0;
    m_FarZLocation = 0;

    m_DrawFarFlag = false;
    m_DrawFarPreFlag = false;
    m_DrawSymmFlag = false;
    m_DrawWakeFlag = false;
    m_DrawBadFlag = false;
}

SimpleCfdMeshSettings::~SimpleCfdMeshSettings()
{

}

void SimpleCfdMeshSettings::CopyFrom( CfdMeshSettings* settings )
{
    m_FarManLocFlag = settings->m_FarManLocFlag.Get();
    m_FarAbsSizeFlag = settings->m_FarAbsSizeFlag.Get();

    m_FarGeomID = settings->m_FarGeomID;

    m_FarXScale = settings->m_FarXScale.Get();
    m_FarYScale = settings->m_FarYScale.Get();
    m_FarZScale = settings->m_FarZScale.Get();

    m_FarLength = settings->m_FarLength.Get();
    m_FarWidth = settings->m_FarWidth.Get();
    m_FarHeight = settings->m_FarHeight.Get();

    m_FarXLocation = settings->m_FarXLocation.Get();
    m_FarYLocation = settings->m_FarYLocation.Get();
    m_FarZLocation = settings->m_FarZLocation.Get();

    m_DrawFarFlag = settings->m_DrawFarFlag.Get();
    m_DrawFarPreFlag = settings->m_DrawFarPreFlag.Get();
    m_DrawSymmFlag = settings->m_DrawSymmFlag.Get();
    m_DrawWakeFlag = settings->m_DrawWakeFlag.Get();
    m_DrawBadFlag = settings->m_DrawBadFlag.Get();

    m_SelectedSetIndex = settings->m_SelectedSetIndex.Get();

    m_ExportFileFlags.clear();
    m_ExportFileFlags.resize( vsp::CFD_NUM_FILE_NAMES );

    for ( size_t i = 0; i < vsp::CFD_NUM_FILE_NAMES; i++ )
    {
        m_ExportFileFlags[i] = settings->m_ExportFileFlags[i].Get();
    }

    m_XYZIntCurveFlag = settings->m_XYZIntCurveFlag.Get();

    m_ExportFileNames = settings->GetExportFileNames();

    SimpleMeshCommonSettings::CopyFrom( settings );
}

string SimpleCfdMeshSettings::GetExportFileName( int type )
{
    if ( type >= 0 && type < vsp::CFD_NUM_FILE_NAMES )
    {
        return m_ExportFileNames[type];
    }

    return string();
}

bool SimpleCfdMeshSettings::GetExportFileFlag( int type )
{
    assert( type >= 0 && type < vsp::CFD_NUM_FILE_NAMES );

    return m_ExportFileFlags[type];
}

//////////////////////////////////////////////////////
//============ SimpleFeaMeshSettings ===============//
//////////////////////////////////////////////////////

SimpleFeaMeshSettings::SimpleFeaMeshSettings()
{
    m_NumEvenlySpacedPart = 0;
    m_DrawNodesFlag = false;
    m_DrawElementOrientVecFlag = false;
    m_XYZIntCurveFlag = false;

    m_NodeOffset = 0;
    m_ElementOffset = 0;
}

SimpleFeaMeshSettings::~SimpleFeaMeshSettings()
{

}

void SimpleFeaMeshSettings::CopyFrom( StructSettings* settings )
{
    m_ExportFileFlags.clear();
    m_ExportFileFlags.resize( vsp::FEA_NUM_FILE_NAMES );

    for ( size_t i = 0; i < vsp::FEA_NUM_FILE_NAMES; i++ )
    {
        m_ExportFileFlags[i] = settings->m_ExportFileFlags[i].Get();
    }

    m_NumEvenlySpacedPart = settings->m_NumEvenlySpacedPart.Get();
    m_DrawNodesFlag = settings->m_DrawNodesFlag.Get();
    m_DrawElementOrientVecFlag = settings->m_DrawElementOrientVecFlag.Get();

    m_NodeOffset = settings->m_NodeOffset.Get();
    m_ElementOffset = settings->m_ElementOffset.Get();

    m_XYZIntCurveFlag = settings->m_XYZIntCurveFlag.Get();

    m_ExportFileNames = settings->GetExportFileNames();

    m_STEPTol = settings->m_STEPTol.Get();
    m_STEPMergePoints = settings->m_STEPMergePoints.Get();
    m_STEPRepresentation = settings->m_STEPRepresentation.Get();

    m_CADLenUnit = settings->m_CADLenUnit.Get();
    m_CADLabelID = settings->m_CADLabelID.Get();
    m_CADLabelName = settings->m_CADLabelName.Get();
    m_CADLabelSurfNo = settings->m_CADLabelSurfNo.Get();
    m_CADLabelDelim = settings->m_CADLabelDelim.Get();
    m_CADLabelSplitNo = settings->m_CADLabelSplitNo.Get();

    SimpleMeshCommonSettings::CopyFrom( settings );
}

string SimpleFeaMeshSettings::GetExportFileName( int type )
{
    if ( type >= 0 && type < vsp::FEA_NUM_FILE_NAMES )
    {
        return m_ExportFileNames[type];
    }

    return string();
}

bool SimpleFeaMeshSettings::GetExportFileFlag( int type )
{
    assert( type >= 0 && type < vsp::FEA_NUM_FILE_NAMES );

    return m_ExportFileFlags[type];
}

//////////////////////////////////////////////////////
//============== SimpleGridDensity =================//
//////////////////////////////////////////////////////

SimpleGridDensity::SimpleGridDensity()
{
    m_RigorLimit = false;
    m_BaseLen = 1;
    m_FarMaxLen = 1;
    m_MinLen = 0;
    m_NCircSeg = 1;
    m_FarNCircSeg = 1;
    m_MaxGap = 1;
    m_FarMaxGap = 1;
    m_GrowRatio = 1;
}

SimpleGridDensity::~SimpleGridDensity()
{

}

void SimpleGridDensity::CopyFrom( GridDensity* gd )
{
    m_RigorLimit = gd->m_RigorLimit.Get();
    m_BaseLen = gd->m_BaseLen.Get();
    m_FarMaxLen = gd->m_FarMaxLen.Get();
    m_MinLen = gd->m_MinLen.Get();
    m_NCircSeg = gd->m_NCircSeg.Get();
    m_FarNCircSeg = gd->m_FarNCircSeg.Get();
    m_MaxGap = gd->m_MaxGap.Get();
    m_FarMaxGap = gd->m_FarMaxGap.Get();
    m_GrowRatio = gd->m_GrowRatio.Get();
    m_Sources = gd->GetSimpleSourceVec();
}

double SimpleGridDensity::GetRadFrac( bool farflag )
{
    if ( !farflag )
    {
        double radFrac;

        if ( m_NCircSeg > 2.0 )
        {
            radFrac = 2.0 * sin( PI / m_NCircSeg );
        }
        else  // Switch to 4/n behavior below well defined range.
        {
            radFrac = 4.0 / m_NCircSeg;
        }

        return radFrac;
    }
    else
    {
        return GetFarRadFrac();
    }
}

double SimpleGridDensity::GetFarRadFrac()
{
    double radFrac;

    if ( m_FarNCircSeg > 2.0 )
    {
        radFrac = 2.0 * sin( PI / m_FarNCircSeg );
    }
    else  // Switch to 4/n behavior below well defined range.
    {
        radFrac = 4.0 / m_FarNCircSeg;
    }

    return radFrac;
}

double SimpleGridDensity::GetTargetLen( vec3d& pos, bool farFlag, const string & geomid, const int & surfindx, const double & u, const double & w )
{
    double target_len;
    double base_len;

    if ( !farFlag )
    {
        target_len = m_BaseLen;
    }
    else
    {
        target_len = m_FarMaxLen;
    }
    base_len = target_len;

    for ( int i = 0; i < (int)m_Sources.size(); i++ )
    {
        double len = m_Sources[i]->GetTargetLen( base_len, pos, geomid, surfindx, u, w );
        if ( len < target_len )
        {
            target_len = len;
        }
    }
    return target_len;
}

void SimpleGridDensity::ScaleMesh( double scale )
{
    m_BaseLen *= scale;
    m_FarMaxLen *= scale;
    m_MinLen *= scale;
    m_NCircSeg /= scale;
    m_FarNCircSeg /= scale;
    m_MaxGap *= scale;
    m_FarMaxGap *= scale;
    // m_GrowRatio;

    ScaleAllSources( scale );
}

void SimpleGridDensity::ScaleAllSources( double scale )
{
    for ( int i = 0; i < (int)m_Sources.size(); i++ )
    {
        m_Sources[i]->AdjustLen( scale );
    }
}

void SimpleGridDensity::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    for ( int i = 0; i < (int)m_Sources.size(); i++ )
    {
        m_Sources[i]->LoadDrawObjs( draw_obj_vec );
    }
}

void SimpleGridDensity::Show( bool flag )
{
    for ( int i = 0; i < (int)m_Sources.size(); i++ )
    {
        m_Sources[i]->Show( flag );
    }
}

void SimpleGridDensity::Highlight( BaseSource * source )
{
    if ( source )
    {
        for ( int i = 0; i < (int)m_Sources.size(); i++ )
        {
            if ( m_Sources[i]->m_OrigSourceID == source->GetID() )
            {
                m_Sources[i]->Highlight( true );
            }
            else
            {
                m_Sources[i]->Highlight( false );
            }
        }
    }
}

//////////////////////////////////////////////////////
//============= SimpleCfdGridDensity ===============//
//////////////////////////////////////////////////////

SimpleCfdGridDensity::SimpleCfdGridDensity()
{

}

SimpleCfdGridDensity::~SimpleCfdGridDensity()
{

}


//////////////////////////////////////////////////////
//============= SimpleFeaGridDensity ===============//
//////////////////////////////////////////////////////

SimpleFeaGridDensity::SimpleFeaGridDensity()
{

}

SimpleFeaGridDensity::~SimpleFeaGridDensity()
{

}


