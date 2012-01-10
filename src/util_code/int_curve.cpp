//******************************************************************************
//    
//   Intersection Curve Class
//  
// 
//   J.R. Gloudemans - 2/3/94
//   Sterling Software
//
//******************************************************************************

#include "int_curve.h"
#include "herm_curve.h"
#include "bezier_surf.h"
#include "util.h"
#include <math.h>

//===== Constructor  =====//
int_curve::int_curve()
{
  valid_flag = TRUE;
  surf1 = 0;
  surf2 = 0;
  id_number = -1;  
}

//===== Destructor  =====//
int_curve::~int_curve()
{
  clear_space();
}

//===== Join Intersection Curves  =====//
void int_curve::clear_space()
{
  while( !node_list.end_of_list() )
    {
      delete node_list.get_curr();
      node_list.remove_curr(); 
    }
  for ( int i = 0 ; i < split_list.dimension() ; i++ )
    {
      delete split_list[i];
    }
  split_list.init(0);

}

//===== Start Curve Segment  =====//
void int_curve::start( vec3d in_pnt[2], double u1[2], double w1[2], double u2[2], double w2[2] )
{
  for ( int i = 0 ; i < 2 ; i++ )
    { 
      int_node* new_node = new int_node;

      node_list.insert_after(new_node);
      new_node->pnt = in_pnt[i];
      new_node->u1  = u1[i];
      new_node->w1  = w1[i];
      new_node->u2  = u2[i];
      new_node->w2  = w2[i];
    }
  set_first_last_nodes();
  
}
//===== Join Intersection Curves  =====//
void int_curve::set_first_last_nodes()
{
  node_list.reset();
  first_node =  node_list.get_curr(); 
  node_list.set_end();
  last_node = node_list.get_curr();
}


//===== Load Pnts Into Array  =====//
void int_curve::load_all_pnts_into_array( dyn_array<vec3d>& pnt_list)
{
  node_list.reset();
  while( !node_list.start_of_list() )
    {
      pnt_list.append( node_list.get_curr()->pnt );
      node_list.inc();
    }
}

//===== Find Minimum Dist Between End Points of Two Curve =====//
double int_curve::min_dist_ends( int_curve& in_crv )
{
  double new_dist = 0.0;

  double min_dist = dist_squared( get_first_pnt(),  in_crv.get_first_pnt());
 
  new_dist = dist_squared( get_first_pnt(),  in_crv.get_last_pnt());
  if ( new_dist < min_dist ) min_dist = new_dist;
   
  new_dist = dist_squared( get_last_pnt(),  in_crv.get_first_pnt());
  if ( new_dist < min_dist ) min_dist = new_dist;

  new_dist = dist_squared( get_last_pnt(),  in_crv.get_last_pnt());
  if ( new_dist < min_dist ) min_dist = new_dist;

  return(min_dist);
}


//===== Find Which CASE Curve Has Minimum Dist Between End Points of Two Curve =====//
int int_curve::case_min_dist_ends( int_curve& in_crv )
{
  double new_dist = 0.0;

  int case_return = FIRST_FIRST;
  
  double min_dist = dist_squared( get_first_pnt(), in_crv.get_first_pnt());
 
  new_dist = dist_squared( get_first_pnt(), in_crv.get_last_pnt());
  if ( new_dist < min_dist )
    { 
      min_dist = new_dist;
      case_return = FIRST_LAST;
    }
   
  new_dist = dist_squared( get_last_pnt(), in_crv.get_first_pnt());
  if ( new_dist < min_dist )
    { 
      min_dist = new_dist;
      case_return = LAST_FIRST;
    }
  new_dist = dist_squared( get_last_pnt(), in_crv.get_last_pnt());
  if ( new_dist < min_dist )
    { 
      min_dist = new_dist;
      case_return = LAST_LAST;
    }
  return(case_return);
}

//===== Join Intersection Curves  =====//
void int_curve::join( int_curve& in_crv )
{

  int join_case = this->case_min_dist_ends( in_crv );

  if ( duplicate_second_nodes( in_crv, join_case )) return;

  switch ( join_case )
    {
      case FIRST_FIRST:
        {
          node_list.reset();
          in_crv.node_list.reset();
          delete in_crv.node_list.get_curr();      
          in_crv.node_list.remove_curr();
          while( !in_crv.node_list.end_of_list() )
            {
              node_list.insert_before(in_crv.node_list.get_curr());
              node_list.dec();
              in_crv.node_list.remove_curr();
            }
        }
      break;
      case FIRST_LAST:
        {
          node_list.reset();
          in_crv.node_list.set_end();     
          delete in_crv.node_list.get_curr();      
          in_crv.node_list.remove_curr();
          while( !in_crv.node_list.start_of_list() )
            {
              node_list.insert_before(in_crv.node_list.get_curr());
              node_list.dec();
              in_crv.node_list.remove_curr();
            }
        }
      break;

      case LAST_FIRST:
        {
          node_list.set_end();
          in_crv.node_list.reset();      
          delete in_crv.node_list.get_curr();      
          in_crv.node_list.remove_curr();
          while( !in_crv.node_list.end_of_list() )
            {
              node_list.insert_after(in_crv.node_list.get_curr());
              node_list.inc();
              in_crv.node_list.remove_curr();
            }
        }
      break;

      case LAST_LAST:
        {
          node_list.set_end();
          in_crv.node_list.set_end();
          delete in_crv.node_list.get_curr();      
          in_crv.node_list.remove_curr();
          while( !in_crv.node_list.start_of_list() )
            {
              node_list.insert_after(in_crv.node_list.get_curr());
              node_list.inc();
              in_crv.node_list.remove_curr();
            }
        }
      break;
    }
          
  set_first_last_nodes();

}

