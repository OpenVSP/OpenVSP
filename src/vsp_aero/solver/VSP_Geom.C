//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Geom.H"

/*##############################################################################
#                                                                              #
#                         VSP_GEOM constructor                                 #
#                                                                              #
##############################################################################*/

VSP_GEOM::VSP_GEOM(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                             VSP_GEOM init                                    #
#                                                                              #
##############################################################################*/

void VSP_GEOM::init(void)
{

    Verbose_ = 0;
    
    LoadDeformationFile_ = 0;
    
    DoGroundEffectsAnalysis_ = 0;
    
    VehicleRotationAngleVector_[0] = 0.;    
    VehicleRotationAngleVector_[1] = 0.;    
    VehicleRotationAngleVector_[2] = 0.;    
    
    DoBladeElementAnalysis_ = 0;

}

/*##############################################################################
#                                                                              #
#                            VSP_GEOM Copy                                     #
#                                                                              #
##############################################################################*/

VSP_GEOM::VSP_GEOM(const VSP_GEOM &VSP_Geom)
{

    init();

    // Just * use the operator = code

    *this = VSP_Geom;

}

/*##############################################################################
#                                                                              #
#                           VSP_GEOM operator=                                 #
#                                                                              #
##############################################################################*/

VSP_GEOM& VSP_GEOM::operator=(const VSP_GEOM &VSP_Geom)
{

    return *this;

}

/*##############################################################################
#                                                                              #
#                        VSP_GEOM destructor                                   #
#                                                                              #
##############################################################################*/

VSP_GEOM::~VSP_GEOM(void)
{


}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM ReadFile                                    #
#                                                                              #
##############################################################################*/

void VSP_GEOM::ReadFile(char *FileName, int &ModelType, int &SurfaceType)
{
 
    char Degen_File_Name[2000];
    char CART3D_File_Name[2000];
    char VSPGEOM_File_Name[2000];
    
    FILE *File;
     
    // VSP Degen file

    SPRINTF(Degen_File_Name,"%s.csv",FileName);
    SPRINTF(CART3D_File_Name,"%s.tri",FileName);
    SPRINTF(VSPGEOM_File_Name,"%s.vspgeom",FileName);

    if ( (File = fopen(Degen_File_Name,"r")) != NULL ) {
        
       fclose(File);
       
       Read_VSP_Degen_File(FileName);
       
       ModelType_ = VLM_MODEL;
       
       SurfaceType_ = DEGEN_SURFACE;
       
    }       
    
    // CART3D file
    
    else if ( (File = fopen(CART3D_File_Name,"r")) != NULL ) {

      fclose(File);
      
      Read_CART3D_File(FileName);
      
      ModelType_ = PANEL_MODEL;
      
      SurfaceType_ = CART3D_SURFACE;
          
    }

    // VSPGEOM file
    
    else {
 
       if ( (File = fopen(VSPGEOM_File_Name,"r")) != NULL ) {

          fclose(File);
       
          Read_VSPGEOM_File(FileName);
          
          ModelType_ = PANEL_MODEL;
          
          SurfaceType_ = VSPGEOM_SURFACE;
                    
       }
       
       else {

          printf("Could not load %s VSP Degen Geometry, CART3D Tri, or VSPGEOM Tri file... \n", FileName);fflush(NULL);

          exit(1);
          
       }
              
    }
        
    printf("NumberOfSurfaces_: %d \n",NumberOfSurfaces_);    

    // Load in FEM analysis data
    
    if ( LoadDeformationFile_ ) LoadFEMDeformationData(FileName);

    // Create meshes for the VSP geometries
    
    MeshGeom();
    
    // If this is a VSPGEOM, determine if it's a VLM or PANEL model
    
    if ( SurfaceType_ == VSPGEOM_SURFACE ) {
       
       DetermineModelType();
       
    }
    
    ModelType = ModelType_;
    
    SurfaceType = SurfaceType_;
            
}

/*##############################################################################
#                                                                              #
#                      VSP_GEOM LoadFEMDeformationData                         #
#                                                                              #
##############################################################################*/

void VSP_GEOM::LoadFEMDeformationData(char *FileName)
{
   
    int i;
    char FEM_File_Name[2000];
    FILE *File;
        
    for ( i = 1 ; i <= NumberOfSurfaces_ ; i++ ) {
       
       // We only do FEM for wings...
       
       if ( VSP_Surface(i).SurfaceType() == DEGEN_WING_SURFACE ) {
 
          SPRINTF(FEM_File_Name,"%s.Surface.%d.dfm",FileName,i);
       
          // If a deformation solution exists... load it in
          
          if ( (File = fopen(FEM_File_Name,"r")) != NULL ) {
           
             fclose(File);
             
             printf("Reading in a FEM deformation file for wing surface: %d \n",i);
          
             VSP_Surface(i).LoadFEMDeformationData(FEM_File_Name);
             
             VSP_Surface(i).FEMDeformGeometry();

          }             
          
       } 
       
    }
   
}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM Read_CART3D_File                            #
#                                                                              #
##############################################################################*/

void VSP_GEOM::Read_CART3D_File(char *FileName)
{

    int Done;
    char VSP_File_Name[2000], VSP_Degen_File_Name[2000], Name[2000], DumChar[2000];
    VSPAERO_DOUBLE Diam, x, y, z, nx, ny, nz;
    FILE *Cart3D_File, *VSP_Degen_File;
 
    SPRINTF(VSP_File_Name,"%s.tri",FileName);
    
    if ( (Cart3D_File = fopen(VSP_File_Name,"r")) == NULL ) {

       printf("Could not load %s CART3D file... \n", VSP_File_Name);fflush(NULL);

       exit(1);

    }    
         
    // Now size the surface list
    
    NumberOfDegenWings_     = 0;
    NumberOfDegenBodies_    = 0;
    NumberOfSurfaces_       = 1;
    
    VSP_Surface_ = new VSP_SURFACE[NumberOfSurfaces_ + 1];

    // If this is a ground effects analysis, set flag
    
    if ( DoGroundEffectsAnalysis_ ) {

       VSP_Surface(1).DoGroundEffectsAnalysis() = 1;
      
       VSP_Surface(1).GroundEffectsRotationAngle() = VehicleRotationAngleVector(1);
      
       VSP_Surface(1).GroundEffectsCGLocation(0) = VehicleRotationAxisLocation(0);
       VSP_Surface(1).GroundEffectsCGLocation(1) = VehicleRotationAxisLocation(1);
       VSP_Surface(1).GroundEffectsCGLocation(2) = VehicleRotationAxisLocation(2);

       VSP_Surface(1).GroundEffectsHeightAboveGround() = HeightAboveGround();

    }
        
    // Read in the cart3d geometry

    SPRINTF(Name,"CART3D");

    VSP_Surface(1).ReadCart3DDataFromFile(Name,Cart3D_File);
    
    NumberOfCart3dSurfaces_ = NumberOfSurfacePatches_ = VSP_Surface(1).NumberOfSurfacePatches();
    
    NumberOfComponents_ = NumberOfCart3dSurfaces_;

    fclose(Cart3D_File);
    
    // Now see if a degen file exists

    SPRINTF(VSP_Degen_File_Name,"%s_DegenGeom.csv",FileName);

    if ( (VSP_Degen_File = fopen(VSP_Degen_File_Name,"r")) != NULL ) {

       // See if any rotors are defined
       
       NumberOfRotors_ = 0;
       
       Done = 0;
       
       while ( !Done ) {
        
          if ( fgets(DumChar,1000,VSP_Degen_File) == NULL ) Done = 1;   
          
          if ( strncmp(DumChar,"DISK",4) == 0 ) NumberOfRotors_++;
          
       }        
          
       if ( NumberOfRotors_ > 0 ) {
          
          rewind(VSP_Degen_File);  
       
          RotorDisk_ = new ROTOR_DISK[NumberOfRotors_ + 1];
          
          NumberOfRotors_ = 0;
          
          Done = 0;
          
          while ( !Done ) {
        
             if ( fgets(DumChar,1000,VSP_Degen_File) == NULL ) Done = 1;   
          
             if ( strncmp(DumChar,"DISK",4) == 0 ) {
                
                NumberOfRotors_++;
                
                fgets(DumChar,1000,VSP_Degen_File);  
                fgets(DumChar,1000,VSP_Degen_File);  
                fgets(DumChar,1000,VSP_Degen_File);  
                fgets(DumChar,1000,VSP_Degen_File);  
             
                sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %lf, %lf",
                       &Diam,
                       &x,
                       &y,
                       &z,
                       &nx,
                       &ny,
                       &nz);
                
                // VSP supplied information
                       
                RotorDisk(NumberOfRotors_).Radius() = 0.5*Diam;
   
                RotorDisk(NumberOfRotors_).XYZ(0) = x;
                RotorDisk(NumberOfRotors_).XYZ(1) = y;
                RotorDisk(NumberOfRotors_).XYZ(2) = z;
   
                RotorDisk(NumberOfRotors_).Normal(0) = -nx;
                RotorDisk(NumberOfRotors_).Normal(1) = -ny;
                RotorDisk(NumberOfRotors_).Normal(2) = -nz;
                
                // Some defaults
                
                RotorDisk(NumberOfRotors_).CT() = 0.400;
                RotorDisk(NumberOfRotors_).CP() = 0.600;
                RotorDisk(NumberOfRotors_).RPM() = 2000.;
                
             }
             
          }
          
       }
       
       printf("Found: %d Rotors \n",NumberOfRotors_);
       
       rewind(VSP_Degen_File);  
              
       // Now search for control surface hinges
       
       // Close file
       
       fclose(VSP_Degen_File);
       
    }    

}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM Read_VSPGEOM_File                           #
#                                                                              #
##############################################################################*/

