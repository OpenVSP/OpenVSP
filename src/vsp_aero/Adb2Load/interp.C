#include "interp.H"

// The code...

/*##############################################################################
#                                                                              #
#                         VSP_GEOM constructor                                 #
#                                                                              #
##############################################################################*/

INTERP::INTERP(void)
{

    Symmetry            = 0;
    CmToMeters          = 0;
    SwapNormals         = 0;
    AnchorFile          = 0;
    IgnoreBox           = 0;
    StrictInterpolation = 0;

}

/*##############################################################################
#                                                                              #
#                            VSP_GEOM Copy                                     #
#                                                                              #
##############################################################################*/

INTERP::INTERP(const INTERP &Interp)
{

    printf("INTERP copy not implemented! \n");fflush(NULL);exit(1);

}

/*##############################################################################
#                                                                              #
#                                 Interpolate                                  #
#                                                                              #
##############################################################################*/

void INTERP::Interpolate(INTERP_MESH *Mesh1, INTERP_MESH *Mesh2)
{

    int i;

    // Calculate centroid locations and normals for each mesh

    CalculateCentroids(Mesh1); CalculateNormals(Mesh1); CalculateBoundingBox(Mesh1);

    CalculateCentroids(Mesh2); CalculateNormals(Mesh2); CalculateBoundingBox(Mesh2);

    // If user tells us to swap normals we do it for mesh 1, assuming mesh 2
    // is the correct one...

    if ( SwapNormals ) SwapSurfaceNormals(Mesh1);

    // Create area weighted nodal values on Mesh 1

    CalculateNodalValues(Mesh1);

    // Interpolate solution from Mesh1 onto Mesh2 ...

    InterpolateSolution(Mesh1, Mesh2);

}

/*##############################################################################
#                                                                              #
#                                  Limiter2D                                   #
#                                                                              #
##############################################################################*/

float INTERP::Limiter2D(float Value, float Val1, float Val2, float Val3)
{

   float LimValue, MinVal, MaxVal;

   MinVal = MIN3(Val1, Val2, Val3);
   MaxVal = MAX3(Val1, Val2, Val3);

   LimValue = MIN(MAX(Value, MinVal), MaxVal);

   return LimValue;

}

/*##############################################################################
#                                                                              #
#                               SOLVER CalculateBoundingBox                    #
#                                                                              #
##############################################################################*/

void INTERP::CalculateBoundingBox(INTERP_MESH *Mesh)
{

    int j, nod1, nod2, nod3;

    // Calculate centroids for each tri

    Mesh->MinX =  1.e9;
    Mesh->MaxX = -1.e9;

    Mesh->MinY =  1.e9;
    Mesh->MaxY = -1.e9;

    Mesh->MinZ =  1.e9;
    Mesh->MaxZ = -1.e9;

    for ( j = 1 ; j <= Mesh->number_of_nodes ; j++ ) {

	   Mesh->MinX = MIN(Mesh->MinX,Mesh->NodeList[j].x);
	   Mesh->MaxX = MAX(Mesh->MaxX,Mesh->NodeList[j].x);

	   Mesh->MinY = MIN(Mesh->MinY,Mesh->NodeList[j].y);
	   Mesh->MaxY = MAX(Mesh->MaxY,Mesh->NodeList[j].y);

	   Mesh->MinZ = MIN(Mesh->MinZ,Mesh->NodeList[j].z);
	   Mesh->MaxZ = MAX(Mesh->MaxZ,Mesh->NodeList[j].z);

    }

    printf("X min,max: %f, %f \n",Mesh->MinX,Mesh->MaxX);
    printf("Y min,max: %f, %f \n",Mesh->MinY,Mesh->MaxY);
    printf("Z min,max: %f, %f \n",Mesh->MinZ,Mesh->MaxZ);

}

/*##############################################################################
#                                                                              #
#                               SOLVER CalculateCentroids                      #
#                                                                              #
##############################################################################*/

