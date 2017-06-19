//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROMgr.cpp: VSPAERO Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "APIDefines.h"
#include "LinkMgr.h"
#include "MeshGeom.h"
#include "ParmMgr.h"
#include "StlHelper.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "VSPAEROMgr.h"
#include "WingGeom.h"

#include "StringUtil.h"
#include "FileUtil.h"

#include <regex>

//==== Constructor ====//
VspAeroControlSurf::VspAeroControlSurf()
{
    isGrouped = false;
}

//==== Constructor ====//
VSPAEROMgrSingleton::VSPAEROMgrSingleton() : ParmContainer()
{
    m_Name = "VSPAEROSettings";
    string groupname = "VSPAERO";

    m_GeomSet.Init( "GeomSet", groupname, this, 0, 0, 12 );
    m_GeomSet.SetDescript( "Geometry set" );

    m_AnalysisMethod.Init( "AnalysisMethod", groupname, this, vsp::VORTEX_LATTICE, vsp::VORTEX_LATTICE, vsp::PANEL );
    m_AnalysisMethod.SetDescript( "Analysis method: 0=VLM, 1=Panel" );

    m_LastPanelMeshGeomId = string();

    m_Sref.Init( "Sref", groupname, this, 100.0, 0.0, 1e12 );
    m_Sref.SetDescript( "Reference area" );

    m_bref.Init( "bref", groupname, this, 1.0, 0.0, 1e6 );
    m_bref.SetDescript( "Reference span" );

    m_cref.Init( "cref", groupname, this, 1.0, 0.0, 1e6 );
    m_cref.SetDescript( "Reference chord" );

    m_RefFlag.Init( "RefFlag", groupname, this, vsp::MANUAL_REF, 0, vsp::NUM_REF_TYPES - 1 );
    m_RefFlag.SetDescript( "Reference quantity flag" );

    m_CGGeomSet.Init( "MassSet", groupname, this, 0, 0, 12 );
    m_CGGeomSet.SetDescript( "Mass property set" );

    m_NumMassSlice.Init( "NumMassSlice", groupname, this, 10, 10, 200 );
    m_NumMassSlice.SetDescript( "Number of mass property slices" );

    m_Xcg.Init( "Xcg", groupname, this, 0.0, -1.0e12, 1.0e12 );
    m_Xcg.SetDescript( "X Center of Gravity" );

    m_Ycg.Init( "Ycg", groupname, this, 0.0, -1.0e12, 1.0e12 );
    m_Ycg.SetDescript( "Y Center of Gravity" );

    m_Zcg.Init( "Zcg", groupname, this, 0.0, -1.0e12, 1.0e12 );
    m_Zcg.SetDescript( "Z Center of Gravity" );


    // Flow Condition
    m_AlphaStart.Init( "AlphaStart", groupname, this, 1.0, -180, 180 );
    m_AlphaStart.SetDescript( "Angle of attack (Start)" );
    m_AlphaEnd.Init( "AlphaEnd", groupname, this, 10.0, -180, 180 );
    m_AlphaEnd.SetDescript( "Angle of attack (End)" );
    m_AlphaNpts.Init( "AlphaNpts", groupname, this, 3, 1, 100 );
    m_AlphaNpts.SetDescript( "Angle of attack (Num Points)" );

    m_BetaStart.Init( "BetaStart", groupname, this, 0.0, -180, 180 );
    m_BetaStart.SetDescript( "Angle of sideslip (Start)" );
    m_BetaEnd.Init( "BetaEnd", groupname, this, 0.0, -180, 180 );
    m_BetaEnd.SetDescript( "Angle of sideslip (End)" );
    m_BetaNpts.Init( "BetaNpts", groupname, this, 1, 1, 100 );
    m_BetaNpts.SetDescript( "Angle of sideslip (Num Points)" );

    m_MachStart.Init( "MachStart", groupname, this, 0.0, 0.0, 5.0 );
    m_MachStart.SetDescript( "Freestream Mach number (Start)" );
    m_MachEnd.Init( "MachEnd", groupname, this, 0.0, 0.0, 5.0 );
    m_MachEnd.SetDescript( "Freestream Mach number (End)" );
    m_MachNpts.Init( "MachNpts", groupname, this, 1, 1, 100 );
    m_MachNpts.SetDescript( "Freestream Mach number (Num Points)" );


    // Case Setup
    m_NCPU.Init( "NCPU", groupname, this, 4, 1, 255 );
    m_NCPU.SetDescript( "Number of processors to use" );

    //    wake parameters
    m_WakeNumIter.Init( "WakeNumIter", groupname, this, 5, 1, 255 );
    m_WakeNumIter.SetDescript( "Number of wake iterations to execute, Default = 5" );
    m_WakeAvgStartIter.Init( "WakeAvgStartIter", groupname, this, 0, 0, 255 );
    m_WakeAvgStartIter.SetDescript( "Iteration at which to START averaging the wake. Default=0 --> No wake averaging" );
    m_WakeSkipUntilIter.Init( "WakeSkipUntilIter", groupname, this, 0, 0, 255 );
    m_WakeSkipUntilIter.SetDescript( "Iteration at which to START calculating the wake. Default=0 --> Wake calculated on each iteration" );

    m_StabilityCalcFlag.Init( "StabilityCalcFlag", groupname, this, false, false, true );
    m_StabilityCalcFlag.SetDescript( "Flag to calculate stability derivatives" );

    m_BatchModeFlag.Init( "BatchModeFlag", groupname, this, true, false, true );
    m_BatchModeFlag.SetDescript( "Flag to calculate in batch mode" );
    m_BatchModeFlag = true;

    // This sets all the filename members to the appropriate value (for example: empty strings if there is no vehicle)
    UpdateFilenames();

    m_SolverProcessKill = false;

    // Plot limits
    m_ConvergenceXMinIsManual.Init( "m_ConvergenceXMinIsManual", groupname, this, 0, 0, 1 );
    m_ConvergenceXMaxIsManual.Init( "m_ConvergenceXMaxIsManual", groupname, this, 0, 0, 1 );
    m_ConvergenceYMinIsManual.Init( "m_ConvergenceYMinIsManual", groupname, this, 0, 0, 1 );
    m_ConvergenceYMaxIsManual.Init( "m_ConvergenceYMaxIsManual", groupname, this, 0, 0, 1 );
    m_ConvergenceXMin.Init( "m_ConvergenceXMin", groupname, this, -1, -1e12, 1e12 );
    m_ConvergenceXMax.Init( "m_ConvergenceXMax", groupname, this, 1, -1e12, 1e12 );
    m_ConvergenceYMin.Init( "m_ConvergenceYMin", groupname, this, -1, -1e12, 1e12 );
    m_ConvergenceYMax.Init( "m_ConvergenceYMax", groupname, this, 1, -1e12, 1e12 );

    m_LoadDistXMinIsManual.Init( "m_LoadDistXMinIsManual", groupname, this, 0, 0, 1 );
    m_LoadDistXMaxIsManual.Init( "m_LoadDistXMaxIsManual", groupname, this, 0, 0, 1 );
    m_LoadDistYMinIsManual.Init( "m_LoadDistYMinIsManual", groupname, this, 0, 0, 1 );
    m_LoadDistYMaxIsManual.Init( "m_LoadDistYMaxIsManual", groupname, this, 0, 0, 1 );
    m_LoadDistXMin.Init( "m_LoadDistXMin", groupname, this, -1, -1e12, 1e12 );
    m_LoadDistXMax.Init( "m_LoadDistXMax", groupname, this, 1, -1e12, 1e12 );
    m_LoadDistYMin.Init( "m_LoadDistYMin", groupname, this, -1, -1e12, 1e12 );
    m_LoadDistYMax.Init( "m_LoadDistYMax", groupname, this, 1, -1e12, 1e12 );

    m_SweepXMinIsManual.Init( "m_SweepXMinIsManual", groupname, this, 0, 0, 1 );
    m_SweepXMaxIsManual.Init( "m_SweepXMaxIsManual", groupname, this, 0, 0, 1 );
    m_SweepYMinIsManual.Init( "m_SweepYMinIsManual", groupname, this, 0, 0, 1 );
    m_SweepYMaxIsManual.Init( "m_SweepYMaxIsManual", groupname, this, 0, 0, 1 );
    m_SweepXMin.Init( "m_SweepXMin", groupname, this, -1, -1e12, 1e12 );
    m_SweepXMax.Init( "m_SweepXMax", groupname, this, 1, -1e12, 1e12 );
    m_SweepYMin.Init( "m_SweepYMin", groupname, this, -1, -1e12, 1e12 );
    m_SweepYMax.Init( "m_SweepYMax", groupname, this, 1, -1e12, 1e12 );

    // Disk Element Parms
    m_Diameter.Init( "Diameter", groupname, this, 10.0, 0.0, 1e6 );
    m_Diameter.SetDescript( "Diameter of Disk Geom Set in Geom Window" );
    m_HubDiameter.Init( "HubDiameter", groupname, this, 0.0, 0.0, 1e6 );
    m_HubDiameter.SetDescript( "Hub Diameter with Max Diameter at Disk Diameter");
    m_RPM.Init( "RPM", groupname, this, 2000.0, -1e12, 1e12 );
    m_RPM.SetDescript( "RPM of Disk Element" );
    m_CT.Init( "CT", groupname, this, 0.4, 0.0, 1e2 );
    m_CT.SetDescript( "Coefficiect of Thrust" );
    m_CP.Init( "CP", groupname, this, 0.6, 0.0, 1e2 );
    m_CP.SetDescript( "Coefficient of Power" );

    // Other Setup Parameters
    m_Vinf.Init( "Vinf", groupname, this, 100, 0, 1e6 );
    m_Vinf.SetDescript( "Freestream Velocity Through Disk Component" );
    m_Rho.Init( "Rho", groupname, this, 0.002377, 0, 1e3 );
    m_Rho.SetDescript( "Freestream Density" );
    m_ReCref.Init( "ReCref", groupname, this, 10000000., 0, 1e12 );
    m_ReCref.SetDescript( "Reynolds Number along Reference Chord" );
    m_Symmetry.Init( "Symmetry", groupname, this, false, false, true );
    m_Symmetry.SetDescript( "Toggle X-Z Symmetry to Improve Calculation Time" );
    m_Write2DFEMFlag.Init( "Write2DFEMFlag", groupname, this, false, false, true );
    m_Write2DFEMFlag.SetDescript( "Toggle File Write for 2D FEM" );
    m_ClMax.Init( "Clmax", groupname, this, -1, 0, 1e3 );
    m_ClMax.SetDescript( "Cl Max of Aircraft" );
    m_ClMaxToggle.Init( "ClmaxToggle", groupname, this, false, false, true );
    m_MaxTurnAngle.Init( "MaxTurnAngle", groupname, this, -1, 0, 360 );
    m_MaxTurnAngle.SetDescript( "Max Turning Angle of Aircraft" );
    m_MaxTurnToggle.Init( "MaxTurnToggle", groupname, this, false, false, true );
    m_FarDist.Init( "FarDist", groupname, this, -1, 0, 1e6 );
    m_FarDist.SetDescript( "Far Field Distance for Wake Adaptation" );
    m_FarDistToggle.Init( "FarDistToggle", groupname, this, false, false, true );

    m_CurrentCSGroupIndex = -1;
    m_CurrentRotorDiskIndex = -1;
    m_LastSelectedType = -1;

    m_Verbose = false;
}

void VSPAEROMgrSingleton::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

void VSPAEROMgrSingleton::Renew()
{
    m_ControlSurfaceGroupVec.clear();
    m_CompleteControlSurfaceVec.clear();
    m_ActiveControlSurfaceVec.clear();
    m_RotorDiskVec.clear();

    m_DegenGeomVec.clear();

    m_CurrentCSGroupIndex = -1;
    m_CurrentRotorDiskIndex = -1;
    m_LastSelectedType = -1;
}

xmlNodePtr VSPAEROMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    char str[256];
    xmlNodePtr VSPAEROsetnode = xmlNewChild( node, NULL, BAD_CAST"VSPAEROSettings", NULL );

    ParmContainer::EncodeXml( VSPAEROsetnode ); // Encode VSPAEROMgr Parms

    // Encode Control Surface Groups using Internal Encode Method
    XmlUtil::AddIntNode( VSPAEROsetnode, "ControlSurfaceGroupCount", m_ControlSurfaceGroupVec.size() );
    for ( size_t i = 0; i < m_ControlSurfaceGroupVec.size(); ++i )
    {
        sprintf( str, "Control_Surface_Group_%u", i );
        xmlNodePtr csgnode = xmlNewChild( VSPAEROsetnode, NULL, BAD_CAST str, NULL );
        m_ControlSurfaceGroupVec[i]->EncodeXml( csgnode );
    }

    // Encode Rotor Disks using Internal Encode Method
    XmlUtil::AddIntNode( VSPAEROsetnode, "RotorDiskCount", m_RotorDiskVec.size() );
    for ( size_t i = 0; i < m_RotorDiskVec.size(); ++i )
    {
        sprintf( str, "Rotor_%u", i );
        xmlNodePtr rotornode = xmlNewChild( VSPAEROsetnode, NULL, BAD_CAST str, NULL );
        m_RotorDiskVec[i]->EncodeXml( rotornode );
    }

    return VSPAEROsetnode;
}