//===== Join Intersection Curves  =====//
int int_curve::duplicate_second_nodes( int_curve& in_crv, int join_case )
{
  double Dist_tol = 0.0000001;
  int_node* n1 = 0;
  int_node* n2 = 0;

  switch ( join_case )
    {
      case FIRST_FIRST:
        {
          node_list.reset();
          node_list.inc();
          n1 = node_list.get_curr();

          in_crv.node_list.reset();
          in_crv.node_list.inc();
          n2 = in_crv.node_list.get_curr();
        }
      break;
      case FIRST_LAST:
        {
          node_list.reset();
          node_list.inc();
          n1 = node_list.get_curr();

          in_crv.node_list.set_end();
          in_crv.node_list.dec();
          n2 = in_crv.node_list.get_curr();
        }
      break;

      case LAST_FIRST:
        {
          node_list.set_end();
          node_list.dec();
          n1 = node_list.get_curr();

          in_crv.node_list.reset();
          in_crv.node_list.inc();
          n2 = in_crv.node_list.get_curr();
        }
      break;

      case LAST_LAST:
        {
          node_list.set_end();
          node_list.dec();
          n1 = node_list.get_curr();

          in_crv.node_list.set_end();
          in_crv.node_list.dec();
          n2 = in_crv.node_list.get_curr();

        }
      break;
    }
  if ( dist_squared( n1->pnt, n2->pnt) < Dist_tol )
    return(1);

  return(0);


}


//===== Dump Even Spaced  Points  =====//
void int_curve::dump_even_spaced_pnts( )
{
  float dump_num_pnt_mult = 2.0;

  herm_curve tmp_crv;

cout << " Int_Curve = " << this << " BS1 = " <<  surf1  << " BS2 = " <<  surf2 << endl;
  node_list.reset();
  tmp_crv.init(node_list.num_objects());
  for ( int i = 0 ; i < node_list.num_objects() ; i++ )
    {
double u1 = node_list.get_curr()->u1;
double w1 = node_list.get_curr()->w1;
double u2 = node_list.get_curr()->u2;
double w2 = node_list.get_curr()->w2;

cout << " Node = " << i << " u1 = " << u1 << " w1 = " << w1 
                        << " u2 = " << u2 << " w2 = " << w2 << endl;
vec3d pnt  = node_list.get_curr()->pnt;
vec3d pnt1 = surf1->comp_pnt(node_list.get_curr()->u1, node_list.get_curr()->w1 );
vec3d pnt2 = surf2->comp_pnt(node_list.get_curr()->u2, node_list.get_curr()->w2 );


//cout << " Pnt = " << pnt << " Dist_1 = " << dist(pnt, pnt1) << " Dist_2 = " << dist(pnt, pnt2) << endl;

      tmp_crv.load_pnt(i, node_list.get_curr()->pnt);
      node_list.inc();
    }
  
  tmp_crv.comp_tans();

  int num_dump_pnts = (int)(node_list.num_objects()*dump_num_pnt_mult);

/******************  
  
  cout << endl;
  cout << " INT_CURVE " << endl;
  cout << " COMPONENT NUMBER =    1 " << endl;
  cout << " COMPONENT COLOR =     3 " << endl;
  cout << " CROSS SECTIONS =      1"  << endl;
  cout << " PTS/CROSS SECTION =   "   << num_dump_pnts << endl;

  for ( i = 0 ; i < num_dump_pnts ; i++ )
    {
      cout << tmp_crv.comp_pnt_per_length((float)i/(float)(num_dump_pnts-1)) << endl;
           

    } 
*****************/     

}


//===== Dump Even Spaced  Points  =====//
void int_curve::dump_even_spaced_pnts_felisa(FILE* file_id )
{
  int i = 0;
  float dump_num_pnt_mult = 8.0;

  herm_curve tmp_crv;

  node_list.reset();
  tmp_crv.init(node_list.num_objects());
  for (  i = 0 ; i < node_list.num_objects() ; i++ )
    {
      tmp_crv.load_pnt(i, node_list.get_curr()->pnt);
      node_list.inc();
    }
  
  tmp_crv.comp_tans();

  int num_dump_pnts = (int)(node_list.num_objects()*dump_num_pnt_mult);

  fprintf(file_id, "%d	   1\n",id_number+1);
  fprintf(file_id, "%d\n",num_dump_pnts);

  vec3d new_pnt;
  for ( i = 0 ; i < num_dump_pnts ; i++ )
    {
      new_pnt = tmp_crv.comp_pnt_per_length((float)i/(float)(num_dump_pnts-1));
      fprintf(file_id, "%f  %f  %f \n", new_pnt.x(), new_pnt.y(), new_pnt.z());
    }      

}

