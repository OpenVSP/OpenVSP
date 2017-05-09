//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParasiteDragMgr.cpp: ParasiteDrag Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "Vehicle.h"
#include "VehicleMgr.h"
#include "ParmMgr.h"
#include "StlHelper.h"
#include "APIDefines.h"
#include "WingGeom.h"
#include "ResultsMgr.h"
#include "ParasiteDragMgr.h"

#include "eli/mutil/nls/newton_raphson_method.hpp"

#include <numeric>

//==== Constructor ====//
ParasiteDragMgrSingleton::ParasiteDragMgrSingleton() : ParmContainer()
{
    // Initial Values for certain variables
    SetDefaultStruct();
    m_Name = "ParasiteDragSettings";
    string groupname = "ParasiteDrag";
    m_RefGeomID = "";
    m_CompGeomResults = NULL;

    // ==== Parm Initialize and Description Setting ==== //
    // Reference Qualities Parms
    m_RefFlag.Init("RefFlag", groupname, this, MANUAL_REF, MANUAL_REF, COMPONENT_REF);
    m_RefFlag.SetDescript("Reference quantity flag");

    m_Sref.Init("Sref", groupname, this, 100.0, 0.0, 1e12);
    m_Sref.SetDescript("Reference area");

    m_LamCfEqnType.Init("LamCfEqnType", groupname, this, vsp::CF_LAM_BLASIUS, vsp::CF_LAM_BLASIUS, vsp::CF_LAM_BLASIUS_W_HEAT);
    m_LamCfEqnType.SetDescript("Laminar Cf Equation Choice");

    m_TurbCfEqnType.Init("TurbCfEqnType", groupname, this, vsp::CF_TURB_POWER_LAW_BLASIUS, vsp::CF_TURB_EXPLICIT_FIT_SPALDING,
        vsp::CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH);
    m_TurbCfEqnType.SetDescript("Turbulent Cf Equation Choice");

    m_AltLengthUnit.Init("AltLengthUnit", groupname, this, vsp::PD_UNITS_IMPERIAL, vsp::PD_UNITS_IMPERIAL, vsp::PD_UNITS_METRIC);
    m_AltLengthUnit.SetDescript("Altitude Units");

    m_LengthUnit.Init("LengthUnit", groupname, this, vsp::LEN_FT, vsp::LEN_MM, vsp::LEN_UNITLESS);
    m_LengthUnit.SetDescript("Length Units");

    m_TempUnit.Init("TempUnit", groupname, this, vsp::TEMP_UNIT_F, vsp::TEMP_UNIT_K, vsp::TEMP_UNIT_R);
    m_TempUnit.SetDescript("Temperature Units");

    // Air Qualities Parms
    m_FreestreamType.Init("FreestreamType", groupname, this, vsp::ATMOS_TYPE_US_STANDARD_1976,
        vsp::ATMOS_TYPE_US_STANDARD_1976, vsp::ATMOS_TYPE_MANUAL_RE_L);
    m_FreestreamType.SetDescript("Assigns the desired inputs to describe the freestream properties");

    m_Mach.Init("Mach", groupname, this, 0.0, 0.0, 1000.0);
    m_Mach.SetDescript("Mach Number for Current Flight Condition");

    m_ReqL.Init("Re_L", groupname, this, 0.0, 0.0, 1e12);
    m_ReqL.SetDescript("Reynolds Number Per Unit Length");

    m_Temp.Init("Temp", groupname, this, 288.15, -459.67, 1e12);
    m_Temp.SetDescript("Temperature");

    m_Pres.Init("Pres", groupname, this, 2116.221, 1e-4, 1e12);
    m_Pres.SetDescript("Pressure");

    m_Rho.Init("Density", groupname, this, 0.07647, 1e-12, 1e12);
    m_Rho.SetDescript("Density");

    m_DynaVisc.Init("DynaVisc", groupname, this, 0.0, 1e-12, 1e12);
    m_DynaVisc.SetDescript("Dynamic Viscosity for Current Condition");

    m_SpecificHeatRatio.Init("SpecificHeatRatio", groupname, this, 1.4, -1, 1e3);
    m_SpecificHeatRatio.SetDescript("Specific Heat Ratio");

    //m_MediumType.Init("Medium", groupname, this, Atmosphere::MEDIUM_AIR, Atmosphere::MEDIUM_AIR, Atmosphere::MEDIUM_PURE_WATER);
    //m_MediumType.SetDescript("Wind Tunnel Medium");

    m_Vinf.Init("Vinf", groupname, this, 500.0, 0.0, 1e12);
    m_Vinf.SetDescript("Free Stream Velocity");

    m_VinfUnitType.Init("VinfUnitType", groupname, this, vsp::V_UNIT_FT_S, vsp::V_UNIT_FT_S, vsp::V_UNIT_KTAS);
    m_VinfUnitType.SetDescript("Units for Freestream Velocity");

    m_Hinf.Init("Alt", groupname, this, 20000.0, 0.0, 271823.3);
    m_Hinf.SetDescript("Physical Altitude from Sea Level");

    m_DeltaT.Init("DeltaTemp", groupname, this, 0.0, -1e12, 1e12);
    m_DeltaT.SetDescript("Delta Temperature from STP");
}

void ParasiteDragMgrSingleton::Renew()
{
    m_TableRowVec.clear();

    m_DegenGeomVec.clear();
    m_CompGeomResults = NULL;

    SetDefaultStruct();

    m_RefGeomID = "";
}

void ParasiteDragMgrSingleton::SetDefaultStruct()
{
    m_DefaultStruct.GeomID = "";
    m_DefaultStruct.SubSurfID = "";
    m_DefaultStruct.Label = "";
    m_DefaultStruct.Swet = -1;
    m_DefaultStruct.Lref = -1;
    m_DefaultStruct.Re = -1;
    m_DefaultStruct.Roughness = -1;
    m_DefaultStruct.TeTwRatio = -1;
    m_DefaultStruct.TawTwRatio = -1;
    m_DefaultStruct.PercLam = 0.0;
    m_DefaultStruct.Cf = -1;
    m_DefaultStruct.fineRat = -1;
    m_DefaultStruct.GeomShapeType = 0;
    m_DefaultStruct.FFEqnChoice = 0;
    m_DefaultStruct.FFEqnName = "";
    m_DefaultStruct.FF = -1;
    m_DefaultStruct.Q = 1;
    m_DefaultStruct.f = -1;
    m_DefaultStruct.CD = -1;
    m_DefaultStruct.PercTotalCd = -1;
    m_DefaultStruct.SurfNum = 0;
    m_DefaultStruct.GroupedAncestorGen = 0;
    m_DefaultStruct.ExpandedList = false;
}

void ParasiteDragMgrSingleton::ParmChanged(Parm* parm_ptr, int type)
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if (veh)
    {
        veh->ParmChanged(parm_ptr, type);
    }
}

void ParasiteDragMgrSingleton::InitTableVec()
{
    m_TableRowVec.clear();
    for (int i = 0; i < m_RowSize; ++i)
    {
        m_TableRowVec.push_back(m_DefaultStruct);
    }
}

