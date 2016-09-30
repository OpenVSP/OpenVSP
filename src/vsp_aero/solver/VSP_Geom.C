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

int VSP_GEOM::ReadFile(char *FileName)
{
 
    char VSP_File_Name[2000];
    FILE *File;
     
    // VSP Degen file

    sprintf(VSP_File_Name,"%s.csv",FileName);

    if ( (File = fopen(VSP_File_Name,"r")) != NULL ) {
        
       fclose(File);
       
       Read_VSP_Degen_File(FileName);
       
       ModelType_ = VLM_MODEL;
       
    }       
    
    // CART3D file
    
    else {
       
       sprintf(VSP_File_Name,"%s.tri",FileName);
       
       if ( (File = fopen(VSP_File_Name,"r")) != NULL ) {

          fclose(File);
       
          Read_CART3D_File(FileName);
          
          ModelType_ = PANEL_MODEL;
          
       }
       
       else {

          printf("Could not load %s VSP Degen Geometry or CART3D Tri file... \n", FileName);fflush(NULL);

          exit(1);
          
       }
              
    }
    
    // Load in FEM analysis data
    
    if ( LoadDeformationFile_ ) LoadFEMDeformationData(FileName);

    // Create meshes for the VSP geometries
    
    MeshGeom();
      
    return ModelType_;
      
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
 
          sprintf(FEM_File_Name,"%s.Surface.%d.dfm",FileName,i);
       
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
    double Diam, x, y, z, nx, ny, nz;
    FILE *Cart3D_File, *VSP_Degen_File;
 
    sprintf(VSP_File_Name,"%s.tri",FileName);
    
    if ( (Cart3D_File = fopen(VSP_File_Name,"r")) == NULL ) {

       printf("Could not load %s CART3D file... \n", VSP_File_Name);fflush(NULL);

       exit(1);

    }    
         
    // Now size the surface list
    
    NumberOfDegenWings_     = 0;
    NumberOfDegenBodies_    = 0;
    NumberOfCart3dSurfaces_ = 1;
    NumberOfSurfaces_       = 1;
    
    VSP_Surface_ = new VSP_SURFACE[NumberOfSurfaces_ + 1];
    
    // Read in the wing data

    sprintf(Name,"CART3D");

    VSP_Surface(1).ReadCart3DDataFromFile(Name,Cart3D_File);
    
    NumberOfSurfacePatches_ = VSP_Surface(1).NumberOfSurfacePatches();

    fclose(Cart3D_File);
    
    // Now see if a degen file exists, open it, and look for rotor info

    sprintf(VSP_Degen_File_Name,"%s_DegenGeom.csv",FileName);

    if ( (VSP_Degen_File = fopen(VSP_Degen_File_Name,"r")) != NULL ) {

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
       
    }    

}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM Read_VSP_Degen_File                         #
#                                                                              #
##############################################################################*/

