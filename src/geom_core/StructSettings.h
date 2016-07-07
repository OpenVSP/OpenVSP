//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// StructSettings.h
//////////////////////////////////////////////////////////////////////


#if !defined(StructSettings__INCLUDED_)
#define StructSettings__INCLUDED_

#include "GridDensity.h"
#include "Vehicle.h"

#include "Vec2d.h"
#include "Vec3d.h"
#include "DrawObj.h"
#include "APIDefines.h"

#include <assert.h>

#include <set>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;

//////////////////////////////////////////////////////////////////////
class StructSettings : public ParmContainer
{
public:
	StructSettings();
    virtual ~StructSettings();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void ReadV2File( xmlNodePtr &root );

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual string GetFarGeomID()
    {
        return m_FarGeomID;
    }
    virtual void SetFarGeomID( string gid )
    {
        m_FarGeomID = gid;
    }
    virtual bool GetFarMeshFlag()
    {
        return m_FarMeshFlag.Get();
    }
    virtual void SetFarMeshFlag( bool f )
    {
        m_FarMeshFlag = f;
    }
    virtual bool GetFarCompFlag()
    {
        return m_FarCompFlag.Get();
    }
    virtual void SetFarCompFlag( bool f )
    {
        m_FarCompFlag = f;
    }
    virtual bool GetFarManLocFlag()
    {
        return m_FarManLocFlag.Get();
    }
    virtual void SetFarManLocFlag( bool f )
    {
        m_FarManLocFlag = f;
    }

    //Symmetry Plane Item Quad BoolParm
    virtual bool GetSymSplittingOnFlag()
    {
        return m_SymSplittingOnFlag.Get();
    }
    virtual void SetSymSplittingOnFlag(bool isOn)
    {
        m_SymSplittingOnFlag.Set(isOn);
    }

    virtual bool GetFarAbsSizeFlag()
    {
        return m_FarAbsSizeFlag.Get();
    }
    virtual void SetFarAbsSizeFlag( bool f )
    {
        m_FarAbsSizeFlag = f;
    }
    virtual bool GetHalfMeshFlag()
    {
        return m_HalfMeshFlag.Get();
    }
    virtual void SetHalfMeshFlag( bool f )
    {
        m_HalfMeshFlag = f;
    }
    virtual void SetWakeScale( double s )
    {
        m_WakeScale = s;
    }
    virtual double GetWakeScale()
    {
        return m_WakeScale();
    }
    virtual void SetWakeAngle( double a )
    {
        m_WakeAngle = a;
    }
    virtual double GetWakeAngle()
    {
        return m_WakeAngle();
    }
    virtual void SetIntersectSubSurfs( bool f )
    {
        m_IntersectSubSurfs = f;
    }
    virtual bool GetIntersectSubSurfs()
    {
        return m_IntersectSubSurfs();
    }

    string GetExportFileName( int type );
    void SetExportFileName( const string &fn, int type );
    void ResetExportFileNames();
    void ResetExportFileNames( string basename );

    BoolParm* GetExportFileFlag( int type );
    void SetAllFileExportFlags( bool flag );
    void SetFileExportFlag( int type, bool flag );

    //Symmetry Plane Boolean Items
    BoolParm m_SymSplittingOnFlag;

    BoolParm m_FarMeshFlag;
    BoolParm m_FarCompFlag;
    BoolParm m_FarManLocFlag;
    BoolParm m_FarAbsSizeFlag;
    BoolParm m_HalfMeshFlag;

    string m_FarGeomID;

    FractionParm m_FarXScale;
    FractionParm m_FarYScale;
    FractionParm m_FarZScale;

    Parm m_FarLength;
    Parm m_FarWidth;
    Parm m_FarHeight;

    Parm m_FarXLocation;
    Parm m_FarYLocation;
    Parm m_FarZLocation;

    Parm m_WakeScale;
    Parm m_WakeAngle;

    BoolParm m_DrawMeshFlag;
    BoolParm m_DrawSourceFlag;
    BoolParm m_DrawFarFlag;
    BoolParm m_DrawFarPreFlag;
    BoolParm m_DrawBadFlag;
    BoolParm m_DrawSymmFlag;
    BoolParm m_DrawWakeFlag;
    BoolParm m_ColorTagsFlag;

    BoolParm m_IntersectSubSurfs;

    IntParm m_SelectedSetIndex;

    BoolParm m_ExportFileFlags[vsp::NUM_FEA_FILE_NAMES];
    BoolParm m_XYZIntCurveFlag;

protected:

    // These file names do not get written to file.  They are reset each time
    // the file name is set (save/save as/open).  There is no way to have good
    // default behavior based on the main file name -- and to use the user-set
    // file names.
    string m_ExportFileNames[vsp::NUM_FEA_FILE_NAMES];

};

#endif



