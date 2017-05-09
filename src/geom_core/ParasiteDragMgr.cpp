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
                    geo_Q.push_back(geom->m_Q());
                    geo_Roughness.push_back(geom->m_Roughness());
                    geo_TeTwRatio.push_back(geom->m_TeTwRatio());
                    geo_TawTwRatio.push_back(geom->m_TawTwRatio());
                }

                geo_shapeType.push_back(geom->GetSurfPtr(j)->GetSurfType()); // Form Factor Shape Type

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
                    geo_Q.push_back(geo_Q[geo_Q.size() - 1]); // TODO: Add Q to SubSurf
                    geo_Roughness.push_back(geo_Roughness[geo_Roughness.size() - 1]); //TODO: Add Roughness to SubSurf
                    geo_TeTwRatio.push_back(geo_TeTwRatio[geo_TeTwRatio.size() - 1]);
                    geo_TawTwRatio.push_back(geo_TawTwRatio[geo_TawTwRatio.size() - 1]);
                    geo_surfNum.push_back(k);

                    geo_shapeType.push_back(geom->GetSurfPtr(k)->GetSurfType()); // Form Factor Shape Type

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
    geo_fineRat.clear();
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
