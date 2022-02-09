//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParasiteDragMgr.cpp: ParasiteDrag Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "Vehicle.h"
#include "ParmMgr.h"
#include "StlHelper.h"
#include "WingGeom.h"
#include "ParasiteDragMgr.h"

#include <numeric>

//==== Constructor ====//
ParasiteDragMgrSingleton::ParasiteDragMgrSingleton() : ParmContainer()
{
    // Initial Values for certain variables
    SetDefaultStruct();
    m_Name = "ParasiteDragSettings";
    m_FileName = "ParasiteDragBuildUp.csv";
    string groupname = "ParasiteDrag";
    m_LamCfEqnName = "Blasius";
    m_TurbCfEqnName = "Blasius Power Law";
    m_RefGeomID = "";
    m_CurrentExcresIndex = -1;
    m_CompGeomResults = NULL;

    // ==== Parm Initialize and Description Setting ==== //
    // Reference Qualities Parms
    m_SortByFlag.Init( "SortBy", groupname, this, PD_SORT_NONE, PD_SORT_NONE, PD_SORT_PERC_CD );
    m_SortByFlag.SetDescript( "Flag to determine what geometries are sorted by" );

    m_RefFlag.Init( "RefFlag", groupname, this, vsp::MANUAL_REF, vsp::MANUAL_REF, vsp::COMPONENT_REF );
    m_RefFlag.SetDescript( "Reference quantity flag" );

    m_Sref.Init( "Sref", groupname, this, 100.0, 0.0, 1e12 );
    m_Sref.SetDescript( "Reference area" );

    m_LamCfEqnType.Init( "LamCfEqnType", groupname, this, vsp::CF_LAM_BLASIUS, vsp::CF_LAM_BLASIUS, vsp::CF_LAM_BLASIUS_W_HEAT );
    m_LamCfEqnType.SetDescript( "Laminar Cf Equation Choice" );

    m_TurbCfEqnType.Init( "TurbCfEqnType", groupname, this, m_TurbTypeDefault, vsp::CF_TURB_EXPLICIT_FIT_SPALDING,
                          vsp::CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH );
    m_TurbCfEqnType.SetDescript( "Turbulent Cf Equation Choice" );

    m_AltLengthUnit.Init( "AltLengthUnit", groupname, this, vsp::PD_UNITS_IMPERIAL, vsp::PD_UNITS_IMPERIAL, vsp::PD_UNITS_METRIC );
    m_AltLengthUnit.SetDescript( "Altitude Units" );

    m_LengthUnit.Init( "LengthUnit", groupname, this, vsp::LEN_FT, vsp::LEN_MM, vsp::LEN_UNITLESS );
    m_LengthUnit.SetDescript( "Length Units" );

    m_TempUnit.Init( "TempUnit", groupname, this, vsp::TEMP_UNIT_F, vsp::TEMP_UNIT_K, vsp::TEMP_UNIT_R );
    m_TempUnit.SetDescript( "Temperature Units" );

    // Air Qualities Parms
    m_FreestreamType.Init( "FreestreamType", groupname, this, vsp::ATMOS_TYPE_US_STANDARD_1976,
                           vsp::ATMOS_TYPE_US_STANDARD_1976, vsp::ATMOS_TYPE_MANUAL_RE_L );
    m_FreestreamType.SetDescript( "Assigns the desired inputs to describe the freestream properties" );

    m_Mach.Init( "Mach", groupname, this, 0.0, 0.0, 1000.0 );
    m_Mach.SetDescript( "Mach Number for Current Flight Condition" );

    m_ReqL.Init( "Re_L", groupname, this, 0.0, 0.0, 1e12 );
    m_ReqL.SetDescript( "Reynolds Number Per Unit Length" );

    m_Temp.Init( "Temp", groupname, this, 288.15, -459.67, 1e12 );
    m_Temp.SetDescript( "Temperature" );

    m_Pres.Init( "Pres", groupname, this, 2116.221, 1e-4, 1e12 );
    m_Pres.SetDescript( "Pressure" );

    m_Rho.Init( "Density", groupname, this, 0.07647, 1e-12, 1e12 );
    m_Rho.SetDescript( "Density" );

    m_DynaVisc.Init( "DynaVisc", groupname, this, 0.0, 1e-12, 1e12 );
    m_DynaVisc.SetDescript( "Dynamic Viscosity for Current Condition" );

    m_SpecificHeatRatio.Init( "SpecificHeatRatio", groupname, this, 1.4, -1, 1e3 );
    m_SpecificHeatRatio.SetDescript( "Specific Heat Ratio" );

    //m_MediumType.Init("Medium", groupname, this, Atmosphere::MEDIUM_AIR, Atmosphere::MEDIUM_AIR, Atmosphere::MEDIUM_PURE_WATER);
    //m_MediumType.SetDescript("Wind Tunnel Medium");

    m_Vinf.Init( "Vinf", groupname, this, 500.0, 0.0, 1e12 );
    m_Vinf.SetDescript( "Free Stream Velocity" );

    m_VinfUnitType.Init( "VinfUnitType", groupname, this, vsp::V_UNIT_FT_S, vsp::V_UNIT_FT_S, vsp::V_UNIT_MACH );
    m_VinfUnitType.SetDescript( "Units for Freestream Velocity" );

    m_Hinf.Init( "Alt", groupname, this, 20000.0, 0.0, 271823.3 );
    m_Hinf.SetDescript( "Physical Altitude from Sea Level" );

    m_DeltaT.Init( "DeltaTemp", groupname, this, 0.0, -1e12, 1e12 );
    m_DeltaT.SetDescript( "Delta Temperature from STP" );

    m_ExportSubCompFlag.Init( "ExportSubCompFlag", groupname, this, false, false, true );
    m_ExportSubCompFlag.SetDescript( "Flag to Export Sub-Component Information" );

    // Excrescence Parm
    m_ExcresValue.Init( "ExcresVal", groupname, this, 0.0, 0.0, 200 );
    m_ExcresValue.SetDescript( "Excrescence Value" );

    m_ExcresType.Init( "ExcresType", groupname, this, vsp::EXCRESCENCE_COUNT, vsp::EXCRESCENCE_COUNT, vsp::EXCRESCENCE_DRAGAREA );
    m_ExcresType.SetDescript( "Excrescence Type" );

    m_SetChoice.Init( "Set", groupname, this, DEFAULT_SET, 0, NUM_SETS + 2 );
    m_SetChoice.SetDescript( "Selected set for operation" );

    // Recompute flag, if true degen/compgeom will be run even if an existing degen geom and comp geom exist from
    // a previous calculation
    m_RecomputeGeom = true;

    // Populate black-list of equations to not use.
    int tmp[] = { vsp::DO_NOT_USE_CF_TURB_IMPLICIT_KARMAN,
                  vsp::DO_NOT_USE_CF_TURB_SCHLICHTING_INCOMPRESSIBLE,
                  vsp::DO_NOT_USE_CF_TURB_SCHLICHTING_PRANDTL,
                  vsp::DO_NOT_USE_CF_TURB_SCHULTZ_GRUNOW_HIGH_RE,
                  vsp::DO_NOT_USE_CF_TURB_WHITE_CHRISTOPH_COMPRESSIBLE,
                  vsp::DO_NOT_USE_CF_TURB_ROUGHNESS_SCHLICHTING_LOCAL,
                  vsp::DO_NOT_USE_CF_TURB_ROUGHNESS_WHITE };
    m_TurbBlackList.insert( m_TurbBlackList.begin(), tmp, &tmp[ sizeof( tmp ) / sizeof( *tmp ) ] );

    // Populate list of alternate equations for black-listed ones.
    int tmp2[] = { m_TurbTypeDefault,
                   m_TurbTypeDefault,
                   m_TurbTypeDefault,
                   vsp::CF_TURB_SCHULTZ_GRUNOW_SCHOENHERR,
                   m_TurbTypeDefault,
                   m_TurbTypeDefault,
                   vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG };
    m_TurbAlternateList.insert( m_TurbAlternateList.begin(), tmp2, &tmp2[ sizeof( tmp2 ) / sizeof( *tmp2 ) ] );

}

void ParasiteDragMgrSingleton::Renew()
{
    m_TableRowVec.clear();
    m_ExcresRowVec.clear();

    m_DegenGeomVec.clear();
    m_CompGeomResults = NULL;

    SetDefaultStruct();

    m_FileName = "ParasiteDragBuildUp.csv";
    m_LamCfEqnName = "Blasius";
    m_TurbCfEqnName = "Blasius Power Law";
    m_RefGeomID = "";

    m_ExcresType = 0;
    m_ExcresValue = 0;

    m_CurrentExcresIndex = -1;
}

void ParasiteDragMgrSingleton::SetDefaultStruct()
{
    m_DefaultStruct.MasterRow = false;
    m_DefaultStruct.GeomID = "";
    m_DefaultStruct.SubSurfID = "";
    m_DefaultStruct.Label = "";
    m_DefaultStruct.Swet = -1;
    m_DefaultStruct.Lref = -1;
    m_DefaultStruct.Re = -1;
    m_DefaultStruct.Roughness = 0.0;
    m_DefaultStruct.TeTwRatio = 1;
    m_DefaultStruct.TawTwRatio = 1;
    m_DefaultStruct.PercLam = 0.0;
    m_DefaultStruct.Cf = -1;
    m_DefaultStruct.fineRatorToC = -1;
    m_DefaultStruct.GeomShapeType = 0;
    m_DefaultStruct.FFEqnChoice = 0;
    m_DefaultStruct.FF = -1;
    m_DefaultStruct.Q = 1;
    m_DefaultStruct.f = -1;
    m_DefaultStruct.CD = -1;
    m_DefaultStruct.PercTotalCD = -1;
    m_DefaultStruct.SurfNum = 0;
    m_DefaultStruct.GroupedAncestorGen = 0;
    m_DefaultStruct.ExpandedList = false;
}

void ParasiteDragMgrSingleton::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;
        return;
    }

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

void ParasiteDragMgrSingleton::InitTableVec()
{
    m_TableRowVec.clear();
    for ( int i = 0; i < m_RowSize; ++i )
    {
        m_TableRowVec.push_back( m_DefaultStruct );
    }
}

void ParasiteDragMgrSingleton::LoadMainTableUserInputs()
{
    Geom* geom;
    char str[256];
    string lastGeomID = "";
    string lastSSID = "";
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return;
    }

    for ( int i = 0; i < m_PDGeomIDVec.size(); i++ )
    {
        geom = veh->FindGeom( m_PDGeomIDVec[i] );
        if ( geom )
        {
            if ( geom->GetID().compare( lastGeomID ) != 0 )
            {
                // Create Master Geom Row
                m_geo_masterRow.push_back( true );
                m_geo_name.push_back( geom->GetName() );
                m_geo_groupedAncestorGen.push_back( geom->m_GroupedAncestorGen() );
                m_geo_percLam.push_back( geom->m_PercLam() );
                m_geo_ffIn.push_back( geom->m_FFUser() );
                m_geo_Q.push_back( geom->m_Q() );
                m_geo_Roughness.push_back( geom->m_Roughness() );
                m_geo_TeTwRatio.push_back( geom->m_TeTwRatio() );
                m_geo_TawTwRatio.push_back( geom->m_TawTwRatio() );
                m_geo_surfNum.push_back( 0 );
                m_geo_expandedList.push_back( geom->m_ExpandedListFlag() );

                if ( geom->GetType().m_Type == CUSTOM_GEOM_TYPE )
                {
                    if ( geom->GetSurfType(0) == vsp::NORMAL_SURF )
                    {
                        sprintf( str, "[B] %s", geom->GetName().c_str() );
                    }
                    else
                    {
                        sprintf( str, "[W] %s", geom->GetName().c_str() );
                    }
                }
                else
                {
                    sprintf( str, "%s", geom->GetName().c_str() );
                }

                m_geo_shapeType.push_back( geom->GetSurfType(0) ); // Form Factor Shape Type

                if ( geom->GetSurfType(0) == vsp::NORMAL_SURF )
                {
                    m_geo_ffType.push_back( geom->m_FFBodyEqnType() );
                }
                else
                {
                    m_geo_ffType.push_back( geom->m_FFWingEqnType() );
                }
                m_geo_geomID.push_back( geom->GetID() );
                m_geo_subsurfID.push_back( "" );

                // Assign Label to Geom
                m_geo_label.push_back( str );

                lastGeomID = geom->GetID();
                --i;
            }
            else
            {
                for ( int j = 0; j < geom->GetNumTotalSurfs(); j++ )
                {
                    // Custom Geom Check: if surf type is the same, apply same qualities
                    if ( j > 0 && geom->GetSurfType(j) == geom->GetSurfType(j - 1) )
                    {
                        m_geo_masterRow.push_back( false );
                        m_geo_name.push_back( geom->GetName() );
                        m_geo_groupedAncestorGen.push_back( geom->m_GroupedAncestorGen() );
                        m_geo_percLam.push_back( m_geo_percLam[m_geo_percLam.size() - 1] );
                        m_geo_ffIn.push_back( m_geo_ffIn[m_geo_ffIn.size() - 1] );
                        m_geo_Q.push_back( m_geo_Q[m_geo_Q.size() - 1] );
                        m_geo_Roughness.push_back( m_geo_Roughness[m_geo_Roughness.size() - 1] );
                        m_geo_TeTwRatio.push_back( m_geo_TeTwRatio[m_geo_TeTwRatio.size() - 1] );
                        m_geo_TawTwRatio.push_back( m_geo_TawTwRatio[m_geo_TawTwRatio.size() - 1] );
                        m_geo_surfNum.push_back( j );
                        m_geo_expandedList.push_back( false );
                        sprintf( str, "%s_%d", geom->GetName().c_str(), j );
                    }
                    else
                    {
                        if ( geom->GetType().m_Type == CUSTOM_GEOM_TYPE )
                        {
                            if ( geom->GetSurfType(j) == vsp::NORMAL_SURF )
                            {
                                sprintf( str, "[B] %s", geom->GetName().c_str() );
                            }
                            else
                            {
                                sprintf( str, "[W] %s", geom->GetName().c_str() );
                            }
                            m_geo_surfNum.push_back( j );
                        }
                        else
                        {
                            sprintf( str, "%s_%i", geom->GetName().c_str(), j );
                            m_geo_surfNum.push_back( 0 );
                        }
                        m_geo_masterRow.push_back( false );
                        m_geo_name.push_back( geom->GetName() );
                        m_geo_groupedAncestorGen.push_back( geom->m_GroupedAncestorGen() );
                        m_geo_percLam.push_back( m_geo_percLam[m_geo_percLam.size() - 1] );
                        m_geo_ffIn.push_back( m_geo_ffIn[m_geo_ffIn.size() - 1] );
                        m_geo_Q.push_back( m_geo_Q[m_geo_Q.size() - 1] );
                        m_geo_Roughness.push_back( m_geo_Roughness[m_geo_Roughness.size() - 1] );
                        m_geo_TeTwRatio.push_back( m_geo_TeTwRatio[m_geo_TeTwRatio.size() - 1] );
                        m_geo_TawTwRatio.push_back( m_geo_TawTwRatio[m_geo_TawTwRatio.size() - 1] );
                        m_geo_expandedList.push_back( false );
                    }

                    m_geo_shapeType.push_back( geom->GetSurfType(j) ); // Form Factor Shape Type

                    if ( geom->GetSurfType(j) == vsp::NORMAL_SURF )
                    {
                        m_geo_ffType.push_back( geom->m_FFBodyEqnType() );
                    }
                    else
                    {
                        m_geo_ffType.push_back( geom->m_FFWingEqnType() );
                    }
                    m_geo_geomID.push_back( geom->GetID() );
                    m_geo_subsurfID.push_back( "" );

                    // Assign Label to Geom
                    m_geo_label.push_back( str );
                }

                // Sub Surfaces
                for ( int j = 0; j < geom->GetSubSurfVec().size(); j++ )
                {
                    SubSurface* surf = geom->GetSubSurfVec()[j];
                    for ( int k = 0; k < geom->GetNumTotalSurfs(); ++k )
                    {
                        if ( surf->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT && lastSSID.compare( surf->GetID() ) != 0 )
                        {
                            m_geo_masterRow.push_back( true );
                            m_geo_name.push_back( surf->GetName() );
                            m_geo_percLam.push_back( surf->m_PercLam() );
                            m_geo_ffIn.push_back( surf->m_FFUser() );
                            m_geo_Q.push_back( surf->m_Q() );
                            m_geo_Roughness.push_back( surf->m_Roughness() );
                            m_geo_TeTwRatio.push_back( surf->m_TeTwRatio() );
                            m_geo_TawTwRatio.push_back( surf->m_TawTwRatio() );
                            if ( geom->GetSurfType(k) == vsp::NORMAL_SURF )
                            {
                                m_geo_ffType.push_back( surf->m_FFBodyEqnType() );
                            }
                            else
                            {
                                m_geo_ffType.push_back( surf->m_FFWingEqnType() );
                            }
                        }
                        else
                        {
                            m_geo_masterRow.push_back( false );
                            m_geo_name.push_back( surf->GetName() );
                            m_geo_percLam.push_back( m_geo_percLam[m_geo_percLam.size() - 1] );
                            m_geo_ffIn.push_back( m_geo_ffIn[m_geo_ffIn.size() - 1] );
                            m_geo_Q.push_back( m_geo_Q[m_geo_Q.size() - 1] );
                            m_geo_Roughness.push_back( m_geo_Roughness[m_geo_Roughness.size() - 1] );
                            m_geo_TeTwRatio.push_back( m_geo_TeTwRatio[m_geo_TeTwRatio.size() - 1] );
                            m_geo_TawTwRatio.push_back( m_geo_TawTwRatio[m_geo_TawTwRatio.size() - 1] );

                            if ( geom->GetSurfType(k) == vsp::NORMAL_SURF )
                            {
                                m_geo_ffType.push_back( surf->m_FFBodyEqnType() );
                            }
                            else
                            {
                                m_geo_ffType.push_back( surf->m_FFWingEqnType() );
                            }
                        }

                        m_geo_groupedAncestorGen.push_back( -1 );
                        m_geo_surfNum.push_back( k );
                        m_geo_shapeType.push_back( geom->GetSurfType(k) ); // Form Factor Shape Type
                        m_geo_expandedList.push_back( false );
                        m_geo_geomID.push_back( geom->GetID() );
                        m_geo_subsurfID.push_back( surf->GetID() );

                        if ( lastSSID.compare( surf->GetID() ) != 0 && surf->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT )
                        {
                            sprintf( str, "[ss] %s", surf->GetName().c_str() );
                            m_geo_label.push_back( str );
                            lastSSID = surf->GetID();
                            --k;
                        }
                        else
                        {
                            sprintf( str, "[ss] %s_%i", surf->GetName().c_str(), k );
                            m_geo_label.push_back( str );
                        }
                    }
                }
            }
        }
    }
}

