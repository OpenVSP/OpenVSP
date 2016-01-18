//
//
// Packaging.h: Packaging data and algorythms
//
//////////////////////////////////////////////////////////////////////

#include "APIDefines.h"
#include "Packaging.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "ParmMgr.h"

Packaging::Packaging() : ParmContainer()
{
    m_Name = "Packaging";

    m_CollisionSet = vsp::SET_SHOWN;
    m_CollisionDetection.Init( "ActiveCollision", "Collision", this, false, 0, 1 );
    m_CollisionTargetDist.Init( "CollisionTargetDist", "Collision", this, 0, 1.0e-06, 1e6 );
    m_CollisionErrorFlag = vsp::COLLISION_OK;
    m_CollisionMinDist = 0.0;


}

Packaging::~Packaging()
{
}

xmlNodePtr Packaging::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr cfdsetnode = xmlNewChild( node, NULL, BAD_CAST"Packaging", NULL );

    ParmContainer::EncodeXml( cfdsetnode );

//    XmlUtil::AddStringNode( cfdsetnode, "FarFieldGeomID", m_FarGeomID );

    return cfdsetnode;
}

xmlNodePtr Packaging::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr cfdsetnode = XmlUtil::GetNode( node, "Packaging", 0 );
    if ( cfdsetnode )
    {
        ParmContainer::DecodeXml( cfdsetnode );
//        m_FarGeomID   = XmlUtil::FindString( cfdsetnode, "FarFieldGeomID", m_FarGeomID );
    }

    return cfdsetnode;
}

//==== Parm Changed ====//
void Packaging::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, Parm::SET );
    }
}


void Packaging::PreventCollision( const string & geom_id, const string & parm_id )
{
    MessageMgr::getInstance().Send( "ScreenMgr", "CheckCollisionKey" );

    if ( !m_CollisionDetection() )
        return;

    Geom* geom_ptr = VehicleMgr.GetVehicle()->FindGeom( geom_id );
    if ( !geom_ptr )        { return; }

    Parm* parm_ptr = ParmMgr.FindParm( parm_id );
    if ( !parm_ptr )        { return; }

    //==== Save Vals ====//
    double orig_val = parm_ptr->Get();
    double last_val = parm_ptr->GetLastVal();

    parm_ptr->Set( last_val );

    bool inc_flag = true;
    if ( last_val > orig_val )
        inc_flag = false;

    AdjParmToMinDist( parm_id, inc_flag );
}

//===== Vectors of TMeshs with Bounding Boxes Already Set Up ====//
bool Packaging::CheckIntersect( Geom* geom_ptr, const vector<TMesh*> & other_tmesh_vec )
{
    bool intsect_flag = false;

    vector< TMesh* > tmesh_vec = geom_ptr->CreateTMeshVec();
    for ( int i = 0 ; i < (int)tmesh_vec.size() ; i++ )
    {
        tmesh_vec[i]->LoadBndBox();
        for ( int j = 0 ; j < (int)other_tmesh_vec.size() ; j++ )
        {
            if ( tmesh_vec[i]->CheckIntersect( other_tmesh_vec[j] ) )
            {
                    intsect_flag = true;
                    break;
            }
        }
        if ( intsect_flag )
            break;
    }

    for ( int i = 0 ; i < (int)tmesh_vec.size() ; i++ )
    {
        delete tmesh_vec[i];
    }

    return intsect_flag;
}

//==== Returns Large Neg Number If Error and 0.0 If Collision ====//
double Packaging::FindMinDistance( const string & geom_id, const vector< TMesh* > & other_tmesh_vec, bool & intersect_flag )
{
    intersect_flag = false;
    Geom* geom_ptr = VehicleMgr.GetVehicle()->FindGeom( geom_id );
    if ( !geom_ptr )    return -1.0e12;

    if ( CheckIntersect( geom_ptr, other_tmesh_vec ) )
    {
        intersect_flag = true;
        return 0.0;
    }

    vector< TMesh* > tmesh_vec = geom_ptr->CreateTMeshVec();        // Must Delete!!!

    //==== Find Min Dist ====//
    double min_dist = 1.0e12;
    for ( int i = 0 ; i < (int)tmesh_vec.size() ; i++ )
    {
        tmesh_vec[i]->LoadBndBox();
        for ( int j = 0 ; j < (int)other_tmesh_vec.size() ; j++ )
        {
            double d =  tmesh_vec[i]->MinDistance(  other_tmesh_vec[j], min_dist );
            min_dist = min( d, min_dist );
        }
    }

    for ( int i = 0 ; i < (int)tmesh_vec.size() ; i++ )
    {
        delete tmesh_vec[i];
    }

    return min_dist;
}

