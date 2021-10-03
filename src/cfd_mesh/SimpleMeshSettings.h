// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// SimpleMeshSettings.h
// Justin Gravett
//////////////////////////////////////////////////////////////////////

#if !defined(SIMPLEMESHSETTINGS_SIMPLEMESHSETTINGS__INCLUDED_)
#define SIMPLEMESHSETTINGS_SIMPLEMESHSETTINGS__INCLUDED_

#include "Defines.h"
#include "APIDefines.h"
#include "Vehicle.h"
#include "MeshCommonSettings.h"
#include "GridDensity.h"

using namespace std;

class SimpleMeshCommonSettings
{
public:
    SimpleMeshCommonSettings();
    virtual ~SimpleMeshCommonSettings();

    virtual void CopyFrom( MeshCommonSettings* settings );

    bool m_SymSplittingOnFlag;

    bool m_DrawMeshFlag;
    bool m_ColorTagsFlag;

    bool m_DrawSourceWakeFlag;

    bool m_DrawBorderFlag;
    bool m_DrawIsectFlag;
    bool m_DrawRawFlag;
    bool m_DrawBinAdaptFlag;
    bool m_DrawCurveFlag;
    bool m_DrawPntsFlag;

    double m_RelCurveTol;
    bool m_ExportRawFlag;
    double m_ExportRelCurveTol;

    bool m_IntersectSubSurfs;

    bool m_DemoteSurfsCubicFlag;
    double m_CubicSurfTolerance;

    bool m_FarMeshFlag;
    bool m_FarCompFlag;
    bool m_HalfMeshFlag;

    int m_SelectedSetIndex;

    bool m_XYZIntCurveFlag;

    double m_STEPTol;
    bool m_STEPMergePoints;
    int m_STEPRepresentation;

    int m_CADLenUnit;
    bool m_CADLabelID;
    bool m_CADLabelName;
    bool m_CADLabelSurfNo;
    bool m_CADLabelSplitNo;
    int m_CADLabelDelim;

protected:

};

class SimpleIntersectSettings : public SimpleMeshCommonSettings
{
public:
    SimpleIntersectSettings();
    virtual ~SimpleIntersectSettings();

    virtual void CopyFrom( IntersectSettings* settings );

    string GetExportFileName( int type );
    bool GetExportFileFlag( int type );

    vector < bool > m_ExportFileFlags;

protected:

    vector < string > m_ExportFileNames;

};

class SimpleCfdMeshSettings : public SimpleMeshCommonSettings
{
public:
    SimpleCfdMeshSettings();
    virtual ~SimpleCfdMeshSettings();

    virtual void CopyFrom( CfdMeshSettings* settings );

    string GetExportFileName( int type );
    bool GetExportFileFlag( int type );

    bool m_FarManLocFlag;
    bool m_FarAbsSizeFlag;

    string m_FarGeomID;

    double m_FarXScale; // Check FractionParm to double is appropriate data conversion
    double m_FarYScale;
    double m_FarZScale;

    double m_FarLength;
    double m_FarWidth;
    double m_FarHeight;

    double m_FarXLocation;
    double m_FarYLocation;
    double m_FarZLocation;

    bool m_DrawFarFlag;
    bool m_DrawFarPreFlag;
    bool m_DrawSymmFlag;
    bool m_DrawWakeFlag;
    bool m_DrawBadFlag;

    vector < bool > m_ExportFileFlags;

protected:

    vector < string > m_ExportFileNames;

};

class SimpleFeaMeshSettings : public SimpleMeshCommonSettings
{
public:
    SimpleFeaMeshSettings();
    virtual ~SimpleFeaMeshSettings();

    virtual void CopyFrom( StructSettings* settings );

    string GetExportFileName( int type );
    bool GetExportFileFlag( int type );

    vector < bool > m_ExportFileFlags;
    int m_NumEvenlySpacedPart;
    bool m_DrawNodesFlag;
    bool m_DrawElementOrientVecFlag;

protected:

    vector < string > m_ExportFileNames;

};

class SimpleGridDensity
{
public:

    SimpleGridDensity();
    virtual ~SimpleGridDensity();

    virtual void CopyFrom( GridDensity* gd );

    double GetBaseLen( bool farflag = false )
    {
        if ( !farflag )
        {
            return m_BaseLen;
        }
        else
        {
            return m_FarMaxLen;
        }
    }

    double GetMaxGap( bool farflag = false )
    {
        if ( !farflag )
        {
            return m_MaxGap;
        }
        else
        {
            return m_FarMaxGap;
        }
    }

    double GetRadFrac( bool farflag );
    double GetFarRadFrac();
    double GetTargetLen( vec3d& pos, bool farFlag = false, const string & geomid = string(), const int & surfindx = 0, const double & u = 0.0, const double &w = 0.0 );

    void ClearSources()
    {
        m_Sources.clear();    //Deleted in Geom
    }
    void AddSource( BaseSimpleSource* s )
    {
        m_Sources.push_back( s );
    }
    int  GetNumSources()
    {
        return m_Sources.size();
    }

    void ScaleAllSources( double scale );

    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );
    void Show( bool flag );
    void Highlight( BaseSource * source ); // TODO: Update to SimpleSource

    bool m_RigorLimit;
    double m_BaseLen;
    double m_FarMaxLen;
    double m_MinLen;
    double m_NCircSeg;
    double m_FarNCircSeg;
    double m_MaxGap;
    double m_FarMaxGap;
    double m_GrowRatio;

protected:

    vector< BaseSimpleSource* > m_Sources;

};

class SimpleCfdGridDensity : public SimpleGridDensity
{
public:
    SimpleCfdGridDensity();
    virtual ~SimpleCfdGridDensity();

};

class SimpleFeaGridDensity : public SimpleGridDensity
{
public:
    SimpleFeaGridDensity();
    virtual ~SimpleFeaGridDensity();

};

#endif