xmlNodePtr VSPAEROMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    char str[256];
    int def = 0;
    string strdef = "";
    double dbldef = 0.0;

    xmlNodePtr VSPAEROsetnode = XmlUtil::GetNode( node, "VSPAEROSettings", 0 );
    if ( VSPAEROsetnode )
    {
        ParmContainer::DecodeXml( VSPAEROsetnode ); // Decode VSPAEROMgr Parms

        // Decode Control Surface Groups using Internal Decode Method
        int num_groups = XmlUtil::FindInt(VSPAEROsetnode, "ControlSurfaceGroupCount", def);
        for (size_t i = 0; i < num_groups; ++i)
        {
            sprintf(str, "Control_Surface_Group_%u", i);
            xmlNodePtr csgnode = XmlUtil::GetNode(VSPAEROsetnode, str, 0);
            if (csgnode)
            {
                AddControlSurfaceGroup();
                m_ControlSurfaceGroupVec.back()->DecodeXml( csgnode );
            }
        }

        UpdateCompleteControlSurfVec();
        UpdateControlSurfaceGroups(); // Replace Shell SubSurfs with those from CompleteSurfaceVec

        // Decode Rotor Disks using Internal Decode Method
        int num_rotor = XmlUtil::FindInt( VSPAEROsetnode, "RotorDiskCount", def );
        m_RotorDiskVec.resize( num_rotor );
        for (size_t i = 0; i < num_rotor; ++i )
        {
            sprintf( str, "Rotor_%u", i );
            xmlNodePtr rotornode = XmlUtil::GetNode(VSPAEROsetnode, str, 0 );
            m_RotorDiskVec[i] = new RotorDisk();
            m_RotorDiskVec[i]->DecodeXml(rotornode);
        }

        UpdateRotorDisks();
    }

    return VSPAEROsetnode;
}


void VSPAEROMgrSingleton::Update()
{
    UpdateSref();

    UpdateFilenames();

    UpdateRotorDisks();

    UpdateCompleteControlSurfVec();
    UpdateUngroupedVec();
    UpdateActiveControlSurfVec();

    UpdateControlSurfaceGroups();
}

void VSPAEROMgrSingleton::UpdateSref()
{
    if( m_RefFlag() == vsp::MANUAL_REF )
    {
        m_Sref.Activate();
        m_bref.Activate();
        m_cref.Activate();
    }
    else
    {
        Geom* refgeom = VehicleMgr.GetVehicle()->FindGeom( m_RefGeomID );

        if( refgeom )
        {
            if( refgeom->GetType().m_Type == MS_WING_GEOM_TYPE )
            {
                WingGeom* refwing = ( WingGeom* ) refgeom;
                m_Sref.Set( refwing->m_TotalArea() );
                m_bref.Set( refwing->m_TotalSpan() );
                m_cref.Set( refwing->m_TotalChord() );

                m_Sref.Deactivate();
                m_bref.Deactivate();
                m_cref.Deactivate();
            }
        }
        else
        {
            m_RefGeomID = string();
        }
    }
}

void VSPAEROMgrSingleton::UpdateSetupParmLimits()
{
    if (m_ClMaxToggle())
    {
        m_ClMax.SetLowerLimit(0.0);
        m_ClMax.Activate();
    }
    else
    {
        m_ClMax.SetLowerLimit(-1.0);
        m_ClMax.Set(-1.0);
        m_ClMax.Deactivate();
    }

    if (m_MaxTurnToggle())
    {
        m_MaxTurnAngle.SetLowerLimit(0.0);
        m_MaxTurnAngle.Activate();
    }
    else
    {
        m_MaxTurnAngle.SetLowerLimit(-1.0);
        m_MaxTurnAngle.Set(-1.0);
        m_MaxTurnAngle.Deactivate();
    }

    if (m_FarDistToggle())
    {
        m_FarDist.SetLowerLimit(0.0);
        m_FarDist.Activate();
    }
    else
    {
        m_FarDist.SetLowerLimit(-1.0);
        m_FarDist.Set(-1.0);
        m_FarDist.Deactivate();
    }
}

void VSPAEROMgrSingleton::UpdateFilenames()    //A.K.A. SetupDegenFile()
{
    // Initialize these to blanks.  if any of the checks fail the variables will at least contain an empty string
    m_ModelNameBase     = string();
    m_DegenFileFull     = string();
    m_CompGeomFileFull  = string();     // TODO this is set from the get export name
    m_SetupFile         = string();
    m_AdbFile           = string();
    m_HistoryFile       = string();
    m_LoadFile          = string();
    m_StabFile          = string();

    Vehicle *veh = VehicleMgr.GetVehicle();
    if( veh )
    {
        // Generate the base name based on the vsp3filename without the extension
        int pos = -1;
        switch ( m_AnalysisMethod.Get() )
        {
        case vsp::VORTEX_LATTICE:
            // The base_name is dependent on the DegenFileName
            // TODO extra "_DegenGeom" is added to the m_ModelBase
            m_DegenFileFull = veh->getExportFileName( vsp::DEGEN_GEOM_CSV_TYPE );

            m_ModelNameBase = m_DegenFileFull;
            pos = m_ModelNameBase.find( ".csv" );
            if ( pos >= 0 )
            {
                m_ModelNameBase.erase( pos, m_ModelNameBase.length() - 1 );
            }

            m_CompGeomFileFull  = string(); //This file is not used for vortex lattice analysis
            m_SetupFile         = m_ModelNameBase + string( ".vspaero" );
            m_AdbFile           = m_ModelNameBase + string( ".adb" );
            m_HistoryFile       = m_ModelNameBase + string( ".history" );
            m_LoadFile          = m_ModelNameBase + string( ".lod" );
            m_StabFile          = m_ModelNameBase + string( ".stab" );

            break;

        case vsp::PANEL:
            m_CompGeomFileFull = veh->getExportFileName( vsp::VSPAERO_PANEL_TRI_TYPE );

            m_ModelNameBase = m_CompGeomFileFull;
            pos = m_ModelNameBase.find( ".tri" );
            if ( pos >= 0 )
            {
                m_ModelNameBase.erase( pos, m_ModelNameBase.length() - 1 );
            }

            m_DegenFileFull     = m_ModelNameBase + string( "_DegenGeom.csv" );
            m_SetupFile         = m_ModelNameBase + string( ".vspaero" );
            m_AdbFile           = m_ModelNameBase + string( ".adb" );
            m_HistoryFile       = m_ModelNameBase + string( ".history" );
            m_LoadFile          = m_ModelNameBase + string( ".lod" );
            m_StabFile          = m_ModelNameBase + string( ".stab" );

            break;

        default:
            // TODO print out an error here
            break;
        }
    }
}

void VSPAEROMgrSingleton::UpdatePropElemParms()
{
    if ( ValidRotorDiskIndex( m_CurrentRotorDiskIndex ) )
    {
        m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_Diameter = m_Diameter();
        m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_HubDiameter = m_HubDiameter();
        m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_RPM = m_RPM();
        m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_CT = m_CT();
        m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_CP = m_CP();
    }
}

void VSPAEROMgrSingleton::UpdateRotorDisks()
{
    Vehicle * veh = VehicleMgr.GetVehicle();
    char str[256];

    if (veh)
    {
        if (ValidRotorDiskIndex(m_CurrentRotorDiskIndex))
        {
            SetCurrentRotorDiskFromParms();
        }

        vector < RotorDisk* > temp;
        bool contained = false;

        vector < VspSurf > surfvec;
        vector <string> currgeomvec = veh->GetGeomSet(m_GeomSet());

        for (size_t i = 0; i < currgeomvec.size(); ++i)
        {
            veh->FindGeom(currgeomvec[i])->GetSurfVec(surfvec);
            for (size_t iSubsurf = 0; iSubsurf < surfvec.size(); ++iSubsurf)
            {
                contained = false;
                if (surfvec[iSubsurf].GetSurfType() == vsp::DISK_SURF)
                {
                    Geom* geom = veh->FindGeom(veh->GetGeomVec()[i]);
                    for (size_t j = 0; j < m_RotorDiskVec.size(); ++j)
                    {
                        // If Rotor Disk and Corresponding Surface Num Already Exists within m_RotorDiskVec
                        if (m_RotorDiskVec[j]->m_ParentGeomId == veh->GetGeomVec()[i] && m_RotorDiskVec[j]->GetSurfNum() == iSubsurf)
                        {
                            contained = true;
                            temp.push_back(m_RotorDiskVec[j]);
                            for (size_t k = 0; k < m_DegenGeomVec.size(); ++k)
                            {
                                if (m_DegenGeomVec[k].getParentGeom()->GetID().compare(m_RotorDiskVec[j]->m_ParentGeomId) == 0)
                                {
                                    int indxToSearch = k + temp.back()->m_ParentGeomSurfNdx;
                                    temp.back()->m_XYZ = m_DegenGeomVec[indxToSearch].getDegenDisk().x;
                                    temp.back()->m_Normal = m_DegenGeomVec[indxToSearch].getDegenDisk().nvec * -1.0;
                                    break;
                                }
                            }
                            sprintf(str, "%s_%u", geom->GetName().c_str(), iSubsurf);
                            temp.back()->SetName(str);
                        }
                    }

                    // If Rotor Disk and Corresponding Surface Num Do NOT Exist within m_RotorDiskVec
                    // Create New Rotor Disk Parm Container
                    if (!contained)
                    {
                        RotorDisk *rotor = new RotorDisk();
                        temp.push_back(rotor);
                        temp.back()->m_ParentGeomId = veh->GetGeomVec()[i];
                        temp.back()->m_ParentGeomSurfNdx = iSubsurf;
                        sprintf(str, "%s_%u", geom->GetName().c_str(), iSubsurf);
                        temp.back()->SetName(str);
                    }

                    string dia_id = geom->FindParm("Diameter", "Design");
                    temp.back()->m_Diameter.Set( ParmMgr.FindParm(dia_id)->Get() );
                    if (temp.back()->m_HubDiameter() > temp.back()->m_Diameter())
                    {
                        temp.back()->m_HubDiameter.Set(temp.back()->m_Diameter());
                    }
                    temp.back()->UpdateParmGroupName();
                }
            }
        }

        m_RotorDiskVec.clear();
        m_RotorDiskVec = temp;

        SetParmsFromCurrentRotorDisk();
    }
}

void VSPAEROMgrSingleton::UpdateControlSurfaceGroups()
{
    for (size_t i = 0; i < m_ControlSurfaceGroupVec.size(); ++i)
    {
        for (size_t k = 0; k < m_ControlSurfaceGroupVec[i]->m_ControlSurfVec.size(); ++k)
        {
            for (size_t j = 0; j < m_CompleteControlSurfaceVec.size(); ++j)
            {
                // If Control Surface ID AND Reflection Number Match - Replace Subsurf within Control Surface Group
                if (m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].SSID.compare(m_CompleteControlSurfaceVec[j].SSID) == 0 &&
                    m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].iReflect == m_CompleteControlSurfaceVec[j].iReflect)
                {
                    m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].fullName = m_CompleteControlSurfaceVec[j].fullName;
                    m_CompleteControlSurfaceVec[j].isGrouped = true;
                    m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].isGrouped = true;
                }
            }
            // Remove Sub Surfaces with Parent Geoms That No Longer Exist
            if (!VehicleMgr.GetVehicle()->FindGeom(m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].parentGeomId))
            {
                m_ControlSurfaceGroupVec[i]->RemoveSubSurface(m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].SSID,
                    m_ControlSurfaceGroupVec[i]->m_ControlSurfVec[k].iReflect);
            }
        }
        m_ControlSurfaceGroupVec[i]->UpdateParmGroupName();
        m_ControlSurfaceGroupVec[i]->SetParentContainer(GetID());
    }
}

void VSPAEROMgrSingleton::CleanCompleteControlSurfVec()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if (veh)
    {
        // Clean Out No Longer Existing Control Surfaces (Due to Geom Changes)
        for (size_t i = 0; i < m_CompleteControlSurfaceVec.size(); ++i)
        {
            Geom* geom = veh->FindGeom(m_CompleteControlSurfaceVec[i].parentGeomId);
            if (!geom)
            {
                m_CompleteControlSurfaceVec.erase(m_CompleteControlSurfaceVec.begin() + i);
                --i;
            }
            else if (!geom->GetSubSurf(m_CompleteControlSurfaceVec[i].SSID))
            {
                m_CompleteControlSurfaceVec.erase(m_CompleteControlSurfaceVec.begin() + i);
                --i;
            }
        }
    }
}

void VSPAEROMgrSingleton::UpdateCompleteControlSurfVec()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if (veh)
    {
        vector< string > geom_vec = veh->GetGeomVec();
        for ( size_t i = 0; i < geom_vec.size(); ++i )
        {
            Geom *g = veh->FindGeom( geom_vec[i] );
            if ( g )
            {
                vector < SubSurface* > sub_surf_vec = g->GetSubSurfVec();
                for (size_t j = 0; j < sub_surf_vec.size(); ++j)
                {
                    SubSurface *ssurf = sub_surf_vec[j];
                    if ( ssurf )
                    {
                        for (size_t iReflect = 0; iReflect < g->GetNumSymmCopies(); ++iReflect)
                        {
                            bool contained = false;
                            if ( ssurf->GetType() == vsp::SS_CONTROL || ssurf->GetType() == vsp::SS_RECTANGLE)
                            {
                                for (size_t k = 0; k < m_CompleteControlSurfaceVec.size(); ++k)
                                {
                                    // If CS and Corresponding Surface Num Already Exists within m_CompleteControlSurfaceVec
                                    if (m_CompleteControlSurfaceVec[k].SSID.compare( ssurf->GetID()) == 0 && m_CompleteControlSurfaceVec[k].iReflect == iReflect)
                                    {
                                        char str[256];
                                        sprintf(str, "%s_Surf%u_%s", g->GetName().c_str(), iReflect, ssurf->GetName().c_str());
                                        m_CompleteControlSurfaceVec[k].fullName = string( str );
                                        contained = true;
                                        break;
                                    }
                                }

                                // If CS and Corresponding Surface Num Do NOT Exist within m_CompleteControlSurfaceVec
                                // Create New CS Parm Container
                                if (!contained)
                                {
                                    VspAeroControlSurf newSurf;
                                    newSurf.SSID = ssurf->GetID();
                                    char str[256];
                                    sprintf(str, "%s_Surf%u_%s", g->GetName().c_str(), iReflect, ssurf->GetName().c_str());
                                    newSurf.fullName = string( str );
                                    newSurf.parentGeomId = ssurf->GetParentContainer();
                                    newSurf.iReflect = iReflect;

                                    m_CompleteControlSurfaceVec.push_back(newSurf);
                                }
                            }
                        }
                    }
                }
            }
		}

        CleanCompleteControlSurfVec();
    }
}

