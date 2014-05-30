#if !defined(VSP_LIGHTS__INCLUDED_)
#define VSP_LIGHTS__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"

/*!
* Light info.
*/
class Light : public ParmContainer
{
public:
    /*!
    * Construct a light.
    */
    Light();
    /*!
    * Destructor.
    */
    virtual ~Light();

public:
    /*!
    * Override ParmContainer.  Update vehicle on change.
    */
    virtual void ParmChanged( Parm* parm_ptr, int type );

public:
    /*!
    * Is light enabled?
    */
    BoolParm m_Active;

    /*!
    * Light position on x-axis.
    */
    Parm m_X;
    /*!
    * Light position on y-axis.
    */
    Parm m_Y;
    /*!
    * Light position on z-axis.
    */
    Parm m_Z;

    /*!
    * Ambient value of this light.
    */
    Parm m_Amb;
    /*!
    * Diffuse value of this light.
    */
    Parm m_Diff;
    /*!
    * Specular value of this light.
    */
    Parm m_Spec;
};
#endif