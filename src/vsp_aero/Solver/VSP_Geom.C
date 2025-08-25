//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Geom.H"

#include "START_NAME_SPACE.H"

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
    
    SolveOnMGLevel_ = 1;
    
    LoadDeformationFile_ = 0;
    
    DoGroundEffectsAnalysis_ = 0;
    
    VehicleRotationAngleVector_[0] = 0.;    
    VehicleRotationAngleVector_[1] = 0.;    
    VehicleRotationAngleVector_[2] = 0.;    
    
    NumberOfWakeTrailingNodes_ = 10;
    
    NumberOfWakeResidualEquations_ = 0;
    
    DoSymmetryPlaneSolve_ = 0;
    
    VortexSheet_ = NULL;
    
    SetFarFieldDist_ = 0;
    
    FarFieldDist_ = 0.;
    
    NumberOfControlSurfaces_ = 0;
    
    UserSuppliedGroupsFile_ = 0;
    
    ThereAreRotors_ = 0;
    
    ComponentIDForComponent_ = NULL;
    
    VSPComponentIDForVSPAEROComponent_ = NULL;

    pComponentWettedAreapMesh_ = NULL;
    
    pComponentLengthpMesh_ = NULL;
   
    pComponentFineNessRatiopMesh_ = NULL;  
    
    ComponentWettedArea_    = NULL;
    ComponentLength_        = NULL;
    ComponentFineNessRatio_ = NULL; 
    
    pComponentWettedAreapAlpha_ = NULL;
    pComponentWettedAreapBeta_  = NULL;   
 
    pComponentLengthpAlpha_ = NULL;
    pComponentLengthpBeta_  = NULL;  
    
    pComponentFineNessRatiopAlpha_ = NULL;
    pComponentFineNessRatiopBeta_  = NULL;
        
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
#              VSP_GEOM SetNumberOfComponentGroupsTo                           #
#                                                                              #
##############################################################################*/

void VSP_GEOM::SetNumberOfComponentGroupsTo(int NumberOfComponentGroups)
{

    NumberOfComponentGroups_ = NumberOfComponentGroups;

    ComponentGroupList_ = new COMPONENT_GROUP[NumberOfComponentGroups_ + 1];
          
}
    
/*##############################################################################
#                                                                              #
#                         VSP_GEOM ReadFile                                    #
#                                                                              #
##############################################################################*/

void VSP_GEOM::ReadFile(char *FileName)
{

    char CART3D_File_Name[MAX_CHAR_SIZE];
    char VSPGEOM_File_Name[MAX_CHAR_SIZE];
    char ControlSurfaceTagListFileName[MAX_CHAR_SIZE];    
    FILE *File;
     
    // File name possibilities...

    snprintf(CART3D_File_Name,sizeof(CART3D_File_Name)*sizeof(char),"%s.tri",FileName);
    
    snprintf(VSPGEOM_File_Name,sizeof(VSPGEOM_File_Name)*sizeof(char),"%s.vspgeom",FileName);

    if ( (File = fopen(VSPGEOM_File_Name,"r")) != NULL ) {

       fclose(File);
    
       Read_VSPGEOM_File(FileName);
      
    }
    
    // CART3D file
    
    else if ( (File = fopen(CART3D_File_Name,"r")) != NULL ) {

      fclose(File);
      
      Read_CART3D_File(FileName);

    }
    
    else {

       printf("Could not load %s OpenVSP VSPGEOM file, or CART3D Tri file... n", FileName);fflush(NULL);

       exit(1);
       
    }
    
    // Look for control surfaces
    
    snprintf(ControlSurfaceTagListFileName,sizeof(ControlSurfaceTagListFileName)*sizeof(char),"%s.ControlSurfaces.taglist",FileName);
    
    printf("Looking for %s control surface definition file ... \n",ControlSurfaceTagListFileName);
    
    if ( (File = fopen(ControlSurfaceTagListFileName,"r")) != NULL ) {

      fclose(File);
      
      printf("There is control surface information... \n");
      
      ReadControlSurfaceInformation(FileName);
      
    }
    
    else {
       
       printf("Did not find any control surface definitions... \n");
       
    }    
    
    // Look for a groups files
    
    ReadGroupFile(FileName);

    // Create meshes for the VSP geometries
    
    MeshGeom();

}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM Read_CART3D_File                            #
#                                                                              #
##############################################################################*/

void VSP_GEOM::Read_CART3D_File(char *FileName)
{

    int i, Done, *ComponentList;
    char VSP_File_Name[MAX_CHAR_SIZE], TKEY_File_Name[MAX_CHAR_SIZE], VSP_Degen_File_Name[MAX_CHAR_SIZE], Name[MAX_CHAR_SIZE], DumChar[MAX_CHAR_SIZE];
    double Diam, x, y, z, nx, ny, nz;
    FILE *Cart3D_File, *TKEY_File, *VSP_Degen_File;
 
    snprintf(VSP_File_Name,sizeof(VSP_File_Name)*sizeof(char),"%s.tri",FileName);
    
    if ( (Cart3D_File = fopen(VSP_File_Name,"r")) == NULL ) {

       printf("Could not load %s CART3D file... \n", VSP_File_Name);fflush(NULL);

       exit(1);

    }    

    snprintf(TKEY_File_Name,sizeof(TKEY_File_Name)*sizeof(char),"%s.tkey",FileName);
    
    if ( (TKEY_File = fopen(TKEY_File_Name,"r")) == NULL ) {

       printf("Could not load %s TKEY file... so I won't use it... ;-) \n", TKEY_File_Name);fflush(NULL);

       TKEY_File = NULL;

    }    
 
    // Read in the cart3d geometry

    snprintf(Name,sizeof(Name)*sizeof(char),"CART3D");

    ReadCart3DDataFromFile(Name,Cart3D_File,TKEY_File);

    // Determine number of components
    
    ComponentList = new int[Grid().NumberOfTris() + 1];
     
    for ( i = 1 ; i <= Grid().NumberOfTris() ; i++ ) {
 
       ComponentList[Grid().TriList(i).ComponentID()] = 1;
       
    }
    
    NumberOfComponents_ = 0;
    
    for ( i = 1 ; i <= Grid().NumberOfTris() ; i++ ) {
    
       NumberOfComponents_ += ComponentList[i];
       
    }    
    
    printf("Found %d components for cart3d geometry ... \n",NumberOfComponents_);fflush(NULL);

    GeometryComponentIsFixed_ = new int[NumberOfComponents_ + 1];

    GeometryGroupID_ = new int[NumberOfComponents_ + 1];
              
    delete [] ComponentList;
    
    fclose(Cart3D_File);
    
    if ( TKEY_File != NULL ) fclose(TKEY_File);
    
    // Now see if a degen file exists

    snprintf(VSP_Degen_File_Name,sizeof(VSP_Degen_File_Name)*sizeof(char),"%s_DegenGeom.csv",FileName);

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

    int i, Done, *ComponentList;
    char VSPGEOM_File_Name[MAX_CHAR_SIZE], VSP_Degen_File_Name[MAX_CHAR_SIZE], Name[MAX_CHAR_SIZE], DumChar[MAX_CHAR_SIZE];
    char VKEY_File_Name[MAX_CHAR_SIZE];
    double Diam, x, y, z, nx, ny, nz;
    FILE *VSPGEOM_File, *VKEY_File, *VSP_Degen_File;
 
    snprintf(VSPGEOM_File_Name,sizeof(VSPGEOM_File_Name)*sizeof(char),"%s.vspgeom",FileName);
    
    if ( (VSPGEOM_File = fopen(VSPGEOM_File_Name,"r")) == NULL ) {

       printf("Could not load %s VSPGEOM file... \n", VSPGEOM_File_Name);fflush(NULL);

       exit(1);

    }    

    snprintf(VKEY_File_Name,sizeof(VKEY_File_Name)*sizeof(char),"%s.vkey",FileName);
    
    if ( (VKEY_File = fopen(VKEY_File_Name,"r")) == NULL ) {

       printf("Could not load %s VKEY file... so I won't use it... ;-) \n", VKEY_File_Name);fflush(NULL);

       VKEY_File = NULL;

    }    

    // Read in the vspgeom mesh file

    snprintf(Name,sizeof(Name)*sizeof(char),"VSPGEOM");

    ReadVSPGeomDataFromFile(Name,VSPGEOM_File,VKEY_File);
 
    fclose(VSPGEOM_File);
    
    if ( VKEY_File != NULL ) fclose(VKEY_File);
 
    // Now see if a degen file exists

    snprintf(VSP_Degen_File_Name,sizeof(VSP_Degen_File_Name)*sizeof(char),"%s_DegenGeom.csv",FileName);

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
#                  VSP_GEOM ReadControlSurfaceInformation                      #
#                                                                              #
##############################################################################*/

void VSP_GEOM::ReadControlSurfaceInformation(char *FileName)
{
   
    int i, j, k, LoopC, LoopF, *OnControlSurface, NumberOfControlSurfaceTris;
    char ControlSurfaceTagListFileName[MAX_CHAR_SIZE], TagListFileName[MAX_CHAR_SIZE];
    FILE *TagListFile;
    
    snprintf(ControlSurfaceTagListFileName,sizeof(ControlSurfaceTagListFileName)*sizeof(char),"%s.ControlSurfaces.taglist",FileName);
    
    if ( (TagListFile = fopen(ControlSurfaceTagListFileName,"r")) == NULL ) {

       printf("Could not load %s Taglist file... \n", ControlSurfaceTagListFileName);fflush(NULL);

       exit(1);

    }   
 
    // Read in the number of tag regions... ie, number of control surfaces and their names
        
    fscanf(TagListFile,"%d",&NumberOfControlSurfaces_);
    
    ControlSurface_ = new CONTROL_SURFACE[NumberOfControlSurfaces_ + 1];
    
    for ( i = 1 ; i <= NumberOfControlSurfaces_ ; i++ ) {
       
       fscanf(TagListFile,"%s",TagListFileName);

       ControlSurface_[i].LoadFile(FileName,TagListFileName);
  
    }

    // If the user supplied an ngon mesh, we need to modify this tag list
    
    if ( InputMeshIsMixedPolys_ ) {
    
       OnControlSurface = new int[Grid().NumberOfLoops() + 1];
       
       for ( i = 1 ; i <= NumberOfControlSurfaces_ ; i++ ) {
          
          zero_int_array(OnControlSurface, Grid().NumberOfLoops());
          
          for ( j = 1 ; j <= ControlSurface_[i].NumberOfLoops() ; j++ ) {
          
             LoopC = ControlSurface_[i].LoopList(j);
             
             for ( k = 1 ; k <= GridC_->LoopList(LoopC).NumberOfFineGridLoops() ; k++ ) {
                
                LoopF = GridC_->LoopList(LoopC).FineGridLoop(k);
             
                OnControlSurface[LoopF] = 1;
                
             }
             
          }
          
          NumberOfControlSurfaceTris = 0;
          
          for ( j = 1 ; j <= Grid().NumberOfLoops() ; j++ ) {
          
             NumberOfControlSurfaceTris += OnControlSurface[j];
             
          }
       
          ControlSurface_[i].SizeLoopList(NumberOfControlSurfaceTris);
          
          NumberOfControlSurfaceTris = 0;
              
          for ( j = 1 ; j <= Grid().NumberOfLoops() ; j++ ) {
          
             if ( OnControlSurface[j] ) ControlSurface_[i].LoopList(++NumberOfControlSurfaceTris) = j;
             
          }
          
       }
       
    }
                     
    fclose(TagListFile);

}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM ReadGroupFile                               #
#                                                                              #
##############################################################################*/

void VSP_GEOM::ReadGroupFile(char *FileName)
{

    int i, j, k, c;
    char GroupFileName[MAX_CHAR_SIZE], DumChar[MAX_CHAR_SIZE];
    FILE *GroupFile;

    // Open the group file... if it exists
    
    snprintf(GroupFileName,sizeof(GroupFileName)*sizeof(char),"%s.groups",FileName);
    
    if ( (GroupFile = fopen(GroupFileName, "r")) != NULL ) {
 
        // Load in the data
       
       fscanf(GroupFile,"%d\n",&NumberOfComponentGroups_);
       
       printf("There are %d component groups \n",NumberOfComponentGroups_); fflush(NULL);
       
       SetNumberOfComponentGroupsTo(NumberOfComponentGroups_);
       
       for ( i = 1 ; i <= NumberOfComponentGroups_ ; i++ ) {
          
          fgets(DumChar,80,GroupFile);
          
          ComponentGroupList(i).LoadData(GroupFile);
          
       }
       
       fclose(GroupFile);
       
       UserSuppliedGroupsFile_ = 1;
       
       // Update the component ID numbers... as we re-ordered them
       
       for ( i = 1 ; i <= NumberOfComponentGroups_ ; i++ ) {
       
          for ( j = 1 ; j <= ComponentGroupList(i).NumberOfComponents() ; j++ ) {
             
             ComponentGroupList(i).ComponentList(j) = VSPAEROComponentIDForVSPComponent_[ComponentGroupList(i).ComponentList(j)];
             
          }
       
       }

    }
    
    // No group file... so just set up a single group
    
    else {
    
       printf("Did not find a groups file... so setting up a single group for steady analysis \n");fflush(NULL);

       NumberOfComponentGroups_ = 1;
       
       SetNumberOfComponentGroupsTo(NumberOfComponentGroups_);
       
       snprintf(ComponentGroupList(1).GroupName(),MAX_CHAR_SIZE*sizeof(char),"%s",FileName);
       
       ComponentGroupList(1).AngleMax() = 0.;
       
       ComponentGroupList(1).GeometryIsARotor() = 0;
       ComponentGroupList(1).Omega()            = 0.;
       ComponentGroupList(1).RotorDiameter()    = 0.;
       
       ComponentGroupList(1).OVec(0) = 0.;
       ComponentGroupList(1).OVec(1) = 0.;
       ComponentGroupList(1).OVec(2) = 0.;
       
       ComponentGroupList(1).RVec(0) = 0.;
       ComponentGroupList(1).RVec(1) = 0.;
       ComponentGroupList(1).RVec(2) = 0.;
       
       ComponentGroupList(1).SizeList(NumberOfComponents());
       
       for ( j = 1 ; j <= NumberOfComponents() ; j++ ) {
          
          ComponentGroupList(1).ComponentList(j) = j;
          
       }
       
       UserSuppliedGroupsFile_ = 0;
       
    }
    
    // Create a '0' component group that has everything in it....
    
    snprintf(ComponentGroupList(0).GroupName(),MAX_CHAR_SIZE*sizeof(char),"%s",FileName);
    
    ComponentGroupList(0).AngleMax() = 0.;
                      
    ComponentGroupList(0).GeometryIsARotor() = 0;
    ComponentGroupList(0).Omega()            = 0.;
    ComponentGroupList(0).RotorDiameter()    = 0.;
                      
    ComponentGroupList(0).OVec(0) = 0.;
    ComponentGroupList(0).OVec(1) = 0.;
    ComponentGroupList(0).OVec(2) = 0.;
                     
    ComponentGroupList(0).RVec(0) = 0.;
    ComponentGroupList(0).RVec(1) = 0.;
    ComponentGroupList(0).RVec(2) = 0.;
                     
    ComponentGroupList(0).SizeList(NumberOfComponents());
    
    for ( j = 1 ; j <= NumberOfComponents() ; j++ ) {
       
       ComponentGroupList(0).ComponentList(j) = j;
       
    }
        
    // Check that components listed by user seem valid
    
    printf("NumberOfComponentGroups_: %d \n",NumberOfComponentGroups_); fflush(NULL);
    
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       for ( j = 1 ; j <= ComponentGroupList(c).NumberOfComponents() ; j++ ) {
          
          if ( ComponentGroupList(c).ComponentList(j) > NumberOfComponents() ) {
             
             printf("Component Group: %d lists non-existant Component: %d \n", c, ComponentGroupList(c).ComponentList(j));
             
             fflush(NULL);exit(1);
          
          }    

       }
       
    }
 
    // Determine which component groups are fixed
   
    for ( i = 1 ; i <= NumberOfComponents() ; i++ ) {
       
       GeometryComponentIsFixed(i) = 1;
       
       GeometryGroupID(i) = 0;
       
    }
           
    AllComponentsAreFixed_ = 0;
            
    for ( i = 1 ; i <= NumberOfComponentGroups_ ; i++ ) {
       
       if ( !ComponentGroupList(i).GeometryIsFixed() ) {
       
          for ( j = 1 ; j <= ComponentGroupList(i).NumberOfComponents() ; j++ ) {
          
             GeometryComponentIsFixed(ComponentGroupList(i).ComponentList(j)) = 0;

          }

       }
       
       else {
          
          AllComponentsAreFixed_++;
          
       }
 
       for ( j = 1 ; j <= ComponentGroupList(i).NumberOfComponents() ; j++ ) {

          GeometryGroupID(ComponentGroupList(i).ComponentList(j)) = i;
          
       }
              
    }

    if ( AllComponentsAreFixed_ == NumberOfComponentGroups_ ) {
       
       AllComponentsAreFixed_ = 1;
       
    }
    
    else {
        
       AllComponentsAreFixed_ = 0;
       
    }

    printf("AllComponentsAreFixed_: %d \n",AllComponentsAreFixed_); fflush(NULL);
    
    // If all components are not fixed, check that there is indeed relative motion
    
    if ( AllComponentsAreFixed_ == 0 && NumberOfComponentGroups_ > 1 ) ThereIsRelativeComponentMotion_ = 1;
    
    printf("ThereIsRelativeComponentMotion_: %d \n",ThereIsRelativeComponentMotion_);
    
}

/*##############################################################################
#                                                                              #
#               VSP_GEOM AssociateWakesWithComponentGroups                     #
#                                                                              #
##############################################################################*/

void VSP_GEOM::AssociateWakesWithComponentGroups(void)
{
   
   
//edge --> (component_id,u,v) --> store TE that we sit inside... we can do this in the chord code...
   
    int i, j, c, CompSurfs, *WakeSurfaceUsed;

    //  // Check for lifting surfaces
    //  
    //  for ( i = 1 ; i <= NumberOfComponentGroups_ ; i++ ) {
    //  
    //     for ( j = 1 ; j <= ComponentGroupList(i).NumberOfComponents() ; j++ ) {
    //        
    //        printf("NumberOfVortexSheets_: %d \n",NumberOfVortexSheets_);fflush(NULL);
    //        
    //        for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
    //           
    //           printf("VortexSheet(%d).ComponentID(): %d ... and looking for: %d \n",
    //           k,
    //           VortexSheet(k).ComponentID(),
    //           ComponentGroupList(i).ComponentList(j));fflush(NULL);
    //           
    //           if ( VortexSheet(k).ComponentID() == ComponentGroupList(i).ComponentList(j) ) {
    //              
    //              printf("Found 1! \n");fflush(NULL);exit(1);
    //              
    //           }
    //           
    //        }
    //        
    //     }
    //  
    //  }    
    //  
    //  exit(1);   
             
    // Keep track of which components are wings
    

    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {  
    
       ComponentIsLifting_[i] = 0;
       
    }
    
    for ( i = 1 ; i <= Grid(SolveOnMGLevel_).NumberOfKuttaNodes() ; i++ ) {  
       
       ComponentIsLifting_[Grid(SolveOnMGLevel_).ComponentIDForKuttaNode(i)] = 1;
       
    }
           
    // Determine the number of lifting surfaces per component group
    
    WakeSurfaceUsed = new int[NumberOfVortexSheets() + 1];
                  
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
  
       zero_int_array(WakeSurfaceUsed, NumberOfVortexSheets());
  
       CompSurfs = 0;

       for ( j = 1 ; j <= ComponentGroupList(c).NumberOfComponents() ; j++ ) {

          i = 1;
        
          while ( i <= Grid(SolveOnMGLevel_).NumberOfKuttaNodes() ) {  
             
             if ( ComponentGroupList(c).ComponentList(j) == Grid(SolveOnMGLevel_).ComponentIDForKuttaNode(i) ) {

                WakeSurfaceUsed[Grid(SolveOnMGLevel_).WingSurfaceForKuttaNode(i)] = 1;
          
             }
             
             i++;
             
          }   
 
       }

       CompSurfs = 0;
         
       for ( i = 1 ; i <= NumberOfVortexSheets() ; i++ ) {
          
          CompSurfs += WakeSurfaceUsed[i];
          
       }
            
       printf("Found %d wake surfaces for component group: %d \n",CompSurfs, c); fflush(NULL);

       if ( CompSurfs > 0 ) {
          
          ComponentGroupList(c).GeometryHasWings() = 1;

       }
       
       zero_int_array(WakeSurfaceUsed, NumberOfVortexSheets());

       CompSurfs = 0;

       for ( j = 1 ; j <= ComponentGroupList(c).NumberOfComponents() ; j++ ) {

          i = 1;
          
          while ( i <= Grid(SolveOnMGLevel_).NumberOfKuttaNodes() ) {
             
             if ( ComponentGroupList(c).ComponentList(j) == Grid(SolveOnMGLevel_).ComponentIDForKuttaNode(i) ) {
           
                WakeSurfaceUsed[Grid(SolveOnMGLevel_).WingSurfaceForKuttaNode(i)] = 1;

             }
             
             i++;
             
          }   
           
       }
       
       CompSurfs = 0;
       
       for ( i = 1 ; i <= NumberOfVortexSheets() ; i++ ) {
          
          if ( WakeSurfaceUsed[i] ) {
             
             CompSurfs++;
             
//djk this needs fixing             ComponentGroupList(c).SpanLoadData(CompSurfs).WakeSurface() = i;
             
             printf("Found vortex sheet: %d for component group: %d \n",i,c); fflush(NULL);
             
          }
       
       }     
          
    }
     
    delete [] WakeSurfaceUsed;
             
    for ( c = 1 ; c <= NumberOfComponentGroups_ ; c++ ) {
       
       for ( j = 1 ; j <= ComponentGroupList(c).NumberOfComponents() ; j++ ) {
          
          if ( ComponentGroupList(c).ComponentList(j) > NumberOfComponents() ) {
             
             printf("Component Group: %d lists non-existant Component: %d \n", c, ComponentGroupList(c).ComponentList(j)); fflush(NULL);
             
             fflush(NULL);exit(1);
          
          }    

       }
       
    }
 
}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM MeshGeom                                    #
#                                                                              #
##############################################################################*/

void VSP_GEOM::MeshGeom(void)
{
   
    int i;
    int Done;
    double AreaTotal;
    VSP_GRID *TempGrid[2];
    
    // Mesh is all thick

    for ( i = 1 ; i <= Grid().NumberOfLoops() ; i++ ) {

       if ( Grid().LoopList(i).ComponentID() == 0 ) {
          
          printf("wtf before! \n");fflush(NULL);exit(1);
          
       }
       
    }

    
    if ( NumberOfThinSurfaces_ == 0 ) {
       
       Grid().CalculateTriNormalsAndCentroids();
           
       Grid().CreateTriEdges();
       
       // Check the consistency of the mesh 
       
       if (  Grid().CheckGridConsistency() ) {
       
          WriteOutTriMesh();
   
          printf("Mesh is not logically valid... stopping analysis! \n");
          printf("Mesh with whacked tris marked written out... \n");
          
          fflush(NULL); exit(1);
          
       }       
           
       Grid().CalculateUpwindEdges();  
           
       Grid().CreateUpwindEdgeData();
   
       Grid().MarkBoundaries(DoSymmetryPlaneSolve_);
       
       Grid().DetermineSurfaceMeshSize();
              
    }
    
    // Sanitize meshes with thin surfaces
    
    else {
          
       SanitizeThinMeshes();
     
    }
    
  //  for ( i = 1 ; i <= Grid().NumberOfNodes() ; i++ ) {
  //     
  //     printf("Node %d --> CompID: %d \n",i,Grid().NodeList(i).ComponentID());
  //     
  //  }
    
    // Mark all 'true' surface - surface intersection edges, and nodes
    
    MarkSurfaceSurfaceIntersections();

    // Set up Kutta nodes/edges

    for ( i = 1 ; i <= Grid().NumberOfLoops() ; i++ ) {

       if ( Grid().LoopList(i).ComponentID() == 0 ) {
          
          printf("wtf after! \n");fflush(NULL);exit(1);
          
       }
       
    }
    printf("Number of kutta nodes before trimming: %d \n",NumberOfKuttaNodes_);

    if ( NumberOfKuttaNodes_ > 0 ) {
       
       FindSharpEdges(NumberOfKuttaNodes_,KuttaNodeList_);
    
       delete [] KuttaNodeList_;
       
    }
    
    else {
       
      FindSharpEdges(0,NULL);
      
    }       
    
    for ( i = 1 ; i <= Grid().NumberOfLoops() ; i++ ) {

       if ( Grid().LoopList(i).ComponentID() == 0 ) {
          
          printf("wtf after sharp! \n");fflush(NULL);exit(1);
          
       }
       
    }    

    // Create wake grids
    
    CreateWakeGrids();
 
    // Merge in the wake grids

    printf("Merging fine wake grids... \n");fflush(NULL);
        
    TempGrid[0] = MergeGrids(Grid_[0],GridWF_,0);
         
    if ( InputMeshIsMixedPolys_ ) {
         
       printf("Merging coarse wake grids... \n");fflush(NULL);
       
       TempGrid[1] = MergeGrids(GridC_,GridWC_,Grid_[0]->NumberOfLoops());
       
       delete Grid_[0];
       
       delete GridC_;
       
       GridC_ = TempGrid[1];
       
    }

    Grid_[0] = TempGrid[0];
 
    // State model type

    if ( ModelType_ == VLM_MODEL   ) printf("Model is a VSPGEOM VLM geometry \n");fflush(NULL);
    if ( ModelType_ == PANEL_MODEL ) printf("Model is a VSPGEOM Panel geometry \n");fflush(NULL);
        
    Grid().ModelType() = ModelType_;        
    
    // Calculate surface areas per surface ID region
    
    AreaTotal = 0.;
    
    for ( i = 1 ; i <= Grid().NumberOfSurfaceLoops() ; i++ ) {
       
       AreaTotal += Grid().LoopList(i).Area();
       
    }
    
    // Calculate bounding boxes for all components in the grid
    
    CreateComponentBBoxData();
    
    Grid().MinLoopArea() = AreaTotal;
   
    printf("Total NumberOfNodes:      %d \n",Grid().NumberOfNodes());
    printf("Total NumberOfLoops:      %d \n",Grid().NumberOfLoops());
    printf("Total NumberOfEdges:      %d \n",Grid().NumberOfEdges());
    printf("Total NumberOfKuttaNodes: %d \n",Grid().NumberOfKuttaNodes());
    printf("NumberOfSurfaces_:        %d \n",NumberOfSurfaces_);
    
    printf("Total surface area: %lf \n",AreaTotal);
    printf("Minimum loop area constraint set to: %lf \n",Grid().MinLoopArea());
 
    printf("Agglomerating mesh... \n");fflush(NULL);

    VSP_AGGLOM Agglomerate;

    printf("Grid:%d --> # loops: %10d ...# Edges: %10d ...# Nodes: %10d  \n",0,Grid_[0]->NumberOfLoops(),Grid_[0]->NumberOfEdges(),Grid_[0]->NumberOfNodes());
    
    // Simplify the grid... ie merge tris into quads and get rid of sliver tris on panel models

    if ( InputMeshIsMixedPolys_ ) {
       
       Grid_[1] = Agglomerate.SimplifyMesh(*(Grid_[0]), *(GridC_));
       
       
    }
    
    else {
     
       Grid_[1] = Agglomerate.SimplifyMesh(*(Grid_[0]));
      
    }

    Grid_[1]->CalculateUpwindEdges();   
    
    Grid_[1]->CreateUpwindEdgeData();
              
    printf("Grid:%d --> # loops: %10d ...# Edges: %10d ...# Nodes: %10d  \n",1,Grid_[1]->NumberOfLoops(),Grid_[1]->NumberOfEdges(),Grid_[1]->NumberOfNodes());
   
    // Now recusively agglomerate the meshes
    
    i = 2;
    
    Done = 0;

    while ( !Done && 
            i < MaxNumberOfGridLevels_ &&
            Grid_[i-1]->NumberOfLoops() > NumberOfSurfaces_ ) {

       Grid_[i] = Agglomerate.Agglomerate(*(Grid_[i-1]));
             
       if ( i <= 2 ||     
            (    Grid_[i]->NumberOfLoops() >  100 &&
                 Grid_[i]->NumberOfEdges() >  100 &&
               2*Grid_[i]->NumberOfLoops() <= Grid_[i-1]->NumberOfLoops() ) ) {

          Grid_[i]->CalculateUpwindEdges();   
       
          Grid_[i]->CreateUpwindEdgeData();

          printf("Grid:%d --> # loops: %10d ...# Edges: %10d ...# Nodes: %10d \n",i,
           Grid_[i]->NumberOfLoops(),
           Grid_[i]->NumberOfEdges(),
           Grid_[i]->NumberOfNodes());
       
          i++;
          
       }
       
       else {
          
          Done = 1;
          
       }
 
    }

    NumberOfGridLevels_ = i - 1;

    printf("NumberOfGridLevels_: %d \n",NumberOfGridLevels_);    

    printf("NumberOfSurfaces_: %d \n",NumberOfSurfaces_);

    // Update the meshes
    
    UpdateMeshes(0);
    
    // Ouput the coarse grid mesh info
    
    if ( Verbose_ ) OutputCoarseGridInfo();

    // Set up wake data structures
    
    CreateVortexSheets();
         
    // Find and store kutta edges
    
    StoreWakeKuttaEdges();    
    
    // Associate wakes with component groups
    
    AssociateWakesWithComponentGroups();
    
    // Clear up some memory
    
    delete [] NodeToTriList_;
    delete [] NumberOfTrisForNode_;
}

/*##############################################################################
#                                                                              #
#                          VSP_GEOM WriteOutTriMesh                           #
#                                                                              #
##############################################################################*/

void VSP_GEOM::WriteOutTriMesh(void)
{
 
    int i;
    char FileName[MAX_CHAR_SIZE];
    FILE *TriMesh;
    
    snprintf(FileName,sizeof(FileName)*sizeof(char),"whacked.tri");
    
    if ( (TriMesh = fopen(FileName,"w")) == NULL ) {

       printf("Could not open formatted output file for whacked tri cart3d file! \n");
       fflush(NULL);exit(1);
                 
    }
        
    fprintf(TriMesh,"%d %d \n",Grid_[0]->NumberOfNodes(),Grid_[0]->NumberOfTris());
    
    for ( i = 1 ; i <= Grid_[0]->NumberOfNodes() ; i++ ) {
       
       fprintf(TriMesh,"%f %f %f \n",
               Grid_[0]->NodeList(i).x(),
               Grid_[0]->NodeList(i).y(),
               Grid_[0]->NodeList(i).z());
       
    }
    
    for ( i = 1 ; i <= Grid_[0]->NumberOfTris() ; i++ ) {
       
       fprintf(TriMesh,"%d %d %d \n",
               Grid_[0]->TriList(i).Node1(),
               Grid_[0]->TriList(i).Node2(),
               Grid_[0]->TriList(i).Node3());
       
    }
  
  
    for ( i = 1 ; i <= Grid_[0]->NumberOfTris() ; i++ ) {
       
       fprintf(TriMesh,"%d \n",
               Grid_[0]->TriList(i).SurfaceID());
              
    }  
    
    fclose(TriMesh);
    
}
  
/*##############################################################################
#                                                                              #
#                          VSP_GEOM SanitizeThinMeshes                         #
#                                                                              #
##############################################################################*/