void VSPAEROMgrSingleton::UpdateUngroupedVec()
{
    m_UngroupedCS.clear();
    for ( size_t i = 0; i < m_CompleteControlSurfaceVec.size(); ++i )
    {
        if ( !m_CompleteControlSurfaceVec[i].isGrouped )
        {
            m_UngroupedCS.push_back( m_CompleteControlSurfaceVec[i] );
        }
    }
}

void VSPAEROMgrSingleton::UpdateActiveControlSurfVec()
{
    m_ActiveControlSurfaceVec.clear();
    if ( m_CurrentCSGroupIndex != -1 )
    {
        vector < VspAeroControlSurf > sub_surf_vec = m_ControlSurfaceGroupVec[ m_CurrentCSGroupIndex ]->m_ControlSurfVec;
        for ( size_t j = 0; j < sub_surf_vec.size(); ++j )
        {
            m_ActiveControlSurfaceVec.push_back( sub_surf_vec[j] );
        }
    }
}

void VSPAEROMgrSingleton::AddLinkableParms( vector < string > & linkable_parm_vec, const string & link_container_id )
{
    ParmContainer::AddLinkableParms( linkable_parm_vec );

    for (size_t i = 0; i < m_ControlSurfaceGroupVec.size(); ++i )
    {
        m_ControlSurfaceGroupVec[i]->AddLinkableParms( linkable_parm_vec, m_ID );
    }

    for (size_t i = 0; i < m_RotorDiskVec.size(); ++i )
    {
        m_RotorDiskVec[i]->AddLinkableParms( linkable_parm_vec, m_ID );
    }
}

// InitControlSurfaceGroups - creates the initial default grouping for the control surfaces
//  The initial grouping collects all surface copies of the subsurface into a single group.
//  For example if a wing is defined with an aileron and that wing is symetrical about the
//  xz plane there will be a surface copy of the master wing surface as well as a copy of
//  the subsurface. The two subsurfaces may get deflected differently during analysis
//  routines and can be identified uniquely by the control_surf.fullname.
//  The initial grouping routine implemented here finds all copies of that subsurface
//  that have the same sub surf ID and places them into a single control group.
void VSPAEROMgrSingleton::InitControlSurfaceGroups()
{
    Vehicle * veh = VehicleMgr.GetVehicle();
    ControlSurfaceGroup * csg;
    char str [256];
    bool exists = false;

    for ( size_t i = 0 ; i < m_UngroupedCS.size(); ++i )
    {
        // Construct a default group name
        string curr_csg_id = m_UngroupedCS[i].parentGeomId + "_" + m_UngroupedCS[i].SSID;
        exists = false;

        // Has CS been placed into init group?
        // --> No create group with any reflected groups
        // --> Yes Skip
        for (size_t j = 0; j < m_ControlSurfaceGroupVec.size(); ++j)
        {
            if (m_ControlSurfaceGroupVec[j]->m_ControlSurfVec.size() > 0)
            {
                sprintf(str, "%s_%s", m_ControlSurfaceGroupVec[j]->m_ParentGeomBaseID.c_str(),
                    m_ControlSurfaceGroupVec[j]->m_ControlSurfVec[0].SSID.c_str());
                if (curr_csg_id == str) // Update Existing Control Surface Group
                {
                    csg = m_ControlSurfaceGroupVec[j];
                    csg->AddSubSurface(m_UngroupedCS[i]);
                    csg->UpdateParmGroupName();
                    csg->SetParentContainer(GetID());
                    m_ControlSurfaceGroupVec.back() = csg;
                    for (size_t j = 0; j < m_CompleteControlSurfaceVec.size(); ++j)
                    {
                        if (m_UngroupedCS[i].SSID.compare(m_CompleteControlSurfaceVec[j].SSID) == 0)
                        {
                            m_CompleteControlSurfaceVec[j].isGrouped = true;
                        }
                    }
                    exists = true;
                    break;
                }
            }
        }

        if ( !exists ) // Create New Control Surface Group
        {
            Geom* geom = veh->FindGeom( m_UngroupedCS[i].parentGeomId );
            if (geom)
            {
                csg = new ControlSurfaceGroup;
                csg->AddSubSurface( m_UngroupedCS[i] );
                sprintf( str, "%s_%s", geom->GetName().c_str(),
                    geom->GetSubSurf(m_UngroupedCS[i].SSID)->GetName().c_str() );
                csg->SetName( str );
                csg->m_ParentGeomBaseID = m_UngroupedCS[i].parentGeomId;
                csg->UpdateParmGroupName();
                csg->SetParentContainer( GetID() );
                m_ControlSurfaceGroupVec.push_back( csg );
                for ( size_t j = 0; j < m_CompleteControlSurfaceVec.size(); ++j )
                {
                    if ( m_UngroupedCS[i].SSID.compare( m_CompleteControlSurfaceVec[j].SSID ) == 0 )
                    {
                        m_CompleteControlSurfaceVec[j].isGrouped = true;
                    }
                }
            }
        }
    }

    UpdateUngroupedVec();
}

string VSPAEROMgrSingleton::ComputeGeometry()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        fprintf( stderr, "ERROR: Unable to get vehicle \n\tFile: %s \tLine:%d\n", __FILE__, __LINE__ );
        return string();
    }

    // Loop through control sub surfaces and set NumPoints to 1 before generating degengeom
    vector<std::pair<string,int>> tNumPointsParmIdValVec;
    for ( size_t iControlSurf = 0; iControlSurf < m_CompleteControlSurfaceVec.size(); iControlSurf++ )
    {
        Geom* geom = veh->FindGeom( m_CompleteControlSurfaceVec[iControlSurf].parentGeomId );
        string tSubSurfId = m_CompleteControlSurfaceVec[iControlSurf].SSID.substr(0,10);
        string tNumPointsId = geom->GetSubSurf( tSubSurfId )->FindParm( "Tess_Num", "SS_Control" );
        IntParm* tNumPointsParm = (IntParm*)ParmMgr.FindParm( tNumPointsId );
        if ( tNumPointsParm && tNumPointsParm->Get()!=1 )
        {
            tNumPointsParmIdValVec.push_back( std::make_pair(tNumPointsId, tNumPointsParm->Get()) );

            // Send warning message to user
            fprintf( stderr, "NOTE: Temporarily setting Control_Surf NumPoints = 1 for VSPAERO compatible DegenGeom generation for Control_Surf: %s.\n",
                geom->GetSubSurf( tSubSurfId )->GetName().c_str() );

            tNumPointsParm->Set( 1 );
            geom->Update();
        }
    }

    m_DegenGeomVec.clear();
    veh->CreateDegenGeom( m_GeomSet() );
    m_DegenGeomVec = veh->GetDegenGeomVec();

    // Reset ControlSurf NumPoints
    for ( size_t i = 0; i < tNumPointsParmIdValVec.size(); i++ )
    {
        ParmMgr.FindParm( tNumPointsParmIdValVec[i].first )->Set( tNumPointsParmIdValVec[i].second );
    }

    //Update information derived from the degenerate geometry
    UpdateRotorDisks();
    UpdateCompleteControlSurfVec();

    // record original values
    bool exptMfile_orig = veh->getExportDegenGeomMFile();
    bool exptCSVfile_orig = veh->getExportDegenGeomCsvFile();
    veh->setExportDegenGeomMFile( false );
    veh->setExportDegenGeomCsvFile( true );

    UpdateFilenames();

    // Note: while in panel mode the degen file required by vspaero is
    // dependent on the tri filename and not necessarily what the current
    // setting is for the vsp::DEGEN_GEOM_CSV_TYPE
    string degenGeomFile_orig = veh->getExportFileName( vsp::DEGEN_GEOM_CSV_TYPE );
    veh->setExportFileName( vsp::DEGEN_GEOM_CSV_TYPE, m_DegenFileFull );

    veh->WriteDegenGeomFile();

    // restore original values
    veh->setExportDegenGeomMFile( exptMfile_orig );
    veh->setExportDegenGeomCsvFile( exptCSVfile_orig );
    veh->setExportFileName( vsp::DEGEN_GEOM_CSV_TYPE, degenGeomFile_orig );

    WaitForFile( m_DegenFileFull );
    if ( !FileExist( m_DegenFileFull ) )
    {
        fprintf( stderr, "WARNING: DegenGeom file not found: %s\n\tFile: %s \tLine:%d\n", m_DegenFileFull.c_str(), __FILE__, __LINE__ );
    }

    // Generate *.tri geometry file for Panel method
    if ( m_AnalysisMethod.Get() == vsp::PANEL )
    {
        // Cleanup previously created meshGeom IDs created from VSPAEROMgr
        if ( veh->FindGeom( m_LastPanelMeshGeomId ) )
        {
            veh->DeleteGeom( m_LastPanelMeshGeomId );
        }

        // Compute intersected and trimmed geometry
        int halfFlag = 0;
        m_LastPanelMeshGeomId = veh->CompGeomAndFlatten( m_GeomSet(), halfFlag );

        // After CompGeomAndFlatten() is run all the geometry is hidden and the intersected & trimmed mesh is the only one shown
        veh->WriteTRIFile( m_CompGeomFileFull , vsp::SET_SHOWN );
        WaitForFile( m_CompGeomFileFull );
        if ( !FileExist( m_CompGeomFileFull ) )
        {
            fprintf( stderr, "WARNING: CompGeom file not found: %s\n\tFile: %s \tLine:%d\n", m_CompGeomFileFull.c_str(), __FILE__, __LINE__ );
        }

    }

    // Clear previous results
    while ( ResultsMgr.GetNumResults( "VSPAERO_Geom" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Geom",  0 ) );
    }
    // Write out new results
    Results* res = ResultsMgr.CreateResults( "VSPAERO_Geom" );
    if ( !res )
    {
        fprintf( stderr, "ERROR: Unable to create result in result manager \n\tFile: %s \tLine:%d\n", __FILE__, __LINE__ );
        return string();
    }
    res->Add( NameValData( "GeometrySet", m_GeomSet() ) );
    res->Add( NameValData( "AnalysisMethod", m_AnalysisMethod.Get() ) );
    res->Add( NameValData( "DegenGeomFileName", m_DegenFileFull ) );
    if ( m_AnalysisMethod.Get() == vsp::PANEL )
    {
        res->Add( NameValData( "CompGeomFileName", m_CompGeomFileFull ) );
        res->Add( NameValData( "Mesh_GeomID", m_LastPanelMeshGeomId ) );
    }
    else
    {
        res->Add( NameValData( "CompGeomFileName", string() ) );
        res->Add( NameValData( "Mesh_GeomID", string() ) );
    }

    return res->GetID();

}