//===== Dump Intersection Curves  =====//
void int_curve::dump( )
{

  cout << endl;
  cout << " INT_CURVE " << endl;
  cout << " COMPONENT NUMBER =    1 " << endl;
  cout << " COMPONENT COLOR =     3 " << endl;
  cout << " CROSS SECTIONS =      1" << endl;
  cout << " PTS/CROSS SECTION =   " << node_list.num_objects() << endl;

//  cout << " u           w " << endl;

  node_list.reset();
  while( !node_list.start_of_list() )
    {
//      cout << node_list.get_curr()->pnt << endl;

//      cout  << node_list.get_curr()->u1 << " " << node_list.get_curr()->w1 << endl;
      node_list.inc();
    }

}


//===== Split Intersection Curves  =====//
int_curve* int_curve::split_if_loop(  )
{
  int_curve* new_crv = 0;
  double Dist_tol = 0.0000001;
  vec3d first_pnt = get_first_pnt();
  vec3d last_pnt  = get_last_pnt();

  if ( dist(first_pnt, last_pnt) <  Dist_tol )
    {
      new_crv = new int_curve;

      int num_pnts = node_list.num_objects()/2;
      
      node_list.reset();
      for ( int i = 0 ; i < num_pnts ; i++ )
        {
          new_crv->load_node(node_list.get_curr());
          node_list.remove_curr();
        }
      new_crv->load_node(node_list.get_curr());

      return(new_crv);
    }
  return(new_crv);    
}

//===== Split Intersection Curves  =====//
void int_curve::load_node( int_node* new_node )
{
  if ( node_list.num_objects() == 0 )
    {
      first_node = new_node;
    }
  node_list.set_end();
  node_list.insert_after(new_node);
  last_node = new_node;

}


//===== Intersect Intersection Curves  =====//
void int_curve::intersect_int_curve( int_curve& in_crv )
{
//---- Took Out Local Node Assignments ---- jrg97 //
//   int_node* n1;
//   int_node* n2;
 
  node_list.reset();
  node_list.inc();
//   n1 = node_list.get_curr();

  in_crv.node_list.reset();
  in_crv.node_list.inc();
//   n2 = in_crv.node_list.get_curr();

}


//===== Dump Intersection Curves  =====//
void int_curve::dump_uw(int surf_id )
{
  
  cout << " u           w " << endl;

  node_list.reset();
  int num_pnts = node_list.num_objects();

  if ( surf_id == 0 ) 
    {
      for ( int i = 0 ; i < num_pnts ; i++ )
        {
          cout  << node_list.get_curr()->u1 << "     " << 
                   node_list.get_curr()->w1 << endl;
          node_list.inc();
        }
    }
  else
    {
      for ( int i = 0 ; i < num_pnts ; i++ )
        {
          cout  << node_list.get_curr()->u2 << "     " <<  
                   node_list.get_curr()->w2 << endl;
          node_list.inc();
        }
    }
}


//===== Intersect Intersection Curves  =====//
void intersect( int_curve* ic_A, int_curve* ic_B )
{

  int cnt = 0;
  int A_flag[4];
  int B_flag[4];

  if ( ic_A->surf1 == ic_B->surf1 )
    {
      A_flag[cnt] = 0;	B_flag[cnt] = 0; 
      cnt++;
    }
  if ( ic_A->surf1 == ic_B->surf2 )
    {
      A_flag[cnt] = 0;	B_flag[cnt] = 1; 
      cnt++;
    }
  if ( ic_A->surf2 == ic_B->surf1 )
    {
      A_flag[cnt] = 1;	B_flag[cnt] = 0; 
      cnt++;
    }
  if ( ic_A->surf2 == ic_B->surf2 )
    {
      A_flag[cnt] = 1;	B_flag[cnt] = 1; 
      cnt++;
    }

  if ( !cnt ) return;

  int i=0, j=0;

  dyn_array<int_node*> A_nodes;
  ic_A->node_list.reset();
  int num_uw_A = ic_A->node_list.num_objects();
  A_nodes.init(num_uw_A);
  for ( i = 0 ; i < num_uw_A ; i++ )
    {
      A_nodes[i] = ic_A->node_list.get_curr();
      ic_A->node_list.inc();
    }

  dyn_array<int_node*> B_nodes;
  ic_B->node_list.reset();
  int num_uw_B = ic_B->node_list.num_objects();
  B_nodes.init(num_uw_B);
  for ( i = 0 ; i < num_uw_B ; i++ )
    {
      B_nodes[i] = ic_B->node_list.get_curr();
      ic_B->node_list.inc();
    }

  for ( i = 0 ; i < num_uw_A-1 ; i++ )
    {
      for ( j = 0 ; j < num_uw_B-1 ; j++ )
        {
//          for ( k = 0 ; k < cnt ; k++ )
//            intersect_nodes( ic_A, ic_B, A_flag[k], B_flag[k], A_nodes[i], A_nodes[i+1],  B_nodes[j], B_nodes[j+1]); 
          intersect_nodes( ic_A, ic_B, A_flag[0], B_flag[0], A_nodes[i], A_nodes[i+1],  B_nodes[j], B_nodes[j+1]); 
        }
    }

/********************************************
  for ( i = 0 ; i < num_uw_A-1 ; i++ )
    {
      for ( k = 0 ; k < cnt ; k++ )
        {
      intersect_end_nodes( ic_A, A_flag[k], B_flag[k], A_nodes[i], A_nodes[i+1], B_nodes[0]);
      intersect_end_nodes( ic_A, A_flag[k], B_flag[k], A_nodes[i], A_nodes[i+1], B_nodes[num_uw_B-1]);
        }
    }

  for ( i = 0 ; i < num_uw_B-1 ; i++ )
    {
      for ( k = 0 ; k < cnt ; k++ )
        {
      intersect_end_nodes( ic_B, B_flag[k], A_flag[k], B_nodes[i], B_nodes[i+1], A_nodes[0]);
      intersect_end_nodes( ic_B, B_flag[k], A_flag[k], B_nodes[i], B_nodes[i+1], A_nodes[num_uw_A-1]);
        }
    }
**********************************************/
  
} 

