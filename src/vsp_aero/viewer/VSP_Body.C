//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Body.H"

/*##############################################################################
#                                                                              #
#                               VSP_BODY constructor                           #
#                                                                              #
##############################################################################*/

VSP_BODY::VSP_BODY(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                                   VSP_BODY init                              #
#                                                                              #
##############################################################################*/

void VSP_BODY::init(void)
{

    Verbose_ = 0;
    
}

/*##############################################################################
#                                                                              #
#                                  VSP_BODY Copy                               #
#                                                                              #
##############################################################################*/

VSP_BODY::VSP_BODY(const VSP_BODY &VSP_Wing)
{

    init();

    // Just * use the operator = code

    *this = VSP_Wing;

}

/*##############################################################################
#                                                                              #
#                           VSP_BODY operator=                                 #
#                                                                              #
##############################################################################*/

VSP_BODY& VSP_BODY::operator=(const VSP_BODY &VSP_Wing)
{

    return *this;

}

/*##############################################################################
#                                                                              #
#                        VSP_BODY destructor                                   #
#                                                                              #
##############################################################################*/

VSP_BODY::~VSP_BODY(void)
{


}

/*##############################################################################
#                                                                              #
#                      VSP_BODY SizeGeometryLists                              #
#                                                                              #
##############################################################################*/

void VSP_BODY::SizeGeometryLists(int NumI, int NumJ)
{

    NumGeomI_ = NumI;
    NumGeomJ_ = NumJ;
    
    x_ = new double[NumGeomI_*NumGeomJ_ + 1];
    y_ = new double[NumGeomI_*NumGeomJ_ + 1];
    z_ = new double[NumGeomI_*NumGeomJ_ + 1];    
    
    u_ = new double[NumGeomI_*NumGeomJ_ + 1];
    v_ = new double[NumGeomI_*NumGeomJ_ + 1];    
    
}

/*##############################################################################
#                                                                              #
#                      VSP_BODY SizeFlatPlateLists                             #
#                                                                              #
##############################################################################*/

void VSP_BODY::SizeFlatPlateLists(int NumI, int NumJ)
{

    NumPlateI_ = NumI;
    NumPlateJ_ = NumJ;
    
    x_plate_ = new double[NumPlateI_*NumPlateJ_ + 1];
    y_plate_ = new double[NumPlateI_*NumPlateJ_ + 1];
    z_plate_ = new double[NumPlateI_*NumPlateJ_ + 1];    
    
    Nx_plate_ = new double[NumPlateI_*NumPlateJ_ + 1];
    Ny_plate_ = new double[NumPlateI_*NumPlateJ_ + 1];
    Nz_plate_ = new double[NumPlateI_*NumPlateJ_ + 1];    
    
    LocalChord_ = new double[NumPlateI_ + 1];    
    
}

/*##############################################################################
#                                                                              #
#                             VSP_BODY ReadFile                                #
#                                                                              #
##############################################################################*/

void VSP_BODY::ReadFile(char *Name, int Case, FILE *VSP_Degen_File)
{
 
    int i, j, NumI, NumJ, Wing, Done;
    double DumFloat;
    char VSP_File_Name[2000], DumChar[2000], Stuff[2000];

    // Save the component name

    sprintf(ComponentName_,"%s",Name);
    
    // Slice type
    
    SliceType_ = Case;
    
    // Read in the wing data
    
    fgets(DumChar,1000,VSP_Degen_File);
    fgets(DumChar,1000,VSP_Degen_File); sscanf(DumChar,"SURFACE_NODE,%d,%d",&NumI,&NumJ);
    fgets(DumChar,1000,VSP_Degen_File);
    
    if ( Verbose_ ) printf("NumI, NumJ: %d %d \n",NumI,NumJ);

    Wing = 1;
    
    SizeGeometryLists(NumI,NumJ);
 
    for ( i = 1 ; i <= NumI ; i++ ) {
     
       for ( j = 1 ; j <= NumJ ; j++ ) {
        
          fgets(DumChar,1000,VSP_Degen_File);  
          
          sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %s",
                 &x(i,j),
                 &y(i,j),
                 &z(i,j),
                 &u(i,j),
                 &v(i,j),
                 Stuff);


          if ( Verbose_ ) printf("xyz: %lf %lf %lf %lf %lf \n",
                                 x(i,j),
                                 y(i,j),
                                 z(i,j),
                                 u(i,j),
                                 v(i,j));
          
       }
       
    }

    // Skip over data until we find the plate data
    
    Done = 0;
    
    while ( !Done ) {
    
       fgets(DumChar,1000,VSP_Degen_File);
       
       if ( strstr(DumChar,"PLATE") != NULL ) Done = 1;
       
    }
    
    if ( SliceType_ == HORIZONTAL ) {
     
    Done = 0;
    
       while ( !Done ) {
       
          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strstr(DumChar,"PLATE") != NULL ) Done = 1;
          
       }
    
    }
    
    // Read in the plate surface size
    
    sscanf(DumChar,"PLATE,%d,%d",&NumI,&NumJ);
    
    if ( Verbose_ ) printf("NumI, NumJ: %d %d \n",NumI,NumJ);
    
    SizeFlatPlateLists(NumI,NumJ);
    
    // Skip over normals information
    
    for ( i = 1 ; i <= NumI + 2 ; i++ ) fgets(DumChar,1000,VSP_Degen_File);  
     
    // Now read in the flat plate representation of the wing
 
    for ( i = 1 ; i <= NumI ; i++ ) {
  
       for ( j = 1 ; j <= NumJ ; j++ ) {
         
          fgets(DumChar,1000,VSP_Degen_File);  
   
          sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %s",
                 &x_plate(i,j),
                 &y_plate(i,j),
                 &z_plate(i,j),
                 &DumFloat,
                 &DumFloat,
                 &Nx_plate(i,j),
                 &Ny_plate(i,j),
                 &Nz_plate(i,j),
                 Stuff);

          if ( Verbose_ ) printf("Plate xyz: %lf %lf %lf %lf %lf %lf %s \n",
                                  x_plate(i,j),
                                  y_plate(i,j),
                                  z_plate(i,j),
                                  Nx_plate(i,j),
                                  Ny_plate(i,j),
                                  Nz_plate(i,j),
                                  Stuff);
          
       }
       
       // Fudge the first and last normals...
     
       Nx_plate(i,1) = Nx_plate(i,2);
       Ny_plate(i,1) = Ny_plate(i,2);
       Nz_plate(i,1) = Nz_plate(i,2);
       
       Nx_plate(i,NumJ) = Nx_plate(i,NumJ-1);
       Ny_plate(i,NumJ) = Ny_plate(i,NumJ-1);
       Nz_plate(i,NumJ) = Nz_plate(i,NumJ-1);
       
    }    
   
    CheckForDegenerateXSections();
    fflush(NULL);
}

