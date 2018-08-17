#if !defined(VSP_MEASURE_MANAGER__INCLUDED_)
#define VSP_MEASURE_MANAGER__INCLUDED_

#include "Measure.h"
#include "DrawObj.h"

#include <vector>

class MeasureMgrSingleton
{
public:

    static MeasureMgrSingleton& getInstance()
    {
        static MeasureMgrSingleton instance;
        return instance;
    }

    void Renew();

    Ruler * CreateAndAddRuler();
    string CreateAndAddRuler( const string & startgeomid, int startsurfindx, double startu, double startw,
                            const string & endgeomid, int endsurfindx, double endu, double endw, const string & name );

    Probe * CreateAndAddProbe();
    string CreateAndAddProbe( const string & geomid, int surfindx, double u, double w, const string & name );

    Ruler * GetCurrentRuler();
    std::vector < Ruler * > GetRulerVec();

    Probe * GetCurrentProbe();
    std::vector < Probe * > GetProbeVec();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    void LoadDrawObjs( vector < DrawObj * > & draw_obj_vec );

    void SetCurrRulerIndex( int i )             { m_CurrRulerIndex = i; }
    int GetCurrRulerIndex()                   { return m_CurrRulerIndex; }
    void DelAllRulers();
    void ShowAllRulers();
    void HideAllRulers();
    Ruler *  GetRuler( const string &id );
    vector < string > GetAllRulers();
    void DelRuler( const int &i );
    void DelRuler( const string &id );

    void SetCurrProbeIndex( int i )             { m_CurrProbeIndex = i; }
    int GetCurrProbeIndex()                   { return m_CurrProbeIndex; }
    void DelAllProbes();
    void ShowAllProbes();
    void HideAllProbes();
    Probe * GetProbe( const string &id );
    vector < string > GetAllProbes();
    void DelProbe( const int &i );
    void DelProbe( const string &id );

    void DeleteInvalid();
    void Update();

private:

    std::vector < Ruler * > m_Rulers;
    std::vector < Probe * > m_Probes;

    int m_CurrRulerIndex;
    int m_CurrProbeIndex;

    void UpdateDrawObjs();


    MeasureMgrSingleton();
    MeasureMgrSingleton( MeasureMgrSingleton const& copy );          // Not Implemented
    MeasureMgrSingleton& operator=( MeasureMgrSingleton const& copy ); // Not Implemented

    void Init();
    void Wype();
};

#define MeasureMgr MeasureMgrSingleton::getInstance()

#endif