void INTERP::CalculateCentroids(INTERP_MESH *Mesh)
{

    int j, nod1, nod2, nod3;

    // Calculate centroids for each tri

    for ( j = 1 ; j <= Mesh->number_of_tris ; j++ ) {

	   nod1 = Mesh->TriList[j].node1;
	   nod2 = Mesh->TriList[j].node2;
	   nod3 = Mesh->TriList[j].node3;

	   Mesh->TriList[j].x = ( Mesh->NodeList[nod1].x
	                      +   Mesh->NodeList[nod2].x
	                      +   Mesh->NodeList[nod3].x )/3.;

	   Mesh->TriList[j].y = ( Mesh->NodeList[nod1].y
	                      +   Mesh->NodeList[nod2].y
	                      +   Mesh->NodeList[nod3].y )/3.;

	   Mesh->TriList[j].z = ( Mesh->NodeList[nod1].z
	                      +   Mesh->NodeList[nod2].z
	                      +   Mesh->NodeList[nod3].z )/3.;

    }

}

/*##############################################################################
#                                                                              #
#                               SOLVER CalculateNormals                        #
#                                                                              #
##############################################################################*/

void INTERP::CalculateNormals(INTERP_MESH *Mesh)
{

    int j, nod1, nod2, nod3;
    float xp[3], yp[3], zp[3], vec1[3], vec2[3], normal[3], area, Area, Volume;

    // Calculate centroids for each tri

    Volume = 0.;

    Area = 0.;

    for ( j = 1 ; j <= Mesh->number_of_tris ; j++ ) {

	   nod1 = Mesh->TriList[j].node1;
	   nod2 = Mesh->TriList[j].node2;
	   nod3 = Mesh->TriList[j].node3;

       xp[0] = Mesh->NodeList[nod1].x;
       yp[0] = Mesh->NodeList[nod1].y;
       zp[0] = Mesh->NodeList[nod1].z;

       xp[1] = Mesh->NodeList[nod2].x;
       yp[1] = Mesh->NodeList[nod2].y;
       zp[1] = Mesh->NodeList[nod2].z;

       xp[2] = Mesh->NodeList[nod3].x;
       yp[2] = Mesh->NodeList[nod3].y;
       zp[2] = Mesh->NodeList[nod3].z;
       
       vec1[0] = xp[1] - xp[0];
       vec1[1] = yp[1] - yp[0];
       vec1[2] = zp[1] - zp[0];

       vec2[0] = xp[2] - xp[0];
       vec2[1] = yp[2] - yp[0];
       vec2[2] = zp[2] - zp[0];

       vector_cross(vec1,vec2,normal);

       area = 0.5*sqrt(vector_dot(normal,normal));

       Mesh->TriList[j].nx = 0.5*normal[0]/area;
       Mesh->TriList[j].ny = 0.5*normal[1]/area;
       Mesh->TriList[j].nz = 0.5*normal[2]/area;

       Mesh->TriList[j].area = area;

       Volume += (xp[0] + xp[1] + xp[2])*normal[0]/3.;

       Area += area;

    }

    printf("Volume: %f \n",Volume);
    printf("Area: %f \n",Area);

}

/*##############################################################################
#                                                                              #
#                             SOLVER SwapSurfaceNormals                        #
#                                                                              #
##############################################################################*/

void INTERP::SwapSurfaceNormals(INTERP_MESH *Mesh)
{

    int j;

    // Swap the surface normals

    for ( j = 1 ; j <= Mesh->number_of_tris ; j++ ) {

       Mesh->TriList[j].nx *= -1.;
       Mesh->TriList[j].ny *= -1.;
       Mesh->TriList[j].nz *= -1.;

    }

}

/*##############################################################################
#                                                                              #
#                               SOLVER CalculateNodalValues                    #
#                                                                              #
##############################################################################*/

