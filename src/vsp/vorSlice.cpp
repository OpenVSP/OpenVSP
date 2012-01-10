
#include "vorSlice.h"
#include "vec3d.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#ifdef WIN32
#include <windows.h>		
#endif

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

//jrg There are leaks!!!!

VorSlice::VorSlice()
{
	int i;

//	key_slice_tolerance = 0.001f;
	key_slice_tolerance = 0.005f;
	saved_ncamber = 0;
	default_nslice = 2;
	zm_camb = 1.0;
	zm_vor  = 1.0;
	do_scaling = 0;
	user_scale = 1.0;
	swap_xy = 0;
	sprintf(aircraftName, "Aircraft" );

	/*--- Allocate memory for vor object ----*/
    Vor = (VOROBJ *)malloc(sizeof(VOROBJ)); 

	Vor->ncomps = 0;
	Vor->ncamb = 20; 
	Vor->flat_flag = 0;

	Vor->cp_min = 0.0f;
	Vor->cp_max = 0.0f;
	Vor->min_clamp = 0.0f;
	Vor->max_clamp = 0.0f;
	Vor->scale = 1.0f;
	Vor->ntotcsf = 0;
	Vor->ntraps  = 0;

	reset_Mach = 0;
	reset_Re   = 0;
	new_Mach   = 0.5;
	new_Re     = 1.0;
//	key_slice_tolerance = 0.001f;
	key_slice_tolerance = 0.005f;
	saved_ncamber  = Vor->ncamb;
	default_nslice = 2;
	initKeyFlag = 0;

	Vor->herm    = NULL;
//    Vor->sherm   = NULL;
    Vor->ncross  = NULL;
    Vor->npts    = NULL;

	Vor->cpval   = NULL;
	Vor->cpcol   = NULL;

	Vor->nsubp  = 0;
	Vor->subp = NULL;
	Vor->ssubp = NULL;

	Vor->curr_csf = 0;

	key_slice.current_key = 0;
	key_slice.nkey = 0;
	key_slice.ytemp = 0.0;
	key_slice.keypt = NULL;
	key_slice.nslice = NULL;
	key_slice.sy = NULL;

	curr_trap = 0;


  /* --- Allocate Space for Control Surface Data ---- */
  Vor->ncsftrap    = (int *)calloc(MAX_CSF, sizeof(int));
  Vor->csftype     = (int *)calloc(MAX_CSF, sizeof(int));
  Vor->csfreflect  = (int *)calloc(MAX_CSF, sizeof(int));
  Vor->csfdefangle = (float *)calloc(MAX_CSF, sizeof(float));
  Vor->csfchord    = (float *)calloc(MAX_CSF, sizeof(float));
  Vor->csftrap     = (int **)calloc(MAX_CSF, sizeof(int *));
  Vor->csfsymmetry = (int *)calloc(MAX_CSF, sizeof(int));
  Vor->csfhingepercent = (float *)calloc(MAX_CSF, sizeof(float));

  Vor->csfname = (char **) calloc(MAX_CSF, sizeof(char*));

  for ( i = 0 ; i < MAX_CSF ; i++ ) 
  {
     Vor->csfname[i] = (char *) calloc(80, sizeof(char));
	 Vor->ncsftrap[i] = 0;
     Vor->csftrap[i] = (int *)calloc(MAX_TRAP, sizeof(int));
  }

}

VorSlice::~VorSlice()
{
	free( Vor->ncsftrap );
	free( Vor->csftype );
	free( Vor->csfreflect );
	free( Vor->csfdefangle );
	free( Vor->csfchord );
	free( Vor->csfsymmetry );
	free( Vor->csfhingepercent );

	for ( int i = 0 ; i < MAX_CSF ; i++ ) 
	{
		free( Vor->csfname[i] );
		free( Vor->csftrap[i] );
	}

	free( Vor->csftrap );
	free( Vor->csfname );

	free(Vor);
}


void VorSlice::drawherm()
{
	int icomp, i, j;

	/* --- Loop Through Components ----*/
	for ( icomp = 0 ; icomp < Vor->ncomps ; icomp++)
	{
		for ( i = 0 ; i < Vor->ncross[icomp] ; i++)
		{
          /* ---- Draw Cross Sections ----*/
	      glBegin( GL_LINE_STRIP );
          for ( j = 0 ; j < Vor->npts[icomp] ; j++)
          {
  			  glVertex3fv( Vor->herm[icomp][i][j] );
          }
          glEnd();
        }
		for ( j = 0 ; j < Vor->npts[icomp] ; j++)
        {
          /* ---- Draw Stringer Lines ----*/
		  glBegin( GL_LINE_STRIP );
          for ( i = 0 ; i < Vor->ncross[icomp] ; i++)
          {
			  glVertex3fv( Vor->herm[icomp][i][j] );
          }
          glEnd();
        }
    }
}

int VorSlice::is_key_slice_y( float y_val )
{
	int numKey = get_num_key_slices(); 
	for ( int i = 0 ; i < numKey ; i++ )
	{
		float y = key_slice.y[i];
		if ( fabs( y - y_val ) < 0.000001f )
			return 1;
	}
	return 0;
}

/*======================================================================*
* Module Name:  drawslices
*=======================================================================*
* Description: Draw wireframe sliced trapizoid geometry
*
* Input:  None
*
* Output: None
*=======================================================================*/
void VorSlice::drawslices()
{

	GLboolean smoothFlag = 0;

	glGetBooleanv( GL_LINE_SMOOTH, &smoothFlag );
	if ( smoothFlag )
		glDisable( GL_LINE_SMOOTH );


	glLineWidth( 3.0 );
	glColor3ub( 150, 150, 150 );
	for ( int islc = 0 ; islc < Vor->ntraps ; islc++) 
	{

		// ---- Draw Outline of Traps ----//
		glBegin( GL_LINE_LOOP );
		for (int i = 0 ; i < 4 ; i++)
		{
			//if ( is_key_slice_y( Vor->trap[islc][i][1] )  )
			//{
			//	glColor3ub( 150, 0, 150 );
			//}
			//else
				glColor3ub( 150, 150, 150 );
			glVertex3fv( Vor->trap[islc][i] );
		}
		glEnd();
	}


	//==== Draw Current Key Slice ====//
	int numKey = get_num_key_slices(); 
	if ( numKey > 0 )
	{
		for ( int i = 0 ; i < numKey ; i++ )
		{
			float y = key_slice.y[i];
			glLineWidth( 1.0 );
			glColor3ub( 000, 200, 000 );

			if ( i == key_slice.current_key && !get_key_slice_temp_flag() )
			{
				glLineWidth( 3.0 );
				glColor3ub( 255, 0, 255 );
			}
			if ( key_slice.symmetry_flag && i == get_opposite_keyslice() )
			{
				glLineWidth( 3.0 );
				glColor3ub( 255, 0, 255 );
			}

			glBegin( GL_LINE_LOOP );
				glVertex3f( key_slice.xmin, y, key_slice.zmin );
				glVertex3f( key_slice.xmax, y, key_slice.zmin );
				glVertex3f( key_slice.xmax, y, key_slice.zmax );
				glVertex3f( key_slice.xmin, y, key_slice.zmax );
			glEnd();

			for ( int j = 0 ; j < 8 ; j++ )
			{
				float fract = (float)(j+1)/10.0f;
				float xval = key_slice.xmin + fract*(key_slice.xmax - key_slice.xmin);
				glBegin( GL_LINES );
					glVertex3f( xval, y, key_slice.zmin );
					glVertex3f( xval, y, key_slice.zmax );
				glEnd();
			}
		}


		if ( get_key_slice_temp_flag() )
		{
			float y = key_slice.ytemp;
			glLineWidth( 2.0 );
			glColor3ub( 255, 0, 0 );

			glBegin( GL_LINE_LOOP );
				glVertex3f( key_slice.xmin, y, key_slice.zmin );
				glVertex3f( key_slice.xmax, y, key_slice.zmin );
				glVertex3f( key_slice.xmax, y, key_slice.zmax );
				glVertex3f( key_slice.xmin, y, key_slice.zmax );
			glEnd();

			for ( int j = 0 ; j < 8 ; j++ )
			{
				float fract = (float)(j+1)/10.0f;
				float xval = key_slice.xmin + fract*(key_slice.xmax - key_slice.xmin);
				glBegin( GL_LINES );
					glVertex3f( xval, y, key_slice.zmin );
					glVertex3f( xval, y, key_slice.zmax );
				glEnd();
			}
		}
	}


	if ( smoothFlag )
		glEnable( GL_LINE_SMOOTH );

}

void VorSlice::drawcsf()
{
	int i, icsf, itrap;
	float vert[3];
	float x1, x2, z1, z2;

	//==== Draw Current Trap ====//
	int ctrap = this->get_curr_trap();
	if ( ctrap >= 0 )
	{
		glLineWidth( 6.0 );
		glEnable(GL_LINE_STIPPLE);
		glLineStipple( 2, 0x0F0F );
		glColor3ub( 255, 0, 0 );

		glBegin( GL_LINE_LOOP );
			for ( i = 0 ; i < 4 ; i++)
				glVertex3fv( Vor->trap[ctrap][i] );
		glEnd();

		glLineWidth( 1.0 );
		glDisable(GL_LINE_STIPPLE);
	}

	glLineWidth( 3.0 );

	//==== Loop Thru All Control Surfaces ====//
	for (icsf = 0 ; icsf < Vor->ntotcsf ; icsf++)
    {
		//==== Load Color For The Control Surface ====//
		int r, g, b;
		get_csf_color(icsf, &r, &g, &b);
		glColor3ub( r, g, b );

		//==== Loop Through Each Trap In The Control Surface ====//
		for (itrap = 0 ; itrap < Vor->ncsftrap[icsf] ; itrap++)
		{
			//==== Find Trap Index ====//
			int trap_ind = Vor->csftrap[icsf][itrap];
  
			//==== Check Control Surface Type ====//
			if (Vor->csftype[icsf] == 0)		// Flap
			{
				glBegin( GL_POLYGON );

				glVertex3fv(Vor->trap[trap_ind][1]);

				x1 = Vor->trap[trap_ind][0][0];
				x2 = Vor->trap[trap_ind][1][0];
                vert[0] = (x1 - x2)*Vor->csfchord[icsf] + x2;

				vert[1] = Vor->trap[trap_ind][0][1];

				z1 = Vor->trap[trap_ind][0][2];
				z2 = Vor->trap[trap_ind][1][2];
                vert[2] = (z1 - z2)*Vor->csfchord[icsf] + z2;

				glVertex3fv(vert);

				x1 = Vor->trap[trap_ind][3][0];
				x2 = Vor->trap[trap_ind][2][0];
                vert[0] = (x1 - x2)*Vor->csfchord[icsf] + x2;

				vert[1] = Vor->trap[trap_ind][2][1];

				z1 = Vor->trap[trap_ind][3][2];
				z2 = Vor->trap[trap_ind][2][2];
                vert[2] = (z1 - z2)*Vor->csfchord[icsf] + z2;

				glVertex3fv(vert);
				glVertex3fv(Vor->trap[trap_ind][2]);
				
				glEnd();
             }
            else if (Vor->csftype[icsf] == 1)	// Slat
             {
				glBegin( GL_POLYGON );

				x1 = Vor->trap[trap_ind][0][0];
				x2 = Vor->trap[trap_ind][1][0];

                vert[0] = x1;
				vert[1] = Vor->trap[trap_ind][0][1];
				vert[2] = Vor->trap[trap_ind][0][2];
				glVertex3fv(vert);

				vert[0] = (x2 - x1)*Vor->csfchord[icsf] + x1;

				z1 = Vor->trap[trap_ind][0][2];
				z2 = Vor->trap[trap_ind][1][2];
				vert[2] = (z2 - z1)*Vor->csfchord[icsf] + z1;
 
				glVertex3fv(vert);

				x1 = Vor->trap[trap_ind][3][0];
				x2 = Vor->trap[trap_ind][2][0];
                vert[0] = (x2 - x1)*Vor->csfchord[icsf] + x1;
				vert[1] = Vor->trap[trap_ind][2][1];

				z1 = Vor->trap[trap_ind][3][2];
				z2 = Vor->trap[trap_ind][2][2];
                vert[2] = (z2 - z1)*Vor->csfchord[icsf] + z1;

				glVertex3fv(vert);

				glVertex3fv(Vor->trap[trap_ind][3]);
				
				glEnd();

             }
           else		// All
             {
               glBegin( GL_POLYGON );
               for (i = 0 ; i < 4 ; i++)
                 {
                   glVertex3fv(Vor->trap[trap_ind][i]);
                 }
               glEnd();
             }
         }
    }      
	glLineWidth( 1.0 );
}

/*
 * _draw_point_
 *
 * this func draws a point at the specified x,y,z location.
 * the type of point is specified as an argument...
 *
 * point types are:
 *   1 - main point with cross hair...
 *   2 - other point (smaller and solid)...
 */
void VorSlice::draw_point(float xyz[],int pnt)
{
   glPointSize(8.0);
   glBegin( GL_POINTS );
   glVertex3fv( xyz );
   glEnd();
}

void VorSlice::draw_main_point(float xyz[])
{
   draw_point(xyz,1);
}

void VorSlice::draw_other_point(float xyz[])
{
   draw_point(xyz,2);
}

/*======================================================================*
* Module Name:  drawsubpols
*=======================================================================*
* Description: Draw subdivided polygons
*
* Input:  None
*
* Output: None
*=======================================================================*/
void VorSlice::drawsubpols()
{
  int isub, i;

  glLineWidth( 1.0 );


  /* ---- Loop Through All Sub-Polys ----*/
  for ( isub = 0 ; isub < Vor->nsubp ; isub++)
    {
      glBegin( GL_LINE_LOOP );
      for (i = 0 ; i < 4 ; i++)
        {
         glVertex3fv( Vor->subp[isub][i] );
        }
      glEnd();
    }
}

/*======================================================================*
* Module Name:  drawcpvals
*=======================================================================*
* Description: Draw filled polygons using color to represent Delta Cp
*
* Input:  None
*
* Output: None
*=======================================================================*/
void VorSlice::drawcpvals()
{
  int isub, i;
 

  /* ---- Check If Cp Data Matches Sub-Poly Data ---*/
  if (Vor->ncpv == Vor->nsubp)
    {
      /* ---- Loop Through Sub-Polygons ----*/
      for ( isub = 0 ; isub < Vor->nsubp ; isub++)
        {
      
          /* ---- Set Color For Each Polygon ----*/
          glColor3ub( (unsigned char)Vor->cpcol[isub][0], 
			          (unsigned char)Vor->cpcol[isub][1], 
                      (unsigned char)Vor->cpcol[isub][2] );

          glBegin( GL_POLYGON );
          for (i = 0 ; i < 4 ; i++)
          {
              glVertex3fv( Vor->subp[isub][i] );
          }
          glEnd();
        }
     }
}

/*======================================================================*
* Module Name:  drawrgb
*=======================================================================*
* Description: Draw cp contours on hermite surface.
*
* Input:  None
*
* Output: None
*=======================================================================*/
void VorSlice::drawrgb()
{
  int icomp, i, j;
  float pcent;
  short cvec[3];

  /* --- Loop Through Components ----*/
  for ( icomp = 0 ; icomp < Vor->ncomps ; icomp++)
    {
      for ( i = 0 ; i < Vor->ncross[icomp]-1 ; i++)
        {
          for ( j = 0 ; j < Vor->npts[icomp]-1 ; j++)
            {
              glBegin( GL_POLYGON ); 

                 pcent = (Vor->herm[icomp][i][j][3]-Vor->cp_surf_min_clamp)/
                         (Vor->cp_surf_max_clamp-Vor->cp_surf_min_clamp);
                 find_rgb(pcent, cvec);
				glColor3ub( (unsigned char)cvec[0], (unsigned char)cvec[1], (unsigned char)cvec[2] );
				glVertex3fv( Vor->herm[icomp][i][j] );

                 pcent = (Vor->herm[icomp][i+1][j][3]-Vor->cp_surf_min_clamp)/
                         (Vor->cp_surf_max_clamp-Vor->cp_surf_min_clamp);

                 find_rgb(pcent, cvec);
				glColor3ub( (unsigned char)cvec[0], (unsigned char)cvec[1], (unsigned char)cvec[2] );
				glVertex3fv( Vor->herm[icomp][i+1][j] );

                 pcent= (Vor->herm[icomp][i+1][j+1][3]-Vor->cp_surf_min_clamp)/
                         (Vor->cp_surf_max_clamp-Vor->cp_surf_min_clamp);

                 find_rgb(pcent, cvec);
				glColor3ub( (unsigned char)cvec[0], (unsigned char)cvec[1], (unsigned char)cvec[2] );
				glVertex3fv( Vor->herm[icomp][i+1][j+1] );

                 pcent = (Vor->herm[icomp][i][j+1][3]-Vor->cp_surf_min_clamp)/
                         (Vor->cp_surf_max_clamp-Vor->cp_surf_min_clamp);

                 find_rgb(pcent, cvec);
				glColor3ub( (unsigned char)cvec[0], (unsigned char)cvec[1], (unsigned char)cvec[2] );
				glVertex3fv( Vor->herm[icomp][i][j+1] );

			  glEnd();
            }
        }
    }
}


/*======================================================================*
* Module Name:  write_surf_press_pnts
*=======================================================================*
* Description: Call routines to write hermite geom for surf press calc
*
* Input: flname	-	Current input file name
*
* Output: None
*=======================================================================*/
int VorSlice::write_surf_press_pnts(char* srfname)
{
   int icomp, i, j;
   int num_nodes = 0;

   for ( icomp = 0 ; icomp < Vor->ncomps ; icomp++) 
      for ( i = 0 ; i < Vor->ncross[icomp] ; i++) 
         for ( j = 0 ; j < Vor->npts[icomp] ; j++)
            num_nodes++;


   //===== Write File =====//
   FILE* fptr = fopen( srfname, "w" );

   if ( !fptr )
	   return 0;

   fprintf( fptr, "%d\n", num_nodes );
   
   int node = 0;
   for ( icomp = 0 ; icomp < Vor->ncomps ; icomp++) 
   {
		for ( i = 0 ; i < Vor->ncross[icomp] ; i++) 
		{
			for ( j = 0 ; j < Vor->npts[icomp] ; j++) 
			{            
				node++;
	            fprintf( fptr, "%f %f %f 0.0 \n", Vor->herm[icomp][i][j][0], 
					                              Vor->herm[icomp][i][j][1], 
											      Vor->herm[icomp][i][j][2] );
			}
		}
	}

   fclose(fptr);

   return 1;
}

/*======================================================================*
* Module Name:  calc_surf_press
*=======================================================================*
* Description: Call routines to calculate surface pressures on hermite geom.
*
* Input: flname	-	Current input file name
*
* Output: None
*=======================================================================*/
int VorSlice::calc_surf_press(char* srfname)
{
   /* count number of nodes */
   int i, j, icomp, node, num_nodes, hits;
   float *x, *y, *z, *cp, *dcp, cp_c;

   num_nodes = 0;

   for ( icomp = 0 ; icomp < Vor->ncomps ; icomp++) 
      for ( i = 0 ; i < Vor->ncross[icomp] ; i++) 
         for ( j = 0 ; j < Vor->npts[icomp] ; j++)
            num_nodes++;
  
   x   = (float *) calloc(num_nodes+2,sizeof(float));
   y   = (float *) calloc(num_nodes+2,sizeof(float));
   z   = (float *) calloc(num_nodes+2,sizeof(float));
   cp  = (float *) calloc(num_nodes+2,sizeof(float));
   dcp = (float *) calloc(num_nodes+2,sizeof(float));


   //===== Read File =====//
   FILE* fptr = fopen( srfname, "r" );

   if ( !fptr )
	   return 0;

   int read_num_nodes;
   fscanf( fptr, "%d", &read_num_nodes );

   if ( read_num_nodes != num_nodes )
	   return 0;

   for ( i = 0 ; i < num_nodes ; i++ )
   {
		fscanf( fptr, "%f %f %f %f", &x[i], &y[i], &z[i], &cp[i] );
		dcp[i] = 0;
   }
   
   fclose(fptr);

   node = 0;
   for ( icomp = 0 ; icomp < Vor->ncomps ; icomp++) 
   {
		for ( i = 0 ; i < Vor->ncross[icomp] ; i++) 
		{
			for ( j = 0 ; j < Vor->npts[icomp] ; j++) 
			{                         
				node++;
				Vor->herm[icomp][i][j][3] = cp[node];
			}
		}
   }

   node = 0;
   for ( icomp = 0 ; icomp < Vor->ncomps ; icomp++) 
   {
		for ( i = 0 ; i < Vor->ncross[icomp] ; i++) 
		{
			for ( j = 0 ; j < Vor->npts[icomp] ; j++) 
			{                                     
				hits = 0;
				node++;

				cp_c = Vor->herm[icomp][i][j][3];

				if ( i - 1 > 0                  ) { hits++ ; dcp[node] += Vor->herm[icomp][i-1][j  ][3] - cp_c; }
				if ( i + 1 < Vor->ncross[icomp] ) { hits++ ; dcp[node] += Vor->herm[icomp][i+1][j  ][3] - cp_c; }
				if ( j - 1 > 0                  ) { hits++ ; dcp[node] += Vor->herm[icomp][i  ][j-1][3] - cp_c; }
				if ( j + 1 < Vor->npts[icomp]   ) { hits++ ; dcp[node] += Vor->herm[icomp][i  ][j+1][3] - cp_c; }

				dcp[node] /= hits;
			}
		}
   }

   Vor->cp_surf_min =  1.e6;
   Vor->cp_surf_max = -1.e6;

   for ( i = 1 ; i <= num_nodes ; i++ ) {

      cp[i] += dcp[i];

      Vor->cp_surf_min = MIN(cp[i],Vor->cp_surf_min);
      Vor->cp_surf_max = MAX(cp[i],Vor->cp_surf_max);

   }

   Vor->cp_surf_min_clamp = MAX(-3.0f,Vor->cp_surf_min);
   Vor->cp_surf_max_clamp = Vor->cp_surf_max;

   printf("Vor->cp_surf_min,Vor->cp_surf_max: %f, %f \n",
           Vor->cp_surf_min,Vor->cp_surf_max);

   free(x);
   free(y);
   free(z);
   free(cp);
   free(dcp);

   return(1);

}  

/*======================================================================*
* Module Name:  drawcamber
*=======================================================================*
* Description: Draw camber lines
*
* Input:  None
*
* Output: None
*=======================================================================*/

void VorSlice::drawcamber()
{
  int islc, icamb;
  float x1, x2, xdelt;
  float y1, y2, ydelt;
  float z1, z2, zdelt;
  float vert[3], x, y, z, f, delta;
  float zmax = 0.; /* lep debug */
  float normal[3];
  
  /* ---- Loop Through All Trapizoids ----*/
  for ( islc = 0 ; islc < Vor->ntraps ; islc++)
    {
    
      vert[1] = (Vor->trap[islc][0][1] + Vor->trap[islc][3][1])/2.0f;
      x1 = (Vor->trap[islc][0][0] + Vor->trap[islc][3][0])/2.0f;
      x2 = (Vor->trap[islc][1][0] + Vor->trap[islc][2][0])/2.0f;
      xdelt = (x2 - x1)/(float)(Vor->ncamb - 1);

      y1 = (Vor->trap[islc][0][1] + Vor->trap[islc][3][1])/2.0f;
      y2 = (Vor->trap[islc][1][1] + Vor->trap[islc][2][1])/2.0f;
      ydelt = (y2 - y1)/(float)(Vor->ncamb - 1);

      z1 = (Vor->trap[islc][0][2] + Vor->trap[islc][3][2])/2.0f;
      z2 = (Vor->trap[islc][1][2] + Vor->trap[islc][2][2])/2.0f;
      zdelt = (z2 - z1)/(float)(Vor->ncamb - 1);

      /* Calculate normal for this trap */           
      get_trap_normal(islc,normal);
      
      /* Draw the camber line */
      glColor3f( 1.0, 0.0, 1.0 );
           
      glBegin( GL_LINE_STRIP );
  
         for (icamb = 0 ; icamb < Vor->ncamb ; icamb++) {

             x = x1 + (float)(icamb)*xdelt;
             y = y1 + (float)(icamb)*ydelt;
             z = z1 + (float)(icamb)*zdelt;
             
             f = Vor->camb[islc][icamb];

             vert[0] = x;
             vert[1] = y + f*zm_camb*normal[1];
             vert[2] = z + f*zm_camb*normal[2];
             
	     glVertex3fv( vert );

	     if (vert[2] > zmax) /* lep debug */
	        zmax = vert[2];  /* lep debug */

         }
        
      glEnd();
      
      /* Draw the normal vector */      
      glColor3ub( 0, 255, 0 );

      glBegin( GL_LINES );      
      
         vert[0] = x1;
         vert[1] = y1;
         vert[2] = z1;
         
	 glVertex3fv( vert );
         
         delta = Vor->scale;        
         vert[0] = x1 + delta*normal[0];
         vert[1] = y1 + delta*normal[1];
         vert[2] = z1 + delta*normal[2];
                  
	 glVertex3fv( vert );
         
      glEnd();         

    }
}

/*======================================================================*
* Module Name:  get_trap_normal
*=======================================================================*
* Description: Calculate normal to trap poly
*
* Input:  None
*
* Output: None
*=======================================================================*/

void VorSlice::get_trap_normal(int islc, float *normal)
{
 
    float mag;
    float p1[3], p2[3], p3[3], p4[3];
    float v1[3], v2[3], v3[3], v4[3];
    
    /* Get 4 points that make up this trap */
    
    p1[0] = Vor->trap[islc][0][0];
    p1[1] = Vor->trap[islc][0][1];
    p1[2] = Vor->trap[islc][0][2];

    p2[0] = Vor->trap[islc][1][0];
    p2[1] = Vor->trap[islc][1][1];
    p2[2] = Vor->trap[islc][1][2];

    p3[0] = Vor->trap[islc][2][0];
    p3[1] = Vor->trap[islc][2][1];
    p3[2] = Vor->trap[islc][2][2];

    p4[0] = Vor->trap[islc][3][0];
    p4[1] = Vor->trap[islc][3][1];
    p4[2] = Vor->trap[islc][3][2];
    
    /* 1st triangle is 1-2-3 */
    
    v1[0] = p2[0] - p1[0];
    v1[1] = p2[1] - p1[1];
    v1[2] = p2[2] - p1[2];
    
    v2[0] = p3[0] - p1[0];
    v2[1] = p3[1] - p1[1];
    v2[2] = p3[2] - p1[2];
    
    vector_cross(v1,v2,v3);

    /* 2nd triangle is 1-3-4*/
    
    v1[0] = p3[0] - p1[0];
    v1[1] = p3[1] - p1[1];
    v1[2] = p3[2] - p1[2];
    
    v2[0] = p4[0] - p1[0];
    v2[1] = p4[1] - p1[1];
    v2[2] = p4[2] - p1[2];
    
    vector_cross(v1,v2,v4);

    /* Average and normalize */
    
    normal[0] = v3[0] + v4[0];
    normal[1] = v3[1] + v4[1];
    normal[2] = v3[2] + v4[2];
    
    mag = (float)sqrt(vector_dot(normal,normal));
    
    normal[0] /= mag;
    normal[1] /= mag;
    normal[2] /= mag;
        
}

