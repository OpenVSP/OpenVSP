//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Cross Section Surface Class
//  
// 
//   J.R. Gloudemans - 10/16/93
//   Sterling Software
//
//******************************************************************************

#include "xsec_surf.h"
#include "vector_util.h"
#include "geom.h"

#include "bezier_surf.h"

#include "tMesh.h"

// Include OpenNurbs for Rhino Dump
// ON Needs to be undefined for it to compile
//
#undef ON
#include "opennurbs.h"
#include "opennurbs_extensions.h"


//==== Constructor =====//
Xsec_surf::Xsec_surf()
{
  num_xsecs = 0;
  num_pnts = 0;

  refl_pnts_xsecs_code = NO_SYM;
  refl_hidden_code     = NO_SYM;
  refl_normals_code    = NO_SYM;
  
  fast_draw_flag = 0;

  highlight_xsec_flag = 0;
  highlight_xsec_id   = 0;
  highlight_xsec_color = vec3d( 0.75, 0.0, 0.0 );

  draw_flag = true;


}

//==== Destructor =====//
Xsec_surf::~Xsec_surf()
{
}

//==== Set Number Of Cross Sections =====//
void Xsec_surf::set_num_xsecs(int num_in)
{
  num_xsecs = num_in;
  pnts_xsecs.init(num_xsecs, num_pnts);
  clear_xsec_tan_flags();

}

//==== Set Number Of Points/Xsec =====//
void Xsec_surf::set_num_pnts(int num_in)
{
  num_pnts = num_in;
  pnts_xsecs.init(num_xsecs, num_pnts);
  clear_pnt_tan_flags();
}
void Xsec_surf::get_xsec( int ixs, vector< vec3d > & pvec )
{
	pvec.resize( num_pnts );
	for ( int j = 0 ; j < num_pnts ; j++ )
    {
		pvec[j] = pnts_xsecs( ixs, j );
	}
}
   
void Xsec_surf::get_stringer( int ipnt, vector< vec3d > & pvec )
{
	pvec.resize( num_xsecs );
	for ( int i = 0 ; i < num_xsecs ; i++ )
    {
		pvec[i] = pnts_xsecs( i, ipnt );
	}
}


//==== Roatate Cross Section About X Axis =====//
void Xsec_surf::rotate_xsec_x(int index, double alpha_deg)
{
  double alpha_rad = DEG2RAD(alpha_deg);
  double cos_alpha = cos(alpha_rad);
  double sin_alpha = sin(alpha_rad);

  for ( int j = 0 ; j < num_pnts ; j++ )
    {
      pnts_xsecs(index,j).rotate_x(cos_alpha, sin_alpha);
    }  
}

//==== Roatate Cross Section About X Axis =====//
void Xsec_surf::rotate_xsec_y(int index, double alpha_deg)
{
  double  alpha_rad = DEG2RAD(alpha_deg);
  double cos_alpha = cos(alpha_rad);
  double sin_alpha = sin(alpha_rad);

  for ( int j = 0 ; j < num_pnts ; j++ )
    {
      pnts_xsecs(index,j).rotate_y(cos_alpha, sin_alpha);
    }  
}

//==== Roatate Cross Section About X Axis =====//
void Xsec_surf::rotate_xsec_z(int index, double alpha_deg)
{
  double  alpha_rad = DEG2RAD(alpha_deg);
  double cos_alpha = cos(alpha_rad);
  double sin_alpha = sin(alpha_rad);

  for (  int j = 0 ; j < num_pnts ; j++ )
    {
      pnts_xsecs(index,j).rotate_z(cos_alpha, sin_alpha);
    }  
}

//==== Roatate Cross Section About X Axis < Zero Out X Translation >=====//
void Xsec_surf::rotate_xsec_z_zero_x(int index, double alpha_deg)
{
  double  alpha_rad = DEG2RAD(alpha_deg);
  double cos_alpha = cos(alpha_rad);
  double sin_alpha = sin(alpha_rad);

  for (  int j = 0 ; j < num_pnts ; j++ )
    {
      pnts_xsecs(index,j).rotate_z_zero_x(cos_alpha, sin_alpha);
    }  
}

//==== Scale Cross Section About X Cordinate =====//
void Xsec_surf::scale_xsec_x(int index, double scale)
{
  for (  int j = 0 ; j < num_pnts ; j++ )
    {
      pnts_xsecs(index,j).scale_x(scale);
    }
}

//==== Scale Cross Section About Y Cordinate =====//
void Xsec_surf::scale_xsec_y(int index, double scale)
{
  for (  int j = 0 ; j < num_pnts ; j++ )
    {
      pnts_xsecs(index,j).scale_y(scale);
    }
}

//==== Scale Cross Section About Z Cordinate =====//
void Xsec_surf::scale_xsec_z(int index, double scale)
{
  for (  int j = 0 ; j < num_pnts ; j++ )
    {
      pnts_xsecs(index,j).scale_z(scale);
    }
}

//==== Offset Cross Section About X Cordinate =====//
void Xsec_surf::offset_xsec_x(int index, double offset)
{
  for (  int j = 0 ; j < num_pnts ; j++ )
    {
      pnts_xsecs(index,j).offset_x(offset);
    }
}

//==== Offset Cross Section About Y Cordinate =====//
void Xsec_surf::offset_xsec_y(int index, double offset)
{
  for (  int j = 0 ; j < num_pnts ; j++ )
    {
      pnts_xsecs(index,j).offset_y(offset);
    }
}

//==== Offset Cross Section About Z Cordinate =====//
void Xsec_surf::offset_xsec_z(int index, double offset)
{
  for (  int j = 0 ; j < num_pnts ; j++ )
    {
      pnts_xsecs(index,j).offset_z(offset);
    }
}

//==== Get Center Pnt Of Cross Section =====//
vec3d Xsec_surf::get_xsec_center( int ixs )
{
  vec3d center(0.0, 0.0, 0.0);

  for (  int j = 0 ; j < num_pnts ; j++ )
    {
      center = center + pnts_xsecs(ixs, j);
    }
  
  return( center/(double)num_pnts );
}


//==== Get Center Pnt Of Cross Section =====//
double Xsec_surf::get_max_dist_from_xsec( int ixs, const vec3d& pnt_in)
{
  double temp_dist;
  double max_dist = 0.0;

  for (  int j = 0 ; j < num_pnts ; j++ )
    {
      temp_dist = dist_squared( pnts_xsecs(ixs, j), pnt_in );

      if ( temp_dist > max_dist )
        max_dist = temp_dist;
    }

  if ( max_dist <= 0.0 )
    return(0.0);  

  return( sqrt(max_dist) );

}



//==== Linear Interpolate Cross Section =====//
vec3d Xsec_surf::linear_interpolate_xsec(int ixs, double fract)
{
  int num_half_pnts = (num_pnts-1)/2 + 1;

  if ( fract <= 0.0 ) return( pnts_xsecs(ixs, num_half_pnts) );
  if ( fract >= 1.0 ) return( pnts_xsecs(ixs, 0) );


  //..array<double> dist_sqr;
  vector<double> dist_sqr;
  //..dist_sqr.init(num_half_pnts);
  dist_sqr.resize(num_half_pnts);

  double total_dist = dist_sqr[0] = 0.0;
  for ( int i = 1 ; i < num_half_pnts ; i++ )
    { 
      total_dist += dist_squared( pnts_xsecs(ixs, i), pnts_xsecs(ixs, i-1));
      dist_sqr[i] = total_dist;
    }
  double target_dist = fract*dist_sqr[num_half_pnts-1];

  //..int ivl = dist_sqr.find_interval(fract*total_dist);
  int ivl = FindInterval(dist_sqr, fract*total_dist);
  //..double percent = dist_sqr.interpolate(fract*total_dist, ivl);
  double percent = Interpolate( dist_sqr, fract*total_dist, ivl);

  return(  pnts_xsecs(ixs,ivl) + 
          (pnts_xsecs(ixs,ivl+1) - pnts_xsecs(ixs,ivl))*percent );
  

}

void Xsec_surf::copy_xsec( Xsec_surf* fromSurf, int fromXs, int toXs )
{
	if ( fromSurf->get_num_pnts() != get_num_pnts() )
		return;

	for ( int i = 0 ; i < get_num_pnts() ; i++ )
	{
		set_pnt( toXs, i, fromSurf->get_pnt( fromXs, i ) );
	}
}


vec3d Xsec_surf::comp_uv_pnt( double u, double v )
{
	double dx = u*(num_xsecs-1);
	double dp =	v*(num_pnts-1);

	if ( dx < 0.0 )
		dx = 0;
	else if ( dx >= num_xsecs-1 )
		dx = num_xsecs - 1.000001;

	if ( dp < 0.0 )
		dp = 0;
	else if ( dp >= num_pnts-1 )
		dp = num_pnts - 1.000001;

	int ix = (int)dx;
	int ip = (int)dp;

	double fractx = dx - (double)ix;
	double fractp = dp - (double)ip;

	//==== Interpolate On XSecs ====//
	vec3d xp0 = pnts_xsecs(ix, ip)  + (pnts_xsecs(ix, ip+1)  - pnts_xsecs(ix, ip))*fractp;
	vec3d xp1 = pnts_xsecs(ix+1, ip)+ (pnts_xsecs(ix+1, ip+1)- pnts_xsecs(ix+1, ip))*fractp;


	vec3d pnt = xp0 + (xp1 - xp0)*fractx;

	return pnt;

}




