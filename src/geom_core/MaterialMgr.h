#if !defined(VSP_MATERIAL_REPOSITORY__INCLUDE_)
#define VSP_MATERIAL_REPOSITORY__INCLUDE_

#include "ParmContainer.h"
#include "Parm.h"

#include <string>
#include <vector>

class MaterialPref
{
public:
    MaterialPref();
    virtual ~MaterialPref();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );


    void SetMaterialToDefault( );
    void SetMaterial( MaterialPref * material );
    void SetMaterial( std::string name, double ambi[], double diff[], double spec[], double emis[], double shin );


    std::string m_Name;

    double m_Ambi[4];
    double m_Diff[4];
    double m_Spec[4];
    double m_Emis[4];

    float m_Shininess;
};

/*!
* Repository for materials.
*/
class MaterialMgrSingleton
{
public:
    /*!
    * Singleton entry.
    */
    static MaterialMgrSingleton& GetInstance()
    {
        static MaterialMgrSingleton repo;
        return repo;
    }

public:
    /*!
    * Find material with specific name.
    */
    bool FindMaterial( std::string name, MaterialPref& mat_out);
    /*!
    * Find mateiral with index.
    */
    bool FindMaterial( int index, MaterialPref& mat_out );
    /*!
    * Get all material names.
    */
    std::vector<std::string> GetNames();

protected:
    /*!
    * Constructor.
    */
    MaterialMgrSingleton();
    /*!
    * Destructor.
    */
    virtual ~MaterialMgrSingleton();

private:
    std::vector<MaterialPref> m_Materials;
};

#define MaterialMgr MaterialMgrSingleton::GetInstance()

#endif