void ParasiteDragMgrSingleton::LoadMainTableUserInputs()
{
    Geom* geom;
    char str[256];
    Vehicle *veh = VehicleMgr.GetVehicle();

    for (int i = 0; i < m_PDGeomIDVec.size(); i++)
    {
        geom = veh->FindGeom(m_PDGeomIDVec[i]);
        if (geom)
        {
            for (int j = 0; j < geom->GetNumTotalSurfs(); j++)
            {
                // Custom Geom Check: if surf type is the same, apply same qualities
                if (j > 0 && geom->GetSurfPtr(j)->GetSurfType() == geom->GetSurfPtr(j - 1)->GetSurfType())
                {
                    geo_percLam.push_back(geo_percLam[geo_percLam.size() - 1]);
                    geo_ffIn.push_back(geo_ffIn[geo_ffIn.size() - 1]);
                    geo_Q.push_back(geo_Q[geo_Q.size() - 1]);
                    geo_Roughness.push_back(geo_Roughness[geo_Roughness.size() - 1]);
                    geo_TeTwRatio.push_back(geo_TeTwRatio[geo_TeTwRatio.size() - 1]);
                    geo_TawTwRatio.push_back(geo_TawTwRatio[geo_TawTwRatio.size() - 1]);
                    geo_surfNum.push_back(j);
                    sprintf(str, "%s_%d", geom->GetName().c_str(), j);
                }
                else
                {
                    if (geom->GetType().m_Type == CUSTOM_GEOM_TYPE)
                    {
                        if (geom->GetSurfPtr(j)->GetSurfType() == vsp::NORMAL_SURF)
                        {
                            sprintf(str, "[B] %s", geom->GetName().c_str());
                        }
                        else
                        {
                            sprintf(str, "[W] %s", geom->GetName().c_str());
                        }
                        geo_surfNum.push_back(j);
                    }
                    else
                    {
                        sprintf(str, "%s", geom->GetName().c_str());
                        geo_surfNum.push_back(0);
                    }
                    geo_percLam.push_back(geom->m_PercLam());
                    geo_ffIn.push_back(geom->m_FFUser());
                    geo_Q.push_back(geom->m_Q());
                    geo_Roughness.push_back(geom->m_Roughness());
                    geo_TeTwRatio.push_back(geom->m_TeTwRatio());
                    geo_TawTwRatio.push_back(geom->m_TawTwRatio());
                }

                geo_shapeType.push_back(geom->GetSurfPtr(j)->GetSurfType()); // Form Factor Shape Type

                if (geom->GetSurfPtr(j)->GetSurfType() == vsp::NORMAL_SURF)
                {
                    geo_ffType.push_back(geom->m_FFBodyEqnType());
                }
                else
                {
                    geo_ffType.push_back(geom->m_FFWingEqnType());
                }
                geo_geomID.push_back(geom->GetID());
                geo_subsurfID.push_back("");

                // Assign Label to Geom
                geo_label.push_back(str);
            }

            // Sub Surfaces
            for (int j = 0; j < geom->GetSubSurfVec().size(); j++)
            {
                for (int k = 0; k < geom->GetNumTotalSurfs(); ++k)
                {
                    geo_percLam.push_back(geo_percLam[geo_percLam.size() - 1]); //TODO: Add Perc Lam to SubSurf
                    geo_ffIn.push_back(geo_ffIn[geo_ffIn.size() - 1]);
                    geo_Q.push_back(geo_Q[geo_Q.size() - 1]); // TODO: Add Q to SubSurf
                    geo_Roughness.push_back(geo_Roughness[geo_Roughness.size() - 1]); //TODO: Add Roughness to SubSurf
                    geo_TeTwRatio.push_back(geo_TeTwRatio[geo_TeTwRatio.size() - 1]);
                    geo_TawTwRatio.push_back(geo_TawTwRatio[geo_TawTwRatio.size() - 1]);
                    geo_surfNum.push_back(k);

                    geo_shapeType.push_back(geom->GetSurfPtr(k)->GetSurfType()); // Form Factor Shape Type

                    if (geom->GetSurfPtr(k)->GetSurfType() == vsp::NORMAL_SURF)
                    {
                        geo_ffType.push_back(geom->m_FFBodyEqnType());
                    }
                    else
                    {
                        geo_ffType.push_back(geom->m_FFWingEqnType());
                    }
                    geo_geomID.push_back(geom->GetID());
                    geo_subsurfID.push_back(geom->GetSubSurf(j)->GetID());
                    sprintf(str, "[ss] %s_%i", geom->GetSubSurfVec()[j]->GetName().c_str(), k);

                    // Assign Label to Geom
                    geo_label.push_back(str);
                }
            }
        }
    }
}

void ParasiteDragMgrSingleton::SetupFullCalculation()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if (veh)
    {
        veh->ClearDegenGeom();
        ResultsMgr.DeleteResult(ResultsMgr.FindResultsID("Comp_Geom"));
        ClearInputVectors();
        ClearOutputVectors();

        veh->CreateDegenGeom(m_SetChoice());
        string meshID = veh->CompGeomAndFlatten(m_SetChoice(), 0);
        veh->DeleteGeom(meshID);
        veh->ShowOnlySet(m_SetChoice());

        // First Assignment of DegenGeomVec, Will Carry Through to Rest of Calculate_X
        m_DegenGeomVec = veh->GetDegenGeomVec();

        // First Assignment of CompGeon, Will Carry Through to Rest of Calculate_X
        m_CompGeomResults = ResultsMgr.FindResults("Comp_Geom");
    }
}