/*##############################################################################

                        Function vector_cross

Function Description:

The function finds cross product of two 3D vectors.

Coded By: David J. Kinney
    Date: 11 - 4 - 1994

##############################################################################*/

void VorSlice::vector_cross(float *vec1, float *vec2, float *cross)
{

    cross[0] =  ( vec1[1]*vec2[2] - vec2[1]*vec1[2] );
    cross[1] = -( vec1[0]*vec2[2] - vec2[0]*vec1[2] );
    cross[2] =  ( vec1[0]*vec2[1] - vec2[0]*vec1[1] );
}

/*##############################################################################

                        Function vector_dot

Function Description:

The function finds dot product of two 3D vectors.

Coded By: David J. Kinney
    Date: 11 - 4 - 1994

##############################################################################*/

float VorSlice::vector_dot(float *vec1, float *vec2)
{
    float dot;
    dot = vec1[0]*vec2[0] + vec1[1]*vec2[1] + vec1[2]*vec2[2]; 
    return(dot);
}


/*
 * trap_area
 *
 * this func computes the area of the specified trapezoid and returns that
 * value...
 * NOTE: z is ignored in this computation; basically we compute the area of
 * the trapezoid as if it were projected onto a constant z plane...
 */
float VorSlice::trap_area(const trap_face* const tface)
{
   const float* const lf = tface->low_front;
   const float* const hf = tface->high_front;
   const float* const lb = tface->low_back;
   const float* const hb = tface->high_back;
   float maxx, minx, bb_base, t1_base, t2_base, height, area;

   minx = lf[0] < hf[0] ? lf[0] : hf[0];
   maxx = lb[0] > hb[0] ? lb[0] : hb[0];

   bb_base = maxx - minx;

   t1_base = fabsf(lf[0] - hf[0]);
   t2_base = fabsf(lb[0] - hb[0]);

   height = hf[1] - lf[1];

   area = height * (bb_base - 0.5f * (t1_base + t2_base));

   return (area);
}


/*
 * area_between
 *
 * this func computes the area of all the trapezoids that lie
 * completely within the specified y range.  The total area is returned.
 * See trap_area func above...
 */
float VorSlice::area_between(float ylow, float yhigh)
{
   float area = 0.0, lfy, hfy;
   int it;

   for (it = 0; it < Vor->ntraps; it ++) {
      lfy = Trap_face[it].low_front[1];
      hfy = Trap_face[it].high_front[1];
      if (ylow <= lfy && hfy <= yhigh)
	 area += trap_area(&Trap_face[it]);
   }
   return(area);
}


/*
 * build_trap_face_array
 *
 * allocate and fill up the trap_face array with face information from each 
 * trapezoid in the vorobj struct...
 */
void VorSlice::build_trap_face_array()
{
   int nt, it, ip;
   float ylow, yhigh;

/* allocate the trap_face array */

   nt = Vor->ntraps;
   Trap_face = (trap_face*) malloc(sizeof(trap_face) * nt);
   if (!Trap_face) {
      fprintf(stderr,"FATAL error in build_trap_face_array: malloc=0\n");
      exit(1);
   }

/* fill the trap face array */

   for (it = 0; it < nt; it ++) {
      Trap_face[it].low_front = 0;
      Trap_face[it].high_front = 0;
      Trap_face[it].low_back = 0;
      Trap_face[it].high_back = 0;
      ylow = Vor->trap[it][0][1];
      yhigh = ylow;
      for (ip = 1; ip < 3 && ylow == yhigh; ip ++) {
         if (Vor->trap[it][ip][1] < ylow) {
            ylow = Vor->trap[it][ip][1];
         }
         else if (Vor->trap[it][ip][1] > yhigh) {
            yhigh = Vor->trap[it][ip][1];
         }
      }
      for (ip = 0; ip < 4; ip ++) {
         if (Vor->trap[it][ip][1] == ylow) {
            if (Trap_face[it].low_front) {
               if (Vor->trap[it][ip][0] < Trap_face[it].low_front[0]) {
                  Trap_face[it].low_back = Trap_face[it].low_front;
                  Trap_face[it].low_front = Vor->trap[it][ip];
               }
               else {
                  Trap_face[it].low_back = Vor->trap[it][ip];
               }
            }
            else {
               Trap_face[it].low_front = Vor->trap[it][ip];
            }
         }
         else {
            if (Trap_face[it].high_front) {
               if (Vor->trap[it][ip][0] < Trap_face[it].high_front[0]) {
                  Trap_face[it].high_back = Trap_face[it].high_front;
                  Trap_face[it].high_front = Vor->trap[it][ip];
               }
               else {
                  Trap_face[it].high_back = Vor->trap[it][ip];
               }
            }
            else {
               Trap_face[it].high_front = Vor->trap[it][ip];
            }
         }
      }
   }
}


/*
 * free_trap_face_array
 *
 * free up the storage for the trapezoid face array...
 */
void VorSlice::free_trap_face_array()
{
   if (Trap_face) {
      free(Trap_face);
      Trap_face = 0;
   }
}


/*
 * find_low_key_slices
 *
 * this func is called by find_key_slices.  it takes as arguments the trapezoid
 * number (it), and the threshold for the cosine angle comparison (cos_limit). 
 * this func checks to see if any key slices should pass thru the bottom (low
 * y value) of the specified trapezoid.  If a key slice is desired then 1 is 
 * returned, else 0.
 *
 * The lowest y value (ylow_out) in the trapezoid is passed back...
 */
int VorSlice::find_low_key_slices(const int it, 
                               const float cos_limit, float *ylow_out)
{
   const float ylowest = Trap_face[0].low_front[1];
   int it2, front_match = 0, back_match = 0, loop;
   float ylow, yhigh2, x, z, x2, z2;

   ylow = Trap_face[it].low_front[1];
   if (ylow > ylowest) {
      for (it2 = it - 1, loop = 1; it2 >= 0 && loop; it2 --) {
         yhigh2 = Trap_face[it2].high_front[1];
         if (ylow == yhigh2) {
            x = Trap_face[it].low_front[0];
            z = Trap_face[it].low_front[2];
            x2 = Trap_face[it2].high_front[0];
            z2 = Trap_face[it2].high_front[2];
            if (x == x2 && z == z2) { /* we have a tentative match; check
                                         angle... */
               if (angles_match(Trap_face[it].low_front,
                                Trap_face[it].high_front,
                                Trap_face[it2].low_front,
                                Trap_face[it2].high_front, cos_limit)) {
                  front_match = 1;
               }
               else {
                  loop = 0;
               }
            }
            if (loop) {
               x = Trap_face[it].low_back[0];
               z = Trap_face[it].low_back[2];
               x2 = Trap_face[it2].high_back[0];
               z2 = Trap_face[it2].high_back[2];
               if (x == x2 && z == z2) { /* we have a tentative match; check
                                            angle... */
                  if (angles_match(Trap_face[it].low_back,
                                   Trap_face[it].high_back,
                                   Trap_face[it2].low_back,
                                   Trap_face[it2].high_back, cos_limit)) {
                     back_match = 1;
                  }
                  else {
                     loop = 0;
                  }
               }
            }/*if*/
         }
         else if (ylow > yhigh2) {
            loop = 0;
         }/*if*/
         loop = loop && !(front_match && back_match);
      }/*for*/
   }/*if*/

   *ylow_out = ylow;

/* should make sure it returns 0 if ylow == ylowest ? */

   return(!(front_match && back_match));
}


/*
 * find_high_key_slices
 *
 * this func is called by find_key_slices.  it takes as arguments the trapezoid
 * number (it), and the threshold for the cosine angle comparison (cos_limit). 
 * this func checks to see if any key slices should pass thru the top (high y
 * value) of the specified trapezoid.  If a key slice is desired then 1 is 
 * returned, else 0.
 *
 * The highest y value (yhigh_out) in the trapezoid is passed back...
 */
int VorSlice::find_high_key_slices(const int it, 
                               const float cos_limit, float *yhigh_out)
{
   const float yhighest = Trap_face[Vor->ntraps - 1].high_front[1];
   int it2, front_match = 0, back_match = 0, loop;
   float yhigh, ylow2, x, z, x2, z2;

   yhigh = Trap_face[it].high_front[1];
   if (yhigh < yhighest) {
      for (it2 = it + 1, loop = 1; it2 < Vor->ntraps && loop; it2 ++) {
         ylow2 = Trap_face[it2].low_front[1];
         if (yhigh == ylow2) {
            x = Trap_face[it].high_front[0];
            z = Trap_face[it].high_front[2];
            x2 = Trap_face[it2].low_front[0];
            z2 = Trap_face[it2].low_front[2];
            if (x == x2 && z == z2) { /* we have a tentative match; check
                                         angle... */
               if (angles_match(Trap_face[it].low_front,
                                Trap_face[it].high_front,
                                Trap_face[it2].low_front,
                                Trap_face[it2].high_front, cos_limit)) {
                  front_match = 1;
               }
               else {
                  loop = 0;
               }
            }
            if (loop) {
               x = Trap_face[it].high_back[0];
               z = Trap_face[it].high_back[2];
               x2 = Trap_face[it2].low_back[0];
               z2 = Trap_face[it2].low_back[2];
               if (x == x2 && z == z2) { /* we have a tentative match; check
                                            angle... */
                  if (angles_match(Trap_face[it].low_back,
                                   Trap_face[it].high_back,
                                   Trap_face[it2].low_back,
                                   Trap_face[it2].high_back, cos_limit)) {
                     back_match = 1;
                  }
                  else {
                     loop = 0;
                  }
               }
            }/*if*/
         }
         else if (yhigh < ylow2) {
            loop = 0;
         }/*if*/
         loop = loop && !(front_match && back_match);
      }/*for*/
   }/*if*/

   *yhigh_out = yhigh;

/* should make sure it returns 0 if yhigh == yhighest ? */

   return(!(front_match && back_match));
}



/*
 * find_key_slices
 *
 * Look thru the trapezoid face array for places where key slices should be
 * added (and add 'em).  Those places are:
 *   1) points which are found in only one trapezoid, and
 *   2) points where the angle between the faces is "large"
 * It is assumed that the trapezoids are sorted according to ascending y...
 *
 * NOTE: we turn off symmetry flag temporarily so that this auto-slicing will
 * work with asymmetric models also...
 */
int VorSlice::find_key_slices()
{
   const int temp_sym_flag = get_symmetry_flag();
   const float cos_limit = 0.984f;
   int itlow, ithigh, keys_found = 0; 
   float ylow, yhigh;

   set_symmetry_flag(0);

   if (find_high_key_slices(0,cos_limit,&yhigh)) {
      set_temp_key_slice_y(yhigh);
      if (! key_slice_exists()) {
         add_key_slice();
         keys_found = 1;
      }
   }
   if (find_low_key_slices(Vor->ntraps - 1,cos_limit,&ylow)) {
      set_temp_key_slice_y(ylow);
      if (! key_slice_exists()) {
         add_key_slice();
         keys_found = 1;
      }
   }
   for (itlow = 1, ithigh = Vor->ntraps - 2; itlow <= ithigh; itlow ++, 
                                                              ithigh --) {
      if (find_low_key_slices(itlow,cos_limit,&ylow)) {
         set_temp_key_slice_y(ylow);
         if (! key_slice_exists()) {
            add_key_slice();
            keys_found = 1;
         }
      }
      if (find_high_key_slices(ithigh,cos_limit,&yhigh)) {
         set_temp_key_slice_y(yhigh);
         if (! key_slice_exists()) {
            add_key_slice();
            keys_found = 1;
         }
      }
      if (itlow != ithigh) {
         if (find_high_key_slices(itlow,cos_limit,&yhigh)) {
            set_temp_key_slice_y(yhigh);
            if (! key_slice_exists()) {
               add_key_slice();
               keys_found = 1;
            }
         }
         if (find_low_key_slices(ithigh,cos_limit,&ylow)) {
            set_temp_key_slice_y(ylow);
            if (! key_slice_exists()) {
               add_key_slice();
               keys_found = 1;
            }
         }
      }
   }

   set_key_slice_temp_flag(0);

   set_symmetry_flag(temp_sym_flag);

   return(keys_found);
}

/*
 * compute_area_ratio
 *
 * This func:
 *   1. computes the area between the key slices on either side of the
 *      current key slice.
 *   2. deletes the current key slice.
 *   3. computes the area between the two aforementioned key slices
 *   4. divides the difference between the two areas (computed in steps 1 and
 *      3) by the area computed in step 1.
 * The absolute value of the result is passed back in the first argument.
 * The second argument contains the y coordinate of the deleted slice; this can
 * be used by the caller to re-add the deleted key slice.
 *
 * This func is only called by eliminate_superfluous_keys...
 */
void VorSlice::compute_area_ratio(float *ratio, float *ytemp)
{
   float ylow, yhigh, orig_area, new_area;

   down_current_key_slice();
   ylow = get_current_key_slice_y();
   up_current_key_slice();
   up_current_key_slice();
   yhigh = get_current_key_slice_y();
   down_current_key_slice();
   build_trap_face_array();
   orig_area = area_between(ylow,yhigh);
   free_trap_face_array();
   *ytemp = get_current_key_slice_y();
   delete_key_slice();
   compute_slice();
   build_trap_face_array();
   new_area = area_between(ylow,yhigh);
   free_trap_face_array();
   *ratio = fabsf((orig_area - new_area) / orig_area);
}


/*
 * eliminate_superfluous_keys
 *
 * This func eliminates any key slices which do not have a major impact on
 * the area.  We do this by eliminating the key slices one by one to see what
 * the impact on the local area is.  We eliminate those which have a small
 * impact on area (below a certain threshold).
 *
 * This seems to work as well for the f16 case as the algorithm which only
 * removes one slice per pass... and this one is much faster...
 *
 * This func no longer assumes that key slices have been found using the 
 * "outline discontinuity" method (implemented by find_key_slices) and no does
 * not assume that the number of slices in each region (area bounded by two 
 * adjacent key slices) is two (this func sets it to two in each region).
 *
 */
void VorSlice::eliminate_superfluous_keys()
{
   const float tolerance = 0.02f; /* .05 is good enough for f16 but not others*/
				 /* .01 is best for e7 case so far */
				 /* .03 good enough for f16 but not e7 */
				 /* .02 is good enough for f16 although there
				    are some slight asymmetries; .02 is not bad
				    for e7 but nose is flat (still no center
				    slice) and leading edges of wings are
				    located differently due to slight
				    asymmetry in key slices/points */
   const int temp_sym_flag = get_symmetry_flag();
   float ratio, ytemp;
   int nkey, ikey, midlo, midhi;

   set_symmetry_flag(0);

   nkey = get_num_key_slices();

   for (ikey = set_current_key_slice(nkey - 1); ikey >= 0; ikey --)
      set_nslice(2);

   midhi = nkey / 2;
   midlo = (nkey % 2) ? midhi : midhi - 1;
   ikey = set_current_key_slice(midhi + 1);
   while (ikey < nkey - 1) {
      compute_area_ratio(&ratio, &ytemp);
      if (ratio >= tolerance) {
         set_temp_key_slice_y(ytemp);
         add_key_slice();
         compute_slice();
         ikey = set_current_key_slice(ikey + 1);
      }
      else {
         nkey --;
         ikey = set_current_key_slice(ikey);
      }
   }

   ikey = set_current_key_slice(midlo - 1);
   while (ikey > 0) {
      compute_area_ratio(&ratio, &ytemp);
      if (ratio >= tolerance) {
         set_temp_key_slice_y(ytemp);
         add_key_slice();
         compute_slice();
      }
      else {
         nkey --;
         midhi --;
         midlo --;
      }
      ikey = set_current_key_slice(ikey - 1);
   }

   ikey = set_current_key_slice(midhi);
   compute_area_ratio(&ratio, &ytemp);
   if (ratio >= tolerance) {
      set_temp_key_slice_y(ytemp);
      add_key_slice();
      compute_slice();
   }

   if (midlo != midhi) {
      ikey = set_current_key_slice(midlo);
      compute_area_ratio(&ratio, &ytemp);
      if (ratio >= tolerance) {
         set_temp_key_slice_y(ytemp);
         add_key_slice();
         compute_slice();
      }
   }


   set_key_slice_temp_flag(0);
   set_symmetry_flag(temp_sym_flag);
}


/*
 * merge_close_keys
 *
 * merge key slice pairs that are too close together... do this until all pairs
 * of adjacent key slices are far enough apart...
 */
void VorSlice::merge_close_keys()
{
   const int temp_sym_flag = get_symmetry_flag();
   int did_merge, icenter, i, nkey;


   set_symmetry_flag(0);
   do {
      did_merge = 0;
      icenter = find_centermost_key_slice();
      for (i = icenter; i > 0; i --) {
	 if (small_key_slice_gap(i,i-1)) {
	    if (i == 1)
	       merge_key_slices(i-1,i);
	    else
	       merge_key_slices(i,i-1);
	    i --;
	    did_merge = 1;
	 }
      }
      nkey = get_num_key_slices();
      icenter = find_centermost_key_slice();
      for (i = icenter; i < nkey - 1; i ++) {
	 if (small_key_slice_gap(i,i+1)) {
	    if (i == nkey - 2)
	       merge_key_slices(i+1,i);
	    else
	       merge_key_slices(i,i+1);
	    did_merge = 1;
            nkey = get_num_key_slices();
	 }
      }
      compute_slice();
   } while (did_merge);
   set_symmetry_flag(temp_sym_flag);

}


/*
 * auto_find
 *
 * find places where key slices should go (discontinuities in outline of
 * geometry) and insert them.  this will hopefully give user a reasonable 
 * starting point...
 */
void VorSlice::auto_find()
{

   int new_keys;
   int temp_sym_flag = get_symmetry_flag();

   reset_key_slice();

   zero_camber();
   set_symmetry_flag(1);
   set_nslice(555);

   compute_slice();

   build_trap_face_array();

   new_keys = find_key_slices();

   free_trap_face_array();
   unzero_camber();

   if (!new_keys)
      set_nslice(2); 
   
   compute_slice();

   set_symmetry_flag(temp_sym_flag);
}


/*
 * auto_eliminate
 *
 * eliminates key slices that do not have a big impact on area...
 */
void VorSlice::auto_eliminate(void)
{
      zero_camber();
      eliminate_superfluous_keys();
      unzero_camber();
}

/*
 * auto_merge
 *
 * merges key slices that are too close together...
 */
void VorSlice::auto_merge(void)
{
      zero_camber();
      merge_close_keys();
      unzero_camber();
}

/*
 * auto_slice
 *
 * sets number of slices between key slices automatically...
 */
void VorSlice::auto_slice(void)
{
   const int ntotal = 40; /* target slice total for entire geometry */
   const int nkey = get_num_key_slices();
   const int temp_sym = get_symmetry_flag();
   int i;
   float ymin, ymax, yrange;

   set_symmetry_flag(0);
   set_current_key_slice(0);
   ymin = get_current_key_slice_y();
   set_current_key_slice(nkey-1);
   ymax = get_current_key_slice_y();
   yrange = ymax - ymin;
   set_current_key_slice(0);
   for (i = 0; i < nkey - 1; i ++) {
      float ylow = get_current_key_slice_y();
      float yhigh, yspan;
      int nslc;

      up_current_key_slice();
      yhigh = get_current_key_slice_y();
      down_current_key_slice();
      yspan = yhigh - ylow;
      nslc = (int)nearest_int((float)ntotal * yspan / yrange);
      set_nslice(nslc);
      up_current_key_slice();
   }
   set_symmetry_flag(temp_sym);
}

void VorSlice::zero_camber(void)
{
  saved_ncamber = Vor->ncamb;
  Vor->ncamb = 0;
}

void VorSlice::unzero_camber(void)
{
  Vor->ncamb = saved_ncamber;
}



//===== Utility ====//
float VorSlice::fabsf( float x )
{
	return (float)fabs(x);
}

float VorSlice::sqrtf( float x )
{
	return (float)sqrt(x);
}

float VorSlice::ceilf( float x )
{
	return (float)ceil(x);
}

float VorSlice::floorf( float x )
{
	return (float)floor(x);
}

int VorSlice::isstrdigit(char* in_string)
{
   int num_char, i;
   char dum[255];
   
   num_char = strlen(in_string);

   for (i = 0 ; i < num_char ; i++)
     {
       if (!isdigit(in_string[i]))
         {
         
           dum[0] = in_string[i];
                     
           if(strcmp(dum,".")!=0) {
           
              if(strcmp(dum,"-")!=0) {

                if(strcmp(dum,"+")!=0) {
              
                    return(0);
                    
                }
              
              }
              
           }
           
         }
     }
   return(1);
}
 
void VorSlice::info_mess_(char* message, int* pause_time)
 {
   int p_time;

   p_time = *pause_time;
   info_message(message, p_time);
 }

 
void VorSlice::time_keep_(float* fract_done)
 {
   float dum;

   dum = *fract_done;
//jrg fix   time_keep(dum);
 }

  
float** VorSlice::falloc_2d(int d1, int d2)
{
  int i;
  float **temp;

  temp = (float **)calloc(d1, sizeof(float *));
 
  for ( i = 0 ; i < d1 ; i++)
    {
      temp[i] = (float *)calloc(d2, sizeof(float));
    }
  return(temp);
}

int** VorSlice::ialloc_2d(int d1, int d2)
{
  int i;
  int **temp;

  temp = (int **)calloc(d1, sizeof(int *));
 
  for ( i = 0 ; i < d1 ; i++)
    {
      temp[i] = (int *)calloc(d2, sizeof(int));
    }
  return(temp);
}

float*** VorSlice::falloc_3d(int d1,int d2,int d3)
{
  int i, j;
  float ***temp;

  temp = (float ***)calloc(d1, sizeof(float **));
 
  for ( i = 0 ; i < d1 ; i++)
    {
      temp[i] = (float **)calloc(d2, sizeof(float *));
      
      for ( j = 0 ; j < d2 ; j++)
        {
          temp[i][j] = (float *)calloc(d3, sizeof(float));
        }
      
    }
  return(temp);
}
void VorSlice::ffree_2d(float **farray, int d1)
{
  int i;

  for ( i = 0 ; i < d1 ; i++)
    {
      free(farray[i]);
    }
  free(farray);
}
void VorSlice::ifree_2d(int **iarray, int d1)
{
  int i;

  for ( i = 0 ; i < d1 ; i++)
    {
      free(iarray[i]);
    }
  free(iarray);
}
void VorSlice::ffree_3d(float*** farray, int d1, int d2)
{
  int i, j;

  for ( i = 0 ; i < d1 ; i++)
    {
      for ( j = 0 ; j < d2 ; j++)
        {
          free(farray[i][j]);
        }
      free(farray[i]);
      
    }

  free(farray);

}

void VorSlice::find_trap_y_minmax(float **trap,float *ymin,float *ymax)
{
   int ip;
   float ymintmp, ymaxtmp;

   ymintmp = trap[0][1];
   ymaxtmp = trap[0][1];
   for (ip = 1; ip < 4; ip ++) {
      if (trap[ip][1] < ymintmp)
	 ymintmp = trap[ip][1];
      else if (trap[ip][1] > ymaxtmp)
	 ymaxtmp = trap[ip][1];
   }
   *ymin = ymintmp;
   *ymax = ymaxtmp;
}

/*
 * angles_match
 *
 * lep
 *
 * this routine takes as arguments two vectors (v1 and v2) defined by an
 * origin (v1s and v2s) and an end (v1e and v2e).  It computes the dot product
 * of the two vectors and divides by their magnitudes.  The result is
 * compared with the argument cos_limit.  If the result is greater than or
 * equal to cos_limit then 1 is returned; otherwise 0 is returned.
 *
 * Basically, we are computing the cosine of the angle between two vectors
 * and returning 1 or 0 depending on a threshold...
 *
 */
int VorSlice::angles_match(const float *v1s, const float *v1e, const float *v2s, 
		 const float *v2e, const float cos_limit)
{
   int i;
   float v1[3], v2[3];
   float dot, mag1, mag2, result;

/* compute vectors */

   for (i = 0; i < 3; i ++) {
      v1[i] = v1e[i] - v1s[i];
      v2[i] = v2e[i] - v2s[i];
   }

/* compute dot product */

   dot = 0;
   for (i = 0; i < 3; i ++) {
      dot += v1[i] * v2[i];
   }

/* compute magnitudes */

   mag1 = 0;
   mag2 = 0;
   for (i = 0; i < 3; i ++) {
      mag1 += v1[i] * v1[i];
      mag2 += v2[i] * v2[i];
   }
   mag1 = sqrtf(mag1);
   mag2 = sqrtf(mag2);

/* compare result to cosine limit */

   result = dot / (mag1 * mag2);

   return( result >= cos_limit);
   
}


/*
 * ranges_overlap
 *
 * lep
 *
 * takes as args:
 *    - x1low: lower end of first range
 *    - x1high: higher end of first range
 *    - x2low: lower end of second range
 *    - x2high: higher end of second range
 * and compares the two ranges to see if the overlap at all.  If they do
 * overlap, then 1 is returned (otherwise 0 is returned).
 */
int VorSlice::ranges_overlap(float x1low, float x1high, float x2low, float x2high)
{
   float temp;

/* swap, if necessary, to obtain right ordering... */

   if (x1low > x1high) {
      temp = x1low;
      x1low = x1high;
      x1high = temp;
   }
   if (x2low > x2high) {
      temp = x2low;
      x2low = x2high;
      x2high = temp;
   }

/* now compare ranges... */

   if (x1low <= x2low && x2low <= x1high)
      return(1);
   else if (x1low <= x2high && x2high <= x1high)
      return(1);
   else if (x2low <= x1low && x1low <= x2high)
      return(1);
   else if (x2low <= x1high && x1high <= x2high)
      return(1);
   else
      return(0);
}

/*
 * nearest_int
 *
 * lep
 *
 * return the nearest integer (as a float) to the argument
 */
float VorSlice::nearest_int(float x)
{
   float c = ceilf(x), f = floorf(x);
   return( fabs(x-c) < fabs(x-f) ? c : f );
}


//=================================================//
//
//   Key_Slice 
//
//=================================================//
/*
 * init_key_slice...
 *
 * set key slices to default initial state (2 keys slices at either extremity
 * and 1 smack dab in the middle).  Compute span, etc...
 */