//==== Draw Wire Frame =====//
void Xsec_surf::draw_wire( )
{
  int i, j, fast_1, fast_2;
  double dpnt[3];
  double color[4];

  //==== Set Line Width ====//  
  glLineWidth(1);

  fast_1 = fast_2 = 1;
  if ( fast_draw_flag ) 
  { 
    fast_1 = MAX(MIN( 4, num_xsecs - 1 ), 1 );
    fast_2 = MAX(MIN( 4, num_pnts  - 1 ), 1 );
    
    if ( num_xsecs <= 4 ) fast_1 = 1;
    if ( num_pnts  <= 4 ) fast_2 = 1;   
  }
  
  //==== Draw Cross Sections ====//  
  for ( i = 0 ; i < num_xsecs ; i += fast_1 )
  {

      if ( highlight_xsec_flag && i == highlight_xsec_id )
	  {
//		  glPushAttrib(GL_CURRENT_BIT);

		  glGetDoublev(GL_CURRENT_COLOR, color);
		  glColor3d( highlight_xsec_color.x(), highlight_xsec_color.y(), highlight_xsec_color.z() );
		  glLineWidth( 3.0 );
	  }


      glBegin( GL_LINE_STRIP );
	  for ( j = 0 ; j < num_pnts ; j += fast_2 )
        {
          pnts_xsecs(i,j).get_pnt(dpnt);	glVertex3dv(dpnt);
        }
      glEnd();

	  if ( highlight_xsec_flag && i == highlight_xsec_id )
	  {
//		  glPopAttrib();
		  glColor3dv(color);
		  glLineWidth( 1.0 );
	  }

  }

  //==== Draw Stringers ====//  
  for ( i = 0 ; i < num_pnts ; i += fast_2 )
    {
      glBegin( GL_LINE_STRIP );
      for ( j = 0 ; j < num_xsecs ; j += fast_1 )
        {
			pnts_xsecs(j,i).get_pnt(dpnt);	glVertex3dv(dpnt);
        }
      glEnd();
    }
}

//==== Draw Reflected Wire Frame =====//
void Xsec_surf::draw_refl_wire( int sym_code_in) 
{
  if (sym_code_in == NO_SYM) return;

  int i, j, fast_1, fast_2;
  double dpnt[3];

  fast_1 = fast_2 = 1;
  if ( fast_draw_flag ) {
  
    fast_1 = MAX(MIN( 4, num_xsecs - 1 ), 1 );
    fast_2 = MAX(MIN( 4, num_pnts  - 1 ), 1 );
    
    if ( num_xsecs <= 4 ) fast_1 = 1;
    if ( num_pnts  <= 4 ) fast_2 = 1;
    
  }
  
  if ( sym_code_in != refl_pnts_xsecs_code )
    {
      refl_pnts_xsecs_code =  sym_code_in;
      load_refl_pnts_xsecs();  
    }

  //==== Set Line Width ====//  
  glLineWidth(1);

  //==== Draw Cross Sections ====//  
  for ( i = 0 ; i < num_xsecs ; i += fast_1 )
    {
      glBegin( GL_LINE_STRIP );
      for ( j = 0 ; j < num_pnts ; j += fast_2 )
        {
          refl_pnts_xsecs(i,j).get_pnt(dpnt);	glVertex3dv(dpnt);
        }
      glEnd();
    }

  //==== Draw Stringers ====//  
  for ( i = 0 ; i < num_pnts ; i += fast_2 )
    {
      glBegin( GL_LINE_STRIP );
      for ( j = 0 ; j < num_xsecs ; j += fast_1 )
        {
	  refl_pnts_xsecs(j,i).get_pnt(dpnt);	glVertex3dv(dpnt);
        }
      glEnd();
    }
}

//==== Draw Hidden Surf =====//
void Xsec_surf::draw_hidden()
{
  int i, j, fast_1, fast_2;

  double dpnt[3];
  draw_wire();

  //==== Draw Hidden Surface ====//
  glPolygonOffset(2.0, 1);
#ifndef __APPLE__
  glEnable(GL_POLYGON_OFFSET_EXT);
#endif
  glColor3f( 1.0f, 1.0f, 1.0f );

 

  fast_1 = fast_2 = 1;
  if ( fast_draw_flag ) {
  
    fast_1 = MAX(MIN( 4, num_xsecs - 1 ), 1 );
    fast_2 = MAX(MIN( 4, num_pnts  - 1 ), 1 );
    
    if ( num_xsecs <= 4 ) fast_1 = 1;
    if ( num_pnts  <= 4 ) fast_2 = 1;
    
  }
    
  for ( i = 0 ; i < num_xsecs-1 ; i += fast_1 )
    {
      for ( j = 0 ; j < num_pnts-1 ; j += fast_2 )
        {
          glBegin( GL_POLYGON );
           //hidden_surf(i,j).get_pnt(dpnt);	glVertex3dv(dpnt);
           //hidden_surf(i+1,j).get_pnt(dpnt);	glVertex3dv(dpnt);
           //hidden_surf(i+1,j+1).get_pnt(dpnt);	glVertex3dv(dpnt);
           //hidden_surf(i,j+1).get_pnt(dpnt);	glVertex3dv(dpnt);
           pnts_xsecs(i,j).get_pnt(dpnt);	glVertex3dv(dpnt);
           pnts_xsecs(i+1,j).get_pnt(dpnt);	glVertex3dv(dpnt);
           pnts_xsecs(i+1,j+1).get_pnt(dpnt);	glVertex3dv(dpnt);
           pnts_xsecs(i,j+1).get_pnt(dpnt);	glVertex3dv(dpnt);
          glEnd();
         }
     }
#ifndef __APPLE__
    glDisable(GL_POLYGON_OFFSET_EXT);
#endif

}

//==== Draw Reflected Hidden Surf =====//
void Xsec_surf::draw_refl_hidden( int sym_code_in)
{
  if (sym_code_in == NO_SYM) return;

  int i, j, fast_1, fast_2;
  double dpnt[3];

  fast_1 = fast_2 = 1;
  if ( fast_draw_flag ) {
  
    fast_1 = MAX(MIN( 4, num_xsecs - 1 ), 1 );
    fast_2 = MAX(MIN( 4, num_pnts  - 1 ), 1 );
    
    if ( num_xsecs <= 4 ) fast_1 = 1;
    if ( num_pnts  <= 4 ) fast_2 = 1;
    
  }
  
  if ( sym_code_in != refl_pnts_xsecs_code )
    {
      refl_pnts_xsecs_code =  sym_code_in;
      load_refl_pnts_xsecs();  
    }
  if ( sym_code_in != refl_hidden_code )
    {
      refl_hidden_code =  sym_code_in;
      load_refl_hidden_surf();  
    }

  draw_refl_wire(sym_code_in);

  //==== Draw Hidden Surface ====//
  glColor3f( 1.0f, 1.0f, 1.0f );	
  glPolygonOffset(2.0, 1);
#ifndef __APPLE__
  glEnable(GL_POLYGON_OFFSET_EXT);
#endif

  for ( i = 0 ; i < num_xsecs-1 ; i += fast_1 )
    {
      for ( j = 0 ; j < num_pnts-1 ; j += fast_2 )
        {
          glBegin( GL_POLYGON );
	       refl_pnts_xsecs(i,j).get_pnt(dpnt);		glVertex3dv(dpnt);
           refl_pnts_xsecs(i+1,j).get_pnt(dpnt);	glVertex3dv(dpnt);
           refl_pnts_xsecs(i+1,j+1).get_pnt(dpnt);	glVertex3dv(dpnt);
           refl_pnts_xsecs(i,j+1).get_pnt(dpnt);	glVertex3dv(dpnt);
 	       //refl_hidden_surf(i,j).get_pnt(dpnt);		glVertex3dv(dpnt);
         //  refl_hidden_surf(i+1,j).get_pnt(dpnt);	glVertex3dv(dpnt);
         //  refl_hidden_surf(i+1,j+1).get_pnt(dpnt);	glVertex3dv(dpnt);
         //  refl_hidden_surf(i,j+1).get_pnt(dpnt);	glVertex3dv(dpnt);
         glEnd();
         }
     }
#ifndef __APPLE__
    glDisable(GL_POLYGON_OFFSET_EXT);
#endif
}

//==== Draw Shaded Surf =====//
void Xsec_surf::draw_shaded()
{
  int i, j;
  int fast_1, fast_2;
  float  fpnt[3];
  double dpnt[3];

  fast_1 = fast_2 = 1;
  if ( fast_draw_flag ) {
  
    fast_1 = MAX(MIN( 4, num_xsecs - 1 ), 1 );
    fast_2 = MAX(MIN( 4, num_pnts  - 1 ), 1 );
    
    if ( num_xsecs <= 6 ) fast_1 = 1;
    if ( num_pnts  <= 6 ) fast_2 = 1;
    
  }

	//==== jrg fix shading 
	glEnable( GL_LIGHTING );
          
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_BLEND );
//	glEnable( GL_CULL_FACE );
//	glCullFace( GL_BACK );

	for ( i = 0 ; i < num_xsecs-1 ; i += fast_1 )
    {
      for ( j = 0 ; j < num_pnts-1 ; j += fast_2 )
        {
          glBegin( GL_POLYGON );
           normals(i,j).get_pnt(fpnt);			glNormal3fv(fpnt);
           pnts_xsecs(i,j).get_pnt(dpnt);		glVertex3dv(dpnt);
           normals(i+1,j).get_pnt(fpnt);		glNormal3fv(fpnt);
           pnts_xsecs(i+1,j).get_pnt(dpnt);		glVertex3dv(dpnt);
           normals(i+1,j+1).get_pnt(fpnt);		glNormal3fv(fpnt);
           pnts_xsecs(i+1,j+1).get_pnt(dpnt);		glVertex3dv(dpnt);
           normals(i,j+1).get_pnt(fpnt);		glNormal3fv(fpnt);
           pnts_xsecs(i,j+1).get_pnt(dpnt);		glVertex3dv(dpnt);
          glEnd();
        }
     }
  glDisable( GL_LIGHTING );
  
}
//==== Remap Texture Coordinates - Handle Translation/Scale and Seams ====//
void Xsec_surf::remap_texture( double upos, double width, bool wrapFlag, vector< double > & uVec, 
						vector< int > & uIndex, vector< double > & uRemap )
{
	if ( uVec.size() < 2 )
		return;

	int i;
	double ulow  = upos - width/2.0;			// Range of U Vals
	double uhigh = upos + width/2.0;

	//==== Check if 0-1 boundry crossed ====//
	int uCrossFlag = 0;
	if ( ulow < 0.0 )
	{
		ulow += -(int)(ulow - 1);
		uCrossFlag = 1;
	}
	if ( uhigh > 1.0 )
	{
		uhigh -= (int)uhigh;
		uCrossFlag = 1;
	}


	double ulowSlop = 0.0;
	int ulowIndex = 0;
	for ( i = uVec.size()-1 ; i >= 0 ; i-- )
	{
		if ( uVec[i] <= ulow )
		{
			ulowIndex = i;
			ulowSlop  = ulow - uVec[i];
			break;
		}
	}

	double uhighSlop = 0.0;
	int uhighIndex = uVec.size()-1;
	for ( i = 0 ; i < (int)uVec.size() ; i ++ )
	{
		if ( uVec[i] >= uhigh )
		{
			uhighIndex = i;
			uhighSlop  = uVec[i] - uhigh;
			break;
		}
	}		
		
	if ( !uCrossFlag )
	{
		for  ( i = ulowIndex ; i <= uhighIndex ; i++ )
		{
			uIndex.push_back( i );
			uRemap.push_back( uVec[i] );
		}
	}
	else
	{
		for ( i = ulowIndex ; i < (int)uVec.size() ; i ++ )
		{
			uIndex.push_back( i );
			uRemap.push_back( uVec[i] );
		}

		int iStart = 0;
		if ( wrapFlag )
			iStart = 1;

		for ( i = iStart ; i <= uhighIndex ; i++ )
		{
			uIndex.push_back( i );
			uRemap.push_back( uVec[i] + 1.0 );
		}
	}

	//==== Find Total U Range =====//
	double totalU = 1.0;
	if ( uRemap.size() )
	{
		totalU = uRemap.back() - uRemap[0];
	}
	totalU = totalU - ulowSlop - uhighSlop;

	double uZero = 0.0;
	uZero = uRemap[0]/totalU + ulowSlop/totalU;

	for ( i = 0 ; i < (int)uRemap.size() ; i++ )
	{
		uRemap[i] = uRemap[i]/totalU - uZero;
	}
}