void ParasiteDragMgrSingleton::SetupFullCalculation()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh && (m_RecomputeGeom || (m_DegenGeomVec.size() == 0 && !m_CompGeomResults)))
    {
        veh->ClearDegenGeom();
        ResultsMgr.DeleteResult( ResultsMgr.FindResultsID( "Comp_Geom" ) );
        ClearInputVectors();
        ClearOutputVectors();

        vector < string > geomIDVec = veh->GetGeomSet( m_SetChoice() );

        veh->CreateDegenGeom( m_SetChoice() );
        string meshID = veh->CompGeomAndFlatten( m_SetChoice(), 0 );
        veh->DeleteGeom( meshID );

        // Restore set visibility. At this point, all geoms in the set will only be in the 
        //  Not_Shown set. We want the Parasite Drag table to contain the same geoms before 
        //  and after tool execution.
        if ( m_SetChoice() == vsp::SET_NOT_SHOWN )
        {
            veh->ShowSet( 0 ); // show all
        }

        for ( size_t i = 0; i < geomIDVec.size(); i++ )
        {
            Geom* geom = veh->FindGeom( geomIDVec[i] );
            if ( geom )
            {
                if ( m_SetChoice() == vsp::SET_NOT_SHOWN ) // Place back in Not_Shown set 
                {
                    geom->SetSetFlag( vsp::SET_SHOWN, false );
                    geom->SetSetFlag( vsp::SET_NOT_SHOWN, true );
                }
                else // Show only indicated set
                {
                    geom->SetSetFlag( vsp::SET_SHOWN, true );
                    geom->SetSetFlag( vsp::SET_NOT_SHOWN, false );
                }
            }
        }

        // First Assignment of DegenGeomVec, Will Carry Through to Rest of Calculate_X
        m_DegenGeomVec = veh->GetDegenGeomVec();

        // First Assignment of CompGeon, Will Carry Through to Rest of Calculate_X
        m_CompGeomResults = ResultsMgr.FindResults( "Comp_Geom" );
    }
}

int ParasiteDragMgrSingleton::CalcRowSize()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return 0;
    }

    m_RowSize = 0; // Reset every call
    for ( int i = 0; i < m_PDGeomIDVec.size(); i++ )
    {
        Geom* geom = veh->FindGeom( m_PDGeomIDVec[i] );
        if ( geom )
        {
            m_RowSize += geom->GetNumTotalSurfs() + 1; // +1 for Master Geom Row that keeps track of totals

            // Add Row / SubSurface
            for ( size_t j = 0; j < geom->GetSubSurfVec().size(); ++j )
            {
                for ( size_t k = 0; k < geom->GetNumSymmCopies(); ++k )
                {
                    ++m_RowSize;
                }

                // Subsurfaces are essentially separate geometries and as such get their own master row
                if ( geom->GetSubSurfVec()[j]->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT )
                {
                    ++m_RowSize;
                }
            }
        }
    }
    return m_RowSize;
}

void ParasiteDragMgrSingleton::Calculate_Swet()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    char str[256];
    string newstr;
    int searchIndex;

    if ( !veh )
    {
        return;
    }

    int iSurf = 0;
    for ( int i = 0; i < m_RowSize; ++i )
    {
        if ( !m_DegenGeomVec.empty() )
        {
            Geom* geom = veh->FindGeom( m_geo_geomID[i] );
            if (geom)
            {
                // If DegenGeom Exists Pull Swet
                if ( !m_geo_masterRow[i] )
                {
                    vector < string > tagidvec = m_CompGeomResults->Find( "Tag_ID" ).GetStringData();
                    if ( m_geo_subsurfID[i].compare( "" ) == 0 )
                    {
                        sprintf( str, "%s_Surf%i", geom->GetID().c_str(), m_geo_surfNum[i] );
                        newstr = str;
                        searchIndex = vector_find_val( tagidvec, newstr );
                        m_geo_swet.push_back( m_CompGeomResults->Find( "Tag_Wet_Area" ).GetDouble( searchIndex ) );
                        ++iSurf;
                    }
                    else
                    {
                        sprintf( str, "%s_Surf%i,%s", geom->GetID().c_str(), m_geo_surfNum[i],
                             m_geo_subsurfID[i].c_str() );
                        newstr = str;
                        searchIndex = vector_find_val( tagidvec, newstr );
                        m_geo_swet.push_back( m_CompGeomResults->Find( "Tag_Wet_Area" ).GetDouble( searchIndex ) );
                    }
                }
                else
                {
                    m_geo_swet.push_back( 0.0 ); // Master Geom Swet is 0 until updated by UpdateWettedAreaTotals()
                }
            }
            else
            {
                m_geo_swet.push_back( 0.0 );
            }
        }
        else
        {
            // Else Push Back Default Val
            m_geo_swet.push_back( -1 );
        }
    }

    UpdateWettedAreaTotals();
}

void ParasiteDragMgrSingleton::Calculate_Lref()
{
    // Initialize Variablse
    int iSurf = 0;

    for ( int i = 0; i < m_RowSize; ++i )
    {
        if ( !m_DegenGeomVec.empty() )
        {
            Geom* geom = VehicleMgr.GetVehicle()->FindGeom(m_geo_geomID[i]);
            if ( geom )
            {
                // If DegenGeom Exists Calculate Lref
                if ( m_geo_masterRow[i] )
                {
                    if ( m_geo_subsurfID[i].compare( "" ) == 0 )
                    {
                        if ( m_DegenGeomVec[iSurf].getType() != DegenGeom::DISK_TYPE )
                        {
                            m_geo_lref.push_back( CalcReferenceLength( iSurf ) );

                            if (geom->GetType().m_Type != PROP_GEOM_TYPE)
                            {
                                iSurf += geom->GetNumSymmCopies();
                            }
                            else
                            {
                                string numBladesID = geom->FindParm("NumBlade", "Design");
                                IntParm* tNumBladeParm = (IntParm*)ParmMgr.FindParm(numBladesID);
                                if (tNumBladeParm)
                                {
                                    iSurf += tNumBladeParm->Get() * geom->GetNumSymmCopies();
                                }
                            }
                        }
                        else
                        {
                            --i;
                            iSurf += geom->GetNumSymmCopies();
                        }
                    }
                    else
                    {
                        m_geo_lref.push_back( CalcReferenceLength( iSurf - 1 ) );
                    }
                }
                else
                {
                    m_geo_lref.push_back( m_geo_lref[m_geo_lref.size() - 1] );
                }
            }
            else
            {
                // Else Push Back Default Val
                m_geo_lref.push_back( -1 );
            }
        }
        else
        {
            // Else Push Back Default Val
            m_geo_lref.push_back( -1 );
        }
    }
}

double ParasiteDragMgrSingleton::CalcReferenceLength( int index )
{
    double lref = 0;
    if ( m_DegenGeomVec[index].getType() == DegenGeom::BODY_TYPE )
    {
        lref = CalcReferenceBodyLength( index );

        if ( lref <= 1e-6 )
        {
            lref = CalcReferenceChord( index );
        }
    }
    else if ( m_DegenGeomVec[index].getType() == DegenGeom::SURFACE_TYPE )
    {
        lref = CalcReferenceChord( index );

        if ( lref <= 1e-6 )
        {
            lref = CalcReferenceBodyLength( index );
        }
    }

    if ( lref <= 1e-6 )
    {
        return 1.0;
    }
    else
    {
        return lref;
    }
}

// Use Bounding Box to approximate x directional length
double ParasiteDragMgrSingleton::CalcReferenceBodyLength( int index )
{
    // TODO: Improve Reference Length Calculations
    double delta_x, delta_y, delta_z, lref;
    vector <DegenStick> m_DegenStick = m_DegenGeomVec[index].getDegenSticks();
    delta_x = abs( m_DegenStick[0].xle.front().x() - m_DegenStick[0].xle.back().x() );
    delta_y = abs( m_DegenStick[0].xle.front().y() - m_DegenStick[0].xle.back().y() );
    delta_z = abs( m_DegenStick[0].xle.front().z() - m_DegenStick[0].xle.back().z() );
    lref = sqrt( pow( delta_x, 2.0 ) + pow( delta_y, 2.0 ) + pow( delta_z, 2.0 ) );

    return lref;
}

// Use weighted average to approximate reference chord
double ParasiteDragMgrSingleton::CalcReferenceChord( int index )
{
    // TODO: Improve Reference Length Calculations
    vector <DegenStick> m_DegenStick = m_DegenGeomVec[index].getDegenSticks();
    double secArea, totalArea = 0, weightedChordSum = 0;
    double delta_x, delta_y, delta_z, section_span;
    for ( size_t j = 0; j <= m_DegenStick[0].areaTop.size() - 1; ++j )
    {
        delta_x = abs( m_DegenStick[0].xle[j].x() - m_DegenStick[0].xle[j + 1].x() );
        delta_y = abs( m_DegenStick[0].xle[j].y() - m_DegenStick[0].xle[j + 1].y() );
        delta_z = abs( m_DegenStick[0].xle[j].z() - m_DegenStick[0].xle[j + 1].z() );
        section_span = sqrt( pow( delta_x, 2.0 ) + pow( delta_y, 2.0 ) + pow( delta_z, 2.0 ) );
        secArea = section_span * ( 0.5 * ( m_DegenStick[0].chord[j] + m_DegenStick[0].chord[j + 1] ) );

        totalArea += secArea;

        weightedChordSum += ( ( 0.5 * ( m_DegenStick[0].chord[j] + m_DegenStick[0].chord[j + 1] ) ) * secArea );
    }

    return weightedChordSum / totalArea;
}

void ParasiteDragMgrSingleton::Calculate_Re()
{
    for ( int i = 0; i < m_RowSize; ++i )
    {
        if ( !m_DegenGeomVec.empty() )
        {
            // If DegenGeom Exists Calculate Re
            if ( m_geo_subsurfID[i].compare( "" ) == 0 )
            {
                ReynoldsNumCalc( i );
            }
            else
            {
                m_geo_Re.push_back( m_geo_Re[m_geo_Re.size() - 1] );
            }

        }
        else
        {
            // Else Push Back Default Val
            m_geo_Re.push_back( -1 );
        }
    }

    CalcRePowerDivisor();
}

void ParasiteDragMgrSingleton::CalcRePowerDivisor()
{
    if ( !m_geo_Re.empty() )
    {
        vector<double>::const_iterator it = max_element( m_geo_Re.begin(), m_geo_Re.end() );
        m_ReynoldsPowerDivisor = mag( *it );
    }
    else
    {
        m_ReynoldsPowerDivisor = 1;
    }
}

void ParasiteDragMgrSingleton::ReynoldsNumCalc( int index )
{
    double vinf, lref;
    if ( m_FreestreamType() != vsp::ATMOS_TYPE_MANUAL_RE_L )
    {
        if ( m_VinfUnitType() == vsp::V_UNIT_MACH )
        {
            double soundspeed = m_Atmos.GetSoundSpeed();
            soundspeed = ConvertVelocity( soundspeed, vsp::V_UNIT_M_S, m_VinfUnitType() );
            vinf = m_Vinf() * soundspeed;
        }
        else if ( m_VinfUnitType() == vsp::V_UNIT_KEAS )
        {
            vinf = m_Vinf() * sqrt( 1.0 / m_Atmos.GetDensityRatio() );
        }
        else
        {
            vinf = m_Vinf();
        }
        lref = m_geo_lref[index];

        if ( m_AltLengthUnit() == vsp::PD_UNITS_IMPERIAL )
        {
            vinf = ConvertVelocity( vinf, m_VinfUnitType.Get(), vsp::V_UNIT_FT_S );
            lref = ConvertLength( lref, m_LengthUnit(), vsp::LEN_FT );
        }
        else if ( m_AltLengthUnit() == vsp::PD_UNITS_METRIC )
        {
            vinf = ConvertVelocity( vinf, m_VinfUnitType.Get(), vsp::V_UNIT_M_S );
            lref = ConvertLength( lref, m_LengthUnit(), vsp::LEN_M );
        }

        m_geo_Re.push_back( ( vinf * lref ) / m_KineVisc() );
    }
    else
    {
        // Any other freestream definition type
        m_geo_Re.push_back( m_ReqL() * m_geo_lref[index] );
    }
}

void ParasiteDragMgrSingleton::Calculate_Cf()
{
    // Initialize Variables
    double lref, rho, kineVisc, vinf;

    for ( int i = 0; i < m_RowSize; ++i )
    {
        if ( !m_DegenGeomVec.empty() )
        {
            // If DegenGeom Exists Calculate Cf
            if ( m_geo_subsurfID[i].compare( "" ) == 0 ||
                    VehicleMgr.GetVehicle()->FindGeom( m_geo_geomID[i] )->GetSubSurf( m_geo_subsurfID[i] )->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT )
            {
                vinf = ConvertVelocity( m_Vinf(), m_VinfUnitType.Get(), vsp::V_UNIT_M_S );
                rho = ConvertDensity( m_Atmos.GetDensity(), m_AltLengthUnit(), vsp::RHO_UNIT_KG_M3 ); // lb/ft3 to kg/m3
                lref = ConvertLength( m_geo_lref[i], m_LengthUnit(), vsp::LEN_M );
                kineVisc = m_Atmos.GetDynaVisc() / rho;

                // Will calculate full turbulence
                m_geo_cf.push_back( CalcPartialTurbulence( m_geo_percLam[i], m_geo_Re[i], m_geo_lref[i], m_ReqL(),
                   m_geo_Roughness[i], m_geo_TawTwRatio[i], m_geo_TeTwRatio[i] ) );
            }
            else
            {
                m_geo_cf.push_back( m_geo_cf[m_geo_cf.size() - 1] );
            }
        }
        else
        {
            // Else push back default value
            m_geo_cf.push_back( -1 );
        }
    }
}

double ParasiteDragMgrSingleton::CalcPartialTurbulence( double perclam, double re, double lref, double reqL,
    double roughness, double tawtwrat, double tetwrat )
{
    double cf = 0;
    if ( re > 0 )
    {
        // Prevent dividing by 0 in some equations
        double LamPerc = ( perclam / 100 );
        double CffullTurb = CalcTurbCf( re, lref, m_TurbCfEqnType(), roughness, m_SpecificHeatRatio(), tawtwrat, tetwrat );
        double CffullLam = CalcLamCf( re, m_LamCfEqnType.Get() ); // WARNING: Not used

        double LamPercRefLen = LamPerc * lref;

        double ReLam = reqL * LamPercRefLen;

        double CfpartLam = CalcLamCf( ReLam, m_LamCfEqnType() );
        double CfpartTurb = CalcTurbCf( ReLam, lref, m_TurbCfEqnType(), roughness, m_SpecificHeatRatio(), tawtwrat, tetwrat );

        m_TurbCfEqnName = AssignTurbCfEqnName( m_TurbCfEqnType() );
        m_LamCfEqnName = AssignLamCfEqnName( m_LamCfEqnType() );

        cf = CffullTurb - ( CfpartTurb * LamPerc ) + ( CfpartLam * LamPerc );
    }
    return cf;
}

void ParasiteDragMgrSingleton::Calculate_fineRat_and_toc()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    char str[256];
    string newstr;
    int searchIndex;

    if ( !veh )
    {
        return;
    }

    int iSurf = 0;

    for ( int i = 0; i < m_RowSize; ++i )
    {
        if ( !m_DegenGeomVec.empty() )
        {
            Geom* geom = veh->FindGeom( m_geo_geomID[i] );
            if (geom)
            {
                // If DegenGeom Exists Calculate Fineness Ratio
                if ( m_geo_masterRow[i] )
                {
                    if ( m_geo_subsurfID[i].compare( "" ) == 0 )
                    {
                        if ( m_DegenGeomVec[iSurf].getType() != DegenGeom::DISK_TYPE )
                        {
                            m_geo_fineRat_or_toc.push_back( CalculateFinessRatioAndTOC( iSurf, i) );

                            if ( geom->GetType().m_Type != PROP_GEOM_TYPE )
                            {
                                iSurf += geom->GetNumSymmCopies();
                            }
                            else
                            {
                                string numBladesID = geom->FindParm( "NumBlade", "Design" );
                                IntParm* tNumBladeParm = (IntParm*) ParmMgr.FindParm( numBladesID );
                                if ( tNumBladeParm )
                                {
                                    iSurf += tNumBladeParm->Get() * geom->GetNumSymmCopies();
                                }
                            }
                        }
                        else
                        {
                            --i;
                            iSurf += geom->GetNumSymmCopies();
                        }
                    }
                    else
                    {
                        m_geo_fineRat_or_toc.push_back( CalculateFinessRatioAndTOC( iSurf - 1, i) );
                    }
                }
                else
                {
                    m_geo_fineRat_or_toc.push_back( m_geo_fineRat_or_toc[m_geo_fineRat_or_toc.size() - 1] );
                }
            }
            else
            {
                m_geo_fineRat_or_toc.push_back( -1 );
            }
        }
        else
        {
            // Else Push Back Default Val
            m_geo_fineRat_or_toc.push_back( -1 );
        }
    }
}

