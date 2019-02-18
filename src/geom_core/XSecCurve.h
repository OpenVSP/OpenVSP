//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XSec.h: Cross Section Base Class and Derived Classes
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(XSECCURVE__INCLUDED_)
#define XSECCURVE__INCLUDED_

#include "Parm.h"
#include "ParmContainer.h"
#include "Vec3d.h"
#include "VspSurf.h"

#include <string>
#include <vector>
#include <memory>
#include <utility>

using std::string;

class Geom;

class XSecCurve : public ParmContainer
{
public:
    XSecCurve();

    virtual void ParmChanged( Parm* parm_ptr, int type );
    virtual void Update();

    virtual void SetGroupDisplaySuffix( int num );

    virtual string GetName();

    virtual VspCurve& GetCurve();

    virtual int  GetType()
    {
        return m_Type;
    }

    //==== Copy Between Different Types ====//
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );

    virtual void CopyFrom( XSecCurve* from_crv );

    virtual double GetWidth()
    {
        return 0.0;
    }
    virtual double GetHeight()
    {
        return 0.0;
    }
    virtual void SetWidthHeight( double w, double h )           {}
    virtual void SetScale( double scale );
    virtual string GetWidthParmID()                                    { return string(); }
    virtual string GetHeightParmID()                                   { return string(); }

    virtual void SetDesignLiftCoeff( double cli )               {}
    virtual double GetDesignLiftCoeff()                                { return 0.0; }

    virtual void OffsetCurve( double offset_val );

    // FakeWidth is introduced to provide a scale value for leading/trailing edge
    // modifications when a unit-chord is forced for lofting blended wings.
    virtual double GetFakeWidth()                                      { return m_FakeWidth; }
    virtual void SetFakeWidth( double w )                              { m_FakeWidth = w; }
    virtual void SetUseFakeWidth( double b )                           { m_UseFakeWidth = b; }

    virtual double ComputeArea();

    virtual void CloseTE( bool wingtype );
    virtual void CloseLE( bool wingtype );
    virtual void TrimTE( bool wingtype );
    virtual void TrimLE( bool wingtype );
    virtual void CapTE( bool wingtype );
    virtual void CapLE( bool wingtype );

    virtual void RotTransScale();

    virtual void ReadV2FileFuse2( xmlNodePtr &root );

    virtual void Interp( XSecCurve *start, XSecCurve *end, double frac );
    static void InterpCurve( VspCurve & cout, XSecCurve *start, XSecCurve *end, double frac );

    IntParm m_TECloseType;
    IntParm m_TECloseAbsRel;

    Parm m_TECloseThick;
    Parm m_TECloseThickChord;

    IntParm m_LECloseType;
    IntParm m_LECloseAbsRel;

    Parm m_LECloseThick;
    Parm m_LECloseThickChord;


    IntParm m_TETrimType;
    IntParm m_TETrimAbsRel;

    Parm m_TETrimX;
    Parm m_TETrimXChord;
    Parm m_TETrimThick;
    Parm m_TETrimThickChord;

    IntParm m_LETrimType;
    IntParm m_LETrimAbsRel;

    Parm m_LETrimX;
    Parm m_LETrimXChord;
    Parm m_LETrimThick;
    Parm m_LETrimThickChord;

    IntParm m_TECapType;
    Parm m_TECapLength;
    Parm m_TECapOffset;
    Parm m_TECapStrength;

    IntParm m_LECapType;
    Parm m_LECapLength;
    Parm m_LECapOffset;
    Parm m_LECapStrength;

    Parm m_Theta;
    Parm m_Scale;
    Parm m_DeltaX;
    Parm m_DeltaY;
    Parm m_ShiftLE;

protected:

    bool m_UseFakeWidth;
    double m_FakeWidth;

    double m_yscale;

    int m_Type;

    string m_GroupName;
    int m_GroupSuffix;

    VspCurve m_Curve;
};

//==========================================================================//
//==========================================================================//
//==========================================================================//

class PointXSec : public XSecCurve
{
public:

    PointXSec( );

    virtual void Update();

};

//==========================================================================//
//==========================================================================//
//==========================================================================//

class CircleXSec : public XSecCurve
{
public:

    CircleXSec( );

    virtual void Update();

    //==== Values to Set/Get When Changing Types ====//
    virtual double GetWidth()
    {
        return m_Diameter();
    }
    virtual double GetHeight()
    {
        return m_Diameter();
    }
    virtual void SetWidthHeight( double w, double h );
    virtual string GetWidthParmID()                                    { return m_Diameter.GetID(); }

    virtual void OffsetCurve( double off );

    Parm m_Diameter;
};

//==========================================================================//
//==========================================================================//
//==========================================================================//