//==== Draw Textured Surf =====//
void Xsec_surf::draw_texture( AppliedTex& tex )
{
	if ( num_xsecs <= 0 || num_pnts <= 0 )
		return;

	int i, j;

	if ( tex.repeatFlag == false )
	{
		//==== Remap U Dimension ====//
		vector< int > uIndex;
		vector< double > uVal;
		vector< double > uVec;
		for ( i = 0 ; i < (int)uArray.size() ; i++ )
			uVec.push_back( uArray[i] );

		remap_texture( tex.u, tex.scaleu, tex.wrapUFlag,  uVec, uIndex, uVal );

		if ( tex.flipUFlag )
		{
			for ( i = 0 ; i < (int)uVal.size() ; i++ )
				uVal[i] = 1.0 - uVal[i];
		}


		//==== Remap W Dimension ====//
		vector< int > wIndex;
		vector< double > wVal;
		vector< double > wVec;
		for ( i = 0 ; i < (int)wArray.size() ; i++ )
			wVec.push_back( wArray[i] );

		remap_texture( tex.w, tex.scalew, tex.wrapWFlag, wVec, wIndex, wVal );
		if ( tex.flipWFlag )
		{
			for ( i = 0 ; i < (int)wVal.size() ; i++ )
				wVal[i] = 1.0 - wVal[i];
		}

		for ( i = 0 ; i < (int)uIndex.size()-1 ; i ++ )
		{
			int iu = uIndex[i];
			int iuplus = uIndex[i+1];
			for ( j = 0 ; j < (int)wIndex.size()-1 ; j ++ )
			{
				int iw = wIndex[j];
				int iwplus = wIndex[j+1];
				glBegin( GL_POLYGON );
			
				glNormal3dv(normals(iu,iw).v);
				glTexCoord2d( uVal[i], wVal[j] );
				glVertex3dv(pnts_xsecs(iu,iw).v);

				glNormal3dv(normals(iuplus,iw).v);
				glTexCoord2d( uVal[i+1], wVal[j] );
				glVertex3dv(pnts_xsecs(iuplus,iw).v);

				glNormal3dv(normals(iuplus,iwplus).v);
				glTexCoord2d( uVal[i+1], wVal[j+1] );
				glVertex3dv(pnts_xsecs(iuplus,iwplus).v);

				glNormal3dv(normals(iu,iwplus).v);
				glTexCoord2d( uVal[i], wVal[j+1] );
				glVertex3dv(pnts_xsecs(iu,iwplus).v);

				glEnd();
			}
		}
	}
	else					// Repeating Texture
	{
		for ( i = 0 ; i < (int)uArray.size()-1 ; i++ )
		{
			for ( j = 0 ; j < (int)wArray.size()-1 ; j ++ )
			{
			  glBegin( GL_POLYGON );		
				glNormal3dv(normals(i,j).v);
				glTexCoord2d( uArray[i]/tex.scaleu + tex.u, wArray[j]/tex.scalew + tex.w );
				glVertex3dv(pnts_xsecs(i,j).v);

				glNormal3dv(normals(i+1,j).v);
				glTexCoord2d( uArray[i+1]/tex.scaleu + tex.u, wArray[j]/tex.scalew + tex.w );
				glVertex3dv(pnts_xsecs(i+1,j).v);

				glNormal3dv(normals(i+1,j+1).v);
				glTexCoord2d( uArray[i+1]/tex.scaleu + tex.u, wArray[j+1]/tex.scalew + tex.w );
				glVertex3dv(pnts_xsecs(i+1,j+1).v);

				glNormal3dv(normals(i,j+1).v);
				glTexCoord2d( uArray[i]/tex.scaleu + tex.u, wArray[j+1]/tex.scalew + tex.w );
				glVertex3dv(pnts_xsecs(i,j+1).v);
			  glEnd();
			}
		}
	}
		
}


//==== Draw Reflected Textured Surf =====//
void Xsec_surf::draw_refl_texture( AppliedTex& tex, int sym_code_in )
{
	if (sym_code_in == NO_SYM) return;

	if ( num_xsecs <= 0 || num_pnts <= 0 )
		return;

	int i, j;

	if ( sym_code_in != refl_pnts_xsecs_code )
    {
		refl_pnts_xsecs_code =  sym_code_in;
		load_refl_pnts_xsecs();  
	}
	if ( sym_code_in != refl_normals_code )
    {
		refl_normals_code =  sym_code_in;
		load_refl_normals();  
	}


	if ( tex.repeatFlag == false )
	{
		//==== Remap U Dimension ====//
		vector< int > uIndex;
		vector< double > uVal;
		vector< double > uVec;
		for ( i = 0 ; i < (int)uArray.size() ; i++ )
			uVec.push_back( uArray[i] );

		remap_texture( tex.u, tex.scaleu, tex.wrapUFlag,  uVec, uIndex, uVal );
		if ( tex.reflFlipUFlag )
		{
			for ( i = 0 ; i < (int)uVal.size() ; i++ )
				uVal[i] = 1.0 - uVal[i];
		}

		//==== Remap W Dimension ====//
		vector< int > wIndex;
		vector< double > wVal;
		vector< double > wVec;
		for ( i = 0 ; i < (int)wArray.size() ; i++ )
			wVec.push_back( wArray[i] );

		remap_texture( tex.w, tex.scalew, tex.wrapWFlag, wVec, wIndex, wVal );
		if ( tex.reflFlipWFlag )
		{
			for ( i = 0 ; i < (int)wVal.size() ; i++ )
				wVal[i] = 1.0 - wVal[i];
		}

		for ( i = 0 ; i < (int)uIndex.size()-1 ; i ++ )
		{
			int iu = uIndex[i];
			int iuplus = uIndex[i+1];
			for ( j = 0 ; j < (int)wIndex.size()-1 ; j ++ )
			{
				int iw = wIndex[j];
				int iwplus = wIndex[j+1];
				glBegin( GL_POLYGON );
			
				glNormal3dv(refl_normals(iu,iw).v);
				glTexCoord2d( uVal[i], wVal[j] );

				glVertex3dv(refl_pnts_xsecs(iu,iw).v);
				glNormal3dv(refl_normals(iu,iwplus).v);
				glTexCoord2d( uVal[i], wVal[j+1] );
				glVertex3dv(refl_pnts_xsecs(iu,iwplus).v);

				glNormal3dv(refl_normals(iuplus,iwplus).v);
				glTexCoord2d( uVal[i+1], wVal[j+1] );
				glVertex3dv(refl_pnts_xsecs(iuplus,iwplus).v);

				glNormal3dv(refl_normals(iuplus,iw).v);
				glTexCoord2d( uVal[i+1], wVal[j] );
				glVertex3dv(refl_pnts_xsecs(iuplus,iw).v);

				glEnd();
			}
		}
	}
	else					// Repeating Texture
	{
		for ( i = 0 ; i < (int)uArray.size()-1 ; i++ )
		{
			for ( j = 0 ; j < (int)wArray.size()-1 ; j ++ )
			{
			  glBegin( GL_POLYGON );		
				glNormal3dv(refl_normals(i,j).v);
				glTexCoord2d( uArray[i]/tex.scaleu, wArray[j]/tex.scalew );
				glVertex3dv(refl_pnts_xsecs(i,j).v);

				glNormal3dv(refl_normals(i,j+1).v);
				glTexCoord2d( uArray[i]/tex.scaleu, wArray[j+1]/tex.scalew );
				glVertex3dv(refl_pnts_xsecs(i,j+1).v);

				glNormal3dv(refl_normals(i+1,j+1).v);
				glTexCoord2d( uArray[i+1]/tex.scaleu, wArray[j+1]/tex.scalew );
				glVertex3dv(refl_pnts_xsecs(i+1,j+1).v);

				glNormal3dv(refl_normals(i+1,j).v);
				glTexCoord2d( uArray[i+1]/tex.scaleu, wArray[j]/tex.scalew );
				glVertex3dv(refl_pnts_xsecs(i+1,j).v);


			  glEnd();
			}
		}
	}
		
}