void VSP_GEOM::SanitizeThinMeshes(void)
{
 
    int i, j, k, n, p, t, DumInt, Node1, Node2, Node3, SurfaceID, Done;
    int ModelType, **TriPerm, NumberOfEdges;
    int NumberOfThinTris, NumberOfThickTris, NumberOfMeshes, TotalNumberOfThinTris;    
    int NumKuttaNodeLists, NumKuttaNodes, NumNodesInList, *KuttaNodeList, *TempList;
    int CurrentNumberOfEdges, CurrentNumberOfTris, ThinTris, ThickTris;
    VSP_GRID *GridList;
     
    // If any of the surfaces are thin ... we need to tweak the mesh data structure so we
    // do not have more than 2 tris attached to any one edge. We do this by creating 
    // seperate edge data structures for the thin surfaces

    printf("There are thin %d components \n", NumberOfThinComponents_);fflush(NULL);

    GridList = new VSP_GRID[NumberOfThinComponents_ + 2];
    
    TriPerm = new int*[NumberOfThinComponents_ + 2];
    
    // Sanity check on number of thick and thin tris
    
    ThinTris = 0;
       
    for ( n = 1 ; n <= Grid().NumberOfTris() ; n++ ) {
    
       if ( Grid().TriList(n).SurfaceType() != THICK_SURFACE ) ThinTris++;
       
    }    
       
    ThickTris = 0;
       
    for ( n = 1 ; n <= Grid().NumberOfTris() ; n++ ) {
    
       if ( Grid().TriList(n).SurfaceType() == THICK_SURFACE ) ThickTris++;
       
    }               
    
    printf("Number of thin  tris:        %d \n",ThinTris);
    printf("Number of thick tris:        %d \n",ThickTris);
    printf("Number of thin + thick tris: %d \n",ThinTris+ThickTris);
    printf("Number actual total tris:    %d \n",Grid().NumberOfTris());
    fflush(NULL);
    
    int *UsedTri;
    
    UsedTri = new int[Grid().NumberOfTris() + 1];
    
    zero_int_array(UsedTri,Grid().NumberOfTris());
    
    for ( k = 1 ; k <= NumberOfComponents_ ; k++ ) {
       
       NumberOfThinTris = 0;
       
       if ( !(ComponentIsThick(k)) ) {
       
          for ( n = 1 ; n <= Grid().NumberOfTris() ; n++ ) {
       
             if ( Grid().TriList(n).ComponentID() == ComponentIDForComponent(k) ) UsedTri[n] = 1;
             
          }
          
       }
       
    }
    
    for ( n = 1 ; n <= Grid().NumberOfTris() ; n++ ) {
       
       if ( Grid().TriList(n).SurfaceType() != THICK_SURFACE && UsedTri[n] == 0 ) {
          
          printf("Tri: %d is thin and is not being used... and has componentID: %d \n",n,Grid().TriList(n).ComponentID());fflush(NULL);
          
       }
       
    }
    
    delete UsedTri;
             
    // Calculate total number of nodes and tris, including the wake
        
    if ( NumberOfThinComponents_ > 0 ) {

       printf("There are thin components... renumbering for edge data structures \n");fflush(NULL);
       
       p = 0;
              
       // Create a single mesh with all the thick surfaces
              
       NumberOfThickTris = 0;
          
       for ( n = 1 ; n <= Grid().NumberOfTris() ; n++ ) {
       
          if ( Grid().TriList(n).SurfaceType() == THICK_SURFACE ) NumberOfThickTris++;
          
       }
       
       printf("NumberOfThickTris: %d \n",NumberOfThickTris);
       
       if ( NumberOfThickTris > 0 ) {

          printf("There are thick components as well... it's a mixed mesh \n");fflush(NULL);
       
          p++;

          printf("Thick tris are being stored in temp mesh: %d \n",p);fflush(NULL);
          
          GridList[p].SizeNodeList(Grid().NumberOfNodes());
          
          GridList[p].SizeTriList(NumberOfThickTris);
              
          TriPerm[p] = new int[NumberOfThickTris + 1];
          
          zero_int_array(TriPerm[p], NumberOfThickTris);

          // Copy over node data
          
          for ( n = 1 ; n <= Grid().NumberOfNodes() ; n++ ) {
             
             GridList[p].NodeList(n).x() = Grid().NodeList(n).x();
             GridList[p].NodeList(n).y() = Grid().NodeList(n).y();
             GridList[p].NodeList(n).z() = Grid().NodeList(n).z();
          
             GridList[p].NodeList(n).IsTrailingEdgeNode()   = Grid().NodeList(n).IsTrailingEdgeNode();
             
             GridList[p].NodeList(n).IsBoundaryEdgeNode()   = Grid().NodeList(n).IsBoundaryEdgeNode();
             
             GridList[p].NodeList(n).IsBoundaryCornerNode() = Grid().NodeList(n).IsBoundaryCornerNode();

             GridList[p].NodeList(n).SurfaceID()            = Grid().NodeList(n).SurfaceID();

             GridList[p].NodeList(n).ComponentID()          = Grid().NodeList(n).ComponentID();
             
             GridList[p].NodeList(n).OpenVSP_ComponentID()  = Grid().NodeList(n).OpenVSP_ComponentID();
             

          }  
          
          // Copy over tri data

          t = 0; 
          
          for ( n = 1 ; n <= Grid().NumberOfTris() ; n++ ) {
             
             if ( Grid().TriList(n).SurfaceType() == THICK_SURFACE ) {
                
                t++;
                
                TriPerm[p][t] = n;
             
                GridList[p].TriList(t).Node1() = Grid().TriList(n).Node1();
                GridList[p].TriList(t).Node2() = Grid().TriList(n).Node2();
                GridList[p].TriList(t).Node3() = Grid().TriList(n).Node3();
                    
                GridList[p].TriList(t).U_Node(1) = Grid().TriList(n).U_Node(1);
                GridList[p].TriList(t).U_Node(2) = Grid().TriList(n).U_Node(2);
                GridList[p].TriList(t).U_Node(3) = Grid().TriList(n).U_Node(3);
                                    
                GridList[p].TriList(t).V_Node(1) = Grid().TriList(n).V_Node(1);
                GridList[p].TriList(t).V_Node(2) = Grid().TriList(n).V_Node(2);
                GridList[p].TriList(t).V_Node(3) = Grid().TriList(n).V_Node(3);

                GridList[p].TriList(t).Uc() = Grid().TriList(n).Uc();
                GridList[p].TriList(t).Vc() = Grid().TriList(n).Vc();
                                          
                GridList[p].TriList(t).SurfaceID()           = Grid().TriList(n).SurfaceID();
                
                GridList[p].TriList(t).ComponentID()         = Grid().TriList(n).ComponentID();
         
                GridList[p].TriList(t).OpenVSP_ComponentID() = Grid().TriList(n).OpenVSP_ComponentID();
                                                  
                GridList[p].TriList(t).SurfaceType()         = Grid().TriList(n).SurfaceType();
                                    
                GridList[p].TriList(t).VortexSheet()         = Grid().TriList(n).VortexSheet();
                                                        
                GridList[p].TriList(t).SpanStation()         = Grid().TriList(n).SpanStation() ;
                                    
                GridList[p].TriList(t).IsTrailingEdgeTri()   = Grid().TriList(n).IsTrailingEdgeTri();
                                    
             }
     
          }            
          
       }

       for ( k = 1 ; k <= NumberOfComponents_ ; k++ ) {

          printf("ComponentIDForComponent(%d): %d \n",k,ComponentIDForComponent(k));
          
       }
              
       for ( k = 1 ; k <= NumberOfComponents_ ; k++ ) {

          printf("ComponentIsThick(%d): %d \n",k,ComponentIsThick(k));
          
       }
       
       
 //      fflush(NULL);exit(1);
 //for ( n = 1 ; n <= Grid().NumberOfTris() ; n++ ) {
 //
 //   printf("Grid().TriList(%d).ComponentID(): %d ... and thick/thin: %d \n",
 //   n,
 //   Grid().TriList(n).ComponentID(),
 //   ComponentIsThick(Grid().TriList(n).ComponentID()));
 //   
 //   
 //}
                    
       // Create a separate grid for each thin component
       
       TotalNumberOfThinTris = 0;
        
       for ( k = 1 ; k <= NumberOfComponents_ ; k++ ) {
          
          NumberOfThinTris = 0;
          
          if ( !(ComponentIsThick(k)) ) {
          
             for ( n = 1 ; n <= Grid().NumberOfTris() ; n++ ) {
          
                if ( Grid().TriList(n).ComponentID() == ComponentIDForComponent(k) ) NumberOfThinTris++;
                
             }
             
             TotalNumberOfThinTris += NumberOfThinTris;
                                     
             p++;

             printf("Surface %d has %d tris and is being stored in temp mesh: %d \n",k,NumberOfThinTris,p);fflush(NULL);
             
             GridList[p].SizeNodeList(Grid().NumberOfNodes());
             
             GridList[p].SizeTriList(NumberOfThinTris);
             
             TriPerm[p] = new int[NumberOfThinTris + 1];
             
             zero_int_array(TriPerm[p], NumberOfThinTris);
             
             // Copy over node data
             
             for ( n = 1 ; n <= Grid().NumberOfNodes() ; n++ ) {
                
                GridList[p].NodeList(n).x() = Grid().NodeList(n).x();
                GridList[p].NodeList(n).y() = Grid().NodeList(n).y();
                GridList[p].NodeList(n).z() = Grid().NodeList(n).z();
             
                GridList[p].NodeList(n).IsTrailingEdgeNode()   = Grid().NodeList(n).IsTrailingEdgeNode();
                
                GridList[p].NodeList(n).IsBoundaryEdgeNode()   = Grid().NodeList(n).IsBoundaryEdgeNode();
                
                GridList[p].NodeList(n).IsBoundaryCornerNode() = Grid().NodeList(n).IsBoundaryCornerNode();

                GridList[p].NodeList(n).IsSymmetryPlaneNode()  = Grid().NodeList(n).IsSymmetryPlaneNode();
                
                GridList[p].NodeList(n).SurfaceID()            = Grid().NodeList(n).SurfaceID();

                GridList[p].NodeList(n).ComponentID()          = Grid().NodeList(n).ComponentID();

                GridList[p].NodeList(n).OpenVSP_ComponentID()  = Grid().NodeList(n).OpenVSP_ComponentID();
     
             }  
             
             // Copy over tri data
             
             t = 0;
             
             for ( n = 1 ; n <= Grid().NumberOfTris() ; n++ ) {
                
                if ( Grid().TriList(n).ComponentID() == ComponentIDForComponent(k) ){
                   
                   t++;
                   
                   TriPerm[p][t] = n;
            
                   GridList[p].TriList(t).Node1() = Grid().TriList(n).Node1();
                   GridList[p].TriList(t).Node2() = Grid().TriList(n).Node2();
                   GridList[p].TriList(t).Node3() = Grid().TriList(n).Node3();
            
                   GridList[p].TriList(t).U_Node(1) = Grid().TriList(n).U_Node(1);
                   GridList[p].TriList(t).U_Node(2) = Grid().TriList(n).U_Node(2);
                   GridList[p].TriList(t).U_Node(3) = Grid().TriList(n).U_Node(3);
                                       
                   GridList[p].TriList(t).V_Node(1) = Grid().TriList(n).V_Node(1);
                   GridList[p].TriList(t).V_Node(2) = Grid().TriList(n).V_Node(2);
                   GridList[p].TriList(t).V_Node(3) = Grid().TriList(n).V_Node(3);
              
                   GridList[p].TriList(t).Uc() = Grid().TriList(n).Uc();
                   GridList[p].TriList(t).Vc() = Grid().TriList(n).Vc();
                                                          
                   GridList[p].TriList(t).SurfaceID()           = Grid().TriList(n).SurfaceID();
                                                               
                   GridList[p].TriList(t).ComponentID()         = Grid().TriList(n).ComponentID();
                                              
                   GridList[p].TriList(t).OpenVSP_ComponentID() = Grid().TriList(n).OpenVSP_ComponentID();
                                       
                   GridList[p].TriList(t).SurfaceType()         = Grid().TriList(n).SurfaceType();
                                                                
                   GridList[p].TriList(t).VortexSheet()         = Grid().TriList(n).VortexSheet();
                                                                
                   GridList[p].TriList(t).SpanStation()         = Grid().TriList(n).SpanStation() ;
                                                                
                   GridList[p].TriList(t).IsTrailingEdgeTri()   = Grid().TriList(n).IsTrailingEdgeTri();
                                    
                }
             
             }                 
                       
          }
          
       }
          
       NumberOfMeshes = p;
       
       if ( NumberOfThickTris ) printf("Created one thick surface mesh structure... \n");
       
       printf("Created %d thin surface mesh structures \n", NumberOfThinComponents_);
       
       printf("Created %d total mesh structures \n", NumberOfMeshes);
       
       // Now create edge data for each mesh
       
       NumberOfEdges = 0;
       
       for ( p = 1 ; p <= NumberOfMeshes ; p++ ) {
          
          GridList[p].CalculateTriNormalsAndCentroids();
          
          GridList[p].CreateTriEdges();
    
          GridList[p].CalculateUpwindEdges();  
          
          GridList[p].CreateUpwindEdgeData();
          
          NumberOfEdges += GridList[p].NumberOfEdges();
          
       }
       
       // Now merge the meshes
       
       Grid().SizeEdgeList(NumberOfEdges);
       
       // Merge the meshes
       
       k = 0;

       CurrentNumberOfEdges = CurrentNumberOfTris = 0;

       for ( p = 1 ; p <= NumberOfMeshes ; p++ ) {
   
          // Pack the edges
                    
          for ( i = 1 ; i <= GridList[p].NumberOfEdges() ; i++ ) {
     
             k++;
             
             // Copy over data
             
             Grid().EdgeList(k) = GridList[p].EdgeList(i);
             
             Grid().EdgeList(k).Loop1() = TriPerm[p][GridList[p].EdgeList(i).Loop1()];
             Grid().EdgeList(k).Loop2() = TriPerm[p][GridList[p].EdgeList(i).Loop2()]; 

             Grid().EdgeList(k).LoopL() = TriPerm[p][GridList[p].EdgeList(i).LoopL()]; 
             Grid().EdgeList(k).LoopR() = TriPerm[p][GridList[p].EdgeList(i).LoopR()]; 

          }

          // Pack the tris
                    
          for ( i = 1 ; i <= GridList[p].NumberOfTris() ; i++ ) {
     
             n = TriPerm[p][i];
             
             Grid().LoopList(n) = GridList[p].LoopList(i);

             Grid().LoopList(n).Edge1() += CurrentNumberOfEdges;
             Grid().LoopList(n).Edge2() += CurrentNumberOfEdges;
             Grid().LoopList(n).Edge3() += CurrentNumberOfEdges;

          }

          CurrentNumberOfEdges += GridList[p].NumberOfEdges();
                 
          CurrentNumberOfTris += GridList[p].NumberOfTris();
                 
       }   
       
       if ( NumberOfThickTris + TotalNumberOfThinTris != Grid().NumberOfTris() ) {
          
          printf("Error... after mesh reordering the number of tris is not consisent! \n");

          printf("Final number of merged tris: %d... and started with %d tris \n",
           CurrentNumberOfTris,
           Grid().NumberOfTris());fflush(NULL);
          
          printf("TotalNumberOfThinTris: %d \n",TotalNumberOfThinTris);
          
          printf("NumberOfThickTris+TotalNumberOfThinTris: %d \n",
          NumberOfThickTris+TotalNumberOfThinTris);fflush(NULL);     
          
          fflush(NULL);exit(1);
          
       }    

       // Mark the boundaries on the mesh
       
       Grid().MarkBoundaries(DoSymmetryPlaneSolve_);
       
       Grid().DetermineSurfaceMeshSize();

    }

}

/*##############################################################################
#                                                                              #
#                   VSP_GEOM MarkSurfaceSurfaceIntersections                   #
#                                                                              #
##############################################################################*/

void VSP_GEOM::MarkSurfaceSurfaceIntersections(void)
{
   
    int i, j, Loop1, Loop2;
   
    // All thick... easy case
    
    if ( NumberOfThinSurfaces_ == 0 ) {
       
       for ( i = 1 ; i <= Grid().NumberOfSurfaceEdges() ; i++ ) {
          
          Loop1 = Grid().EdgeList(i).Loop1();
          Loop2 = Grid().EdgeList(i).Loop2();

          if ( Loop1 != Loop2 && Grid().EdgeList(i).IsBoundaryEdge() ) {
             
             Grid().EdgeList(i).IsIntersectionEdge() = 1;
             
             Grid().NodeList(Grid().EdgeList(i).Node1()).IsIntersectionNode() = 1;

             Grid().NodeList(Grid().EdgeList(i).Node2()).IsIntersectionNode() = 1;
          
          }
          
       }
       
    }
    
    else {
       
       printf("Searching for thin/thick or thin/thin intersections... \n");fflush(NULL);
       
       // Mark those nodes that are on more than one surface
       
       int Node, *NodeIsOnIntersection;
       
       NodeIsOnIntersection = new int[Grid().NumberOfNodes() + 1];
       
       zero_int_array(NodeIsOnIntersection, Grid().NumberOfNodes());
       
       for ( j = 1 ; j <= Grid().NumberOfSurfaceLoops() ; j++ ) {
          
          for ( i = 1 ; i <= Grid().LoopList(j).NumberOfNodes() ; i++ ) {
             
             Node =  Grid().LoopList(j).Node(i);
             
             if ( NodeIsOnIntersection[Node] == 0 ) {
                
                NodeIsOnIntersection[Node] = Grid().LoopList(j).ComponentID();
                
             }
             
             else if ( NodeIsOnIntersection[Node] > 0 ) {
                
                if ( NodeIsOnIntersection[Node] != Grid().LoopList(j).ComponentID() ) {
                   
                  // printf("Old ComponentID: %d ... and Loop ComponentID: %d \n",
                  // NodeIsOnIntersection[Node], Grid().LoopList(j).ComponentID());fflush(NULL);
                   
                   NodeIsOnIntersection[Node] = -1;
                   
                }
                
             }
             
          }
          
       }
       
       for ( i = 1 ; i <= Grid().NumberOfSurfaceEdges() ; i++ ) {
          
          if ( NodeIsOnIntersection[Grid().EdgeList(i).Node1()] == -1 &&
               NodeIsOnIntersection[Grid().EdgeList(i).Node2()] == -1 ) {
                  
             Grid().EdgeList(i).IsIntersectionEdge() = 1;
                  
             Grid().NodeList(Grid().EdgeList(i).Node1()).IsIntersectionNode() = 1;

             Grid().NodeList(Grid().EdgeList(i).Node2()).IsIntersectionNode() = 1;
             
          }       
          
       }
       
       delete [] NodeIsOnIntersection;       
  
    }
     
}

/*##############################################################################
#                                                                              #
#                          VSP_GEOM DetermineModelType                         #
#                                                                              #
##############################################################################*/