string VSPAEROMgrSingleton::CreateSetupFile()
{
    string retStr = string();

    UpdateFilenames();

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        fprintf( stderr, "ERROR %d: Unable to get vehicle \n\tFile: %s \tLine:%d\n", vsp::VSP_INVALID_PTR, __FILE__, __LINE__ );
        return retStr;
    }

    // Clear existing setup file
    if ( FileExist( m_SetupFile ) )
    {
        remove( m_SetupFile.c_str() );
    }


    FILE * case_file = fopen( m_SetupFile.c_str(), "w" );
    if ( case_file == NULL )
    {
        fprintf( stderr, "ERROR %d: Unable to create case file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_INVALID_PTR, m_SetupFile.c_str(), __FILE__, __LINE__ );
        return retStr;
    }
    fprintf( case_file, "Sref = %lf \n", m_Sref() );
    fprintf( case_file, "Cref = %lf \n", m_cref() );
    fprintf( case_file, "Bref = %lf \n", m_bref() );
    fprintf( case_file, "X_cg = %lf \n", m_Xcg() );
    fprintf( case_file, "Y_cg = %lf \n", m_Ycg() );
    fprintf( case_file, "Z_cg = %lf \n", m_Zcg() );

    vector<double> alphaVec;
    vector<double> betaVec;
    vector<double> machVec;
    GetSweepVectors( alphaVec, betaVec, machVec );

    if ( !m_BatchModeFlag.Get() )
    {
        //truncate the vectors to just the first element
        machVec.resize( 1 );
        alphaVec.resize( 1 );
        betaVec.resize( 1 );
    }
    unsigned int i;
    // Mach vector
    fprintf( case_file, "Mach = " );
    for ( i = 0; i < machVec.size() - 1; i++ )
    {
        fprintf( case_file, "%lf, ", machVec[i] );
    }
    fprintf( case_file, "%lf \n", machVec[i++] );

    // Alpha vector
    fprintf( case_file, "AoA = " );
    for ( i = 0; i < alphaVec.size() - 1; i++ )
    {
        fprintf( case_file, "%lf, ", alphaVec[i] );
    }
    fprintf( case_file, "%lf \n", alphaVec[i++] );

    // Beta vector
    fprintf( case_file, "Beta = " );
    for ( i = 0; i < betaVec.size() - 1; i++ )
    {
        fprintf( case_file, "%lf, ", betaVec[i] );
    }
    fprintf( case_file, "%lf \n", betaVec[i++] );

    string sym;
    if ( m_Symmetry() )
        sym = "Y";
    else
        sym = "NO";
    fprintf( case_file, "Vinf = %lf \n", m_Vinf() );
    fprintf( case_file, "Rho = %lf \n", m_Rho() );
    fprintf( case_file, "ReCref = %lf \n", m_ReCref() );
    fprintf( case_file, "ClMax = %lf \n", m_ClMax() );
    fprintf( case_file, "MaxTurningAngle = %lf \n", m_MaxTurnAngle() );
    fprintf( case_file, "Symmetry = %s \n", sym.c_str() );
    fprintf( case_file, "FarDist = %lf \n", m_FarDist() );
    fprintf( case_file, "NumWakeNodes = %d \n", -1 );       //TODO add to VSPAEROMgr as parm
    fprintf( case_file, "WakeIters = %d \n", m_WakeNumIter.Get() );

    // RotorDisks
    unsigned int numUsedRotors = 0;
    for ( unsigned int iRotor = 0; iRotor < m_RotorDiskVec.size(); iRotor++ )
    {
        if ( m_RotorDiskVec[iRotor]->m_IsUsed )
        {
            numUsedRotors++;
        }
    }
    fprintf( case_file, "NumberOfRotors = %d \n", numUsedRotors );           //TODO add to VSPAEROMgr as parm
    int iPropElement = 0;
    for ( unsigned int iRotor = 0; iRotor < m_RotorDiskVec.size(); iRotor++ )
    {
        if ( m_RotorDiskVec[iRotor]->m_IsUsed )
        {
            iPropElement++;
            fprintf( case_file, "PropElement_%d\n", iPropElement );     //read in by, but not used, in vspaero and begins at 1
            fprintf( case_file, "%d\n", iPropElement );                 //read in by, but not used, in vspaero
            m_RotorDiskVec[iRotor]->Write_STP_Data( case_file );
        }
    }

    // ControlSurfaceGroups
    unsigned int numUsedCSGs = 0;
    for ( size_t iCSG = 0; iCSG < m_ControlSurfaceGroupVec.size(); iCSG++ )
    {
        if ( m_ControlSurfaceGroupVec[iCSG]->m_IsUsed() )
        {
            numUsedCSGs++;
        }
    }

    if ( m_AnalysisMethod.Get() == vsp::PANEL )
    {
        // control surfaces are currently not supported for panel method
        numUsedCSGs = 0;
        fprintf( case_file, "NumberOfControlGroups = %d \n", numUsedCSGs );
    }
    else
    {
        fprintf( case_file, "NumberOfControlGroups = %d \n", numUsedCSGs );
        for ( size_t iCSG = 0; iCSG < m_ControlSurfaceGroupVec.size(); iCSG++ )
        {
            if ( m_ControlSurfaceGroupVec[iCSG]->m_IsUsed() )
            {
                m_ControlSurfaceGroupVec[iCSG]->Write_STP_Data( case_file );
            }
        }
    }

    //Finish up by closing the file and making sure that it appears in the file system
    fclose( case_file );

    // Wait until the setup file shows up on the file system
    WaitForFile( m_SetupFile );

    // Add and return a result
    Results* res = ResultsMgr.CreateResults( "VSPAERO_Setup" );

    if ( !FileExist( m_SetupFile ) )
    {
        // shouldn't be able to get here but create a setup file with the correct settings
        fprintf( stderr, "ERROR %d: setup file not found, file %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, m_SetupFile.c_str(), __FILE__, __LINE__ );
        retStr = string();
    }
    else if ( !res )
    {
        fprintf( stderr, "ERROR: Unable to create result in result manager \n\tFile: %s \tLine:%d\n", __FILE__, __LINE__ );
        retStr = string();
    }
    else
    {
        res->Add( NameValData( "SetupFile", m_SetupFile ) );
        retStr = res->GetID();
    }

    // Send the message to update the screens
    MessageData data;
    data.m_String = "UpdateAllScreens";
    MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );

    return retStr;

}

void VSPAEROMgrSingleton::ClearAllPreviousResults()
{
    while ( ResultsMgr.GetNumResults( "VSPAERO_History" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_History",  0 ) );
    }
    while ( ResultsMgr.GetNumResults( "VSPAERO_Load" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Load",  0 ) );
    }
    while ( ResultsMgr.GetNumResults( "VSPAERO_Stab" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Stab",  0 ) );
    }
    while ( ResultsMgr.GetNumResults( "VSPAERO_Wrapper" ) > 0 )
    {
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "VSPAERO_Wrapper",  0 ) );
    }
}

void VSPAEROMgrSingleton::GetSweepVectors( vector<double> &alphaVec, vector<double> &betaVec, vector<double> &machVec )
{
    // grab current parm values
    double alphaStart = m_AlphaStart.Get();
    double alphaEnd = m_AlphaEnd.Get();
    int alphaNpts = m_AlphaNpts.Get();

    double betaStart = m_BetaStart.Get();
    double betaEnd = m_BetaEnd.Get();
    int betaNpts = m_BetaNpts.Get();

    double machStart = m_MachStart.Get();
    double machEnd = m_MachEnd.Get();
    int machNpts = m_MachNpts.Get();

    // Calculate spacing
    double alphaDelta = 0.0;
    if ( alphaNpts > 1 )
    {
        alphaDelta = ( alphaEnd - alphaStart ) / ( alphaNpts - 1.0 );
    }
    for ( int iAlpha = 0; iAlpha < alphaNpts; iAlpha++ )
    {
        //Set current alpha value
        alphaVec.push_back( alphaStart + double( iAlpha ) * alphaDelta );
    }

    double betaDelta = 0.0;
    if ( betaNpts > 1 )
    {
        betaDelta = ( betaEnd - betaStart ) / ( betaNpts - 1.0 );
    }
    for ( int iBeta = 0; iBeta < betaNpts; iBeta++ )
    {
        //Set current alpha value
        betaVec.push_back( betaStart + double( iBeta ) * betaDelta );
    }

    double machDelta = 0.0;
    if ( machNpts > 1 )
    {
        machDelta = ( machEnd - machStart ) / ( machNpts - 1.0 );
    }
    for ( int iMach = 0; iMach < machNpts; iMach++ )
    {
        //Set current alpha value
        machVec.push_back( machStart + double( iMach ) * machDelta );
    }
}

/* ComputeSolver(FILE * logFile)
Returns a result with a vector of results id's under the name ResultVec
Optional input of logFile allows outputting to a log file or the console
*/
string VSPAEROMgrSingleton::ComputeSolver( FILE * logFile )
{
    UpdateFilenames();
    if ( m_BatchModeFlag.Get() )
    {
        return ComputeSolverBatch( logFile );
    }
    else
    {
        return ComputeSolverSingle( logFile );
    }

    return string();
}

/* ComputeSolverSingle(FILE * logFile)
*/
string VSPAEROMgrSingleton::ComputeSolverSingle( FILE * logFile )
{
    std::vector <string> res_id_vector;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {

        string adbFileName = m_AdbFile;
        string historyFileName = m_HistoryFile;
        string loadFileName = m_LoadFile;
        string stabFileName = m_StabFile;
        string modelNameBase = m_ModelNameBase;

        bool stabilityFlag = m_StabilityCalcFlag.Get();
        vsp::VSPAERO_ANALYSIS_METHOD analysisMethod = ( vsp::VSPAERO_ANALYSIS_METHOD )m_AnalysisMethod.Get();

        int ncpu = m_NCPU.Get();

        int wakeAvgStartIter = m_WakeAvgStartIter.Get();
        int wakeSkipUntilIter = m_WakeSkipUntilIter.Get();


        //====== Modify/Update the setup file ======//
        CreateSetupFile();

        //====== Loop over flight conditions and solve ======//
        vector<double> alphaVec;
        vector<double> betaVec;
        vector<double> machVec;
        GetSweepVectors( alphaVec, betaVec, machVec );

        for ( int iAlpha = 0; iAlpha < alphaVec.size(); iAlpha++ )
        {
            //Set current alpha value
            double current_alpha = alphaVec[iAlpha];

            for ( int iBeta = 0; iBeta < betaVec.size(); iBeta++ )
            {
                //Set current beta value
                double current_beta = betaVec[iBeta];

                for ( int iMach = 0; iMach < machVec.size(); iMach++ )
                {
                    //Set current mach value
                    double current_mach = machVec[iMach];

                    //====== Clear VSPAERO output files ======//
                    if ( FileExist( adbFileName ) )
                    {
                        remove( adbFileName.c_str() );
                    }
                    if ( FileExist( historyFileName ) )
                    {
                        remove( historyFileName.c_str() );
                    }
                    if ( FileExist( loadFileName ) )
                    {
                        remove( loadFileName.c_str() );
                    }
                    if ( FileExist( stabFileName ) )
                    {
                        remove( stabFileName.c_str() );
                    }

                    //====== Send command to be executed by the system at the command prompt ======//
                    vector<string> args;
                    // Set mach, alpha, beta (save to local "current*" variables to use as header information in the results manager)
                    args.push_back( "-fs" );       // "freestream" override flag
                    args.push_back( StringUtil::double_to_string( current_mach, "%f" ) );
                    args.push_back( "END" );
                    args.push_back( StringUtil::double_to_string( current_alpha, "%f" ) );
                    args.push_back( "END" );
                    args.push_back( StringUtil::double_to_string( current_beta, "%f" ) );
                    args.push_back( "END" );
                    // Set number of openmp threads
                    args.push_back( "-omp" );
                    args.push_back( StringUtil::int_to_string( m_NCPU.Get(), "%d" ) );
                    // Set stability run arguments
                    if ( stabilityFlag )
                    {
                        args.push_back( "-stab" );
                    }
                    // Force averaging startign at wake iteration N
                    if( wakeAvgStartIter >= 1 )
                    {
                        args.push_back( "-avg" );
                        args.push_back( StringUtil::int_to_string( wakeAvgStartIter, "%d" ) );
                    }
                    if( wakeSkipUntilIter >= 1 )
                    {
                        // No wake for first N iterations
                        args.push_back( "-nowake" );
                        args.push_back( StringUtil::int_to_string( wakeSkipUntilIter, "%d" ) );
                    }

                    if( m_Write2DFEMFlag() )
                    {
                        args.push_back( "-write2dfem" );
                    }

                    // Add model file name
                    args.push_back( modelNameBase );

                    //Print out execute command
                    string cmdStr = m_SolverProcess.PrettyCmd( veh->GetExePath(), veh->GetVSPAEROCmd(), args );
                    if( logFile )
                    {
                        fprintf( logFile, "%s", cmdStr.c_str() );
                    }
                    else
                    {
                        MessageData data;
                        data.m_String = "VSPAEROSolverMessage";
                        data.m_StringVec.push_back( cmdStr );
                        MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
                    }

                    // Execute VSPAero
                    m_SolverProcess.ForkCmd( veh->GetExePath(), veh->GetVSPAEROCmd(), args );

                    // ==== MonitorSolverProcess ==== //
                    MonitorSolver( logFile );


                    // Check if the kill solver flag has been raised, if so clean up and return
                    //  note: we could have exited the IsRunning loop if the process was killed
                    if( m_SolverProcessKill )
                    {
                        m_SolverProcessKill = false;    //reset kill flag

                        return string();    //return empty result ID vector
                    }

                    //====== Read in all of the results ======//
                    // read the files if there is new data that has not successfully been read in yet
                    ReadHistoryFile( historyFileName, res_id_vector, analysisMethod );
                    ReadLoadFile( loadFileName, res_id_vector, analysisMethod );
                    if ( stabilityFlag )
                    {
                        ReadStabFile( stabFileName, res_id_vector, analysisMethod );      //*.STAB stability coeff file
                    }

                    // Send the message to update the screens
                    MessageData data;
                    data.m_String = "UpdateAllScreens";
                    MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );

                }    //Mach sweep loop

            }    //beta sweep loop

        }    //alpha sweep loop

    }

    // Create "wrapper" result to contain a vector of result IDs (this maintains compatibility to return a single result after computation)
    Results *res = ResultsMgr.CreateResults( "VSPAERO_Wrapper" );
    if( !res )
    {
        return string();
    }
    else
    {
        res->Add( NameValData( "ResultsVec", res_id_vector ) );
        return res->GetID();
    }
}