int ParasiteDragMgrSingleton::CalcRowSize()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if (!veh)
    {
        return 0;
    }

    m_RowSize = 0; // Reset every call
    for (int i = 0; i < m_PDGeomIDVec.size(); i++)
    {
        Geom* geom = veh->FindGeom(m_PDGeomIDVec[i]);
        if (geom)
        {
            m_RowSize += geom->GetNumTotalSurfs();
            for (size_t j = 0; j < geom->GetSubSurfVec().size(); ++j)
            {
                for (size_t k = 0; k < geom->GetNumSymmCopies(); ++k)
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

    if (!veh)
    {
        return;
    }

    int iSurf = 0;
    for (int i = 0; i < m_RowSize; ++i)
    {
        if (!m_DegenGeomVec.empty())
        { // If DegenGeom Exists Pull Swet
            vector < string > tagnamevec = m_CompGeomResults->Find("Tag_Name").GetStringData();
            if (geo_subsurfID[i].compare("") == 0)
            {
                sprintf(str, "%s%i", veh->FindGeom(geo_geomID[i])->GetName().c_str(), geo_surfNum[i]);
                newstr = str;
                searchIndex = vector_find_val(tagnamevec, newstr);
                geo_swet.push_back(m_CompGeomResults->Find("Tag_Wet_Area").GetDouble(searchIndex));
                ++iSurf;
            }
            else
            {
                sprintf(str, "%s%i,%s", veh->FindGeom(geo_geomID[i])->GetName().c_str(), geo_surfNum[i],
                    veh->FindGeom(geo_geomID[i])->GetSubSurf(geo_subsurfID[i])->GetName().c_str());
                newstr = str;
                searchIndex = vector_find_val(tagnamevec, newstr);
                geo_swet.push_back(m_CompGeomResults->Find("Tag_Wet_Area").GetDouble(searchIndex));
            }
        }
        else
        { // Else Push Back Default Val
            geo_swet.push_back(-1);
        }
    }
}

void ParasiteDragMgrSingleton::Calculate_Lref()
{
    // Initialize Variables
    int iSurf = 0;

    for (int i = 0; i < m_RowSize; ++i)
    {
        if (!m_DegenGeomVec.empty())
        { // If DegenGeom Exists Calculate Lref
            if (geo_subsurfID[i].compare("") == 0)
            {
                if (m_DegenGeomVec[iSurf].getType() != DegenGeom::DISK_TYPE)
                {
                    if (m_DegenGeomVec[iSurf].getType() == DegenGeom::SURFACE_TYPE)
                    {
                        CalcReferenceChord(iSurf);
                    }
                    else if (m_DegenGeomVec[iSurf].getType() == DegenGeom::BODY_TYPE)
                    {
                        CalcReferenceBodyLength(iSurf);
                    }
                    ++iSurf;
                }
                else
                {
                    --i;
                    ++iSurf;
                }
            }
            else
            {
                geo_lref.push_back(geo_lref[geo_lref.size() - 1]);
            }
        }
        else
        { // Else Push Back Default Val
            geo_lref.push_back(-1);
        }
    }
}

// Use Bounding Box to approximate x directional length
void ParasiteDragMgrSingleton::CalcReferenceBodyLength(int index)
{
    // TODO: Improve Reference Length Calculations
    double delta_x, delta_y, delta_z, lref;
    vector <DegenStick> m_DegenStick = m_DegenGeomVec[index].getDegenSticks();
    delta_x = abs(m_DegenStick[0].xle.front().x() - m_DegenStick[0].xle.back().x());
    delta_y = abs(m_DegenStick[0].xle.front().y() - m_DegenStick[0].xle.back().y());
    delta_z = abs(m_DegenStick[0].xle.front().z() - m_DegenStick[0].xle.back().z());
    lref = sqrt( pow(delta_x,2.0) + pow(delta_y,2.0) + pow(delta_z,2.0) );

    if (lref <= 1e-6) // Is it basically zero?
    {
        // Attempt to use chord from DegenGeom
        CalcReferenceChord(index);
    }

    // If STILL 0
    if (lref <= 1e-6)
    {
        geo_lref.push_back(1.0);
    }
    else
    {
        geo_lref.push_back(lref);
    }
}

// Use weighted average to approximate reference chord
void ParasiteDragMgrSingleton::CalcReferenceChord(int index)
{
    // TODO: Improve Reference Length Calculations
    vector <DegenStick> m_DegenStick = m_DegenGeomVec[index].getDegenSticks();
    double secArea, totalArea = 0, weightedChordSum = 0;
    double delta_x, delta_y, delta_z, section_span;
    for (size_t j = 0; j <= m_DegenStick[0].areaTop.size() - 1; ++j)
    {
        delta_x = abs(m_DegenStick[0].xle[j].x() - m_DegenStick[0].xle[j + 1].x());
        delta_y = abs(m_DegenStick[0].xle[j].y() - m_DegenStick[0].xle[j + 1].y());
        delta_z = abs(m_DegenStick[0].xle[j].z() - m_DegenStick[0].xle[j + 1].z());
        section_span = sqrt( pow(delta_x,2.0) + pow(delta_y,2.0) + pow(delta_z,2.0) );
        secArea = section_span* (0.5 * (m_DegenStick[0].chord[j] + m_DegenStick[0].chord[j + 1]));

        totalArea += secArea;

        weightedChordSum += m_DegenStick[0].chord[j] * secArea;
    }
    double lref = weightedChordSum / totalArea;

    if (lref <= 1e-6) // Is it basically zero?
    {
        // Attempt to use chord from DegenGeom
        CalcReferenceBodyLength(index);
    }

    // If STILL 0
    if (lref <= 1e-6)
    {
        geo_lref.push_back(1.0);
    }
    else
    {
        geo_lref.push_back(lref);
    }
}

void ParasiteDragMgrSingleton::Calculate_Re()
{
    for (int i = 0; i < m_RowSize; ++i)
    {
        if (!m_DegenGeomVec.empty())
        { // If DegenGeom Exists Calculate Re
            if (geo_subsurfID[i].compare("") == 0)
            {
                ReynoldsNumCalc(i);
            }
            else
            {
                geo_Re.push_back(geo_Re[geo_Re.size() - 1]);
            }

        }
        else
        { // Else Push Back Default Val
            geo_Re.push_back(-1);
        }
    }

    CalcRePowerDivisor();
}

void ParasiteDragMgrSingleton::CalcRePowerDivisor()
{
    if (!geo_Re.empty())
    {
        vector<double>::const_iterator it = max_element(geo_Re.begin(), geo_Re.end());
        m_ReynoldsPowerDivisor = mag(*it);
    }
    else
    {
        m_ReynoldsPowerDivisor = 1;
    }
}

void ParasiteDragMgrSingleton::ReynoldsNumCalc(int index)
{
    double vinf, lref;
    if (m_FreestreamType() != vsp::ATMOS_TYPE_MANUAL_RE_L)
    {
        vinf = m_Vinf();
        lref = geo_lref[index];

        if (m_AltLengthUnit() == vsp::PD_UNITS_IMPERIAL)
        {
            vinf = ConvertVelocity(vinf, m_VinfUnitType.Get(), vsp::V_UNIT_FT_S);
            lref = ConvertLength(lref, m_LengthUnit(), vsp::LEN_FT);
        }
        else if (m_AltLengthUnit() == vsp::PD_UNITS_METRIC)
        {
            vinf = ConvertVelocity(vinf, m_VinfUnitType.Get(), vsp::V_UNIT_M_S);
            lref = ConvertLength(lref, m_LengthUnit(), vsp::LEN_M);
        }

        geo_Re.push_back((vinf * lref) / m_KineVisc());
    }
    else
    { // Any other freestream definition type
        geo_Re.push_back(m_ReqL() * geo_lref[index]);
    }
}

void ParasiteDragMgrSingleton::Calculate_Cf()
{
    // Initialize Variables
    double lref, rho, kineVisc, vinf;

    for (int i = 0; i < m_RowSize; ++i)
    {
        if (!m_DegenGeomVec.empty())
        { // If DegenGeom Exists Calculate Cf
            if (geo_subsurfID[i].compare("") == 0)
            {
                vinf = ConvertVelocity(m_Vinf(), m_VinfUnitType.Get(), vsp::V_UNIT_M_S);
                rho = ConvertDensity(m_Atmos.GetDensity(), m_AltLengthUnit(), vsp::RHO_UNIT_KG_M3); // lb/ft3 to kg/m3
                lref = ConvertLength(geo_lref[i], m_LengthUnit(), vsp::LEN_M);
                kineVisc = m_Atmos.GetDynaVisc() / rho;

                if (geo_percLam[i] == 0 || geo_percLam[i] == -1)
                { // Assume full turbulence
                    geo_cf.push_back(CalcTurbCf(geo_Re[i], geo_lref[i], m_TurbCfEqnType(),
                        m_SpecificHeatRatio(), geo_Roughness[i], geo_TawTwRatio[i], geo_TeTwRatio[i]));
                }
                else
                { // Not full turbulence 
                    CalcPartialTurbulence(i, lref, vinf, kineVisc);
                }
            }
            else
            {
                geo_cf.push_back(geo_cf[geo_cf.size() - 1]);
            }
        }
        else
        { // Else push back default value
            geo_cf.push_back(-1);
        }
    }
}

void ParasiteDragMgrSingleton::CalcPartialTurbulence(int i, double lref, double vinf, double kineVisc)
{
    if (geo_Re[i] != 0)
    { // Prevent dividing by 0 in some equations
        double LamPerc = (geo_percLam[i] / 100);
        double CffullTurb = CalcTurbCf(geo_Re[i], geo_lref[i], m_TurbCfEqnType(),
            m_SpecificHeatRatio(), geo_Roughness[i], geo_TawTwRatio[i], geo_TeTwRatio[i]);
        double CffullLam = CalcLamCf(geo_Re[i], m_LamCfEqnType.Get());

        double LamPercRefLen = LamPerc * lref;

        double ReLam = (vinf * LamPercRefLen) / kineVisc;

        double CfpartLam = CalcLamCf(ReLam, m_LamCfEqnType());
        double CfpartTurb = CalcTurbCf(ReLam, geo_lref[i], m_TurbCfEqnType(),
            m_SpecificHeatRatio(), geo_Roughness[i], geo_TawTwRatio[i], geo_TeTwRatio[i]);

        geo_cf.push_back(CffullTurb - (CfpartTurb * LamPerc) +
            (CfpartLam * LamPerc));
    }
    else
    {
        geo_cf.push_back(0);
    }
}

void ParasiteDragMgrSingleton::Calculate_fineRat()
{
    // Initialize Variables
    vector<double>::const_iterator it;
    double max_xsecarea, dia;
    int iSurf = 0;

    for (int i = 0; i < m_RowSize; ++i)
    {
        if (!m_DegenGeomVec.empty())
        { // If DegenGeom Exists Calculate Fineness Ratio
            if (geo_subsurfID[i].compare("") == 0)
            {
                // Grab Degen Sticks for Appropriate Geom
                vector <DegenStick> degenSticks = m_DegenGeomVec[iSurf].getDegenSticks();

                if (m_DegenGeomVec[iSurf].getType() != DegenGeom::DISK_TYPE)
                {
                    if (m_DegenGeomVec[iSurf].getType() == DegenGeom::SURFACE_TYPE)
                    { // Wing Type
                        it = max_element(degenSticks[0].toc.begin(), degenSticks[0].toc.end());
                        geo_fineRat.push_back(*it);
                    }
                    else if (m_DegenGeomVec[iSurf].getType() == DegenGeom::BODY_TYPE)
                    {
                        it = max_element(degenSticks[0].sectarea.begin(), degenSticks[0].sectarea.end());
                        max_xsecarea = *it;

                        // Use Max X-Sectional Area to find "Nominal" Diameter
                        dia = 2 * sqrt((max_xsecarea / (PI)));

                        geo_fineRat.push_back(dia / geo_lref[i]);
                    }
                    ++iSurf;
                }
                else
                {
                    --i;
                    ++iSurf;
                }
            }
            else
            {
                geo_fineRat.push_back(geo_fineRat[geo_fineRat.size() - 1]);
            }
        }
        else
        { // Else Push Back Default Val
            geo_fineRat.push_back(-1);
        }
    }
}

void ParasiteDragMgrSingleton::Calculate_FF()
{
    // Initialize Variables
    vector<double>::const_iterator it;
    double toc;
    double fin_rat, longF, Area;
    vector <double> hVec, wVec;
    int iSurf = 0;

    for (int i = 0; i < m_RowSize; ++i)
    {
        if (!m_DegenGeomVec.empty())
        { // If DegenGeom Exists Calculate Form Factor
            if (geo_subsurfID[i].compare("") == 0)
            {
                // Grab Degen Sticks for Appropriate Geom
                vector <DegenStick> degenSticks = m_DegenGeomVec[iSurf].getDegenSticks();

                if (m_DegenGeomVec[iSurf].getType() == DegenGeom::SURFACE_TYPE)
                { // Wing Type

                    toc = geo_fineRat[i];

                    Calculate_AvgSweep(degenSticks);

                    geo_ffOut.push_back(CalcFFWing(toc, geo_ffType[i], geo_percLam[i], m_Sweep25, m_Sweep50));
                    if (geo_ffType[i] == vsp::FF_W_JENKINSON_TAIL)
                    {
                        geo_Q[i] = 1.2;
                    }
                }
                else if (m_DegenGeomVec[iSurf].getType() == DegenGeom::BODY_TYPE)
                {
                    // Get Fine Rat
                    fin_rat = geo_fineRat[i];

                    // Invert Fineness Ratio
                    longF = pow(geo_fineRat[i], -1);

                    // Max Cross Sectional Area
                    Area = *max_element(degenSticks[0].areaTop.begin(), degenSticks[0].areaTop.end());

                    geo_ffOut.push_back(CalcFFBody(longF, geo_ffType[i], geo_lref[i], Area));
                }
                else
                {
                    --i;
                }
                ++iSurf;
            }
            else
            {
                geo_ffOut.push_back(geo_ffOut[geo_ffOut.size() - 1]);
                geo_ffName.push_back(geo_ffName[geo_ffName.size() - 1]);
            }
        }
        // Else Push Back Default Val
        else
        {
            geo_ffOut.push_back(-1);
            geo_ffName.push_back("");
        }
    }
}

void ParasiteDragMgrSingleton::Calculate_AvgSweep(vector<DegenStick> degenSticks)
{
    // Find Quarter Chord Using Derived Eqn from Geometry
    double width, secSweep25, secSweep50, secArea, weighted25Sum = 0, weighted50Sum = 0, totalArea = 0;
    for (int j = 0; j < degenSticks[0].areaTop.size(); j++)
    {
        width = degenSticks[0].areaTop[j] /
            ((degenSticks[0].perimTop[j] + degenSticks[0].perimTop[j + 1.0]) / 2.0);

        // Section Quarter Chord Sweep
        secSweep25 = atan(tan(degenSticks[0].sweeple[j] * PI / 180.0) +
            (0.25 * ((degenSticks[0].chord[j] - degenSticks[0].chord[j + 1.0]) / width))) *
            180.0 / PI;

        // Section Half Chord Sweep
        secSweep50 = atan(tan(degenSticks[0].sweeple[j] * PI / 180.0) +
            (0.50 * ((degenSticks[0].chord[j] - degenSticks[0].chord[j + 1.0]) / width))) *
            180.0 / PI;

        // Section Area
        secArea = degenSticks[0].chord[j] * width;

        // Add Weighted Value to Weighted Sum
        weighted25Sum += secArea*secSweep25;
        weighted50Sum += secArea*secSweep50;

        // Continue to sum up Total Area
        totalArea += secArea;
    }

    // Calculate Sweep @ c/4 & c/2
    m_Sweep25 = weighted25Sum / totalArea * PI / 180.0; // Into Radians
    m_Sweep50 = weighted50Sum / totalArea * PI / 180.0; // Into Radians
}

// ================================== //
// ====== Iterative Functions ======= //
// ================================== //

struct Schoenherr_functor
{
    double operator()(const double &Cf) const
    {
        return (0.242 / (sqrt(Cf) * log10(Re * Cf))) - 1.0;
    }
    double Re;
};

struct Schoenherr_p_functor
{
    double operator()(const double &Cf) const
    {
        return ((-0.278613 * log(Cf * Re)) - 0.557226) /
            (pow(Cf, 1.5) * pow(log(Re * Cf), 2.0));
    }
    double Re;
};

struct Karman_functor
{
    double operator()(const double &Cf) const
    {
        return ((4.15 * log10(Re * Cf) + 1.70) * sqrt(Cf)) - 1.0;
    }
    double Re;
};

struct Karman_p_functor
{
    double operator()(const double &Cf) const
    {
        return (0.901161 * log(Re * Cf) + 2.65232) / sqrt(Cf);
    }
    double Re;
};

struct Karman_Schoenherr_functor
{
    double operator()(const double &Cf) const
    {
        return ((4.13 * log10(Re * Cf)) * sqrt(Cf)) - 1.0;
    }
    double Re;
};

struct Karman_Schoenherr_p_functor
{
    double operator()(const double &Cf) const
    {
        return (0.896818 * log(Re * Cf) + 1.79364) / sqrt(Cf);
    }
    double Re;
};

// ================================== //
// ================================== //
// ================================== //

double ParasiteDragMgrSingleton::CalcTurbCf(double ReyIn, double ref_leng, int cf_case,
    double roughness_h = 0, double gamma = 1.4, double taw_tw_ratio = 0, double te_tw_ratio = 0)
{
    double CfOut, CfGuess, f, heightRatio, multiBy;
    double r = 0.89; // Recovery Factor
    double n = 0.67; // Viscosity Power-Law Exponent

    eli::mutil::nls::newton_raphson_method < double > nrm;

    if (m_LengthUnit.Get() == vsp::LEN_FT)
    {
        multiBy = 12.0;
    }
    else if (m_LengthUnit.Get() == vsp::LEN_M)
    {
        multiBy = 39.3701;
    }

    switch (cf_case)
    {
    case vsp::CF_TURB_WHITE_CHRISTOPH_COMPRESSIBLE:
        CfOut = 0.42 / pow(log(0.056 * ReyIn), 2.0);
        break;

    case vsp::CF_TURB_SCHLICHTING_PRANDTL:
        CfOut = 1 / pow((2 * log10(ReyIn) - 0.65), 2.3);
        break;

    case vsp::CF_TURB_SCHLICHTING_COMPRESSIBLE:
        CfOut = 0.455 / pow(log10(ReyIn), 2.58);
        break;

    case vsp::CF_TURB_SCHLICHTING_INCOMPRESSIBLE:
        CfOut = 0.472 / pow(log10(ReyIn), 2.5);
        break;

    case vsp::CF_TURB_SCHULTZ_GRUNOW_SCHOENHERR:
        CfOut = 0.427 / pow((log10(ReyIn) - 0.407), 2.64);
        break;

    case vsp::CF_TURB_SCHULTZ_GRUNOW_HIGH_RE:
        CfOut = 0.37 / pow(log10(ReyIn), 2.584);
        break;

    case vsp::CF_TURB_POWER_LAW_BLASIUS:
        CfOut = 0.0592 / pow(ReyIn, 0.2);
        break;

    case vsp::CF_TURB_POWER_LAW_PRANDTL_LOW_RE:
        CfOut = 0.074 / pow(ReyIn, 0.2);
        break;

    case vsp::CF_TURB_POWER_LAW_PRANDTL_MEDIUM_RE:
        CfOut = 0.027 / pow(ReyIn, 1.0 / 7.0);
        break;

    case vsp::CF_TURB_POWER_LAW_PRANDTL_HIGH_RE:
        CfOut = 0.058 / pow(ReyIn, 0.2);
        break;

    case vsp::CF_TURB_EXPLICIT_FIT_SPALDING:
        CfOut = 0.455 / pow(log(0.06 * ReyIn), 2.0);
        break;

    case vsp::CF_TURB_EXPLICIT_FIT_SPALDING_CHI:
        CfOut = 0.225 / pow(log10(ReyIn), 2.32);
        break;

    case vsp::CF_TURB_EXPLICIT_FIT_SCHOENHERR:
        CfOut = pow((1.0 / ((3.46* log10(ReyIn)) - 5.6)), 2.0);
        break;

    case vsp::CF_TURB_IMPLICIT_SCHOENHERR:
        Schoenherr_functor sfun;
        sfun.Re = ReyIn;
        Schoenherr_p_functor sfunprm;
        sfunprm.Re = ReyIn;

        CfGuess = pow((1.0 / ((3.46* log10(ReyIn)) - 5.6)), 2.0);

        nrm.set_initial_guess(CfGuess);
        nrm.find_root(CfOut, sfun, sfunprm, 0.0);
        break;

    case vsp::CF_TURB_IMPLICIT_KARMAN:
        Karman_functor kfun;
        kfun.Re = ReyIn;
        Karman_p_functor kfunprm;
        kfunprm.Re = ReyIn;

        CfGuess = 0.455 / pow(log10(ReyIn), 2.58);

        nrm.set_initial_guess(CfGuess);
        nrm.find_root(CfOut, kfun, kfunprm, 0.0);
        break;

    case vsp::CF_TURB_IMPLICIT_KARMAN_SCHOENHERR:
        Karman_Schoenherr_functor ksfun;
        ksfun.Re = ReyIn;
        Karman_Schoenherr_p_functor ksfunprm;
        ksfunprm.Re = ReyIn;

        CfGuess = pow((1.0 / ((3.46* log10(ReyIn)) - 5.6)), 2.0);

        nrm.set_initial_guess(CfGuess);
        nrm.find_root(CfOut, ksfun, ksfunprm, 0.0);
        break;

    case vsp::CF_TURB_ROUGHNESS_WHITE:
        heightRatio = ref_leng / roughness_h;
        CfOut = pow((1.4 + (3.7 * log10(heightRatio))), -2.0);
        break;

    case vsp::CF_TURB_ROUGHNESS_SCHLICHTING_LOCAL:
        heightRatio = ref_leng / roughness_h;
        CfOut = pow((1.4 + (3.7 * log10(heightRatio))), -2.0);
        break;

    case vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG:
        heightRatio = ref_leng / (roughness_h * multiBy);
        CfOut = pow((1.89 + (1.62 * log10(heightRatio))), -2.5);
        break;

    case vsp::CF_TURB_ROUGHNESS_SCHLICHTING_AVG_FLOW_CORRECTION:
        heightRatio = ref_leng / (roughness_h * multiBy);
        CfOut = pow((1.89 + (1.62 * log10(heightRatio))), -2.5) /
            (pow((1.0 + ((gamma - 1.0) / 2.0) * m_Mach()), 0.467));
        break;

    case vsp::CF_TURB_HEATTRANSFER_WHITE_CHRISTOPH:
        f = (1 + (0.22 * r * (((roughness_h * multiBy) - 1.0) / 2.0) *
            m_Mach() * m_Mach() * te_tw_ratio)) /
            (1 + (0.3 * (taw_tw_ratio - 1.0)));

        CfOut = (0.451 * f * f * te_tw_ratio) /
            (log(0.056 * f * pow(te_tw_ratio, 1.0 + n) * ReyIn));

        break;

    default:
        CfOut = 0;
        break;
    }

    return CfOut;
}

double ParasiteDragMgrSingleton::CalcLamCf(double ReyIn, int cf_case)
{
    double CfOut;

    switch (cf_case)
    {
    case vsp::CF_LAM_BLASIUS:
        CfOut = 1.32824 / pow(ReyIn, 0.5);
        break;

    case vsp::CF_LAM_BLASIUS_W_HEAT:
        CfOut = 0;
        break;

    default:
        CfOut = 0;
    }

    return CfOut;
}

double ParasiteDragMgrSingleton::CalcFFWing(double toc, int ff_case,
    double perc_lam = 0, double sweep25 = 0, double sweep50 = 0)
{
    // Values recreated using Plot Digitizer and fitted to a 3rd power polynomial
    double Interval[] = { 0.25, 0.6, 0.8, 0.9 };
    size_t nint = 4;
    double ff;

    switch (ff_case)
    {
    case vsp::FF_W_MANUAL:
        ff = 1;
        break;

    case vsp::FF_W_EDET_CONV:
        ff = 1.0 + toc*(2.94206 + toc*(7.16974 + toc*(48.8876 +
            toc*(-1403.02 + toc*(8598.76 + toc*(-15834.3))))));
        break;

    case vsp::FF_W_EDET_ADV:
        ff = 1.0 + 4.275 * toc;
        break;

    case vsp::FF_W_HOERNER:
        ff = 1.0 + 2.0 * toc + 60.0 * pow(toc, 4.0);
        break;

    case vsp::FF_W_COVERT:
        ff = 1.0 + 1.8 * toc + (50.0 * pow(toc, 4.0));
        break;

    case vsp::FF_W_SHEVELL:
        double Z;
        Z = ((2.0 - pow(m_Mach(), 2.0)) * cos(sweep25)) / (sqrt(1.0 -
            (pow(m_Mach(), 2.0) * pow(cos(sweep25), 2))));
        ff = 1.0 + (Z * toc) + (100.0 * pow(toc, 4.0));
        break;

    case vsp::FF_W_KROO:
        double part1A, part1B, part2A, part2B;
        part1A = (2.2 * pow(cos(sweep25), 2.0) * toc);
        part1B = (sqrt(1.0 - (pow(m_Mach(), 2.0) * pow(cos(sweep25), 2.0))));
        part2A = (4.84 * pow(cos(sweep25), 2.0)* (1.0 + 5.0 * pow(cos(sweep25), 2.0)) * pow(toc, 2.0));
        part2B = (2.0 * (1.0 - (pow(m_Mach(), 2.0) * pow(cos(sweep25), 2.0))));
        ff = 1.0 + (part1A / part1B) + (part2A / part2B);
        break;

    case vsp::FF_W_TORENBEEK:
        ff = 1.0 + 2.7*toc + 100.0 * pow(toc, 4.0);
        break;

    case vsp::FF_W_DATCOM:
        double L, Rls, x, RLS_Low, RLS_High;;

        // L value Decided based on xtrans/c
        if (perc_lam <= 0.30)
            L = 2.0;
        else
            L = 1.2;

        for (size_t i = 0; i < nint; ++i)
        {
            if (m_Mach() <= Interval[0])
            {
                Rls = -2.0292 * pow(cos(sweep25), 3.0) + 3.6345 * pow(cos(sweep25), 2.0) - 1.391 * cos(sweep25) + 0.8521;
            }
            else if (m_Mach() > Interval[3])
            {
                Rls = -1.8316 * pow(cos(sweep25), 3.0) + 3.3944 * pow(cos(sweep25), 2.0) - 1.3596 * cos(sweep25) + 1.1567;
            }
            else if (m_Mach() >= Interval[i])
            {
                x = (m_Mach() - Interval[i]) / (Interval[i + 1] - Interval[i]);
                if (i == 0)
                {
                    RLS_Low = -2.0292 * pow(cos(sweep25), 3.0) + 3.6345 * pow(cos(sweep25), 2.0) - 1.391 * cos(sweep25) + 0.8521;
                    RLS_High = -1.9735 * pow(cos(sweep25), 3.0) + 3.4504 * pow(cos(sweep25), 2.0) - 1.186 * cos(sweep25) + 0.858;
                }
                else if (i == 1)
                {
                    RLS_Low = -1.9735 * pow(cos(sweep25), 3.0) + 3.4504 * pow(cos(sweep25), 2.0) - 1.186 * cos(sweep25) + 0.858;
                    RLS_High = -1.6538 * pow(cos(sweep25), 3.0) + 2.865 * pow(cos(sweep25), 2.0) - 0.886 * cos(sweep25) + 0.934;
                }
                else if (i == 2)
                {
                    RLS_Low = -1.6538 * pow(cos(sweep25), 3.0) + 2.865 * pow(cos(sweep25), 2.0) - 0.886 * cos(sweep25) + 0.934;
                    RLS_High = -1.8316 * pow(cos(sweep25), 3.0) + 3.3944 * pow(cos(sweep25), 2.0) - 1.3596 * cos(sweep25) + 1.1567;
                }

                Rls = x * (RLS_High - RLS_Low) + RLS_Low;
            }
        }

        ff = (1.0 + (L * toc) + 100.0 * pow(toc, 4.0)) * Rls;
        break;

    case vsp::FF_W_SCHEMENSKY_6_SERIES_AF:
        ff = 1.0 + (1.44 * toc) + (2.0 * pow(toc, 2.0));
        break;

    case vsp::FF_W_SCHEMENSKY_4_SERIES_AF:
        ff = 1.0 + (1.68 * toc) + (3.0 * pow(toc, 2.0));
        break;

        //case vsp::FF_W_SCHEMENSKY_SUPERCRITICAL_AF :
        //    geo_ffName.push_back("Schemensky Supercritical AF");
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

        Fstar = 1.0 + (3.3 * toc) - (0.008 * pow(toc, 2.0)) + (27.0 * pow(toc, 3.0));

        ff = ((Fstar - 1.0) * pow(cos(sweep50), 2.0)) + 1.0;
        break;

    case vsp::FF_W_JENKINSON_TAIL:

        Fstar = 1.0 + 3.52 * toc;

        ff = ((Fstar - 1.0) * pow(cos(sweep50), 2.0)) + 1.0;
        break;

    default:
        ff = 0;
    }
    return ff;
}

double ParasiteDragMgrSingleton::CalcFFBody(double longF, double FR, int ff_case, double ref_leng, double max_x_area)
{
    double ff;
    switch (ff_case)
    {
    case vsp::FF_B_MANUAL:
        ff = 1.0;
        break;

    case vsp::FF_B_SCHEMENSKY_FUSE:
        ff = 1.0 + (60.0 / pow(FR, 3.0)) + (0.0025 * FR);
        break;

    case vsp::FF_B_SCHEMENSKY_NACELLE:
        ff = 1.0 + 0.35 / FR;
        break;

    case vsp::FF_B_HOERNER_STREAMBODY:
        ff = 1.0 + (1.5 / pow(longF, 1.5)) +
            (7.0 / pow(longF, 3.0));
        break;

    case vsp::FF_B_TORENBEEK:
        ff = 1.0 + (2.2 / pow(longF, 1.5)) +
            (3.8 / pow(longF, 3.0));
        break;

    case vsp::FF_B_SHEVELL:
        ff = 1.0 + (2.8 / pow(longF, 1.5)) +
            (3.8 / pow(longF, 3.0));
        break;

    case vsp::FF_B_JENKINSON_FUSE:
        double Lambda;
        Lambda = ref_leng / (pow((4.0 / PI) * max_x_area, 0.5));

        ff = 1.0 + (2.2 / pow(Lambda, 1.5)) -
            (0.9 / pow(Lambda, 3.0));
        break;

    case vsp::FF_B_JENKINSON_WING_NACELLE:
        ff = 1.25;
        break;

    case vsp::FF_B_JENKINSON_AFT_FUSE_NACELLE:
        ff = 1.5;
        break;

    case vsp::FF_B_JOBE:
        ff = 1.02 + (1.5 / (pow(longF, 1.5))) + (7.0 / (0.6 * pow(longF, 3.0) * (1.0 - pow(m_Mach(), 3.0))));
        break;

    default:
        ff = 0.0;
        break;
    }
    return ff;
}

void ParasiteDragMgrSingleton::SetActiveGeomVec()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    vector <string> geomVec = veh->GetGeomSet(m_SetChoice());

    m_PDGeomIDVec.clear();
    for (int i = 0; i < geomVec.size(); i++)
    {
        Geom* geom = veh->FindGeom(geomVec[i]);
        if (geom->GetType().m_Type != HINGE_GEOM_TYPE && geom->GetType().m_Type != BLANK_GEOM_TYPE)
        {
            if (geom->GetSurfPtr(0)->GetSurfType() != vsp::DISK_SURF)
            {
                m_PDGeomIDVec.push_back(geomVec[i]);
            }
        }
    }
}

void ParasiteDragMgrSingleton::SetFreestreamParms()
{
    m_Temp.Set(m_Atmos.GetTemp());
    m_Pres.Set(m_Atmos.GetPres());
    m_Rho.Set(m_Atmos.GetDensity());
    m_DynaVisc.Set(m_Atmos.GetDynaVisc());
}

double ParasiteDragMgrSingleton::GetLrefSigFig()
{
    double lrefmag;
    if (!geo_lref.empty())
    {
        vector<double>::const_iterator it = max_element(geo_lref.begin(), geo_lref.end());
        lrefmag = mag(*it);
    }
    else
    {
        lrefmag = 1;
    }

    if (lrefmag > 1)
    {
        return 1;
    }
    else if (lrefmag == 1)
    {
        return 2;
    }
    else
    {
        return 3;
    }
}

void ParasiteDragMgrSingleton::Update()
{
    UpdateRefWing();

    UpdateTempLimits();
    UpdateAtmos();

    UpdateParmActivity();

    SetFreestreamParms();
}

void ParasiteDragMgrSingleton::UpdateWettedAreaTotals()
{
}

void ParasiteDragMgrSingleton::UpdateRefWing()
{
    // Update Reference Area Section 
    if (m_RefFlag() == MANUAL_REF)
    { // Allow Manual Input
        m_Sref.Activate();
    }
    else
    { // Pull from existing geometry
        Geom* refgeom = VehicleMgr.GetVehicle()->FindGeom(m_RefGeomID);

        if (refgeom)
        {
            if (refgeom->GetType().m_Type == MS_WING_GEOM_TYPE)
            {
                WingGeom* refwing = (WingGeom*)refgeom;
                m_Sref.Set(refwing->m_TotalArea());

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
    if (m_FreestreamType() == vsp::ATMOS_TYPE_US_STANDARD_1976)
    {
        m_Atmos.USStandardAtmosphere1976(m_Hinf(), m_DeltaT(), m_AltLengthUnit(), m_TempUnit.Get(), m_PresUnit());
        m_Atmos.UpdateMach(vinf, m_SpecificHeatRatio(), m_TempUnit(), m_VinfUnitType());
    }
    else if (m_FreestreamType() == vsp::ATMOS_TYPE_HERRINGTON_1966)
    {
        m_Atmos.USAF1966(m_Hinf(), m_DeltaT(), m_AltLengthUnit(), m_TempUnit.Get(), m_PresUnit());
        m_Atmos.UpdateMach(vinf, m_SpecificHeatRatio(), m_TempUnit(), m_VinfUnitType());
    }
    else if (m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_P_R)
    {
        m_Atmos.SetManualQualities(vinf, temp, pres, rho, dynavisc, m_SpecificHeatRatio(),
            m_AltLengthUnit(), m_VinfUnitType(), m_TempUnit(), m_FreestreamType());
    }
    else if (m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_P_T)
    {
        m_Atmos.SetManualQualities(vinf, temp, pres, rho, dynavisc, m_SpecificHeatRatio(),
            m_AltLengthUnit(), m_VinfUnitType(), m_TempUnit(), m_FreestreamType());
    }
    else if (m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_R_T)
    {
        m_Atmos.SetManualQualities(vinf, temp, pres, rho, dynavisc, m_SpecificHeatRatio(),
            m_AltLengthUnit(), m_VinfUnitType(), m_TempUnit(), m_FreestreamType());
    }
    else if (m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_RE_L)
    {
        vinf = m_Atmos.GetMach() * m_Atmos.GetSoundSpeed();
        UpdateVinf(m_VinfUnitType());
    }

    if (m_FreestreamType() != vsp::ATMOS_TYPE_MANUAL_RE_L)
    {
        m_Hinf.Set(m_Atmos.GetAlt());
        m_DeltaT.Set(m_Atmos.GetDeltaT());
        m_Temp.Set(m_Atmos.GetTemp());
        m_Pres.Set(m_Atmos.GetPres());
        m_Rho.Set(m_Atmos.GetDensity());
        m_Mach.Set(m_Atmos.GetMach());

        if (m_AltLengthUnit() == vsp::PD_UNITS_IMPERIAL)
        {
            vinf = ConvertVelocity(vinf, m_VinfUnitType.Get(), vsp::V_UNIT_FT_S);
        }
        else if (m_AltLengthUnit() == vsp::PD_UNITS_METRIC)
        {
            vinf = ConvertVelocity(vinf, m_VinfUnitType.Get(), vsp::V_UNIT_M_S);
        }

        m_KineVisc = m_Atmos.GetDynaVisc() / m_Rho();

        LqRe = m_KineVisc() / vinf;

        if (m_AltLengthUnit() == vsp::PD_UNITS_IMPERIAL)
        {
            vinf = ConvertLength(LqRe, vsp::LEN_FT, m_LengthUnit());
        }
        else if (m_AltLengthUnit() == vsp::PD_UNITS_METRIC)
        {
            vinf = ConvertLength(LqRe, vsp::LEN_M, m_LengthUnit());
        }

        m_ReqL.Set(1.0 / LqRe);
    }
}

void ParasiteDragMgrSingleton::UpdateVinf(int newunit)
{
    double new_vinf;
    if (m_VinfUnitType() == vsp::V_UNIT_KEAS)
    {
        m_Vinf *= sqrt(1.0 / m_Atmos.GetDensityRatio());
    }
    new_vinf = ConvertVelocity(m_Vinf(), m_VinfUnitType(), newunit);
    if (newunit == vsp::V_UNIT_KEAS)
    {
        new_vinf /= sqrt(1.0 / m_Atmos.GetDensityRatio());
    }
    new_vinf = ConvertVelocity(m_Vinf(), m_VinfUnitType(), newunit);
    m_Vinf.Set(new_vinf);
    m_VinfUnitType.Set(newunit);
}

void ParasiteDragMgrSingleton::UpdateAlt(int newunit)
{
    double new_alt;
    if (newunit == vsp::PD_UNITS_IMPERIAL && m_AltLengthUnit() == vsp::PD_UNITS_METRIC)
    {
        new_alt = ConvertLength(m_Hinf(), vsp::LEN_M, vsp::LEN_FT);
    }
    else if (newunit == vsp::PD_UNITS_METRIC && m_AltLengthUnit() == vsp::PD_UNITS_IMPERIAL)
    {
        new_alt = ConvertLength(m_Hinf(), vsp::LEN_FT, vsp::LEN_M);
    }

    m_Hinf.Set(new_alt);
    m_AltLengthUnit.Set(newunit);
}

void ParasiteDragMgrSingleton::UpdateAltLimits()
{
    switch (m_AltLengthUnit())
    {
    case vsp::PD_UNITS_IMPERIAL:
        m_Hinf.SetUpperLimit(278385.83);
        break;

    case vsp::PD_UNITS_METRIC:
        m_Hinf.SetUpperLimit(84852.0);
        break;

    default:
        break;
    }
}

void ParasiteDragMgrSingleton::UpdateTemp(int newunit)
{
    double new_temp = ConvertTemperature(m_Temp(), m_TempUnit(), newunit);
    m_Temp.Set(new_temp);
    m_TempUnit.Set(newunit);
}

void ParasiteDragMgrSingleton::UpdateTempLimits()
{
    switch (m_TempUnit())
    {
    case vsp::TEMP_UNIT_C:
        m_Temp.SetLowerLimit(-273.15);
        break;

    case vsp::TEMP_UNIT_F:
        m_Temp.SetLowerLimit(-459.666);
        break;

    case vsp::TEMP_UNIT_K:
        m_Temp.SetLowerLimit(0.0);
        break;

    case vsp::TEMP_UNIT_R:
        m_Temp.SetLowerLimit(0.0);
        break;
    }
}

void ParasiteDragMgrSingleton::UpdatePres(int newunit)
{
    double new_pres = ConvertPressure(m_Pres(), m_PresUnit(), newunit);
    m_Pres.Set(new_pres);
    m_PresUnit.Set(newunit);
}

void ParasiteDragMgrSingleton::UpdateParmActivity()
{
    // Activate/Deactivate Appropriate Flow Condition Parameters
    DeactivateParms();

    if (m_FreestreamType() == vsp::ATMOS_TYPE_US_STANDARD_1976 || m_FreestreamType() == vsp::ATMOS_TYPE_HERRINGTON_1966)
    {
        m_Vinf.Activate();
        m_Hinf.Activate();
    }
    else if (m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_P_R)
    {
        m_Vinf.Activate();
        m_Pres.Activate();
        m_Rho.Activate();
        m_SpecificHeatRatio.Activate();
    }
    else if (m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_P_T)
    {
        m_Vinf.Activate();
        m_Temp.Activate();
        m_Pres.Activate();
        m_SpecificHeatRatio.Activate();
    }
    else if (m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_R_T)
    {
        m_Vinf.Activate();
        m_Temp.Activate();
        m_Rho.Activate();
        m_SpecificHeatRatio.Activate();
    }
    else if (m_FreestreamType() == vsp::ATMOS_TYPE_MANUAL_RE_L)
    {
        m_ReqL.Activate();
        m_Mach.Activate();
        m_SpecificHeatRatio.Activate();
    }
}

// ========================================================== //
// =================== General Methods ====================== //
// ========================================================== //

void ParasiteDragMgrSingleton::ClearInputVectors()
{
    geo_geomID.clear();
    geo_subsurfID.clear();
    geo_label.clear();
    geo_percLam.clear();
    geo_shapeType.clear();
    geo_ffIn.clear();
    geo_Q.clear();
    geo_Roughness.clear();
    geo_TeTwRatio.clear();
    geo_TawTwRatio.clear();
    geo_surfNum.clear();
}

void ParasiteDragMgrSingleton::ClearOutputVectors()
{
    geo_swet.clear();
    geo_lref.clear();
    geo_Re.clear();
    geo_cf.clear();
    geo_fineRat.clear();
    geo_ffType.clear();
    geo_ffName.clear();
    geo_ffOut.clear();
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

    vector <string> newIDVec = veh->GetGeomSet(m_SetChoice());
    vector < string > newVecToCompare;
    for (int i = 0; i < newIDVec.size(); i++)
    {
        if (veh->FindGeom(newIDVec[i])->GetType().m_Type != MESH_GEOM_TYPE &&
            veh->FindGeom(newIDVec[i])->GetType().m_Type != BLANK_GEOM_TYPE &&
            veh->FindGeom(newIDVec[i])->GetType().m_Type != HINGE_GEOM_TYPE)
        {
            if (veh->FindGeom(newIDVec[i])->GetSurfPtr(0)->GetSurfType() != vsp::DISK_SURF)
            {
                newVecToCompare.push_back(newIDVec[i]);
            }
        }
    }

    int temprowsize = 0;
    for (int i = 0; i < newVecToCompare.size(); i++)
    {
        Geom* geom = veh->FindGeom(newVecToCompare[i]);
        if (geom)
        {
            temprowsize += geom->GetNumTotalSurfs();
            for (size_t j = 0; j < geom->GetSubSurfVec().size(); ++j)
            {
                for (size_t k = 0; k < geom->GetNumSymmCopies(); ++k)
                {
                    ++temprowsize;
                }
            }
        }
    }

    if (temprowsize == m_RowSize && newVecToCompare == m_PDGeomIDVec)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool ParasiteDragMgrSingleton::IsNotZeroLineItem(int index)
{
    return false;
}

void ParasiteDragMgrSingleton::RefreshDegenGeom()
{    // Check For Similarity in Geom Set
    if (!IsSameGeomSet())
    {
        // Reset Geo Vecs & Clear Degen Geom
        VehicleMgr.GetVehicle()->ClearDegenGeom();
        m_DegenGeomVec.clear();
        ClearInputVectors();
        ClearOutputVectors();

        // Set New Active Geom Vec
        SetActiveGeomVec();
    }
}
