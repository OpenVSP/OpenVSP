//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ParmUndo.h"

//==== Constructor ====//
ParmUndo::ParmUndo()
{
    m_ParmID = string( "NONE" );
    m_Val = 0.0;
    m_LastVal = 0.0;
}

ParmUndo::ParmUndo( Parm* parm_ptr )
{
    m_ParmID = parm_ptr->GetID();
    m_Val = parm_ptr->Get();
    m_LastVal = parm_ptr->GetLastVal();
}