/* ComputeSolverBatch(FILE * logFile)
*/
string VSPAEROMgrSingleton::ComputeSolverBatch( FILE * logFile )
{
    std::vector <string> res_id_vector;

    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( veh )
    {

        string adbFileName = m_AdbFile;
        string historyFileName = m_HistoryFile;
        string loadFileName = m_LoadFile;
        string stabFileName = m_StabFile;
        string modelNameBase = m_ModelNameBase;

        bool stabilityFlag = m_StabilityCalcFlag.Get();
        vsp::VSPAERO_ANALYSIS_METHOD analysisMethod = ( vsp::VSPAERO_ANALYSIS_METHOD )m_AnalysisMethod.Get();

        int ncpu = m_NCPU.Get();

        int wakeAvgStartIter = m_WakeAvgStartIter.Get();
        int wakeSkipUntilIter = m_WakeSkipUntilIter.Get();


        //====== Modify/Update the setup file ======//
        if ( m_Verbose ) printf( "Writing vspaero setup file: %s\n", m_SetupFile.c_str() );
        // if the setup file doesn't exist, create one with the current settings
        // TODO output a warning to the user that we are creating a default file
        CreateSetupFile();

        vector<double> alphaVec;
        vector<double> betaVec;
        vector<double> machVec;
        GetSweepVectors( alphaVec, betaVec, machVec );

        //====== Clear VSPAERO output files ======//
        if ( FileExist( adbFileName ) )
        {
            remove( adbFileName.c_str() );
        }
        if ( FileExist( historyFileName ) )
        {
            remove( historyFileName.c_str() );
        }
        if ( FileExist( loadFileName ) )
        {
            remove( loadFileName.c_str() );
        }
        if ( FileExist( stabFileName ) )
        {
            remove( stabFileName.c_str() );
        }

        //====== generate batch mode command to be executed by the system at the command prompt ======//
        vector<string> args;
        // Set mach, alpha, beta (save to local "current*" variables to use as header information in the results manager)
        args.push_back( "-fs" );       // "freestream" override flag

        //====== Loop over flight conditions and solve ======//
        // Mach
        for ( int iMach = 0; iMach < machVec.size(); iMach++ )
        {
            args.push_back( StringUtil::double_to_string( machVec[iMach], "%f " ) );
        }
        args.push_back( "END" );
        // Alpha
        for ( int iAlpha = 0; iAlpha < alphaVec.size(); iAlpha++ )
        {
            args.push_back( StringUtil::double_to_string( alphaVec[iAlpha], "%f " ) );
        }
        args.push_back( "END" );
        // Beta
        for ( int iBeta = 0; iBeta < betaVec.size(); iBeta++ )
        {
            args.push_back( StringUtil::double_to_string( betaVec[iBeta], "%f " ) );
        }
        args.push_back( "END" );

        // Set number of openmp threads
        args.push_back( "-omp" );
        args.push_back( StringUtil::int_to_string( m_NCPU.Get(), "%d" ) );

        // Set stability run arguments
        if ( stabilityFlag )
        {
            args.push_back( "-stab" );
        }

        // Force averaging startign at wake iteration N
        if( wakeAvgStartIter >= 1 )
        {
            args.push_back( "-avg" );
            args.push_back( StringUtil::int_to_string( wakeAvgStartIter, "%d" ) );
        }
        if( wakeSkipUntilIter >= 1 )
        {
            // No wake for first N iterations
            args.push_back( "-nowake" );
            args.push_back( StringUtil::int_to_string( wakeSkipUntilIter, "%d" ) );
        }

        if( m_Write2DFEMFlag() )
        {
            args.push_back( "-write2dfem" );
        }

        // Add model file name
        args.push_back( modelNameBase );

        //Print out execute command
        string cmdStr = m_SolverProcess.PrettyCmd( veh->GetExePath(), veh->GetVSPAEROCmd(), args );
        if( logFile )
        {
            fprintf( logFile, "%s", cmdStr.c_str() );
        }
        else
        {
            MessageData data;
            data.m_String = "VSPAEROSolverMessage";
            data.m_StringVec.push_back( cmdStr );
            MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
        }

        // Execute VSPAero
        m_SolverProcess.ForkCmd( veh->GetExePath(), veh->GetVSPAEROCmd(), args );

        // ==== MonitorSolverProcess ==== //
        MonitorSolver( logFile );


        // Check if the kill solver flag has been raised, if so clean up and return
        //  note: we could have exited the IsRunning loop if the process was killed
        if( m_SolverProcessKill )
        {
            m_SolverProcessKill = false;    //reset kill flag

            return string();    //return empty result ID vector
        }

        //====== Read in all of the results ======//
        ReadHistoryFile( historyFileName, res_id_vector, analysisMethod );
        ReadLoadFile( loadFileName, res_id_vector, analysisMethod );
        if ( stabilityFlag )
        {
            ReadStabFile( stabFileName, res_id_vector, analysisMethod );      //*.STAB stability coeff file
        }

        // Send the message to update the screens
        MessageData data;
        data.m_String = "UpdateAllScreens";
        MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );

    }

    // Create "wrapper" result to contain a vector of result IDs (this maintains compatibility to return a single result after computation)
    Results *res = ResultsMgr.CreateResults( "VSPAERO_Wrapper" );
    if( !res )
    {
        return string();
    }
    else
    {
        res->Add( NameValData( "ResultsVec", res_id_vector ) );
        return res->GetID();
    }
}

void VSPAEROMgrSingleton::MonitorSolver( FILE * logFile )
{
    // ==== MonitorSolverProcess ==== //
    int bufsize = 1000;
    char *buf;
    buf = ( char* ) malloc( sizeof( char ) * ( bufsize + 1 ) );
    unsigned long nread = 1;
    bool runflag = m_SolverProcess.IsRunning();
    while ( runflag || nread > 0 )
    {
        m_SolverProcess.ReadStdoutPipe( buf, bufsize, &nread );
        if( nread > 0 && nread != (unsigned long) -1 )
        {
            if ( buf )
            {
                buf[nread] = 0;
                StringUtil::change_from_to( buf, '\r', '\n' );
                if( logFile )
                {
                    fprintf( logFile, "%s", buf );
                }
                else
                {
                    MessageData data;
                    data.m_String = "VSPAEROSolverMessage";
                    data.m_StringVec.push_back( string( buf ) );
                    MessageMgr::getInstance().Send( "ScreenMgr", NULL, data );
                }
            }
        }

        SleepForMilliseconds( 100 );
        runflag = m_SolverProcess.IsRunning();
    }
}

void VSPAEROMgrSingleton::AddResultHeader( string res_id, double mach, double alpha, double beta, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod )
{
    // Add Flow Condition header to each result
    Results * res = ResultsMgr.FindResultsPtr( res_id );
    if ( res )
    {
        res->Add( NameValData( "AnalysisMethod", analysisMethod ) );
    }
}

// helper thread functions for VSPAERO GUI interface and multi-threaded impleentation
bool VSPAEROMgrSingleton::IsSolverRunning()
{
    return m_SolverProcess.IsRunning();
}

void VSPAEROMgrSingleton::KillSolver()
{
    // Raise flag to break the compute solver thread
    m_SolverProcessKill = true;
    return m_SolverProcess.Kill();
}

ProcessUtil* VSPAEROMgrSingleton::GetSolverProcess()
{
    return &m_SolverProcess;
}

// function is used to wait for the result to show up on the file system
int VSPAEROMgrSingleton::WaitForFile( string filename )
{
    // Wait until the results show up on the file system
    int n_wait = 0;
    // wait no more than 5 seconds = (50*100)/1000
    while ( ( !FileExist( filename ) ) & ( n_wait < 50 ) )
    {
        n_wait++;
        SleepForMilliseconds( 100 );
    }
    SleepForMilliseconds( 100 );  //additional wait for file

    if ( FileExist( filename ) )
    {
        return vsp::VSP_OK;
    }
    else
    {
        return vsp::VSP_FILE_DOES_NOT_EXIST;
    }
}

