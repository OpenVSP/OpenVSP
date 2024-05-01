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

    void DeleteInvalid();
    void Update();

private:

    std::vector < Background3D * > m_Background3Ds;

    int m_CurrBackground3DIndex;

    static void UpdateDrawObjs();


    Background3DMgrSingleton();
    Background3DMgrSingleton( Background3DMgrSingleton const& copy );          // Not Implemented
    Background3DMgrSingleton& operator=( Background3DMgrSingleton const& copy ); // Not Implemented

    static void Init();
    void Wype();


    DrawObj m_LightingDO;
};

#define Background3DMgr Background3DMgrSingleton::getInstance()

#endif