void VSP_GEOM::DetermineModelType(void)
{
   
    int i, Done, Node1, Node2, Tri1, Tri2;
    double y, z, Epsilon;
    
    Epsilon = 0.0001; // Yeah, this needs more work...
    
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

     //     z = 0.5*( Grid().NodeList(Node1).z() +  Grid().NodeList(Node2).z() );
     //     
     //     if ( y < Epsilon && z > Epsilon ) {
     //        
     //        ModelType_ = VLM_MODEL;
     //        
     //        Done = 1;
     //        
     //     }
                    
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
    
    printf("NumberOfComponents_: %d\n",NumberOfComponents_);fflush(NULL);

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

/*##############################################################################
#                                                                              #
#                        VSP_GEOM DeflectControlSurfaces                       #
#                                                                              #
##############################################################################*/

void VSP_GEOM::DeflectControlSurfaces(void)
{

    int i, j, k, p, Loop, Edge, Node, *DoThisNode, *LoopIsOnControlSurface;
    double xyz[3];
    
    DoThisNode = new int[Grid(0).NumberOfNodes() + 1];
    
    LoopIsOnControlSurface = new int[Grid(0).NumberOfLoops() + 1];
     
    for ( k = 1 ; k <= NumberOfControlSurfaces() ; k++ ) {
    
       zero_int_array(LoopIsOnControlSurface, Grid(0).NumberOfLoops());

       zero_int_array(DoThisNode, Grid(0).NumberOfNodes());
    
       // Mark tris on this control surface
       
       for ( p = 1 ; p <= ControlSurface(k).NumberOfLoops() ; p++ ) {
       
          LoopIsOnControlSurface[ControlSurface(k).LoopList(p)] = 1;
          
       }

       // Mark nodes on control surface

       for ( p = 1 ; p <= ControlSurface(k).NumberOfLoops() ; p++ ) {
       
          Loop = ControlSurface(k).LoopList(p);
          
          for ( j = 1 ; j <= Grid(0).LoopList(Loop).NumberOfNodes() ; j++ ) {
             
             DoThisNode[Grid(0).LoopList(Loop).Node(j)] = 1;
             
          }
          
       }
                
       // Mark nodes completely within control surface       

       for ( p = 1 ; p <= ControlSurface(k).NumberOfLoops() ; p++ ) {
       
          Loop = ControlSurface(k).LoopList(p);
          
          for ( j = 1 ; j <= Grid(0).LoopList(Loop).NumberOfEdges() ; j++ ) {
             
             Edge = Grid(0).LoopList(Loop).Edge(j);
             
             if ( LoopIsOnControlSurface[Grid(0).EdgeList(Edge).Loop1()] +
                  LoopIsOnControlSurface[Grid(0).EdgeList(Edge).Loop2()] == 1 ) {
                     
                DoThisNode[Grid(0).EdgeList(Edge).Node1()] = 0;
                DoThisNode[Grid(0).EdgeList(Edge).Node2()] = 0;
                
             }
             
          }
          
       }
                     
       for ( p = 1 ; p <= ControlSurface(k).NumberOfLoops() ; p++ ) {
       
          Loop = ControlSurface(k).LoopList(p);
          
          for ( j = 1 ; j <= Grid(0).LoopList(Loop).NumberOfNodes() ; j++ ) {
             
             Node = Grid(0).LoopList(Loop).Node(j);
             
             if ( DoThisNode[Node] ) {
                
                DoThisNode[Node] = 0;
               
                xyz[0] = Grid(0).NodeList(Node).x();
                xyz[1] = Grid(0).NodeList(Node).y();
                xyz[2] = Grid(0).NodeList(Node).z();
                
                ControlSurface(k).RotateXYZPoint(xyz);
             
                Grid(0).NodeList(Node).x() = xyz[0];
                Grid(0).NodeList(Node).y() = xyz[1];
                Grid(0).NodeList(Node).z() = xyz[2];
                
             }
             
          }
            
       }
       
    }

    UpdateMeshes();
    
    delete [] DoThisNode;

    delete [] LoopIsOnControlSurface;

    for ( k = 1 ; k <= NumberOfVortexSheets() ; k++ ) {

       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {
 
          for ( j = 1 ; j <= VortexSheet(k).TrailingVortex(i).NumberOfNodes() ; j++ ) {

             // Wake node on MGLevel_ mesh
             
             Node = VortexSheet(k).TrailingVortex(i).GlobalNode(j);
        
             // Update the wake data
                 
             VortexSheet(k).TrailingVortex(i).WakeNodeX(j) = Grid(0).NodeList(Node).x();
             VortexSheet(k).TrailingVortex(i).WakeNodeY(j) = Grid(0).NodeList(Node).y();
             VortexSheet(k).TrailingVortex(i).WakeNodeZ(j) = Grid(0).NodeList(Node).z();

          }

       }
       
    }
      
}

/*##############################################################################
#                                                                              #
#                        VSP_GEOM UnDeflectControlSurfaces                     #
#                                                                              #
##############################################################################*/

void VSP_GEOM::UnDeflectControlSurfaces(void)
{

    int i, j, k, p, Loop, Edge, Node, *DoThisNode, *LoopIsOnControlSurface;
    double xyz[3];
    
    DoThisNode = new int[Grid(0).NumberOfNodes() + 1];
    
    LoopIsOnControlSurface = new int[Grid(0).NumberOfLoops() + 1];
     
    for ( k = 1 ; k <= NumberOfControlSurfaces() ; k++ ) {
    
       zero_int_array(LoopIsOnControlSurface, Grid(0).NumberOfLoops());

       zero_int_array(DoThisNode, Grid(0).NumberOfNodes());
    
       // Mark tris on this control surface
       
       for ( p = 1 ; p <= ControlSurface(k).NumberOfLoops() ; p++ ) {
       
          LoopIsOnControlSurface[ControlSurface(k).LoopList(p)] = 1;
          
       }

       // Mark nodes on control surface

       for ( p = 1 ; p <= ControlSurface(k).NumberOfLoops() ; p++ ) {
       
          Loop = ControlSurface(k).LoopList(p);
          
          for ( j = 1 ; j <= Grid(0).LoopList(Loop).NumberOfNodes() ; j++ ) {
             
             DoThisNode[Grid(0).LoopList(Loop).Node(j)] = 1;
             
          }
          
       }
                
       // Mark nodes completely within control surface       

       for ( p = 1 ; p <= ControlSurface(k).NumberOfLoops() ; p++ ) {
       
          Loop = ControlSurface(k).LoopList(p);
          
          for ( j = 1 ; j <= Grid(0).LoopList(Loop).NumberOfEdges() ; j++ ) {
             
             Edge = Grid(0).LoopList(Loop).Edge(j);
             
             if ( LoopIsOnControlSurface[Grid(0).EdgeList(Edge).Loop1()] +
                  LoopIsOnControlSurface[Grid(0).EdgeList(Edge).Loop2()] == 1 ) {
                     
                DoThisNode[Grid(0).EdgeList(Edge).Node1()] = 0;
                DoThisNode[Grid(0).EdgeList(Edge).Node2()] = 0;
                
             }
             
          }
          
       }
                     
       for ( p = 1 ; p <= ControlSurface(k).NumberOfLoops() ; p++ ) {
       
          Loop = ControlSurface(k).LoopList(p);
          
          for ( j = 1 ; j <= Grid(0).LoopList(Loop).NumberOfNodes() ; j++ ) {
             
             Node = Grid(0).LoopList(Loop).Node(j);
             
             if ( DoThisNode[Node] ) {
                
                DoThisNode[Node] = 0;
               
                xyz[0] = Grid(0).NodeList(Node).x();
                xyz[1] = Grid(0).NodeList(Node).y();
                xyz[2] = Grid(0).NodeList(Node).z();
                
                ControlSurface(k).UnRotateXYZPoint(xyz);
             
                Grid(0).NodeList(Node).x() = xyz[0];
                Grid(0).NodeList(Node).y() = xyz[1];
                Grid(0).NodeList(Node).z() = xyz[2];
                
             }
             
          }
            
       }
       
    }

    UpdateMeshes();
    
    delete [] DoThisNode;

    delete [] LoopIsOnControlSurface;

    for ( k = 1 ; k <= NumberOfVortexSheets() ; k++ ) {

       for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {
 
          for ( j = 1 ; j <= VortexSheet(k).TrailingVortex(i).NumberOfNodes() ; j++ ) {

             // Wake node on MGLevel_ mesh
             
             Node = VortexSheet(k).TrailingVortex(i).GlobalNode(j);
        
             // Update the wake data
                 
             VortexSheet(k).TrailingVortex(i).WakeNodeX(j) = Grid(0).NodeList(Node).x();
             VortexSheet(k).TrailingVortex(i).WakeNodeY(j) = Grid(0).NodeList(Node).y();
             VortexSheet(k).TrailingVortex(i).WakeNodeZ(j) = Grid(0).NodeList(Node).z();

          }

       }
       
    }
      
}

/*##############################################################################
#                                                                              #
#                            VSP_GEOM UpdateMeshes                             #
#                                                                              #
##############################################################################*/

void VSP_GEOM::UpdateSurfacesMeshes(void)
{
   
   UpdateMeshes(0);
   
}

/*##############################################################################
#                                                                              #
#                            VSP_GEOM UpdateMeshes                             #
#                                                                              #
##############################################################################*/

void VSP_GEOM::UpdateMeshes(void)
{
   
   UpdateMeshes(1);
   
}

/*##############################################################################
#                                                                              #
#                            VSP_GEOM UpdateMeshes                             #
#                                                                              #
##############################################################################*/

void VSP_GEOM::UpdateMeshes(int UpdateWakes)
{
   
    int i, j, k, j1, j2, Node, Node1, Node2, LE_Edge, TE_Edge;

    // Update the nodes on the wake that attach to the trailing edge nodes

    if ( UpdateWakes ) {
       
       for ( k = 1 ; k <= NumberOfVortexSheets() ; k++ ) {
      
          for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {
             
             // Node on trailing edge of wing
             
             j1 = VortexSheet(k).TrailingVortex(i).TE_Node();
             
             // First node on wake that is attached to TE node on wing
             
             j2 = VortexSheet(k).TrailingVortex(i).GlobalNode(1);
             
             Grid(0).NodeList(j2).x() = Grid(0).NodeList(j1).x();
             Grid(0).NodeList(j2).y() = Grid(0).NodeList(j1).y();
             Grid(0).NodeList(j2).z() = Grid(0).NodeList(j1).z();
                                 
          }
      
       }    
       
    }
        
    // Update the loop data finest grid
    
    Grid(0).CalculateTriNormalsAndCentroids();

    // Update the edge data on the finest mesh
    
    for ( i = 1 ; i <= Grid(0).NumberOfEdges() ; i++ ) {
       
       Node1 = Grid(0).EdgeList(i).Node1();
       Node2 = Grid(0).EdgeList(i).Node2();
   
       Grid(0).EdgeList(i).UpdateGeometryLocation(Grid(0).NodeList(Node1), Grid(0).NodeList(Node2));
  
    }

    // Update the loop circumferences
    
    Grid(0).CalculateLoopCircumferences();
    
    // Update the kutta node location data
    
    for ( i = 1 ; i <= Grid(0).NumberOfKuttaNodes() ; i++ ) {

       Node = Grid(0).KuttaNode(i);
       
       Grid(0).WakeTrailingEdgeX(i) = Grid(0).NodeList(Node).x();
       Grid(0).WakeTrailingEdgeY(i) = Grid(0).NodeList(Node).y();
       Grid(0).WakeTrailingEdgeZ(i) = Grid(0).NodeList(Node).z(); 
       
    }        
       
    // Now update the coarser meshes recursively
     
    for ( i = 1 ; i <= NumberOfGridLevels_ ; i++ ) {
       
       UpdateCoarseMesh(Grid(i-1),Grid(i));
       
    }

    // Update the wake data
    
    if ( VortexSheet_ != NULL ) {
       
       // Update the trailing vortex xyz's for rotors, as we need the dS values updated for the wake edges
       
       for ( k = 1 ; k <= NumberOfVortexSheets() ; k++ ) {
          
          if ( VortexSheet(k).IsARotor() ) {
      
             for ( i = 1 ; i <= VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {
                
                for ( j = 2 ; j <= VortexSheet(k).TrailingVortex(i).NumberOfNodes() ; j++ ) {
   
                   // Wake node on MGLevel_ mesh
                   
                   Node = VortexSheet(k).TrailingVortex(i).GlobalNode(j);
                   
                   // Corresponding node on finest mesh
                   
                   Node = Grid(SolveOnMGLevel_).NodeList(Node).FineGridNode();
                                
                   // Update the trailing vortices
      
                   VortexSheet(k).TrailingVortex(i).WakeNodeX(j) = Grid(0).NodeList(Node).x();
                   VortexSheet(k).TrailingVortex(i).WakeNodeY(j) = Grid(0).NodeList(Node).y();
                   VortexSheet(k).TrailingVortex(i).WakeNodeZ(j) = Grid(0).NodeList(Node).z();
                
                }
                
          //      VortexSheet(k).TrailingVortex(i).UpdateWakeSpacing();
                
             }
             
          }
          
       }       
       
       // Update the chord lengths
       
       for ( k = 1 ; k <= NumberOfVortexSheets() ; k++ ) {
      
          for ( i = 1 ; i < VortexSheet(k).NumberOfTrailingVortices() ; i++ ) {
             
             LE_Edge = ABS(VortexSheet(k).TrailingVortex(i).LE_Edge());
             TE_Edge = ABS(VortexSheet(k).TrailingVortex(i).TE_Edge());
   
             VortexSheet(k).TrailingVortex(i).LocalChord() = sqrt( pow(Grid(SolveOnMGLevel_).EdgeList(LE_Edge).Xc() - Grid(SolveOnMGLevel_).EdgeList(TE_Edge).Xc(), 2.)
                                                                 + pow(Grid(SolveOnMGLevel_).EdgeList(LE_Edge).Yc() - Grid(SolveOnMGLevel_).EdgeList(TE_Edge).Yc(), 2.)
                                                                 + pow(Grid(SolveOnMGLevel_).EdgeList(LE_Edge).Zc() - Grid(SolveOnMGLevel_).EdgeList(TE_Edge).Zc(), 2.) );
                 
          }
          
       }
       
    }
       
}

/*##############################################################################
#                                                                              #
#                        VSP_GEOM UpdateCoarseMesh                             #              
#                                                                              #
##############################################################################*/

void VSP_GEOM::UpdateCoarseMesh(VSP_GRID &FineGrid, VSP_GRID &CoarseGrid)
{
    
    int i, j, k, Node, Node1, Node2, Next;
    double Length, Area, Mag, Xb, Yb, Zb;
    double x1, y1, z1, x2, y2, z2;
        
    // Update the coarse grid nodes
    
    for ( i = 1 ; i <= CoarseGrid.NumberOfNodes() ; i++ ) {
       
       Node1 = CoarseGrid.NodeList(i).FineGridNode();
       
       CoarseGrid.NodeList(i).x() = FineGrid.NodeList(Node1).x();
       CoarseGrid.NodeList(i).y() = FineGrid.NodeList(Node1).y();
       CoarseGrid.NodeList(i).z() = FineGrid.NodeList(Node1).z();
       
    }
      
    // Update the coarse grid edges
    
    for ( i = 1 ; i <= CoarseGrid.NumberOfEdges() ; i++ ) {
       
       Node1 = CoarseGrid.EdgeList(i).Node1();
       Node2 = CoarseGrid.EdgeList(i).Node2();
   
       CoarseGrid.EdgeList(i).UpdateGeometryLocation(CoarseGrid.NodeList(Node1), CoarseGrid.NodeList(Node2));
  
    }

  //  // Save old loop circumferences for time accurate solution
  //  
  //  for ( i = 1 ; i <= CoarseGrid.NumberOfLoops() ; i++ ) {
  //     
  //     CoarseGrid.LoopList(i).Circumference(0) = CoarseGrid.LoopList(i).Circumference(1);
  //     
  //  }
    
    // Update the loop circumferences
    
    CoarseGrid.CalculateLoopCircumferences();
        
    // Update the coarse grid loops
    
    for ( i = 1 ; i <= CoarseGrid.NumberOfLoops() ; i++ ) {

       // Area
     
       CoarseGrid.LoopList(i).Area() = 0.;
     
       // Reference length
     
       CoarseGrid.LoopList(i).RefLength() = 0.;
     
       // Loop Normal
     
       CoarseGrid.LoopList(i).Nx() = 0.;
       CoarseGrid.LoopList(i).Ny() = 0.;
       CoarseGrid.LoopList(i).Nz() = 0.; 
    
       // Centroid
       
       CoarseGrid.LoopList(i).Xc() = 0.;
       CoarseGrid.LoopList(i).Yc() = 0.;
       CoarseGrid.LoopList(i).Zc() = 0.;         
         
       // UV Centroid
       
       CoarseGrid.LoopList(i).Uc() = 0.;
       CoarseGrid.LoopList(i).Vc() = 0.;

       // Bounding box
       
       CoarseGrid.LoopList(i).BoundBox().x_min =  1.e9;
       CoarseGrid.LoopList(i).BoundBox().x_max = -1.e9;

       CoarseGrid.LoopList(i).BoundBox().y_min =  1.e9;
       CoarseGrid.LoopList(i).BoundBox().y_max = -1.e9;

       CoarseGrid.LoopList(i).BoundBox().z_min =  1.e9;
       CoarseGrid.LoopList(i).BoundBox().z_max = -1.e9;     
     
    }

    // Agglomerate, area weighted, the fine grid data to the coarse grid
    
    for ( i = 1 ; i <= FineGrid.NumberOfLoops() ; i++ ) {

        Next = FineGrid.LoopList(i).CoarseGridLoop();
        
        // Area
        
        Area = FineGrid.LoopList(i).Area();
       
        CoarseGrid.LoopList(Next).Area() += Area;
      
        // Reference Length
      
        CoarseGrid.LoopList(Next).RefLength() += Area*FineGrid.LoopList(i).RefLength();
    
        // Loop Normal
        
        CoarseGrid.LoopList(Next).Nx() += Area*FineGrid.LoopList(i).Nx();
        CoarseGrid.LoopList(Next).Ny() += Area*FineGrid.LoopList(i).Ny();
        CoarseGrid.LoopList(Next).Nz() += Area*FineGrid.LoopList(i).Nz();

        // UV Centroid
        
        CoarseGrid.LoopList(Next).Uc() += Area*FineGrid.LoopList(i).Uc();
        CoarseGrid.LoopList(Next).Vc() += Area*FineGrid.LoopList(i).Vc();      
        
        // Bounding box information
        
        CoarseGrid.LoopList(Next).BoundBox().x_min = MIN(CoarseGrid.LoopList(Next).BoundBox().x_min, FineGrid.LoopList(i).BoundBox().x_min );
        CoarseGrid.LoopList(Next).BoundBox().x_max = MAX(CoarseGrid.LoopList(Next).BoundBox().x_max, FineGrid.LoopList(i).BoundBox().x_max );
       
        CoarseGrid.LoopList(Next).BoundBox().y_min = MIN(CoarseGrid.LoopList(Next).BoundBox().y_min, FineGrid.LoopList(i).BoundBox().y_min );
        CoarseGrid.LoopList(Next).BoundBox().y_max = MAX(CoarseGrid.LoopList(Next).BoundBox().y_max, FineGrid.LoopList(i).BoundBox().y_max );
       
        CoarseGrid.LoopList(Next).BoundBox().z_min = MIN(CoarseGrid.LoopList(Next).BoundBox().z_min, FineGrid.LoopList(i).BoundBox().z_min );
        CoarseGrid.LoopList(Next).BoundBox().z_max = MAX(CoarseGrid.LoopList(Next).BoundBox().z_max, FineGrid.LoopList(i).BoundBox().z_max );

    }  
   
    // Size the bounding box

    for ( i = 1 ; i <= CoarseGrid.NumberOfLoops() ; i++ ) {

       box_calculate_size(CoarseGrid.LoopList(i).BoundBox());

    }    
     
    // Area average the results
  
    for ( i = 1 ; i <= CoarseGrid.NumberOfLoops() ; i++ ) {

       // Area
            
       Area = CoarseGrid.LoopList(i).Area();

       // Length

       CoarseGrid.LoopList(i).Length() = sqrt(CoarseGrid.LoopList(i).Area());

       // Reference length;
       
       CoarseGrid.LoopList(i).RefLength() /= Area;
     
       // Loop Normal
       
       Mag = sqrt(vector_dot(CoarseGrid.LoopList(i).Normal(), CoarseGrid.LoopList(i).Normal()));
        
       CoarseGrid.LoopList(i).Nx() /= Mag;
       CoarseGrid.LoopList(i).Ny() /= Mag;
       CoarseGrid.LoopList(i).Nz() /= Mag;

       // UV Centroid
       
       CoarseGrid.LoopList(i).Uc() /= Area;
       CoarseGrid.LoopList(i).Vc() /= Area;   
 
    }    
 
     // Calculate the loop centroid
    
    for ( i = 1 ; i <= CoarseGrid.NumberOfLoops() ; i++ ) {
  
       CoarseGrid.LoopList(i).Xc() = 0.;
       CoarseGrid.LoopList(i).Yc() = 0.;
       CoarseGrid.LoopList(i).Zc() = 0.;  
                 
       for ( j = 1 ; j <= CoarseGrid.LoopList(i).NumberOfNodes() ; j++ ) {
          
          // Centroid
          
          Node = CoarseGrid.LoopList(i).Node(j);
          
          CoarseGrid.LoopList(i).Xc() += CoarseGrid.NodeList(Node).x();
          CoarseGrid.LoopList(i).Yc() += CoarseGrid.NodeList(Node).y();
          CoarseGrid.LoopList(i).Zc() += CoarseGrid.NodeList(Node).z();    
          
       }
  
       CoarseGrid.LoopList(i).Xc() /= CoarseGrid.LoopList(i).NumberOfNodes();
       CoarseGrid.LoopList(i).Yc() /= CoarseGrid.LoopList(i).NumberOfNodes();
       CoarseGrid.LoopList(i).Zc() /= CoarseGrid.LoopList(i).NumberOfNodes();
       
    }   
    
    // Calculate centroid offset
  
    for ( i = 1 ; i <= CoarseGrid.NumberOfLoops() ; i++ ) {
    
       Xb = 0.5*( CoarseGrid.LoopList(i).BoundBox().x_max + CoarseGrid.LoopList(i).BoundBox().x_min );
       Yb = 0.5*( CoarseGrid.LoopList(i).BoundBox().y_max + CoarseGrid.LoopList(i).BoundBox().y_min );
       Zb = 0.5*( CoarseGrid.LoopList(i).BoundBox().z_max + CoarseGrid.LoopList(i).BoundBox().z_min );

           CoarseGrid.LoopList(i).CentroidOffSet() = sqrt( pow(CoarseGrid.LoopList(i).Xc() - Xb,2.)
                                                         + pow(CoarseGrid.LoopList(i).Yc() - Yb,2.)
                                                         + pow(CoarseGrid.LoopList(i).Zc() - Zb,2.) );

    }            

    // Recalculate the loop length using the nodal data
    
    for ( k = 1 ; k <= CoarseGrid.NumberOfLoops() ; k++ ) {
    
       Length = 0.;
       
       for ( i = 1 ; i <= CoarseGrid.LoopList(k).NumberOfNodes() ; i++ ) {
          
          Node1 = CoarseGrid.LoopList(k).Node(i);
          
          x1 = CoarseGrid.NodeList(Node1).x();
          y1 = CoarseGrid.NodeList(Node1).y();
          z1 = CoarseGrid.NodeList(Node1).z();
       
          for ( j = 1 ; j <= CoarseGrid.LoopList(k).NumberOfNodes() ; j++ ) {
       
             Node2 = CoarseGrid.LoopList(k).Node(j);
             
             x2 = CoarseGrid.NodeList(Node2).x();
             y2 = CoarseGrid.NodeList(Node2).y();
             z2 = CoarseGrid.NodeList(Node2).z();
             
             Length = MAX(Length,sqrt( pow(x1-x2,2.) + pow(y1-y2,2.) + pow(z1-z2,2.) ));

          }
          
       }
       
       CoarseGrid.LoopList(k).Length() = MAX(Length, CoarseGrid.LoopList(k).Length());
       
    }

    // Copy over any kutta node information
   
    for ( i = 1 ; i <= CoarseGrid.NumberOfKuttaNodes() ; i++ ) {

       Node = CoarseGrid.KuttaNode(i);
       
       CoarseGrid.WakeTrailingEdgeX(i) = CoarseGrid.NodeList(Node).x();
       CoarseGrid.WakeTrailingEdgeY(i) = CoarseGrid.NodeList(Node).y();
       CoarseGrid.WakeTrailingEdgeZ(i) = CoarseGrid.NodeList(Node).z(); 
       
    }
           
}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM UpdateVortexCoreSize                        #
#                                                                              #
##############################################################################*/

void VSP_GEOM::UpdateVortexCoreSize(void)
{

    int i, j, k, Edge, Loop, TE_Edge, WakeNode;
    double *CoreSize, *Denom, Length;
    
    CoreSize = new double[Grid(0).NumberOfNodes() + 1];
    
    Denom = new double[Grid(0).NumberOfNodes() + 1];

    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       zero_double_array(CoreSize, Grid(0).NumberOfNodes());
       
       zero_double_array(Denom, Grid(0).NumberOfNodes());
           
       // Sum up kutta edge lengths at each trailing edge node
           
       for ( j = 1 ; j < VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
          
          TE_Edge = VortexSheet(k).TrailingVortex(j).TE_Edge();
          
          Length = Grid(SolveOnMGLevel_).EdgeList(TE_Edge).Length();
          
          CoreSize[Grid(SolveOnMGLevel_).EdgeList(TE_Edge).Node1()] += Length;
          
          CoreSize[Grid(SolveOnMGLevel_).EdgeList(TE_Edge).Node2()] += Length;

          Denom[Grid(SolveOnMGLevel_).EdgeList(TE_Edge).Node1()] += 1.;

          Denom[Grid(SolveOnMGLevel_).EdgeList(TE_Edge).Node2()] += 1.;

       }
       
       // Calculate average length and core size

       for ( j = 1 ; j < VortexSheet(k).NumberOfKuttaEdges() ; j++ ) {
          
          CoreSize[VortexSheet(k).TrailingVortex(j).TE_Node()] /= Denom[VortexSheet(k).TrailingVortex(j).TE_Node()];
                    
          CoreSize[VortexSheet(k).TrailingVortex(j).TE_Node()] *= 0.25 * CoreSizeFactor_;
          
       }

       // Distribute core size out to the wake nodes

       for ( j = 1 ; j < VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
            
          for ( i = 1 ; i <= VortexSheet(k).NumberOfWakeTrailingNodes() ; i++ ) {
             
             WakeNode = VortexSheet(k).TrailingVortex(j).GlobalNode(i);
             
             CoreSize[WakeNode] = CoreSize[VortexSheet(k).TrailingVortex(j).TE_Node()];
             
          }
          
       }
       
       // Distribute core size to the wake edges
             
       for ( i = 1 ; i <= VortexSheet(k).NumberOfWakeLoops() ; i++ ) {

          Loop = VortexSheet(k).WakeLoopList(i).GlobalLoop();
          
          for ( j = 1 ; j <= Grid(SolveOnMGLevel_).LoopList(Loop).NumberOfEdges() ; j++ ) {
             
             Edge = Grid(SolveOnMGLevel_).LoopList(Loop).Edge(j);
             
             Grid(SolveOnMGLevel_).EdgeList(Edge).CoreWidth() = 0.5*( CoreSize[Grid(SolveOnMGLevel_).EdgeList(Edge).Node1()]
                                                                    + CoreSize[Grid(SolveOnMGLevel_).EdgeList(Edge).Node2()] );
                                                                    
          }
             
       }
       
    }
      
} 
       
/*##############################################################################
#                                                                              #
#                             VSP_GEOM ReadCart3DDataFromFile                  #
#                                                                              #
##############################################################################*/

void VSP_GEOM::ReadCart3DDataFromFile(char *Name, FILE *CART3D_File, FILE *TKEY_File)
{
 
    int i, k, n, NumNodes, NumTris, Node1, Node2, Node3, SurfaceID, Done;
    int *SurfaceList, Found, DumInt, *ComponentIDForVSPSurface;
    int *SurfaceIsUsed, NumberOfVSPSurfaces;
    char DumChar[MAX_CHAR_SIZE], Comma[MAX_CHAR_SIZE], *Next;
    double x, y, z, u1, u2, u3, v1, v2, v3;
    double Ymin, Ymax, Zmin, Zmax;
    
    InputMeshIsMixedPolys_ = 0;

    snprintf(Comma,sizeof(Comma)*sizeof(char),",");

    // Save the component name
    
    snprintf(ComponentName_,sizeof(ComponentName_)*sizeof(char),"%s_Cart3d",Name);
    
    // Set surface type
    
    SurfaceType_ = CART3D_SURFACE;
    
    // Read in the header
    
    fscanf(CART3D_File,"%d %d",&NumNodes,&NumTris);

    printf("NumNodes, NumTris: %d %d \n",NumNodes, NumTris);
    
    // Estimate the maximum number of grid levels
    
    MaxNumberOfGridLevels_ = 4*(int) ( log(0.5 * NumTris) / log(4.0) );
    
    printf("MaxNumberOfGridLevels_: %d \n",MaxNumberOfGridLevels_);
    
    Grid_ = new VSP_GRID*[MaxNumberOfGridLevels_ + 1];
    
    // Calculate total number of nodes and tris, including the wake

    Grid_[0] = new VSP_GRID;
     
    Grid().SizeNodeList(NumNodes);
    
    Grid().SizeTriList(NumTris);
    
    // Set surface type
    
    Grid().SurfaceType() = CART3D_SURFACE;
    
    // Kutta nodes... 

    Grid().SizeKuttaNodeList(0);    

    // Read in xyz data
    
    for ( n = 1 ; n <= NumNodes ; n++ ) {
       
       fscanf(CART3D_File,"%lf %lf %lf \n",&x,&y,&z);

       Grid().NodeList(n).x() = x;
       Grid().NodeList(n).y() = y;
       Grid().NodeList(n).z() = z;
    
       Grid().NodeList(n).IsTrailingEdgeNode()   = 0;

       Grid().NodeList(n).IsBoundaryEdgeNode()   = 0;
       
       Grid().NodeList(n).IsBoundaryCornerNode() = 0;
         
    }    

    // Read in connectivity
    
    for ( n = 1 ; n <= NumTris ; n++ ) {
       
       fscanf(CART3D_File,"%d %d %d \n",&Node1,&Node2,&Node3);

       Grid().TriList(n).Node1() = Node1;
       Grid().TriList(n).Node2() = Node2;
       Grid().TriList(n).Node3() = Node3;
          
    }

    // Adjust geometry for ground effects analysis
    
    if ( DoGroundEffectsAnalysis_ ) RotateGeometry_About_Y_Axis();
    
    // Read in the surface ID
    
    SurfaceIsUsed = new int[NumTris + 1];
    
    zero_int_array(SurfaceIsUsed, NumTris);
    
    for ( n = 1 ; n <= NumTris ; n++ ) {
       
       fscanf(CART3D_File,"%d \n",&SurfaceID);

       Grid().TriList(n).SurfaceID()         = SurfaceID;
       
       Grid().TriList(n).SurfaceType()       = THICK_SURFACE;
       
       Grid().TriList(n).VortexSheet()       = 0;
       
       Grid().TriList(n).SpanStation()       = 0;

       Grid().TriList(n).IsTrailingEdgeTri() = 0;
       
       SurfaceIsUsed[SurfaceID] = 1;
       
    }
 
    // Determine the number of surfaces
    
    NumberOfSurfaces_ = 0;
    
    SurfaceList = new int[NumTris + 1];
    
    zero_int_array(SurfaceList, NumTris);
    
    for ( n = 1 ; n <= NumTris ; n++ ) {
       
       SurfaceID = Grid().TriList(n).SurfaceID();
     
       i = 1;
       
       Found = 0;
       
       while ( !Found && i <= NumberOfSurfaces_ ) {
          
          if ( SurfaceList[i] == SurfaceID ) Found = 1;
          
          i++;
          
       }
       
       if ( !Found ) {
          
          NumberOfSurfaces_++;
          
          SurfaceList[NumberOfSurfaces_] = SurfaceID;
          
       }
       
    }
    
    SurfaceNameList_ = new char*[NumberOfSurfaces_ + 1];
    
    SurfaceGIDList_ = new char*[NumberOfSurfaces_ + 1];
    
    BoundaryConditionForSurface_ = new BOUNDARY_CONDITION_DATA[NumberOfSurfaces_ + 2];
    
    ComponentIDForSurface_ = new int[NumberOfSurfaces_ + 1];

    SurfaceIsThick_ = new int[NumberOfSurfaces_ + 1];
    
    SurfaceDegenType_ = new int[NumberOfSurfaces_ + 1];
    
    ComponentIsThick_ = new int[NumberOfSurfaces_ + 1];
    
    ComponentIsLifting_ = new int[NumberOfSurfaces_ + 1];
    
    for ( i = 1 ; i <= NumberOfSurfaces_ ; i++ ) {
       
       SurfaceIsThick_[i] = 1;
     
       ComponentIsThick_[i] = 1;
       
       ComponentIsLifting_[i] = 0;
       
    }

    printf("Found %d CART3D Surfaces \n",NumberOfSurfaces_);

    if ( TKEY_File != NULL ) {
       
       // Read in the vkey data
       
       fgets(DumChar,MAX_CHAR_SIZE,TKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);
       fgets(DumChar,MAX_CHAR_SIZE,TKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);
       fgets(DumChar,MAX_CHAR_SIZE,TKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);
       
       sscanf(DumChar,"%d\n",&NumberOfVSPSurfaces);

       ComponentIDForVSPSurface = new int[NumberOfVSPSurfaces + 1];
       
       printf("NumberOfVSPSurfaces: %d \n",NumberOfVSPSurfaces);

       fgets(DumChar,MAX_CHAR_SIZE,TKEY_File);
      
       k = 0;
       
       for ( n = 1 ; n <= NumberOfVSPSurfaces ; n++ ) {
          
          fgets(DumChar,MAX_CHAR_SIZE,TKEY_File);
 
          Next = strtok(DumChar,Comma); 

          DumInt = atoi(Next);
          
          Next = strtok(NULL,Comma);  Next[strcspn(Next, "\n")] = 0;
          
          if ( SurfaceIsUsed[DumInt] ) {
             
             k++;
          
             ComponentIDForVSPSurface[n] = k;
             
             ComponentIDForSurface_[k] = n;
             
             SurfaceNameList_[k] = new char[MAX_CHAR_SIZE];
                          
             snprintf(SurfaceNameList_[k],MAX_CHAR_SIZE*sizeof(char),"%s",Next);
          
             printf("Surface: %d exists in tringulation and will be surface: %d with OpenVSP Name: %s \n",DumInt,k,SurfaceNameList_[k]);
  
          }
          
       }
       
       if ( k != NumberOfSurfaces_ ) {
          
          printf("Error... number of used surfaces in .tri and .tkey files do not match! \n");
          fflush(NULL);exit(1);
          
       }
          
    }
    
    else {
       
       for ( n = 1 ; n <= NumberOfSurfaces_ ; n++ ) {
 
          ComponentIDForSurface_[n] = n;
          
          SurfaceNameList_[n] = new char[MAX_CHAR_SIZE];
          
          snprintf(SurfaceNameList_[n],MAX_CHAR_SIZE*sizeof(char),"Surface_%d",n);
          
       }
       
    }

    for ( i = 1 ; i <= NumberOfSurfaces_ ; i++ ) {
       
       printf("SurfaceList[%d]: %d \n",i,SurfaceList[i]);
       
    }

    fflush(NULL);
    
    if ( TKEY_File != NULL ) {
    
       for ( i = 1 ; i <= NumberOfVSPSurfaces ; i++ ) {
          
          printf("ComponentIDForVSPSurface[%d]: %d \n",i,ComponentIDForVSPSurface[i]);
          
       }
       
       fflush(NULL);

       for ( i = 1 ; i <= NumberOfSurfaces_ ; i++ ) {
          
         ComponentIDForSurface_[i] = ComponentIDForVSPSurface[ComponentIDForSurface_[i]];
          
       }
       
    }
    
    for ( i = 1 ; i <= NumberOfSurfaces_ ; i++ ) {
       
       printf("ComponentIDForSurface_[%d]: %d \n",i,ComponentIDForSurface_[i]);
       
    }
    
    fflush(NULL);
        
    // Renumber the surfaces
    
    for ( n = 1 ; n <= NumTris ; n++ ) {
    
       Done = 0;
       
       i = 1;
       
       while ( !Done && i <= NumberOfSurfaces_) {
          
          if ( Grid().TriList(n).SurfaceID() == SurfaceList[i] ) {
             
             Done = 1;

             if ( TKEY_File != NULL ) {
                
                Grid().TriList(n).ComponentID() = ComponentIDForVSPSurface[Grid().TriList(n).SurfaceID()];
                
             }
             
             else {
                
                Grid().TriList(n).ComponentID() = i;
                
             }
                          
             Grid().TriList(n).SurfaceID() = i;

          }
          
          i++;
          
       }
       
       if ( !Done ) {
          
          printf("Error in determing number of surfaces in CART3D file! \n");
          fflush(NULL);
          exit(1);
          
       }
       
    }

    delete [] SurfaceList;
    
    if ( TKEY_File ) delete [] ComponentIDForVSPSurface;
    
    // Hack for u,v data...

    for ( i = 1 ; i <= NumberOfSurfaces_ ; i++ ) {
       
       Ymin = 1.e9;
       Ymax = -Ymin;
       
       Zmin = 1.e6;
       Zmax = -Zmin;
       
       for ( n = 1 ; n <= NumTris ; n++ ) {
          
          // Find min/max in Y and Z
            
          if ( i == Grid().TriList(n).SurfaceID() ) {
             
             Ymin = MIN(Grid().NodeList(Grid().TriList(n).Node1()).y(),Ymin);
             Ymax = MAX(Grid().NodeList(Grid().TriList(n).Node1()).y(),Ymax);
             
             Ymin = MIN(Grid().NodeList(Grid().TriList(n).Node2()).y(),Ymin);
             Ymax = MAX(Grid().NodeList(Grid().TriList(n).Node2()).y(),Ymax);
                          
             Ymin = MIN(Grid().NodeList(Grid().TriList(n).Node3()).y(),Ymin);
             Ymax = MAX(Grid().NodeList(Grid().TriList(n).Node3()).y(),Ymax);
                          
             Zmin = MIN(Grid().NodeList(Grid().TriList(n).Node1()).z(),Zmin);
             Zmax = MAX(Grid().NodeList(Grid().TriList(n).Node1()).z(),Zmax);

             Zmin = MIN(Grid().NodeList(Grid().TriList(n).Node2()).z(),Zmin);
             Zmax = MAX(Grid().NodeList(Grid().TriList(n).Node2()).z(),Zmax);
             
             Zmin = MIN(Grid().NodeList(Grid().TriList(n).Node3()).z(),Zmin);
             Zmax = MAX(Grid().NodeList(Grid().TriList(n).Node3()).z(),Zmax);
                                       
          }
          
       }
       
       for ( n = 1 ; n <= NumTris ; n++ ) {
                    
          if ( i == Grid().TriList(n).SurfaceID() ) {
             
             // If larger in Y... assume we can interpolate in Y for span loading

             if ( ABS(Ymax - Ymin) > ABS(Zmax - Zmin) ) {
                
                u1 = Grid().NodeList(Grid().TriList(n).Node1()).y(); 
                u2 = Grid().NodeList(Grid().TriList(n).Node2()).y();
                u3 = Grid().NodeList(Grid().TriList(n).Node3()).y();
                
                v1 = v2 = v3 = 0.;
                
             }
             
             // Otherwise... Z
             
             else {
                
                u1 = Grid().NodeList(Grid().TriList(n).Node1()).z(); 
                u2 = Grid().NodeList(Grid().TriList(n).Node2()).z();
                u3 = Grid().NodeList(Grid().TriList(n).Node3()).z();
                
                v1 = v2 = v3 = 0.;
                                
             }          
       
             Grid().TriList(n).U_Node(1) = u1; Grid().TriList(n).V_Node(1) = v1;
             Grid().TriList(n).U_Node(2) = u2; Grid().TriList(n).V_Node(2) = v2;
             Grid().TriList(n).U_Node(3) = u3; Grid().TriList(n).V_Node(3) = v3;
             
          }
             
       }
       
    }

}

/*##############################################################################
#                                                                              #
#                          VSP_GEOM ReadVSPGeomDataFromFile                    #
#                                                                              #
##############################################################################*/

void VSP_GEOM::ReadVSPGeomDataFromFile(char *Name, FILE *VSPGeom_File, FILE *VKEY_File)
{
 
    int i, j, k, n, DumInt, NumNodes, NumLoops, NumTris;
    int Node, Node1, Node2, Node3, SurfaceID, SubSurfaceID, Done;
    int *SurfaceList, Found, CompID, SurfID, ThickThinDataExists, NumTriNodes;
    int *SurfaceIsUsed, NumberOfVSPSurfaces, ModelType, *OriginalSurfaceIsThick;
    int NumberOfRefineLevels, WakeNodes;    
    int NumKuttaNodeLists, NumNodesInList, FileVersion;
    char DumChar[MAX_CHAR_SIZE], DumChar2[MAX_CHAR_SIZE], Comma[MAX_CHAR_SIZE];
    char TempName[MAX_CHAR_SIZE], TempGIDName[MAX_CHAR_SIZE], Space[MAX_CHAR_SIZE], *Next;
    double x, y, z, u, v, w, u1, v1, u2, v2, u3, v3, Eps;
    fpos_t TopOfTriangluation;
    
    int *Temp_ComponentIDForSurface;
    int *Temp_OpenVSP_ComponentIDForSurface;
    char **Temp_SurfaceNameList;
    char **Temp_SurfaceGIDList;
        
    // Read in first line and look for v2 files
    
    fgets(DumChar,MAX_CHAR_SIZE,VSPGeom_File);
    
    FileVersion = 1;
    
    if ( strstr(DumChar,"v2") != NULL ) FileVersion = 2;

    if ( strstr(DumChar,"v3") != NULL ) FileVersion = 3;
    
    if ( FileVersion == 1 ) rewind(VSPGeom_File);

    snprintf(Comma,sizeof(Comma)*sizeof(char),",");

    snprintf(Space,sizeof(Space)*sizeof(char)," ");

    // Save the component name
    
    snprintf(ComponentName_,sizeof(ComponentName_)*sizeof(char),"%s_VSPGeom",Name);
    
    // Set surface type
    
    SurfaceType_ = VSPGEOM_SURFACE;
    
    // Read in the number of levels
    
    if ( FileVersion == 3 ) fscanf(VSPGeom_File,"%d",&NumberOfRefineLevels);
    
    // Read in xyz data
    
    if ( FileVersion == 2 ) {
    
       fscanf(VSPGeom_File,"%d",&NumNodes);
       
       printf("NumNodes: %d \n",NumNodes);
       
    }
    
    else if ( FileVersion == 3 ) {
       
       fscanf(VSPGeom_File,"%d %d %d",&NumNodes,&NumLoops,&WakeNodes);
       
       printf("NumNodes: %d \n",NumNodes);       
       
    }
    
    else {
       
       printf("Unknown VSPGEOM file version! \n");
       
       fflush(NULL);exit(1);
       
    }
    
    // Estimate the maximum number of grid levels
    
    MaxNumberOfGridLevels_ = 4*(int) ( log(1.0 * NumNodes) / log(4.0) );
    
    printf("MaxNumberOfGridLevels_: %d \n",MaxNumberOfGridLevels_);
        
    Grid_ = new VSP_GRID*[MaxNumberOfGridLevels_ + 1];
    
    // Calculate total number of nodes and tris, including the wake

    Grid_[0] = new VSP_GRID;    
    
    Grid().SizeNodeList(NumNodes);
    
    // Read in xyz data
       
    for ( n = 1 ; n <= NumNodes ; n++ ) {
       
       fscanf(VSPGeom_File,"%lf %lf %lf \n",&x,&y,&z);

       Grid().NodeList(n).x() = x;
       Grid().NodeList(n).y() = y;
       Grid().NodeList(n).z() = z;
    
       Grid().NodeList(n).IsTrailingEdgeNode()   = 0;
       Grid().NodeList(n).IsBoundaryEdgeNode()   = 0;
       Grid().NodeList(n).IsBoundaryCornerNode() = 0;
         
    }

    // Mark nodes on symmetry plane
    
    if ( DoSymmetryPlaneSolve_ ) {
       
       Eps = 1.e-6;
       
       for ( n = 1 ; n <= Grid().NumberOfNodes() ; n++ ) {

          if ( Grid().NodeList(n).y() <= Eps )  {
             
             printf("Marking node %d as on symmetry plane ... \n",n);fflush(NULL);
             
             Grid().NodeList(n).IsSymmetryPlaneNode() = 1;
            
          }
          
       }    
       
    }     
       
    // Read in the tri data

    fscanf(VSPGeom_File,"%d\n",&NumLoops);

    printf("NumLoops: %d \n",NumLoops);    

    Grid().SizeLoopList(NumLoops);
    
    // Read in connectivity

    InputMeshIsMixedPolys_ = 0;

    for ( n = 1 ; n <= NumLoops ; n++ ) {
       
    //   fscanf(VSPGeom_File,"%d %d %d %d \n",&DumInt, &Node1,&Node2,&Node3);
       
       fgets(DumChar,MAX_CHAR_SIZE,VSPGeom_File);

       Next = strtok(DumChar,Space);

       NumTriNodes = atoi(Next);
       
       Grid().LoopList(n).SizeNodeList(NumTriNodes);
                      
       Grid().LoopList(n).SizeEdgeList(NumTriNodes);

       for ( i = 1 ; i <= NumTriNodes ; i++ ) {
          
          Next = strtok(NULL,Space);
          
          Grid().LoopList(n).Node(i) = atoi(Next);
          
       }

     // printf("Loop: %d ... ",n);
     //
     // for ( i = 1 ; i <= NumTriNodes ; i++ ) {
     //    
     //    printf("%d ",Grid().LoopList(n).Node(i));
     //    
     // }
     // 
     // printf("\n");
       
       if ( NumTriNodes > 3 ) InputMeshIsMixedPolys_ = 1;

    }    

    // Set surface type
    
    Grid().SurfaceType() = VSPGEOM_SURFACE;
    
    // Kutta nodes... 

    Grid().SizeKuttaNodeList(0);    

    // Adjust geometry for ground effects analysis
    
    if ( DoGroundEffectsAnalysis_ ) RotateGeometry_About_Y_Axis();
    
    // Read in the surface UV data
    
    SurfaceIsUsed = new int[NumLoops + 1];
    
    zero_int_array(SurfaceIsUsed, NumLoops);

    for ( n = 1 ; n <= NumLoops ; n++ ) {
             
       //fscanf(VSPGeom_File,"%d %d %lf %lf %lf %lf %lf %lf \n",&SurfaceID,&SubSurfaceID,&u1,&v1,&u2,&v2,&u3,&v3);

       fgets(DumChar,MAX_CHAR_SIZE,VSPGeom_File);
       
       Next = strtok(DumChar,Space); SurfaceID = atoi(Next);
       Next = strtok(NULL,Space);    SubSurfaceID = atoi(Next);

       Grid().LoopList(n).Uc() = 0.;
       Grid().LoopList(n).Vc() = 0.;

       for ( i = 1 ; i <= Grid().LoopList(n).NumberOfNodes() ; i++ ) {
          
          Next = strtok(NULL,Space); u = atof(Next);
          Next = strtok(NULL,Space); v = atof(Next);          
            
          Grid().LoopList(n).U_Node(i) = u;
          Grid().LoopList(n).V_Node(i) = v;

          Grid().LoopList(n).Uc() += u;
          Grid().LoopList(n).Vc() += v;
         
       }

       Grid().LoopList(n).Uc() /= Grid().LoopList(n).NumberOfNodes();
       
       Grid().LoopList(n).Vc() /= Grid().LoopList(n).NumberOfNodes();
       
       Grid().LoopList(n).SurfaceID()         = SurfaceID;
       
       Grid().LoopList(n).ComponentID()       = SurfaceID; // This gets renumbered based on the .vkey file 
      
       Grid().LoopList(n).SurfaceType()       = VSPGEOM_SURFACE;
       
       Grid().LoopList(n).VortexSheet()       = 0;
       
       Grid().LoopList(n).SpanStation()       = 0;

       Grid().LoopList(n).IsTrailingEdgeTri() = 0;       
       
       SurfaceIsUsed[SurfaceID] = 1;

    }
    
    // Read in parent information... this is redundant for the first level
    
    if ( FileVersion == 3 ) {
       
       for ( n = 1 ; n <= NumLoops ; n++ ) {
          
          fscanf(VSPGeom_File,"%d %d\n",&DumInt,&DumInt);
       
       }
       
    }

    // Read in the number of kutta lists
    
    NumberOfKuttaNodes_ = 0;
    
    KuttaNodeList_ = new int[NumNodes + 1];
    
    fscanf(VSPGeom_File,"%d",&NumKuttaNodeLists);

    for ( j = 1 ; j <= NumKuttaNodeLists ; j++ ) {
    
       int WakePartNum;
       // Read in the Kutta nodes in this list
       fscanf(VSPGeom_File,"%d %d",&NumNodesInList, &WakePartNum);

       bool BodyWakeFlag = false;
       if ( NumNodesInList < 0 ) {
          NumNodesInList = -NumNodesInList;
          BodyWakeFlag = true;
       }
       
      printf("NumNodesInList: %d \n",NumNodesInList);fflush(NULL);
       
       if ( NumNodesInList > 0 ) {
          
          for ( i = 1 ; i <= NumNodesInList ; i++ ) {
             
             fscanf(VSPGeom_File,"%d",&(KuttaNodeList_[++NumberOfKuttaNodes_]));
             
             printf("KuttaNodeList_[%d]: %d \n",NumberOfKuttaNodes_,KuttaNodeList_[NumberOfKuttaNodes_]);
             
          }
          
       }

       if ( KuttaNodeList_[NumberOfKuttaNodes_] == KuttaNodeList_[1] ) {
          
          printf("Kutta node list %d is periodic per OpenVSP... \n",j);
          
          NumberOfKuttaNodes_ -= 1;
          
       }
           
    }
    
    printf("VSPGEOM defined NumKuttaNodes: %d \n",NumberOfKuttaNodes_);

    // Determine the number of surfaces
    
    NumberOfSurfaces_ = 0;
    
    SurfaceList = new int[NumLoops + 1];
    
    zero_int_array(SurfaceList, NumLoops);
 
    for ( n = 1 ; n <= NumLoops ; n++ ) {
       
       SurfaceID = Grid().LoopList(n).SurfaceID();
     
       i = 1;
       
       Found = 0;
       
       while ( !Found && i <= NumberOfSurfaces_ ) {
          
          if ( SurfaceList[i] == SurfaceID ) Found = 1;
          
          i++;
          
       }
       
       if ( !Found ) {
          
          NumberOfSurfaces_++;
          
          SurfaceList[NumberOfSurfaces_] = SurfaceID;
                    
       }
       
    }

    zero_int_array(SurfaceList, NumLoops);
    
    printf("Found %d VSPGEOM Surfaces \n",NumberOfSurfaces_); fflush(NULL);
    
    SurfaceNameList_ = new char*[NumberOfSurfaces_ + 1];
    
    SurfaceGIDList_ = new char*[NumberOfSurfaces_ + 1];    
    
    SurfaceIsThick_ = new int[NumberOfSurfaces_ + 1];
    
    SurfaceDegenType_ = new int[NumberOfSurfaces_ + 1];

    ComponentIsThick_ = new int[NumberOfSurfaces_ + 1];
    
    ComponentIsLifting_ = new int[NumberOfSurfaces_ + 1];
    
    BoundaryConditionForSurface_ = new BOUNDARY_CONDITION_DATA[NumberOfSurfaces_ + 2];
    
    ComponentIDForSurface_ = new int[NumLoops + 1];
 
    VSPSurfaceIDForSurface_ = new int[NumLoops + 1];
    
    OpenVSP_ComponentIDForSurface_ = new int[NumLoops + 1];
    
    OriginalSurfaceIsThick = new int[NumLoops + 1];
   
    zero_int_array(ComponentIsThick_, NumberOfSurfaces_);

    zero_int_array(ComponentIsLifting_, NumberOfSurfaces_);
   
    zero_int_array(ComponentIDForSurface_, NumLoops);

    zero_int_array(VSPSurfaceIDForSurface_, NumLoops);
    
    zero_int_array(OpenVSP_ComponentIDForSurface_, NumLoops);
    
    zero_int_array(OriginalSurfaceIsThick, NumLoops);
    
    printf("Found %d VSPGEOM Surfaces \n",NumberOfSurfaces_);
    
    if ( VKEY_File != NULL ) {
       
       // Read in the vkey headers
       
       fgets(DumChar,MAX_CHAR_SIZE,VKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);
       
       fgets(DumChar,MAX_CHAR_SIZE,VKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);
       
       // Get the number of surfaces in the vkey file
       
       fgets(DumChar,MAX_CHAR_SIZE,VKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);
       
       sscanf(DumChar,"%d\n",&NumberOfVSPSurfaces);
       
       printf("NumberOfVSPSurfaces: %d \n",NumberOfVSPSurfaces);

       // Skip a blank line
              
       fgets(DumChar,MAX_CHAR_SIZE,VKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);
       
       // Read in what data exists
       
       fgets(DumChar,MAX_CHAR_SIZE,VKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);

       // Check for thick / thin flag
       
       ThickThinDataExists = 0;
       
       if ( strstr(DumChar,"thick") != NULL ) ThickThinDataExists = 1;
       
       if ( ThickThinDataExists ) {
          
          printf("Thick/thin data exists... DumChar: %s \n",DumChar);
          
       }
       
       else {
          
          printf("No thick/thin identifiers found in vkey file: %s \n",DumChar);
          
       }
             
       k = 0;
       
       for ( n = 1 ; n <= NumberOfVSPSurfaces ; n++ ) {
          
          fgets(DumChar,MAX_CHAR_SIZE,VKEY_File);
          
          //  sscanf(DumChar,"%d,%d,%d,%s,%s",&DumInt,&CompID,&DumInt2,DumChar2,DumChar);
         
          //# part#,geom#,surf#,gname,gid,thick,plate,copy#,geomcopy#
            
          Next = strtok(DumChar,Comma); // Part #

          DumInt = atoi(Next);
          
          Next = strtok(NULL,Comma); // Geom... my Component ID - 1
          
          CompID = atoi(Next);
          
          Next = strtok(NULL,Comma); // Surface ID
          
          SurfID = atoi(Next);
          
          printf("Part: %d ... CompID: %d ... SurfID: %d \n",DumInt,CompID,SurfID);fflush(NULL);

          Next = strtok(NULL,Comma); Next[strcspn(Next, "\n")] = 0; snprintf(TempName,MAX_CHAR_SIZE*sizeof(char),"%s",Next);  // Surface name, per user
          
          Next = strtok(NULL,Comma); Next[strcspn(Next, "\n")] = 0; snprintf(TempGIDName,MAX_CHAR_SIZE*sizeof(char),"%s",Next);// VSP gid
          
          if ( ThickThinDataExists ) Next = strtok(NULL,Comma); // Thick/thin flag
                    
          if ( SurfaceIsUsed[DumInt] ) {

             printf("Surface: %d exists in triangulation and has OpenVSP Name: %s \n",DumInt,TempName);
             
             k++;
                                       
             printf("CompID: %d \n",CompID);
             
             OpenVSP_ComponentIDForSurface_[DumInt] = CompID;
                          
             printf("Setting OpenVSP_ComponentIDForSurface_ to : %d \n",OpenVSP_ComponentIDForSurface_[DumInt]);
             
             VSPSurfaceIDForSurface_[k] = SurfID;
             
             SurfaceList[k] = DumInt;

             SurfaceNameList_[k] = new char[MAX_CHAR_SIZE];

             SurfaceGIDList_[k] = new char[MAX_CHAR_SIZE];
             
             snprintf(SurfaceNameList_[k],MAX_CHAR_SIZE*sizeof(char),"%s",TempName);    

             snprintf(SurfaceGIDList_[k],MAX_CHAR_SIZE*sizeof(char),"%s",TempGIDName);

             SurfaceIsThick_[k] = 0;
  
             if ( ThickThinDataExists ) {
                
                SurfaceIsThick_[k] = atoi(Next);
             
                OriginalSurfaceIsThick[DumInt] = SurfaceIsThick_[k];
                
             }
             
             if ( SurfaceIsThick_[k] ) {
                
                printf("Surface: %d is thick \n",k);
                
             }
             
             else {
             
                printf("Surface: %d is thin \n",k);
                
             }
             
             if ( ThickThinDataExists ) {
                
                Next = strtok(NULL,Comma);
                
                SurfaceDegenType_[k] = atoi(Next);
             
             }
             
             // Symmetry flag
             
             Next = strtok(NULL,Comma);
             
             // VSPAERO Component ID
             
             Next = strtok(NULL,Comma);
             
             ComponentIDForSurface_[DumInt] = atoi(Next); 
             
      //       ComponentIDForSurface_[DumInt] = CompID + 1;

             printf("Setting ComponentIDForSurface_ to : %d \n",ComponentIDForSurface_[DumInt]);
                                   
          }      
          
       }
      
    }

    else {
       
       printf("Could not find vkey file... exiting! \n");
       fflush(NULL);exit(1);
       
    }

    printf("Found %d used surfaces based on parsing VSPGEOM file \n",NumberOfSurfaces_);
    printf("Found %d used surfaces based on parsing VKEY file ... \n",k);
    
    // Mark the tris on thick and thin surfaces
    
    printf("Marking tris on thick and thin surfaces \n");fflush(NULL);
    
    for ( n = 1 ; n <= NumLoops ; n++ ) {

       if ( OriginalSurfaceIsThick[Grid().LoopList(n).SurfaceID()] ) {
        
          Grid().LoopList(n).SurfaceType() = THICK_SURFACE;
    
       }
       
       else {
        
          Grid().LoopList(n).SurfaceType() = THIN_SURFACE;
          
       }          
   
    }   
    
    delete [] OriginalSurfaceIsThick;
        
    // Determine the number of thin and thick surfaces
    
    printf("Determining the number of thin and thick surfaces \n");fflush(NULL);
    
    NumberOfThinSurfaces_ = NumberOfThickSurfaces_ = 0;
    
    for ( k = 1 ; k <= NumberOfSurfaces_ ; k++ ) {
    
       if ( !SurfaceIsThick_[k] ) NumberOfThinSurfaces_++;
       
       if (  SurfaceIsThick_[k] ) NumberOfThickSurfaces_++;
       
    }
    
    printf("NumberOfThinSurfaces_: %d \n",NumberOfThinSurfaces_);
    
    printf("NumberOfThickSurfaces_: %d \n",NumberOfThickSurfaces_);
    
    ModelType_ = PANEL_MODEL;
    
    if ( NumberOfThickSurfaces_ == 0 ) ModelType_ = VLM_MODEL;
    
    if ( NumberOfThinSurfaces_  == 0 ) ModelType_ = PANEL_MODEL;
 
    // Renumber the surfaces
   
    printf("Renumbering surfaces \n");fflush(NULL);

    for ( n = 1 ; n <= NumLoops ; n++ ) {
 
       Done = 0;
       
       i = 1;
       
       while ( !Done && i <= NumberOfSurfaces_ ) {
          
          if ( Grid().LoopList(n).SurfaceID() == SurfaceList[i] ) {
             
             Done = 1;
              
             Grid().LoopList(n).ComponentID() = ComponentIDForSurface_[Grid().LoopList(n).SurfaceID()];

             Grid().LoopList(n).OpenVSP_ComponentID() = OpenVSP_ComponentIDForSurface_[Grid().LoopList(n).SurfaceID()];

             Grid().LoopList(n).SurfaceID() = i;

          }
          
          i++;
          
       }
       
       if ( Grid().LoopList(n).ComponentID() == 0 ) {
          
          printf("wtf ! Loop %d is not associated with any component! \n",n);fflush(NULL);exit(1);
          
       }
       
       if ( !Done ) {
          
          printf("Error in determing number of surfaces in VSPGEOM file! \n");
          fflush(NULL);
          exit(1);
          
       }
       
    }
    
    printf("Renumbering surface lists \n");fflush(NULL);
    
    Temp_ComponentIDForSurface = NULL;
    
    Temp_OpenVSP_ComponentIDForSurface = NULL;
    
    Temp_SurfaceNameList = NULL;
    
    Temp_SurfaceGIDList = NULL;
    
    if ( VKEY_File != NULL ) {
           
       Temp_ComponentIDForSurface = new int[NumberOfSurfaces_ + 1];

       Temp_OpenVSP_ComponentIDForSurface = new int[NumberOfSurfaces_ + 1];
       
       Temp_SurfaceNameList = new char*[NumberOfSurfaces_ + 1];
       
       Temp_SurfaceGIDList = new char*[NumberOfSurfaces_ + 1];
              
       for ( i = 1 ; i <= NumberOfSurfaces_ ; i++ ) {
                    
          Temp_ComponentIDForSurface[i] = ComponentIDForSurface_[SurfaceList[i]];
          
          Temp_OpenVSP_ComponentIDForSurface[i] = OpenVSP_ComponentIDForSurface_[SurfaceList[i]];       
       
          Temp_SurfaceNameList[i] = new char[MAX_CHAR_SIZE];
          
          Temp_SurfaceGIDList[i] = new char[MAX_CHAR_SIZE];
       
          snprintf(Temp_SurfaceNameList[i],MAX_CHAR_SIZE*sizeof(char),"%s",SurfaceNameList_[i]);
          
          snprintf(Temp_SurfaceGIDList[i],MAX_CHAR_SIZE*sizeof(char),"%s",SurfaceGIDList_[i]);
                              
       }

       delete [] ComponentIDForSurface_;
       delete [] OpenVSP_ComponentIDForSurface_;
       
       for ( i = 1 ; i <= NumberOfSurfaces_ ; i++ ) {

          delete [] SurfaceNameList_[i];
          delete [] SurfaceGIDList_[i];
          
       }
       
       delete [] SurfaceNameList_;
       delete [] SurfaceGIDList_;              
       
       ComponentIDForSurface_         = Temp_ComponentIDForSurface;
       OpenVSP_ComponentIDForSurface_ = Temp_OpenVSP_ComponentIDForSurface;
       SurfaceNameList_               = Temp_SurfaceNameList;
       SurfaceGIDList_                = Temp_SurfaceGIDList;       
       
    }
       
    delete [] SurfaceList;
    
    // Determine the number of unique components

    printf("Determining number of unique components \n"); fflush(NULL);
        
    ComponentIDForComponent_ = new int[NumberOfSurfaces_ + 1];
    
    VSPComponentIDForVSPAEROComponent_ = new int[NumberOfSurfaces_ + 1];
    
    VSPAEROComponentIDForVSPComponent_ = new int[NumberOfSurfaces_ + 1];
    
    zero_int_array(ComponentIDForComponent_, NumberOfSurfaces_);

    zero_int_array(VSPComponentIDForVSPAEROComponent_, NumberOfSurfaces_);

    zero_int_array(VSPAEROComponentIDForVSPComponent_, NumberOfSurfaces_);
    
    NumberOfComponents_ = 0;
    
    for ( n = 1 ; n <= NumberOfSurfaces_ ; n++ ) {
       
       CompID = ComponentIDForSurface_[n];
     
       i = 1;
       
       Found = 0;
       
       while ( !Found && i <= NumberOfComponents_ ) {
          
          if ( ComponentIDForComponent_[i] == CompID ) Found = 1;
          
          i++;
          
       }
       
       if ( !Found ) {
          
          NumberOfComponents_++;
          
          ComponentIDForComponent_[NumberOfComponents_] = CompID;
          
       }
       
    }

    printf("Found %d unique components \n",NumberOfComponents_);
    
    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {
       
       printf("ComponentIDForComponent_[%d]: %d \n",i,ComponentIDForComponent_[i]);
       
    }

    // Update the surface to component ID list
    
    for ( n = 1 ; n <= NumberOfSurfaces_ ; n++ ) {
    
       Done = 0;
       
       i = 1;
       
       while ( i <= NumberOfComponents_ && !Done ) {
          
          if ( ComponentIDForSurface_[n] == ComponentIDForComponent_[i] ) {
             
             ComponentIDForSurface_[n] = i;
             
             printf("ComponentIDForSurface_[%d]: %d \n",n,ComponentIDForSurface_[n]);
             
             Done = 1;
             
          }
          
          i++;
          
       }
           
    }
       
    // Renumber component ID for loops
    
    for ( n = 1 ; n <= Grid().NumberOfLoops() ; n++ ) {
       
       Done = 0;
       
       i = 1;
       
       while ( i <= NumberOfComponents_ && !Done ) {
          
          if ( Grid().LoopList(n).ComponentID() == ComponentIDForComponent_[i] ) {
             
             Grid().LoopList(n).ComponentID() = i;
             
             Done = 1;
             
          }
          
          i++;
          
       }
       
       if ( !Done ) {
          
          printf("WTF! Could not reorder the component list! \n");fflush(NULL);exit(1);
          
       }
       
    }
    
    for ( n = 1 ; n <= NumberOfComponents_ ; n++ ) {

       printf("ComponentIDForComponent_[%d]: %d \n",
       n,
       ComponentIDForComponent_[n]);
       
    }
        
    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {
       
       VSPComponentIDForVSPAEROComponent_[i] = ComponentIDForComponent_[i];
       
       ComponentIDForComponent_[i] = i;
       
    }
    
    // Create inverse component ID

    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {
       
       VSPAEROComponentIDForVSPComponent_[VSPComponentIDForVSPAEROComponent_[i]] = i;
       
    }
      
    for ( n = 1 ; n <= NumberOfSurfaces_ ; n++ ) {

       printf("Surface: %d --> Component: %d --> SurfaceName: %s \n",
       n,
       ComponentIDForSurface_[n],
       SurfaceNameList_[n]);
       
    }

    for ( k = 1 ; k <= NumberOfSurfaces_ ; k++ ) {
    
       printf("SurfaceIsThick_[%d]: %d \n",k,SurfaceIsThick_[k]);
       
    }
                  
    GeometryComponentIsFixed_ = new int[NumberOfComponents_ + 1];

    GeometryGroupID_ = new int[NumberOfComponents_ + 1];
          
    // Mark components as either thick or thin
    
    zero_int_array(ComponentIsThick_,NumberOfComponents_);
    
    for ( k = 1 ; k <= NumberOfSurfaces_ ; k++ ) {
    
       if ( SurfaceIsThick_[k] ) {
          
          CompID = ComponentIDForSurface_[k];
          
          i = 1;
          
          Done = 0;
          
          while ( i <= NumberOfComponents_ && !Done ) {
             
             if ( CompID == ComponentIDForComponent_[i] ) {
                
                ComponentIsThick_[i] = 1;
                
                Done = 1;
                
             }
             
             i++;
             
          }
          
          if ( !Done ) {
             
             printf("Error in marking components as thick or thin... \n");fflush(NULL);
             exit(1);
             
          }
          
       }
                          
    }
    
    NumberOfThinComponents_ = NumberOfThickComponents_ = 0;
    
    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {
       
       if ( !ComponentIsThick_[i] ) {
          
          NumberOfThinComponents_++;
          
          printf("Component: %d with ID: %d is thin \n",i,ComponentIDForComponent_[i]);
          
       }
                             
       if (  ComponentIsThick_[i] ) {
          
          NumberOfThickComponents_++;
          
          printf("Component: %d with ID: %d is thick \n",i,ComponentIDForComponent_[i]);
          
       }

    }
    
   
 //    fflush(NULL);exit(1);
   
    // If the mesh we just read in is a n-gon mesh, we also need to read in the
    // triangulated version
    
    if ( InputMeshIsMixedPolys_ ) {

       fgets(DumChar,MAX_CHAR_SIZE,VSPGeom_File);
           
       fgetpos(VSPGeom_File,&TopOfTriangluation);
       
       // Loop over list to determine the total number of tris
       
       NumTris = 0;
       
       for ( n = 1 ; n <= NumLoops ; n++ ) {
          
          fgets(DumChar,MAX_CHAR_SIZE,VSPGeom_File);
      
          Next = strtok(DumChar,Space);
          Next = strtok(NULL,Space); NumTris += atoi(Next);
          
       }
       
       printf("Pure tri mesh has %d tris \n",NumTris);
   
       fsetpos(VSPGeom_File,&TopOfTriangluation);
       
       GridF_ = new VSP_GRID;    
       
       GridF_->SizeNodeList(NumNodes);
           
       // Copy over the nodal data
       
       for ( n = 1 ; n <= NumNodes ; n++ ) {
      
          GridF_->NodeList(n).x() = Grid().NodeList(n).x();
          GridF_->NodeList(n).y() = Grid().NodeList(n).y();
          GridF_->NodeList(n).z() = Grid().NodeList(n).z();
          
          GridF_->NodeList(n).IsTrailingEdgeNode()   = Grid().NodeList(n).IsTrailingEdgeNode();
          GridF_->NodeList(n).IsBoundaryEdgeNode()   = Grid().NodeList(n).IsBoundaryEdgeNode();
          GridF_->NodeList(n).IsBoundaryCornerNode() = Grid().NodeList(n).IsBoundaryCornerNode();
          GridF_->NodeList(n).IsSymmetryPlaneNode()  = Grid().NodeList(n).IsSymmetryPlaneNode();
  
       }
   
       GridF_->SizeTriList(NumTris);
       
       // Read in the tri connectivity
       
       NumTris = 0;
       
       for ( n = 1 ; n <= NumLoops ; n++ ) {
          
          fgets(DumChar,MAX_CHAR_SIZE,VSPGeom_File);
   
          Next = strtok(DumChar,Space); DumInt = atoi(Next);
          Next = strtok(NULL,Space);    DumInt = atoi(Next);
          
          Grid().LoopList(n).SizeFineGridLoopList(DumInt);

          for ( i = 1 ; i <= DumInt ; i++ ) {
             
             NumTris++;
             
             // Fine to coarse loop list
             
             Grid().LoopList(n).FineGridLoop(i) = NumTris;
             
             Next = strtok(NULL,Space); Node1 = atoi(Next);
             Next = strtok(NULL,Space); Node2 = atoi(Next);
             Next = strtok(NULL,Space); Node3 = atoi(Next);
             
             // Tri connectivity
             
             GridF_->LoopList(NumTris).Node1() = Node1;
             GridF_->LoopList(NumTris).Node2() = Node2;
             GridF_->LoopList(NumTris).Node3() = Node3;

             
             // Copy over surface flags from ngon mesh
             
             GridF_->LoopList(NumTris).CoarseGridLoop() = n;
             
             GridF_->LoopList(NumTris).SurfaceID()           = Grid().LoopList(n).SurfaceID();
                                                             
             GridF_->LoopList(NumTris).ComponentID()         = Grid().LoopList(n).ComponentID();
                                                           
             GridF_->LoopList(NumTris).SurfaceType()         = Grid().LoopList(n).SurfaceType();
             
             GridF_->LoopList(NumTris).OpenVSP_ComponentID() = Grid().LoopList(n).OpenVSP_ComponentID();
             
             GridF_->LoopList(NumTris).VortexSheet()         = Grid().LoopList(n).VortexSheet();
                                                             
             GridF_->LoopList(NumTris).SpanStation()         = Grid().LoopList(n).SpanStation();
                                                             
             GridF_->LoopList(NumTris).IsTrailingEdgeTri()   = Grid().LoopList(n).IsTrailingEdgeTri();
         
             // Build up UV data
      
             for ( j = 1 ; j <= 3 ; j++ ) {
                
                Node = GridF_->LoopList(NumTris).Node(j);
                
                Found = 0;
                
                k = 1;
                
                while ( k <= Grid().LoopList(n).NumberOfNodes() && !Found ) {
                   
                   if ( Node == Grid().LoopList(n).Node(k) ) {
                      
                      GridF_->LoopList(NumTris).U_Node(j) = Grid().LoopList(n).U_Node(k);
                      GridF_->LoopList(NumTris).V_Node(j) = Grid().LoopList(n).V_Node(k);
                      
                      Found = 1;
                
                   }
                   
                   k++;
                   
                }
                
                if ( !Found ) {
                   
                   printf("Error in determining UV data for triangulated version of NGON mesh! \n");
                   fflush(NULL);exit(1);
                   
                }
                
             }
             
             GridF_->LoopList(NumTris).Uc() = 0.;
             GridF_->LoopList(NumTris).Vc() = 0.;
             
             for ( j = 1 ; j <= 3 ; j++ ) {
                
                GridF_->LoopList(NumTris).Uc() += GridF_->LoopList(NumTris).U_Node(j);
                GridF_->LoopList(NumTris).Vc() += GridF_->LoopList(NumTris).V_Node(j);
                
             }
                
             GridF_->LoopList(NumTris).Uc() /= 3;
             GridF_->LoopList(NumTris).Vc() /= 3;       
                        
          }

       }

       printf("After building ...pure tri mesh has %d tris \n",NumTris);
       
       // Swap the grids

       GridC_ = Grid_[0];
       
       Grid_[0] = GridF_;
                  
    }


//    for ( n = 1 ; n <= Grid().NumberOfLoops() ; n++ ) {
//    
//       if ( Grid().LoopList(n).SurfaceID() == 4 || Grid().LoopList(n).SurfaceID() == 5 ) {
//          
//          int node;
//          
//          for ( i = 1 ; i <= Grid().LoopList(n).NumberOfNodes() ; i++ ) {
//             
//             node = Grid().LoopList(n).Node(i);
//             
//             
//             
//             printf("Fine grid: Surface: %d --> Loop: %d --> xyz: %f %f %f \n",
//             Grid().LoopList(n).SurfaceID(),
//             n,
//             Grid().NodeList(node).x(),
//             Grid().NodeList(node).y(),
//             Grid().NodeList(node).z());fflush(NULL);
//             
//          }
//          
//       }
//       
//    }


//    for ( n = 1 ; n <= GridC_->NumberOfLoops() ; n++ ) {
//    
//       if ( GridC_->LoopList(n).SurfaceID() == 4 || GridC_->LoopList(n).SurfaceID() == 5 ) {
//          
//          int node;
//          
//          for ( i = 1 ; i <= GridC_->LoopList(n).NumberOfNodes() ; i++ ) {
//             
//             node = GridC_->LoopList(n).Node(i);
//             
//             
//             printf("CoarseGrid: Surface: %d --> Loop: %d --> xyz: %f %f %f \n",
//             GridC_->LoopList(n).SurfaceID(),
//             n,
//             GridC_->NodeList(node).x(),
//             GridC_->NodeList(node).y(),
//             GridC_->NodeList(node).z());fflush(NULL);
//             
//          }
//          
//       }
//       
//    }   
    
}

/*##############################################################################
#                                                                              #
#                       VSP_GEOM RotateGeometry_About_Y_Axis                   #
#                                                                              #
##############################################################################*/

void VSP_GEOM::RotateGeometry_About_Y_Axis(void)  
{

    int i, j;
    double x, y, z, RotAngle, DeltaHeight, Xcg, Zcg; 
       
    // Rotate geometry about Y axis

    Xcg = VehicleRotationAxisLocation(0);
    Zcg = VehicleRotationAxisLocation(2);
    
    RotAngle = VehicleRotationAngleVector(1) * TORAD;
       
    DeltaHeight = HeightAboveGround() - Zcg;

    for ( i = 1 ; i <= Grid().NumberOfNodes() ; i++ ) {
       
       x = Grid().NodeList(i).x();
       y = Grid().NodeList(i).y();
       z = Grid().NodeList(i).z();
    
       Grid().NodeList(i).x() = (x - Xcg)*cos(RotAngle) - (z - Zcg)*sin(RotAngle) + Xcg;
       Grid().NodeList(i).y() = y;
       Grid().NodeList(i).z() = (x - Xcg)*sin(RotAngle) + (z - Zcg)*cos(RotAngle) + Zcg + DeltaHeight;    
    
    }    

}

/*##############################################################################
#                                                                              #
#                          VSP_GEOM FindSharpEdges                             #
#                                                                              #
##############################################################################*/

void VSP_GEOM::FindSharpEdges(int NumberOfSharpNodes, int *SharpNodeList)
{

    int i, j, k, p, pold, Loop1, Loop2, Node, Node1, Node2, Node3, Done;
    int NumberOfKuttaEdges, NumberOfKuttaNodes, Hits, Tri;
    int Edge, Edge1, Edge2, Next, VortexSheet, SheetIsPeriodic;
    int *KuttaEdgeList, *IsKuttaEdge, *PermArray, *NodeUsed;
    int *NodeIsSharp, CurrentComponentID, ListSize;
    double vec1[3], vec2[3], vec3[3], Xvec[3], dot, angle, mag1, mag2;
    KUTTA_NODE *KuttaNodeList;

    // Create a node to tri list
   
    NumberOfTrisForNode_ = new int[Grid().NumberOfNodes() + 1];
    
    zero_int_array(NumberOfTrisForNode_, Grid().NumberOfNodes());
    
    for ( i = 1 ; i <= Grid().NumberOfTris() ; i++ ) {
       
       for ( j = 1 ; j <= Grid().LoopList(i).NumberOfNodes() ; j++ ) {
          
          Node = Grid().LoopList(i).Node(j);
          
          NumberOfTrisForNode_[Node]++;
          
       }
       
    }
  
    NodeToTriList_ = new int*[Grid().NumberOfNodes() + 1];
    
    for ( i = 1 ; i <= Grid().NumberOfNodes() ; i++ ) {
       
       NodeToTriList_[i] = new int[NumberOfTrisForNode_[i] + 1];
       
    }

    zero_int_array(NumberOfTrisForNode_, Grid().NumberOfNodes());
    
    for ( i = 1 ; i <= Grid().NumberOfTris() ; i++ ) {
       
       for ( j = 1 ; j <= Grid().LoopList(i).NumberOfNodes() ; j++ ) {
          
          Node = Grid().LoopList(i).Node(j);
          
          NumberOfTrisForNode_[Node]++;
          
          NodeToTriList_[Node][NumberOfTrisForNode_[Node]] = i;
          
       }
       
    }    
    
    NodeIsSharp = new int[Grid().NumberOfNodes() + 1];
    
    zero_int_array(NodeIsSharp, Grid().NumberOfNodes());
    
    if ( NumberOfSharpNodes ) {

       for ( i = 1 ; i <= NumberOfSharpNodes ; i++ ) {

          NodeIsSharp[SharpNodeList[i]] = 1;
          
       }
       
    }
    
    // Find all possible wake edges
    
    IsKuttaEdge = new int[Grid().NumberOfEdges() + 1];
    
    zero_int_array(IsKuttaEdge, Grid().NumberOfEdges());
        
    IncidentKuttaEdgesOnNode_ = new int[Grid().NumberOfNodes() + 1];
    
    zero_int_array(IncidentKuttaEdgesOnNode_, Grid().NumberOfNodes());
    
    Xvec[0] = 1.;
    Xvec[1] = 0.;
    Xvec[2] = 0.;
    
    for ( i = 1 ; i <= Grid().NumberOfEdges() ; i++ ) {
       
       Node1 = Grid().EdgeList(i).Node1();
       Node2 = Grid().EdgeList(i).Node2();
       
       // Geometry provided list of sharp nodes... use it!
       
       if ( NumberOfSharpNodes ) {
          
          if ( NodeIsSharp[Node1] && NodeIsSharp[Node2] ) {

//printf("Edge: %d --> Component: %d --> Surface: %d --> Tri1,2: %d %d ---> ComponentID_1,2: %d %d ---> Node1,2: %d %d out of a total of %d nodes \n",
//i,
//Grid().EdgeList(i).ComponentID(),
//Grid().EdgeList(i).SurfaceID(),
//Grid().EdgeList(i).Loop1(),
//Grid().EdgeList(i).Loop2(),
//Grid().LoopList(Grid().EdgeList(i).Loop1()).ComponentID(),
//Grid().LoopList(Grid().EdgeList(i).Loop2()).ComponentID(),
//
//Node1,Node2,Grid().NumberOfNodes());

             IncidentKuttaEdgesOnNode_[Node1]++;
             
             IncidentKuttaEdgesOnNode_[Node2]++;
             
             IsKuttaEdge[i] = 1;     
             
         //    printf("Found kutta edge! \n");fflush(NULL);
              
          } 
          
       }       
       
       // Check if edge is mostly normal to the free stream flow
       
       else if ( SurfaceType_ != VSPGEOM_SURFACE )  {
          
          vec1[0] = Grid().NodeList(Node2).x() - Grid().NodeList(Node1).x();
          vec1[1] = Grid().NodeList(Node2).y() - Grid().NodeList(Node1).y();
          vec1[2] = Grid().NodeList(Node2).z() - Grid().NodeList(Node1).z();
          
          dot = sqrt(vector_dot(vec1,vec1));
          
          vec1[0] /= dot;
          vec1[1] /= dot;
          vec1[2] /= dot;
          
          dot = ABS(vector_dot(Xvec, vec1));          
                
          if ( dot <= 0.98 ) {
   
             Loop1 = Grid().EdgeList(i).Loop1();
             Loop2 = Grid().EdgeList(i).Loop2();
      
             // Form dot product of opposing triangles
      
             vec1[0] = Grid().LoopList(Loop1).Nx();
             vec1[1] = Grid().LoopList(Loop1).Ny();
             vec1[2] = Grid().LoopList(Loop1).Nz();
      
             vec2[0] = Grid().LoopList(Loop2).Nx();
             vec2[1] = Grid().LoopList(Loop2).Ny();
             vec2[2] = Grid().LoopList(Loop2).Nz();
      
             mag1 = sqrt(vector_dot(vec1,vec1));
             mag2 = sqrt(vector_dot(vec2,vec2));
      
             dot = vector_dot(vec1,vec2);
      
             angle = (180./3.141592)*acos(MAX(-1.,MIN(dot,1.)));
      
             // If angle greater than some tolerance... , mark nodes as sharp
      
             if ( angle > 65. ) {
      
               Node1 = Grid().EdgeList(i).Node1();
               Node2 = Grid().EdgeList(i).Node2();         
               Node3 = Grid().LoopList(Loop2).Node1() + Grid().LoopList(Loop2).Node2() + Grid().LoopList(Loop2).Node3() - Node1 - Node2;
            
               vec3[0] = Grid().NodeList(Node3).x() - Grid().NodeList(Node1).x();
               vec3[1] = Grid().NodeList(Node3).y() - Grid().NodeList(Node1).y();
               vec3[2] = Grid().NodeList(Node3).z() - Grid().NodeList(Node1).z();
               
               dot = vector_dot(vec3,vec1);
      
               // Check if volume is negative... otherwise it's not a sharp TE, but a cove of some sort
                
               if ( dot < 0. ) {
                 
                  vec3[0] = vec1[0] + vec2[0];
                  vec3[1] = vec1[1] + vec2[1];
                  vec3[2] = vec1[2] + vec2[2];
         
                  dot = sqrt(vector_dot(vec3,vec3));
         
                  vec3[0] /= dot;
                  vec3[1] /= dot;
                  vec3[2] /= dot;
         
                  // Check if the normal points aft
         
                  if ( vec3[0] > 0. ) { 
                     
                     IncidentKuttaEdgesOnNode_[Node1]++;
                     
                     IncidentKuttaEdgesOnNode_[Node2]++;
               
                     IsKuttaEdge[i] = 1;
   
                   }
                   
                }
         
             }
             
          }
          
       }

    }
    
    delete [] NodeIsSharp;

    // Find and mark the trailing edges

    KuttaEdgeList = new int[Grid().NumberOfEdges() + 1];
    
    zero_int_array(KuttaEdgeList, Grid().NumberOfEdges());
    
    KuttaNodeList = new KUTTA_NODE[Grid().NumberOfNodes() + 1];

    for ( i = 1 ; i <= Grid().NumberOfNodes() ; i++ ) {
       
       KuttaNodeList[i].IsKuttaNode  = 0;
       
       KuttaNodeList[i].Node  = 0;
       KuttaNodeList[i].Edge1 = 0;
       KuttaNodeList[i].Edge2 = 0;
       
    }
        
    NumberOfKuttaEdges = 0;

    for ( i = 1 ; i <= Grid().NumberOfEdges() ; i++ ) {

       if ( IsKuttaEdge[i] ) {
          
          Node1 = Grid().EdgeList(i).Node1();
          Node2 = Grid().EdgeList(i).Node2();

         // Check Node 1
         
         KuttaNodeList[Node1].IsKuttaNode = 1;
         
         if ( KuttaNodeList[Node1].Edge1 == 0 ) {
            
            KuttaNodeList[Node1].Edge1 = i;
            
         }
         
         else {
           
            KuttaNodeList[Node1].Edge2 = i;
            
         }
            
         // Check Node 2
         
         KuttaNodeList[Node2].IsKuttaNode = 1;
         
         if ( KuttaNodeList[Node2].Edge1 == 0 ) {
            
            KuttaNodeList[Node2].Edge1 = i;
            
         }
         
         else {
           
            KuttaNodeList[Node2].Edge2 = i;
            
         }
         
         KuttaEdgeList[++NumberOfKuttaEdges] = i;
         
       }
       
    }

    // Determine the number of kutta nodes
    
    NumberOfKuttaNodes = 0;
    
    for ( i = 1 ; i <= Grid().NumberOfNodes() ; i++ ) {
       
       NumberOfKuttaNodes += KuttaNodeList[i].IsKuttaNode;
       
    }       
    
    printf("NumberOfKuttaNodes after trimming: %d \n",NumberOfKuttaNodes);

    // Size the kutta node list

    Grid().SizeKuttaNodeList(NumberOfKuttaNodes);  
    
    // Mark the kutta edges
    
    for ( i = 1 ; i <= NumberOfKuttaEdges ; i++ ) {
       
       Node1 = Grid().EdgeList(KuttaEdgeList[i]).Node1();
       Node2 = Grid().EdgeList(KuttaEdgeList[i]).Node2();
       
       if ( KuttaNodeList[Node1].IsKuttaNode && KuttaNodeList[Node2].IsKuttaNode ) {
                    
          Grid().EdgeList(KuttaEdgeList[i]).IsTrailingEdge() = 1;
          
          if ( Verbose_ ) printf("Marking %d kutta edge %d as a trailing edge... \n",i,KuttaEdgeList[i]);
          
       }
      
    }
    
    // Iterate until all the kutta nodes are assigned to a vortex sheet
    
    NodeUsed = new int[Grid().NumberOfNodes() + 1];

    zero_int_array(NodeUsed, Grid().NumberOfNodes());
    
    PermArray = new int[NumberOfKuttaNodes + 1];
  
    p = pold = 0;
    
    VortexSheet = 1;
    
    printf("NumberOfKuttaNodes: %d \n",NumberOfKuttaNodes);
    printf("NumberOfKuttaEdges: %d \n",NumberOfKuttaEdges);
    
    while ( p < NumberOfKuttaNodes ) {
       
       printf("Working on VortexSheet: %d \n",VortexSheet);fflush(NULL);
       
       SheetIsPeriodic = 0;
       
       // Start with a node on the end of the list... tip node, possibly root node if there's a body
       
       Done = 0;
       
       k = 1;
   
       while ( k <= Grid().NumberOfNodes() && !Done ) {
       
          if ( KuttaNodeList[k].IsKuttaNode && IncidentKuttaEdgesOnNode_[k] == 1 && NodeUsed[k] == 0 ) {
             
             printf("KuttaNodeList[k].IsKuttaNode: %d \n",KuttaNodeList[k].IsKuttaNode);
             printf("IncidentKuttaEdgesOnNode_[k]: %d \n",IncidentKuttaEdgesOnNode_[k]);fflush(NULL);
             
             Done = 1;
          
          }
          
          k++;
          
       }
       
       k--;
       
       if ( Done ) printf("Found a starting wake sheet node... %d \n",k);fflush(NULL);

       // If we did not find a wing tip node... then this wake is either closed like a nacelle,
       // or it could be something like a pylon with no tips... so now we have to figure that out
       
       // Find any ones >= 3 
       
       // Find a kutta edge that only has one node marked as a kutta node
       
       if ( !Done ) {
          
          printf("Did not find a singly marked wake sheet node... \n");fflush(NULL);
          
          i = 1;
          
          while ( i <= NumberOfKuttaEdges && !Done ) {
             
             Edge = KuttaEdgeList[i];
  
             Node1 = Grid().EdgeList(Edge).Node1();
             Node2 = Grid().EdgeList(Edge).Node2();
             
             if ( KuttaNodeList[Node1].IsKuttaNode + KuttaNodeList[Node2].IsKuttaNode == 1 ) {
                
                if ( KuttaNodeList[Node1].IsKuttaNode && NodeUsed[Node1] == 0 ) { k = Node1; Done = 1; };

                if ( KuttaNodeList[Node2].IsKuttaNode && NodeUsed[Node2] == 0 ) { k = Node2; Done = 1; };

             }
    
             i++;                
             
          }
          
          if ( Done ) printf("Found node: %d \n",k);
          
       }
               
       // Find any kutta node... must be a loop now
              
       SheetIsPeriodic = 0;
   
       if ( !Done ) {
          
          printf("Did not find any singly marked kutta node... \n");fflush(NULL);
          
          k = 1;
   
          while ( k <= Grid().NumberOfNodes() && !Done ) {
       
             if ( KuttaNodeList[k].IsKuttaNode && NodeUsed[k] == 0 ) Done = 1;
             
             k++;
             
          }
          
          k--;
          
          SheetIsPeriodic = 1;
          
          if ( SheetIsPeriodic ) printf("Sheet is periodic... \n");fflush(NULL);
          
       }
 
       // Loop over kutta nodes and sort
       
       zero_int_array(PermArray, NumberOfKuttaNodes);
   
       Done = 0;
       
       ListSize = Next = 1;
       
       Node = k;
 
       Edge1 = KuttaNodeList[Node].Edge1;
       
       Edge2 = KuttaNodeList[Node].Edge2;
                 
       if ( Edge1 > 0 ) CurrentComponentID = Grid().EdgeList(Edge1).ComponentID();
       
       if ( Edge2 > 0 ) CurrentComponentID = Grid().EdgeList(Edge2).ComponentID();
       
       printf("Working on CurrentComponentID: %d \n",CurrentComponentID);fflush(NULL);     
         
    //   printf("Starting with node %d ... which is identified as being a kutta node \n",Node);
       
       PermArray[Next] = Node;
       
       NodeUsed[Node] = 1;
    
       while ( !Done ) {
          
          Done = 1;
          
          Node = PermArray[Next];
          
       //   printf("Current Node: %d \n",Node);
         
          Edge1 = KuttaNodeList[Node].Edge1;
          Edge2 = KuttaNodeList[Node].Edge2;
                          
          Node1 = Grid().EdgeList(Edge1).Node1() + Grid().EdgeList(Edge1).Node2() - Node;
          Node2 = Grid().EdgeList(Edge2).Node1() + Grid().EdgeList(Edge2).Node2() - Node;

          // Add node 1
   
          if ( Edge1 > 0 && KuttaNodeList[Node1].IsKuttaNode && NodeUsed[Node1] == 0 && Grid().EdgeList(Edge1).ComponentID() == CurrentComponentID ) {
             
             PermArray[++Next] = Node1;
    
             NodeUsed[Node1] = 1;
             
             Done = 0;
             
          }
          
          // Add node 2
          
          else if ( Edge2 > 0 && KuttaNodeList[Node2].IsKuttaNode && NodeUsed[Node2] == 0 && Grid().EdgeList(Edge2).ComponentID() == CurrentComponentID ) {

             PermArray[++Next] = Node2;
 
             NodeUsed[Node2] = 1;
             
             Done = 0;
                      
          }
   
       }
     
       for ( i = 1 ; i <= NumberOfKuttaNodes ; i++ ) {
                    
          Node = PermArray[i];
      
          if ( Node > 0 ) {

             if ( !SurfaceAtNodeIsConvex(Node) && IncidentKuttaEdgesOnNode_[Node] == 1 ) printf("Kutta node: %d... ie Node: %d is not convex! \n",i,Node);fflush(NULL);

             p++;
             
             Grid().KuttaNode(p) = Node;
             
             Grid().KuttaNodeSoverB(p) = 0.;

             Grid().WingSurfaceForKuttaNode(p) = VortexSheet;
          
             Grid().WingSurfaceForKuttaNodeIsPeriodic(p) = SheetIsPeriodic;
             
             Grid().ComponentIDForKuttaNode(p) = Grid().NodeList(Node).ComponentID();
            
             Grid().WakeTrailingEdgeX(p) = Grid().NodeList(Node).x();
             Grid().WakeTrailingEdgeY(p) = Grid().NodeList(Node).y();
             Grid().WakeTrailingEdgeZ(p) = Grid().NodeList(Node).z();
                   
             Grid().NodeList(Node).IsTrailingEdgeNode() = 1;
             
          }
   
       }   

       printf("VortexSheet: %d finished... which has %d kutta nodes ... and %d out of %d kutta nodes are now allocated \n",VortexSheet,p-pold,p,NumberOfKuttaNodes);fflush(NULL);
       
       pold = p;
       
       VortexSheet++;
                     
    }
    
    // Now find any sharp boundary edges
        
    for ( i = 1 ; i <= Grid().NumberOfEdges() ; i++ ) {

       Loop1 = Grid().EdgeList(i).Loop1();
       Loop2 = Grid().EdgeList(i).Loop2();

       // Form dot product of opposing triangles

       vec1[0] = Grid().LoopList(Loop1).Nx();
       vec1[1] = Grid().LoopList(Loop1).Ny();
       vec1[2] = Grid().LoopList(Loop1).Nz();

       vec2[0] = Grid().LoopList(Loop2).Nx();
       vec2[1] = Grid().LoopList(Loop2).Ny();
       vec2[2] = Grid().LoopList(Loop2).Nz();

       dot = vector_dot(vec1,vec2);

       angle = (180./3.141592)*acos(MAX(-1.,MIN(dot,1.)));

       if ( SurfaceType_ == CART3D_SURFACE && angle >= 45. && !Grid().EdgeList(i).IsTrailingEdge() ) {

          Grid().EdgeList(i).IsBoundaryEdge() = 1;
 
       }

    }    
    
    // Mark edges that are on an open boundary. I am
    // assuming these are on a symmetry plane
    
    for ( i = 1 ; i <= Grid().NumberOfEdges() ; i++ ) {

       Loop1 = Grid().EdgeList(i).Loop1();
       Loop2 = Grid().EdgeList(i).Loop2();

       if ( Loop1 == Loop2 ) {

          Grid().EdgeList(i).IsBoundaryEdge() = 1;
 
       }

    }   
    
    // Search for wing tip kutta nodes
    
    for ( i = 1 ; i <= NumberOfKuttaEdges ; i++ ) {
       
       Node1 = Grid().EdgeList(KuttaEdgeList[i]).Node1();
       Node2 = Grid().EdgeList(KuttaEdgeList[i]).Node2();
    
       Node = 0;
       
       if ( IncidentKuttaEdgesOnNode_[Node1] == 1 ) Node = Node1;
       if ( IncidentKuttaEdgesOnNode_[Node2] == 1 ) Node = Node2;
      
       if ( Node > 0 && SurfaceAtNodeIsConvex(Node) ) {

         vec1[0] = Grid().NodeList(Node2).x() - Grid().NodeList(Node1).x();
         vec1[1] = Grid().NodeList(Node2).y() - Grid().NodeList(Node1).y();
         vec1[2] = Grid().NodeList(Node2).z() - Grid().NodeList(Node1).z();
         
         mag1 = sqrt(vector_dot(vec1,vec1));
         
         vec1[0] /= mag1;
         vec1[1] /= mag1;
         vec1[2] /= mag1;
 
         // Calculate angle between all neighbor tris and trailing edge vector
         
         j = 1;
         
         Hits = 0;
         
         Done = 0;
         
         while ( j <= NumberOfTrisForNode_[Node] && !Done ) {
            
            Tri = NodeToTriList_[Node][j];
            
            k = 1;
 
            while ( k <= Grid().LoopList(Tri).NumberOfEdges() && !Done ) {
               
               if ( Grid().EdgeList(Grid().LoopList(Tri).Edge(k)).Loop1() == Grid().EdgeList(Grid().LoopList(Tri).Edge(k)).Loop2() ) {
                  
                  //printf("Kutta node is on symmetry plane! \n"); fflush(NULL);
                  
                  Done = 1;
               
               }
               
               k++;
               
            }     
            
            if ( !Done ) {
               
               // Angle current loop normal and edge vector
                                 
               mag1 = vector_dot(Grid().LoopList(Tri).Normal(), vec1);
               
               mag1 = MAX(-1.,MIN(1.,mag1));
               
               angle = acos(mag1);
               
               if ( angle <= 45.*TORAD || angle >= 135.*TORAD ) {
      
                  Hits++;
                  
               }
               
            }
            
            j++;
            
         }
         
         if ( Hits > 0 ) {
            
            if ( Grid().NodeList(Node).IsTrailingEdgeNode() ) {
            
               Grid().NodeList(Node).IsTrailingEdgeNode() = -1;
               
            }
            
         }

       }
       
    }

    for ( i = 1 ; i <= NumberOfKuttaNodes ; i++ ) {
             
       Node = Grid().KuttaNode(i);
       
       if ( Grid().NodeList(Node).IsTrailingEdgeNode() == -1 ) {
         
          Grid().NodeList(Node).IsTrailingEdgeNode() = 1;
          
          Grid().KuttaNodeIsOnWingTip(i) = 1;
          
       }
    
    }
    
    // Finally calculate U value for each kutta node
    
    double *Uval;
    
    Uval = new double[Grid().NumberOfNodes() + 1];
    
    for ( i = 1 ; i <= Grid().NumberOfLoops() ; i++ ) {
       
       for ( j = 1 ; j <= Grid().LoopList(i).NumberOfNodes() ; j++ ) {
          
          Uval[Grid().LoopList(i).Node(j)] = Grid().LoopList(i).U_Node(j);
          
       }
       
    }
    
    for ( i = 1 ; i <= NumberOfKuttaNodes ; i++ ) {
             
       Node = Grid().KuttaNode(i);
        
       Grid().KuttaNodeSoverB(i) = Uval[Node] - 1.;
    
    }
    
    delete [] Uval;

    delete [] KuttaNodeList;
    delete [] KuttaEdgeList;
    
    delete [] IsKuttaEdge;    
    
}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM SurfaceAtNodeIsConvex                       #
#                                                                              #
##############################################################################*/

int VSP_GEOM::SurfaceAtNodeIsConvex(int Node)
{

    int i, j, k, Tri;
    double Vec[3], VecAvg[3], P1[3], P2[3], Dot, Angle;
   
    if ( NumberOfTrisForNode_[Node] == 2 ) return 1;
    
    // Calculate averaged normal at Node.. I removed the area weighting of normals
    // that used to be here... small tris on a trailing edge of the wing and the
    // large tris on default bodies meant the averaged normal was not really
    // very representative of what we wanted to test against...
    
    VecAvg[0] = VecAvg[1] = VecAvg[2] = 0.;
    
    for ( i = 1 ; i <= NumberOfTrisForNode_[Node] ; i++ ) {
    
       Tri = NodeToTriList_[Node][i];
       
       VecAvg[0] += Grid().LoopList(Tri).Normal()[0];
       VecAvg[1] += Grid().LoopList(Tri).Normal()[1];
       VecAvg[2] += Grid().LoopList(Tri).Normal()[2];
       
    }
    
    Dot = sqrt(vector_dot(VecAvg,VecAvg));
    
    VecAvg[0] /= Dot;
    VecAvg[1] /= Dot;
    VecAvg[2] /= Dot;
    
    // Check angle between averaged normal and surface tangents to centroids
    
    P1[0] = Grid().NodeList(Node).x();
    P1[1] = Grid().NodeList(Node).y();
    P1[2] = Grid().NodeList(Node).z();
    
    for ( i = 1 ; i <= NumberOfTrisForNode_[Node] ; i++ ) {
    
       Tri = NodeToTriList_[Node][i];
       
       P2[0] = P2[1] = P2[2] = 0.;
       
       for ( j = 1 ; j <= Grid().LoopList(Tri).NumberOfNodes() ; j++ ) {
          
          k = Grid().LoopList(Tri).Node(j);
          
          P2[0] += Grid().NodeList(k).x();
          P2[1] += Grid().NodeList(k).y();
          P2[2] += Grid().NodeList(k).z();
          
       }
       
       P2[0] /= Grid().LoopList(Tri).NumberOfNodes();
       P2[1] /= Grid().LoopList(Tri).NumberOfNodes();
       P2[2] /= Grid().LoopList(Tri).NumberOfNodes();
       
       Vec[0] = P2[0] - P1[0];
       Vec[1] = P2[1] - P1[1];
       Vec[2] = P2[2] - P1[2];      
       
       Dot = sqrt(vector_dot(Vec,Vec));
       
       Vec[0] /= Dot;
       Vec[1] /= Dot;
       Vec[2] /= Dot;
      
       // Calculate angle between surface tangent vector and averaged normal
       
       Dot = vector_dot(VecAvg,Vec);
       
       Angle = (180./3.141592)*acos(MAX(MIN(Dot,1.),-1.));

       if ( Angle <= 90. ) return 0; // Not convex

    }    
      
    return 1; // Convex
   
}

/*##############################################################################
#                                                                              #
#                        VSP_GEOM CreateVortexSheets                           #
#                                                                              #
##############################################################################*/

void VSP_GEOM::CreateVortexSheets(void)
{
 
    int i, j, k, jj, kk, l, m, p, q, NumNodes, Node1, Node2, Loop;
    int NodeA, NodeB, Found, Done;
    int NumberOfSheets, NumberOfKuttaNodes;
    double WakeAngle[3], *Sigma, Dist;
    double Scale_X, Scale_Y, Scale_Z, WakeDist;
    double Xmin, Xmax, Ymin, Ymax, Zmin, Zmax;
    VSP_NODE VSP_Node1, VSP_Node2;
    
    Node1 = 0;
    Node2 = 0;
    
    // Determine min/max of geometry
    
    Xmin = 1.e9;
    Xmax = -Xmin;
    
    Ymin = 1.e9;
    Ymax = -Ymin;
    
    Zmin = 1.e9;
    Zmax = -Zmin;        

    for ( j = 1 ; j <= Grid(SolveOnMGLevel_).NumberOfSurfaceNodes() ; j++ ) {
       
        Xmin = MIN(Xmin,Grid(SolveOnMGLevel_).NodeList(j).x());
        Xmax = MAX(Xmax,Grid(SolveOnMGLevel_).NodeList(j).x());
                             
        Ymin = MIN(Ymin,Grid(SolveOnMGLevel_).NodeList(j).y());
        Ymax = MAX(Ymax,Grid(SolveOnMGLevel_).NodeList(j).y());
                                        
        Zmin = MIN(Zmin,Grid(SolveOnMGLevel_).NodeList(j).z());
        Zmax = MAX(Zmax,Grid(SolveOnMGLevel_).NodeList(j).z());
                            
    }

    // Initial wake in the free stream direction

    WakeAngle[0] = 1.;
    WakeAngle[1] = 0.;
    WakeAngle[2] = 0.;
       
    printf("WakeAngle: %f %f %f \n",WakeAngle[0],WakeAngle[1],WakeAngle[2]);fflush(NULL);

    // Determine how far to allow wakes to adapt... beyond this the wakes go straight off to
    // 'infinity' in the free stream direction
    
    Scale_X = Scale_Y = Scale_Z = 1.;
    
    if ( DoSymmetryPlaneSolve_ ) Scale_Y = 2.;

    printf("Xmax_ - Xmin_: %f \n", double(Xmax - Xmin));
    printf("Ymax_ - Ymin_: %f \n", double(Ymax - Ymin));
    printf("Zmax_ - Zmin_: %f \n", double(Zmax - Zmin));
    printf("\n");

    printf("Wake FarFieldDist_ set to: %f \n",FarFieldDist_);
    printf("\n");

    // Determine the minimum trailing edge spacing for edge kutta node

    Sigma = new double[Grid(SolveOnMGLevel_).NumberOfNodes() + 1];
    
    for ( j = 1 ; j <= Grid(SolveOnMGLevel_).NumberOfNodes() ; j++ ) {

       Sigma[j] = 1.e9;

    }

    for ( i = 1 ; i <= Grid(SolveOnMGLevel_).NumberOfEdges() ; i++ ) {
     
       if ( Grid(SolveOnMGLevel_).EdgeList(i).IsTrailingEdge() ) {     

          Node1 = Grid(SolveOnMGLevel_).EdgeList(i).Node1();
          Node2 = Grid(SolveOnMGLevel_).EdgeList(i).Node2();
          
          Sigma[Node1] = MIN(Sigma[Node1], Grid(SolveOnMGLevel_).EdgeList(i).Length());
          Sigma[Node2] = MIN(Sigma[Node2], Grid(SolveOnMGLevel_).EdgeList(i).Length());
   
       }
       
    } 

    // Determine the number of vortex sheets
    
    NumberOfSheets = 0;
    
    for ( j = 1 ; j <= Grid(SolveOnMGLevel_).NumberOfKuttaNodes() ; j++ ) {

       NumberOfSheets = MAX(NumberOfSheets, Grid(SolveOnMGLevel_).WingSurfaceForKuttaNode(j));
       
    }    
   
    NumberOfVortexSheets_ = 0;
    
    for ( k = 1 ; k <= NumberOfSheets ; k++ ) {
       
       NumberOfKuttaNodes = 0;
       
       for ( j = 1 ; j <= Grid(SolveOnMGLevel_).NumberOfKuttaNodes() ; j++ ) {
          
          if ( Grid(SolveOnMGLevel_).WingSurfaceForKuttaNode(j) == k ) NumberOfKuttaNodes++;
          
       }
       
       if ( NumberOfKuttaNodes > 1 ) NumberOfVortexSheets_++;
       
    }
    
    printf("There are: %10d Vortex Sheets \n", NumberOfVortexSheets_);
    
    VortexSheet_ = new VORTEX_SHEET[NumberOfVortexSheets_ + 1];

    printf("Creating vortex sheet data... \n"); fflush(NULL);

    // Setup each vortex sheet
    
    i = 0;
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
       
       NumberOfKuttaNodes = 0;
       
       for ( j = 1 ; j <= Grid(SolveOnMGLevel_).NumberOfKuttaNodes() ; j++ ) {
          
          if ( Grid(SolveOnMGLevel_).WingSurfaceForKuttaNode(j) == k ) NumberOfKuttaNodes++;

          if ( Grid(SolveOnMGLevel_).WingSurfaceForKuttaNodeIsPeriodic(j) ) VortexSheet(k).IsPeriodic() = 1;
          
       }
       
       if ( NumberOfKuttaNodes > 1 ){
          
          i++;
          
          VortexSheet(i).SizeTrailingVortexList(NumberOfKuttaNodes);
          
          VortexSheet(i).WingSurface() = k;
          
          printf("There are: %d kutta nodes for sheet: %d \n",NumberOfKuttaNodes,k);
          
       }
       
       else {
          
          printf("Warning ... zero kutta nodes for sheet: %d \n",k);
          fflush(NULL);
          
       }
       
    }

    TimeAccurate_ = 0;
    
    TimeAnalysisType_ = 0; // not sure this is even used anymore...
    
    CoreSizeFactor_ = 1.;
    
    DoVortexStretching_ = 0.;
    
    OptimizationSolve_ = 0.;

    FlowIs2D_ = 0;
    
    Vinf_ = 1.;
    
    WakeRelax_ = 1.;
 
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       NumNodes = 0;
       
       VortexSheet(k).NumberOfWakeTrailingNodes() = NumberOfWakeTrailingNodes_;
     
       VortexSheet(k).TimeAccurate() = TimeAccurate_;
     
       VortexSheet(k).TimeAnalysisType() = TimeAnalysisType_;
  
       VortexSheet(k).Vinf() = SGN(Vinf_)*MAX(0.000001,ABS(Vinf_));
    
       VortexSheet(k).DeltaTime() = 0.;  

       VortexSheet(k).CoreSizeFactor() = CoreSizeFactor_;
       
       VortexSheet(k).DoVortexStretching() = DoVortexStretching_;
       
       VortexSheet(k).Is2D() = FlowIs2D_;
           
       VortexSheet(k).FreeStreamVelocity(0) = 1.;
       VortexSheet(k).FreeStreamVelocity(1) = 0.;
       VortexSheet(k).FreeStreamVelocity(2) = 0.;
     
       for ( j = 1 ; j <= Grid(SolveOnMGLevel_).NumberOfKuttaNodes() ; j++ ) {
          
          if ( Grid(SolveOnMGLevel_).WingSurfaceForKuttaNode(j) == VortexSheet(k).WingSurface() ) {
          
             NumNodes++;
             
             VortexSheet(k).TrailingVortex(NumNodes).TimeAccurate() = TimeAccurate_;
                             
             VortexSheet(k).TrailingVortex(NumNodes).TimeAnalysisType() = TimeAnalysisType_;

             VortexSheet(k).TrailingVortex(NumNodes).RotorAnalysis() = 0;
                                  
             VortexSheet(k).TrailingVortex(NumNodes).DoGroundEffectsAnalysis() = DoGroundEffectsAnalysis();
             
             VortexSheet(k).TrailingVortex(NumNodes).Vinf() = SGN(Vinf_)*MAX(0.000001,ABS(Vinf_));

             VortexSheet(k).TrailingVortex(NumNodes).BladeRPM() = 0.;
             
             VortexSheet(k).TrailingVortex(NumNodes).DeltaTime() = 0.;   
    
             // Pointer to the wing this trailing vortex leaves from
      
             VortexSheet(k).TrailingVortex(NumNodes).Wing() = k;
             
             // Vortex stretching model flag
            
             VortexSheet(k).TrailingVortex(NumNodes).DoVortexStretching() = DoVortexStretching_;
             
             // Flag if the vortex sheet is periodic (eg would be a nacelle)
             
             VortexSheet(k).IsPeriodic() = Grid(SolveOnMGLevel_).WingSurfaceForKuttaNodeIsPeriodic(j);

             // Pointer to the kutta node
             
             VortexSheet(k).TrailingVortex(NumNodes).TE_Node() = Grid(SolveOnMGLevel_).KuttaNode(j);
             
         //    printf("Grid(SolveOnMGLevel_).KuttaNode(%d): %d \n",j,Grid(SolveOnMGLevel_).KuttaNode(j));
             
             // Note if this node is located in a concave region
         
             if ( !SurfaceAtNodeIsConvex(Grid(SolveOnMGLevel_).KuttaNode(j)) && IncidentKuttaEdgesOnNode_[Grid(SolveOnMGLevel_).KuttaNode(j)] == 1 ) {

                printf("Marking kutta node %d on sheet %d as concave \n",NumNodes, k);fflush(NULL);

                VortexSheet(k).TrailingVortex(NumNodes).TE_Node_Region_Is_Concave() = 1;
             
             }

             // Note if this node is located on a thin-thick, or thin-thin intersection

             if ( Grid(SolveOnMGLevel_).NodeList(Grid(SolveOnMGLevel_).KuttaNode(j)).IsIntersectionNode() ) {

                printf("Marking kutta node %d on sheet %d as on thin/thick or thin/thin intersection \n",NumNodes, k);fflush(NULL);

                VortexSheet(k).TrailingVortex(NumNodes).TE_Node_Region_Is_Concave() = 1;
             
             }
                   
             // Location along span of this kutta node S over Span
             
             VortexSheet(k).TrailingVortex(NumNodes).SoverB() = Grid(SolveOnMGLevel_).KuttaNodeSoverB(j);
             
             // Component ID for the entire vortex sheet... this is a bit kludgey
             
             if ( NumNodes > 1 ) VortexSheet(k).ComponentID() = Grid(SolveOnMGLevel_).ComponentIDForKuttaNode(j);
             
             // Component ID for each trailing vortex
            
             VortexSheet(k).TrailingVortex(NumNodes).ComponentID() = Grid(SolveOnMGLevel_).ComponentIDForKuttaNode(j);

             // Set sigma

             VortexSheet(k).TrailingVortex(NumNodes).Sigma() = 0.25*Sigma[Grid(SolveOnMGLevel_).KuttaNode(j)];

             // Size the list

             VortexSheet(k).TrailingVortex(NumNodes).SizeList(NumberOfWakeTrailingNodes_);                   
              
          }
             
       }
   
       VortexSheet(k).SetupVortexSheets();

       if ( VortexSheet(k).IsPeriodic() ) {
          
          printf("There are: %10d kutta nodes for vortex sheet: %10d     <----- Periodic Wake \n",VortexSheet(k).NumberOfTrailingVortices(),k); fflush(NULL);
          
       }
       
       else {
          
          printf("There are: %10d kutta nodes for vortex sheet: %10d  \n",VortexSheet(k).NumberOfTrailingVortices(),k); fflush(NULL);
          
       }

    }

    delete [] Sigma;
 
    printf("Done creating vortex sheet data... \n");fflush(NULL);
    
    // Distribute vortex core widths
    
}