void VorSlice::init_key_slice()
{

   float **bbox;
   float spany;
   int icomp;

   initKeyFlag = 1;

   bbox = falloc_2d(Vor->ncomps, 6);

   /* ---- Find Bounding Box for each component --- */
   comp_bound_box(bbox);

   key_slice.xmin = key_slice.xmax = bbox[0][0];
   key_slice.ymin = key_slice.ymax = bbox[0][2];
   key_slice.zmin = key_slice.zmax = bbox[0][4];

   for (icomp = 0; icomp < Vor->ncomps; icomp ++) {
      if (bbox[icomp][0] < key_slice.xmin) key_slice.xmin = bbox[icomp][0];
      if (bbox[icomp][1] > key_slice.xmax) key_slice.xmax = bbox[icomp][1];
      if (bbox[icomp][2] < key_slice.ymin) key_slice.ymin = bbox[icomp][2];
      if (bbox[icomp][3] > key_slice.ymax) key_slice.ymax = bbox[icomp][3];
      if (bbox[icomp][4] < key_slice.zmin) key_slice.zmin = bbox[icomp][4];
      if (bbox[icomp][5] > key_slice.zmax) key_slice.zmax = bbox[icomp][5];
   }
   key_slice.zavg = (key_slice.zmin + key_slice.zmax) / 2.0f;
   spany = key_slice.ymax - key_slice.ymin;

   key_slice.ymin = key_slice.ymin + 0.25f * key_slice_tolerance * spany;
   key_slice.ymax = key_slice.ymax - 0.25f * key_slice_tolerance * spany;

   ffree_2d(bbox,     Vor->ncomps);

   key_slice.nkey = 3; /* default is 2 key slices */
   key_slice.current_key = 0;
   key_slice.nslice = (int*) malloc(sizeof(int) * 2);
   if (!key_slice.nslice) {
      fprintf(stderr,"unable to malloc key_slice.nslice\n");
      exit(1);
   }
   key_slice.nslice[0] = default_nslice; /* default # of slices */
   key_slice.nslice[1] = default_nslice; /* default # of slices */
   key_slice.y = (float*) malloc(sizeof(float) * 3);
   if (!key_slice.y) {
      fprintf(stderr,"unable to malloc key_slice.y\n");
      exit(1);
   }
   key_slice.sy = (float*) malloc(sizeof(float) * 3);
   if (!key_slice.sy) {
      fprintf(stderr,"unable to malloc key_slice.sy\n");
      exit(1);
   }
   key_slice.y[0] = key_slice.ymin;
   key_slice.y[2] = key_slice.ymax;
   key_slice.y[1] = (key_slice.y[0] + key_slice.y[2]) / 2.0f;

   key_slice.nkeypt = get_num_key_point_list(key_slice.nkey);
//   key_slice.keypt = alloc_key_point_list(key_slice.nkeypt);
   alloc_key_point_list(&key_slice.keypt, key_slice.nkeypt);

   key_slice.temp_flag = 0;

   key_slice.symmetry_flag = 1;

}

/* 
 * uninit_key_slice
 *
 * This func is used by reset_key_slice to free up memory allocated by
 * init_key_slice in preparation for calling init_key_slice again...
 */
void VorSlice::uninit_key_slice(void)
{
   int i;

   if ( !initKeyFlag )
	   return;

   if (key_slice.nslice)
      free(key_slice.nslice);
   key_slice.nslice = 0;

   if (key_slice.y)
      free(key_slice.y);
   key_slice.y = 0;

   if (key_slice.sy)
      free(key_slice.sy);
   key_slice.sy = 0;

   if (key_slice.nkeypt > 0 && key_slice.keypt) {
      for (i = 0; i < key_slice.nkeypt; i ++)
	 free_key_point_list(&(key_slice.keypt[i]));
      free(key_slice.keypt);
   }
   key_slice.nkeypt = 0;
   key_slice.keypt = 0;
}

/*
 * reset_key_slice
 *
 * reset key slices to default state (2 key slices at either extremity and
 * one key slice smack dab in the middle)...
 */
void VorSlice::reset_key_slice(void)
{
   int temp_sym_flag = get_symmetry_flag();
   uninit_key_slice();
   init_key_slice();
   set_symmetry_flag(temp_sym_flag);
}

/*
 * return number of key slices 
 */
int VorSlice::get_num_key_slices()
{
   return(key_slice.nkey);
}

/*
 * return y location of current key slice
 */
float VorSlice::get_current_key_slice_y()
{
   return(key_slice.y[key_slice.current_key]);
}

void  VorSlice::set_current_key_slice_y(int id, float y)
{
	float tol = key_slice_tolerance + 0.0001f;
	float min_y = key_slice.ymin;
	if ( id > 0 )
		min_y = key_slice.y[id-1] + tol;

	float max_y = key_slice.ymax; 
	if ( id < (key_slice.nkey - 1) )
		max_y = key_slice.y[id+1] - tol;

	if ( y < min_y ) y = min_y;
	if ( y > max_y ) y = max_y;

	key_slice.y[id] = y;
}

int VorSlice::get_current_nslices()
{
	int id = get_current_region();
	return key_slice.nslice[id];
}


/*
 * return total number of slices (not just key slices but all slices)
 */
int VorSlice::get_total_nslices()
{
   int i, nslice;

   nslice = 0;
   for (i = 0; i < key_slice.nkey - 1; i ++) {
      nslice += key_slice.nslice[i];
   }
   nslice = nslice - key_slice.nkey + 2;

   return(nslice);
}

/*
 * return index of current region based on index of current key slice...
 */
int VorSlice::get_current_region()
{
   if (key_slice.current_key == key_slice.nkey - 1)
      return(key_slice.current_key - 1);
   else
      return(key_slice.current_key);
}

/*
 * return index of region opposite (symmetry based on index) the current one
 */
int VorSlice::get_opposite_region()
{
   return(key_slice.nkey - get_current_region() - 2);
}


int VorSlice::get_opposite_keyslice()
{
  return(key_slice.nkey - key_slice.current_key - 1);
}
/* 
 * set the current key slice to one with specified index
 */
int VorSlice::set_current_key_slice(int ikey)
{
   if (0 <= ikey && ikey < key_slice.nkey)
      key_slice.current_key = ikey;
   else
      printf("set_current_key_slice: warning: bad argument ignored\n");
   return(key_slice.current_key);
}

/* 
 * set the current key slice to one with specified index
 */
int VorSlice::get_current_key_slice()
{
	return key_slice.current_key;
}

/*
 * set current key slice to next one (up in y; wraps around to lowest)
 */
int VorSlice::up_current_key_slice()
{
   int at_top = (key_slice.current_key == key_slice.nkey - 1);
   return(set_current_key_slice(at_top ? 0 : key_slice.current_key + 1));
}

/*
 * set current key slice to previous one (down in y; wraps around to highest)
 */
int VorSlice::down_current_key_slice()
{
   int at_bottom = (key_slice.current_key == 0);
   return(set_current_key_slice(at_bottom ? key_slice.nkey - 1 : 
					    key_slice.current_key - 1));
}

/*
 * set current key slice to lowest one (lowest in y)
 */
int VorSlice::reset_current_key_slice()
{
   return(set_current_key_slice(0));
}


void VorSlice::set_current_nslices(int n)
{
   int creg = get_current_region();
   int oreg = get_opposite_region();

   if (key_slice.symmetry_flag)
      key_slice.nslice[oreg] = n;
   
   key_slice.nslice[creg] = n;
}



/*
 * increment the number of slices in the current region
 */
int VorSlice::up_nslice()
{
   int creg = get_current_region();
   int oreg = get_opposite_region();

   if (key_slice.symmetry_flag && oreg != creg)
      ++ key_slice.nslice[oreg];
   return(++ key_slice.nslice[creg]);
}

/* 
 * decrement number of slices in current region
 */
int VorSlice::down_nslice()
{
   int creg = get_current_region();
   int oreg = get_opposite_region();

   if (key_slice.symmetry_flag && oreg != creg) {
      if (key_slice.nslice[oreg] > 2)
	 -- key_slice.nslice[oreg];
   }
   if (key_slice.nslice[creg] > 2)
      return(-- key_slice.nslice[creg]);
   else
      return(key_slice.nslice[creg]);
}

/*
 * set_temp_key_slice_y
 *
 * Takes as argument a y coordinate and sets the temp key slice to that
 * location...
 */
void VorSlice::set_temp_key_slice_y(float yval)
{
   if (key_slice.ymin <= yval && yval <= key_slice.ymax) {
      key_slice.temp_flag = 1;
      key_slice.ytemp = yval;
   }
   else {
      key_slice.temp_flag = 0;
   }
}


/* 
 * set_temp_key_slice
 *
 * Takes as argument a normalized y coordinate (between 0 and 1) and sets
 * the temp key slice to that location...
 */
void VorSlice::set_temp_key_slice(float ypercent)
{
   float yspan, ymin, ymax, yresult;

   if (0. <= ypercent && ypercent <= 1.) {
      ymin = key_slice.ymin;
      ymax = key_slice.ymax;
      yspan = ymax - ymin;
      yresult = ymin + ypercent * yspan;
      yresult = yresult > ymax ? ymax : yresult;
      yresult = yresult < ymin ? ymin : yresult;
      set_temp_key_slice_y(yresult);
   }
   else
      key_slice.temp_flag = 0;
}

float VorSlice::get_temp_key_slice_y()
{
	float ymin = key_slice.ymin;
	float ymax = key_slice.ymax;
	float yspan = ymax - ymin;

	if ( yspan > 0.0000001 )
	{
		return ( (key_slice.ytemp - ymin)/yspan );
	}

	return 0.0;
}

void VorSlice::adjust_temp_key_slice_y(float dir)
{
	float ymin = key_slice.ymin;
	float ymax = key_slice.ymax;
	float yspan = ymax - ymin;
 
	float yresult = key_slice.ytemp + 0.0005f*dir*yspan;
	yresult = yresult > ymax ? ymax : yresult;
	yresult = yresult < ymin ? ymin : yresult;
	set_temp_key_slice_y(yresult);
}

/*
 * key_slice_exists
 *
 * returns 1 if the temp key slice is already in the permanent list; else
 * returns 0...
 *
 * assumes that key slice stuff has been initialized, and that we have a
 * valid temp key slice...
 */
int VorSlice::key_slice_exists()
{
   float ytol;
   int ikey;

   ytol = key_slice_tolerance * (key_slice.ymax - key_slice.ymin);
   for (ikey = 0; ikey < key_slice.nkey; ikey ++) {
      if (fabsf(key_slice.y[ikey] - key_slice.ytemp) < ytol) {
         return(1);
      }
   }
   return(0);
}

void VorSlice::set_close_key_slice_y( float y_val )
{
   float ytol;
   int ikey;

   ytol = key_slice_tolerance * (key_slice.ymax - key_slice.ymin);
   for (ikey = 0; ikey < key_slice.nkey; ikey ++) 
   {
      if (fabsf(key_slice.y[ikey] - y_val) < ytol) 
	  {
         key_slice.y[ikey] = y_val;
      }
   }

}



/*
 * add_key_slice
 *
 * adds the temp key slice to the permanent list...
 */
void VorSlice::add_key_slice()
{
   int index, ikey;

/* turn off flag so temp slice isn't drawn anymore... */

   key_slice.temp_flag = 0;

/* make sure temp key slice isn't already in permanent list... */
   if (key_slice_exists()) {
	   set_close_key_slice_y( key_slice.ytemp );
	   if ( key_slice.symmetry_flag )
	   {
			float opp_y = -key_slice.ytemp;
			set_close_key_slice_y( opp_y );
	   }
      return; 
   }

/* figure out where in list new key slice should go... */

   index = key_slice.nkey;
   for (ikey = 0; ikey < key_slice.nkey && index == key_slice.nkey; ikey ++) {
      if (key_slice.ytemp < key_slice.y[ikey]) {
	 index = ikey;
      }
   }

/* make key slice list bigger... */

   key_slice.y = (float*)realloc(key_slice.y,sizeof(float) * (key_slice.nkey + 1));
   if (! key_slice.y) {
      fprintf(stderr,"unable to realloc key_slice.y array\n");
      exit(-1);
   }
   key_slice.sy = (float*)realloc(key_slice.sy,sizeof(float) * (key_slice.nkey + 1));
   if (! key_slice.sy) {
      fprintf(stderr,"unable to realloc key_slice.sy array\n");
      exit(-1);
   }
   key_slice.nslice = (int*)realloc(key_slice.nslice,sizeof(int) * 
			       (key_slice.nkey));
   if (! key_slice.nslice) {
      fprintf(stderr,"unable to realloc key_slice.nslice array\n");
      exit(-1);
   }
   key_slice.keypt = realloc_key_point_list(key_slice.keypt,key_slice.nkeypt,
			       get_num_key_point_list(key_slice.nkey+1),
			       get_key_point_list(index) - 
			       (index == key_slice.nkey ? 1 : 0));
   key_slice.nkeypt = get_num_key_point_list(key_slice.nkey+1);

/* insert new key slice in proper position... shift existing key slices as
   necessary... */

   if (index == key_slice.nkey) { /* insert after last */
      key_slice.y[key_slice.nkey]  = key_slice.ytemp;
      key_slice.sy[key_slice.nkey] = key_slice.sytemp;
      key_slice.nslice[key_slice.nkey - 1] = default_nslice;
   }
   else {
      for (ikey = key_slice.nkey - 1; ikey >= index; ikey --) {
	 key_slice.y[ikey + 1] = key_slice.y[ikey];
	 key_slice.sy[ikey + 1] = key_slice.sy[ikey];
	 if (ikey < key_slice.nkey - 1)
	    key_slice.nslice[ikey + 1] = key_slice.nslice[ikey];
      }
      key_slice.y[index] = key_slice.ytemp;
      key_slice.sy[index] = key_slice.sytemp;
      key_slice.nslice[index] = default_nslice;
      if (index > 0)
	 key_slice.nslice[index-1] = default_nslice;
   }

/* increment number of key slices... */

   key_slice.nkey ++;

/* make newest key slice the current one */

   key_slice.current_key = index;

/* if symmetry exists then copy new key slice to opposite side of model... */
   if (key_slice.symmetry_flag)
      copy_key_slice();
}

/*
 * add_middle_key_slice
 *
 * adds a key slice right in the middle...
 */
void VorSlice::add_middle_key_slice(void)
{
   int temp_flag = key_slice.symmetry_flag;
   key_slice.symmetry_flag = 0;
   set_temp_key_slice(0.5);
   add_key_slice();
   key_slice.symmetry_flag = temp_flag;
}

/*
 *
 * _delete_key_slice_
 *
 * delete the current key slice... if allow_end_deletion is non-zero then
 * it is ok to delete an end key slice...
 */
void VorSlice::_delete_key_slice_(int allow_end_deletion)
{
   int ikey, opposite_key;
   static int depth = 0;

   depth ++;
   if ((key_slice.current_key == 0 || 
       key_slice.current_key == key_slice.nkey - 1) && !allow_end_deletion) {
      info_message("Cannot delete current slice!",4);
   }
   else {
      for (ikey = key_slice.current_key; ikey < key_slice.nkey - 1; ikey ++) {
	 key_slice.y[ikey] = key_slice.y[ikey + 1];
	 key_slice.sy[ikey] = key_slice.sy[ikey + 1];
	 if (ikey < key_slice.nkey - 2)
	    key_slice.nslice[ikey] = key_slice.nslice[ikey + 1];
      }
      key_slice.nslice[key_slice.current_key ? key_slice.current_key - 1 : 0] =
						 default_nslice;
      key_slice.keypt = realloc_key_point_list(key_slice.keypt,key_slice.nkeypt,
			  get_num_key_point_list(key_slice.nkey - 1),
			  get_key_point_list(key_slice.current_key) -
			 (key_slice.current_key == key_slice.nkey - 1 ? 1 : 0));
      key_slice.nkeypt = get_num_key_point_list(key_slice.nkey - 1);
      key_slice.nkey --;
      if (depth == 1) {
         opposite_key = key_slice.nkey - key_slice.current_key;
         if (key_slice.symmetry_flag && opposite_key != key_slice.current_key) {
            if (opposite_key > key_slice.current_key)
	       opposite_key --;
	    key_slice.current_key = opposite_key;
	    _delete_key_slice_(allow_end_deletion);
         }
	 if (key_slice.current_key)
	    key_slice.current_key --;
      }
   }
   depth --;
}

/*
 * delete_key_slice
 *
 * delete the current key slice, but not if it is one of the end keys...
 */
void VorSlice::delete_key_slice()
{
   _delete_key_slice_(0);
}

/*
 * delete_key_slice_end
 *
 * delete the current key slice, even if it is one of the end keys...
 */
void VorSlice::delete_key_slice_end()
{
   _delete_key_slice_(1);
}

/*
 * copy_key_slice
 *
 * make a copy of the current key slice on the opposite side of symmetry
 * plane...
 */
void VorSlice::copy_key_slice()
{
   float spany, ypercent, ymin;
   static int call_count = 0; /* prevents this routine from being called via
				 indirect recursion */

   if (call_count == 0) {
      call_count = 1;
      ymin = key_slice.ymin;
      spany = key_slice.ymax - ymin;
      ypercent = 1.0f - (key_slice.y[key_slice.current_key] - ymin) / spany;
      set_temp_key_slice(ypercent);
      add_key_slice();
   }
   call_count = 0;
}

/*
 * set number of slices in current region using argument
 */
void VorSlice::set_nslice(int new_nslice)
{
   if (new_nslice >= 2) {
      key_slice.nslice[get_current_region()] = new_nslice;
      if (key_slice.symmetry_flag)
         key_slice.nslice[get_opposite_region()] = new_nslice;
   }
}

/*
 * set the symmetry flag (the flag which determines whether user operations
 * affect one side or both sides of geometry; symmetry based on index) as
 * per argument.
 */
int VorSlice::set_symmetry_flag(int flag_value)
{
   return(key_slice.symmetry_flag = flag_value);
}

/*
 * return current value of symmetry flag (non-zero = on, zero = off)
 */
int VorSlice::get_symmetry_flag()
{
   return(key_slice.symmetry_flag);
}

/*
 * toggle the symmetry flag
 */
int VorSlice::toggle_symmetry_flag()
{
   return(set_symmetry_flag( ! key_slice.symmetry_flag));
}

/*
 * read key slices from a .key (by convention) file.  Info in files includes
 * number of key slices, location of key slices, and number of slices in each
 * region...
 */
int VorSlice::read_key_slice(char *basename)
{
   static const char *ext = ".key";
   char filename[255];
   FILE *fp;
   int temp_nkey;
   float *temp_y;
   float *temp_sy;
   int *temp_nslice;
   int ikey;
   int islice;

   init_key_slice();

   strcpy(filename,basename);
   strcat(filename,ext);

   fp = fopen(filename,"r");
   if (fp == NULL)
      return(1);

   fscanf(fp,"%d",&temp_nkey);
   if (feof(fp) || ferror(fp)) {
      fclose(fp);
      return(1);
   }
   else if (temp_nkey < 2) {
      fclose(fp);
      return(1);
   }

   temp_y = (float*) malloc(sizeof(float) * temp_nkey);
   temp_sy = (float*) malloc(sizeof(float) * temp_nkey);
   temp_nslice = (int*) malloc(sizeof(int) * (temp_nkey - 1));
   if (!temp_y || !temp_sy || !temp_nslice) {
      fprintf(stderr,"unable to malloc, exiting...\n");
      exit(1);
   }

   for (ikey = 0; ikey < temp_nkey; ikey ++) {
      fscanf(fp,"%g",&temp_y[ikey]);
      if (feof(fp) || ferror(fp)) {
	 free(temp_y);
	 free(temp_nslice);
         fclose(fp);
         return(1);
      }
/* check to make sure all the slices are in range... */
      else if (temp_y[ikey] < key_slice.ymin) {
	 if (ikey == 0)
	    temp_y[ikey] = key_slice.ymin;
	 else {
	    free(temp_y);
	    free(temp_nslice);
	    fclose(fp);
	    return(1);
	 }
      }
      else if (temp_y[ikey] > key_slice.ymax) {
	 if (ikey == temp_nkey - 1)
	    temp_y[ikey] = key_slice.ymax;
	 else {
	    free(temp_y);
	    free(temp_nslice);
	    fclose(fp);
	    return(1);
	 }
      }
   }

   for (islice = 0; islice < temp_nkey - 1; islice ++) {
      fscanf(fp,"%d",&temp_nslice[islice]);
      if (feof(fp) || ferror(fp)) {
	 free(temp_y);
	 free(temp_nslice);
         fclose(fp);
         return(1);
      }
      else if (temp_nslice[islice] < 2) {
	 free(temp_y);
	 free(temp_nslice);
         fclose(fp);
         return(1);
      }
   }


   free(key_slice.y);
   free(key_slice.sy);
   free(key_slice.nslice);
   key_slice.y = temp_y;
   key_slice.sy = temp_sy;
   key_slice.nslice = temp_nslice;
   key_slice.nkey = temp_nkey;

   free_key_point_list_array();
   key_slice.nkeypt = get_num_key_point_list(key_slice.nkey);
   //key_slice.keypt = alloc_key_point_list(key_slice.nkeypt);
   alloc_key_point_list(&key_slice.keypt, key_slice.nkeypt);


   fclose(fp);
   return(0);
}


/*
 * write key slices to .key file (see read_key_slice)
 */
int VorSlice::write_key_slice(char *basename)
{
   static const char *ext = ".key";
   char filename[255];
   FILE *fp;
   int ikey;
   int islice;

   strcpy(filename,basename);
   strcat(filename,ext);

   fp = fopen(filename,"w");
   if (fp == NULL)
      return(1);

   fprintf(fp,"%d\n",key_slice.nkey);
   if (ferror(fp)) {
      fclose(fp);
      return(1);
   }

   fprintf(fp,"\n");
   for (ikey = 0; ikey < key_slice.nkey; ikey ++) {
      fprintf(fp,"  %g\n",key_slice.y[ikey]);
      if (ferror(fp)) {
         fclose(fp);
         return(1);
      }
   }

   fprintf(fp,"\n");
   for (islice = 0; islice < key_slice.nkey - 1; islice ++) {
      fprintf(fp,"  %d\n",key_slice.nslice[islice]);
      if (ferror(fp)) {
         fclose(fp);
         return(1);
      }
   }

   fclose(fp);
   return(0);
}

/*
 * get_key_point_list
 *
 * returns the index of the (low) key point list.  High key point list is
 * one higher than what is returned.  In the case of an end key slice, low and
 * high are the same...
 */
int VorSlice::get_key_point_list(int slice)
{
   int low = 2 * slice - 1;
   if (low < 0)
      low = 0;
   return(low);
}

/*
 * get_num_key_point_list
 *
 * returns the number of key point lists (based on the number of key slices,
 * which is passed in)...
 *
 * NOTE: only gives correct results for nslice >= 2...
 */
int VorSlice::get_num_key_point_list(int nslice)
{
   return(nslice * 2 - 2);
}

/*
 * find_key_points
 *
 * searches thru the trapezoids for points that lie on key slices; these
 * points are added to the key points lists...
 */
void VorSlice::find_key_points()
{
   int islc, it, ip, low;
   float ymin, ymax, yslice;

/* need to use delete flag here */

   for (it = 0; it < Vor->ntraps; it ++) {
      find_trap_y_minmax(Vor->trap[it],&ymin,&ymax);
      for (islc= 0; islc < key_slice.nkey; islc ++) {
	 yslice = key_slice.y[islc];
	 for (ip = 0; ip < 4; ip ++) {
	    if (yslice == Vor->trap[it][ip][1]) {
	       low = get_key_point_list(islc);
	       if (yslice == ymax || low == 0)
	          insert_key_point(&key_slice.keypt[low],it,ip);
	       else
	          insert_key_point(&key_slice.keypt[low+1],it,ip);
	    }
	 }
      }
   }
}


/*
 * key_point_index
 *
 * given an x,y,z point (1st arg), this func looks thru the key point lists
 * and tries to find that point.  If it finds the point, it returns 1 and
 * passes back the list and element numbers (in cases where the point exists
 * in 2 adjacent lists, the list index and element index for each are passed
 * back; if the point exists in only one list then we pass back 
 * ilist == ilist2 and ielement == ielement2).  If it doesn't find the point 
 * it returns 0...
 *
 */
int VorSlice::key_point_index(float xyz[3],int *ilist,int *ielement,int *ilist2,
				            int *ielement2)
{
   int i;

   for (i = 0; i < key_slice.nkeypt; i ++) {
      if (Key_point_index(xyz,&(key_slice.keypt[i]),ielement)) {
	 *ilist = i;
	 if (i < key_slice.nkeypt - 1 && i % 2 == 1) {
	    if (Key_point_index(xyz,&(key_slice.keypt[i+1]),ielement2)) {
	       *ilist2 = i + 1;
	    }
	    else {
	       *ilist2 = i;
	       *ielement2 = *ielement;
	    }
	 }
	 else {
	    *ilist2 = i;
	    *ielement2 = *ielement;
	 }
	 return(1);
      }
   }
   return(0);
}


/*
 * is_key_point
 *
 * this func takes as argument an x,y,z point.  It checks the key point lists to
 * see if that point is in any of the key point lists
 */
int VorSlice::is_key_point(float xyz[3])
{
   int ilist, ielement, ilist2, ielement2;

   return (key_point_index(xyz,&ilist,&ielement,&ilist2,&ielement2));
}

/*
 * set_current_key_point
 *
 * sets the current key point (the point user may relocate)...
 */
void VorSlice::set_current_key_point(int ilist,int ielement)
{
   if (0 <= ilist && ilist < key_slice.nkeypt) {
      key_slice.current_keypt = ilist;
      Set_current_key_point(&(key_slice.keypt[ilist]),ielement);
   }
   else {
      info_message("Fatal error in set_current_key_point - bad list index!",4);
      exit(-1);
   }
}

/*
 * move_current_key_point
 *
 * moves the current key point to the specified xyz location...
 */
void VorSlice::move_current_key_point(float xyz[3])
{
   if (0 <= key_slice.current_keypt && 
	    key_slice.current_keypt < key_slice.nkeypt) {
      Move_current_key_point(&(key_slice.keypt[key_slice.current_keypt]),xyz);
   }
   else {
      info_message("Fatal error in move_current_key_point - bad current #!",4);
      exit(-1);
   }
}

/*
 * move_opposite_key_point
 *
 * moves the key point that is opposite the current one to the specified x
 * location (y and z stay the same)...
 */
void VorSlice::move_opposite_key_point(float x)
{
   if (0 <= key_slice.current_keypt && 
	    key_slice.current_keypt < key_slice.nkeypt) {
      int okeypt = key_slice.nkeypt - key_slice.current_keypt - 1;
      Move_opposite_key_point(&(key_slice.keypt[key_slice.current_keypt]),
			      &(key_slice.keypt[okeypt]),x);
   }
   else {
      info_message("Fatal error in move_opposite_key_point - bad current #!",4);
      exit(-1);
   }
}

/*
 * free_key_point_list_array
 *
 * free the key point list array, set the size to 0 and current element to -1...
 */
void VorSlice::free_key_point_list_array()
{
   int i;

   if (key_slice.keypt) {
      for (i = 0; i < key_slice.nkeypt; i ++) {
	 free_key_point_list(&key_slice.keypt[i]);
      }
      key_slice.keypt = 0;
   }
   key_slice.nkeypt = 0;
   key_slice.current_keypt = -1;
}

/*
 * reset_current_key_point
 *
 * reset current key point to its original location...
 */
