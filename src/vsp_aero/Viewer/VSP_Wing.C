//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Wing.H"

/*##############################################################################
#                                                                              #
#                               VSP_WING constructor                           #
#                                                                              #
##############################################################################*/

VSP_WING::VSP_WING(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                                   VSP_WING init                              #
#                                                                              #
##############################################################################*/

void VSP_WING::init(void)
{

    Verbose_ = 0;
    
}

/*##############################################################################
#                                                                              #
#                                  VSP_WING Copy                               #
#                                                                              #
##############################################################################*/

VSP_WING::VSP_WING(const VSP_WING &VSP_Wing)
{

    init();

    // Just * use the operator = code

    *this = VSP_Wing;

}

/*##############################################################################
#                                                                              #
#                           VSP_WING operator=                                 #
#                                                                              #
##############################################################################*/

VSP_WING& VSP_WING::operator=(const VSP_WING &VSP_Wing)
{

    return *this;

}

/*##############################################################################
#                                                                              #
#                        VSP_WING destructor                                   #
#                                                                              #
##############################################################################*/

VSP_WING::~VSP_WING(void)
{


}

/*##############################################################################
#                                                                              #
#                      VSP_WING SizeGeometryLists                              #
#                                                                              #
##############################################################################*/

void VSP_WING::SizeGeometryLists(int NumI, int NumJ)
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
#                      VSP_WING SizeFlatPlateLists                             #
#                                                                              #
##############################################################################*/

void VSP_WING::SizeFlatPlateLists(int NumI, int NumJ)
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
#                             VSP_WING ReadFile                                #
#                                                                              #
##############################################################################*/

void VSP_WING::ReadFile(char *Name, FILE *VSP_Degen_File)
{
 
    int i, j, NumI, NumJ, Wing, Done;
    double DumFloat, zCamber;
    char VSP_File_Name[2000], DumChar[2000], Stuff[2000];

    // Save the component name
    
    sprintf(ComponentName_,"%s",Name);
    
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

/* Old way, using camber line information... 
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
*/

          // Use camber line z value to define the surface, instead of linearizing
          // the tangential BC about the flat plate model
          
          sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %s",
                 &x_plate(i,j),
                 &y_plate(i,j),
                 &z_plate(i,j),
                 &zCamber,
                 &DumFloat,
                 &Nx_plate(i,j),
                 &Ny_plate(i,j),
                 &Nz_plate(i,j),
                 Stuff);
         
          x_plate(i,j) += zCamber * Nx_plate(i,j);
          y_plate(i,j) += zCamber * Ny_plate(i,j);
          z_plate(i,j) += zCamber * Nz_plate(i,j);
                       
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
    fflush(NULL);
}

/*##############################################################################
#                                                                              #
#                             VSP_WING CreateMesh                              #
#                                                                              #
##############################################################################*/

