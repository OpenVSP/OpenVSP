//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParasiteDragMgr.cpp: ParasiteDrag Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include <cmath>

#include "Vehicle.h"
#include "ParmMgr.h"
#include "IDMgr.h"
#include "StlHelper.h"
#include "WingGeom.h"
#include "ParasiteDragMgr.h"
#include "ModeMgr.h"
#include "StringUtil.h"

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
    m_ModeID = "";
    m_CurrentExcresIndex = -1;

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
                          vsp::DO_NOT_USE_CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH );
    m_TurbCfEqnType.SetDescript( "Turbulent Cf Equation Choice" );

    m_AltLengthUnit.Init( "AltLengthUnit", groupname, this, vsp::PD_UNITS_IMPERIAL, vsp::PD_UNITS_IMPERIAL, vsp::PD_UNITS_METRIC );
    m_AltLengthUnit.SetDescript( "Altitude Units" );

    m_LengthUnit.Init( "LengthUnit", groupname, this, vsp::LEN_FT, vsp::LEN_MM, vsp::LEN_UNITLESS );
    m_LengthUnit.SetDescript( "Length Units" );

    m_TempUnit.Init( "TempUnit", groupname, this, vsp::TEMP_UNIT_F, vsp::TEMP_UNIT_K, vsp::TEMP_UNIT_R );
    m_TempUnit.SetDescript( "Temperature Units" );

    m_PresUnit.Init( "PresUnit", groupname, this, vsp::PRES_UNIT_PSF, vsp::PRES_UNIT_PSF, vsp::PRES_UNIT_ATM );
    m_PresUnit.SetDescript( "Pressure Units" );

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

    m_SetChoice.Init( "Set", groupname, this, DEFAULT_SET, 0, vsp::MAX_NUM_SETS );
    m_SetChoice.SetDescript( "Selected set for operation" );

    m_UseMode.Init( "UseModeMassFlag", groupname, this, false, 0, 1 );

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
                  vsp::DO_NOT_USE_CF_TURB_ROUGHNESS_WHITE,
                  vsp::DO_NOT_USE_CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH };
    m_TurbBlackList.insert( m_TurbBlackList.begin(), tmp, &tmp[ sizeof( tmp ) / sizeof( *tmp ) ] );

    // Populate list of alternate equations for black-listed ones.
    int tmp2[] = { m_TurbTypeDefault,
                   m_TurbTypeDefault,
                   m_TurbTypeDefault,
                   vsp::CF_TURB_SCHULTZ_GRUNOW_SCHOENHERR,
                   m_TurbTypeDefault,
                   m_TurbTypeDefault,
                   vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG,
                   m_TurbTypeDefault };
    m_TurbAlternateList.insert( m_TurbAlternateList.begin(), tmp2, &tmp2[ sizeof( tmp2 ) / sizeof( *tmp2 ) ] );

}