void VorSlice::reset_current_key_point()
{
   int ckp = key_slice.current_keypt;
   int nkp = key_slice.nkeypt;

   if (0 <= ckp && ckp < nkp) {
       Reset_current_key_point(&(key_slice.keypt[ckp]));
       if (key_slice.symmetry_flag) {
          int okeypt = nkp - ckp - 1;
          Reset_opposite_key_point(&(key_slice.keypt[ckp]),
                                   &(key_slice.keypt[okeypt]));
       }
   }
   else {
      info_message("Reset ignored - no current key point.",4);
   }
}


/*
 * get_key_point_trap
 *
 * this func takes as args:
 *    - ilist: the index of a keypoint list
 *    - ielement: the index of an element in keypoint list
 * and returns the trapezoid index corresponding to the given key point...
 */
int VorSlice::get_key_point_trap(int ilist,int ielement)
{
   if (0 <= ilist && ilist < key_slice.nkeypt) {
      return(Get_key_point_trap(&(key_slice.keypt[ilist]),ielement));
   }
   else {
      char str[255];
      sprintf(str,"Error in get_key_point_trap: bad ilist value (%d)",ilist);
      info_message(str,4);
      exit(1);
   }
}

/*
 * locate_key_point_by_trap
 *
 * this func takes as args:
 *    - ilist: the index of a keypoint list
 *    - itrap: index of a trapezoid
 * this func searches thru the specified keypoint list for a keypoint that
 * belongs to (i.e. is a corner point of) the specified trapezoid.
 * The index of the first key point found is returned (if none is found then
 * -1 is returned).
 */
int VorSlice::locate_key_point_by_trap(int ilist, int itrap)
{
   if (0 > ilist || ilist >= key_slice.nkeypt) {
      char str[255];
      sprintf(str,"Error in locate_key_point_by_trap: bad ilist value (%d)",ilist);
      info_message(str,4);
      exit(1);
   }
   else if (0 > itrap || itrap >= Vor->ntraps) {
      char str[255];
      sprintf(str,"Error in locate_key_point_by_trap: bad itrap value (%d)",itrap);
      info_message(str,4);
      exit(1);
   }
   else {
      return(Locate_key_point_by_trap(&(key_slice.keypt[ilist]),itrap));
   }
}

/*
 * locate_key_point_pair
 *
 * this func takes as args:
 *    - ilist: index of key point list
 *    - ielement_start: index into key point list where search should begin
 * this func searches for a pair of keypoints that belong to the same
 * trapezoid.  it returns the lowest index (or -1 if no pair found).
 */
int VorSlice::locate_key_point_pair(int ilist, int ielement_start)
{
   if (0 <= ilist && ilist < key_slice.nkeypt) {
      return(Locate_key_point_pair(&(key_slice.keypt[ilist]),ielement_start));
   }
   else {
      char str[255];
      sprintf(str,"Error in locate_key_point_pair: bad ilist value (%d)",ilist);
      info_message(str,4);
      exit(1);
   }
}

/*
 * num_key_points
 *
 * This func takes as arg the index of a key point list.  It returns the
 * number of key points in that key point list.
 */
int VorSlice::num_key_points(int ilist)
{
   if (0 <= ilist && ilist < key_slice.nkeypt)
      return(Num_key_points(&(key_slice.keypt[ilist])));
   else {
      char str[255];
      sprintf(str,"Error in num_key_points: bad ilist value (%d)",ilist);
      info_message(str,4);
      exit(1);
   }
}

/*
 * get_key_point_new_xyz
 *
 * this func takes as args:
 *    - ilist: index of a key point list
 *    - ielement: index into key point list
 * and passes back the "new" (i.e. possibly adjusted by user or program) 
 * {x,y,z} location of the specified key point.
 *
 */
void VorSlice::get_key_point_new_xyz(int ilist, int ielement, 
			   float* x, float* y, float* z)
{
   if (0 <= ilist && ilist < key_slice.nkeypt) {
      Get_key_point_new_xyz(&(key_slice.keypt[ilist]),ielement, x, y, z);
   }
   else {
      char str[255];
      sprintf(str,"Error in get_key_point_new_xyz: bad ilist value (%d)",ilist);
      info_message(str,4);
      exit(1);
   }
}

/*
 * set_key_point_new_xyz
 *
 * this func takes as args:
 *    - ilist: index of a key point list
 *    - ielement: index into key point list
 *    - x,y,z: an {x,y,z} coordinate
 * and sets the "new" (i.e. possibly adjusted by user or program) 
 * {x,y,z} location of the specified key point.
 *
 */
void VorSlice::set_key_point_new_xyz(int ilist, int ielement, 
			   float x, float y, float z)
{
   if (0 <= ilist && ilist < key_slice.nkeypt) {
      Set_key_point_new_xyz(&(key_slice.keypt[ilist]),ielement, x, y, z);
   }
   else {
      char str[255];
      sprintf(str,"Error in set_key_point_new_xyz: bad ilist value (%d)",ilist);
      info_message(str,4);
      exit(1);
   }
}

/*
 * read_key_point
 *
 * read key point data from .kpt file.  The base file name is passed in.
 * Status is returned (0=ok)...
 *
 * Note: what is read in is the delta-x for each key point.  This delta is
 * added to the x in the original point to produce a new point...
 */
int VorSlice::read_key_point(char *basename)
{
   static char *ext = ".kpt";
   char filename[255];
   FILE *fp;
   int nlist, i;

   strcpy(filename,basename);
   strcat(filename,ext);

   fp = fopen(filename,"r");
   if (fp == NULL)
      return(1);

   fscanf(fp,"%d",&nlist);
   if (feof(fp) || ferror(fp)) {
      fclose(fp);
      return(1);
   }
   else if (nlist != key_slice.nkeypt) {
      fclose(fp);
      return(1);
   }

   for (i = 0; i < nlist; i ++) {
      if (Read_key_point(fp,&(key_slice.keypt[i]))) {
	 fclose(fp);
	 return(1);
      }
   }

   fclose(fp);
   return(0);
}


/*
 * write_key_point
 *
 * write key point data to .kpt file.  The base file name is passed in.
 * Status is returned (0=ok)...
 *
 * Note: what is written is the delta-x for each key point.  This delta is
 * x difference between the original point and the new (modified) point...
 */
int VorSlice::write_key_point(char *basename)
{
   static char *ext = ".kpt";
   char filename[255];
   FILE *fp;
   int nlist, i;

   strcpy(filename,basename);
   strcat(filename,ext);

   fp = fopen(filename,"w");
   if (fp == NULL)
      return(1);

   nlist = key_slice.nkeypt;
   fprintf(fp,"%d\n\n",nlist);
   if (ferror(fp)) {
      fclose(fp);
      return(1);
   }

   for (i = 0; i < nlist; i ++) {
      int low_list = i % 2;
      if (Write_key_point(fp,&(key_slice.keypt[i]),low_list)) {
	 fclose(fp);
	 return(1);
      }
   }

   fclose(fp);
   return(0);
}


/*
 * get_key_slice_temp_flag
 *
 * get temp flag value from key_slice struct...
 */
int VorSlice::get_key_slice_temp_flag()
{
   return(key_slice.temp_flag);
}

/*
 * set_key_slice_temp_flag
 *
 * set temp flag in key_slice struct; return value of flag...
 */
int VorSlice::set_key_slice_temp_flag(int flag)
{
   return(key_slice.temp_flag = flag);
}

/*
 * find_centermost_key_slice
 *
 * find the key slice that is closest to the (y) center of geometry.  Return
 * the index of that key slice...
 */
int VorSlice::find_centermost_key_slice()
{
   int i, nk, imiddle, ibest;
   float center, dist, newdist;

   center = (key_slice.ymin + key_slice.ymax) / 2.0f;
   nk = key_slice.nkey;
   imiddle = nk / 2;
   ibest = imiddle;
   dist = fabsf(key_slice.y[ibest] - center);
   newdist = dist;
   if (ibest < nk - 1)
      for (i = ibest + 1; i < nk && newdist == dist; i ++) {
	 newdist = fabsf(key_slice.y[i] - center);
	 if (newdist < dist) {
	    ibest = i;
	    dist = newdist;
	 }
      }
   if (ibest == imiddle) /* need to try the other way... */
      if (ibest > 0) {
	 newdist = dist;
	 for (i = ibest - 1; i >= 0 && newdist == dist; i --) {
	    newdist = fabsf(key_slice.y[i] - center);
	    if (newdist < dist) {
	       ibest = i;
	       dist = newdist;
	    }
	 }
      }
   return(ibest);
}

/*
 * small_key_slice_gap
 *
 * takes as args the indices of two key slices; it measures the (y) gap between
 * them and returns 1 if the gap is small (otherwise returns 0).
 */
int VorSlice::small_key_slice_gap(int ikey1, int ikey2)
{
   const float tolerance = key_slice_tolerance * 10.0f; /* 0.06 ok for f16 */
   const float small_gap = (key_slice.ymax - key_slice.ymin) * tolerance;

   if (ikey1 < 0 || ikey1 >= key_slice.nkey || ikey2 < 0 
		 || ikey2 >= key_slice.nkey) {
      fprintf(stderr,"Error: bad arg(s) to small_key_slice_gap...\n");
      return(0);
   }
   return (fabsf(key_slice.y[ikey1] - key_slice.y[ikey2]) <= small_gap);
}

/* 
 * merge_key_slices
 *
 * this func takes as args the indices of two key slices.  The x locations of
 * key points in the "other" key slice replace the x locations of corresponding 
 * key points in the "target" key slice.  The "other" key slice is then deleted.
 * Note that the key slices should be adjacent... also, the "other" key slice
 * should not be an end key slice...
 *
 */
void VorSlice::merge_key_slices(int target, int other)
{
   int tkplist, okplist1, okplist2, tn, on, tkp, ttrap, okp1, nfound, okp2;
   int okp2_save;
   float x1l, y1l, z1l, x1r, y1r, z1r, x2l, y2l, z2l, x2r, y2r, z2r;
   float xkeep, ykeep, zkeep, xtoss, ytoss, ztoss;

   if (target < 0 || target >= key_slice.nkey || other < 0 || 
       other >= key_slice.nkey) {
      info_message("Error in merge_key_slices: bad target or other arg\n",4);
      exit(-1);
   }
   else if (target == other + 1 || target == other - 1) {
      if (other == 0 || other == key_slice.nkey - 1) {
	 info_message("Error in merge_key_slices: attempt to eliminate end slice.\n",4);
	 exit(-1);
      }
      if (target == other + 1) {
         tkplist = get_key_point_list(target);
         okplist2 = get_key_point_list(other);
         okplist1 = okplist2 + 1;
      }
      else {
/*         tkplist = get_key_point_list(target) + 1; */
         tkplist = get_key_point_list(target) + (target == 0 ? 0 : 1);
         okplist1 = get_key_point_list(other);
         okplist2 = okplist1 + 1;
      }
      tn = num_key_points(tkplist);
      on = num_key_points(okplist2);
      tkp = 0;
      while (0 <= tkp && tkp < tn - 1) {
	 tkp = locate_key_point_pair(tkplist,tkp);
	 if (0 <= tkp) {
	    ttrap = get_key_point_trap(tkplist,tkp);
	    if (0 <= (okp1 = locate_key_point_by_trap(okplist1,ttrap))) {
	       if (okp1 == locate_key_point_pair(okplist1,okp1)) {
		  get_key_point_new_xyz(okplist1,okp1,&x1l,&y1l,&z1l);
		  get_key_point_new_xyz(okplist1,okp1+1,&x1r,&y1r,&z1r);
		  nfound = 0;
		  okp2 = 0;
		  while (0 <= okp2 && okp2 < on - 1 && nfound < 2) {
		     okp2 = locate_key_point_pair(okplist2,okp2);
		     if (0 <= okp2) {
			get_key_point_new_xyz(okplist2,okp2,&x2l,&y2l,&z2l);
			get_key_point_new_xyz(okplist2,okp2+1,&x2r,&y2r,&z2r);
			if (ranges_overlap(x1l,x1r,x2l,x2r)) {
			   nfound ++;
			   okp2_save = okp2;
			}
			okp2 += 2;
		     }
		  }
		  if (nfound == 1) {
		     get_key_point_new_xyz(tkplist,tkp,&xtoss,&ykeep,&zkeep);
		     get_key_point_new_xyz(okplist2,okp2_save,&xkeep,&ytoss,
							      &ztoss);
		     set_key_point_new_xyz(tkplist,tkp,xkeep,ykeep,zkeep);
		     get_key_point_new_xyz(tkplist,tkp+1,&xtoss,&ykeep,&zkeep);
		     get_key_point_new_xyz(okplist2,okp2_save+1,&xkeep,&ytoss,
								&ztoss);
		     set_key_point_new_xyz(tkplist,tkp+1,xkeep,ykeep,zkeep);
		  }
	       }
	    }
	    tkp += 2;
	 }
      }
      set_current_key_slice(other);
      delete_key_slice();
   }
   else {
      info_message("Error in merge_key_slices: target and other not adjacent.\n",4);
      exit(-1);
   }
}

/*
 * get_key_slice_percent_bounds
 *
 * This func passes back the allowable range of locations (represented as a
 * percent of the y range of geometry) for the current key slice.  For non-end 
 * key slices, this range is anywhere between the end key slices.  For end
 * key slices, the range is limited by the nearest key slice (if symmetry flag
 * is on then opposite end must be considered also)...
 *
 * This func will be used to limit the locations where key slices can be moved
 * to or added (an easy way out of some annoying bugs)...
 *
 * I use the term "percent" loosely.  This func passes back values between 0
 * and 1...
 *
 */
void VorSlice::get_key_slice_percent_bounds(int base_on_current, float *pmin, float *pmax)
{
   const float grace = MINFLOAT;
   float yextent, yextent2;
   float ymin = key_slice.ymin;
   float ymax = key_slice.ymax;
   float yspan = ymax - ymin;
   float ytol = key_slice_tolerance * yspan;
   int nkey = key_slice.nkey;
   int current_key = key_slice.current_key;

   if (base_on_current && (current_key == 0 || current_key == nkey - 1)) {
      if (current_key == 0) {
         *pmin = 0.;
         yextent = (key_slice.y[1] - ymin) / yspan;
         if (key_slice.symmetry_flag) {
            yextent = (yextent > 0.5f) ? 0.5f : yextent;
            yextent2 = (ymax - key_slice.y[nkey - 2]) / yspan;
            yextent2 = (yextent2 > 0.5f) ? 0.5f : yextent2;
            if (yextent < yextent2)
               *pmax = yextent - 1.1f * key_slice_tolerance - grace;
            else
               *pmax = yextent2 - 1.1f * key_slice_tolerance - grace;
         }
         else
            *pmax = yextent - 1.1f * key_slice_tolerance - grace; 
printf( "111 pmin pmax %f %f \n", *pmin, *pmax );
      }
      else {
         *pmax = 1.;
         yextent = (ymax - key_slice.y[nkey - 2]) / yspan;
         if (key_slice.symmetry_flag) {
            yextent = (yextent > 0.5f) ? 0.5f : yextent;
            yextent2 = (key_slice.y[1] - ymin) / yspan;
            yextent2 = (yextent2 > 0.5f) ? 0.5f : yextent2;
            if (yextent < yextent2)
               *pmin = 1.0f - yextent + 1.1f * key_slice_tolerance + grace;
            else
               *pmin = 1.0f - yextent2 + 1.1f * key_slice_tolerance + grace;
         }
         else
            *pmin = 1.0f - yextent + 1.1f * key_slice_tolerance + grace;
printf( "222 pmin pmax %f %f \n", *pmin, *pmax );

      }
   }
   else {
      yextent = (key_slice.y[0] - ymin) / yspan;
      yextent2 = (ymax - key_slice.y[nkey - 1]) / yspan;
      if (key_slice.symmetry_flag) {
         if (yextent >= .5 || yextent2 >= .5) { /* no can do */
            *pmin = 1.;
            *pmax = 0.;
         }
         else {
            if (yextent > yextent2) {
               *pmin = yextent + 1.1f * key_slice_tolerance + grace;
               *pmax = 1.0f - yextent - 1.1f * key_slice_tolerance - grace;
            }
            else {
               *pmin = yextent2 + 1.1f * key_slice_tolerance + grace;
               *pmax = 1.0f - yextent2 - 1.1f * key_slice_tolerance - grace;
            }
         }
printf( "333 pmin pmax %f %f \n", *pmin, *pmax );
      }
      else {
         *pmin = yextent + 1.1f * key_slice_tolerance + grace;
         *pmax = 1.0f - yextent2 - 1.1f * key_slice_tolerance - grace;
printf( "444 pmin pmax %f %f \n", *pmin, *pmax );
      }
   }

printf( "done pmin pmax %f %f \n", *pmin, *pmax );

}



/*
 ****************************************************************
 */

/*
 * Num_key_points
 *
 * This func takes as arg a pointer to a key point list.  It returns the
 * number of key points in the specified key point list.
 */
int VorSlice::Num_key_points(Key_point_list* keyptlst)
{
   if (keyptlst)
      return(keyptlst->n_key_point);
   else {
      info_message("Error in Num_key_points: null key point list.\n",4);
      exit(-1);
   }
   return 0;
}

/* alloc_key_point_list
 *
 * allocate and initialize an array of key point lists.  argument specifies how
 * many elements.  return pointer to it...
 */
Key_point_list* VorSlice::alloc_key_point_list(Key_point_list **keypt_out, int n)
{
	if ( *keypt_out )
		free( *keypt_out );
   Key_point_list *keypt;
   int i;

   keypt = (Key_point_list*) malloc(sizeof(Key_point_list) * n);
   if (! keypt) {
      fprintf(stderr,"unable to malloc key point list array\n");
      exit(-1);
   }
   for (i = 0; i < n; i ++) {
      keypt[i].n_key_point = 0;
      keypt[i].current_key_point = 0;
      keypt[i].key_point = 0;
   }
   *keypt_out = keypt;
   return(keypt);
}


/* realloc_key_point_list
 *
 * reallocate and initialize an array of key point lists.  
 * old and new size of array are specified as arguments.  if old size is less
 * than new size then new_index tells where new stuff will be inserted (so
 * we can shift appropriately)...  if old size is greater than new size then
 * new_index tells where stuff will be deleted (so we can shift appropriately).
 * return pointer to reallocated array...
 */
Key_point_list* VorSlice::realloc_key_point_list(Key_point_list* keypt,int old_n,
                                       int new_n,int new_index)
{
   int i, nnew, ndel;


   if (old_n == new_n) /* do nothing if no size change */
      return(keypt);
   else if (old_n < new_n) { /* shift elements appropriately */
      keypt = (Key_point_list*) realloc(keypt,sizeof(Key_point_list) * new_n);
      if (! keypt) {
         fprintf(stderr,"unable to realloc key point list array\n");
         exit(-1);
      }
      nnew = new_n - old_n;
      for (i = old_n - 1; i >= new_index; i --)
	 keypt[i + nnew] = keypt[i];
      for (i = new_index; i < new_index + nnew; i ++) {
         keypt[i].n_key_point = 0;
         keypt[i].current_key_point = 0;
         keypt[i].key_point = 0;
      }
   }
   else {
      ndel = old_n - new_n;
      if (new_index + ndel > old_n) {
	 fprintf(stderr,"bad parameters to realloc_key_point_list\n");
	 exit(-1);
      }
      for (i = new_index; i < old_n; i ++) {
	 if (i < new_index + ndel)
	    free_key_point_list(&keypt[i]);
	 if (i + ndel < old_n)
	    keypt[i] = keypt[i + ndel];
      }
      keypt = (Key_point_list*) realloc(keypt,sizeof(Key_point_list) * new_n);
      if (! keypt) {
         fprintf(stderr,"unable to realloc key point list array\n");
         exit(-1);
      }
   }
   return(keypt);
}



/*
 * insert_key_point
 *
 * insert the given key point in key point list (if it isn't already there)
 * if the point is already in the list then copy the "new" (user set) values
 * into the corresponding point in trapezoid...
 */
void VorSlice::insert_key_point(Key_point_list *keyptlst,int itrap,int ipoint)
{
   int ip, ip2;

/* use delete flag here ? */

   if (keyptlst->n_key_point > 0) {
      for (ip = 0; ip < keyptlst->n_key_point; ip ++) {
	 if (equal_point(&(keyptlst->key_point[ip].orig_point),
				    Vor->trap[itrap][ipoint])) {
	    keyptlst->key_point[ip].trap = itrap;
	    keyptlst->key_point[ip].point = ipoint;
	    get_point(&(keyptlst->key_point[ip].new_point),
						 &(Vor->trap[itrap][ipoint][0]),
	                                         &(Vor->trap[itrap][ipoint][1]),
	                                         &(Vor->trap[itrap][ipoint][2]));
	    return;
	 }
	 else if (greater_than_point(&(keyptlst->key_point[ip].orig_point),
				      Vor->trap[itrap][ipoint])) {
	    keyptlst->key_point = (Key_point*) realloc(keyptlst->key_point,
			      (keyptlst->n_key_point + 1) * sizeof(Key_point));
	    if (keyptlst->key_point) {
	       for (ip2 = keyptlst->n_key_point; ip2 > ip; ip2 --)
		  keyptlst->key_point[ip2] = keyptlst->key_point[ip2-1];
	       keyptlst->key_point[ip].trap = itrap;
	       keyptlst->key_point[ip].point = ipoint;
	       set_point(&(keyptlst->key_point[ip].orig_point),
						Vor->trap[itrap][ipoint][0],
	                                        Vor->trap[itrap][ipoint][1],
	                                        Vor->trap[itrap][ipoint][2]);
	       set_point(&(keyptlst->key_point[ip].new_point), 
						Vor->trap[itrap][ipoint][0],
	                                        Vor->trap[itrap][ipoint][1],
	                                        Vor->trap[itrap][ipoint][2]);
	    }
	    else {
	       fprintf(stderr,"unable to realloc array of key points\n");
	       exit(1);
	    }
	    keyptlst->n_key_point ++;
	    return;
	 }
      }/*for*/
      keyptlst->key_point =(Key_point*) realloc(keyptlst->key_point,
			     (keyptlst->n_key_point + 1) * sizeof(Key_point));
      if (keyptlst->key_point) {
	 keyptlst->key_point[keyptlst->n_key_point].trap = itrap;
	 keyptlst->key_point[keyptlst->n_key_point].point = ipoint;
	 set_point(&(keyptlst->key_point[keyptlst->n_key_point].orig_point), 
						Vor->trap[itrap][ipoint][0],
	                                        Vor->trap[itrap][ipoint][1],
	                                        Vor->trap[itrap][ipoint][2]);
	 set_point(&(keyptlst->key_point[keyptlst->n_key_point].new_point), 
						Vor->trap[itrap][ipoint][0],
	                                        Vor->trap[itrap][ipoint][1],
	                                        Vor->trap[itrap][ipoint][2]);
	 keyptlst->n_key_point ++;
      }
      else {
	 fprintf(stderr,"unable to realloc array of key points\n");
	 exit(1);
      }
   }
   else {
      keyptlst->key_point = (Key_point*) malloc(sizeof(Key_point));
      if (keyptlst->key_point) {
	 keyptlst->n_key_point = 1;
	 keyptlst->current_key_point = 0;
	 keyptlst->key_point[0].trap = itrap;
	 keyptlst->key_point[0].point = ipoint;
	 set_point(&(keyptlst->key_point[0].orig_point),
						   Vor->trap[itrap][ipoint][0],
	                                           Vor->trap[itrap][ipoint][1],
	                                           Vor->trap[itrap][ipoint][2]);
	 set_point(&(keyptlst->key_point[0].new_point), 
						   Vor->trap[itrap][ipoint][0],
	                                           Vor->trap[itrap][ipoint][1],
	                                           Vor->trap[itrap][ipoint][2]);
      }
      else {
	 fprintf(stderr,"unable to malloc key point list\n");
	 exit(1);
      }
   }
}

/*
 * Key_point_index
 *
 * Look for the given xyz point in the given key point list.  If the point is
 * found, pass back the index into the list and return 1.  If the point is not
 * found, return 0.
 * 
 */
int VorSlice::Key_point_index(float xyz[3],Key_point_list *keypt,int *element)
{
   int i;

   if (keypt->n_key_point > 0) {
      for (i = 0; i < keypt->n_key_point; i ++) {
	 if (equal_point(&(keypt->key_point[i].new_point),xyz) ||
	     equal_point(&(keypt->key_point[i].orig_point),xyz)) {
	    *element = i;
	    return(1);
	 }
      }
   }
   return(0);
}

/* 
 * Set_current_key_point
 *
 * sets the current key point in the given key point list to the given element.
 * this sets the point the user may relocate...
 */
void VorSlice::Set_current_key_point(Key_point_list *keypt,int element)
{
   if (0 <= element && element < keypt->n_key_point) {
      keypt->current_key_point = element;
   }
   else {
      info_message("Fatal error in Set_current_key_point - bad element #!",4);
      exit(-1);
   }
}


/*
 * Move_current_key_point
 *
 * move the current key point (what ever it is) to the specified xyz location...
 */
void VorSlice::Move_current_key_point(Key_point_list *keypt,float xyz[3])
{
   if (0 <= keypt->current_key_point && 
            keypt->current_key_point < keypt->n_key_point) {
      set_point(&(keypt->key_point[keypt->current_key_point].new_point),
		xyz[0],xyz[1],xyz[2]);
   }
   else {
      info_message("Fatal error in Move_current_key_point - bad current #!",4);
      exit(-1);
   }
}

/*
 * Move_opposite_key_point
 *
 * move the key point opposite the current one (what ever it is) to the 
 * specified x location (y and z stay the same)... 
 */
void VorSlice::Move_opposite_key_point(Key_point_list *ckeypt,Key_point_list *okeypt,
                             float x)
{
   float xdum, y, z;
   int trap, point;

   if (ckeypt->n_key_point != okeypt->n_key_point)
      info_message("Unable to move opposite key point (not symmetric)! Try turning off symmetry...",4);
   else if (0 <= ckeypt->current_key_point &&
            ckeypt->current_key_point < ckeypt->n_key_point &&
            ckeypt->current_key_point < okeypt->n_key_point) {
      get_point(&(okeypt->key_point[ckeypt->current_key_point].new_point),
                &xdum,&y,&z);
      set_point(&(okeypt->key_point[ckeypt->current_key_point].new_point),
                x,y,z);
      trap = okeypt->key_point[ckeypt->current_key_point].trap;
      point = okeypt->key_point[ckeypt->current_key_point].point;
      Vor->trap[trap][point][0] = x;
   }
   else {
      info_message("Fatal error in Move_opposite_key_point - bad current #!",4);
      exit(-1);
   }
}


/*
 * free_key_point_list
 *
 * free the array of key points in the key point list, set the counter to 0,
 * and set current index to -1...
 */
void VorSlice::free_key_point_list(Key_point_list *keyptlst)
{
   if (keyptlst) {
      if (keyptlst->key_point) {
	 free(keyptlst->key_point);
	 keyptlst->key_point = 0;
      }
      keyptlst->n_key_point = 0;
      keyptlst->current_key_point = -1;
   }
}

/*
 * Reset_current_key_point
 *
 * reset the current key point in the given key point list to its
 * original location...
 */