void VSP_GEOM::Read_VSPGEOM_File(char *FileName)
{

    int Done;
    char VSPGEOM_File_Name[2000], VSP_Degen_File_Name[2000], Name[2000], DumChar[2000];
    VSPAERO_DOUBLE Diam, x, y, z, nx, ny, nz;
    FILE *VSPGEOM_File, *VSP_Degen_File;
 
    SPRINTF(VSPGEOM_File_Name,"%s.vspgeom",FileName);
    
    if ( (VSPGEOM_File = fopen(VSPGEOM_File_Name,"r")) == NULL ) {

       printf("Could not load %s VSPGEOM file... \n", VSPGEOM_File_Name);fflush(NULL);

       exit(1);

    }    
         
    // Now size the surface list
    
    NumberOfDegenWings_     = 0;
    NumberOfDegenBodies_    = 0;
    NumberOfSurfaces_       = 1;
    
    VSP_Surface_ = new VSP_SURFACE[NumberOfSurfaces_ + 1];

    // If this is a ground effects analysis, set flag
    
    if ( DoGroundEffectsAnalysis_ ) {

       VSP_Surface(1).DoGroundEffectsAnalysis() = 1;
      
       VSP_Surface(1).GroundEffectsRotationAngle() = VehicleRotationAngleVector(1);
      
       VSP_Surface(1).GroundEffectsCGLocation(0) = VehicleRotationAxisLocation(0);
       VSP_Surface(1).GroundEffectsCGLocation(1) = VehicleRotationAxisLocation(1);
       VSP_Surface(1).GroundEffectsCGLocation(2) = VehicleRotationAxisLocation(2);

       VSP_Surface(1).GroundEffectsHeightAboveGround() = HeightAboveGround();

    }
        
    // Read in the geometry

    printf("1... \n");fflush(NULL);

    SPRINTF(Name,"VSPGEOM");

    VSP_Surface(1).ReadVSPGeomDataFromFile(Name,VSPGEOM_File);
    
    NumberOfCart3dSurfaces_ = NumberOfSurfacePatches_ = VSP_Surface(1).NumberOfSurfacePatches();

    NumberOfComponents_ = NumberOfCart3dSurfaces_;

    fclose(VSPGEOM_File);
 
    // Now see if a degen file exists

    SPRINTF(VSP_Degen_File_Name,"%s_DegenGeom.csv",FileName);

    if ( (VSP_Degen_File = fopen(VSP_Degen_File_Name,"r")) != NULL ) {

       // See if any rotors are defined
       
       NumberOfRotors_ = 0;
       
       Done = 0;
       
       while ( !Done ) {
        
          if ( fgets(DumChar,1000,VSP_Degen_File) == NULL ) Done = 1;   
          
          if ( strncmp(DumChar,"DISK",4) == 0 ) NumberOfRotors_++;
          
       }        
          
       if ( NumberOfRotors_ > 0 ) {
          
          rewind(VSP_Degen_File);  
       
          RotorDisk_ = new ROTOR_DISK[NumberOfRotors_ + 1];
          
          NumberOfRotors_ = 0;
          
          Done = 0;
          
          while ( !Done ) {
        
             if ( fgets(DumChar,1000,VSP_Degen_File) == NULL ) Done = 1;   
          
             if ( strncmp(DumChar,"DISK",4) == 0 ) {
                
                NumberOfRotors_++;
                
                fgets(DumChar,1000,VSP_Degen_File);  
                fgets(DumChar,1000,VSP_Degen_File);  
                fgets(DumChar,1000,VSP_Degen_File);  
                fgets(DumChar,1000,VSP_Degen_File);  
             
                sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %lf, %lf",
                       &Diam,
                       &x,
                       &y,
                       &z,
                       &nx,
                       &ny,
                       &nz);
                
                // VSP supplied information
                       
                RotorDisk(NumberOfRotors_).Radius() = 0.5*Diam;
   
                RotorDisk(NumberOfRotors_).XYZ(0) = x;
                RotorDisk(NumberOfRotors_).XYZ(1) = y;
                RotorDisk(NumberOfRotors_).XYZ(2) = z;
   
                RotorDisk(NumberOfRotors_).Normal(0) = -nx;
                RotorDisk(NumberOfRotors_).Normal(1) = -ny;
                RotorDisk(NumberOfRotors_).Normal(2) = -nz;
                
                // Some defaults
                
                RotorDisk(NumberOfRotors_).CT() = 0.400;
                RotorDisk(NumberOfRotors_).CP() = 0.600;
                RotorDisk(NumberOfRotors_).RPM() = 2000.;
                
             }
             
          }
          
       }
       
       printf("Found: %d Rotors \n",NumberOfRotors_);
       
       rewind(VSP_Degen_File);  
              
       // Now search for control surface hinges
       
       // Close file
       
       fclose(VSP_Degen_File);
       
    }    

}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM Read_VSP_Degen_File                         #
#                                                                              #
##############################################################################*/

