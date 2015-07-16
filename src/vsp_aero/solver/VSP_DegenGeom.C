//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_DegenGeom.H"

/*##############################################################################
#                                                                              #
#                         VSP_DEGEN_GEOM constructor                           #
#                                                                              #
##############################################################################*/

VSP_DEGEN_GEOM::VSP_DEGEN_GEOM(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                             VSP_DEGEN_GEOM init                              #
#                                                                              #
##############################################################################*/

void VSP_DEGEN_GEOM::init(void)
{

    Verbose_ = 0;
    
}

/*##############################################################################
#                                                                              #
#                            VSP_DEGEN_GEOM Copy                               #
#                                                                              #
##############################################################################*/

VSP_DEGEN_GEOM::VSP_DEGEN_GEOM(const VSP_DEGEN_GEOM &VSP_DegenGeom)
{

    init();

    // Just * use the operator = code

    *this = VSP_DegenGeom;

}

/*##############################################################################
#                                                                              #
#                           VSP_DEGEN_GEOM operator=                           #
#                                                                              #
##############################################################################*/

VSP_DEGEN_GEOM& VSP_DEGEN_GEOM::operator=(const VSP_DEGEN_GEOM &VSP_DegenGeom)
{

    return *this;

}

/*##############################################################################
#                                                                              #
#                        VSP_DEGEN_GEOM destructor                             #
#                                                                              #
##############################################################################*/

VSP_DEGEN_GEOM::~VSP_DEGEN_GEOM(void)
{


}

/*##############################################################################
#                                                                              #
#                         VSP_DEGEN_GEOM ReadFile                              #
#                                                                              #
##############################################################################*/

void VSP_DEGEN_GEOM::ReadFile(char *FileName)
{
 
    int Wing, Done, NumberOfBodySets, BodySet;
    int Surface;
    double Diam, x, y, z, nx, ny, nz;
    char VSP_File_Name[2000], DumChar[2000], Type[2000], Name[2000];
    FILE *VSP_Degen_File;
    
    // Open setup file

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
    
    // Now scan the file and determine how many wings
    
    NumberOfWings_ = 0;
    
    Done = 0;
    
    while ( !Done ) {
     
       if ( fgets(DumChar,1000,VSP_Degen_File) == NULL ) Done = 1;   
       
       if ( strncmp(DumChar,"LIFTING_SURFACE",15) == 0 ) NumberOfWings_++;
       
    }
    
    printf("NumberOfWings_: %d \n",NumberOfWings_);
    
    rewind(VSP_Degen_File);
    
    // Now scan the file and determine how many bodies
    
    NumberOfBodies_ = 0;
    
    Done = 0;
    
    while ( !Done ) {
     
       if ( fgets(DumChar,1000,VSP_Degen_File) == NULL ) Done = 1;   
       
       if ( strncmp(DumChar,"BODY",4) == 0 ) NumberOfBodies_++;
       
    }    

    rewind(VSP_Degen_File);

    // Now scan the file and determine how actuator disks

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
    
    NumberOfBodySets = NumberOfBodies_;
    
    NumberOfBodies_ *= 4;

    printf("NumberOfBodies_: %d \n",NumberOfBodies_);

    rewind(VSP_Degen_File);

    // Now size the surface list
    
    NumberOfSurfaces_ = NumberOfWings_ + NumberOfBodies_;

    VSP_Surface_ = new VSP_SURFACE[NumberOfSurfaces_ + 1];
    
    // Read in the wing data
    
    Surface = 0;
    
    for ( Wing = 1 ; Wing <= NumberOfWings_ ; Wing++ ) {
     
       Surface++;
    
       Done = 0;
       
       while ( !Done ) {

          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strncmp(DumChar,"LIFTING_SURFACE",15) == 0 ) Done = 1;
          
       }

       sscanf(DumChar,"%15s,%s",Type,Name);

       printf("Working on reading wing: %d --> %s \n",Wing,Name);
  
       VSP_Surface(Surface).ReadWingDataFromFile(Name,VSP_Degen_File);
       
    }
        
    // Read in the body data

    for ( BodySet = 1 ; BodySet <= NumberOfBodySets ; BodySet++ ) {
        
       // Load in the vertical slices
    
       rewind(VSP_Degen_File);
    
       Surface++;
            
       Done = 0;
       
       while ( Done < BodySet ) {

          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strncmp(DumChar,"BODY",4) == 0 ) Done++;
                    
       }

       sscanf(DumChar,"%4s,%s",Type,Name);

       printf("Working on reading body: %d --> %s \n",BodySet,Name);

       if ( Verbose_ ) printf("Working on reading vertical slice for body: %d --> %s \n",BodySet,Name);
              
       VSP_Surface(Surface).ReadBodyDataFromFile(Name,2,VSP_Degen_File);

       // Load in the vertical slices
    
       rewind(VSP_Degen_File);
    
       Surface++;
            
       Done = 0;
       
       while ( Done < BodySet ) {

          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strncmp(DumChar,"BODY",4) == 0 ) Done++;
                    
       }

       sscanf(DumChar,"%4s,%s",Type,Name);

       if ( Verbose_ ) printf("Working on reading vertical slice for body: %d --> %s \n",BodySet,Name);
              
       VSP_Surface(Surface).ReadBodyDataFromFile(Name,1,VSP_Degen_File);
       
       // Load in the horizontal slices
       
       rewind(VSP_Degen_File);
     
       Surface++;
            
       Done = 0;
       
       while ( Done < BodySet ) {

          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strncmp(DumChar,"BODY",4) == 0 ) Done++;
                    
       }
       
       sscanf(DumChar,"%4s,%s",Type,Name);

       if ( Verbose_ ) printf("Working on reading horizontal slice for body: %d --> %s \n",BodySet,Name);
              
       VSP_Surface(Surface).ReadBodyDataFromFile(Name,3,VSP_Degen_File);     
    
       // Load in the horizontal slices
       
       rewind(VSP_Degen_File);
     
       Surface++;
            
       Done = 0;
       
       while ( Done < BodySet ) {

          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strncmp(DumChar,"BODY",4) == 0 ) Done++;
                    
       }
       
       sscanf(DumChar,"%4s,%s",Type,Name);

       if ( Verbose_ ) printf("Working on reading horizontal slice for body: %d --> %s \n",BodySet,Name);
              
       VSP_Surface(Surface).ReadBodyDataFromFile(Name,4,VSP_Degen_File);    
       
    }
    
    fclose(VSP_Degen_File);
    
    printf("Done loading in geometry! \n");fflush(NULL);

    // Create meshes for the VSP geometries
    
    MeshGeom();

}

