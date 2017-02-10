//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// MeshCommonSettings.h
//////////////////////////////////////////////////////////////////////

#if !defined(MESHCOMMONSETTINGS__INCLUDED_)
#define MESHCOMMONSETTINGS__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
class MeshCommonSettings : public ParmContainer
{
    public:
    MeshCommonSettings();
    virtual ~MeshCommonSettings();

    virtual void InitCommonParms();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual void SetIntersectSubSurfs( bool f )
    {
        m_IntersectSubSurfs = f;
    }
    virtual bool GetIntersectSubSurfs()
    {
        return m_IntersectSubSurfs();
    }

    virtual bool GetFarMeshFlag()
    {
        return m_FarMeshFlag.Get();
    }
    virtual void SetFarMeshFlag( bool f )
    {
        m_FarMeshFlag = f;
    }
    virtual bool GetFarCompFlag()
    {
        return m_FarCompFlag.Get();
    }
    virtual void SetFarCompFlag( bool f )
    {
        m_FarCompFlag = f;
    }

    virtual bool GetHalfMeshFlag()
    {
        return m_HalfMeshFlag.Get();
    }
    virtual void SetHalfMeshFlag( bool f )
    {
        m_HalfMeshFlag = f;
    }

    //Symmetry Plane Item Quad BoolParm
    virtual bool GetSymSplittingOnFlag()
    {
        return m_SymSplittingOnFlag.Get();
    }
    virtual void SetSymSplittingOnFlag( bool f )
    {
        m_SymSplittingOnFlag = f;
    }

    //Symmetry Plane Boolean Items
    BoolParm m_SymSplittingOnFlag;

    BoolParm m_DrawMeshFlag;
    BoolParm m_DrawBadFlag;
    BoolParm m_ColorTagsFlag;

    BoolParm m_IntersectSubSurfs;

    BoolParm m_FarMeshFlag;
    BoolParm m_FarCompFlag;
    BoolParm m_HalfMeshFlag;

};

#endif // !defined(MESHCOMMONSETTINGS__INCLUDED_)