void VSP_GEOM::Read_VSP_Degen_File(char *FileName)
{

    int i, Wing, Done, NumberOfBodySets, BodySet, Surface;
    int TotalNumberOfWings, TotalNumberOfBodies;
    int *ReadInThisWing, *ReadInThisBody;
    double Diam, x, y, z, nx, ny, nz, Epsilon, MinVal, MaxVal;
    char VSP_File_Name[2000], DumChar[2000], Type[2000], Name[2000];
    VSP_SURFACE SurfaceParser;
    BBOX ComponentBBox;
    FILE *VSP_Degen_File;
    
    MinVal = MaxVal = 0.;
    
    // Open degen file

    sprintf(VSP_File_Name,"%s.csv",FileName);

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
    
    printf("NumberOfComponents: %d \n",NumberOfComponents_);
    
    // Now scan the file and determine how many wings in total
    
    TotalNumberOfWings = 0;
    
    Done = 0;
    
    while ( !Done ) {
     
       if ( fgets(DumChar,1000,VSP_Degen_File) == NULL ) Done = 1;   
       
       if ( strncmp(DumChar,"LIFTING_SURFACE",15) == 0 ) TotalNumberOfWings++;
       
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
             
             if ( DoSymmetryPlaneSolve_ == SYM_X && ComponentBBox.x_min >= 0. ) { NumberOfDegenWings_++ ; ReadInThisWing[i] = 1; };
             
             if ( DoSymmetryPlaneSolve_ == SYM_Y && ComponentBBox.y_min >= 0. ) { NumberOfDegenWings_++ ; ReadInThisWing[i] = 1; };
              
             if ( DoSymmetryPlaneSolve_ == SYM_Z && ComponentBBox.z_min >= 0. ) { NumberOfDegenWings_++ ; ReadInThisWing[i] = 1; };
                           
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
                
                printf("ReadInThisBody[i]: %d \n",ReadInThisBody[i]);
                
             }
          
          }
                                        
       }
       
    }
    
    printf("NumberOfDegenBodies_: %d \n",NumberOfDegenBodies_);
    
    rewind(VSP_Degen_File);    

    // Now scan the file and determine how many actuator disks
    
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
    
    // Read in the wing data
    
    Surface = 0;
    
    for ( Wing = 1 ; Wing <= TotalNumberOfWings ; Wing++ ) {

       Done = 0;
       
       while ( !Done ) {

          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strncmp(DumChar,"LIFTING_SURFACE",15) == 0 ) Done = 1;
          
       }
       
       if ( ReadInThisWing[Wing] ) {
          
          Surface++;
              
          char *Next;
          char delimiters[] = ",\n";

          Next = strtok( DumChar, delimiters );

          sprintf( Type, "%s\0", Next );
          
          Next = strtok( NULL, delimiters );

          sprintf( Name, "%s\0", Next );
          
          printf("Working on reading wing: %d --> %s \n",Wing,Name);
  
          VSP_Surface(Surface).ReadWingDataFromFile(Name,VSP_Degen_File);
          
       }
       
    }
        
    // Read in the body data

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
            
          sscanf(DumChar,"%4s,%s",Type,Name);

          if ( Verbose_ ) printf("Working on reading #2 horizontal slice for body: %d --> %s ... SymFlag: %d \n",BodySet,Name,ReadInThisBody[i]); fflush(NULL);
                 
          VSP_Surface(Surface).ReadBodyDataFromFile(Name,2,VSP_Degen_File);
          
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
   
          sscanf(DumChar,"%4s,%s",Type,Name);
   
          if ( Verbose_ ) printf("Working on reading #1 horizontal slice for body: %d --> %s ... SymFlag: %d \n",BodySet,Name,ReadInThisBody[i]); fflush(NULL);
                 
          VSP_Surface(Surface).ReadBodyDataFromFile(Name,1,VSP_Degen_File);
          
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
       
          sscanf(DumChar,"%4s,%s",Type,Name);
   
          if ( Verbose_ ) printf("Working on reading vertical slice for body: %d --> %s \n",BodySet,Name);
                 
          VSP_Surface(Surface).ReadBodyDataFromFile(Name,3,VSP_Degen_File);    
          
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
                      
          sscanf(DumChar,"%4s,%s",Type,Name);
   
          if ( Verbose_ ) printf("Working on reading vertical slice for body: %d --> %s \n",BodySet,Name);
                 
          VSP_Surface(Surface).ReadBodyDataFromFile(Name,4,VSP_Degen_File);    
          
       }
          
    }
    
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
    double AreaTotal;
    
    // Loop over the surface and create a mesh for each

    for ( Surface = 1 ; Surface <= NumberOfSurfaces_ ; Surface++ ) {
     
       if ( VSP_Surface(Surface).SurfaceType() != CART3D_SURFACE ) {
         
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
          
          if ( VSP_Surface(Surface).SurfaceType() != CART3D_SURFACE ) {
          
             Grid().KuttaNode(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().KuttaNode(i) + NodeOffSet;
             
             Grid().WingSurface(NumberOfKuttaNodes) = Surface;
             
             Grid().WingSurfaceIsPeriodic(NumberOfKuttaNodes) = 0;
   
             Grid().WakeTrailingEdgeX(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WakeTrailingEdgeX(i);
             Grid().WakeTrailingEdgeY(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WakeTrailingEdgeY(i);
             Grid().WakeTrailingEdgeZ(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WakeTrailingEdgeZ(i);
             
          }
          
          else {
             
             Grid().KuttaNode(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().KuttaNode(i) + NodeOffSet;
             
             Grid().WingSurface(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WingSurface(i);
             
             Grid().WingSurfaceIsPeriodic(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WingSurfaceIsPeriodic(i);
   
             Grid().WakeTrailingEdgeX(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WakeTrailingEdgeX(i);
             Grid().WakeTrailingEdgeY(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WakeTrailingEdgeY(i);
             Grid().WakeTrailingEdgeZ(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WakeTrailingEdgeZ(i);
             
          }             
          
       }
       
       NodeOffSet += VSP_Surface(Surface).Grid().NumberOfNodes();
       
    }
    
    printf("NumberOfKuttaNodes: %d \n",NumberOfKuttaNodes);
    
    Grid().CalculateTriNormalsAndCentroids();
    
    Grid().CreateTriEdges();
    
    Grid().CalculateUpwindEdges();   
    
    Grid().CreateUpwindEdgeData();
    
    for ( Surface = 1 ; Surface <= NumberOfSurfaces_ ; Surface++ ) {
     
       if ( VSP_Surface(Surface).SurfaceType() == CART3D_SURFACE ) {
         
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
    
    Grid().MinLoopArea() = AreaTotal/200.;
    
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
       
       if (     Grid_[i]->NumberOfLoops() > 0 &&
                Grid_[i]->NumberOfEdges() > 0 &&
            1.5*Grid_[i]->NumberOfLoops() <= Grid_[i-1]->NumberOfLoops() ) {
          
          Grid_[i]->CalculateUpwindEdges();   
       
          Grid_[i]->CreateUpwindEdgeData();

          printf("Grid:%d --> # loops: %10d ...# Edges: %10d  \n",i,Grid_[i]->NumberOfLoops(),Grid_[i]->NumberOfEdges());
       
          i++;
          
       }
       
       else {
          
          printf("Stopped aggloemeration at.... Grid:%d --> # loops: %10d ...# Edges: %10d  \n",i,Grid_[i]->NumberOfLoops(),Grid_[i]->NumberOfEdges());
          
          Done = 1;
          
       }

    }

    NumberOfGridLevels_ = i;

    printf("NumberOfGridLevels_: %d \n",NumberOfGridLevels_);    
    
    // Find vortex loops lying within any control surface regions
    
    FindControlSurfaceVortexLoops();
    
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
#                     VSP_SOLVER FindControlSurfaceVortexLoops                 #
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

             VSP_Surface(Surface).ControlSurface(k).SizeList(NumberOfTrisInControlSurface);
             
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


