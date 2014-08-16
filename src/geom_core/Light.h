#if !defined(VSP_LIGHTS__INCLUDED_)
#define VSP_LIGHTS__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"

class Light : public ParmContainer
{
public:
    Light();
    virtual ~Light();

public:
    /*!
    * Override ParmContainer.  Update vehicle on change.
    */
    virtual void ParmChanged( Parm* parm_ptr, int type );

public:
    BoolParm m_Active;

    Parm m_X;
    Parm m_Y;
    Parm m_Z;

    Parm m_Amb;
    Parm m_Diff;
    Parm m_Spec;
};
#endif