void VSP_GEOM::Read_VSP_Degen_File(char *FileName)
{

    int i, Wing, Done, NumberOfBodySets, BodySet, Surface, GeomIDFlags, SymCopyNdxFlags;
    int TotalNumberOfWings, TotalNumberOfBodies;
    int *ReadInThisWing, *ReadInThisBody, ComponentID, SurfFlag ;
    VSPAERO_DOUBLE Diam, x, y, z, nx, ny, nz, Epsilon, MinVal, MaxVal;
    char VSP_File_Name[2000], DumChar[2000], Type[2000], Name[2000];
    char GeomID[2000], LastGeomID[2000], SurfNdx[2000], LastSurfNdx[2000];
    char MainSurfNdx[2000], SymCopyNdx[2000], LastSymCopyNdx[2000];
    char Comma[2000], *Next;
    VSP_SURFACE SurfaceParser;
    BBOX ComponentBBox;
    FILE *VSP_Degen_File;
    
    MinVal = MaxVal = 0.;
    
    ComponentID = 0;
    
    GeomIDFlags = 0;
    
    SymCopyNdxFlags = 0;
    
    SPRINTF(Comma,",");
    
    // Open degen file

    SPRINTF(VSP_File_Name,"%s.csv",FileName);

    if ( (VSP_Degen_File = fopen(VSP_File_Name,"r")) == NULL ) {

       // No VSP degen file... exit

       printf("Could not load %s VSP Degen Geometry file... \n", VSP_File_Name);fflush(NULL);

       exit(1);

    }    
    
    // Now read in the data
    
    fgets(DumChar,1000,VSP_Degen_File);
    fgets(DumChar,1000,VSP_Degen_File);
    fgets(DumChar,1000,VSP_Degen_File);
    fgets(DumChar,1000,VSP_Degen_File); sscanf(DumChar,"%d\n",&NumberOfComponents_);
    
    printf("Number Of Surfaces: %d \n",NumberOfComponents_);
    
    // Now scan the file and determine how many wings in total
    
    TotalNumberOfWings = 0;
    
    Done = 0;
    
    while ( !Done ) {
     
       if ( fgets(DumChar,1000,VSP_Degen_File) == NULL ) Done = 1;   
       
       if ( strstr(DumChar,"GeomID") != NULL ) GeomIDFlags = 1;
       
       if ( strstr(DumChar,"SymCopyNdx") != NULL ) SymCopyNdxFlags = 1;
       
       if ( strncmp(DumChar,"LIFTING_SURFACE",15) == 0 ) TotalNumberOfWings++;
       
    }
    
    if ( GeomIDFlags ) {
       
       printf("GeomIDFlags are defined! \n");
       
    }
    
    else {
       
       printf("GeomIDFlags are NOT defined! \n");       
       
    }
    
    ReadInThisWing = new int[TotalNumberOfWings + 1];
    
    zero_int_array(ReadInThisWing, TotalNumberOfWings);
    
    rewind(VSP_Degen_File);
        
    // Now, depending on symmetry, read in the correct subset of wings
    
    if ( DoSymmetryPlaneSolve_ == 0 ) {
       
       NumberOfDegenWings_ = TotalNumberOfWings;
       
       for ( i = 1 ; i <= NumberOfDegenWings_ ; i++ ) {
          
          ReadInThisWing[i] = 1;
          
       }
       
    }
    
    else {
        
       NumberOfDegenWings_ = 0;
       
       i = Done = 0;
       
       while ( !Done ) {
        
          if ( fgets(DumChar,1000,VSP_Degen_File) == NULL ) Done = 1;   
          
          if ( strncmp(DumChar,"LIFTING_SURFACE",15) == 0 ) {
             
             // Determine if this wing component should be kept

             SurfaceParser.GetComponentBBox(VSP_Degen_File, ComponentBBox);
             
             i++;
 
             if ( DoSymmetryPlaneSolve_ == SYM_X ) { Epsilon = MAX(0.025*(ComponentBBox.x_max - ComponentBBox.x_min), 1.e-5); };
             
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) { Epsilon = MAX(0.025*(ComponentBBox.y_max - ComponentBBox.y_min), 1.e-5); };
              
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) { Epsilon = MAX(0.025*(ComponentBBox.z_max - ComponentBBox.z_min), 1.e-5); };

             if ( DoSymmetryPlaneSolve_ == SYM_X && ComponentBBox.x_min >= -Epsilon ) { NumberOfDegenWings_++ ; ReadInThisWing[i] = 1; };
             
             if ( DoSymmetryPlaneSolve_ == SYM_Y && ComponentBBox.y_min >= -Epsilon ) { NumberOfDegenWings_++ ; ReadInThisWing[i] = 1; };
              
             if ( DoSymmetryPlaneSolve_ == SYM_Z && ComponentBBox.z_min >= -Epsilon ) { NumberOfDegenWings_++ ; ReadInThisWing[i] = 1; };
                           
          }
                                        
       }
       
    }
    
    printf("NumberOfDegenWings_: %d \n",NumberOfDegenWings_);
    
    rewind(VSP_Degen_File);
    
    // Now scan the file and determine how many bodies
    
    TotalNumberOfBodies = 0;
    
    Done = 0;
    
    while ( !Done ) {
     
       if ( fgets(DumChar,1000,VSP_Degen_File) == NULL ) Done = 1;   
       
       if ( strncmp(DumChar,"BODY",4) == 0 ) TotalNumberOfBodies++;
       
    }  
    
    ReadInThisBody = new int[TotalNumberOfBodies + 1];
    
    zero_int_array(ReadInThisBody, TotalNumberOfBodies);
    
    rewind(VSP_Degen_File);
    
    // Now, depending on symmetry, read in the correct subset of bodies
    
    Epsilon = 1.e-5;
    
    if ( DoSymmetryPlaneSolve_ == 0 ) {
       
       NumberOfDegenBodies_ = TotalNumberOfBodies;
       
       for ( i = 1 ; i <= TotalNumberOfBodies ; i++ ) {
          
          ReadInThisBody[i] = 1;
          
       }
       
    }
    
    else {
        
       NumberOfDegenBodies_ = 0;
       
       i = Done = 0;
       
       while ( !Done ) {
        
          if ( fgets(DumChar,1000,VSP_Degen_File) == NULL ) Done = 1;   
          
          if ( strncmp(DumChar,"BODY",4) == 0 ) {
             
             // Determine if this body component should be kept

             SurfaceParser.GetComponentBBox(VSP_Degen_File, ComponentBBox);
             
             i++;
             
             if ( DoSymmetryPlaneSolve_ == SYM_X ) {
                
                MinVal = ComponentBBox.x_min;
                MaxVal = ComponentBBox.x_max;
                
             }
             
             if ( DoSymmetryPlaneSolve_ == SYM_Y ) {
                
                MinVal = ComponentBBox.y_min;
                MaxVal = ComponentBBox.y_max;
                
             }
             
             if ( DoSymmetryPlaneSolve_ == SYM_Z ) {
                
                MinVal = ComponentBBox.z_min;
                MaxVal = ComponentBBox.z_max;
                
             }                          

             if ( MinVal >= 0. ) {
                   
                NumberOfDegenBodies_++ ;
                
                ReadInThisBody[i] = 1;
                
             }
             
             else if ( ABS(MinVal + MaxVal) <= Epsilon ) {
                
                NumberOfDegenBodies_++;
                
                ReadInThisBody[i] = -DoSymmetryPlaneSolve_;

             }
          
          }
                                        
       }
       
    }
    
    printf("NumberOfDegenBodies_: %d \n",NumberOfDegenBodies_);
    
    rewind(VSP_Degen_File);    

    // Now scan the file and determine how many actuator disks / blade element models
    
    NumberOfRotors_ = NumberOfBladeElementSurfaces_ = 0;
    
    Done = 0;
    
    while ( !Done ) {
     
       if ( fgets(DumChar,1000,VSP_Degen_File) == NULL ) Done = 1;   
       
       if ( strncmp(DumChar,"DISK",4) == 0 ) {
          
          NumberOfRotors_++;
       
       }
       
    }        
       
    if ( DoBladeElementAnalysis_ ) NumberOfBladeElementSurfaces_ = NumberOfRotors_;

    if ( NumberOfRotors_ > 0 ) {
       
       rewind(VSP_Degen_File);  
    
       RotorDisk_ = new ROTOR_DISK[NumberOfRotors_ + 1];
       
       NumberOfRotors_ = 0;
       
       Done = 0;
       
       while ( !Done ) {
     
          if ( fgets(DumChar,1000,VSP_Degen_File) == NULL ) Done = 1;   
       
          if ( strncmp(DumChar,"DISK",4) == 0 ) {
             
             NumberOfRotors_++;
             
             fgets(DumChar,1000,VSP_Degen_File);  
             fgets(DumChar,1000,VSP_Degen_File);  
             fgets(DumChar,1000,VSP_Degen_File);  
             fgets(DumChar,1000,VSP_Degen_File);  
          
             sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %lf, %lf",
                    &Diam,
                    &x,
                    &y,
                    &z,
                    &nx,
                    &ny,
                    &nz);
             
             // VSP supplied information
                    
             RotorDisk(NumberOfRotors_).Radius() = 0.5*Diam;

             RotorDisk(NumberOfRotors_).XYZ(0) = x;
             RotorDisk(NumberOfRotors_).XYZ(1) = y;
             RotorDisk(NumberOfRotors_).XYZ(2) = z;

             RotorDisk(NumberOfRotors_).Normal(0) = -nx;
             RotorDisk(NumberOfRotors_).Normal(1) = -ny;
             RotorDisk(NumberOfRotors_).Normal(2) = -nz;
             
             // Some defaults
             
             RotorDisk(NumberOfRotors_).CT() = 0.400;
             RotorDisk(NumberOfRotors_).CP() = 0.600;
             RotorDisk(NumberOfRotors_).RPM() = 2000.;
             
          }
          
       }
       
    }
    
    printf("Found: %d Rotors \n",NumberOfRotors_);
    
    // Split body data into 4 sets... top/bottom vertical, and left/right horizontal slices
    
    NumberOfBodySets = NumberOfDegenBodies_;
    
    NumberOfDegenBodies_ = 0;
    
    for ( i = 1 ; i <= TotalNumberOfBodies ; i++ ) {
       
       if( ReadInThisBody[i] == 1 ) NumberOfDegenBodies_ += 4;
       
       if( ReadInThisBody[i] <  0 ) NumberOfDegenBodies_ += 1;
       
    }

    printf("NumberOfDegenBodies_: %d \n",NumberOfDegenBodies_);

    rewind(VSP_Degen_File);

    // Now size the surface list
    
    NumberOfSurfaces_ = NumberOfSurfacePatches_ = NumberOfDegenWings_ + NumberOfDegenBodies_;

    VSP_Surface_ = new VSP_SURFACE[NumberOfSurfaces_ + 1];
    
    // If this is a ground effects analysis, set flag
    
    if ( DoGroundEffectsAnalysis_ ) {
    
       for ( i = 1 ; i <= NumberOfSurfaces_ ; i++ ) {
         
          VSP_Surface(i).DoGroundEffectsAnalysis() = 1;
         
          VSP_Surface(i).GroundEffectsRotationAngle() = VehicleRotationAngleVector(1);
         
          VSP_Surface(i).GroundEffectsCGLocation(0) = VehicleRotationAxisLocation(0);
          VSP_Surface(i).GroundEffectsCGLocation(1) = VehicleRotationAxisLocation(1);
          VSP_Surface(i).GroundEffectsCGLocation(2) = VehicleRotationAxisLocation(2);

          VSP_Surface(i).GroundEffectsHeightAboveGround() = HeightAboveGround();
         
       }
      
    }
 
    // Read in the wing data
    
    SPRINTF(LastGeomID," ");
    
    SPRINTF(LastSymCopyNdx," ");
    
    Surface = 0;

    for ( Wing = 1 ; Wing <= TotalNumberOfWings ; Wing++ ) {

       Done = 0;
       
       while ( !Done ) {

          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strncmp(DumChar,"LIFTING_SURFACE",15) == 0 ) Done = 1;
     
       }
       
       if ( ReadInThisWing[Wing] ) {
          
          Surface++;

          Next = strtok(DumChar,Comma); Next[strcspn(Next, "\n")] = 0;  SPRINTF(Type,"%s",Next);            
          Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;  SPRINTF(Name,"%s",Next);       
          
          SurfFlag = 0;
                       
          if ( GeomIDFlags ) {
        
             Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;  SPRINTF(DumChar,    "%s",Next);    
             Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;  SPRINTF(GeomID,     "%s",Next);

             if ( SymCopyNdxFlags ) {
                
                Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;  SPRINTF(MainSurfNdx,"%s",Next);
                Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;  SPRINTF(SymCopyNdx, "%s",Next);
                
             }

             if ( strncmp(DumChar,"1",1) == 0 ) SurfFlag = 1;
             
          }
          
          else {
             
              SPRINTF(GeomID,"%s",Name);
              
          }

          if ( Verbose_ ) printf("Working on reading wing: %d --> Name: %s with GeomID: %s ... MainSurfNdx: %s ... SymCopyNdx: %s \n",Wing,Name,GeomID,MainSurfNdx,SymCopyNdx);

          VSP_Surface(Surface).ReadWingDataFromFile(Name,VSP_Degen_File);
          
          if ( strcmp(LastGeomID,GeomID) != 0 || ( strcmp(LastGeomID,GeomID) == 0 && strcmp(LastSymCopyNdx,SymCopyNdx) != 0 ) ) ComponentID++;
          
          SPRINTF(LastGeomID,"%s",GeomID);
          
          SPRINTF(LastSymCopyNdx,"%s",SymCopyNdx);
          
          VSP_Surface(Surface).ComponentID() = ComponentID;
          
       }
       
    }
        
    // Read in the body data

    SPRINTF(LastGeomID," ");
    
    SPRINTF(LastSurfNdx," ");
        
    for ( BodySet = 1 ; BodySet <= NumberOfBodySets ; BodySet++ ) {
        
       // Load in the horizontal slices ... read in full geometry
    
       rewind(VSP_Degen_File);
    
       Done = 0;
       
       i = 0;
       
       while ( Done < BodySet && i < TotalNumberOfBodies ) {

          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strncmp(DumChar,"BODY",4) == 0 ) {
             
             i++;
             
             if ( ReadInThisBody[i] == 1 ) Done++;
             
          }
                    
       }
       
       if ( Done == BodySet ) {
          
          Surface++;
            
          Next = strtok(DumChar,Comma); Next[strcspn(Next, "\n")] = 0;  SPRINTF(Type,"%s",Next);            
          Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;  SPRINTF(Name,"%s",Next);   
          
          if ( GeomIDFlags ) {
                 
             Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;  SPRINTF(SurfNdx,"%s",Next);
             Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;  SPRINTF(GeomID,"%s",Next);
             
          }
          
          else {
             
             SPRINTF(GeomID,"%s",Name);
             
          } 

          if ( Verbose_ ) printf("Working on reading #1 horizontal slice for body: %d --> %s ... SymFlag: %d and Component ID: %s \n",BodySet,Name,ReadInThisBody[i],GeomID); fflush(NULL);
                 
          VSP_Surface(Surface).ReadBodyDataFromFile(Name,2,VSP_Degen_File);
          
          if ( Verbose_ ) printf("LastGeomID, GeomID: %s %s .... LastSurfNdx,SurfNdx: %s %s \n",LastGeomID, GeomID, LastSurfNdx,SurfNdx);
          
          if ( strcmp(LastGeomID,GeomID) != 0 || ( strcmp(LastGeomID,GeomID) == 0 && strcmp(LastSurfNdx,SurfNdx) != 0 ) ) ComponentID++;
          
          SPRINTF(LastGeomID,"%s",GeomID);
          
          SPRINTF(LastSurfNdx,"%s",SurfNdx);
          
          VSP_Surface(Surface).ComponentID() = ComponentID;

       }

       // Load in the horizontal slices ... read in half span geometry ... as it lies on the symmetry plane
    
       rewind(VSP_Degen_File);
    
       Done = 0;
       
       i = 0;
       
       while ( Done < BodySet && i < TotalNumberOfBodies ) {

          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strncmp(DumChar,"BODY",4) == 0 ) {
             
             i++;
             
             if ( ReadInThisBody[i] == 1 || ReadInThisBody[i] == -2 ) Done++;
             
          }
                    
       }

       if ( Done == BodySet ) {
          
          Surface++;
   
          Next = strtok(DumChar,Comma); Next[strcspn(Next, "\n")] = 0;  SPRINTF(Type,"%s",Next);            
          Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;  SPRINTF(Name,"%s",Next);       
          
          if ( GeomIDFlags ) {
             
             Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;      
             Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;  SPRINTF(GeomID,"%s",Next); 
             
          }
          
          else {
             
             SPRINTF(GeomID,"%s",Name);
             
          }
             
          if ( Verbose_ ) printf("Working on reading #2 horizontal slice for body: %d --> %s ... SymFlag: %d and Component ID: %s \n",BodySet,Name,ReadInThisBody[i],GeomID); fflush(NULL);
                 
          VSP_Surface(Surface).ReadBodyDataFromFile(Name,1,VSP_Degen_File);
          
          if ( strcmp(LastGeomID,GeomID) != 0 ) ComponentID++;
          
          SPRINTF(LastGeomID,"%s",GeomID);
          
          VSP_Surface(Surface).ComponentID() = ComponentID;
          
       }
          
       // Load in the vertical slices
       
       rewind(VSP_Degen_File);
            
       Done = 0;
       
       i = 0;
       
       while ( Done < BodySet && i < TotalNumberOfBodies ) {

          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strncmp(DumChar,"BODY",4) == 0 ) {
             
             i++;
             
             if ( ReadInThisBody[i] == 1 ) Done++;
             
          }
                    
       }
       
       if ( Done == BodySet ) {
       
          Surface++;
       
          Next = strtok(DumChar,Comma); Next[strcspn(Next, "\n")] = 0;  SPRINTF(Type,"%s",Next);            
          Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;  SPRINTF(Name,"%s",Next);   
          
          if ( GeomIDFlags ) {
                  
             Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;      
             Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;  SPRINTF(GeomID,"%s",Next); 
             
          }
          
          else {
             
             SPRINTF(GeomID,"%s",Name);
             
          }
          
          if ( Verbose_ ) printf("Working on reading #1 vertical slice for body: %d --> %s ... SymFlag: %d and Component ID: %s \n",BodySet,Name,ReadInThisBody[i],GeomID); fflush(NULL);
                            
          VSP_Surface(Surface).ReadBodyDataFromFile(Name,3,VSP_Degen_File);    
          
          if ( strcmp(LastGeomID,GeomID) != 0 ) ComponentID++;
          
          SPRINTF(LastGeomID,"%s",GeomID);
          
          VSP_Surface(Surface).ComponentID() = ComponentID;         
           
       } 
       
       // Load in the vertical slices
       
       rewind(VSP_Degen_File);

       Done = 0;
       
       i = 0;
       
       while ( Done < BodySet && i < TotalNumberOfBodies ) {

          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strncmp(DumChar,"BODY",4) == 0 ) {
             
             i++;
             
             if ( ReadInThisBody[i] == 1 || ReadInThisBody[i] == -3 ) Done++;
             
          }
                    
       }
       
       if ( Done == BodySet ) {
               
          Surface++;
                      
          Next = strtok(DumChar,Comma); Next[strcspn(Next, "\n")] = 0;  SPRINTF(Type,"%s",Next);            
          Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;  SPRINTF(Name,"%s",Next);    
          
          if ( GeomIDFlags ) {
                           
             Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;      
             Next = strtok(NULL,Comma);    Next[strcspn(Next, "\n")] = 0;  SPRINTF(GeomID,"%s",Next); 

          }
          
          else {
             
             SPRINTF(GeomID,"%s",Name);
             
          }
          
          if ( Verbose_ ) printf("Working on reading #2 vertical slice for body: %d --> %s ... SymFlag: %d and Component ID: %s \n",BodySet,Name,ReadInThisBody[i],GeomID); fflush(NULL);
                 
          VSP_Surface(Surface).ReadBodyDataFromFile(Name,4,VSP_Degen_File);    
          
          if ( strcmp(LastGeomID,GeomID) != 0 ) ComponentID++;
          
          SPRINTF(LastGeomID,"%s",GeomID);
          
          VSP_Surface(Surface).ComponentID() = ComponentID;   
          
       }
          
    }
    
    NumberOfComponents_ = ComponentID;
    
    printf("NumberOfComponents_: %d \n", NumberOfComponents_);
    
    fclose(VSP_Degen_File);
    
    delete [] ReadInThisBody;
    delete [] ReadInThisWing;
    
    printf("Done loading in geometry! \n");fflush(NULL);

}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM MeshGeom                                    #
#                                                                              #
##############################################################################*/