//===== Intersect Intersection Curves  =====//
void intersect_nodes( int_curve* ic_A, int_curve* ic_B, int A_flag, int B_flag, 
        int_node* A_nde1, int_node* A_nde2, int_node* B_nde1, int_node* B_nde2)
{
  vec2d A_uw1, A_uw2;
  vec2d B_uw1, B_uw2;
  vec2d int_uw;

  if ( A_flag ) 
    {
      A_uw1.set_xy(A_nde1->u2, A_nde1->w2);
      A_uw2.set_xy(A_nde2->u2, A_nde2->w2);
    }
  else
    {
      A_uw1.set_xy(A_nde1->u1, A_nde1->w1);
      A_uw2.set_xy(A_nde2->u1, A_nde2->w1);
    }
  if ( B_flag ) 
    {
      B_uw1.set_xy(B_nde1->u2, B_nde1->w2);
      B_uw2.set_xy(B_nde2->u2, B_nde2->w2);
    }
  else
    {
      B_uw1.set_xy(B_nde1->u1, B_nde1->w1);
      B_uw2.set_xy(B_nde2->u1, B_nde2->w1);
    }

  if ( seg_seg_intersect(A_uw1, A_uw2, B_uw1, B_uw2, int_uw) )
    {
      vec3d int_pnt;
      double fract_u=0.0, fract_w=0.0;

      //==== Find Actual Intersection Point ====//
      if ( A_flag ) 
        int_pnt = ic_A->surf2->comp_pnt( int_uw.x(), int_uw.y() );
      else
        int_pnt = ic_A->surf1->comp_pnt( int_uw.x(), int_uw.y() );

      int_node* split_nde_A = new int_node;
      int_node* split_nde_B = new int_node;
    
      split_nde_A->pnt = int_pnt;
      split_nde_B->pnt = int_pnt;

      if ( A_flag ) 
        {
          fract_u =  FRACT(int_uw.x(), A_nde1->u2, A_nde2->u2);              
          fract_w =  FRACT(int_uw.y(), A_nde1->w2, A_nde2->w2); 
          split_nde_A->u2 = int_uw.x();              
          split_nde_A->w2 = int_uw.y();  
          split_nde_A->u1 = fract_u*(A_nde2->u1 - A_nde1->u1) +  A_nde1->u1;         
          split_nde_A->w1 = fract_w*(A_nde2->w1 - A_nde1->w1) +  A_nde1->w1; 
        }
      else        
        {
          fract_u =  FRACT(int_uw.x(), A_nde1->u1, A_nde2->u1);              
          fract_w =  FRACT(int_uw.y(), A_nde1->w1, A_nde2->w1); 
          split_nde_A->u1 = int_uw.x();              
          split_nde_A->w1 = int_uw.y();  
          split_nde_A->u2 = fract_u*(A_nde2->u2 - A_nde1->u2) +  A_nde1->u2;         
          split_nde_A->w2 = fract_w*(A_nde2->w2 - A_nde1->w2) +  A_nde1->w2; 
        }
      if ( B_flag ) 
        {
          fract_u =  FRACT(int_uw.x(), B_nde1->u2, B_nde2->u2);              
          fract_w =  FRACT(int_uw.y(), B_nde1->w2, B_nde2->w2); 
          split_nde_B->u2 = int_uw.x();              
          split_nde_B->w2 = int_uw.y();  
          split_nde_B->u1 = fract_u*(B_nde2->u1 - B_nde1->u1) +  B_nde1->u1;         
          split_nde_B->w1 = fract_w*(B_nde2->w1 - B_nde1->w1) +  B_nde1->w1; 
        }
      else        
        {
          fract_u =  FRACT(int_uw.x(), B_nde1->u1, B_nde2->u1);              
          fract_w =  FRACT(int_uw.y(), B_nde1->w1, B_nde2->w1); 
          split_nde_B->u1 = int_uw.x();              
          split_nde_B->w1 = int_uw.y();  
          split_nde_B->u2 = fract_u*(B_nde2->u2 - B_nde1->u2) +  B_nde1->u2;         
          split_nde_B->w2 = fract_w*(B_nde2->w2 - B_nde1->w2) +  B_nde1->w2; 
        }

      ic_A->split_list.append(split_nde_A);
      ic_B->split_list.append(split_nde_B);
    }
      
}


