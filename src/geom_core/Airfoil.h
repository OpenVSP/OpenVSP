//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Airfoil.h: Airfoil XSec Class
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(AIRFOIL__INCLUDED_)
#define AIRFOIL__INCLUDED_

#include "Defines.h"
#include "XSecCurve.h"
#include "Vec3d.h"
#include "VspCurve.h"

#include <string>
#include <vector>
#include <memory>
#include <utility>

#include "eli/geom/curve/piecewise.hpp"
#include "eli/geom/curve/piecewise_creator.hpp"
#include "eli/geom/curve/piecewise_cst_airfoil_creator.hpp"
#include "eli/geom/curve/piecewise_cst_airfoil_fitter.hpp"
#include "eli/geom/curve/pseudo/cst_airfoil.hpp"
#include "eli/geom/curve/pseudo/naca_af.hpp"
#include "eli/geom/curve/pseudo/five_digit.hpp"
#include "eli/geom/curve/pseudo/five_digit_mod.hpp"
#include "eli/geom/curve/pseudo/four_digit.hpp"
#include "eli/geom/curve/pseudo/four_digit_mod.hpp"
#include "eli/geom/curve/pseudo/one_six_series.hpp"


typedef piecewise_curve_type::index_type curve_index_type;
typedef piecewise_curve_type::point_type curve_point_type;
typedef piecewise_curve_type::tolerance_type curve_tolerance_type;

typedef eli::geom::curve::piecewise_four_digit_creator<double, 3, curve_tolerance_type> piecewise_four_digit_creator;
typedef eli::geom::curve::piecewise_cst_airfoil_creator<double, 3, curve_tolerance_type> piecewise_cst_creator;
typedef eli::geom::curve::pseudo::cst_airfoil<double> cst_airfoil_type;
typedef eli::geom::curve::piecewise_cst_airfoil_fitter<double, 3, curve_tolerance_type> cst_fitter_type;
typedef eli::geom::curve::pseudo::naca_af<double> naca_airfoil_type;
typedef eli::geom::curve::pseudo::five_digit<double> five_digit_airfoil_type;
typedef eli::geom::curve::pseudo::five_digit_mod<double> five_digit_mod_airfoil_type;
typedef eli::geom::curve::pseudo::four_digit<double> four_digit_airfoil_type;
typedef eli::geom::curve::pseudo::four_digit_mod<double> four_digit_mod_airfoil_type;
typedef eli::geom::curve::pseudo::one_six_series<double> one_six_series_airfoil_type;

#define MAX_CST_DEG 30

using std::string;

double CalcFourDigitCLi( double m, double p );
double CalcFourDigitCamber( double CLi, double p );

//==== Base Class For Airfoils ====//
class Airfoil : public XSecCurve
{
public:
    Airfoil( );                                                   // Default Constructor

    virtual void Update();
    virtual void UpdateCurve( bool updateParms = true );

    //==== Values to Set/Get When Changing Types ====//
    virtual double GetWidth();
    virtual double GetHeight();
    virtual void SetWidthHeight( double w, double h );
    virtual string GetWidthParmID()                                { return m_Chord.GetID(); }
    virtual string GetHeightParmID()                               { return m_ThickChord.GetID(); }
    virtual void GetLiftCamberParmID( vector < string > &ids )     {};
    virtual void OffsetCurve( double offset_val );

    virtual VspCurve& GetOrigCurve();

    virtual void ReadV2File( xmlNodePtr &root );

    BoolParm m_Invert;
    Parm m_Chord;
    Parm m_ThickChord;                  // Thick/Chord

    IntParm m_FitDegree;

protected:

    virtual double CalculateThick();

    VspCurve m_OrigCurve;
};

//==========================================================================//
//=======================  NACA Base Airfoil   =============================//
//==========================================================================//

class NACABase : public Airfoil
{
public:

    NACABase();

    virtual void BuildCurve( const naca_airfoil_type & af );

    virtual string GetAirfoilName() = 0;
};

//==========================================================================//
//=======================  Four Series Airfoil   ===========================//
//==========================================================================//

class FourSeries : public NACABase
{
public:

    FourSeries( );

    virtual void UpdateCurve( bool updateParms = true );
    virtual void SetDesignLiftCoeff( double cli );
    virtual double GetDesignLiftCoeff();
    virtual void UpdateDesignLiftCoeff();
    virtual void GetLiftCamberParmID( vector < string > &ids );