/*##############################################################################
#                                                                              #
#                          VSP_GEOM StoreWakeKuttaEdges                        #
#                                                                              #
##############################################################################*/

void VSP_GEOM::StoreWakeKuttaEdges(void)
{

    int i, j, jm, jp, k, p, q, SurfaceID;
    int TotalNodes, Node1, Node2, NodeA, NodeB, Loop, Loop1, Loop2, TotalLoops;
    int Node, Edge, Found, TE_Edge;
    int *NumberOfEdgesForNode, **NodeToEdgeList;
    double U1, U2, V1, V2, U3, U4, V3, V4, Chord, Length, Vec[2], Mag;
        
    if ( SolveOnMGLevel_ > NumberOfGridLevels_ ) {
       
       printf("Attempting to solve on a grid level that does not exist... bailing out! \n");
       fflush(NULL);
       exit(1);
       
    }
    
    // Find trailing edge for each trailing vortex

    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
    
       for ( j = 1 ; j <= VortexSheet(k).NumberOfKuttaEdges() ; j++ ) {
          
          VortexSheet(k).TrailingVortex(j).TE_Edge() = 0;
         
       }
       
    }
    
    for ( i = 1 ; i <= Grid(SolveOnMGLevel_).NumberOfEdges() ; i++ ) {
       
       if ( Grid(SolveOnMGLevel_).EdgeList(i).IsTrailingEdge() ) {
             
          Node1 = Grid(SolveOnMGLevel_).EdgeList(i).Node1();
          Node2 = Grid(SolveOnMGLevel_).EdgeList(i).Node2();
          
          for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
    
             for ( j = 1 ; j < VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
                
                NodeA = VortexSheet(k).TrailingVortex(j  ).TE_Node();
                NodeB = VortexSheet(k).TrailingVortex(j+1).TE_Node();
                
                if ( Node1 == NodeA && Node2 == NodeB ) VortexSheet(k).TrailingVortex(j).TE_Edge() = -i;
                                                        
                if ( Node1 == NodeB && Node2 == NodeA ) VortexSheet(k).TrailingVortex(j).TE_Edge() =  i;
                
             }
             
             if ( VortexSheet(k).IsPeriodic() ) {
                
               j = VortexSheet(k).NumberOfTrailingVortices();
                  
               NodeA = VortexSheet(k).TrailingVortex(j).TE_Node();
               NodeB = VortexSheet(k).TrailingVortex(1).TE_Node();
               
               if ( Node1 == NodeA && Node2 == NodeB ) VortexSheet(k).TrailingVortex(j).TE_Edge() = -i;
                                                       
               if ( Node1 == NodeB && Node2 == NodeA ) VortexSheet(k).TrailingVortex(j).TE_Edge() =  i;
                
             }

          }
          
       }
       
    }
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
    
       for ( j = 1 ; j < VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
          
          if ( VortexSheet(k).TrailingVortex(j).TE_Edge() == 0 ) {
             
             NodeA = VortexSheet(k).TrailingVortex(j  ).TE_Node();
             NodeB = VortexSheet(k).TrailingVortex(j+1).TE_Node();             
          
             printf("Failed to find matching TE edge for Wake %d ... Trailing Vortex: %d ... with nodes %d, %d \n",k,j,NodeA,NodeB);

          }
          
       }
       
    }                    
    
    // Calculate chord for each trailing edge

    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       for ( j = 1 ; j < VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
          
          Edge = ABS(VortexSheet(k).TrailingVortex(j).TE_Edge());
         
          Node1 = Grid(SolveOnMGLevel_).EdgeList(Edge).Node1();
          
          Node2 = Grid(SolveOnMGLevel_).EdgeList(Edge).Node2();
                                                                       
          SurfaceID = Grid(SolveOnMGLevel_).EdgeList(Edge).SurfaceID();

          Loop = Grid(SolveOnMGLevel_).EdgeList(Edge).Loop1();
          
          // Find trailing edge node U values, as they are stored per loop
          
          Found = 0;
          
          i = 1;
         
          while ( i <= Grid(SolveOnMGLevel_).LoopList(Loop).NumberOfNodes() && Found < 2 ) {
                                                       
             if ( Grid(SolveOnMGLevel_).LoopList(Loop).Node(i) == Node1 ) { U1 = Grid(SolveOnMGLevel_).LoopList(Loop).U_Node(i) ; V1 = Grid(SolveOnMGLevel_).LoopList(Loop).V_Node(i) ; Found++; };
             if ( Grid(SolveOnMGLevel_).LoopList(Loop).Node(i) == Node2 ) { U2 = Grid(SolveOnMGLevel_).LoopList(Loop).U_Node(i) ; V2 = Grid(SolveOnMGLevel_).LoopList(Loop).V_Node(i) ; Found++; };
             
             i++;
             
          } 
          
          if ( Found != 2 ) {
             
             printf("Error in determining TE edge U values! \n");
             printf("Edge: %d \n",Edge);
             printf("Node1,2: %d %d \n",Node1,Node2);
             fflush(NULL);exit(1);
             
          }        
      
          VortexSheet(k).TrailingVortex(j).LocalChord() = 0.;
          
          for ( p = 1 ; p <= Grid(SolveOnMGLevel_).NumberOfSurfaceEdges() ; p++ ) {
             
             if ( Grid(SolveOnMGLevel_).EdgeList(p).SurfaceID() == SurfaceID ) {
             
                Node1 = Grid(SolveOnMGLevel_).EdgeList(p).Node1();          
                Node2 = Grid(SolveOnMGLevel_).EdgeList(p).Node2();
                       
                Loop1 = Grid(SolveOnMGLevel_).EdgeList(p).Loop1();
                Loop2 = Grid(SolveOnMGLevel_).EdgeList(p).Loop2();
                
                if ( Grid(SolveOnMGLevel_).LoopList(Loop1).SurfaceID() == Grid(SolveOnMGLevel_).LoopList(Loop2).SurfaceID() ) {
                
                   Loop = Loop1;

                   if ( Loop == 0 ) {
                      
                      printf("Error in determing edge surface, and UV values... ! \n");fflush(NULL);exit(1);
                      
                   }
                   
                   // Find the U values for this edge
                   
                   Found = 0;
                   
                   i = 1;
                  
                   while ( i <= Grid(SolveOnMGLevel_).LoopList(Loop).NumberOfNodes() && Found < 2 ) {
                                                                
                      if ( Grid(SolveOnMGLevel_).LoopList(Loop).Node(i) == Node1 ) { U3 = Grid(SolveOnMGLevel_).LoopList(Loop).U_Node(i) ; V3 = Grid(SolveOnMGLevel_).LoopList(Loop).V_Node(i) ; Found++; };
                      if ( Grid(SolveOnMGLevel_).LoopList(Loop).Node(i) == Node2 ) { U4 = Grid(SolveOnMGLevel_).LoopList(Loop).U_Node(i) ; V4 = Grid(SolveOnMGLevel_).LoopList(Loop).V_Node(i) ; Found++; };
                      
                      i++;
                      
                   } 
                   
                   if ( Found != 2 ) {
                      
                      printf("Error in determing TE edge U values! \n");fflush(NULL);exit(1);
                      
                   }      
          
                   // Edge sits inside this strip... mark it so we can do spanwise loading calculations later...
                   
                  // if ( 0.5*(U3 + U4) >= MIN(U1,U2) &&
                  //      0.5*(U3 + U4) <= MAX(U1,U2) ) {
                  //    
                  //    if ( Grid(SolveOnMGLevel_).EdgeList(p).VortexSheet() == 0 ) {
                  //            
                  //       Grid(SolveOnMGLevel_).EdgeList(p).VortexSheet() = k;
                  //       
                  //       Grid(SolveOnMGLevel_).EdgeList(p).KuttaNode() = j;
                  //
                  //    }
                  //       
                  // }
                   
                   // On a constant chord line
                   
                   if ( V3 == V4 ) {
                                
                      if ( 0.5*(U3 + U4) >= MIN(U1,U2) &&
                           0.5*(U3 + U4) <= MAX(U1,U2) ) {
                           
                         Length = sqrt( pow(Grid(SolveOnMGLevel_).EdgeList(p).Xc() - Grid(SolveOnMGLevel_).EdgeList(Edge).Xc(), 2.)
                                      + pow(Grid(SolveOnMGLevel_).EdgeList(p).Yc() - Grid(SolveOnMGLevel_).EdgeList(Edge).Yc(), 2.)
                                      + pow(Grid(SolveOnMGLevel_).EdgeList(p).Zc() - Grid(SolveOnMGLevel_).EdgeList(Edge).Zc(), 2.) );
                                      
                         Vec[0] = U3 + U4 - U1 - U2;
                         Vec[1] = V3 + V4 - V1 - V2;       
                         
                         Mag = sqrt( Vec[0]*Vec[0] + Vec[1]*Vec[1] );
                         
                         Vec[0] /= Mag;
                         Vec[1] /= Mag;
                         
                      //   printf("k,j %d, %d --> Length: %f --> Vec: %f %f ... U1,2: %f %f ... U*: %f ... V3,4: %f %f \n",k,j,Length,Vec[0],Vec[1],U1,U2,0.5*(U3+U4),V3,V4);
                         
                         Length *= (V3+V4);  // At leading edge V3+V4 = 1.                    
                   
                         if ( Length > VortexSheet(k).TrailingVortex(j).LocalChord() ) {
                      
                            VortexSheet(k).TrailingVortex(j).LocalChord() = Length;
                            
                            VortexSheet(k).TrailingVortex(j).LE_Edge() = p;
                            
                         }
                
                      }
                      
                   }
                   
                }
                     
             }
                   
          }
                  
       }
       
    }

    // Distribute edges for strip wise force/moment integrations

    for ( p = 1 ; p <= Grid(SolveOnMGLevel_).NumberOfSurfaceEdges() ; p++ ) {

       Grid(SolveOnMGLevel_).EdgeList(p).VortexSheet() = 0;
       
       Grid(SolveOnMGLevel_).EdgeList(p).KuttaNode(1) = 0;
       
       Grid(SolveOnMGLevel_).EdgeList(p).KuttaNode(2) = 0;
       
    }

    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       for ( j = 1 ; j < VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
          
          Edge = ABS(VortexSheet(k).TrailingVortex(j).TE_Edge());
         
          Node1 = Grid(SolveOnMGLevel_).EdgeList(Edge).Node1();
          
          Node2 = Grid(SolveOnMGLevel_).EdgeList(Edge).Node2();
                                                                       
          SurfaceID = Grid(SolveOnMGLevel_).EdgeList(Edge).SurfaceID();

          Loop = Grid(SolveOnMGLevel_).EdgeList(Edge).Loop1();
        
          // Find trailing edge node U values, as they are stored per loop
          
          Found = 0;
          
          i = 1;
         
          while ( i <= Grid(SolveOnMGLevel_).LoopList(Loop).NumberOfNodes() && Found < 2 ) {
                                                       
             if ( Grid(SolveOnMGLevel_).LoopList(Loop).Node(i) == Node1 ) { U1 = Grid(SolveOnMGLevel_).LoopList(Loop).U_Node(i) ; V1 = Grid(SolveOnMGLevel_).LoopList(Loop).V_Node(i) ; Found++; };
             if ( Grid(SolveOnMGLevel_).LoopList(Loop).Node(i) == Node2 ) { U2 = Grid(SolveOnMGLevel_).LoopList(Loop).U_Node(i) ; V2 = Grid(SolveOnMGLevel_).LoopList(Loop).V_Node(i) ; Found++; };
             
             i++;
             
          } 
          
          if ( Found != 2 ) {
             
             printf("Error in determining TE edge U values! \n");
             printf("Edge: %d \n",Edge);
             printf("Node1,2: %d %d \n",Node1,Node2);
             fflush(NULL);exit(1);
             
          }        
          
          printf("j: %d --> U1,2: %f %f \n",j,U1,U2);fflush(NULL);

          for ( p = 1 ; p <= Grid(SolveOnMGLevel_).NumberOfSurfaceLoops() ; p++ ) {
             
             if ( Grid(SolveOnMGLevel_).LoopList(p).SurfaceID() == SurfaceID ) {
                
                U3 = 0.;
                V3 = 0.;
                
                for ( i = 1 ; i <= Grid(SolveOnMGLevel_).LoopList(p).NumberOfNodes() ; i++ ) {
                   
                   U3 += Grid(SolveOnMGLevel_).LoopList(p).U_Node(i);
                   V3 += Grid(SolveOnMGLevel_).LoopList(p).V_Node(i);
                   
                }
                
                U3 /= Grid(SolveOnMGLevel_).LoopList(p).NumberOfNodes();
                V3 /= Grid(SolveOnMGLevel_).LoopList(p).NumberOfNodes();
                
                // Loop is in this strip
                   
                if ( U3 >= MIN(U1,U2) && U3 <= MAX(U1,U2) ) {     
           
                   for ( i = 1 ; i <= Grid(SolveOnMGLevel_).LoopList(p).NumberOfEdges() ; i++ ) {
                      
                      q = ABS(Grid(SolveOnMGLevel_).LoopList(p).Edge(i));
                      
                      if ( Grid(SolveOnMGLevel_).EdgeList(q).VortexSheet() == 0 ) {
                              
                         Grid(SolveOnMGLevel_).EdgeList(q).VortexSheet() = k;
                         
                         Grid(SolveOnMGLevel_).EdgeList(q).KuttaNode(1) = j;
                  
                      }         
                      
                      else {

                         Grid(SolveOnMGLevel_).EdgeList(q).KuttaNode(2) = j;
                        
                      }
                      
                   }
                   
                }
                
             }
             
          }
          
       }
                                   
    }
    
    for ( i = 1 ; i <= Grid(SolveOnMGLevel_).NumberOfSurfaceEdges() ; i++ ) {
   
       if ( Grid(SolveOnMGLevel_).EdgeList(i).VortexSheet() > 0 && Grid(SolveOnMGLevel_).EdgeList(i).IsBoundaryEdge() ) {
       
          Grid(SolveOnMGLevel_).EdgeList(i).KuttaNode(2) = Grid(SolveOnMGLevel_).EdgeList(i).KuttaNode(1);
       
       }
       
    }
        
    // Pointer to global node
    
    TotalNodes = Grid(SolveOnMGLevel_).NumberOfSurfaceNodes();
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       for ( j = 1 ; j <= VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
     
          for ( i = 1 ; i <= VortexSheet(k).NumberOfWakeTrailingNodes() ; i++ ) {
        
             TotalNodes++;
             
             VortexSheet(k).TrailingVortex(j).GlobalNode(i) = TotalNodes;
             
             VortexSheet(k).TrailingVortex(j).WakeNodeX(i) = Grid(SolveOnMGLevel_).NodeList(TotalNodes).x();
             VortexSheet(k).TrailingVortex(j).WakeNodeY(i) = Grid(SolveOnMGLevel_).NodeList(TotalNodes).y();
             VortexSheet(k).TrailingVortex(j).WakeNodeZ(i) = Grid(SolveOnMGLevel_).NodeList(TotalNodes).z();

          }
          
          VortexSheet(k).TrailingVortex(j).UpdateWakeSpacing();
          
       }

    }
      
    // Global loop and trailing edge for each wake loop
    
    TotalLoops = 0;
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       Loop = 0;
       
       if ( Verbose_ ) printf("checking... VortexSheet(k).NumberOfKuttaEdges(): %d \n",VortexSheet(k).NumberOfKuttaEdges());fflush(NULL);
       
       for ( j = 1 ; j <= VortexSheet(k).NumberOfKuttaEdges() ; j++ ) {
     
          if ( Verbose_ ) printf("VortexSheet(k).TrailingVortex(%d).TE_Edge(): %d \n", j,VortexSheet(k).TrailingVortex(j).TE_Edge());
     
          for ( i = 1 ; i <= VortexSheet(k).NumberOfWakeTrailingNodes() - 1 ; i++ ) {
             
             Loop++;
             
             TotalLoops++;
             
             VortexSheet(k).WakeLoopList(Loop).KuttaEdge() = j;
             
             VortexSheet(k).WakeLoopList(Loop).GlobalLoop() = TotalLoops + Grid(SolveOnMGLevel_).NumberOfSurfaceLoops();
             
             VortexSheet(k).WakeLoopList(Loop).GlobalTrailingEdge() = VortexSheet(k).TrailingVortex(j).TE_Edge();
             
          }
          
       }

    }
    
    // Create node to edge list

    NumberOfEdgesForNode = new int[Grid(SolveOnMGLevel_).NumberOfNodes() + 1];
    
    NodeToEdgeList = new int*[Grid(SolveOnMGLevel_).NumberOfNodes() + 1];
    
    zero_int_array(NumberOfEdgesForNode, Grid(SolveOnMGLevel_).NumberOfNodes());
    
    for ( Edge = 1 ; Edge <= Grid(SolveOnMGLevel_).NumberOfEdges() ; Edge++ ) {
   
       Node = Grid(SolveOnMGLevel_).EdgeList(Edge).Node1();
       
       NumberOfEdgesForNode[Node]++;
       
       Node = Grid(SolveOnMGLevel_).EdgeList(Edge).Node2();
       
       NumberOfEdgesForNode[Node]++;       

    }
    
    for ( i = 1 ; i <= Grid(SolveOnMGLevel_).NumberOfNodes() ; i++ ) {
       
       NodeToEdgeList[i] = new int[NumberOfEdgesForNode[i] + 1];
       
    }
    
    zero_int_array(NumberOfEdgesForNode, Grid(SolveOnMGLevel_).NumberOfNodes());
    
    for ( Edge = 1 ; Edge <= Grid(SolveOnMGLevel_).NumberOfEdges() ; Edge++ ) {
 
       Node = Grid(SolveOnMGLevel_).EdgeList(Edge).Node1();
       
       NumberOfEdgesForNode[Node]++;
       
       NodeToEdgeList[Node][NumberOfEdgesForNode[Node]] = Edge;
       
       Node = Grid(SolveOnMGLevel_).EdgeList(Edge).Node2();
       
       NumberOfEdgesForNode[Node]++;    

       NodeToEdgeList[Node][NumberOfEdgesForNode[Node]] = Edge;
       
    }    
        
    // Global edge list for each trailing vortex

    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       for ( j = 1 ; j <= VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
     
          for ( i = 1 ; i <= VortexSheet(k).NumberOfWakeTrailingNodes() - 1 ; i++ ) {
             
             Node1 = VortexSheet(k).TrailingVortex(j).GlobalNode(i  );
             Node2 = VortexSheet(k).TrailingVortex(j).GlobalNode(i+1);
             
             Found = 0;
             
             p = 1;
            
             while ( !Found && p <= NumberOfEdgesForNode[Node1] ) {
                
                Edge = NodeToEdgeList[Node1][p];
                
                NodeA = Grid(SolveOnMGLevel_).EdgeList(Edge).Node1() + Grid(SolveOnMGLevel_).EdgeList(Edge).Node2() - Node1;

                if ( Node2 == NodeA ) {
                   
                   VortexSheet(k).TrailingVortex(j).GlobalEdge(i) = Edge;
                       
                   VortexSheet(k).TrailingVortex(j).EdgeList(i) = &(Grid(SolveOnMGLevel_).EdgeList(Edge));
 
                   Found = 1;
                   
                }
                
                p++;
                
             }
             
             if ( !Found ) {
                
                printf("Could not find global edge for trailing wake! \n");
                printf("Looking for edge with nodes: %d, %d \n",Node1,Node2);
                fflush(NULL);exit(1);
                
             }

          }
          
       }

    }
     
    // Free up memory
    
    for ( i = 1 ; i <= Grid(SolveOnMGLevel_).NumberOfNodes() ; i++ ) {
       
       if ( NumberOfEdgesForNode[i] > 0 ) delete [] NodeToEdgeList[i];
       
    } 
    
    delete [] NodeToEdgeList;
    delete [] NumberOfEdgesForNode;
    
    // Set up trailing wake equation numbering

    p = 0;
   
    for ( i = 1 ; i <= NumberOfVortexSheets() ; i++ ) {
       
       for ( j = 1 ; j <= VortexSheet(i).NumberOfTrailingVortices() ; j++ ) {
         
          for ( k = 1 ; k <= VortexSheet(i).TrailingVortex(j).NumberOfNodes() ; k++ ) {
            
             VortexSheet(i).TrailingVortex(j).WakeResidualEquationNumberX(k) = Grid(SolveOnMGLevel_).NumberOfSurfaceLoops() + ++p;
             VortexSheet(i).TrailingVortex(j).WakeResidualEquationNumberY(k) = Grid(SolveOnMGLevel_).NumberOfSurfaceLoops() + ++p; 
             VortexSheet(i).TrailingVortex(j).WakeResidualEquationNumberZ(k) = Grid(SolveOnMGLevel_).NumberOfSurfaceLoops() + ++p; 

          }
          
       }
       
    }    

    NumberOfWakeResidualEquations_ = p;
    
    // Initialize wake edges with data...
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       for ( j = 1 ; j <= VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
     
          for ( i = 1 ; i <= VortexSheet(k).NumberOfWakeTrailingNodes() - 1 ; i++ ) {    
             
             Node1 = VortexSheet(k).TrailingVortex(j).GlobalNode(i  );
             
             Node2 = VortexSheet(k).TrailingVortex(j).GlobalNode(i+1);
                          
             Edge = VortexSheet(k).TrailingVortex(j).GlobalEdge(i);
             
             Grid(SolveOnMGLevel_).EdgeList(Edge).CreateWakeEdgeSpace();
                
             if ( Grid(SolveOnMGLevel_).EdgeList(Edge).Node1() == Node1 ) {
                
                Grid(SolveOnMGLevel_).EdgeList(Edge).WakeEdgeData().Direction() = 1;
                
             }
             
             else {
             
                Grid(SolveOnMGLevel_).EdgeList(Edge).WakeEdgeData().Direction() = -1;
                
             }
             
             TE_Edge = VortexSheet(k).TrailingVortex(j).TE_Edge();
             
             Grid(SolveOnMGLevel_).EdgeList(Edge).WakeEdgeData().GlobalTrailingEdgeEdge() = TE_Edge;

             Grid(SolveOnMGLevel_).EdgeList(Edge).WakeEdgeData().GlobalTrailingEdgeLoopL() = Grid(SolveOnMGLevel_).EdgeList(ABS(TE_Edge)).LoopL();

             Grid(SolveOnMGLevel_).EdgeList(Edge).WakeEdgeData().GlobalTrailingEdgeLoopR() = Grid(SolveOnMGLevel_).EdgeList(ABS(TE_Edge)).LoopR();

             Grid(SolveOnMGLevel_).EdgeList(Edge).WakeEdgeData().dS() = VortexSheet(k).TrailingVortex(j).dS(i);
             
             if ( i == VortexSheet(k).NumberOfWakeTrailingNodes() - 1 ) Grid(SolveOnMGLevel_).EdgeList(Edge).WakeEdgeData().dS() = 0.;

             Grid(SolveOnMGLevel_).EdgeList(Edge).WakeEdgeData().WakeResidualEquationNumberForX() = VortexSheet(k).TrailingVortex(j).WakeResidualEquationNumberX(i+1);

          }
          
       }
       
    }

    // Mark edges coming off convex regions
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       for ( j = 1 ; j <= VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
                
          // Mark concave edges
          
          if ( VortexSheet(k).TrailingVortex(j).TE_Node_Region_Is_Concave() ) {
                        
             for ( i = 1 ; i <= VortexSheet(k).NumberOfWakeTrailingNodes() - 1 ; i++ ) {    
   
                Edge = VortexSheet(k).TrailingVortex(j).GlobalEdge(i);
   
                Grid(SolveOnMGLevel_).EdgeList(Edge).IsConcaveTrailingEdge() = 1;
   
             }
             
          }
          
       }
       
    }
                                      
    // Recursively mark wake equation edges on coarse grid
    
    for ( i = SolveOnMGLevel_ ; i < NumberOfGridLevels_ ; i++ ) {

       RestrictWakeEquations(i);
       
    }
    
    // Calculate the number of stall equations
    
    p = 0;
    
    for ( i = 1 ; i <= NumberOfVortexSheets() ; i++ ) {
       
       for ( j = 1 ; j < VortexSheet(i).NumberOfTrailingVortices() ; j++ ) {
          
          VortexSheet(i).TrailingVortex(j).KuttaStallEquationNumber() = Grid(SolveOnMGLevel_).NumberOfSurfaceLoops() 
                                                                      + NumberOfWakeResidualEquations_
                                                                      + ++p;
          
       }
       
    }
    
    NumberOfStallResidualEquations_ = p;
    
    // Pack pointer from wake loop to the kutta stall equation
        
    for ( k = 1 ; k <= NumberOfVortexSheets() ; k++ ) {
       
       for ( i = 1 ; i <= VortexSheet(k).NumberOfWakeLoops() ; i++ ) {
          
          j = VortexSheet(k).WakeLoopList(i).KuttaEdge();

          VortexSheet(k).WakeLoopList(i).KuttaStallEquationNumber() = VortexSheet(k).TrailingVortex(j).KuttaStallEquationNumber();

       }
 
    }  
    
    // Determine number of vortex stretching equations
    
    p = 0;
    
    for ( k = 1 ; k <= NumberOfVortexSheets() ; k++ ) {
       
       for ( i = 1 ; i <= VortexSheet(k).NumberOfWakeLoops() ; i++ ) {
       
           VortexSheet(k).WakeLoopList(i).VortexStretchingRatioEquationNumber() = Grid(SolveOnMGLevel_).NumberOfSurfaceLoops() 
                                                                                + NumberOfWakeResidualEquations_
                                                                                + NumberOfStallResidualEquations_
                                                                                + ++p;
                                                                      
       }

    }      
        
    NumberOfVortexStretchingRatioEquations_ = p;
        
}