//===== Intersect End Nodes With Int Curves  =====//
void intersect_end_nodes( int_curve* ic_A, int A_flag, int B_flag, 
        int_node* A_nde1, int_node* A_nde2, int_node* B_end)
{
  vec2d A_uw1, A_uw2;
  vec2d B_uw;
  vec2d int_uw;

  if ( A_flag ) 
    {
      A_uw1.set_xy(A_nde1->u2, A_nde1->w2);
      A_uw2.set_xy(A_nde2->u2, A_nde2->w2);
    }
  else
    {
      A_uw1.set_xy(A_nde1->u1, A_nde1->w1);
      A_uw2.set_xy(A_nde2->u1, A_nde2->w1);
    }
  if ( B_flag ) 
    {
      B_uw.set_xy(B_end->u2, B_end->w2);
    }
  else
    {
      B_uw.set_xy(B_end->u1, B_end->w1);
    }

  double Dist_Tol = 0.000001;
  
  vec2d proj_pnt = proj_pnt_on_line_seg(A_uw1, A_uw2, B_uw);
  if ( dist(proj_pnt, B_uw) < Dist_Tol )
    {
      vec3d int_pnt;
      double fract_u=0.0, fract_w=0.0;

      //==== Find Actual Intersection Point ====//
      if ( A_flag ) 
        int_pnt = ic_A->surf2->comp_pnt( proj_pnt.x(), proj_pnt.y() );
      else
        int_pnt = ic_A->surf1->comp_pnt( proj_pnt.x(), proj_pnt.y() );

      int_node* split_nde_A = new int_node;
    
      split_nde_A->pnt = int_pnt;

      if ( A_flag ) 
        {
          fract_u =  FRACT(proj_pnt.x(), A_nde1->u2, A_nde2->u2);              
          fract_w =  FRACT(proj_pnt.y(), A_nde1->w2, A_nde2->w2); 
          split_nde_A->u2 = proj_pnt.x();              
          split_nde_A->w2 = proj_pnt.y();  
          split_nde_A->u1 = fract_u*(A_nde2->u1 - A_nde1->u1) +  A_nde1->u1;         
          split_nde_A->w1 = fract_w*(A_nde2->w1 - A_nde1->w1) +  A_nde1->w1; 
        }
      else        
        {
          fract_u =  FRACT(proj_pnt.x(), A_nde1->u1, A_nde2->u1);              
          fract_w =  FRACT(proj_pnt.y(), A_nde1->w1, A_nde2->w1); 
          split_nde_A->u1 = proj_pnt.x();              
          split_nde_A->w1 = proj_pnt.y();  
          split_nde_A->u2 = fract_u*(A_nde2->u2 - A_nde1->u2) +  A_nde1->u2;         
          split_nde_A->w2 = fract_w*(A_nde2->w2 - A_nde1->w2) +  A_nde1->w2; 
        }

      ic_A->split_list.append(split_nde_A);
    }
      
}

   
//===== Clear Split Arrays  =====//
void int_curve::clear_split_list()
{
  int i=0;
  for ( i = 0 ; i < split_list.dimension() ; i++ )
    {
      delete split_list[i];
    }

  split_list.init(0);
}