double ParasiteDragMgrSingleton::CalculateFinessRatioAndTOC( int isurf, int irow )
{
    // Initialize Variables
    vector<double>::const_iterator it;
    double max_xsecarea, dia;
    double finerat = 1.0;

    // Grab Degen Sticks for Appropriate Geom
    vector <DegenStick> degenSticks = m_DegenGeomVec[isurf].getDegenSticks();

    if ( m_DegenGeomVec[isurf].getType() == DegenGeom::SURFACE_TYPE )
    {
        // Wing Type
        it = max_element( degenSticks[0].toc.begin(), degenSticks[0].toc.end() );
        finerat = *it;
    }
    else if ( m_DegenGeomVec[isurf].getType() == DegenGeom::BODY_TYPE )
    {
        it = max_element( degenSticks[0].sectarea.begin(), degenSticks[0].sectarea.end() );
        max_xsecarea = *it;

        // Use Max X-Sectional Area to find "Nominal" Diameter
        dia = 2.0 * sqrt( max_xsecarea / PI );

        finerat = m_geo_lref[irow] / dia;
    }

    return finerat;
}

void ParasiteDragMgrSingleton::Calculate_FF()
{
    int iSurf = 0;

    for ( int i = 0; i < m_RowSize; ++i )
    {
        if ( !m_DegenGeomVec.empty() )
        {
            // If DegenGeom Exists Calculate Form Factor
            if ( m_geo_masterRow[i] )
            {
                if ( m_geo_subsurfID[i].compare( "" ) == 0 )
                {
                    if ( m_DegenGeomVec[iSurf].getType() != DegenGeom::DISK_TYPE )
                    {
                        m_geo_ffOut.push_back( CalculateFormFactor( iSurf, i ) );

                        if ( m_DegenGeomVec[iSurf].getType() == DegenGeom::SURFACE_TYPE )
                        {
                            m_geo_ffName.push_back( AssignFFWingEqnName( m_geo_ffType[i] ) );
                        }
                        else
                        {
                            m_geo_ffName.push_back( AssignFFBodyEqnName( m_geo_ffType[i] ) );
                        }
                    }
                    else
                    {
                        --i;
                    }

                    Geom* geom = VehicleMgr.GetVehicle()->FindGeom( m_geo_geomID[i] );
                    if (geom)
                    {
                        if ( geom->GetType().m_Type != PROP_GEOM_TYPE )
                        {
                            iSurf += geom->GetNumSymmCopies();
                        }
                        else
                        {
                            string numBladesID = geom->FindParm( "NumBlade", "Design" );
                            IntParm* tNumBladeParm = (IntParm*) ParmMgr.FindParm( numBladesID );
                            if ( tNumBladeParm )
                            {
                                iSurf += tNumBladeParm->Get() * geom->GetNumSymmCopies();
                            }
                        }
                    }
                }
                else
                {
                    m_geo_ffOut.push_back( CalculateFormFactor( iSurf - 1, i ) );
                    if ( m_DegenGeomVec[iSurf - 1].getType() == DegenGeom::SURFACE_TYPE )
                    {
                        m_geo_ffName.push_back( AssignFFWingEqnName( m_geo_ffType[i] ) );
                    }
                    else
                    {
                        m_geo_ffName.push_back( AssignFFBodyEqnName( m_geo_ffType[i] ) );
                    }
                }
            }
            else
            {
                m_geo_ffOut.push_back( m_geo_ffOut[m_geo_ffOut.size() - 1] );
                if ( m_geo_ffType[i] == vsp::FF_W_JENKINSON_TAIL )
                {
                    m_geo_Q[i] = 1.2;
                }
                m_geo_ffName.push_back( m_geo_ffName[m_geo_ffName.size() - 1] );
            }
        }
        // Else Push Back Default Val
        else
        {
            m_geo_ffOut.push_back( -1 );
            m_geo_ffName.push_back( "" );
        }
    }
}

double ParasiteDragMgrSingleton::CalculateFormFactor( int isurf, int irow )
{
    // Initialize Variables
    vector<double>::const_iterator it;
    double toc;
    vector <double> hVec, wVec;
    double formfactor = 1.0;

    // Grab Degen Sticks for Appropriate Geom
    vector <DegenStick> degenSticks = m_DegenGeomVec[isurf].getDegenSticks();

    if ( m_DegenGeomVec[isurf].getType() == DegenGeom::SURFACE_TYPE )
    {
        // Wing Type

        toc = m_geo_fineRat_or_toc[irow];

        Calculate_AvgSweep( degenSticks );

        formfactor = CalcFFWing( toc, m_geo_ffType[irow], m_geo_percLam[irow], m_Sweep25, m_Sweep50 );
        if ( m_geo_ffType[irow] == vsp::FF_W_JENKINSON_TAIL )
        {
            m_geo_Q[irow] = 1.2;
        }
    }
    else if ( m_DegenGeomVec[isurf].getType() == DegenGeom::BODY_TYPE )
    {
        formfactor = CalcFFBody( m_geo_fineRat_or_toc[irow], m_geo_ffType[irow] );
    }

    return formfactor;
}

void ParasiteDragMgrSingleton::Calculate_AvgSweep( vector<DegenStick> degenSticks )
{
    // Find Quarter Chord Using Derived Eqn from Geometry
    double width, secSweep25, secSweep50, secArea, weighted25Sum = 0, weighted50Sum = 0, totalArea = 0;
    for ( int j = 0; j < degenSticks[0].areaTop.size(); j++ )
    {
        width = degenSticks[0].areaTop[j] /
                ( ( degenSticks[0].perimTop[j] + degenSticks[0].perimTop[j + 1] ) / 2.0 );

        // Section Quarter Chord Sweep
        secSweep25 = atan( tan( degenSticks[0].sweeple[j] * PI / 180.0 ) +
                           ( 0.25 * ( ( degenSticks[0].chord[j] - degenSticks[0].chord[j + 1] ) / width ) ) ) *
                     180.0 / PI;

        // Section Half Chord Sweep
        secSweep50 = atan( tan( degenSticks[0].sweeple[j] * PI / 180.0 ) +
                           ( 0.50 * ( ( degenSticks[0].chord[j] - degenSticks[0].chord[j + 1] ) / width ) ) ) *
                     180.0 / PI;

        // Section Area
        secArea = degenSticks[0].chord[j] * width;

        // Add Weighted Value to Weighted Sum
        weighted25Sum += secArea * secSweep25;
        weighted50Sum += secArea * secSweep50;

        // Continue to sum up Total Area
        totalArea += secArea;
    }

    // Calculate Sweep @ c/4 & c/2
    m_Sweep25 = weighted25Sum / totalArea * PI / 180.0; // Into Radians
    m_Sweep50 = weighted50Sum / totalArea * PI / 180.0; // Into Radians
}

void ParasiteDragMgrSingleton::Calculate_f()
{
    // Initialize Variables
    double Q, ff;

    for ( int i = 0; i < m_RowSize; ++i )
    {
        // If no value input as Q, use 1
        if ( m_geo_Q[i] != -1 )
        {
            Q = m_geo_Q[i];
        }
        else
        {
            Q = 1;
        }

        // If no value input as FF, use calculated
        if ( m_geo_ffType[i] == vsp::FF_B_MANUAL || m_geo_ffType[i] == vsp::FF_W_MANUAL )
        {
            ff = m_geo_ffIn[i];
        }
        else
        {
            ff = m_geo_ffOut[i];
        }

        if ( IsNotZeroLineItem( i ) )
        {
            if ( !m_DegenGeomVec.empty() )
            {
                // If DegenGeom Exists Calculate f
                m_geo_f.push_back( m_geo_swet[i] * Q * m_geo_cf[i] * ff );
            }
            else
            {
                // Else Push Back Default Val
                m_geo_f.push_back( -1 );
            }
        }
        else
        {
            if ( !m_DegenGeomVec.empty() )
            {
                m_geo_f.push_back( 0.0 );
            }
            else
            {
                m_geo_f.push_back( -1 );
            }
        }
    }
}

void ParasiteDragMgrSingleton::Calculate_CD()
{
    for ( int i = 0; i < m_RowSize; ++i )
    {
        if ( IsNotZeroLineItem( i ) )
        {
            if ( !m_DegenGeomVec.empty() )
            {
                // If DegenGeom Exists Calculate CD
                if ( ! ( m_geo_f[i] != m_geo_f[i] ) )
                {
                    m_geo_CD.push_back( m_geo_f[i] / m_Sref.Get() );
                }
                else
                {
                    m_geo_CD.push_back( 0.0 );
                }
            }
            else
            {
                // Else Push Back Default Val
                m_geo_CD.push_back( -1 );
            }
        }
        else
        {
            if ( !m_DegenGeomVec.empty() )
            {
                m_geo_CD.push_back( 0.0 );
            }
            else
            {
                m_geo_CD.push_back( -1 );
            }
        }
    }
}

void ParasiteDragMgrSingleton::Calculate_ALL()
{
    ClearOutputVectors();
    ClearInputVectors();
    LoadMainTableUserInputs(); // Load User Input Values

    // Calculate All Necessary Values
    Calculate_Swet();
    Calculate_Lref();
    Calculate_Re();
    Calculate_Cf();
    Calculate_fineRat_and_toc();
    Calculate_FF();
    OverwritePropertiesFromAncestorGeom();
    Calculate_f();
    Calculate_CD();

    UpdateExcres();
    UpdatePercentageCD();

    InitTableVec(); // Initialize Map Size

    ParasiteDragTableRow tempStruct = m_DefaultStruct;
    for ( int i = 0; i < m_RowSize; i++ )
    {
        tempStruct.MasterRow = m_geo_masterRow[i];
        tempStruct.GroupedAncestorGen = m_geo_groupedAncestorGen[i];
        tempStruct.GeomID = m_geo_geomID[i];
        tempStruct.SubSurfID = m_geo_subsurfID[i];
        tempStruct.Label = m_geo_label[i];
        tempStruct.Swet = m_geo_swet[i];
        tempStruct.Lref = m_geo_lref[i];
        tempStruct.Re = m_geo_Re[i];
        tempStruct.PercLam = m_geo_percLam[i];
        tempStruct.Cf = m_geo_cf[i];
        tempStruct.fineRatorToC = m_geo_fineRat_or_toc[i];
        tempStruct.FFEqnChoice = m_geo_ffType[i];
        tempStruct.Roughness = m_geo_Roughness[i];
        tempStruct.TeTwRatio = m_geo_TeTwRatio[i];
        tempStruct.TawTwRatio = m_geo_TawTwRatio[i];
        tempStruct.GeomShapeType = m_geo_shapeType[i];
        tempStruct.SurfNum = m_geo_surfNum[i];
        if ( m_geo_ffType[i] == vsp::FF_B_MANUAL || m_geo_ffType[i] == vsp::FF_W_MANUAL )
        {
            tempStruct.FF = m_geo_ffIn[i];
        }
        else
        {
            tempStruct.FF = m_geo_ffOut[i];
        }
        tempStruct.Q = m_geo_Q[i];
        tempStruct.f = m_geo_f[i];
        tempStruct.CD = m_geo_CD[i];
        tempStruct.PercTotalCD = m_geo_percTotalCD[i];
        tempStruct.SurfNum = m_geo_surfNum[i];
        tempStruct.ExpandedList = m_geo_expandedList[i];

        m_TableRowVec[i] = tempStruct;
    }
}

void ParasiteDragMgrSingleton::CorrectTurbEquation()
{
    if ( IsTurbBlacklisted( m_TurbCfEqnType() ) )
    {
        int newcf = FindAlternateTurb( m_TurbCfEqnType() );

        string oname = AssignTurbCfEqnName( m_TurbCfEqnType() );
        string nname = AssignTurbCfEqnName( newcf );

        m_TurbCfEqnType.Set( newcf );


        MessageData errMsgData;
        errMsgData.m_String = "Error";
        errMsgData.m_IntVec.push_back( vsp::VSP_INVALID_CF_EQN );
        string msg = "Error:  Attempt to use turbulent C_f equation " + oname + " marked DO_NOT_USE.  Using " + nname + " instead.";

        errMsgData.m_StringVec.push_back( msg );

        MessageMgr::getInstance().SendAll( errMsgData );
    }
}

string ParasiteDragMgrSingleton::ComputeBuildUp()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        CorrectTurbEquation();

        SetupFullCalculation();
        SetActiveGeomVec();
        CalcRowSize();

        Update();

        Calculate_ALL();

        UpdateExportLabels();

        return ExportToCSV();
    }
    return "";
}

void ParasiteDragMgrSingleton::OverwritePropertiesFromAncestorGeom()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        for ( size_t i = 0; i < m_RowSize; ++i )
        {
            Geom *geom = veh->FindGeom( m_geo_geomID[i] );
            if ( geom )
            {
                if ( m_geo_groupedAncestorGen[i] > 0 )
                {
                    for ( size_t j = 0; j < m_RowSize; ++j )
                    {
                        if ( m_geo_geomID[j].compare( geom->GetAncestorID( m_geo_groupedAncestorGen[i] ) ) == 0 &&
                                m_geo_surfNum[j] == 0 )
                        {
                            m_geo_lref[i] = m_geo_lref[j];
                            m_geo_Re[i] = m_geo_Re[j];
                            m_geo_fineRat_or_toc[i] = m_geo_fineRat_or_toc[j];
                            m_geo_ffOut[i] = m_geo_ffOut[j];
                            m_geo_ffType[i] = m_geo_ffType[j];
                            m_geo_percLam[i] = m_geo_percLam[j];
                            m_geo_Q[i] = m_geo_Q[j];
                            m_geo_cf[i] = m_geo_cf[j];
                        }
                    }
                }
            }
        }
    }
}

// ================================== //
// ====== Iterative Functions ======= //
// ================================== //

struct Schoenherr_functor
{
    double operator()( const double &Cf ) const
    {
        return ( 0.242 / ( sqrt( Cf ) * log10( Re * Cf ) ) ) - 1.0;            // Hoerner 2-5 eq 25
    }
    double Re;
};

struct Schoenherr_p_functor
{
    double operator()( const double &Cf ) const
    {
        return ( ( -0.278613 * log( Cf * Re ) ) - 0.557226 ) /
               ( pow( Cf, 1.5 ) * pow( log( Re * Cf ), 2.0 ) );
    }
    double Re;
};

struct Karman_functor
{
    double operator()( const double &Cf ) const
    {
        return ( ( 4.15 * log10( Re * Cf ) + 1.70 ) * sqrt( Cf ) ) - 1.0;   // Local coeff (wrong) White 7-120, White & Christoph 1971 p. 46 eq 59
    }
    double Re;
};

struct Karman_p_functor
{
    double operator()( const double &Cf ) const
    {
        return ( 0.901161 * log( Re * Cf ) + 2.65232 ) / sqrt( Cf );
    }
    double Re;
};

struct Karman_Schoenherr_functor
{
    double operator()( const double &Cf ) const
    {
        return ( ( 4.13 * log10( Re * Cf ) ) * sqrt( Cf ) ) - 1.0;         // White below 7-123, Schlicting 21-17
    }
    double Re;
};

struct Karman_Schoenherr_p_functor
{
    double operator()( const double &Cf ) const
    {
        return ( 0.896818 * log( Re * Cf ) + 1.79364 ) / sqrt( Cf );
    }
    double Re;
};

// ================================== //
// ================================== //
// ================================== //

