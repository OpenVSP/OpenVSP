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

}

SimpleMeshCommonSettings::~SimpleMeshCommonSettings()
{

}

void SimpleMeshCommonSettings::CopyFrom( MeshCommonSettings* settings )
{
    m_SymSplittingOnFlag = settings->m_SymSplittingOnFlag.Get();

    m_DrawMeshFlag = settings->m_DrawMeshFlag.Get();
    m_DrawBadFlag = settings->m_DrawBadFlag.Get();
    m_ColorTagsFlag = settings->m_ColorTagsFlag.Get();

    m_IntersectSubSurfs = settings->m_IntersectSubSurfs.Get();

    m_FarMeshFlag = settings->m_FarMeshFlag.Get();
    m_FarCompFlag = settings->m_FarCompFlag.Get();
    m_HalfMeshFlag = settings->m_HalfMeshFlag.Get();
}

//////////////////////////////////////////////////////
//============ SimpleCfdMeshSettings ===============//
//////////////////////////////////////////////////////

SimpleCfdMeshSettings::SimpleCfdMeshSettings()
{

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

    m_WakeScale = settings->m_WakeScale.Get();
    m_WakeAngle = settings->m_WakeAngle.Get();

    m_DrawSourceFlag = settings->m_DrawSourceFlag.Get();
    m_DrawFarFlag = settings->m_DrawFarFlag.Get();
    m_DrawFarPreFlag = settings->m_DrawFarPreFlag.Get();
    m_DrawSymmFlag = settings->m_DrawSymmFlag.Get();
    m_DrawWakeFlag = settings->m_DrawWakeFlag.Get();

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

    m_ExportFileNames = settings->GetExportFileNames();

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

double SimpleGridDensity::GetTargetLen( vec3d& pos, bool farFlag )
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
        double len = m_Sources[i]->GetTargetLen( base_len, pos );
        if ( len < target_len )
        {
            target_len = len;
        }
    }
    return target_len;
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


