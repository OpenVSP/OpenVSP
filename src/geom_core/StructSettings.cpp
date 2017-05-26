//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// StructSettings.cpp
//
//////////////////////////////////////////////////////////////////////

#include "Geom.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "Util.h"
#include "StructSettings.h"
#include "ParmMgr.h"

#ifdef DEBUG_CFD_MESH
#include <direct.h>
#endif

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

StructSettings::StructSettings() : MeshCommonSettings()
{
    m_Name = "StructSettings";

    m_ExportFileFlags[ vsp::MASS_FILE_NAME ].Init( "MASS_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::NASTRAN_FILE_NAME ].Init( "NASTRAN_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::CALCULIX_FILE_NAME ].Init( "CALCULIX_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::STL_FEA_NAME ].Init( "STL_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[vsp::GMSH_FEA_NAME].Init( "GMSH_Export", "ExportFEA", this, true, 0, 1 );

    InitCommonParms();

    SetFarCompFlag( false );
    SetFarMeshFlag( false );
    SetSymSplittingOnFlag( false );

    m_NumEvenlySpacedPart.Init( "NumEvenlySpacedPart", "StructSettings", this, 10, 0, 1000 );
    m_NumEvenlySpacedPart.SetDescript( "Number of Evenly Spaced FeaParts to Add" );

    m_MultiSliceSpacing.Init( "MultiSliceSpacing", "StructSettings", this, 20, 1e-8, 1e8 );
    m_MultiSliceSpacing.SetDescript( "Spacing Between Multiple Added Slices" );

    m_MultSliceIncludedElements.Init( "MultSliceIncludedElements", "StructSettings", this, TRIS, TRIS, BOTH_ELEMENTS );
    m_MultSliceIncludedElements.SetDescript( "Indicates the FeaElements to be Included for the Added Slices" );

    m_DrawFeaPartsFlag.Init( "DrawFeaPartsFlag", "FeaPart", this, true, false, true );
    m_DrawFeaPartsFlag.SetDescript( "Flag to Draw FeaParts" );

    m_DrawNodesFlag.Init( "DrawNodesFlag", "FeaPart", this, false, false, true );
    m_DrawNodesFlag.SetDescript( "Flag to Draw FeaNodes" );

    m_DrawElementOrientVecFlag.Init( "DrawElementOrientVecFlag", "FeaPart", this, false, false, true );
    m_DrawElementOrientVecFlag.SetDescript( "Flag to Draw FeaElement Orientation Vectors" );

    ResetExportFileNames();

    m_MultPropertyIndex = 0;
    m_MultCapPropertyIndex = 1;
}

StructSettings::~StructSettings()
{
}

string StructSettings::GetExportFileName( int type )
{
    if ( type >= 0 && type < vsp::NUM_FEA_FILE_NAMES )
    {
        return m_ExportFileNames[type];
    }

    return string();
}

void StructSettings::SetExportFileName( const string &fn, int type )
{
    if ( type >= 0 && type < vsp::NUM_FEA_FILE_NAMES )
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
    const char *suffix[] = {"_mass.dat", "_NASTRAN.dat", "_calculix.dat", ".stl", ".msh" };

    for ( int i = 0 ; i < vsp::NUM_FEA_FILE_NAMES ; i++ )
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
    assert( type >= 0 && type < vsp::NUM_FEA_FILE_NAMES );

    return &m_ExportFileFlags[type];
}

void StructSettings::SetAllFileExportFlags( bool flag )
{
    for ( int i = 0 ; i < vsp::NUM_FEA_FILE_NAMES ; i++ )
    {
        m_ExportFileFlags[i] = flag;
    }
}

void StructSettings::SetFileExportFlag( int type, bool flag )
{
    if ( type >= 0 && type < vsp::NUM_FEA_FILE_NAMES )
        m_ExportFileFlags[type] = flag;
}

vector < string > StructSettings::GetExportFileNames()
{
    vector < string > ret_vec;

    for ( size_t i = 0; i < vsp::NUM_FEA_FILE_NAMES; i++ )
    {
        ret_vec.push_back( m_ExportFileNames[i] );
    }
    return ret_vec;
}