/*##############################################################################
#                                                                              #
#                   VSP_BODY CheckForDegenerateXSections                       #
#                                                                              #
##############################################################################*/

void VSP_BODY::CheckForDegenerateXSections(void)
{
 
    int i, j, ii, *BadXSection, NumBadXSections, NumI;
    double  Distance, Tolerance;
    double *x_plate_new, *y_plate_new, *z_plate_new;
    double *Nx_plate_new, *Ny_plate_new, *Nz_plate_new;
    double *LocalChord_new;
    
    Tolerance = 1.e-7;
    
    printf("Checking for degenerate x-sections... \n");fflush(NULL);
    
    // Determine if the any two x-sections are at the same x-location
        
    BadXSection = new int[NumPlateI_ + 1];
    
    zero_int_array(BadXSection, NumPlateI_);
    
    NumBadXSections = 0;
    
    for ( i = 1 ; i < NumPlateI_ ; i++ ) {    
       
       Distance = 0.;
     
       for ( j = 1 ; j <= NumPlateJ_ ; j++ ) {

          Distance += pow(x_plate(i,j)-x_plate(i+1,j),2.);
        
       }
    
       Distance /= NumPlateJ_;

       if ( Distance <= Tolerance ) {
        
          printf("X sections %d and %d are the same... located at x: %lf \n",i,i+1,x_plate(i,1));fflush(NULL);
          
          BadXSection[i] = 1;
          
          NumBadXSections++;
          
       }
        
    }
   
    // If we have and x-sections at the same station... delete one
    
    if ( NumBadXSections > 0 ) {
     
       NumI = NumPlateI_ - NumBadXSections;
     
       printf("NumI: %d \n",NumI);fflush(NULL);
       
       x_plate_new = new double[NumI*NumPlateJ_ + 1];
       y_plate_new = new double[NumI*NumPlateJ_ + 1];
       z_plate_new = new double[NumI*NumPlateJ_ + 1];    
       
       Nx_plate_new = new double[NumI*NumPlateJ_ + 1];
       Ny_plate_new = new double[NumI*NumPlateJ_ + 1];
       Nz_plate_new = new double[NumI*NumPlateJ_ + 1];    
       
       LocalChord_new = new double[NumI + 1];    

       ii = 0;
       
       for ( i = 1 ; i <= NumPlateI_ ; i++ ) {
        
          if ( i == 1 || i == NumPlateI_ || !BadXSection[i] ) {
           
             ii++;
           
             for ( j = 1 ; j <= NumPlateJ_ ; j++ ) {
              
                x_plate_new[(ii-1)*NumPlateJ_ + j] = x_plate_[(i-1)*NumPlateJ_ + j];
                y_plate_new[(ii-1)*NumPlateJ_ + j] = y_plate_[(i-1)*NumPlateJ_ + j];
                z_plate_new[(ii-1)*NumPlateJ_ + j] = z_plate_[(i-1)*NumPlateJ_ + j];
    
                Nx_plate_new[(ii-1)*NumPlateJ_ + j] = Nx_plate_[(i-1)*NumPlateJ_ + j];
                Ny_plate_new[(ii-1)*NumPlateJ_ + j] = Ny_plate_[(i-1)*NumPlateJ_ + j];
                Nz_plate_new[(ii-1)*NumPlateJ_ + j] = Nz_plate_[(i-1)*NumPlateJ_ + j];
                
             }

             LocalChord_new[ii] = LocalChord_[i];

          }
          
       }
       
       NumPlateI_ = NumI;
       
       delete[] x_plate_;
       delete[] y_plate_;
       delete[] z_plate_;
       
       delete[] Nx_plate_;
       delete[] Ny_plate_;
       delete[] Nz_plate_;
       
       delete[] LocalChord_;
       
       x_plate_ = x_plate_new;
       y_plate_ = y_plate_new;
       z_plate_ = z_plate_new;
       
       Nx_plate_ = Nx_plate_new;
       Ny_plate_ = Ny_plate_new;
       Nz_plate_ = Nz_plate_new;
       
       LocalChord_ = LocalChord_new;
    
    }    
   
    delete [] BadXSection;    
    
}

