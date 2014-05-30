#if !defined(VSP_LIGHTS_MANAGER__INCLUDED_)
#define VSP_LIGHTS_MANAGER__INCLUDED_

#include "Light.h"

#include <vector>

#define NUMOFLIGHTS 8

/*!
* This class keep tracks list of lights.
*/
class LightMgr
{
public:
    static LightMgr * getInstance()
    {
        static LightMgr lightMgr;
        return &lightMgr;
    }

protected:
    /*!
    * Construct a list of lights.
    */
    LightMgr();
    /*!
    * Destructor.
    */
    virtual ~LightMgr();

public:
    /*!
    * Get light info at index.  The range of index
    * is defined by the value in NUMOFLIGHTS. Return
    * NULL if index is not valid.
    */
    Light * Get( unsigned int index );

    /*!
    * Get all lights in a vector.
    */
    std::vector< Light* > GetVec();

private:
    std::vector< Light* > m_Lights;
};
#endif