//===== Find The Min Distance For Each Point And Returns Max =====//
double Packaging::FindMaxMinDistance( const vector< TMesh* > & mesh_vec_1, const vector< TMesh* > & mesh_vec_2 )
{
    double max_dist = 0.0;

    if ( mesh_vec_1.size() != mesh_vec_2.size() )
        return max_dist;

    for ( int i = 0 ; i < (int)mesh_vec_1.size() ; i++ )
    {
        TMesh* tm1 = mesh_vec_1[i];
        TMesh* tm2 = mesh_vec_2[i];

        if ( tm1->m_NVec.size() == tm2->m_NVec.size() )
        {
            for ( int m = 0 ; m < tm1->m_NVec.size() ; m++ )
            {
                double d2 = dist_squared( tm1->m_NVec[m]->m_Pnt, tm2->m_NVec[m]->m_Pnt );
                max_dist = max( max_dist, d2 );
            }
        }
    }
    return sqrt( max_dist );
}

void Packaging::AdjParmToMinDist( const string & parm_id, bool inc_flag )
{
    Parm* parm_ptr = ParmMgr.FindParm( parm_id );
    if ( !parm_ptr )   return;

    string geom_id = parm_ptr->GetLinkContainerID();
    Geom* geom_ptr = VehicleMgr.GetVehicle()->FindGeom( geom_id );
    if ( !geom_ptr )    return;

    Vehicle* veh = VehicleMgr.GetVehicle();

    //==== Find Other Geoms ====//
    vector< string > geom_id_vec = veh->GetGeomSet( m_CollisionSet );
    vector< string > other_geom_vec;
    for ( int i = 0 ; i < (int)geom_id_vec.size() ; i++ )
    {
        if ( geom_id != geom_id_vec[i] )
            other_geom_vec.push_back( geom_id_vec[i] );
    }

    //==== Create TMeshes =====//
    vector< TMesh* > other_tmesh_vec;
    for ( int i = 0 ; i < (int)other_geom_vec.size() ; i++ )
    {
        Geom* g_ptr = veh->FindGeom( other_geom_vec[i] );
        if ( g_ptr )
        {
             vector< TMesh* > tvec = g_ptr->CreateTMeshVec();       /////////// MUST DELETE!!!
             for ( int j = 0 ; j < (int)tvec.size() ; j++ )
             {
                tvec[j]->LoadBndBox();
                other_tmesh_vec.push_back( tvec[j] );
             }
        }
    }

    double direction = 1.0;
    if ( !inc_flag )
        direction = -1.0;

    double orig_val = parm_ptr->Get();
    double v_in  = orig_val;
    double v_out = orig_val;       

    //==== Create Geom TMesh Vec. Adjust Parm And Create Again ====//
    double del_val = 0.01;
    vector< TMesh* > tmesh_orig = geom_ptr->CreateTMeshVec();       // Must Delete
    parm_ptr->Set( orig_val + del_val );
    VehicleMgr.GetVehicle()->Update( false );
    vector< TMesh* > tmesh_adj  = geom_ptr->CreateTMeshVec();       // Must Delete

    double max_min = FindMaxMinDistance( tmesh_orig, tmesh_adj );   // Find Max Dist Change Of Mesh 
    max_min = max( max_min, 1.0e-06 );

    //==== Deleting TMeshes ====//
    for ( int i = 0 ; i < (int)tmesh_orig.size() ; i++ )
        delete tmesh_orig[i];
    for ( int i = 0 ; i < (int)tmesh_adj.size() ; i++ )
        delete tmesh_adj[i];
    parm_ptr->Set( orig_val );                                      // Restore Value
    veh->Update( false );

    //==== Find Reasonable Range For Val =====//
    double model_size = veh->GetBndBox().DiagDist();
    double val_range = (model_size*del_val)/max_min;

    //===== Parm Limits ====//
    double limit = parm_ptr->GetLowerLimit();
    if ( inc_flag )
        limit = parm_ptr->GetUpperLimit();

    //==== Limits Near Zero Can Cause Incorrect Collision Results ====//
    if ( fabs(limit) < 0.0001 ) limit = 0.0001;

    //==== Limit Range =====//
    val_range = min( fabs(limit - orig_val), val_range );

    //==== Move Geom Close To Other Body In Correct Direction ====//
    int cnt = 0;
    bool init_col_flag = CheckIntersect( geom_ptr, other_tmesh_vec );

    //==== Step Forward To Find First Opposite of Collision Flag (col_flag)      ====//
    //==== This Could Be Faster But Might Skip Over Possible Solns (Still Might) ====//
    bool found_flag = false;
    for ( int i = 1 ; i <= 20 ; i++ )
    {
        double fract = (double)(i*i)/400.0;     // Closer Spaced Near Init Point
        double val = orig_val + direction*val_range*fract;
        parm_ptr->Set( val );
        veh->Update( false );
        bool col_flag =  CheckIntersect( geom_ptr, other_tmesh_vec );

        if ( !col_flag )
        {
            v_out = val;
            if ( init_col_flag )
            {
                found_flag = true;
                break;
            }
        }
        else
        {
            v_in = val;
            if ( !init_col_flag )
            {
                found_flag = true;
                break;
            }
        }
    }

    //==== Nothing Changed - Return ====//
    if ( !found_flag  )
    {
        if ( init_col_flag )
            m_CollisionErrorFlag = vsp::COLLISION_INTERSECT_NO_SOLUTION;
        else
            m_CollisionErrorFlag = vsp::COLLISION_CLEAR_NO_SOLUTION;
        parm_ptr->Set( orig_val );              // Restore Val
        veh->Update( false );
        for ( int i = 0 ; i < (int)other_tmesh_vec.size() ; i++ )
            delete other_tmesh_vec[i];
        return;
    }

    //==== Use BiSection To Refine In/Out Solutions ====//
    for ( int i = 0 ; i < 5 ; i++ )
    {
        double val = (v_in + v_out)*0.5;
        parm_ptr->Set( val );
        veh->Update( false );
        bool col_flag =  CheckIntersect( geom_ptr, other_tmesh_vec );

        if( col_flag )
            v_in = val;
        else
            v_out = val;
    }

    //==== Find Del Min Dist to Del Val And Iterate Soln ====//
    bool iflag;
    double v0 = v_out;
    double closest_err = 1.0e12;
    for ( int i = 0 ; i < 5 ; i++ )
    {
        parm_ptr->Set( v0 );
        veh->Update( false );
        double d0 = FindMinDistance( geom_id, other_tmesh_vec, iflag );

        //==== Check For Intersect ====//
        if ( iflag )
            break;

        //==== Save Best Soln ====//
        double err = fabs( d0 - m_CollisionTargetDist() );
        if ( err < closest_err )
        {
            v_out = v0;
            closest_err = err;

            //==== Good Enough Soln? ====//
            if ( closest_err < 1.0e-06*model_size ) 
            {
                break;
            }
        }

        //==== Slightly Offset v_out ====// 
        double v1 = (v_out - v_in)*0.0001 + v_out;        // Away From v_in
        parm_ptr->Set( v1 );
        veh->Update( false );
        double d1 = FindMinDistance( geom_id, other_tmesh_vec, iflag );

        //==== Check For Intersect ====//
        if ( iflag )
            break;

        double denom = d1 - d0;
        if ( fabs( denom ) < 1.0e-12 )
            break;

        double fract = ( m_CollisionTargetDist() - d0 )/denom;
        double val = v0 + fract*(v1 - v0);

        //==== Check If Predicted Point Intersects ====//
        parm_ptr->Set( val );
        veh->Update( false );
        if ( CheckIntersect( geom_ptr, other_tmesh_vec ) )
        {
            val = v0 + 0.5*fract*(v1 - v0);         // Only go half way            
        }
        v0 = val;
    }

    //==== Best Soln is v_out ====//
    parm_ptr->Set( v_out );
    veh->Update( true );

    m_CollisionMinDist = FindMinDistance( geom_id, other_tmesh_vec, iflag );
    m_CollisionErrorFlag = vsp::COLLISION_OK;

    //==== Delete Created TMeshes ====//
    for ( int i = 0 ; i < (int)other_tmesh_vec.size() ; i++ )
        delete other_tmesh_vec[i];
}