void INTERP::CalculateNodalValues(INTERP_MESH *Mesh)
{

    int i, j, nod1, nod2, nod3, InterpTri;
    float *Weight, x1, y1, z1, x2, y2, z2, Area, Mag;
    float ds, dsMin, u, v, vec1[3], vec2[3], vec3[3], N1, N2, N3;
    float Cp, Cp_Unsteady, Gamma;

    // Calculate nodal values of solution for Mesh 1

    Weight = new float[Mesh->number_of_nodes + 1];

    for ( j = 1 ; j <= Mesh->number_of_nodes ; j++ ) {

	    Mesh->NodeList[j].Cp          = 0.;
	    Mesh->NodeList[j].Cp_Unsteady = 0.;
	    Mesh->NodeList[j].Gamma       = 0.;

       Weight[j] = 0.;

    }

    for ( j = 1 ; j <= Mesh->number_of_tris ; j++ ) {

	    nod1 = Mesh->TriList[j].node1;
	    nod2 = Mesh->TriList[j].node2;
	    nod3 = Mesh->TriList[j].node3;
       
	    Area = Mesh->TriList[j].area;
       
	    Cp          = Mesh->TriList[j].Cp;
	    Cp_Unsteady = Mesh->TriList[j].Cp_Unsteady;
	    Gamma       = Mesh->TriList[j].Gamma;
      
	    Mesh->NodeList[nod1].Cp          += Area * Cp;
	    Mesh->NodeList[nod1].Cp_Unsteady += Area * Cp_Unsteady;
	    Mesh->NodeList[nod1].Gamma       += Area * Gamma;
       
	    Mesh->NodeList[nod2].Cp          += Area * Cp;
	    Mesh->NodeList[nod2].Cp_Unsteady += Area * Cp_Unsteady;
	    Mesh->NodeList[nod2].Gamma       += Area * Gamma;
       
       Mesh->NodeList[nod3].Cp          += Area * Cp;
	    Mesh->NodeList[nod3].Cp_Unsteady += Area * Cp_Unsteady;
	    Mesh->NodeList[nod3].Gamma       += Area * Gamma;
 
       Weight[nod1] += Area;
       Weight[nod2] += Area;
       Weight[nod3] += Area;

    }

    for ( j = 1 ; j <= Mesh->number_of_nodes ; j++ ) {

	    Mesh->NodeList[j].Cp          /= Weight[j];
	    Mesh->NodeList[j].Cp_Unsteady /= Weight[j];
       Mesh->NodeList[j].Gamma       /= Weight[j];

    }

    delete [] Weight;

}

/*##############################################################################
#                                                                              #
#                           SOLVER CalculateCentroidValues                     #
#                                                                              #
##############################################################################*/

void INTERP::CalculateCentroidValues(INTERP_MESH *Mesh)
{

    int j ,nod1, nod2, nod3;

    // Calculate triangle values

    for ( j = 1 ; j <= Mesh->number_of_tris ; j++ ) {

	   nod1 = Mesh->TriList[j].node1;
	   nod2 = Mesh->TriList[j].node2;
	   nod3 = Mesh->TriList[j].node3;

	   Mesh->TriList[j].Cp          = ( Mesh->NodeList[nod1].Cp          + Mesh->NodeList[nod2].Cp          + Mesh->NodeList[nod3].Cp          )/3.;
	   Mesh->TriList[j].Cp_Unsteady = ( Mesh->NodeList[nod1].Cp_Unsteady + Mesh->NodeList[nod2].Cp_Unsteady + Mesh->NodeList[nod3].Cp_Unsteady )/3.;
	   Mesh->TriList[j].Gamma       = ( Mesh->NodeList[nod1].Gamma       + Mesh->NodeList[nod2].Gamma       + Mesh->NodeList[nod3].Gamma       )/3.;

    }

}

/*##############################################################################

                          Function InterpolateSolution

Function Description:

The function transfers the surface pressures from the CFD mesh to the FEM mesh.

Coded By: David J. Kinney
    Date: 12 - 24 - 1997

##############################################################################*/

