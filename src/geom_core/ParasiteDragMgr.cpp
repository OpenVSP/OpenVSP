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

    // Air Qualities Parms
    m_Mach.Init("Mach", groupname, this, 0.0, 0.0, 1000.0);
    m_Mach.SetDescript("Mach Number for Current Flight Condition");

    m_Temp.Init("Temp", groupname, this, 288.15, -459.67, 1e12);
    m_Temp.SetDescript("Temperature");

    m_Vinf.Init("Vinf", groupname, this, 500.0, 0.0, 1e12);
    m_Vinf.SetDescript("Free Stream Velocity");

    m_Hinf.Init("Alt", groupname, this, 20000.0, 0.0, 271823.3);
    m_Hinf.SetDescript("Physical Altitude from Sea Level");
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

    UpdateParmActivity();
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

void ParasiteDragMgrSingleton::UpdateAltLimits()
{
}

void ParasiteDragMgrSingleton::UpdateTempLimits()
{
}

void ParasiteDragMgrSingleton::UpdateParmActivity()
{
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
    geo_fineRat.clear();
}

void ParasiteDragMgrSingleton::DeactivateParms()
{
    m_Vinf.Deactivate();
    m_Hinf.Deactivate();
    m_Temp.Deactivate();
    m_Mach.Deactivate();
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