double ParasiteDragMgrSingleton::CalcTurbCf( double ReyIn, double ref_leng, int cf_case,
        double roughness_h = 0, double gamma = 1.4, double taw_tw_ratio = 0, double te_tw_ratio = 0 )
{
    double CfOut = 0;
    double CfGuess, f, heightRatio, multiBy = 1.0;
    double r = 0.89; // Recovery Factor
    double n = 0.67; // Viscosity Power-Law Exponent

    if (ReyIn == 0)
    {
        return CfOut;
    }

    eli::mutil::nls::newton_raphson_method < double > nrm;

    if ( m_LengthUnit.Get() == vsp::LEN_FT )
    {
        multiBy = 12.0;
    }
    else if ( m_LengthUnit.Get() == vsp::LEN_M )
    {
        multiBy = 39.3701;
    }

    // Unfortunately, there is substantial opportunity for confusion around these equations.  The primary source of
    // confusion is inconsistent nomenclature in the literature around two related quantities -- the local skin friction
    // coefficient vs. the flat plate average skin coefficient.  Different references use inconsistent nomenclature to
    // differentiate these quantities.  A reader must be pedantic to verify that they understand the notation for any
    // given publication.  Often, the nomenclature is defined far away from where the equations are presented.
    //
    // Without clear definition of nomenclature, one can sometimes fall back to observe which type of Reynolds
    // number is used in the equation -- the local Reynolds number (often Re_x) or the Reynolds number at the end of
    // the flat plate (often Re_l).  The former would be used in a local coefficient and the latter in a plate average
    // coefficient.
    //
    //  Local       Average      Reference
    //    C_f         C_D          White
    //    C_tau       C_f          Hoerner
    //    c_f'        c_f          Schlicting
    //    c_f         C_f          Anderson
    //    C_f         C_D          White & Christoph 1971


    switch ( cf_case )
    {
    case vsp::DO_NOT_USE_CF_TURB_WHITE_CHRISTOPH_COMPRESSIBLE:
        // CfOut = 0.42 / pow( log( 0.056 * ReyIn ), 2.0 );        // Local equation (wrong) White & Christoph 1971, p. 46 eq 59
        // Covert p. 139 -- appears correct, but is in error.
        CfOut = -1.0e-3;
        break;

    case vsp::DO_NOT_USE_CF_TURB_SCHLICHTING_PRANDTL:
        // CfOut = 1 / pow( ( 2 * log10( ReyIn ) - 0.65 ), 2.3 );    // Local equation (wrong) White & Christoph 1971 p. 46 eq 59
        CfOut = -1.0e-3;
        break;

    case vsp::CF_TURB_SCHLICHTING_COMPRESSIBLE:
        CfOut = 0.455 / pow( log10( ReyIn ), 2.58 );       // Hoerner 2-5 eq 24, Schlicting 21.16
        break;

    case vsp::DO_NOT_USE_CF_TURB_SCHLICHTING_INCOMPRESSIBLE:
        //CfOut = 0.472 / pow( log10( ReyIn ), 2.5 );        // Can not verify to base reference.  Appears too high.
        CfOut = -1.0e-3;
        break;

    case vsp::CF_TURB_SCHULTZ_GRUNOW_SCHOENHERR:
        CfOut = 0.427 / pow( ( log10( ReyIn ) - 0.407 ), 2.64 );    // Hoerner 2-5 eq 27, Schlicting 21.19b
        break;

    case vsp::DO_NOT_USE_CF_TURB_SCHULTZ_GRUNOW_HIGH_RE:
        // CfOut = 0.37 / pow( log10( ReyIn ), 2.584 );         // Local equation (wrong) Hoerner.
        // Should be CF_TURB_SCHULTZ_GRUNOW_SCHOENHERR
        CfOut = -1.0e-3;
        break;

    case vsp::CF_TURB_POWER_LAW_BLASIUS:
        // CfOut = 0.0592 / pow( ReyIn, 0.2 );               // Local equation ( wrong ) White & Christoph 1971 p. 46 eq 59
        // 0.0592 integrates up to 0.074 equation below.
        // CfOut = 0.0576 / pow( ReyIn, 0.2 );               // Local equation ( wrong ) White & Christoph 1971 p. 46 eq 59
        // 0.0576 integrates up to 0.072
        CfOut = 0.072 / pow( ReyIn, 0.2 );               // Schlicting above 21.11, immediate below equation better.
        // Not recommended
        break;

    case vsp::CF_TURB_POWER_LAW_PRANDTL_LOW_RE:
        CfOut = 0.074 / pow( ReyIn, 0.2 );                // Hoerner 2-5 eq 23, Schlicting 21.11
        // Not recommended
        break;

    case vsp::CF_TURB_POWER_LAW_PRANDTL_MEDIUM_RE:
        // CfOut = 0.027 / pow( ReyIn, 1.0 / 7.0 );          // Local equation (wrong) White 6-70
        CfOut = 0.0315 / pow( ReyIn, 1.0 / 7.0 );          // White 6-80
        break;

    case vsp::CF_TURB_POWER_LAW_PRANDTL_HIGH_RE:
        // CfOut = 0.058 / pow( ReyIn, 0.2 );                // Local equation (wrong) White 6-72
        CfOut = 0.0725 / pow( ReyIn, 0.2 );               // Symbolic computer integral in wxMaxima 'integrate( 0.058 / (x^0.2),x,0,L)/L;'
        // Not recommended
        break;

    case vsp::CF_TURB_EXPLICIT_FIT_SPALDING:
        // CfOut = 0.455 / pow( log( 0.06 * ReyIn ), 2.0 );  // Local equation (wrong) White 6-78
        CfOut = 0.523 / pow( log( 0.06 * ReyIn ), 2.0 );  // White 6-81
        break;

    case vsp::CF_TURB_EXPLICIT_FIT_SPALDING_CHI:
        // CfOut = 0.225 / pow( log10( ReyIn ), 2.32 );         // Local equation ( wrong) White & Christoph 1971, p. 48, eq 62
        CfOut = 0.430 / pow( log10( ReyIn ), 2.56 );         // White & Christoph 1971, p. 48, eq 62
        break;

    case vsp::CF_TURB_EXPLICIT_FIT_SCHOENHERR:
        CfOut = pow( ( 1.0 / ( ( 3.46 * log10( ReyIn ) ) - 5.6 ) ), 2.0 );        // Hoerner
        break;

    case vsp::CF_TURB_IMPLICIT_SCHOENHERR:
        Schoenherr_functor sfun;
        sfun.Re = ReyIn;
        Schoenherr_p_functor sfunprm;
        sfunprm.Re = ReyIn;

        CfGuess = pow( ( 1.0 / ( ( 3.46 * log10( ReyIn ) ) - 5.6 ) ), 2.0 );

        nrm.set_initial_guess( CfGuess );
        nrm.find_root( CfOut, sfun, sfunprm, 0.0 );
        break;

    case vsp::DO_NOT_USE_CF_TURB_IMPLICIT_KARMAN:
        // Karman_functor kfun;
        // kfun.Re = ReyIn;
        // Karman_p_functor kfunprm;
        // kfunprm.Re = ReyIn;
        //
        // CfGuess = 0.455 / pow( log10( ReyIn ), 2.58 );
        //
        // nrm.set_initial_guess( CfGuess );
        // nrm.find_root( CfOut, kfun, kfunprm, 0.0 );
        CfOut = -1.0e-3;
        break;

    case vsp::CF_TURB_IMPLICIT_KARMAN_SCHOENHERR:
        Karman_Schoenherr_functor ksfun;
        ksfun.Re = ReyIn;
        Karman_Schoenherr_p_functor ksfunprm;
        ksfunprm.Re = ReyIn;

        CfGuess = pow( ( 1.0 / ( ( 3.46 * log10( ReyIn ) ) - 5.6 ) ), 2.0 );

        nrm.set_initial_guess( CfGuess );
        nrm.find_root( CfOut, ksfun, ksfunprm, 0.0 );
        break;

    case vsp::DO_NOT_USE_CF_TURB_ROUGHNESS_WHITE:
        heightRatio = ref_leng / roughness_h;
        // CfOut = pow( ( 2.87 + ( 1.58 * log10( heightRatio ) ) ), -2.5 );    // Local equation (wrong) White 6-84a, Schlicting 21.37
        // Should be CF_TURB_ROUGHNESS_SCHLICHTING_AVG.
        CfOut = -1.0e-3;
        break;

    case vsp::DO_NOT_USE_CF_TURB_ROUGHNESS_SCHLICHTING_LOCAL:
        heightRatio = ref_leng / roughness_h;
        // CfOut = pow( ( 1.4 + ( 3.7 * log10( heightRatio ) ) ), -2.0 );    // Local equation (wrong) White 6-83
        CfOut = -1.0e-3;
        break;

    case vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG:
        heightRatio = ref_leng / ( roughness_h * multiBy );
        CfOut = pow( ( 1.89 + ( 1.62 * log10( heightRatio ) ) ), -2.5 );    // White 6-84b, Schlicting 21.38
        break;

    case vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION:
        heightRatio = ref_leng / ( roughness_h * multiBy );
        CfOut = pow( ( 1.89 + ( 1.62 * log10( heightRatio ) ) ), -2.5 ) /
                ( pow( ( 1.0 + ( ( gamma - 1.0 ) / 2.0 ) * m_Mach() ), 0.467 ) );
        break;

    case vsp::CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH:
        f = ( 1 + ( 0.22 * r * ( ( roughness_h  - 1.0 ) / 2.0 ) *
                    m_Mach() * m_Mach() * te_tw_ratio ) ) /
            ( 1 + ( 0.3 * ( taw_tw_ratio - 1.0 ) ) );

        CfOut = ( 0.451 * f * f * te_tw_ratio ) /
                ( log( 0.056 * f * pow( te_tw_ratio, 1.0 + n ) * ReyIn ) );

        break;

    default:
        CfOut = 0;
        break;
    }

    if ( CfOut < 0 ) // Should be impossible.
    {
        MessageData errMsgData;
        errMsgData.m_String = "Error";
        errMsgData.m_IntVec.push_back( vsp::VSP_INVALID_CF_EQN );
        string msg = "Error:  C_f equation resulted in negative C_f.";

        errMsgData.m_StringVec.push_back( msg );

        MessageMgr::getInstance().SendAll( errMsgData );
    }

    return CfOut;
}

double ParasiteDragMgrSingleton::CalcLamCf( double ReyIn, int cf_case )
{
    double CfOut = 0;

    if (ReyIn != 0)
    {
        switch (cf_case)
        {
        case vsp::CF_LAM_BLASIUS:
            CfOut = 1.32824 / sqrt( ReyIn );             // Verified RM 6/30/19 (White)

            //CfOut = 1.32824 * (Tw/Te)^(-1/3) / sqrt( ReyIn );             // White 7-39 (times 2)
            break;

        case vsp::CF_LAM_BLASIUS_W_HEAT:
            CfOut = 0;
            break;
        }
    }

    return CfOut;
}

string ParasiteDragMgrSingleton::AssignTurbCfEqnName( int cf_case )
{
    string eqn_name;
    switch ( cf_case )
    {
    case vsp::DO_NOT_USE_CF_TURB_WHITE_CHRISTOPH_COMPRESSIBLE:
        eqn_name = "(Do Not Use) Compressible White-Christoph";
        break;

    case vsp::DO_NOT_USE_CF_TURB_SCHLICHTING_PRANDTL:
        eqn_name = "(Do Not Use) Schlichting-Prandtl";
        break;

    case vsp::CF_TURB_SCHLICHTING_COMPRESSIBLE:
        eqn_name = "Compressible Schlichting";
        break;

    case vsp::DO_NOT_USE_CF_TURB_SCHLICHTING_INCOMPRESSIBLE:
        eqn_name = "(Do Not Use) Incompressible Schlichting";
        break;

    case vsp::CF_TURB_SCHULTZ_GRUNOW_SCHOENHERR:
        eqn_name = "Schultz-Grunow Schoenherr";
        break;

    case vsp::DO_NOT_USE_CF_TURB_SCHULTZ_GRUNOW_HIGH_RE:
        eqn_name = "(Do Not Use) High Reynolds Number Schultz-Grunow";
        break;

    case vsp::CF_TURB_POWER_LAW_BLASIUS:
        eqn_name = "Blasius Power Law";
        break;

    case vsp::CF_TURB_POWER_LAW_PRANDTL_LOW_RE:
        eqn_name = "Low Reynolds Number Prandtl Power Law";
        break;

    case vsp::CF_TURB_POWER_LAW_PRANDTL_MEDIUM_RE:
        eqn_name = "Medium Reynolds Number Prandtl Power Law";
        break;

    case vsp::CF_TURB_POWER_LAW_PRANDTL_HIGH_RE:
        eqn_name = "High Reynolds Number Prandtl Power Law";
        break;

    case vsp::CF_TURB_EXPLICIT_FIT_SPALDING:
        eqn_name = "Spalding Explicit Empirical Fit";
        break;

    case vsp::CF_TURB_EXPLICIT_FIT_SPALDING_CHI:
        eqn_name = "Spalding-Chi Explicit Empirical Fit";
        break;

    case vsp::CF_TURB_EXPLICIT_FIT_SCHOENHERR:
        eqn_name = "Schoenherr Explicit Empirical Fit";
        break;

    case vsp::CF_TURB_IMPLICIT_SCHOENHERR:
        eqn_name = "Schoenherr Implicit";
        break;

    case vsp::DO_NOT_USE_CF_TURB_IMPLICIT_KARMAN:
        eqn_name = "(Do Not Use) Von Karman Implicit";
        break;

    case vsp::CF_TURB_IMPLICIT_KARMAN_SCHOENHERR:
        eqn_name = "Karman-Schoenherr Implicit";
        break;

    case vsp::DO_NOT_USE_CF_TURB_ROUGHNESS_WHITE:
        eqn_name = "(Do Not Use) White Roughness";
        break;

    case vsp::DO_NOT_USE_CF_TURB_ROUGHNESS_SCHLICHTING_LOCAL:
        eqn_name = "(Do Not Use) Schlichting Local Roughness";
        break;

    case vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG:
        eqn_name = "Schlichting Avg Roughness";
        break;

    case vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION:
        m_TurbCfEqnName = "Schlichting Avg Roughness w Flow Correctioin";
        break;

    case vsp::CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH:
        eqn_name = "White-Christoph w Heat Transfer";
        break;

    default:
        eqn_name = "ERROR";
        break;
    }
    return eqn_name;
}

string ParasiteDragMgrSingleton::AssignLamCfEqnName( int cf_case )
{
    string eqn_name;
    switch ( cf_case )
    {
    case vsp::CF_LAM_BLASIUS:
        eqn_name = "Laminar Blasius";
        break;

    case vsp::CF_LAM_BLASIUS_W_HEAT:
        eqn_name = "Laminar Blasius w Heat Transfer";
        break;

    default:
        eqn_name = "ERROR";
    }
    return eqn_name;
}

bool ParasiteDragMgrSingleton::IsTurbBlacklisted( int cf_case )
{
    return vector_contains_val( m_TurbBlackList, cf_case );
}

int ParasiteDragMgrSingleton::FindAlternateTurb( int cf_case )
{
    int indx = vector_find_val( m_TurbBlackList, cf_case );
    if( indx != -1 )
    {
        return m_TurbAlternateList[ indx ];
    }
    return cf_case;
}

double ParasiteDragMgrSingleton::CalcFFWing( double toc, int ff_case,
        double perc_lam = 0, double sweep25 = 0, double sweep50 = 0 )
{
    // Values recreated using Plot Digitizer and fitted to a 3rd power polynomial
    double Interval[] = { 0.25, 0.6, 0.8, 0.9 };
    double ff;
    double Rls = 1.0;

    double mach = m_Atmos.GetMach();

    switch ( ff_case )
    {
    case vsp::FF_W_MANUAL:
        ff = 1;
        break;

    case vsp::FF_W_EDET_CONV:
        ff = 1.0 + toc * ( 2.94206 + toc * ( 7.16974 + toc * ( 48.8876 +
                                             toc * ( -1403.02 + toc * ( 8598.76 + toc * ( -15834.3 ) ) ) ) ) );
        break;

    case vsp::FF_W_EDET_ADV:
        ff = 1.0 + 4.275 * toc;
        break;

    case vsp::FF_W_HOERNER:
        ff = 1.0 + 2.0 * toc + 60.0 * pow( toc, 4.0 );
        break;

    case vsp::FF_W_COVERT:
        ff = 1.0 + 1.8 * toc + ( 50.0 * pow( toc, 4.0 ) );
        break;

    case vsp::FF_W_SHEVELL:
        double Z;
        Z = ( ( 2.0 - pow( mach, 2.0 ) ) * cos( sweep25 ) ) / ( sqrt( 1.0 -
                ( pow( mach, 2.0 ) * pow( cos( sweep25 ), 2 ) ) ) );
        ff = 1.0 + ( Z * toc ) + ( 100.0 * pow( toc, 4.0 ) );
        break;

    case vsp::FF_W_KROO:
        double part1A, part1B, part2A, part2B;
        part1A = ( 2.2 * pow( cos( sweep25 ), 2.0 ) * toc );
        part1B = ( sqrt( 1.0 - ( pow( mach, 2.0 ) * pow( cos( sweep25 ), 2.0 ) ) ) );
        part2A = ( 4.84 * pow( cos( sweep25 ), 2.0 ) * ( 1.0 + 5.0 * pow( cos( sweep25 ), 2.0 ) ) * pow( toc, 2.0 ) );
        part2B = ( 2.0 * ( 1.0 - ( pow( mach, 2.0 ) * pow( cos( sweep25 ), 2.0 ) ) ) );
        ff = 1.0 + ( part1A / part1B ) + ( part2A / part2B );
        break;

    case vsp::FF_W_TORENBEEK:
        ff = 1.0 + 2.7 * toc + 100.0 * pow( toc, 4.0 );
        break;

    case vsp::FF_W_DATCOM:
        double L, x, RLS_Low, RLS_High;

        // L value Decided based on xtrans/c
        if ( perc_lam <= 30.0 )
        { L = 2.0; }
        else
        { L = 1.2; }

        if ( mach < Interval[0] )
        {
            Rls = -2.0292 * pow( cos( sweep25 ), 3.0 ) + 3.6345 * pow( cos( sweep25 ), 2.0 ) - 1.391 * cos( sweep25 ) + 0.8521;
        }
        else if ( mach == Interval[0] )
        {
            Rls = -2.0292 * pow( cos( sweep25 ), 3.0 ) + 3.6345 * pow( cos( sweep25 ), 2.0 ) - 1.391 * cos( sweep25 ) + 0.8521;
        }
        else if ( mach > Interval[0] && mach < Interval[1] )
        {
            x = ( mach - Interval[0] ) / ( Interval[1] - Interval[0] );
            RLS_Low = -2.0292 * pow( cos( sweep25 ), 3.0 ) + 3.6345 * pow( cos( sweep25 ), 2.0 ) - 1.391 * cos( sweep25 ) + 0.8521;
            RLS_High = -1.9735 * pow( cos( sweep25 ), 3.0 ) + 3.4504 * pow( cos( sweep25 ), 2.0 ) - 1.186 * cos( sweep25 ) + 0.858;
            Rls = x * ( RLS_High - RLS_Low ) + RLS_Low;
        }
        else if ( mach == Interval[1] )
        {
            Rls = -1.9735 * pow( cos( sweep25 ), 3.0 ) + 3.4504 * pow( cos( sweep25 ), 2.0 ) - 1.186 * cos( sweep25 ) + 0.858;
        }
        else if ( mach > Interval[1] && mach < Interval[2] )
        {
            x = ( mach - Interval[1] ) / ( Interval[2] - Interval[1] );
            RLS_Low = -1.9735 * pow( cos( sweep25 ), 3.0 ) + 3.4504 * pow( cos( sweep25 ), 2.0 ) - 1.186 * cos( sweep25 ) + 0.858;
            RLS_High = -1.6538 * pow( cos( sweep25 ), 3.0 ) + 2.865 * pow( cos( sweep25 ), 2.0 ) - 0.886 * cos( sweep25 ) + 0.934;
            Rls = x * ( RLS_High - RLS_Low ) + RLS_Low;
        }
        else if ( mach == Interval[2] )
        {
            Rls = -1.6538 * pow( cos( sweep25 ), 3.0 ) + 2.865 * pow( cos( sweep25 ), 2.0 ) - 0.886 * cos( sweep25 ) + 0.934;
        }
        else if ( mach > Interval[2] && mach < Interval[3] )
        {
            x = ( mach - Interval[2] ) / ( Interval[3] - Interval[2] );
            RLS_Low = -1.6538 * pow( cos( sweep25 ), 3.0 ) + 2.865 * pow( cos( sweep25 ), 2.0 ) - 0.886 * cos( sweep25 ) + 0.934;
            RLS_High = -1.8316 * pow( cos( sweep25 ), 3.0 ) + 3.3944 * pow( cos( sweep25 ), 2.0 ) - 1.3596 * cos( sweep25 ) + 1.1567;
            Rls = x * ( RLS_High - RLS_Low ) + RLS_Low;
        }
        else if ( mach >= Interval[3] )
        {
            Rls = -1.8316 * pow( cos( sweep25 ), 3.0 ) + 3.3944 * pow( cos( sweep25 ), 2.0 ) - 1.3596 * cos( sweep25 ) + 1.1567;
        }

        ff = ( 1.0 + ( L * toc ) + 100.0 * pow( toc, 4.0 ) ) * Rls;
        break;

    case vsp::FF_W_SCHEMENSKY_6_SERIES_AF:
        ff = 1.0 + ( 1.44 * toc ) + ( 2.0 * pow( toc, 2.0 ) );
        break;

    case vsp::FF_W_SCHEMENSKY_4_SERIES_AF:
        ff = 1.0 + ( 1.68 * toc ) + ( 3.0 * pow( toc, 2.0 ) );
        break;

    //case vsp::FF_W_SCHEMENSKY_SUPERCRITICAL_AF :
    //    m_geo_ffName.push_back("Schemensky Supercritical AF");
    //    // Need Design Camber and Critical Mach #
    //    double K1, deltaM;

    //    deltaM = m_Mach() - Mcr;

    //    if ( deltaM <= -0.2 )
    //        K1 = 0.3;
    //    else if ( deltaM > 0.2 && deltaM < 0.0 )
    //        K1 = ( 6.7964 * pow( deltaM, 2 ) ) + ( 2.3605 * deltaM ) + 0.5059;
    //    else if ( deltaM >= 0.0 )
    //        K1 = 0.5;

    //    ff =  1 + ( K1* C1d ) + ( 1.44 * toc ) + ( 2 * pow( toc, 2 ) ) );
    //    break;

    case vsp::FF_W_JENKINSON_WING:
        double Fstar;

        Fstar = 1.0 + ( 3.3 * toc ) - ( 0.008 * pow( toc, 2.0 ) ) + ( 27.0 * pow( toc, 3.0 ) );

        ff = ( ( Fstar - 1.0 ) * pow( cos( sweep50 ), 2.0 ) ) + 1.0;
        break;

    case vsp::FF_W_JENKINSON_TAIL:

        Fstar = 1.0 + 3.52 * toc;

        ff = ( ( Fstar - 1.0 ) * pow( cos( sweep50 ), 2.0 ) ) + 1.0;
        break;

    default:
        ff = 0;
    }
    return ff;
}