//==== Draw Reflected Shaded Surf =====//
void Xsec_surf::draw_refl_shaded( int sym_code_in)
{
  if (sym_code_in == NO_SYM) return;

  int i, j;
  int fast_1, fast_2;
  float  fpnt[3];
  double dpnt[3];

  fast_1 = fast_2 = 1;
  if ( fast_draw_flag ) {
  
    fast_1 = MAX(MIN( 4, num_xsecs - 1 ), 1 );
    fast_2 = MAX(MIN( 4, num_pnts  - 1 ), 1 );
    
    if ( num_xsecs <= 6 ) fast_1 = 1;
    if ( num_pnts  <= 6 ) fast_2 = 1;
    
  }
  
  glEnable( GL_LIGHTING );

  if ( sym_code_in != refl_pnts_xsecs_code )
    {
      refl_pnts_xsecs_code =  sym_code_in;
      load_refl_pnts_xsecs();  
    }
  if ( sym_code_in != refl_normals_code )
    {
      refl_normals_code =  sym_code_in;
      load_refl_normals();  
    }

	glEnable( GL_LIGHTING );
          
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_BLEND );
//	glEnable( GL_CULL_FACE );
//	glCullFace( GL_BACK );

  for ( i = 0 ; i < num_xsecs-1 ; i += fast_1 )
    {
      for ( j = 0 ; j < num_pnts-1 ; j += fast_2 )
        {
          glBegin( GL_POLYGON );
           refl_normals(i,j).get_pnt(fpnt);		glNormal3fv(fpnt);
           refl_pnts_xsecs(i,j).get_pnt(dpnt);		glVertex3dv(dpnt);
           refl_normals(i,j+1).get_pnt(fpnt);		glNormal3fv(fpnt);
           refl_pnts_xsecs(i,j+1).get_pnt(dpnt);	glVertex3dv(dpnt);
           refl_normals(i+1,j+1).get_pnt(fpnt);		glNormal3fv(fpnt);
           refl_pnts_xsecs(i+1,j+1).get_pnt(dpnt);	glVertex3dv(dpnt);
           refl_normals(i+1,j).get_pnt(fpnt);		glNormal3fv(fpnt);
           refl_pnts_xsecs(i+1,j).get_pnt(dpnt);	glVertex3dv(dpnt);
          glEnd();
        }
     }
  glDisable( GL_LIGHTING );

}

//==== Load Reflected Pnts And Xsec Array =====//
void Xsec_surf::load_refl_pnts_xsecs()
{
  if (refl_pnts_xsecs_code == NO_SYM) return;

  int i, j;
  vec3d sym_vec;

  refl_pnts_xsecs.init(num_xsecs, num_pnts);

  if ( refl_pnts_xsecs_code == XY_SYM )
    {
      for ( i = 0 ; i < num_xsecs ; i++ )
        for ( j = 0 ; j < num_pnts ; j++ )
          refl_pnts_xsecs(i,j) = pnts_xsecs(i,j).reflect_xy();
    }
  else if ( refl_pnts_xsecs_code == XZ_SYM )
    {
      for ( i = 0 ; i < num_xsecs ; i++ )
        for ( j = 0 ; j < num_pnts ; j++ )
          refl_pnts_xsecs(i,j) = pnts_xsecs(i,j).reflect_xz();
    }
  else if ( refl_pnts_xsecs_code == YZ_SYM )
    {
      for ( i = 0 ; i < num_xsecs ; i++ )
        for ( j = 0 ; j < num_pnts ; j++ )
          refl_pnts_xsecs(i,j) = pnts_xsecs(i,j).reflect_yz();
    }
}

//==== Load Reflected Hidden Surf Array =====//
void Xsec_surf::load_refl_hidden_surf()
{
  //if (refl_hidden_code == NO_SYM) return;

  //int i, j;
  //vec3d sym_vec;

  //if ( refl_hidden_code == XY_SYM )
  //  sym_vec = vec3d( 1.0,  1.0, -1.0);
  //else if ( refl_hidden_code == XZ_SYM )
  //  sym_vec = vec3d( 1.0, -1.0,  1.0);
  //else if ( refl_hidden_code == YZ_SYM )
  //  sym_vec = vec3d(-1.0,  1.0,  1.0);
  // 
  //refl_hidden_surf.init(num_xsecs, num_pnts);
  //for ( i = 0 ; i < num_xsecs ; i++ )
  //  {
  //    for ( j = 0 ; j < num_pnts ; j++ )
  //      {
  //        refl_hidden_surf(i,j) = hidden_surf(i,j)*sym_vec;
  //      }
  //  }
}

//==== Load Reflected Normals Array =====//
void Xsec_surf::load_refl_normals()
{
  if (refl_normals_code == NO_SYM) return;

  int i, j;
  vec3d sym_vec;

  if ( refl_normals_code == XY_SYM )
    sym_vec = vec3d( 1.0,  1.0, -1.0);
  else if ( refl_normals_code == XZ_SYM )
    sym_vec = vec3d( 1.0, -1.0,  1.0);
  else if ( refl_normals_code == YZ_SYM )
    sym_vec = vec3d(-1.0,  1.0,  1.0);
   
  refl_normals.init(num_xsecs, num_pnts);
  for ( i = 0 ; i < num_xsecs ; i++ )
    {
      for ( j = 0 ; j < num_pnts ; j++ )
        {
          refl_normals(i,j) = normals(i,j)*sym_vec;
        }
    }
}

//==== Load Hidden Surf Array =====//
void Xsec_surf::load_hidden_surf()
{
  //hidden_surf.init(num_xsecs, num_pnts);

  //if ( num_xsecs <= 0 || num_pnts <= 0 ) return;

  //double offset_factor = 0.005;
  //double xs1, xs2, xsec_size;

  //int i,j;
  //vec3d pnt0, pnt1, pnt2, pnt3;
  //vec3d edge10, edge32, normal;

  //vec3d center_pnt = (pnts_xsecs(0,0) + 
  //                    pnts_xsecs(num_xsecs-1,num_pnts-1)) * 0.5; 
  //
  //for ( i = 0 ; i < num_xsecs ; i++ )
  //  {
  //    double ind = (double)num_pnts/4.0f;
  //    xs1 = (pnts_xsecs(i, (int)(ind*2.0)) - pnts_xsecs(i, 0)).mag();
  //    xs2 = (pnts_xsecs(i, (int)(ind*3.0)) - pnts_xsecs(i, (int)ind)).mag();
  //    xsec_size = xs1 + xs2;
  //    
  //    if (xsec_size < 1.0e-06)
  //      { 
  //        if (i == 0)
  //          xsec_size = 2*(pnts_xsecs(1,0) - pnts_xsecs(0,0)).mag();
  //        else
  //          xsec_size = 2*(pnts_xsecs(i,0) - pnts_xsecs(i-1,0)).mag();
  //      }

  //    for ( j = 0 ; j < num_pnts ; j++ )
  //      {
  //        if ( i == 0 )
  //          pnt0 = pnts_xsecs(i,j);
  //        else 
  //          pnt0 = pnts_xsecs(i-1,j);

  //        if ( i == num_xsecs-1 )
  //          pnt1 = pnts_xsecs(i,j);
  //        else 
  //          pnt1 = pnts_xsecs(i+1,j);
 
  //        edge10 = pnt1 - pnt0;
  //
  //        if ( j == 0 )
  //          pnt2 = pnts_xsecs(i,num_pnts-2);
  //        else 
  //          pnt2 = pnts_xsecs(i,j-1);

  //        if ( j == num_pnts-1 )
  //          pnt3 = pnts_xsecs(i,1);
  //        else 
  //          pnt3 = pnts_xsecs(i,j+1);

  //        edge32 = pnt3 - pnt2;

  //        normal = cross(edge10, edge32);
  //        normal.normalize();

  //        if ( normal.mag() < 1.0e-06 )
  //          {
  //            normal = pnts_xsecs(i,j) - center_pnt;  
  //            normal.normalize();
  //          }
  //        hidden_surf(i,j) = pnts_xsecs(i,j) - normal*(offset_factor*xsec_size);
  //      }
  //   }
  //load_refl_hidden_surf();
}

//==== Load Normals Array =====//
void Xsec_surf::load_normals()
{
  normals.init(num_xsecs, num_pnts);

  int i, j;
  vec3d tan_u, tan_w;

  for ( i = 0 ; i < num_xsecs ; i++ )
    {
      for ( j = 0 ; j < num_pnts ; j++ )
        {
          //==== Compute Tangent In U Direction ====//
          if ( i == 0 )
            tan_u = pnts_xsecs(1, j) - pnts_xsecs(0, j);
          else if ( i == num_xsecs-1 )
            tan_u = pnts_xsecs(num_xsecs-1, j) - pnts_xsecs(num_xsecs-2, j);
          else
            tan_u = pnts_xsecs( i+1, j) - pnts_xsecs(i-1, j);

          //==== Compute Tangent In W Direction ====//
          if ( j == 0 || j == num_pnts-1 )
            tan_w = pnts_xsecs(i, 1) - pnts_xsecs(i, num_pnts-2);  
          else
            tan_w = pnts_xsecs(i, j+1) - pnts_xsecs(i, j-1);

          if ( tan_w.mag() <= 0.0 )
            {
              if ( i == 0 && j == num_pnts-1)
                tan_w = pnts_xsecs(1, 1) - pnts_xsecs(0, j);  
              else if ( i == 0 )
                tan_w = pnts_xsecs(1, j+1) - pnts_xsecs(0, j);
              else if ( i == num_xsecs-1 && j == num_pnts-1)
                tan_w = pnts_xsecs(num_xsecs-2, 1) - pnts_xsecs(num_xsecs-1, j);  
              else if ( i == num_xsecs-1 )
                tan_w = pnts_xsecs(num_xsecs-2, j+1) - pnts_xsecs(num_xsecs-1, j); 
              else if ( j == num_pnts-1 )
                tan_w = pnts_xsecs(i+1, 1) - pnts_xsecs(i, j); 
              else
                tan_w = pnts_xsecs(i+1, j) - pnts_xsecs(i, j+1); 
            }
          normals(i, j) = cross(tan_u, tan_w); 
          normals(i, j).normalize();
        }
    }
  load_refl_normals();
}