/*******************************************************
Read .HISTORY file output from VSPAERO
analysisMethod is passed in because the parm it is set by might change by the time we are done calculating the solution
See: VSP_Solver.C in vspaero project
line 4351 - void VSP_SOLVER::OutputStatusFile(int Type)
line 4407 - void VSP_SOLVER::OutputZeroLiftDragToStatusFile(void)
TODO:
- Update this function to use the generic table read as used in: string VSPAEROMgrSingleton::ReadStabFile()
*******************************************************/
void VSPAEROMgrSingleton::ReadHistoryFile( string filename, vector <string> &res_id_vector, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod )
{
    //TODO return success or failure
    FILE *fp = NULL;
    //size_t result;
    bool read_success = false;

    //HISTORY file
    WaitForFile( filename );
    fp = fopen( filename.c_str(), "r" );
    if ( fp == NULL )
    {
        fprintf( stderr, "ERROR %d: Could not open History file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, m_HistoryFile.c_str(), __FILE__, __LINE__ );
        return;
    }

    Results* res = NULL;
    std::vector<string> data_string_array;

    char seps[]   = " :,\t\n";
    while ( !feof( fp ) )
    {
        data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below

        if ( CheckForCaseHeader( data_string_array ) )
        {
            res = ResultsMgr.CreateResults( "VSPAERO_History" );
            res_id_vector.push_back( res->GetID() );

            if ( ReadVSPAEROCaseHeader( res, fp, analysisMethod ) != 0 )
            {
                // Failed to read the case header
                fprintf( stderr, "ERROR %d: Could not read case header in VSPAERO file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_READ_FAILURE, m_StabFile.c_str(), __FILE__, __LINE__ );
                return;
            }

        }

        //READ wake iteration table
        /* Example wake iteration table
        Iter      Mach       AoA      Beta       CL         CDo       CDi      CDtot      CS        L/D        E        CFx       CFy       CFz       CMx       CMy       CMz       T/QS
        1   0.00000   1.00000   0.00000   0.03329   0.00364   0.00009   0.00373  -0.00000   8.93773 395.42033  -0.00049  -0.00000   0.03329  -0.00000  -0.09836  -0.00000   0.00000
        2   0.00000   1.00000   0.00000   0.03329   0.00364   0.00009   0.00373  -0.00000   8.93494 394.87228  -0.00049  -0.00000   0.03328  -0.00000  -0.09834  -0.00000   0.00000
        ...
        */
        int wake_iter_table_columns = 18;
        if( data_string_array.size() == wake_iter_table_columns )
        {
            //discard the header row and read the next line assuming that it is numeric
            data_string_array = ReadDelimLine( fp, seps );

            // create new vectors for this set of results information
            std::vector<int> i;
            std::vector<double> Mach;
            std::vector<double> Alpha;
            std::vector<double> Beta;
            std::vector<double> CL;
            std::vector<double> CDo;
            std::vector<double> CDi;
            std::vector<double> CDtot;
            std::vector<double> CS;
            std::vector<double> LoD;
            std::vector<double> E;
            std::vector<double> CFx;
            std::vector<double> CFy;
            std::vector<double> CFz;
            std::vector<double> CMx;
            std::vector<double> CMy;
            std::vector<double> CMz;
            std::vector<double> ToQS;

            while ( data_string_array.size() == wake_iter_table_columns )
            {
                i.push_back(        std::stoi( data_string_array[0] ) );

                Mach.push_back(     std::stod( data_string_array[1] ) );
                Alpha.push_back(    std::stod( data_string_array[2] ) );
                Beta.push_back(     std::stod( data_string_array[3] ) );

                CL.push_back(       std::stod( data_string_array[4] ) );
                CDo.push_back(      std::stod( data_string_array[5] ) );
                CDi.push_back(      std::stod( data_string_array[6] ) );
                CDtot.push_back(    std::stod( data_string_array[7] ) );
                CS.push_back(       std::stod( data_string_array[8] ) );

                LoD.push_back(      std::stod( data_string_array[9] ) );
                E.push_back(        std::stod( data_string_array[10] ) );

                CFx.push_back(      std::stod( data_string_array[11] ) );
                CFy.push_back(      std::stod( data_string_array[12] ) );
                CFz.push_back(      std::stod( data_string_array[13] ) );

                CMx.push_back(      std::stod( data_string_array[14] ) );
                CMy.push_back(      std::stod( data_string_array[15] ) );
                CMz.push_back(      std::stod( data_string_array[16] ) );

                ToQS.push_back(     std::stod( data_string_array[17] ) );

                data_string_array = ReadDelimLine( fp, seps );
            }

            //add to the results manager
            if ( res )
            {
                res->Add( NameValData( "WakeIter", i ) );
                res->Add( NameValData( "Mach", Mach ) );
                res->Add( NameValData( "Alpha", Alpha ) );
                res->Add( NameValData( "Beta", Beta ) );
                res->Add( NameValData( "CL", CL ) );
                res->Add( NameValData( "CDo", CDo ) );
                res->Add( NameValData( "CDi", CDi ) );
                res->Add( NameValData( "CDtot", CDtot ) );
                res->Add( NameValData( "CS", CS ) );
                res->Add( NameValData( "L/D", LoD ) );
                res->Add( NameValData( "E", E ) );
                res->Add( NameValData( "CFx", CFx ) );
                res->Add( NameValData( "CFy", CFy ) );
                res->Add( NameValData( "CFz", CFz ) );
                res->Add( NameValData( "CMx", CMx ) );
                res->Add( NameValData( "CMy", CMy ) );
                res->Add( NameValData( "CMz", CMz ) );
                res->Add( NameValData( "T/QS", ToQS ) );
            }

        } // end of wake iteration

    } //end feof loop to read entire history file

    fclose ( fp );

    return;
}

/*******************************************************
Read .LOD file output from VSPAERO
See: VSP_Solver.C in vspaero project
line 2851 - void VSP_SOLVER::CalculateSpanWiseLoading(void)
TODO:
- Update this function to use the generic table read as used in: string VSPAEROMgrSingleton::ReadStabFile()
- Read in Component table information, this is the 2nd table at the bottom of the .lod file
*******************************************************/
void VSPAEROMgrSingleton::ReadLoadFile( string filename, vector <string> &res_id_vector, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod )
{
    FILE *fp = NULL;
    bool read_success = false;

    //LOAD file
    WaitForFile( filename );
    fp = fopen( filename.c_str(), "r" );
    if ( fp == NULL )
    {
        fprintf( stderr, "ERROR %d: Could not open Load file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, m_LoadFile.c_str(), __FILE__, __LINE__ );
        return;
    }

    Results* res = NULL;
    std::vector< std::string > data_string_array;
    std::vector< std::vector< double > > data_array;

    double cref = 1.0;

    char seps[]   = " :,\t\n";
    while ( !feof( fp ) )
    {
        data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below

        if ( CheckForCaseHeader( data_string_array ) )
        {
            res = ResultsMgr.CreateResults( "VSPAERO_Load" );
            res_id_vector.push_back( res->GetID() );

            if ( ReadVSPAEROCaseHeader( res, fp, analysisMethod ) != 0 )
            {
                // Failed to read the case header
                fprintf( stderr, "ERROR %d: Could not read case header in VSPAERO file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_READ_FAILURE, m_StabFile.c_str(), __FILE__, __LINE__ );
                return;
            }

            cref = res->FindPtr( "FC_Cref_" )->GetDouble( 0 );

        }

        // Sectional distribution table
        int nSectionalDataTableCols = 13;
        if ( data_string_array.size() == nSectionalDataTableCols )
        {
            //discard the header row and read the next line assuming that it is numeric
            data_string_array = ReadDelimLine( fp, seps );

            // Raw data vectors
            std::vector<int> WingId;
            std::vector<double> Yavg;
            std::vector<double> Chord;
            std::vector<double> VoVinf;
            std::vector<double> Cl;
            std::vector<double> Cd;
            std::vector<double> Cs;
            std::vector<double> Cx;
            std::vector<double> Cy;
            std::vector<double> Cz;
            std::vector<double> Cmx;
            std::vector<double> Cmy;
            std::vector<double> Cmz;

            //normalized by local chord
            std::vector<double> Clc_cref;
            std::vector<double> Cdc_cref;
            std::vector<double> Csc_cref;
            std::vector<double> Cxc_cref;
            std::vector<double> Cyc_cref;
            std::vector<double> Czc_cref;
            std::vector<double> Cmxc_cref;
            std::vector<double> Cmyc_cref;
            std::vector<double> Cmzc_cref;

            double chordRatio;

            // read the data rows
            while ( data_string_array.size() == nSectionalDataTableCols )
            {
                // Store the raw data
                WingId.push_back( std::stoi( data_string_array[0] ) );
                Yavg.push_back(   std::stod( data_string_array[1] ) );
                Chord.push_back(  std::stod( data_string_array[2] ) );
                VoVinf.push_back( std::stod( data_string_array[3] ) );
                Cl.push_back(     std::stod( data_string_array[4] ) );
                Cd.push_back(     std::stod( data_string_array[5] ) );
                Cs.push_back(     std::stod( data_string_array[6] ) );
                Cx.push_back(     std::stod( data_string_array[7] ) );
                Cy.push_back(     std::stod( data_string_array[8] ) );
                Cz.push_back(     std::stod( data_string_array[9] ) );
                Cmx.push_back(    std::stod( data_string_array[10] ) );
                Cmy.push_back(    std::stod( data_string_array[11] ) );
                Cmz.push_back(    std::stod( data_string_array[12] ) );

                chordRatio = Chord.back() / cref;

                // Normalized by local chord
                Clc_cref.push_back( Cl.back() * chordRatio );
                Cdc_cref.push_back( Cd.back() * chordRatio );
                Csc_cref.push_back( Cs.back() * chordRatio );
                Cxc_cref.push_back( Cx.back() * chordRatio );
                Cyc_cref.push_back( Cy.back() * chordRatio );
                Czc_cref.push_back( Cz.back() * chordRatio );
                Cmxc_cref.push_back( Cmx.back() * chordRatio );
                Cmyc_cref.push_back( Cmy.back() * chordRatio );
                Cmzc_cref.push_back( Cmz.back() * chordRatio );

                // Read the next line and loop
                data_string_array = ReadDelimLine( fp, seps );
            }

            // Finish up by adding the data to the result res
            res->Add( NameValData( "WingId", WingId ) );
            res->Add( NameValData( "Yavg", Yavg ) );
            res->Add( NameValData( "Chord", Chord ) );
            res->Add( NameValData( "V/Vinf", Chord ) );
            res->Add( NameValData( "cl", Cl ) );
            res->Add( NameValData( "cd", Cd ) );
            res->Add( NameValData( "cs", Cs ) );
            res->Add( NameValData( "cx", Cx ) );
            res->Add( NameValData( "cy", Cy ) );
            res->Add( NameValData( "cz", Cz ) );
            res->Add( NameValData( "cmx", Cmx ) );
            res->Add( NameValData( "cmy", Cmy ) );
            res->Add( NameValData( "cmz", Cmz ) );

            res->Add( NameValData( "cl*c/cref", Clc_cref ) );
            res->Add( NameValData( "cd*c/cref", Cdc_cref ) );
            res->Add( NameValData( "cs*c/cref", Csc_cref ) );
            res->Add( NameValData( "cx*c/cref", Cxc_cref ) );
            res->Add( NameValData( "cy*c/cref", Cyc_cref ) );
            res->Add( NameValData( "cz*c/cref", Czc_cref ) );
            res->Add( NameValData( "cmx*c/cref", Cmxc_cref ) );
            res->Add( NameValData( "cmy*c/cref", Cmyc_cref ) );
            res->Add( NameValData( "cmz*c/cref", Cmzc_cref ) );

        } // end sectional table read

    } // end file loop

    std::fclose ( fp );

    return;
}

/*******************************************************
Read .STAB file output from VSPAERO
See: VSP_Solver.C in vspaero project
*******************************************************/
void VSPAEROMgrSingleton::ReadStabFile( string filename, vector <string> &res_id_vector, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod )
{
    FILE *fp = NULL;
    bool read_success = false;
    WaitForFile( filename );
    fp = fopen( filename.c_str() , "r" );
    if ( fp == NULL )
    {
        fprintf( stderr, "ERROR %d: Could not open Stab file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_DOES_NOT_EXIST, m_StabFile.c_str(), __FILE__, __LINE__ );
        return;
    }

    Results* res = NULL;

    std::vector<string> table_column_names;
    std::vector<string> data_string_array;

    // Read in all of the data into the results manager
    char seps[]   = " :,\t\n";
    while ( !feof( fp ) )
    {
        data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below

        if ( CheckForCaseHeader( data_string_array ) )
        {
            res = ResultsMgr.CreateResults( "VSPAERO_Stab" );
            res_id_vector.push_back( res->GetID() );

            if ( ReadVSPAEROCaseHeader( res, fp, analysisMethod ) != 0 )
            {
                // Failed to read the case header
                fprintf( stderr, "ERROR %d: Could not read case header in VSPAERO file: %s\n\tFile: %s \tLine:%d\n", vsp::VSP_FILE_READ_FAILURE, m_StabFile.c_str(), __FILE__, __LINE__ );
                return;
            }
        }
        else if ( data_string_array.size() > 0 )
        {
            // Parse if this is not a comment line
            if ( res && strncmp( data_string_array[0].c_str(), "#", 1 ) != 0 )
            {

                //================ Table Data ================//
                // Checks for table header format
                if ( ( data_string_array.size() != table_column_names.size() ) || ( table_column_names.size() == 0 ) )
                {
                    //Indicator that the data table has changed or has not been initialized.
                    table_column_names.clear();
                    table_column_names = data_string_array;

                    // map control group names to full control surface group names
                    int i_field_offset = -1;
                    for ( unsigned int i_field = 0; i_field < data_string_array.size(); i_field++ )
                    {
                        if ( strstr( table_column_names[i_field].c_str(), "ConGrp_" ) )
                        {
                            //  Set field offset based on the first ConGrp_ found
                            if ( i_field_offset==-1 )
                            {
                                i_field_offset = i_field;
                            }

                            if ( m_Verbose ) printf( "\tMapping table col name to CSG name: \n" );
                            if ( m_Verbose ) printf( "\ti_field = %d --> i_field_offset = %d\n", i_field, i_field - i_field_offset );
                            if ( (i_field - i_field_offset) < m_ControlSurfaceGroupVec.size() )
                            {
                                if ( m_Verbose ) printf( "\t%s --> %s\n", table_column_names[i_field].c_str(), m_ControlSurfaceGroupVec[i_field - i_field_offset]->GetName().c_str() );
                                table_column_names[i_field] = m_ControlSurfaceGroupVec[i_field - i_field_offset]->GetName();
                            }
                            else
                            {
                                printf( "\tERROR (i_field - i_field_offset) > m_ControlSurfaceGroupVec.size()\n" );
                                printf( "\t      (  %d    -    %d         ) >            %lu             \n", i_field,i_field_offset,m_ControlSurfaceGroupVec.size());
                            }

                        }
                    }

                }
                else
                {
                    //This is a continuation of the current table and add this row to the results manager
                    for ( unsigned int i_field = 1; i_field < data_string_array.size(); i_field++ )
                    {
                        //attempt to read a double if that fails then treat it as a string result
                        double temp_val = 0;
                        int result = 0;
                        result = sscanf( data_string_array[i_field].c_str(), "%lf", &temp_val );
                        if ( result == 1 )
                        {
                            res->Add( NameValData( data_string_array[0] + "_" + table_column_names[i_field], temp_val ) );
                        }
                        else
                        {
                            res->Add( NameValData( data_string_array[0] + "_" + table_column_names[i_field], data_string_array[i_field] ) );
                        }
                    }
                } //end new table check

            } // end comment line check

        } // end data_string_array.size()>0 check

    } //end for while !feof(fp)

    std::fclose ( fp );

    return;
}

vector <string> VSPAEROMgrSingleton::ReadDelimLine( FILE * fp, char * delimeters )
{

    vector <string> dataStringVector;
    dataStringVector.clear();

    char strbuff[1024];                // buffer for entire line in file
    if ( fgets( strbuff, 1024, fp ) != NULL )
    {
        char * pch = strtok ( strbuff, delimeters );
        while ( pch != NULL )
        {
            dataStringVector.push_back( pch );
            pch = strtok ( NULL, delimeters );
        }
    }

    return dataStringVector;
}

bool VSPAEROMgrSingleton::CheckForCaseHeader( std::vector<string> headerStr )
{
    if ( headerStr.size() == 1 )
    {
        if ( strcmp( headerStr[0].c_str(), "*****************************************************************************************************************************************************************************************" ) == 0 )
        {
            return true;
        }
    }

    return false;
}

int VSPAEROMgrSingleton::ReadVSPAEROCaseHeader( Results * res, FILE * fp, vsp::VSPAERO_ANALYSIS_METHOD analysisMethod )
{
    // check input arguments
    if ( res == NULL )
    {
        // Bad pointer
        fprintf( stderr, "ERROR %d: Invalid results pointer\n\tFile: %s \tLine:%d\n", vsp::VSP_INVALID_PTR, __FILE__, __LINE__ );
        return -1;
    }
    if ( fp == NULL )
    {
        // Bad pointer
        fprintf( stderr, "ERROR %d: Invalid file pointer\n\tFile: %s \tLine:%d\n", vsp::VSP_INVALID_PTR, __FILE__, __LINE__ );
        return -2;
    }

    char seps[]   = " :,\t\n";
    std::vector<string> data_string_array;

    //skip any blank lines before the header
    while ( !feof( fp ) && data_string_array.size() == 0 )
    {
        data_string_array = ReadDelimLine( fp, seps ); //this is also done in some of the embedded loops below
    }

    // Read header table
    bool needs_header = true;
    bool mach_found = false;
    bool alpha_found = false;
    bool beta_found = false;
    double current_mach = -FLT_MAX;
    double current_alpha = -FLT_MAX;
    double current_beta = -FLT_MAX;
    double value;
    while ( !feof( fp ) && data_string_array.size() > 0 )
    {
        // Parse if this is not a comment line
        if ( ( strncmp( data_string_array[0].c_str(), "#", 1 ) != 0 ) && ( data_string_array.size() == 3 ) )
        {
            // assumption that the 2nd entry is a number
            if ( sscanf( data_string_array[1].c_str(), "%lf", &value ) == 1 )
            {
                res->Add( NameValData( "FC_" + data_string_array[0], value ) );

                // save flow condition information to be added to the header later
                if ( strcmp( data_string_array[0].c_str(), "Mach_" ) == 0 )
                {
                    current_mach = value;
                    mach_found = true;
                }
                if ( strcmp( data_string_array[0].c_str(), "AoA_" ) == 0 )
                {
                    current_alpha = value;
                    alpha_found = true;
                }
                if ( strcmp( data_string_array[0].c_str(), "Beta_" ) == 0 )
                {
                    current_beta = value;
                    beta_found = true;
                }

                // check if the information needed for the result header has been read in
                if ( mach_found && alpha_found && beta_found && needs_header )
                {
                    AddResultHeader( res->GetID(), current_mach, current_alpha, current_beta, analysisMethod );
                    needs_header = false;
                }
            }
        }

        // read the next line
        data_string_array = ReadDelimLine( fp, seps );

    } // end while

    if ( needs_header )
    {
        fprintf( stderr, "WARNING: Case header incomplete \n\tFile: %s \tLine:%d\n", __FILE__, __LINE__ );
        return -3;
    }

    return 0; // no errors
}

//Export Results to CSV
int VSPAEROMgrSingleton::ExportResultsToCSV( string fileName )
{
    int retVal = vsp::VSP_FILE_WRITE_FAILURE;

    // Get the results
    string resId = ResultsMgr.FindLatestResultsID( "VSPAERO_Wrapper" );
    if ( resId == string() )
    {
        retVal = vsp::VSP_CANT_FIND_NAME;
        fprintf( stderr, "ERROR %d: Unable to find ""VSPAERO_Wrapper"" result \n\tFile: %s \tLine:%d\n", retVal, __FILE__, __LINE__ );
        return retVal;
    }

    Results* resptr = ResultsMgr.FindResultsPtr( resId );
    if ( !resptr )
    {
        retVal = vsp::VSP_INVALID_PTR;
        fprintf( stderr, "ERROR %d: Unable to get pointer to ""VSPAERO_Wrapper"" result \n\tFile: %s \tLine:%d\n", retVal, __FILE__, __LINE__ );
        return retVal;
    }

    // Get all the child results and write out to csv using a vector of results
    vector <string> resIdVector = ResultsMgr.GetStringResults( resId, "ResultsVec" );
    if ( resIdVector.size()==0 )
    {
        fprintf( stderr, "WARNING %d: ""VSPAERO_Wrapper"" result contains no child results \n\tFile: %s \tLine:%d\n", retVal, __FILE__, __LINE__ );
    }

    // Export to CSV file
    retVal = ResultsMgr.WriteCSVFile( fileName, resIdVector );

    // Check that the file made it to the file system and return status
    return WaitForFile( fileName );
}

void VSPAEROMgrSingleton::SetCurrentRotorDiskFromParms()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( ValidRotorDiskIndex( m_CurrentRotorDiskIndex ) )
    {
        if ( m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_HubDiameter() > m_Diameter() )
        {
            m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_HubDiameter = m_Diameter();
            m_HubDiameter.Set(m_Diameter());
            m_HubDiameter.SetUpperLimit(m_Diameter());
        }
        else
        {
            m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_HubDiameter = m_HubDiameter();
        }
        m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_RPM = m_RPM();
        m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_CP = m_CP();
        m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_CT = m_CT();
    }
}
void VSPAEROMgrSingleton::SetParmsFromCurrentRotorDisk()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( ValidRotorDiskIndex(m_CurrentRotorDiskIndex) )
    {
        m_Diameter = m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_Diameter();

        m_HubDiameter = m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_HubDiameter();
        m_RPM = m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_RPM();
        m_CT = m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_CT();
        m_CP = m_RotorDiskVec[ m_CurrentRotorDiskIndex ]->m_CP();
    }
}

bool VSPAEROMgrSingleton::ValidRotorDiskIndex( int index )
{
    if ( (index >= 0) && (index < m_RotorDiskVec.size()) && m_RotorDiskVec.size()>0 )
    {
        return true;
    }

    return false;
}

void VSPAEROMgrSingleton::RemoveFromUngrouped( const string & ssid, int reflec_num )
{
    for (size_t i = 0; i < m_UngroupedCS.size(); ++i )
    {
        if ( m_UngroupedCS[i].SSID.compare( ssid ) == 0 && m_UngroupedCS[i].iReflect == reflec_num )
        {
            m_UngroupedCS.erase( m_UngroupedCS.begin() + i);
            break;
        }
    }
}

void VSPAEROMgrSingleton::AddControlSurfaceGroup()
{
    ControlSurfaceGroup* new_cs = new ControlSurfaceGroup;
    m_ControlSurfaceGroupVec.push_back( new_cs );

    m_CurrentCSGroupIndex = m_ControlSurfaceGroupVec.size()-1;

    m_SelectedGroupedCS.clear();
    UpdateActiveControlSurfVec();
}

void VSPAEROMgrSingleton::RemoveControlSurfaceGroup()
{
    if ( m_CurrentCSGroupIndex != -1 )
    {
        for ( size_t i = 0; i < m_ActiveControlSurfaceVec.size(); ++i )
        {
            for ( size_t j = 0; j < m_CompleteControlSurfaceVec.size(); ++j )
            {
                if ( m_CompleteControlSurfaceVec[j].SSID.compare( m_ActiveControlSurfaceVec[i].SSID ) == 0 )
                {
                    m_CompleteControlSurfaceVec[j].isGrouped = false;
                }
            }
        }

        delete m_ControlSurfaceGroupVec[m_CurrentCSGroupIndex];
        m_ControlSurfaceGroupVec.erase( m_ControlSurfaceGroupVec.begin() + m_CurrentCSGroupIndex );

        if ( m_ControlSurfaceGroupVec.size() > 0 )
        {
            m_CurrentCSGroupIndex = 0;
        }
        else
        {
            m_CurrentCSGroupIndex = -1;
        }
    }
    m_SelectedGroupedCS.clear();
    UpdateActiveControlSurfVec();
}

void VSPAEROMgrSingleton::AddSelectedToCSGroup()
{
    vector < int > selected = m_SelectedUngroupedCS;
    if ( m_CurrentCSGroupIndex != -1 )
    {
        for ( size_t i = 0; i < selected.size(); ++i )
        {

            m_ControlSurfaceGroupVec[ m_CurrentCSGroupIndex ]->AddSubSurface( m_UngroupedCS[ selected[ i ] - 1 ] );
            for ( size_t j = 0; j < m_CompleteControlSurfaceVec.size(); ++j )
            {
                if ( m_UngroupedCS[selected[i]-1].SSID.compare( m_CompleteControlSurfaceVec[j].SSID ) == 0 )
                {
                    if ( m_UngroupedCS[selected[i]-1].iReflect == m_CompleteControlSurfaceVec[j].iReflect )
                    {
                        m_CompleteControlSurfaceVec[ j ].isGrouped = true;
                    }
                }
            }
        }
    }
    m_SelectedUngroupedCS.clear();
    m_SelectedGroupedCS.clear();
    UpdateActiveControlSurfVec();
}

void VSPAEROMgrSingleton::AddAllToCSGroup()
{
    if ( m_CurrentCSGroupIndex != -1 )
    {
        for ( size_t i = 0; i < m_UngroupedCS.size(); ++i )
        {
            m_ControlSurfaceGroupVec[ m_CurrentCSGroupIndex ]->AddSubSurface( m_UngroupedCS[ i ] );
            for ( size_t j = 0; j < m_CompleteControlSurfaceVec.size(); ++j )
            {
                if ( m_UngroupedCS[i].SSID.compare( m_CompleteControlSurfaceVec[j].SSID ) == 0 )
                {
                    if ( m_UngroupedCS[i].iReflect == m_CompleteControlSurfaceVec[j].iReflect )
                    {
                        m_CompleteControlSurfaceVec[ j ].isGrouped = true;
                    }
                }
            }
        }
    }
    m_SelectedUngroupedCS.clear();
    m_SelectedGroupedCS.clear();
    UpdateActiveControlSurfVec();
}

void VSPAEROMgrSingleton::RemoveSelectedFromCSGroup()
{
    vector < int > selected = m_SelectedGroupedCS;
    if ( m_CurrentCSGroupIndex != -1 )
    {
        for ( size_t i = 0; i < selected.size(); ++i )
        {
            m_ControlSurfaceGroupVec[ m_CurrentCSGroupIndex ]->RemoveSubSurface( m_ActiveControlSurfaceVec[selected[i] - 1].SSID,
                m_ActiveControlSurfaceVec[selected[i] - 1].iReflect);
            for ( size_t j = 0; j < m_CompleteControlSurfaceVec.size(); ++j )
            {
                if ( m_ActiveControlSurfaceVec[selected[i]-1].SSID.compare( m_CompleteControlSurfaceVec[j].SSID ) == 0 )
                {
                    if ( m_ActiveControlSurfaceVec[selected[i]-1].iReflect == m_CompleteControlSurfaceVec[j].iReflect )
                    {
                        m_CompleteControlSurfaceVec[ j ].isGrouped = false;
                    }
                }
            }
        }
    }
    m_SelectedGroupedCS.clear();
    UpdateUngroupedVec();
    UpdateActiveControlSurfVec();
}

void VSPAEROMgrSingleton::RemoveAllFromCSGroup()
{
    if ( m_CurrentCSGroupIndex != -1 )
    {
        for ( size_t i = 0; i < m_ActiveControlSurfaceVec.size(); ++i )
        {
            m_ControlSurfaceGroupVec[ m_CurrentCSGroupIndex ]->RemoveSubSurface( m_ActiveControlSurfaceVec[i].SSID, m_ActiveControlSurfaceVec[i].iReflect );
            for ( size_t j = 0; j < m_CompleteControlSurfaceVec.size(); ++j )
            {
                if ( m_ActiveControlSurfaceVec[i].SSID.compare( m_CompleteControlSurfaceVec[j].SSID ) == 0 )
                {
                    if ( m_ActiveControlSurfaceVec[i].iReflect == m_CompleteControlSurfaceVec[j].iReflect )
                    {
                        m_CompleteControlSurfaceVec[ j ].isGrouped = false;
                    }
                }
            }
        }
    }
    m_SelectedGroupedCS.clear();
    UpdateUngroupedVec();
    UpdateActiveControlSurfVec();
}

string VSPAEROMgrSingleton::GetCurrentCSGGroupName()
{
    if ( m_CurrentCSGroupIndex != -1 )
    {
        return m_ControlSurfaceGroupVec[ m_CurrentCSGroupIndex ]->GetName();
    }
    else
    {
        return "";
    }
}

void VSPAEROMgrSingleton::SetCurrentCSGroupName(const string & name)
{
    if ( m_CurrentCSGroupIndex != -1 )
    {
        m_ControlSurfaceGroupVec[ m_CurrentCSGroupIndex ]->SetName( name );
    }
}

void VSPAEROMgrSingleton::HighlightSelected( int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    veh->ClearActiveGeom();

    if ( type == ROTORDISK )
    {
        VSPAEROMgr.SetCurrentType( ROTORDISK );
    }
    else if ( type == CONTROL_SURFACE )
    {
        VSPAEROMgr.SetCurrentType( CONTROL_SURFACE );
    }
    else
    {
        return;
    }
}

void VSPAEROMgrSingleton::LoadDrawObjs( vector < DrawObj* > & draw_obj_vec )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    if ( m_LastSelectedType == ROTORDISK )
    {
        UpdateBBox( draw_obj_vec );
    }
    else if ( m_LastSelectedType == CONTROL_SURFACE )
    {
        UpdateHighlighted( draw_obj_vec );
    }
}

void VSPAEROMgrSingleton::UpdateBBox( vector < DrawObj* > & draw_obj_vec )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if (!veh)
    {
        return;
    }

    //==== Load Bounding Box ====//
    m_BBox.Reset();
    BndBox bb;

    // If there is no selected rotor size is zero ( like blank geom )
    // set bbox to zero size
    if (m_CurrentRotorDiskIndex == -1)
    {
        m_BBox.Update(vec3d(0, 0, 0));
    }
    else
    {
        vector < VspSurf > surf_vec;
        Geom* geom = veh->FindGeom(m_RotorDiskVec[m_CurrentRotorDiskIndex]->GetParentID() );
        if ( geom )
        {
            geom->GetSurfVec(surf_vec);
            surf_vec[m_RotorDiskVec[m_CurrentRotorDiskIndex]->GetSurfNum()].GetBoundingBox(bb);
            m_BBox.Update(bb);
        }
        else
            m_CurrentRotorDiskIndex = -1;
    }

    //==== Bounding Box ====//
    m_HighlightDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_HighlightDrawObj.m_GeomID = BBOXHEADER + m_ID;
    m_HighlightDrawObj.m_LineWidth = 2.0;
    m_HighlightDrawObj.m_LineColor = vec3d(1.0, 0., 0.0);
    m_HighlightDrawObj.m_Type = DrawObj::VSP_LINES;

    m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();

    draw_obj_vec.push_back(&m_HighlightDrawObj);
}

void VSPAEROMgrSingleton::UpdateHighlighted( vector < DrawObj* > & draw_obj_vec )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    string parentID = "";
    string ssid = "";
    int sub_surf_indx;
    if (m_CurrentCSGroupIndex != -1)
    {
        vector < VspAeroControlSurf > cont_surf_vec = m_ActiveControlSurfaceVec;
        vector < VspAeroControlSurf > cont_surf_vec_ungrouped = m_UngroupedCS;
        if (m_SelectedGroupedCS.size() == 0 && m_SelectedUngroupedCS.size() == 0)
        {
            for (size_t i = 0; i < cont_surf_vec.size(); ++i)
            {
                vec3d color(0,1,0); // Green
                parentID = cont_surf_vec[i].parentGeomId;
                sub_surf_indx = cont_surf_vec[i].iReflect;
                ssid = cont_surf_vec[i].SSID;
                Geom* geom = veh->FindGeom(parentID);
                if (geom)
                {
                    SubSurface* subsurf = geom->GetSubSurf(ssid);
                    if (subsurf)
                    {
                        subsurf->LoadPartialColoredDrawObjs(ssid, sub_surf_indx, draw_obj_vec, color);
                    }
                }
            }
        }
        else
        {
            for (size_t i = 0; i < m_SelectedGroupedCS.size(); ++i)
            {
                vec3d color(0,1,0); // Green
                parentID = cont_surf_vec[m_SelectedGroupedCS[i]-1].parentGeomId;
                sub_surf_indx = cont_surf_vec[m_SelectedGroupedCS[i]-1].iReflect;
                ssid = cont_surf_vec[m_SelectedGroupedCS[i]-1].SSID;
                Geom* geom = veh->FindGeom(parentID);
                SubSurface* subsurf = geom->GetSubSurf(ssid);
                if (subsurf)
                {
                    subsurf->LoadPartialColoredDrawObjs(ssid, sub_surf_indx, draw_obj_vec, color);
                }
            }
            for (size_t i = 0; i < m_SelectedUngroupedCS.size(); ++i )
            {
                vec3d color(1,0,0); // Red
                parentID = cont_surf_vec_ungrouped[m_SelectedUngroupedCS[i]-1].parentGeomId;
                sub_surf_indx = cont_surf_vec_ungrouped[m_SelectedUngroupedCS[i]-1].iReflect;
                ssid = cont_surf_vec_ungrouped[m_SelectedUngroupedCS[i]-1].SSID;
                Geom* geom = veh->FindGeom(parentID);
                SubSurface* subsurf = geom->GetSubSurf(ssid);
                if (subsurf)
                {
                    subsurf->LoadPartialColoredDrawObjs(ssid, sub_surf_indx, draw_obj_vec, color);
                }
            }
        }
    }
}


