#if !defined(_VSP_PREFERENCES_LOADER__INCLUDE_)
#define _VSP_PREFERENCES_LOADER__INCLUDE_

#include <vector>

class VspPreferences
{
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
    static VspPreferences& getInstance()
    {
        static VspPreferences instance;
        return instance;
    }

public:
    /*!
    * Get list of all materials in preference file.
    */
    std::vector<MaterialPref> getMaterialPrefs()
    {
        return m_MaterialPrefs;
    }
    /*!
    * Find material with name.
    */
    bool findMaterialPref( std::string name, MaterialPref& out );
    /*!
    * Find material base on index.
    */
    bool findMaterialPref( int index, MaterialPref& out );

protected:
    /*!
    * Constructor.
    */
    VspPreferences();
    /*!
    * Destructor.
    */
    virtual ~VspPreferences();

protected:
    std::vector<MaterialPref> m_MaterialPrefs;

};
#endif