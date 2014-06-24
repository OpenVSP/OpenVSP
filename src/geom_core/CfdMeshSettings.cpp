//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CfdMeshSettings.cpp
//
//////////////////////////////////////////////////////////////////////

#include "Geom.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "Util.h"
#include "SubSurfaceMgr.h"
#include "SubSurface.h"
#include "CfdMeshSettings.h"

#ifdef DEBUG_CFD_MESH
#include <direct.h>
#endif

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

CfdMeshSettings::CfdMeshSettings() : ParmContainer()
{
    m_DrawMeshFlag.Init( "Draw Mesh", "DrawCFD", this, true, 0, 1 );
    m_DrawSourceFlag.Init( "Draw Source & Wake", "DrawCFD", this, true, 0, 1 );
    m_DrawFarFlag.Init( "Draw Far Field", "DrawCFD", this, true, 0, 1 );
    m_DrawFarPreFlag.Init( "Draw Far Field Preview", "DrawCFD", this, true, 0, 1 );
    m_DrawBadFlag.Init( "Draw Bad Mesh Elements", "DrawCFD", this, true, 0, 1 );
    m_DrawSymmFlag.Init( "Draw Symmetry Plane", "DrawCFD", this, true, 0, 1 );
    m_DrawWakeFlag.Init( "Draw Wake", "DrawCFD", this, true, 0, 1 );
    m_ColorTagsFlag.Init( "Color Tags Flag", "DrawCFD", this, true, 0, 1 );

    m_HalfMeshFlag = false;
    m_FarCompFlag = false;
    m_FarMeshFlag = false;
    m_FarManLocFlag = false;
    m_FarAbsSizeFlag = false;

    m_FarXScale.Init( "Far X Scale", "FarField", this, 4.0, 1.0, 1.0e12 );
    m_FarXScale.SetDescript( "Far field X scale" );

    m_FarYScale.Init( "Far Y Scale", "FarField", this, 4.0, 1.0, 1.0e12 );
    m_FarYScale.SetDescript( "Far field Y scale" );

    m_FarZScale.Init( "Far Z Scale", "FarField", this, 4.0, 1.0, 1.0e12 );
    m_FarZScale.SetDescript( "Far field Z scale" );

    m_FarXLocation.Init( "Far X Location", "FarField", this, 0.0, -1.0e12, 1.0e12 );
    m_FarXLocation.SetDescript( "Far field X location" );

    m_FarYLocation.Init( "Far Y Location", "FarField", this, 0.0, -1.0e12, 1.0e12 );
    m_FarYLocation.SetDescript( "Far field Y location" );

    m_FarZLocation.Init( "Far Z Location", "FarField", this, 0.0, -1.0e12, 1.0e12 );
    m_FarZLocation.SetDescript( "Far field Z location" );

    m_WakeScale.Init( "Wake Scale", "Wake", this, 2.0, 1.0, 1.0e12 );
    m_WakeScale.SetDescript( "Wake length scale" );

    m_WakeAngle.Init( "Wake Angle", "Wake", this, 0.0, -90.0, 90.0 );
    m_WakeAngle.SetDescript( "Wake angle" );

    m_ExportFileFlags[ CfdMeshSettings::DAT_FILE_NAME ].Init( "DAT Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ CfdMeshSettings::KEY_FILE_NAME ].Init( "KEY Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ CfdMeshSettings::OBJ_FILE_NAME ].Init( "OBJ Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ CfdMeshSettings::POLY_FILE_NAME ].Init( "POLY Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ CfdMeshSettings::STL_FILE_NAME ].Init( "STL Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ CfdMeshSettings::TRI_FILE_NAME ].Init( "TRI Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ CfdMeshSettings::GMSH_FILE_NAME ].Init( "GMSH Export", "ExportCFD", this, true, 0, 1 );
    m_ExportFileFlags[ CfdMeshSettings::SRF_FILE_NAME ].Init( "SRF Export", "ExportCFD", this, true, 0, 1 );
}

CfdMeshSettings::~CfdMeshSettings()
{
}

//==== Parm Changed ====//
void CfdMeshSettings::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr::getInstance().GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, Parm::SET );
    }
}

string CfdMeshSettings::GetExportFileName( int type )
{
    if ( type >= 0 && type < NUM_FILE_NAMES )
    {
        return m_ExportFileNames[type];
    }

    return string();
}

void CfdMeshSettings::SetExportFileName( const string &fn, int type )
{
    if ( type >= 0 && type < NUM_FILE_NAMES )
    {
        m_ExportFileNames[type] = fn;
    }
}

void CfdMeshSettings::ResetExportFileNames()
{
    Vehicle* veh = VehicleMgr::getInstance().GetVehicle();
    if ( veh )
    {
        ResetExportFileNames( veh->GetVSP3FileName() );
    }
}

void CfdMeshSettings::ResetExportFileNames( string basename )
{
    int pos;
    const char *suffix[] = {".stl", ".poly", ".tri", ".obj", "_NASCART.dat", "_NASCART.key", ".msh", ".srf"};

    for ( int i = 0 ; i < NUM_FILE_NAMES ; i++ )
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

BoolParm CfdMeshSettings::GetExportFileFlag( int type )
{
    assert( type >= 0 && type < NUM_FILE_NAMES );

    return m_ExportFileFlags[type];
}
