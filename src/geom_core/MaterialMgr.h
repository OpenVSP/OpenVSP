#if !defined(VSP_MATERIAL_REPOSITORY__INCLUDE_)
#define VSP_MATERIAL_REPOSITORY__INCLUDE_

#include "ParmContainer.h"
#include "Parm.h"

#include <string>
#include <vector>

class Material
{
public:
    Material();
    virtual ~Material();

    virtual xmlNodePtr EncodeNameXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeNameXml( xmlNodePtr & node );

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    void SetMaterialToDefault( );
    void SetMaterial( Material * material );
    void SetMaterial( std::string name, double ambi[], double diff[], double spec[], double emis[], double shin );

    void SetAmbient( vec3d color );
    void SetDiffuse( vec3d color );
    void SetSpecular( vec3d color );
    void SetEmissive( vec3d color );
    void SetAlpha( double alpha );
    void SetShininess( double shiny );

    void GetAmbient( vec3d &color );
    void GetDiffuse( vec3d &color );
    void GetSpecular( vec3d &color );
    void GetEmissive( vec3d &color );
    void GetAlpha( double &alpha );
    void GetShininess( double &shiny );


    std::string m_Name;

    double m_Ambi[4];
    double m_Diff[4];
    double m_Spec[4];
    double m_Emis[4];

    float m_Shininess;

    bool m_UserMaterial;
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
    bool FindMaterial( std::string name, Material& mat_out);
    /*!
    * Find mateiral with index.
    */
    bool FindMaterial( int index, Material& mat_out );
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
    std::vector<Material> m_Materials;
};

#define MaterialMgr MaterialMgrSingleton::GetInstance()

#endif