//===== Split Curves At Points In Split List =====//
void int_curve::split()
{
  int i=0;
cout << "*** Splitting Int Curve *** " << this << " Surf 1 = " << surf1 << " Surf 2 = " << surf2 << endl;
  for (  i = 0 ; i < split_list.dimension() ; i++ )
    {
      cout << " Split Node = " << split_list[i] << " u1, w1 = " << 
      split_list[i]->u1 << " " << split_list[i]->w1 << " " <<
      split_list[i]->u2 << " " << split_list[i]->w2 << endl;
//      cout << "    Pnt = " << split_list[i]->pnt << endl;
    }
  //==== Compute Distance Tolerances ====//
  vec2d uw;
  int_node* nde = 0;
  double uw_dist=0.0, act_dist=0.0;
  double max_uw_dist1, max_uw_dist2, max_act_dist;
  max_uw_dist1 = max_uw_dist2 = max_act_dist = 0.0;
  vec2d uw_1 = vec2d(first_node->u1, first_node->w1);
  vec2d uw_2 = vec2d(first_node->u2, first_node->w2);

  node_list.reset();
  while( !node_list.end_of_list() )
    {
      nde = node_list.get_curr();

      uw.set_xy(nde->u1, nde->w1);
      uw_dist = dist(uw, uw_1);
      if ( uw_dist > max_uw_dist1) max_uw_dist1 = uw_dist;

      uw.set_xy(nde->u2, nde->w2);
      uw_dist = dist(uw, uw_2);
      if ( uw_dist > max_uw_dist2) max_uw_dist2 = uw_dist;

      act_dist = dist( nde->pnt, first_node->pnt );
      if ( act_dist > max_act_dist) max_act_dist = act_dist;
             
      node_list.inc();
    }
  double Tol_Multiplier = 0.001;
  double uw_dist_tol_1 = max_uw_dist1*Tol_Multiplier;
  double uw_dist_tol_2 = max_uw_dist2*Tol_Multiplier;
  double act_dist_tol  = max_act_dist*Tol_Multiplier;

  //==== Remove Split Nodes At End Points ====//
  int remove_flag = 0;
  int cnt = 0; 
  while ( cnt < split_list.dimension() )
    {
      if  ( fabs ( split_list[cnt]->u1 - first_node->u1) < uw_dist_tol_1 &&
            fabs ( split_list[cnt]->w1 - first_node->w1) < uw_dist_tol_1 )
        remove_flag = TRUE;
      else if ( fabs ( split_list[cnt]->u1 - last_node->u1) < uw_dist_tol_1 &&
                fabs ( split_list[cnt]->w1 - last_node->w1) < uw_dist_tol_1 )
        remove_flag = TRUE;
      else if ( fabs ( split_list[cnt]->u2 - first_node->u2) < uw_dist_tol_2 &&
                fabs ( split_list[cnt]->w2 - first_node->w2) < uw_dist_tol_2 )
        remove_flag = TRUE;
      else if ( fabs ( split_list[cnt]->u2 - last_node->u2) < uw_dist_tol_2 &&
                fabs ( split_list[cnt]->w2 - last_node->w2) < uw_dist_tol_2 )
        remove_flag = TRUE;
      else
        remove_flag = FALSE;

      if ( remove_flag )
        {
          delete split_list[cnt];
          split_list.del_index(cnt);
        }
      else
        {
          cnt++;
        }
    }
cout << "*** AFTER END POINT REMOVAL Splitting Int Curve *** " << this << " Surf 1 = " << surf1 << " Surf 2 = " << surf2 << endl;
  for ( i = 0 ; i < split_list.dimension() ; i++ )
    {
      cout << " Split Node = " << split_list[i] << " u1, w1 = " << 
      split_list[i]->u1 << " " << split_list[i]->w1 << " " <<
      split_list[i]->u2 << " " << split_list[i]->w2 << endl;
//      cout << "    Pnt = " << split_list[i]->pnt << endl;
    }

  //==== Remove Duplicate Nodes In  Split List  ====//
  for ( i = 0 ; i < split_list.dimension() ; i++ )
    {
      cnt = i+1; 
      while ( cnt < split_list.dimension() )
        {
          if ( dist( split_list[i]->pnt, split_list[cnt]->pnt ) < act_dist_tol )
            {
              delete split_list[cnt];
              split_list.del_index(cnt);
            }
          else
            {
              cnt++;
            }
        }
    }

cout << "*** AFTER DUPLICATE POINT REMOVAL Splitting Int Curve *** " << this << " Surf 1 = " << surf1 << " Surf 2 = " << surf2 << endl;
  for ( i = 0 ; i < split_list.dimension() ; i++ )
    {
      cout << " Split Node = " << split_list[i] << " u1, w1 = " << 
      split_list[i]->u1 << " " << split_list[i]->w1 << " " <<
      split_list[i]->u2 << " " << split_list[i]->w2 << endl;
//      cout << "    Pnt = " << split_list[i]->pnt << endl;
    }

  if ( split_list.dimension() <= 0 ) return;

  //==== Add Split Nodes To Node List  ====//
  for ( i = 0 ; i < split_list.dimension() ; i++ )
    {
      insert_int_node(split_list[i]);
    }
  //==== Create New Intersection Curves  ====//
  dyn_array< int_curve* > new_curves;
  new_curves.init(split_list.dimension()+1); 
  for ( i = 0 ; i < split_list.dimension()+1 ; i++ )
    {
      new_curves[i] = new int_curve;
    }

  //==== Load Nodes Into New Intersection Curves  ====//
  int_node* new_nde;
  int new_crv_cnt = 0;
  node_list.reset();
  while( !node_list.end_of_list() )
    {
      nde = node_list.get_curr();
      new_nde = new int_node;
      new_nde->u1 = nde->u1;
      new_nde->w1 = nde->w1;
      new_nde->u2 = nde->u2;
      new_nde->w2 = nde->w2;
      new_nde->pnt = nde->pnt;
     
      new_curves[new_crv_cnt]->load_node(new_nde);  

      for ( i = 0 ; i < split_list.dimension() ; i++ )
        {
          if ( nde == split_list[i] )
            {
              new_crv_cnt++;
              new_nde = new int_node;
              new_nde->u1 = nde->u1;
              new_nde->w1 = nde->w1;
              new_nde->u2 = nde->u2;
              new_nde->w2 = nde->w2;
              new_nde->pnt = nde->pnt;
              new_curves[new_crv_cnt]->load_node(new_nde);
            }  
        }
      node_list.inc(); 
    }

  //==== Copy Surface Ptrs Into New Int Curves  ====//
  for ( i = 0 ; i < new_curves.dimension() ; i++ )
    {
      new_curves[i]->set_surf_ptrs(surf1, surf2);
      new_curves[i]->set_first_last_nodes();
      surf1->add_int_curve(new_curves[i]);
    }

  //==== Delete This Int Curves From Surface  ====//
  valid_flag = FALSE;
}