void VSP_GEOM::MeshGeom(void)
{
 
    int i, Surface, NumberOfNodes, NumberOfLoops, NumberOfEdges, NumberOfKuttaNodes;
    int MaxNumberOfGridLevels, NodeOffSet, Done;
    VSPAERO_DOUBLE AreaTotal;
  
    // Loop over the surface and create a mesh for each

    for ( Surface = 1 ; Surface <= NumberOfSurfaces_ ; Surface++ ) {
     
       if ( VSP_Surface(Surface).SurfaceType() != CART3D_SURFACE &&
            VSP_Surface(Surface).SurfaceType() != VSPGEOM_SURFACE ) {
         
          VSP_Surface(Surface).CreateMesh(Surface);
         
       }
       
    }
    
    // Now merge the meshes into one single mesh
    
    NumberOfNodes = NumberOfLoops = NumberOfEdges = NumberOfKuttaNodes = 0;
    
    for ( Surface = 1 ; Surface <= NumberOfSurfaces_ ; Surface++ ) {
     
       NumberOfNodes      += VSP_Surface(Surface).Grid().NumberOfNodes();
       NumberOfLoops      += VSP_Surface(Surface).Grid().NumberOfLoops();
       NumberOfEdges      += VSP_Surface(Surface).Grid().NumberOfEdges();
       NumberOfKuttaNodes += VSP_Surface(Surface).Grid().NumberOfKuttaNodes();
       
    }    
           
    MaxNumberOfGridLevels = 100;
    
    Grid_ = new VSP_GRID*[MaxNumberOfGridLevels + 1];
    
    Grid_[0] = new VSP_GRID;

    Grid().SizeNodeList(NumberOfNodes);
    
    Grid().SizeTriList(NumberOfLoops);
 
    Grid().SizeKuttaNodeList(NumberOfKuttaNodes);    
    
    // Copy over the grid surface type
    
    Grid().SurfaceType() = VSP_Surface(1).Grid().SurfaceType();
    
    // Copy over nodes
    
    NumberOfNodes = 0;
    
    for ( Surface = 1 ; Surface <= NumberOfSurfaces_ ; Surface++ ) {
     
       for ( i = 1 ; i <= VSP_Surface(Surface).Grid().NumberOfNodes() ; i++ ) {
        
          Grid().NodeList(++NumberOfNodes) = VSP_Surface(Surface).Grid().NodeList(i);
          
       }
       
    }

    // Copy over loops
    
    NumberOfLoops = NodeOffSet = 0;

    for ( Surface = 1 ; Surface <= NumberOfSurfaces_ ; Surface++ ) {       
       
       for ( i = 1 ; i <= VSP_Surface(Surface).Grid().NumberOfLoops() ; i++ ) {
        
          Grid().LoopList(++NumberOfLoops) = VSP_Surface(Surface).Grid().LoopList(i);
          
          Grid().LoopList(NumberOfLoops).Node1() += NodeOffSet;
          Grid().LoopList(NumberOfLoops).Node2() += NodeOffSet;
          Grid().LoopList(NumberOfLoops).Node3() += NodeOffSet;
          
       }
       
       NodeOffSet += VSP_Surface(Surface).Grid().NumberOfNodes();
       
    }
    
    // Copy over kutta nodes
    
    NumberOfKuttaNodes = NodeOffSet = 0;
    
    for ( Surface = 1 ; Surface <= NumberOfSurfaces_ ; Surface++ ) {
     
       for ( i = 1 ; i <= VSP_Surface(Surface).Grid().NumberOfKuttaNodes() ; i++ ) {
        
          NumberOfKuttaNodes++;
          
          if ( VSP_Surface(Surface).SurfaceType() != CART3D_SURFACE && VSP_Surface(Surface).SurfaceType() != VSPGEOM_SURFACE ) {
          
             Grid().KuttaNode(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().KuttaNode(i) + NodeOffSet;

             Grid().KuttaNodeSoverB(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().KuttaNodeSoverB(i);
             
             Grid().WingSurfaceForKuttaNode(NumberOfKuttaNodes) = Surface;
             
             Grid().ComponentIDForKuttaNode(NumberOfKuttaNodes) = VSP_Surface(Surface).ComponentID();
            
             Grid().WingSurfaceForKuttaNodeIsPeriodic(NumberOfKuttaNodes) = 0;
   
             Grid().WakeTrailingEdgeX(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WakeTrailingEdgeX(i);
             Grid().WakeTrailingEdgeY(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WakeTrailingEdgeY(i);
             Grid().WakeTrailingEdgeZ(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WakeTrailingEdgeZ(i);
             
          }
          
          else {
             
             Grid().KuttaNode(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().KuttaNode(i) + NodeOffSet;
             
             Grid().KuttaNodeSoverB(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().KuttaNodeSoverB(i);
             
             Grid().WingSurfaceForKuttaNode(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WingSurfaceForKuttaNode(i);
             
             Grid().WingSurfaceForKuttaNodeIsPeriodic(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WingSurfaceForKuttaNodeIsPeriodic(i);
             
             Grid().ComponentIDForKuttaNode(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().ComponentIDForKuttaNode(i);
             
             Grid().KuttaNodeIsOnWingTip(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().KuttaNodeIsOnWingTip(i);

             Grid().WakeTrailingEdgeX(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WakeTrailingEdgeX(i);
             Grid().WakeTrailingEdgeY(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WakeTrailingEdgeY(i);
             Grid().WakeTrailingEdgeZ(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WakeTrailingEdgeZ(i);
            
          }             
          
       }
       
       NodeOffSet += VSP_Surface(Surface).Grid().NumberOfNodes();
       
    }
    
    printf("NumberOfKuttaNodes: %d \n",NumberOfKuttaNodes);

#ifdef AUTODIFF

    // Start recording here to get partials wrt mesh 

    adept::Stack *AutoDiffStack;
    
    AutoDiffStack = adept::active_stack();

    AutoDiffStack->continue_recording();
    
    AutoDiffStack->new_recording();
      
#endif
      
    Grid().CalculateTriNormalsAndCentroids();
    
    Grid().CreateTriEdges();
    
    Grid().CalculateUpwindEdges();   
    
    Grid().CreateUpwindEdgeData();

    for ( Surface = 1 ; Surface <= NumberOfSurfaces_ ; Surface++ ) {
     
       if ( VSP_Surface(Surface).SurfaceType() == CART3D_SURFACE || VSP_Surface(Surface).SurfaceType() == VSPGEOM_SURFACE ) {
         
          // Copy over edge data
          
          printf("Copying edge data... \n");fflush(NULL);
          
          for ( i = 1 ; i <= Grid().NumberOfEdges() ; i++ ) {
  
             Grid().EdgeList(i).IsTrailingEdge() = VSP_Surface(Surface).Grid().EdgeList(i).IsTrailingEdge();
             
             Grid().EdgeList(i).IsBoundaryEdge() = VSP_Surface(Surface).Grid().EdgeList(i).IsBoundaryEdge();
             
             Grid().EdgeList(i).IsLeadingEdge()  = VSP_Surface(Surface).Grid().EdgeList(i).IsLeadingEdge();
             
             Grid().EdgeList(i).EdgeType()       = VSP_Surface(Surface).Grid().EdgeList(i).EdgeType();

          }
         
       }
       
    }    
    
    // Calculate surface areas per surface ID region
    
    AreaTotal = 0.;
    
    for ( i = 1 ; i <= Grid().NumberOfLoops() ; i++ ) {
       
       AreaTotal += Grid().LoopList(i).Area();
       
    }
    
    // Calculate bounding boxes for all components in the grid
    
    CreateComponentBBoxData();
    
    //Grid().MinLoopArea() = AreaTotal/50.;
    Grid().MinLoopArea() = AreaTotal;
   
    printf("Total NumberOfNodes:      %d \n",Grid().NumberOfNodes());
    printf("Total NumberOfLoops:      %d \n",Grid().NumberOfLoops());
    printf("Total NumberOfEdges:      %d \n",Grid().NumberOfEdges());
    printf("Total NumberOfKuttaNodes: %d \n",Grid().NumberOfKuttaNodes());
    
    printf("Total surface area: %lf \n",AreaTotal);
    printf("Minimum loop area constraint set to: %lf \n",Grid().MinLoopArea());
 
    printf("Agglomerating mesh... \n");fflush(NULL);

    VSP_AGGLOM Agglomerate;

    printf("Grid:%d --> # loops: %10d ...# Edges: %10d \n",0,Grid_[0]->NumberOfLoops(),Grid_[0]->NumberOfEdges());
    
    // First attempt to simplify the grid

    Grid_[1] = Agglomerate.SimplifyMesh(*(Grid_[0]));

    Grid_[1]->CalculateUpwindEdges();   
    
    Grid_[1]->CreateUpwindEdgeData();
              
    printf("Grid:%d --> # loops: %10d ...# Edges: %10d \n",1,Grid_[1]->NumberOfLoops(),Grid_[1]->NumberOfEdges());
   
    i = 2;
    
    Done = 0;

    while ( !Done && 
            i < MaxNumberOfGridLevels &&
            Grid_[i-1]->NumberOfLoops() > NumberOfSurfacePatches_ ) {

       Grid_[i] = Agglomerate.Agglomerate(*(Grid_[i-1]));
              
       if ( i <= 2 ||      
            (    Grid_[i]->NumberOfLoops() >   50 &&
                 Grid_[i]->NumberOfEdges() >   50 &&
              2.*Grid_[i]->NumberOfLoops() <= Grid_[i-1]->NumberOfLoops() ) ) {
          
          Grid_[i]->CalculateUpwindEdges();   
       
          Grid_[i]->CreateUpwindEdgeData();

          printf("Grid:%d --> # loops: %10d ...# Edges: %10d  \n",i,Grid_[i]->NumberOfLoops(),Grid_[i]->NumberOfEdges());
       
          i++;
          
       }
       
       else {
          
          Done = 1;
          
       }

    }

    NumberOfGridLevels_ = i - 1;

    printf("NumberOfGridLevels_: %d \n",NumberOfGridLevels_);    
    printf("NumberOfSurfacePatches_: %d \n",NumberOfSurfacePatches_);
    
    // Output the coarse grid mesh info
    
    if ( Verbose_ ) OutputCoarseGridInfo();

    // Find vortex loops lying within any control surface regions
    
    FindControlSurfaceVortexLoops();

}

/*##############################################################################
#                                                                              #
#                          VSP_GEOM DetermineModelType                         #
#                                                                              #
##############################################################################*/

void VSP_GEOM::DetermineModelType(void)
{
   
    int i, Done, Node1, Node2, Tri1, Tri2;
    VSPAERO_DOUBLE y, Epsilon;
    
    Epsilon = 0.01; // Yeah, this needs more work...
    
    ModelType_ = PANEL_MODEL;
    
    Done = 0;
    
    i = 1;
    
    while (i <= Grid().NumberOfEdges() && !Done ) {
       
       Node1 = Grid().EdgeList(i).Node1();
       Node2 = Grid().EdgeList(i).Node2();

       Tri1 = Grid().EdgeList(i).Tri1();
       Tri2 = Grid().EdgeList(i).Tri2();
       
       if ( Tri1 == Tri2 ) {
          
          y = 0.5*( Grid().NodeList(Node1).y() +  Grid().NodeList(Node2).y() );
          
          if ( y > Epsilon ) {
             
             ModelType_ = VLM_MODEL;
             
             Done = 1;
             
             
             
          }
          
       }
       
       i++;
       
    }
    
    if ( ModelType_ == VLM_MODEL   ) printf("Model is a VSPGEOM VLM geometry \n");fflush(NULL);
    if ( ModelType_ == PANEL_MODEL ) printf("Model is a VSPGEOM Panel geometry \n");fflush(NULL);

}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM CreateComponentBBoxData                     #
#                                                                              #
##############################################################################*/

void VSP_GEOM::CreateComponentBBoxData(void)
{
   
    int i, c, j, Node;
    
    printf("NumberOfComponents_: %d\n",NumberOfComponents_);

    fflush(NULL);
    
    BBoxForComponent_ = new BBOX[NumberOfComponents_ + 1];
   
    for ( c = 1 ; c <= NumberOfComponents_ ; c++ ) {
       
       BBoxForComponent(c).x_min =  1.e9;
       BBoxForComponent(c).x_max = -1.e9;
       BBoxForComponent(c).y_min =  1.e9;
       BBoxForComponent(c).y_max = -1.e9;
       BBoxForComponent(c).z_min =  1.e9;
       BBoxForComponent(c).z_max = -1.e9;
       
    }
       
    for ( i = 1 ; i <= Grid().NumberOfLoops() ; i++ ) {
       
       c = Grid().LoopList(i).ComponentID();
       
       for ( j = 1 ; j <= Grid().LoopList(i).NumberOfNodes() ; j++ ) {
          
          Node = Grid().LoopList(i).Node(j);
         
          BBoxForComponent(c).x_min = MIN(BBoxForComponent(c).x_min, Grid().NodeList(Node).x());
          BBoxForComponent(c).x_max = MAX(BBoxForComponent(c).x_max, Grid().NodeList(Node).x());
          
          BBoxForComponent(c).y_min = MIN(BBoxForComponent(c).y_min, Grid().NodeList(Node).y()); 
          BBoxForComponent(c).y_max = MAX(BBoxForComponent(c).y_max, Grid().NodeList(Node).y()); 
          
          BBoxForComponent(c).z_min = MIN(BBoxForComponent(c).z_min, Grid().NodeList(Node).z()); 
          BBoxForComponent(c).z_max = MAX(BBoxForComponent(c).z_max, Grid().NodeList(Node).z()); 
          
       }
       
    }

}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM AgglomerateMeshes                           #
#                                                                              #
##############################################################################*/

void VSP_GEOM::AgglomerateMeshes(void)
{
 
    int Surface;
   
    for ( Surface = 1 ; Surface <= NumberOfSurfaces_ ; Surface++ ) {
     
       VSP_Surface(Surface).AgglomerateMesh();
       
    }
    
    printf("Done agglomerating meshes! \n");fflush(NULL);
     
}

/*##############################################################################
#                                                                              #
#                     VSP_GEOM FindControlSurfaceVortexLoops                   #
#                                                                              #
##############################################################################*/

void VSP_GEOM::FindControlSurfaceVortexLoops(void)
{

    int Surface, k, j, NumberOfTrisInControlSurface;
    
    for ( Surface = 1 ; Surface <= NumberOfSurfaces_ ; Surface++ ) {
       
       if ( VSP_Surface(Surface).SurfaceType() == DEGEN_WING_SURFACE ) {
    
          for ( k = 1 ; k <= VSP_Surface(Surface).NumberOfControlSurfaces() ; k++ ) {
             
             NumberOfTrisInControlSurface = 0;
             
             for ( j = 1 ; j <= Grid(1).NumberOfLoops() ; j++ ) {
                
                if ( Grid(1).TriList(j).SurfaceID() == Surface ) {
  
                   if ( VSP_Surface(Surface).ControlSurface(k).TriInside(Grid(1).LoopList(j).UVc()) ) {
                      
                      NumberOfTrisInControlSurface++;
                      
                   }
                   
                }
                
             }

             VSP_Surface(Surface).ControlSurface(k).SizeLoopList(NumberOfTrisInControlSurface);
             
          }
          
       }
       
    }
    
    for ( Surface = 1 ; Surface <= NumberOfSurfaces_ ; Surface++ ) {
       
       if ( VSP_Surface(Surface).SurfaceType() == DEGEN_WING_SURFACE ) {
    
          for ( k = 1 ; k <= VSP_Surface(Surface).NumberOfControlSurfaces() ; k++ ) {
             
             NumberOfTrisInControlSurface = 0;
             
             for ( j = 1 ; j <= Grid(1).NumberOfLoops() ; j++ ) {
                
                if ( Grid(1).TriList(j).SurfaceID() == Surface ) {
                
                   if ( VSP_Surface(Surface).ControlSurface(k).TriInside(Grid(1).LoopList(j).UVc()) ) {
                      
                      NumberOfTrisInControlSurface++;
                      
                      VSP_Surface(Surface).ControlSurface(k).LoopList(NumberOfTrisInControlSurface) = j;
                      
                   }
                   
                }
                
             }
    
          }
          
       }
       
    }
 
}

/*##############################################################################
#                                                                              #
#                           VSP_GEOM OutputCoarseGridInfo                      #
#                                                                              #
##############################################################################*/

void VSP_GEOM::OutputCoarseGridInfo(void)
{

    int i, j, NumberOfLoops, Loop, Edge, Level;

    Level = NumberOfGridLevels_;

    for ( i = 1 ; i <= Grid(Level).NumberOfLoops() ; i++ ) {

       NumberOfLoops = CalculateNumberOfFineLoops(Level, Grid(Level).LoopList(i));
       
       printf("Coarse loop %d contains %d fine loops \n",i,NumberOfLoops);
       
       for ( j = 1 ; j <= Grid(Level).LoopList(i).NumberOfEdges() ; j++ ) {
          
          Edge = Grid(Level).LoopList(i).Edge(j);
          
          Loop = Grid(Level).EdgeList(Edge).Loop1()
               + Grid(Level).EdgeList(Edge).Loop2() - i;
                
          printf("   Boundary Loop: %d: %d ... pairs: %d %d\n",j,Loop,
          Grid(Level).EdgeList(Edge).Loop1(),
          Grid(Level).EdgeList(Edge).Loop2());
          
       }

    }

}

/*##############################################################################
#                                                                              #
#                       VSP_GEOM CalculateNumberOfFineLoops                    #
#                                                                              #
##############################################################################*/

int VSP_GEOM::CalculateNumberOfFineLoops(int Level, VSP_LOOP &Loop)
{

    int i, FineLoops;

    FineLoops = 0;
    
    if ( Level == 2 ) {

       FineLoops = Loop.NumberOfFineGridLoops();

    }
    
    else {
       
       for ( i = 1 ; i <= Loop.NumberOfFineGridLoops() ; i++ ) {
    
          FineLoops += CalculateNumberOfFineLoops(Level-1,Grid(Level-1).LoopList(Loop.FineGridLoop(i)));

       }
       
    }
    
    return FineLoops;

}

