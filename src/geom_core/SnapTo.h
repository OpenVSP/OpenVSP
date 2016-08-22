//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SnapTo.h: Collision and min distance code
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(SNAPTO__INCLUDED_)
#define SNAPTO__INCLUDED_

#include "Parm.h"
#include "ParmContainer.h"
#include "TMesh.h"

//==== SnapTo ====//
class SnapTo : public ParmContainer
{
public:
    SnapTo();
    virtual ~SnapTo();
    virtual void ParmChanged( Parm* parm_ptr, int type );

    void PreventCollision( const string & geom_id, const string & parm_id );
    double FindMinDistance(const string & geom_id, const vector< TMesh* > & other_tmesh_vec, bool & intersect_flag ); 
    double FindMaxMinDistance( const vector< TMesh* > & mesh_1, const vector< TMesh* > & mesh_2 ); 
    bool CheckIntersect( Geom* geom_ptr, const vector<TMesh*> & other_tmesh_vec );
    void AdjParmToMinDist( const string & parm_id, bool inc_flag );
    void CheckClearance(  );


    //==== Collision Stuff ====//
    BoolParm m_CollisionDetection;
    int m_CollisionSet;
    Parm m_CollisionTargetDist;
    int m_CollisionErrorFlag;
    double m_CollisionMinDist;

protected:

    //===== Store Last Values ====//
    string m_LastParmID;
    double m_LastParmVal;
    double m_LastMinDist;
    double m_LastTargetDist;
    bool m_LastIncFlag;

};




#endif // !defined(SNAPTO__INCLUDED_)
