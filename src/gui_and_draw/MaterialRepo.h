#if !defined(VSP_MATERIAL_REPOSITORY__INCLUDE_)
#define VSP_MATERIAL_REPOSITORY__INCLUDE_

#include <string>
#include <vector>

/*!
* Repository for materials.
*/
class MaterialRepo
{
public:
    /*!
    * Singleton entry.
    */
    static MaterialRepo * GetInstance()
    {
        static MaterialRepo repo;
        return &repo;
    }

public:
    struct MaterialPref
    {
        std::string name;

        double ambi[4];
        double diff[4];
        double spec[4];
        double emis[4];

        float shininess;
    };

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
    MaterialRepo();
    /*!
    * Destructor.
    */
    virtual ~MaterialRepo();

private:
    std::vector<MaterialPref> m_Materials;
};
#endif