//==== Load Sharp Normals Array =====//
void Xsec_surf::load_sharp_normals()
{
  normals.init(num_xsecs, num_pnts);

  int i, j;
  vec3d tan_u, tan_w;

  for ( i = 0 ; i < num_xsecs ; i++ )
    {
      for ( j = 0 ; j < num_pnts ; j++ )
        {
          //==== Compute Tangent In U Direction ====//
          if ( i == 0 )
            tan_u = pnts_xsecs(1, j) - pnts_xsecs(0, j);
          else if ( i == 1 )
            tan_u = pnts_xsecs(2, j) - pnts_xsecs(1, j);
          else if ( i == num_xsecs-1 )
            tan_u = pnts_xsecs(num_xsecs-1, j) - pnts_xsecs(num_xsecs-2, j);
          else if ( i == num_xsecs-2 )
            tan_u = pnts_xsecs(num_xsecs-2, j) - pnts_xsecs(num_xsecs-3, j);
          else
            tan_u = pnts_xsecs( i+1, j) - pnts_xsecs(i-1, j);

          //==== Compute Tangent In W Direction ====//
          if ( j == 0 )
            tan_w = pnts_xsecs(i, 1) - pnts_xsecs(i, 0);  
           else if ( j == num_pnts-1 )
            tan_w = pnts_xsecs(i, num_pnts-1) - pnts_xsecs(i, num_pnts-2);  
          else
            tan_w = pnts_xsecs(i, j+1) - pnts_xsecs(i, j-1);

          if ( tan_w.mag() <= 0.0 )
            {
              if ( i == 0 && j == num_pnts-1)
                tan_w = pnts_xsecs(1, 1) - pnts_xsecs(0, j);  
              else if ( i == 0 )
                tan_w = pnts_xsecs(1, j+1) - pnts_xsecs(0, j);
              else if ( i == num_xsecs-1 && j == num_pnts-1)
                tan_w = pnts_xsecs(num_xsecs-2, 1) - pnts_xsecs(num_xsecs-1, j);  
              else if ( i == num_xsecs-1 )
                tan_w = pnts_xsecs(num_xsecs-2, j+1) - pnts_xsecs(num_xsecs-1, j); 
              else if ( j == num_pnts-1 )
                tan_w = pnts_xsecs(i+1, 1) - pnts_xsecs(i, j); 
              else
                tan_w = pnts_xsecs(i+1, j) - pnts_xsecs(i, j+1); 
            }
          normals(i, j) = cross(tan_u, tan_w); 
          normals(i, j).normalize();
        }
    }
  load_refl_normals();
}

//==== Load UV Array =====//
void Xsec_surf::load_uw()
{
  int i, j;
  uArray.resize( num_xsecs );
  wArray.resize( num_pnts  );

  if ( num_xsecs == 0 || num_pnts == 0 )
	  return;

  double total_d = 0;
  uArray[0] = total_d;
  for ( i = 1 ; i < num_xsecs ; i++ )
  {
	total_d += dist( pnts_xsecs(i, num_pnts/2), pnts_xsecs(i-1, num_pnts/2) ) + 0.000001;
	uArray[i] = total_d;
  }

  for ( i = 1 ; i < num_xsecs ; i++ )
	uArray[i] = uArray[i]/total_d;

  total_d = 0;
  wArray[0] = total_d;
  for ( j = 1 ; j < num_pnts ; j++ )
  {
	total_d += dist( pnts_xsecs(num_xsecs/2, j-1), pnts_xsecs(num_xsecs/2, j) ) + 0.000001;
	wArray[j] = total_d;
  }
  for ( j = 1 ; j < num_pnts ; j++ )
	wArray[j] = wArray[j]/total_d;

}

//==== Write XML =====//
void Xsec_surf::write(xmlNodePtr node)
{
	int i, j;
	xmlAddIntNode( node, "Num_Pnts", num_pnts);
	xmlAddIntNode( node, "Num_Xsecs", num_xsecs);

	//===== Write Cross Sections =====//
	xmlNodePtr xs_node;
	xmlNodePtr xs_list_node = xmlNewChild( node, NULL, (const xmlChar *)"Cross_Section_List", NULL );

	for ( i = 0 ; i < num_xsecs ; i++ )
	{
		xs_node = xmlNewChild( xs_list_node, NULL, (const xmlChar *)"Cross_Section", NULL );

		Stringc xsstr;
		char numstr[255];
		for (  j = 0 ; j < num_pnts ; j++)
		{
			vec3d pnt = pnts_xsecs(i, j);
			sprintf( numstr, "%lf, %lf, %lf,", pnt.x(), pnt.y(), pnt.z() );
			xsstr.concatenate(numstr);
		}
		xsstr.concatenate("\0");

		xmlAddStringNode( xs_node, "Pnts", xsstr );
	}

}
  
void Xsec_surf::read(xmlNodePtr node)
{
	int i, j;
	num_pnts = xmlFindInt( node, "Num_Pnts", num_pnts );
	num_xsecs = xmlFindInt( node, "Num_Xsecs", num_xsecs );
	pnts_xsecs.init(num_xsecs, num_pnts);

	xmlNodePtr xs_list_node = xmlGetNode( node, "Cross_Section_List", 0 );
	if ( xs_list_node )
	{
		int num_xsecs =  xmlGetNumNames( xs_list_node, "Cross_Section" );
		for ( i = 0 ; i < num_xsecs ; i++ )
		{
			xmlNodePtr xs_node = xmlGetNode( xs_list_node, "Cross_Section", i );
			xmlNodePtr pnts_node = xmlGetNode( xs_node, "Pnts", 0 );
			if ( pnts_node )
			{
				int num_arr =  xmlGetNumArray( pnts_node, ',' );
				double* arr = (double*)malloc( num_arr*sizeof(double) );
				xmlExtractDoubleArray( pnts_node, ',', arr, num_arr );

				assert ( num_arr/3 == num_pnts );

				for ( j = 0 ; j < num_arr ; j+=3 )
				{
					pnts_xsecs(i,j/3).set_xyz( arr[j], arr[j+1], arr[j+2] ); 
				}
				free( arr );
			}
		}
	}

}


//==== Write Cross Section =====//
void Xsec_surf::write_xsec( int index, float mat[4][4], FILE* file_id, double scale_factor )
{
  vec3d temp;

  for ( int j = 0 ; j < num_pnts ; j++ )
    {
      temp = (pnts_xsecs(index, j).transform(mat)) * scale_factor;
      fprintf(file_id, "%25.17e  %25.17e  %25.17e\n", temp.x(), temp.y(),temp.z());
    }

}


//==== Write Cross Section =====//
void Xsec_surf::write_refl_xsec( int sym_code_in, int index, float mat[4][4], FILE* file_id, double scale_factor )
{
  vec3d temp;

  if ( sym_code_in != refl_pnts_xsecs_code )
    {
      refl_pnts_xsecs_code =  sym_code_in;
      load_refl_pnts_xsecs();  
    }

  for ( int j = 0 ; j < num_pnts ; j++ )
    {
      temp = ( refl_pnts_xsecs(index, j).transform(mat) ) * scale_factor;
      fprintf(file_id, "%25.17e  %25.17e  %25.17e\n", temp.x(), temp.y(),temp.z());
    }

}

//==== Write Rhino File =====//
void Xsec_surf::write_rhino_file(int sym_code_in, float mat[4][4], float refl_mat[4][4],
								 ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes )
{
  int i, j;

  //==== Create Bezier Surf and Convert BiCubuc Surf ====//
  bezier_surf besurf;

  load_bezier_surface( &besurf );

  const int bIsRational = false;
  const int dim = 3;
  const int u_degree = 3;
  const int v_degree = 3;
  const int u_cv_count = besurf.get_num_pnts_u();
  const int v_cv_count = besurf.get_num_pnts_w();

  // write a bspline on the default layer
  ON_NurbsSurface nurbs_surface( dim, bIsRational, 
                        u_degree+1, v_degree+1,
                        u_cv_count, v_cv_count );

  double knot_val = 0;
  for ( i = 0; i < nurbs_surface.KnotCount(0); i++ )
  {
    nurbs_surface.SetKnot( 0, i, knot_val );
	if ( i%3 == 2 )
		knot_val += 1.0;
  }

  knot_val = 0;
  for ( i = 0; i < nurbs_surface.KnotCount(1); i++ )
  {
    nurbs_surface.SetKnot( 1, i, knot_val );
	if ( i%3 == 2 )
		knot_val += 1.0;
  }

  ON_3dPoint cvPnt;
  for ( i = 0; i < nurbs_surface.CVCount(0); i++ ) 
  {
    for ( j = 0; j < nurbs_surface.CVCount(1); j++ ) 
	{
		vec3d p = besurf.get_control_pnt( i, j ).transform(mat);
		cvPnt.x = p.x();
		cvPnt.y = p.y();
		cvPnt.z = p.z();

      nurbs_surface.SetCV( i, j, cvPnt );
    }
  }

  if ( nurbs_surface.IsValid() ) 
  {
	archive->Write3dmObject( nurbs_surface, attributes );
  }

  //==== No Reflected Surface - RETURN ====//
  if (sym_code_in == NO_SYM) return;

  // write a bspline on the default layer
  ON_NurbsSurface reflnurbs_surface( dim, bIsRational, 
                        u_degree+1, v_degree+1,
                        u_cv_count, v_cv_count );

  knot_val = 0;
  for ( i = 0; i < reflnurbs_surface.KnotCount(0); i++ )
  {
    reflnurbs_surface.SetKnot( 0, i, knot_val );
	if ( i%3 == 2 )
		knot_val += 1.0;
  }

  knot_val = 0;
  for ( i = 0; i < reflnurbs_surface.KnotCount(1); i++ )
  {
    reflnurbs_surface.SetKnot( 1, i, knot_val );
	if ( i%3 == 2 )
		knot_val += 1.0;
  }

  vec3d sym_vec;
  if ( sym_code_in == XY_SYM )
    sym_vec = vec3d( 1.0,  1.0, -1.0);
  else if ( sym_code_in == XZ_SYM )
    sym_vec = vec3d( 1.0, -1.0,  1.0);
  else if ( sym_code_in == YZ_SYM )
    sym_vec = vec3d(-1.0,  1.0,  1.0);

  int num_cpnts = reflnurbs_surface.CVCount(1);
  for ( i = 0; i < reflnurbs_surface.CVCount(0); i++ ) 
    for ( j = 0; j < reflnurbs_surface.CVCount(1); j++ ) 
	{
		vec3d p = besurf.get_control_pnt( i, num_cpnts - 1 - j )*sym_vec;
		p = p.transform(refl_mat);

		cvPnt.x = p.x();
		cvPnt.y = p.y();
		cvPnt.z = p.z();

      reflnurbs_surface.SetCV( i, j, cvPnt );
    }

  if ( reflnurbs_surface.IsValid() ) 
  {
	archive->Write3dmObject( reflnurbs_surface, attributes );
  }
  else
	  printf("ERROR Xsec_surf::write_rhino_file - reflnurbs_surface NOT valid \n" );

}


