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
    
    LoadDeformationFile_ = 0;
    
    DoGroundEffectsAnalysis_ = 0;
    
    VehicleRotationAngleVector_[0] = 0.;    
    VehicleRotationAngleVector_[1] = 0.;    
    VehicleRotationAngleVector_[2] = 0.;    

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

    
    char CART3D_File_Name[2000];
    char VSPGEOM_File_Name[2000];
    char ControlSurfaceTagListFileName[2000];    
    FILE *File;
     
    // File name possibilities...

    SPRINTF(CART3D_File_Name,"%s.tri",FileName);
    SPRINTF(VSPGEOM_File_Name,"%s.vspgeom",FileName);

    if ( (File = fopen(VSPGEOM_File_Name,"r")) != NULL ) {

       fclose(File);
    
       Read_VSPGEOM_File(FileName);
                 
       SurfaceType_ = VSPGEOM_SURFACE;
                 
    }
    
    // CART3D file
    
    else if ( (File = fopen(CART3D_File_Name,"r")) != NULL ) {

      fclose(File);
      
      Read_CART3D_File(FileName);
      
      ModelType_ = PANEL_MODEL;
      
      SurfaceType_ = CART3D_SURFACE;
      
    }
    
    else {

       PRINTF("Could not load %s OpenVSP VSPGEOM file, or CART3D Tri file... n", FileName);fflush(NULL);

       exit(1);
       
    }
    
    // Look for control surfaces
    
    SPRINTF(ControlSurfaceTagListFileName,"%s.ControlSurfaces.taglist",FileName);
    
    PRINTF("Looking for %s control surface definition file ... \n",ControlSurfaceTagListFileName);
    
    if ( (File = fopen(ControlSurfaceTagListFileName,"r")) != NULL ) {

      fclose(File);
      
      PRINTF("There is control surface information... \n");
      
      ReadControlSurfaceInformation(FileName);
      
    }
    
    else {
       
       PRINTF("Did not find any control surface definitions... \n");
       
    }    
    
    // AUTODIFF: Continue recording
   
    CONTINUE_AUTO_DIFF();
    
    // Create meshes for the VSP geometries
    
    MeshGeom();

    ModelType = ModelType_;
    
    SurfaceType = SurfaceType_;
              
}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM Read_CART3D_File                            #
#                                                                              #
##############################################################################*/

