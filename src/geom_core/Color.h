#if !defined(VSP_COLOR__INCLUDED_)
#define VSP_COLOR__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"

/*!
* Color Information.
*/
class Color : public ParmContainer
{
public:
    /*!
    * Construct a color.
    */
    Color();
    /*!
    * Construct a specific color. rgba are range from 0 to 255.
    */
    Color( double r, double g, double b, double a );
    /*!
    * Destructor.
    */
    virtual ~Color();

public:
    /*!
    * Override from ParmContainer.
    */
    virtual void ParmChanged( Parm* parm_ptr, int type );

public:
    /*!
    * Color values.
    */
    Parm m_Red;
    Parm m_Green;
    Parm m_Blue;
    Parm m_Alpha;
};
#endif