/*##############################################################################
#                                                                              #
#                  VSP_GEOM RestrictWakeEquations                              #
#                                                                              #
##############################################################################*/

void VSP_GEOM::RestrictWakeEquations(int Level)
{

    int i, Edge;
    
    for ( i = 1 ; i <= Grid(Level).NumberOfEdges() ; i++ ) {
       
       if ( Grid(Level).EdgeList(i).IsWakeEdge() ) {
          
          Edge = Grid(Level).EdgeList(i).CoarseGridEdge();
          
          Grid(Level+1).EdgeList(Edge).CreateWakeEdgeSpace();
          
          Grid(Level+1).EdgeList(Edge).IsConcaveTrailingEdge() = Grid(Level).EdgeList(i).IsConcaveTrailingEdge();
          
          Grid(Level+1).EdgeList(Edge).CoreWidth() = Grid(Level).EdgeList(i).CoreWidth();
          
       }
       
    }
   
}

/*##############################################################################
#                                                                              #
#                          VSP_GEOM CreateWakeGrids                            #
#                                                                              #
##############################################################################*/

void VSP_GEOM::CreateWakeGrids(void)
{
 
    int i, j, k, Node, Loop, Tri, Ni, Nj;
    int NumberOfSheets, NumberOfKuttaNodes, IsPeriodic;
    int NumberOfWakeNodes, NumberOfWakeLoops, NumNodes, WakeNodes;
    double WakeAngle[3], Scale_X, Scale_Y, Scale_Z, WakeDist;
    VSP_NODE VSP_Node1, VSP_Node2;
    VORTEX_TRAIL TrailingVortex;
    double Xmin, Xmax, Ymin, Ymax, Zmin, Zmax;
 
    printf("Creating wake grids... \n");fflush(NULL);

    // Determine min/max of geometry
    
    Xmin = 1.e9;
    Xmax = -Xmin;
    
    Ymin = 1.e9;
    Ymax = -Ymin;
    
    Zmin = 1.e9;
    Zmax = -Zmin;        

    for ( j = 1 ; j <= Grid().NumberOfNodes() ; j++ ) {
       
        Xmin = MIN(Xmin,Grid().NodeList(j).x());
        Xmax = MAX(Xmax,Grid().NodeList(j).x());

        Ymin = MIN(Ymin,Grid().NodeList(j).y());
        Ymax = MAX(Ymax,Grid().NodeList(j).y());
                                        
        Zmin = MIN(Zmin,Grid().NodeList(j).z());
        Zmax = MAX(Zmax,Grid().NodeList(j).z());
                            
    }

    // Initial wake in the free stream direction

    WakeAngle[0] = 1.;
    WakeAngle[1] = 0.;
    WakeAngle[2] = 0.;
    
    printf("WakeAngle: %f %f %f \n",WakeAngle[0],WakeAngle[1],WakeAngle[2]);fflush(NULL);

    // Determine how far to allow wakes to adapt... beyond this the wakes go straight off to
    // 'infinity' in the free stream direction
    
    Scale_X = Scale_Y = Scale_Z = 1.;
    
    if ( DoSymmetryPlaneSolve_ ) Scale_Y = 2.;
 
    if ( !SetFarFieldDist_ ) {
       
       FarFieldDist_ = MAX3(Scale_X*(Xmax - Xmin), Scale_Y*(Ymax-Ymin), Scale_Z*(Zmax-Zmin));
       
    }

    printf("Xmax_ - Xmin_: %f \n", double(Xmax - Xmin));
    printf("Ymax_ - Ymin_: %f \n", double(Ymax - Ymin));
    printf("Zmax_ - Zmin_: %f \n", double(Zmax - Zmin));
    printf("\n");

    printf("Wake FarFieldDist_ set to: %f \n",FarFieldDist_);
    printf("\n");    

    // Determine the number of vortex sheets
    
    NumberOfSheets = 0;
    
    for ( j = 1 ; j <= Grid().NumberOfKuttaNodes() ; j++ ) {

       NumberOfSheets = MAX(NumberOfSheets, Grid().WingSurfaceForKuttaNode(j));
       
    }    
   
    NumberOfVortexSheets_ = 0;
    
    for ( k = 1 ; k <= NumberOfSheets ; k++ ) {
       
       NumberOfKuttaNodes = 0;
       
       for ( j = 1 ; j <= Grid().NumberOfKuttaNodes() ; j++ ) {
          
          if ( Grid().WingSurfaceForKuttaNode(j) == k ) NumberOfKuttaNodes++;
          
       }
       
       if ( NumberOfKuttaNodes > 1 ) NumberOfVortexSheets_++;
       
    }
    
    printf("There are: %10d Vortex Sheets \n", NumberOfVortexSheets_);

    // Determine total number of wake nodes and loops
      
    NumberOfWakeNodes = NumberOfWakeLoops = 0;
            
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       IsPeriodic = NumNodes = 0;
       
       for ( j = 1 ; j <= Grid().NumberOfKuttaNodes() ; j++ ) {
          
          if ( Grid().WingSurfaceForKuttaNode(j) == k ) {
          
             NumNodes++;
             
             if ( Grid().WingSurfaceForKuttaNodeIsPeriodic(j) ) IsPeriodic = 1;
             
          }
          
       }

       NumberOfWakeNodes += NumNodes;
                                        
       NumberOfWakeLoops += NumNodes - 1;
       
       if ( IsPeriodic ) NumberOfWakeLoops++;
       
       if ( IsPeriodic ) printf("Wake %d is periodic \n",k);
       
    }
    
    NumberOfWakeNodes *= (NumberOfWakeTrailingNodes_    );
    
    NumberOfWakeLoops *= (NumberOfWakeTrailingNodes_ - 1);
    
    // Size node and loop list for coarse mesh
    
    printf("NumberOfWakeTrailingNodes_: %d \n",NumberOfWakeTrailingNodes_);
    printf("NumberOfWakeNodes: %d \n",NumberOfWakeNodes);
    printf("NumberOfWakeLoops: %d \n",NumberOfWakeLoops);fflush(NULL);
    
    GridWC_ = new VSP_GRID;
    
    GridWC_->SizeNodeList(NumberOfWakeNodes);
        
    GridWC_->SizeLoopList(NumberOfWakeLoops);

    // Size node and loop list for fine mesh
    
    GridWF_ = new VSP_GRID;
         
    GridWF_->SizeNodeList(NumberOfWakeNodes);
         
    GridWF_->SizeLoopList(2*NumberOfWakeLoops);   
                       
    // Create wake nodes
    
    Node = WakeNodes = 0;
    
    Tri = Loop = 0;
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
     
       NumNodes = 0;
       
       IsPeriodic = 0;
       
       for ( j = 1 ; j <= Grid().NumberOfKuttaNodes() ; j++ ) {
          
          if ( Grid().WingSurfaceForKuttaNode(j) == k ) {
             
             if ( Grid().WingSurfaceForKuttaNodeIsPeriodic(j) ) IsPeriodic = 1;
             
             NumNodes++;
             
             // Pass in edge data and create edge coefficients
             
             VSP_Node1.x() = Grid().WakeTrailingEdgeX(j);
             VSP_Node1.y() = Grid().WakeTrailingEdgeY(j);
             VSP_Node1.z() = Grid().WakeTrailingEdgeZ(j);

             VSP_Node2.x() = Grid().WakeTrailingEdgeX(j) + WakeAngle[0] * 1.e4;
             VSP_Node2.y() = Grid().WakeTrailingEdgeY(j) + WakeAngle[1] * 1.e4;
             VSP_Node2.z() = Grid().WakeTrailingEdgeZ(j) + WakeAngle[2] * 1.e4;

             // Create trailing wakes... specify number of sub vortices per trail
   
             WakeDist = MAX(VSP_Node1.x() + 0.5*FarFieldDist_, Xmax + 0.25*FarFieldDist_) - VSP_Node1.x();
                 
             // Create a temporary trailing vortex 
             
             TrailingVortex.Setup(NumberOfWakeTrailingNodes_,WakeDist,VSP_Node1,VSP_Node2);                   
          
             for ( i = 1 ; i <= NumberOfWakeTrailingNodes_ ; i++ ) {
                
                Node++;
             
                GridWC_->NodeList(Node).x() = TrailingVortex.WakeNodeX(i);
                GridWC_->NodeList(Node).y() = TrailingVortex.WakeNodeY(i);
                GridWC_->NodeList(Node).z() = TrailingVortex.WakeNodeZ(i);
             
                GridWF_->NodeList(Node).x() = TrailingVortex.WakeNodeX(i);
                GridWF_->NodeList(Node).y() = TrailingVortex.WakeNodeY(i);
                GridWF_->NodeList(Node).z() = TrailingVortex.WakeNodeZ(i);
                
                if ( i == 1 ) {
                   
                   GridWC_->NodeList(Node).IsWakeTrailingEdgeNode() = 1;
                   GridWF_->NodeList(Node).IsWakeTrailingEdgeNode() = 1;
                   
                }
                
                if ( Grid().NodeList(Grid().KuttaNode(j)).IsSymmetryPlaneNode() ) {

                   printf("Setting Kutta node %d ... which is grid node: %d as on symmetry plane \n",j,Node);
                   
                   GridWC_->NodeList(Node).IsSymmetryPlaneNode() = 1;
                   GridWF_->NodeList(Node).IsSymmetryPlaneNode() = 1;
                  
                }                   
                          
             }    
             
          }                   
           
       }

       // Create Loop data

       Nj = NumNodes;
       
       for ( j = 1 ; j <= Nj - 1 ; j++ ) {
          
          Ni = NumberOfWakeTrailingNodes_;
          
          for ( i = 1 ; i <= Ni - 1 ; i++ ) {
             
             // Coarse grid
             
             Loop++;
    
             GridWC_->LoopList(Loop).SizeNodeList(4);
             GridWC_->LoopList(Loop).SizeEdgeList(4);
             
             GridWC_->LoopList(Loop).Node1() = WakeNodes + (j-1)*Ni + i;
             GridWC_->LoopList(Loop).Node2() = WakeNodes + (j  )*Ni + i;
             GridWC_->LoopList(Loop).Node3() = WakeNodes + (j  )*Ni + i + 1;
             GridWC_->LoopList(Loop).Node4() = WakeNodes + (j-1)*Ni + i + 1;
             
             GridWC_->LoopList(Loop).SizeFineGridLoopList(2);
             
             GridWC_->LoopList(Loop).FineGridLoop(1) = Tri + 1;
             GridWC_->LoopList(Loop).FineGridLoop(2) = Tri + 2;
             
             GridWC_->LoopList(Loop).SurfaceID() = 0;
             
             GridWC_->LoopList(Loop).MinValidTimeStep() = i;
                          
             GridWC_->LoopList(Loop).UpwindWakeLoop() = 0;
             
             if ( i > 1 ) GridWC_->LoopList(Loop).UpwindWakeLoop() = Loop - 1;

             // Fine grid
             
             Tri++;
          
             GridWF_->LoopList(Tri).SizeNodeList(3);
             GridWF_->LoopList(Tri).SizeEdgeList(3);
                  
             GridWF_->LoopList(Tri).Node1() = WakeNodes + (j-1)*Ni + i;
             GridWF_->LoopList(Tri).Node2() = WakeNodes + (j  )*Ni + i;
             GridWF_->LoopList(Tri).Node3() = WakeNodes + (j  )*Ni + i + 1;
   
             GridWF_->LoopList(Tri).SurfaceID() = 0;    
             
             GridWF_->LoopList(Tri).MinValidTimeStep() = i;
             
             GridWF_->LoopList(Tri).UpwindWakeLoop() = 0;
             
             if ( i > 1 ) GridWF_->LoopList(Tri).UpwindWakeLoop() = Tri - 2;
             
             Tri++;

             GridWF_->LoopList(Tri).SizeNodeList(3);
             GridWF_->LoopList(Tri).SizeEdgeList(3);
                  
             GridWF_->LoopList(Tri).Node1() = WakeNodes + (j-1)*Ni + i;
             GridWF_->LoopList(Tri).Node2() = WakeNodes + (j  )*Ni + i + 1;
             GridWF_->LoopList(Tri).Node3() = WakeNodes + (j-1)*Ni + i + 1;
             
             GridWF_->LoopList(Tri).SurfaceID() = 0;             

             GridWF_->LoopList(Tri).MinValidTimeStep() = i;
             
             GridWF_->LoopList(Tri).UpwindWakeLoop() = 0;

             if ( i > 1 ) GridWF_->LoopList(Tri).UpwindWakeLoop() = Tri - 2;
             
          }
          
       }
       
       // Wake is periodic... add final strip of loops/tris
       
       if ( IsPeriodic ) {
          
          j = NumNodes;
          
          Ni = NumberOfWakeTrailingNodes_;
          
          for ( i = 1 ; i <= Ni - 1 ; i++ ) {
             
             // Coarse grid
             
             Loop++;
    
             GridWC_->LoopList(Loop).SizeNodeList(4);
             GridWC_->LoopList(Loop).SizeEdgeList(4);
             
             GridWC_->LoopList(Loop).Node1() = WakeNodes + (j-1)*Ni + i;
             GridWC_->LoopList(Loop).Node2() = WakeNodes            + i;
             GridWC_->LoopList(Loop).Node3() = WakeNodes            + i + 1;
             GridWC_->LoopList(Loop).Node4() = WakeNodes + (j-1)*Ni + i + 1;
             
             GridWC_->LoopList(Loop).SizeFineGridLoopList(2);
             
             GridWC_->LoopList(Loop).FineGridLoop(1) = Tri + 1;
             GridWC_->LoopList(Loop).FineGridLoop(2) = Tri + 2;
             
             GridWC_->LoopList(Loop).SurfaceID() = 0;
             
             GridWC_->LoopList(Loop).MinValidTimeStep() = i;
                          
             GridWC_->LoopList(Loop).UpwindWakeLoop() = 0;
             
             if ( i > 1 ) GridWC_->LoopList(Loop).UpwindWakeLoop() = Loop - 1;

             // Fine grid
             
             Tri++;
          
             GridWF_->LoopList(Tri).SizeNodeList(3);
             GridWF_->LoopList(Tri).SizeEdgeList(3);
                  
             GridWF_->LoopList(Tri).Node1() = WakeNodes + (j-1)*Ni + i;
             GridWF_->LoopList(Tri).Node2() = WakeNodes            + i;
             GridWF_->LoopList(Tri).Node3() = WakeNodes            + i + 1;
   
             GridWF_->LoopList(Tri).SurfaceID() = 0;    
             
             GridWF_->LoopList(Tri).MinValidTimeStep() = i;
             
             GridWF_->LoopList(Tri).UpwindWakeLoop() = 0;
             
             if ( i > 1 ) GridWF_->LoopList(Tri).UpwindWakeLoop() = Tri - 2;
             
             Tri++;

             GridWF_->LoopList(Tri).SizeNodeList(3);
             GridWF_->LoopList(Tri).SizeEdgeList(3);
                  
             GridWF_->LoopList(Tri).Node1() = WakeNodes + (j-1)*Ni + i;
             GridWF_->LoopList(Tri).Node2() = WakeNodes            + i + 1;
             GridWF_->LoopList(Tri).Node3() = WakeNodes + (j-1)*Ni + i + 1;
             
             GridWF_->LoopList(Tri).SurfaceID() = 0;             

             GridWF_->LoopList(Tri).MinValidTimeStep() = i;
             
             GridWF_->LoopList(Tri).UpwindWakeLoop() = 0;

             if ( i > 1 ) GridWF_->LoopList(Tri).UpwindWakeLoop() = Tri - 2;
             
          }       
          
       }   
       
       WakeNodes = Node;

    }
    
    // Setup the coarse grid

    GridWC_->CalculateTriNormalsAndCentroids();
         
    GridWC_->CreateTriEdges();
         
    GridWC_->CalculateUpwindEdges();  
         
    GridWC_->CreateUpwindEdgeData();
         
    GridWC_->MarkBoundaries(DoSymmetryPlaneSolve_);    
    
    GridWC_->DetermineSurfaceMeshSize();
    
    // Setup the fine grid
    
    GridWF_->CalculateTriNormalsAndCentroids();
  
    GridWF_->CreateTriEdges();
  
    GridWF_->CalculateUpwindEdges();  
  
    GridWF_->CreateUpwindEdgeData();

    GridWF_->MarkBoundaries(DoSymmetryPlaneSolve_);        
    
    GridWF_->DetermineSurfaceMeshSize();

}

