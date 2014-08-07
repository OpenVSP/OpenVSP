#if !defined(VSP_MATERIAL_REPOSITORY__INCLUDE_)
#define VSP_MATERIAL_REPOSITORY__INCLUDE_

#include <string>
#include <vector>


struct MaterialPref
{
    std::string name;

    double ambi[4];
    double diff[4];
    double spec[4];
    double emis[4];

    float shininess;
};

/*!
* Repository for materials.
*/
class MaterialRepoSingleton
{
public:
    /*!
    * Singleton entry.
    */
    static MaterialRepoSingleton& GetInstance()
    {
        static MaterialRepoSingleton repo;
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
    MaterialRepoSingleton();
    /*!
    * Destructor.
    */
    virtual ~MaterialRepoSingleton();

private:
    std::vector<MaterialPref> m_Materials;
};

#define MaterialRepo MaterialRepoSingleton::GetInstance()

#endif