void VorSlice::Reset_current_key_point(Key_point_list *keyptlst)
{
   Point *orig_point, *new_point;
   int trap, point, ckp;
   float x, y, z;

   ckp = keyptlst->current_key_point;
   if (0 <= ckp  && ckp < keyptlst->n_key_point) {
      orig_point = &(keyptlst->key_point[ckp].orig_point);
      new_point  = &(keyptlst->key_point[ckp].new_point);
      copy_point(orig_point,new_point);
      trap = keyptlst->key_point[ckp].trap;
      point = keyptlst->key_point[ckp].point;
      get_point(new_point,&x,&y,&z);
      Vor->trap[trap][point][0] = x;
      Vor->trap[trap][point][1] = y;
      Vor->trap[trap][point][2] = z;
   }
   else {
      info_message("Reset ignored - no current key point.",4);
   }
}

/*
 * Reset_opposite_key_point
 *
 * reset the key point opposite the current one in the given key point list 
 * to its original location...
 *
 * Note that the current key point list (ckeyptlst) is passed in so that we
 * can get the current key point index.  The opposite key point list (keyptlst)
 * is the one we work on...
 */
void VorSlice::Reset_opposite_key_point(Key_point_list *ckeyptlst, 
			      Key_point_list *keyptlst)
{
   Point *orig_point, *new_point;
   int trap, point, ckp;
   float x, y, z;

   ckp = ckeyptlst->current_key_point;
   if (0 <= ckp  && ckp < keyptlst->n_key_point) {
      orig_point = &(keyptlst->key_point[ckp].orig_point);
      new_point  = &(keyptlst->key_point[ckp].new_point);
      copy_point(orig_point,new_point);
      trap = keyptlst->key_point[ckp].trap;
      point = keyptlst->key_point[ckp].point;
      get_point(new_point,&x,&y,&z);
      Vor->trap[trap][point][0] = x;
      Vor->trap[trap][point][1] = y;
      Vor->trap[trap][point][2] = z;
   }
   else {
      info_message("Reset ignored - no opposite key point.",4);
   }
}


/*
 * Get_key_point_trap
 *
 * this func takes as args:
 *    - a key point list (pointer to it)
 *    - index into the key point list
 * and returns the index of the trapezoid corresponding to the given key point.
 *
 */
int VorSlice::Get_key_point_trap(Key_point_list *keyptlst,int ielement)
{
   if (keyptlst) {
      if (0 <= ielement && ielement < keyptlst->n_key_point) {
	 return(keyptlst->key_point[ielement].trap);
      }
      else {
	 char str[255];
	 sprintf(str,"Error in Get_key_point_trap: bad ielement value (%d)",
		 ielement);
	 info_message(str,4);
	 exit(1);
      }
   }
   else {
      info_message("Error in Get_key_point_trap: null key point list ptr!",4);
      exit(1);
   }
   return 0;
}

/*
 * Locate_key_point_by_trap
 *
 * This func takes as args:
 *    - keyptlst: a pointer to a key point list
 *    - itrap: index of a trapezoid
 * this func searches thru the specified key point list for a key point which
 * belongs to (is the corner point of) the specified trapezoid.
 * The index of the first key point found is returned (if none is found then
 * -1 is returned)...
 */
int VorSlice::Locate_key_point_by_trap(Key_point_list *keyptlst, int itrap)
{
   int ikp = -1, nkp, i;

   if (keyptlst) {
      if (0 <= itrap && itrap < Vor->ntraps) {
	 nkp = keyptlst->n_key_point;
	 if (nkp > 0)
	    for (i = 0; i < nkp && ikp < 0; i ++)
	       if (keyptlst->key_point[i].trap == itrap)
		  ikp = i;
      }
      else {
	 char str[255];
	 sprintf(str,"Error in Locate_key_point_by_trap: bad itrap value (%d)",
		 itrap);
	 info_message(str,4);
	 exit(1);
      }
   }
   else {
      info_message("Error in Locate_key_point_by_trap: null key point list ptr!",4);
      exit(1);
   }
   return(ikp);
}

/*
 * Locate_key_point_pair
 *
 * This func takes as args:
 *    - keyptlst: a pointer to a key point list
 *    - ielement_start: the element in the key point list where search starts
 * This func searches thru the specified key point list at the specified
 * starting location for a pair of key points that share a common trapezoid.
 * The lowest index is returned (or -1 if no pair found).
 *
 */
int VorSlice::Locate_key_point_pair(Key_point_list *keyptlst, int ielement_start)
{
   if (keyptlst) {
      int nkp = keyptlst->n_key_point;
      if (0 <= ielement_start && ielement_start < nkp) {
	 int index = -1;
	 int i;
	 if (ielement_start < nkp - 1)
	    for (i = ielement_start; i < nkp - 1 && index < 0; i ++)
	       if (keyptlst->key_point[i].trap == keyptlst->key_point[i+1].trap)
	          index = i;
	 return(index);
      }
      else {
	 char str[255];
	 sprintf(str,"Error in Locate_key_point_pair: bad ielement_start value (%d)",
		 ielement_start);
	 info_message(str,4);
	 exit(1);
      }
   }
   else {
      info_message("Error in Locate_key_point_pair: null key point list ptr!",4);
      exit(1);
   }
   return 0;
}

/*
 * Get_key_point_new_xyz
 *
 * this func takes as args:
 *    - keyptlst: a pointer to a key point list
 *    - ielement: index of a particular key point in key point list
 * and passes back the "new" (ie possibly modified by user or program) 
 * {x,y,z} location of the specified key point.
 *
 */
void VorSlice::Get_key_point_new_xyz(Key_point_list* keyptlst, int ielement, 
		           float* x, float* y, float* z)
{
   if (keyptlst) {
      int nkp = keyptlst->n_key_point;
      if (0 <= ielement && ielement < nkp) {
	 get_point(&(keyptlst->key_point[ielement].new_point), x, y, z);
      }
      else {
	 char str[255];
	 sprintf(str,"Error in Get_key_point_new_xyz: bad ielement value (%d)",
		 ielement);
	 info_message(str,4);
	 exit(1);
      }
   }
   else {
      info_message("Error in Get_key_point_new_xyz: null key point list ptr!",4);
      exit(1);
   }
}

/*
 * Set_key_point_new_xyz
 *
 * this func takes as args:
 *    - keyptlst: a pointer to a key point list
 *    - ielement: index of a particular key point in key point list
 *    - x,y,z: an {x,y,z} coordinate
 * and sets the "new" (ie possibly modified by user or program) 
 * {x,y,z} location of the specified key point.
 *
 */
void VorSlice::Set_key_point_new_xyz(Key_point_list* keyptlst, int ielement, 
			   float x, float y, float z)
{
   if (keyptlst) {
      int nkp = keyptlst->n_key_point;
      if (0 <= ielement && ielement < nkp) {
	 set_point(&(keyptlst->key_point[ielement].new_point), x, y, z);
      }
      else {
	 char str[255];
	 sprintf(str,"Error in Set_key_point_new_xyz: bad ielement value (%d)",
		 ielement);
	 info_message(str,4);
	 exit(1);
      }
   }
   else {
      info_message("Error in Set_key_point_new_xyz: null key point list ptr!",4);
      exit(1);
   }
}


/*
 * Read_key_point
 *
 * read key point data from .kpt file.  The file is opened by caller
 * (read_key_point).  A file pointer and a pointer to a key point list are
 * passed in...
 * Status is returned (0=ok)...
 *
 * Note: what is read in is the delta-x for each key point.  This delta is
 * added to the x in the original point to produce a new point...
 */
int VorSlice::Read_key_point(FILE *fp, Key_point_list *keyptlst)
{
   int npt, j;
   float delta_x, x, y, z;

   fscanf(fp,"%d",&npt);
   if (feof(fp) || ferror(fp)) {
      return(1);
   }
   else if (npt != keyptlst->n_key_point) {
      return(1);
   }
   for (j = 0; j < npt; j ++) {
      fscanf(fp,"%g",&delta_x);
      if (feof(fp) || ferror(fp)) {
	 return(1);
      }
      get_point(&(keyptlst->key_point[j].orig_point),&x,&y,&z);
      x += delta_x;
      set_point(&(keyptlst->key_point[j].new_point),x,y,z);
   }
    
   return(0);
}


/*
 * Write_key_point
 *
 * write key point data to .kpt file.  The file is opened by the caller
 * (write_key_point) and passed in, along with a pointer to a key point
 * list.  The 3rd argument is a flag; if it is non-zero then it means that
 * the key-point list is on the low-y side of a key slice; if it is zero then
 * it means that the key-point list is on the high-y side of a key-slice...
 * Status is returned (0=ok)...
 *
 * Note: what is written is the delta-x for each key point.  This delta is
 * x difference between the original point and the new (modified) point...
 *
 * Update 7/95: added 3rd argument and logic so that only key points that
 * correspond to actual trapezoid vertices are written out...
 */
int VorSlice::Write_key_point(FILE *fp, Key_point_list *keyptlst, int low_list)
{
   int npt, j;
   float delta_x, xo, yo, zo, xn, yn, zn;
   static Key_point **temp_list = 0;
   static int n_temp_list = 0;
   int npt_actual = 0;

   npt = keyptlst->n_key_point;
   assert(npt > 0);
   if (temp_list) {
      if (npt > n_temp_list) {
	 n_temp_list = npt;
	 temp_list = (Key_point**) realloc(temp_list, sizeof(Key_point*) * 
						      n_temp_list);
      }
   }
   else {
      n_temp_list = npt;
      temp_list = (Key_point**) malloc(sizeof(Key_point*) * n_temp_list);
   }
   assert(temp_list);

   for (j = 0; j < npt; j ++) {
      int itrap = keyptlst->key_point[j].trap;
      if (itrap < Vor->ntraps) {
	 int ipoint = keyptlst->key_point[j].point;
	 assert(0 <= ipoint && ipoint <= 3);
	 if (equal_point(&(keyptlst->key_point[j].new_point),
			 Vor->trap[itrap][ipoint])) {
	    float ymin, ymax;
	    find_trap_y_minmax(Vor->trap[itrap], &ymin, &ymax);
	    get_point(&(keyptlst->key_point[j].new_point),&xn,&yn,&zn);
	    if ((low_list && yn == ymax) || (!low_list && yn == ymin)) {
	       temp_list[npt_actual] = &(keyptlst->key_point[j]);
	       npt_actual ++;
	    }
	 }
      }
   }
   assert(npt_actual);

   fprintf(fp,"%d\n",npt_actual);
   if (ferror(fp)) {
      return(1);
   }
   for (j = 0; j < npt_actual; j ++) {
      get_point(&(temp_list[j]->orig_point),&xo,&yo,&zo);
      get_point(&(temp_list[j]->new_point),&xn,&yn,&zn);
      delta_x = xn - xo;
      fprintf(fp,"%g\n",delta_x);
      if (ferror(fp)) {
         return(1);
      }
   }

   return(0);
}


/*======================================================================*
* Module Name:  compute_slice
*=======================================================================*
* Description: Compute slices for current surface model
*
* Input: None
*
* Output: Slice data
*=======================================================================*/

void VorSlice::compute_slice()
{
  int icomp, islice, comp_slc_cnt;
  int *num_all_slc;
  float **bbox, *slice, **comp_slc, ***all_slc;

 init_trapezoid(); /* lep */

 if (1)
/*  if (get_num_slc_camb()) */
   {

     Vor->nslices = get_total_nslices(); /* lep */

     /* ---- Allocate space for bounding box and component slices--- */
     bbox = falloc_2d(Vor->ncomps, 6);
     comp_slc = falloc_2d(Vor->ncomps, 4);

     /* ---- Find Bounding Box for each component --- */
     comp_bound_box(bbox);

     /* ---- Allocate space for slices ---- */
     slice = (float *)calloc(Vor->nslices, sizeof(float));
     all_slc = (float ***)calloc(Vor->nslices, sizeof(float **));
     num_all_slc = (int *)calloc(Vor->nslices, sizeof(int));

     /* ---- Find Slice Locations ---- */
     slice_locations(slice); /* lep: no need for bbox arg now */

     for (islice = 0 ; islice < Vor->nslices ; islice++)
       {
         comp_slc_cnt = 0;
         for (icomp = 0 ; icomp < Vor->ncomps ; icomp++)
           {
              /* ---- Check Bounding Box for Possible Intersection --- */
              if (check_bbox(bbox[icomp], slice[islice]))
                {

                  /* ---- Perform Slice Computations and Return Comp Slice --- */
                  comp_slice(icomp, slice[islice], comp_slc[comp_slc_cnt]);

                  /* ---- Increment Number of Component Slices ---- */
                  comp_slc_cnt++;
                }
           }
         /* --- Allocate Space for Worst Case all_slc ---*/
         all_slc[islice] = falloc_2d(comp_slc_cnt, 4);
 
         /* --- Order and Process Slices ---- */
         num_all_slc[islice] = order_slices(comp_slc_cnt, comp_slc,  
                                            all_slc[islice]);
       } 

     /* ---- Assemble Slices ---- */
     assemble_slices(slice, num_all_slc, all_slc);

     /* ---- Allocate Space for Normalized Traps ---- */
//     Vor->strap = falloc_3d(Vor->ntraps, 4, 3);
     Vor->ttrap = falloc_3d(Vor->ntraps, 4, 3);

     /* ---- Find Camber Values ---- */
     if (Vor->ncamb > 4)
       {
         Vor->camb  = falloc_2d(Vor->ntraps, Vor->ncamb);
//         Vor->scamb = falloc_2d(Vor->ntraps, Vor->ncamb);
         trap_camb(bbox);
       }

     /* ---- Free Up Memory --- */
     ffree_2d(bbox,     Vor->ncomps);
     ffree_2d(comp_slc, Vor->ncomps);
     free(slice);
	 free(num_all_slc);
//jrg FIX free all_slc

 
     /* ---- Normalize Slices ---- */
     normslice();

//     zoomvor(1.0);

     showslc = 1;

#if 0
     printf("compute_slice BEFORE BEFORE BEFORE BEFORE *************************:\n"); /* lep */
     dump_key_points(); /* lep debug */
#endif
     find_key_points(); /* lep */
#if 0
     printf("compute_slice AFTER AFTER AFTER AFTER AFTER ***********************:\n"); /* lep */
     dump_key_points(); /* lep debug */
#endif
  }
}
/*======================================================================*
* Module Name:  comp_bound_box
*=======================================================================*
* Description: Find bounding x,y,z values for each component
*
* Input: None
*
* Output: bbox - 	Bounding box coordinates
*=======================================================================*/

void VorSlice::comp_bound_box(float** bbox)
{
  int icomp, i, j;
  float minx, maxx, miny, maxy;
  float minz, maxz, x, y, z;

  /* ---- Loop Thru All Comps ---- */
  for ( icomp = 0 ; icomp < Vor->ncomps ; icomp++)
   {

    /* ---- Init Min/Max xyz For Each Comp ----*/
    minx = maxx = Vor->herm[icomp][0][0][0];
    miny = maxy = Vor->herm[icomp][0][0][1];
    minz = maxz = Vor->herm[icomp][0][0][2];

    /* ---- Loop Thru Points and Find Min/Max xyz ----*/
    for ( i = 0 ; i < Vor->ncross[icomp] ; i++)
      {
        for ( j = 0 ; j < Vor->npts[icomp] ; j++)
          {
            x = Vor->herm[icomp][i][j][0];
            y = Vor->herm[icomp][i][j][1];
            z = Vor->herm[icomp][i][j][2];
            if (x > maxx) maxx = x;
            else if (x < minx) minx = x;
            if (y > maxy) maxy = y;
            else if (y < miny) miny = y;
            if (z > maxz) maxz = z;
            else if (z < minz) minz = z;

          }
      }
    bbox[icomp][0] = minx;
    bbox[icomp][1] = maxx;
    bbox[icomp][2] = miny;
    bbox[icomp][3] = maxy;
    bbox[icomp][4] = minz;
    bbox[icomp][5] = maxz;
   }
}

/*======================================================================*
* Module Name:  slice_locations
*=======================================================================*
* Description: Find y locations for slice
*
* Input: key slice locations (global)
*
* Output: slice - 	Slice locations
*=======================================================================*/
void VorSlice::slice_locations(float* slice)
{
  int islc = 0, jslc, i, jmax;
  float miny, maxy, spany, delty;

/* lep: modified to work with "key" slices... */
/* loop thru the key slice regions, computing slices in each... */
  for (i = 0; i < key_slice.nkey - 1; i ++) {
     maxy = key_slice.y[i+1];
     miny = key_slice.y[i];
     spany = maxy - miny;
     jmax = key_slice.nslice[i] - 1;
     delty = spany / ((float) jmax);
     for (jslc = 0; jslc <= jmax; jslc ++) {
	if (jslc == 0)
	   slice[islc] = miny;
	else if (jslc == jmax)
	   slice[islc + jmax] = maxy;
	else
	   slice[islc + jslc] = miny + (float)jslc*delty;
     }
     islc += jmax;
  }

}
     
/*======================================================================*
* Module Name:  check_bbox
*=======================================================================*
* Description: Check for bounding box interference
*
* Input: box - 	current bounding box
*	 slc -  current slice location
*
* Output: Interference flag
*=======================================================================*/
int VorSlice::check_bbox(float* box, float slc)
{
  if (slc >= box[2] && slc <= box[3])
    {
      return(1);
    }
  return(0);
}
   
/*======================================================================*
* Module Name:  comp_slice
*=======================================================================*
* Description: Compute slices for current component
*
* Input: comp_num - 	current comp number
*	 slc	  - 	current slice number
*
* Output: comp_slc-	component slice data
*=======================================================================*/
void VorSlice::comp_slice(int comp_num, float slc, float* comp_slc)
{
  int nxs, npt, i, j;
  float xint[2], x1, x2, y1, y2;
  float zint[2], z1, z2;

  /* ---- Initalize Intersection values ----*/
  xint[0] = 1.0E06;
  xint[1] = -1.0E06;
  zint[0] = 1.0E06;
  zint[1] = -1.0E06;
 
  nxs = Vor->ncross[comp_num];
  npt = Vor->npts[comp_num];

  /* ---- Find All Intersections With X-Sec Lines ----*/  
  for (i = 0 ; i < nxs ; i++)
    {
      for (j = 0 ; j < npt-1 ; j++)
        {
          x1 = Vor->herm[comp_num][i][j][0];
          x2 = Vor->herm[comp_num][i][j+1][0];
          y1 = Vor->herm[comp_num][i][j][1];
          y2 = Vor->herm[comp_num][i][j+1][1];
          z1 = Vor->herm[comp_num][i][j][2];
          z2 = Vor->herm[comp_num][i][j+1][2];
         
          if (slc >= y1 && slc <= y2) 
            {
              slc_int(slc, x1, x2, y1, y2, z1, z2, xint, zint);
            }
          else if (slc >= y2 && slc <= y1)
            {
              slc_int(slc, x2, x1, y2, y1, z2, z1, xint, zint);
            }
        }
     }

  /* ---- Find All Intersections With Stringer Lines ----*/  
  for (j = 0 ; j < npt ; j++)
    {
      for (i = 0 ; i < nxs-1 ; i++)
        {
          x1 = Vor->herm[comp_num][i][j][0];
          x2 = Vor->herm[comp_num][i+1][j][0];
          y1 = Vor->herm[comp_num][i][j][1];
          y2 = Vor->herm[comp_num][i+1][j][1];
          z1 = Vor->herm[comp_num][i][j][2];
          z2 = Vor->herm[comp_num][i+1][j][2];
         
          if (slc >= y1 && slc <= y2) 
            {
              slc_int(slc, x1, x2, y1, y2, z1, z2, xint, zint);
            }
          else if (slc >= y2 && slc <= y1)
            {
              slc_int(slc, x2, x1, y2, y1, z2, z1, xint, zint);
            }
        }
     }

  /* ---- Set Slice Locations ----*/  
   comp_slc[0] = xint[0];
   comp_slc[1] = xint[1];
   comp_slc[2] = zint[0];
   comp_slc[3] = zint[1];

   if (Vor->flat_flag)
     {
       comp_slc[2] = 0.0;
       comp_slc[3] = 0.0;
     }
}

/*======================================================================*
* Module Name:  slc_int
*=======================================================================*
* Description: Compute x intersection values at slice location
*
* Input: slc	- y slice location
*	 x1, x2 - polygon x values
*	 y1, y2 - polygon y values
*
* Output: xint	- x intersection values
*=======================================================================*/

void VorSlice::slc_int(float slc, float x1, float x2, float y1, float y2, float z1, float z2, 
			 float* xint, float* zint)
{
   float ydiff, yfract, xval, zval, tol;

   tol = 0.00001f*Vor->scale;

   /* ---- Find Y Intersection Values ---- */
   ydiff = y2 - y1;
   if (ydiff > 0.0)
     {
       yfract = (slc - y1)/ydiff;
       xval = (x2 - x1)*yfract + x1;
       zval = (z2 - z1)*yfract + z1;
     }
   else
     {
       xval = x1;
       zval = z1;
     }

   /* ---- Find Z Intersection Values ---- */
   if (fabs(xval - xint[0]) < tol)
     {
       if (xval < xint[0])
         {
           xint[0] = xval;
           zint[0] = (zint[0] + zval)/2.0f;
         }
       else
         {
           zint[0] = (zint[0] + zval)/2.0f;
         }
     }
   else if (xval < xint[0]) 
     {
       xint[0] = xval;
       zint[0] = zval;
     }

   if (fabs(xval - xint[1]) < tol)
     {
       if (xval > xint[1])
         {
           xint[1] = xval;
           zint[1] = (zint[1] + zval)/2.0f;
         }
       else
         {
           zint[1] = (zint[1] + zval)/2.0f;
         }
     }
   else if (xval > xint[1]) 
     {
       xint[1] = xval;
       zint[1] = zval;
     }

}

/*======================================================================*
* Module Name:  order_slices
*=======================================================================*
* Description: Order slice intersections in ascending x location
*
* Input: 
*
* Output: 
*=======================================================================*/

int VorSlice::order_slices(int comp_slc_cnt, float** comp_slc, float** all_s)
{
  int i, j, add_flag, n_all;
  float temp1, temp2, xmin, xmax, axmin, axmax, tol;
  float temp3, temp4, zmin, zmax;

  /* --- Sort slice points ---- */
  for (i = 0 ; i < comp_slc_cnt ; i++)
    {
      for (j = 0 ; j < (comp_slc_cnt - i - 1) ; j++)
        {
           if (comp_slc[j][0] > comp_slc[j+1][0]) 
             {
               temp1 = comp_slc[j][0]; temp2 = comp_slc[j][1];
               temp3 = comp_slc[j][2]; temp4 = comp_slc[j][3];
               comp_slc[j][0] = comp_slc[j+1][0];
               comp_slc[j][1] = comp_slc[j+1][1];
               comp_slc[j][2] = comp_slc[j+1][2];
               comp_slc[j][3] = comp_slc[j+1][3];
               comp_slc[j+1][0] = temp1; comp_slc[j+1][1] = temp2;
               comp_slc[j+1][2] = temp3; comp_slc[j+1][3] = temp4;

             }
        }
    }

  /* --- Find tolerance --- */
  tol = -0.001f*(float)fabs(comp_slc[0][0] - comp_slc[comp_slc_cnt-1][1]);
  
  /* --- Set First Slice Points ---- */
  all_s[0][0] = comp_slc[0][0];
  all_s[0][1] = comp_slc[0][1];
  all_s[0][2] = comp_slc[0][2];
  all_s[0][3] = comp_slc[0][3];
  n_all = 1;

  /* --- Loop Through All Slice Points and Condense Slices --- */
  for (i = 1 ; i < comp_slc_cnt ; i++)
    {
      xmin = comp_slc[i][0];
      xmax = comp_slc[i][1];
      zmin = comp_slc[i][2];
      zmax = comp_slc[i][3];

      add_flag = 1;
 
      for (j = 0 ; j < n_all ; j++)
        {
          if (add_flag)
            {
              axmin = all_s[j][0];
              axmax = all_s[j][1];
 
              if (xmin >= axmin && xmax <= axmax) 
                {
                  add_flag = 0;
                }
              else if (xmin <= axmin && xmax >= axmax) 
                {
                  all_s[j][0] = xmin;
                  all_s[j][1] = xmax;
                  all_s[j][2] = zmin;
                  all_s[j][3] = zmax;
                  add_flag = 0;
                }
              else if ((xmin-axmin) > tol && (axmax-xmin) > tol)
                {
                  all_s[j][1] = xmax;
                  all_s[j][3] = zmax;
                  add_flag = 0;
                }
              else if ((xmax-axmax) > tol && (axmax-xmax) > tol)
                {
                  all_s[j][0] = xmin;
                  all_s[j][2] = zmin;
                  add_flag = 0;
                }

            }
        }
      if (add_flag) 
        {
           all_s[n_all][0] = xmin;
           all_s[n_all][1] = xmax;
           all_s[n_all][2] = zmin;
           all_s[n_all][3] = zmax;
           n_all++;
        }
   }

  return(n_all);

}
/*======================================================================*
* Module Name:  assemble_slices
*=======================================================================*
* Description: Assemble slices into planform geometry of the aircraft
*
* Input: 
*
* Output: 
*=======================================================================*/