//==== Write Bezier for CFD Meshing ====//
void Xsec_surf::write_split_rhino_file( int sym_code_in, float mat[4][4], 
		float refl_mat[4][4], vector <int> & split_u, vector <int> & split_w, bool singleCompFlag,
		ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes )
{
	int i, j;

	//==== Create Bezier Surf and Convert BiCubuc Surf ====//
	bezier_surf besurf;
	load_bezier_surface( &besurf );

	int num_sections = (split_u.size()-1)*(split_w.size()-1);
	if ( singleCompFlag && sym_code_in != NO_SYM )
		num_sections *= 2;

	for ( int iu = 0 ; iu < (int)split_u.size()-1 ; iu++ )
	{
		for ( int iw = 0 ; iw < (int)split_w.size()-1 ; iw++ )
		{
			//==== Write Section ====//
			int num_u = split_u[iu+1] - split_u[iu] + 1;
			int num_w = split_w[iw+1] - split_w[iw] + 1;

			vector< vector< vec3d > > control_vec;
			control_vec.resize( num_u );
			for ( i = 0 ; i < num_u ; i++ )
				control_vec[i].resize( num_w );

			for ( i = 0; i < num_u ; i++ ) 
				for ( j = 0; j < num_w ; j++ ) 
				{
					int ioff = i + split_u[iu];
					int joff = j + split_w[iw];
					control_vec[i][j] = besurf.get_control_pnt( ioff, joff ).transform(mat);
				}

			write_rhino_nurbs_surface( control_vec, archive, attributes );
		}
	}

	//==== No Reflected Surface - RETURN ====//
	if (sym_code_in == NO_SYM) return;

	vec3d sym_vec;
	if ( sym_code_in == XY_SYM )
		sym_vec = vec3d( 1.0,  1.0, -1.0);
	else if ( sym_code_in == XZ_SYM )
		sym_vec = vec3d( 1.0, -1.0,  1.0);
	else if ( sym_code_in == YZ_SYM )
		sym_vec = vec3d(-1.0,  1.0,  1.0);

	for ( int iu = 0 ; iu < (int)split_u.size()-1 ; iu++ )
	{
		for ( int iw = 0 ; iw < (int)split_w.size()-1 ; iw++ )
		{
			//==== Write Section ====//
			int num_u = split_u[iu+1] - split_u[iu] + 1;
			int num_w = split_w[iw+1] - split_w[iw] + 1;

			vector< vector< vec3d > > control_vec;
			control_vec.resize( num_u );
			for ( i = 0 ; i < num_u ; i++ )
				control_vec[i].resize( num_w );

			for ( i = 0; i < num_u ; i++ ) 
				for ( j = 0; j < num_w ; j++ ) 
				{
					int ioff = i + split_u[iu];
					int joff = j + split_w[iw];
					vec3d p = besurf.get_control_pnt( ioff, joff )*sym_vec;
					control_vec[i][j] =p.transform(refl_mat);
				}

			write_rhino_nurbs_surface( control_vec, archive, attributes );
		}
	}


}

//==== Write To Rhino ====//
void Xsec_surf::write_rhino_nurbs_surface( vector< vector< vec3d > > & control_vec, 
		ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes )
{
	int i, j;
	int num_u = control_vec.size();
	if ( num_u < 4 )
		return;
	int num_w = control_vec[0].size();

	int bIsRational = false;
	int dim = 3;
	int u_degree = 3;
	int v_degree = 3;

	// write a bspline on the default layer
	ON_NurbsSurface nurbs_surface( dim, bIsRational, 
							u_degree+1, v_degree+1,
							num_u, num_w );

	double knot_val = 0;
	for ( i = 0; i < nurbs_surface.KnotCount(0); i++ )
	{
		nurbs_surface.SetKnot( 0, i, knot_val );
		if ( i%3 == 2 )
			knot_val += 1.0;
	}

	knot_val = 0;
	for ( i = 0; i < nurbs_surface.KnotCount(1); i++ )
	{
		nurbs_surface.SetKnot( 1, i, knot_val );
		if ( i%3 == 2 )
			knot_val += 1.0;
	}

	ON_3dPoint cvPnt;
	for (  i = 0; i < num_u ; i++ ) 
		for (  j = 0; j < num_w ; j++ ) 
		{
			cvPnt.x = control_vec[i][j].x();
			cvPnt.y = control_vec[i][j].y();
			cvPnt.z = control_vec[i][j].z();
			nurbs_surface.SetCV( i, j, cvPnt );
		}

	if ( nurbs_surface.IsValid() ) 
	{
		archive->Write3dmObject( nurbs_surface, attributes );
	}
}

//==== Write Bezier for CFD Meshing ====//
void Xsec_surf::write_bezier_file( FILE* file_id, int sym_code_in, float mat[4][4], float refl_mat[4][4],  
								    vector <int> & split_u, vector <int> & split_w, bool singleCompFlag )
{
	//==== Create Bezier Surf and Convert BiCubuc Surf ====//
	bezier_surf besurf;
	load_bezier_surface( &besurf );

	int num_sections = (split_u.size()-1)*(split_w.size()-1);

	if ( singleCompFlag && sym_code_in != NO_SYM )
		num_sections *= 2;

	fprintf( file_id, "0 Component\n" );
	fprintf( file_id, "%d  Num_Sections\n", num_sections );
	for ( int iu = 0 ; iu < (int)split_u.size()-1 ; iu++ )
	{
		for ( int iw = 0 ; iw < (int)split_w.size()-1 ; iw++ )
		{
			//==== Write Section ====//
			int num_u = split_u[iu+1] - split_u[iu] + 1;
			int num_w = split_w[iw+1] - split_w[iw] + 1;
			fprintf (file_id, "%d %d  NumU, NumW\n", num_u, num_w );

			for ( int i = split_u[iu] ; i <= split_u[iu+1] ; i++ )
				for ( int j = split_w[iw] ; j <= split_w[iw+1] ; j++ )
				{
					vec3d p = besurf.get_control_pnt( i, j ).transform(mat);

					fprintf( file_id, "%20.20lf %20.20lf %20.20lf\n", p.x(), p.y(), p.z() );
				}
		}
	}

	if (sym_code_in == NO_SYM) return;

	if ( !singleCompFlag )
	{
		fprintf( file_id, "1 Component\n" );
		fprintf( file_id, "%d  Num_Sections\n", num_sections );
	}

	vec3d sym_vec;
	if ( sym_code_in == XY_SYM )
		sym_vec = vec3d( 1.0,  1.0, -1.0);
	else if ( sym_code_in == XZ_SYM )
		sym_vec = vec3d( 1.0, -1.0,  1.0);
	else if ( sym_code_in == YZ_SYM )
		sym_vec = vec3d(-1.0,  1.0,  1.0);

	for ( int iu = 0 ; iu < (int)split_u.size()-1 ; iu++ )
	{
		for ( int iw = 0 ; iw < (int)split_w.size()-1 ; iw++ )
		{
			//==== Write Section ====//
			int num_u = split_u[iu+1] - split_u[iu] + 1;
			int num_w = split_w[iw+1] - split_w[iw] + 1;
			fprintf (file_id, "%d %d  NumU, NumW\n", num_u, num_w );

			for ( int i = split_u[iu] ; i <= split_u[iu+1] ; i++ )
				for ( int j = split_w[iw+1] ; j >= split_w[iw] ; j-- )
				{
					int num_cpnts = besurf.get_num_pnts_w();
					vec3d p = besurf.get_control_pnt( i, j )*sym_vec;
					p = p.transform(refl_mat);

					fprintf( file_id, "%20.20lf %20.20lf %20.20lf\n", p.x(), p.y(), p.z() );
				}
		}
	}



}




