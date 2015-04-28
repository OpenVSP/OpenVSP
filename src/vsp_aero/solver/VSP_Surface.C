//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Surface.H"

/*##############################################################################
#                                                                              #
#                            VSP_SURFACE constructor                           #
#                                                                              #
##############################################################################*/

VSP_SURFACE::VSP_SURFACE(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                                VSP_SURFACE init                              #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::init(void)
{

    Verbose_ = 0;
    
}

/*##############################################################################
#                                                                              #
#                               VSP_SURFACE Copy                               #
#                                                                              #
##############################################################################*/

VSP_SURFACE::VSP_SURFACE(const VSP_SURFACE &VSP_Wing)
{

    init();

    // Just * use the operator = code

    *this = VSP_Wing;

}

/*##############################################################################
#                                                                              #
#                        VSP_SURFACE operator=                                 #
#                                                                              #
##############################################################################*/

VSP_SURFACE& VSP_SURFACE::operator=(const VSP_SURFACE &VSP_Surface)
{

    return *this;

}

/*##############################################################################
#                                                                              #
#                     VSP_SURFACE destructor                                   #
#                                                                              #
##############################################################################*/

VSP_SURFACE::~VSP_SURFACE(void)
{


}

/*##############################################################################
#                                                                              #
#                   VSP_SURFACE SizeGeometryLists                              #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::SizeGeometryLists(int NumI, int NumJ)
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
#                   VSP_SURFACE SizeFlatPlateLists                             #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::SizeFlatPlateLists(int NumI, int NumJ)
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
#                          VSP_SURFACE ReadWingDataFromFile                    #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::ReadWingDataFromFile(char *Name, FILE *VSP_Degen_File)
{
 
    int i, j, NumI, NumJ, Wing, Done;
    double DumFloat, zCamber;
    char DumChar[2000], Stuff[2000];

    // Save the component name
    
    sprintf(ComponentName_,"%s",Name);
    
    // Set surface type
    
    SurfaceType_ = WING_SURFACE;
    
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
    
    // Skip over data until we find the POINT data
    
    Done = 0;
    
    while ( !Done ) {
    
       fgets(DumChar,1000,VSP_Degen_File);
       
       if ( strstr(DumChar,"POINT") != NULL ) Done = 1;
       
    }    
    
    fgets(DumChar,1000,VSP_Degen_File);
    fgets(DumChar,1000,VSP_Degen_File);
    
    sscanf(DumChar,"%lf, %lf, %lf, %lf, %s ",
                     &DumFloat,
                     &DumFloat,
                     &DumFloat,
                     &WettedArea_,
                     Stuff);
              
    // Check for degenerate span stations
    
    CheckForDegenerateSpanSections();
    
}

/*##############################################################################
#                                                                              #
#                      VSP_SURFACE  ReadBodyDataFromFile                       #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::ReadBodyDataFromFile(char *Name, int Case, FILE *VSP_Degen_File)
{
 
    int i, j, k, NumI, NumJ, Done, jStart, jEnd;
    double DumFloat;
    char DumChar[2000], Stuff[2000];

    // Save the component name

    sprintf(ComponentName_,"%s",Name);
    
    // Set surface type
    
    SurfaceType_ = BODY_SURFACE;    

    // Read in the body data
    
    fgets(DumChar,1000,VSP_Degen_File);
    fgets(DumChar,1000,VSP_Degen_File); sscanf(DumChar,"SURFACE_NODE,%d,%d",&NumI,&NumJ);
    fgets(DumChar,1000,VSP_Degen_File);
    
    if ( Verbose_ ) printf("NumI, NumJ: %d %d \n",NumI,NumJ);

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
    
    if ( Case >= 3 ) {
     
       Done = 0;
    
       while ( !Done ) {
       
          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strstr(DumChar,"PLATE") != NULL ) Done = 1;
          
       }
    
    }
    
    // Read in the plate surface size
    
    sscanf(DumChar,"PLATE,%d,%d",&NumI,&NumJ);
    
    if ( Verbose_ ) printf("NumI, NumJ: %d %d \n",NumI,NumJ);
    
    SizeFlatPlateLists(NumI,NumJ/2+1);

    // Skip over normals information
        
    for ( i = 1 ; i <= NumI + 2 ; i++ ) fgets(DumChar,1000,VSP_Degen_File);  
     
    // Now read in the flat plate representation of the wing
    
    for ( i = 1 ; i <= NumI ; i++ ) {

       if ( Case == 1 || Case == 3 ) { 

          k = 0;
          
          jStart = 1 ; jEnd = NumJ/2 + 1;

          for ( j = jStart ; j <= jEnd ; j++ ) {

             fgets(DumChar,1000,VSP_Degen_File);  
      
             k++;
             
             sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %s",
                    &x_plate(i,k),
                    &y_plate(i,k),
                    &z_plate(i,k),
                    &DumFloat,
                    &DumFloat,
                    &Nx_plate(i,k),
                    &Ny_plate(i,k),
                    &Nz_plate(i,k),
                    Stuff);

          }

          jStart = NumJ/2 + 2; jEnd = NumJ;
          
          for ( j = jStart ; j <= jEnd ; j++ ) {

             fgets(DumChar,1000,VSP_Degen_File);  
  
             sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %s",
                    &DumFloat,
                    &DumFloat,
                    &DumFloat,
                    &DumFloat,
                    &DumFloat,
                    &DumFloat,
                    &DumFloat,
                    &DumFloat,
                    Stuff);

          }
           
       }
       
       else {

          jStart = 1 ; jEnd = NumJ/2;
        
          for ( j = jStart ; j <= jEnd ; j++ ) {

             fgets(DumChar,1000,VSP_Degen_File);  
  
             sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %s",
                    &DumFloat,
                    &DumFloat,
                    &DumFloat,
                    &DumFloat,
                    &DumFloat,
                    &DumFloat,
                    &DumFloat,
                    &DumFloat,
                    Stuff);

          }
        
          k = NumJ/2 + 2;
          
          jStart = NumJ/2 + 1; jEnd = NumJ;
        
          for ( j = jStart ; j <= jEnd ; j++ ) {

             fgets(DumChar,1000,VSP_Degen_File);  
      
             k--;
             
             sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %s",
                    &x_plate(i,k),
                    &y_plate(i,k),
                    &z_plate(i,k),
                    &DumFloat,
                    &DumFloat,
                    &Nx_plate(i,k),
                    &Ny_plate(i,k),
                    &Nz_plate(i,k),
                    Stuff);

          }
          
       }

    }    
    
    // Skip over data until we find the POINT data
    
    Done = 0;
    
    while ( !Done ) {
    
       fgets(DumChar,1000,VSP_Degen_File);
       
       if ( strstr(DumChar,"POINT") != NULL ) Done = 1;
       
    }    

    fgets(DumChar,1000,VSP_Degen_File);
    fgets(DumChar,1000,VSP_Degen_File);
    
    sscanf(DumChar,"%lf, %lf, %lf, %lf, %s ",
                     &DumFloat,
                     &DumFloat,
                     &DumFloat,
                     &WettedArea_,
                     Stuff);

    // Check for degenerate body x-sections
    
    CheckForDegenerateXSections();
   
}

/*##############################################################################
#                                                                              #
#                   VSP_SURFACE  CheckForDegenerateXSections                   #
#                                                                              #
##############################################################################*/

void VSP_SURFACE ::CheckForDegenerateXSections(void)
{
 
    int i, j, ii, *BadXSection, NumBadXSections, NumI;
    double  Distance, Tolerance, Ymin, Ymax, Zmin, Zmax, Span;
    double *x_plate_new, *y_plate_new, *z_plate_new;
    double *Nx_plate_new, *Ny_plate_new, *Nz_plate_new;
    double *LocalChord_new;
    
    Tolerance = 1.e-3;
  
    // Determine if the any two x-sections are at the same x-location
        
    BadXSection = new int[NumPlateI_ + 1];
    
    zero_int_array(BadXSection, NumPlateI_);
    
    NumBadXSections = 0;
    
    for ( i = 1 ; i < NumPlateI_ ; i++ ) {    
       
       Ymin = Zmin = 1.e9;
       Ymax = Zmax = -1.e9;
       
       Distance = 0.;
     
       for ( j = 1 ; j <= NumPlateJ_ ; j++ ) {

          Ymin = MIN3(y_plate(i,j),y_plate(i+1,j),Ymin);
          Ymax = MAX3(y_plate(i,j),y_plate(i+1,j),Ymax);
          
          Zmin = MIN3(z_plate(i,j),z_plate(i+1,j),Zmin);
          Zmax = MAX3(z_plate(i,j),z_plate(i+1,j),Zmax);
           
          Distance += pow(x_plate(i,j)-x_plate(i+1,j),2.);
        
       }
       
       Span = MAX(Ymax - Ymin, Zmax - Zmin);
    
       Distance /= NumPlateJ_;

       if ( i != 1 && i != NumPlateI_ && Distance <= Tolerance * Span ) {
        
          if ( Verbose_ ) printf("X sections %d and %d are the same... located at x: %lf \n",i,i+1,x_plate(i,1));
          
          BadXSection[i] = 1;
          
          NumBadXSections++;
          
       }
        
    }
   
    // If we have multiple x-sections at the same station... delete one
    
    if ( NumBadXSections > 0 ) {
     
       NumI = NumPlateI_ - NumBadXSections;

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
#                   VSP_SURFACE  CheckForDegenerateSpanSections                #
#                                                                              #
##############################################################################*/

void VSP_SURFACE ::CheckForDegenerateSpanSections(void)
{
 
    int i, j, ii, *BadSpanSection, NumBadSpanSections, NumI;
    double  Distance, Tolerance;
    double *x_plate_new, *y_plate_new, *z_plate_new;
    double *Nx_plate_new, *Ny_plate_new, *Nz_plate_new;
    double *LocalChord_new, x1, y1, z1, x2, y2, z2;
    
    Tolerance = 1.e-3;
  
    // Determine if the any two x-sections are at the same span-location
        
    BadSpanSection = new int[NumPlateI_ + 1];
    
    zero_int_array(BadSpanSection, NumPlateI_);
    
    NumBadSpanSections = 0;
    
    for ( i = 1 ; i < NumPlateI_/2 ; i++ ) {    
       
       Distance = 0.;
     
       for ( j = 1 ; j <= NumPlateJ_ ; j++ ) {

          Distance +=   pow(x_plate(i,j)-x_plate(i+1,j),2.)
                      + pow(y_plate(i,j)-y_plate(i+1,j),2.)
                      + pow(z_plate(i,j)-z_plate(i+1,j),2.);
        
       }
    
       Distance /= NumPlateJ_;

       // Calculate local chord
       
       j = 1;
       
       x1 = 0.5*( x_plate(i,j) + x_plate(i+1,j) );
       y1 = 0.5*( y_plate(i,j) + y_plate(i+1,j) );
       z1 = 0.5*( z_plate(i,j) + z_plate(i+1,j) );
 
       j = NumPlateJ_;
       
       x2 = 0.5*( x_plate(i,j) + x_plate(i+1,j) );
       y2 = 0.5*( y_plate(i,j) + y_plate(i+1,j) );
       z2 = 0.5*( z_plate(i,j) + z_plate(i+1,j) );
       
       LocalChord_[i] = sqrt( pow(x2-x1,2.) + pow(y2-y1,2.) + pow(z2-z1,2.) );

       // Decide if this is a duplicate section
       
       if ( Distance <= Tolerance * LocalChord_[i] ) {
        
          if ( Verbose_ ) printf("Span sections %d and %d are the same... located at y: %lf \n",i,i+1,y_plate(i,1)); fflush(NULL);
          
          BadSpanSection[i] = 1;
          
          NumBadSpanSections++;
          
       }
        
    }
    
    for ( i = NumPlateI_ ; i >= NumPlateI_/2 + 1 ; i-- ) {    
       
       Distance = 0.;
     
       for ( j = 1 ; j <= NumPlateJ_ ; j++ ) {

          Distance +=   pow(x_plate(i,j)-x_plate(i-1,j),2.)
                      + pow(y_plate(i,j)-y_plate(i-1,j),2.)
                      + pow(z_plate(i,j)-z_plate(i-1,j),2.);
        
       }
    
       Distance /= NumPlateJ_;

       // Calculate local chord
       
       j = 1;
       
       x1 = 0.5*( x_plate(i,j) + x_plate(i-1,j) );
       y1 = 0.5*( y_plate(i,j) + y_plate(i-1,j) );
       z1 = 0.5*( z_plate(i,j) + z_plate(i-1,j) );
 
       j = NumPlateJ_;
       
       x2 = 0.5*( x_plate(i,j) + x_plate(i-1,j) );
       y2 = 0.5*( y_plate(i,j) + y_plate(i-1,j) );
       z2 = 0.5*( z_plate(i,j) + z_plate(i-1,j) );
       
       LocalChord_[i] = sqrt( pow(x2-x1,2.) + pow(y2-y1,2.) + pow(z2-z1,2.) );

       // Decide if this is a duplicate section
       
       if ( Distance <= Tolerance * LocalChord_[i] ) {
        
          if ( Verbose_ ) printf("Span sections %d and %d are the same... located at y: %lf \n",i,i-1,y_plate(i,1)); fflush(NULL);
          
          BadSpanSection[i] = 1;
          
          NumBadSpanSections++;
          
       }
        
    }    

    // If we have multiple span sections at the same station... delete one
    
    if ( NumBadSpanSections > 0 ) {
     
       NumI = NumPlateI_ - NumBadSpanSections;
     
       x_plate_new = new double[NumI*NumPlateJ_ + 1];
       y_plate_new = new double[NumI*NumPlateJ_ + 1];
       z_plate_new = new double[NumI*NumPlateJ_ + 1];    
       
       Nx_plate_new = new double[NumI*NumPlateJ_ + 1];
       Ny_plate_new = new double[NumI*NumPlateJ_ + 1];
       Nz_plate_new = new double[NumI*NumPlateJ_ + 1];    
       
       LocalChord_new = new double[NumI + 1];    

       ii = 0;
       
       for ( i = 1 ; i <= NumPlateI_ ; i++ ) {
        
          if ( !BadSpanSection[i] ) {
           
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
   
    delete [] BadSpanSection;    
    
}

/*##############################################################################
#                                                                              #
#                             VSP_SURFACE CreateMesh                           #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::CreateMesh(int SurfaceID)
{

    int j;
    
    // Create the surface mesh
    
    if ( SurfaceType_ == WING_SURFACE ) CreateWingTriMesh(SurfaceID);
    
    if ( SurfaceType_ == BODY_SURFACE ) CreateBodyTriMesh(SurfaceID);

    // Surface type, and ID data at the tri / loop level
    
    for ( j = 1 ; j <= Grid().NumberOfTris() ; j++ ) {

      if ( SurfaceType_ == WING_SURFACE ) {
        
          Grid().TriList(j).SurfaceType() = WING_SURFACE;
          Grid().TriList(j).BodyID() = 0;
          Grid().TriList(j).WingID() = SurfaceID;  
          
       }
       
       else if (  SurfaceType_ == BODY_SURFACE ) {
        
          Grid().TriList(j).SurfaceType() = BODY_SURFACE;
          Grid().TriList(j).BodyID() = SurfaceID;
          Grid().TriList(j).WingID() = 0;  
          
       }

    }
    
    // Create upwind edge data    
    
    CreateUpwindEdgeData(SurfaceID);
    
    // If wetted area is zero... then estimate it from the mesh
    
    if ( WettedArea_ <= 0. ) { 
       
        WettedArea_ = 0.;
        
        for ( j = 1 ; j <= Grid().NumberOfTris() ; j++ ) {
         
           WettedArea_ += Grid().TriList(j).Area();
            
        }
         
     }
     
     WettedArea_ *= 0.5*PI;
     
}

/*##############################################################################
#                                                                              #
#                      VSP_SURFACE CreateWingTriMesh                           #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::CreateWingTriMesh(int SurfaceID)
{
 
    int i, j, n, nk, NumNodes, NumTris, node1, node2, node3, node4, Flipped;
    double vec1[3], vec2[3], vec3[3], normal[3], mag;
    double x1, y1, z1, x2, y2, z2;
        
    // Estimate the maximum number of grid levels
    
    MaxNumberOfGridLevels_ = 4*(int) ( log(1.0 * NumPlateI_ * NumPlateJ_) / log(4.0) );
    
    Grid_ = new VSP_GRID*[MaxNumberOfGridLevels_ + 1];
    
    // Calculate total number of nodes and tris, including the wake

    NumNodes = NumPlateI_*NumPlateJ_;
    
    NumTris  = 2*(NumPlateI_-1)*(NumPlateJ_-1);
    
    Grid_[0] = new VSP_GRID;
     
    Grid().SizeNodeList(NumNodes);
    
    Grid().SizeTriList(NumTris);
    
    // Kutta nodes... Trailing edge runs in the I direction...

    if ( strstr(ComponentName_,"NOWAKE") == NULL ) {
       
       IsLiftingSurface_ = 1;

       Grid().SizeKuttaNodeList(NumPlateI_);    
       
    }
    
    else {
       
       IsLiftingSurface_ = 0;

       Grid().SizeKuttaNodeList(0);    
       
    }       
 
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
          
          if ( j == 1 && IsLiftingSurface_ ) {
           
             Grid().NodeList(n).IsTrailingEdgeNode() = 1;
          
             nk++;
             
             Grid().KuttaNode(nk) = n;
             
             Grid().WakeTrailingEdgeX(nk) = x_plate(i,j);
             Grid().WakeTrailingEdgeY(nk) = y_plate(i,j);
             Grid().WakeTrailingEdgeZ(nk) = z_plate(i,j);
                                
          }
          
          if ( j == 1 && IsLiftingSurface_ ) Grid().NodeList(n).IsBoundaryEdgeNode() = 1;
          
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
          
          if ( j == 1 && IsLiftingSurface_ ) Grid().TriList(n).IsTrailingEdgeTri() = 1;
          
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
        
    AverageChord_ = 0.;
    
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
       
       AverageChord_ += LocalChord_[i];
       
    }
    
    AverageChord_ /= (NumPlateI_-1);

    // Calculate tri normals and build edge data structure

    Grid().CalculateTriNormalsAndCentroids();
    
    Grid().CreateTriEdges();
    
    Grid().CalculateUpwindEdges();   

}

/*##############################################################################
#                                                                              #
#                      VSP_SURFACE  CreateBodyTriMesh                          #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::CreateBodyTriMesh(int SurfaceID)
{
 
    int i, j, k, n, nk, NumNodes, NumTris, node1, node2, node3, node4, Flipped;
    int NodeA, NodeB, NodeC, *NodePerm, TailNode, NoseNode;
    double vec1[3], vec2[3], vec3[3], normal[3], mag;
    double x1, y1, z1, x2, y2, z2, Nx, Ny, Nz, Distance, Tolerance;

    Tolerance = 1.e-7;
    
    // Estimate the maximum number of grid levels
    
    MaxNumberOfGridLevels_ = 4*(int) ( log(1.0 * NumPlateI_ * NumPlateJ_) / log(4.0) );

    Grid_ = new VSP_GRID*[MaxNumberOfGridLevels_ + 1];
    
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
  
    // Size the node list

    NumNodes = NumPlateI_*NumPlateJ_;
    
    if ( NoseIsClosed_ ) NumNodes -= ( NumPlateJ_ - 1 );
    
    if ( TailIsClosed_ ) NumNodes -= ( NumPlateJ_ - 1 );
    
    if ( Verbose_ ) {
       
       if ( NoseIsClosed_ ) {
      
          printf("Nose is closed... \n");
          
       }
       
       else {
          
          printf("Nose is open... \n");
          
       }      
      
       if ( TailIsClosed_ ) {
      
          printf("Tail is closed... \n");
          
       }
       
       else {
          
          printf("Tail is open... \n");
          
       } 
       
    }
    
    Grid_[0] = new VSP_GRID;
    
    Grid().SizeNodeList(NumNodes);
 
    // Size the tri list
    
    NumTris  = 2*(NumPlateI_-1)*(NumPlateJ_-1);

    if ( NoseIsClosed_ ) NumTris -= ( NumPlateJ_ - 1 );
    
    if ( TailIsClosed_ ) NumTris -= ( NumPlateJ_ - 1 );

    Grid().SizeTriList(NumTris);

    // Kutta nodes... Trailing edge runs in the I direction...

    IsLiftingSurface_ = 0;
    
    Grid().SizeKuttaNodeList(0);    
     
    // Node node permutation array
    
    NodePerm = new int[NumPlateI_ * NumPlateJ_ + 1];
    
    for ( i = 1 ; i <= NumPlateI_ * NumPlateJ_ ; i++ ) {
     
       NodePerm[i] = 0;
       
    }    
    
    n = k = 0;
    
    for ( i = 1 ; i <= NumPlateI_ ; i++ ) {
     
       for ( j = 1 ; j <= NumPlateJ_ ; j++ ) {
        
          k++;

          // Modify permutation array at tail
          
          if ( i == NumPlateI_ && TailIsClosed_ ) {
           
            if ( j == 1 ) {
                            
               n++;             
               
               TailNode = n;

               NodePerm[k] = n;
               
            }
            
            else {
             
               NodePerm[k] = -TailNode;
               
            }
            
          }
          
          else if ( i == 1 && NoseIsClosed_ ) {
           
            if ( j == 1 ) {
             
               n++;          
                           
               NoseNode = n;
               
               NodePerm[k] = n;
               
            }
            
            else {
             
               NodePerm[k] = -NoseNode;
               
            }
            
          }
          
          else {
           
             n++;
             
             NodePerm[k] = n;
             
          }
                    
       }
       
    } 

    if ( n != NumNodes ) {
     
       printf("Error in determining the number of valid nodes in body mesh! \n"); fflush(NULL);
       
       exit(1);
       
    }
  
    // XYZ data
    
    k = n = nk = 0;
 
    // Surface nodes

    for ( i = 1 ; i <= NumPlateI_ ; i++ ) {
     
       for ( j = 1 ; j <= NumPlateJ_ ; j++ ) {

          k++;
        
          if ( NodePerm[k] > 0 ) {
           
             n = NodePerm[k];
           
             Grid().NodeList(n).x() = x_plate(i,j);
             Grid().NodeList(n).y() = y_plate(i,j);
             Grid().NodeList(n).z() = z_plate(i,j);
             
             Grid().NodeList(n).IsTrailingEdgeNode() = 0;
             Grid().NodeList(n).IsLeadingEdgeNode() = 0;
             
             if (0&& i == NumPlateI_ && !TailIsClosed_ ) {
      
                Grid().NodeList(n).IsTrailingEdgeNode() = 1;
             
                nk++;
                
                Grid().KuttaNode(nk) = n;
                
                Grid().WakeTrailingEdgeX(nk) = x_plate(i,j);
                Grid().WakeTrailingEdgeY(nk) = y_plate(i,j);
                Grid().WakeTrailingEdgeZ(nk) = z_plate(i,j);
                                  
             }
                 
             if ( i == 1          && !NoseIsClosed_  ) Grid().NodeList(n).IsLeadingEdgeNode() = 1;
              
             if ( j == 1          || j == NumPlateJ_ ) Grid().NodeList(n).IsBoundaryEdgeNode() = 1;
             if ( i == 1          && NoseIsClosed_   ) Grid().NodeList(n).IsBoundaryEdgeNode() = 1;
             if ( i == NumPlateI_ && TailIsClosed_   ) Grid().NodeList(n).IsBoundaryEdgeNode() = 1;
             
             if ( i == 1          && j == 1          ) Grid().NodeList(n).IsBoundaryCornerNode() = 1;
             if ( i == 1          && j == NumPlateJ_ ) Grid().NodeList(n).IsBoundaryCornerNode() = 1;
             if ( i == NumPlateI_ && j == 1          ) Grid().NodeList(n).IsBoundaryCornerNode() = 1;
             if ( i == NumPlateI_ && j == NumPlateJ_ ) Grid().NodeList(n).IsBoundaryCornerNode() = 1;  
             
          }
          
       }
       
    }    

    // Surface tris connectivity
    
    Flipped = 0;
    
    n = 0;
         
    for ( j = 1 ; j < NumPlateJ_ ; j++ ) {
    
       for ( i = 1 ; i < NumPlateI_ ; i++ ) {
        
          node1 = ABS(NodePerm[(i-1)*NumPlateJ_ + j    ]); // i,   j
          node2 = ABS(NodePerm[(i  )*NumPlateJ_ + j    ]); // i+1, j
          node3 = ABS(NodePerm[(i  )*NumPlateJ_ + j + 1]); // i+1, j + 1
          node4 = ABS(NodePerm[(i-1)*NumPlateJ_ + j + 1]); // i,   j + 1
      
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
          
          if ( NodeA != NodeB && NodeA != NodeC && NodeB != NodeC ) {

             n++;
          
             vec1[0] = Grid().NodeList(NodeB).x() - Grid().NodeList(NodeA).x();
             vec1[1] = Grid().NodeList(NodeB).y() - Grid().NodeList(NodeA).y();
             vec1[2] = Grid().NodeList(NodeB).z() - Grid().NodeList(NodeA).z();

             vec2[0] = Grid().NodeList(NodeC).x() - Grid().NodeList(NodeA).x();
             vec2[1] = Grid().NodeList(NodeC).y() - Grid().NodeList(NodeA).y();
             vec2[2] = Grid().NodeList(NodeC).z() - Grid().NodeList(NodeA).z();

             vector_cross(vec1,vec2,vec3);
             
             mag = sqrt(vector_dot(vec3,vec3));        
                     
             Grid().TriList(n).Node1() = NodeA;
             Grid().TriList(n).Node2() = NodeB;
             Grid().TriList(n).Node3() = NodeC;
           
             normal[0] /= mag;
             normal[1] /= mag;
             normal[2] /= mag;
               
             Grid().TriList(n).NxCamber() = normal[0];
             Grid().TriList(n).NyCamber() = normal[1];
             Grid().TriList(n).NzCamber() = normal[2];

             Grid().TriList(n).SurfaceID() = SurfaceID;
             
             Grid().TriList(n).SpanStation() = i;
     
             Grid().TriList(n).IsTrailingEdgeTri() = 0;
             
             Grid().TriList(n).IsLeadingEdgeTri() = 0;
             
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
          
          if ( NodeA != NodeB && NodeA != NodeC && NodeB != NodeC ) {
           
             n++;
          
             vec1[0] = Grid().NodeList(NodeB).x() - Grid().NodeList(NodeA).x();
             vec1[1] = Grid().NodeList(NodeB).y() - Grid().NodeList(NodeA).y();
             vec1[2] = Grid().NodeList(NodeB).z() - Grid().NodeList(NodeA).z();

             vec2[0] = Grid().NodeList(NodeC).x() - Grid().NodeList(NodeA).x();
             vec2[1] = Grid().NodeList(NodeC).y() - Grid().NodeList(NodeA).y();
             vec2[2] = Grid().NodeList(NodeC).z() - Grid().NodeList(NodeA).z();

             vector_cross(vec1,vec2,vec3);
             
             mag = sqrt(vector_dot(vec3,vec3));                  
                     
             Grid().TriList(n).Node1() = NodeA;
             Grid().TriList(n).Node2() = NodeB;
             Grid().TriList(n).Node3() = NodeC;
           
             normal[0] /= mag;
             normal[1] /= mag;
             normal[2] /= mag;
               
             Grid().TriList(n).NxCamber() = normal[0];
             Grid().TriList(n).NyCamber() = normal[1];
             Grid().TriList(n).NzCamber() = normal[2];

             Grid().TriList(n).SurfaceID() = SurfaceID;
             
             Grid().TriList(n).SpanStation() = i;
     
             Grid().TriList(n).IsTrailingEdgeTri() = 0;
             
             Grid().TriList(n).IsLeadingEdgeTri() = 0;
             
          }
      
       }
              
    }  
    
    if ( n != NumTris ) {
     
       printf("Error in determining the number of valid tris in body mesh! \n"); fflush(NULL);
       
       exit(1);
       
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
    
    Grid().CalculateTriNormalsAndCentroids();
   
    // Calculate average normals
    
    Nx = Ny = Nz = 0.;
    
    for ( n = 1 ; n <= NumTris ; n++ ) {
     
       Nx += Grid().TriList(n).Nx();
       Ny += Grid().TriList(n).Ny();
       Nz += Grid().TriList(n).Nz();
       
    } 
    
    Nx /= NumTris;
    Ny /= NumTris;
    Nz /= NumTris;
    
   //if ( ABS(Nz) > ABS(Nx) && ABS(Nz) > ABS(Ny) ) {
     if ( 1) {
   
       if ( Nz < 0. ) {
        
          for ( n = 1 ; n <= NumTris ; n++ ) {
           
             node1 = Grid().TriList(n).Node1();
             node2 = Grid().TriList(n).Node2();
             node3 = Grid().TriList(n).Node3();
             
             Grid().TriList(n).Node1() = node3;
             Grid().TriList(n).Node2() = node2;
             Grid().TriList(n).Node3() = node1;

          }   

         Grid().CalculateTriNormalsAndCentroids();
         
       }
           
    }
    
    // Calculate body length
    
    Distance = 0.;
    
    AverageChord_ = sqrt( pow(x_plate(1,1) - x_plate(NumPlateI_,1), 2.)
                        + pow(y_plate(1,1) - y_plate(NumPlateI_,1), 2.)
                        + pow(z_plate(1,1) - z_plate(NumPlateI_,1), 2.) );
                          
    // Create edge data
    
    Grid().CreateTriEdges();

    Grid().CalculateUpwindEdges();
    
    // Free up space
    
    delete [] NodePerm;
 
}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER CreateUpwindEdgeData                      #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::CreateUpwindEdgeData(int SurfaceID)
{
    
    int j, Node1, Node2;
    int Tri1, Tri2, NodeA, NodeB, NodeC, Case;
    int VortexLoop1IsDownWind, VortexLoop2IsDownWind;
    double VortexLoop1DownWindWeight, VortexLoop2DownWindWeight;

    VSP_NODE VSP_Node1, VSP_Node2;
    
    // Setup the surface vortex edge data
   
    for ( j = 1 ; j <= Grid().NumberOfEdges() ; j++ ) {

       // Pointers from vortex edge back to the parent wing or body and grid edge
       
       if ( SurfaceType() == WING_SURFACE ) {
       
          Grid().EdgeList(j).Wing() = SurfaceID;
          
          Grid().EdgeList(j).Body() = 0.;

       }
       
       else if ( SurfaceType() == BODY_SURFACE ) {
   
          Grid().EdgeList(j).Wing() = 0;
          
          Grid().EdgeList(j).Body() = SurfaceID;

       }

       // Pass in edge data and create edge cofficients
       
       Node1 = Grid().EdgeList(j).Node1();
       Node2 = Grid().EdgeList(j).Node2();

       Grid().EdgeList(j).Setup(Grid().NodeList(Node1), Grid().NodeList(Node2));
                                                  
       // Determine direction of edges
       
       Tri1 = Grid().EdgeList(j).Tri1();
       Tri2 = Grid().EdgeList(j).Tri2();
       
       NodeA = Grid().TriList(Tri1).Node1();
       NodeB = Grid().TriList(Tri1).Node2();
       NodeC = Grid().TriList(Tri1).Node3();
       
       Case = 1;
       
       if ( Node2 == NodeA && Node1 == NodeB ) Case = 2;
       if ( Node2 == NodeB && Node1 == NodeC ) Case = 2;
       if ( Node2 == NodeC && Node1 == NodeA ) Case = 2;

       // Based on directions, set loop 1 and 2 for each vortex edge 

       if ( Case == 1 ) {
        
          Grid().EdgeList(j).TriL() = Tri1;
          Grid().EdgeList(j).TriR() = Tri2;
          
          if ( Grid().EdgeList(j).TriL() == Grid().EdgeList(j).TriR() ) Grid().EdgeList(j).TriR() = 0;
          
          VortexLoop1IsDownWind = VortexLoop2IsDownWind = 0;
          
          // Check if loop 1 is Downwind of this edge

          if ( j == Grid().TriList(Tri1).Edge1() &&
                    Grid().TriList(Tri1).Edge1IsUpWind() ) { VortexLoop1IsDownWind = 1; VortexLoop1DownWindWeight = Grid().TriList(Tri1).Edge1UpwindWeight(); };

          if ( j == Grid().TriList(Tri1).Edge2() &&
                    Grid().TriList(Tri1).Edge2IsUpWind() ) { VortexLoop1IsDownWind = 1; VortexLoop1DownWindWeight = Grid().TriList(Tri1).Edge2UpwindWeight(); };

          if ( j == Grid().TriList(Tri1).Edge3() &&
                    Grid().TriList(Tri1).Edge3IsUpWind() ) { VortexLoop1IsDownWind = 1; VortexLoop1DownWindWeight = Grid().TriList(Tri1).Edge3UpwindWeight(); };

          // Check if loop 2 is Downwind of this edge
          
          if ( j == Grid().TriList(Tri2).Edge1() &&
                    Grid().TriList(Tri2).Edge1IsUpWind() ) { VortexLoop2IsDownWind = 1; VortexLoop2DownWindWeight = Grid().TriList(Tri2).Edge1UpwindWeight(); };

          if ( j == Grid().TriList(Tri2).Edge2() &&
                    Grid().TriList(Tri2).Edge2IsUpWind() ) { VortexLoop2IsDownWind = 1; VortexLoop2DownWindWeight = Grid().TriList(Tri2).Edge2UpwindWeight(); };

          if ( j == Grid().TriList(Tri2).Edge3() &&
                    Grid().TriList(Tri2).Edge3IsUpWind() ) { VortexLoop2IsDownWind = 1; VortexLoop2DownWindWeight = Grid().TriList(Tri2).Edge3UpwindWeight(); };

       }
       
       else {
        
          Grid().EdgeList(j).TriL() = Tri2;
          Grid().EdgeList(j).TriR() = Tri1;

          if ( Grid().EdgeList(j).TriL() == Grid().EdgeList(j).TriR() ) Grid().EdgeList(j).TriL() = 0;
 
          VortexLoop1IsDownWind = VortexLoop2IsDownWind = 0;
          
          // Check if loop 1 is Down Wind of this edge

          if ( j == Grid().TriList(Tri2).Edge1() &&
                    Grid().TriList(Tri2).Edge1IsUpWind() ) { VortexLoop1IsDownWind = 1; VortexLoop1DownWindWeight = Grid().TriList(Tri2).Edge1UpwindWeight(); };

          if ( j == Grid().TriList(Tri2).Edge2() &&
                    Grid().TriList(Tri2).Edge2IsUpWind() ) { VortexLoop1IsDownWind = 1; VortexLoop1DownWindWeight = Grid().TriList(Tri2).Edge2UpwindWeight(); };

          if ( j == Grid().TriList(Tri2).Edge3() &&
                    Grid().TriList(Tri2).Edge3IsUpWind() ) { VortexLoop1IsDownWind = 1; VortexLoop1DownWindWeight = Grid().TriList(Tri2).Edge3UpwindWeight(); };

          // Check if loop 2 is Down Wind of this edge
          
          if ( j == Grid().TriList(Tri1).Edge1() &&
                    Grid().TriList(Tri1).Edge1IsUpWind() ) { VortexLoop2IsDownWind = 1; VortexLoop2DownWindWeight = Grid().TriList(Tri1).Edge1UpwindWeight(); };

          if ( j == Grid().TriList(Tri1).Edge2() &&
                    Grid().TriList(Tri1).Edge2IsUpWind() ) { VortexLoop2IsDownWind = 1; VortexLoop2DownWindWeight = Grid().TriList(Tri1).Edge2UpwindWeight(); };

          if ( j == Grid().TriList(Tri1).Edge3() &&
                    Grid().TriList(Tri1).Edge3IsUpWind() ) { VortexLoop2IsDownWind = 1; VortexLoop2DownWindWeight = Grid().TriList(Tri1).Edge3UpwindWeight(); };
 
       }                                  

       // And keep track of which loops are down wind of this edge, and their weights
    
       Grid().EdgeList(j).VortexLoop1IsDownWind() = VortexLoop1IsDownWind;
       Grid().EdgeList(j).VortexLoop2IsDownWind() = VortexLoop2IsDownWind;
       
       Grid().EdgeList(j).VortexLoop1DownWindWeight() = VortexLoop1DownWindWeight;         
       Grid().EdgeList(j).VortexLoop2DownWindWeight() = VortexLoop2DownWindWeight;
     
    }

}

/*##############################################################################
#                                                                              #
#                                 VSP_SURFACE                                  #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::AgglomerateMesh(void)
{

    int i;

    VSP_AGGLOM Agglomerate;
    
    printf("Wing MaxNumberOfGridLevels_: %d \n",MaxNumberOfGridLevels_);
 
    if ( SurfaceType_ == WING_SURFACE ) printf("Wing Grid:%d --> Number of loops: %d \n",0,Grid_[0]->NumberOfTris());
    if ( SurfaceType_ == BODY_SURFACE ) printf("Body Grid:%d --> Number of loops: %d \n",0,Grid_[0]->NumberOfTris());
    
    // First attempt to simplify the grid

    Grid_[1] = Agglomerate.SimplifyMesh(*(Grid_[0]));

    if ( SurfaceType_ == WING_SURFACE ) printf("Wing Grid:%d --> Number of loops: %d \n",1,Grid_[1]->NumberOfTris());
    if ( SurfaceType_ == BODY_SURFACE ) printf("Body Grid:%d --> Number of loops: %d \n",1,Grid_[1]->NumberOfTris());
        
    i = 2;
    
    while ( i < MaxNumberOfGridLevels_ && Grid_[i-1]->NumberOfTris() > 1 ) {

       Grid_[i] = Agglomerate.Agglomerate(*(Grid_[i-1]));
       
       if ( SurfaceType_ == WING_SURFACE ) printf("Wing Grid:%d --> Number of loops: %d \n",i,Grid_[i]->NumberOfTris());
       if ( SurfaceType_ == BODY_SURFACE ) printf("Body Grid:%d --> Number of loops: %d \n",i,Grid_[i]->NumberOfTris());
       
       i++;

    }
    
    NumberOfGridLevels_ = i;
    
    printf("NumberOfGridLevels_: %d \n",NumberOfGridLevels_);
 
}



