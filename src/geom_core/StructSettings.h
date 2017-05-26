//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// StructSettings.h
//////////////////////////////////////////////////////////////////////


#if !defined(StructSettings__INCLUDED_)
#define StructSettings__INCLUDED_

#include "Vec2d.h"
#include "Vec3d.h"
#include "DrawObj.h"
#include "APIDefines.h"

#include "MeshCommonSettings.h"

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
class StructSettings : public MeshCommonSettings
{
public:
    StructSettings();
    virtual ~StructSettings();

    string GetExportFileName( int type );
    void SetExportFileName( const string &fn, int type );
    void ResetExportFileNames();
    void ResetExportFileNames( string basename );

    BoolParm* GetExportFileFlag( int type );
    void SetAllFileExportFlags( bool flag );
    void SetFileExportFlag( int type, bool flag );
    vector < string > GetExportFileNames();

    BoolParm m_ExportFileFlags[vsp::NUM_FEA_FILE_NAMES];
    IntParm m_NumEvenlySpacedPart;
    Parm m_MultiSliceSpacing;
    IntParm m_MultSliceIncludedElements;
    BoolParm m_DrawFeaPartsFlag;
    BoolParm m_DrawNodesFlag;
    BoolParm m_DrawElementOrientVecFlag;

    virtual int GetMultPropertyIndex()
    {
        return m_MultPropertyIndex;
    }
    virtual void SetMultPropertyIndex( int index )
    {
        m_MultPropertyIndex = index;
    }

    virtual int GetMultCapPropertyIndex()
    {
        return m_MultCapPropertyIndex;
    }
    virtual void SetMultCapPropertyIndex( int index )
    {
        m_MultCapPropertyIndex = index;
    }

protected:

    // These file names do not get written to file.  They are reset each time
    // the file name is set (save/save as/open).  There is no way to have good
    // default behavior based on the main file name -- and to use the user-set
    // file names.
    string m_ExportFileNames[vsp::NUM_FEA_FILE_NAMES];

    int m_MultPropertyIndex;
    int m_MultCapPropertyIndex;

};

#endif