double ParasiteDragMgrSingleton::CalcFFBody(double FR, int ff_case)
{

    double ff;
    double mach = m_Atmos.GetMach();
    switch ( ff_case )
    {
    case vsp::FF_B_MANUAL:
        ff = 1.0;
        break;

    case vsp::FF_B_SCHEMENSKY_FUSE:
        ff = 1.0 + ( 60.0 / pow( FR, 3.0 ) ) + ( 0.0025 * FR );
        break;

    case vsp::FF_B_SCHEMENSKY_NACELLE:
        ff = 1.0 + 0.35 / FR;
        break;

    case vsp::FF_B_HOERNER_STREAMBODY:
        ff = 1.0 + ( 1.5 / pow(FR, 1.5 ) ) +
             ( 7.0 / pow(FR, 3.0 ) );
        break;

    case vsp::FF_B_TORENBEEK:
        ff = 1.0 + ( 2.2 / pow(FR, 1.5 ) ) +
             ( 3.8 / pow(FR, 3.0 ) );
        break;

    case vsp::FF_B_SHEVELL:
        ff = 1.0 + ( 2.8 / pow(FR, 1.5 ) ) +
             ( 3.8 / pow(FR, 3.0 ) );
        break;

    case vsp::FF_B_COVERT:
        ff = 1.02 * (1.0 + ( 1.5 / ( pow(FR, 1.5 ) ) ) + (7.0 / (pow(FR, 3.0 ) * pow((1.0 - pow(mach, 3 ) ), 0.6 ) ) ) );
        break;

    case vsp::FF_B_JENKINSON_FUSE:
        ff = 1.0 + ( 2.2 / pow(FR, 1.5 ) ) -
             ( 0.9 / pow(FR, 3.0 ) );
        break;

    case vsp::FF_B_JENKINSON_WING_NACELLE:
        ff = 1.25;
        break;

    case vsp::FF_B_JENKINSON_AFT_FUSE_NACELLE:
        ff = 1.5;
        break;

    default:
        ff = 0.0;
        break;
    }
    return ff;
}

string ParasiteDragMgrSingleton::AssignFFWingEqnName( int ff_case )
{
    string ff_name;
    switch ( ff_case )
    {
    case vsp::FF_W_MANUAL:
        ff_name = "Manual";
        break;

    case vsp::FF_W_EDET_CONV:
        ff_name = "EDET Conventional";
        break;

    case vsp::FF_W_EDET_ADV:
        ff_name = "EDET Advanced";
        break;

    case vsp::FF_W_HOERNER:
        ff_name = "Hoerner";
        break;

    case vsp::FF_W_COVERT:
        ff_name = "Covert";
        break;

    case vsp::FF_W_SHEVELL:
        ff_name = "Shevell";
        break;

    case vsp::FF_W_KROO:
        ff_name = "Kroo";
        break;

    case vsp::FF_W_TORENBEEK:
        ff_name = "Torenbeek";
        break;

    case vsp::FF_W_DATCOM:
        ff_name = "DATCOM";
        break;

    case vsp::FF_W_SCHEMENSKY_6_SERIES_AF:
        ff_name = "Schemensky 6 Series AF";
        break;

    case vsp::FF_W_SCHEMENSKY_4_SERIES_AF:
        ff_name = "Schemensky 4 Series AF";
        break;

    //case vsp::FF_W_SCHEMENSKY_SUPERCRITICAL_AF :
    //    ff_name = "Schemensky Supercritical AF");
    //    break;

    case vsp::FF_W_JENKINSON_WING:
        ff_name = "Jenkinson Wing";
        break;

    case vsp::FF_W_JENKINSON_TAIL:
        ff_name = "Jenkinson Tail";
        break;

    default:
        ff_name = "ERROR";
    }
    return ff_name;
}

string ParasiteDragMgrSingleton::AssignFFBodyEqnName( int ff_case )
{
    string ff_name;
    switch ( ff_case )
    {
    case vsp::FF_B_MANUAL:
        ff_name = "Manual";
        break;

    case vsp::FF_B_SCHEMENSKY_FUSE:
        ff_name = "Schemensky/DATCOM/RAND Fuselage";
        break;

    case vsp::FF_B_SCHEMENSKY_NACELLE:
        ff_name = "Schemensky/DATCOM/RAND Nacelle";
        break;

    case vsp::FF_B_HOERNER_STREAMBODY:
        ff_name = "Hoerner Streamlined Body";
        break;

    case vsp::FF_B_TORENBEEK:
        ff_name = "Torenbeek";
        break;

    case vsp::FF_B_SHEVELL:
        ff_name = "Shevell";
        break;

    case vsp::FF_B_COVERT:
        ff_name = "Covert";
        break;

    case vsp::FF_B_JENKINSON_FUSE:
        ff_name = "Jenkinson Fuselage";
        break;

    case vsp::FF_B_JENKINSON_WING_NACELLE:
        ff_name = "Jenkinson Wing Nacelle";
        break;

    case vsp::FF_B_JENKINSON_AFT_FUSE_NACELLE:
        ff_name = "Jenkinson Aft Fuse Nacelle";
        break;

    default:
        ff_name = "ERROR";
        break;
    }
    return ff_name;
}

void ParasiteDragMgrSingleton::SetActiveGeomVec()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        vector <string> geomVec = veh->GetGeomSet( m_SetChoice() );

        m_PDGeomIDVec.clear();
        for ( int i = 0; i < geomVec.size(); i++ )
        {
            bool geom_skipped = true;
            Geom* geom = veh->FindGeom( geomVec[i] );
            if ( geom )
            {
                if ( geom->GetType().m_Type != HINGE_GEOM_TYPE &&
                     geom->GetType().m_Type != BLANK_GEOM_TYPE &&
                     geom->GetType().m_Type != MESH_GEOM_TYPE &&
                     geom->GetType().m_Type != HUMAN_GEOM_TYPE )          // NOT SURE
                {
                    if ( geom->GetSurfType(0) != vsp::DISK_SURF )
                    {
                        m_PDGeomIDVec.push_back(geomVec[i]);
                        geom_skipped = false;
                    }
                }
            }

            if ( geom_skipped )
            {
                string message = "Warning: Geom ID " + geomVec[i] + "not included in Parasite Drag calculation\n";
                printf( "%s", message.c_str() );
            }
        }
    }
}

void ParasiteDragMgrSingleton::SetFreestreamParms()
{
    m_Hinf.Set( m_Atmos.GetAlt() );
    m_Temp.Set( m_Atmos.GetTemp() );
    m_DeltaT.Set( m_Atmos.GetDeltaT() );
    m_Pres.Set( m_Atmos.GetPres() );
    m_Rho.Set( m_Atmos.GetDensity() );
    m_DynaVisc.Set( m_Atmos.GetDynaVisc() );
    m_Mach.Set( m_Atmos.GetMach() );
}

void ParasiteDragMgrSingleton::SetExcresLabel( const string & newLabel )
{
    if ( m_CurrentExcresIndex != -1 )
    {
        m_ExcresRowVec[m_CurrentExcresIndex].Label = newLabel;
    }
}

int ParasiteDragMgrSingleton::GetLrefSigFig()
{
    double lrefmag;
    if ( !m_geo_lref.empty() )
    {
        vector<double>::const_iterator it = max_element( m_geo_lref.begin(), m_geo_lref.end() );
        lrefmag = mag( *it );
    }
    else
    {
        lrefmag = 1;
    }

    if ( lrefmag > 1 )
    {
        return 1;
    }
    else if ( lrefmag == 1 )
    {
        return 2;
    }
    else
    {
        return 3;
    }
}

double ParasiteDragMgrSingleton::GetGeometryCD()
{
    double sum = 0;
    for ( int i = 0; i < m_geo_CD.size(); i++ )
    {
        if ( m_geo_masterRow[i] )
        {
            if ( m_geo_CD[i] > 0.0 )
            {
                sum += m_geo_CD[i];
            }
        }
    }
    return sum;
}

double ParasiteDragMgrSingleton::GetSubTotalCD()
{
    return GetGeometryCD() + GetSubTotalExcresCD();
}

double ParasiteDragMgrSingleton::GetTotalCD()
{
    for ( size_t i = 0; i < m_ExcresRowVec.size(); ++i )
    {
        if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_MARGIN )
        {
            return GetGeometryCD() + GetTotalExcresCD();
        }
    }
    return GetSubTotalCD();
}

string ParasiteDragMgrSingleton::GetCurrentExcresLabel()
{
    if ( m_CurrentExcresIndex != -1 )
    {
        return m_ExcresRowVec[m_CurrentExcresIndex].Label;
    }
    return string();
}

int ParasiteDragMgrSingleton::GetCurrentExcresType()
{
    if ( m_CurrentExcresIndex != -1 )
    {
        return m_ExcresRowVec[m_CurrentExcresIndex].Type;
    }
    return 0;
}

void ParasiteDragMgrSingleton::AddExcrescence()
{
    ExcrescenceTableRow tempStruct;
    ostringstream strs;
    char str[256];

    if ( m_ExcresRowVec.size() > 0 )
    {
        for ( size_t i = 0; i < m_ExcresRowVec.size(); ++i )
        {
            if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_MARGIN && m_ExcresType() == vsp::EXCRESCENCE_MARGIN )
            {
                return;
            }
        }
    }

    if ( m_ExcresName.empty() )
    {
        sprintf( str, "EXCRES_%zu", m_ExcresRowVec.size() );
        tempStruct.Label = string( str );
    }
    else
    {
        tempStruct.Label = m_ExcresName;
    }

    m_ExcresName.clear();

    tempStruct.Input = m_ExcresValue.Get();

    if ( m_ExcresType() == vsp::EXCRESCENCE_COUNT )
    {
        tempStruct.Amount = m_ExcresValue() / 10000.0;
        tempStruct.TypeString = "Count (10000*CD)";
    }
    else if ( m_ExcresType() == vsp::EXCRESCENCE_CD )
    {
        tempStruct.Amount = m_ExcresValue();
        tempStruct.TypeString = "CD";
    }
    else if ( m_ExcresType() == vsp::EXCRESCENCE_PERCENT_GEOM )
    {
        tempStruct.Amount = 0.0; // Calculated in UpdateExcres()
        tempStruct.TypeString = "% of CD_Geom";
    }
    else if ( m_ExcresType() == vsp::EXCRESCENCE_MARGIN )
    {
        tempStruct.Amount = 0.0; // Calculated in UpdateExcres()
        tempStruct.TypeString = "Margin";
    }
    else if ( m_ExcresType() == vsp::EXCRESCENCE_DRAGAREA )
    {
        tempStruct.Amount = 0.0; // Calculated in UpdateExcres()
        tempStruct.TypeString = "Drag Area (D/q)";
    }

    tempStruct.Type = m_ExcresType();

    tempStruct.f = tempStruct.Amount * m_Sref();

    tempStruct.PercTotalCD = 0.0; // Initializing this to 0.0

    m_ExcresRowVec.push_back( tempStruct );

    m_CurrentExcresIndex = m_ExcresRowVec.size() - 1;
}

void ParasiteDragMgrSingleton::AddExcrescence( const std::string & excresName, int excresType, double excresVal )
{
    m_ExcresValue.Set( excresVal );
    m_ExcresType.Set( excresType );
    m_ExcresName = excresName;

    AddExcrescence();
}

void ParasiteDragMgrSingleton::DeleteExcrescence()
{
    if ( m_CurrentExcresIndex != -1 )
    {
        m_ExcresRowVec.erase( m_ExcresRowVec.begin() + m_CurrentExcresIndex );
    }

    if ( m_ExcresRowVec.size() > 0 )
    {
        m_CurrentExcresIndex = 0;
        UpdateCurrentExcresVal();
    }
    else
    {
        m_CurrentExcresIndex = -1;
    }
}

void ParasiteDragMgrSingleton::DeleteExcrescence( int index )
{
    m_CurrentExcresIndex = index;
    DeleteExcrescence();
}

double ParasiteDragMgrSingleton::CalcPercentageGeomCD( double val )
{
    if ( !m_DegenGeomVec.empty() )
    {
        if ( GetGeometryCD() > 0.0 )
        {
            return val / 100.0 * GetGeometryCD();
        }
        else
        {
            return 0;
        }
    }

    return 0;
}

double ParasiteDragMgrSingleton::CalcPercentageTotalCD( double val )
{
    if ( !m_DegenGeomVec.empty() )
    {
        if ( GetSubTotalCD() > 0.0 )
        {
            return GetSubTotalCD() / ( ( 100.0 - val ) / 100.0 ) - GetSubTotalCD();
        }
        else
        {
            return 0;
        }
    }

    return 0;
}

double ParasiteDragMgrSingleton::CalcDragAreaCD( double val )
{
    return val / m_Sref.Get();
}

double ParasiteDragMgrSingleton::GetSubTotalExcresCD()
{
    double sum = 0;

    for ( size_t i = 0; i < m_ExcresRowVec.size(); ++i )
    {
        if ( m_ExcresRowVec[i].Type != vsp::EXCRESCENCE_MARGIN )
        {
            sum += m_ExcresRowVec[i].Amount;
        }
    }

    return sum;
}

double ParasiteDragMgrSingleton::GetTotalExcresCD()
{
    double sum = 0;

    for ( size_t i = 0; i < m_ExcresRowVec.size(); ++i )
    {
        sum += m_ExcresRowVec[i].Amount;
    }

    return sum;
}

void ParasiteDragMgrSingleton::ConsolidateExcres()
{
    m_excres_Label.clear();
    m_excres_Type.clear();
    m_excres_Input.clear();
    m_excres_f.clear();
    m_excres_Amount.clear();
    m_excres_PercTotalCD.clear();
    ExcrescenceTableRow excresRowStruct;
    for ( size_t i = 0; i < m_ExcresRowVec.size(); ++i )
    {
        m_excres_Label.push_back( m_ExcresRowVec[i].Label.c_str() );
        m_excres_Type.push_back( m_ExcresRowVec[i].TypeString.c_str() );
        m_excres_Input.push_back( m_ExcresRowVec[i].Input );
        m_excres_f.push_back( m_ExcresRowVec[i].f );
        m_excres_Amount.push_back( m_ExcresRowVec[i].Amount );
        m_excres_PercTotalCD.push_back( m_ExcresRowVec[i].PercTotalCD );
    }
}

