//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParasiteDragMgr.cpp: ParasiteDrag Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "Vehicle.h"
#include "VehicleMgr.h"
#include "ParasiteDragMgr.h"


//==== Constructor ====//
ParasiteDragMgrSingleton::ParasiteDragMgrSingleton() : ParmContainer()
{
}

void ParasiteDragMgrSingleton::Renew()
{
}

void ParasiteDragMgrSingleton::SetDefaultStruct()
{
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
}

void ParasiteDragMgrSingleton::LoadMainTableUserInputs()
{
}

void ParasiteDragMgrSingleton::SetupFullCalculation()
{
}

int ParasiteDragMgrSingleton::CalcRowSize()
{
    return 0;
}

void ParasiteDragMgrSingleton::Calculate_Swet()
{
}

void ParasiteDragMgrSingleton::Calculate_Lref()
{
}

void ParasiteDragMgrSingleton::CalcReferenceBodyLength(int index)
{
}

void ParasiteDragMgrSingleton::CalcReferenceChord(int index)
{
}

void ParasiteDragMgrSingleton::Calculate_fineRat()
{
}

void ParasiteDragMgrSingleton::SetActiveGeomVec()
{
}

double ParasiteDragMgrSingleton::GetLrefSigFig()
{
    return 0.0;
}

void ParasiteDragMgrSingleton::Update()
{
}

void ParasiteDragMgrSingleton::UpdateWettedAreaTotals()
{
}

void ParasiteDragMgrSingleton::UpdateRefWing()
{
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

void ParasiteDragMgrSingleton::ClearInputVectors()
{
}

void ParasiteDragMgrSingleton::ClearOutputVectors()
{
}

void ParasiteDragMgrSingleton::DeactivateParms()
{
}

bool ParasiteDragMgrSingleton::IsSameGeomSet()
{
    return false;
}

bool ParasiteDragMgrSingleton::IsNotZeroLineItem(int index)
{
    return false;
}

void ParasiteDragMgrSingleton::RefreshDegenGeom()
{
}