/*##############################################################################
#                                                                              #
#                       VSP_GEOM MarkAndSetupRotorWakes                        #
#                                                                              #
##############################################################################*/

void VSP_GEOM::MarkAndSetupRotorWakes(void)
{
 
    int g, i, j, k, m;
    int *ComponentInThisGroup;
               
    // Mark components in groups that are rotors

    ComponentInThisGroup = new int[NumberOfComponents() + 1];
 
    zero_int_array(ComponentInThisGroup, NumberOfComponents());
 
    for ( g = 1 ; g <= NumberOfComponentGroups_ ; g++ ) {
        
       for ( j = 1 ; j <= ComponentGroupList_[g].NumberOfComponents() ; j++ ) {
       
          ComponentInThisGroup[ComponentGroupList_[g].ComponentList(j)] = g;
          
       }

       // Set the group flag for all edges
       
       for ( m = 1 ; m <= NumberOfGridLevels_ ; m++ ) {
          
          for ( j = 1 ; j <= Grid(m).NumberOfEdges() ; j++ ) {
             
             if ( ComponentInThisGroup[Grid(m).EdgeList(j).ComponentID()] ) {
                
                Grid(m).EdgeList(j).Group() = g;
                
             }
             
          }
          
       }
              
       zero_int_array(ComponentInThisGroup,NumberOfComponents());
       
    }
                    
    // Mark components in groups that are rotors

    ComponentInThisGroup = new int[NumberOfComponents() + 1];
 
    zero_int_array(ComponentInThisGroup, NumberOfComponents());
    
    ThereAreRotors_ = 0;
 
    for ( i = 1 ; i <= NumberOfComponentGroups_ ; i++ ) {
       
        if ( ComponentGroupList_[i].GeometryIsARotor() ) {
           
          ThereAreRotors_ = 1;
 
          for ( j = 1 ; j <= ComponentGroupList_[i].NumberOfComponents() ; j++ ) {
          
             ComponentInThisGroup[ComponentGroupList_[i].ComponentList(j)] = i;
             
          }
 
       }
       
    }
    
    // Mark and setup any vortex sheets / vortex trails that come off rotors
                 
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       VortexSheet(k).TimeAccurate() = TimeAccurate_;

       for ( j = 1 ; j <= VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {

          if ( ComponentInThisGroup[VortexSheet(k).TrailingVortex(j).ComponentID()] != 0 ) {
             
             g = ComponentInThisGroup[VortexSheet(k).TrailingVortex(j).ComponentID()];
             
             VortexSheet(k).IsARotor() = ComponentGroupList_[g].GeometryIsARotor();
             
             VortexSheet(k).TrailingVortex(j).Group() = g;
             
             VortexSheet(k).TrailingVortex(j).IsARotor() = ComponentGroupList_[g].GeometryIsARotor();
 
             VortexSheet(k).TrailingVortex(j).RotorAnalysis() = 1;
             
             VortexSheet(k).TrailingVortex(j).RotorOrigin(0) = ComponentGroupList_[g].OVec(0);   
             VortexSheet(k).TrailingVortex(j).RotorOrigin(1) = ComponentGroupList_[g].OVec(1);   
             VortexSheet(k).TrailingVortex(j).RotorOrigin(2) = ComponentGroupList_[g].OVec(2);   
       
             VortexSheet(k).TrailingVortex(j).RotorThrustVector(0) = ComponentGroupList_[g].RVec(0);   
             VortexSheet(k).TrailingVortex(j).RotorThrustVector(1) = ComponentGroupList_[g].RVec(1);   
             VortexSheet(k).TrailingVortex(j).RotorThrustVector(2) = ComponentGroupList_[g].RVec(2);
             
printf("ComponentGroupList_[g].RVec: %f %f %f \n",
ComponentGroupList_[g].RVec(0),
ComponentGroupList_[g].RVec(1),
ComponentGroupList_[g].RVec(2));fflush(NULL);
             VortexSheet(k).TrailingVortex(j).BladeRPM() = 60. * ComponentGroupList_[g].Omega() / (2.*PI);

          }
          
       }
       
    }
    
    delete [] ComponentInThisGroup;
    
}

/*##############################################################################
#                                                                              #
#                       VSP_GEOM InitializeWakeGrid                            #
#                                                                              #
##############################################################################*/

void VSP_GEOM::InitializeWakeGrid(double Vinf, 
                                  double FreeStreamVelocity_[3], 
                                  double WakeAngle[3], 
                                  double Xmax, 
                                  int TimeAccurate)
{
 
    int i, j, k, Node, NumNodes;
    double WakeDist, Vmag, FreeStreamDirection[3];
    VSP_NODE VSP_Node1, VSP_Node2;
    VORTEX_TRAIL TrailingVortex;
 
    printf("Updating wake grids... \n");fflush(NULL);

    TimeAccurate_ = TimeAccurate;
    
    Vinf_ = Vinf;

    Vmag = sqrt(vector_dot(FreeStreamVelocity_,FreeStreamVelocity_));

    if ( Vmag > 0. ) {
       
       FreeStreamDirection[0] = FreeStreamVelocity_[0]/Vmag;
       FreeStreamDirection[1] = FreeStreamVelocity_[1]/Vmag; 
       FreeStreamDirection[2] = FreeStreamVelocity_[2]/Vmag; 
       
    }
    
    else {
       
       FreeStreamDirection[0] = 0.;
       FreeStreamDirection[1] = 0.;
       FreeStreamDirection[2] = 0.;
       
    }
 
    // Now setup the initial wake shape
                  
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       VortexSheet(k).TimeAccurate() = TimeAccurate;

       for ( j = 1 ; j <= VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {

          Node = VortexSheet(k).TrailingVortex(j).TE_Node();

          VSP_Node1.x() = Grid(SolveOnMGLevel_).NodeList(Node).x();
          VSP_Node1.y() = Grid(SolveOnMGLevel_).NodeList(Node).y();
          VSP_Node1.z() = Grid(SolveOnMGLevel_).NodeList(Node).z();

          VSP_Node2.x() = Grid(SolveOnMGLevel_).NodeList(Node).x() + WakeAngle[0] * 1.e4;
          VSP_Node2.y() = Grid(SolveOnMGLevel_).NodeList(Node).y() + WakeAngle[1] * 1.e4;
          VSP_Node2.z() = Grid(SolveOnMGLevel_).NodeList(Node).z() + WakeAngle[2] * 1.e4;    

          WakeDist = MAX(VSP_Node1.x() + 0.5*FarFieldDist_, Xmax + 0.25*FarFieldDist_) - VSP_Node1.x();
          
          TrailingVortex.Vinf() = SGN(Vinf_)*MAX(ABS(Vinf_),1.e-6);

          TrailingVortex.RotorAnalysis() = VortexSheet(k).TrailingVortex(j).RotorAnalysis();
          
          TrailingVortex.BladeRPM() = VortexSheet(k).TrailingVortex(j).BladeRPM();
          
          TrailingVortex.RotorOrigin(0) = VortexSheet(k).TrailingVortex(j).RotorOrigin(0);
          TrailingVortex.RotorOrigin(1) = VortexSheet(k).TrailingVortex(j).RotorOrigin(1);
          TrailingVortex.RotorOrigin(2) = VortexSheet(k).TrailingVortex(j).RotorOrigin(2);

          TrailingVortex.RotorThrustVector(0) = VortexSheet(k).TrailingVortex(j).RotorThrustVector(0);
          TrailingVortex.RotorThrustVector(1) = VortexSheet(k).TrailingVortex(j).RotorThrustVector(1);
          TrailingVortex.RotorThrustVector(2) = VortexSheet(k).TrailingVortex(j).RotorThrustVector(2);
          
          TrailingVortex.FreeStreamDirection(0) = FreeStreamDirection[0];
          TrailingVortex.FreeStreamDirection(1) = FreeStreamDirection[1];
          TrailingVortex.FreeStreamDirection(2) = FreeStreamDirection[2];
                    
          // Create a temporary trailing vortex 
                   
          TrailingVortex.Setup(NumberOfWakeTrailingNodes_,WakeDist,VSP_Node1,VSP_Node2);    
                               
          for ( i = 1 ; i <= VortexSheet(k).NumberOfWakeTrailingNodes() ; i++ ) {
             
             // Update wake mesh
             
             Node = VortexSheet(k).TrailingVortex(j).GlobalNode(i);

             Node = Grid(SolveOnMGLevel_).NodeList(Node).FineGridNode();
                            
             Grid(0).NodeList(Node).x() = TrailingVortex.WakeNodeX(i);
             Grid(0).NodeList(Node).y() = TrailingVortex.WakeNodeY(i);
             Grid(0).NodeList(Node).z() = TrailingVortex.WakeNodeZ(i);

             // Update trailing vortex data
             
             VortexSheet(k).TrailingVortex(j).WakeNodeX(i) = TrailingVortex.WakeNodeX(i);
             VortexSheet(k).TrailingVortex(j).WakeNodeY(i) = TrailingVortex.WakeNodeY(i);
             VortexSheet(k).TrailingVortex(j).WakeNodeZ(i) = TrailingVortex.WakeNodeZ(i);


          }

          VortexSheet(k).TrailingVortex(j).Vinf() = Vinf_;
          
          VortexSheet(k).TrailingVortex(j).FreeStreamDirection(0) = FreeStreamDirection[0];
          VortexSheet(k).TrailingVortex(j).FreeStreamDirection(1) = FreeStreamDirection[1];
          VortexSheet(k).TrailingVortex(j).FreeStreamDirection(2) = FreeStreamDirection[2];
                              
          VortexSheet(k).TrailingVortex(j).UpdateWakeSpacing();
          
          VortexSheet(k).TrailingVortex(j).TimeAccurate() = TimeAccurate_;
          
       }
       
    }
    
    UpdateMeshes();

}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM FreezeWakeGrids                             #
#                                                                              #
##############################################################################*/

void VSP_GEOM::FreezeWakeGrids(void)
{
 
    int i, j, k, Edge;
    
    // Freeze the entire wake grid
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       for ( j = 1 ; j <= VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
     
          VortexSheet(k).TrailingVortex(j).IsFrozenTrailingEdge() = 1;
                             
          // Mark edges as frozen
          
          for ( i = 1 ; i <= VortexSheet(k).NumberOfWakeTrailingNodes() - 1 ; i++ ) {    

             Edge = VortexSheet(k).TrailingVortex(j).GlobalEdge(i);
     
             // Mark edge as frozen
                          
             Grid(SolveOnMGLevel_).EdgeList(Edge).IsFrozenTrailingEdge() = 1;
                  
          }
          
       }
       
    }
    
}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM FreezeWakeRootVortices                      #
#                                                                              #
##############################################################################*/

