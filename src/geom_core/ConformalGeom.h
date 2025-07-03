//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CustomGeom.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////



#if !defined(VSPCONFORMALGEOM__INCLUDED_)
#define VSPCONFORMALGEOM__INCLUDED_

#include "Defines.h"
#include "Geom.h"
#include "XSec.h"
#include "XSecSurf.h"

class ConformalSpine;

typedef piecewise_surface_type::piecewise_curve_type curve_type;


//==== Conformal Geom ====//
class ConformalGeom : public Geom
{
public:
    ConformalGeom( Vehicle* vehicle_ptr );
    virtual ~ConformalGeom();

    virtual void Scale();

    Parm m_Offset;                  // Offset to Conformal Surface

    BoolParm m_OffsetEnds;

    IntParm m_UMinTrimTypeFlag;
    IntParm m_UMaxTrimTypeFlag;

    BoolParm m_UTrimFlag;
    Parm m_UTrimMin;
    Parm m_UTrimMax;

    Parm m_LTrimMin;
    BoolParm m_L01Min;
    Parm m_L0LenTrimMin;

    Parm m_LTrimMax;
    BoolParm m_L01Max;
    Parm m_L0LenTrimMax;

    Parm m_EtaTrimMin;
    Parm m_EtaTrimMax;

    IntParm m_CapUMinTrimOption;
    IntParm m_CapUMaxTrimOption;

    Parm m_CapUMinTrimLength;
    Parm m_CapUMinTrimOffset;
    Parm m_CapUMinTrimStrength;
    BoolParm m_CapUMinTrimSweepFlag;

    Parm m_CapUMaxTrimLength;
    Parm m_CapUMaxTrimOffset;
    Parm m_CapUMaxTrimStrength;
    BoolParm m_CapUMaxTrimSweepFlag;

    BoolParm m_V1TrimFlag;
    Parm m_V1TrimBegin;
    Parm m_V1TrimEnd;

    BoolParm m_V2TrimFlag;
    Parm m_V2TrimBegin;
    Parm m_V2TrimEnd;

    BoolParm m_ChordTrimFlag;
    Parm m_ChordTrimMin;
    Parm m_ChordTrimMax;

    BoolParm m_Side1TrimFlag;
    Parm m_Side1Trim;

    BoolParm m_Side2TrimFlag;
    Parm m_Side2Trim;

    BoolParm m_Side3TrimFlag;
    Parm m_Side3Trim;

    BoolParm m_Side4TrimFlag;
    Parm m_Side4Trim;

    bool IsWingParent()                             { return m_WingParentFlag; }


protected:

    virtual void UpdateSurf();
    virtual void UpdateCopyParms();

    virtual void UpdateDrawObj();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual void OffsetCurve( curve_type & crv, double offset );
    virtual void TranslateCurve( curve_type & crv, const vec3d & offset );
    virtual vec3d ComputeCenter( curve_type & crv );
    virtual void ReSkin( VspSurf & surf, vector< rib_data_type > & rib_vec );

    virtual bool CheckIfRibIsPoint( rib_data_type & rib );

    virtual void CenterRibCurves( VspSurf & surf, const VspSurf & ref_surf, double offset );
    virtual void OffsetEndRibs( VspSurf & surf, double offset );
    virtual void AdjustShape( VspSurf & surf, const VspSurf & ref_surf, double offset );

    virtual double FindEndUOffsetCurve( VspSurf & surf, double offset, bool start_flag, curve_type & crv );
    virtual double AdjustForSurfaceDist( const VspSurf & surf, const ConformalSpine & spine, double u, double offset, bool backward_flag );

    virtual void TrimU( VspSurf & surf );
    virtual void TrimV( VspSurf & surf );

    virtual void SetWingTrimParms( VspSurf & surf );
    virtual void UpdateParms( VspSurf & surf );

    virtual void FindDists( const VspSurf & surf, piecewise_curve_type & curve, double u0, vector< double > & dist_vec );
    virtual double ComputeAvgOffset( VspSurf & surf, const VspSurf & ref_surf, double u );

    virtual double ComputeMaxOffsetError( VspSurf & surf, VspSurf &  ref_surf, double offset, int num_u, int num_w );


    bool m_WingParentFlag;

};



class ConformalSpine
{
public:

    ConformalSpine()
    {
        m_MaxDist = 0;
    }
    virtual ~ConformalSpine()            {};

    void Build( const VspSurf & surf );

    int GetNumSamps() const               { return m_DistVec.size(); }
    double GetMaxDist() const             { return m_MaxDist; }
    double FindUGivenDist( double d ) const;
    vec3d FindCenterGivenU( double u ) const;


    double FindUGivenLengthAlongSpine( double length ) const;
    double GetSpineLength() const;

protected:

    vector< double > m_UVec;
    vector< vec3d > m_CenterVec;
    vector< double > m_DistVec;

    double m_MaxDist;
};




#endif // !defined(VSPCONFORMALGEOM__INCLUDED_)
