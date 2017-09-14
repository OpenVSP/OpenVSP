//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// StructSettings.cpp
//
//////////////////////////////////////////////////////////////////////

#include "Vehicle.h"
#include "StructSettings.h"

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

StructSettings::StructSettings() : MeshCommonSettings()
{
    m_Name = "StructSettings";

    m_ExportFileFlags[ vsp::FEA_MASS_FILE_NAME ].Init( "MASS_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::FEA_NASTRAN_FILE_NAME ].Init( "NASTRAN_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::FEA_CALCULIX_FILE_NAME ].Init( "CALCULIX_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::FEA_STL_FILE_NAME ].Init( "STL_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[vsp::FEA_GMSH_FILE_NAME].Init( "GMSH_Export", "ExportFEA", this, true, 0, 1 );

    InitCommonParms();

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

    m_MultPropertyIndex = 0;
    m_MultCapPropertyIndex = 1;
}

StructSettings::~StructSettings()
{
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
    const char *suffix[] = {"_mass.txt", "_NASTRAN.dat", "_calculix.dat", ".stl", ".msh" };

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