void INTERP::InterpolateSolution(INTERP_MESH *Mesh1, INTERP_MESH *Mesh2)
{

    int k, n, p, nod, *did_this_node, iter, max_iter, closest, max_radius;
    int OutOfBox, NormalOff, Done, SymShear, node1, node2, node3;
    double tol_x, tol_y, tol_z, Tolerance, yc;
    LEAF  *root;
    TNODE *node;

    /* copy over mach, q, alpha data */

    Mesh2->num_Mach  = Mesh1->num_Mach;
    Mesh2->num_Bars	= Mesh1->num_Bars;
    Mesh2->num_Alpha = Mesh1->num_Alpha;

    Mesh2->MachList  = Mesh1->MachList;
    Mesh2->BarsList  = Mesh1->BarsList;
    Mesh2->AlphaList = Mesh1->AlphaList;

    /* create the CFD binary search tree */

    printf("Creating the binary search tree ... \n");

    root = create_cfd_tree(Mesh1);


    /* search criteria */

    closest = 0;

    max_radius = 0;

    max_iter = 1;


    /* search the binary tree */

    node = (TNODE *) calloc(1, sizeof(TNODE));

    closest = 0;

    printf("Working on tris ... \n");

    printf("Mesh2->number_of_tris: %d \n",Mesh2->number_of_tris);

    OutOfBox = NormalOff = 0;

    tol_x = 0.01*ABS(Mesh1->MaxX - Mesh1->MinX);
    tol_y = 0.01*ABS(Mesh1->MaxY - Mesh1->MinY);
    tol_z = 0.01*ABS(Mesh1->MaxZ - Mesh1->MinZ);

    Tolerance = 2.*MAX3(tol_x,tol_y,tol_z);

    printf("Tolerance: %f \n",Tolerance);

    for ( k = 1 ; k <= Mesh2->number_of_tris ; k++ ) {

       if ( k - 500*(int)(k/500.) == 0 ) {

          printf("Finished %d tris \r",k);

          fflush(NULL);

       }

       node->xyz[0] = Mesh2->TriList[k].x;
       node->xyz[1] = Mesh2->TriList[k].y;
       node->xyz[2] = Mesh2->TriList[k].z;

       node->normal[0] = Mesh2->TriList[k].nx;
       node->normal[1] = Mesh2->TriList[k].ny;
       node->normal[2] = Mesh2->TriList[k].nz;

       node->area = Mesh2->TriList[k].area;
       
       node->DonorArea = 0.;

       SymShear = 0;

       if ( Symmetry == 1 && node->xyz[1] < 0. ) {

	      node->xyz[1] *= -1.;
	      node->normal[1] *= -1.;

	      SymShear = 1;

	   }

       if ( Symmetry == 2 && node->xyz[1] > 0. ) {

	      node->xyz[1] *= -1.;
	      node->normal[1] *= -1.;

	      SymShear = 1;

	   }

	   // Check that we are within the bounding box of the first mesh

	   if ( IgnoreBox ||
	        ( ( node->xyz[0] - Mesh1->MinX ) >= -tol_x && ( Mesh1->MaxX - node->xyz[0] ) >= -tol_x &&
  	          ( node->xyz[1] - Mesh1->MinY ) >= -tol_y && ( Mesh1->MaxY - node->xyz[1] ) >= -tol_y &&
	          ( node->xyz[2] - Mesh1->MinZ ) >= -tol_z && ( Mesh1->MaxZ - node->xyz[2] ) >= -tol_z    ) ) {

 	      node->found = 0;

 	      node->ignore_normals = 0;

 	      Done = 0;

 	      while ( !Done ) {

	         node->search_radius = 1;

	         iter = 1;

	         while ( iter <= max_iter && node->found != 1 ) {

                node->found = 0;

                node->distance = 1.e20;

                node->normal_distance = 1.e20;

                search_tree(root,node,Tolerance);

                if ( node->found != 1 && iter < max_iter ) {

                   node->search_radius += 50;

                }

                iter++;

	         }

            if ( node->found == 2 ) closest++;

            max_radius = MAX(max_radius,node->search_radius);

	         if ( node->found == 0 ) {

		   	   if ( node->ignore_normals == 1 ) {

	               printf("Error in finding interpolation donor! \n");

	               exit(1);

               }

               if ( StrictInterpolation ) {

  			         OutOfBox++;

			         for ( p = 0 ; p <= 16 ; p++ ) {

                     node->Variable[p] = -999. * 0.;

			         }

			         node->InterpNode[0] = 0;
			         node->InterpNode[1] = 0;
			         node->InterpNode[2] = 0;

			         node->InterpWeight[0] = 0.;
			         node->InterpWeight[1] = 0.;
			         node->InterpWeight[2] = 0.;

			         Done = 1;

               }
               
               else {
                  
			         node->ignore_normals = 1;

                  NormalOff++;
               
               }                  

		     }

		     else {

		        Done = 1;

		     }

	      }

	   }

	   else {

		  OutOfBox++;

	      for ( p = 0 ; p <= 16 ; p++ ) {

		     node->Variable[p] = -999. * 0.;

		  }

		  node->InterpNode[0] = 0;
		  node->InterpNode[1] = 0;
		  node->InterpNode[2] = 0;

		  node->InterpWeight[0] = 0.;
		  node->InterpWeight[1] = 0.;
		  node->InterpWeight[2] = 0.;

	   }

	   // Check symmetry constraints

	   if ( Symmetry != 0 ) {

	      node1 = node->InterpNode[0];
	      node2 = node->InterpNode[1];
	      node3 = node->InterpNode[2];

          yc = ( Mesh1->NodeList[node1].y
               + Mesh1->NodeList[node2].y
               + Mesh1->NodeList[node2].y );

	      if ( node->xyz[1] * yc < 0 && ABS(yc) > tol_y ) {

	         OutOfBox++;

	         for ( p = 0 ; p <= 16 ; p++ ) {

	            node->Variable[p] = -999. * 0.;

	         }

	         node->InterpNode[0] = 0;
	         node->InterpNode[1] = 0;
	         node->InterpNode[2] = 0;

	         node->InterpWeight[0] = 0.;
	         node->InterpWeight[1] = 0.;
	         node->InterpWeight[2] = 0.;


		  }

	   }
      
       if ( ( node->found == 1 && node->normal_distance > 3.*sqrt(node->DonorArea) ) ||
            ( node->found == 2 && node->distance        > 2.*sqrt(node->DonorArea) ) ) {

	       OutOfBox++;
          
	       for ( p = 0 ; p <= 16 ; p++ ) {
          
	          node->Variable[p] = -999. * 0.;
          
	       }
                    
       }
       
	    Mesh2->TriList[k].InterpNode[0] = node->InterpNode[0];
	    Mesh2->TriList[k].InterpNode[1] = node->InterpNode[1];
	    Mesh2->TriList[k].InterpNode[2] = node->InterpNode[2];
       
	    Mesh2->TriList[k].InterpWeight[0] = node->InterpWeight[0];
	    Mesh2->TriList[k].InterpWeight[1] = node->InterpWeight[1];
	    Mesh2->TriList[k].InterpWeight[2] = node->InterpWeight[2];

       Mesh2->TriList[k].Cp          = node->Variable[ 0];
       Mesh2->TriList[k].Cp_Unsteady = node->Variable[ 1];
       Mesh2->TriList[k].Gamma       = node->Variable[ 2];

    }

    printf("Finished %d tris \n",k);

    printf("Used closest point for %d nodes \n",closest);

    printf("Turn off normal contraints for %d nodes \n",NormalOff);

    printf("Used a maximum radius of %d \n",max_radius);

    if ( OutOfBox > 0 ) printf("There were %d nodes on mesh 2 are outside of the bounding box of mesh 1 ! \n",OutOfBox);

    free(node);

}