class EllipseXSec : public XSecCurve
{
public:

    EllipseXSec( );

    virtual void Update();

    //==== Values to Set/Get When Changing Types ====//
    virtual double GetWidth()
    {
        return m_Width();
    }
    virtual double GetHeight()
    {
        return m_Height();
    }
    virtual void SetWidthHeight( double w, double h );
    virtual string GetWidthParmID()                                    { return m_Width.GetID(); }
    virtual string GetHeightParmID()                                    { return m_Height.GetID(); }

    Parm m_Width;
    Parm m_Height;
};

//==========================================================================//
//==========================================================================//
//==========================================================================//

class SuperXSec : public XSecCurve
{
public:

    SuperXSec( );

    virtual void Update();

    //==== Values to Set/Get When Changing Types ====//
    virtual double GetWidth()
    {
        return m_Width();
    }
    virtual double GetHeight()
    {
        return m_Height();
    }
    virtual void SetWidthHeight( double w, double h );
    virtual string GetWidthParmID()                                    { return m_Width.GetID(); }
    virtual string GetHeightParmID()                                    { return m_Height.GetID(); }

    virtual void Interp( XSecCurve *start, XSecCurve *end, double frac );

    Parm m_Width;
    Parm m_Height;
    Parm m_M;
    Parm m_N;
    Parm m_M_bot;
    Parm m_N_bot;
    Parm m_MaxWidthLoc;
    BoolParm m_TopBotSym;
};

//==========================================================================//
//==========================================================================//
//==========================================================================//

class RoundedRectXSec : public XSecCurve
{
public:

    RoundedRectXSec( );

    virtual void Update();

    //==== Values to Set/Get When Changing Types ====//
    virtual double GetWidth()
    {
        return m_Width();
    }
    virtual double GetHeight()
    {
        return m_Height();
    }
    virtual void SetWidthHeight( double w, double h );
    virtual void SetScale( double scale );
    virtual string GetWidthParmID()                                    { return m_Width.GetID(); }
    virtual string GetHeightParmID()                                    { return m_Height.GetID(); }

    virtual void ReadV2FileFuse2( xmlNodePtr &root );

    virtual void Interp( XSecCurve *start, XSecCurve *end, double frac );

    Parm m_Width;
    Parm m_Height;
    Parm m_Radius;
    Parm m_Skew;
    Parm m_Keystone;
    BoolParm m_KeyCornerParm;
};

//==========================================================================//
//==========================================================================//
//==========================================================================//

class GeneralFuseXSec : public XSecCurve
{
public:

    GeneralFuseXSec( );

    virtual void Update();

    //==== Values to Set/Get When Changing Types ====//
    virtual double GetWidth()
    {
        return m_Width();
    }
    virtual double GetHeight()
    {
        return m_Height();
    }
    virtual void SetWidthHeight( double w, double h );
    virtual string GetWidthParmID()                                    { return m_Width.GetID(); }
    virtual string GetHeightParmID()                                    { return m_Height.GetID(); }

    virtual void ReadV2FileFuse2( xmlNodePtr &root );

    virtual void Interp( XSecCurve *start, XSecCurve *end, double frac );

    Parm m_Width;
    Parm m_Height;
    Parm m_MaxWidthLoc;
    Parm m_CornerRad;
    Parm m_TopTanAngle;
    Parm m_BotTanAngle;
    Parm m_TopStr;
    Parm m_BotStr;
    Parm m_UpStr;
    Parm m_LowStr;
};

//==========================================================================//
//==========================================================================//
//==========================================================================//

class FileXSec : public XSecCurve
{
public:

    FileXSec( );

    virtual void Update();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    //==== Values to Set/Get When Changing Types ====//
    virtual double GetWidth()
    {
        return m_Width();
    }
    virtual double GetHeight()
    {
        return m_Height();
    }
    virtual void SetWidthHeight( double w, double h );
    vector< vec3d > GetUnityFilePnts()
    {
        return m_UnityFilePnts;
    }
    virtual string GetWidthParmID()                                    { return m_Width.GetID(); }
    virtual string GetHeightParmID()                                    { return m_Height.GetID(); }

    //===== Read File ====//
    bool ReadXsecFile( string file_name );
    void SetPnts( vector< vec3d > & pnt_vec );

    virtual void ReadV2FileFuse2( xmlNodePtr &root );

    Parm m_Width;
    Parm m_Height;

    string m_FileName;


protected:

    vector< vec3d > m_UnityFilePnts;

    bool ReadOldXSecFile( FILE* file_id );
    bool ReadXSecFile( FILE* file_id );


};





#endif // !defined(XSECCURVE__INCLUDED_)
