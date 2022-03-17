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

#include "Defines.h"
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
class EditCurveXSec;
class XSecCurve;

//==== XSecCurve Driver Group ====//
class XSecCurveDriverGroup : public DriverGroup
{
public:
    XSecCurveDriverGroup( int Nvar, int Nchoice );

    XSecCurve *m_Parent;

    double m_prevArea;
};

class HWXSecCurveDriverGroup : public XSecCurveDriverGroup
{
public:
    HWXSecCurveDriverGroup();

    virtual void UpdateGroup( vector< string > parmIDs );
    virtual bool ValidDrivers( vector< int > choices );
};

class DXSecCurveDriverGroup : public XSecCurveDriverGroup
{
public:
    DXSecCurveDriverGroup();

    virtual void UpdateGroup( vector< string > parmIDs );
    virtual bool ValidDrivers( vector< int > choices );
};

class XSecCurve : public ParmContainer
{
public:
    XSecCurve();
    ~XSecCurve();

    virtual void ParmChanged( Parm* parm_ptr, int type );
    virtual void Update();
    virtual void UpdateCurve( bool updateParms = true ) = 0;

    virtual void SetGroupDisplaySuffix( int num );

    virtual string GetName();

    virtual VspCurve& GetCurve();

    virtual VspCurve& GetBaseEditCurve()                { return m_BaseEditCurve; }

    virtual int  GetType()
    {
        return m_Type;
    }

    //==== Copy Between Different Types ====//
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

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

    virtual void SetForceWingType( double f )                          { m_ForceWingType = f; }
    virtual bool DetermineWingType();

    virtual double ComputeArea();
    virtual double AreaNoUpdate();

    virtual void CloseTE( bool wingtype );
    virtual void CloseLE( bool wingtype );
    virtual void TrimTE( bool wingtype );
    virtual void TrimLE( bool wingtype );
    virtual void CapTE( bool wingtype );
    virtual void CapLE( bool wingtype );
    virtual void Chevron();
    virtual void RotTransScale();

    virtual void ReadV2FileFuse2( xmlNodePtr &root );

    virtual void Interp( XSecCurve *start, XSecCurve *end, double frac );

    // Convert any XSec into an editable type. This function will default the XSec to 
    // cubic Bezier with symmetry off.
    virtual EditCurveXSec* ConvertToEdit();

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

    IntParm m_ChevronType;

    Parm m_ChevTopAmplitude;
    Parm m_ChevBottomAmplitude;
    Parm m_ChevRightAmplitude;
    Parm m_ChevLeftAmplitude;

    IntParm m_ChevNumber;

    Parm m_ChevOnDuty;
    Parm m_ChevOffDuty;

    IntParm m_ChevronExtentMode;
    IntParm m_ChevW01StartGuide;
    Parm m_ChevW01Start;
    IntParm m_ChevW01EndGuide;
    Parm m_ChevW01End;
    IntParm m_ChevW01CenterGuide;
    Parm m_ChevW01Center;
    Parm m_ChevW01Width;

    Parm m_ChevTopAngle;
    Parm m_ChevBottomAngle;
    Parm m_ChevRightAngle;
    Parm m_ChevLeftAngle;

    Parm m_ChevTopSlew;
    Parm m_ChevBottomSlew;
    Parm m_ChevRightSlew;
    Parm m_ChevLeftSlew;

    BoolParm m_ChevDirAngleAllSymFlag;
    BoolParm m_ChevDirAngleTBSymFlag;
    BoolParm m_ChevDirAngleRLSymFlag;

    Parm m_ValleyRad;
    Parm m_PeakRad;

    // XSec Background Parms
    BoolParm m_XSecImagePreserveAR;
    BoolParm m_XSecLockImageFlag;
    BoolParm m_XSecImageFlag;
    FractionParm m_XSecImageW;
    FractionParm m_XSecImageH;
    FractionParm m_XSecImageXOffset;
    FractionParm m_XSecImageYOffset;
    BoolParm m_XSecFlipImageFlag;


    virtual vector< string > GetDriverParms();
    DriverGroup *m_DriverGroup;


    Parm m_Area;
    Parm m_HWRatio;

    virtual void SetImageFile( const string & file ) { m_ImageFile = file; }
    virtual string GetImageFile() { return m_ImageFile; }
    virtual void CopyBackgroundSettings( XSecCurve* xsc );

protected:

    bool m_UseFakeWidth;
    bool m_ForceWingType;
    double m_FakeWidth;

    double m_yscale;

    int m_Type;

    string m_GroupName;
    int m_GroupSuffix;

    VspCurve m_Curve;
    VspCurve m_BaseEditCurve;

    string m_ImageFile;
};

//==========================================================================//
//==========================================================================//
//==========================================================================//

class PointXSec : public XSecCurve
{
public:

    PointXSec( );

    virtual void UpdateCurve( bool updateParms = true );

};

//==========================================================================//
//==========================================================================//
//==========================================================================//

class CircleXSec : public XSecCurve
{
public:

    CircleXSec( );

    virtual void Update();
    virtual void UpdateCurve( bool updateParms = true );

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

    virtual vector< string > GetDriverParms();

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

    virtual void UpdateCurve( bool updateParms = true );

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

    virtual void UpdateCurve( bool updateParms = true );

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

    virtual void UpdateCurve( bool updateParms = true );

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

    virtual EditCurveXSec* ConvertToEdit();

    Parm m_Width;
    Parm m_Height;
    IntParm m_RadiusSymmetryType;
    Parm m_RadiusBR;
    Parm m_RadiusBL;
    Parm m_RadiusTL;
    Parm m_RadiusTR;
    Parm m_Skew;
    Parm m_Keystone;
    BoolParm m_KeyCornerParm;
    Parm m_VSkew;
};

//==========================================================================//
//==========================================================================//
//==========================================================================//

class GeneralFuseXSec : public XSecCurve
{
public:

    GeneralFuseXSec( );

    virtual void UpdateCurve( bool updateParms = true );

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

    virtual void UpdateCurve( bool updateParms = true );

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

    bool ReadXSecFile( FILE* file_id );


};

//==========================================================================//
//==========================================================================//
//==========================================================================//

class EditCurveXSec : public XSecCurve
{
public:

    EditCurveXSec();

    virtual void UpdateCurve( bool updateParms = true );

    virtual void RoundCorners();

    virtual void SetScale( double scale );

    virtual xmlNodePtr EncodeXml( xmlNodePtr& node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr& node );

    // Used to enforce G1 and trigger updated
    virtual void ParmChanged( Parm* parm_ptr, int type );

    // Initialize a circle, ellipse, or rectangle XSec to provide an initial starting point
    virtual void InitShape();

    // Ensure the control points have the required parameter values and limits. For instance, 
    // intermediate bezier control points are required to be parameterized in 1/3 fractions between
    // points on the curve. 
    virtual void EnforcePtOrder( double rfirst = 0.0, double rlast = 1.0 );

    // Get either the relative or absolute control point vector
    virtual vector< vec3d > GetCtrlPntVec( bool non_dimensional = false, bool skip_last = false );

    // Functions to set the control point vector, parameterization, and G1 enforcement vector
    virtual void SetPntVecs( vector < double > u_vec, vector < double > x_pnt_vec, vector < double > y_pnt_vec, vector < double > z_pnt_vec, vector < double > r_vec, vector < bool > g1_vec = {}, vector < bool > fix_u_vec = {}, bool force_update = true );
    virtual void SetPntVecs( vector < double > u_vec, vector < vec3d > pnt_vec, vector < double > r_vec, vector < bool > g1_vec = {}, vector < bool > fix_u_vec = {}, bool force_update = true );

    // Move a control point of input index to a new 2D location. If the point moving is 
    // cubic Bezier and located on the curve, the neighboring points will move with it. 
    // Note, this is an anlternative to directly adjusting the parm values. The force_update
    // flag is automatically set to true when called from the API.
    virtual void MovePnt( int index, vec3d new_pnt, bool force_update = false );
    // Move the currently selected control point to the new x and y position. The 
    // neighbors_only flag is used to move CEDIT neighbors when the point on the
    // curve is set by GUI elements
    virtual void MovePnt( double x, double y, double z, bool neighbors_only = false );

    virtual void MovePnt( vec3d mpt, int iignore, bool neighbors_only = false );

    // Setter and getter for the currently selected point index
    virtual void SetSelectPntID( int id );
    virtual int GetSelectedPntID() { return m_SelectPntID; }

