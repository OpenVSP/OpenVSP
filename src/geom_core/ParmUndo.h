//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmUndo.h: Parm data needed for undo
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(PARMUNDO__INCLUDED_)
#define PARMUNDO__INCLUDED_

#include "Parm.h"
#include "Geom.h"

#include <string>

//==== Undo Data ====//
class ParmUndo
{
public:

    ParmUndo();
    ParmUndo( Parm* parm_ptr );

    string GetID()
    {
        return m_ParmID;
    }
    double GetLastVal()
    {
        return m_LastVal;
    }

protected:

    string m_ParmID;
    double m_Val;
    double m_LastVal;

};


#endif // !defined(PARMUNDO__INCLUDED_)