void VSP_GEOM::Read_CART3D_File(char *FileName)
{

    int i, Done, *ComponentList;
    char VSP_File_Name[2000], TKEY_File_Name[2000], VSP_Degen_File_Name[2000], Name[2000], DumChar[2000];
    VSPAERO_DOUBLE Diam, x, y, z, nx, ny, nz;
    FILE *Cart3D_File, *TKEY_File, *VSP_Degen_File;
 
    SPRINTF(VSP_File_Name,"%s.tri",FileName);
    
    if ( (Cart3D_File = fopen(VSP_File_Name,"r")) == NULL ) {

       PRINTF("Could not load %s CART3D file... \n", VSP_File_Name);fflush(NULL);

       exit(1);

    }    

    SPRINTF(TKEY_File_Name,"%s.tkey",FileName);
    
    if ( (TKEY_File = fopen(TKEY_File_Name,"r")) == NULL ) {

       PRINTF("Could not load %s TKEY file... so I won't use it... ;-) \n", TKEY_File_Name);fflush(NULL);

       TKEY_File = NULL;

    }    
 
    // Read in the cart3d geometry

    SPRINTF(Name,"CART3D");

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
    
    PRINTF("Found %d components for cart3d geometry ... \n",NumberOfComponents_);fflush(NULL);
    
    delete [] ComponentList;
    
    fclose(Cart3D_File);
    
    if ( TKEY_File != NULL ) fclose(TKEY_File);
    
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
       
       PRINTF("Found: %d Rotors \n",NumberOfRotors_);
       
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
    char VSPGEOM_File_Name[2000], VSP_Degen_File_Name[2000], Name[2000], DumChar[2000];
    char VKEY_File_Name[2000];
    VSPAERO_DOUBLE Diam, x, y, z, nx, ny, nz;
    FILE *VSPGEOM_File, *VKEY_File, *VSP_Degen_File;
 
    SPRINTF(VSPGEOM_File_Name,"%s.vspgeom",FileName);
    
    if ( (VSPGEOM_File = fopen(VSPGEOM_File_Name,"r")) == NULL ) {

       PRINTF("Could not load %s VSPGEOM file... \n", VSPGEOM_File_Name);fflush(NULL);

       exit(1);

    }    

    SPRINTF(VKEY_File_Name,"%s.vkey",FileName);
    
    if ( (VKEY_File = fopen(VKEY_File_Name,"r")) == NULL ) {

       PRINTF("Could not load %s VKEY file... so I won't use it... ;-) \n", VKEY_File_Name);fflush(NULL);

       VKEY_File = NULL;

    }    

    // Read in the vspgeom mesh file

    SPRINTF(Name,"VSPGEOM");

    ReadVSPGeomDataFromFile(Name,VSPGEOM_File,VKEY_File);
 
    fclose(VSPGEOM_File);
    
    if ( VKEY_File != NULL ) fclose(VKEY_File);
 
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
       
       PRINTF("Found: %d Rotors \n",NumberOfRotors_);
       
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
    char ControlSurfaceTagListFileName[2000], TagListFileName[2000];
    FILE *TagListFile;
    
    SPRINTF(ControlSurfaceTagListFileName,"%s.ControlSurfaces.taglist",FileName);
    
    if ( (TagListFile = fopen(ControlSurfaceTagListFileName,"r")) == NULL ) {

       PRINTF("Could not load %s Taglist file... \n", ControlSurfaceTagListFileName);fflush(NULL);

       exit(1);

    }   
 
    // Read in the number of tag regions... ie, number of control surfaces and their names
        
    fscanf(TagListFile,"%d",&NumberOfControlSurfaces_);
    
    ControlSurface_ = new CONTROL_SURFACE[NumberOfControlSurfaces_ + 1];
    
    for ( i = 1 ; i <= NumberOfControlSurfaces_ ; i++ ) {
       
       fscanf(TagListFile,"%s",TagListFileName);

       PRINTF("Reading in looplist for control surface: %d --> %s \n",i,TagListFileName);
       
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
#                         VSP_GEOM MeshGeom                                    #
#                                                                              #
##############################################################################*/

void VSP_GEOM::MeshGeom(void)
{
   
    int i;
    int Done;
    VSPAERO_DOUBLE AreaTotal;

    // Sanitize meshes with thin surfaces
    
    if ( NumberOfThinSurfaces_ == 0 ) {
       
       Grid().CalculateTriNormalsAndCentroids();
           
       Grid().CreateTriEdges();
       
       // Check the consistency of the mesh 
       
       if (  Grid().CheckGridConsistency() ) {
       
          WriteOutTriMesh();
   
          PRINTF("Mesh is not logically valid... stopping analysis! \n");
          PRINTF("Mesh with whacked tris marked written out... \n");
          
          fflush(NULL); exit(1);
          
       }       
           
       Grid().CalculateUpwindEdges();  
           
       Grid().CreateUpwindEdgeData();
       
       Grid().MarkBoundaries();
       
                
    }
    
    else {
          
       SanitizeThinMeshes();
     
    }

    PRINTF("Number of kutta nodes before trimming: %d \n",NumberOfKuttaNodes_);

    if ( NumberOfKuttaNodes_ > 0 ) {
       
       FindSharpEdges(NumberOfKuttaNodes_,KuttaNodeList_);
    
       delete [] KuttaNodeList_;
       
    }
    
    else {
       
      FindSharpEdges(0,NULL);
      
    }       
           
    // State model type

    if ( ModelType_ == VLM_MODEL   ) PRINTF("Model is a VSPGEOM VLM geometry \n");fflush(NULL);
    if ( ModelType_ == PANEL_MODEL ) PRINTF("Model is a VSPGEOM Panel geometry \n");fflush(NULL);
        
    Grid().ModelType() = ModelType_;        
    
    // Calculate surface areas per surface ID region
    
    AreaTotal = 0.;
    
    for ( i = 1 ; i <= Grid().NumberOfLoops() ; i++ ) {
       
       AreaTotal += Grid().LoopList(i).Area();
       
    }
    
    // Calculate bounding boxes for all components in the grid
    
    CreateComponentBBoxData();
    
    Grid().MinLoopArea() = AreaTotal;
   
    PRINTF("Total NumberOfNodes:      %d \n",Grid().NumberOfNodes());
    PRINTF("Total NumberOfLoops:      %d \n",Grid().NumberOfLoops());
    PRINTF("Total NumberOfEdges:      %d \n",Grid().NumberOfEdges());
    PRINTF("Total NumberOfKuttaNodes: %d \n",Grid().NumberOfKuttaNodes());
    PRINTF("NumberOfSurfaces_:        %d \n",NumberOfSurfaces_);
    
    PRINTF("Total surface area: %lf \n",AreaTotal);
    PRINTF("Minimum loop area constraint set to: %lf \n",Grid().MinLoopArea());
 
    PRINTF("Agglomerating mesh... \n");fflush(NULL);

    VSP_AGGLOM Agglomerate;

    PRINTF("Grid:%d --> # loops: %10d ...# Edges: %10d ...# Nodes: %10d  \n",0,Grid_[0]->NumberOfLoops(),Grid_[0]->NumberOfEdges(),Grid_[0]->NumberOfNodes());
    
    // Simplify the grid... ie merge tris into quads and get rid of sliver tris on panel models

    if ( InputMeshIsMixedPolys_ ) {
       
       Grid_[1] = Agglomerate.SimplifyMesh(*(Grid_[0]), *(GridC_));
       
       
    }
    
    else {
     
       Grid_[1] = Agglomerate.SimplifyMesh(*(Grid_[0]));
      
    }

    Grid_[1]->CalculateUpwindEdges();   
    
    Grid_[1]->CreateUpwindEdgeData();
              
    PRINTF("Grid:%d --> # loops: %10d ...# Edges: %10d ...# Nodes: %10d  \n",1,Grid_[1]->NumberOfLoops(),Grid_[1]->NumberOfEdges(),Grid_[1]->NumberOfNodes());
   
    // Now recusively agglomerate the meshes
    
    i = 2;
    
    Done = 0;

    while ( !Done && 
            i < MaxNumberOfGridLevels_ &&
            Grid_[i-1]->NumberOfLoops() > NumberOfSurfaces_ ) {

       Grid_[i] = Agglomerate.Agglomerate(*(Grid_[i-1]));
             
       if ( i <= 2 ||     
            (    Grid_[i]->NumberOfLoops() >  250 &&
                 Grid_[i]->NumberOfEdges() >  250 &&
               2*Grid_[i]->NumberOfLoops() <= Grid_[i-1]->NumberOfLoops() ) ) {

          Grid_[i]->CalculateUpwindEdges();   
       
          Grid_[i]->CreateUpwindEdgeData();

          PRINTF("Grid:%d --> # loops: %10d ...# Edges: %10d ...# Nodes: %10d \n",i,
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

    PRINTF("NumberOfGridLevels_: %d \n",NumberOfGridLevels_);    
    PRINTF("NumberOfSurfaces_: %d \n",NumberOfSurfaces_);
    
    // Ouput the coarse grid mesh info
    
    if ( Verbose_ ) OutputCoarseGridInfo();

    // Find vortex loops lying within any control surface regions
    
    FindControlSurfaceVortexLoops();

}

/*##############################################################################
#                                                                              #
#                          VSP_GEOM WriteOutTriMesh                           #
#                                                                              #
##############################################################################*/

void VSP_GEOM::WriteOutTriMesh(void)
{
 
    int i;
    char FileName[2000];
    FILE *TriMesh;
    
    sprintf(FileName,"whacked.tri");
    
    if ( (TriMesh = fopen(FileName,"w")) == NULL ) {

       PRINTF("Could not open formatted output file for whacked tri cart3d file! \n");
       fflush(NULL);exit(1);
                 
    }
        
    FPRINTF(TriMesh,"%d %d \n",Grid_[0]->NumberOfNodes(),Grid_[0]->NumberOfTris());
    
    for ( i = 1 ; i <= Grid_[0]->NumberOfNodes() ; i++ ) {
       
       FPRINTF(TriMesh,"%f %f %f \n",
               Grid_[0]->NodeList(i).x(),
               Grid_[0]->NodeList(i).y(),
               Grid_[0]->NodeList(i).z());
       
    }
    
    for ( i = 1 ; i <= Grid_[0]->NumberOfTris() ; i++ ) {
       
       FPRINTF(TriMesh,"%d %d %d \n",
               Grid_[0]->TriList(i).Node1(),
               Grid_[0]->TriList(i).Node2(),
               Grid_[0]->TriList(i).Node3());
       
    }
  
  
    for ( i = 1 ; i <= Grid_[0]->NumberOfTris() ; i++ ) {
       
       FPRINTF(TriMesh,"%d \n",
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
    int NumberOfThinTris, NumberOfThickTris, NumberOfMeshes;    
    int NumKuttaNodeLists, NumKuttaNodes, NumNodesInList, *KuttaNodeList, *TempList;
    int CurrentNumberOfEdges, CurrentNumberOfTris;
    VSP_GRID *GridList;
     
    // If any of the surfaces are thin ... we need to tweak the mesh data structure so we
    // do not have more than 2 tris attached to any one edge. We do this by creating 
    // seperate edge data structures for the thin surfaces

    PRINTF("There are thin %d components \n", NumberOfThinComponents_);fflush(NULL);

    GridList = new VSP_GRID[NumberOfThinComponents_ + 2];
    
    TriPerm = new int*[NumberOfThinComponents_ + 2];
    
    // Calculate total number of nodes and tris, including the wake
        
    if ( NumberOfThinComponents_ > 0 ) {

       PRINTF("There are thin components... renumbering for edge data structures \n");fflush(NULL);
       
       p = 0;
              
       // Create a single mesh with all the thick surfaces
              
       NumberOfThickTris = 0;
          
       for ( n = 1 ; n <= Grid().NumberOfTris() ; n++ ) {
       
          if ( Grid().TriList(n).SurfaceType() == THICK_SURFACE ) NumberOfThickTris++;
          
       }
       
       if ( NumberOfThickTris > 0 ) {

          PRINTF("There are thick components as well... it's a mixed mesh \n");fflush(NULL);
       
          p++;

          PRINTF("Thick tris are being stored in temp mesh: %d \n",p);fflush(NULL);
          
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

             GridList[p].NodeList(n).ComponentID()          = Grid().NodeList(n).ComponentID();

             GridList[p].NodeList(n).SurfaceID()            = Grid().NodeList(n).SurfaceID();

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
                                    
                GridList[p].TriList(t).SurfaceID()         = Grid().TriList(n).SurfaceID();
                                    
                GridList[p].TriList(t).ComponentID()       = Grid().TriList(n).ComponentID();
                                                      
                GridList[p].TriList(t).SurfaceType()       = Grid().TriList(n).SurfaceType();
                                    
                GridList[p].TriList(t).VortexSheet()       = Grid().TriList(n).VortexSheet();
                                                        
                GridList[p].TriList(t).SpanStation()       = Grid().TriList(n).SpanStation() ;
                                    
                GridList[p].TriList(t).IsTrailingEdgeTri() = Grid().TriList(n).IsTrailingEdgeTri();
                                    
             }
     
          }            
          
       }
       
       // Create a separate grid for each thin component
        
       for ( k = 1 ; k <= NumberOfComponents_ ; k++ ) {
          
          NumberOfThinTris = 0;
          
          if ( !(ComponentIsThick(k)) ) {
          
             for ( n = 1 ; n <= Grid().NumberOfTris() ; n++ ) {
          
                if ( Grid().TriList(n).ComponentID() == ComponentIDForSurfaceComponent(k) ) NumberOfThinTris++;
                
             }
                                     
             p++;

             PRINTF("Surface %d has %d tris and is being stored in temp mesh: %d \n",k,NumberOfThinTris,p);fflush(NULL);
             
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

                GridList[p].NodeList(n).ComponentID()          = Grid().NodeList(n).ComponentID();

                GridList[p].NodeList(n).SurfaceID()            = Grid().NodeList(n).SurfaceID();

             }  
             
             // Copy over tri data
             
             t = 0;
             
             for ( n = 1 ; n <= Grid().NumberOfTris() ; n++ ) {
                
                if ( Grid().TriList(n).ComponentID() == ComponentIDForSurfaceComponent(k) ){
                   
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
                                       
                   GridList[p].TriList(t).SurfaceID()         = Grid().TriList(n).SurfaceID();
                                       
                   GridList[p].TriList(t).ComponentID()       = Grid().TriList(n).ComponentID();
                                              
                   GridList[p].TriList(t).SurfaceType()       = Grid().TriList(n).SurfaceType();
                                       
                   GridList[p].TriList(t).VortexSheet()       = Grid().TriList(n).VortexSheet();
                                                        
                   GridList[p].TriList(t).SpanStation()       = Grid().TriList(n).SpanStation() ;
                                    
                   GridList[p].TriList(t).IsTrailingEdgeTri() = Grid().TriList(n).IsTrailingEdgeTri();
                                    
                }
             
             }                 
                       
          }
          
       }
          
       NumberOfMeshes = p;
       
       if ( NumberOfThickTris ) PRINTF("Created one thick surface mesh structure... \n");
       
       PRINTF("Created %d thin surface mesh structures \n", NumberOfThinComponents_);
       
       PRINTF("Created %d total mesh structures \n", NumberOfMeshes);
       
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

             Grid().EdgeList(k).VortexLoop1() = TriPerm[p][GridList[p].EdgeList(i).VortexLoop1()]; 
             Grid().EdgeList(k).VortexLoop2() = TriPerm[p][GridList[p].EdgeList(i).VortexLoop2()]; 

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
       
       // Mark the boundaries on the mesh
       
       Grid().MarkBoundaries();
   
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
    VSPAERO_DOUBLE y, z, Epsilon;
    
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

    if ( ModelType_ == VLM_MODEL   ) PRINTF("Model is a VSPGEOM VLM geometry \n");fflush(NULL);
    if ( ModelType_ == PANEL_MODEL ) PRINTF("Model is a VSPGEOM Panel geometry \n");fflush(NULL);

}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM CreateComponentBBoxData                     #
#                                                                              #
##############################################################################*/

void VSP_GEOM::CreateComponentBBoxData(void)
{
   
    int i, c, j, Node;
    
    PRINTF("NumberOfComponents_: %d\n",NumberOfComponents_);

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
#                     VSP_GEOM FindControlSurfaceVortexLoops                   #
#                                                                              #
##############################################################################*/

void VSP_GEOM::FindControlSurfaceVortexLoops(void)
{

//    int Surface, k, j, NumberOfTrisInControlSurface;
//    
//    for ( Surface = 1 ; Surface <= NumberOfSurfaces_ ; Surface++ ) {
//       
//       if ( VSP_Surface(Surface).SurfaceType() == DEGEN_WING_SURFACE ) {
//    
//          for ( k = 1 ; k <= VSP_Surface(Surface).NumberOfControlSurfaces() ; k++ ) {
//             
//             NumberOfTrisInControlSurface = 0;
//             
//             for ( j = 1 ; j <= Grid(1).NumberOfLoops() ; j++ ) {
//                
//                if ( Grid(1).TriList(j).SurfaceID() == Surface ) {
//  
//                   if ( VSP_Surface(Surface).ControlSurface(k).TriInside(Grid(1).LoopList(j).UVc()) ) {
//                      
//                      NumberOfTrisInControlSurface++;
//                      
//                   }
//                   
//                }
//                
//             }
//
//             VSP_Surface(Surface).ControlSurface(k).SizeLoopList(NumberOfTrisInControlSurface);
//             
//          }
//          
//       }
//       
//    }
//    
//    for ( Surface = 1 ; Surface <= NumberOfSurfaces_ ; Surface++ ) {
//       
//       if ( VSP_Surface(Surface).SurfaceType() == DEGEN_WING_SURFACE ) {
//    
//          for ( k = 1 ; k <= VSP_Surface(Surface).NumberOfControlSurfaces() ; k++ ) {
//             
//             NumberOfTrisInControlSurface = 0;
//             
//             for ( j = 1 ; j <= Grid(1).NumberOfLoops() ; j++ ) {
//                
//                if ( Grid(1).TriList(j).SurfaceID() == Surface ) {
//                
//                   if ( VSP_Surface(Surface).ControlSurface(k).TriInside(Grid(1).LoopList(j).UVc()) ) {
//                      
//                      NumberOfTrisInControlSurface++;
//                      
//                      VSP_Surface(Surface).ControlSurface(k).LoopList(NumberOfTrisInControlSurface) = j;
//                      
//                   }
//                   
//                }
//                
//             }
//    
//          }
//          
//       }
//       
//    }
 
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
       
       PRINTF("Coarse loop %d contains %d fine loops \n",i,NumberOfLoops);
       
       for ( j = 1 ; j <= Grid(Level).LoopList(i).NumberOfEdges() ; j++ ) {
          
          Edge = Grid(Level).LoopList(i).Edge(j);
          
          Loop = Grid(Level).EdgeList(Edge).Loop1()
               + Grid(Level).EdgeList(Edge).Loop2() - i;
                
          PRINTF("   Boundary Loop: %d: %d ... pairs: %d %d\n",j,Loop,
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
#                            VSP_GEOM UpdateMeshes                             #
#                                                                              #
##############################################################################*/

void VSP_GEOM::UpdateMeshes(void)
{
   
    int i, Node, Node1, Node2;
    
    // Update the loop data finest grid
    
    Grid(0).CalculateTriNormalsAndCentroids();

    // Update the edge data on the finest mesh
    
    for ( i = 1 ; i <= Grid(0).NumberOfEdges() ; i++ ) {
       
       Node1 = Grid(0).EdgeList(i).Node1();
       Node2 = Grid(0).EdgeList(i).Node2();
   
       Grid(0).EdgeList(i).UpdateGeometryLocation(Grid(0).NodeList(Node1), Grid(0).NodeList(Node2));
  
    }
    
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
      
}

/*##############################################################################
#                                                                              #
#                        VSP_GEOM UpdateCoarseMesh                             #              
#                                                                              #
##############################################################################*/

void VSP_GEOM::UpdateCoarseMesh(VSP_GRID &FineGrid, VSP_GRID &CoarseGrid)
{
    
    int i, j, k, Node, Node1, Node2, Next;
    VSPAERO_DOUBLE Length, Area, Mag, Xb, Yb, Zb;
    VSPAERO_DOUBLE x1, y1, z1, x2, y2, z2;
        
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
   
       // Camber
       
       CoarseGrid.LoopList(i).Camber() = 0.;
       
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
        
        // Camber
        
        CoarseGrid.LoopList(Next).Camber() += Area*FineGrid.LoopList(i).Camber();
        
        // Centroid
        
        CoarseGrid.LoopList(Next).Xc() += Area*FineGrid.LoopList(i).Xc();
        CoarseGrid.LoopList(Next).Yc() += Area*FineGrid.LoopList(i).Yc();
        CoarseGrid.LoopList(Next).Zc() += Area*FineGrid.LoopList(i).Zc();     
        
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
     
       // Camber
       
       CoarseGrid.LoopList(i).Camber() /= Area;   
       
       // Centroid
       
       CoarseGrid.LoopList(i).Xc() /= Area;
       CoarseGrid.LoopList(i).Yc() /= Area;
       CoarseGrid.LoopList(i).Zc() /= Area; 
       
       // UV Centroid
       
       CoarseGrid.LoopList(i).Uc() /= Area;
       CoarseGrid.LoopList(i).Vc() /= Area;   
 
    }    
    
    // Calculate centroid offset
  
    for ( i = 1 ; i <= CoarseGrid.NumberOfLoops() ; i++ ) {
    
       Xb = 0.5*( CoarseGrid.LoopList(i).BoundBox().x_max + CoarseGrid.LoopList(i).BoundBox().x_min );
       Yb = 0.5*( CoarseGrid.LoopList(i).BoundBox().y_max + CoarseGrid.LoopList(i).BoundBox().y_min );
       Zb = 0.5*( CoarseGrid.LoopList(i).BoundBox().z_max + CoarseGrid.LoopList(i).BoundBox().z_min );

      // if ( Xb == CoarseGrid.LoopList(i).Xc() &&
      //      Yb == CoarseGrid.LoopList(i).Yc() && 
      //      Zb == CoarseGrid.LoopList(i).Zc()){
      //
      //     CoarseGrid.LoopList(i).CentroidOffSet() = 0.0;
      //
      // }
      //
      // else{

           CoarseGrid.LoopList(i).CentroidOffSet() = sqrt( pow(CoarseGrid.LoopList(i).Xc() - Xb,2.)
                                                         + pow(CoarseGrid.LoopList(i).Yc() - Yb,2.)
                                                         + pow(CoarseGrid.LoopList(i).Zc() - Zb,2.) );
    //   }

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
       
            // if ( i != j ) {

                Node2 = CoarseGrid.LoopList(k).Node(j);
       
                x2 = CoarseGrid.NodeList(Node2).x();
                y2 = CoarseGrid.NodeList(Node2).y();
                z2 = CoarseGrid.NodeList(Node2).z();
       
                Length = MAX(Length,sqrt( pow(x1-x2,2.) + pow(y1-y2,2.) + pow(z1-z2,2.) ));

          //   }
             
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
#                             VSP_GEOM ReadCart3DDataFromFile                  #
#                                                                              #
##############################################################################*/

void VSP_GEOM::ReadCart3DDataFromFile(char *Name, FILE *CART3D_File, FILE *TKEY_File)
{
 
    int i, k, n, NumNodes, NumTris, Node1, Node2, Node3, SurfaceID, Done;
    int *SurfaceList, Found, DumInt, *ComponentIDForVSPSurface;
    int *SurfaceIsUsed, NumberOfVSPSurfaces;
    char DumChar[2000], Comma[2000], *Next;
    VSPAERO_DOUBLE x, y, z, u1, u2, u3, v1, v2, v3;
    VSPAERO_DOUBLE Ymin, Ymax, Zmin, Zmax;
    
    InputMeshIsMixedPolys_ = 0;

    SPRINTF (Comma,",");

    // Save the component name
    
    SPRINTF (ComponentName_,"%s_Cart3d",Name);
    
    // Set surface type
    
    SurfaceType_ = CART3D_SURFACE;
    
    // Read in the header
    
    fscanf(CART3D_File,"%d %d",&NumNodes,&NumTris);

    PRINTF("NumNodes, NumTris: %d %d \n",NumNodes, NumTris);
    
    // Estimate the maximum number of grid levels
    
    MaxNumberOfGridLevels_ = 4*(int) ( log(0.5 * NumTris) / log(4.0) );
    
    PRINTF("MaxNumberOfGridLevels_: %d \n",MaxNumberOfGridLevels_);
    
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
        
    BoundaryConditionForSurface_ = new BOUNDARY_CONDITION_DATA[NumberOfSurfaces_ + 2];
    
    ComponentIDForSurface_ = new int[NumberOfSurfaces_ + 1];

    SurfaceIsThick_ = new int[NumberOfSurfaces_ + 1];
    
    ComponentIsThick_ = new int[NumberOfSurfaces_ + 1];
    
    for ( i = 1 ; i <= NumberOfSurfaces_ ; i++ ) {
       
       SurfaceIsThick_[i] = 1;
     
       ComponentIsThick_[i] = 1;
       
    }

    PRINTF("Found %d CART3D Surfaces \n",NumberOfSurfaces_);

    if ( TKEY_File != NULL ) {
       
       // Read in the vkey data
       
       fgets(DumChar,2000,TKEY_File);PRINTF("DumChar: %s \n",DumChar); fflush(NULL);
       fgets(DumChar,2000,TKEY_File);PRINTF("DumChar: %s \n",DumChar); fflush(NULL);
       fgets(DumChar,2000,TKEY_File);PRINTF("DumChar: %s \n",DumChar); fflush(NULL);
       
       sscanf(DumChar,"%d\n",&NumberOfVSPSurfaces);

       ComponentIDForVSPSurface = new int[NumberOfVSPSurfaces + 1];
       
       PRINTF("NumberOfVSPSurfaces: %d \n",NumberOfVSPSurfaces);

       fgets(DumChar,2000,TKEY_File);
      
       k = 0;
       
       for ( n = 1 ; n <= NumberOfVSPSurfaces ; n++ ) {
          
          fgets(DumChar,2000,TKEY_File);
 
          Next = strtok(DumChar,Comma); 

          DumInt = atoi(Next);
          
          Next = strtok(NULL,Comma);  Next[strcspn(Next, "\n")] = 0;
          
          if ( SurfaceIsUsed[DumInt] ) {
             
             k++;
          
             ComponentIDForVSPSurface[n] = k;
             
             ComponentIDForSurface_[k] = n;
             
             SurfaceNameList_[k] = new char[2000];
                          
             SPRINTF(SurfaceNameList_[k],"%s",Next);
          
             PRINTF("Surface: %d exists in tringulation and will be surface: %d with OpenVSP Name: %s \n",DumInt,k,SurfaceNameList_[k]);
  
          }
          
       }
       
       if ( k != NumberOfSurfaces_ ) {
          
          PRINTF("Error... number of used surfaces in .tri and .tkey files do not match! \n");
          fflush(NULL);exit(1);
          
       }
          
    }
    
    else {
       
       for ( n = 1 ; n <= NumberOfSurfaces_ ; n++ ) {
 
          ComponentIDForSurface_[n] = n;
          
          SurfaceNameList_[n] = new char[2000];
          
          SPRINTF(SurfaceNameList_[n],"Surface_%d",n);
          
       }
       
    }

    for ( i = 1 ; i <= NumberOfSurfaces_ ; i++ ) {
       
       PRINTF("SurfaceList[%d]: %d \n",i,SurfaceList[i]);
       
    }

    fflush(NULL);
    
    if ( TKEY_File != NULL ) {
    
       for ( i = 1 ; i <= NumberOfVSPSurfaces ; i++ ) {
          
          PRINTF("ComponentIDForVSPSurface[%d]: %d \n",i,ComponentIDForVSPSurface[i]);
          
       }
       

       fflush(NULL);

       for ( i = 1 ; i <= NumberOfSurfaces_ ; i++ ) {
          
         ComponentIDForSurface_[i] = ComponentIDForVSPSurface[ComponentIDForSurface_[i]];
          
       }
       
    }
    
    for ( i = 1 ; i <= NumberOfSurfaces_ ; i++ ) {
       
       PRINTF("ComponentIDForSurface_[%d]: %d \n",i,ComponentIDForSurface_[i]);
       
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
          
          PRINTF("Error in determing number of surfaces in CART3D file! \n");
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
    int Node, Node1, Node2, Node3, SurfaceID, Done;
    int *SurfaceList, Found, CompID, ThickThinDataExists, NumTriNodes;
    int *SurfaceIsUsed, NumberOfVSPSurfaces, ModelType;    
    int NumKuttaNodeLists, NumNodesInList, *TempList, FileVersion;
    char DumChar[2000], DumChar2[2000], Comma[2000], TempName[2000], Space[2000], *Next;
    VSPAERO_DOUBLE x, y, z, u, v, w, u1, v1, u2, v2, u3, v3;
    fpos_t TopOfTriangluation;

    // Read in first line and look for v2 files
    
    fgets(DumChar,2000,VSPGeom_File);
    
    FileVersion = 1;
    
    if ( strstr(DumChar,"v2") != NULL ) FileVersion = 2;
    
    if ( FileVersion == 1 ) rewind(VSPGeom_File);

    SPRINTF (Comma,",");

    SPRINTF (Space," ");

    // Save the component name
    
    SPRINTF (ComponentName_,"%s_VSPGeom",Name);
    
    // Set surface type
    
    SurfaceType_ = VSPGEOM_SURFACE;
    
    // Read in xyz data
    
    fscanf(VSPGeom_File,"%d",&NumNodes);

    PRINTF("NumNodes: %d \n",NumNodes);
        
    // Estimate the maximum number of grid levels
    
    MaxNumberOfGridLevels_ = 4*(int) ( log(1.0 * NumNodes) / log(4.0) );
    
    PRINTF("MaxNumberOfGridLevels_: %d \n",MaxNumberOfGridLevels_);
        
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
    
    // Read in the tri data

    fscanf(VSPGeom_File,"%d\n",&NumLoops);

    PRINTF("NumLoops: %d \n",NumLoops);    

    Grid().SizeLoopList(NumLoops);
    
    // Read in connectivity

    InputMeshIsMixedPolys_ = 0;

    for ( n = 1 ; n <= NumLoops ; n++ ) {
       
    //   fscanf(VSPGeom_File,"%d %d %d %d \n",&DumInt, &Node1,&Node2,&Node3);
       
       fgets(DumChar,2000,VSPGeom_File);

       Next = strtok(DumChar,Space);

       NumTriNodes = atoi(Next);

       Grid().LoopList(n).SizeNodeList(NumTriNodes);
                      
       Grid().LoopList(n).SizeEdgeList(NumTriNodes);

       for ( i = 1 ; i <= NumTriNodes ; i++ ) {
          
          Next = strtok(NULL,Space);
         
          Grid().LoopList(n).Node(i) = atoi(Next);
          
       }

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
              
       //fscanf(VSPGeom_File,"%d %d %lf %lf %lf %lf %lf %lf \n",&SurfaceID,&DumInt,&u1,&v1,&u2,&v2,&u3,&v3);

       fgets(DumChar,2000,VSPGeom_File);
       
       Next = strtok(DumChar,Space); SurfaceID = atoi(Next);
       Next = strtok(NULL,Space);    DumInt    = atoi(Next);
       
       for ( i = 1 ; i <= Grid().LoopList(n).NumberOfNodes() ; i++ ) {
          
          Next = strtok(NULL,Space); u = atof(Next);
          Next = strtok(NULL,Space); v = atof(Next);          
            
          Grid().LoopList(n).U_Node(i) = u;
          Grid().LoopList(n).V_Node(i) = v;
   
       }

       Grid().LoopList(n).SurfaceID()         = SurfaceID;
        
       Grid().LoopList(n).ComponentID()       = SurfaceID;
       
       Grid().LoopList(n).SurfaceType()       = VSPGEOM_SURFACE;
       
       Grid().LoopList(n).VortexSheet()       = 0;
       
       Grid().LoopList(n).SpanStation()       = 0;

       Grid().LoopList(n).IsTrailingEdgeTri() = 0;       
       
       SurfaceIsUsed[SurfaceID] = 1;

    }

    // Read in the number of kutta lists
    
    NumberOfKuttaNodes_ = 0;
    
    printf("Sizing kutta node list to length of: %d \n",NumNodes);fflush(NULL);
        
    KuttaNodeList_ = new int[NumNodes + 1];
    
    fscanf(VSPGeom_File,"%d",&NumKuttaNodeLists);

    for ( j = 1 ; j <= NumKuttaNodeLists ; j++ ) {
    
       // Read in the Kutta nodes in this list
       
       fscanf(VSPGeom_File,"%d",&NumNodesInList);
       
    //   PRINTF("NumNodesInList: %d \n",NumNodesInList);fflush(NULL);
       
       if ( NumNodesInList > 0 ) {
          
          for ( i = 1 ; i <= NumNodesInList ; i++ ) {
             
             fscanf(VSPGeom_File,"%d",&(KuttaNodeList_[++NumberOfKuttaNodes_]));
             
           //  PRINTF("KuttaNodeList_[%d]: %d \n",NumberOfKuttaNodes_,KuttaNodeList_[NumberOfKuttaNodes_]);
             
          }
          
       }
       
    }
    
    PRINTF("VSPGEOM defined NumKuttaNodes: %d \n",NumberOfKuttaNodes_);

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
    
    PRINTF("Found %d VSPGEOM Surfaces \n",NumberOfSurfaces_); fflush(NULL);
    
    SurfaceNameList_ = new char*[NumberOfSurfaces_ + 1];
    
    SurfaceIsThick_ = new int[NumberOfSurfaces_ + 1];
    
    ComponentIsThick_ = new int[NumberOfSurfaces_ + 1];
    
    BoundaryConditionForSurface_ = new BOUNDARY_CONDITION_DATA[NumberOfSurfaces_ + 2];
    
    ComponentIDForSurface_ = new int[NumLoops + 1];
    
    zero_int_array(ComponentIDForSurface_, NumLoops);
    
    PRINTF("Found %d VSPGEOM Surfaces \n",NumberOfSurfaces_);
    
    if ( VKEY_File != NULL ) {
       
       // Read in the vkey headers
       
       fgets(DumChar,2000,VKEY_File);PRINTF("DumChar: %s \n",DumChar); fflush(NULL);
       fgets(DumChar,2000,VKEY_File);PRINTF("DumChar: %s \n",DumChar); fflush(NULL);
       
       // Get the number of surfaces in the vkey file
       
       fgets(DumChar,2000,VKEY_File);PRINTF("DumChar: %s \n",DumChar); fflush(NULL);
       
       sscanf(DumChar,"%d\n",&NumberOfVSPSurfaces);
       
       PRINTF("NumberOfVSPSurfaces: %d \n",NumberOfVSPSurfaces);

       // Skip a blank line
              
       fgets(DumChar,2000,VKEY_File);PRINTF("DumChar: %s \n",DumChar); fflush(NULL);
       
       // Read in what data exists
       
       fgets(DumChar,2000,VKEY_File);PRINTF("DumChar: %s \n",DumChar); fflush(NULL);

       // Check for thick / thin flag
       
       ThickThinDataExists = 0;
       
       if ( strstr(DumChar,"thick") != NULL ) ThickThinDataExists = 1;
       
       if ( ThickThinDataExists ) {
          
          PRINTF("Thick/thin data exists... DumChar: %s \n",DumChar);
          
       }
       
       else {
          
          PRINTF("No thick/thin identifiers found in vkey file: %s \n",DumChar);
          
       }
             
       k = 0;
       
       for ( n = 1 ; n <= NumberOfVSPSurfaces ; n++ ) {
          
          fgets(DumChar,2000,VKEY_File);
          
        //  sscanf(DumChar,"%d,%d,%d,%s,%s",&DumInt,&CompID,&DumInt2,DumChar2,DumChar);

        //# part#,geom#,surf#,gname,gid,thick
                

          Next = strtok(DumChar,Comma);

          DumInt = atoi(Next);
          
          Next = strtok(NULL,Comma);
          
          CompID = atoi(Next);
          
          Next = strtok(NULL,Comma);

          Next = strtok(NULL,Comma);  Next[strcspn(Next, "\n")] = 0; SPRINTF(TempName,"%s",Next); 
          
          Next = strtok(NULL,Comma);
          
          if ( ThickThinDataExists ) Next = strtok(NULL,Comma);
                    
          if ( SurfaceIsUsed[DumInt] ) {

             PRINTF("Surface: %d exists in triangulation and has OpenVSP Name: %s \n",DumInt,TempName);
             
             k++;
                          
             PRINTF("CompID: %d \n",CompID);
             
             ComponentIDForSurface_[DumInt] = CompID + 1;
             
             SurfaceNameList_[k] = new char[2000];
             
             SPRINTF(SurfaceNameList_[k],"%s",TempName);    

             SurfaceIsThick_[k] = 0;
  
             if ( ThickThinDataExists ) SurfaceIsThick_[k] = atoi(Next);
             
             if ( SurfaceIsThick_[k] ) {
                
                PRINTF("Surface: %d is thick \n",k);
                
             }
             
             else {
             
                PRINTF("Surface: %d is thin \n",k);
                
             }
                                       
          }      
          
       }
      
    }

    else {
       
       PRINTF("Could not find vkey file... exiting! \n");
       fflush(NULL);exit(1);
       
    }
 
    // Renumber the surfaces
   
    PRINTF("Renumbering surfaces \n");fflush(NULL);
   
    for ( n = 1 ; n <= NumLoops ; n++ ) {
 
       Done = 0;
       
       i = 1;
       
       while ( !Done && i <= NumberOfSurfaces_) {
          
          if ( Grid().LoopList(n).SurfaceID() == SurfaceList[i] ) {
             
             Done = 1;
 
             Grid().LoopList(n).ComponentID() = ComponentIDForSurface_[Grid().LoopList(n).SurfaceID()];

             Grid().LoopList(n).SurfaceID() = i;

          }
          
          i++;
          
       }
       
       if ( !Done ) {
          
          PRINTF("Error in determing number of surfaces in VSPGEOM file! \n");
          fflush(NULL);
          exit(1);
          
       }
       
    }
    
    // Renumber surface patch to componenent ID list 
    
    PRINTF("Renumbering component IDs \n");fflush(NULL);
    
    if ( VKEY_File != NULL ) {
       
       TempList = new int[NumberOfVSPSurfaces + 1];
    
       n = 0;
       
       for ( i = 1 ; i <= NumLoops ; i++ ) {
          
          if ( SurfaceIsUsed[i] ) {
          
             TempList[++n] = ComponentIDForSurface_[i];
             
          }
          
       }
                    
       delete [] ComponentIDForSurface_;
                 
       ComponentIDForSurface_ = TempList;

    }
    
    delete [] SurfaceList;

    // Mark the tris on thick and thin surfaces
    
    PRINTF("Marking tris on thick and thin surfaces \n");fflush(NULL);
    
    for ( n = 1 ; n <= NumLoops ; n++ ) {

       if ( SurfaceIsThick_[Grid().LoopList(n).SurfaceID()] ) {
        
          Grid().LoopList(n).SurfaceType() = THICK_SURFACE;
    
       }
       
       else {
        
          Grid().LoopList(n).SurfaceType() = THIN_SURFACE;
          
       }          
   
    }   
        
    // Determine the number of thin and thick surfaces
    
    PRINTF("Determining the number of thin and thick surfaces \n");fflush(NULL);
    
    NumberOfThinSurfaces_ = NumberOfThickSurfaces_ = 0;
    
    for ( k = 1 ; k <= NumberOfSurfaces_ ; k++ ) {
    
       if ( !SurfaceIsThick_[k] ) NumberOfThinSurfaces_++;
       
       if (  SurfaceIsThick_[k] ) NumberOfThickSurfaces_++;
       
    }
    
    PRINTF("NumberOfThinSurfaces_: %d \n",NumberOfThinSurfaces_);
    
    PRINTF("NumberOfThickSurfaces_: %d \n",NumberOfThickSurfaces_);
    
    ModelType_ = PANEL_MODEL;
    
    if ( NumberOfThickSurfaces_ == 0 ) ModelType_ = VLM_MODEL;
    
    if ( NumberOfThinSurfaces_  == 0 ) ModelType_ = PANEL_MODEL;

    // Determine the number of unique components
    
    ComponentIDForComponent_ = new int[NumberOfSurfaces_ + 1];
    
    zero_int_array(ComponentIDForComponent_, NumberOfSurfaces_);
    
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
    
    PRINTF("Found %d components \n",NumberOfComponents_);
          
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
             
             PRINTF("Error in marking components as thick or thin... \n");fflush(NULL);
             exit(1);
             
          }
          
       }
                          
    }
    
    NumberOfThinComponents_ = NumberOfThickComponents_ = 0;
    
    for ( i = 1 ; i <= NumberOfComponents_ ; i++ ) {
       
       if ( !ComponentIsThick_[i] ) {
          
          NumberOfThinComponents_++;
          
          PRINTF("Component: %d with ID: %d is thin \n",i,ComponentIDForComponent_[i]);
          
       }
                             
       if (  ComponentIsThick_[i] ) {
          
          NumberOfThickComponents_++;
          
          PRINTF("Component: %d with ID: %d is thick \n",i,ComponentIDForComponent_[i]);
          
       }

    }
    
    // If the mesh we just read in is a n-gon mesh, we also need to read in the
    // triangulated version
    
    if ( InputMeshIsMixedPolys_ ) {

       fgets(DumChar,2000,VSPGeom_File);
           
       fgetpos(VSPGeom_File,&TopOfTriangluation);
       
       // Loop over list to determine the total number of tris
       
       NumTris = 0;
       
       for ( n = 1 ; n <= NumLoops ; n++ ) {
          
          fgets(DumChar,2000,VSPGeom_File);
      
          Next = strtok(DumChar,Space);
          Next = strtok(NULL,Space); NumTris += atoi(Next);
          
       }
       
       PRINTF("Pure tri mesh has %d tris \n",NumTris);
   
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
   
       }
   
       GridF_->SizeTriList(NumTris);
       
       // Read in the tri connectivity
       
       NumTris = 0;
       
       for ( n = 1 ; n <= NumLoops ; n++ ) {
          
          fgets(DumChar,2000,VSPGeom_File);
   
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
             
             GridF_->LoopList(NumTris).SurfaceID()         = Grid().LoopList(n).SurfaceID();
              
             GridF_->LoopList(NumTris).ComponentID()       = Grid().LoopList(n).ComponentID();
             
             GridF_->LoopList(NumTris).SurfaceType()       = Grid().LoopList(n).SurfaceType();
                                                      
             GridF_->LoopList(NumTris).VortexSheet()       = Grid().LoopList(n).VortexSheet();
                                                      
             GridF_->LoopList(NumTris).SpanStation()       = Grid().LoopList(n).SpanStation();
             
             GridF_->LoopList(NumTris).IsTrailingEdgeTri() = Grid().LoopList(n).IsTrailingEdgeTri();
             
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
                   
                   PRINTF("Error in determining UV data for triangulated version of NGON mesh! \n");
                   fflush(NULL);exit(1);
                   
                }
                
             }
                              
          }


       }

       PRINTF("After building ...pure tri mesh has %d tris \n",NumTris);
       
       // Swap the grids

       GridC_ = Grid_[0];
       
       Grid_[0] = GridF_;
                  
    }
       
}

/*##############################################################################
#                                                                              #
#                       VSP_GEOM RotateGeometry_About_Y_Axis                   #
#                                                                              #
##############################################################################*/

void VSP_GEOM::RotateGeometry_About_Y_Axis(void)  
{

    int i, j;
    VSPAERO_DOUBLE x, y, z, RotAngle, DeltaHeight, Xcg, Zcg; 
       
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
    int *KuttaEdgeList, *IncidentKuttaEdges, *IsKuttaEdge, *PermArray, *NodeUsed;
    int *NodeIsSharp, CurrentComponentID;
    VSPAERO_DOUBLE vec1[3], vec2[3], vec3[3], Xvec[3], dot, angle, mag1, mag2;
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
        
    IncidentKuttaEdges = new int[Grid().NumberOfNodes() + 1];
    
    zero_int_array(IncidentKuttaEdges, Grid().NumberOfNodes());
    
    Xvec[0] = 1.;
    Xvec[1] = 0.;
    Xvec[2] = 0.;
    
    for ( i = 1 ; i <= Grid().NumberOfEdges() ; i++ ) {
       
       Node1 = Grid().EdgeList(i).Node1();
       Node2 = Grid().EdgeList(i).Node2();
       
       // Geometry provided list of sharp nodes... use it!
       
       if ( NumberOfSharpNodes ) {
          
          if ( NodeIsSharp[Node1] && NodeIsSharp[Node2] ) {
      
              IncidentKuttaEdges[Node1]++;
              
              IncidentKuttaEdges[Node2]++;
              
              IsKuttaEdge[i] = 1;     

          } 
          
       }       
       
       // Check if edge is mostly normal to the free stream flow
       
       else {
          
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
                     
                     IncidentKuttaEdges[Node1]++;
                     
                     IncidentKuttaEdges[Node2]++;
               
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

          if ( ( SurfaceAtNodeIsConvex(Node1) || IncidentKuttaEdges[Node1] >= 2 ) || 
               ( SurfaceAtNodeIsConvex(Node2) || IncidentKuttaEdges[Node2] >= 2 ) ) {
         
             // Check Node 1
             
             if ( SurfaceAtNodeIsConvex(Node1) || IncidentKuttaEdges[Node1] >= 2 ) {
              
                KuttaNodeList[Node1].IsKuttaNode = 1;
                
                if ( KuttaNodeList[Node1].Edge1 == 0 ) {
                   
                   KuttaNodeList[Node1].Edge1 = i;
                   
                }
                
                else {
                  
                   KuttaNodeList[Node1].Edge2 = i;
                   
                }
                
             }
             
             // Check Node 2
             
             if ( SurfaceAtNodeIsConvex(Node2) || IncidentKuttaEdges[Node2] >= 2 ) {
               
                KuttaNodeList[Node2].IsKuttaNode = 1;
                
                if ( KuttaNodeList[Node2].Edge1 == 0 ) {
                   
                   KuttaNodeList[Node2].Edge1 = i;
                   
                }
                
                else {
                  
                   KuttaNodeList[Node2].Edge2 = i;
                   
                }
             
             }

             KuttaEdgeList[++NumberOfKuttaEdges] = i;
         
          }
  
       }
       
    }

    // Determine the number of kutta nodes
    
    NumberOfKuttaNodes = 0;
    
    for ( i = 1 ; i <= Grid().NumberOfNodes() ; i++ ) {
       
       NumberOfKuttaNodes += KuttaNodeList[i].IsKuttaNode;
       
    }       
    
    PRINTF("NumberOfKuttaNodes after trimming: %d \n",NumberOfKuttaNodes);

    // Size the kutta node list

    Grid().SizeKuttaNodeList(NumberOfKuttaNodes);  
    
    // Mark the kutta edges
    
    for ( i = 1 ; i <= NumberOfKuttaEdges ; i++ ) {
       
       Node1 = Grid().EdgeList(KuttaEdgeList[i]).Node1();
       Node2 = Grid().EdgeList(KuttaEdgeList[i]).Node2();
       
       if ( KuttaNodeList[Node1].IsKuttaNode || KuttaNodeList[Node2].IsKuttaNode ) {
                    
          Grid().EdgeList(KuttaEdgeList[i]).IsTrailingEdge() = 1;
          
          if (Verbose_ ) PRINTF("Marking %d kutta edge %d as a trailing edge... \n",i,KuttaEdgeList[i]);
          
       }
      
    }
    
    // Iterate until all the kutta nodes are assigned to a vortex sheet
    
    NodeUsed = new int[Grid().NumberOfNodes() + 1];

    zero_int_array(NodeUsed, Grid().NumberOfNodes());
    
    PermArray = new int[NumberOfKuttaNodes + 1];
  
    p = pold = 0;
    
    VortexSheet = 1;
    
    PRINTF("NumberOfKuttaNodes: %d \n",NumberOfKuttaNodes);
    PRINTF("NumberOfKuttaEdges: %d \n",NumberOfKuttaEdges);
    
    while ( p < NumberOfKuttaNodes ) {
       
       PRINTF("Working on VortexSheet: %d \n",VortexSheet);fflush(NULL);
       
       SheetIsPeriodic = 0;
       
       // Try NOT to start with a wing tip node
       
       Done = 0;
       
       k = 1;
   
       while ( k <= Grid().NumberOfNodes() && !Done ) {
       
          if ( KuttaNodeList[k].IsKuttaNode && IncidentKuttaEdges[k] != 2 && NodeUsed[k] == 0 ) Done = 1;
          
          k++;
          
       }
       
       k--;
       
       if ( Done ) PRINTF("Found a starting wake sheet node... %d \n",k);fflush(NULL);

       // If we did not find a wing tip node... then this wake is either closed like a nacelle,
       // or it could be something like a pylon with no tips... so now we have to figure that out
       
       // Find any ones >= 3 
       
       // Find a kutta edge that only has one node marked as a kutta node
       
       if ( !Done ) {
          
          PRINTF("Did not find a singly marked wake sheet node... \n");fflush(NULL);
          
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
          
          if ( Done ) PRINTF("Found node: %d \n",k);
          
       }
               
       // Find any kutta node... must be a loop now
              
       SheetIsPeriodic = 0;
   
       if ( !Done ) {
          
          PRINTF("Did not find any singly marked kutta node... \n");fflush(NULL);
          
          k = 1;
   
          while ( k <= Grid().NumberOfNodes() && !Done ) {
       
             if ( KuttaNodeList[k].IsKuttaNode && NodeUsed[k] == 0 ) Done = 1;
             
             k++;
             
          }
          
          k--;
          
          SheetIsPeriodic = 1;
          
          if ( SheetIsPeriodic ) PRINTF("Sheet is periodic... \n");fflush(NULL);
          
       }
 
       // Loop over kutta nodes and sort
       
       zero_int_array(PermArray, NumberOfKuttaNodes);
   
       Done = 0;
       
       Next = 1;
       
       Node = k;
       
       CurrentComponentID = Grid().NodeList(Node).ComponentID();
       
       PermArray[Next] = Node;
       
       NodeUsed[Node] = 1;
    
       while ( !Done ) {
          
          Done = 1;
          
          Node = PermArray[Next];
         
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

       PRINTF("VortexSheet: %d finished... which has %d kutta nodes ... and %d out of %d kutta nodes are now allocated \n",VortexSheet,p-pold,p,NumberOfKuttaNodes);fflush(NULL);
       
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

       if ( Loop1 == Loop2) {

          Grid().EdgeList(i).IsBoundaryEdge() = 1; // djk 2
 
       }

    }   
    
    // Search for wing tip kutta nodes
    
    for ( i = 1 ; i <= NumberOfKuttaEdges ; i++ ) {
       
       Node1 = Grid().EdgeList(KuttaEdgeList[i]).Node1();
       Node2 = Grid().EdgeList(KuttaEdgeList[i]).Node2();
    
       Node = 0;
       
       if ( IncidentKuttaEdges[Node1] == 1 ) Node = Node1;
       if ( IncidentKuttaEdges[Node2] == 1 ) Node = Node2;
      
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
                  
                  //PRINTF("Kutta node is on symmetry plane! \n"); fflush(NULL);
                  
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
    
    VSPAERO_DOUBLE *Uval;
    
    Uval = new VSPAERO_DOUBLE[Grid().NumberOfNodes() + 1];
    
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
    
    delete [] NodeToTriList_;
    delete [] NumberOfTrisForNode_;
    
    delete [] IsKuttaEdge;
    delete [] IncidentKuttaEdges;
    
}

/*##############################################################################
#                                                                              #
#                         VSP_GEOM SurfaceAtNodeIsConvex                       #
#                                                                              #
##############################################################################*/

int VSP_GEOM::SurfaceAtNodeIsConvex(int Node)
{

    int i, j, k, Tri;
    VSPAERO_DOUBLE Vec[3], VecAvg[3], P1[3], P2[3], Dot, Angle;
   
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

#include "END_NAME_SPACE.H"