    // Append a new point to the parameter vectors (i.e. m_XParmVec). Note, this does not ensure 
    // proper parameterization
    virtual void AddPt( double default_u = 0.0, double default_x = 0.0, double default_y = 0.0, double default_z = 0.0, double default_r = 0.0, bool default_g1 = false, bool default_fix_u = false );

    // Delete the currently selected point, or the input index. Intermediate cubic bezier control
    // points can not be deleted.
    virtual void DeletePt();
    virtual void DeletePt( int indx );

    // Get the number of control points for the XSec
    virtual int GetNumPts() { return (int)m_XParmVec.size(); }

    // Update Parm names for the parameter vectors by simply appending their point index
    virtual void RenameParms();

    // Function to convert between CEDIT, LINEAR, and PCHIP
    virtual void ConvertTo( int newtype );

    // Leave all coordinate points unchanged, but reparameterize the curve based on
    // the arc length of each segment. 
    virtual void ReparameterizeEqualArcLength();

    // Return the values from the parameter vectors
    virtual vector < double > GetUVec();
    virtual vector < double > GetTVec();
    virtual vector < double > GetXVec();
    virtual vector < double > GetYVec();
    virtual vector < double > GetZVec();
    virtual vector < double > GetRVec();
    virtual vector < bool > GetG1Vec();
    virtual vector < bool > GetFixedUVec();

    // Split the curve at the current u (0-1) value or input value
    virtual int Split01();
    virtual int Split01( double u_split );

    // Required functions for all XSec types. These allow for instance a wing chord to 
    // set the width of the XSec
    virtual double GetWidth()
    {
        return m_Width();
    }
    virtual double GetHeight()
    {
        return m_Height();
    }

    virtual void SetWidthHeight( double w, double h );

    virtual string GetWidthParmID() { return m_Width.GetID(); }
    virtual string GetHeightParmID() { return m_Height.GetID(); }

    IntParm m_View;

    BoolParm m_CloseFlag;
    BoolParm m_SymType;
    IntParm m_ShapeType;
    Parm m_Width;
    Parm m_Height;
    Parm m_Depth;
    IntParm m_CurveType;
    IntParm m_ConvType;
    Parm m_SplitU;
    BoolParm m_AbsoluteFlag;

    // Parms for XSec background image in GUI
    Parm m_XSecPointSize;
    Parm m_XSecLineThickness;
    BoolParm m_XSecPointColorFlag;
    IntParm m_XSecPointColorWheel;

    vector < Parm* > m_UParmVec; // vector of U (0-1) values for each control point (in reallity 0-4 for XSec curves; T)
    vector < FractionParm* > m_XParmVec; // vector of control point x coordinates
    vector < FractionParm* > m_YParmVec; // vector of control point y coordinates
    vector < FractionParm* > m_ZParmVec; // vector of control point z coordinates
    vector < Parm* > m_RParmVec; // vector of corner radii
    vector < BoolParm* > m_EnforceG1Vec; // indicates whether or not to enforce G1 continuity for each CEDIT point on the curve,
    vector < BoolParm* > m_FixedUVec; // vector that identifies if ach index of m_UParmVec is held constant. This is mainly used for reparameterization

protected:

    // Enforce left/right symmetry for the XSec. Forces control points at the 0.25 and 0.75 u 
    // locations (y-axis). The right side of the XSec is the "master" that gets mirrored.
    virtual void EnforceSymmetry();

    // Restricts cubic Bezier intermediate control points from enforcing G1 continuity and
    // deactivates the left side of the XSec if symmetry is active
    virtual void UpdateG1Parms();

    // Force the starting and ending control point of the XSec to coincide
    virtual void EnforceClosure();

    // Resets the limits of the parameter vectors (should be renamed)
    virtual void ClearPtOrder();

    // Enforce G1 continuity for cubic Bezier control points on the curve. The input 
    // index is used to enforce G1 on a new point by averaging the existing tangent slopes.
    // (called from ParmChanged)
    virtual void EnforceG1( int new_index = -1 );

    int m_SelectPntID; // Index of the currently selected control point

    bool m_EnforceG1Next; // Flag to indicate if G1 should be enforced with the next or previous point

    VspCurve m_UnroundedCurve;
};

//==========================================================================//
//==========================================================================//
//==========================================================================//

class InterpXSec : public XSecCurve
{
public:

    InterpXSec( );

    virtual void UpdateCurve( bool updateParms = true );

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

};

#endif // !defined(XSECCURVE__INCLUDED_)
