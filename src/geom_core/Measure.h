#if !defined(VSP_MEASURE__INCLUDED_)
#define VSP_MEASURE__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"
#include "Vec2d.h"
#include "Vec3d.h"
#include "DrawObj.h"

#include <string>

enum MEASURE_STAGE { STAGE_ZERO,
                     STAGE_ONE,
                     STAGE_TWO,
                     STAGE_COMPLETE
                   };

class Probe : public ParmContainer
{
public:
    Probe();
    virtual ~Probe();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void Reset();

    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual void Update();
    bool Valid();

    virtual void SetLenFromPlacement( const vec3d &placement );

    int m_Stage;

    std::string m_OriginGeomID;

    IntParm m_OriginIndx;

    Parm m_OriginU;
    Parm m_OriginW;

    IntParm m_Precision;

    Parm m_X;
    Parm m_Y;
    Parm m_Z;

    Parm m_NX;
    Parm m_NY;
    Parm m_NZ;

    Parm m_K1;
    Parm m_K2;
    Parm m_Ka;
    Parm m_Kg;

    Parm m_Len;

    BoolParm m_Visible;

protected:

    DrawObj m_LabelDO;
};

class RSTProbe : public ParmContainer
{
public:
    RSTProbe();
    virtual ~RSTProbe();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void Reset();

    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual void Update();
    bool Valid();

    virtual void SetLenFromPlacement( const vec3d &placement );

    int m_Stage;

    std::string m_OriginGeomID;

    IntParm m_OriginIndx;

    Parm m_OriginR;
    Parm m_OriginS;
    Parm m_OriginT;

    IntParm m_Precision;

    Parm m_X;
    Parm m_Y;
    Parm m_Z;

    Parm m_Len;

    BoolParm m_Visible;

protected:

    DrawObj m_LabelDO;
};

class Ruler : public ParmContainer
{
public:
    Ruler();
    virtual ~Ruler();

    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void Reset();

    virtual void Update();
    bool Valid();

    std::string m_EndGeomID;

    IntParm m_EndIndx;

    Parm m_EndU;
    Parm m_EndW;

    Parm m_XOffset;
    Parm m_YOffset;
    Parm m_ZOffset;

    IntParm m_Precision;

    Parm m_DeltaX;
    Parm m_DeltaY;
    Parm m_DeltaZ;

    Parm m_Distance;

    int m_Stage;

    std::string m_OriginGeomID;

    IntParm m_OriginIndx;

    Parm m_OriginU;
    Parm m_OriginW;

    BoolParm m_Visible;

    IntParm m_Component;

protected:

    DrawObj m_LabelDO;

};
#endif