/*##############################################################################
#                                                                              #
#                              RotorDisk                                       #
#                                                                              #
##############################################################################*/

RotorDisk::RotorDisk( void ) : ParmContainer()
{
    InitDisk();
}

RotorDisk::RotorDisk( const RotorDisk &RotorDisk )
{

    m_Name = RotorDisk.m_Name;

    m_IsUsed = RotorDisk.m_IsUsed;

    m_XYZ = RotorDisk.m_XYZ;           // RotorXYZ_
    m_Normal = RotorDisk.m_Normal;        // RotorNormal_

    m_Diameter = RotorDisk.m_Diameter;       // RotorDiameter_
    m_HubDiameter = RotorDisk.m_HubDiameter;    // RotorHubDiameter_
    m_RPM = RotorDisk.m_RPM;       // RotorRPM_

    m_CT = RotorDisk.m_CT;        // Rotor_CT_
    m_CP = RotorDisk.m_CP;        // Rotor_CP_

    m_ParentGeomId = RotorDisk.m_ParentGeomId;
    m_ParentGeomSurfNdx = RotorDisk.m_ParentGeomSurfNdx;
}

// Construct from degenerate geometry
RotorDisk::RotorDisk( DegenGeom &degenGeom )
{
    InitDisk();

    if ( degenGeom.getType() == DegenGeom::DISK_TYPE )
    {
        DegenDisk degenDisk = degenGeom.getDegenDisk();

        m_XYZ = degenDisk.x;
        m_Normal = degenDisk.nvec*-1;   //definition of normal vector in VSPAERO is -1*nvec of degen geom

        m_Diameter = degenDisk.d;

        m_ParentGeomId = degenGeom.getParentGeom()->GetID().c_str();
        m_ParentGeomSurfNdx = degenGeom.getSurfNum();

        m_Name.append( degenGeom.getParentGeom()->GetName() );
        m_Name.append( "_" );
        m_Name.append( to_string( degenGeom.getSurfNum() ) );
    }
}