void VSP_GEOM::FreezeWakeRootVortices(int Level)
{
 
    int i, j, k, jm, jp, Edge;
 
    // Mark as frozen those edges on or next to a convex TE wake line, mark those on a convex edge as on... well... a convex edge ;-) 
    
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {

       for ( j = 1 ; j <= VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {
     
          jm = j - 1;
          jp = j + 1;
          
          if ( j == 1 ) jm = 1;
          if ( j == VortexSheet(k).NumberOfTrailingVortices() ) jp = j;

          // Mark trailing wake as frozen
          
          if ( VortexSheet(k).TrailingVortex(jm).TE_Node_Region_Is_Concave() ||
               VortexSheet(k).TrailingVortex(j ).TE_Node_Region_Is_Concave() ||
               VortexSheet(k).TrailingVortex(jp).TE_Node_Region_Is_Concave()    ) VortexSheet(k).TrailingVortex(j ).IsFrozenTrailingEdge() = 1;
                             
          // Mark edges as frozen of concave as required
          
          for ( i = 1 ; i <= VortexSheet(k).NumberOfWakeTrailingNodes() - 1 ; i++ ) {    

             Edge = VortexSheet(k).TrailingVortex(j).GlobalEdge(i);

             // Mark concave edges as frozen
       
             if ( VortexSheet(k).TrailingVortex(j).TE_Node_Region_Is_Concave() ) Grid(SolveOnMGLevel_).EdgeList(Edge).IsFrozenTrailingEdge() = 1;
                 
             // Mark edges near concave edges as frozen
             
             if ( Level == 2 ) {
                             
                if ( VortexSheet(k).TrailingVortex(jm).TE_Node_Region_Is_Concave() ||
                     VortexSheet(k).TrailingVortex(j ).TE_Node_Region_Is_Concave() ||
                     VortexSheet(k).TrailingVortex(jp).TE_Node_Region_Is_Concave()    ) Grid(SolveOnMGLevel_).EdgeList(Edge).IsFrozenTrailingEdge() = 1;
                     
             }
             
          }
          
       }
       
    }
    
}
        
/*##############################################################################
#                                                                              #
#                          VSP_GEOM MergeGrids                                 #
#                                                                              #
##############################################################################*/

VSP_GRID *VSP_GEOM::MergeGrids(VSP_GRID *Grid1, VSP_GRID *Grid2, int CoarseGridLoopOffSet)
{
 
    int NumberOfNodes, NumberOfLoops, NumberOfEdges;
    int i, n, NewNode, NewLoop, NewEdge, g;
    int NodeOffSet, LoopOffSet, CoarseLoopOffSet, EdgeOffSet;
    VSP_GRID *TempGrid, *Grid;
    
    // Merged Grid
     
    NumberOfNodes = Grid1->NumberOfNodes() + Grid2->NumberOfNodes();
    
    NumberOfLoops = Grid1->NumberOfLoops() + Grid2->NumberOfLoops();

    NumberOfEdges = Grid1->NumberOfEdges() + Grid2->NumberOfEdges();

    TempGrid = new VSP_GRID;
   
    TempGrid->SizeNodeList(NumberOfNodes);
    
    TempGrid->SizeLoopList(NumberOfLoops);
    
    TempGrid->SizeEdgeList(NumberOfEdges);
    
    NewNode = NewLoop = NewEdge = 0;
    
    for ( g = 1 ; g <= 2 ; g++ ) {
       
       if ( g == 1 ) Grid = Grid1;
       if ( g == 2 ) Grid = Grid2;
    
       // Copy in node data
       
       for ( n = 1 ; n <= Grid->NumberOfNodes() ; n++ ) {
          
          NewNode++;
          
          TempGrid->NodeList(NewNode) = Grid->NodeList(n);
          
       }

       // Offsets
       
       if ( g == 1 ) NodeOffSet = 0;
       if ( g == 2 ) NodeOffSet = Grid1->NumberOfNodes();

       if ( g == 1 ) EdgeOffSet = 0;
       if ( g == 2 ) EdgeOffSet = Grid1->NumberOfEdges();       
       
       if ( g == 1 ) LoopOffSet = 0;
       if ( g == 2 ) LoopOffSet = Grid1->NumberOfLoops();
       
       if ( g == 1 ) CoarseLoopOffSet = 0;
       if ( g == 2 ) CoarseLoopOffSet = CoarseGridLoopOffSet;
    
       // Copy in tri connectivity data
           
       for ( n = 1 ; n <= Grid->NumberOfLoops() ; n++ ) {
          
          NewLoop++;
 
          TempGrid->LoopList(NewLoop) = Grid->LoopList(n);
          
          // Update connectivity
       
          for ( i = 1 ; i <= Grid->LoopList(n).NumberOfNodes() ; i++ ) {
      
             TempGrid->LoopList(NewLoop).Node(i) += NodeOffSet;
 
          }

          for ( i = 1 ; i <= Grid->LoopList(n).NumberOfEdges() ; i++ ) {
      
             TempGrid->LoopList(NewLoop).Edge(i) += EdgeOffSet;

          }

          for ( i = 1 ; i <= Grid->LoopList(n).NumberOfFineGridLoops() ; i++ ) {
             
             TempGrid->LoopList(NewLoop).FineGridLoop(i) += CoarseLoopOffSet;
             
          }
          
          // Upwind wake loop

          if ( TempGrid->LoopList(NewLoop).UpwindWakeLoop() > 0 ) TempGrid->LoopList(NewLoop).UpwindWakeLoop() += LoopOffSet;
                    
          // Flags
          
          TempGrid->LoopList(NewLoop).SurfaceID()         = Grid->LoopList(n).SurfaceID();
                              
          TempGrid->LoopList(NewLoop).ComponentID()       = Grid->LoopList(n).ComponentID();
                                                                
          TempGrid->LoopList(NewLoop).SurfaceType()       = Grid->LoopList(n).SurfaceType();
                                                                
          TempGrid->LoopList(NewLoop).VortexSheet()       = Grid->LoopList(n).VortexSheet();
                                                               
          TempGrid->LoopList(NewLoop).SpanStation()       = Grid->LoopList(n).SpanStation() ;
                                                              
          TempGrid->LoopList(NewLoop).IsTrailingEdgeTri() = Grid->LoopList(n).IsTrailingEdgeTri();
 
       }            

       // Copy in the edge connectivity data
    
       if ( g == 1 ) LoopOffSet = 0;
       if ( g == 2 ) LoopOffSet = Grid1->NumberOfLoops();       
    
       for ( i = 1 ; i <= Grid->NumberOfEdges() ; i++ ) {
  
          NewEdge++;
          
          // Copy over data
          
          TempGrid->EdgeList(NewEdge) = Grid->EdgeList(i);
        
          TempGrid->EdgeList(NewEdge).Node1() += NodeOffSet;
          TempGrid->EdgeList(NewEdge).Node2() += NodeOffSet;
          
          TempGrid->EdgeList(NewEdge).Loop1() += LoopOffSet;
          TempGrid->EdgeList(NewEdge).Loop2() += LoopOffSet; 
                                              
          TempGrid->EdgeList(NewEdge).LoopL() += LoopOffSet; 
          TempGrid->EdgeList(NewEdge).LoopR() += LoopOffSet; 
    
       }
          
    }
    
    TempGrid->CalculateTriNormalsAndCentroids();

    TempGrid->CopyKuttaList(Grid1);
    
    TempGrid->DetermineSurfaceMeshSize();
    
    return TempGrid;     
    
}

/*##############################################################################
#                                                                              #
#            VSP_GEOM TestComponentGeometryGradients                           #
#                                                                              #
##############################################################################*/

void VSP_GEOM::TestComponentGeometryGradients(double Alpha, double Beta)
{
   
   int i, c;
   double x, y, z, dx, dy, dz;
   double WettedArea[2], Length[2], FR[2];
   double pWettedArea_p;
   double pLength_p;
   double pFR_p;
   double Error[3];
   double A, B;
   double dAlpha, dBeta;
   
   // Calculate exact gradients
   
   CalculateThickComponentAeroGeometryGradientswrtMesh(Alpha, Beta);
   
   // Gradients wrt alpha and Beta
   
   dAlpha = dBeta = 1.e-12;
      
   // Gradients wrt alpha
      
   A = Alpha - dAlpha;
   B = Beta;

   CalculateThickComponentAeroGeometry(A, B);
   
      WettedArea[0] = ComponentWettedArea(1);
          Length[0] = ComponentLength(1);
              FR[0] = ComponentFineNessRatio(1);
              
   A = Alpha + dAlpha;
   B = Beta;

   CalculateThickComponentAeroGeometry(A, B);
   
   WettedArea[1] = ComponentWettedArea(1);
       Length[1] = ComponentLength(1);
           FR[1] = ComponentFineNessRatio(1);      
           
   pWettedArea_p = ( WettedArea[1] - WettedArea[0] )/(2.*dAlpha);           
       pLength_p = (     Length[1] -     Length[0] )/(2.*dAlpha);           
           pFR_p = (         FR[1] -         FR[0] )/(2.*dAlpha);     
           
   Error[0] = ABS(   pComponentWettedAreapAlpha(1) -  pWettedArea_p);                   
   Error[1] = ABS(       pComponentLengthpAlpha(1) -      pLength_p);                   
   Error[2] = ABS(pComponentFineNessRatiopAlpha(1) -          pFR_p);    
                 
   if ( Error[0] <= 1.e-4 ) printf("pWettedArea_pAlpha: Exact: %12.6f ... FD: %12.6f \n",                  pComponentWettedAreapAlpha(1), pWettedArea_p);                   
   if ( Error[1] <= 1.e-4 ) printf("pLength_pAlpha:     Exact: %12.6f ... FD: %12.6f \n",                      pComponentLengthpAlpha(1),     pLength_p);                   
   if ( Error[2] <= 1.e-4 ) printf("pFR_pAlpha:         Exact: %12.6f ... FD: %12.6f \n",               pComponentFineNessRatiopAlpha(1),         pFR_p);                   
                                                                                                                                     
   if ( Error[0] >  1.e-4 ) printf("pWettedArea_pAlpha: Exact: %12.6f ... FD: %12.6f <------------- \n",   pComponentWettedAreapAlpha(1), pWettedArea_p);                   
   if ( Error[1] >  1.e-4 ) printf("pLength_pAlpha:     Exact: %12.6f ... FD: %12.6f <------------- \n",       pComponentLengthpAlpha(1),     pLength_p);                   
   if ( Error[2] >  1.e-4 ) printf("pFR_pAlpha:         Exact: %12.6f ... FD: %12.6f <------------- \n",pComponentFineNessRatiopAlpha(1),         pFR_p);                   


   // Gradients wrt Beta
      
   A = Alpha;
   B = Beta - dBeta;

   CalculateThickComponentAeroGeometry(A, B);
   
      WettedArea[0] = ComponentWettedArea(1);
          Length[0] = ComponentLength(1);
              FR[0] = ComponentFineNessRatio(1);

   A = Alpha;
   B = Beta + dBeta;

   CalculateThickComponentAeroGeometry(A, B);
   
   WettedArea[1] = ComponentWettedArea(1);
       Length[1] = ComponentLength(1);
           FR[1] = ComponentFineNessRatio(1);      
           
   pWettedArea_p = ( WettedArea[1] - WettedArea[0] )/(2.*dBeta);           
       pLength_p = (     Length[1] -     Length[0] )/(2.*dBeta);           
           pFR_p = (         FR[1] -         FR[0] )/(2.*dBeta);     
           
   Error[0] = ABS(   pComponentWettedAreapAlpha(1) -  pWettedArea_p);                   
   Error[1] = ABS(       pComponentLengthpAlpha(1) -      pLength_p);                   
   Error[2] = ABS(pComponentFineNessRatiopAlpha(1) -          pFR_p);    
                 
   if ( Error[0] <= 1.e-4 ) printf("pWettedArea_pBeta: Exact: %12.6f ... FD: %12.6f \n",                  pComponentWettedAreapBeta(1), pWettedArea_p);                   
   if ( Error[1] <= 1.e-4 ) printf("pLength_pBeta:     Exact: %12.6f ... FD: %12.6f \n",                      pComponentLengthpBeta(1),     pLength_p);                   
   if ( Error[2] <= 1.e-4 ) printf("pFR_pBeta:         Exact: %12.6f ... FD: %12.6f \n",               pComponentFineNessRatiopBeta(1),         pFR_p);                   
                                                                                                                                   
   if ( Error[0] >  1.e-4 ) printf("pWettedArea_pBeta: Exact: %12.6f ... FD: %12.6f <------------- \n",   pComponentWettedAreapBeta(1), pWettedArea_p);                   
   if ( Error[1] >  1.e-4 ) printf("pLength_pBeta:     Exact: %12.6f ... FD: %12.6f <------------- \n",       pComponentLengthpBeta(1),     pLength_p);                   
   if ( Error[2] >  1.e-4 ) printf("pFR_pBeta:         Exact: %12.6f ... FD: %12.6f <------------- \n",pComponentFineNessRatiopBeta(1),         pFR_p);                   

   
   // Gradients wrt mesh
   
   dx = dy = dz = 1.e-7;
   
   // Component to test... basically assumes we just have 1 component... ;-) 
   
   c = 1;
   
   for ( i = 1 ; i <= Grid(0).NumberOfSurfaceNodes() ; i++ ) {
      
      // X
      
      x = Grid(0).NodeList(i).x();
      
      Grid(0).NodeList(i).x() = x - dx;
      
      UpdateSurfacesMeshes();
      
      CalculateThickComponentAeroGeometry(Alpha, Beta);
      
      WettedArea[0] = ComponentWettedArea(1);
          Length[0] = ComponentLength(1);
              FR[0] = ComponentFineNessRatio(1);
      
      Grid(0).NodeList(i).x() = x + dx;
      
      UpdateSurfacesMeshes();
      
      CalculateThickComponentAeroGeometry(Alpha, Beta);
      
      WettedArea[1] = ComponentWettedArea(1);
          Length[1] = ComponentLength(1);
              FR[1] = ComponentFineNessRatio(1);    
              
      pWettedArea_p = ( WettedArea[1] - WettedArea[0] )/(2.*dx);           
          pLength_p = (     Length[1] -     Length[0] )/(2.*dx);           
              pFR_p = (         FR[1] -         FR[0] )/(2.*dx);      
              
      Grid(0).NodeList(i).x() = x;
           
      UpdateSurfacesMeshes();
      
      Error[0] = ABS(   pComponentWettedAreapMesh(1,i).px() -  pWettedArea_p);                   
      Error[1] = ABS(       pComponentLengthpMesh(1,i).px() -      pLength_p);                   
      Error[2] = ABS(pComponentFineNessRatiopMesh(1,i).px() -          pFR_p);    
                    
      if ( Error[0] <= 1.e-4 ) printf("Node: %d --> pWettedArea_pX: Exact: %12.6f ... FD: %12.6f \n",               i,   pComponentWettedAreapMesh(1,i).px(), pWettedArea_p);                   
      if ( Error[1] <= 1.e-4 ) printf("Node: %d --> pLength_pX:     Exact: %12.6f ... FD: %12.6f \n",               i,       pComponentLengthpMesh(1,i).px(),     pLength_p);                   
      if ( Error[2] <= 1.e-4 ) printf("Node: %d --> pFR_pX:         Exact: %12.6f ... FD: %12.6f \n",               i,pComponentFineNessRatiopMesh(1,i).px(),         pFR_p);                   
                                                                                                                                                  
      if ( Error[0] >  1.e-4 ) printf("Node: %d --> pWettedArea_pX: Exact: %12.6f ... FD: %12.6f <------------- \n",i,   pComponentWettedAreapMesh(1,i).px(), pWettedArea_p);                   
      if ( Error[1] >  1.e-4 ) printf("Node: %d --> pLength_pX:     Exact: %12.6f ... FD: %12.6f <------------- \n",i,       pComponentLengthpMesh(1,i).px(),     pLength_p);                   
      if ( Error[2] >  1.e-4 ) printf("Node: %d --> pFR_pX:         Exact: %12.6f ... FD: %12.6f <------------- \n",i,pComponentFineNessRatiopMesh(1,i).px(),         pFR_p);                   


      // Y
      
      y = Grid(0).NodeList(i).y();
      
      Grid(0).NodeList(i).y() = y - dy;
      
      UpdateSurfacesMeshes();
      
      CalculateThickComponentAeroGeometry(Alpha, Beta);
      
      WettedArea[0] = ComponentWettedArea(1);
          Length[0] = ComponentLength(1);
              FR[0] = ComponentFineNessRatio(1);
      
      Grid(0).NodeList(i).y() = y + dy;
      
      UpdateSurfacesMeshes();
      
      CalculateThickComponentAeroGeometry(Alpha, Beta);
      
      WettedArea[1] = ComponentWettedArea(1);
          Length[1] = ComponentLength(1);
              FR[1] = ComponentFineNessRatio(1);    
              
      pWettedArea_p = ( WettedArea[1] - WettedArea[0] )/(2.*dy);           
          pLength_p = (     Length[1] -     Length[0] )/(2.*dy);           
              pFR_p = (         FR[1] -         FR[0] )/(2.*dy);      
              
      Grid(0).NodeList(i).y() = y;
           
      UpdateSurfacesMeshes();
      
      Error[0] = ABS(   pComponentWettedAreapMesh(1,i).py() -  pWettedArea_p);                   
      Error[1] = ABS(       pComponentLengthpMesh(1,i).py() -      pLength_p);                   
      Error[2] = ABS(pComponentFineNessRatiopMesh(1,i).py() -          pFR_p);    
                    
      if ( Error[0] <= 1.e-4 ) printf("Node: %d --> pWettedArea_pY: Exact: %12.6f ... FD: %12.6f \n",               i,   pComponentWettedAreapMesh(1,i).py(), pWettedArea_p);                   
      if ( Error[1] <= 1.e-4 ) printf("Node: %d --> pLength_pY:     Exact: %12.6f ... FD: %12.6f \n",               i,       pComponentLengthpMesh(1,i).py(),     pLength_p);                   
      if ( Error[2] <= 1.e-4 ) printf("Node: %d --> pFR_pY:         Exact: %12.6f ... FD: %12.6f \n",               i,pComponentFineNessRatiopMesh(1,i).py(),         pFR_p);                   
                                                                                                                                                   
      if ( Error[0] >  1.e-4 ) printf("Node: %d --> pWettedArea_pY: Exact: %12.6f ... FD: %12.6f <------------- \n",i,   pComponentWettedAreapMesh(1,i).py(), pWettedArea_p);                   
      if ( Error[1] >  1.e-4 ) printf("Node: %d --> pLength_pY:     Exact: %12.6f ... FD: %12.6f <------------- \n",i,       pComponentLengthpMesh(1,i).py(),     pLength_p);                   
      if ( Error[2] >  1.e-4 ) printf("Node: %d --> pFR_pY:         Exact: %12.6f ... FD: %12.6f <------------- \n",i,pComponentFineNessRatiopMesh(1,i).py(),         pFR_p);                   


      // Z
      
      z = Grid(0).NodeList(i).z();
      
      Grid(0).NodeList(i).z() = z - dz;
      
      UpdateSurfacesMeshes();
      
      CalculateThickComponentAeroGeometry(Alpha, Beta);
      
      WettedArea[0] = ComponentWettedArea(1);
          Length[0] = ComponentLength(1);
              FR[0] = ComponentFineNessRatio(1);
      
      Grid(0).NodeList(i).z() = z + dz;
      
      UpdateSurfacesMeshes();
      
      CalculateThickComponentAeroGeometry(Alpha, Beta);
      
      WettedArea[1] = ComponentWettedArea(1);
          Length[1] = ComponentLength(1);
              FR[1] = ComponentFineNessRatio(1);    
              
      pWettedArea_p = ( WettedArea[1] - WettedArea[0] )/(2.*dz);           
          pLength_p = (     Length[1] -     Length[0] )/(2.*dz);           
              pFR_p = (         FR[1] -         FR[0] )/(2.*dz);      
              
      Grid(0).NodeList(i).z() = z;
           
      UpdateSurfacesMeshes();
      
      Error[0] = ABS(   pComponentWettedAreapMesh(1,i).pz() -  pWettedArea_p);                   
      Error[1] = ABS(       pComponentLengthpMesh(1,i).pz() -      pLength_p);                   
      Error[2] = ABS(pComponentFineNessRatiopMesh(1,i).pz() -          pFR_p);    
                    
      if ( Error[0] <= 1.e-4 ) printf("Node: %d --> pWettedArea_pZ: Exact: %12.6f ... FD: %12.6f \n",               i,   pComponentWettedAreapMesh(1,i).pz(), pWettedArea_p);                   
      if ( Error[1] <= 1.e-4 ) printf("Node: %d --> pLength_pZ:     Exact: %12.6f ... FD: %12.6f \n",               i,       pComponentLengthpMesh(1,i).pz(),     pLength_p);                   
      if ( Error[2] <= 1.e-4 ) printf("Node: %d --> pFR_pZ:         Exact: %12.6f ... FD: %12.6f \n",               i,pComponentFineNessRatiopMesh(1,i).pz(),         pFR_p);                   
                                                                                                                                                  
      if ( Error[0] >  1.e-4 ) printf("Node: %d --> pWettedArea_pZ: Exact: %12.6f ... FD: %12.6f <------------- \n",i,   pComponentWettedAreapMesh(1,i).pz(), pWettedArea_p);                   
      if ( Error[1] >  1.e-4 ) printf("Node: %d --> pLength_pZ:     Exact: %12.6f ... FD: %12.6f <------------- \n",i,       pComponentLengthpMesh(1,i).pz(),     pLength_p);                   
      if ( Error[2] >  1.e-4 ) printf("Node: %d --> pFR_pZ:         Exact: %12.6f ... FD: %12.6f <------------- \n",i,pComponentFineNessRatiopMesh(1,i).pz(),         pFR_p);                   

       
   }
   
   fflush(NULL);exit(1);

}

/*##############################################################################
#                                                                              #
#            VSP_GEOM CalculateThickComponentAeroGeometry                      #
#                                                                              #
##############################################################################*/

void VSP_GEOM::CalculateThickComponentAeroGeometry(double Alpha, double Beta)
{

    int i, Component;
    double CA, SA, CB, SB, FreeStreamDirection[3];
    double X, Y, Z, Xp, Yp, Zp;
    double Dot, ProjectedArea, WettedArea, Length, Diameter, FR;
    double Xmin, Xmax, Ymin, Ymax, Zmin, Zmax;
        
    if ( ComponentWettedArea_    != NULL ) delete [] ComponentWettedArea_;
    if ( ComponentLength_        != NULL ) delete [] ComponentLength_;
    if ( ComponentFineNessRatio_ != NULL ) delete [] ComponentFineNessRatio_;

    ComponentWettedArea_    = new double[NumberOfComponents_ + 1];
    ComponentLength_        = new double[NumberOfComponents_ + 1];
    ComponentFineNessRatio_ = new double[NumberOfComponents_ + 1];
    
    CA = cos(Alpha);
    SA = sin(Alpha);

    CB = cos(Beta);
    SB = sin(Beta);
    
    FreeStreamDirection[0] = CA*CB;
    FreeStreamDirection[1] =   -SB;
    FreeStreamDirection[2] = SA*CB;
    
    for ( Component = 1 ; Component <= NumberOfComponents_ ; Component++ ) {
 
       if ( !ComponentIsLifting_[Component] ) {

          // Calculate length
   
          Xmin = 1.e9;
          Xmax = -Xmin;
          
          for ( i = 1 ; i <= Grid(SolveOnMGLevel_).NumberOfSurfaceNodes() ; i++ ) {
             
             if ( Grid(SolveOnMGLevel_).NodeList(i).ComponentID() == Component ) {
                
                // Rotate geometry so that X lies along the free stream direction
                
                X = Grid(SolveOnMGLevel_).NodeList(i).x();
                Y = Grid(SolveOnMGLevel_).NodeList(i).y();
                Z = Grid(SolveOnMGLevel_).NodeList(i).z();
                
                Xp = ( X * CA + Z * SA ) * CB - Y * SB;
                Yp = ( X * CA + Z * SA ) * SB + Y * CB;
                Zp = (-X * SA + Z * CA );
                
                if ( Xp <= Xmin ) {
                   
                   Xmin = Xp;
                   
                  // iMinX = i;
                   
                }
                
                if ( Xp >= Xmax ) {
                   
                   Xmax = Xp;
                   
                //   iMaxX = i;
                   
                }    
   
             }
             
          }
          
          Length = Xmax - Xmin;
 
          ProjectedArea = WettedArea = 0.;
          
          // Calculate the projected and wetted area
          
          for ( i = 1 ; i <= Grid(SolveOnMGLevel_).NumberOfSurfaceLoops() ; i++ ) {
             
             if ( Grid(SolveOnMGLevel_).LoopList(i).ComponentID() == Component ) {
                
                WettedArea += Grid(SolveOnMGLevel_).LoopList(i).Area();
                
                if ( ComponentIsThick_[Component] ) {         

                   Dot = vector_dot(FreeStreamDirection,Grid(SolveOnMGLevel_).LoopList(i).Normal());
                   
                   if ( Dot < 0. ) {
                      
                      ProjectedArea -= Dot*Grid(SolveOnMGLevel_).LoopList(i).Area();
                      
                   }
                   
                }
      
             }
             
          }
          
          FR = 1.e9;
          
          if ( ComponentIsThick_[Component] ) {         
             
             Diameter = sqrt(4.*ProjectedArea/PI);
             
             // Calculate fineness ratio
             
             FR = Length / Diameter;
             
          }
          
             ComponentWettedArea_[Component] = WettedArea;
                 ComponentLength_[Component] = Length;
          ComponentFineNessRatio_[Component] = FR;

       }
       
       else {
          
             ComponentWettedArea_[Component] = 0.;
                 ComponentLength_[Component] = 0.;
          ComponentFineNessRatio_[Component] = 0.;
          
       }
              
    }

}

/*##############################################################################
#                                                                              #
#        VSP_GEOM CalculateThickComponentAeroGeometryGradientswrtMesh          #
#                                                                              #
##############################################################################*/

void VSP_GEOM::CalculateThickComponentAeroGeometryGradientswrtMesh(double Alpha, double Beta)
{

    int i, p, Node, Component, iMinX, iMinY, iMinZ, iMaxX, iMaxY, iMaxZ;
    double CA, SA, CB, SB, FreeStreamDirection[3];
    double X, Y, Z, Xp, Yp, Zp;
    double Dot, ProjectedArea, WettedArea, Length, Diameter, FR;
    double Xmin, Xmax, Ymin, Ymax, Zmin, Zmax;
    
    double pWettedArea_pArea;
    double pDot_pFreeStreamDirection[3];
    double pDot_pNormal[3];
    double pProjectedArea_pDot;
    double pProjectedArea_pArea;
    double pDiameter_pProjectedArea;
    double pFR_pLength;
    double pFR_pDiameter;
             
    double pXmin_pX;
    double pXmin_pY;
    double pXmin_pZ;

    double pXmax_pX;
    double pXmax_pY;
    double pXmax_pZ;
    
    double pYmin_pX;
    double pYmin_pY;
    double pYmin_pZ;
                 
    double pYmax_pX;
    double pYmax_pY;
    double pYmax_pZ;
                  
    double pZmin_pX;
    double pZmin_pY;
    double pZmin_pZ;
               
    double pZmax_pX;
    double pZmax_pY;
    double pZmax_pZ;
              
    double pLength_pXmin;
    double pLength_pXmax;

    double pDiameter_pYmin;
    double pDiameter_pYmax;
                        
    double pDiameter_pZmin;
    double pDiameter_pZmax;
             
    double pWettedArea_pLength;             
    double pWettedArea_pDiameter;    
                          
    double pXmin_pAlpha;
    double pXmax_pAlpha;
    
    double pXmin_pBeta;
    double pXmax_pBeta;
    
    double pYmin_pAlpha;
    double pYmax_pAlpha;
            
    double pYmin_pBeta;
    double pYmax_pBeta;
    
    double pZmin_pAlpha;
    double pZmax_pAlpha;
           
    double pZmin_pBeta;
    double pZmax_pBeta;       
    
    double pFreeStreamDirection_pAlpha[3];
    double pFreeStreamDirection_pBeta[3];
     
    
    double *NormalGradients;
    double *AreaGradients;
    
    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {
       
       if ( !ComponentIsLifting_[i] ) {
          
              if ( pComponentWettedAreapMesh_    != NULL ) delete [] pComponentWettedAreapMesh_[i];
              if ( pComponentLengthpMesh_        != NULL ) delete [] pComponentLengthpMesh_[i];
              if ( pComponentFineNessRatiopMesh_ != NULL ) delete [] pComponentFineNessRatiopMesh_[i];    
    
       }
       
    }
        
    if ( pComponentWettedAreapMesh_    != NULL ) delete [] pComponentWettedAreapMesh_;
    if ( pComponentLengthpMesh_        != NULL ) delete [] pComponentLengthpMesh_;
    if ( pComponentFineNessRatiopMesh_ != NULL ) delete [] pComponentFineNessRatiopMesh_;    

    pComponentWettedAreapMesh_    = new MESH_GRADIENT*[NumberOfComponents_ + 1];
    pComponentLengthpMesh_        = new MESH_GRADIENT*[NumberOfComponents_ + 1];
    pComponentFineNessRatiopMesh_ = new MESH_GRADIENT*[NumberOfComponents_ + 1];
    
    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {
       
       if ( !ComponentIsLifting_[i] ) {
              
          pComponentWettedAreapMesh_[i]    = new MESH_GRADIENT[Grid(SolveOnMGLevel_).NumberOfSurfaceNodes() + 1];
          pComponentLengthpMesh_[i]        = new MESH_GRADIENT[Grid(SolveOnMGLevel_).NumberOfSurfaceNodes() + 1];
          pComponentFineNessRatiopMesh_[i] = new MESH_GRADIENT[Grid(SolveOnMGLevel_).NumberOfSurfaceNodes() + 1];
          
       }
       
    }
    
    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {
    
       for ( p = 1 ; p <= Grid(SolveOnMGLevel_).NumberOfSurfaceNodes() ; p++ ) {
          
          if ( !ComponentIsLifting_[i] ) {
             
                pComponentWettedAreapMesh_[i][p].Zero();
                    pComponentLengthpMesh_[i][p].Zero();
             pComponentFineNessRatiopMesh_[i][p].Zero();
                
          }
       
       }
       
    }
    
    if ( pComponentWettedAreapAlpha_    != NULL ) delete [] pComponentWettedAreapAlpha_;
    if ( pComponentWettedAreapBeta_     != NULL ) delete [] pComponentWettedAreapBeta_;
                              
    if ( pComponentLengthpAlpha_        != NULL ) delete [] pComponentLengthpAlpha_;
    if ( pComponentLengthpBeta_         != NULL ) delete [] pComponentLengthpBeta_; 
                               
    if ( pComponentFineNessRatiopAlpha_ != NULL ) delete [] pComponentFineNessRatiopAlpha_;
    if ( pComponentFineNessRatiopBeta_  != NULL ) delete [] pComponentFineNessRatiopBeta_;
        
    pComponentWettedAreapAlpha_    = new double[NumberOfComponents_ + 1];
    pComponentWettedAreapBeta_     = new double[NumberOfComponents_ + 1];
                                   
    pComponentLengthpAlpha_        = new double[NumberOfComponents_ + 1];
    pComponentLengthpBeta_         = new double[NumberOfComponents_ + 1]; 
    
    pComponentFineNessRatiopAlpha_ = new double[NumberOfComponents_ + 1];
    pComponentFineNessRatiopBeta_  = new double[NumberOfComponents_ + 1];   
    
    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {

       pComponentWettedAreapAlpha_[i]    = 0.;
       pComponentWettedAreapBeta_[i]     = 0.;
                                         
       pComponentLengthpAlpha_[i]        = 0.;
       pComponentLengthpBeta_[i]         = 0.;
       
       pComponentFineNessRatiopAlpha_[i] = 0.;
       pComponentFineNessRatiopBeta_[i]  = 0.; 
    
    }
            
    CA = cos(Alpha);
    SA = sin(Alpha);

    CB = cos(Beta);
    SB = sin(Beta);
    
    FreeStreamDirection[0] = CA*CB;
    FreeStreamDirection[1] =   -SB;
    FreeStreamDirection[2] = SA*CB;
    
    pFreeStreamDirection_pAlpha[0] = -SA*CB;    
    pFreeStreamDirection_pAlpha[1] =     0.;
    pFreeStreamDirection_pAlpha[2] =  CA*CB;

    pFreeStreamDirection_pBeta[0] = -CA*SB; 
    pFreeStreamDirection_pBeta[1] =    -CB;
    pFreeStreamDirection_pBeta[2] = -SA*SB;
    
    for ( Component = 1 ; Component <= NumberOfComponents_ ; Component++ ) {
 
       if ( !ComponentIsLifting_[Component] ) {

          // Thick component
          
          // Calculate the projected area, wetted area
   
          ProjectedArea = WettedArea = 0.;
          
          for ( i = 1 ; i <= Grid(SolveOnMGLevel_).NumberOfLoops() ; i++ ) {
             
             if ( Grid(SolveOnMGLevel_).LoopList(i).ComponentID() == Component ) {
                
                WettedArea += Grid(SolveOnMGLevel_).LoopList(i).Area();
                
                if ( ComponentIsThick_[Component] ) {
                   
                   Dot = vector_dot(FreeStreamDirection,Grid(SolveOnMGLevel_).LoopList(i).Normal());
                   
                   if ( Dot <= 0. ) {
                      
                      ProjectedArea -= Dot*Grid(SolveOnMGLevel_).LoopList(i).Area();
                      
                   }
                   
                }
      
             }
             
          }
          
          // Calculate diameter
          
          Diameter = pDiameter_pProjectedArea = 0.;
          
          if ( ComponentIsThick_[Component] ) {
          
             Diameter = sqrt(4.*ProjectedArea/PI);
             
             pDiameter_pProjectedArea = (2./PI)/sqrt(4.*ProjectedArea/PI);
             
          }
       
          // Calculate length
          
          Xmin = 1.e9;
          Xmax = -Xmax;

          iMinX = iMaxX = 0;
          
          for ( i = 1 ; i <= Grid(SolveOnMGLevel_).NumberOfNodes() ; i++ ) {
             
             if ( Grid(SolveOnMGLevel_).NodeList(i).ComponentID() == Component ) {
                
                // Rotate geometry so that X lies along the free stream direction
                
                X = Grid(SolveOnMGLevel_).NodeList(i).x();
                Y = Grid(SolveOnMGLevel_).NodeList(i).y();
                Z = Grid(SolveOnMGLevel_).NodeList(i).z();
                
                Xp = ( X * CA + Z * SA ) * CB - Y * SB;
                Yp = ( X * CA + Z * SA ) * SB + Y * CB;
                Zp = (-X * SA + Z * CA );
                
                if ( Xp <= Xmin ) {
                   
                   Xmin = Xp;
                   
                   iMinX = i;
                   
                }
                
                if ( Xp >= Xmax ) {
                   
                   Xmax = Xp;
                   
                   iMaxX = i;
                   
                }                   

             }
             
          }
          
          X = Grid(SolveOnMGLevel_).NodeList(iMinX).x();
          Y = Grid(SolveOnMGLevel_).NodeList(iMinX).y();
          Z = Grid(SolveOnMGLevel_).NodeList(iMinX).z();
          
          Xmin = ( X * CA + Z * SA ) * CB - Y * SB;
          
          pXmin_pX =  CA * CB;
          pXmin_pY = -SB;
          pXmin_pZ =  SA * CB;
          
          pXmin_pAlpha = ( -X * SA + Z * CA ) * CB;

          pXmin_pBeta = -( X * CA + Z * SA ) * SB - Y * CB;

          X = Grid(SolveOnMGLevel_).NodeList(iMaxX).x();
          Y = Grid(SolveOnMGLevel_).NodeList(iMaxX).y();
          Z = Grid(SolveOnMGLevel_).NodeList(iMaxX).z();
          
          Xmax = ( X * CA + Z * SA ) * CB - Y * SB;                   
                             
          pXmax_pX =  CA * CB;
          pXmax_pY = -SB;
          pXmax_pZ =  SA * CB;      
                      
          pXmax_pAlpha = ( -X * SA + Z * CA ) * CB;
                 
          pXmax_pBeta = -( X * CA + Z * SA ) * SB - Y * CB;
                              
          Length = Xmax - Xmin;
          
          //printf("iMinX, iMaxX: %d %d \n",iMinX,iMaxX);
          //printf("Xmin, Xmax: %f %f \n",Xmin,Xmax);
          //printf("Length: %f ... ComponentLength(1): %f \n",Length,ComponentLength(1));
          
          pLength_pXmax =  1.;
          
          pLength_pXmin = -1.;
          
          pComponentLengthpMesh_[Component][iMinX].px() = pLength_pXmin * pXmin_pX;
          pComponentLengthpMesh_[Component][iMinX].py() = pLength_pXmin * pXmin_pY;
          pComponentLengthpMesh_[Component][iMinX].pz() = pLength_pXmin * pXmin_pZ;
                               
          pComponentLengthpMesh_[Component][iMaxX].px() = pLength_pXmax * pXmax_pX;
          pComponentLengthpMesh_[Component][iMaxX].py() = pLength_pXmax * pXmax_pY;
          pComponentLengthpMesh_[Component][iMaxX].pz() = pLength_pXmax * pXmax_pZ;
          
          pComponentLengthpAlpha_[Component] = pLength_pXmin * pXmin_pAlpha + pLength_pXmax * pXmax_pAlpha;
                       
          pComponentLengthpBeta_[Component] = pLength_pXmin * pXmin_pBeta + pLength_pXmax * pXmax_pBeta;
                       
          // Calculate fineness ratio
          
          FR = 1.e9;
          
          if ( ComponentIsThick_[Component] ) {
             
             FR = Length / Diameter;
             
             //printf("FR: %f ... ComponentFineNessRatio(1): %f \n",FR,ComponentFineNessRatio(1));fflush(NULL);
            
             pFR_pLength = 1./Diameter;
             
             pFR_pDiameter = -Length/(Diameter*Diameter);
             
             pComponentFineNessRatiopMesh_[Component][iMinX].px() += pFR_pLength * pLength_pXmin * pXmin_pX;
             pComponentFineNessRatiopMesh_[Component][iMinX].py() += pFR_pLength * pLength_pXmin * pXmin_pY;
             pComponentFineNessRatiopMesh_[Component][iMinX].pz() += pFR_pLength * pLength_pXmin * pXmin_pZ;
                                         
             pComponentFineNessRatiopMesh_[Component][iMaxX].px() += pFR_pLength * pLength_pXmax * pXmax_pX;
             pComponentFineNessRatiopMesh_[Component][iMaxX].py() += pFR_pLength * pLength_pXmax * pXmax_pY;
             pComponentFineNessRatiopMesh_[Component][iMaxX].pz() += pFR_pLength * pLength_pXmax * pXmax_pZ;
             
             pComponentFineNessRatiopAlpha_[Component] += pFR_pLength * pComponentLengthpAlpha_[Component];
             
             pComponentFineNessRatiopBeta_[Component] += pFR_pLength * pComponentLengthpBeta_[Component];
             
          }
                     
          // Calculate the projected and wetted area gradients
          
          ProjectedArea = WettedArea = 0.;
          
          for ( i = 1 ; i <= Grid(SolveOnMGLevel_).NumberOfSurfaceLoops() ; i++ ) {
             
             if ( Grid(SolveOnMGLevel_).LoopList(i).ComponentID() == Component ) {
                
                NormalGradients = new double[9*Grid(SolveOnMGLevel_).LoopList(i).NumberOfNodes() + 1];
                
                AreaGradients = new double[3*Grid(SolveOnMGLevel_).LoopList(i).NumberOfNodes() + 1];
                
                CalculateLoopNormalGradients_wrt_Mesh(SolveOnMGLevel_, i, NormalGradients, AreaGradients);
                      
                WettedArea += Grid(SolveOnMGLevel_).LoopList(i).Area();
                
                pWettedArea_pArea = 1.;
                
                for ( p = 1 ; p <= Grid(SolveOnMGLevel_).LoopList(i).NumberOfNodes() ; p++ ) {
                   
                   Node = Grid(SolveOnMGLevel_).LoopList(i).Node(p);
                                         
                   pComponentWettedAreapMesh_[Component][Node].px() += pWettedArea_pArea * AreaGradients[3*p-2];
                   pComponentWettedAreapMesh_[Component][Node].py() += pWettedArea_pArea * AreaGradients[3*p-1];
                   pComponentWettedAreapMesh_[Component][Node].pz() += pWettedArea_pArea * AreaGradients[3*p  ];
                   
                }
                                            
                if ( ComponentIsThick_[Component] ) {
   
                   Dot = vector_dot(FreeStreamDirection,Grid(SolveOnMGLevel_).LoopList(i).Normal());
                   
                   pDot_pFreeStreamDirection[0] = Grid(SolveOnMGLevel_).LoopList(i).Normal()[0];
                   pDot_pFreeStreamDirection[1] = Grid(SolveOnMGLevel_).LoopList(i).Normal()[1];
                   pDot_pFreeStreamDirection[2] = Grid(SolveOnMGLevel_).LoopList(i).Normal()[2];
                   
                   pDot_pNormal[0] = FreeStreamDirection[0];
                   pDot_pNormal[1] = FreeStreamDirection[1];
                   pDot_pNormal[2] = FreeStreamDirection[2];
                   
                   if ( Dot < 0. ) {
                      
                      ProjectedArea -= Dot*Grid(SolveOnMGLevel_).LoopList(i).Area();
                      
                      pProjectedArea_pDot = -Grid(SolveOnMGLevel_).LoopList(i).Area();
                      
                      pProjectedArea_pArea = -Dot;
                      
                      for ( p = 1 ; p <= Grid(SolveOnMGLevel_).LoopList(i).NumberOfNodes() ; p++ ) {
                         
                         Node = Grid(SolveOnMGLevel_).LoopList(i).Node(p);
                         
                         // wrt x
                         
                         pComponentFineNessRatiopMesh_[Component][Node].px() += pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pArea *                      AreaGradients[3*p - 2]
                                                                              + pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pDot  * pDot_pNormal[0] *  NormalGradients[9*p - 8]
                                                                              + pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pDot  * pDot_pNormal[1] *  NormalGradients[9*p - 5]
                                                                              + pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pDot  * pDot_pNormal[2] *  NormalGradients[9*p - 2];
                         
                         // wrt y
                         
                         pComponentFineNessRatiopMesh_[Component][Node].py() += pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pArea *                      AreaGradients[3*p - 1]
                                                                              + pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pDot  * pDot_pNormal[0] *  NormalGradients[9*p - 7]
                                                                              + pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pDot  * pDot_pNormal[1] *  NormalGradients[9*p - 4]
                                                                              + pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pDot  * pDot_pNormal[2] *  NormalGradients[9*p - 1];
   
   
                         // wrt z
                         
                         pComponentFineNessRatiopMesh_[Component][Node].pz() += pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pArea *                      AreaGradients[3*p    ]
                                                                              + pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pDot  * pDot_pNormal[0] *  NormalGradients[9*p - 6]
                                                                              + pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pDot  * pDot_pNormal[1] *  NormalGradients[9*p - 3]
                                                                              + pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pDot  * pDot_pNormal[2] *  NormalGradients[9*p    ];
   
                      }
                      
                      pComponentFineNessRatiopAlpha_[Component] += pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pDot * pDot_pFreeStreamDirection[0] * pFreeStreamDirection_pAlpha[0]
                                                                 + pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pDot * pDot_pFreeStreamDirection[1] * pFreeStreamDirection_pAlpha[1]
                                                                 + pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pDot * pDot_pFreeStreamDirection[2] * pFreeStreamDirection_pAlpha[2];
   
                      pComponentFineNessRatiopBeta_[Component] += pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pDot * pDot_pFreeStreamDirection[0] * pFreeStreamDirection_pBeta[0]
                                                                + pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pDot * pDot_pFreeStreamDirection[1] * pFreeStreamDirection_pBeta[1]
                                                                + pFR_pDiameter * pDiameter_pProjectedArea * pProjectedArea_pDot * pDot_pFreeStreamDirection[2] * pFreeStreamDirection_pBeta[2];
   
                   }
                   
                }
      
                delete [] AreaGradients;
                delete [] NormalGradients;
                     
             }
             
          }
      
       }
              
    }

}

/*##############################################################################
#                                                                              #
#            VSP_GEOM   CalculateLoopNormalGradients_wrt_Mesh                  #
#                                                                              #
##############################################################################*/

void VSP_GEOM::CalculateLoopNormalGradients_wrt_Mesh(int Level, int Loop, double *NormalGradients, double *AreaGradients)
{
   
    int i, j, Tri, Node1, Node2, Node3, Found;
    
    if ( Level != SolveOnMGLevel_ ) {
       
       printf("Loop normal gradients for meshes other than MGLevel_ not working yet! \n");
       fflush(NULL);
       
    }
    
    double Normal[3], Nmag;
    
    double dNx_dx1;
    double dNx_dy1;
    double dNx_dz1;
                         
    double dNy_dx1;
    double dNy_dy1;
    double dNy_dz1;
                         
    double dNz_dx1;
    double dNz_dy1;
    double dNz_dz1;
                       
    double dNx_dx2;
    double dNx_dy2;
    double dNx_dz2;
                         
    double dNy_dx2;
    double dNy_dy2;
    double dNy_dz2;
                          
    double dNz_dx2;
    double dNz_dy2;
    double dNz_dz2;    
                         
    double dNx_dx3;
    double dNx_dy3;
    double dNx_dz3;
                        
    double dNy_dx3;
    double dNy_dy3;
    double dNy_dz3;
                         
    double dNz_dx3;
    double dNz_dy3;
    double dNz_dz3;

    double dNx_Normalized_dx1;
    double dNx_Normalized_dy1;
    double dNx_Normalized_dz1; 
                                     
    double dNy_Normalized_dx1;
    double dNy_Normalized_dy1;
    double dNy_Normalized_dz1; 
                                     
    double dNz_Normalized_dx1;
    double dNz_Normalized_dy1;
    double dNz_Normalized_dz1;
                                     
    double dNx_Normalized_dx2;
    double dNx_Normalized_dy2;
    double dNx_Normalized_dz2; 
                                     
    double dNy_Normalized_dx2;
    double dNy_Normalized_dy2;
    double dNy_Normalized_dz2; 
                                     
    double dNz_Normalized_dx2;
    double dNz_Normalized_dy2;
    double dNz_Normalized_dz2;     
                                     
    double dNx_Normalized_dx3;
    double dNx_Normalized_dy3;
    double dNx_Normalized_dz3; 
                                     
    double dNy_Normalized_dx3;
    double dNy_Normalized_dy3;
    double dNy_Normalized_dz3; 
                                     
    double dNz_Normalized_dx3;
    double dNz_Normalized_dy3;
    double dNz_Normalized_dz3;
                                                
    double dArea_dx1;
    double dArea_dy1;
    double dArea_dz1;            
                           
    double dArea_dx2;
    double dArea_dy2;
    double dArea_dz2;       
                           
    double dArea_dx3;
    double dArea_dy3;
    double dArea_dz3;
                                                          
    // Loop over fine grid tris

    zero_double_array(NormalGradients, 9*Grid(Level).LoopList(Loop).NumberOfNodes());

    zero_double_array(AreaGradients, 3*Grid(Level).LoopList(Loop).NumberOfNodes());

    Nmag = 2.*Grid(Level).LoopList(Loop).Area();
    
    Normal[0] = Nmag * Grid(Level).LoopList(Loop).Nx();
    Normal[1] = Nmag * Grid(Level).LoopList(Loop).Ny();
    Normal[2] = Nmag * Grid(Level).LoopList(Loop).Nz();
           
    for ( i = 1 ; i <= Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
       
       Tri = Grid(Level).LoopList(Loop).FineGridLoop(i);

       Node1 = Grid(0).LoopList(Tri).Node1();
       Node2 = Grid(0).LoopList(Tri).Node2();
       Node3 = Grid(0).LoopList(Tri).Node3();
       
       CalculateTriNormalGradients_wrt_Mesh(Tri,
                                            dNx_dx1,
                                            dNx_dy1,
                                            dNx_dz1, 
                                             
                                            dNy_dx1,
                                            dNy_dy1,
                                            dNy_dz1, 
                                            
                                            dNz_dx1,
                                            dNz_dy1,
                                            dNz_dz1,
                                             
                                            dNx_dx2,
                                            dNx_dy2,
                                            dNx_dz2, 
                                               
                                            dNy_dx2,
                                            dNy_dy2,
                                            dNy_dz2, 
                                              
                                            dNz_dx2,
                                            dNz_dy2,
                                            dNz_dz2,     
                                                
                                            dNx_dx3,
                                            dNx_dy3,
                                            dNx_dz3, 
                                             
                                            dNy_dx3,
                                            dNy_dy3,
                                            dNy_dz3, 
                                            
                                            dNz_dx3,
                                            dNz_dy3,
                                            dNz_dz3,
                                            
                                            dNx_Normalized_dx1,
                                            dNx_Normalized_dy1,
                                            dNx_Normalized_dz1, 
                                               
                                            dNy_Normalized_dx1,
                                            dNy_Normalized_dy1,
                                            dNy_Normalized_dz1, 
                                               
                                            dNz_Normalized_dx1,
                                            dNz_Normalized_dy1,
                                            dNz_Normalized_dz1,
                                               
                                            dNx_Normalized_dx2,
                                            dNx_Normalized_dy2,
                                            dNx_Normalized_dz2, 
                                               
                                            dNy_Normalized_dx2,
                                            dNy_Normalized_dy2,
                                            dNy_Normalized_dz2, 
                                               
                                            dNz_Normalized_dx2,
                                            dNz_Normalized_dy2,
                                            dNz_Normalized_dz2,     
                                                
                                            dNx_Normalized_dx3,
                                            dNx_Normalized_dy3,
                                            dNx_Normalized_dz3, 
                                               
                                            dNy_Normalized_dx3,
                                            dNy_Normalized_dy3,
                                            dNy_Normalized_dz3, 
                                               
                                            dNz_Normalized_dx3,
                                            dNz_Normalized_dy3,
                                            dNz_Normalized_dz3,
                                                                                                  
                                            dArea_dx1,
                                            dArea_dy1,
                                            dArea_dz1,            
                                                    
                                            dArea_dx2,
                                            dArea_dy2,
                                            dArea_dz2,       
                                                    
                                            dArea_dx3,
                                            dArea_dy3,
                                            dArea_dz3);                                         
                        
       Found = 0;
       
       j = 1;
                                            
       while ( Found < 3 && j <= Grid(Level).LoopList(Loop).NumberOfNodes() ) {
                                          
           if ( Grid(Level).LoopList(Loop).Node(j) == Node1 ) {
              
              NormalGradients[9*j - 8] += dNx_dx1/Nmag - Normal[0]*(Normal[0]*dNx_dx1 + Normal[1]*dNy_dx1 + Normal[2]*dNz_dx1)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 7] += dNx_dy1/Nmag - Normal[0]*(Normal[0]*dNx_dy1 + Normal[1]*dNy_dy1 + Normal[2]*dNz_dy1)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 6] += dNx_dz1/Nmag - Normal[0]*(Normal[0]*dNx_dz1 + Normal[1]*dNy_dz1 + Normal[2]*dNz_dz1)/(Nmag*Nmag*Nmag);
                                                                                                                
              NormalGradients[9*j - 5] += dNy_dx1/Nmag - Normal[1]*(Normal[0]*dNx_dx1 + Normal[1]*dNy_dx1 + Normal[2]*dNz_dx1)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 4] += dNy_dy1/Nmag - Normal[1]*(Normal[0]*dNx_dy1 + Normal[1]*dNy_dy1 + Normal[2]*dNz_dy1)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 3] += dNy_dz1/Nmag - Normal[1]*(Normal[0]*dNx_dz1 + Normal[1]*dNy_dz1 + Normal[2]*dNz_dz1)/(Nmag*Nmag*Nmag);
                                                                                                                  
              NormalGradients[9*j - 2] += dNz_dx1/Nmag - Normal[2]*(Normal[0]*dNx_dx1 + Normal[1]*dNy_dx1 + Normal[2]*dNz_dx1)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 1] += dNz_dy1/Nmag - Normal[2]*(Normal[0]*dNx_dy1 + Normal[1]*dNy_dy1 + Normal[2]*dNz_dy1)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 0] += dNz_dz1/Nmag - Normal[2]*(Normal[0]*dNx_dz1 + Normal[1]*dNy_dz1 + Normal[2]*dNz_dz1)/(Nmag*Nmag*Nmag);
              
              AreaGradients[3*j - 2] += dArea_dx1;
              AreaGradients[3*j - 1] += dArea_dy1;
              AreaGradients[3*j - 0] += dArea_dz1;

             // NormalGradients[9*j - 5] += dNx_dx1/Nmag; 
             // NormalGradients[9*j - 4] += dNx_dy1/Nmag; 
             // NormalGradients[9*j - 3] += dNx_dz1/Nmag;  
             //                                                                                                   
             // NormalGradients[9*j - 5] += dNy_dx1/Nmag; 
             // NormalGradients[9*j - 4] += dNy_dy1/Nmag; 
             // NormalGradients[9*j - 3] += dNy_dz1/Nmag;  
             //                                        
             // NormalGradients[9*j - 2] += dNz_dx1/Nmag; 
             // NormalGradients[9*j - 1] += dNz_dy1/Nmag; 
             // NormalGradients[9*j    ] += dNz_dz1/Nmag; 
              
              Found++;
              
           }
           
           if ( Grid(Level).LoopList(Loop).Node(j) == Node2 ) {

              NormalGradients[9*j - 8] += dNx_dx2/Nmag - Normal[0]*(Normal[0]*dNx_dx2 + Normal[1]*dNy_dx2 + Normal[2]*dNz_dx2)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 7] += dNx_dy2/Nmag - Normal[0]*(Normal[0]*dNx_dy2 + Normal[1]*dNy_dy2 + Normal[2]*dNz_dy2)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 6] += dNx_dz2/Nmag - Normal[0]*(Normal[0]*dNx_dz2 + Normal[1]*dNy_dz2 + Normal[2]*dNz_dz2)/(Nmag*Nmag*Nmag);
                                                                                                                  
              NormalGradients[9*j - 5] += dNy_dx2/Nmag - Normal[1]*(Normal[0]*dNx_dx2 + Normal[1]*dNy_dx2 + Normal[2]*dNz_dx2)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 4] += dNy_dy2/Nmag - Normal[1]*(Normal[0]*dNx_dy2 + Normal[1]*dNy_dy2 + Normal[2]*dNz_dy2)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 3] += dNy_dz2/Nmag - Normal[1]*(Normal[0]*dNx_dz2 + Normal[1]*dNy_dz2 + Normal[2]*dNz_dz2)/(Nmag*Nmag*Nmag);
                                                                                                                 
              NormalGradients[9*j - 2] += dNz_dx2/Nmag - Normal[2]*(Normal[0]*dNx_dx2 + Normal[1]*dNy_dx2 + Normal[2]*dNz_dx2)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 1] += dNz_dy2/Nmag - Normal[2]*(Normal[0]*dNx_dy2 + Normal[1]*dNy_dy2 + Normal[2]*dNz_dy2)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 0] += dNz_dz2/Nmag - Normal[2]*(Normal[0]*dNx_dz2 + Normal[1]*dNy_dz2 + Normal[2]*dNz_dz2)/(Nmag*Nmag*Nmag);
                               
              AreaGradients[3*j - 2] += dArea_dx2;
              AreaGradients[3*j - 1] += dArea_dy2;
              AreaGradients[3*j - 0] += dArea_dz2;
                                                                 
            //  NormalGradients[9*j - 8] += dNx_dx2/Nmag;
            //  NormalGradients[9*j - 7] += dNx_dy2/Nmag;
            //  NormalGradients[9*j - 6] += dNx_dz2/Nmag; 
            //                                          
            //  NormalGradients[9*j - 5] += dNy_dx2/Nmag;
            //  NormalGradients[9*j - 4] += dNy_dy2/Nmag;
            //  NormalGradients[9*j - 3] += dNy_dz2/Nmag; 
            //                                          
            //  NormalGradients[9*j - 2] += dNz_dx2/Nmag;
            //  NormalGradients[9*j - 1] += dNz_dy2/Nmag;
            //  NormalGradients[9*j    ] += dNz_dz2/Nmag;
                            
              Found++;
              
           }

           if ( Grid(Level).LoopList(Loop).Node(j) == Node3 ) {
                                        
              NormalGradients[9*j - 8] += dNx_dx3/Nmag - Normal[0]*(Normal[0]*dNx_dx3 + Normal[1]*dNy_dx3 + Normal[2]*dNz_dx3)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 7] += dNx_dy3/Nmag - Normal[0]*(Normal[0]*dNx_dy3 + Normal[1]*dNy_dy3 + Normal[2]*dNz_dy3)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 6] += dNx_dz3/Nmag - Normal[0]*(Normal[0]*dNx_dz3 + Normal[1]*dNy_dz3 + Normal[2]*dNz_dz3)/(Nmag*Nmag*Nmag);
                                                                                                            
              NormalGradients[9*j - 5] += dNy_dx3/Nmag - Normal[1]*(Normal[0]*dNx_dx3 + Normal[1]*dNy_dx3 + Normal[2]*dNz_dx3)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 4] += dNy_dy3/Nmag - Normal[1]*(Normal[0]*dNx_dy3 + Normal[1]*dNy_dy3 + Normal[2]*dNz_dy3)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 3] += dNy_dz3/Nmag - Normal[1]*(Normal[0]*dNx_dz3 + Normal[1]*dNy_dz3 + Normal[2]*dNz_dz3)/(Nmag*Nmag*Nmag);
                                                                                                              
              NormalGradients[9*j - 2] += dNz_dx3/Nmag - Normal[2]*(Normal[0]*dNx_dx3 + Normal[1]*dNy_dx3 + Normal[2]*dNz_dx3)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 1] += dNz_dy3/Nmag - Normal[2]*(Normal[0]*dNx_dy3 + Normal[1]*dNy_dy3 + Normal[2]*dNz_dy3)/(Nmag*Nmag*Nmag);
              NormalGradients[9*j - 0] += dNz_dz3/Nmag - Normal[2]*(Normal[0]*dNx_dz3 + Normal[1]*dNy_dz3 + Normal[2]*dNz_dz3)/(Nmag*Nmag*Nmag);

              AreaGradients[3*j - 2] += dArea_dx3;
              AreaGradients[3*j - 1] += dArea_dy3;
              AreaGradients[3*j - 0] += dArea_dz3;
              
          //    NormalGradients[9*j - 8] += dNx_dx3/Nmag;
          //    NormalGradients[9*j - 7] += dNx_dy3/Nmag;
          //    NormalGradients[9*j - 6] += dNx_dz3/Nmag; 
          //                                    
          //    NormalGradients[9*j - 5] += dNy_dx3/Nmag;
          //    NormalGradients[9*j - 4] += dNy_dy3/Nmag;
          //    NormalGradients[9*j - 3] += dNy_dz3/Nmag; 
          //                                    
          //    NormalGradients[9*j - 2] += dNz_dx3/Nmag;
          //    NormalGradients[9*j - 1] += dNz_dy3/Nmag;
          //    NormalGradients[9*j    ] += dNz_dz3/Nmag;
                            
              Found++;
                                                 
           }
           
           j++;
           
       }
       
       if ( Found != 3 ) {
          
          printf("Error in distributing the normal gradients! \n");
          fflush(NULL);
          exit(1);
          
       }
       
    }

}

