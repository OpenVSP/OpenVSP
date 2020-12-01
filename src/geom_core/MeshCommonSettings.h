//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// MeshCommonSettings.h
//////////////////////////////////////////////////////////////////////

#if !defined(MESHCOMMONSETTINGS__INCLUDED_)
#define MESHCOMMONSETTINGS__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"
#include "APIDefines.h"

//////////////////////////////////////////////////////////////////////
class MeshCommonSettings : public ParmContainer
{
public:
    MeshCommonSettings();
    virtual ~MeshCommonSettings();

    virtual void InitCommonParms();

    //Symmetry Plane Boolean Items
    BoolParm m_SymSplittingOnFlag;

    BoolParm m_DrawMeshFlag;
    BoolParm m_ColorTagsFlag;

    BoolParm m_DrawSourceWakeFlag;

    BoolParm m_DrawBorderFlag;
    BoolParm m_DrawIsectFlag;
    BoolParm m_DrawRawFlag;
    BoolParm m_DrawBinAdaptFlag;
    BoolParm m_DrawCurveFlag;
    BoolParm m_DrawPntsFlag;

    Parm m_RelCurveTol;
    BoolParm m_ExportRawFlag;

    BoolParm m_IntersectSubSurfs;

    BoolParm m_FarMeshFlag;
    BoolParm m_FarCompFlag;
    BoolParm m_HalfMeshFlag;

    BoolParm m_DemoteSurfsCubicFlag;
    Parm m_CubicSurfTolerance;

    BoolParm m_ConvertToQuadsFlag;
    BoolParm m_HighOrderElementFlag;

    Parm m_STEPTol;
    BoolParm m_STEPMergePoints;
    IntParm m_STEPRepresentation;

    IntParm m_CADLenUnit;
    BoolParm m_CADLabelID;
    BoolParm m_CADLabelName;
    BoolParm m_CADLabelSurfNo;
    BoolParm m_CADLabelSplitNo;
    IntParm m_CADLabelDelim;

};

//////////////////////////////////////////////////////////////////////
class IntersectSettings : public MeshCommonSettings
{
public:
    IntersectSettings();
    virtual ~IntersectSettings();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    string GetExportFileName( int type );
    void SetExportFileName( const string &fn, int type );
    void ResetExportFileNames( const string& basename );

    BoolParm* GetExportFileFlag( int type );
    void SetAllFileExportFlags( bool flag );
    void SetFileExportFlag( int type, bool flag );
    vector < string > GetExportFileNames();

    vector <BoolParm> m_ExportFileFlags;
    IntParm m_SelectedSetIndex;
    IntParm m_SelectedDegenSetIndex;

    BoolParm m_XYZIntCurveFlag;

protected:

    // These file names do not get written to file.  They are reset each time
    // the file name is set (save/save as/open).  There is no way to have good
    // default behavior based on the main file name -- and to use the user-set
    // file names.
    vector < string > m_ExportFileNames;

};

//////////////////////////////////////////////////////////////////////
class CfdMeshSettings : public MeshCommonSettings
{
public:
    CfdMeshSettings();
    virtual ~CfdMeshSettings();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void ReadV2File( xmlNodePtr &root );

    virtual string GetFarGeomID()
    {
        return m_FarGeomID;
    }
    virtual void SetFarGeomID( const string& gid )
    {
        m_FarGeomID = gid;
    }

    string GetExportFileName( int type );
    void SetExportFileName( const string &fn, int type );
    void ResetExportFileNames( const string& basename );
    vector < string > GetExportFileNames();

    BoolParm* GetExportFileFlag( int type );
    void SetAllFileExportFlags( bool flag );
    void SetFileExportFlag( int type, bool flag );

    BoolParm m_FarManLocFlag;
    BoolParm m_FarAbsSizeFlag;

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

    BoolParm m_DrawFarFlag;
    BoolParm m_DrawFarPreFlag;
    BoolParm m_DrawSymmFlag;
    BoolParm m_DrawBadFlag;
    BoolParm m_DrawWakeFlag;

    IntParm m_SelectedSetIndex;
    IntParm m_SelectedDegenSetIndex;

    BoolParm m_ExportFileFlags[vsp::CFD_NUM_FILE_NAMES];
    BoolParm m_XYZIntCurveFlag;

protected:

    // These file names do not get written to file.  They are reset each time
    // the file name is set (save/save as/open).  There is no way to have good
    // default behavior based on the main file name -- and to use the user-set
    // file names.
    vector < string > m_ExportFileNames;

};

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
    void ResetExportFileNames( const string& basename );

    BoolParm* GetExportFileFlag( int type );
    void SetAllFileExportFlags( bool flag );
    void SetFileExportFlag( int type, bool flag );
    vector < string > GetExportFileNames();

    BoolParm m_ExportFileFlags[vsp::FEA_NUM_FILE_NAMES];
    IntParm m_NumEvenlySpacedPart;
    Parm m_MultiSliceSpacing;
    IntParm m_MultSliceIncludedElements;
    BoolParm m_DrawNodesFlag;
    BoolParm m_DrawBCNodesFlag;
    BoolParm m_DrawElementOrientVecFlag;
    BoolParm m_XYZIntCurveFlag;

    IntParm m_NodeOffset;
    IntParm m_ElementOffset;

    BoolParm m_BeamPerElementNormal;

protected:

    // These file names do not get written to file.  They are reset each time
    // the file name is set (save/save as/open).  There is no way to have good
    // default behavior based on the main file name -- and to use the user-set
    // file names.
    vector < string > m_ExportFileNames;

};

//////////////////////////////////////////////////////////////////////
class AssemblySettings : public ParmContainer
{
public:
    AssemblySettings();
    virtual ~AssemblySettings();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    string GetExportFileName( int type );
    void SetExportFileName( const string &fn, int type );
    void ResetExportFileNames( const string& basename );

    BoolParm* GetExportFileFlag( int type );
    void SetAllFileExportFlags( bool flag );
    void SetFileExportFlag( int type, bool flag );
    vector < string > GetExportFileNames();

    BoolParm m_DrawAsMeshFlag;
    BoolParm m_DrawMeshFlag;
    BoolParm m_ColorTagsFlag;

    BoolParm m_ExportFileFlags[vsp::FEA_NUM_FILE_NAMES];
    BoolParm m_DrawNodesFlag;
    BoolParm m_DrawBCNodesFlag;
    BoolParm m_DrawElementOrientVecFlag;

protected:

    // These file names do not get written to file.  They are reset each time
    // the file name is set (save/save as/open).  There is no way to have good
    // default behavior based on the main file name -- and to use the user-set
    // file names.
    vector < string > m_ExportFileNames;

};

#endif // !defined(MESHCOMMONSETTINGS__INCLUDED_)