/*##############################################################################
#                                                                              #
#                         VSP_DEGEN_GEOM MeshGeom                              #
#                                                                              #
##############################################################################*/

void VSP_DEGEN_GEOM::MeshGeom(void)
{
 
    int i, Surface, NumberOfNodes, NumberOfLoops, NumberOfEdges, NumberOfKuttaNodes;
    int MaxNumberOfGridLevels, NodeOffSet;
    
    // Loop over the surface and create a mesh for each

    for ( Surface = 1 ; Surface <= NumberOfSurfaces_ ; Surface++ ) {
     
       VSP_Surface(Surface).CreateMesh(Surface);
       
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
          
          Grid().KuttaNode(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().KuttaNode(i) + NodeOffSet;

          Grid().WakeTrailingEdgeX(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WakeTrailingEdgeX(i);
          Grid().WakeTrailingEdgeY(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WakeTrailingEdgeY(i);
          Grid().WakeTrailingEdgeZ(NumberOfKuttaNodes) = VSP_Surface(Surface).Grid().WakeTrailingEdgeZ(i);
          
       }
       
       NodeOffSet += VSP_Surface(Surface).Grid().NumberOfNodes();
       
    }
    
    Grid().CalculateTriNormalsAndCentroids();
    
    Grid().CreateTriEdges();
    
    Grid().CalculateUpwindEdges();   
    
    Grid().CreateUpwindEdgeData();
    
    printf("Total NumberOfNodes:      %d \n",Grid().NumberOfNodes());
    printf("Total NumberOfLoops:      %d \n",Grid().NumberOfLoops());
    printf("Total NumberOfEdges:      %d \n",Grid().NumberOfEdges());
    printf("Total NumberOfKuttaNodes: %d \n",Grid().NumberOfKuttaNodes());
     
    printf("Agglomerating mesh... \n");fflush(NULL);

    VSP_AGGLOM Agglomerate;
    
    double Work, Work_0;
    
    Work = Work_0 = Grid_[0]->NumberOfLoops()*Grid_[0]->NumberOfEdges();
    
    printf("Grid:%d --> # loops: %10d ...# Edges: %10d \n",0,Grid_[0]->NumberOfLoops(),Grid_[0]->NumberOfEdges());
    
    // First attempt to simplify the grid

    Grid_[1] = Agglomerate.SimplifyMesh(*(Grid_[0]));

    Grid_[1]->CalculateUpwindEdges();   
 
    Grid_[1]->CreateUpwindEdgeData();

    Work = Grid_[1]->NumberOfLoops()*Grid_[1]->NumberOfEdges();
    
    printf("Grid:%d --> # loops: %10d ...# Edges: %10d \n",1,Grid_[1]->NumberOfLoops(),Grid_[1]->NumberOfEdges());
        
    i = 2;
    
    while ( i < MaxNumberOfGridLevels && Grid_[i-1]->NumberOfLoops() > NumberOfSurfaces_ ) {

       Grid_[i] = Agglomerate.Agglomerate(*(Grid_[i-1]));
       
       Grid_[i]->CalculateUpwindEdges();   
       
       Grid_[i]->CreateUpwindEdgeData();
           
       Work = Grid_[i]->NumberOfLoops()*Grid_[i]->NumberOfEdges();
       
       printf("Grid:%d --> # loops: %10d ...# Edges: %10d  \n",i,Grid_[i]->NumberOfLoops(),Grid_[i]->NumberOfEdges());
       
       i++;

    }
    
    NumberOfGridLevels_ = i;

    printf("NumberOfGridLevels_: %d \n",NumberOfGridLevels_);    

    
}

/*##############################################################################
#                                                                              #
#                         VSP_DEGEN_GEOM AgglomerateMeshes                     #
#                                                                              #
##############################################################################*/

void VSP_DEGEN_GEOM::AgglomerateMeshes(void)
{
 
    int Surface;
   
    for ( Surface = 1 ; Surface <= NumberOfSurfaces_ ; Surface++ ) {
     
       VSP_Surface(Surface).AgglomerateMesh();
       
    }
    
    printf("Done agglomerating meshes! \n");fflush(NULL);
     
}