void ParasiteDragMgrSingleton::Update()
{
    // Update Filename
    ParasiteDragMgr.m_FileName = VehicleMgr.GetVehicle()->getExportFileName( vsp::DRAG_BUILD_CSV_TYPE );

    UpdateRefWing();

    UpdateTempLimits();
    UpdateAtmos();

    UpdateParmActivity();

    UpdateExcres();
}

void ParasiteDragMgrSingleton::UpdateWettedAreaTotals()
{
    if ( !m_DegenGeomVec.empty() )
    {
        // Subsurfaces Addition First
        for ( size_t i = 0; i < m_RowSize; ++i )
        {
            for ( size_t j = 0; j < m_RowSize; ++j )
            {
                if ( i != j ) // If not the same geom
                {
                    if ( m_geo_masterRow[i] && m_geo_subsurfID[j].compare( "" ) != 0 )
                    {
                        if ( ShouldAddSubSurfToMasterGeom( i, j ) )
                        {
                            m_geo_swet[i] += m_geo_swet[j];
                        }
                    }
                }
            }
        }

        // Add Geom Surf Wetted Areas
        for ( size_t i = 0; i < m_RowSize; ++i )
        {
            for ( size_t j = 0; j < m_RowSize; ++j )
            {
                if ( i != j ) // If not the same geom
                {
                    if ( m_geo_masterRow[i] && m_geo_subsurfID[j].compare( "" ) == 0 && m_geo_subsurfID[i].compare( "" ) == 0 )
                    {
                        if ( ShouldAddGeomToMasterGeom( i, j ) )
                        {
                            m_geo_swet[i] += m_geo_swet[j];
                        }
                    }
                }
            }
        }
    }
}

bool ParasiteDragMgrSingleton::ShouldAddSubSurfToMasterGeom( const size_t &i, const size_t &j )
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        if ( i != j ) // If not the same geom
        {
            if ( m_geo_masterRow[i] && m_geo_subsurfID[j].compare( "" ) != 0 )
            {
                Geom *geom = veh->FindGeom(m_geo_geomID[j]);
                if (geom)
                {
                    return ((m_geo_geomID[i].compare(m_geo_geomID[j]) == 0 &&
                             geom->GetSubSurf(m_geo_subsurfID[j])->m_IncludeType() == vsp::SS_INC_TREAT_AS_PARENT) ||
                            (m_geo_geomID[i].compare(geom->GetAncestorID(m_geo_groupedAncestorGen[j])) == 0 &&
                             geom->GetSubSurf(m_geo_subsurfID[j])->m_IncludeType() == vsp::SS_INC_TREAT_AS_PARENT) ||
                            (geom->GetSubSurf(m_geo_subsurfID[j])->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT &&
                             m_geo_subsurfID[i].compare(m_geo_subsurfID[j]) == 0));
                }
                else
                {
                    return false;
                }
            }
        }
    }
    return false;
}

bool ParasiteDragMgrSingleton::ShouldAddGeomToMasterGeom( const size_t &i, const size_t &j )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return false;
    }

    Geom *geom = veh->FindGeom( m_geo_geomID[j] );

    if ( !geom )
    {
        return false;
    }

    // IF
    // ==========
    // Same Geom AND Geom[i] is main surface
    // OR
    // Not the same Geom AND Ancestor of Geom[j] is Geom[i] AND Geom[i] is 0th surface AND Geom[j] not an expanded list
    // OR
    // is custom geom (TODO: this could use some work)
    // ==========
    // AND
    // ==========
    // Shape types are the same AND
    if ( i != j ) // If not the same geom
    {
        if ( m_geo_masterRow[i] && m_geo_subsurfID[j].compare( "" ) == 0 && m_geo_subsurfID[i].compare( "" ) == 0 )
        {
            return ( ( m_geo_geomID[i].compare( m_geo_geomID[j] ) == 0 ) ||
                     ( m_geo_geomID[i].compare( m_geo_geomID[j] ) != 0 &&
                       m_geo_geomID[i].compare( geom->GetAncestorID( m_geo_groupedAncestorGen[j] ) ) == 0 ) ||
                     ( ( m_geo_label[i].substr( 0, 3 ).compare( "[W]" ) == 0 || m_geo_label[i].substr( 0, 3 ).compare( "[B]" ) == 0 ) &&
                       ( m_geo_geomID[i].compare( m_geo_geomID[j] ) == 0 ) ) ) &&
                   ( m_geo_shapeType[i] == m_geo_shapeType[j] );
        }
    }
    return false;
}

void ParasiteDragMgrSingleton::UpdateRefWing()
{
    // Update Reference Area Section
    if ( m_RefFlag() == vsp::MANUAL_REF )
    {
        // Allow Manual Input
        m_Sref.Activate();
    }
    else
    {
        // Pull from existing geometry
        Geom* refgeom = VehicleMgr.GetVehicle()->FindGeom( m_RefGeomID );

        if ( refgeom )
        {
            if ( refgeom->GetType().m_Type == MS_WING_GEOM_TYPE )
            {
                WingGeom* refwing = dynamic_cast < WingGeom* > ( refgeom );

                if ( refwing )
                {
                    m_Sref.Set( refwing->m_TotalArea() );
                    m_Sref.Deactivate();
                }
            }
        }
        else
        {
            m_RefGeomID = string();
        }
    }
}

void ParasiteDragMgrSingleton::UpdateAtmos()
{
    double LqRe;
    double vinf, temp, pres, rho, dynavisc;
    vinf = m_Vinf();
    temp = m_Temp();
    pres = m_Pres();
    rho = m_Rho();
    dynavisc = m_DynaVisc();

    // Determine Which Atmos Variant will Calculate Atmospheric Properties
    if ( m_FreestreamType() == vsp::ATMOS_TYPE_US_STANDARD_1976 )
    {
        m_Atmos.USStandardAtmosphere1976( m_Hinf(), m_DeltaT(), m_AltLengthUnit(), m_TempUnit.Get(), m_PresUnit(), m_SpecificHeatRatio() );
        if ( m_VinfUnitType() == vsp::V_UNIT_MACH )
        {
            m_Atmos.SetMach( m_Vinf() );
        }
        else
        {
            m_Atmos.UpdateMach( vinf, m_TempUnit(), m_VinfUnitType() );
        }
    }
    else if ( m_FreestreamType() == vsp::ATMOS_TYPE_HERRINGTON_1966 )
    {
        m_Atmos.USAF1966( m_Hinf(), m_DeltaT(), m_AltLengthUnit(), m_TempUnit.Get(), m_PresUnit(), m_SpecificHeatRatio() );
        if ( m_VinfUnitType() == vsp::V_UNIT_MACH )
        {
            m_Atmos.SetMach( m_Vinf() );
        }
        else
        {
            m_Atmos.UpdateMach( vinf, m_TempUnit(), m_VinfUnitType() );
        }
    }
    else if ( m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_P_R || m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_P_T || m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_R_T )
    {
        m_Atmos.SetManualQualities( vinf, temp, pres, rho, dynavisc, m_SpecificHeatRatio(),
                                    m_Hinf(), m_AltLengthUnit(), m_VinfUnitType(), 
                                    m_TempUnit(), m_PresUnit(), m_FreestreamType() );
    }
    else if ( m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_RE_L )
    {
        UpdateVinf( m_VinfUnitType() );
    }

    if ( m_FreestreamType() != vsp::ATMOS_TYPE_MANUAL_RE_L )
    {
        SetFreestreamParms();

        if ( m_VinfUnitType() != vsp::V_UNIT_MACH )
        {
            if ( m_VinfUnitType() == vsp::V_UNIT_KEAS ) // KEAS to KTAS
            {
                vinf *= sqrt( 1.0 / m_Atmos.GetDensityRatio() );
            }

            if ( m_AltLengthUnit() == vsp::PD_UNITS_IMPERIAL )
            {
                vinf = ConvertVelocity( vinf, m_VinfUnitType.Get(), vsp::V_UNIT_FT_S );
            }
            else if ( m_AltLengthUnit() == vsp::PD_UNITS_METRIC )
            {
                vinf = ConvertVelocity( vinf, m_VinfUnitType.Get(), vsp::V_UNIT_M_S );
            }
        }
        else
        {
            vinf *= m_Atmos.GetSoundSpeed(); // m/s
            if ( m_AltLengthUnit() == vsp::PD_UNITS_IMPERIAL )
            {
                vinf = ConvertVelocity( vinf, vsp::V_UNIT_M_S, vsp::V_UNIT_FT_S );
            }
        }

        m_KineVisc = m_Atmos.GetDynaVisc() / m_Rho();

        LqRe = m_KineVisc() / vinf; // Either ft or m

        if ( m_AltLengthUnit() == vsp::PD_UNITS_IMPERIAL )
        {
            LqRe = ConvertLength( LqRe, vsp::LEN_FT, m_LengthUnit() );
        }
        else if ( m_AltLengthUnit() == vsp::PD_UNITS_METRIC )
        {
            LqRe = ConvertLength( LqRe, vsp::LEN_M, m_LengthUnit() );
        }

        m_ReqL.Set( 1.0 / LqRe );
    }
}

void ParasiteDragMgrSingleton::UpdateVinf( int newunit )
{
    if ( newunit == m_VinfUnitType() )
    {
        return;
    }

    double new_vinf;
    if ( newunit != vsp::V_UNIT_MACH )
    {
        if ( m_VinfUnitType() == vsp::V_UNIT_MACH )
        {
            m_Vinf *= m_Atmos.GetSoundSpeed(); // m/s
            new_vinf = ConvertVelocity( m_Vinf(), vsp::V_UNIT_M_S, newunit ); // KTAS

            if ( newunit == vsp::V_UNIT_KEAS )
            {
                new_vinf /= sqrt( 1.0 / m_Atmos.GetDensityRatio() ); // KTAS to KEAS
            }
        }
        else if ( m_VinfUnitType() == vsp::V_UNIT_KEAS )
        {
            m_Vinf *= sqrt( 1.0 / m_Atmos.GetDensityRatio() ); // KEAS to KTAS
            new_vinf = ConvertVelocity( m_Vinf(), m_VinfUnitType(), newunit );
        }
        else if ( newunit == vsp::V_UNIT_KEAS )
        {
            new_vinf = ConvertVelocity( m_Vinf(), m_VinfUnitType(), newunit ); // KTAS
            new_vinf /= sqrt( 1.0 / m_Atmos.GetDensityRatio() ); // KTAS to KEAS
        }
        else
        {
            new_vinf = ConvertVelocity( m_Vinf(), m_VinfUnitType(), newunit );
        }
    }
    else
    {
        new_vinf = m_Atmos.GetMach();
    }
    m_Vinf.Set( new_vinf );
    m_VinfUnitType.Set( newunit );
}

void ParasiteDragMgrSingleton::UpdateAlt( int newunit )
{
    double new_alt = m_Hinf();
    double new_rho = m_Rho();
    double new_dyvisc = m_DynaVisc();
    if ( newunit == vsp::PD_UNITS_IMPERIAL && m_AltLengthUnit() == vsp::PD_UNITS_METRIC )
    {
        new_alt = ConvertLength( m_Hinf(), vsp::LEN_M, vsp::LEN_FT );
        new_rho = ConvertDensity( new_rho, vsp::RHO_UNIT_KG_M3, vsp::RHO_UNIT_SLUG_FT3 ); // slug/ft^3
        new_dyvisc = ConvertDynaVis( new_dyvisc, vsp::PD_UNITS_METRIC, vsp::PD_UNITS_IMPERIAL ); // slug/ft*s
    }
    else if ( newunit == vsp::PD_UNITS_METRIC && m_AltLengthUnit() == vsp::PD_UNITS_IMPERIAL )
    {
        new_alt = ConvertLength( m_Hinf(), vsp::LEN_FT, vsp::LEN_M );
        new_rho = ConvertDensity( new_rho, vsp::RHO_UNIT_SLUG_FT3, vsp::RHO_UNIT_KG_M3 ); // kg/m^3
        new_dyvisc = ConvertDynaVis( new_dyvisc, vsp::PD_UNITS_IMPERIAL, vsp::PD_UNITS_METRIC ); // kg/m*s
    }

    m_Hinf.Set( new_alt );
    m_Rho.Set( new_rho );
    m_DynaVisc.Set( new_dyvisc );
    m_AltLengthUnit.Set( newunit );
}

void ParasiteDragMgrSingleton::UpdateTemp( int newunit )
{
    // Temporarily decrease lower limit of m_Temp to allow setting to lower value than previous limit. 
    //  For example, if m_Temp == 200 K, the conversion would set it to -73.15 F which would be below 
    //  the current limit
    m_Temp.SetLowerLimit( -500 );

    double new_temp = ConvertTemperature( m_Temp(), m_TempUnit(), newunit );
    m_Temp.Set( new_temp );

    new_temp = ConvertTemperature( m_DeltaT(), m_TempUnit(), newunit ) - ConvertTemperature( 0.0, m_TempUnit(), newunit );
    m_DeltaT.Set( new_temp );

    m_TempUnit.Set( newunit );
    UpdateTempLimits();
}

void ParasiteDragMgrSingleton::UpdateTempLimits()
{
    switch ( m_TempUnit() )
    {
    case vsp::TEMP_UNIT_C:
        m_Temp.SetLowerLimit( -273.15 );
        break;

    case vsp::TEMP_UNIT_F:
        m_Temp.SetLowerLimit( -459.666 );
        break;

    case vsp::TEMP_UNIT_K:
        m_Temp.SetLowerLimit( 0.0 );
        break;

    case vsp::TEMP_UNIT_R:
        m_Temp.SetLowerLimit( 0.0 );
        break;
    }
}

void ParasiteDragMgrSingleton::UpdatePres( int newunit )
{
    double new_pres = ConvertPressure( m_Pres(), m_PresUnit(), newunit );
    m_Pres.Set( new_pres );
    m_PresUnit.Set( newunit );
}

void ParasiteDragMgrSingleton::UpdatePercentageCD()
{
    double totalCD0 = GetTotalCD();
    double ftotal = 0;
    double percTotal = 0;

    for ( int i = 0; i < m_geo_CD.size(); i++ )
    {
        if ( !m_DegenGeomVec.empty() )
        {
            if ( !m_geo_masterRow[i] )
            {
                m_geo_percTotalCD.push_back( m_geo_CD[i] / totalCD0 );
                percTotal += m_geo_CD[i] / totalCD0;
                ftotal += m_geo_f[i];
            }
            else
            {
                m_geo_percTotalCD.push_back( 0.0 );
            }
        }
        else
        {
            m_geo_percTotalCD.push_back( 0 );
        }
    }

    m_GeomfTotal = ftotal;
    m_GeomPercTotal = percTotal;

    ftotal = 0;
    percTotal = 0;
    for ( size_t i = 0; i < m_ExcresRowVec.size(); ++i )
    {
        if ( !m_DegenGeomVec.empty() )
        {
            m_ExcresRowVec[i].PercTotalCD = m_ExcresRowVec[i].Amount / totalCD0;
            percTotal += m_ExcresRowVec[i].Amount / totalCD0;
            ftotal += m_ExcresRowVec[i].f;
        }
        else
        {
            m_ExcresRowVec[i].PercTotalCD = 0.0;
        }
    }

    m_ExcresfTotal = ftotal;
    m_ExcresPercTotal = percTotal;

    UpdateMasterPercCD();
}

void ParasiteDragMgrSingleton::UpdateMasterPercCD()
{
    if ( !m_DegenGeomVec.empty() )
    {
        for ( size_t i = 0; i < m_RowSize; ++i )
        {
            for ( size_t j = 0; j < m_RowSize; ++j )
            {
                if ( ShouldAddSubSurfToMasterGeom( i, j ) )
                {
                    m_geo_percTotalCD[i] += m_geo_percTotalCD[j];
                }
            }
        }

        for ( size_t i = 0; i < m_RowSize; ++i )
        {
            for ( size_t j = 0; j < m_RowSize; ++j )
            {
                if ( ShouldAddGeomToMasterGeom( i, j ) )
                {
                    m_geo_percTotalCD[i] += m_geo_percTotalCD[j];
                }
            }
        }
    }
}


void ParasiteDragMgrSingleton::UpdateParmActivity()
{
    // Activate/Deactivate Appropriate Flow Condition Parameters
    DeactivateParms();

    if ( m_FreestreamType() == vsp::ATMOS_TYPE_US_STANDARD_1976 || m_FreestreamType() == vsp::ATMOS_TYPE_HERRINGTON_1966 )
    {
        m_Vinf.Activate();
        m_Hinf.Activate();
        m_DeltaT.Activate();
    }
    else if ( m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_P_R )
    {
        m_Vinf.Activate();
        m_Pres.Activate();
        m_Rho.Activate();
        m_SpecificHeatRatio.Activate();
    }
    else if ( m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_P_T )
    {
        m_Vinf.Activate();
        m_Temp.Activate();
        m_Pres.Activate();
        m_SpecificHeatRatio.Activate();
    }
    else if ( m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_R_T )
    {
        m_Vinf.Activate();
        m_Temp.Activate();
        m_Rho.Activate();
        m_SpecificHeatRatio.Activate();
    }
    else if ( m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_RE_L )
    {
        m_ReqL.Activate();
        m_Mach.Activate();
        m_SpecificHeatRatio.Activate();
    }

    if ( m_TurbCfEqnType() == vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION ||
         m_TurbCfEqnType() == vsp::CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH )
    {
        m_SpecificHeatRatio.Activate();
    }
}

