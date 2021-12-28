#if !defined(VSP_LIGHTS_MANAGER__INCLUDED_)
#define VSP_LIGHTS_MANAGER__INCLUDED_

#include "Light.h"

#include <vector>

#define NUMOFLIGHTS 8

/*!
* This class keep tracks list of lights.
*/
class LightMgrSingleton
{
public:
    static LightMgrSingleton & getInstance()
    {
        static LightMgrSingleton instance;
        return instance;
    }

protected:
    /*!
    * Construct a list of lights.
    */
    LightMgrSingleton();
    /*!
    * Destructor.
    */
    virtual ~LightMgrSingleton();

public:

    void Init();

    void Wype();

    void Renew();

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

public:
    /*!
    * Encode lights information to xml.
    */
    xmlNodePtr EncodeXml( xmlNodePtr node );
    /*!
    * Decode lights information from xml.
    */
    xmlNodePtr DecodeXml( xmlNodePtr node );

private:
    std::vector< Light* > m_Lights;
};

#define LightMgr LightMgrSingleton::getInstance()

#endif