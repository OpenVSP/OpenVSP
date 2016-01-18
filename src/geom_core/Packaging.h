//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Packaging.h: interface for the Parm Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(PACKAGING__INCLUDED_)
#define PACKAGING__INCLUDED_

#include "Parm.h"
#include "ParmContainer.h"
#include "TMesh.h"

//==== Packaging ====//
class Packaging : public ParmContainer
{
public:
	Packaging();
    virtual ~Packaging();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void ParmChanged( Parm* parm_ptr, int type );

    void PreventCollision( const string & geom_id, const string & parm_id );
    double FindMinDistance(const string & geom_id, const vector< TMesh* > & other_tmesh_vec, bool & intersect_flag ); 
    double FindMaxMinDistance( const vector< TMesh* > & mesh_1, const vector< TMesh* > & mesh_2 ); 
    bool CheckIntersect( Geom* geom_ptr, const vector<TMesh*> & other_tmesh_vec );
    void AdjParmToMinDist( const string & parm_id, bool inc_flag );


    //==== Collision Stuff ====//
    BoolParm m_CollisionDetection;
    int m_CollisionSet;
    Parm m_CollisionTargetDist;
    int m_CollisionErrorFlag;
    double m_CollisionMinDist;

protected:

};




#endif // !defined(PACKAGING__INCLUDED_)