void VSP_WING::CreateMesh(int SurfaceID)
{
 
    int i, j, n, nk, NumNodes, NumTris, node1, node2, node3, node4, Flipped;
    double vec1[3], vec2[3], vec3[3], normal[3], mag;
    double x1, y1, z1, x2, y2, z2;
    
    // Trailing edge runs in the I direction...
    
    NumberOfKuttaNodes_ = NumPlateI_;

    KuttaNode_ = new int[NumberOfKuttaNodes_ + 1];
    
    WakeTrailingEdgeX_ = new double[NumberOfKuttaNodes_ + 1];
    WakeTrailingEdgeY_ = new double[NumberOfKuttaNodes_ + 1];
    WakeTrailingEdgeZ_ = new double[NumberOfKuttaNodes_ + 1];    
    
    printf("NumberOfKuttaNodes_: %d \n",NumberOfKuttaNodes_);      
    
    // Estimate the maximum number of grid levels
    
    MaxNumberOfGridLevels_ = 4*(int) ( log(1.0 * NumPlateI_ * NumPlateJ_) / log(4.0) );
    
    Grid_ = new VSP_GRID*[MaxNumberOfGridLevels_ + 1];
    
    // Calculate total number of nodes and tris, including the wake

    NumNodes = NumPlateI_*NumPlateJ_;
    
    NumTris  = 2*(NumPlateI_-1)*(NumPlateJ_-1);
    
    Grid_[0] = new VSP_GRID;
     
    Grid().SizeNodeList(NumNodes);
    
    Grid().SizeTriList(NumTris);
 
    // XYZ data
    
    n = nk = 0;
 
    // Surface nodes
    
    for ( i = 1 ; i <= NumPlateI_ ; i++ ) {
     
       for ( j = 1 ; j <= NumPlateJ_ ; j++ ) {

          n++;
          
          Grid().NodeList(n).x() = x_plate(i,j);
          Grid().NodeList(n).y() = y_plate(i,j);
          Grid().NodeList(n).z() = z_plate(i,j);
          
          Grid().NodeList(n).IsTrailingEdgeNode() = 0;
          Grid().NodeList(n).IsLeadingEdgeNode() = 0;
          Grid().NodeList(n).IsBoundaryEdgeNode() = 0;
          Grid().NodeList(n).IsBoundaryCornerNode() = 0;
          
          if ( j == 1 ) {
           
             Grid().NodeList(n).IsTrailingEdgeNode() = 1;
          
             nk++;
             
             KuttaNode_[nk] = n;
             
             WakeTrailingEdgeX_[nk] = x_plate(i,j);
             WakeTrailingEdgeY_[nk] = y_plate(i,j);
             WakeTrailingEdgeZ_[nk] = z_plate(i,j);
                                
          }
          
          if ( j == NumPlateJ_ ) Grid().NodeList(n).IsLeadingEdgeNode() = 1;
          
          if ( i == 1 || i == NumPlateI_ ) Grid().NodeList(n).IsBoundaryEdgeNode() = 1;
          
          if ( i == 1          && j == 1          ) Grid().NodeList(n).IsBoundaryCornerNode() = 1;
          if ( i == 1          && j == NumPlateJ_ ) Grid().NodeList(n).IsBoundaryCornerNode() = 1;
          if ( i == NumPlateI_ && j == 1          ) Grid().NodeList(n).IsBoundaryCornerNode() = 1;
          if ( i == NumPlateI_ && j == NumPlateJ_ ) Grid().NodeList(n).IsBoundaryCornerNode() = 1;
          
       }
       
    }    

    // Surface tris connectivity
    
    Flipped = 0;
    
    n = 0;
         
    for ( j = 1 ; j < NumPlateJ_ ; j++ ) {
    
       for ( i = 1 ; i < NumPlateI_ ; i++ ) {
        
          node1 = (i-1)*NumPlateJ_ + j;              // i,   j
          node2 = (i  )*NumPlateJ_ + j;              // i+1, j
          node3 = (i  )*NumPlateJ_ + j + 1;          // i+1, j + 1
          node4 = (i-1)*NumPlateJ_ + j + 1;          // i,   j + 1
          
   //      normal[0] = 0.25 * ( Nx_plate(i,j) + Nx_plate(i+1,j) + Nx_plate(i+1,j+1) + Nx_plate(i,j+1) );
   //       normal[1] = 0.25 * ( Ny_plate(i,j) + Ny_plate(i+1,j) + Ny_plate(i+1,j+1) + Ny_plate(i,j+1) );
   //       normal[2] = 0.25 * ( Nz_plate(i,j) + Nz_plate(i+1,j) + Nz_plate(i+1,j+1) + Nz_plate(i,j+1) );
          
          // Tri 1
          
          n++;

          normal[0] = ( Nx_plate(i,j) + Nx_plate(i+1,j) + Nx_plate(i+1,j+1) ) / 3.;
          normal[1] = ( Ny_plate(i,j) + Ny_plate(i+1,j) + Ny_plate(i+1,j+1) ) / 3.;
          normal[2] = ( Nz_plate(i,j) + Nz_plate(i+1,j) + Nz_plate(i+1,j+1) ) / 3.;
          
          mag = sqrt(vector_dot(normal,normal));
          
          normal[0] /= mag;
          normal[1] /= mag;
          normal[2] /= mag;
          
          Grid().TriList(n).Node1() = node1;
          Grid().TriList(n).Node2() = node2;
          Grid().TriList(n).Node3() = node3;
          
          Grid().TriList(n).NxCamber() = normal[0];
          Grid().TriList(n).NyCamber() = normal[1];
          Grid().TriList(n).NzCamber() = normal[2];
          
          Grid().TriList(n).SurfaceID() = SurfaceID;
          
          Grid().TriList(n).SpanStation() = i;
  
          Grid().TriList(n).IsTrailingEdgeTri() = 0;
          
          Grid().TriList(n).IsLeadingEdgeTri() = 0;
          
          if ( j == 1          ) Grid().TriList(n).IsTrailingEdgeTri() = 1;
          
          vec1[0] = Grid().NodeList(Grid().TriList(n).Node2()).x() - Grid().NodeList(Grid().TriList(n).Node1()).x();
          vec1[1] = Grid().NodeList(Grid().TriList(n).Node2()).y() - Grid().NodeList(Grid().TriList(n).Node1()).y();
          vec1[2] = Grid().NodeList(Grid().TriList(n).Node2()).z() - Grid().NodeList(Grid().TriList(n).Node1()).z();

          vec2[0] = Grid().NodeList(Grid().TriList(n).Node3()).x() - Grid().NodeList(Grid().TriList(n).Node1()).x();
          vec2[1] = Grid().NodeList(Grid().TriList(n).Node3()).y() - Grid().NodeList(Grid().TriList(n).Node1()).y();
          vec2[2] = Grid().NodeList(Grid().TriList(n).Node3()).z() - Grid().NodeList(Grid().TriList(n).Node1()).z();
          
          vector_cross(vec1,vec2,vec3);
          
          mag = sqrt(vector_dot(vec3,vec3));
          
          vec3[0] /= mag;
          vec3[1] /= mag;
          vec3[2] /= mag;

          if ( vector_dot(vec3,normal) < 0. ) {
           
             Grid().TriList(n).Node1() = node3;
             Grid().TriList(n).Node2() = node2;
             Grid().TriList(n).Node3() = node1;
             
             if ( Verbose_ ) printf("Case 1---> \n");
             if ( Verbose_ ) printf("vec3: %f %f %f \n",vec3[0],vec3[1],vec3[2]);
             if ( Verbose_ ) printf("Normal: %f %f %f \n",normal[0],normal[1],normal[2]);
             
             Flipped = 1;
             
          }
          
          // Tri 2
          
          n++;

          normal[0] = ( Nx_plate(i,j) + Nx_plate(i+1,j+1) + Nx_plate(i,j+1) ) / 3.;
          normal[1] = ( Ny_plate(i,j) + Ny_plate(i+1,j+1) + Ny_plate(i,j+1) ) / 3.;
          normal[2] = ( Nz_plate(i,j) + Nz_plate(i+1,j+1) + Nz_plate(i,j+1) ) / 3.;
          
          mag = sqrt(vector_dot(normal,normal));
          
          normal[0] /= mag;
          normal[1] /= mag;
          normal[2] /= mag;
          
          Grid().TriList(n).Node1() = node1;
          Grid().TriList(n).Node2() = node3;
          Grid().TriList(n).Node3() = node4;
         
          Grid().TriList(n).NxCamber() = normal[0];
          Grid().TriList(n).NyCamber() = normal[1];
          Grid().TriList(n).NzCamber() = normal[2];
          
          Grid().TriList(n).SurfaceID() = SurfaceID;
          
          Grid().TriList(n).SpanStation() = i;

          Grid().TriList(n).IsTrailingEdgeTri() = 0;
          
          Grid().TriList(n).IsLeadingEdgeTri() = 0;
          
          if ( j == NumPlateJ_ - 1) Grid().TriList(n).IsLeadingEdgeTri() = 1;
          
          vec1[0] = Grid().NodeList(Grid().TriList(n).Node2()).x() - Grid().NodeList(Grid().TriList(n).Node1()).x();
          vec1[1] = Grid().NodeList(Grid().TriList(n).Node2()).y() - Grid().NodeList(Grid().TriList(n).Node1()).y();
          vec1[2] = Grid().NodeList(Grid().TriList(n).Node2()).z() - Grid().NodeList(Grid().TriList(n).Node1()).z();

          vec2[0] = Grid().NodeList(Grid().TriList(n).Node3()).x() - Grid().NodeList(Grid().TriList(n).Node1()).x();
          vec2[1] = Grid().NodeList(Grid().TriList(n).Node3()).y() - Grid().NodeList(Grid().TriList(n).Node1()).y();
          vec2[2] = Grid().NodeList(Grid().TriList(n).Node3()).z() - Grid().NodeList(Grid().TriList(n).Node1()).z();
          
          vector_cross(vec1,vec2,vec3);

          mag = sqrt(vector_dot(vec3,vec3));
          
          vec3[0] /= mag;
          vec3[1] /= mag;
          vec3[2] /= mag;
          
          if ( vector_dot(vec3,normal) < 0. ) {
           
             Grid().TriList(n).Node1() = node4;
             Grid().TriList(n).Node2() = node3;
             Grid().TriList(n).Node3() = node1;

             if ( Verbose_ ) printf("Case 2---> \n");
             if ( Verbose_ ) printf("vec3: %f %f %f \n",vec3[0],vec3[1],vec3[2]);
             if ( Verbose_ ) printf("Normal: %f %f %f \n",normal[0],normal[1],normal[2]);
             
             Flipped = 1;
             
          }
          
       }
       
    }     
    
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

    Grid().CalculateNormalsAndCentroids();
    
    Grid().CreateTriEdges();
    
    Grid().CalculateUpwindEdges();
    fflush(NULL);
}

/*##############################################################################
#                                                                              #
#                             VSP_BODY AgglomerateMesh                         #
#                                                                              #
##############################################################################*/

void VSP_WING::AgglomerateMesh(void)
{

    int i;

    VSP_AGGLOM Agglomerate;
 
    printf("Wing MaxNumberOfGridLevels_: %d \n",MaxNumberOfGridLevels_);
    
    i = 1;
    
    while ( i < MaxNumberOfGridLevels_ && Grid(i-1).NumberOfTris() > 1 ) {

       Grid_[i] = Agglomerate.Agglomerate(Grid(i-1));
       
       printf("Wing Grid:%d --> Number of loops: %d \n",i,Grid(i).NumberOfTris());
       
       i++;

    }
    
    NumberOfGridLevels_ = i;
    fflush(NULL);
}