void VorSlice::assemble_slices(float* slice,  int* num_all_slc, float*** all_slc)
{
  int is, ip, ip1, ip2, ntrap, tot_trap;
  int npts1, npts2;
  int ntrap_per_slc, start_trap_num; 
  float xmin, xmax;
  float y1, y2, xmin1, xmin2, xmax1, xmax2;
  float zmin, zmax;
  float zmin1, zmin2, zmax1, zmax2;
  float diffmn, xavg1, xavg2, xdiff;
  
  /* --- Find maximum number of traps and allocate trap space --- */
  tot_trap = 0;
  for (is = 0 ; is < Vor->nslices ; is++)
    {
       tot_trap += num_all_slc[is];
    }

  Vor->trap = falloc_3d(tot_trap, 4, 3);
  
  ntrap = 0;
  
  for (is = 0 ; is < (Vor->nslices)-1 ; is++)
    {
       start_trap_num = ntrap;

       y1 = slice[is];
       y2 = slice[is+1];

       npts1 = num_all_slc[is];
       npts2 = num_all_slc[is+1];
 
       if (npts1 == npts2) 
         {
           for (ip = 0 ; ip < npts1 ; ip ++)
             {
               load_trap(ntrap, all_slc[is][ip][0], all_slc[is][ip][1],
                         all_slc[is+1][ip][1], all_slc[is+1][ip][0], y1, y2,
                         all_slc[is][ip][2], all_slc[is][ip][3],
                         all_slc[is+1][ip][3], all_slc[is+1][ip][2]);
               ntrap++;
             }
         }
       else if (npts1 > npts2)
         {
           for (ip2 = 0 ; ip2 < npts2 ; ip2++)
             {
               xmin2 = all_slc[is+1][ip2][0];
               xmax2 = all_slc[is+1][ip2][1];
               zmin2 = all_slc[is+1][ip2][2];
               zmax2 = all_slc[is+1][ip2][3];
               xavg2 = (xmin2+xmax2)/2.0f;
               diffmn = 1.0e06;

               for (ip1 = 0 ; ip1 < npts1 ; ip1++)
                 {
                   xmin1 = all_slc[is][ip1][0];
                   xmax1 = all_slc[is][ip1][1];
                   zmin1 = all_slc[is][ip1][2];
                   zmax1 = all_slc[is][ip1][3];
                   xavg1 = (xmin1+xmax1)/2.0f;
                   xdiff = (float)fabs(xavg2-xavg1);
                   if (xdiff < diffmn)
                     {
                        diffmn = xdiff;
			xmin = xmin1;
			xmax = xmax1;
			zmin = zmin1;
			zmax = zmax1;
		     }
		 }

               for (ip1 = 0 ; ip1 < npts1 ; ip1++)
                 {
                   xmin1 = all_slc[is][ip1][0];
                   xmax1 = all_slc[is][ip1][1];
                   zmin1 = all_slc[is][ip1][2];
                   zmax1 = all_slc[is][ip1][3];
                   xavg1 = (xmin1+xmax1)/2.0f;
                   if (xavg1 > xmin2 && xavg1 < xmax2)
                     {
			if (xmin1 < xmin) 
                          {
                            xmin = xmin1;
                            zmin = zmin1;
                          }
			if (xmax1 > xmax) 
                          {
                            xmax = xmax1;
                            zmax = zmax1;
                          }
		     }
		 }
               load_trap(ntrap, xmin, xmax, xmax2, xmin2, y1, y2, 
                                zmin, zmax, zmax2, zmin2);
	       ntrap++;
             }
         }
       else
	 {
	   for (ip1 = 0 ; ip1 < npts1 ; ip1++)
             {
               xmin1 = all_slc[is][ip1][0];
               xmax1 = all_slc[is][ip1][1];
               zmin1 = all_slc[is][ip1][2];
               zmax1 = all_slc[is][ip1][3];
               xavg1 = (xmin1+xmax1)/2.0f;
               diffmn = 1.0e06;
               for (ip2 = 0 ; ip2 < npts2 ; ip2++)
                 {
                   xmin2 = all_slc[is+1][ip2][0];
                   xmax2 = all_slc[is+1][ip2][1];
                   zmin2 = all_slc[is+1][ip2][2];
                   zmax2 = all_slc[is+1][ip2][3];
                   xavg2 = (xmin2+xmax2)/2.0f;
		   xdiff = (float)fabs(xavg2-xavg1);
                   if (xdiff < diffmn) 
       		     {
                        diffmn = xdiff;
			xmin = xmin2;
			xmax = xmax2;
			zmin = zmin2;
			zmax = zmax2;
		     }
                 }
               for (ip2 = 0 ; ip2 < npts2 ; ip2++)
                 {
                   xmin2 = all_slc[is+1][ip2][0];
                   xmax2 = all_slc[is+1][ip2][1];
                   zmin2 = all_slc[is+1][ip2][2];
                   zmax2 = all_slc[is+1][ip2][3];
                   xavg2 = (xmin2+xmax2)/2.0f;
		   if (xavg2 > xmin1 && xavg2 < xmax1)
		     {
			if (xmin2 < xmin) 
			  {
                            xmin = xmin2;
                            zmin = zmin2;
                          }
			if (xmax2 > xmax) 
                          {
                            xmax = xmax2;
                            zmax = zmax2;
                          }
           
		     }
                 }
               load_trap(ntrap, xmin1, xmax1, xmax, xmin, y1, y2, 
                                zmin1, zmax1, zmax, zmin);
	       ntrap++;
             }
         }
      ntrap_per_slc = ntrap - start_trap_num;
      if ((is > (Vor->nslices - 1)/2) && ntrap_per_slc > 1)
        {
           flip_trap_order(start_trap_num, ntrap-1);
        }
    }
  Vor->ntraps = ntrap;
}

/*======================================================================*
* Module Name:  load_trap
*=======================================================================*
* Description: Load trapezoid data into data structure
*
* Input: 
*
* Output: 
*=======================================================================*/

void VorSlice::load_trap(int ntrap,float x1, float x2, float x3, float x4, float y1, float y2, 
			   float z1, float z2, float z3, float z4)
{
  Vor->trap[ntrap][0][0] = x1;
  Vor->trap[ntrap][0][1] = y1;
  Vor->trap[ntrap][0][2] = z1;
  Vor->trap[ntrap][1][0] = x2;
  Vor->trap[ntrap][1][1] = y1;
  Vor->trap[ntrap][1][2] = z2;
  Vor->trap[ntrap][2][0] = x3;
  Vor->trap[ntrap][2][1] = y2;
  Vor->trap[ntrap][2][2] = z3;
  Vor->trap[ntrap][3][0] = x4;
  Vor->trap[ntrap][3][1] = y2;
  Vor->trap[ntrap][3][2] = z4;
}
/*======================================================================*
* Module Name:  flip_trap_order
*=======================================================================*
* Description: Reverse trapizoid order to reflect about center line
*
* Input: 
*
* Output:
*=======================================================================*/
void VorSlice::flip_trap_order(int s_ind, int e_ind)
{
  float temp[4][3];
  int i, j, n_ind;
  

  for (i = s_ind ; i <= (e_ind+s_ind)/2 ; i++)
    {
      n_ind = e_ind - i + s_ind;

      for (j = 0 ; j < 4 ; j++)
        {
           temp[j][0] = Vor->trap[i][j][0];
           temp[j][1] = Vor->trap[i][j][1];
           temp[j][2] = Vor->trap[i][j][2];
           Vor->trap[i][j][0] = Vor->trap[n_ind][j][0];
           Vor->trap[i][j][1] = Vor->trap[n_ind][j][1];
           Vor->trap[i][j][2] = Vor->trap[n_ind][j][2];
           Vor->trap[n_ind][j][0] = temp[j][0];
           Vor->trap[n_ind][j][1] = temp[j][1];
           Vor->trap[n_ind][j][2] = temp[j][2];
        }
    }
}
 

/*======================================================================*
* Module Name:  write_trap
*=======================================================================*
* Description: Writes trapezoid vertices into .slc file
*
* Input: 
*
* Output: 
*=======================================================================*/

void VorSlice::write_trap(char* flname)
{
  FILE *in_file;
  int i, j;
  static float zero = 0.0;

  char tempfl[255];
  char slc[8] = {".slc"};

  strcpy(tempfl, flname);
  strcat(tempfl, slc);

  /* --- Open file --- */
  in_file = fopen(tempfl, "w");

  fprintf(in_file, " %d \n", Vor->ntraps);

  for (i = 0; i < Vor->ntraps ; i++)
    {

      if ( swap_xy == 0 ) {

         fprintf(in_file,
           "%8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f\n", 
           Vor->trap[i][0][0], Vor->trap[i][0][1], Vor->trap[i][0][2], 
           Vor->trap[i][1][0], Vor->trap[i][1][1], Vor->trap[i][1][2],
           Vor->trap[i][2][0], Vor->trap[i][2][1], Vor->trap[i][2][2], 
           Vor->trap[i][3][0], Vor->trap[i][3][1], Vor->trap[i][3][2]); 

      }

      else {

         fprintf(in_file,
           "%8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f\n", 
           Vor->trap[i][0][0], Vor->trap[i][0][2], Vor->trap[i][0][1], 
           Vor->trap[i][1][0], Vor->trap[i][1][2], Vor->trap[i][1][1],
           Vor->trap[i][2][0], Vor->trap[i][2][2], Vor->trap[i][2][1], 
           Vor->trap[i][3][0], Vor->trap[i][3][2], Vor->trap[i][3][1]); 

      }

    }
  fprintf(in_file, "%d \n", Vor->ncamb);
 
  if (Vor->ncamb > 4)
    { 
      for (i = 0 ; i < Vor->ntraps ; i++)
        {
          for (j = 0 ; j < Vor->ncamb-1 ; j++)
            {
               fprintf(in_file,"%f ", Vor->camb[i][j]);
            }
          fprintf(in_file,"%f \n", Vor->camb[i][Vor->ncamb-1]);
        }
    }
  fprintf(in_file, "  \n");

  fclose(in_file);
}
/*======================================================================*
* Module Name:  write_csf_trap
*=======================================================================*
* Description: Write trapizoid and camber data which has been modified
*	       by the current control surface layout
*
* Input: flname	-	filename
*
* Output: None
*=======================================================================*/

void VorSlice::write_csf_trap(char* flname)
{
  FILE *slice_file;
  int i, j, k;
  int *num_csf_pntrs, **csf_pntrs;
  int trap_num, curr_num, csf, num_temp_camb;
  float trap_len, camb_len;
  float x1,y1,z1, x2,y2,z2, x3,y3,z3, x4,y4,z4;
  float xavg, yavg, zavg;
  float trap_angle, tan_theta; 
  float *temp_camb;

  char tempfl[255];
  char slc[8] = {".slc"};

  /* --- Set-up Control Surface Pointers ---- */
  num_csf_pntrs = (int *)calloc(Vor->ntraps, sizeof(int));
  csf_pntrs = ialloc_2d(Vor->ntraps, MAX_CSF);
  temp_camb = (float *)calloc(MAX_CAMB, sizeof(float));

  for (i = 0; i < Vor->ntraps ; i++)
    {
      num_csf_pntrs[i] = 0;
    }
 
  for (i = 0 ; i < Vor->ntotcsf ; i++)
    {
      for (j = 0 ; j < Vor->ncsftrap[i] ; j++)
        {
          trap_num = Vor->csftrap[i][j];
          curr_num = num_csf_pntrs[trap_num];
          csf_pntrs[trap_num][curr_num] = i;
          num_csf_pntrs[trap_num]++;
        }
     }

  /* --- Open slice file --- */
 
  strcpy(tempfl, flname);
  strcat(tempfl, slc);

  slice_file = fopen(tempfl, "w");
      
  /* write out slice file */  

  fprintf(slice_file, " %d \n", Vor->ntraps);

  for (i = 0; i < Vor->ntraps ; i++) {
  
     x1 = Vor->trap[i][0][0]; y1 = Vor->trap[i][0][1]; z1 = Vor->trap[i][0][2];
     x2 = Vor->trap[i][1][0]; y2 = Vor->trap[i][1][1]; z2 = Vor->trap[i][1][2];
     x3 = Vor->trap[i][2][0]; y3 = Vor->trap[i][2][1]; z3 = Vor->trap[i][2][2];
     x4 = Vor->trap[i][3][0]; y4 = Vor->trap[i][3][1]; z4 = Vor->trap[i][3][2];

     xavg = 0.25f*( x1 + x1 + x3 + x4 );
     yavg = 0.25f*( y1 + y1 + y3 + y4 );
     zavg = 0.25f*( z1 + z2 + z3 + z4 );

     trap_angle = 0;

     if (num_csf_pntrs[i] > 0) {
      
        for (j = 0 ; j < num_csf_pntrs[i] ; j++) {
        
           csf = csf_pntrs[i][j];
             
           if (Vor->csftype[csf] == 2) {
               
              trap_angle = Vor->csfdefangle[csf];

              /* Adjust for asymmetric deflections */
              
              if ( Vor->csfsymmetry[csf] == 0 ) {
          
                 if ( ( swap_xy == 0 && yavg > 0. ) ||
                      ( swap_xy == 1 && zavg < 0. ) ) {

                    trap_angle *= -1.;

                 }
     
              }
              
           }
              
        }
        
     }
        
     if (trap_angle != 0.0) {
     
       tan_theta = (float)tan((2.0f*3.14159f*trap_angle)/360.0);
       z1 =  0.25f*(x2-x1)*tan_theta + z1;
       z2 = -0.75f*(x2-x1)*tan_theta + z2;
       z4 =  0.25f*(x3-x4)*tan_theta + z4;
       z3 = -0.75f*(x3-x4)*tan_theta + z3;
     }
 
     if ( swap_xy == 0 ) {

        fprintf(slice_file,
        "%8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f\n", 
        x1,y1,z1, x2,y2,z2, x3,y3,z3, x4,y4,z4); 

    }

    else {

       fprintf(slice_file,
       "%8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f\n", 
       x1,z1,y1, x2,z2,y2, x3,z3,y3, x4,z4,y4); 

    }

  }

  if (Vor->ncamb > 4)
    {
      num_temp_camb = Vor->ncamb;
    }
  else
    {
      num_temp_camb = MAX_CAMB;
    }

  fprintf(slice_file, "%d \n", num_temp_camb);

  for (i = 0 ; i < Vor->ntraps ; i++)
    {
      if (Vor->ncamb > 4)
        {
           for (j = 0 ; j < num_temp_camb ; j ++)
             {
               temp_camb[j] = Vor->camb[i][j];
             }
        }
      else
        {
          for (j = 0 ; j < num_temp_camb ; j ++)
             {
               temp_camb[j] = 0.0;
             }
        }

      trap_len = (float)((fabs(Vor->trap[i][1][0] - Vor->trap[i][0][0]) +
                  fabs(Vor->trap[i][3][0] - Vor->trap[i][2][0]))/2.0f);
 
      camb_len = trap_len/(float)(num_temp_camb-1);

      for (j = 0 ; j < num_csf_pntrs[i] ; j++)
        {
           csf = csf_pntrs[i][j];
           if (Vor->csftype[csf] == 0 || Vor->csftype[csf] == 1)
             {

               /* --- Modify camber to include flaps and slats ---- */
               flap_slat_camb(i, num_temp_camb, temp_camb, csf, camb_len);
             }
           else if (Vor->csftype[csf] == 3 )
             {
               for (k = 0 ; k < num_temp_camb  ; k++)
                 {
                   temp_camb[k] = 0.0; 
                 }
             }
 
        }

      for (j = 0 ; j < num_temp_camb-1 ; j++)
        {
          fprintf(slice_file,"%f ", temp_camb[j]);
        }
      fprintf(slice_file,"%f \n", temp_camb[j]);
    }


  fclose(slice_file);

  /* ---- Free control surface pointers ---- */
  free(num_csf_pntrs);
  free(temp_camb);
  ifree_2d(csf_pntrs, Vor->ntraps);
 
}

/*======================================================================*
* Module Name:  flap_slat_camb
*=======================================================================*
* Description: Compute effect of the control surface on the camber
*
* Input: 
*
* Output: 
*=======================================================================*/

void VorSlice::flap_slat_camb(int trap, int num_temp_camb, float* temp_camb, int csf, float camb_len)
{
  int i, hinge_ind; 
  float csf_angle, tan_angle, new_pt;
  float x1,y1,z1, x2,y2,z2, x3,y3,z3, x4,y4,z4, xavg, yavg, zavg;

  csf_angle = (Vor->csfdefangle[csf]*2.0f*3.14159f)/360.0f;
  tan_angle = (float)tan(csf_angle);

  /* for asymmetric deflections of symmetric control surfaces,
     change sign of left deflection */

  x1 = Vor->trap[trap][0][0]; y1 = Vor->trap[trap][0][1]; z1 = Vor->trap[trap][0][2];
  x2 = Vor->trap[trap][1][0]; y2 = Vor->trap[trap][1][1]; z2 = Vor->trap[trap][1][2];
  x3 = Vor->trap[trap][2][0]; y3 = Vor->trap[trap][2][1]; z3 = Vor->trap[trap][2][2];
  x4 = Vor->trap[trap][3][0]; y4 = Vor->trap[trap][3][1]; z4 = Vor->trap[trap][3][2];

  xavg = 0.25f*( x1 + x1 + x3 + x4 );
  yavg = 0.25f*( y1 + y1 + y3 + y4 );
  zavg = 0.25f*( z1 + z2 + z3 + z4 );
    
  if ( Vor->csfsymmetry[csf] == 0 ) {
          
     if ( ( swap_xy == 0 && yavg > 0. ) ||
          ( swap_xy == 1 && zavg < 0. ) ) {

        tan_angle *= -1.;

     }

  }
  
  /* FLAP */

  if (Vor->csftype[csf] == 0)
    {
      hinge_ind = (int)((float)num_temp_camb*(1.0 - Vor->csfchord[csf]));

      for (i = hinge_ind+1 ; i < num_temp_camb ; i++)
        {
          new_pt = (float)(hinge_ind - i)*camb_len*tan_angle +
                   temp_camb[i];

          temp_camb[i] = new_pt;
        }
    }

  /* SLAT */

  else if (Vor->csftype[csf] == 1)
    {
      hinge_ind = (int)((float)num_temp_camb*(Vor->csfchord[csf]));

      for (i = 0 ; i < hinge_ind ; i++)
        {
          new_pt = (float)(i - hinge_ind)*camb_len*tan_angle +
                   temp_camb[i];

          temp_camb[i] = new_pt;
        }
    }

  /* --- Calculate new camber values --- 
  for (i = 1 ; i < num_temp_camb-1 ; i++)
    {
       fract = (float)i/(float)(num_temp_camb-1);
       base_z = fract*temp_camb[num_temp_camb-1] +
                (1.0-fract)*temp_camb[0];

       temp_camb[i] = temp_camb[i] - base_z;
    }
  temp_camb[0] = 0.0;
  temp_camb[num_temp_camb-1] = 0.0; */
}

void VorSlice::reslice( int num_char, char* instr, int id ) 
{
  int numslc;
  char str[255];

  strncpy( str, instr, num_char );
  numslc = atoi( str );

  if ( numslc < 2 || numslc > 1000 )
  {
    setInfoMessage( "Incorrect number of slices.\n", 4000 );
    return;
  }
  set_nslice(numslc);
  
  compute_slice();
  write_trap(aircraftName);
  write_key_slice(aircraftName);
  write_key_point(aircraftName);
}
  


void VorSlice::recamber( int num_char, char* instr, int id ) 
{
  int numcamb;
  char str[255];

  strncpy( str, instr, num_char );
  numcamb = atoi( str );

  if ( numcamb < 0 || numcamb > 50 )
  {
    setInfoMessage( "Incorrect number of camber points.\n", 4000 );
    return;
  }
  Vor->ncamb = numcamb;
  
  compute_slice();
  write_trap(aircraftName);
  write_key_slice(aircraftName);
  write_key_point(aircraftName);


}


/*======================================================================*
* Module Name:  get_title_string 
*=======================================================================*
* Description: Get the number of camber points from user...
*
* Input: None
*
* Output: None
*=======================================================================*/
void VorSlice::getFirstTitleString( int num_char, char* str, int id )
{

  str[num_char] = '\0';
  sprintf( Vor->title1, "%s", str );
	        
  title_write(aircraftName);
  showtitle = 1;

  setInpMessage("  Enter Sub-Title String: \n");
  setInpType( INPUT_TEXT );
//jrg  setInpCBPtr( getSecondTitleString );

}

void VorSlice::getSecondTitleString( int num_char, char* str, int id )
{
  str[num_char] = '\0';
  sprintf( Vor->title2, "%s", str );

  title_write(aircraftName);
  showtitle = 1;

}

void VorSlice::init_trapezoid()
{
   trapezoid.temp = -1;
   trapezoid.point = -1;
}

/*
 * set (x,y,z) location of point
 */
void VorSlice::set_point(Point* p,float x,float y,float z)
{
   p->x = x;
   p->y = y;
   p->z = z;
}

/*
 * get (x,y,z) location of point
 */
void VorSlice::get_point(Point* p,float *x,float *y,float *z)
{
   *x = p->x;
   *y = p->y;
   *z = p->z;
}

/*
 * copy location of first point to second point
 */
void VorSlice::copy_point(Point *pfrom,Point *pto)
{
   pto->x = pfrom->x;
   pto->y = pfrom->y;
   pto->z = pfrom->z;
}

/*
 * return non-zero if Point contains the specified (x,y,z) location
 */
int VorSlice::equal_point(Point *p,float xyz[3])
{
   return(p->x == xyz[0] && p->y == xyz[1] && p->z == xyz[2]);
}

/*
 * if x in Point is greater than x in (x,y,z) location, return 1.
 * or if x's are equal, check z's; if z in Point is greater than z in (x,y,z)
 * location return 1.
 * Otherwise return 0.
 */
int VorSlice::greater_than_point(Point *p,float xyz[3])
{
   if (p->x > xyz[0])
      return(1);
   else if (p->x == xyz[0]) {
      if (p->z > xyz[2])
	 return(1);
      else
	 return(0);
   }
   else
      return(0);
}


/*======================================================================*
* Module Name:  zoomvor
*=======================================================================*
* Description: Scale all Vorview geometry
*
* Input:  zfactor	- Zoom Multiplier
*
* Output: None
*=======================================================================*/

void VorSlice::zoomvor(float zfactor)
{
/*
  int islc, i, j, icomp, isub;
  int ikey; // lep //
  float mult_fact;

  // --- Find Zoom Mult Factor ---//  
  zm_vor *= zfactor;
  mult_fact = zm_vor/Vor->scale;

  // --- Loop Through Components ----//
  for ( icomp = 0 ; icomp < Vor->ncomps ; icomp++)
    {
      for ( i = 0 ; i < Vor->ncross[icomp] ; i++)
        {
          for ( j = 0 ; j < Vor->npts[icomp] ; j++)
            {
             // --- Load Scaled Hermite Geom ----//
             Vor->sherm[icomp][i][j][0] = Vor->herm[icomp][i][j][0] - Vor->x_trans;
             Vor->sherm[icomp][i][j][0] *= mult_fact; 
             Vor->sherm[icomp][i][j][1] = Vor->herm[icomp][i][j][1] - Vor->y_trans;
             Vor->sherm[icomp][i][j][1] *= mult_fact;
             Vor->sherm[icomp][i][j][2] = Vor->herm[icomp][i][j][2]*mult_fact;
            }
        }
    }

  // --- Loop Through Trapizoids ----//
  for (islc = 0 ; islc < Vor->ntraps ; islc++)
    {
      for (i = 0 ; i < 4 ; i++)
        {
          // --- Load Scaled Trapizoid Geom ----//
          Vor->strap[islc][i][0] = Vor->trap[islc][i][0] - Vor->x_trans; 
          Vor->strap[islc][i][0] *= mult_fact; 
          Vor->strap[islc][i][1] = Vor->trap[islc][i][1] - Vor->y_trans; 
          Vor->strap[islc][i][1] *= mult_fact;
          Vor->strap[islc][i][2] = Vor->trap[islc][i][2];
          Vor->strap[islc][i][2] *= mult_fact; 
        }
// lep: for correct results, always call zoomcamb: //
#if 1
      zoomcamb(1.);
#else
      for (icamb = 0 ; icamb < Vor->ncamb ; icamb++)
        {
          // -- Load Scaled Camber Geom ----//
          Vor->scamb[islc][icamb] = Vor->camb[islc][icamb]*mult_fact;
        }
#endif

    }

  // --- Loop Through Subdivide Trapizoids ----//
  for (isub = 0 ; isub < Vor->nsubp ; isub++)
    {
      for (i = 0 ; i < 4 ; i++)
        {
          // --- Load Scaled SubPoly Geom ----//
          Vor->ssubp[isub][i][0] = Vor->subp[isub][i][0] - Vor->x_trans; 
          Vor->ssubp[isub][i][0] *= mult_fact; 
          Vor->ssubp[isub][i][1] = Vor->subp[isub][i][1] - Vor->y_trans; 
          Vor->ssubp[isub][i][1] *= mult_fact; 
          Vor->ssubp[isub][i][2] = Vor->subp[isub][i][2]; 
          Vor->ssubp[isub][i][2] *= mult_fact; 

        }
    }  

  // lep: loop thru key slices too... //
  key_slice.sxmin = key_slice.xmin - Vor->x_trans;
  key_slice.sxmin *= mult_fact;
  key_slice.sxmax = key_slice.xmax - Vor->x_trans;
  key_slice.sxmax *= mult_fact;
  key_slice.symin = key_slice.ymin - Vor->y_trans;
  key_slice.symin *= mult_fact;
  key_slice.symax = key_slice.ymax - Vor->y_trans;
  key_slice.symax *= mult_fact;
  key_slice.szavg = key_slice.zavg;
  key_slice.szavg *= mult_fact;
  for (ikey = 0; ikey < key_slice.nkey; ikey ++) {
    key_slice.sy[ikey] = key_slice.y[ikey] - Vor->y_trans;
    key_slice.sy[ikey] *= mult_fact;
  }

  // lep: do temp key slice too... //
  key_slice.sytemp = key_slice.ytemp - Vor->y_trans;
  key_slice.sytemp *= mult_fact;
*/
}

/*======================================================================*
* Module Name:  zoomcamb
*=======================================================================*
* Description: Scale camber data
*
* Input:  zfactor	- Zoom Multiplier
*
* Output: None
*=======================================================================*/

void VorSlice::zoomcamb(float zfactor)
{
/*
  int islc, icamb;
  float mult_fact;

  // --- Find Zoom Mult Factor ---//  
  zm_camb *= zfactor;
  mult_fact = zm_camb/Vor->scale;

  // --- Loop Through Slices ---- //
  for (islc = 0 ; islc < Vor->ntraps ; islc++)
    {
      for (icamb = 0 ; icamb < Vor->ncamb ; icamb++)
        {
          // ---- Load Scaled Camber Data ---- //
          Vor->scamb[islc][icamb] = Vor->camb[islc][icamb]*mult_fact;
        }

    }
*/
}


/*======================================================================*
* Module Name:  title_write
*=======================================================================*
* Description: Write Out Title Strings Into File
*
* Input:  flname 	- Filename
*
* Output: None
*=======================================================================*/
  
void VorSlice::title_write(char* flname)
{
  FILE *in_file;
  char tempfl[255];
  static char ttl[] = {".ttl"};

  int ierr;

  ierr = 0;

  /* ---- Add .ttl To Filename ---*/
  strcpy(tempfl, flname);
  strcat(tempfl, ttl);

  /* --- Open file --- */
  in_file = fopen(tempfl, "w");

  /* --- Write Out Titles ---- */
  fprintf(in_file, " %s \n", Vor->title1);
  fprintf(in_file, " %s \n", Vor->title2);

  fclose(in_file);

}


int VorSlice::info_message(const char *message, int pause_time)
{
printf("info_message %s\n", message );
  return 1;
}

/*======================================================================*
* Module Name:  drawit
*=======================================================================*
* Description: Draw the transformed geometry
*
* Input:  None
*
* Output: None
*=======================================================================*/

void VorSlice::drawit()
{

}

//==== jrg Calls to Fortran =====//
int VorSlice::vorinp_()
{
	return 0;
}

int VorSlice::srfprs_()
{
	return 0;
}

int VorSlice::clsstb_()
{
//      INTEGER FUNCTION CLSSTB()

//      ISOUT = 51
//      IFOUT = 52
      
//      CLOSE(UNIT=ISOUT)
//      CLOSE(UNIT=IFOUT)
      
//      RETURN
//      END
	return 0;
}


//==== jrg Removed Guts =====//

void VorSlice::setInfoMessage( const char* msg, int msecs )
{

}

void VorSlice::setInpCBPtr(  void (*fptr)(int, char*, int) )
{
}

void VorSlice::setInpType( int t )
{
}

void VorSlice::setInpID( int id )
{
}

void VorSlice::setInpMessage( const char* msg )
{
}

void VorSlice::setFractDone( float f )
{
}

void VorSlice::setFractDoneOff()
{
}

void VorSlice::setFractDoneOn()
{
}