void ParasiteDragMgrSingleton::UpdateExportLabels()
{
    // Flow Qualities
    switch ( m_AltLengthUnit.Get() )
    {
    case vsp::PD_UNITS_IMPERIAL:
        m_RhoLabel = "Density (slug/ft^3)";
        m_AltLabel = "Altitude (ft)";
        break;

    case vsp::PD_UNITS_METRIC:
        m_RhoLabel = "Density (kg/m^3)";
        m_AltLabel = "Altitude (m)";
        break;
    }

    // Length
    switch ( m_LengthUnit.Get() )
    {
    case vsp::LEN_MM:
        m_LrefLabel = "L_ref (mm)";
        m_SrefLabel = "S_ref (mm^2)";
        m_fLabel = "f (mm^2)";
        m_SwetLabel = "S_wet (mm^2)";
        break;

    case vsp::LEN_CM:
        m_LrefLabel = "L_ref (cm)";
        m_SrefLabel = "S_ref (cm^2)";
        m_fLabel = "f (cm^2)";
        m_SwetLabel = "S_wet (cm^2)";
        break;

    case vsp::LEN_M:
        m_LrefLabel = "L_ref (m)";
        m_SrefLabel = "S_ref (m^2)";
        m_fLabel = "f (m^2)";
        m_SwetLabel = "S_wet (m^2)";
        break;

    case vsp::LEN_IN:
        m_LrefLabel = "L_ref (in)";
        m_SrefLabel = "S_ref (in^2)";
        m_fLabel = "f (in^2)";
        m_SwetLabel = "S_wet (in^2)";
        break;

    case vsp::LEN_FT:
        m_LrefLabel = "L_ref (ft)";
        m_SrefLabel = "S_ref (ft^2)";
        m_fLabel = "f (ft^2)";
        m_SwetLabel = "S_wet (ft^2)";
        break;

    case vsp::LEN_YD:
        m_LrefLabel = "L_ref (yd)";
        m_SrefLabel = "S_ref (yd^2)";
        m_fLabel = "f (yd^2)";
        m_SwetLabel = "S_wet (yd^2)";
        break;

    case vsp::LEN_UNITLESS:
        m_LrefLabel = "L_ref (LU)";
        m_SrefLabel = "S_ref (LU^2)";
        m_fLabel = "f (LU^2)";
        m_SwetLabel = "S_wet (LU^2)";
        break;
    }

    // Vinf
    switch ( m_VinfUnitType.Get() )
    {
    case vsp::V_UNIT_FT_S:
        m_VinfLabel = "Vinf (ft/s)";
        break;

    case vsp::V_UNIT_M_S:
        m_VinfLabel = "Vinf (m/s)";
        break;

    case vsp::V_UNIT_KEAS:
        m_VinfLabel = "Vinf (KEAS)";
        break;

    case vsp::V_UNIT_KTAS:
        m_VinfLabel = "Vinf (KTAS)";
        break;

    case vsp::V_UNIT_MPH:
        m_VinfLabel = "Vinf (mph)";
        break;

    case vsp::V_UNIT_KM_HR:
        m_VinfLabel = "Vinf (km/hr)";
        break;

    case vsp::V_UNIT_MACH:
        m_VinfLabel = "Vinf (Mach)";
    }

    // Temperature
    switch ( m_TempUnit.Get() )
    {
    case vsp::TEMP_UNIT_C:
        m_TempLabel = "Temp (C)";
        break;

    case vsp::TEMP_UNIT_F:
        m_TempLabel = "Temp (F)";
        break;

    case vsp::TEMP_UNIT_K:
        m_TempLabel = "Temp (K)";
        break;

    case vsp::TEMP_UNIT_R:
        m_TempLabel = "Temp (R)";
        break;
    }

    // Pressure
    switch ( m_PresUnit() )
    {
    case vsp::PRES_UNIT_PSF:
        m_PresLabel = "Pressure (lbf/ft^2)";
        break;

    case vsp::PRES_UNIT_PSI:
        m_PresLabel = "Pressure (lbf/in^2)";
        break;

    case vsp::PRES_UNIT_BA:
        m_PresLabel = "Pressure (Ba)";
        break;

    case vsp::PRES_UNIT_PA:
        m_PresLabel = "Pressure (Pa)";
        break;

    case vsp::PRES_UNIT_KPA:
        m_PresLabel = "Pressure (kPa)";
        break;

    case vsp::PRES_UNIT_MPA:
        m_PresLabel = "Pressure (MPa)";
        break;

    case vsp::PRES_UNIT_INCHHG:
        m_PresLabel = "Pressure (\"Hg)";
        break;

    case vsp::PRES_UNIT_MMHG:
        m_PresLabel = "Pressure (mmHg)";
        break;

    case vsp::PRES_UNIT_MMH20:
        m_PresLabel = "Pressure (mmH20)";
        break;

    case vsp::PRES_UNIT_MB:
        m_PresLabel = "Pressure (mB)";
        break;

    case vsp::PRES_UNIT_ATM:
        m_PresLabel = "Pressure (atm)";
        break;
    }
}

void ParasiteDragMgrSingleton::UpdateExcres()
{
    // Updates Current Excres Value & Updates Excres Values that are a % of Cd_Geom
    for ( size_t i = 0; i < m_ExcresRowVec.size(); ++i )
    {
        if ( i == m_CurrentExcresIndex )
        {
            m_ExcresRowVec[i].Input = m_ExcresValue();

            if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_CD )
            {
                m_ExcresRowVec[i].Amount = m_ExcresValue();
            }
            else if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_COUNT )
            {
                m_ExcresRowVec[i].Amount = m_ExcresValue() / 10000.0;
            }
            else if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_PERCENT_GEOM )
            {
                m_ExcresRowVec[i].Amount = CalcPercentageGeomCD( m_ExcresValue() );
            }
            else if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_MARGIN )
            {
                m_ExcresRowVec[i].Amount = CalcPercentageTotalCD( m_ExcresValue() );
            }
            else if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_DRAGAREA )
            {
                m_ExcresRowVec[i].Amount = CalcDragAreaCD( m_ExcresValue() );
            }
        }
        else
        {
            if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_PERCENT_GEOM )
            {
                m_ExcresRowVec[i].Amount = CalcPercentageGeomCD( m_ExcresRowVec[i].Input );
            }
            else if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_MARGIN )
            {
                m_ExcresRowVec[i].Amount = CalcPercentageTotalCD( m_ExcresRowVec[i].Input );
            }
            else if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_DRAGAREA )
            {
                m_ExcresRowVec[i].Amount = CalcDragAreaCD( m_ExcresRowVec[i].Input );
            }
        }
    }

    // Calculates Individual f
    for ( size_t i = 0; i < m_ExcresRowVec.size(); ++i )
    {
        if ( GetSubTotalCD() > 0.0 )
        {
            m_ExcresRowVec[i].f = m_ExcresRowVec[i].Amount * m_Sref.Get();
        }
    }

    ConsolidateExcres();
}

void ParasiteDragMgrSingleton::UpdateCurrentExcresVal()
{
    m_ExcresType.Set( m_ExcresRowVec[m_CurrentExcresIndex].Type );
    if ( m_ExcresType() == vsp::EXCRESCENCE_CD )
    {
        m_ExcresValue.SetLowerUpperLimits( 0.0, 10.0 );
    }
    else if ( m_ExcresType() == vsp::EXCRESCENCE_COUNT )
    {
        m_ExcresValue.SetLowerUpperLimits( 0.0, 100000.0 );
    }
    else if ( m_ExcresType() == vsp::EXCRESCENCE_PERCENT_GEOM )
    {
        m_ExcresValue.SetLowerUpperLimits( 0.0, 1000.0 );
    }
    else if ( m_ExcresType() == vsp::EXCRESCENCE_MARGIN )
    {
        m_ExcresValue.SetLowerUpperLimits( 0.0, 1000.0 );
    }
    else if ( m_ExcresType() == vsp::EXCRESCENCE_DRAGAREA )
    {
        m_ExcresValue.SetLowerUpperLimits( 0.0, 10000.0 );
    }
    m_ExcresValue.Set( m_ExcresRowVec[m_CurrentExcresIndex].Input );
}

string ParasiteDragMgrSingleton::ExportToCSV()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return string();
    }

    // Create Results
    Results* res = ResultsMgr.CreateResults( "Parasite_Drag" );

    if ( !res )
    {
        return string();
    }

    // Add Results to ResultsMgr
    UpdateExcres();
    UpdateExportLabels();

    res->Add( NameValData( "Alt_Label", m_AltLabel ) );
    res->Add( NameValData( "Vinf_Label", m_VinfLabel ) );
    res->Add( NameValData( "Sref_Label", m_SrefLabel ) );
    res->Add( NameValData( "Temp_Label", m_TempLabel ) );
    res->Add( NameValData( "Pres_Label", m_PresLabel ) );
    res->Add( NameValData( "Rho_Label", m_RhoLabel ) );
    res->Add( NameValData( "LamCfEqnName", m_LamCfEqnName ) );
    res->Add( NameValData( "TurbCfEqnName", m_TurbCfEqnName ) );
    res->Add( NameValData( "Swet_Label", m_SwetLabel ) );
    res->Add( NameValData( "Lref_Label", m_LrefLabel ) );
    res->Add( NameValData( "f_Label", m_fLabel ) );

    // Flow Condition
    res->Add( NameValData( "FC_Mach", m_Mach() ) );
    res->Add( NameValData( "FC_Alt", m_Hinf.Get() ) );
    res->Add( NameValData( "FC_Vinf", m_Vinf.Get() ) );
    res->Add( NameValData( "FC_Sref", m_Sref.Get() ) );
    res->Add( NameValData( "FC_dTemp", m_DeltaT.Get() ) );
    res->Add( NameValData( "FC_Temp", m_Temp.Get() ) );
    res->Add( NameValData( "FC_Pres", m_Pres.Get() ) );
    res->Add( NameValData( "FC_Rho", m_Rho.Get() ) );

    // Component Related
    if ( !m_ExportSubCompFlag() )
    {
        // Only create results for the primary components

        vector < string > new_ID_vec, new_geo_label, new_geo_ffName;
        vector < double > new_geo_swet, new_geo_lref, new_geo_Re, new_geo_percLam, new_geo_cf, new_geo_fineRatorToC,
            new_geo_ffIn, new_geo_ffOut, new_geo_Roughness, new_geo_TeTwRatio, new_geo_TawTwRatio, new_geo_Q,
            new_geo_f, new_geo_CD, new_geo_percTotalCD;
        vector <int> new_geo_ffType, new_geo_surfNum;

        for ( size_t i = 0; i < m_geo_geomID.size(); i++ )
        {
            if ( std::find( new_ID_vec.begin(), new_ID_vec.end(), m_geo_geomID[i] ) == new_ID_vec.end() )
            {
                new_ID_vec.push_back( m_geo_geomID[i] );
                new_geo_label.push_back( m_geo_label[i] );
                new_geo_swet.push_back( m_geo_swet[i] );
                new_geo_lref.push_back( m_geo_lref[i] );
                new_geo_Re.push_back( m_geo_Re[i] );
                new_geo_percLam.push_back( m_geo_percLam[i] );
                new_geo_cf.push_back( m_geo_cf[i] );
                new_geo_fineRatorToC.push_back( m_geo_fineRat_or_toc[i] );
                new_geo_ffType.push_back( m_geo_ffType[i] );
                new_geo_ffName.push_back( m_geo_ffName[i] );
                new_geo_ffIn.push_back( m_geo_ffIn[i] );
                new_geo_ffOut.push_back( m_geo_ffOut[i] );
                new_geo_Roughness.push_back( m_geo_Roughness[i] );
                new_geo_TeTwRatio.push_back( m_geo_TeTwRatio[i] );
                new_geo_TawTwRatio.push_back( m_geo_TawTwRatio[i] );
                new_geo_Q.push_back( m_geo_Q[i] );
                new_geo_f.push_back( m_geo_f[i] );
                new_geo_CD.push_back( m_geo_CD[i] );
                new_geo_percTotalCD.push_back( m_geo_percTotalCD[i] );
                new_geo_surfNum.push_back( m_geo_surfNum[i] );
            }
        }

        int num_comp = new_ID_vec.size();

        res->Add( NameValData( "Num_Comp", num_comp ) );
        res->Add( NameValData( "Comp_ID", new_ID_vec ) );
        res->Add( NameValData( "Comp_Label", new_geo_label ) );
        res->Add( NameValData( "Comp_Swet", new_geo_swet ) );
        res->Add( NameValData( "Comp_Lref", new_geo_lref ) );
        res->Add( NameValData( "Comp_Re", new_geo_Re ) );
        res->Add( NameValData( "Comp_PercLam", new_geo_percLam ) );
        res->Add( NameValData( "Comp_Cf", new_geo_cf ) );
        res->Add( NameValData( "Comp_FineRat", new_geo_fineRatorToC ) );
        res->Add( NameValData( "Comp_FFEqn", new_geo_ffType ) );
        res->Add( NameValData( "Comp_FFEqnName", new_geo_ffName ) );
        res->Add( NameValData( "Comp_FFIn", new_geo_ffIn ) );
        res->Add( NameValData( "Comp_FFOut", new_geo_ffOut ) );
        res->Add( NameValData( "Comp_Roughness", new_geo_Roughness ) );
        res->Add( NameValData( "Comp_TeTwRatio", new_geo_TeTwRatio ) );
        res->Add( NameValData( "Comp_TawTwRatio", new_geo_TawTwRatio ) );
        res->Add( NameValData( "Comp_Q", new_geo_Q ) );
        res->Add( NameValData( "Comp_f", new_geo_f ) );
        res->Add( NameValData( "Comp_CD", new_geo_CD ) );
        res->Add( NameValData( "Comp_PercTotalCD", new_geo_percTotalCD ) );
        res->Add( NameValData( "Comp_SurfNum", m_geo_surfNum ) );
    }
    else
    {
        res->Add( NameValData( "Num_Comp", m_RowSize ) );
        res->Add( NameValData( "Comp_ID", m_geo_geomID ) );
        res->Add( NameValData( "Comp_Label", m_geo_label ) );
        res->Add( NameValData( "Comp_Swet", m_geo_swet ) );
        res->Add( NameValData( "Comp_Lref", m_geo_lref ) );
        res->Add( NameValData( "Comp_Re", m_geo_Re ) );
        res->Add( NameValData( "Comp_PercLam", m_geo_percLam ) );
        res->Add( NameValData( "Comp_Cf", m_geo_cf ) );
        res->Add( NameValData( "Comp_FineRat", m_geo_fineRat_or_toc ) );
        res->Add( NameValData( "Comp_FFEqn", m_geo_ffType ) );
        res->Add( NameValData( "Comp_FFEqnName", m_geo_ffName ) );
        res->Add( NameValData( "Comp_FFIn", m_geo_ffIn ) );
        res->Add( NameValData( "Comp_FFOut", m_geo_ffOut ) );
        res->Add( NameValData( "Comp_Roughness", m_geo_Roughness ) );
        res->Add( NameValData( "Comp_TeTwRatio", m_geo_TeTwRatio ) );
        res->Add( NameValData( "Comp_TawTwRatio", m_geo_TawTwRatio ) );
        res->Add( NameValData( "Comp_Q", m_geo_Q ) );
        res->Add( NameValData( "Comp_f", m_geo_f ) );
        res->Add( NameValData( "Comp_CD", m_geo_CD ) );
        res->Add( NameValData( "Comp_PercTotalCD", m_geo_percTotalCD ) );
        res->Add( NameValData( "Comp_SurfNum", m_geo_surfNum ) );
    }

    // Excres Related
    res->Add( NameValData( "Num_Excres", ( int )m_ExcresRowVec.size() ) );
    res->Add( NameValData( "Excres_Label", m_excres_Label ) );
    res->Add( NameValData( "Excres_Type", m_excres_Type ) );
    res->Add( NameValData( "Excres_Input", m_excres_Input ) );
    res->Add( NameValData( "Excres_f", m_excres_f ) );
    res->Add( NameValData( "Excres_Amount", m_excres_Amount ) );
    res->Add( NameValData( "Excres_PercTotalCD", m_excres_PercTotalCD ) );

    // Totals
    res->Add( NameValData( "Geom_f_Total", GetGeomfTotal() ) );
    res->Add( NameValData( "Geom_CD_Total", GetGeometryCD() ) );
    res->Add( NameValData( "Geom_Perc_Total", GetGeomPercTotal() ) );
    res->Add( NameValData( "Excres_f_Total", GetExcresfTotal() ) );
    res->Add( NameValData( "Excres_CD_Total", GetTotalExcresCD() ) );
    res->Add( NameValData( "Excres_Perc_Total", GetExcresPercTotal() ) );
    res->Add( NameValData( "Total_f_Total", GetfTotal() ) );
    res->Add( NameValData( "Total_CD_Total", GetTotalCD() ) );
    res->Add( NameValData( "Total_Perc_Total", GetPercTotal() ) );

    string f_name = veh->getExportFileName( vsp::DRAG_BUILD_CSV_TYPE );
    res->WriteParasiteDragFile( f_name );

    return res->GetID();
}

string ParasiteDragMgrSingleton::ExportToCSV( const string & file_name )
{
    m_FileName = file_name;
    return ExportToCSV();
}

// ========================================================== //
// =================== General Methods ====================== //
// ========================================================== //

