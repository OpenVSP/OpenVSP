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

    virtual void SetIntersectSubSurfs( bool f )
    {
        m_IntersectSubSurfs = f;
    }
    virtual bool GetIntersectSubSurfs()
    {
        return m_IntersectSubSurfs();
    }

    virtual bool GetHalfMeshFlag()
    {
        return m_HalfMeshFlag.Get();
    }
    virtual void SetHalfMeshFlag( bool f )
    {
        m_HalfMeshFlag = f;
    }

    string GetExportFileName( int type );
    void SetExportFileName( const string &fn, int type );
    void ResetExportFileNames();
    void ResetExportFileNames( string basename );

    BoolParm* GetExportFileFlag( int type );
    void SetAllFileExportFlags( bool flag );
    void SetFileExportFlag( int type, bool flag );

    BoolParm m_DrawMeshFlag;
    BoolParm m_DrawBadFlag;
    BoolParm m_ColorTagsFlag;

    BoolParm m_IntersectSubSurfs;

    BoolParm m_HalfMeshFlag;

    BoolParm m_ExportFileFlags[vsp::NUM_FEA_FILE_NAMES];

protected:

    // These file names do not get written to file.  They are reset each time
    // the file name is set (save/save as/open).  There is no way to have good
    // default behavior based on the main file name -- and to use the user-set
    // file names.
    string m_ExportFileNames[vsp::NUM_FEA_FILE_NAMES];

};

#endif