/*##############################################################################
#                                                                              #
#                             VSP_BODY CreateMesh                              #
#                                                                              #
##############################################################################*/

void VSP_BODY::CreateMesh(int SurfaceID)
{
 
    int i, j, n, nk, NumNodes, NumTris, node1, node2, node3, node4, Flipped;
    int NodeA, NodeB, NodeC;
    double vec1[3], vec2[3], vec3[3], normal[3], mag;
    double x1, y1, z1, x2, y2, z2, Nx, Ny, Nz, Distance, Tolerance;
    
    // Trailing edge runs in the I direction...

    NumberOfKuttaNodes_ = NumPlateI_;

    KuttaNode_ = new int[NumberOfKuttaNodes_ + 1];
    
    WakeTrailingEdgeX_ = new double[NumberOfKuttaNodes_ + 1];
    WakeTrailingEdgeY_ = new double[NumberOfKuttaNodes_ + 1];
    WakeTrailingEdgeZ_ = new double[NumberOfKuttaNodes_ + 1];    
    
    printf("NumberOfKuttaNodes_: %d \n",NumberOfKuttaNodes_);fflush(NULL);
    
    Tolerance = 1.e-7;
    
    // Determine if the body is open at the nose
    
    Distance = 0.;
    
    i = 1;
    
    for ( j = 2 ; j <= NumPlateJ_ ; j++ ) {

       Distance += ( pow(x_plate(i,j)-x_plate(i,1),2.) + 
                     pow(y_plate(i,j)-y_plate(i,1),2.) + 
                     pow(z_plate(i,j)-z_plate(i,1),2.) );
       
    }
    
    Distance /= NumPlateJ_;
    
    NoseIsClosed_ = 0;
    
    if ( Distance <= Tolerance ) NoseIsClosed_ = 1;
    
    if ( NoseIsClosed_ ) printf("NoseIsClosed... \n");
    
    // Determine if the body is open at the tail
    
    Distance = 0.;
    
    i = NumPlateI_;
    
    for ( j = 2 ; j <= NumPlateJ_ ; j++ ) {

       Distance += ( pow(x_plate(i,j)-x_plate(i,1),2.) + 
                     pow(y_plate(i,j)-y_plate(i,1),2.) + 
                     pow(z_plate(i,j)-z_plate(i,1),2.) );
       
    }
       
    Distance /= NumPlateJ_;

    TailIsClosed_ = 0;
    
    if ( Distance <= Tolerance ) TailIsClosed_ = 1;
    
    if ( TailIsClosed_ ) printf("TailIsClosed... \n");
    
    // Calculate total number of nodes and tris, including the wake

    NumNodes = NumPlateI_*NumPlateJ_;
    
    NumTris  = 2*(NumPlateI_-1)*(NumPlateJ_-1);

printf("Max possible tri: %d \n",NumTris);

    if ( NoseIsClosed_ ) NumTris -= ( NumPlateJ_ - 1 );
    
    if ( TailIsClosed_ ) NumTris -= ( NumPlateJ_ - 1 );

    printf("NumTris: %d \n",NumTris);
    
    Grid_.SizeNodeList(NumNodes);
     
    // XYZ data
    
    n = nk = 0;
 
    // Surface nodes
    
    for ( i = 1 ; i <= NumPlateI_ ; i++ ) {
     
       for ( j = 1 ; j <= NumPlateJ_ ; j++ ) {

          n++;
          
          Grid_.NodeList(n).x() = x_plate(i,j);
          Grid_.NodeList(n).y() = y_plate(i,j);
          Grid_.NodeList(n).z() = z_plate(i,j);
          
          Grid_.NodeList(n).IsTrailingEdgeNode() = 0;
          Grid_.NodeList(n).IsLeadingEdgeNode() = 0;
          
          if ( j == 1 ) {
           
             Grid_.NodeList(n).IsTrailingEdgeNode() = 1;
          
             nk++;
             
             KuttaNode_[nk] = n;
             
             WakeTrailingEdgeX_[nk] = x_plate(i,j);
             WakeTrailingEdgeY_[nk] = y_plate(i,j);
             WakeTrailingEdgeZ_[nk] = z_plate(i,j);
                                
          }
          
          if ( j == NumPlateJ_ ) Grid_.NodeList(n).IsLeadingEdgeNode() = 1;
          
       }
       
    }    
    
    // Determine how many valid tris
    
    n = 0;
    
    for ( j = 1 ; j < NumPlateJ_ ; j++ ) {
    
       for ( i = 1 ; i < NumPlateI_ ; i++ ) {
        
          node1 = (i-1)*NumPlateJ_ + j;              // i,   j
          node2 = (i  )*NumPlateJ_ + j;              // i+1, j
          node3 = (i  )*NumPlateJ_ + j + 1;          // i+1, j + 1
          node4 = (i-1)*NumPlateJ_ + j + 1;          // i,   j + 1
      
          // Tri 1
  
          if ( j <= NumPlateJ_/2 ) {
              
             NodeA = node1;
             NodeB = node2;
             NodeC = node3;
             

          }
          
          else {
           
             NodeA = node1;
             NodeB = node2;
             NodeC = node4;

          }
          
          vec1[0] = Grid_.NodeList(NodeB).x() - Grid_.NodeList(NodeA).x();
          vec1[1] = Grid_.NodeList(NodeB).y() - Grid_.NodeList(NodeA).y();
          vec1[2] = Grid_.NodeList(NodeB).z() - Grid_.NodeList(NodeA).z();

          vec2[0] = Grid_.NodeList(NodeC).x() - Grid_.NodeList(NodeA).x();
          vec2[1] = Grid_.NodeList(NodeC).y() - Grid_.NodeList(NodeA).y();
          vec2[2] = Grid_.NodeList(NodeC).z() - Grid_.NodeList(NodeA).z();

          vector_cross(vec1,vec2,vec3);
          
          mag = sqrt(vector_dot(vec3,vec3));
             
          if ( mag > Tolerance*Tolerance ) n++;
          
          // Tri 2
             
          if ( j <= NumPlateJ_/2 ) {
              
             NodeA = node1;
             NodeB = node3;
             NodeC = node4;
         
          }
          
          else {
           
             NodeA = node4;
             NodeB = node2;
             NodeC = node3;
         
          }      
          
          vec1[0] = Grid_.NodeList(NodeB).x() - Grid_.NodeList(NodeA).x();
          vec1[1] = Grid_.NodeList(NodeB).y() - Grid_.NodeList(NodeA).y();
          vec1[2] = Grid_.NodeList(NodeB).z() - Grid_.NodeList(NodeA).z();

          vec2[0] = Grid_.NodeList(NodeC).x() - Grid_.NodeList(NodeA).x();
          vec2[1] = Grid_.NodeList(NodeC).y() - Grid_.NodeList(NodeA).y();
          vec2[2] = Grid_.NodeList(NodeC).z() - Grid_.NodeList(NodeA).z();

          vector_cross(vec1,vec2,vec3);
          
          mag = sqrt(vector_dot(vec3,vec3));        
          
          if ( mag > Tolerance*Tolerance ) n++;
           
       
       }
              
    }  

    NumTris = n;
    
    Grid_.SizeTriList(NumTris);

    printf("Number of valid tris: %d \n",NumTris);fflush(NULL);

    // Surface tris connectivity
    
    Flipped = 0;
    
    n = 0;
         
    for ( j = 1 ; j < NumPlateJ_ ; j++ ) {
    
       for ( i = 1 ; i < NumPlateI_ ; i++ ) {
        
          node1 = (i-1)*NumPlateJ_ + j;              // i,   j
          node2 = (i  )*NumPlateJ_ + j;              // i+1, j
          node3 = (i  )*NumPlateJ_ + j + 1;          // i+1, j + 1
          node4 = (i-1)*NumPlateJ_ + j + 1;          // i,   j + 1
      
          // Tri 1
       
          if ( j <= NumPlateJ_/2 ) {
           
             normal[0] = ( Nx_plate(i,j) + Nx_plate(i+1,j) + Nx_plate(i+1,j+1) ) / 3.;
             normal[1] = ( Ny_plate(i,j) + Ny_plate(i+1,j) + Ny_plate(i+1,j+1) ) / 3.;
             normal[2] = ( Nz_plate(i,j) + Nz_plate(i+1,j) + Nz_plate(i+1,j+1) ) / 3.;
             
             NodeA = node1;
             NodeB = node2;
             NodeC = node3;

          }
          
          else {
           
             normal[0] = ( Nx_plate(i,j) + Nx_plate(i+1,j) + Nx_plate(i,j+1) ) / 3.;
             normal[1] = ( Ny_plate(i,j) + Ny_plate(i+1,j) + Ny_plate(i,j+1) ) / 3.;
             normal[2] = ( Nz_plate(i,j) + Nz_plate(i+1,j) + Nz_plate(i,j+1) ) / 3.;
             
             NodeA = node1;
             NodeB = node2;
             NodeC = node4;

          }
          
          vec1[0] = Grid_.NodeList(NodeB).x() - Grid_.NodeList(NodeA).x();
          vec1[1] = Grid_.NodeList(NodeB).y() - Grid_.NodeList(NodeA).y();
          vec1[2] = Grid_.NodeList(NodeB).z() - Grid_.NodeList(NodeA).z();

          vec2[0] = Grid_.NodeList(NodeC).x() - Grid_.NodeList(NodeA).x();
          vec2[1] = Grid_.NodeList(NodeC).y() - Grid_.NodeList(NodeA).y();
          vec2[2] = Grid_.NodeList(NodeC).z() - Grid_.NodeList(NodeA).z();

          vector_cross(vec1,vec2,vec3);
          
          mag = sqrt(vector_dot(vec3,vec3));        
          
          if ( mag > Tolerance*Tolerance ) {
           
             n++;
                 
             Grid_.TriList(n).Node1() = NodeA;
             Grid_.TriList(n).Node2() = NodeB;
             Grid_.TriList(n).Node3() = NodeC;
           
             normal[0] /= mag;
             normal[1] /= mag;
             normal[2] /= mag;
               
             Grid_.TriList(n).NxCamber() = normal[0];
             Grid_.TriList(n).NyCamber() = normal[1];
             Grid_.TriList(n).NzCamber() = normal[2];

             Grid_.TriList(n).SurfaceID() = SurfaceID;
             
             Grid_.TriList(n).SpanStation() = i;
     
             Grid_.TriList(n).IsTrailingEdgeTri() = 0;
             
             Grid_.TriList(n).IsLeadingEdgeTri() = 0;
             
          }

          // Tri 2

          if ( j <= NumPlateJ_/2 ) {
          
             normal[0] = ( Nx_plate(i,j) + Nx_plate(i+1,j+1) + Nx_plate(i,j+1) ) / 3.;
             normal[1] = ( Ny_plate(i,j) + Ny_plate(i+1,j+1) + Ny_plate(i,j+1) ) / 3.;
             normal[2] = ( Nz_plate(i,j) + Nz_plate(i+1,j+1) + Nz_plate(i,j+1) ) / 3.;
             
             NodeA = node1;
             NodeB = node3;
             NodeC = node4;
         
          }
          
          else {
           
             normal[0] = ( Nx_plate(i,j+1) + Nx_plate(i+1,j) + Nx_plate(i+1,j+1) ) / 3.;
             normal[1] = ( Ny_plate(i,j+1) + Ny_plate(i+1,j) + Ny_plate(i+1,j+1) ) / 3.;
             normal[2] = ( Nz_plate(i,j+1) + Nz_plate(i+1,j) + Nz_plate(i+1,j+1) ) / 3.;
             
             NodeA = node4;
             NodeB = node2;
             NodeC = node3;
         
          }        
          
          vec1[0] = Grid_.NodeList(NodeB).x() - Grid_.NodeList(NodeA).x();
          vec1[1] = Grid_.NodeList(NodeB).y() - Grid_.NodeList(NodeA).y();
          vec1[2] = Grid_.NodeList(NodeB).z() - Grid_.NodeList(NodeA).z();

          vec2[0] = Grid_.NodeList(NodeC).x() - Grid_.NodeList(NodeA).x();
          vec2[1] = Grid_.NodeList(NodeC).y() - Grid_.NodeList(NodeA).y();
          vec2[2] = Grid_.NodeList(NodeC).z() - Grid_.NodeList(NodeA).z();

          vector_cross(vec1,vec2,vec3);
          
          mag = sqrt(vector_dot(vec3,vec3));                  
              
          if ( mag > Tolerance*Tolerance ) {
           
             n++;
                 
             Grid_.TriList(n).Node1() = NodeA;
             Grid_.TriList(n).Node2() = NodeB;
             Grid_.TriList(n).Node3() = NodeC;
           
             normal[0] /= mag;
             normal[1] /= mag;
             normal[2] /= mag;
               
             Grid_.TriList(n).NxCamber() = normal[0];
             Grid_.TriList(n).NyCamber() = normal[1];
             Grid_.TriList(n).NzCamber() = normal[2];

             Grid_.TriList(n).SurfaceID() = SurfaceID;
             
             Grid_.TriList(n).SpanStation() = i;
     
             Grid_.TriList(n).IsTrailingEdgeTri() = 0;
             
             Grid_.TriList(n).IsLeadingEdgeTri() = 0;
             
          }
      
       }
              
    }  
  
    printf("n: %d \n",n);
    
    // Calculate local chord lengths
        
    for ( i = 1 ; i < NumPlateI_ ; i++ ) {
     
       j = 1;
       
       x1 = 0.5*( x_plate(i,j) + x_plate(i+1,j) );
       y1 = 0.5*( y_plate(i,j) + y_plate(i+1,j) );
       z1 = 0.5*( z_plate(i,j) + z_plate(i+1,j) );
 
       j = NumPlateJ_;
       
       x2 = 0.5*( x_plate(i,j) + x_plate(i+1,j) );
       y2 = 0.5*( y_plate(i,j) + y_plate(i+1,j) );
       z2 = 0.5*( z_plate(i,j) + z_plate(i+1,j) );
       
       LocalChord_[i] = sqrt( pow(x2-x1,2.) + pow(y2-y1,2.) + pow(z2-z1,2.) );
       
    }

    // Calculate tri normals and build edge data structure

    printf("normals for body component: %s \n",ComponentName_);
    
    Grid_.CalculateNormalsAndCentroids();
   
    // Calculate average normals
    
    Nx = Ny = Nz = 0.;
    
    for ( n = 1 ; n <= NumTris ; n++ ) {
     
       Nx += Grid_.TriList(n).Nx();
       Ny += Grid_.TriList(n).Ny();
       Nz += Grid_.TriList(n).Nz();
       
    } 
    
    Nx /= NumTris;
    Ny /= NumTris;
    Nz /= NumTris;
    
   //if ( ABS(Nz) > ABS(Nx) && ABS(Nz) > ABS(Ny) ) {
     if ( 1) {
     
      printf("Nz: %lf \n",Nz);
      
       if ( Nz < 0. ) {
        
          for ( n = 1 ; n <= NumTris ; n++ ) {
           
             node1 = Grid_.TriList(n).Node1();
             node2 = Grid_.TriList(n).Node2();
             node3 = Grid_.TriList(n).Node3();
             
             Grid_.TriList(n).Node1() = node3;
             Grid_.TriList(n).Node2() = node2;
             Grid_.TriList(n).Node3() = node1;

          }   
          
         printf("Swapping... \n");

         Grid_.CalculateNormalsAndCentroids();
         
       }
           
    }
     fflush(NULL);
    Grid_.CreateTriEdges();

    Grid_.CalculateUpwindEdges();
 
}



