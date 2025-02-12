//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// InterferenceMgr.h
//
// Rob McDonald
//////////////////////////////////////////////////////////////////////

#if !defined(INTERFERENCEMGR__INCLUDED_)
#define INTERFERENCEMGR__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"
#include "TMesh.h"
#include "DrawObj.h"

class InterferenceCase : public ParmContainer
{
public:

    InterferenceCase();

    void Update();

    string GetPrimaryName() const;
    string GetSecondaryName() const;

    vector< TMesh* > GetPrimaryTMeshVec();
    vector< TMesh* > GetSecondaryTMeshVec();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    string Evaluate();
    void UpdateDrawObj();

    void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    IntParm m_PrimarySet;
    IntParm m_PrimaryType; // Mode, Set, or Geom
    string m_PrimaryModeID;
    string m_PrimaryGeomID;

    IntParm m_SecondarySet;
    IntParm m_SecondaryType; // Set or Geom
    string m_SecondaryGeomID;

    IntParm m_IntererenceCheckType;

    string m_LastResult;
    Parm m_LastResultValue;

    vector< TMesh* > m_TMeshVec;
    vector < vec3d > m_PtsVec;


    vector < DrawObj > m_MeshResultDO_vec;
    DrawObj m_LineResultDO;
};


class InterferenceMgrSingleton
{
protected:
    InterferenceMgrSingleton();

public:

    static InterferenceMgrSingleton& getInstance()
    {
        static InterferenceMgrSingleton instance;
        return instance;
    }

    virtual ~InterferenceMgrSingleton();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    void Renew();
    void Wype();

    virtual void Update();

    string EvaluateAll();

    void AddLinkableContainers( vector< string > & linkable_container_vec );


    string AddInterferenceCase();
    void DeleteInterferenceCase( const string &id );
    void DeleteInterferenceCase( int indx );
    void DeleteAllInterferenceCases();

    InterferenceCase * GetInterferenceCase( int indx ) const;
    InterferenceCase * GetInterferenceCase( const string &id ) const;
    int GetInterferenceCaseIndex( const string &id ) const;

    vector < InterferenceCase* > GetAllInterferenceCases() const            { return m_ICaseVec; };


protected:
    vector < InterferenceCase* > m_ICaseVec;

};

#define InterferenceMgr InterferenceMgrSingleton::getInstance()

#endif