int VorSlice::getFractDoneActive()
{
	return 0;
}


/*======================================================================*
* Module Name:  normherm
*=======================================================================*
* Description: Translate and Scale Hermite Geometry To Fit In Viewport
*
* Input:  None
*
* Output: None
*=======================================================================*/

void VorSlice::normherm()
{
/*******************************
  int icomp, i, j;
  float maxx, maxy, minx, miny; 
  float x, y, xdist, ydist;

  // --- Initalize Min/Max ---//
  minx = maxx = Vor->herm[0][0][0][0];
  miny = maxy = Vor->herm[0][0][0][1];

  // --- Loop Through Components ---//
  for ( icomp = 0 ; icomp < Vor->ncomps ; icomp++)
    {
      // ---- Loop Through Each X-Sec ---//
      for ( i = 0 ; i < Vor->ncross[icomp] ; i++)
        {
          // ---- Loop Through Each Point ---//
          for ( j = 0 ; j < Vor->npts[icomp] ; j++)
            {
              // ---- Find Min/Max ----//
              x = Vor->herm[icomp][i][j][0];
              y = Vor->herm[icomp][i][j][1];
              if (x > maxx) maxx = x;
              if (x < minx) minx = x;
              if (y > maxy) maxy = y;
              if (y < miny) miny = y;
            }
        }
    }

  // --- Find Translation Distance ----//
  xdist = maxx-minx;
  ydist = maxy-miny;

  Vor->x_trans = (maxx+minx)/2.0f;
  Vor->y_trans = (maxy+miny)/2.0f;

  // --- Find Scale Factor ---//
  if (xdist > (1.25*ydist)) 
    { Vor->scale = xdist; }
  else 
    { Vor->scale = ydist; }

  // --- Loop Through Components ----//
  for ( icomp = 0 ; icomp < Vor->ncomps ; icomp++)
    {
      // --- Loop Through Points and X-Sections ----//
      for ( i = 0 ; i < Vor->ncross[icomp] ; i++)
        {
          for ( j = 0 ; j < Vor->npts[icomp] ; j++)
            {
             // ---- Load Scaled Hermite Geometry ----//
             Vor->sherm[icomp][i][j][0] = Vor->herm[icomp][i][j][0] - Vor->x_trans;
             Vor->sherm[icomp][i][j][0] /= Vor->scale;
             Vor->sherm[icomp][i][j][1] = Vor->herm[icomp][i][j][1] - Vor->y_trans;
             Vor->sherm[icomp][i][j][1] /= Vor->scale;
             Vor->sherm[icomp][i][j][2] = Vor->herm[icomp][i][j][2]/Vor->scale;
            }
        }
    }
*******************/
 
}


/*======================================================================*
* Module Name:  normslice
*=======================================================================*
* Description: Translate and Scale Sliced Geometry To Fit In Viewport
*
* Input:  None
*
* Output: None
*=======================================================================*/

void VorSlice::normslice()
{ 
/************************
  int islc, i, icamb;

  // ---- Loop Through Trapizoids ----//
  for (islc = 0 ; islc < Vor->ntraps ; islc++)
    {
      // --- Scale and Translate Each Point ---//
      for (i = 0 ; i < 4 ; i++)
        {
          Vor->strap[islc][i][0] = Vor->trap[islc][i][0] - Vor->x_trans; 
          Vor->strap[islc][i][0] /= Vor->scale; 
          Vor->strap[islc][i][1] = Vor->trap[islc][i][1] - Vor->y_trans; 
          Vor->strap[islc][i][1] /= Vor->scale;
          Vor->strap[islc][i][2] = Vor->trap[islc][i][2]; 
          Vor->strap[islc][i][2] /= Vor->scale; 
        }

      // --- Scale and Translate Each Camber Point ---//
      for (icamb = 0 ; icamb < Vor->ncamb ; icamb++)
        {
          Vor->scamb[islc][icamb] = Vor->camb[islc][icamb]/Vor->scale;
        }
    }

// lep: loop thru key slices and scale & translate... //
   for (islc = 0; islc < key_slice.nkey; islc ++) {
      key_slice.sy[islc] = key_slice.y[islc] - Vor->y_trans;
      key_slice.sy[islc] /= Vor->scale;
   }
// lep: adjust other coords in key slice: //
   key_slice.sxmin = key_slice.xmin - Vor->x_trans;
   key_slice.sxmin /= Vor->scale;
   key_slice.sxmax = key_slice.xmax - Vor->x_trans;
   key_slice.sxmax /= Vor->scale;
   key_slice.symin = key_slice.ymin - Vor->y_trans;
   key_slice.symin /= Vor->scale;
   key_slice.symax = key_slice.ymax - Vor->y_trans;
   key_slice.symax /= Vor->scale;
   key_slice.szavg = key_slice.zavg;
   key_slice.szavg /= Vor->scale;

// lep: scale & translate temp key slice: //
   key_slice.sytemp = key_slice.ytemp - Vor->y_trans;
   key_slice.sytemp /= Vor->scale;
**************************************/
}


/*======================================================================*
* Module Name:  normsubpol
*=======================================================================*
* Description: Translate and Scale Subdivided Polygon Geometry 
*              To Fit In Viewport
*
* Input:  None
*
* Output: None
*=======================================================================*/

void VorSlice::normsubpol()
{
  int isub, i;

  /* ---- Loop Through Sub-Polygons ----*/
  for (isub = 0 ; isub < Vor->nsubp ; isub++)
    {
      /* --- Load Scaled Sub-Polygons ----*/
      for (i = 0 ; i < 4 ; i++)
        {
          Vor->ssubp[isub][i][0] = Vor->subp[isub][i][0] - Vor->x_trans; 
          Vor->ssubp[isub][i][0] /= Vor->scale; 
          Vor->ssubp[isub][i][1] = Vor->subp[isub][i][1] - Vor->y_trans; 
          Vor->ssubp[isub][i][1] /= Vor->scale; 
          Vor->ssubp[isub][i][2] = Vor->subp[isub][i][2]; 
          Vor->ssubp[isub][i][2] /= Vor->scale; 
        }
     }
}

/*======================================================================*
* Module Name:  title_write
*=======================================================================*
* Description: Read in Title Strings From File
*
* Input:  flname 	- Filename
*
* Output: None
*=======================================================================*/
  
int VorSlice::title_read(char* flname)
{
  FILE *in_file;
  char tempfl[255];
  static char ttl[] = {".ttl"};

  int ierr;

  ierr = 0;

  /* ---- Add .ttl To Filename ---*/
  strcpy(tempfl, flname);
  strcat(tempfl, ttl);

  /* --- Open file --- */

  if ( (in_file = fopen(tempfl, "r")) == (FILE *)NULL ) {
   
     ierr = 1;

     showtitle = 0;

  }

  else {

     /* --- Read in Titles ----*/

     fgets(Vor->title1,80,in_file);
     fgets(Vor->title2,80,in_file);

     fclose(in_file);

     showtitle = 1;

  }

  return(ierr);

}


/*======================================================================*
* Module Name:  minmax_cp
*=======================================================================*
* Description: Find min/max Cp values
*
* Input:  None
*
* Output: None
*=======================================================================*/

void VorSlice::minmax_cp()
{
  int i, ind;
  float fract;

  /* --- Initialize Min/Max Cp Values ---*/
  Vor->cp_min =  1.0e06;
  Vor->cp_max = -1.0e06;

  /* --- Check All Cp Values ---*/
  for (i = 0 ; i < Vor->nsubp ; i++)
    {
      if (Vor->cpval[i] < Vor->cp_min)
        Vor->cp_min = Vor->cpval[i];

      if (Vor->cpval[i] > Vor->cp_max)
        Vor->cp_max = Vor->cpval[i];
    }

  if ( Vor->cp_min == Vor->cp_max )
    Vor->cp_max += 0.001f;

  /* --- Load Cp Value Frequency Data ---*/

  for (i = 0 ; i < NUM_CP_FREQ ; i++) {
     Vor->cp_freq[i] = 0;
  }

  for (i = 0 ; i < Vor->nsubp ; i++)
    {
      fract = (Vor->cpval[i] - Vor->cp_min)/
              (Vor->cp_max - Vor->cp_min);

      ind = (int)(fract*(float)NUM_CP_FREQ*0.9999);
      Vor->cp_freq[ind]++;
    }
}

/*======================================================================*
* Module Name:  findrgbcolors
*=======================================================================*
* Description: Find color lookup table for the Cp values
*
* Input:  min_clamp_flag	- Change min clamp 
*	  max_clamp_flag	- Change max clamp
*
* Output: None
*=======================================================================*/

void VorSlice::findrgbcolors(int min_clamp_flag, int max_clamp_flag)
{
   float delt_cp;

  /* --- Find Delta Clamp Value ---*/
  delt_cp = (Vor->cp_surf_max - Vor->cp_surf_min)/100.0f;

  if (min_clamp_flag)
    {
      /* --- Change Min Clamp ----*/
      Vor->cp_surf_min_clamp += (float)min_clamp_flag*delt_cp;
    }

  if (max_clamp_flag)
    {
      /* --- Change Max Clamp ----*/
      Vor->cp_surf_max_clamp += (float)max_clamp_flag*delt_cp;
    }

}  

/*======================================================================*
* Module Name:  findcpcolors
*=======================================================================*
* Description: Find color lookup table for the Cp values
*
* Input:  min_clamp_flag	- Change min clamp 
*	  max_clamp_flag	- Change max clamp
*
* Output: None
*=======================================================================*/

void VorSlice::findcpcolors(int min_clamp_flag, int max_clamp_flag)
{
  short cvec[3];
  int i;
  float pcent, delt_clamp, delt_cp;

  /* --- Find Delta Clamp Value ---*/
  delt_cp = (Vor->cp_max - Vor->cp_min)/100.0f;

  if (min_clamp_flag)
    {
      /* --- Change Min Clamp ----*/
      Vor->min_clamp += (float)min_clamp_flag*delt_cp;
    }

  if (max_clamp_flag)
    {
      /* --- Change Max Clamp ----*/
      Vor->max_clamp += (float)max_clamp_flag*delt_cp;
    }
    
  delt_clamp = Vor->max_clamp - Vor->min_clamp;

  /* ---- Find Percentages For Each Cp Value ---*/
  for (i = 0 ; i < Vor->nsubp ; i++)
    {
 
      if (delt_clamp > 0.0) 
        {
          pcent = (Vor->cpval[i] - Vor->min_clamp)/delt_clamp;
        }
      else
        {
          pcent = 10.0;
        }
      /* ---- Find RGB Color For Percent ---*/      
      find_rgb(pcent, cvec);
  
      /* ---- Load Colors ----*/
      Vor->cpcol[i][0] = cvec[0];
      Vor->cpcol[i][1] = cvec[1];
      Vor->cpcol[i][2] = cvec[2];
    } 
  
}  

/*======================================================================*
* Module Name:  initclamps
*=======================================================================*
* Description: Set Initial Clamp Values
*
* Input:  None
*
* Output: None
*=======================================================================*/

void VorSlice::initclamps()
{
  float delt_cp, clfactor;
  int i, tot_freq, stop_flag;

  clfactor = 0.05f;
  delt_cp = Vor->cp_max - Vor->cp_min;
 
  tot_freq = 0; i = 0;
  stop_flag = 1;
 
  /* --- Total Up Cp Freqs ---*/
  while ((i < NUM_CP_FREQ) && (stop_flag) )
    {
      tot_freq += Vor->cp_freq[i];

      /* --- Check For Large Enough Freq Count  ----*/ 
      if ((float)tot_freq > clfactor*(float)Vor->nsubp)
        {
          /* --- Set Min Clamp ----*/
          Vor->min_clamp = ((float)i/(float)NUM_CP_FREQ)*delt_cp +
                      Vor->cp_min;

          if (Vor->min_clamp < Vor->cp_min) Vor->min_clamp = Vor->cp_min;

          stop_flag = 0;
        }
      i++;
     }

  tot_freq = 0; i = NUM_CP_FREQ-1;
  stop_flag = 1;

  while ( (i  >=  0) && (stop_flag) )
    {
      tot_freq += Vor->cp_freq[i];

      if ((float)tot_freq > clfactor*(float)Vor->nsubp)
        {
          Vor->max_clamp = ((float)(i+1)/(float)NUM_CP_FREQ)*delt_cp +
                      Vor->cp_min;

          if (Vor->max_clamp > Vor->cp_max) Vor->max_clamp = Vor->cp_max;

          stop_flag = 0;
        }
      i--;
     }

}

/*======================================================================*
* Module Name:  find_rgb
*=======================================================================*
* Description: Find color lookup table for the Cp values
*
* Input:  pcnt		- Fraction Between Clamps 
*
* Output: cvec		- Color Vector
*=======================================================================*/

void VorSlice::find_rgb(float pcent, short cvec[3])
{
  short red, green, blue;

  /* --- Initalize Color Components ----*/
  red = blue = green = 0;

  if (pcent < 0.0)
    {			/* --- Purple ---*/
            red   = 100;
            green = 0;
            blue  = 100;
    }
  else if (pcent < 0.20)
    {			/* --- Blue ---*/
            red   = (short)(5.0*(0.20-pcent)*255.0);
            green = 0;
            blue  = 255;
    }

  else if (pcent < 0.40)
    {			/* --- Lt Blue ---*/
            red   = 0;
            green = (short)((1.0 - 5.0*(0.40-pcent))*255.0);
            blue  = 255;
    }

  else if (pcent < 0.60)
    {			/* --- Green ---*/
            red   = 0;
            green = 255;
            blue  = (short)(5.0*(0.60-pcent)*255.0);
    }

  else if (pcent < 0.80)
    {			/* --- Yellow ----*/
            red   = (short)((1.0 - 5.0*(0.80-pcent))*255.0);
            green = 255;
            blue  = 0;
    }

  else if (pcent < 1.00)
    {			/* --- Red ----*/
            red   = 255;
            green = (short)(5.0*(1.0-pcent)*255.0);
            blue  = 0;
    }
  else
    {
            red   = 100;
            green = 0;
            blue  = 0;
    }

  /* --- Set Color Vector ---*/
  cvec[0] = red;
  cvec[1] = green;
  cvec[2] = blue;
}


/*======================================================================*
* Module Name:  trap_camb
*=======================================================================*
* Description: Compute camber for each trapzoid
*
* Input: bbox	- bounding box data for each component
*
* Output: None
*=======================================================================*/

void VorSlice::trap_camb(float **bbox)
{
  float *z_val, yloc, xloc, x1, x2, xdif;
  float z_min_max[2];
  static float tol = 0.0001f;
  int *exist_flag, itrap, icamb, icomp;
  int z_cnt;

  /* ---- Allocate Space For Arrays ---- */
  z_val = (float *)calloc(Vor->ncamb, sizeof(float));
  exist_flag = (int *)calloc(Vor->ncamb, sizeof(int));

  /* ---- Loop Through All Trapizoids ---- */  
  for ( itrap = 0 ; itrap < Vor->ntraps ; itrap++)
    {

       /* ---- Find Y Locations and X Values ---- */
       yloc = (Vor->trap[itrap][0][1] + Vor->trap[itrap][3][1])/2.0f;
       x1   = (Vor->trap[itrap][0][0] + Vor->trap[itrap][3][0])/2.0f + tol;
       x2   = (Vor->trap[itrap][1][0] + Vor->trap[itrap][2][0])/2.0f;
       xdif = x2 - x1 - tol;

       /* ---- Loop Thru Camber Locations ---- */
       for (icamb = 0 ; icamb < Vor->ncamb ; icamb++)
         {
 	    /* ---- Compute Current X Location and Init Z-Min/Max --- */
            xloc = x1 + ((float)icamb/(float)(Vor->ncamb - 1))*xdif;
            z_cnt = 0;
            z_min_max[0] = 10e06; z_min_max[1] = -10e06; 

	    /* ---- Loop Thru Components ---- */
            for (icomp = 0 ; icomp < Vor->ncomps ; icomp++)
              {

                 /* ---- Check Bounding Box for Possible Intersection --- */
                 if (chk_xy_box(bbox[icomp], xloc, yloc))
                   {
                     /* --- Find Min/Max Z Values --- */
                     min_max_z_val(icomp, xloc, yloc, &z_cnt, z_min_max);
                   }
              }

            if (z_cnt >= 1) 
              {
                  /* ---- Find Average Z Val ---- */
                  z_val[icamb] = (z_min_max[0] + z_min_max[1])/2.0f;
                  exist_flag[icamb] = 1;
              }
            else
              {
		  /* ---- Zero Z Value and Set Non-Exist Flag ---- */
                  z_val[icamb] = 0.0;
                  exist_flag[icamb] = 0;
              }
          }
      
        /*---- Find Leading Edge Z-Vals if Needed ---- */
        lead_z_vals(z_val, exist_flag);


        /*---- Find Trailing Edge Z-Vals if Needed ---- */
        trail_z_vals(z_val, exist_flag);
 
        /*---- Normalize Each Section of Z-vals ---- */
        normalize_z_vals(z_val, exist_flag); 

        /* ---- Compute the Mean-Line and Camber --- */
        for (icamb = 0 ; icamb < Vor->ncamb ; icamb++)
          {
            Vor->camb[itrap][icamb] = z_val[icamb];
          }
     }

   /* ---- Free Allocated Memory ---- */
   free(z_val);
   free(exist_flag);
}
       

/*======================================================================*
* Module Name:  min_max_z_val
*=======================================================================*
* Description: Find Minimum and Maximum Z Values at Xloc and Yloc
*
* Input: icomp		- Current Component
*	 xloc		- X-location
*	 yloc		- Y-location
*
* Output: z_cnt		- Number of Z Values
*	  z_min_max	- Min/max Z Values
*=======================================================================*/

void VorSlice::min_max_z_val(int icomp, float xloc, float yloc, int* z_cnt, float* z_min_max)
{
  float **tpts, z_val;
  int i, j, k; 

  tpts = falloc_2d(4,3);

 /* --- Loop Thru Points and X-Sections  ---- */
 for ( i = 0 ; i < Vor->ncross[icomp]-1 ; i++)
   {
     for ( j = 0 ; j < Vor->npts[icomp]-1 ; j++)
       {
         for ( k = 0 ; k < 3 ; k++)
           {
	     /* ---- Load Trapizoid Array --- */
             tpts[0][k] = Vor->herm[icomp][i][j][k];
             tpts[1][k] = Vor->herm[icomp][i+1][j][k];
             tpts[2][k] = Vor->herm[icomp][i+1][j+1][k];
             tpts[3][k] = Vor->herm[icomp][i][j+1][k];
           }

         /* --- Find Z Value For Poly ---- */
         if (find_poly_z(xloc, yloc, tpts, &z_val))
           {
	      /* ---- Increment Z Count and Check For Min/Max ---- */
              *z_cnt = *z_cnt + 1;
              if (z_val < z_min_max[0])
                z_min_max[0] = z_val;

              if (z_val > z_min_max[1])
                z_min_max[1] = z_val;
           }
        }
    }
  ffree_2d(tpts, 4);
}

/*======================================================================*
* Module Name:  lead_z_vals
*=======================================================================*
* Description: Compute slices for current surface model
*
* Input: z_val 		- Array of z_values
*	 exist_flag	- Array of existance flags
*
* Output: Z-values at leading edge
*=======================================================================*/

void VorSlice::lead_z_vals(float* z_val, int* exist_flag)
 {
   int camb_cnt, i;
   float delt_z, start_z;

   /* --- Initialize Camber Point Count ----*/
   camb_cnt = 0;

   /* --- Find First Good Camber Point --- */
   while((exist_flag[camb_cnt] == 0) && (camb_cnt < Vor->ncamb-2))
     {
       camb_cnt++;
     }

   /* ---- Compute Slope For Good Camber Points ----*/
   delt_z  = z_val[camb_cnt+1] - z_val[camb_cnt];
   start_z = z_val[camb_cnt];

   /* ---- Adjust Bad Camber Points --- */
   for (i = 0 ; i < camb_cnt ; i++)
     {
       z_val[i] = start_z - (float)(camb_cnt-i)*delt_z;
       exist_flag[i] = 1;
     }
 }

/*======================================================================*
* Module Name:  trail_z_vals
*=======================================================================*
* Description: Compute slices for current surface model
*
* Input: z_val 		- Array of z_values
*	 exist_flag	- Array of existance flags
*
* Output: Z-values at trailing edge
*=======================================================================*/

void VorSlice::trail_z_vals(float* z_val, int* exist_flag)
 {
   int camb_cnt, i;
   float delt_z, start_z;

   /* --- Initialize Camber Point Count ----*/
   camb_cnt = Vor->ncamb-1;

   /* --- Find First Good Camber Point --- */
   while((exist_flag[camb_cnt] == 0) && (camb_cnt > 1))
     {
       camb_cnt--;
     }

   /* ---- Compute Slope For Good Camber Points ----*/
   delt_z  = z_val[camb_cnt-1] - z_val[camb_cnt];
   start_z = z_val[camb_cnt];

   /* ---- Adjust Bad Camber Points --- */
   for (i = Vor->ncamb-1 ; i > camb_cnt ; i--)
     {
       z_val[i] = start_z - (float)(i-camb_cnt)*delt_z;
       exist_flag[i] = 1;
     }
}

/*======================================================================*
* Module Name:  normalize_z_vals
*=======================================================================*
* Description: Compute the camber from the list of Z values
*	       by connecting a line between the first and last
*	       point calculating the distance from the line to the 
*	       Z values.
*
* Input: z_val 		- Array of z_values
*	 exist_flag	- Array of existance flags
*
* Output: Camber Data
*
* NOTE: DJK, The camber data is non-dimesonalized in vorinp.f routines
* since vorlax wants this data as percent of local cord...
*
*=======================================================================*/
void VorSlice::normalize_z_vals(float* z_val, int* exist_flag)
 {
   int first_ind, last_ind, istop, icamb;
   float fract, ref_z;

   /* --- Initalize First Point Indicator ---*/
   first_ind = 0;

   /* --- Loop Thru all Camber Points ---*/
   for (istop = 1 ; istop < Vor->ncamb ; istop++)
     {
       /* ---- Check Existance Flag ---*/
       if ((exist_flag[istop] == 0) || (istop == Vor->ncamb-1))
         {
           if (istop == Vor->ncamb-1)
             last_ind = istop;
           else
             last_ind = istop - 1;
 
	   /* --- Loop From First Ind To Last ---- */
           for (icamb = first_ind+1 ; icamb < last_ind+1 ; icamb++)
             {
	       /* --- Compute Camber Value --- */
               fract = (float)icamb/(float)(last_ind-first_ind);
               ref_z = (1.0f-fract)*z_val[first_ind] + 
                       fract*z_val[last_ind];
               z_val[icamb] =  z_val[icamb] - ref_z;

             }
	   /* --- Set First and Last Point to Zero ----*/
           z_val[first_ind] = 0.0;
           z_val[last_ind] = 0.0;

	   /* --- Increment First Pointer ---- */
           first_ind = istop+1;
         }
       
      }
}

/*======================================================================*
* Module Name:  chk_xy_box
*=======================================================================*
* Description: Check Bounding Box For Possible Intersection
*
* Input: bbox	- bounding box data
*	 xloc	- X Location
*	 yloc 	- Y Location
*
* Output: Interference Indicator ( 0 or 1 )
*=======================================================================*/
int VorSlice::chk_xy_box(float* bbox, float xloc, float yloc)
{
  if ((xloc >= bbox[0]) && (xloc <= bbox[1]))
    {
      if ((yloc >= bbox[2]) && (yloc <= bbox[3]))
        {
           return(1);
        }
    }
  return(0);
}

/*======================================================================*
* Module Name:  find_poly_z
*=======================================================================*
* Description: Compute Z value for polygon at the x and y location
*
* Input: xloc	- X Value
*	 yloc	- Y Value
*	 tpts	- Polygon Points
*
* Output: zval	- Z Value
*=======================================================================*/
int VorSlice::find_poly_z(float xloc, float yloc,float** tpts,float* zval)
{
  int i, num_int, loop_cnt, new_z_flag;
  float xfract, xint[4], zint[4];
  new_z_flag = 0;

  /* ---- Stop if Point is Outside of Bounding Box ---*/

  if ((xloc < tpts[0][0]) && (xloc < tpts[1][0]) &&
      (xloc < tpts[2][0]) && (xloc < tpts[3][0]))
        {
          return(0);
        }
   else if ((xloc > tpts[0][0]) && (xloc > tpts[1][0]) && 
           (xloc > tpts[2][0]) && (xloc > tpts[3][0]))
    {
      return(0);
    }
  else if ((yloc < tpts[0][1]) && (yloc < tpts[1][1]) && 
           (yloc < tpts[2][1]) && (yloc < tpts[3][1]))
    {
      return(0);
    }
  else if ((yloc > tpts[0][1]) && (yloc > tpts[1][1]) && 
           (yloc > tpts[2][1]) && (yloc > tpts[3][1]))
    {
      return(0);
    }

  /* --- Check if the Point Lies on Any of the Polygons Vertices ---*/  
  for (i = 0 ; i < 4 ; i++)
    {
      if (xloc == tpts[i][0] && yloc == tpts[i][1])
        {
          *zval = tpts[i][2];
          return(1);
        }
    }

  num_int = 0;
  loop_cnt = 0;
  
  /* --- Find Intersection With Y Value ---- */ 
  while((num_int < 2) && (loop_cnt < 4)) 
    {
      find_y_int(yloc, tpts, loop_cnt, &num_int, xint, zint);
      loop_cnt++;
    }

  /* --- Check if There Were Intersections with the Y Value ---*/
  if (num_int >= 2)
    {
      if (xloc == xint[0] && xint[0] == xint[1])
        {
           *zval = zint[0];
           return(1);
        }

      /* --- Check X Values for Intersection ---*/
      if ((xloc >= xint[0]) && (xloc <= xint[1]))
        {
           xfract = (xloc-xint[0])/(xint[1]-xint[0]);
           *zval = xfract*(zint[1]-zint[0]) + zint[0];
           return(1);
        }
      else if ((xloc >= xint[1]) && (xloc <= xint[0]))
        {
           xfract = (xloc-xint[1])/(xint[0]-xint[1]);
           *zval = xfract*(zint[0]-zint[1]) + zint[1];
           return(1);
        }
    }
  return(0);
  
}
 
/*======================================================================*
* Module Name:  find_y_int
*=======================================================================*
* Description: Find Intersection Point For A Constant Y Line
*
* Input:  yloc		- Y Location of Line
*	  tpts		- Polygon Points
*	  ind1		- Point Indicator
*
* Output: num_int	- Number of Intersection
*	  xint		- X Intersection
*	  zint		- Z Intersection
*=======================================================================*/
void VorSlice::find_y_int(float yloc,float** tpts, int ind1, int* num_int, float* xint,float* zint)
{
  int ind2;
  float x1, x2, y1, y2, z1, z2;
  float yfract;
  
  /* --- Find Second Point Indicator ---*/
  ind2 = (ind1+1)%4;

  /* --- Find End Points of the Line ---*/
  x1 = tpts[ind1][0];  x2 = tpts[ind2][0];
  y1 = tpts[ind1][1];  y2 = tpts[ind2][1];
  z1 = tpts[ind1][2];  z2 = tpts[ind2][2];

  /* --- Find X and Z Intersection Values ---*/
  if (y1 != y2)
    {
      if ((yloc >= y1) && (yloc < y2)) 
        {
          yfract = (yloc-y1)/(y2-y1);
          xint[*num_int] = yfract*(x2-x1) + x1;
          zint[*num_int] = yfract*(z2-z1) + z1;
          *num_int = *num_int + 1;
        }
      else if ((yloc > y2) && (yloc <= y1)) 
        {
          yfract = (yloc-y2)/(y1-y2);
          xint[*num_int] = yfract*(x1-x2) + x2;
          zint[*num_int] = yfract*(z1-z2) + z2;
          *num_int = *num_int +1;
        }
    }
}