//===== Insert Intersection Node  =====//
void int_curve::insert_int_node(int_node* new_nde)
{
  double Equal_Tol = 0.0000001;
  double u = new_nde->u1; 
  double w = new_nde->w1;

  double u_low=0.0, u_high=0.0, w_low=0.0, w_high=0.0;
  int_node* nde_1 = 0; 
  int_node* nde_2 = 0; 

  node_list.reset();
  nde_1 = node_list.get_curr();
  node_list.inc();
  while( !node_list.end_of_list() )
    {
      nde_2 = node_list.get_curr();
      if ( nde_1->u1 < nde_2->u1 )
        { u_low  = nde_1->u1;  u_high = nde_2->u1; }
      else
        { u_low  = nde_2->u1;  u_high = nde_1->u1; }

      if ( nde_1->w1 < nde_2->w1 )
        { w_low  = nde_1->w1;  w_high = nde_2->w1; }
      else
        { w_low  = nde_2->w1;  w_high = nde_1->w1; }

      if ( fabs( u_high - u_low ) < Equal_Tol && 
           fabs( u - u_low ) < Equal_Tol && 
           w >= w_low && w <= w_high )
        {
          node_list.insert_before( new_nde );
          node_list.set_end();
        }
      else if ( fabs( w_high - w_low ) < Equal_Tol && 
           fabs( w - w_low ) < Equal_Tol && 
           u >= u_low && u <= u_high )
        {
          node_list.insert_before( new_nde );
          node_list.set_end();
        }
      else if ( u >= u_low && u <= u_high && w >= w_low && w <= w_high )
        {
          node_list.insert_before( new_nde );
          node_list.set_end();
        }
      nde_1 = nde_2;
      node_list.inc();
     }
}

//===== Check If In/Out Of Bezier Surfaces  =====//
void int_curve::check_in_out_surfs(dyn_array<bezier_surf*>& surf_list)
{
  int i=0, j=0, k=0;
  int half_way = node_list.num_objects()/2 - 1;

  node_list.reset();
  for ( i = 0 ; i < half_way ; i++ )
    node_list.inc();

  vec3d half_pnt1 = node_list.get_curr()->pnt;
  node_list.inc();
  vec3d half_pnt2 = node_list.get_curr()->pnt;

  vec3d half_pnt = (half_pnt1 + half_pnt2)*0.5;


  vec3d out_pnt( -10.1, 10.1, 10.1 );
 
  dyn_array<vec3d> int_pnts;
  dyn_array<int> groups;

  for ( i = 0 ; i <  surf_list.dimension() ; i++ )
    {
      if (  surf_list[i] != surf1 && surf_list[i] != surf2 )
        {
          if (  surf_list[i]->get_group_id() != surf1->get_group_id() && 
                surf_list[i]->get_group_id() != surf2->get_group_id() )
            {
              int_pnts.init(0);
              surf_list[i]->intersect_line(half_pnt, out_pnt, int_pnts);

              //==== Remove Duplicate Intersection Points ====//
              double Dist_tol = 0.01;
              for ( j = 0 ; j <  int_pnts.dimension() ; j++ )
                {
                  for ( k = j+1 ; k <  int_pnts.dimension() ; k++ )
                    {
                      if ( dist( int_pnts[j], int_pnts[k] ) < Dist_tol )
                        {
                          int_pnts.del_index(k);
                        } 
                    }
                }
                  

              for ( j = 0 ; j <  int_pnts.dimension() ; j++ )
                {
                  groups.append( surf_list[i]->get_group_id() );
                } 

if ( int_pnts.dimension() > 0 )
  {
  cout << endl;
  cout << " RAY_CURVE " << endl;
  cout << " COMPONENT NUMBER =    1 " << endl;
  cout << " COMPONENT COLOR =     3 " << endl;
  cout << " CROSS SECTIONS =      1" << endl;
  cout << " PTS/CROSS SECTION =   2" << endl;
//  cout << half_pnt << endl;
//  cout << out_pnt << endl;

  for ( k = 0 ; k <  int_pnts.dimension() ; k++ )
    {

  cout << endl;
  cout << " RAY_CURVE " << endl;
  cout << " COMPONENT NUMBER =    1 " << endl;
  cout << " COMPONENT COLOR =     4 " << endl;
  cout << " CROSS SECTIONS =      4" << endl;
  cout << " PTS/CROSS SECTION =   2" << endl;
//  cout << (int_pnts[k] + vec3d(-0.1, -0.1, -0.1))  << endl;
//  cout << (int_pnts[k] + vec3d(-0.1,  0.1, -0.1)) << endl;
//  cout << (int_pnts[k] + vec3d( 0.1, -0.1, -0.1))  << endl;
//  cout << (int_pnts[k] + vec3d( 0.1,  0.1, -0.1)) << endl;
//  cout << (int_pnts[k] + vec3d(-0.1, -0.1,  0.1))  << endl;
// cout << (int_pnts[k] + vec3d(-0.1,  0.1,  0.1)) << endl;
//  cout << (int_pnts[k] + vec3d( 0.1, -0.1,  0.1))  << endl;
//  cout << (int_pnts[k] + vec3d( 0.1,  0.1,  0.1)) << endl;
    }
  }

            }
        }
    }
  for ( i = 0 ; i <  groups.dimension() ; i++ )
    {
      int num_hits = 1;
      for ( j = 0 ; j <  groups.dimension() ; j++ )
        {
          if ( i !=j && groups[i] == groups[j] )
            num_hits++;
        }
      if ( num_hits%2 == 1 )
        valid_flag = FALSE;
    }
      

  //==== Check For Curves Collapsed To Point ====//
  double Dist_Tol = 0.05;
  if ( ( dist ( first_node->pnt, half_pnt ) < Dist_Tol ) && 
       ( dist ( last_node->pnt,  half_pnt ) < Dist_Tol ) )
    {
      valid_flag = FALSE;
    }

}