void ParasiteDragMgrSingleton::Renew()
{
    // Every Parm, including the excrescence type and value this used to set by hand.
    ResetToInitVals();

    m_TableRowVec.clear();
    m_ExcresRowVec.clear();

    SetDegenGeomVec( vector < DegenGeom > () );
    m_TagWetAreaMap.clear();

    SetDefaultStruct();

    m_FileName = "ParasiteDragBuildUp.csv";
    m_LamCfEqnName = "Blasius";
    m_TurbCfEqnName = "Blasius Power Law";
    m_RefGeomID = "";
    m_ModeID = "";

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
    m_DefaultStruct.Grouped = false;
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

//==== A disk, or a surface CFD sees through (a prop disk, a gear's ground plane), has no skin ====//
static bool NoWettedArea( Geom* geom, int isurf )
{
    if ( geom->GetSurfType( isurf ) == vsp::DISK_SURF )
    {
        return true;
    }
    const VspSurf* surf = geom->GetSurfPtr( isurf );
    return surf && surf->GetSurfCfdType() == vsp::CFD_TRANSPARENT;
}

//==== The first surface with wetted area, whose DegenGeom gives a Geom its references ====//
static int FirstAreaSurf( Geom* geom )
{
    for ( int j = 0; j < geom->GetNumTotalSurfs(); j++ )
    {
        if ( !NoWettedArea( geom, j ) )
        {
            return j;
        }
    }
    return 0;
}

void ParasiteDragMgrSingleton::LoadMainTableUserInputs()
{
    Geom* geom;
    char str[256];
    string lastGeomID = "";
    string lastSSID = "";
    int igeom = 0;
    int issmaster = 0;
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
            // A Clone may use its original's drag settings
            Geom* dragGeom = geom->GetParasiteDragGeom();

            if ( geom->GetID().compare( lastGeomID ) != 0 )
            {
                // Create Master Geom Row, typed by the surface its DegenGeom comes from
                int iref = FirstAreaSurf( geom );
                igeom = m_geo_masterRow.size();
                m_geo_sourceRow.push_back( igeom );
                m_geo_masterRow.push_back( true );
                m_geo_name.push_back( geom->GetName() );
                m_geo_groupedAncestorGen.push_back( geom->m_GroupedAncestorGen() );
                m_geo_percLam.push_back( dragGeom->m_PercLam() );
                m_geo_ffIn.push_back( dragGeom->m_FFUser() );
                m_geo_Q.push_back( dragGeom->m_Q() );
                m_geo_Roughness.push_back( dragGeom->m_Roughness() );
                m_geo_TeTwRatio.push_back( dragGeom->m_TeTwRatio() );
                m_geo_TawTwRatio.push_back( dragGeom->m_TawTwRatio() );
                m_geo_surfNum.push_back( 0 );
                m_geo_expandedList.push_back( geom->m_ExpandedListFlag() );

                if ( geom->GetBehaviorType() == CUSTOM_GEOM_TYPE )
                {
                    if ( geom->GetSurfType( iref ) == vsp::NORMAL_SURF )
                    {
                        snprintf( str, sizeof( str ),  "[B] %s", geom->GetName().c_str() );
                    }
                    else
                    {
                        snprintf( str, sizeof( str ),  "[W] %s", geom->GetName().c_str() );
                    }
                }
                else
                {
                    snprintf( str, sizeof( str ),  "%s", geom->GetName().c_str() );
                }

                m_geo_shapeType.push_back( geom->GetSurfType( iref ) ); // Form Factor Shape Type
                m_geo_noArea.push_back( false );

                if ( geom->GetSurfType( iref ) == vsp::NORMAL_SURF )
                {
                    m_geo_ffType.push_back( dragGeom->m_FFBodyEqnType() );
                }
                else
                {
                    m_geo_ffType.push_back( dragGeom->m_FFWingEqnType() );
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
                        m_geo_sourceRow.push_back( igeom );
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
                        snprintf( str, sizeof( str ),  "%s_%d", geom->GetName().c_str(), j );
                    }
                    else
                    {
                        if ( geom->GetBehaviorType() == CUSTOM_GEOM_TYPE )
                        {
                            if ( geom->GetSurfType(j) == vsp::NORMAL_SURF )
                            {
                                snprintf( str, sizeof( str ),  "[B] %s", geom->GetName().c_str() );
                            }
                            else
                            {
                                snprintf( str, sizeof( str ),  "[W] %s", geom->GetName().c_str() );
                            }
                            m_geo_surfNum.push_back( j );
                        }
                        else
                        {
                            snprintf( str, sizeof( str ),  "%s_%i", geom->GetName().c_str(), j );
                            m_geo_surfNum.push_back( j );
                        }
                        m_geo_sourceRow.push_back( igeom );
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
                    m_geo_noArea.push_back( NoWettedArea( geom, j ) );

                    if ( geom->GetSurfType(j) == vsp::NORMAL_SURF )
                    {
                        m_geo_ffType.push_back( dragGeom->m_FFBodyEqnType() );
                    }
                    else
                    {
                        m_geo_ffType.push_back( dragGeom->m_FFWingEqnType() );
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
                            issmaster = m_geo_masterRow.size();
                            m_geo_sourceRow.push_back( issmaster );
                            m_geo_masterRow.push_back( true );
                            m_geo_name.push_back( surf->GetName() );
                            m_geo_percLam.push_back( surf->m_PercLam() );
                            m_geo_ffIn.push_back( surf->m_FFUser() );
                            m_geo_Q.push_back( surf->m_Q() );

                            // A subsurface's roughness defaults to -1, unset: use its Geom's
                            if ( surf->m_Roughness() < 0 )
                            {
                                m_geo_Roughness.push_back( dragGeom->m_Roughness() );
                            }
                            else
                            {
                                m_geo_Roughness.push_back( surf->m_Roughness() );
                            }
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
                            // A Separate subsurface's surfaces follow its row, any other its Geom's
                            int isrc = igeom;
                            if ( surf->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT )
                            {
                                isrc = issmaster;
                            }
                            m_geo_sourceRow.push_back( isrc );
                            m_geo_masterRow.push_back( false );
                            m_geo_name.push_back( surf->GetName() );
                            m_geo_percLam.push_back( m_geo_percLam[isrc] );
                            m_geo_ffIn.push_back( m_geo_ffIn[isrc] );
                            m_geo_Q.push_back( m_geo_Q[isrc] );
                            m_geo_Roughness.push_back( m_geo_Roughness[isrc] );
                            m_geo_TeTwRatio.push_back( m_geo_TeTwRatio[isrc] );
                            m_geo_TawTwRatio.push_back( m_geo_TawTwRatio[isrc] );

                            if ( isrc == igeom )
                            {
                                m_geo_ffType.push_back( m_geo_ffType[igeom] );
                            }
                            else if ( geom->GetSurfType(k) == vsp::NORMAL_SURF )
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
                        m_geo_noArea.push_back( NoWettedArea( geom, k ) );
                        m_geo_expandedList.push_back( false );
                        m_geo_geomID.push_back( geom->GetID() );
                        m_geo_subsurfID.push_back( surf->GetID() );

                        if ( lastSSID.compare( surf->GetID() ) != 0 && surf->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT )
                        {
                            snprintf( str, sizeof( str ),  "[ss] %s", surf->GetName().c_str() );
                            m_geo_label.push_back( str );
                            lastSSID = surf->GetID();
                            --k;
                        }
                        else
                        {
                            snprintf( str, sizeof( str ),  "[ss] %s_%i", surf->GetName().c_str(), k );
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

    int set = m_SetChoice();

    if ( m_UseMode() )
    {
        Mode *m = ModeMgr.GetMode( m_ModeID );
        if ( m )
        {
            m->ApplySettings();
            set = m->m_NormalSet();
        }
    }

    if ( veh && ( m_RecomputeGeom || m_DegenGeomVec.empty() ) )
    {
        veh->ClearDegenGeom();

        ClearInputVectors();
        ClearOutputVectors();

        // Don't pass mode parameters here as they've already been applied to 'set'.
        veh->CreateDegenGeom( set, /* useMode */ false, /* modeID */ "" );
        ComputeWetAreas( set );

        // First Assignment of DegenGeomVec, Will Carry Through to Rest of Calculate_X
        SetDegenGeomVec( veh->GetDegenGeomVec() );
    }
}

// Runs CompGeom on the set and keeps a copy of its wetted area by tag
void ParasiteDragMgrSingleton::ComputeWetAreas( int set )
{
    m_TagWetAreaMap.clear();

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    string before_id = ResultsMgr.FindLatestResultsID( "Comp_Geom" );
    // Without hiding the set, so the model's Shown and NotShown sets are left as they were
    string meshID = veh->CompGeomAndFlatten( set, 0, 1, vsp::SET_NONE, false );
    veh->DeleteGeomVec( { meshID } );

    // An empty CompGeom makes no results; an older one is not this model's
    string res_id = ResultsMgr.FindLatestResultsID( "Comp_Geom" );
    if ( res_id == before_id )
    {
        return;
    }

    Results* res = ResultsMgr.FindResultsPtr( res_id );
    if ( !res )
    {
        return;
    }

    NameValData* tag_nvd = res->FindPtr( "Tag_ID" );
    NameValData* area_nvd = res->FindPtr( "Tag_Wet_Area" );
    if ( !tag_nvd || !area_nvd )
    {
        return;
    }

    vector < string > tagvec = tag_nvd->GetStringData();
    for ( int i = 0; i < ( int )tagvec.size(); i++ )
    {
        m_TagWetAreaMap[ tagvec[i] ] = area_nvd->GetDouble( i );
    }
}

double ParasiteDragMgrSingleton::FindWetArea( const string & tag )
{
    map < string, double >::const_iterator it = m_TagWetAreaMap.find( tag );
    if ( it == m_TagWetAreaMap.end() )
    {
        return 0;
    }
    return it->second;
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
            m_RowSize += CalcGeomRowSize( geom ) + 1; // +1 for Master Geom Row that keeps track of totals
        }
    }
    return m_RowSize;
}

// Rows LoadMainTableUserInputs makes for a Geom, less its master row
int ParasiteDragMgrSingleton::CalcGeomRowSize( Geom* geom )
{
    int nsurf = geom->GetNumTotalSurfs();
    int nrow = nsurf;

    // One row per surface for each subsurface
    for ( size_t j = 0; j < geom->GetSubSurfVec().size(); ++j )
    {
        nrow += nsurf;

        // Subsurfaces are essentially separate geometries and as such get their own master row
        if ( geom->GetSubSurfVec()[j]->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT )
        {
            ++nrow;
        }
    }
    return nrow;
}

void ParasiteDragMgrSingleton::Calculate_Swet()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    char str[256];

    if ( !veh )
    {
        return;
    }

    // A region inside several subsurfaces has its own tag listing them all; one of them takes its
    // area: a zero drag one, else the first Separate one, else the first.
    map < string, double > overlap_area;
    for ( map < string, double >::const_iterator it = m_TagWetAreaMap.begin(); it != m_TagWetAreaMap.end(); ++it )
    {
        vector < string > parts = StringUtil::csv_to_vecstr( it->first.c_str() );
        if ( parts.size() > 2 )
        {
            Geom* geom = veh->FindGeom( parts[0].substr( 0, parts[0].find( "_Surf" ) ) );
            if ( geom )
            {
                string zero, separate, first;
                vector < SubSurface* > ssvec = geom->GetSubSurfVec();
                for ( size_t j = 0; j < ssvec.size(); j++ )
                {
                    if ( std::find( parts.begin() + 1, parts.end(), ssvec[j]->GetID() ) == parts.end() )
                    {
                        continue;
                    }
                    if ( first.empty() )
                    {
                        first = ssvec[j]->GetID();
                    }
                    if ( separate.empty() && ssvec[j]->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT )
                    {
                        separate = ssvec[j]->GetID();
                    }
                    if ( zero.empty() && ssvec[j]->m_IncludeType() == vsp::SS_INC_ZERO_DRAG )
                    {
                        zero = ssvec[j]->GetID();
                    }
                }

                string owner = first;
                if ( !zero.empty() )
                {
                    owner = zero;
                }
                else if ( !separate.empty() )
                {
                    owner = separate;
                }
                overlap_area[ parts[0] + "," + owner ] += it->second;
            }
        }
    }

    for ( int i = 0; i < m_RowSize; ++i )
    {
        if ( !m_DegenGeomVec.empty() )
        {
            Geom* geom = veh->FindGeom( m_geo_geomID[i] );
            if ( geom && !m_geo_masterRow[i] )
            {
                // If DegenGeom Exists Pull Swet
                if ( m_geo_subsurfID[i].compare( "" ) == 0 )
                {
                    snprintf( str, sizeof( str ),  "%s_Surf%i", geom->GetID().c_str(), m_geo_surfNum[i] );
                }
                else
                {
                    snprintf( str, sizeof( str ),  "%s_Surf%i,%s", geom->GetID().c_str(), m_geo_surfNum[i],
                         m_geo_subsurfID[i].c_str() );
                }
                double area = 0;
                if ( !m_geo_noArea[i] )
                {
                    area = FindWetArea( str );
                    map < string, double >::const_iterator it = overlap_area.find( str );
                    if ( it != overlap_area.end() )
                    {
                        area += it->second;
                    }
                }
                m_geo_swet.push_back( area );
            }
            else
            {
                m_geo_swet.push_back( 0.0 ); // Master Geom Swet is 0 until updated by UpdateWettedAreaTotals()
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
    for ( int i = 0; i < m_RowSize; ++i )
    {
        double lref = -1;
        if ( m_geo_masterRow[i] )
        {
            int idegen = FindDegenIndex( m_geo_geomID[i] );
            if ( idegen >= 0 )
            {
                lref = CalcReferenceLength( idegen, m_geo_geomID[i] );
            }
        }
        else if ( !m_geo_lref.empty() )
        {
            lref = m_geo_lref.back();
        }
        m_geo_lref.push_back( lref );
    }
}

double ParasiteDragMgrSingleton::CalcReferenceLength( int index, const string & geom_id )
{
    double lref = 0;
    if ( m_DegenGeomVec[index].getType() == DegenGeom::BODY_TYPE )
    {
        lref = CalcReferenceBodyLength( index );

        if ( lref <= 1e-6 )
        {
            lref = CalcReferenceChord( index, geom_id );
        }
    }
    else if ( m_DegenGeomVec[index].getType() == DegenGeom::SURFACE_TYPE )
    {
        lref = CalcReferenceChord( index, geom_id );

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
    if ( m_DegenStick.empty() || m_DegenStick[0].xle.empty() )
    {
        return 0;
    }
    delta_x = abs( m_DegenStick[0].xle.front().x() - m_DegenStick[0].xle.back().x() );
    delta_y = abs( m_DegenStick[0].xle.front().y() - m_DegenStick[0].xle.back().y() );
    delta_z = abs( m_DegenStick[0].xle.front().z() - m_DegenStick[0].xle.back().z() );
    lref = sqrt( pow( delta_x, 2.0 ) + pow( delta_y, 2.0 ) + pow( delta_z, 2.0 ) );

    return lref;
}

// Use weighted average to approximate reference chord
double ParasiteDragMgrSingleton::CalcReferenceChord( int index, const string & geom_id )
{
    // A wing-like Geom computes its own mean aerodynamic chord
    Geom* geom = VehicleMgr.GetVehicle()->FindGeom( geom_id );
    if ( geom )
    {
        double mac = geom->ComputeMAC();
        if ( mac > 0 )
        {
            return mac;
        }
    }

    // Otherwise integral( c^2 ) / integral( c ) along the span, with the chord linear across
    // each strip
    vector <DegenStick> m_DegenStick = m_DegenGeomVec[index].getDegenSticks();
    double totalArea = 0, chordSqSum = 0;
    if ( m_DegenStick.empty() )
    {
        return 0;
    }
    for ( size_t j = 0; j < m_DegenStick[0].areaTop.size(); ++j )
    {
        double c0 = m_DegenStick[0].chord[j];
        double c1 = m_DegenStick[0].chord[j + 1];

        // Strip width across the chord, so sweep does not stretch it
        vec3d dle = m_DegenStick[0].xle[j + 1] - m_DegenStick[0].xle[j];
        vec3d cdir = ( m_DegenStick[0].xte[j] - m_DegenStick[0].xle[j] ) + ( m_DegenStick[0].xte[j + 1] - m_DegenStick[0].xle[j + 1] );
        cdir.normalize();
        double width = ( dle - cdir * dot( dle, cdir ) ).mag();

        totalArea += width * 0.5 * ( c0 + c1 );
        chordSqSum += width * ( c0 * c0 + c0 * c1 + c1 * c1 ) / 3.0;
    }

    if ( totalArea <= 0 )
    {
        return 0;
    }
    return chordSqSum / totalArea;
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
    for ( int i = 0; i < m_RowSize; ++i )
    {
        if ( !m_DegenGeomVec.empty() )
        {
            // If DegenGeom Exists Calculate Cf
            if ( m_geo_subsurfID[i].compare( "" ) == 0 ||
                    VehicleMgr.GetVehicle()->FindGeom( m_geo_geomID[i] )->GetSubSurf( m_geo_subsurfID[i] )->m_IncludeType() == vsp::SS_INC_SEPARATE_TREATMENT )
            {
                // Will calculate full turbulence
                m_geo_cf.push_back( CalcPartialTurbulence( m_geo_percLam[i], m_geo_Re[i], m_geo_lref[i], m_ReqL(),
                   m_geo_Roughness[i], m_geo_TawTwRatio[i], m_geo_TeTwRatio[i] ) );

            }
            else
            {
                m_geo_cf.push_back( m_geo_cf[ m_geo_sourceRow[i] ] );
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

        // At low Re the turbulent fits fall below the laminar Cf, where they no longer hold
        double CffullTurb = max( CalcTurbCf( re, lref, m_TurbCfEqnType(), roughness, m_SpecificHeatRatio(), tawtwrat, tetwrat ),
                                 CalcLamCf( re, m_LamCfEqnType() ) );

        double LamPercRefLen = LamPerc * lref;

        double ReLam = reqL * LamPercRefLen;

        m_TurbCfEqnName = AssignTurbCfEqnName( m_TurbCfEqnType() );
        m_LamCfEqnName = AssignLamCfEqnName( m_LamCfEqnType() );

        cf = CffullTurb;
        if ( LamPerc >= 1.0 )
        {
            cf = CalcLamCf( re, m_LamCfEqnType() );
        }
        else if ( ReLam >= 1.0e3 )
        {
            // Below this the turbulent fits fail; the laminar Cf is above them there, so the
            // laminar run's correction is zero either side
            double CfpartLam = CalcLamCf( ReLam, m_LamCfEqnType() );
            double CfpartTurb = max( CalcTurbCf( ReLam, LamPercRefLen, m_TurbCfEqnType(), roughness, m_SpecificHeatRatio(), tawtwrat, tetwrat ),
                                     CfpartLam );

            cf = CffullTurb - ( CfpartTurb * LamPerc ) + ( CfpartLam * LamPerc );
        }
    }
    return cf;
}

void ParasiteDragMgrSingleton::Calculate_fineRat_and_toc()
{
    for ( int i = 0; i < m_RowSize; ++i )
    {
        double finerat = -1;
        if ( m_geo_masterRow[i] )
        {
            int idegen = FindDegenIndex( m_geo_geomID[i] );
            if ( idegen >= 0 )
            {
                finerat = CalculateFinessRatioAndTOC( idegen, i );
            }
        }
        else if ( !m_geo_fineRat_or_toc.empty() )
        {
            finerat = m_geo_fineRat_or_toc.back();
        }
        m_geo_fineRat_or_toc.push_back( finerat );
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

    if ( m_DegenGeomVec[isurf].getType() == DegenGeom::SURFACE_TYPE && !degenSticks[0].toc2.empty() )
    {
        // Wing Type: planform area weighted t/c, found on the section curve where there is one,
        // with t/c and chord linear across each strip
        const DegenStick &stick = degenSticks[0];
        double area_sum = 0;
        double toc_sum = 0;
        for ( size_t j = 0; j + 1 < stick.toc2.size() && j + 1 < stick.chord.size(); j++ )
        {
            vec3d dle = stick.xle[j + 1] - stick.xle[j];
            vec3d cdir = ( stick.xte[j] - stick.xle[j] ) + ( stick.xte[j + 1] - stick.xle[j + 1] );
            cdir.normalize();
            double width = ( dle - cdir * dot( dle, cdir ) ).mag();

            double c0 = stick.chord[j];
            double c1 = stick.chord[j + 1];
            double t0 = stick.toc2[j];
            double t1 = stick.toc2[j + 1];

            area_sum += width * 0.5 * ( c0 + c1 );
            toc_sum += width * ( 2.0 * c0 * t0 + c0 * t1 + c1 * t0 + 2.0 * c1 * t1 ) / 6.0;
        }

        if ( area_sum > 0 )
        {
            finerat = toc_sum / area_sum;
        }
        else
        {
            it = max_element( stick.toc2.begin(), stick.toc2.end() );
            finerat = *it;
        }
    }
    else if ( m_DegenGeomVec[isurf].getType() == DegenGeom::BODY_TYPE && !degenSticks[0].sectarea.empty() )
    {
        it = max_element( degenSticks[0].sectarea.begin(), degenSticks[0].sectarea.end() );
        max_xsecarea = *it;

        // Use Max X-Sectional Area to find "Nominal" Diameter
        dia = 2.0 * sqrt( max_xsecarea / M_PI );

        finerat = m_geo_lref[irow] / dia;
    }

    return finerat;
}

void ParasiteDragMgrSingleton::Calculate_FF()
{
    for ( int i = 0; i < m_RowSize; ++i )
    {
        if ( m_geo_masterRow[i] )
        {
            int idegen = FindDegenIndex( m_geo_geomID[i] );
            if ( idegen >= 0 )
            {
                m_geo_ffOut.push_back( CalculateFormFactor( idegen, i ) );

                if ( m_DegenGeomVec[idegen].getType() == DegenGeom::SURFACE_TYPE )
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
                m_geo_ffOut.push_back( -1 );
                m_geo_ffName.push_back( "" );
            }
        }
        else if ( !m_geo_ffOut.empty() )
        {
            m_geo_ffOut.push_back( m_geo_ffOut[ m_geo_sourceRow[i] ] );
            if ( m_geo_ffType[i] == vsp::FF_W_JENKINSON_TAIL )
            {
                m_geo_Q[i] = 1.2;
            }
            m_geo_ffName.push_back( m_geo_ffName[ m_geo_sourceRow[i] ] );
        }
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
    double formfactor = 1.0;

    // Grab Degen Sticks for Appropriate Geom
    vector <DegenStick> degenSticks = m_DegenGeomVec[isurf].getDegenSticks();

    if ( m_DegenGeomVec[isurf].getType() == DegenGeom::SURFACE_TYPE )
    {
        // Wing Type

        toc = m_geo_fineRat_or_toc[irow];

        Calculate_AvgSweep( degenSticks );

        // Chord weighted location of maximum thickness, found on the section curve where there is one
        double toc_loc = 0;
        double chord_sum = 0;
        for ( size_t j = 0; j < degenSticks[0].tLoc2.size() && j < degenSticks[0].chord.size(); j++ )
        {
            toc_loc += degenSticks[0].chord[j] * degenSticks[0].tLoc2[j];
            chord_sum += degenSticks[0].chord[j];
        }
        if ( chord_sum > 0 )
        {
            toc_loc /= chord_sum;
        }

        formfactor = CalcFFWing( toc, m_geo_ffType[irow], toc_loc, m_Sweep25, m_Sweep50 );
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

void ParasiteDragMgrSingleton::Calculate_AvgSweep( const vector<DegenStick> &degenSticks )
{
    // Find Quarter Chord Using Derived Eqn from Geometry
    double width, secSweep25, secSweep50, secArea, weighted25Sum = 0, weighted50Sum = 0, totalArea = 0;
    m_Sweep25 = 0;
    m_Sweep50 = 0;
    if ( degenSticks.empty() )
    {
        return;
    }
    for ( int j = 0; j < degenSticks[0].areaTop.size(); j++ )
    {
        width = degenSticks[0].areaTop[j] /
                ( ( degenSticks[0].perimTop[j] + degenSticks[0].perimTop[j + 1] ) / 2.0 );

        // Section Quarter Chord Sweep
        secSweep25 = atan( tan( degenSticks[0].sweeple[j] * M_PI / 180.0 ) +
                           ( 0.25 * ( ( degenSticks[0].chord[j + 1] - degenSticks[0].chord[j] ) / width ) ) ) *
                     180.0 / M_PI;

        // Section Half Chord Sweep
        secSweep50 = atan( tan( degenSticks[0].sweeple[j] * M_PI / 180.0 ) +
                           ( 0.50 * ( ( degenSticks[0].chord[j + 1] - degenSticks[0].chord[j] ) / width ) ) ) *
                     180.0 / M_PI;

        // Section Area
        secArea = 0.5 * ( degenSticks[0].chord[j] + degenSticks[0].chord[j + 1] ) * width;

        // Add Weighted Value to Weighted Sum
        weighted25Sum += secArea * secSweep25;
        weighted50Sum += secArea * secSweep50;

        // Continue to sum up Total Area
        totalArea += secArea;
    }

    if ( totalArea <= 0 )
    {
        return;
    }

    // Calculate Sweep @ c/4 & c/2
    m_Sweep25 = weighted25Sum / totalArea * M_PI / 180.0; // Into Radians
    m_Sweep50 = weighted50Sum / totalArea * M_PI / 180.0; // Into Radians
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
                if ( std::isfinite( m_geo_f[i] ) )
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

    // Every per-row vector is sized from these
    m_RowSize = m_geo_masterRow.size();

    for ( int i = 0; i < m_RowSize; i++ )
    {
        m_geo_groupRow.push_back( GroupMasterRow( i ) );
    }

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
        tempStruct.Grouped = m_geo_groupRow[i] >= 0;
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
        ReportNoDragRows();
        ReportRefWing();

        UpdateExportLabels();

        return ExportToCSV();
    }
    return "";
}

//==== Calculate_CD counts a component whose f is not finite as no drag; say so ====//
// Not from Calculate_ALL: the screen runs that on every update, and an error is a dialog there.
void ParasiteDragMgrSingleton::ReportNoDragRows()
{
    string invalid;
    for ( int i = 0; i < m_RowSize; i++ )
    {
        if ( m_geo_masterRow[i] && IsNotZeroLineItem( i ) && !std::isfinite( m_geo_f[i] ) )
        {
            invalid += " " + m_geo_label[i];
        }
    }
    if ( !invalid.empty() )
    {
        MessageData errMsgData;
        errMsgData.m_String = "Error";
        errMsgData.m_IntVec.push_back( vsp::VSP_INVALID_CF_EQN );
        errMsgData.m_StringVec.push_back( "Error:  No drag for" + invalid +
                                          "; a form factor or Cf equation is not valid at these conditions." );
        MessageMgr::getInstance().SendAll( errMsgData );
    }
}

//==== A reference wing that is not a wing leaves S_ref as it was; say so ====//
void ParasiteDragMgrSingleton::ReportRefWing()
{
    if ( m_RefFlag() != vsp::COMPONENT_REF )
    {
        return;
    }

    Geom* refgeom = VehicleMgr.GetVehicle()->FindGeom( m_RefGeomID );
    if ( !refgeom || refgeom->GetBehaviorType() != MS_WING_GEOM_TYPE )
    {
        MessageData errMsgData;
        errMsgData.m_String = "Error";
        errMsgData.m_IntVec.push_back( vsp::VSP_INVALID_ID );
        errMsgData.m_StringVec.push_back( "Error:  The reference wing '" + m_RefGeomID + "' is not a wing; S_ref was not updated." );
        MessageMgr::getInstance().SendAll( errMsgData );
    }
}

void ParasiteDragMgrSingleton::OverwritePropertiesFromAncestorGeom()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        for ( size_t i = 0; i < m_RowSize; ++i )
        {
            if ( m_geo_subsurfID[i].compare( "" ) == 0 )
            {
                int j = m_geo_groupRow[i];
                if ( j >= 0 )
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
    double CfGuess, f, heightRatio;
    double lnterm;
    double r = 0.89; // Recovery Factor
    double n = 0.67; // Viscosity Power-Law Exponent

    if (ReyIn == 0)
    {
        return CfOut;
    }

    eli::mutil::nls::newton_raphson_method < double > nrm;

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
        if ( nrm.find_root( CfOut, sfun, sfunprm, 0.0 ) != eli::mutil::nls::newton_raphson_method < double >::converged )
        {
            CfOut = CfGuess;
        }
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
        if ( nrm.find_root( CfOut, ksfun, ksfunprm, 0.0 ) != eli::mutil::nls::newton_raphson_method < double >::converged )
        {
            CfOut = CfGuess;
        }
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
        if ( roughness_h > 0 )
        {
            heightRatio = ref_leng / roughness_h;
            CfOut = pow( ( 1.89 + ( 1.62 * log10( heightRatio ) ) ), -2.5 );    // White 6-84b, Schlicting 21.38
        }

        // Fully rough friction; a smoother plate has at least the smooth turbulent friction
        CfOut = max( CfOut, CalcTurbCf( ReyIn, ref_leng, m_TurbTypeDefault, roughness_h, gamma, taw_tw_ratio, te_tw_ratio ) );
        break;

    case vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION:
        if ( roughness_h > 0 )
        {
            heightRatio = ref_leng / roughness_h;
            CfOut = pow( ( 1.89 + ( 1.62 * log10( heightRatio ) ) ), -2.5 );
        }
        CfOut = max( CfOut, CalcTurbCf( ReyIn, ref_leng, m_TurbTypeDefault, roughness_h, gamma, taw_tw_ratio, te_tw_ratio ) );

        // The compressibility correction applies to the smooth floor as well
        CfOut /= pow( ( 1.0 + ( ( gamma - 1.0 ) / 2.0 ) * m_Mach() * m_Mach() ), 0.467 );
        break;

    case vsp::DO_NOT_USE_CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH:
        f = ( 1 + ( 0.22 * r * ( ( gamma - 1.0 ) / 2.0 ) *
                    m_Mach() * m_Mach() * te_tw_ratio ) ) /
            ( 1 + ( 0.3 * ( taw_tw_ratio - 1.0 ) ) );

        lnterm = log( 0.056 * f * pow( te_tw_ratio, 1.0 + n ) * ReyIn );
        CfOut = ( 0.451 * f * f * te_tw_ratio ) / ( lnterm * lnterm );

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
        eqn_name = "Schlichting Avg Roughness w Flow Correction";
        break;

    case vsp::DO_NOT_USE_CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH:
        eqn_name = "(Do Not Use) White-Christoph w Heat Transfer";
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
        double toc_loc = 0, double sweep25 = 0, double sweep50 = 0 )
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
        // No supervelocity from thickness once the normal Mach reaches 1
        if ( pow( mach, 2.0 ) * pow( cos( sweep25 ), 2.0 ) >= 1.0 )
        {
            ff = 1.0;
            break;
        }
        double part1A, part1B, part2A, part2B;
        part1A = ( 2.2 * pow( cos( sweep25 ), 2.0 ) * toc );
        part1B = ( sqrt( 1.0 - ( pow( mach, 2.0 ) * pow( cos( sweep25 ), 2.0 ) ) ) );
        part2A = ( 1.21 * pow( cos( sweep25 ), 2.0 ) * ( 1.0 + 5.0 * pow( cos( sweep25 ), 2.0 ) ) * pow( toc, 2.0 ) );
        part2B = ( 2.0 * ( 1.0 - ( pow( mach, 2.0 ) * pow( cos( sweep25 ), 2.0 ) ) ) );
        ff = 1.0 + ( part1A / part1B ) + ( part2A / part2B );
        break;

    case vsp::FF_W_TORENBEEK:
        ff = 1.0 + 2.7 * toc + 100.0 * pow( toc, 4.0 );
        break;

    case vsp::FF_W_DATCOM:
        double L, x, RLS_Low, RLS_High;

        // L from the chordwise location of maximum thickness
        if ( toc_loc < 0.3 )
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

    int set = m_SetChoice();

    if ( m_UseMode() )
    {
        Mode *m = ModeMgr.GetMode( m_ModeID );
        if ( m )
        {
            // m->ApplySettings();
            set = m->m_NormalSet();
        }
    }

    if ( veh )
    {
        vector <string> geomVec = veh->GetGeomSet( set );

        m_PDGeomIDVec.clear();
        for ( int i = 0; i < geomVec.size(); i++ )
        {
            bool geom_skipped = true;
            Geom* geom = veh->FindGeom( geomVec[i] );
            if ( geom )
            {
                if ( geom->isSurfaceType() )
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
        snprintf( str, sizeof( str ),  "EXCRES_%zu", m_ExcresRowVec.size() );
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
    m_ExcresType.Set( excresType );
    SetExcresValueLimits( m_ExcresType() );
    m_ExcresValue.Set( excresVal );
    m_ExcresName = excresName;

    AddExcrescence();
}

void ParasiteDragMgrSingleton::DeleteExcrescence()
{
    if ( m_CurrentExcresIndex < 0 || m_CurrentExcresIndex >= ( int )m_ExcresRowVec.size() )
    {
        return;
    }

    m_ExcresRowVec.erase( m_ExcresRowVec.begin() + m_CurrentExcresIndex );

    if ( m_ExcresRowVec.empty() )
    {
        m_CurrentExcresIndex = -1;
        return;
    }

    if ( m_CurrentExcresIndex >= ( int )m_ExcresRowVec.size() )
    {
        m_CurrentExcresIndex = m_ExcresRowVec.size() - 1;
    }

    // m_ExcresValue still holds the deleted one's value; UpdateExcres would give it to this one
    UpdateCurrentExcresVal();
}

void ParasiteDragMgrSingleton::DeleteExcrescence( int index )
{
    if ( index < 0 || index >= ( int )m_ExcresRowVec.size() )
    {
        return;
    }

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
                    // Only a Geom's own master row takes the subsurfaces treated as its parent
                    bool geomMaster = m_geo_subsurfID[i].compare( "" ) == 0;
                    return ((geomMaster && m_geo_geomID[i].compare(m_geo_geomID[j]) == 0 &&
                             geom->GetSubSurf(m_geo_subsurfID[j])->m_IncludeType() == vsp::SS_INC_TREAT_AS_PARENT) ||
                            (geomMaster && (int)i == m_geo_groupRow[j] &&
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

    // A Geom's surface rows go to its own master row, and to the master row of the Geom it is
    // grouped into.  Only surface rows move area, so the result does not depend on row order.
    if ( i != j && m_geo_masterRow[i] && !m_geo_masterRow[j] && !m_geo_noArea[j] &&
         m_geo_subsurfID[j].compare( "" ) == 0 && m_geo_subsurfID[i].compare( "" ) == 0 )
    {
        return m_geo_geomID[i].compare( m_geo_geomID[j] ) == 0 || (int)i == m_geo_groupRow[j];
    }
    return false;
}

//==== Master row of the Geom whose drag takes row irow's Geom, or -1 ====//
// Follows groupings up to a Geom that is not grouped; stops at an ancestor not in the set or of
// another shape type, which leaves the Geom with its own drag.
int ParasiteDragMgrSingleton::GroupMasterRow( int irow )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return -1;
    }

    int target = -1;
    int imaster = GeomMasterRow( m_geo_geomID[irow] );

    // Bounded by the row count, so a ring of groupings ends
    for ( int n = 0; n < m_RowSize && imaster >= 0; n++ )
    {
        Geom* geom = veh->FindGeom( m_geo_geomID[imaster] );
        int gen = m_geo_groupedAncestorGen[imaster];
        if ( !geom || gen <= 0 )
        {
            break;
        }

        int ianc = GeomMasterRow( geom->GetAncestorID( gen ) );
        if ( ianc < 0 || ianc == imaster || m_geo_shapeType[ianc] != m_geo_shapeType[imaster] )
        {
            break;
        }

        target = ianc;
        imaster = ianc;
    }
    return target;
}

//==== Master row of a Geom, not of one of its subsurfaces, or -1 ====//
int ParasiteDragMgrSingleton::GeomMasterRow( const string &geom_id )
{
    for ( int i = 0; i < m_RowSize; i++ )
    {
        if ( m_geo_masterRow[i] && m_geo_subsurfID[i].compare( "" ) == 0 && m_geo_geomID[i].compare( geom_id ) == 0 )
        {
            return i;
        }
    }
    return -1;
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
        // Pull from existing geometry; with none chosen, the first wing, as the screen does
        Vehicle* veh = VehicleMgr.GetVehicle();
        if ( m_RefGeomID.empty() )
        {
            vector < string > geom_ids = veh->GetGeomVec();
            for ( size_t i = 0; i < geom_ids.size(); i++ )
            {
                Geom* geom = veh->FindGeom( geom_ids[i] );
                if ( geom && geom->GetBehaviorType() == MS_WING_GEOM_TYPE )
                {
                    m_RefGeomID = geom_ids[i];
                    break;
                }
            }
        }

        Geom* refgeom = veh->FindGeom( m_RefGeomID );

        if ( refgeom )
        {
            WingRole* wing = Geom::CastTo< WingRole >( refgeom );
            if ( wing )
            {
                m_Sref.Set( wing->GetWingTotalArea() );
                m_Sref.Deactivate();
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

        // The form factors read the Mach from the atmosphere
        m_Atmos.SetMach( m_Mach() );
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

    // The master rows carry the geometry's drag, as in GetGeometryCD; the other rows show their
    // own share and are not summed.
    for ( int i = 0; i < m_geo_CD.size(); i++ )
    {
        double perc = 0;
        if ( !m_DegenGeomVec.empty() && totalCD0 > 0 )
        {
            perc = m_geo_CD[i] / totalCD0;
        }
        m_geo_percTotalCD.push_back( perc );

        if ( !m_DegenGeomVec.empty() && m_geo_masterRow[i] && m_geo_CD[i] > 0.0 )
        {
            percTotal += perc;
            ftotal += m_geo_f[i];
        }
    }

    m_GeomfTotal = ftotal;
    m_GeomPercTotal = percTotal;

    ftotal = 0;
    percTotal = 0;
    for ( size_t i = 0; i < m_ExcresRowVec.size(); ++i )
    {
        if ( !m_DegenGeomVec.empty() && totalCD0 > 0 )
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

    if ( m_TurbCfEqnType() == vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION )
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
        }

        if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_CD )
        {
            m_ExcresRowVec[i].Amount = m_ExcresRowVec[i].Input;
        }
        else if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_COUNT )
        {
            m_ExcresRowVec[i].Amount = m_ExcresRowVec[i].Input / 10000.0;
        }
        else if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_PERCENT_GEOM )
        {
            m_ExcresRowVec[i].Amount = CalcPercentageGeomCD( m_ExcresRowVec[i].Input );
        }
        else if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_DRAGAREA )
        {
            m_ExcresRowVec[i].Amount = CalcDragAreaCD( m_ExcresRowVec[i].Input );
        }
        else if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_MARGIN )
        {
            m_ExcresRowVec[i].Amount = 0;
        }
    }

    // The margin is a share of the total, so it follows every other excrescence
    for ( size_t i = 0; i < m_ExcresRowVec.size(); ++i )
    {
        if ( m_ExcresRowVec[i].Type == vsp::EXCRESCENCE_MARGIN )
        {
            m_ExcresRowVec[i].Amount = CalcPercentageTotalCD( m_ExcresRowVec[i].Input );
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
    SetExcresValueLimits( m_ExcresType() );
    m_ExcresValue.Set( m_ExcresRowVec[m_CurrentExcresIndex].Input );
}

void ParasiteDragMgrSingleton::SetExcresValueLimits( int type )
{
    if ( type == vsp::EXCRESCENCE_CD )
    {
        m_ExcresValue.SetLowerUpperLimits( 0.0, 10.0 );
    }
    else if ( type == vsp::EXCRESCENCE_COUNT )
    {
        m_ExcresValue.SetLowerUpperLimits( 0.0, 100000.0 );
    }
    else if ( type == vsp::EXCRESCENCE_PERCENT_GEOM )
    {
        m_ExcresValue.SetLowerUpperLimits( 0.0, 1000.0 );
    }
    else if ( type == vsp::EXCRESCENCE_MARGIN )
    {
        // A share of the total, so below 100%
        m_ExcresValue.SetLowerUpperLimits( 0.0, 99.0 );
    }
    else if ( type == vsp::EXCRESCENCE_DRAGAREA )
    {
        m_ExcresValue.SetLowerUpperLimits( 0.0, 10000.0 );
    }
}

string ParasiteDragMgrSingleton::ExportToCSV()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( !veh )
    {
        return string();
    }

    // Create Results
    Results* res = ResultsMgr.CreateResults( "Parasite_Drag", "Parasite drag buildup results." );

    if ( !res )
    {
        return string();
    }

    // Add Results to ResultsMgr
    UpdateExcres();
    UpdateExportLabels();

    res->Add( new NameValData( "Alt_Label", m_AltLabel, "Altitude label including units." ) );
    res->Add( new NameValData( "Vinf_Label", m_VinfLabel, "Airspeed label including units." ) );
    res->Add( new NameValData( "Sref_Label", m_SrefLabel, "Reference area label including units." ) );
    res->Add( new NameValData( "Temp_Label", m_TempLabel, "Temperature label including units." ) );
    res->Add( new NameValData( "Pres_Label", m_PresLabel, "Pressure label including units." ) );
    res->Add( new NameValData( "Rho_Label", m_RhoLabel, "Density label including units." ) );
    res->Add( new NameValData( "LamCfEqnName", m_LamCfEqnName, "Laminar skin friction coefficient equation name." ) );
    res->Add( new NameValData( "TurbCfEqnName", m_TurbCfEqnName, "Turbulent skin friction coefficient equation name." ) );
    res->Add( new NameValData( "Swet_Label", m_SwetLabel, "Wetted area label including units." ) );
    res->Add( new NameValData( "Lref_Label", m_LrefLabel, "Reference length label including units." ) );
    res->Add( new NameValData( "f_Label", m_fLabel, "Drag area label including units." ) );

    // Flow Condition
    res->Add( new NameValData( "FC_Mach", m_Mach(), "Flow condition Mach." ) );
    res->Add( new NameValData( "FC_Alt", m_Hinf.Get(), "Flow condition altitude." ) );
    res->Add( new NameValData( "FC_Vinf", m_Vinf.Get(), "Flow condition airspeed." ) );
    res->Add( new NameValData( "FC_Sref", m_Sref.Get(), "Reference area." ) );
    res->Add( new NameValData( "FC_dTemp", m_DeltaT.Get(), "Flow condition temperature deviation." ) );
    res->Add( new NameValData( "FC_Temp", m_Temp.Get(), "Flow condition temperature." ) );
    res->Add( new NameValData( "FC_Pres", m_Pres.Get(), "Flow condition pressure." ) );
    res->Add( new NameValData( "FC_Rho", m_Rho.Get(), "Flow condition density." ) );
    res->Add( new NameValData( "FreestreamPropChoice", m_FreestreamType.Get(), "Freestream property input mode choice." ) );
    res->Add( new NameValData( "Re_L", m_ReqL.Get(), "Reynolds number per unit length." ) );
    res->Add( new NameValData( "SpecificHeatRatio", m_SpecificHeatRatio.Get(), "Ratio of specific heats." ) );

    // Component Related
    if ( !m_ExportSubCompFlag() )
    {
        // Only create results for the primary components

        vector < string > new_ID_vec, new_geo_label, new_geo_ffName;
        vector < double > new_geo_swet, new_geo_lref, new_geo_Re, new_geo_percLam, new_geo_cf, new_geo_fineRatorToC,
            new_geo_ffIn, new_geo_ffOut, new_geo_Roughness, new_geo_TeTwRatio, new_geo_TawTwRatio, new_geo_Q,
            new_geo_f, new_geo_CD, new_geo_percTotalCD;
        vector <int> new_geo_ffType, new_geo_surfNum;

        // A component is a master row: a Geom, or a subsurface given separate treatment
        for ( size_t i = 0; i < m_geo_geomID.size(); i++ )
        {
            if ( m_geo_masterRow[i] )
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

        res->Add( new NameValData( "Num_Comp", num_comp, "Number of components." ) );
        res->Add( new NameValData( "Comp_ID", new_ID_vec, "Component IDs." ) );
        res->Add( new NameValData( "Comp_Label", new_geo_label, "Label." ) );
        res->Add( new NameValData( "Comp_Swet", new_geo_swet, "Wetted area." ) );
        res->Add( new NameValData( "Comp_Lref", new_geo_lref, "Reference length." ) );
        res->Add( new NameValData( "Comp_Re", new_geo_Re, "Reynolds number." ) );
        res->Add( new NameValData( "Comp_PercLam", new_geo_percLam, "Percent laminar flow." ) );
        res->Add( new NameValData( "Comp_Cf", new_geo_cf, "Skin friction coefficient." ) );
        res->Add( new NameValData( "Comp_FineRat", new_geo_fineRatorToC, "Fineness ratio or t/c." ) );
        res->Add( new NameValData( "Comp_FFEqn", new_geo_ffType, "Form factor equation." ) );
        res->Add( new NameValData( "Comp_FFEqnName", new_geo_ffName, "Form factor equation." ) );
        res->Add( new NameValData( "Comp_FFIn", new_geo_ffIn, "Form factor in." ) );
        res->Add( new NameValData( "Comp_FFOut", new_geo_ffOut, "Form factor out." ) );
        res->Add( new NameValData( "Comp_Roughness", new_geo_Roughness, "Surface roughness." ) );
        res->Add( new NameValData( "Comp_TeTwRatio", new_geo_TeTwRatio, "Te/Tw temperature ratio." ) );
        res->Add( new NameValData( "Comp_TawTwRatio", new_geo_TawTwRatio, "Taw/Tw temperature ratio." ) );
        res->Add( new NameValData( "Comp_Q", new_geo_Q, "Interference factor." ) );
        res->Add( new NameValData( "Comp_f", new_geo_f, "Drag area." ) );
        res->Add( new NameValData( "Comp_CD", new_geo_CD, "Drag coefficient." ) );
        res->Add( new NameValData( "Comp_PercTotalCD", new_geo_percTotalCD, "Percent of total CD." ) );
        res->Add( new NameValData( "Comp_SurfNum", new_geo_surfNum, "Geom surf number" ) );
    }
    else
    {
        res->Add( new NameValData( "Num_Comp", m_RowSize, "Number of components." ) );
        res->Add( new NameValData( "Comp_ID", m_geo_geomID, "GeomID." ) );
        res->Add( new NameValData( "Comp_Label", m_geo_label, "Label." ) );
        res->Add( new NameValData( "Comp_Swet", m_geo_swet, "Wetted area." ) );
        res->Add( new NameValData( "Comp_Lref", m_geo_lref, "Reference length." ) );
        res->Add( new NameValData( "Comp_Re", m_geo_Re, "Reynolds number." ) );
        res->Add( new NameValData( "Comp_PercLam", m_geo_percLam, "Percent laminar flow." ) );
        res->Add( new NameValData( "Comp_Cf", m_geo_cf, "Skin friction coefficient." ) );
        res->Add( new NameValData( "Comp_FineRat", m_geo_fineRat_or_toc, "Fineness ratio or t/c." ) );
        res->Add( new NameValData( "Comp_FFEqn", m_geo_ffType, "Form factor equation." ) );
        res->Add( new NameValData( "Comp_FFEqnName", m_geo_ffName, "Form factor equation." ) );
        res->Add( new NameValData( "Comp_FFIn", m_geo_ffIn, "Form factor in." ) );
        res->Add( new NameValData( "Comp_FFOut", m_geo_ffOut, "Form factor out." ) );
        res->Add( new NameValData( "Comp_Roughness", m_geo_Roughness, "Surface roughness." ) );
        res->Add( new NameValData( "Comp_TeTwRatio", m_geo_TeTwRatio, "Te/Tw temperature ratio." ) );
        res->Add( new NameValData( "Comp_TawTwRatio", m_geo_TawTwRatio, "Taw/Tw temperature ratio." ) );
        res->Add( new NameValData( "Comp_Q", m_geo_Q, "Interference factor." ) );
        res->Add( new NameValData( "Comp_f", m_geo_f, "Drag area." ) );
        res->Add( new NameValData( "Comp_CD", m_geo_CD, "Drag coefficient." ) );
        res->Add( new NameValData( "Comp_PercTotalCD", m_geo_percTotalCD, "Percent of total CD." ) );
        res->Add( new NameValData( "Comp_SurfNum", m_geo_surfNum, "Geom surf number." ) );
    }

    // Excres Related
    res->Add( new NameValData( "Num_Excres", ( int )m_ExcresRowVec.size(), "Number of excressence contributions." ) );
    res->Add( new NameValData( "Excres_Label", m_excres_Label, "Name." ) );
    res->Add( new NameValData( "Excres_Type", m_excres_Type, "Type." ) );
    res->Add( new NameValData( "Excres_Input", m_excres_Input, "Input value." ) );
    res->Add( new NameValData( "Excres_f", m_excres_f, "Drag area." ) );
    res->Add( new NameValData( "Excres_Amount", m_excres_Amount, "Amount." ) );
    res->Add( new NameValData( "Excres_PercTotalCD", m_excres_PercTotalCD, "Percent of total CD." ) );

    // Totals
    res->Add( new NameValData( "Geom_f_Total", GetGeomfTotal(), "Geom drag area." ) );
    res->Add( new NameValData( "Geom_CD_Total", GetGeometryCD(), "Geom drag coefficient." ) );
    res->Add( new NameValData( "Geom_Perc_Total", GetGeomPercTotal(), "Geom percent of total CD." ) );
    res->Add( new NameValData( "Excres_f_Total", GetExcresfTotal(), "Total excressence drag area." ) );
    res->Add( new NameValData( "Excres_CD_Total", GetTotalExcresCD(), "Total excressence drag coefficient." ) );
    res->Add( new NameValData( "Excres_Perc_Total", GetExcresPercTotal(), "Total excressence percent of CD." ) );
    res->Add( new NameValData( "Total_f_Total", GetfTotal(), "Drag area." ) );
    res->Add( new NameValData( "Total_CD_Total", GetTotalCD(), "Drag coefficient." ) );
    res->Add( new NameValData( "Total_Perc_Total", GetPercTotal(), "Total percent of CD (should be 1.0)." ) );

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
    m_geo_noArea.clear();
    m_geo_sourceRow.clear();
    m_geo_groupRow.clear();
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
    xmlNodePtr ParasiteDragnode = xmlNewChild( node, nullptr, BAD_CAST"ParasiteDragMgr", nullptr );

    ParmContainer::EncodeXml( ParasiteDragnode );
    XmlUtil::AddStringNode( ParasiteDragnode, "ReferenceGeomID", m_RefGeomID );
    XmlUtil::AddStringNode( ParasiteDragnode, "ModeID", m_ModeID );

    xmlNodePtr ExcresDragnode = xmlNewChild( ParasiteDragnode, nullptr, BAD_CAST"Excrescence", nullptr );

    XmlUtil::AddIntNode( ExcresDragnode, "NumExcres", m_ExcresRowVec.size() );

    for ( size_t i = 0; i < m_ExcresRowVec.size(); ++i )
    {
        snprintf( str, sizeof( str ),  "Excres_%zu", i );
        xmlNodePtr excresqualnode = xmlNewChild( ExcresDragnode, nullptr, BAD_CAST str, nullptr );

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
        m_RefGeomID = IDMgr.RemapRefID( XmlUtil::FindString( ParasiteDragnode, "ReferenceGeomID", m_RefGeomID ) );
        m_ModeID = IDMgr.RemapRefID( XmlUtil::FindString( ParasiteDragnode, "ModeID", m_ModeID ) );

        xmlNodePtr ExcresDragnode = XmlUtil::GetNode( ParasiteDragnode, "Excrescence", 0 );

        numExcres = XmlUtil::FindInt( ExcresDragnode, "NumExcres", 0 );

        for ( int i = 0; i < numExcres; i++ )
        {
            snprintf( str, sizeof( str ),  "Excres_%i", i );
            xmlNodePtr excresqualnode = XmlUtil::GetNode( ExcresDragnode, str, 0 );
            if ( !excresqualnode )
            {
                break;
            }

            AddExcrescence( XmlUtil::FindString( excresqualnode, "Label", "" ),
                            XmlUtil::FindInt( excresqualnode, "Type", 0 ),
                            XmlUtil::FindDouble( excresqualnode, "Input", 0.0 ) );
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
    int new_current = m_CurrentExcresIndex;
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

            // The current excrescence moves with its row
            if ( cur_max_ind == m_CurrentExcresIndex )
            {
                new_current = tempExcres.size() - 1;
            }
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
    m_CurrentExcresIndex = new_current;
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

    int set = m_SetChoice();

    if ( m_UseMode() )
    {
        Mode *m = ModeMgr.GetMode( m_ModeID );
        if ( m )
        {
            // m->ApplySettings();
            set = m->m_NormalSet();
        }
    }

    // Create New Vector of IDs to Compare
    vector <string> newIDVec = veh->GetGeomSet( set );
    vector < string > newVecToCompare;
    for ( int i = 0; i < newIDVec.size(); i++ )
    {
        Geom* geom = veh->FindGeom( newIDVec[i] );
        if ( geom )
        {
            if ( geom->isSurfaceType() )
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
            temprowsize += CalcGeomRowSize( geom );
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
        if ( m_geo_groupRow[index] < 0 && !m_geo_noArea[index] )
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
        SetDegenGeomVec( vector < DegenGeom > () );
        m_TagWetAreaMap.clear();

        ClearInputVectors();
        ClearOutputVectors();

        // Set New Active Geom Vec
        SetActiveGeomVec();
    }
}

void ParasiteDragMgrSingleton::RenewDegenGeomVec()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    int set = m_SetChoice();

    if ( m_UseMode() )
    {
        Mode *m = ModeMgr.GetMode( m_ModeID );
        if ( m )
        {
            m->ApplySettings();
            set = m->m_NormalSet();
        }
    }

    if ( veh )
    {
        // Don't pass mode parameters here as they've already been applied to 'set'.
        veh->CreateDegenGeom( set, /* useMode */ false, /* modeID */ "" );
        ComputeWetAreas( set );

        // First Assignment of DegenGeomVec, Will Carry Through to Rest of Calculate_X
        SetDegenGeomVec( veh->GetDegenGeomVec() );
    }
}

void ParasiteDragMgrSingleton::SetDegenGeomVec( const vector < DegenGeom > & degen_vec )
{
    m_DegenGeomVec = degen_vec;

    // Keyed by ID now, while the parent pointers are still good
    m_DegenIndexMap.clear();
    for ( int i = 0; i < ( int )m_DegenGeomVec.size(); i++ )
    {
        Geom* parent = m_DegenGeomVec[i].getParentGeom();
        if ( parent && m_DegenGeomVec[i].getType() != DegenGeom::DISK_TYPE &&
             m_DegenGeomVec[i].getCfdSurfType() != vsp::CFD_TRANSPARENT &&
             m_DegenIndexMap.find( parent->GetID() ) == m_DegenIndexMap.end() )
        {
            m_DegenIndexMap[ parent->GetID() ] = i;
        }
    }
}

int ParasiteDragMgrSingleton::FindDegenIndex( const string & geom_id )
{
    map < string, int >::const_iterator it = m_DegenIndexMap.find( geom_id );
    if ( it == m_DegenIndexMap.end() )
    {
        return -1;
    }

    if ( m_DegenGeomVec[ it->second ].getDegenSticks().empty() )
    {
        return -1;
    }

    return it->second;
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