/*##############################################################################
#                                                                              #
#                               SOLVER WriteADBFile                            #
#                                                                              #
##############################################################################*/

void INTERP::WriteADBFile(INTERP_MESH *Mesh, char *Name)
{

    FILE *adb_file;
    BINARYIO BIO;
    int i_size, c_size, f_size, DumInt, i, j, k, m;
    char file_name_w_ext[200];

    // Sizeof ints and floats

    i_size = sizeof(int);
    c_size = sizeof(char);
    f_size = sizeof(float);

    // Open the aerothermal data base file

    sprintf(file_name_w_ext,"%s.interp.adb",Name);

    if ( (adb_file = fopen(file_name_w_ext,"wb")) == NULL ) {

       printf("Could not open the file: %s for input! \n",file_name_w_ext);

       exit(1);

    }

    // Write out the default in to check on endianess

    DumInt = -123789456 + 4; // Version 4


    BIO.fwrite(&DumInt, i_size, 1, adb_file);

    // Write out header

    BIO.fwrite(&(Mesh->number_of_nodes), i_size, 1, adb_file);
    BIO.fwrite(&(Mesh->number_of_tris),  i_size, 1, adb_file);
    BIO.fwrite(&(Mesh->num_Mach),        i_size, 1, adb_file);
    BIO.fwrite(&(Mesh->num_Bars),        i_size, 1, adb_file);
    BIO.fwrite(&(Mesh->num_Alpha),       i_size, 1, adb_file);
    BIO.fwrite(&(Mesh->num_Beta),        i_size, 1, adb_file);
    BIO.fwrite(&(Mesh->Sref),            f_size, 1, adb_file);
    BIO.fwrite(&(Mesh->Cref),            f_size, 1, adb_file);
    BIO.fwrite(&(Mesh->Bref),            f_size, 1, adb_file);
    BIO.fwrite(&(Mesh->Xcg),             f_size, 1, adb_file);
    BIO.fwrite(&(Mesh->Ycg),             f_size, 1, adb_file);
    BIO.fwrite(&(Mesh->Zcg),             f_size, 1, adb_file);

    // Write out the Mach, Q, and Alpha lists

    for ( k = 1 ; k <= Mesh->num_Mach  ; k++ ) BIO.fwrite(&(Mesh->MachList[k]),  f_size, 1, adb_file);
    for ( k = 1 ; k <= Mesh->num_Bars  ; k++ ) BIO.fwrite(&(Mesh->BarsList[k]),  f_size, 1, adb_file);
    for ( k = 1 ; k <= Mesh->num_Alpha ; k++ ) BIO.fwrite(&(Mesh->AlphaList[k]), f_size, 1, adb_file);
    for ( k = 1 ; k <= Mesh->num_Beta  ; k++ ) BIO.fwrite(&(Mesh->BetaList[k]), f_size, 1, adb_file);

    // Control surface deflection list
    
    BIO.fwrite(&(Mesh->NumberOfControlSurfaces), i_size, 1, adb_file);
           
    for ( k = 1 ; k <= Mesh->NumberOfControlSurfaces ; k++ ) {
       
       BIO.fwrite(&(Mesh->NumberOfControlSurfaceDeflections[k]), i_size, 1, adb_file);

       for ( i = 1 ; i <= Mesh->NumberOfControlSurfaceDeflections[k] ; i++ ) {
          
          BIO.fwrite(&(Mesh->ControlSurfaceDeflection[k][i]), f_size, 1, adb_file);

       }
       
    }

    printf("Writing out geometry... \n");

    // Write out the geometry information

    for ( i = 1 ; i <= Mesh->number_of_tris ; i++ ) {

       // Geometry

       BIO.fwrite(&(Mesh->TriList[i].node1),       i_size, 1, adb_file);
       BIO.fwrite(&(Mesh->TriList[i].node2),       i_size, 1, adb_file);
       BIO.fwrite(&(Mesh->TriList[i].node3),       i_size, 1, adb_file);
       BIO.fwrite(&(Mesh->TriList[i].surface_id),  i_size, 1, adb_file);
       BIO.fwrite(&(Mesh->TriList[i].area),        f_size, 1, adb_file);

       // Surface ID flag and emissivity

       BIO.fwrite(&(Mesh->TriList[i].material_id), i_size, 1, adb_file);
       BIO.fwrite(&(Mesh->TriList[i].emissivity),  f_size, 1, adb_file);

    }

    for ( i = 1 ; i <= Mesh->number_of_nodes ; i++ ) {

       BIO.fwrite(&(Mesh->NodeList[i].x), f_size, 1, adb_file);
       BIO.fwrite(&(Mesh->NodeList[i].y), f_size, 1, adb_file);
       BIO.fwrite(&(Mesh->NodeList[i].z), f_size, 1, adb_file);

    }

    // Scale factor that was used

    BIO.fwrite(&(Mesh->ScaleFactor), f_size, 1, adb_file);

    // Planet ID

    BIO.fwrite(&(Mesh->Planet), i_size, 1, adb_file);

    // Symmetry flag

    BIO.fwrite(&(Mesh->WriteOutHalfGeometry), i_size, 1, adb_file);

    // Write out the aerothermal data base one solution at a time

    fclose(adb_file);

}

