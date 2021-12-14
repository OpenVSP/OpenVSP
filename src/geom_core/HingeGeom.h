//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPHINGEGEOM__INCLUDED_)
#define VSPHINGEGEOM__INCLUDED_

#include "Geom.h"


//==== Hinge Geom ====//
class HingeGeom : public Geom
{
public:
    HingeGeom( Vehicle* vehicle_ptr );
    virtual ~HingeGeom();

    virtual int GetNumMainSurfs() const
    {
        return 0;
    };

    virtual void Scale();

    virtual void UpdateSurf();

    virtual void UpdateMotionFlagsLimits();

    virtual void UpdateDrawObj();
    virtual void LoadDrawObjs(vector< DrawObj* > & draw_obj_vec);

    virtual Matrix4d GetJointMatrix();

    Parm m_JointTranslate;
    BoolParm m_JointTranslateFlag;
    Parm m_JointTransMin;
    BoolParm m_JointTransMinFlag;
    Parm m_JointTransMax;
    BoolParm m_JointTransMaxFlag;
    Parm m_JointRotate;
    BoolParm m_JointRotateFlag;
    Parm m_JointRotMin;
    BoolParm m_JointRotMinFlag;
    Parm m_JointRotMax;
    BoolParm m_JointRotMaxFlag;

    Parm m_PrimXVec;
    Parm m_PrimYVec;
    Parm m_PrimZVec;

    Parm m_PrimXVecRel;
    Parm m_PrimYVecRel;
    Parm m_PrimZVecRel;

    IntParm m_PrimVecAbsRelFlag;

    Parm m_PrimXOff;
    Parm m_PrimYOff;
    Parm m_PrimZOff;

    Parm m_PrimXOffRel;
    Parm m_PrimYOffRel;
    Parm m_PrimZOffRel;

    IntParm m_PrimOffAbsRelFlag;

    Parm m_PrimULoc;
    Parm m_PrimWLoc;

    enum { ORIENT_ROT, ORIENT_VEC, ORIENT_NUM_TYPES };

    IntParm m_OrientType;

    IntParm m_PrimaryDir;
    IntParm m_SecondaryDir;

    enum { VECTOR3D, POINT3D, SURFPT, UDIR, WDIR, NDIR, ORIENT_VEC_TYPES };

    IntParm m_PrimaryType;

    IntParm m_SecVecAbsRelFlag;
    IntParm m_SecondaryVecDir;


protected:

    DrawObj m_MotionLinesDO;
    DrawObj m_MotionArrowsDO;

    DrawObj m_PrimaryLineDO;

    Matrix4d m_JointMatrix;

    vec3d m_PrimEndpt;

    static void SetParmLimits( Parm & p, Parm & pflag, Parm & pmin, Parm & pminflag, Parm & pmax, Parm & pmaxflag );

};

#endif // !defined(VSPHINGEGEOM__INCLUDED_)
