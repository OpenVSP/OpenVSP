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
#include "SubSurfaceMgr.h"
#include "SubSurface.h"
#include "StructSettings.h"
#include "ParmMgr.h"

#ifdef DEBUG_CFD_MESH
#include <direct.h>
#endif

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

StructSettings::StructSettings() : ParmContainer()
{
    m_Name = "StructSettings";

    m_DrawMeshFlag.Init( "DrawMesh", "DrawFea", this, true, 0, 1 );
    m_DrawBadFlag.Init( "DrawBadMeshElements", "DrawFea", this, true, 0, 1 );
    m_ColorTagsFlag.Init( "ColorTagsFlag", "DrawFea", this, true, 0, 1 );

    m_IntersectSubSurfs.Init( "IntersectSubSurfs", "Global", this, true, 0, 1 );
    m_IntersectSubSurfs.SetDescript( "Flag to intersect subsurfaces" );

    m_HalfMeshFlag.Init( "HalfMesh", "FarField", this, false, 0, 1 );

    m_ExportFileFlags[ vsp::MASS_FILE_NAME ].Init( "MASS_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::NASTRAN_FILE_NAME ].Init( "NASTRAN_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::GEOM_FILE_NAME ].Init( "GEOM_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::THICK_FILE_NAME ].Init( "THICK_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::STL_FEA_NAME ].Init( "STL_Export", "ExportFEA", this, true, 0, 1 );

}

StructSettings::~StructSettings()
{
}

xmlNodePtr StructSettings::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr cfdsetnode = xmlNewChild( node, NULL, BAD_CAST"StructSettings", NULL );

    ParmContainer::EncodeXml( cfdsetnode );

    return cfdsetnode;
}

xmlNodePtr StructSettings::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr cfdsetnode = XmlUtil::GetNode( node, "StructSettings", 0 );
    if ( cfdsetnode )
    {
        ParmContainer::DecodeXml( cfdsetnode );
    }

    return cfdsetnode;
}

void StructSettings::ReadV2File( xmlNodePtr &root )
{
}

//==== Parm Changed ====//
void StructSettings::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
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
    const char *suffix[] = {"_mass.dat", "_NASTRAN.dat", "_calculix_geom.dat", "_calculix_thick.dat", ".stl" };

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
