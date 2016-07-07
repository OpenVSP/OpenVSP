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

    m_ExportFileFlags[ vsp::MASS_FILE_NAME ].Init( "MASS_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::NASTRAN_FILE_NAME ].Init( "NASTRAN_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::GEOM_FILE_NAME ].Init( "GEOM_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::THICK_FILE_NAME ].Init( "THICK_Export", "ExportFEA", this, true, 0, 1 );
    m_ExportFileFlags[ vsp::STL_FEA_NAME ].Init( "STL_Export", "ExportFEA", this, true, 0, 1 );

    m_XYZIntCurveFlag.Init( "SRF_XYZIntCurve", "ExportCFD", this, false, 0, 1 );
}

StructSettings::~StructSettings()
{
}

xmlNodePtr StructSettings::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr cfdsetnode = xmlNewChild( node, NULL, BAD_CAST"StructSettings", NULL );

    ParmContainer::EncodeXml( cfdsetnode );

    XmlUtil::AddStringNode( cfdsetnode, "FarFieldGeomID", m_FarGeomID );

    return cfdsetnode;
}

xmlNodePtr StructSettings::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr cfdsetnode = XmlUtil::GetNode( node, "StructSettings", 0 );
    if ( cfdsetnode )
    {
        ParmContainer::DecodeXml( cfdsetnode );
        m_FarGeomID   = XmlUtil::FindString( cfdsetnode, "FarFieldGeomID", m_FarGeomID );
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
