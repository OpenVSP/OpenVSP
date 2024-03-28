//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// GeomEngine.h: Extension to GeomXSec for engine modeling (for stack and fuselage)
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSPGEOMENGINE__INCLUDED_)
#define VSPGEOMENGINE__INCLUDED_

#include "Geom.h"

//==== GeomEngine  ====//
class GeomEngine : public GeomXSec
{
public:
    GeomEngine( Vehicle* vehicle_ptr );
    virtual ~GeomEngine();

    virtual void Update( bool fullupdate = true );
    virtual void UpdateFlags();

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual bool IsModelScaleSensitive();

    IntParm m_EngineGeomIOType;
    IntParm m_EngineGeomInType;
    IntParm m_EngineGeomOutType;

    IntParm m_EngineInFaceMode;
    IntParm m_EngineInLipMode;
    IntParm m_EngineInFaceIndex;
    IntParm m_EngineInLipIndex;
    Parm m_EngineInFaceU;
    Parm m_EngineInLipU;

    IntParm m_EngineOutFaceMode;
    IntParm m_EngineOutLipMode;
    IntParm m_EngineOutFaceIndex;
    IntParm m_EngineOutLipIndex;
    Parm m_EngineOutFaceU;
    Parm m_EngineOutLipU;

    IntParm m_EngineInModeType;
    IntParm m_EngineOutModeType;

    IntParm m_AutoExtensionSet;
    BoolParm m_AutoExtensionFlag;
    Parm m_ExtensionDistance;

protected:

    double roll_t( const double &torig, const double &troll, const double &tmin, const double &tmax );
    double unroll_t( const double &t, const double &troll, const double &tmin, const double &tmax );

    virtual void ValidateParms();
    virtual void Extend( VspSurf &surf, const double & u, bool extbefore );
    virtual void UpdateEngine();
    virtual void UpdateBBox();

    virtual void UpdateHighlightDrawObj();

    bool m_engine_spec[ vsp::ENGINE_LOC_NUM ];

    VspSurf m_OrigSurf;
    vector<DrawObj> m_EngineDrawObj_vec;

    BndBox m_OrigMainBBox;
};

#endif // !defined(VSPGEOMENGINE__INCLUDED_)
