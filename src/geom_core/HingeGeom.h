//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPHINGEGEOM__INCLUDED_)
#define VSPHINGEGEOM__INCLUDED_

#include "Geom.h"
#include "GeomInterface.h"


//==== A Geom that articulates its children ====//
// Implemented by HingeGeom and by a Clone standing in for one.  Children ride the joint.
class JointRole : virtual public GeomInterface
{
public:
    // The behavior type this role belongs to; checked by Geom::CastTo.
    static int BehaviorType()   { return HINGE_GEOM_TYPE; }

    // Which axis of the Geom's own frame the joint moves along and turns about.
    virtual int GetJointPrimaryDir() const = 0;

    // The deflection this Geom is posed at.  Each Clone of a joint has its own.
    virtual double GetJointTranslate() const = 0;
    virtual double GetJointRotate() const = 0;

    // The allowed motion: returns whether it is enabled, with a flag for each limit that is set.
    virtual bool GetJointTransMotion( bool &min_set, double &min_val, bool &max_set, double &max_val ) const = 0;
    virtual bool GetJointRotMotion( bool &min_set, double &min_val, bool &max_set, double &max_val ) const = 0;

    // Apply this joint's motion flags and limits to another Geom's Parms.
    virtual void SetJointParmLimits( Parm &translate, Parm &rotate ) = 0;

    // The joint motion for a given deflection.
    virtual Matrix4d BuildJointMatrix( double translate, double rotate, const Matrix4d &model_matrix ) const = 0;

    // The same motion seen through a flip; the frame stays unreflected.
    Matrix4d BuildFlippedJointMatrix( double translate, double rotate, const Matrix4d &model_matrix, const Matrix4d &flip_mat ) const;

    // Where the joint has carried its children, from this Geom's own deflection and placement.
    virtual Matrix4d GetJointMatrix() const;

    // The line the joint moves along, in world coordinates.  See the definition.
    vec3d GetJointAxis() const;
};

//==== Hinge Geom ====//
class HingeGeom : public Geom, public JointRole
{
public:
    HingeGeom( Vehicle* vehicle_ptr );
    virtual ~HingeGeom();

    virtual void ApplyScale( double currentScale ) override;

    virtual void UpdateXForm() override;


    virtual void UpdateMotionFlagsLimits();

    virtual void UpdateDrawObj() override;
    virtual void LoadMainDrawObjs(vector< DrawObj* > & draw_obj_vec) override;
    virtual void LoadDrawObjs(vector< DrawObj* > & draw_obj_vec) override;

    virtual int GetJointPrimaryDir() const override
    {
        return m_PrimaryDir();
    }

    // Built once in UpdateXForm.
    virtual Matrix4d GetJointMatrix() const override;
    virtual double GetJointTranslate() const override
    {
        return m_JointTranslate();
    }
    virtual double GetJointRotate() const override
    {
        return m_JointRotate();
    }
    virtual Matrix4d BuildJointMatrix( double translate, double rotate, const Matrix4d &model_matrix ) const override;
    virtual void SetJointParmLimits( Parm &translate, Parm &rotate ) override;
    virtual bool GetJointTransMotion( bool &min_set, double &min_val, bool &max_set, double &max_val ) const override;
    virtual bool GetJointRotMotion( bool &min_set, double &min_val, bool &max_set, double &max_val ) const override;

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
    virtual void UpdateSurf() override;

    DrawObj m_MotionLinesDO;
    DrawObj m_MotionArrowsDO;

    DrawObj m_PrimaryLineDO;

    Matrix4d m_JointMatrix;

    vec3d m_PrimEndpt;

    static void SetParmLimits( Parm & p, const Parm & pflag, const Parm & pmin, const Parm & pminflag, const Parm & pmax, const Parm & pmaxflag );

};

#endif // !defined(VSPHINGEGEOM__INCLUDED_)