// Construct from degenerate Disk
RotorDisk::RotorDisk( const DegenDisk degenDisk, string parentGeomId, unsigned int parentGeomSurfNdx )
{
    InitDisk();

    m_XYZ = degenDisk.x;
    m_Normal = degenDisk.nvec*-1;   //definition of normal vector in VSPAERO is -1*nvec of degen geom
    m_Diameter = degenDisk.d;

    m_ParentGeomId = parentGeomId;
    m_ParentGeomSurfNdx = parentGeomSurfNdx;

    m_IsUsed = true;
}

RotorDisk::~RotorDisk( void )
{

    // Nothing to do..

}

void RotorDisk::InitDisk()
{
    m_Name = "Default";
    string groupname = "RotorQualities";

    m_IsUsed = true;

    m_XYZ.set_xyz( 0, 0, 0 );           // RotorXYZ_
    m_Normal.set_xyz( 0, 0, 0 );        // RotorNormal_

    m_Diameter.Init( "Rotor_Diameter", groupname, this, 10.0, 0.0, 1e12 );       // RotorDiameter_
    m_Diameter.SetDescript( "Rotor Diameter" );

    m_HubDiameter.Init( "Rotor_HubDiameter", groupname, this, 0.0, 0.0, 1e12 );    // RotorHubDiameter_
    m_HubDiameter.SetDescript( "Rotor Hub Diameter" );

    m_RPM.Init( "Rotor_RPM", groupname, this, 2000.0, 0.0, 1e12 );       // RotorRPM_
    m_RPM.SetDescript( "Rotor RPM" );

    m_CT.Init( "Rotor_CT", groupname, this, 0.4, 0.0, 1e3);        // Rotor_CT_
    m_CT.SetDescript( "Rotor Coefficient of Thrust" );

    m_CP.Init( "Rotor_CP", groupname, this, 0.6, 0.0, 1e3 );        // Rotor_CP_
    m_CP.SetDescript( "Rotor Coefficient of Power" );

    m_ParentGeomId = "";
    m_ParentGeomSurfNdx = -1;
}

void RotorDisk::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

RotorDisk& RotorDisk::operator=( const RotorDisk &RotorDisk )
{

    m_Name = RotorDisk.m_Name;

    m_XYZ = RotorDisk.m_XYZ;           // RotorXYZ_
    m_Normal = RotorDisk.m_Normal;        // RotorNormal_

    m_Diameter = RotorDisk.m_Diameter;       // RotorRadius_
    m_HubDiameter = RotorDisk.m_HubDiameter;    // RotorHubRadius_
    m_RPM = RotorDisk.m_RPM;       // RotorRPM_

    m_CT = RotorDisk.m_CT;        // Rotor_CT_
    m_CP = RotorDisk.m_CP;        // Rotor_CP_

    m_ParentGeomId = RotorDisk.m_ParentGeomId;
    m_ParentGeomSurfNdx = RotorDisk.m_ParentGeomSurfNdx;

    return *this;

}

void RotorDisk::Write_STP_Data( FILE *InputFile )
{

    // Write out RotorDisk to file

    fprintf( InputFile, "%lf %lf %lf \n", m_XYZ.x(), m_XYZ.y(), m_XYZ.z() );

    fprintf( InputFile, "%lf %lf %lf \n", m_Normal.x(), m_Normal.y(), m_Normal.z() );

    fprintf( InputFile, "%lf \n", m_Diameter() / 2.0 );

    fprintf( InputFile, "%lf \n", m_HubDiameter() / 2.0 );

    fprintf( InputFile, "%lf \n", m_RPM() );

    fprintf( InputFile, "%lf \n", m_CT() );

    fprintf( InputFile, "%lf \n", m_CP() );

}

xmlNodePtr RotorDisk::EncodeXml( xmlNodePtr & node )
{
    if ( node )
    {
        ParmContainer::EncodeXml( node );
        XmlUtil::AddStringNode( node, "ParentID", m_ParentGeomId.c_str() );
        XmlUtil::AddIntNode( node, "SurfIndex", m_ParentGeomSurfNdx );
    }

    return node;
}

xmlNodePtr RotorDisk::DecodeXml( xmlNodePtr & node )
{
    string defstr = "";
    int defint = 0;
    if ( node )
    {
        ParmContainer::DecodeXml( node );
        m_ParentGeomId = XmlUtil::FindString( node, "ParentID", defstr );
        m_ParentGeomSurfNdx = XmlUtil::FindInt( node, "SurfIndex", defint );
    }

    return node;
}

void RotorDisk::UpdateParmGroupName()
{
    char str[256];
    sprintf( str, "RotorQualities_%s", m_Name.c_str() );
    m_Diameter.SetGroupName( str );
    m_HubDiameter.SetGroupName( str );
    m_RPM.SetGroupName( str );
    m_CT.SetGroupName( str );
    m_CP.SetGroupName( str );
}


/*##############################################################################
#                                                                              #
#                        ControlSurfaceGroup                                   #
#                                                                              #
##############################################################################*/

ControlSurfaceGroup::ControlSurfaceGroup( void ) : ParmContainer()
{
    m_Name = "Unnamed Control Group";
    m_ParentGeomBaseID = "";

    m_IsUsed.Init( "ActiveFlag", "CSGQualities", this, true, false, true );
    m_IsUsed.SetDescript( "Flag to determine whether or not this group will be used in VSPAero" );

    for (size_t i = 0; i < m_DeflectionGainVec.size(); ++i)
    {
        delete m_DeflectionGainVec[i];
    }
    m_DeflectionGainVec.clear();

    m_DeflectionAngle.Init( "DeflectionAngle", "CSGQualities", this, 0.0, -1.0e12, 1.0e12 );
    m_DeflectionAngle.SetDescript( "Angle of deflection for the control group" );
}

ControlSurfaceGroup::~ControlSurfaceGroup( void )
{
    // nothing to do
}

void ControlSurfaceGroup::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

void ControlSurfaceGroup::Write_STP_Data( FILE *InputFile )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if (!veh)
    {
        return;
    }

    // Write out Control surface group to .vspaero file
    fprintf( InputFile, "%s\n", m_Name.c_str() );

    // surface names ( Cannot have trailing commas )
    unsigned int i=0;
    for ( i = 0; i < m_ControlSurfVec.size()-1; i++ )
    {
        fprintf( InputFile, "%s,", m_ControlSurfVec[i].fullName.c_str() );
    }
    fprintf( InputFile, "%s\n", m_ControlSurfVec[i++].fullName.c_str() );

    // deflection mixing gains ( Cannot have trailing commas )
    for ( i = 0; i < m_DeflectionGainVec.size()-1; i++ )
    {
        fprintf( InputFile, "%lg, ", m_DeflectionGainVec[i]->Get() );
    }
    fprintf( InputFile, "%lg\n", m_DeflectionGainVec[i]->Get() );

    // group deflection angle
    fprintf( InputFile, "%lg\n", m_DeflectionAngle() );

}

void ControlSurfaceGroup::Load_STP_Data( FILE *InputFile )
{
    //TODO - need to write function to load data from .vspaero file
}

xmlNodePtr ControlSurfaceGroup::EncodeXml( xmlNodePtr & node )
{
    char str[256];
    if (node)
    {
        ParmContainer::EncodeXml(node);

        XmlUtil::AddStringNode(node, "ParentGeomBase", m_ParentGeomBaseID.c_str());

        XmlUtil::AddIntNode(node, "NumberOfControlSubSurfaces", m_ControlSurfVec.size());
        for (size_t i = 0; i < m_ControlSurfVec.size(); ++i)
        {
            sprintf(str, "Control_Surface_%u", i );
            xmlNodePtr csnode = xmlNewChild( node, NULL, BAD_CAST str , NULL );
            sprintf(str, "SSID%u", i);
            XmlUtil::AddStringNode(csnode, str, m_ControlSurfVec[i].SSID.c_str());
            sprintf(str, "ParentGeomID%u", i);
            XmlUtil::AddStringNode(csnode, str, m_ControlSurfVec[i].parentGeomId.c_str());
            sprintf(str, "iReflect%u", i);
            XmlUtil::AddIntNode(csnode, str, m_ControlSurfVec[i].iReflect);
        }

    }

    return node;
}

xmlNodePtr ControlSurfaceGroup::DecodeXml( xmlNodePtr & node )
{
    char str[256];
    unsigned int nControlSubSurfaces = 0;
    string ParentGeomID = "GeomID";
    string SSID = "SSID";
    bool IsGrouped = false;
    bool IsUsed = false;
    int iSubSurf = 0;
    int iReflect = 0;
    VspAeroControlSurf newSurf;

    if ( node )
    {
        ParmContainer::DecodeXml( node );

        m_ParentGeomBaseID = XmlUtil::FindString(node, "ParentGeomBase", ParentGeomID );

        nControlSubSurfaces = XmlUtil::FindInt( node, "NumberOfControlSubSurfaces", nControlSubSurfaces );
        for (size_t i = 0; i < nControlSubSurfaces; ++i )
        {
            sprintf( str, "Control_Surface_%u", i );
            xmlNodePtr csnode = XmlUtil::GetNode(node, str, 0);
            sprintf( str, "SSID%u",i);
            newSurf.SSID = XmlUtil::FindString( csnode, str, SSID );
            sprintf( str, "ParentGeomID%u", i);
            newSurf.parentGeomId = XmlUtil::FindString(csnode, str, ParentGeomID );
            sprintf(str, "iReflect%u",i);
            newSurf.iReflect = XmlUtil::FindInt( csnode, str, iReflect );
            AddSubSurface( newSurf );
        }
    }

    return node;
}

void ControlSurfaceGroup::AddSubSurface( VspAeroControlSurf control_surf )
{
    // Add deflection gain parm to ControlSurfaceGroup container
    Parm* p = ParmMgr.CreateParm( PARM_DOUBLE_TYPE );
    char str[256];

    if ( p )
    {
        //  parm name: control_surf->fullName (example: MainWing_Surf1_Aileron)
        //  group: "CSGQualities"
        //  initial value: control_surf->deflection_gain
        sprintf(str, "Surf%i_Gain", control_surf.iReflect );
        p->Init( str, "CSGQualities", this, 1.0, -1.0e6, 1.0e6 );
        p->SetDescript( "Deflection gain for the individual sub surface to be used for control mixing and allocation within the control surface group" );
        m_DeflectionGainVec.push_back( p );
    }

    m_ControlSurfVec.push_back( control_surf );
}

void ControlSurfaceGroup::RemoveSubSurface( const string & ssid, int reflec_num )
{
    for (size_t i = 0; i < m_ControlSurfVec.size(); ++i)
    {
        if ( m_ControlSurfVec[i].SSID.compare( ssid ) == 0 && m_ControlSurfVec[i].iReflect == reflec_num )
        {
            m_ControlSurfVec.erase( m_ControlSurfVec.begin() + i);
            delete m_DeflectionGainVec[i];
            m_DeflectionGainVec.erase( m_DeflectionGainVec.begin() + i);
            return;
        }
    }
}

void ControlSurfaceGroup::UpdateParmGroupName()
{
    char str[256];
    sprintf( str, "CSGQualities_%s", m_Name.c_str() );
    m_IsUsed.SetGroupName( str );
    m_DeflectionAngle.SetGroupName( str );
    for (size_t i = 0; i < m_DeflectionGainVec.size(); ++i )
    {
        m_DeflectionGainVec[i]->SetGroupName( str );
    }
}