    virtual string GetAirfoilName();

    virtual void ReadV2File( xmlNodePtr &root );
    virtual void Interp( XSecCurve *start, XSecCurve *end, double frac );

    Parm m_Camber;
    Parm m_CamberLoc;
    BoolParm m_SharpTE;

    Parm m_IdealCl;
    IntParm m_CamberInputFlag;
};

//==========================================================================//
//=======================  Four Digit Modified Airfoil   ===================//
//==========================================================================//

class FourDigMod : public NACABase
{
public:

    FourDigMod( );

    virtual void UpdateCurve( bool updateParms = true );
    virtual void SetDesignLiftCoeff( double cli );
    virtual double GetDesignLiftCoeff();
    virtual void UpdateDesignLiftCoeff();
    virtual void GetLiftCamberParmID( vector < string > &ids );

    virtual string GetAirfoilName();
    virtual void Interp( XSecCurve *start, XSecCurve *end, double frac );

    Parm m_Camber;
    Parm m_CamberLoc;
    Parm m_ThickLoc;
    Parm m_LERadIndx;
    BoolParm m_SharpTE;

    Parm m_IdealCl;
    IntParm m_CamberInputFlag;
};

//==========================================================================//
//=======================  Five Digit Airfoil   ============================//
//==========================================================================//

class FiveDig : public NACABase
{
public:

    FiveDig( );

    virtual void UpdateCurve( bool updateParms = true );
    virtual void SetDesignLiftCoeff( double cli );
    virtual double GetDesignLiftCoeff();
    virtual void GetLiftCamberParmID( vector < string > &ids );

    virtual string GetAirfoilName();
    virtual void Interp( XSecCurve *start, XSecCurve *end, double frac );

    Parm m_IdealCl;
    Parm m_CamberLoc;
    BoolParm m_SharpTE;
};

//==========================================================================//
//=======================  Five Digit Mod Airfoil   ========================//
//==========================================================================//

class FiveDigMod : public NACABase
{
public:

    FiveDigMod( );

    virtual void UpdateCurve( bool updateParms = true );
    virtual void SetDesignLiftCoeff( double cli );
    virtual double GetDesignLiftCoeff();
    virtual void GetLiftCamberParmID( vector < string > &ids );

    virtual string GetAirfoilName();
    virtual void Interp( XSecCurve *start, XSecCurve *end, double frac );

    Parm m_IdealCl;
    Parm m_CamberLoc;
    Parm m_ThickLoc;
    Parm m_LERadIndx;
    BoolParm m_SharpTE;
};

//==========================================================================//
//=======================  16 Series Airfoil   =============================//
//==========================================================================//

class OneSixSeries : public NACABase
{
public:

    OneSixSeries( );

    virtual void UpdateCurve( bool updateParms = true );
    virtual void SetDesignLiftCoeff( double cli );
    virtual double GetDesignLiftCoeff();
    virtual void GetLiftCamberParmID( vector < string > &ids );

    virtual string GetAirfoilName();

    Parm m_IdealCl;
    BoolParm m_SharpTE;
};

//==========================================================================//
//================== Six Series Airfoil         ============================//
//==========================================================================//
//==== Fortran Common Block For 6-Series Airfoils =====//
extern "C"
{
    void sixseries_( const int* ser, const float* t, const float* cli, const float* a );
}

extern "C"
{
    extern struct
    {
        int nml;
        int nmu;
        float xxl[1000];
        float xxu[1000];
        float yyl[1000];
        float yyu[1000];
    } sixpnts_;
}

class SixSeries : public Airfoil
{
public:

    SixSeries( );

    virtual void UpdateCurve( bool updateParms = true );
    virtual void SetDesignLiftCoeff( double cli );
    virtual double GetDesignLiftCoeff();
    virtual void GetLiftCamberParmID( vector < string > &ids );

    virtual string GetAirfoilName();

    virtual void ReadV2File( xmlNodePtr &root );
    virtual void Interp( XSecCurve *start, XSecCurve *end, double frac );

    IntParm m_Series;
    Parm m_IdealCl;
    Parm m_A;                   // % Chord Uniform Pressure Dist

    enum { SERIES_63, SERIES_64, SERIES_65, SERIES_66, SERIES_67,
           SERIES_63A, SERIES_64A, SERIES_65A, NUM_SERIES
         };

protected:

};

