//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// StructSettings.h
//////////////////////////////////////////////////////////////////////


#if !defined(StructSettings__INCLUDED_)
#define StructSettings__INCLUDED_

#include "APIDefines.h"
#include "MeshCommonSettings.h"

#include <assert.h>

using namespace std;

//////////////////////////////////////////////////////////////////////
class StructSettings : public MeshCommonSettings
{
public:
    StructSettings();
    virtual ~StructSettings();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    string GetExportFileName( int type );
    void SetExportFileName( const string &fn, int type );
    void ResetExportFileNames();
    void ResetExportFileNames( string basename );

    BoolParm* GetExportFileFlag( int type );
    void SetAllFileExportFlags( bool flag );
    void SetFileExportFlag( int type, bool flag );
    vector < string > GetExportFileNames();

    BoolParm m_ExportFileFlags[vsp::FEA_NUM_FILE_NAMES];
    IntParm m_NumEvenlySpacedPart;
    Parm m_MultiSliceSpacing;
    IntParm m_MultSliceIncludedElements;
    BoolParm m_DrawNodesFlag;
    BoolParm m_DrawElementOrientVecFlag;

protected:

    // These file names do not get written to file.  They are reset each time
    // the file name is set (save/save as/open).  There is no way to have good
    // default behavior based on the main file name -- and to use the user-set
    // file names.
    string m_ExportFileNames[vsp::FEA_NUM_FILE_NAMES];

};

#endif