/*##############################################################################
#                                                                              #
#            VSP_GEOM CalculateTriNormalGradients_wrt_Mesh                     #
#                                                                              #
##############################################################################*/

void VSP_GEOM::CalculateTriNormalGradients_wrt_Mesh(int Loop,
                                                    double &dNx_dx1,
                                                    double &dNx_dy1,
                                                    double &dNx_dz1, 
                                                                     
                                                    double &dNy_dx1,
                                                    double &dNy_dy1,
                                                    double &dNy_dz1, 
                                                                   
                                                    double &dNz_dx1,
                                                    double &dNz_dy1,
                                                    double &dNz_dz1,
                                                                     
                                                    double &dNx_dx2,
                                                    double &dNx_dy2,
                                                    double &dNx_dz2, 
                                                                       
                                                    double &dNy_dx2,
                                                    double &dNy_dy2,
                                                    double &dNy_dz2, 
                                                                      
                                                    double &dNz_dx2,
                                                    double &dNz_dy2,
                                                    double &dNz_dz2,     
                                                                        
                                                    double &dNx_dx3,
                                                    double &dNx_dy3,
                                                    double &dNx_dz3, 
                                                                     
                                                    double &dNy_dx3,
                                                    double &dNy_dy3,
                                                    double &dNy_dz3, 
                                                                  
                                                    double &dNz_dx3,
                                                    double &dNz_dy3,
                                                    double &dNz_dz3,

                                                    double &dNx_Normalized_dx1,
                                                    double &dNx_Normalized_dy1,
                                                    double &dNx_Normalized_dz1, 
                                                                       
                                                    double &dNy_Normalized_dx1,
                                                    double &dNy_Normalized_dy1,
                                                    double &dNy_Normalized_dz1, 
                                                                       
                                                    double &dNz_Normalized_dx1,
                                                    double &dNz_Normalized_dy1,
                                                    double &dNz_Normalized_dz1,
                                                                       
                                                    double &dNx_Normalized_dx2,
                                                    double &dNx_Normalized_dy2,
                                                    double &dNx_Normalized_dz2, 
                                                                       
                                                    double &dNy_Normalized_dx2,
                                                    double &dNy_Normalized_dy2,
                                                    double &dNy_Normalized_dz2, 
                                                                       
                                                    double &dNz_Normalized_dx2,
                                                    double &dNz_Normalized_dy2,
                                                    double &dNz_Normalized_dz2,     
                                                                        
                                                    double &dNx_Normalized_dx3,
                                                    double &dNx_Normalized_dy3,
                                                    double &dNx_Normalized_dz3, 
                                                                       
                                                    double &dNy_Normalized_dx3,
                                                    double &dNy_Normalized_dy3,
                                                    double &dNy_Normalized_dz3, 
                                                                       
                                                    double &dNz_Normalized_dx3,
                                                    double &dNz_Normalized_dy3,
                                                    double &dNz_Normalized_dz3,
                                                    
                                                    double &dArea_dx1,
                                                    double &dArea_dy1,
                                                    double &dArea_dz1,            
                                                                            
                                                    double &dArea_dx2,
                                                    double &dArea_dy2,
                                                    double &dArea_dz2,       
                                                                            
                                                    double &dArea_dx3,
                                                    double &dArea_dy3,
                                                    double &dArea_dz3)
    {

    int Node1, Node2, Node3;
    double xyz1[3], xyz2[3], xyz3[3], Vec1[3], Vec2[3], Normal[3], Mag, Area;
    
    double dVec1X_dX1;
    double dVec1X_dY1;
    double dVec1X_dZ1;
                   
    double dVec1Y_dX1;
    double dVec1Y_dY1;
    double dVec1Y_dZ1;
                   
    double dVec1Z_dX1;
    double dVec1Z_dY1;
    double dVec1Z_dZ1;
                 
    double dVec1X_dX2;
    double dVec1X_dY2;
    double dVec1X_dZ2;
                 
    double dVec1Y_dX2;
    double dVec1Y_dY2;
    double dVec1Y_dZ2;
                 
    double dVec1Z_dX2;
    double dVec1Z_dY2;
    double dVec1Z_dZ2;
    
    double dVec2X_dX1;
    double dVec2X_dY1;
    double dVec2X_dZ1;
         
    double dVec2Y_dX1;
    double dVec2Y_dY1;
    double dVec2Y_dZ1;
         
    double dVec2Z_dX1;
    double dVec2Z_dY1;
    double dVec2Z_dZ1;
        
    double dVec2X_dX3;
    double dVec2X_dY3;
    double dVec2X_dZ3;
              
    double dVec2Y_dX3;
    double dVec2Y_dY3;
    double dVec2Y_dZ3;
              
    double dVec2Z_dX3;
    double dVec2Z_dY3;
    double dVec2Z_dZ3;

    double u;
    double v;
    double w;
        
    double dNx_dVec1X;
    double dNy_dVec1X;
    double dNz_dVec1X;
     
    double dNx_dVec1Y;
    double dNy_dVec1Y;
    double dNz_dVec1Y;
        
    double dNx_dVec1Z;
    double dNy_dVec1Z;
    double dNz_dVec1Z;
    
    double dNx_dVec2X;
    double dNy_dVec2X;
    double dNz_dVec2X;
      
    double dNx_dVec2Y;
    double dNy_dVec2Y;
    double dNz_dVec2Y;
         
    double dNx_dVec2Z;
    double dNy_dVec2Z;
    double dNz_dVec2Z;

    double dMag_dVec1X; 
    double dMag_dVec1Y;
    double dMag_dVec1Z;
                              
    double dMag_dVec2X; 
    double dMag_dVec2Y;
    double dMag_dVec2Z;       

    double dArea_dVec1X; 
    double dArea_dVec1Y;
    double dArea_dVec1Z;
                    
    double dArea_dVec2X; 
    double dArea_dVec2Y;
    double dArea_dVec2Z;       
               
    // Tri nodes

    Node1 = Grid(0).LoopList(Loop).Node1();
    Node2 = Grid(0).LoopList(Loop).Node2();
    Node3 = Grid(0).LoopList(Loop).Node3();
    
    // Edge vectors
    
    xyz1[0] = Grid(0).NodeList(Node1).x();
    xyz1[1] = Grid(0).NodeList(Node1).y();
    xyz1[2] = Grid(0).NodeList(Node1).z();
                                 
    xyz2[0] = Grid(0).NodeList(Node2).x();
    xyz2[1] = Grid(0).NodeList(Node2).y();
    xyz2[2] = Grid(0).NodeList(Node2).z();
                                 
    xyz3[0] = Grid(0).NodeList(Node3).x();
    xyz3[1] = Grid(0).NodeList(Node3).y();
    xyz3[2] = Grid(0).NodeList(Node3).z();

    // Edge vector gradients wrt dx, dy, dz...
            
    Vec1[0] = xyz2[0] - xyz1[0];
    Vec1[1] = xyz2[1] - xyz1[1];
    Vec1[2] = xyz2[2] - xyz1[2];
 
    Vec2[0] = xyz3[0] - xyz1[0];
    Vec2[1] = xyz3[1] - xyz1[1];
    Vec2[2] = xyz3[2] - xyz1[2];
   
    // Vec 1
    
    dVec1X_dX1 = -1.;
    dVec1X_dY1 =  0.;
    dVec1X_dZ1 =  0.;

    dVec1Y_dX1 =  0.;
    dVec1Y_dY1 = -1.;
    dVec1Y_dZ1 =  0.;
    
    dVec1Z_dX1 =  0.;
    dVec1Z_dY1 =  0.;
    dVec1Z_dZ1 = -1.;

    dVec1X_dX2 =  1.;
    dVec1X_dY2 =  0.;
    dVec1X_dZ2 =  0.;
            
    dVec1Y_dX2 =  0.;
    dVec1Y_dY2 =  1.;
    dVec1Y_dZ2 =  0.;
             
    dVec1Z_dX2 =  0.;
    dVec1Z_dY2 =  0.;
    dVec1Z_dZ2 =  1.;
            
    // Vec 2
    
    dVec2X_dX1 = -1.;
    dVec2X_dY1 =  0.;
    dVec2X_dZ1 =  0.;
        
    dVec2Y_dX1 =  0.;
    dVec2Y_dY1 = -1.;
    dVec2Y_dZ1 =  0.;
        
    dVec2Z_dX1 =  0.;
    dVec2Z_dY1 =  0.;
    dVec2Z_dZ1 = -1.;
       
    dVec2X_dX3 =  1.;
    dVec2X_dY3 =  0.;
    dVec2X_dZ3 =  0.;
             
    dVec2Y_dX3 =  0.;
    dVec2Y_dY3 =  1.;
    dVec2Y_dZ3 =  0.;
             
    dVec2Z_dX3 =  0.;
    dVec2Z_dY3 =  0.;
    dVec2Z_dZ3 =  1.;
              
    // Normal
    
    vector_cross(Vec1,Vec2,Normal);
    
    u = Normal[0];
    v = Normal[1];
    w = Normal[2];

    //  vector_cross(Vec1, Vec2, Forces_);
    //
    //  Normal[0] =  ( Vec2[2] * Vec1[1] - Vec2[1] * Vec1[2] );
    //  Normal[1] = -( Vec2[2] * Vec1[0] - Vec2[0] * Vec1[2] );
    //  Normal[2] =  ( Vec2[1] * Vec1[0] - Vec2[0] * Vec1[1] );     
       
    // Gradients of the normals wrt vec1
                
    dNx_dVec1X =       0.;
    dNy_dVec1X = -Vec2[2];
    dNz_dVec1X =  Vec2[1];
    
    dNx_dVec1Y =  Vec2[2];
    dNy_dVec1Y =       0.;
    dNz_dVec1Y = -Vec2[0];
       
    dNx_dVec1Z = -Vec2[1];
    dNy_dVec1Z =  Vec2[0];
    dNz_dVec1Z =       0.;
    
    // Gradients of the normals wrt vec2
                
    dNx_dVec2X =       0.;
    dNy_dVec2X =  Vec1[2];
    dNz_dVec2X = -Vec1[1];
     
    dNx_dVec2Y = -Vec1[2];
    dNy_dVec2Y =       0.;
    dNz_dVec2Y =  Vec1[0];
        
    dNx_dVec2Z =  Vec1[1];
    dNy_dVec2Z = -Vec1[0];
    dNz_dVec2Z =       0.;
    
    // Gradients of normal wrt node 1 
    
    // Node 1
    
    dNx_dx1 = dNx_dVec1X * dVec1X_dX1 + dNx_dVec2X * dVec2X_dX1;
    dNx_dy1 = dNx_dVec1Y * dVec1Y_dY1 + dNx_dVec2Y * dVec2Y_dY1;
    dNx_dz1 = dNx_dVec1Z * dVec1Z_dZ1 + dNx_dVec2Z * dVec2Z_dZ1; 
       
    dNy_dx1 = dNy_dVec1X * dVec1X_dX1 + dNy_dVec2X * dVec2X_dX1;
    dNy_dy1 = dNy_dVec1Y * dVec1Y_dY1 + dNy_dVec2Y * dVec2Y_dY1;
    dNy_dz1 = dNy_dVec1Z * dVec1Z_dZ1 + dNy_dVec2Z * dVec2Z_dZ1; 
       
    dNz_dx1 = dNz_dVec1X * dVec1X_dX1 + dNz_dVec2X * dVec2X_dX1;
    dNz_dy1 = dNz_dVec1Y * dVec1Y_dY1 + dNz_dVec2Y * dVec2Y_dY1;
    dNz_dz1 = dNz_dVec1Z * dVec1Z_dZ1 + dNz_dVec2Z * dVec2Z_dZ1; 
    
    // Node 2
    
    dNx_dx2 = dNx_dVec1X * dVec1X_dX2;
    dNx_dy2 = dNx_dVec1Y * dVec1Y_dY2;
    dNx_dz2 = dNx_dVec1Z * dVec1Z_dZ2; 
        
    dNy_dx2 = dNy_dVec1X * dVec1X_dX2;
    dNy_dy2 = dNy_dVec1Y * dVec1Y_dY2;
    dNy_dz2 = dNy_dVec1Z * dVec1Z_dZ2; 
       
    dNz_dx2 = dNz_dVec1X * dVec1X_dX2;
    dNz_dy2 = dNz_dVec1Y * dVec1Y_dY2;
    dNz_dz2 = dNz_dVec1Z * dVec1Z_dZ2;  
    
    // Node 3
    
    dNx_dx3 = dNx_dVec2X * dVec2X_dX3;
    dNx_dy3 = dNx_dVec2Y * dVec2Y_dY3;
    dNx_dz3 = dNx_dVec2Z * dVec2Z_dZ3; 
         
    dNy_dx3 = dNy_dVec2X * dVec2X_dX3;
    dNy_dy3 = dNy_dVec2Y * dVec2Y_dY3;
    dNy_dz3 = dNy_dVec2Z * dVec2Z_dZ3; 
          
    dNz_dx3 = dNz_dVec2X * dVec2X_dX3;
    dNz_dy3 = dNz_dVec2Y * dVec2Y_dY3;
    dNz_dz3 = dNz_dVec2Z * dVec2Z_dZ3;       
    
    // Magnitude
    
    Mag = sqrt(vector_dot(Normal,Normal));    

    dMag_dVec1X = (Normal[0]*dNx_dVec1X + Normal[1]*dNy_dVec1X + Normal[2]*dNz_dVec1X)/Mag; 
    dMag_dVec1Y = (Normal[0]*dNx_dVec1Y + Normal[1]*dNy_dVec1Y + Normal[2]*dNz_dVec1Y)/Mag;
    dMag_dVec1Z = (Normal[0]*dNx_dVec1Z + Normal[1]*dNy_dVec1Z + Normal[2]*dNz_dVec1Z)/Mag;
                                                                                
    dMag_dVec2X = (Normal[0]*dNx_dVec2X + Normal[1]*dNy_dVec2X + Normal[2]*dNz_dVec2X)/Mag; 
    dMag_dVec2Y = (Normal[0]*dNx_dVec2Y + Normal[1]*dNy_dVec2Y + Normal[2]*dNz_dVec2Y)/Mag;
    dMag_dVec2Z = (Normal[0]*dNx_dVec2Z + Normal[1]*dNy_dVec2Z + Normal[2]*dNz_dVec2Z)/Mag;       
    
    Normal[0] /= Mag; 
    Normal[1] /= Mag; 
    Normal[2] /= Mag; 

    // Gradients of normalized normal wrt vectors v1 and v2

    dNx_dVec1X = dNx_dVec1X/Mag - u * dMag_dVec1X / (Mag*Mag);
    dNx_dVec1Y = dNx_dVec1Y/Mag - u * dMag_dVec1Y / (Mag*Mag); 
    dNx_dVec1Z = dNx_dVec1Z/Mag - u * dMag_dVec1Z / (Mag*Mag); 

    dNy_dVec1X = dNy_dVec1X/Mag - v * dMag_dVec1X / (Mag*Mag);
    dNy_dVec1Y = dNy_dVec1Y/Mag - v * dMag_dVec1Y / (Mag*Mag); 
    dNy_dVec1Z = dNy_dVec1Z/Mag - v * dMag_dVec1Z / (Mag*Mag); 

    dNz_dVec1X = dNz_dVec1X/Mag - w * dMag_dVec1X / (Mag*Mag);
    dNz_dVec1Y = dNz_dVec1Y/Mag - w * dMag_dVec1Y / (Mag*Mag); 
    dNz_dVec1Z = dNz_dVec1Z/Mag - w * dMag_dVec1Z / (Mag*Mag); 
                                       
    dNx_dVec2X = dNx_dVec2X/Mag - u * dMag_dVec2X / (Mag*Mag);
    dNx_dVec2Y = dNx_dVec2Y/Mag - u * dMag_dVec2Y / (Mag*Mag); 
    dNx_dVec2Z = dNx_dVec2Z/Mag - u * dMag_dVec2Z / (Mag*Mag); 
                                               
    dNy_dVec2X = dNy_dVec2X/Mag - v * dMag_dVec2X / (Mag*Mag);
    dNy_dVec2Y = dNy_dVec2Y/Mag - v * dMag_dVec2Y / (Mag*Mag); 
    dNy_dVec2Z = dNy_dVec2Z/Mag - v * dMag_dVec2Z / (Mag*Mag); 
                                               
    dNz_dVec2X = dNz_dVec2X/Mag - w * dMag_dVec2X / (Mag*Mag);
    dNz_dVec2Y = dNz_dVec2Y/Mag - w * dMag_dVec2Y / (Mag*Mag); 
    dNz_dVec2Z = dNz_dVec2Z/Mag - w * dMag_dVec2Z / (Mag*Mag); 
    
    // Finally, gradients of the normal vector wrt xyz at nodes 1, 2, and 3...
    
    // Node 1
    
    dNx_Normalized_dx1 = dNx_dVec1X * dVec1X_dX1 + dNx_dVec2X * dVec2X_dX1;
    dNx_Normalized_dy1 = dNx_dVec1Y * dVec1Y_dY1 + dNx_dVec2Y * dVec2Y_dY1;
    dNx_Normalized_dz1 = dNx_dVec1Z * dVec1Z_dZ1 + dNx_dVec2Z * dVec2Z_dZ1; 
       
    dNy_Normalized_dx1 = dNy_dVec1X * dVec1X_dX1 + dNy_dVec2X * dVec2X_dX1;
    dNy_Normalized_dy1 = dNy_dVec1Y * dVec1Y_dY1 + dNy_dVec2Y * dVec2Y_dY1;
    dNy_Normalized_dz1 = dNy_dVec1Z * dVec1Z_dZ1 + dNy_dVec2Z * dVec2Z_dZ1; 
       
    dNz_Normalized_dx1 = dNz_dVec1X * dVec1X_dX1 + dNz_dVec2X * dVec2X_dX1;
    dNz_Normalized_dy1 = dNz_dVec1Y * dVec1Y_dY1 + dNz_dVec2Y * dVec2Y_dY1;
    dNz_Normalized_dz1 = dNz_dVec1Z * dVec1Z_dZ1 + dNz_dVec2Z * dVec2Z_dZ1; 
    
    // Node 2
    
    dNx_Normalized_dx2 = dNx_dVec1X * dVec1X_dX2;
    dNx_Normalized_dy2 = dNx_dVec1Y * dVec1Y_dY2;
    dNx_Normalized_dz2 = dNx_dVec1Z * dVec1Z_dZ2; 
        
    dNy_Normalized_dx2 = dNy_dVec1X * dVec1X_dX2;
    dNy_Normalized_dy2 = dNy_dVec1Y * dVec1Y_dY2;
    dNy_Normalized_dz2 = dNy_dVec1Z * dVec1Z_dZ2; 
       
    dNz_Normalized_dx2 = dNz_dVec1X * dVec1X_dX2;
    dNz_Normalized_dy2 = dNz_dVec1Y * dVec1Y_dY2;
    dNz_Normalized_dz2 = dNz_dVec1Z * dVec1Z_dZ2;  
    
    // Node 3
    
    dNx_Normalized_dx3 = dNx_dVec2X * dVec2X_dX3;
    dNx_Normalized_dy3 = dNx_dVec2Y * dVec2Y_dY3;
    dNx_Normalized_dz3 = dNx_dVec2Z * dVec2Z_dZ3; 
                    
    dNy_Normalized_dx3 = dNy_dVec2X * dVec2X_dX3;
    dNy_Normalized_dy3 = dNy_dVec2Y * dVec2Y_dY3;
    dNy_Normalized_dz3 = dNy_dVec2Z * dVec2Z_dZ3; 
                     
    dNz_Normalized_dx3 = dNz_dVec2X * dVec2X_dX3;
    dNz_Normalized_dy3 = dNz_dVec2Y * dVec2Y_dY3;
    dNz_Normalized_dz3 = dNz_dVec2Z * dVec2Z_dZ3;   
    
    // Area is 1/2 magnitude

    Area = 0.5 * Mag;

    dArea_dVec1X = 0.5*dMag_dVec1X; 
    dArea_dVec1Y = 0.5*dMag_dVec1Y;
    dArea_dVec1Z = 0.5*dMag_dVec1Z;
              
    dArea_dVec2X = 0.5*dMag_dVec2X; 
    dArea_dVec2Y = 0.5*dMag_dVec2Y;
    dArea_dVec2Z = 0.5*dMag_dVec2Z;   
    
    // Node 1
    
    dArea_dx1 = dArea_dVec1X * dVec1X_dX1 + dArea_dVec2X * dVec2X_dX1;
    dArea_dy1 = dArea_dVec1Y * dVec1Y_dY1 + dArea_dVec2Y * dVec2Y_dY1;
    dArea_dz1 = dArea_dVec1Z * dVec1Z_dZ1 + dArea_dVec2Z * dVec2Z_dZ1; 

    // Node 2
    
    dArea_dx2 = dArea_dVec1X * dVec1X_dX2;
    dArea_dy2 = dArea_dVec1Y * dVec1Y_dY2;
    dArea_dz2 = dArea_dVec1Z * dVec1Z_dZ2; 

    // Node 3
    
    dArea_dx3 = dArea_dVec2X * dVec2X_dX3;
    dArea_dy3 = dArea_dVec2Y * dVec2Y_dY3;
    dArea_dz3 = dArea_dVec2Z * dVec2Z_dZ3; 
                                  
}

#include "END_NAME_SPACE.H"