//==========================================================================//
//======================= Bi Convex                       ==================//
//==========================================================================//

class Biconvex : public Airfoil
{
public:

    Biconvex( );

    virtual void UpdateCurve( bool updateParms = true );

protected:


};

//==========================================================================//
//======================== Wedge        ====================================//
//==========================================================================//

class Wedge : public Airfoil
{
public:

    Wedge( );

    virtual void UpdateCurve( bool updateParms = true );

    virtual void ReadV2File( xmlNodePtr &root );
    virtual void Interp( XSecCurve *start, XSecCurve *end, double frac );

    Parm m_ThickLoc;
    Parm m_ThickLocLow;
    Parm m_FlatUp;
    Parm m_FlatLow;
    Parm m_ZCamber;

    Parm m_UForeUp;
    Parm m_DuUp;
    Parm m_UForeLow;
    Parm m_DuLow;

    BoolParm m_SymmThick;

protected:



};

//==========================================================================//
//======================== Airfoil From File       =========================//
//==========================================================================//

class FileAirfoil : public Airfoil
{
public:

    FileAirfoil( );

    virtual void UpdateCurve( bool updateParms = true );
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void OffsetCurve( double offset_val );

    virtual bool ReadFile( string file_name );

    virtual string GetAirfoilName()
    {
        return m_AirfoilName;
    }

    virtual vector< vec3d > GetUpperPnts()
    {
        return m_UpperPnts;
    }

    virtual vector< vec3d > GetLowerPnts()
    {
        return m_LowerPnts;
    }

    virtual void SetAirfoilPnts( const vector< vec3d > & up_pnt_vec, const vector< vec3d > & low_pnt_vec )
    {
        m_UpperPnts = up_pnt_vec;
        m_LowerPnts = low_pnt_vec;
    }

    virtual void ReadV2File( xmlNodePtr &root );

    Parm m_BaseThickness;

protected:

    virtual bool ReadSeligAirfoil( FILE* file_id );
    virtual bool ReadLednicerAirfoil( FILE* file_id );
    virtual bool ReadVspAirfoil( FILE* file_id );

    virtual void MakeCurve();

    string m_AirfoilName;
    vector< vec3d > m_UpperPnts;
    vector< vec3d > m_LowerPnts;

};


//==========================================================================//
//=============================== CST Airfoil ==============================//
//==========================================================================//

class CSTAirfoil : public Airfoil
{
public:

    CSTAirfoil( );

    virtual void UpdateCurve( bool updateParms = true );

    //==== Encode/Decode XML ====//
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void OffsetCurve( double offset_val );

    virtual void PromoteUpper();
    virtual void PromoteLower();
    virtual void DemoteUpper();
    virtual void DemoteLower();

    virtual void FitCurve( VspCurve c, int deg );

    virtual vector < double > GetUpperCST();
    virtual vector < double > GetLowerCST();

    virtual int GetUpperDegree();
    virtual int GetLowerDegree();

    virtual void SetUpperCST( int deg, const vector < double > &coefs );
    virtual void SetLowerCST( int deg, const vector < double > &coefs );

    IntParm m_UpDeg;
    IntParm m_LowDeg;

    vector< Parm* > m_UpCoeffParmVec;
    vector< Parm* > m_LowCoeffParmVec;

    BoolParm m_ContLERad;

protected:

    virtual void MakeCSTAirfoil( cst_airfoil_type &cst );
    virtual void CSTtoParms( cst_airfoil_type &cst );

    virtual void ZeroParms();
    virtual void ZeroUpParms();
    virtual void ZeroLowParms();

    virtual void ReserveUpDeg( int d );
    virtual void ReserveLowDeg( int d );

    virtual string AddUpParm();
    virtual string AddLowParm();

    virtual void CheckLERad();

};


//==========================================================================//
//=====================  Karman-Trefftz Airfoil   ==========================//
//==========================================================================//

class VKTAirfoil : public Airfoil
{
public:

    VKTAirfoil( );

    virtual void UpdateCurve( bool updateParms = true );

    virtual void OffsetCurve( double offset_val );
    virtual void Interp( XSecCurve *start, XSecCurve *end, double frac );

    Parm m_Epsilon;
    Parm m_Kappa;
    Parm m_Tau;
};

#endif // !defined(AIRFOIL__INCLUDED_)