//===== Check If In/Out Of Bezier Surfaces  =====//
void int_curve::snap_to_edges()
{
  double uw_tol = 0.01;
  double u_max=0.0, w_max=0.0;

/********************************
cout << " _-_-Begin Snap " << endl;
cout << " Surf1 uw Max = " << surf1->get_u_max() << " " << surf1->get_w_max() << endl; 
cout << " Surf2 uw Max = " << surf2->get_u_max() << " " << surf2->get_w_max() << endl;
cout << " First Node u1, w1 = " << first_node->u1 << " " << first_node->w1 << endl;
cout << " First Node u2, w2 = " << first_node->u2 << " " << first_node->w2 << endl;
cout << " Last Node u1, w1 = " << last_node->u1 << " " << last_node->w1 << endl;
cout << " Last Node u2, w2 = " << last_node->u2 << " " << last_node->w2 << endl;
cout << endl;
***********************************/

  u_max = (double) surf1->get_u_max();
  if      ( fabs(first_node->u1 - 0.0)   < uw_tol ) first_node->u1 = 0.0;
  else if ( fabs(first_node->u1 - u_max) < uw_tol ) first_node->u1 = u_max;

  if      ( fabs(last_node->u1 - 0.0)    < uw_tol ) last_node->u1 = 0.0;
  else if ( fabs(last_node->u1 - u_max)  < uw_tol ) last_node->u1 = u_max;

  w_max = (double) surf1->get_w_max();
  if      ( fabs(first_node->w1 - 0.0)   < uw_tol ) first_node->w1 = 0.0;
  else if ( fabs(first_node->w1 - w_max) < uw_tol ) first_node->w1 = w_max;

  if      ( fabs(last_node->w1 - 0.0)    < uw_tol ) last_node->w1 = 0.0;
  else if ( fabs(last_node->w1 - w_max)  < uw_tol ) last_node->w1 = w_max;

  u_max = (double) surf2->get_u_max();
  if      ( fabs(first_node->u2 - 0.0)   < uw_tol ) first_node->u2 = 0.0;
  else if ( fabs(first_node->u2 - u_max) < uw_tol ) first_node->u2 = u_max;

  if      ( fabs(last_node->u2 - 0.0)    < uw_tol ) last_node->u2 = 0.0;
  else if ( fabs(last_node->u2 - u_max)  < uw_tol ) last_node->u2 = u_max;

  w_max = (double) surf2->get_w_max();
  if      ( fabs(first_node->w2 - 0.0)   < uw_tol ) first_node->w2 = 0.0;
  else if ( fabs(first_node->w2 - w_max) < uw_tol ) first_node->w2 = w_max;

  if      ( fabs(last_node->w2 - 0.0)    < uw_tol ) last_node->w2 = 0.0;
  else if ( fabs(last_node->w2 - w_max)  < uw_tol ) last_node->w2 = w_max;

/*********************************
cout << " _-_-End Snap " << endl;
cout << " First Node u1, w1 = " << first_node->u1 << " " << first_node->w1 << endl;
cout << " First Node u2, w2 = " << first_node->u2 << " " << first_node->w2 << endl;
cout << " Last Node u1, w1 = " << last_node->u1 << " " << last_node->w1 << endl;
cout << " Last Node u2, w2 = " << last_node->u2 << " " << last_node->w2 << endl;
cout << endl;
**********************************/

}

void snap_int_curve_end_points(dyn_array<int_curve*>& all_int_curves)
{
  int i=0, j=0;
  double Dist_Tol = 0.05;

  //==== Load All End Points ====//
  dyn_array<vec3d> all_end_pnts;
  for ( i = 0 ; i < all_int_curves.dimension() ; i++ )
    {
      all_end_pnts.append( all_int_curves[i]->get_first_pnt() );
      all_end_pnts.append( all_int_curves[i]->get_last_pnt() );
    }

  //==== Collapse End Points ====//
  int tmp_cnt;
  vec3d tmp_pnt;
  for ( i = 0 ; i < all_end_pnts.dimension() ; i++ )
    {
      tmp_cnt = 1;
      tmp_pnt = all_end_pnts[i];

      for ( j = i+1 ; j < all_end_pnts.dimension() ; j++ )
        {
          if ( dist( all_end_pnts[i], all_end_pnts[j] )  < Dist_Tol )
            {
              tmp_pnt = tmp_pnt + all_end_pnts[j];
              tmp_cnt++;
              all_end_pnts.del_index(j);
              j--;
            }
         }
      //==== Average All Close End Points ====//
      all_end_pnts[i] = tmp_pnt/(float)tmp_cnt;
    }

  //==== Snap End Nodes To List Of Collapsed End Points ====//
  vec3d f_pnt, l_pnt;
  for ( i = 0 ; i < all_int_curves.dimension() ; i++ )
    {
      f_pnt = all_int_curves[i]->get_first_pnt();
      l_pnt = all_int_curves[i]->get_last_pnt();
      for ( j = 0 ; j < all_end_pnts.dimension() ; j++ )
        {
          if ( dist ( f_pnt, all_end_pnts[j] ) < Dist_Tol )
            {
              all_int_curves[i]->first_node->pnt = all_end_pnts[j];
            } 
          if ( dist ( l_pnt, all_end_pnts[j] ) < Dist_Tol )
            {
              all_int_curves[i]->last_node->pnt = all_end_pnts[j];
            }
        }
    } 
}





















  







