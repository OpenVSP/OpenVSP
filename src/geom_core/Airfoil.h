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

#include "XSecCurve.h"
#include "Vec3d.h"
#include "VspCurve.h"

#include <string>
#include <vector>
#include <memory>
#include <utility>

#include "eli/geom/curve/piecewise.hpp"
#include "eli/geom/curve/piecewise_creator.hpp"

typedef piecewise_curve_type::index_type curve_index_type;
typedef piecewise_curve_type::point_type curve_point_type;
typedef piecewise_curve_type::rotation_matrix_type curve_rotation_matrix_type;
typedef piecewise_curve_type::tolerance_type curve_tolerance_type;

typedef eli::geom::curve::piecewise_four_digit_creator<double, 3, curve_tolerance_type> piecewise_four_digit_creator;

using std::string;

//==== Base Class For Airfoils ====//
class Airfoil : public XSecCurve
{
public:
    Airfoil( );                                                   // Default Constructor

    virtual void Update();
    virtual string GetAirfoilName()
    {
        return string();
    }

    //==== Values to Set/Get When Changing Types ====//
    virtual double GetWidth();
    virtual double GetHeight();
    virtual void SetWidthHeight( double w, double h );
    virtual string GetWidthParmID()                                { return m_Chord.GetID(); }

    BoolParm m_Invert;
    Parm m_Chord;
    Parm m_ThickChord;                  // Thick/Chord
};

//==========================================================================//
//=======================  Four Series Airfoil   ===========================//
//==========================================================================//

class FourSeries : public Airfoil
{
public:

    FourSeries( );

    virtual void Update();

    virtual string GetAirfoilName();

    Parm m_Camber;
    Parm m_CamberLoc;

protected:
    piecewise_four_digit_creator m_Creator;

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

    virtual void Update();

    virtual string GetAirfoilName();

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

    virtual void Update();

protected:


};

//==========================================================================//
//======================== Wedge        ====================================//
//==========================================================================//

class Wedge : public Airfoil
{
public:

    Wedge( );

    virtual void Update();

    Parm m_ThickLoc;

protected:



};

//==========================================================================//
//======================== Airfoil From File       =========================//
//==========================================================================//

class FileAirfoil : public Airfoil
{
public:

    FileAirfoil( );

    virtual void Update();
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

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

protected:

    virtual bool ReadSeligAirfoil( FILE* file_id );
    virtual bool ReadLednicerAirfoil( FILE* file_id );
    virtual bool ReadVspAirfoil( FILE* file_id );


    string m_AirfoilName;
    vector< vec3d > m_UpperPnts;
    vector< vec3d > m_LowerPnts;


};




#endif // !defined(AIRFOIL__INCLUDED_)