void VorSlice::clear()
{

	/* -------- Free Old Mem ----------- */
	for ( int comps = 0 ; comps < Vor->ncomps ; comps++ )
	{
			ffree_3d( Vor->herm[comps], Vor->ncross[comps], Vor->npts[comps] );
			Vor->herm[comps] = NULL;
	}
	if ( Vor->ncomps )
	{
			free( Vor->herm );		Vor->herm = NULL;
			free( Vor->ncross );	Vor->ncross = NULL;
			free( Vor->npts );		Vor->npts = NULL;
	}
	Vor->ncomps = 0;

	if ( Vor->ntraps )
	{
		ffree_3d( Vor->trap, Vor->ntraps, 4 );			Vor->trap = NULL;
		ffree_3d( Vor->ttrap, Vor->ntraps, 4 );			Vor->ttrap = NULL;

		if ( Vor->ncamb )
			ffree_2d( Vor->camb, Vor->ntraps );
		Vor->camb = NULL;
	}

	if ( Vor->nsubp )
	{
		ffree_3d( Vor->subp, Vor->nsubp, 4 );			Vor->subp = NULL;
		ffree_3d( Vor->ssubp, Vor->nsubp, 4 );			Vor->ssubp = NULL;
	}

	Vor->ntraps = 0;		//jrg fix leaks...
	Vor->nslices = 0;
	Vor->ncamb = 0;
	Vor->nsubp = 0;
	Vor->ncpv = 0;
	

}


/*======================================================================*
* Module Name:  readherm
*=======================================================================*
* Description: Read Hermite or cross-section data file
*
* Input: flname	-	File Name
*
* Output: None
*=======================================================================*/

int VorSlice::readherm(char* flname)
{
  FILE *in_file;
  int i, j, ierr;
  int comps;
  char buff[255];

  char tempfl[255];
  static char hrm[] = {".hrm"};

  strcpy(tempfl, flname);

  strcat(tempfl, hrm);

  ierr = 0;
  /* --- Make sure file exists --- */
  if ( (in_file = fopen(tempfl, "r")) == (FILE *)NULL )
   {
     printf("File not found ! \n");
     ierr = 1;
   }
  else if ( correct_file(in_file,"HERMITE") != 0 )     
   {
     printf("Incorrect File Type! \n");
     ierr = 1;
	 fclose(in_file);
   }
  else 
   {
     /* -------- Free Old Mem ----------- */
     for ( comps = 0 ; comps < Vor->ncomps ; comps++ )
     {
			ffree_3d( Vor->herm[comps], Vor->ncross[comps], Vor->npts[comps] );
	 }
	 if ( Vor->ncomps )
	 {
			free( Vor->herm );
			free( Vor->ncross );
			free( Vor->npts );
	 }

     /* -------- Read in number of components ----------- */
     fscanf(in_file," NUMBER OF COMPONENTS = %d\n",&(Vor->ncomps));
     Vor->herm    = (float ****)calloc(Vor->ncomps, sizeof(float ***));
     Vor->ncross  = (int *)calloc(Vor->ncomps, sizeof(int));
     Vor->npts    = (int *)calloc(Vor->ncomps, sizeof(int));
      
     /* ------- LOOP THROUGH THE ALL OF THE COMPONENTS --------- */
     for ( comps = 0 ; comps < Vor->ncomps ; comps++ )
       {
         fgets(buff, 80, in_file);
         fgets(buff, 80, in_file);
         fgets(buff, 80, in_file);

         fgets(buff, 80, in_file);
         sscanf(buff," CROSS SECTIONS = %d\n",&(Vor->ncross[comps]));

         fgets(buff, 80, in_file);
         sscanf(buff," PTS/CROSS SECTION = %d\n",&(Vor->npts[comps]));

         Vor->herm[comps]    = falloc_3d(Vor->ncross[comps], Vor->npts[comps], 4);
          
         /* ----- Read in point data ----- */
         for ( i = 0 ; i < Vor->ncross[comps] ; i++)
           {
             for ( j = 0 ; j < Vor->npts[comps] ; j++)
               {

                 fscanf(in_file,"%f %f %f\n",&(Vor->herm[comps][i][j][0]),
                                             &(Vor->herm[comps][i][j][1]),
                                             &(Vor->herm[comps][i][j][2]));
                                            
                 /* possible scaling of geometry if user requests it */
                 
                 if ( do_scaling ) {
                 
                    Vor->herm[comps][i][j][0] *= user_scale;
                    Vor->herm[comps][i][j][1] *= user_scale;
                    Vor->herm[comps][i][j][2] *= user_scale;
                    
                 }                        

               }
           }
        }
     fclose(in_file);
   }
  return(ierr);
}

/*======================================================================*
* Module Name:  readslice
*=======================================================================*
* Description: Read sliced or trapezoid data file
*
* Input: flname	-	File Name
*
* Output: None
*=======================================================================*/
      
int VorSlice::readslice(char* flname)
{
  FILE *in_file;
  int i, j, ierr;

  char tempfl[255];
  static char slc[] = {".slc"};

  ierr = 0;

  strcpy(tempfl, flname);
  strcat(tempfl, slc);


  /* --- Make sure file exists --- */
  if ( (in_file = fopen(tempfl, "r")) == (FILE *)NULL )
   {
     ierr = 1;
   }
  else
   {
     fscanf(in_file,"%d\n",&(Vor->ntraps));

     /*---- Allocate space for slices ----*/
     if (Vor->ntraps > 0) 
       {
         Vor->trap  = falloc_3d(Vor->ntraps, 4, 3);
         Vor->ttrap = falloc_3d(Vor->ntraps, 4, 3);

         for (i = 0; i < Vor->ntraps ; i++)
           {
 
             fscanf(in_file,"%f %f %f %f %f %f %f %f %f %f %f %f\n", 
		&(Vor->trap[i][0][0]), &(Vor->trap[i][0][1]), &(Vor->trap[i][0][2]), 
		&(Vor->trap[i][1][0]), &(Vor->trap[i][1][1]), &(Vor->trap[i][1][2]),
		&(Vor->trap[i][2][0]), &(Vor->trap[i][2][1]), &(Vor->trap[i][2][2]), 
		&(Vor->trap[i][3][0]), &(Vor->trap[i][3][1]), &(Vor->trap[i][3][2])); 

           }

        fscanf(in_file,"%d\n",&(Vor->ncamb));
        if (Vor->ncamb > 0)      
          {
            Vor->camb  = falloc_2d(Vor->ntraps, Vor->ncamb);

            for (i = 0 ; i < Vor->ntraps ; i++)
              {
                for (j = 0 ; j < Vor->ncamb-1 ; j++)
                  {
                    fscanf(in_file,"%f", &(Vor->camb[i][j]));
                  }
                fscanf(in_file,"%f\n", &(Vor->camb[i][j]));
              }
           }
		 else
		 {
			Vor->ncamb = 20;
            Vor->camb  = falloc_2d(Vor->ntraps, Vor->ncamb);
            for (i = 0 ; i < Vor->ntraps ; i++)
              {
                for (j = 0 ; j < Vor->ncamb-1 ; j++)
                {
 					Vor->camb[i][j] = 0.0;
				}
              }


		 }
        }

     fclose(in_file);
   }
  return(ierr);

}

/*======================================================================*
* Module Name:  readsubpol
*=======================================================================*
* Description: Read subdivided panel data file
*
* Input: flname	-	File Name
*
* Output: None
*=======================================================================*/

int VorSlice::readsubpol(char* flname)
{
  FILE *in_file;
  char tempfl[255];
  static char sub[] = {".sub"};

  int i,ierr;

  ierr = 0;

  strcpy(tempfl, flname);
  strcat(tempfl, sub);

  if ( (in_file = fopen(tempfl, "r")) == (FILE *)NULL )
   {
     ierr = 1;
   }
  else
   {
     fscanf(in_file,"%d\n",&(Vor->nsubp));
  
     if (Vor->nsubp > 0)
       {

         /*---- Allocate space for slices ----*/
        Vor->subp  = falloc_3d(Vor->nsubp, 4, 3 );
        Vor->ssubp = falloc_3d(Vor->nsubp, 4, 3 );

         for (i = 0; i < Vor->nsubp ; i++)
           {
             fscanf(in_file,"%f %f %f %f %f %f %f %f %f %f %f %f\n", 
		&(Vor->subp[i][0][0]), &(Vor->subp[i][0][1]), &(Vor->subp[i][0][2]), 
		&(Vor->subp[i][1][0]), &(Vor->subp[i][1][1]), &(Vor->subp[i][1][2]), 
		&(Vor->subp[i][2][0]), &(Vor->subp[i][2][1]), &(Vor->subp[i][2][2]), 
		&(Vor->subp[i][3][0]), &(Vor->subp[i][3][1]), &(Vor->subp[i][3][2])); 

           }
        }

     fclose(in_file);

   }
  return(ierr);

}

/*======================================================================*
* Module Name:  readcpvals
*=======================================================================*
* Description: Read Cp values from data file
*
* Input: flname	-	File Name
*
* Output: None
*=======================================================================*/

int VorSlice::readcpvals(char* flname)
{
  FILE *in_file;
  char tempfl[255];
  static char cpv[] = {".cpv"};

  int i, ierr, idum;
  float fdum;

  ierr = 0;

  strcpy(tempfl, flname);
  strcat(tempfl, cpv);

  if ( (in_file = fopen(tempfl, "r")) == (FILE *)NULL )
   {
     ierr = 1;
   }
  else
   {
	 if ( Vor->cpval ) free( Vor->cpval );
	 if ( Vor->cpcol ) free( Vor->cpcol );

     Vor->ncpv = Vor->nsubp;

     /*---- Allocate space for slices ----*/
     Vor->cpval = (float *)calloc(Vor->nsubp, sizeof(float));
     Vor->cpcol = (short **)calloc(Vor->nsubp, sizeof(short *));

     for (i = 0 ; i < Vor->nsubp ; i++)
       {
         Vor->cpcol[i] = (short *)calloc(3, sizeof(short *));
       } 


     for (i = 0; i < Vor->nsubp ; i++)
       {
         fdum = 0.0;
         if (!ferror(in_file))
           {
             fscanf(in_file,"%d %f \n", &(idum), &(fdum));
           }
         Vor->cpval[i] = fdum;
         
       }

     fscanf(in_file,"%d  \n", &(idum));
     Vor->final_num_sub = idum;
 
     fscanf(in_file,"%f %f %f %f \n", 
            &(Vor->sref), &(Vor->cbar), &(Vor->xbar), &(Vor->zbar));

     fscanf(in_file,"%f %f \n", &(Vor->final_alpha), &(Vor->final_mach));

     fscanf(in_file,"%f %f %f %f %f %f %f %d\n", &(Vor->cltot), 
                                                 &(Vor->cdtot),
                                                 &(Vor->cmtot),
                                                 &(Vor->ep),
                                                 &(Vor->cltrf),
                                                 &(Vor->cdtrf),
                                                 &(Vor->et),
                                                 &(Vor->iters));

      fscanf(in_file,"%f\n",&(Vor->cdf)); 
	  fclose ( in_file );

    }

  return(ierr);
}

/*======================================================================*
* Module Name:  readcsf
*=======================================================================*
* Description: Read control surface parameters from data file
*
* Input: flname	-	File Name
*
* Output: None
*=======================================================================*/

int VorSlice::readcsf(char* flname)
{
  FILE *in_file;
  char tempfl[255];
  static char csf[] = {".csf"};

  int ierr, i, j;

  ierr = 0;

  strcpy(tempfl, flname);
  strcat(tempfl, csf);

  /* --- Allocate Space for Control Surface Data ---- */
/********
  Vor->ncsftrap    = (int *)calloc(MAX_CSF, sizeof(int));
  Vor->csftype     = (int *)calloc(MAX_CSF, sizeof(int));
  Vor->csfreflect  = (int *)calloc(MAX_CSF, sizeof(int));
  Vor->csfdefangle = (float *)calloc(MAX_CSF, sizeof(float));
  Vor->csfchord    = (float *)calloc(MAX_CSF, sizeof(float));
  Vor->csftrap     = (int **)calloc(MAX_CSF, sizeof(int *));
  Vor->csfsymmetry = (int *)calloc(MAX_CSF, sizeof(int));
  Vor->csfhingepercent = (float *)calloc(MAX_CSF, sizeof(float));

  Vor->csfname = (char **) calloc(MAX_CSF, sizeof(char*));

  for ( i = 0 ; i < MAX_CSF ; i++ ) {

     Vor->csfname[i] = (char *) calloc(80, sizeof(char));

  }
********/

  if ( (in_file = fopen(tempfl, "r")) == (FILE *)NULL )
   {
     ierr = 1;
   }
  else
   {
     fscanf(in_file,"%d %d\n",&(Vor->ntotcsf), &(Vor->csfcheck));

     /*---- Allocate space for slices ----*/
     if ((Vor->ntotcsf <= 0) || (Vor->csfcheck != Vor->ntraps)) 
       {
         Vor->ntotcsf = 0;
         ierr = 1;
       }
     else
       {
         for (i = 0 ; i < Vor->ntotcsf ; i++)
           {
             fscanf(in_file,"%d \n", &(Vor->ncsftrap[i]));
//             Vor->csftrap[i] = (int *)calloc(Vor->ntraps, sizeof(int));

             for (j = 0 ; j < Vor->ncsftrap[i]-1 ; j++)
               {
                 fscanf(in_file,"%d ", &(Vor->csftrap[i][j]));
               }
             
             fscanf(in_file,"%d \n", &(Vor->csftrap[i][j]));
             fscanf(in_file,"%d %d %f %f %d %f %s \n", 
                    &(Vor->csftype[i]), 
                    &(Vor->csfreflect[i]),
                    &(Vor->csfdefangle[i]),
                    &(Vor->csfchord[i]),
                    &(Vor->csfsymmetry[i]),
                    &(Vor->csfhingepercent[i]),
                    Vor->csfname[i]);
                  
           }
        }
	  fclose(in_file);
    }
  return(ierr);
}

      
/*======================================================================*
* Module Name:  correct_file
*=======================================================================*
* Description: Checks for appropriate header to file
*
* Input: FILE pointer and expected heading
*
* Output: Flag (1 = yes, 0 = no)
*=======================================================================*/

 int VorSlice::correct_file(FILE* in_file, const char* header)
 {
   char string[255];

   /* --- Read first Line of file and compare against expected header --- */
   fscanf(in_file,"%s INPUT FILE\n\n",string);

   return(strcmp(header,string));
 }

/*======================================================================*
* Module Name:  write_flags
*=======================================================================*
* Description: Write Flags information to file
*
* Input: flname	-	File Name
*
* Output: None
*=======================================================================*/

int VorSlice::write_flags(char* flname)
{
  FILE *in_file;
  char tempfl[255];
  static char csf[] = {".flg"};

  int ierr;

  ierr = 0;

  strcpy(tempfl, flname);
  strcat(tempfl, csf);

  /* --- Make sure file exists --- */
  if ( (in_file = fopen(tempfl, "w")) != (FILE *)NULL ) {
   
     fprintf(in_file,"%d %d %d \n", key_slice.symmetry_flag,Vor->flat_flag,swap_xy);

     fclose(in_file);

     return(1);

  }

  else {

     return(0);

  }

}

/*======================================================================*
* Module Name:  read_flags
*=======================================================================*
* Description: Read Flags information to file
*
* Input: flname	-	File Name
*
* Output: None
*=======================================================================*/

int VorSlice::read_flags(char* flname)
{
  FILE *in_file;
  char tempfl[255];
  static char csf[] = {".flg"};

  int ierr, i, j, comps, old_swap_xy;
 
  float yt, zt;

  ierr = 0;

  strcpy(tempfl, flname);
  strcat(tempfl, csf);

  old_swap_xy = swap_xy;

  /* --- Make sure file exists --- */
  if ( (in_file = fopen(tempfl, "r")) != (FILE *)NULL ) {
   
     fscanf(in_file,"%d %d %d \n", &key_slice.symmetry_flag,&(Vor->flat_flag),&swap_xy);

     fclose(in_file);

     if ( swap_xy != old_swap_xy ) {

        for ( comps = 0 ; comps < Vor->ncomps ; comps++ ) {

           for ( i = 0 ; i < Vor->ncross[comps] ; i++) {

              for ( j = 0 ; j < Vor->npts[comps] ; j++) {

                 yt = Vor->herm[comps][i][j][1];
                 zt = Vor->herm[comps][i][j][2];

                 Vor->herm[comps][i][j][1] = zt;
                 Vor->herm[comps][i][j][2] = yt;

              }

           }

        }

     }

     normherm();

     return(1);

  }

  else {

     return(0);

  }

}

//======================================================================
//======================================================================
//======================================================================
//======================================================================
//======================================================================
//======================================================================
//======================================================================
//======================================================================
//======================================================================
//======================================================================
//======================================================================
//======================================================================
//======================================================================
//======================================================================
//======================================================================




/*======================================================================*
* Module Name:  add_del_csf_trap
*=======================================================================*
* Description: Add or delete trapizoid from current control surface
*
* Input:  curr	- Current Control Surface
*	  trap	- Trapizoid Number
*
* Output: None
*=======================================================================*/

void VorSlice::add_del_csf_trap(int curr, int trap)
{ 
  int add_csf, del_csf, itrap, rtrap;
 
  /* ---- Initalize Add and Delete Flags ---- */ 
  add_csf = 1;
  del_csf = 0;

  /* ---- Loop Through Trapizoids ---- */
  for (itrap = 0 ; itrap < Vor->ncsftrap[curr] ; itrap++)
    {
      /* ---- Check For Delete Flag ---*/
      if (del_csf)
        {
          Vor->csftrap[curr][itrap-1] = Vor->csftrap[curr][itrap];
        }

      /* ---- Check if Trap Picked is Already in Control Surface List */ 
      else if (trap == Vor->csftrap[curr][itrap])
        {
          del_csf = 1;
          add_csf = 0;
        }
    }

  if (del_csf)   /* ---- Check For Delete Flag ---*/
    {
      /* --- Decrease Number of Traps In Current Control Surf ---*/
      Vor->ncsftrap[curr]--;

      /* --- Check For Reflect Flag ----*/
      if (Vor->csfreflect[curr])
        {
          del_csf = 0;

          /* ---- Find Reflected Trap Index ----*/
          rtrap = Vor->ntraps - trap - 1;
          for (itrap = 0 ; itrap < Vor->ncsftrap[curr] ; itrap++)
            {
              /* ---- Check For Delete Flag ---*/
              if (del_csf)
                {
                  Vor->csftrap[curr][itrap-1] = Vor->csftrap[curr][itrap];
                } 
              /* ---- Check if Trap Picked is Already in Control Surface List */ 
              else if (rtrap == Vor->csftrap[curr][itrap])
                {
                  del_csf = 1;
                } 
            }
          /* ---- If Delete Flag Decrease Number of Traps ----*/
          if (del_csf)
            {
              Vor->ncsftrap[curr]--;
            }

        }
    }

   /* ---- Check For Add To Control Surface Flag ---- */
   else if (add_csf)
    {
      /*---- Increase Number Of Trap and Add Trap To List ----*/
      Vor->ncsftrap[curr]++;
      Vor->csftrap[curr][itrap] = trap;

      /*---- Check For Reflect Flag ---- */
      if (Vor->csfreflect[curr])
        {
          /* ---- Find Reflected Trap ---- */
          rtrap = Vor->ntraps - trap - 1;
          for (itrap = 0 ; itrap < Vor->ncsftrap[curr] ; itrap++)
            {
              if (rtrap == Vor->csftrap[curr][itrap])
                {
                  add_csf = 0;
                } 
            }
          /* ---- If Add Flag - Increase Number Of Trap And Add Trap ---*/
          if (add_csf)
            {
              Vor->ncsftrap[curr]++;
              Vor->csftrap[curr][itrap] = rtrap;
            }
             
        }
      
    }

}

/*======================================================================*
* Module Name:  add_csf
*=======================================================================*
* Description: Add Control Surface 
*
* Input:  None 
*
* Output: None
*=======================================================================*/
void VorSlice::add_csf()
{
	if ( Vor->ntraps <= 0 )
		return;

  /* ---- Check If Number Of Control Surfaces Is Less Then Max ---- */
  if (Vor->ntotcsf < (MAX_CSF - 1))
    {
       /* --- Allocate Space For List of Traps ----*/
       Vor->csftrap[Vor->ntotcsf] = (int *)calloc(Vor->ntraps, sizeof(int));

       /* --- Set Default Values For Control Surface ---*/       
       Vor->ncsftrap[Vor->ntotcsf] = 0;
       Vor->csftype[Vor->ntotcsf] = 0;
       Vor->csfreflect[Vor->ntotcsf] = 1;
       Vor->csfdefangle[Vor->ntotcsf] = 0.0;
       Vor->csfchord[Vor->ntotcsf] = 0.25;
       Vor->csfsymmetry[Vor->ntotcsf] = 1;
       Vor->csfhingepercent[Vor->ntotcsf] = 0.;

       sprintf(Vor->csfname[Vor->ntotcsf],"CSF_%d",Vor->ntotcsf); 


       /* --- Change Current Control Surface To New One ---*/
       Vor->curr_csf = Vor->ntotcsf;
 
       Vor->ntotcsf++;
    }
}

/*======================================================================*
* Module Name:  del_csf
*=======================================================================*
* Description: Add Control Surface 
*
* Input:  None 
*
* Output: None
*=======================================================================*/
void VorSlice::del_csf()
{
  int icsf, del_flag;

  /* --- Check If There Is a Control Surface To Delete ----*/
  if (Vor->ntotcsf > 0)
  {
    del_flag = 0;

    /* ---- Loop Through Control Surfaces ----*/
    for (icsf = 0 ; icsf < Vor->ntotcsf ; icsf++)
      {
         /* ---- Check If Trap In List Match The One To Be Deleted ----*/
         if (icsf == Vor->curr_csf)
           {
             del_flag = 1;
           }
         else if (del_flag)
           {
             /* ---- Delete Control Surface Parameters ---*/
             
             Vor->csftrap[icsf-1] = Vor->csftrap[icsf];
       
             Vor->ncsftrap[icsf-1]    = Vor->ncsftrap[icsf];
             Vor->csftype[icsf-1]     = Vor->csftype[icsf];
             Vor->csfreflect[icsf-1]  = Vor->csfreflect[icsf];
             Vor->csfdefangle[icsf-1] = Vor->csfdefangle[icsf];
             Vor->csfchord[icsf-1]    = Vor->csfchord[icsf];
             Vor->csfsymmetry[icsf-1] = Vor->csfsymmetry[icsf];
             Vor->csfhingepercent[icsf-1] = Vor->csfhingepercent[icsf];

             /* Copy over text! Not the pointer (which was what
                Pierce had done */
                
             sprintf(Vor->csfname[icsf-1],"%s",Vor->csfname[icsf]);

           }
       }
    /* ---- Set Current Control Surface And Decrease Number Of CSF --*/
    Vor->curr_csf = 0;
    Vor->ntotcsf--;
   }
}

/*======================================================================*
* Module Name:  flap_write
*=======================================================================*
* Description: Write Out Control Surface Info Into File
*
* Input:  flname 	- Filename
*
* Output: None
*=======================================================================*/
void VorSlice::flap_write(char* flname)
 {
  FILE *in_file;
  char tempfl[255];
  static char csf[] = {".csf"};

  int ierr, i, j;

  ierr = 0;

  Vor->csfcheck = Vor->ntraps;

  /* ---- Add .csf To Filename ---*/
  strcpy(tempfl, flname);
  strcat(tempfl, csf);

  /* --- Open file --- */
  in_file = fopen(tempfl, "w");

  /* --- Write Out Number Of Control Surfaces ----*/
  fprintf(in_file, " %d %d\n", Vor->ntotcsf, Vor->ntraps);

  /* --- Loop Through All Control Surfaces ----*/
  for (i = 0 ; i < Vor->ntotcsf ; i++)
    {

      /* ---- Write Out Number Of Traps ----*/
      fprintf(in_file,"%d \n",Vor->ncsftrap[i]);

      /* ---- Write Out Trap Indices ----*/
      for (j = 0 ; j < Vor->ncsftrap[i]-1 ; j++)
        {
          fprintf(in_file,"%d ", Vor->csftrap[i][j]);
        }
      fprintf(in_file,"%d \n", Vor->csftrap[i][j]);

      /* ---- Write Out Parameters ----*/
      fprintf(in_file,"%d %d %f %f %d %f %s \n",
              Vor->csftype[i], 
              Vor->csfreflect[i],
              Vor->csfdefangle[i],
              Vor->csfchord[i],
              Vor->csfsymmetry[i],
              Vor->csfhingepercent[i],
              Vor->csfname[i]);
                  
    }

  fclose(in_file);

}

void  VorSlice::set_curr_trap( int ind )
{
	if ( ind >= 0 && ind < Vor->ntraps )
		curr_trap = ind;
}
	
int  VorSlice::get_curr_trap()
{
	if ( curr_trap >= 0 && curr_trap < Vor->ntraps )
		return curr_trap;
	else
		return -1;
}

void  VorSlice::set_curr_csf( int id )
{
	if ( id >= 0 && id < Vor->ntotcsf )
		Vor->curr_csf = id;
}
	
int  VorSlice::get_curr_csf()
{
	return Vor->curr_csf;
}


/*======================================================================*
* Module Name:  set_color
*=======================================================================*
* Description: Set Color For Current Control Surface
*
* Input:  index	- Control Surface Index
*
* Output: None
*=======================================================================*/
void  VorSlice::get_csf_color(int index, int* r, int* g, int* b)
{
  static int num_col = 15;
  static int rdd[] = {255,   0,   0, 255,   0, 255, 198, 113, 142, 113, 142,  56,  0, 63,  0};   
  static int grn[] = {  0, 255,   0, 255, 255,   0, 113, 198, 142, 113,  56, 142, 72, 36, 36};
  static int blu[] = {  0,   0, 255,   0, 255, 255, 113, 113,  56, 198, 142, 142,  0,  0, 63};

  /* ---- Check If Index Is Valid ----*/
  if (index >= 0 && index < num_col)
    {
	  *r = rdd[index];
	  *g = grn[index];
	  *b = blu[index];
    }

  /* --- Incorrect Index - Set Color To Black ----*/
  else
    {
	  *r = 0;
	  *g = 0;
	  *b = 0;
    }
}