//==== Write POV Tris =====//
void Xsec_surf::write_stl_tris( int sym_code_in, float mat[4][4], float refl_mat[4][4], FILE* file_id)
{
//jrg check for point cross-sections....

  int i, j;
  vec3d norm;
  vec3d v0, v1, v2, v3;
  vec3d d30, d21;

  for ( i = 0 ; i < num_xsecs-1 ; i++ )
    {
      for ( j = 0 ; j < num_pnts-1 ; j++ )
        {

          v0 = pnts_xsecs(i,   j).transform(mat);
          v1 = pnts_xsecs(i+1, j).transform(mat);
          v2 = pnts_xsecs(i+1, j+1).transform(mat);
          v3 = pnts_xsecs(i,   j+1).transform(mat);

		  d21 = v2 - v1;

		  if ( d21.mag() > 0.000001 )
		  {
			  norm = cross(d21, v0 - v1); 
			  norm.normalize();

			  fprintf(file_id, " facet normal  %lf %lf %lf\n",  norm.x(), norm.y(), norm.z() );
			  fprintf(file_id, "   outer loop\n");
			  fprintf(file_id, "     vertex %lf %lf %lf\n", v0.x(), v0.y(), v0.z());
			  fprintf(file_id, "     vertex %lf %lf %lf\n", v1.x(), v1.y(), v1.z());
			  fprintf(file_id, "     vertex %lf %lf %lf\n", v2.x(), v2.y(), v2.z());
			  fprintf(file_id, "   endloop\n");
			  fprintf(file_id, " endfacet\n");
		  }

		  d30 = v3 - v0;
		  if ( d30.mag() > 0.000001 )
		  {
			  norm = cross(v2 - v0, d30); 
			  norm.normalize();

			  fprintf(file_id, " facet normal  %lf %lf %lf\n",  norm.x(), norm.y(), norm.z() );
			  fprintf(file_id, "   outer loop\n");
			  fprintf(file_id, "     vertex %lf %lf %lf\n", v0.x(), v0.y(), v0.z());
			  fprintf(file_id, "     vertex %lf %lf %lf\n", v2.x(), v2.y(), v2.z());
			  fprintf(file_id, "     vertex %lf %lf %lf\n", v3.x(), v3.y(), v3.z());
			  fprintf(file_id, "   endloop\n");
			  fprintf(file_id, " endfacet\n");
		  }
        }
   }

 if (sym_code_in == NO_SYM) return;

   for ( i = 0 ; i < num_xsecs-1 ; i++ )
    {
      for ( j = 0 ; j < num_pnts-1 ; j++ )
        {

          v0 = refl_pnts_xsecs(i,   j).transform(refl_mat);
          v1 = refl_pnts_xsecs(i+1, j).transform(refl_mat);
          v2 = refl_pnts_xsecs(i+1, j+1).transform(refl_mat);
          v3 = refl_pnts_xsecs(i,   j+1).transform(refl_mat);

		  d21 = v2 - v1;

		  if ( d21.mag() > 0.000001 )
		  {
			  norm = cross(d21, v0 - v1); 
			  norm.normalize();

			  fprintf(file_id, " facet normal  %lf %lf %lf\n",  norm.x(), norm.y(), norm.z() );
			  fprintf(file_id, "   outer loop\n");
			  fprintf(file_id, "     vertex %lf %lf %lf\n", v0.x(), v0.y(), v0.z());
			  fprintf(file_id, "     vertex %lf %lf %lf\n", v1.x(), v1.y(), v1.z());
			  fprintf(file_id, "     vertex %lf %lf %lf\n", v2.x(), v2.y(), v2.z());
			  fprintf(file_id, "   endloop\n");
			  fprintf(file_id, " endfacet\n");
		  }

		  d30 = v3 - v0;
		  if ( d30.mag() > 0.000001 )
		  {
			  norm = cross(v2 - v0, d30); 
			  norm.normalize();

			  fprintf(file_id, " facet normal  %lf %lf %lf\n",  norm.x(), norm.y(), norm.z() );
			  fprintf(file_id, "   outer loop\n");
			  fprintf(file_id, "     vertex %lf %lf %lf\n", v0.x(), v0.y(), v0.z());
			  fprintf(file_id, "     vertex %lf %lf %lf\n", v2.x(), v2.y(), v2.z());
			  fprintf(file_id, "     vertex %lf %lf %lf\n", v3.x(), v3.y(), v3.z());
			  fprintf(file_id, "   endloop\n");
			  fprintf(file_id, " endfacet\n");
		  }
        }
   }

}

int Xsec_surf::buildX3DStrings( int offset, Stringc &crdstr, Stringc &indstr, int sym_code_in, float mat[4][4], float refl_mat[4][4] )
{
	int i, j, k;
	char numstr[255];

	array_2d< int > ptindex;
	ptindex.init(num_xsecs, num_pnts);

	for ( i = 0 ; i < num_xsecs ; i++ )
	{
		for ( j = 0 ; j < num_pnts ; j++ )
		{
			ptindex(i, j) = offset;
			offset++;

			vec3d pnt = pnts_xsecs(i, j).transform(mat);

			sprintf( numstr, "%lf %lf %lf ", pnt.x(), pnt.y(), pnt.z() );
			crdstr.concatenate(numstr);
		}
	}

	for ( i = 0 ; i < num_xsecs - 1 ; i++ )
	{
		for ( j = 0 ; j < num_pnts - 1 ; j++ )
		{
			int i0, i1, i2, i3;
			i0 = ptindex(i, j);
			i1 = ptindex(i+1, j);
			i2 = ptindex(i+1, j+1);
			i3 = ptindex(i, j+1);

			sprintf( numstr, "%d %d %d %d -1 ", i0, i1, i2, i3 );
			indstr.concatenate(numstr);
		}
	}

	if (sym_code_in != NO_SYM)
	{
		if ( sym_code_in != refl_pnts_xsecs_code )
		{
			refl_pnts_xsecs_code = sym_code_in;
			load_refl_pnts_xsecs();
		}

		for ( i = 0 ; i < num_xsecs ; i++ )
		{
			for ( j = 0 ; j < num_pnts ; j++ )
			{
				ptindex(i, j) = offset;
				offset++;

				vec3d pnt = refl_pnts_xsecs(i, j).transform(refl_mat);

				sprintf( numstr, "%lf %lf %lf ", pnt.x(), pnt.y(), pnt.z() );
				crdstr.concatenate(numstr);
			}
		}

		for ( i = 0 ; i < num_xsecs - 1 ; i++ )
		{
			for ( j = 0 ; j < num_pnts - 1 ; j++ )
			{
				int i0, i1, i2, i3;
				i0 = ptindex(i, j);
				i1 = ptindex(i+1, j);
				i2 = ptindex(i+1, j+1);
				i3 = ptindex(i, j+1);

				sprintf( numstr, "%d %d %d %d -1 ", i0, i3, i2, i1 );
				indstr.concatenate(numstr);
			}
		}
	}
	return offset;
}

TMesh* Xsec_surf::createTMesh(float mat[4][4] )
{
//JRG -> DO SAME FOR REFL TMESH!!!!

	TMesh* tMeshPtr = new TMesh();

	int i, j;
	vec3d norm;
	vec3d v0, v1, v2, v3;
	vec3d d21, d01, d03, d23, d20;

	for ( i = 0 ; i < num_xsecs-1 ; i++ )
    {
      for ( j = 0 ; j < num_pnts-1 ; j++ )
        {

          v0 = pnts_xsecs(i,   j).transform(mat);
          v1 = pnts_xsecs(i+1, j).transform(mat);
          v2 = pnts_xsecs(i+1, j+1).transform(mat);
          v3 = pnts_xsecs(i,   j+1).transform(mat);

		  d21 = v2 - v1;
		  d01 = v0 - v1;
		  d20 = v2 - v0;
		  if ( d21.mag() > 0.000001 && d01.mag() > 0.000001 && d20.mag() > 0.000001 )
		  {
			  norm = cross(d21, d01); 
			  norm.normalize();
			  tMeshPtr->addTri( v0, v1, v2, norm );
		  }

		  d03 = v0 - v3;
		  d23 = v2 - v3;
		  if ( d03.mag() > 0.000001 && d23.mag() > 0.000001 && d20.mag() > 0.000001 )
		  {
			  norm = cross(d03, d23); 
			  norm.normalize();
			  tMeshPtr->addTri( v0, v2, v3, norm );
		  }
        }
	}

	return tMeshPtr;
}

TMesh* Xsec_surf::createReflTMesh( int sym_code_in, float refl_mat[4][4] )
{
	
	if ( sym_code_in != refl_pnts_xsecs_code )
    {
      refl_pnts_xsecs_code = sym_code_in;
      load_refl_pnts_xsecs();  
    }


	TMesh* tMeshPtr = new TMesh();
	tMeshPtr->reflected_flag = true;

	int i, j;
	vec3d norm;
	vec3d v0, v1, v2, v3;
	vec3d d21, d01, d30, d23, d20;

	for ( i = 0 ; i < num_xsecs-1 ; i++ )
	{
		for ( j = 0 ; j < num_pnts-1 ; j++ )
		{
			v0 = refl_pnts_xsecs(i,   j).transform(refl_mat);
			v1 = refl_pnts_xsecs(i+1, j).transform(refl_mat);
			v2 = refl_pnts_xsecs(i+1, j+1).transform(refl_mat);
			v3 = refl_pnts_xsecs(i,   j+1).transform(refl_mat);
		  
			d21 = v2 - v1;
			d01 = v0 - v1;
			d20 = v2 - v0;

			if ( d21.mag() > 0.000001 && d01.mag() > 0.000001 && d20.mag() > 0.000001 )
			{
				norm = cross(v0 - v1, d21); 
				norm.normalize();

				tMeshPtr->addTri( v0, v2, v1, norm );
			}

			d30 = v3 - v0;
			d23 = v2 - v3;
			if ( d30.mag() > 0.000001 && d23.mag() > 0.000001 && d20.mag() > 0.000001)
			{
				norm = cross(d30, v2 - v0 ); 
				norm.normalize();

				tMeshPtr->addTri( v0, v3, v2, norm );
			}
		}
	}
	return tMeshPtr;
}