void ParasiteDragMgrSingleton::ClearInputVectors()
{
    m_geo_masterRow.clear();
    m_geo_geomID.clear();
    m_geo_subsurfID.clear();
    m_geo_name.clear();
    m_geo_label.clear();
    m_geo_percLam.clear();
    m_geo_shapeType.clear();
    m_geo_ffIn.clear();
    m_geo_Q.clear();
    m_geo_Roughness.clear();
    m_geo_TeTwRatio.clear();
    m_geo_TawTwRatio.clear();
    m_geo_surfNum.clear();
    m_geo_expandedList.clear();
}

void ParasiteDragMgrSingleton::ClearOutputVectors()
{
    m_geo_groupedAncestorGen.clear();
    m_geo_swet.clear();
    m_geo_lref.clear();
    m_geo_Re.clear();
    m_geo_cf.clear();
    m_geo_fineRat_or_toc.clear();
    m_geo_ffType.clear();
    m_geo_ffName.clear();
    m_geo_ffOut.clear();
    m_geo_f.clear();
    m_geo_CD.clear();
    m_geo_percTotalCD.clear();
}

xmlNodePtr ParasiteDragMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    char str[256];
    xmlNodePtr ParasiteDragnode = xmlNewChild( node, NULL, BAD_CAST"ParasiteDragMgr", NULL );

    ParmContainer::EncodeXml( ParasiteDragnode );
    XmlUtil::AddStringNode( ParasiteDragnode, "ReferenceGeomID", m_RefGeomID );

    xmlNodePtr ExcresDragnode = xmlNewChild( ParasiteDragnode, NULL, BAD_CAST"Excrescence", NULL );

    XmlUtil::AddIntNode( ExcresDragnode, "NumExcres", m_ExcresRowVec.size() );

    for ( size_t i = 0; i < m_ExcresRowVec.size(); ++i )
    {
        sprintf( str, "Excres_%zu", i );
        xmlNodePtr excresqualnode = xmlNewChild( ExcresDragnode, NULL, BAD_CAST str, NULL );

        XmlUtil::AddStringNode( excresqualnode, "Label", m_ExcresRowVec[i].Label );
        XmlUtil::AddIntNode( excresqualnode, "Type", m_ExcresRowVec[i].Type );
        XmlUtil::AddDoubleNode( excresqualnode, "Input", m_ExcresRowVec[i].Input );
    }

    return ParasiteDragnode;
}

xmlNodePtr ParasiteDragMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    char str[256];

    xmlNodePtr ParasiteDragnode = XmlUtil::GetNode( node, "ParasiteDragMgr", 0 );

    int numExcres;
    if ( ParasiteDragnode )
    {
        ParmContainer::DecodeXml( ParasiteDragnode );
        m_RefGeomID = XmlUtil::FindString( ParasiteDragnode, "ReferenceGeomID", m_RefGeomID );

        xmlNodePtr ExcresDragnode = XmlUtil::GetNode( ParasiteDragnode, "Excrescence", 0 );

        numExcres = XmlUtil::FindInt( ExcresDragnode, "NumExcres", 0 );

        for ( int i = 0; i < numExcres; i++ )
        {
            sprintf( str, "Excres_%i", i );
            xmlNodePtr excresqualnode = XmlUtil::GetNode( ExcresDragnode, str, 0 );

            m_ExcresType.Set( XmlUtil::FindInt( excresqualnode, "Type", 0 ) );
            m_ExcresValue.Set( XmlUtil::FindDouble( excresqualnode, "Input", 0.0 ) );
            m_ExcresName = XmlUtil::FindString(excresqualnode, "Label", "");

            AddExcrescence();
        }
    }

    return ParasiteDragnode;
}

void ParasiteDragMgrSingleton::SortMap()
{
    SortMainTableVecByGroupedAncestorGeoms();
    switch ( m_SortByFlag() )
    {
    case PD_SORT_NONE:
        break;

    case PD_SORT_WETTED_AREA:
        SortMapByWettedArea();
        break;

    case PD_SORT_PERC_CD:
        SortMapByPercentageCD();
        break;

    default:
        break;
    }
}

void ParasiteDragMgrSingleton::SortMapByWettedArea()
{
    // Create New Map That Contains Wetted Area Value, Geom ID
    // Sort this Map
    // Recreate m_TableRowMap
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return;
    }

    vector < ParasiteDragTableRow > temp;
    vector < bool > isSorted( m_TableRowVec.size(), false );
    int cur_max_ind = 0;
    int i = 0;

    while ( !CheckAllTrue( isSorted ) )
    {
        if ( !isSorted[i] )
        {
            // Grabs Current Max Index Based on Unsorted Numbers
            cur_max_ind = i;
            for ( size_t j = 0; j < m_TableRowVec.size(); ++j )
            {
                if ( !isSorted[j] )
                {
                    if ( m_TableRowVec[j].Swet > m_TableRowVec[cur_max_ind].Swet )
                    {
                        cur_max_ind = j;
                    }
                }
            }
            isSorted[cur_max_ind] = true;
            temp.push_back( m_TableRowVec[cur_max_ind] );

            // Immediately pushes back any reflected surfaces behind last MAX
            for ( size_t j = 0; j < m_TableRowVec.size(); ++j )
            {
                if ( m_TableRowVec[cur_max_ind].GeomID.compare( m_TableRowVec[j].GeomID ) == 0 && cur_max_ind != j && !isSorted[j] )
                {
                    isSorted[j] = true;
                    temp.push_back( m_TableRowVec[j] );
                }
            }

            // Then pushes back any incorporated geoms behind reflected surfaces
            for ( size_t j = 0; j < m_TableRowVec.size(); ++j )
            {
                Geom* geom = veh->FindGeom( m_TableRowVec[j].GeomID );
                if ( geom )
                {
                    if ( m_TableRowVec[cur_max_ind].GeomID.compare( geom->GetAncestorID( m_TableRowVec[j].GroupedAncestorGen ) ) == 0 && cur_max_ind != j && !isSorted[j] )
                    {
                        isSorted[j] = true;
                        temp.push_back( m_TableRowVec[j] );
                    }
                }
            }
        }
        if ( i != isSorted.size() - 1 )
        {
            ++i;
        }
        else
        {
            i = 0;
        }
    }

    m_TableRowVec = temp;
}

void ParasiteDragMgrSingleton::SortMapByPercentageCD()
{
    SortGeometryByPercTotalCD();
    SortExcresByPercTotalCD();
}

void ParasiteDragMgrSingleton::SortGeometryByPercTotalCD()
{
    vector < ParasiteDragTableRow > temp;
    vector < bool > isSorted( m_TableRowVec.size(), false );
    int cur_max_ind = 0;
    int i = 0;

    while ( !CheckAllTrue( isSorted ) )
    {
        if ( !isSorted[i] )
        {
            // Grabs Current Max Index Based on Unsorted Numbers
            cur_max_ind = i;
            for ( size_t j = 0; j < m_TableRowVec.size(); ++j )
            {
                if ( !isSorted[j] )
                {
                    if ( m_TableRowVec[j].PercTotalCD > m_TableRowVec[cur_max_ind].PercTotalCD )
                    {
                        cur_max_ind = j;
                    }
                }
            }
            isSorted[cur_max_ind] = true;
            temp.push_back( m_TableRowVec[cur_max_ind] );

            // Immediately pushes back any reflected surfaces behind last MAX
            for ( size_t j = 0; j < m_TableRowVec.size(); ++j )
            {
                if ( m_TableRowVec[cur_max_ind].GeomID.compare( m_TableRowVec[j].GeomID ) == 0 && cur_max_ind != j && !isSorted[j] )
                {
                    isSorted[j] = true;
                    temp.push_back( m_TableRowVec[j] );
                }
            }

            // Then pushes back any incorporated geoms behind reflected surfaces
            for ( size_t j = 0; j < m_TableRowVec.size(); ++j )
            {
                Geom* geom = VehicleMgr.GetVehicle()->FindGeom( m_TableRowVec[j].GeomID );
                if ( geom )
                {
                    if ( m_TableRowVec[cur_max_ind].GeomID.compare( geom->GetAncestorID( m_TableRowVec[j].GroupedAncestorGen ) ) == 0 && cur_max_ind != j && !isSorted[j] )
                    {
                        isSorted[j] = true;
                        temp.push_back( m_TableRowVec[j] );
                    }
                }
            }
        }

        // Increase until max reached, then go back through to make sure everything is in its place
        if ( i != isSorted.size() - 1 )
        {
            ++i;
        }
        else
        {
            i = 0;
        }
    }

    m_TableRowVec = temp;
}

void ParasiteDragMgrSingleton::SortExcresByPercTotalCD()
{
    vector < ExcrescenceTableRow > tempExcres;
    vector < bool > isSortedExcres( m_ExcresRowVec.size(), false );
    int cur_max_ind = 0;
    int i = 0;

    while ( !CheckAllTrue( isSortedExcres ) )
    {
        if ( !isSortedExcres[i] )
        {
            // Grabs Current Max Index Based on Unsorted Numbers
            cur_max_ind = i;
            for ( size_t j = 0; j < m_ExcresRowVec.size(); ++j )
            {
                if ( !isSortedExcres[j] )
                {
                    if ( m_ExcresRowVec[j].PercTotalCD > m_ExcresRowVec[cur_max_ind].PercTotalCD )
                    {
                        cur_max_ind = j;
                    }
                }
            }
            isSortedExcres[cur_max_ind] = true;
            tempExcres.push_back( m_ExcresRowVec[cur_max_ind] );
        }

        // Increase until max reached, then go back through to make sure everything is in its place
        if ( i != isSortedExcres.size() - 1 )
        {
            ++i;
        }
        else
        {
            i = 0;
        }
    }

    m_ExcresRowVec = tempExcres;
}

void ParasiteDragMgrSingleton::SortMainTableVecByGroupedAncestorGeoms()
{
    // Takes m_TableRowVec
    // For Each Geom ID Check if other geoms have it as an incorporated ID
    // If yes, place those below current geom
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return;
    }

    vector < ParasiteDragTableRow > temp;
    string masterGeomID;
    vector < bool > isSorted( m_TableRowVec.size(), false );

    for ( size_t i = 0; i < m_TableRowVec.size(); ++i )
    {
        if ( !isSorted[i] )
        {
            temp.push_back( m_TableRowVec[i] );
            isSorted[i] = true;

            // Immediately pushes back any reflected surfaces behind last TableRow
            for ( size_t j = 0; j < m_TableRowVec.size(); ++j )
            {
                if ( m_TableRowVec[i].GeomID.compare( m_TableRowVec[j].GeomID ) == 0 && i != j && !isSorted[j] )
                {
                    isSorted[j] = true;
                    temp.push_back( m_TableRowVec[j] );
                }
            }

            // Then pushes back any incorporated geoms behind reflected surfaces
            for ( size_t j = 0; j < m_TableRowVec.size(); ++j )
            {
                masterGeomID = m_TableRowVec[i].GeomID;
                Geom* geom = veh->FindGeom( m_TableRowVec[j].GeomID );
                if ( geom )
                {
                    if ( geom->GetAncestorID( m_TableRowVec[j].GroupedAncestorGen ).compare( masterGeomID ) == 0 && !isSorted[j] )
                    {
                        isSorted[j] = true;
                        temp.push_back(m_TableRowVec[j]);

                        // TODO push back geoms grouped to last grouped geom
                    }
                }
            }
        }
    }

    m_TableRowVec = temp;
}

bool ParasiteDragMgrSingleton::CheckAllTrue( vector<bool> vec )
{
    for ( size_t i = 0; i < vec.size(); ++i )
    {
        if ( !vec[i] )
        {
            return false;
        }
    }
    return true;
}

void ParasiteDragMgrSingleton::DeactivateParms()
{
    m_Vinf.Deactivate();
    m_Hinf.Deactivate();
    m_Temp.Deactivate();
    m_DeltaT.Deactivate();
    m_Pres.Deactivate();
    m_Rho.Deactivate();
    m_SpecificHeatRatio.Deactivate();
    m_DynaVisc.Deactivate();
    m_KineVisc.Deactivate();
    //m_MediumType.Deactivate();
    m_Mach.Deactivate();
    m_ReqL.Deactivate();
}

bool ParasiteDragMgrSingleton::IsSameGeomSet()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return false;
    }

    // Create New Vector of IDs to Compare
    vector <string> newIDVec = veh->GetGeomSet( m_SetChoice() );
    vector < string > newVecToCompare;
    for ( int i = 0; i < newIDVec.size(); i++ )
    {
        Geom* geom = veh->FindGeom( newIDVec[i] );
        if ( geom )
        {
            if ( geom->GetType().m_Type != MESH_GEOM_TYPE &&
                    geom->GetType().m_Type != HUMAN_GEOM_TYPE &&
                    geom->GetType().m_Type != BLANK_GEOM_TYPE &&
                    geom->GetType().m_Type != HINGE_GEOM_TYPE )
            {
                if ( geom->GetSurfType(0) != vsp::DISK_SURF )
                {
                    newVecToCompare.push_back( newIDVec[i] );
                }
            }
        }
        else
        {
            return false;
        }
    }

    // Create Row Size to Compare
    int temprowsize = 0;
    for ( int i = 0; i < newVecToCompare.size(); i++ )
    {
        Geom* geom = veh->FindGeom( newVecToCompare[i] );
        if ( geom )
        {
            temprowsize += geom->GetNumTotalSurfs();
            for ( size_t j = 0; j < geom->GetSubSurfVec().size(); ++j )
            {
                for ( size_t k = 0; k < geom->GetNumSymmCopies(); ++k )
                {
                    ++temprowsize;
                }

                // Subsurfaces are essentially separate geometries and as such get their own master row
                if ( geom->GetSubSurfVec()[j]->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT )
                {
                    ++temprowsize;
                }
            }
        }
    }

    // Compare IDs
    bool isSameGeomVec = true;
    for ( size_t i = 0; i < newVecToCompare.size(); ++i )
    {
        bool exists = false;
        for ( size_t j = 0; j < m_PDGeomIDVec.size(); ++j )
        {
            if ( newVecToCompare[i].compare( m_PDGeomIDVec[j] ) == 0 )
            {
                exists = true;
                break;
            }
        }
        if ( !exists )
        {
            isSameGeomVec = false;
            break;
        }
    }

    if ( temprowsize == m_RowSize - m_PDGeomIDVec.size() && isSameGeomVec ) // Subtract Number of Master Rows Added
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool ParasiteDragMgrSingleton::IsNotZeroLineItem( int index )
{
    // IF NOT subsurface
    // =============
    // is a custom geom (TODO could use work)
    // (m_geo_label[index].substr(0, 3).compare("[W]") == 0 || m_geo_label[index].substr(0, 3).compare("[B]") == 0)
    // =============
    // AND
    // =============
    // Not grouped with ANY ancestors
    // =============
    // Else
    // =============
    // Sub surface is given separate treatment
    // OR
    // Sub surface is NOT Zero Drag

    // If incorporated, swet is added to chosen ancestor
    // Main surf will never be a zero line item
    // Custom types can have several geom types in them
    // If Geom list is expanded all components use their individual swets
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return false;
    }

    if ( m_geo_subsurfID[index].compare( "" ) == 0 )
    {
        if ( m_geo_groupedAncestorGen[index] == 0 )
        {
            return true;
        }
    }
    else
    {
        Geom* geom = veh->FindGeom( m_geo_geomID[index] );
        if ( geom )
        {
            if ( geom->GetSubSurf( m_geo_subsurfID[index] )->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT ||
                    ( geom->GetSubSurf( m_geo_subsurfID[index] )->m_IncludeType() != vsp::SS_INC_ZERO_DRAG ) )
            {
                return true;
            }
        }
        else
        {
            return false;
        }
    }

    return false;
}

void ParasiteDragMgrSingleton::RefreshBaseDataVectors()
{
    // Check For Similarity in Geom Set
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    if ( !IsSameGeomSet() || !HasSameNames() )
    {
        // Reset Geo Vecs & Clear Degen Geom
        veh->ClearDegenGeom();
        m_DegenGeomVec.clear();

        ClearInputVectors();
        ClearOutputVectors();

        // Set New Active Geom Vec
        SetActiveGeomVec();
    }
}

void ParasiteDragMgrSingleton::RenewDegenGeomVec()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->CreateDegenGeom(m_SetChoice());
        string meshID = veh->CompGeomAndFlatten(m_SetChoice(), 0);
        veh->DeleteGeom(meshID);
        veh->ShowOnlySet(m_SetChoice());

        // First Assignment of DegenGeomVec, Will Carry Through to Rest of Calculate_X
        m_DegenGeomVec = veh->GetDegenGeomVec();
    }
}

bool ParasiteDragMgrSingleton::HasSameNames()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return false;
    }

    // Compare Names (If names have changed, Comp Geom Tags are incorrect)
    vector < bool > sameName( m_geo_name.size(), false );
    if ( m_geo_name.size() > 0 )
    {
        for ( size_t i = 0; i < m_PDGeomIDVec.size(); ++i )
        {
            Geom* geom = veh->FindGeom( m_PDGeomIDVec[i] );
            if ( geom )
            {
                for ( size_t j = 0; j < m_geo_name.size(); ++j )
                {
                    if ( geom->GetName().compare( m_geo_name[j] ) == 0 )
                    {
                        sameName[j] = true;
                    }
                    for ( size_t k = 0; k < geom->GetSubSurfVec().size(); ++k )
                    {
                        if ( geom->GetSubSurfVec()[k]->GetName().compare( m_geo_name[j] ) == 0 )
                        {
                        sameName[j] = true;
                        }
                    }
                }
            }
            else
            {
                return false;
            }
        }

        if ( !CheckAllTrue( sameName ) )
        {
            return false;
        }
    }
    return true;
}

bool ParasiteDragMgrSingleton::IsCaclualted()
{
    if ( m_DegenGeomVec.empty() )
    {
        return false;
    }
    return true;
}
