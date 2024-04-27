#if !defined(VSP_BACKGROUND3D__INCLUDED_)
#define VSP_BACKGROUND3D__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"
#include "Vec2d.h"
#include "Vec3d.h"
#include "DrawObj.h"

#include <string>

class Background3D : public ParmContainer
{
public:
    Background3D();
    virtual ~Background3D();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual void Update();

    virtual void UpdateImageInfo();

    string GetDirectionName();

    std::string m_BGFile;

    IntParm m_Direction;

    Parm m_X;
    Parm m_Y;
    Parm m_Z;
    IntParm m_DepthPos;

    Parm m_NormX;
    Parm m_NormY;
    Parm m_NormZ;

    Parm m_UpX;
    Parm m_UpY;
    Parm m_UpZ;

    IntParm m_ScaleType;

    Parm m_BackgroundWidth;
    Parm m_BackgroundHeight;
    Parm m_Resolution;

    IntParm m_HAlign;
    IntParm m_VAlign;
    IntParm m_ImageX;
    IntParm m_ImageY;

    BoolParm m_Visible;
    BoolParm m_RearVisible;
    BoolParm m_VisAlign;
    Parm m_VisTol;

    IntParm m_ImageW;
    IntParm m_ImageH;

    BoolParm m_ImageFlipLR;
    BoolParm m_ImageFlipUD;
    BoolParm m_ImageAutoTransparent;

    IntParm m_ImageRot;

protected:

    bool m_ImageReady;
    int m_ImageRev;

    DrawObj m_BackgroundDO;
};

#endif