//==== Write POV Tris =====//
void Xsec_surf::write_pov_smooth_tris( int sym_code_in, float mat[4][4], float refl_mat[4][4], FILE* file_id)
{
  int i, j;
  vec3d v0, v1, v2, v3;
  vec3d n0, n1, n2, n3;


  for ( i = 0 ; i < num_xsecs-1 ; i++ )
    {
      for ( j = 0 ; j < num_pnts-1 ; j++ )
        {
          n0 = pnts_xsecs(i,   j) + normals(i,j);
          n1 = pnts_xsecs(i+1, j) + normals(i+1,j);
          n2 = pnts_xsecs(i+1, j+1) + normals(i+1,j+1);
          n3 = pnts_xsecs(i,   j+1) + normals(i,j+1);

          v0 = pnts_xsecs(i, j).transform(mat);
          v1 = pnts_xsecs(i+1, j).transform(mat);
          v2 = pnts_xsecs(i+1, j+1).transform(mat);
          v3 = pnts_xsecs(i,   j+1).transform(mat);

		  n0 = n0.transform(mat) - v0;
		  n1 = n1.transform(mat) - v1;
		  n2 = n2.transform(mat) - v2;
		  n3 = n3.transform(mat) - v3;

          fprintf(file_id, "smooth_triangle { \n");
          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >, ",    v0.x(), v0.z(), v0.y());
          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >,  \n", n0.x(), n0.z(), n0.y());

		  fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >, ",    v2.x(), v2.z(), v2.y());
		  fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >, \n", n2.x(), n2.z(), n2.y());

          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >, ",    v1.x(), v1.z(), v1.y());
		  fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >  }\n", n1.x(), n1.z(), n1.y());


          fprintf(file_id, "smooth_triangle { \n");
          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >, ",    v0.x(), v0.z(), v0.y());
          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >,  \n", n0.x(), n0.z(), n0.y());

		  fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >, ",    v3.x(), v3.z(), v3.y());
          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >,  \n", n3.x(), n3.z(), n3.y());
 
          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >, ",    v2.x(), v2.z(), v2.y());
		  fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >  }\n", n2.x(), n2.z(), n2.y());

     
        }
   }

 if (sym_code_in == NO_SYM) return;

  //==== Load Up Normals ====//
  refl_normals_code =  sym_code_in;
  load_refl_normals();

 for ( i = 0 ; i < num_xsecs-1 ; i++ )
    {
      for ( j = 0 ; j < num_pnts-1 ; j++ )
        {

          n0 = refl_pnts_xsecs(i,   j) + refl_normals(i,j);
          n1 = refl_pnts_xsecs(i+1, j) + refl_normals(i+1,j);
          n2 = refl_pnts_xsecs(i+1, j+1) + refl_normals(i+1,j+1);
          n3 = refl_pnts_xsecs(i,   j+1) + refl_normals(i,j+1);

          v0 = refl_pnts_xsecs(i, j).transform(refl_mat);
          v1 = refl_pnts_xsecs(i+1, j).transform(refl_mat);
          v2 = refl_pnts_xsecs(i+1, j+1).transform(refl_mat);
          v3 = refl_pnts_xsecs(i,   j+1).transform(refl_mat);

		  n0 = n0.transform(refl_mat) - v0;
		  n1 = n1.transform(refl_mat) - v1;
		  n2 = n2.transform(refl_mat) - v2;
		  n3 = n3.transform(refl_mat) - v3;

          fprintf(file_id, "smooth_triangle { \n");
          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >, ",    v0.x(), v0.z(), v0.y());
          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >,  \n", n0.x(), n0.z(), n0.y());

          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >, ",    v1.x(), v1.z(), v1.y());
          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >,  \n", n1.x(), n1.z(), n1.y());

          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >, ",    v2.x(), v2.z(), v2.y());
		  fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >  }\n", n2.x(), n2.z(), n2.y());

          fprintf(file_id, "smooth_triangle { \n");
          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >, ",    v0.x(), v0.z(), v0.y());
          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >,  \n", n0.x(), n0.z(), n0.y());
 
		  fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >, ",    v2.x(), v2.z(), v2.y());
		  fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >  }\n", n2.x(), n2.z(), n2.y());

          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >, ",    v3.x(), v3.z(), v3.y());
          fprintf(file_id, "< %12.8f,  %12.8f,  %12.8f >,  \n", n3.x(), n3.z(), n3.y());


        }
    }
          
}

//==== Load Bezier Surface =====//
void Xsec_surf::load_bezier_surface( bezier_surf* surf )
{
  int i,j;
  double tension = 0.35;
//  double tension = 0.30;

  //==== Check If Xsecs Are Closed ====//
  int closed_xsec_flag = 1;
  for ( i = 0 ; i < num_xsecs ; i++ )
  {
	vec3d del = pnts_xsecs( i, 0 ) - pnts_xsecs( i, num_pnts-1 );
	if ( del.mag() > 0.00001 )
	{
		closed_xsec_flag = 0;
		break;
	}
  }
//  enum { NADA, SHARP, ZERO, ZERO_X, ZERO_Y, ZERO_Z, };

  vector< Bezier_curve > xcrvVec;	// Xsec Curves
  xcrvVec.resize( num_xsecs );
  
  vector< vec3d > pVec;
  pVec.resize( num_pnts );

  vector< int > pFlagVec;
  pFlagVec.resize( num_pnts );

  for ( i = 0 ; i < num_xsecs ; i++ )
  {
	for ( j = 0 ; j < num_pnts ; j++ )
	{
		pVec[j] = pnts_xsecs( i, j );
		pFlagVec[j] = pnt_tan_flags[j];
	}
	xcrvVec[i].buildCurve2( pVec, pFlagVec, tension, closed_xsec_flag );
  }

  //==== Check If Stringers Are Closed ====//
  int closed_stringer_flag = 1;
  for ( j = 0 ; j < num_pnts ; j++ )
  {
	vec3d del = pnts_xsecs( 0, j ) - pnts_xsecs( num_xsecs-1, j );
	if ( del.mag() > 0.00001 )
	{
		closed_stringer_flag = 0;
		break;
	}
  }

  vector< Bezier_curve > scrvVec;	// Stringer Curves

  int numPntsXcrv = xcrvVec[0].get_num_sections()*3 + 1;
  scrvVec.resize( numPntsXcrv );

  pVec.resize( num_xsecs );
  pFlagVec.resize( num_xsecs );

  for ( i = 0 ; i < numPntsXcrv ; i++ )
  {
	for ( j = 0 ; j < num_xsecs ; j++ )
	{
		pVec[j] = xcrvVec[j].get_pnt( i );
		pFlagVec[j] = xsec_tan_flags[j];
	}
	scrvVec[i].buildCurve2( pVec, pFlagVec, tension, closed_stringer_flag );
  }


  if ( pFlagVec.back() == Bezier_curve::MIRROR_END_CAP )
  {
	  for ( int i = 0 ; i < (int)scrvVec.size() ; i++ )
	  {
		//==== Find Tanget At Tip ====//
		int mirror = (int)scrvVec.size()-1-i;
		int num_ctl_pnts = scrvVec[i].get_num_control_pnts();
		vec3d ptop    = scrvVec[i].get_pnt( num_ctl_pnts-4 );
		vec3d pmirror = scrvVec[mirror].get_pnt( num_ctl_pnts-4 );
		vec3d dir = ptop - pmirror;
		double mag = dir.mag();
		if ( mag > DBL_EPSILON )
		{
			dir.normalize();
			vec3d ep = scrvVec[i].get_pnt( num_ctl_pnts-1 );
			vec3d tp = ep + dir*tension*mag;
			scrvVec[i].put_pnt( num_ctl_pnts-2, tp );
		}
	  }
	}



  int numPntsScrv = scrvVec[0].get_num_sections()*3 + 1;

  //==== Load Points into Surf ====//
  surf->init(numPntsScrv, numPntsXcrv);

  for ( i = 0 ; i < numPntsScrv ; i++ )
  {
	for ( j = 0 ; j < numPntsXcrv ; j++ )
	{
		surf->put_pnt( i, j, scrvVec[j].get_pnt(i) );
	}
  }

}

void Xsec_surf::clear_pnt_tan_flags()
{
	pnt_tan_flags.resize( num_pnts );
	for ( int i = 0 ; i < num_pnts ; i++ )
		pnt_tan_flags[i] = Bezier_curve::NADA;
}

void Xsec_surf::clear_xsec_tan_flags()
{
	xsec_tan_flags.resize( num_xsecs );
	for ( int i = 0 ; i < num_xsecs ; i++ )
		xsec_tan_flags[i] = Bezier_curve::NADA;
}

void Xsec_surf::set_pnt_tan_flag( int ind, int flag )
{
	pnt_tan_flags[ind] = flag;
}

int Xsec_surf::get_pnt_tan_flag( int ind )
{
	return pnt_tan_flags[ind];
}

void Xsec_surf::set_xsec_tan_flag( int ind, int flag )
{
	xsec_tan_flags[ind] = flag;
}

int Xsec_surf::get_xsec_tan_flag( int ind )
{
	return xsec_tan_flags[ind];
}


/*

void Xsec_surf::get_vertex_vec(std::vector<vec3d> * vec)
{
	vec->clear();
	for ( int i = 0 ; i < num_xsecs ; i++ )
	{
		for ( int j = 0 ; j < num_pnts ; j ++ )
		{
			vec->push_back(pnts_xsecs(i,j));
		}
	}
}

vec3d Xsec_surf::get_vertex(int v)
{
	int i = v / num_pnts;
	int j = v % num_pnts;
	if (i < 0)			i = 0;
	if (j < 0)			j = 0;
	if (i >= num_xsecs) i = num_xsecs-1;
	if (j >= num_pnts)  j = num_pnts-1;

	return pnts_xsecs(i,j);
}
*/
vec3d Xsec_surf::get_vertex(double x, double p, int r)
{
	if (x < 0)	x = 0;
	if (p < 0)	p = 0;
	if (x > 1)	x = 1;
	if (p > 1)  p = 1;

	int xsec = (int)(x * (num_xsecs-1) + 0.5);
	int pnts = (int)(p * (num_pnts-1) + 0.5);

	if (r)
	{
		if ( refl_pnts_xsecs_code == XY_SYM )
			return pnts_xsecs(xsec,pnts).reflect_xy();
		else if ( refl_pnts_xsecs_code == XZ_SYM )
			return pnts_xsecs(xsec,pnts).reflect_xz();
		else if ( refl_pnts_xsecs_code == YZ_SYM )
			return pnts_xsecs(xsec,pnts).reflect_yz();
		else 
			return pnts_xsecs(xsec,pnts);
	} 
	else
	{
		return pnts_xsecs(xsec,pnts);
	}
}

/*
vec3d Xsec_surf::get_refl_vertex(int x, int p)
{
	if (x < 0)			x = 0;
	if (p < 0)			p = 0;
	if (x >= num_xsecs) x = num_xsecs-1;
	if (p >= num_pnts)  p = num_pnts-1;

	if ( refl_pnts_xsecs_code == XY_SYM )
		return pnts_xsecs(i,j).reflect_xy();
	else if ( refl_pnts_xsecs_code == XZ_SYM )
		return pnts_xsecs(i,j).reflect_xz();
	else if ( refl_pnts_xsecs_code == YZ_SYM )
		return pnts_xsecs(i,j).reflect_yz();
	else 
		return pnts_xsecs(x,p);
}*/
