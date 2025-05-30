#if !defined(VSP_BACKGROUND3D_MANAGER__INCLUDED_)
#define VSP_BACKGROUND3D_MANAGER__INCLUDED_

#include "Background3D.h"
#include "DrawObj.h"

#include <vector>

class Background3DMgrSingleton
{
public:

    static Background3DMgrSingleton& getInstance()
    {
        static Background3DMgrSingleton instance;
        return instance;
    }

    void Renew();

    Background3D * CreateAndAddBackground3D();

    Background3D * GetCurrentBackground3D();
    std::vector < Background3D * > GetBackground3DVec();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    void LoadDrawObjs( vector < DrawObj * > & draw_obj_vec );

    void SetCurrBackground3DIndex( int i );
    int GetCurrBackground3DIndex()                   { return m_CurrBackground3DIndex; }
    void DelAllBackground3Ds();
    void ShowAllBackground3Ds();
    void HideAllBackground3Ds();
    Background3D * GetBackground3D( const string &id );
    vector < string > GetAllBackground3Ds();
    void DelBackground3D( const int &i );
    void DelBackground3D( const string &id );

    void Update();

    int GetNumBackground3Ds();

    vector < string > GetAllBackground3DRelativePaths();
    vector < string > GetAllBackground3DAbsolutePaths();

    string GetBackground3DRelativePath( const string &id );
    string GetBackground3DAbsolutePath( const string &id );
    void SetBackground3DRelativePath( const string &id, const string &fname );
    void SetBackground3DAbsolutePath( const string &id, const string &fname );

private:

    std::vector < Background3D * > m_Background3Ds;

    int m_CurrBackground3DIndex;

    static void UpdateDrawObjs();


    Background3DMgrSingleton();
    Background3DMgrSingleton( Background3DMgrSingleton const& copy ) = delete;          // Not Implemented
    Background3DMgrSingleton& operator=( Background3DMgrSingleton const& copy ) = delete; // Not Implemented

    static void Init();
    void Wype();


    DrawObj m_LightingDO;
};

#define Background3DMgr Background3DMgrSingleton::getInstance()

#endif
