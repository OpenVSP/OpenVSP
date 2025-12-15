//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CobraGeom.h:
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_COBRA_GEOM__INCLUDED_)
#define VSP_COBRA_GEOM__INCLUDED_

#include "Geom.h"


void FindIntersectionPoint (
    double ae, double be, double pe,
    double aline, double xp0, double yp0,
    double XM, double XP,
    int sign_flag,
    double &xp_result, double &yp_result, double &Del );

void SE_SuperEllipse_Line_Pts (
    double ae, double be, double pe,
    double aline, double xline,
    double xp0, double yp0,
    double &xp1, double &yp1,
    double &xp2, double &yp2,
    double &Del, int &Ipts );

void SE_SingleSuperEllipse_FixedPt_TangentLine_Define (
    double ae, double be, double pe,
    double xfx, double yfx,
    double &xp0, double &yp0,
    double &aline, double &xline );

void SE_SingleSuperEllipse_FixedPt_TangentLine_Calc (
    double xrng, double ae, double be,
    double pe, double xfx,
    double yfx, double xp0,
    double yp0, double aline,
    double xline, double xpt, double &ypt, double &dydu );

void SQ_SuperEllipse2D_YZCalc ( double yrib, double zrib, double pow_rib,
                                double dyribdu, double dzribdu, double dpowribdu,
                                double y1, double z1, double &ypt, double &zpt, double &dydu, double &dzdu );

void SQ_SuperEllipse2D_YZCalc_dtheta ( double yrib, double zrib, double pow_rib,
                                       double theta,
                                       double &ypt, double &zpt,
                                       double &dydtheta, double &dzdtheta );

void SQ_SuperEllipse2D_YZCalc ( double yrib, double zrib, double pow_rib,
                                double dyribdu, double dzribdu, double dpowribdu,
                                double y1, double z1,
                                double &ypt, double &zpt,
                                double &dydu, double &dzdu,
                                double &dydtheta, double &dzdtheta,
                                double &d2ydudtheta, double &d2zdudtheta );

//==== Pod Geom ====//
class CobraGeom : public Geom
{
public:
    CobraGeom( Vehicle* vehicle_ptr );
    virtual ~CobraGeom();

    virtual void ComputeCenter();
    virtual void Scale();
    virtual void AddDefaultSources( double base_len = 1.0 );

    virtual void OffsetXSecs( double off );

    Parm m_XradN;
    Parm m_YradN;
    Parm m_PowX;
    Parm m_ZradNL;
    Parm m_PowNL;
    BoolParm m_NoseULSymm;
    Parm m_ZradNU;
    Parm m_PowNU;

    Parm m_XlenA;
    Parm m_YradA;
    Parm m_ZradAL;
    Parm m_PowAL;
    BoolParm m_AftULSymm;
    Parm m_ZradAU;
    Parm m_PowAU;

protected:
    virtual void UpdateSurf();

    double m_Xoff;
};


#endif // !defined(VSP_COBRA_GEOM__INCLUDED_)
