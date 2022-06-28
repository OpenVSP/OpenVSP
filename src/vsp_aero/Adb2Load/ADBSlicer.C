//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "ADBSlicer.H"

/*##############################################################################
#                                                                              #
#                            ADBSLICER Constructor                             #
#                                                                              #
##############################################################################*/

ADBSLICER::ADBSLICER(void)
{

    // Aerothermal database data

    NumberOfMachs   = 0;
    NumberOfBetas   = 0;
    NumberOfAlphas  = 0;
    
    Nx = NULL;
    Ny = NULL;
    Nz = NULL;

    Cp        = NULL;
    MachList  = NULL;
    BetaList  = NULL;
    AlphaList = NULL;

    // Data for tris and nodes

    NumberOfTris  = 0;
    NumberOfNodes = 0;
    NumberOfEdges = 0;

    TriList_  = NULL;
    NodeList_ = NULL;
    EdgeList_ = NULL;

    // Mesh Min/Max

    XMin = 0.;
    XMax = 0.;

    YMin = 0.;
    YMax = 0.;

    ZMin = 0.;
    ZMax = 0.;

    // Propulsion stuff
    
    NumberOfPropulsionElements = 0;
    
    PropulsionElement = NULL;
    
    RotateGeometry = 0;
    
    CosRot = SinRot = 0.;
    
    ByteSwapForADB = 0;
    
    GnuPlot_ = 0;
    
    FindClosestNode_ = 0;
    
    DynamicPressure_ = 1.;
    
    BoundaryTolerance_ = 1.e-4;

    NodeOffSet_= 0;
    
    ElementOffSet_ = 0;

    MaxCalculixNode_ = 0;
    
    MaxCalculixElement_ = 0;
    
    Verbose_ = 1;

    MaxStress_ = 12960000.;
    
    MaxSkinThickness_ = 0.05;
    
    MinSkinThickness_ = 0.01333333;
    
    OptFact_ = 1.;
    
    sprintf(ElementLabel_,"NONE");
  
}

/*##############################################################################
#                                                                              #
#                            ADBSLICER destructor                              #
#                                                                              #
##############################################################################*/

ADBSLICER::~ADBSLICER(void)
{


}

/*##############################################################################
#                                                                              #
#                              ADBSLICER LoadFile                              #
#                                                                              #
##############################################################################*/

void ADBSLICER::LoadFile(char *name)
{

    int i;
    char CommandLine[10000], file_name_w_ext[10000];
    FILE *adb_file;
    
    // Save the file name

    sprintf(file_name,"%s",name);

    // Determine if an adb file exists
    
    sprintf(file_name_w_ext,"%s.adb",file_name);

    if ( (adb_file = fopen(file_name_w_ext,"rb")) != NULL ) {
     
       fclose(adb_file); // It's reopened later

       // Load in the Mesh

       LoadMeshData();
       
       // Calculate the surface normals
   
       CalculateSurfaceNormals(1);
   
       // Create the tri to edge, and edge to tri pointers
   
       CreateTriEdges();
              
       // Load ADB Case list

       LoadSolutionCaseList();
 
    }
    
    // Otherwise exit...
    
    else {
     
       printf("No %s.adb file found! \n",file_name);
       printf("Please run vspaero first. \n");
       printf("You can run vspaero with the -geom option to only write out a viewable geometry. \n");
       fflush(NULL);
       exit(1);  
 
    }
   
}

/*##############################################################################
#                                                                              #
#                           ADBSLICER SliceGeometry                            #
#                                                                              #
##############################################################################*/

void ADBSLICER::SliceGeometry(char *name)
{

    int i;
    char CommandLine[10000], file_name_w_ext[10000];
    FILE *cuts_file;
    
    // Save the file name

    sprintf(file_name,"%s",name);

    // Determine if an cut file exists
    
    sprintf(file_name_w_ext,"%s.cuts",file_name);

    if ( (cuts_file = fopen(file_name_w_ext,"r")) != NULL ) {
     
       fclose(cuts_file); // It's reopened later

       // Load in the cut list file

       LoadCutsFile();
     
       // Load in the solution data and slice it
       
       sprintf(file_name_w_ext,"%s.slc",file_name);

       if ( (SliceFile = fopen(file_name_w_ext,"w")) != NULL ) {
          
          for ( i = 1 ; i <= NumberOfADBCases_ ; i++ ) {

             LoadSolutionData(i);

             FindSolutionMinMax();
            
             Slice(i);
             
          }
             
          fclose(SliceFile);
          
       }
             
    }
    
    // Otherwise exit...
    
    else {
     
       printf("No %s.cuts file found! \n",file_name);
       fflush(NULL);
       exit(1);  
 
    }
   
}

/*##############################################################################
#                                                                              #
#                  ADBSLICER ParseCalculixFile                                 #
#                                                                              #
##############################################################################*/

void ADBSLICER::ParseCalculixFile(char *name)
{

    int i;
    char CommandLine[10000], file_name_w_ext[10000];
    FILE *InpFile;
    INTERP Interp;

    if ( !AddLabel_ ) sprintf(Label_,"");
        
    // Save the file name

    sprintf(CalculixFileName,"%s",name);

    // Determine if an calculix file exists
    
    sprintf(file_name_w_ext,"%s.inp",CalculixFileName);

    if ( (InpFile = fopen(file_name_w_ext,"r")) != NULL ) {
     
       fclose(InpFile); // It's reopened later

       // Load in Calculix file
       
       LoadCalculixINPFileSurfaceElements(CalculixFileName);
       
       // Output statistics
       
       if ( Verbose_ ) printf("Max node number: %d \n",MaxCalculixNode_);
       if ( Verbose_ ) printf("Max element number: %d \n",MaxCalculixElement_);
       
       NodeOffSet_ = (MaxCalculixNode_/10000)*10000 + 20000;
       
       ElementOffSet_ = (MaxCalculixElement_/10000)*10000 + 20000;
       
       if ( Verbose_ ) printf("Recommended Node/Element OffSets: %d %d \n",NodeOffSet_, ElementOffSet_);
       
       if ( !Verbose_ ) printf("%d %d \n",NodeOffSet_, ElementOffSet_);
      
    }
    
}

/*##############################################################################
#                                                                              #
#                  ADBSLICER FindNearestNodeInCalculixFile                     #
#                                                                              #
##############################################################################*/

void ADBSLICER::FindNearestNodeInCalculixFile(char *name, float *xyz)
{

    int i;
    char CommandLine[10000], file_name_w_ext[10000];
    FILE *InpFile;
    INTERP Interp;
    
    FindClosestNode_ = 1;
    
    xyz_find_[0] = xyz[0];
    xyz_find_[1] = xyz[1];
    xyz_find_[2] = xyz[2];
    
    if ( !AddLabel_ ) sprintf(Label_,"");
        
    // Save the file name

    sprintf(CalculixFileName,"%s",name);

    // Determine if an calculix file exists
    
    sprintf(file_name_w_ext,"%s.inp",CalculixFileName);

    if ( (InpFile = fopen(file_name_w_ext,"r")) != NULL ) {
     
       fclose(InpFile); // It's reopened later

       // Load in Calculix file
       
       LoadCalculixINPFileSurfaceElements(CalculixFileName);
    
    }
    
}
/*##############################################################################
#                                                                              #
#                  ADBSLICER InterpolateSolutionToCalculix                     #
#                                                                              #
##############################################################################*/

void ADBSLICER::InterpolateSolutionToCalculix(char *name)
{

    int i;
    char CommandLine[10000], file_name_w_ext[10000];
    FILE *InpFile;
    INTERP Interp;

    if ( !AddLabel_ ) sprintf(Label_,"");
        
    // Save the file name

    sprintf(CalculixFileName,"%s",name);

    printf("CalculixFileName: %s \n",CalculixFileName);fflush(NULL);

    // Determine if an calculix file exists
    
    sprintf(file_name_w_ext,"%s.inp",CalculixFileName);

    if ( (InpFile = fopen(file_name_w_ext,"r")) != NULL ) {

       fclose(InpFile); // It's reopened later
       
       LoadSolutionData(1);

       FindSolutionMinMax();
       
       // Load in Calculix file
       
       LoadCalculixINPFileSurfaceElements(CalculixFileName);
       
       // Create VSP Mesh data
       
       CreateVSPInterpMesh();
       
       // Interpolate from VSP to FEM mesh
       
       if ( ModelType == VLM_MODEL ) {
       
          if ( Verbose_ ) printf("VSPAERO model is VLM \n");
          
          Interp.IngoreBoundingBox();
          
          Interp.ForceStrictInterpolation();
       
       }
       
       Interp.Interpolate(&VSP_Mesh, &FEM_Mesh);
              
       // Write out static and buckling analysis input files
       
       WriteOutCalculixStaticAnalysisFile(name,CALCULIX_STATIC);
 
       WriteOutCalculixStaticAnalysisFile(name,CALCULIX_BUCKLE);

       // Output statistics
       
       if ( Verbose_ ) printf("Max node number with offset: %d \n",MaxCalculixNode_ + NodeOffSet_);
       if ( Verbose_ ) printf("Max element number with offset: %d \n",MaxCalculixElement_ + ElementOffSet_);
                    
    }
    
    // Otherwise exit...
    
    else {
     
       printf("No Calculix %s file found! \n",CalculixFileName);
       fflush(NULL);
       exit(1);  
 
    }
   
}

/*##############################################################################
#                                                                              #
#                              ADBSLICER LoadMeshData                          #
#                                                                              #
##############################################################################*/

void ADBSLICER::LoadMeshData(void)
{

    char file_name_w_ext[10000], DumChar[1000], GridName[1000];
    int i, j, k, p, DumInt, Level, Edge, NumberOfControlSurfaceNodes, TimeAnalysisType;
    int TotNum, i_size, f_size, c_size, NumberOfRotors, NumberOfNozzles;
    float DumFloat;
    FILE *adb_file, *madb_file;
    BINARYIO BIO;

    // Sizeof ints and floats

    i_size = sizeof(int);
    f_size = sizeof(float);
    c_size = sizeof(char);

    // Check on endian issues

    if ( ByteSwapForADB ) BIO.TurnByteSwapForReadsOn();

    // Open the aerothermal data base file

    sprintf(file_name_w_ext,"%s.adb",file_name);

    if ( (adb_file = fopen(file_name_w_ext,"rb")) == NULL ) {

       printf("Could not open either an adb or madb file... ! \n");fflush(NULL);
                     
       exit(1);
       
    }

    // Read in the default in to check on endianess

    BIO.fread(&DumInt, i_size, 1, adb_file);

    if ( DumInt != -123789456 && DumInt != -123789456 + 3 ) {

       BIO.TurnByteSwapForReadsOn();

       rewind(adb_file);

       BIO.fread(&DumInt, i_size, 1, adb_file);

    }

    FILE_VERSION = 2;
    
    if ( DumInt == -123789456 + 3 ) FILE_VERSION = 3;
           
    // Read in model type... VLM or PANEL
    
    BIO.fread(&ModelType, i_size, 1, adb_file);
    
    // Read in symmetry flag
    
    BIO.fread(&SymmetryFlag, i_size, 1, adb_file);    
    
    // Read in unsteady analysis flag
    
    BIO.fread(&TimeAnalysisType, i_size, 1, adb_file);       

    // Read in header

    BIO.fread(&NumberOfVortexLoops,        i_size, 1, adb_file);
    BIO.fread(&NumberOfNodes,              i_size, 1, adb_file);
    BIO.fread(&NumberOfTris,               i_size, 1, adb_file);
    BIO.fread(&NumberOfSurfaceVortexEdges, i_size, 1, adb_file);
    
    BIO.fread(&Sref,           f_size, 1, adb_file);
    BIO.fread(&Cref,           f_size, 1, adb_file);
    BIO.fread(&Bref,           f_size, 1, adb_file);
    BIO.fread(&Xcg,            f_size, 1, adb_file);
    BIO.fread(&Ycg,            f_size, 1, adb_file);
    BIO.fread(&Zcg,            f_size, 1, adb_file);
    
    NumberOfMachs = NumberOfAlphas = NumberOfBetas = 1;

    printf("NumberOfNodes:  %d \n",NumberOfNodes);
    printf("NumberOfTris:   %d \n",NumberOfTris);
    printf("Sref:           %f \n",Sref);
    printf("Cref:           %f \n",Cref);
    printf("Bref:           %f \n",Bref);
    printf("Xcg:            %f \n",Xcg);
    printf("Ycg:            %f \n",Ycg);
    printf("Zcg:            %f \n",Zcg);fflush(NULL);

    TotNum = NumberOfMachs * NumberOfBetas * NumberOfAlphas;

    // Create space to store grid and solution

    NodeList_ = new NODE[NumberOfNodes + 1];

    TriList_ = new TRI[NumberOfTris + 1];

    Cp         = new float[NumberOfTris + 1];
    CpUnsteady = new float[NumberOfTris + 1];
    Gamma      = new float[NumberOfTris + 1];
    
    CpNode     = new float[NumberOfNodes + 1];
    TotalArea  = new float[NumberOfNodes + 1];

    MachList = new float[NumberOfMachs + 1];
    BetaList = new float[NumberOfBetas + 1];
    AlphaList = new float[NumberOfAlphas + 1];

    // Read in wing ID flags, names...
 
    fread(&NumberOfWings_, i_size, 1, adb_file);
    
    WingListName_ = new char*[NumberOfWings_ + 1];
  
    for ( i = 1 ; i <= NumberOfWings_ ; i++ ) { 
     
       fread(&DumInt, i_size, 1, adb_file);
       
       WingListName_[i] = new char[200];
 
       fread(WingListName_[i], c_size, 100, adb_file);
       
       printf("Wing: %d ... %s \n",i,WingListName_[i]);fflush(NULL);
       
       fread(&DumInt, i_size, 1, adb_file);
       
       printf("ComponentID: %d \n",DumInt);
     
    }
    
    // Read in body ID flags, names...

    fread(&NumberOfBodies_, i_size, 1, adb_file);
    
    BodyListName_ = new char*[NumberOfBodies_ + 1];
    
    for ( i = 1 ; i <= NumberOfBodies_ ; i++ ) { 
     
       fread(&DumInt, i_size, 1, adb_file);
       
       BodyListName_[i] = new char[200];

       fread(BodyListName_[i], c_size, 100, adb_file);
       
       printf("Body: %d ... %s \n",i,BodyListName_[i]);fflush(NULL);

       fread(&DumInt, i_size, 1, adb_file);
       
       printf("ComponentID: %d \n",DumInt);
            
    } 
    
    // Read in Cart3d ID flags, names...

    fread(&NumberOfCart3dSurfaces_, i_size, 1, adb_file);
    
    Cart3dListName_ = new char*[NumberOfCart3dSurfaces_ + 1];
    
    for ( i = 1 ; i <= NumberOfCart3dSurfaces_ ; i++ ) { 
     
       fread(&DumInt, i_size, 1, adb_file);
       
       Cart3dListName_[i] = new char[200];

       fread(Cart3dListName_[i], c_size, 100, adb_file);
       
       fread(&DumInt, i_size, 1, adb_file);
       
       printf("Cart3d: %d ... %s \n",i,Cart3dListName_[i]);fflush(NULL);
     
    }     

    // Load in the geometry and surface information

    for ( i = 1 ; i <= NumberOfTris ; i++ ) {

       // Geometry

       BIO.fread(&(TriList_[i].node1),        i_size, 1, adb_file);
       BIO.fread(&(TriList_[i].node2),        i_size, 1, adb_file);
       BIO.fread(&(TriList_[i].node3),        i_size, 1, adb_file);
       BIO.fread(&(TriList_[i].surface_type), i_size, 1, adb_file);
       BIO.fread(&(TriList_[i].surface_id),   i_size, 1, adb_file);
       BIO.fread(&(TriList_[i].area),         f_size, 1, adb_file);

    }

    for ( i = 1 ; i <= NumberOfNodes ; i++ ) {

       BIO.fread(&(NodeList_[i].x), f_size, 1, adb_file);
       BIO.fread(&(NodeList_[i].y), f_size, 1, adb_file);
       BIO.fread(&(NodeList_[i].z), f_size, 1, adb_file);

    }
    
    // Find Min/Max of geometry

    FindMeshMinMax();
    
    // Read in any propulsion data
    
    BIO.fread(&(NumberOfRotors), i_size, 1, adb_file); 

    if ( FILE_VERSION == 3 ) BIO.fread(&(NumberOfNozzles), i_size, 1, adb_file);
        
    printf("NumberOfRotors: %d \n",NumberOfRotors);fflush(NULL);

    if ( FILE_VERSION == 3 ) printf("There are %d nozzles defined \n",NumberOfNozzles);
     
    NumberOfPropulsionElements = NumberOfRotors + NumberOfNozzles;
    
    PropulsionElement = new PROPULSION_ELEMENT[NumberOfPropulsionElements + 1];
    
    // Read in the rotor data
    
    printf("Reading in the rotor data ... \n");
    
    j = 0;
    
    for ( i = 1 ; i <= NumberOfRotors ; i++ ) {
    
       j++;
        
       PropulsionElement[j].Rotor.Read_Binary_STP_Data(adb_file);
    
       PropulsionElement[j].Type = PROP_ROTOR;
    
    }

    // Read in the nozzle data
    
    if ( FILE_VERSION == 3 ) {
       
       for ( i = 1 ; i <= NumberOfNozzles ; i++ ) {
       
          j++;
           
          PropulsionElement[j].Engine.Read_Binary_STP_Data(adb_file);
          
          PropulsionElement[j].Type = ENGINE_NOZZLE;
       
       }
       
    }
    
    // Read in any coarse mesh edge data
    
    BIO.fread(&(NumberOfMeshLevels), i_size, 1, adb_file); 
    
    printf("NumberOfMeshLevels: %d \n",NumberOfMeshLevels);fflush(NULL);
    
    CoarseNodeList_ = new NODE*[NumberOfMeshLevels + 1];
    CoarseEdgeList_ = new EDGE*[NumberOfMeshLevels + 1];
    
    NumberOfCourseNodesForLevel = new int[NumberOfMeshLevels + 1];
    NumberOfCourseEdgesForLevel = new int[NumberOfMeshLevels + 1];
    
    for ( Level = 1 ; Level <= NumberOfMeshLevels ; Level++ ) {
     
       BIO.fread(&(NumberOfCourseNodesForLevel[Level]), i_size, 1, adb_file);    
       BIO.fread(&(NumberOfCourseEdgesForLevel[Level]), i_size, 1, adb_file);          
     
       printf("Number of course nodes for level: %d is: %d \n",Level,NumberOfCourseNodesForLevel[Level]);fflush(NULL);
       printf("Number of course edges for level: %d is: %d \n",Level,NumberOfCourseEdgesForLevel[Level]);fflush(NULL);
  
       CoarseNodeList_[Level] = new NODE[NumberOfCourseNodesForLevel[Level] + 1];
       CoarseEdgeList_[Level] = new EDGE[NumberOfCourseEdgesForLevel[Level] + 1];

       for ( i = 1 ; i <= NumberOfCourseNodesForLevel[Level] ; i++ ) {
 
          BIO.fread(&(CoarseNodeList_[Level][i].x), f_size, 1, adb_file);       
          BIO.fread(&(CoarseNodeList_[Level][i].y), f_size, 1, adb_file);  
          BIO.fread(&(CoarseNodeList_[Level][i].z), f_size, 1, adb_file);       
          
       }
         
       for ( i = 1 ; i <= NumberOfCourseEdgesForLevel[Level] ; i++ ) {
 
          BIO.fread(&(CoarseEdgeList_[Level][i].SurfaceID), i_size, 1, adb_file);       
        
          if ( CoarseEdgeList_[Level][i].SurfaceID < 0 ) {
             
             CoarseEdgeList_[Level][i].SurfaceID = -CoarseEdgeList_[Level][i].SurfaceID;

             CoarseEdgeList_[Level][i].IsBoundaryEdge = 1;    
 
          }
                  
          BIO.fread(&(CoarseEdgeList_[Level][i].node1), i_size, 1, adb_file);       
          BIO.fread(&(CoarseEdgeList_[Level][i].node2), i_size, 1, adb_file);       
          
          CoarseEdgeList_[Level][i].IsKuttaEdge = 0;
          
       }
    
    }    
    
    // Read in the kutta edge data
    
    Level = 1;
    
    BIO.fread(&(NumberOfKuttaEdges), i_size, 1, adb_file);       

    for ( i = 1 ; i <= NumberOfKuttaEdges; i++ ) {
       
       BIO.fread(&(Edge), i_size, 1, adb_file);      
       
       CoarseEdgeList_[Level][Edge].IsKuttaEdge = 1;
        
    }
    
    // Read in the kutta node data
    
    Level = 1;
    
    BIO.fread(&(NumberOfKuttaNodes), i_size, 1, adb_file);       

    for ( i = 1 ; i <= NumberOfKuttaNodes; i++ ) {
       
       BIO.fread(&(DumInt), i_size, 1, adb_file);      

    }    
    
    // Read in any control surfaces
    
    BIO.fread(&(NumberOfControlSurfaces), i_size, 1, adb_file);       
    
    printf("NumberOfControlSurfaces: %d \n",NumberOfControlSurfaces);
    
    ControlSurface = new CONTROL_SURFACE[NumberOfControlSurfaces + 1];
    
    for ( i = 1 ; i <= NumberOfControlSurfaces ; i++ ) {
       
       BIO.fread(&(NumberOfControlSurfaceNodes), i_size, 1, adb_file);       
       
       ControlSurface[i].NumberOfNodes = NumberOfControlSurfaceNodes;
       
       ControlSurface[i].NodeList = new float*[NumberOfControlSurfaceNodes + 1];
       
       printf("NumberOfControlSurfaceNodes: %d \n",NumberOfControlSurfaceNodes);
       
       for ( j = 1 ; j <= NumberOfControlSurfaceNodes ; j++ ) {
          
          ControlSurface[i].NodeList[j] = new float[3];
          
       }
       
       for ( j = 1 ; j <= NumberOfControlSurfaceNodes ; j++ ) {

          BIO.fread(&(ControlSurface[i].NodeList[j][0]), f_size, 1, adb_file);     
          BIO.fread(&(ControlSurface[i].NodeList[j][1]), f_size, 1, adb_file);      
          BIO.fread(&(ControlSurface[i].NodeList[j][2]), f_size, 1, adb_file); 
          
       }          
       
       // Hinge nodes and vector
       
       BIO.fread(&(ControlSurface[i].HingeNode1[0]), f_size, 1, adb_file);     
       BIO.fread(&(ControlSurface[i].HingeNode1[1]), f_size, 1, adb_file);      
       BIO.fread(&(ControlSurface[i].HingeNode1[2]), f_size, 1, adb_file);           
                        
       BIO.fread(&(ControlSurface[i].HingeNode2[0]), f_size, 1, adb_file);     
       BIO.fread(&(ControlSurface[i].HingeNode2[1]), f_size, 1, adb_file);      
       BIO.fread(&(ControlSurface[i].HingeNode2[2]), f_size, 1, adb_file); 
       
       BIO.fread(&(ControlSurface[i].HingeVec[0]), f_size, 1, adb_file);   
       BIO.fread(&(ControlSurface[i].HingeVec[1]), f_size, 1, adb_file);    
       BIO.fread(&(ControlSurface[i].HingeVec[2]), f_size, 1, adb_file);
              
       // Affected loops
       
       BIO.fread(&(ControlSurface[i].NumberOfLoops), i_size, 1, adb_file);
       
       ControlSurface[i].LoopList = new int[ControlSurface[i].NumberOfLoops + 1];
       
       for ( p = 1 ; p <= ControlSurface[i].NumberOfLoops ; p++ ) {
          
          BIO.fread(&(ControlSurface[i].LoopList[p]), i_size, 1, adb_file);
          
       }          
       
       // Zero out control surface deflection
       
       ControlSurface[i].DeflectionAngle = 0.;
                            
    }     
    
    // Mark all the loops on a control surface
    
    ControlSurfaceLoop = new int[NumberOfTris + 1];
    
    for ( j = 1 ; j <= NumberOfTris ; j++ ) {
        
       ControlSurfaceLoop[j] = 0;
        
    }
    
    for ( i = 1 ; i <= NumberOfControlSurfaces ; i++ ) {
       
       for ( j = 1 ; j <= ControlSurface[i].NumberOfLoops ; j++ ) {

          ControlSurfaceLoop[ControlSurface[i].LoopList[j]] = i;

       }       
       
    }          
    
    // Store the current location in the file

    fgetpos(adb_file, &StartOfWallTemperatureData);

    // Close the adb file

    fclose(adb_file);

    // Zero out arrays

    zero_float_array(CpNode,    NumberOfNodes);
    zero_float_array(TotalArea, NumberOfNodes);

}

/*##############################################################################
#                                                                              #
#                     ADBSLICER LoadSolutionCaseList                           #
#                                                                              #
##############################################################################*/

void ADBSLICER::LoadSolutionCaseList(void)
{
 
    int i;
    char file_name_w_ext[10000], DumChar[10000];
    FILE *adb_file;
    
    // Open the solution case list

    sprintf(file_name_w_ext,"%s.adb.cases",file_name);

    if ( (adb_file = fopen(file_name_w_ext,"r")) == NULL ) {

       printf("Could not open the adb case list file... ! \n");fflush(NULL);
                     
       exit(1);

    }       
    
    // Read in the cases until eof
    
    NumberOfADBCases_ = 0;
    
    while ( fgets(DumChar,10000,adb_file) != NULL ) {
       
       NumberOfADBCases_++;
       
    }
    
    rewind(adb_file);
    
    ADBCaseList_ = new SOLUTION_CASE[NumberOfADBCases_ + 1];

    for ( i = 1 ; i <= NumberOfADBCases_ ; i++ ) {

       fscanf(adb_file,"%f %f %f",
       &(ADBCaseList_[i].Mach),
       &(ADBCaseList_[i].Alpha),
       &(ADBCaseList_[i].Beta));
        
       fgets(ADBCaseList_[i].CommentLine,200,adb_file);
        
    }

    fclose(adb_file); 
   
}

/*##############################################################################
#                                                                              #
#                         ADBSLICER LoadSolutionData                           #
#                                                                              #
##############################################################################*/

void ADBSLICER::LoadSolutionData(int Case)
{

    char file_name_w_ext[10000], DumChar[100], GridName[100];
    int i, j, k, m, p, Level, node1, node2, node3;
    int i_size, f_size, c_size, d_size;
    int DumInt, nod1, nod2, nod3, CFDCaseFlag, Edge;
    float FreeStreamPressure, DynamicPressure, Xc, Yc, Zc, Fx, Fy, Fz, Cf;
    float BoundaryLayerThicknessCode, LaminarDelta, TurbulentDelta, DumFloat;
    float Area;
    FILE *adb_file, *madb_file;
    BINARYIO BIO;
    long OffSet;
    double DumDouble;

    // Sizeof ints and floats

    i_size = sizeof(int);
    f_size = sizeof(float);
    d_size = sizeof(double);    
    c_size = sizeof(char);

    // Check on endian issues

    if ( ByteSwapForADB ) BIO.TurnByteSwapForReadsOn();

    // Open the aerothermal data base file

    sprintf(file_name_w_ext,"%s.adb",file_name);

    if ( (adb_file = fopen(file_name_w_ext,"rb")) == NULL ) {

       printf("Could not open either an adb or madb file... ! \n");fflush(NULL);
                     
       exit(1);

    } 
    
    // Read in the default in to check on endianess

    BIO.fread(&DumInt, i_size, 1, adb_file);

    if ( DumInt != -123789456 && DumInt != -123789456 + 3 ) {

       BIO.TurnByteSwapForReadsOn();

       rewind(adb_file);

       BIO.fread(&DumInt, i_size, 1, adb_file);

    }

    FILE_VERSION = 2;
    
    if ( DumInt == -123789456 + 3 ) FILE_VERSION = 3;

    // Set the file position to the top of the temperature data

    fsetpos(adb_file, &StartOfWallTemperatureData);
    
    for ( p = 1 ; p <= Case ; p++ ) {  
   
       // Read in the EdgeMach, Q, and Alpha lists
   
       for ( k = 1 ; k <= NumberOfMachs  ; k++ ) BIO.fread(&MachList[k],    f_size, 1, adb_file);
       for ( k = 1 ; k <= NumberOfAlphas ; k++ ) { BIO.fread(&AlphaList[k], f_size, 1, adb_file); AlphaList[k] /= TORAD; };
       for ( k = 1 ; k <= NumberOfBetas  ; k++ ) { BIO.fread(&BetaList[k],  f_size, 1, adb_file); BetaList[k]  /= TORAD; };

       // Read in data set 
   
       BIO.fread(&(CpMinSoln), f_size, 1, adb_file); // Min Cp from solver
       BIO.fread(&(CpMaxSoln), f_size, 1, adb_file); // Max Cp from solver
   
       // Solution on computational mesh
       
       for ( m = 1 ; m <= NumberOfVortexLoops ; m++ ) {
 
          BIO.fread(&(DumDouble), d_size, 1, adb_file); // Gamma
          BIO.fread(&(DumDouble), d_size, 1, adb_file); // Unsteady dCP

       }

       // Vortex edge forces on computational mesh
       
       for ( m = 1 ; m <= NumberOfSurfaceVortexEdges ; m++ ) {
 
          BIO.fread(&(DumDouble), d_size, 1, adb_file); 
          BIO.fread(&(DumDouble), d_size, 1, adb_file); 
          BIO.fread(&(DumDouble), d_size, 1, adb_file);

       }
      
       // Solution on computational mesh
      
       for ( m = 1 ; m <= NumberOfVortexLoops ; m++ ) {
   
          BIO.fread(&(DumDouble), d_size, 1, adb_file); // U
          BIO.fread(&(DumDouble), d_size, 1, adb_file); // V
          BIO.fread(&(DumDouble), d_size, 1, adb_file); // W

       }
          
       for ( m = 1 ; m <= NumberOfTris ; m++ ) {
   
          BIO.fread(&(Cp[m]),         f_size, 1, adb_file); // Cp, Steady
          BIO.fread(&(CpUnsteady[m]), f_size, 1, adb_file); // Cp, Unsteady
          BIO.fread(&(Gamma[m]),      f_size, 1, adb_file); // Gamma
    
       }
      
       // Read in the wake location data
       
       BIO.fread(&(NumberOfTrailingVortexEdges_), i_size, 1, adb_file); // Number of trailing wake vortices
   
       XWake_ = new float*[NumberOfTrailingVortexEdges_ + 1];
       YWake_ = new float*[NumberOfTrailingVortexEdges_ + 1];
       ZWake_ = new float*[NumberOfTrailingVortexEdges_ + 1];
       
       for ( i = 1 ; i <= NumberOfTrailingVortexEdges_ ; i++ ) {

          BIO.fread(&DumInt, i_size, 1, adb_file); // Wing ID
          
          BIO.fread(&DumFloat, f_size, 1, adb_file); // Span Location
        
          BIO.fread(&(NumberOfSubVortexNodes_), i_size, 1, adb_file); // Number of sub vortices
   
          XWake_[i] = new float[NumberOfSubVortexNodes_ + 1];
          YWake_[i] = new float[NumberOfSubVortexNodes_ + 1];
          ZWake_[i] = new float[NumberOfSubVortexNodes_ + 1];
          
          for ( j = 1 ; j <= NumberOfSubVortexNodes_ ; j++ ) {
          
             BIO.fread(&(XWake_[i][j]), f_size, 1, adb_file); // X
             BIO.fread(&(YWake_[i][j]), f_size, 1, adb_file); // Y
             BIO.fread(&(ZWake_[i][j]), f_size, 1, adb_file); // Z

           }
          
       }
       
    }
    
    // Read in any control surface deflection data

    for ( i = 1 ; i <= NumberOfControlSurfaces ; i++ ) {

       BIO.fread(&(ControlSurface[i].DeflectionAngle), f_size, 1, adb_file); 
       
       printf("ControlSurface[%d].DeflectionAngle: %f \n",i,ControlSurface[i].DeflectionAngle);
  
    }      
    
    // Calculate nodal values

    for ( i = 1 ; i <= NumberOfNodes ; i++ ) {

       CpNode[i] = TotalArea[i] = 0.;

    }     
        
    for ( i = 1 ; i <= NumberOfTris ; i++ ) {

        node1 = TriList_[i].node1;
        node2 = TriList_[i].node2;
        node3 = TriList_[i].node3;

        Area = TriList_[i].area;

        CpNode[node1] += Cp[i] * Area;
        CpNode[node2] += Cp[i] * Area;
        CpNode[node3] += Cp[i] * Area;

        TotalArea[node1] += Area;
        TotalArea[node2] += Area;
        TotalArea[node3] += Area;

     }

     for ( i = 1 ; i <= NumberOfNodes ; i++ ) {

       if ( TotalArea[i] > 0. ) {
         
           CpNode[i] /= TotalArea[i];
          
        }
       
        else {
        
           CpNode[i] = 0.;
       
        }

    }     

    // Close the adb file

    fclose(adb_file);

}

/*##############################################################################
#                                                                              #
#                          ADBSLICER CalculateSurfaceNormals                   #
#                                                                              #
##############################################################################*/

void ADBSLICER::CalculateSurfaceNormals(int Case)
{

    int i, nod1, nod2, nod3;
    float vec1[3], vec2[3], normal[3], dot;

    if ( Case == 1 ) {

       Nx = new float[NumberOfTris + 1];
       Ny = new float[NumberOfTris + 1];
       Nz = new float[NumberOfTris + 1];

    }

    for ( i = 1 ; i <= NumberOfTris ; i++ ) {

       nod1 = TriList_[i].node1;
       nod2 = TriList_[i].node2;
       nod3 = TriList_[i].node3;

       vec1[0] = NodeList_[nod2].x - NodeList_[nod1].x;
       vec1[1] = NodeList_[nod2].y - NodeList_[nod1].y;
       vec1[2] = NodeList_[nod2].z - NodeList_[nod1].z;

       vec2[0] = NodeList_[nod3].x - NodeList_[nod1].x;
       vec2[1] = NodeList_[nod3].y - NodeList_[nod1].y;
       vec2[2] = NodeList_[nod3].z - NodeList_[nod1].z;

       vector_cross(vec1,vec2,normal);

       dot = sqrt(vector_dot(normal,normal));

       Nx[i] = normal[0]/dot;
       Ny[i] = normal[1]/dot;
       Nz[i] = normal[2]/dot;


    }

}

/*##############################################################################
#                                                                              #
#                          ADBSLICER CreateTriEdges                            #
#                                                                              #
##############################################################################*/

void ADBSLICER::CreateTriEdges(void)
{

    int i, j, k, nod1, nod2, noda, nodb, start_edge, surf_edge;
    int level, edge_to_node[4][3], nod_list[4], tri1, tri2;
    int max_edge, new_edge, *jump_pnt, Error;
    GL_EDGE_ENTRY *list, *tlist;

    printf("Finding tri edges... \n");fflush(NULL);

    Error = 0;

    // Make space for a linked list of edges

    jump_pnt = new int[NumberOfNodes + 1];

    max_edge = 4*NumberOfNodes;

    list = new GL_EDGE_ENTRY[max_edge + 1];

    // Zero out the lists

    for ( i = 0 ; i <= NumberOfNodes ; i++ ) {

       jump_pnt[i] = 0;

    }

    for ( i = 0 ; i <= max_edge ; i++ ) {

       list[i].node  = 0;
       list[i].next  = 0;
       list[i].tri_1 = 0;
       list[i].tri_2 = 0;

    }

    // Initialize number of edges

    new_edge = 0;

    // Keep track of first edge

    start_edge = new_edge + 1;

    // List of nodes for each triangle edge

    edge_to_node[1][1] = 1;
    edge_to_node[1][2] = 2;

    edge_to_node[2][1] = 2;
    edge_to_node[2][2] = 3;

    edge_to_node[3][1] = 1;
    edge_to_node[3][2] = 3;

    // Loop over triangles and create the edge list

    for ( j = 1 ; j <= NumberOfTris ; j++ ) {

       // Local copy of node pointers

       nod_list[1] = TriList_[j].node1;
       nod_list[2] = TriList_[j].node2;
       nod_list[3] = TriList_[j].node3;

       // Loop over all edge cases

       for ( i = 1 ; i <= 3 ; i++ ) {

          noda = nod_list[edge_to_node[i][1]];
          nodb = nod_list[edge_to_node[i][2]];

          nod1 = MIN(noda,nodb);
          nod2 = MAX(noda,nodb);

          level = jump_pnt[nod1];

          // New edge, since first edge for this node

          if ( level == 0 ) {

             new_edge++;

             // If this is a 3d case, the surface should not introduce new edges

             if ( start_edge != 1 ) {

                 printf("Error: tris contain edges not in tet grid! \n");fflush(NULL);

                 exit(1);

             }

             // Make sure there is enough room, if not reallocate space

             if ( new_edge >= max_edge ) {

                max_edge = (int) (1.2*max_edge);

                tlist = new GL_EDGE_ENTRY[max_edge + 1];

                for ( k = 1 ; k <= new_edge ; k++ ) {

                   tlist[k] = list[k];

                }

                delete [] list;

                list = tlist;

             }

             list[new_edge].node = nod2;
             list[new_edge].next = 0;

             jump_pnt[nod1] = new_edge;

             // Pack tri to edge pointer

             if ( noda == nod1 ) {

                if ( i == 1 ) TriList_[j].edge1 = new_edge;
                if ( i == 2 ) TriList_[j].edge2 = new_edge;
                if ( i == 3 ) TriList_[j].edge3 = new_edge;

             }

             else {

                if ( i == 1 ) TriList_[j].edge1 = -new_edge;
                if ( i == 2 ) TriList_[j].edge2 = -new_edge;
                if ( i == 3 ) TriList_[j].edge3 = -new_edge;

             }

             // Pack surface edge to tri pointer

             if ( list[new_edge].tri_1 == 0 ) {

                list[new_edge].tri_1 = j;

             }

             else {

                list[new_edge].tri_2 = j;

             }

          }

          // Must check if this edge exists

          else {

             while ( list[level].node != nod2 && list[level].next != 0 ) {

                level = list[level].next;

             }

             // Old edge

             if ( list[level].node == nod2 ) {

                // Pack tri to edge pointer

                if ( noda == nod1 ) {

                   if ( i == 1 ) TriList_[j].edge1 = level;
                   if ( i == 2 ) TriList_[j].edge2 = level;
                   if ( i == 3 ) TriList_[j].edge3 = level;

                }

                else {

                   if ( i == 1 ) TriList_[j].edge1 = -level;
                   if ( i == 2 ) TriList_[j].edge2 = -level;
                   if ( i == 3 ) TriList_[j].edge3 = -level;

                }

                // Pack surface edge to tri pointer

                if ( list[level].tri_1 == 0 ) {

                   list[level].tri_1 = j;

                }

                else {

                   list[level].tri_2 = j;

                }

             }

             // New edge

             else {

                new_edge++;

                // If this is a 3d case, the surface should not introduce new edges

                if ( start_edge != 1 ) {

                    printf("Error: tris contain edges not in tet grid! \n");fflush(NULL);

                    exit(1);

                }

                // Make sure there is enough room, if not reallocate space

                if ( new_edge >= max_edge ) {

                   max_edge = (int) (1.2*max_edge);

                   tlist = new GL_EDGE_ENTRY[max_edge + 1];

                   for ( k = 1 ; k <= new_edge ; k++ ) {

                      tlist[k] = list[k];

                   }

                   delete [] list;

                   list = tlist;

                }

                list[level].next = new_edge;

                list[new_edge].node  = nod2;
                list[new_edge].next  = 0;

                // Pack tri to edge pointer

                if ( noda == nod1 ) {

                   if ( i == 1 ) TriList_[j].edge1 = new_edge;
                   if ( i == 2 ) TriList_[j].edge2 = new_edge;
                   if ( i == 3 ) TriList_[j].edge3 = new_edge;

                }

                else {

                   if ( i == 1 ) TriList_[j].edge1 = -new_edge;
                   if ( i == 2 ) TriList_[j].edge2 = -new_edge;
                   if ( i == 3 ) TriList_[j].edge3 = -new_edge;

                }

                // Pack surface edge to tri pointer

                if ( list[new_edge].tri_1 == 0 ) {

                   list[new_edge].tri_1 = j;

                }

                else {

                   list[new_edge].tri_2 = j;

                }

             }

          }

       }

    }

    // Store edge to node pointers

    if ( start_edge == 1 ) {

       NumberOfEdges = new_edge;

       EdgeList_ = new EDGE[NumberOfEdges + 1];

       // Fill the edge list

       for ( i = 1 ; i <= NumberOfNodes ; i++ ) {

          level = jump_pnt[i];

          while ( level != 0 ) {

			 EdgeList_[level].node1 = i;
			 EdgeList_[level].node2 = list[level].node;

             if ( list[level].tri_2 == 0 ) {

                Error = 1;

             }

             level = list[level].next;

          }

       }

    }

    // Store edge to tri pointers

    for ( i = 1 ; i <= NumberOfNodes ; i++ ) {

       level = jump_pnt[i];

       while ( level != 0 ) {

          if ( list[level].tri_1 != 0 && list[level].tri_1 != 0) {

             if ( start_edge == 1 ) {

                surf_edge = level;

             }

             else {

               surf_edge++;

             }

             // Store pointers to tris

             EdgeList_[level].tri1 = list[level].tri_1;
             EdgeList_[level].tri2 = list[level].tri_2;

          }

          else {

            if ( start_edge == 1 ) {

               printf("Surface triangulation as input is messed up! \n");
               printf("Error found while storing surface edges...\n");fflush(NULL);

               exit(1);

            }

          }

          level = list[level].next;

       }

    }

    printf("Number of nodes: %d \n",NumberOfNodes);
    printf("Number of tris: %d \n",NumberOfTris);
    printf("Number of edges is: %d \n",NumberOfEdges);fflush(NULL);

    // Free up the scratch space

    delete [] jump_pnt;
    delete [] list;

    // Error noting

    if ( Error ) printf("Note... mesh may not be closed! \n");fflush(NULL);

}

/*##############################################################################
#                                                                              #
#                            ADBSLICER FindMeshMinMax                          #
#                                                                              #
##############################################################################*/

void ADBSLICER::FindMeshMinMax(void)
{

    int i;

    XMin = 1.e6;
    XMax = -XMin;

    YMin = 1.e6;
    YMax = -YMin;

    ZMin = 1.e6;
    ZMax = -ZMin;

    for ( i = 1 ; i <= NumberOfNodes ; i++ ) {

	   XMin = MIN(XMin, NodeList_[i].x);
	   YMin = MIN(YMin, NodeList_[i].y);
	   ZMin = MIN(ZMin, NodeList_[i].z);

	   XMax = MAX(XMax, NodeList_[i].x);
	   YMax = MAX(YMax, NodeList_[i].y);
	   ZMax = MAX(ZMax, NodeList_[i].z);

	 }

	 printf("Xmin, Xmax: %f, %f \n",XMin, XMax);
	 printf("Ymin, Ymax: %f, %f \n",YMin, YMax);
	 printf("Zmin, Zmax: %f, %f \n",ZMin, ZMax);fflush(NULL);

}

/*##############################################################################
#                                                                              #
#                            ADBSLICER FindSolutionMinMax                      #
#                                                                              #
##############################################################################*/

void ADBSLICER::FindSolutionMinMax(void)
{

    int i, j, m, Hits;
    float Big = 1.e9, Avg, StdDev;

    CpMinActual = Big;
    CpMaxActual = -Big;
    
    Avg   = 0.;   Hits = 0;
 
    for ( m = 1 ; m <= NumberOfTris ; m++ ) {

       CpMinActual = MIN(CpMinActual, Cp[m]);
       CpMaxActual = MAX(CpMaxActual, Cp[m]);
       
       Avg += Cp[m]; Hits++;

    }
    
    Avg /= Hits;
  
    // Now calculate some statistics
  
    StdDev   = 0.;   Hits = 0;

    for ( m = 1 ; m <= NumberOfTris ; m++ ) {

       StdDev += pow(Avg- Cp[m],2.0f); Hits++;

    }    
    
    StdDev = sqrt(StdDev/Hits);

    // Set Cp min, and max
    
    if ( ModelType == VLM_MODEL ) {
    
       CpMin = MAX(Avg - 1. * StdDev, CpMinActual);
       CpMax = MIN(Avg + 1. * StdDev, CpMaxActual);
       
    }
    
    else {
       
       CpMin = CpMinSoln;
       CpMax = CpMaxSoln;
       
    }

}

/*##############################################################################
#                                                                              #
#                              ADBSLICER LoadCutsFile                          #
#                                                                              #
##############################################################################*/

void ADBSLICER::LoadCutsFile(void)
{
   
    int i;
    char file_name_w_ext[10000], CutType[200];
    FILE *cut_file;
    
    sprintf(file_name_w_ext,"%s.cuts",file_name);

    if ( (cut_file = fopen(file_name_w_ext,"r")) != NULL ) {
       
       fscanf(cut_file,"%d",&NumberOfCutPlanes);
       
       CutPlaneType = new int[NumberOfCutPlanes + 1];
       
       CutPlaneValue = new float[NumberOfCutPlanes + 1];
       
       for ( i = 1 ; i <= NumberOfCutPlanes ; i++ ) {
          
          fscanf(cut_file,"%s %f \n",CutType,&(CutPlaneValue[i]));
          
          printf("CutType: %s ... Value: %f \n",CutType,CutPlaneValue[i]);
          
          if ( strcmp(CutType,"x") == 0 ) CutPlaneType[i] = XCUT;
          if ( strcmp(CutType,"y") == 0 ) CutPlaneType[i] = YCUT;
          if ( strcmp(CutType,"z") == 0 ) CutPlaneType[i] = ZCUT;
          
       }
       
       fclose(cut_file);
       
    }
    
    else {
       
       printf("Could not open cuts file: %s \n",file_name_w_ext);
       
    }
  
}

/*##############################################################################
#                                                                              #
#                              ADBSLICER Slice                                 #
#                                                                              #
##############################################################################*/

void ADBSLICER::Slice(int Case)
{
   
    int c, m, noda, nodb;
    float xyz_1[3], xyz_2[3], xyz_3[3], xyz_4[3];
    float Cp, Cp_1, Cp_2, pnt_1[3], pnt_2[3], tt, uu, ww, x, y, z;
    BBOX plane_box, edge_box;
    
    // Loop over the user defined cutting planes

    for ( c = 1 ; c <= NumberOfCutPlanes ; c++ ) {

       if ( CutPlaneType[c] == XCUT ) {

          xyz_1[0] =  CutPlaneValue[c];
          xyz_1[1] = -1.e6;
          xyz_1[2] = -1.e6;

          xyz_2[0] =  CutPlaneValue[c];
          xyz_2[1] =  1.e6;
          xyz_2[2] = -1.e6;

          xyz_3[0] =  CutPlaneValue[c];
          xyz_3[1] = -1.e6;
          xyz_3[2] =  1.e6;

          xyz_4[0] =  CutPlaneValue[c];
          xyz_4[1] =  1.e6;
          xyz_4[2] =  1.e6;

          fprintf(SliceFile,"BLOCK Cut_%d_at_X:_%f \n", c, CutPlaneValue[c]);

       }

       else if ( CutPlaneType[c] == YCUT ) {

          xyz_1[0] = -1.e6;
          xyz_1[1] =  CutPlaneValue[c];
          xyz_1[2] = -1.e6;

          xyz_2[0] = -1.e6;
          xyz_2[1] =  CutPlaneValue[c];
          xyz_2[2] =  1.e6;

          xyz_3[0] =  1.e6;
          xyz_3[1] =  CutPlaneValue[c];
          xyz_3[2] = -1.e6;

          xyz_4[0] =  1.e6;
          xyz_4[1] =  CutPlaneValue[c];
          xyz_4[2] =  1.e6;

          fprintf(SliceFile,"BLOCK Cut_%d_at_Y:_%f \n", c, CutPlaneValue[c]);

       }

       else {

          xyz_1[0] = -1.e6;
          xyz_1[1] = -1.e6;
          xyz_1[2] =  CutPlaneValue[c];

          xyz_2[0] =  1.e6;
          xyz_2[1] = -1.e6;
          xyz_2[2] =  CutPlaneValue[c];

          xyz_3[0] = -1.e6;
          xyz_3[1] =  1.e6;
          xyz_3[2] =  CutPlaneValue[c];

          xyz_4[0] =  1.e6;
          xyz_4[1] =  1.e6;
          xyz_4[2] =  CutPlaneValue[c];

          fprintf(SliceFile,"BLOCK Cut_%d_at_Z:_%f \n", c, CutPlaneValue[c]);

       }

       // Output headers to file
                       //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890
       fprintf(SliceFile,"Case: %d ... Mach: %f ... Alpha: %f ... Beta: %f ... %s \n",
       Case,
       ADBCaseList_[Case].Mach,
       ADBCaseList_[Case].Alpha,
       ADBCaseList_[Case].Beta,
       ADBCaseList_[Case].CommentLine);       
                                                        //1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890
       if ( ModelType ==   VLM_MODEL ) fprintf(SliceFile,"     x          y          z         dCp\n");       
       if ( ModelType == PANEL_MODEL ) fprintf(SliceFile,"     x          y          z          Cp\n");

       // Calculate bounding box for this cut panel

       plane_box.x_min = MIN4(xyz_1[0],xyz_2[0],xyz_3[0],xyz_4[0]);
       plane_box.x_max = MAX4(xyz_1[0],xyz_2[0],xyz_3[0],xyz_4[0]);

       plane_box.y_min = MIN4(xyz_1[1],xyz_2[1],xyz_3[1],xyz_4[1]);
       plane_box.y_max = MAX4(xyz_1[1],xyz_2[1],xyz_3[1],xyz_4[1]);

       plane_box.z_min = MIN4(xyz_1[2],xyz_2[2],xyz_3[2],xyz_4[2]);
       plane_box.z_max = MAX4(xyz_1[2],xyz_2[2],xyz_3[2],xyz_4[2]);

       // Loop over triangles

       for ( m = 1 ; m <= NumberOfEdges ; m++ ) {

          noda = EdgeList_[m].node1;
          nodb = EdgeList_[m].node2;
 
          pnt_1[0] = NodeList_[noda].x;
          pnt_1[1] = NodeList_[noda].y;
          pnt_1[2] = NodeList_[noda].z;
          
          if ( RotateGeometry ) {
           
             pnt_1[1] = NodeList_[noda].y * CosRot - NodeList_[noda].z * SinRot;
             pnt_1[2] = NodeList_[noda].y * SinRot - NodeList_[noda].z * CosRot;
             
          }

          Cp_1 = CpNode[noda];

          pnt_2[0] = NodeList_[nodb].x;
          pnt_2[1] = NodeList_[nodb].y;
          pnt_2[2] = NodeList_[nodb].z;

          if ( RotateGeometry ) {
           
             pnt_2[1] = NodeList_[nodb].y * CosRot - NodeList_[nodb].z * SinRot;
             pnt_2[2] = NodeList_[nodb].y * SinRot - NodeList_[nodb].z * CosRot;
             
          }
          
          Cp_2 = CpNode[nodb];

          edge_box.x_min = MIN(pnt_1[0],pnt_2[0]);
          edge_box.x_max = MAX(pnt_1[0],pnt_2[0]);

          edge_box.y_min = MIN(pnt_1[1],pnt_2[1]);
          edge_box.y_max = MAX(pnt_1[1],pnt_2[1]);

          edge_box.z_min = MIN(pnt_1[2],pnt_2[2]);
          edge_box.z_max = MAX(pnt_1[2],pnt_2[2]);

          if ( compare_boxes(plane_box,edge_box) == 1 ) {

             // Passed bounding box, so do full intersection

             if ( tri_seg_int(xyz_1,xyz_2,xyz_4,pnt_1,pnt_2,&tt,&uu,&ww) != 0 ||
                  tri_seg_int(xyz_1,xyz_4,xyz_3,pnt_1,pnt_2,&tt,&uu,&ww) != 0 ) {

                tt = MIN(tt,1.);
                tt = MAX(tt,0.);

                pnt_1[0] = NodeList_[noda].x;
                pnt_1[1] = NodeList_[noda].y;
                pnt_1[2] = NodeList_[noda].z;

                pnt_2[0] = NodeList_[nodb].x;
                pnt_2[1] = NodeList_[nodb].y;
                pnt_2[2] = NodeList_[nodb].z;
             
                x = pnt_1[0] + tt*( pnt_2[0] - pnt_1[0] );

                y = pnt_1[1] + tt*( pnt_2[1] - pnt_1[1] );

                z = pnt_1[2] + tt*( pnt_2[2] - pnt_1[2] );

                Cp = Cp_1 + tt*( Cp_2 - Cp_1 );

                fprintf(SliceFile,"%10.4f %10.4f %10.4f %10.4f \n",
                        x,
                        y,
                        z,
                        Cp);


             }

          }

       }
       
       if ( GnuPlot_ ) fprintf(SliceFile,"\n\n\n");

    }

	 fprintf(SliceFile,"\n\n");

}

/*##############################################################################
#                                                                              #
#                         ADBSLICER CleanCalculixInpFile                       #
#                                                                              #
##############################################################################*/

void ADBSLICER::CleanCalculixInpFile(char *filename1, char *newfilename)
{

    int AllDone, Done, NodeID, Editing;
    int j, k, ElementID, Node[6], Whacked;
    float x, y, z;
    char file_name_w_ext[10000], DumChar[10000], SetName[10000];
    char MaterialName1[10000], MaterialName2[10000], MaterialName3[10000], NSkinLabel[10000];
    FILE *File1, *NewFile;
 
    // Open calculix files
    
    sprintf(file_name_w_ext,"%s.inp",filename1);

    if ( (File1 = fopen(file_name_w_ext,"r")) == NULL ) {
       
       printf("Could not open calulix file: %s for input! \n",file_name_w_ext);
       
       fflush(NULL); exit(1);
       
    }

    sprintf(file_name_w_ext,"%s.inp",newfilename);

    if ( (NewFile = fopen(file_name_w_ext,"w")) == NULL ) {
       
       printf("Could not open calulix file: %s for output! \n",file_name_w_ext);
       
       fflush(NULL); exit(1);
       
    }

    AllDone = 0;
    
    while ( !AllDone ) {
    
       if ( fgets(DumChar,2000,File1) == NULL ) AllDone = 1;
       
       // Look for element data

       if ( !AllDone && strstr(DumChar,"*ELEMENT") != NULL ) {
          
          printf("%s",DumChar);   
          
          fprintf(NewFile,"%s",DumChar);   
          
          Done = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,File1);
             
             if ( strlen(DumChar) > 2 && strstr(DumChar,"*") == NULL ) {
         
                sscanf(DumChar,"%d,%d,%d,%d,%d,%d,%d",
                &(ElementID),
                &(Node[0]),
                &(Node[1]),
                &(Node[2]),
                &(Node[3]),
                &(Node[4]),
                &(Node[5]));   
                
                // Check if this element is whacked
                
                Whacked = 0;

                for ( j = 0 ; j <= 5 ; j++ ) {
                   
                   for ( k = j+1 ; k <= 5 ; k++ ) {
                      
                      if ( j != k ) {
                         
                         if ( Node[j] == Node[k] ) {
                            
                            printf("Element %d is whacked --> j,k: %d %d ---> Nodes: %d %d \n",ElementID,j,k,Node[j],Node[k]);
                            
                            Whacked = 1;
                            
                         }
                         
                      }
                      
                   }
                   
                }
                
                if ( !Whacked ) fprintf(NewFile,"%s",DumChar);                
                     
             }
             
             else {
                
                fprintf(NewFile,"%s",DumChar);  
                fprintf(NewFile,"\n");    
                
                Done = 1;
                
             }
             
          }
            
       }
       
       else if ( !AllDone ) {
          
          fprintf(NewFile,"%s",DumChar);
          
       }
       
    }

    fclose(File1);
    fclose(NewFile);
                
}

/*##############################################################################
#                                                                              #
#                         ADBSLICER ScaleCalulixInpPressureLoads               #
#                                                                              #
##############################################################################*/

void ADBSLICER::ScaleCalulixInpPressureLoads(char *filename1, char *newfilename, float ScaleFactor)
{

    int AllDone, Done, NodeID, Editing;
    int j, k, ElementID, Node[6], Whacked, LoadsScaled;
    float x, y, z, Pressure;
    char file_name_w_ext[10000], DumChar[10000], SetName[10000];
    char MaterialName1[10000], MaterialName2[10000], MaterialName3[10000], NSkinLabel[10000];
    FILE *File1, *NewFile;
 
    printf("Scaling pressure... \n");
    
    LoadsScaled = 0;
    
    // Open calculix files
    
    sprintf(file_name_w_ext,"%s.inp",filename1);

    if ( (File1 = fopen(file_name_w_ext,"r")) == NULL ) {
       
       printf("Could not open calulix file: %s for input! \n",file_name_w_ext);
       
       fflush(NULL); exit(1);
       
    }

    sprintf(file_name_w_ext,"%s.inp",newfilename);

    if ( (NewFile = fopen(file_name_w_ext,"w")) == NULL ) {
       
       printf("Could not open calulix file: %s for output! \n",file_name_w_ext);
       
       fflush(NULL); exit(1);
       
    }

    AllDone = 0;
    
    while ( !AllDone ) {
    
       if ( fgets(DumChar,2000,File1) == NULL ) AllDone = 1;
       
       // Look for element data

       if ( !AllDone && strstr(DumChar,"*DLOAD") != NULL ) {
          
          //ok... so P1 not one the same line ad DLOAD...
          
          fprintf(NewFile,"%s",DumChar);   
          
          Done = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,File1);
             
            // printf("Looking at: %s \n",DumChar);
             
             if ( strlen(DumChar) < 2 ) Done = 1;
             
             if ( !Done && strstr(DumChar,"P1") != NULL ) {
         
                sscanf(DumChar,"%d, P1, %f \n",&ElementID,&Pressure);
                
                fprintf(NewFile,"%d, P1, %f \n",ElementID,Pressure*ScaleFactor);
                
                LoadsScaled++;
                
            //    printf("Scaling pressure from: %f to %f \n",Pressure,Pressure*ScaleFactor);
    
             }
             
             else if ( !Done ) {
                
                fprintf(NewFile,"%s",DumChar);  
                
             }

             else {
                
                fprintf(NewFile,"%s",DumChar);  
                fprintf(NewFile,"\n");    
                
                Done = 1;
                
             }
                          
          }
            
       }
       
       else if ( !AllDone ) {
          
          fprintf(NewFile,"%s",DumChar);
          
       }
       
    }
    
    printf("Scaled %d pressure DLOADs \n",LoadsScaled);

    fclose(File1);
    fclose(NewFile);
                
}

/*##############################################################################
#                                                                              #
#                   ADBSLICER OpimizationCalculixInpFile                       #
#                                                                              #
##############################################################################*/

void ADBSLICER::OptimizationCalculixInpFile(char *filename1, char *filename2, char *filename3)
{
 
   int i, NumThick;
   float Smag, Ratio, Thickness, Smax, Savg;
   char file_name_w_ext[2000];
   FILE *SizingFile;
     
    // Load in the frd file
    
    LoadCalculixData(filename2);
       
    // Open the sizing file
    
    printf("Sizing element thickness and saving data... \n");

    sprintf(file_name_w_ext,"%s.sized",filename1);
    
    if ( (SizingFile = fopen(file_name_w_ext,"w")) == NULL ) {

       printf("Could not open the thickness sizing file: %s for output. \n",file_name_w_ext);
       fflush(NULL);exit(1);
       
    }
       
    // Loop over elements and calculate new thicknesses ...  this
    // is done for ALL elements... we will only write out those
    // the user cares about later...
    
    Smax = Savg = 0.;
     
    for ( i = 1 ; i <= NumberOfCalculixElements ; i++ ) {
    
       // 15 node prisms

       if ( CalculixElement[i].NumberOfNodes == 6 ) {
          
          Smag = ( CalculixNode[CalculixElement[i].Node[0]].SMag 
                 + CalculixNode[CalculixElement[i].Node[1]].SMag 
                 + CalculixNode[CalculixElement[i].Node[2]].SMag 
                 + CalculixNode[CalculixElement[i].Node[3]].SMag 
                 + CalculixNode[CalculixElement[i].Node[4]].SMag
                 + CalculixNode[CalculixElement[i].Node[6]].SMag )/6.;
          
       }
       
       //  20 node hexahedra
       
       else if ( CalculixElement[i].NumberOfNodes == 8 ) {
          
          Smag = ( CalculixNode[CalculixElement[i].Node[0]].SMag 
                 + CalculixNode[CalculixElement[i].Node[1]].SMag 
                 + CalculixNode[CalculixElement[i].Node[2]].SMag 
                 + CalculixNode[CalculixElement[i].Node[3]].SMag 
                 + CalculixNode[CalculixElement[i].Node[4]].SMag
                 + CalculixNode[CalculixElement[i].Node[5]].SMag
                 + CalculixNode[CalculixElement[i].Node[6]].SMag
                 + CalculixNode[CalculixElement[i].Node[7]].SMag )/8.;
                              
       }
       
       else {
          
          //printf("Unknown element type! \n");fflush(NULL);
          
       }
       
       Smax = MAX(Smag,Smax);
       
       Savg += Smag/1.e9;

       Ratio = 3. * OptFact_ * Smag / MaxStress_;

       Thickness = Ratio * CalculixElement[i].Thickness;

       Thickness = MAX(MinSkinThickness_,MIN(MaxSkinThickness_,Thickness));
       
       Ratio = Thickness / CalculixElement[i].Thickness;
       
       CalculixElement[i].Thickness = Thickness;
     
       CalculixElement[i].MaxStressRatio = Ratio;
       
       fprintf(SizingFile,"%d %f \n",i,CalculixElement[i].Thickness);
       
       CalculixElement[i].Volume[1] = CalculixElement[i].Thickness * CalculixElement[i].Area;
      
    }

    Savg /= NumberOfCalculixElements;
    
    Savg *= 1.e9;
    
    printf("Max Allowable Stress: %f x e6 \n",MaxStress_/1.e6);
    printf("Max stress during optimization was: %f \n",Smax);
    printf("Max stress/Max Allow: %f \n",Smax/MaxStress_);
    printf("Avg stress during optimization was: %f \n",Savg);
    printf("Avg stress/Max Allow: %f \n",Savg/MaxStress_);
      
    fclose(SizingFile);

    // Resize the skins and write out a new inp file
    
    SmoothSkinThickness(filename1);
    
    ResizeCalculixInputFileSkins(filename1, filename3);
   
}

/*##############################################################################
#                                                                              #
#                   ADBSLICER SmoothSkinThickness                              #
#                                                                              #
##############################################################################*/

void ADBSLICER::SmoothSkinThickness(char *filename1)
{

    int AllDone, Done, NodeID, NumElementsToSmooth, Iter;
    int i, j, k, t, ElementID, Node[6], *ElementData, MaxElements;
    float Thickness, *NewThick, *Area;
    char file_name_w_ext[10000], DumChar[10000], SearchLabel[10000];
    FILE *File1;

    // Create a temp array for the elment data
        
    MaxElements = 250000;
        
    ElementData = new int[7*MaxElements + 1];
    
    // Open calculix files
    
    sprintf(file_name_w_ext,"%s.inp",filename1);

    if ( (File1 = fopen(file_name_w_ext,"r")) == NULL ) {
       
       printf("Could not open calulix file: %s for input! \n",file_name_w_ext);
       
       fflush(NULL); exit(1);
       
    }

    printf("Looking for element list: %s \n",ElementLabel_);
    
    sprintf(SearchLabel,"%s",ElementLabel_);
    
    AllDone = i = 0;
        
    while ( !AllDone ) {
    
       if ( fgets(DumChar,2000,File1) == NULL ) AllDone = 1;
       
       // Look for element data

       if ( !AllDone && strstr(DumChar,"*ELEMENT") != NULL && strstr(DumChar,SearchLabel) != NULL ) {
    
          printf("Grabbing element data for... DumChar: %s \n",DumChar);
                    
          j = Done = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,File1);
             
             if ( strlen(DumChar) > 2 && strstr(DumChar,"*") == NULL ) {
                
                i++;
                
                j++;
         
                sscanf(DumChar,"%d,%d,%d,%d,%d,%d,%d",
                &(ElementID),
                &(Node[0]),
                &(Node[1]),
                &(Node[2]),
                &(Node[3]),
                &(Node[4]),
                &(Node[5]));   
                    
                if ( j > MaxElements ) {
                   
                   printf("Resizing element array... \n");fflush(NULL);
                   
                   ElementData = resize_int_array(ElementData, 7*MaxElements, 1.25*7*MaxElements);
                   
                   MaxElements = 1.25*MaxElements;
                   
                }
                
                // Save element data for later
                
                ElementData[7*j-6] = i;
                ElementData[7*j-5] = Node[0];
                ElementData[7*j-4] = Node[1];
                ElementData[7*j-3] = Node[2];
                ElementData[7*j-2] = Node[3];
                ElementData[7*j-1] = Node[4];
                ElementData[7*j  ] = Node[5];
           
             }
             
             else {
                
                Done = 1;
                
             }

          }
  
       }
       
       else if ( !AllDone && strstr(DumChar,"*ELEMENT") != NULL && strstr(DumChar,SearchLabel) == NULL ) {
          
          //printf("Skipping over element data for... DumChar: %s \n",DumChar);
          
          Done = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,File1);
             
             if ( strlen(DumChar) > 2 && strstr(DumChar,"*") == NULL ) {
                
                i++;
         
             }
             
             else {
                
                Done = 1;
                
             }

          }
                    
       }
       
    }
    
    NumElementsToSmooth = j;

    fclose(File1);
    
    printf("Smoothing %d elements \n",NumElementsToSmooth);
    
    // Now smooth the thickness for this list
    
    NewThick = new float[NumberOfCalculixNodes + 1];
    Area = new float[NumberOfCalculixNodes + 1];
    
    zero_float_array(NewThick, NumberOfCalculixNodes);
    zero_float_array(Area, NumberOfCalculixNodes);
    
    printf("Number of Calculix Elements: %d \n",NumberOfCalculixElements);fflush(NULL);
    
    for ( Iter = 1 ; Iter <= 50 ; Iter++ ) {
       
       for ( i = 1 ; i <= NumElementsToSmooth ; i++ ) {
          
          ElementID = ElementData[7*i-6];
     
          NewThick[ElementData[7*i-5]] += CalculixElement[ElementID].Thickness * CalculixElement[ElementID].Area;
          NewThick[ElementData[7*i-4]] += CalculixElement[ElementID].Thickness * CalculixElement[ElementID].Area;
          NewThick[ElementData[7*i-3]] += CalculixElement[ElementID].Thickness * CalculixElement[ElementID].Area;
          
          Area[ElementData[7*i-5]] += CalculixElement[ElementID].Area;
          Area[ElementData[7*i-4]] += CalculixElement[ElementID].Area;
          Area[ElementData[7*i-3]] += CalculixElement[ElementID].Area;
     
       }
      
       for ( i = 1 ; i <= NumberOfCalculixNodes ; i++ ) {
          
          if ( Area[i] > 0. ) NewThick[i] /= Area[i];
          
       }

       for ( i = 1 ; i <= NumElementsToSmooth ; i++ ) {
          
          ElementID = ElementData[7*i-6];
          
          Thickness = ( NewThick[ElementData[7*i-5]] + NewThick[ElementData[7*i-4]] + NewThick[ElementData[7*i-3]] )/3.;
             
          if ( Thickness > CalculixElement[ElementID].Thickness ) CalculixElement[ElementID].Thickness = Thickness;
   
       }       

       zero_float_array(NewThick, NumberOfCalculixNodes);
       zero_float_array(Area, NumberOfCalculixNodes);
    
    }
    
    delete [] NewThick;
    delete [] Area;    
  
}

/*##############################################################################
#                                                                              #
#                   ADBSLICER ResizeCalculixInputFileSkins                     #
#                                                                              #
##############################################################################*/

void ADBSLICER::ResizeCalculixInputFileSkins(char *filename1, char *newfilename)
{

    int AllDone, Done, NodeID, Editing, NumUsedElements;
    int i, j, k, t, m, ElementID, Node[6], Whacked, *ElementData, NumberOfElements, MaxElements;
    float x, y, z, Volume[2], MinThick, MaxThick, Area, Thickness;
    char file_name_w_ext[10000], DumChar[10000], SetName[10000], *VarArray, *ElementName;
    char SaveLine[10000], SearchLabel[10000];
    FILE *File1, *NewFile;
    
    Area = Volume[0] = Volume[1] = 0.;
    
    MinThick = 1.e9;
    MaxThick = -1.e9;

    // Create a temp array for the elment data
        
    MaxElements = 250000;
        
    ElementData = new int[8*MaxElements + 1];

    // Open calculix files
    
    sprintf(file_name_w_ext,"%s.inp",filename1);

    if ( (File1 = fopen(file_name_w_ext,"r")) == NULL ) {
       
       printf("Could not open calculix file: %s for input! \n",file_name_w_ext);
       
       fflush(NULL); exit(1);
       
    }

    sprintf(file_name_w_ext,"%s.inp",newfilename);

    if ( (NewFile = fopen(file_name_w_ext,"w")) == NULL ) {
       
       printf("Could not open calulix file: %s for output! \n",file_name_w_ext);
       
       fflush(NULL); exit(1);
       
    }
    
    sprintf(SearchLabel,"%s",ElementLabel_);

    AllDone = 0;
        
    i = 0;
        
    while ( !AllDone ) {
    
       if ( fgets(DumChar,2000,File1) == NULL ) AllDone = 1;
       
       // Look for element data

       if ( !AllDone && strstr(DumChar,"*ELEMENT") != NULL && strstr(DumChar,SearchLabel) != NULL ) {
    
          printf("Setting up thickness for... DumChar: %s \n",DumChar);
          
          sprintf(SaveLine,"%s",DumChar);

          j = Done = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,File1);
             
             if ( strlen(DumChar) > 2 && strstr(DumChar,"*") == NULL ) {
                
                // Write this back out
                
          //      fprintf(NewFile,"%s",DumChar);

                // Save element data for later
                                
                i++;
                
                j++;
         
                sscanf(DumChar,"%d,%d,%d,%d,%d,%d,%d",
                &(ElementID),
                &(Node[0]),
                &(Node[1]),
                &(Node[2]),
                &(Node[3]),
                &(Node[4]),
                &(Node[5]));   
                
                Area += CalculixElement[i].Area;
                
                Volume[0] += CalculixElement[i].Volume[0];
                Volume[1] += CalculixElement[i].Volume[1];
                                        
                if ( j > MaxElements ) {
                   
                   printf("Resizing element list... \n");fflush(NULL);
                   
                   ElementData = resize_int_array(ElementData, 8*MaxElements, 1.25*8*MaxElements);
                   
                   MaxElements = 1.25*MaxElements;
                   
                }
                                
                ElementData[8*j-7] = i;                                
                ElementData[8*j-6] = ElementID;
                ElementData[8*j-5] = Node[0];
                ElementData[8*j-4] = Node[1];
                ElementData[8*j-3] = Node[2];
                ElementData[8*j-2] = Node[3];
                ElementData[8*j-1] = Node[4];
                ElementData[8*j  ] = Node[5];
                     
             }
             
             else {
                
                Done = 1;
                
             }
             
          }
             
          NumberOfElements = j;

          // Sort the elements into groups
      
          CalculixMinSizingThickness_ = 1.e9;
          CalculixMaxSizingThickness_ = -1.e9;
              
          for ( k = 1 ; k <= NumberOfElements ; k++ ) {
             
             ElementID = ElementData[8*k-7];
                       
             CalculixMinSizingThickness_ = MIN(CalculixElement[ElementID].Thickness,CalculixMinSizingThickness_);
             CalculixMaxSizingThickness_ = MAX(CalculixElement[ElementID].Thickness,CalculixMaxSizingThickness_);
          
          }
          
          printf("CalculixMinSizingThickness_: %f \n",CalculixMinSizingThickness_);
          printf("CalculixMaxSizingThickness_: %f \n",CalculixMaxSizingThickness_);
          
          NumberOfCalculixSizingGroups_ = 11;
      
          CalculixDThickness_ = (CalculixMaxSizingThickness_ - CalculixMinSizingThickness_)/(NumberOfCalculixSizingGroups_ - 1);
      
          for ( k = 1 ; k <= NumberOfElements ; k++ ) {
             
             ElementID = ElementData[8*k-7];
                 
             CalculixElement[ElementID].SizingGroup = NumberOfCalculixSizingGroups_ - 1;
             
          }
                 
          for ( t = NumberOfCalculixSizingGroups_ - 1 ; t >= 1 ; t-- ) {
             
             Thickness = CalculixMinSizingThickness_ + t*CalculixDThickness_;
             
             printf("Looking at Thickness group: %d --> Max thickness of: %f \n",t, Thickness);
             
             for ( k = 1 ; k <= NumberOfElements ; k++ ) {
                
                ElementID = ElementData[8*k-7];
                  
                if ( CalculixElement[ElementID].Thickness <= Thickness ) {
                   
               //  printf("Working on group: %d... with thickness: %f ... and element: %d has thickness: %f and is set to group: %d \n",
               //  t,
               //  Thickness,
               //  ElementID,
               //  CalculixElement[ElementID].Thickness,
               //  t);
                   
                   CalculixElement[ElementID].SizingGroup = t;
                   
                }
                
             }
             
          }
      
       //  for ( k = 1 ; k <= NumberOfElements ; k++ ) {
       //     
       //      ElementID = ElementData[8*k-7];
       //       
       //      printf("Element: %d has thickness: %f and is set to group: %d \n",
       //      ElementID,
       //      CalculixElement[ElementID].Thickness,
       //      CalculixElement[ElementID].SizingGroup);
       //     
       //  }

          printf("Writing out new element lists... \n");fflush(NULL);
          
          fprintf(NewFile,"%s",DumChar);   
          
          // Now write out element lists for each of these groups
          
          NumUsedElements = 0;
          
          for ( t = 1 ; t < NumberOfCalculixSizingGroups_ ; t++ ) {
       
             printf("Working on group: %d \n",t);fflush(NULL);
             
             fprintf(NewFile,"*ELEMENT, TYPE=S6, ELSET=%s.%-d\n",ElementLabel_,t);
             
             for ( k = 1 ; k <= NumberOfElements ; k++ ) {
                
                m = ElementData[8*k-7];  
                
                if ( CalculixElement[m].SizingGroup == t ) {
                  
                   fprintf(NewFile,"%d,%d,%d,%d,%d,%d,%d\n",
                      ElementData[8*k-6],
                      ElementData[8*k-5],
                      ElementData[8*k-4],
                      ElementData[8*k-3],
                      ElementData[8*k-2],
                      ElementData[8*k-1],
                      ElementData[8*k  ]);
                      
                   NumUsedElements++;
                                               
                }
                                                                     
             }
             
             fprintf(NewFile,"\n");
             fprintf(NewFile,"\n");
             
          }

          fprintf(NewFile,"\n"); 

          fprintf(NewFile,"%s",DumChar);   
          
          Done = 1;
          
          // Now write back out original element set
          
          fprintf(NewFile,"*ELSET, ELSET=%s\n",ElementLabel_);
          
          for ( k = 1 ; k <= NumberOfElements ; k++ ) {
             
             fprintf(NewFile,"%d\n",ElementData[8*k-6]);
              
          }
          
          fprintf(NewFile,"\n");
               
          Done = 1;
          
          printf("NumUsedElements: %d ... and NumberOfElements: %d \n",NumUsedElements,NumberOfElements);
          
       }

       else if ( !AllDone && strstr(DumChar,"*ELEMENT") != NULL && strstr(DumChar,SearchLabel) == NULL ) {
          
          printf("Skipping over element data for... DumChar: %s \n",DumChar);
        
          fprintf(NewFile,"%s",DumChar);
          
          Done = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,File1);
             
             if ( strlen(DumChar) > 2 && strstr(DumChar,"*") == NULL ) {
                
                i++;
                
                fprintf(NewFile,"%s",DumChar);
         
             }
             
             else {
                
                Done = 1;
                
                fprintf(NewFile,"%s",DumChar);
                
             }
             
          }
                    
       }       
       
       // Look for SHELL data

       else if ( !AllDone && strstr(DumChar,"*SHELL") != NULL ) {      
          
          if ( strstr(DumChar,SearchLabel) != NULL ) {
             
             printf("found shell skin statement... %s \n",DumChar); fflush(NULL);
             
             // Read in the 2nd line of this SHELL statement...
             
             fgets(DumChar,2000,File1);
             
             // Now write out element lists for each of these groups
          
             for ( t = 1 ; t < NumberOfCalculixSizingGroups_ ; t++ ) {
          
                Thickness = CalculixMinSizingThickness_ + t*CalculixDThickness_;
          
                fprintf(NewFile,"*SHELL SECTION, ELSET=%s.%-d, MATERIAL=2D_ELEMENT, OFFSET= -0.5\n",ElementLabel_,t);
                fprintf(NewFile,"%f \n",Thickness); 
                fprintf(NewFile,"\n"); 

             }             
             
          }
          
          else {
          
             fprintf(NewFile,"%s",DumChar);          
             
          } 
          
       }
       
       else if ( !AllDone ) {
          
          fprintf(NewFile,"%s",DumChar);
          
       }
       
    }
    
    printf("New minimum thickness: %f \n",CalculixMinSizingThickness_);
    printf("New maximum thickness: %f \n",CalculixMaxSizingThickness_);
    printf("\n");
    printf("Old volume: %f \n",Volume[0]);
    printf("New volume: %f \n",Volume[1]);
    printf("Volume Ratio New/Old: %f \n",Volume[1]/Volume[0]);
    printf("\n");
    printf("Wetted Area: %f \n",Area);
    printf("\n");
    printf("Old average thickness: %f \n",Volume[0]/Area);    
    printf("New average thickness: %f \n",Volume[1]/Area);

    fclose(File1);
    fclose(NewFile);
                
}

/*##############################################################################
#                                                                              #
#                   ADBSLICER ResizeCalculixInputFileSkins                     #
#                                                                              #
##############################################################################*/

void ADBSLICER::ResizeCalculixInputFileSkinsOld(char *filename1, char *newfilename)
{

    int AllDone, Done, NodeID, Editing;
    int i, j, k, ElementID, Node[6], Whacked, *ElementData, NumberOfElements, MaxElements;
    float x, y, z, Volume[2], MinThick, MaxThick, Area;
    char file_name_w_ext[10000], DumChar[10000], SetName[10000], *VarArray, *ElementName;
    char SaveLine[10000], SearchLabel[10000];
    FILE *File1, *NewFile;
    
    Area = Volume[0] = Volume[1] = 0.;
    
    MinThick = 1.e9;
    MaxThick = -1.e9;

    // Create a temp array for the elment data
        
    MaxElements = 250000;
        
    ElementData = new int[7*MaxElements + 1];

    // Open calculix files
    
    sprintf(file_name_w_ext,"%s.inp",filename1);

    if ( (File1 = fopen(file_name_w_ext,"r")) == NULL ) {
       
       printf("Could not open calculix file: %s for input! \n",file_name_w_ext);
       
       fflush(NULL); exit(1);
       
    }

    sprintf(file_name_w_ext,"%s.inp",newfilename);

    if ( (NewFile = fopen(file_name_w_ext,"w")) == NULL ) {
       
       printf("Could not open calulix file: %s for output! \n",file_name_w_ext);
       
       fflush(NULL); exit(1);
       
    }
    
    sprintf(SearchLabel,"%s",ElementLabel_);

    AllDone = 0;
        
    i = 0;
        
    while ( !AllDone ) {
    
       if ( fgets(DumChar,2000,File1) == NULL ) AllDone = 1;
       
       // Look for element data

       if ( !AllDone && strstr(DumChar,"*ELEMENT") != NULL && strstr(DumChar,SearchLabel) != NULL ) {
    
          printf("Setting up thickness for... DumChar: %s \n",DumChar);
          
          sprintf(SaveLine,"%s",DumChar);

          j = Done = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,File1);
             
             if ( strlen(DumChar) > 2 && strstr(DumChar,"*") == NULL ) {
                
                i++;
                
                j++;
         
                sscanf(DumChar,"%d,%d,%d,%d,%d,%d,%d",
                &(ElementID),
                &(Node[0]),
                &(Node[1]),
                &(Node[2]),
                &(Node[3]),
                &(Node[4]),
                &(Node[5]));   
                
                Area += CalculixElement[i].Area;
                
                Volume[0] += CalculixElement[i].Volume[0];
                Volume[1] += CalculixElement[i].Volume[1];
                                        
                if ( j > MaxElements ) {
                   
                   ElementData = resize_int_array(ElementData, MaxElements, 1.25*MaxElements);
                   
                   MaxElements = 1.25*MaxElements;
                   
                }
                
                // Save element data for later
                
                ElementData[7*j-6] = ElementID;
                ElementData[7*j-5] = Node[0];
                ElementData[7*j-4] = Node[1];
                ElementData[7*j-3] = Node[2];
                ElementData[7*j-2] = Node[3];
                ElementData[7*j-1] = Node[4];
                ElementData[7*j  ] = Node[5];

                // Write this back out, along with a SHELL statement
                
                fprintf(NewFile,"*ELEMENT, TYPE=S6, ELSET=%s.%-d\n",ElementLabel_,i);
                
                fprintf(NewFile,"%d,%d,%d,%d,%d,%d,%d\n",
                 ElementID, Node[0], Node[1], Node[2], Node[3], Node[4], Node[5]);
                 
                fprintf(NewFile,"\n"); 
                 
                fprintf(NewFile,"*SHELL SECTION, ELSET=%s.%-d, MATERIAL=2D_ELEMENT, OFFSET= -0.5\n",ElementLabel_,i);
                fprintf(NewFile,"%f \n",CalculixElement[i].Thickness); 
                
                MinThick = MIN(MinThick, CalculixElement[i].Thickness);
                MaxThick = MAX(MaxThick, CalculixElement[i].Thickness);
                
                fprintf(NewFile,"\n"); 
                     
             }
             
             else {
                
                fprintf(NewFile,"%s",DumChar);   
                
                Done = 1;
                
                // Now write back out original element set
                
                fprintf(NewFile,"*ELSET, ELSET=%s\n",ElementLabel_);
                
                NumberOfElements = j;
                
                for ( i = 1 ; i <= NumberOfElements ; i++ ) {
                   
                   fprintf(NewFile,"%d\n",ElementData[7*i-6]);
                    
                }
                
                fprintf(NewFile,"\n");
                
             }
             
          }
  
       }
       
       else if ( !AllDone && strstr(DumChar,"*ELEMENT") != NULL && strstr(DumChar,SearchLabel) == NULL ) {
          
         // printf("Skipping over element data for... DumChar: %s \n",DumChar);
         fprintf(NewFile,"%s",DumChar);
          
          Done = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,File1);
             
             if ( strlen(DumChar) > 2 && strstr(DumChar,"*") == NULL ) {
                
                i++;
         
             }
             
             else {
                
                Done = 1;
                
             }
             
             fprintf(NewFile,"%s",DumChar);

          }
                    
       }       
       
       // Look for SHELL data

       if ( !AllDone && strstr(DumChar,"*SHELL") != NULL ) {      
          
          if ( strstr(DumChar,SearchLabel) != NULL ) {
             
             printf("found shell skin statement... %s \n",DumChar); fflush(NULL);
             
             // Read in the 2nd line of this SHELL statement...
             
             fgets(DumChar,2000,File1);
             
          }
          
          else {
          
             fprintf(NewFile,"%s",DumChar);          
             
          } 
          
       }
       
       else if ( !AllDone ) {
          
          fprintf(NewFile,"%s",DumChar);
          
       }
       
    }
    
    printf("New minimum thickness: %f \n",MinThick);
    printf("New maximum thickness: %f \n",MaxThick);
    printf("\n");
    printf("Old volume: %f \n",Volume[0]);
    printf("New volume: %f \n",Volume[1]);
    printf("Volume Ratio New/Old: %f \n",Volume[1]/Volume[0]);
    printf("\n");
    printf("Wetted Area: %f \n",Area);
    printf("\n");
    printf("Old average thickness: %f \n",Volume[0]/Area);    
    printf("New average thickness: %f \n",Volume[1]/Area);

    fclose(File1);
    fclose(NewFile);
                
}

/*##############################################################################
#                                                                              #
#            ADBSLICER LoadCalculixINPFileSurfaceElements                      #
#                                                                              #
##############################################################################*/

void ADBSLICER::LoadCalculixINPFileSurfaceElements(char *name)
{
   
    int i, Done, AllDone, MoreElements, DumInt1, DumInt2;
    int node1, node2, node3, node4, node5, node6;
    int NumNodes, NumElements;
    int *NodeIsUsed, MaxNodes, BestNode;
    int *TriIsUsed, MaxTris;
    float Distance, MinDistance;
    char file_name_w_ext[10000], DumChar[10000], SaveChar[10000];
    char SetName[2000], TypeName[2000];
    FILE *CalculixFile;

    // Open the aerothermal data base file

    sprintf(file_name_w_ext,"%s.inp",name);

    if ( (CalculixFile = fopen(file_name_w_ext,"r")) == NULL ) {

       printf("Could not open calculix input file file... ! \n");fflush(NULL);
                     
       exit(1);
       
    }
    
    NumNodes = NumElements = 0;
    
    // Determine how many nodes there are...
    
    NumNodes = 0;

    AllDone = 0;
    
    while ( !AllDone ) {
       
       if ( fgets(DumChar,2000,CalculixFile) == NULL ) AllDone = 1;
           
       // Node list
       
       if ( strstr(DumChar,"*NODE") != NULL && strstr(DumChar,"PRINT") == NULL && strstr(DumChar,"FILE") == NULL ) {
          
          Done = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,CalculixFile);
       
             if ( strlen(DumChar) > 2 ) {
                
                NumNodes++;
                
             }
             
             else {
                
                Done = 1;

             }
             
          }
       
       }
      
    }

    if ( Verbose_ ) printf("Found a total of %d nodes \n",NumNodes);

    rewind(CalculixFile);
    
    Done = 0;
    
    while ( !Done ) {

       if ( fgets(DumChar,2000,CalculixFile) == NULL ) Done = 1;
                             
       // Element list

       if ( strstr(DumChar,"*ELEMENT") != NULL && strstr(DumChar,"ELSET=ESkin") != NULL ) {

          NumElements = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,CalculixFile);
   
             if ( strlen(DumChar) > 2 && strstr(DumChar,"*") == NULL ) {
                
                NumElements++;
        
             }
             
             else {
                
                Done = 1;
                
             }
             
          }
            
       }

    }
       
    if ( Verbose_ ) printf("Found %d surface nodes \n",NumNodes);
    if ( Verbose_ ) printf("Found %d surface elements \n",NumElements);
    fflush(NULL);
    
    FEM_Mesh.NodeList = new INTERP_NODE[NumNodes + 1];
    FEM_Mesh.TriList = new INTERP_TRI[NumElements + 1];
    
    rewind(CalculixFile);
    
    // Now read in the node data
   
    i = 0;
   
    AllDone = 0;

    while ( !AllDone ) {
       
       if ( fgets(DumChar,2000,CalculixFile) == NULL ) AllDone = 1;
           
       // Node list
       
       if ( strstr(DumChar,"*NODE") != NULL && strstr(DumChar,"PRINT") == NULL && strstr(DumChar,"FILE") == NULL ) {

          MinDistance = 1.e9;
          
          BestNode = 0;
          
          DumChar[strcspn(DumChar, "\n")] = 0;
              
          sprintf(SaveChar,"%s\0",DumChar);
          
          Done = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,CalculixFile);
        
             if ( strlen(DumChar) > 2 ) {
                
                i++;
              
                sscanf(DumChar,"%d,%f,%f,%f",
                &(FEM_Mesh.NodeList[i].node_id),
                &(FEM_Mesh.NodeList[i].x),
                &(FEM_Mesh.NodeList[i].y),
                &(FEM_Mesh.NodeList[i].z));
                
                MaxCalculixNode_ = MAX(MaxCalculixNode_, FEM_Mesh.NodeList[i].node_id);
                
                if ( FindClosestNode_ ) {
                   
                   // Only look at Spar and Rib nodes
                   
                   if ( strstr(SaveChar,"Skin") == NULL ) {
                   
                      Distance = SQR(FEM_Mesh.NodeList[i].x-xyz_find_[0])
                               + SQR(FEM_Mesh.NodeList[i].y-xyz_find_[1]) 
                               + SQR(FEM_Mesh.NodeList[i].z-xyz_find_[2]);
                               
                      if ( Distance <= MinDistance ) {
                         
                         MinDistance = Distance;
                         
                         BestNode = i;
                         
                      }
                      
                   }
                   
                }
                            
             }
             
             else {
                
                Done = 1;
                
             }
             
          }
          
          if ( FindClosestNode_ && strstr(SaveChar,"Skin") == NULL ) {
             
             printf("Node %d at: %f %f %f with distance of: %f for: %s \n",
              FEM_Mesh.NodeList[BestNode].node_id,
              FEM_Mesh.NodeList[BestNode].x,
              FEM_Mesh.NodeList[BestNode].y,
              FEM_Mesh.NodeList[BestNode].z,
              MinDistance,
              SaveChar);

              
          }

       }
       
    }
 
    rewind(CalculixFile);    

    // Now read in the element data
   
    Done = 0;
    
    while ( !Done ) {
       
       if ( fgets(DumChar,2000,CalculixFile) == NULL ) Done = 1;
           
       // Element list
       
       if ( strstr(DumChar,"*ELEMENT") != NULL && strstr(DumChar,"ELSET=ESkin") != NULL ) {
          
          for ( i = 1 ; i <= NumElements ; i++ ) {
          
             fgets(DumChar,2000,CalculixFile);
             
             sscanf(DumChar,"%d,%d,%d,%d,%d,%d,%d",
             &(FEM_Mesh.TriList[i].element_id),
             &(FEM_Mesh.TriList[i].node1),
             &(FEM_Mesh.TriList[i].node2),
             &(FEM_Mesh.TriList[i].node3),
             &(FEM_Mesh.TriList[i].node4),
             &(FEM_Mesh.TriList[i].node5),
             &(FEM_Mesh.TriList[i].node6));

             MaxCalculixElement_ = MAX(MaxCalculixElement_, FEM_Mesh.TriList[i].element_id);
             
            
          }
       
       }
       
       else if ( strstr(DumChar,"*ELEMENT") != NULL && strstr(DumChar,"ELSET=ESkin") == NULL ) {

          MoreElements = 1;
          
          while ( MoreElements ) {
             
             fgets(DumChar,2000,CalculixFile);
             
             if ( strlen(DumChar) > 2 && strstr(DumChar,"*") == NULL ) {
             
                sscanf(DumChar,"%d,%d,%d,%d,%d,%d,%d",
                &(DumInt1),
                &(DumInt2),
                &(DumInt2),
                &(DumInt2),
                &(DumInt2),
                &(DumInt2),
                &(DumInt2));
                
                MaxCalculixElement_ = MAX(MaxCalculixElement_, DumInt1);
                
             }
             
             else {
                
                MoreElements = 0;
                
             }
             
          }
                
       }
       
    }
    
    fclose(CalculixFile);
     
    // Find the max node and element number... note this is the max element for just the surface tris.
    // It does not include rib or spar elements.... 

    MaxNodes = 0;
    
    for ( i = 1 ; i <= NumNodes ; i++ ) {
       
       MaxNodes = MAX(MaxNodes, FEM_Mesh.NodeList[i].node_id);
       
    }
    
    MaxTris = 0;

    for ( i = 1 ; i <= NumElements ; i++ ) {
       
       MaxTris = MAX(MaxTris, FEM_Mesh.TriList[i].element_id);
       
    }
           
    NodeIsUsed = new int[MaxNodes + 1];
    
    zero_int_array(NodeIsUsed, MaxNodes);
    
    for ( i = 1 ; i <= NumNodes ; i++ ) {
       
       NodeIsUsed[FEM_Mesh.NodeList[i].node_id] = i;
       
    }
        
    // Re-number the element connectivity
    
    for ( i = 1 ; i <= NumElements ; i++ ) {
       
       node1 = NodeIsUsed[FEM_Mesh.TriList[i].node1];
       node2 = NodeIsUsed[FEM_Mesh.TriList[i].node2];
       node3 = NodeIsUsed[FEM_Mesh.TriList[i].node3];
       node4 = NodeIsUsed[FEM_Mesh.TriList[i].node4];
       node5 = NodeIsUsed[FEM_Mesh.TriList[i].node5];
       node6 = NodeIsUsed[FEM_Mesh.TriList[i].node6];


       FEM_Mesh.TriList[i].node1 = node1;
       FEM_Mesh.TriList[i].node2 = node2;
       FEM_Mesh.TriList[i].node3 = node3;
       FEM_Mesh.TriList[i].node4 = node4;
       FEM_Mesh.TriList[i].node5 = node5;
       FEM_Mesh.TriList[i].node6 = node6;
       
    }    
    
    FEM_Mesh.number_of_nodes = NumNodes;
    FEM_Mesh.number_of_tris = NumElements;    

    delete [] NodeIsUsed;
         
}

/*##############################################################################
#                                                                              #
#                      ADBSLICER RenumberCalulixINPFile                        #
#                                                                              #
##############################################################################*/

int *ADBSLICER::RenumberCalulixINPFile(char *name)
{
   
    int i, Done, AllDone, MoreElements, DumInt1, DumInt2;
    int node1, node2, node3, node4, node5, node6;
    int NumNodes, NumElements;
    int *NodeIsUsed, MaxNodes, BestNode;
    int *TriIsUsed, MaxTris;
    float Distance, MinDistance;
    char file_name_w_ext[10000], DumChar[10000], SaveChar[10000];
    char SetName[2000], TypeName[2000];
    FILE *CalculixFile;

    // Open the aerothermal data base file

    sprintf(file_name_w_ext,"%s.inp",name);

    if ( (CalculixFile = fopen(file_name_w_ext,"r")) == NULL ) {

       printf("Could not open calculix input file file... ! \n");fflush(NULL);
                     
       exit(1);
       
    }
    
    NumNodes = NumElements = 0;
    
    // Determine how many nodes there are...
    
    NumNodes = 0;

    AllDone = 0;
    
    while ( !AllDone ) {
       
       if ( fgets(DumChar,2000,CalculixFile) == NULL ) AllDone = 1;
           
       // Node list
       
       if ( strstr(DumChar,"*NODE") != NULL && strstr(DumChar,"PRINT") == NULL && strstr(DumChar,"FILE") == NULL ) {
          
          Done = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,CalculixFile);
       
             if ( strlen(DumChar) > 2 ) {
                
                NumNodes++;
                
             }
             
             else {
                
                Done = 1;

             }
             
          }
       
       }
      
    }

    if ( Verbose_ ) printf("Found a total of %d nodes \n",NumNodes);

    rewind(CalculixFile);
    
    AllDone = Done = 0;
    
    NumElements = 0;

    while ( !AllDone ) {

       if ( fgets(DumChar,2000,CalculixFile) == NULL ) AllDone = 1;
                             
       // Element list

       if ( strstr(DumChar,"*ELEMENT") != NULL ) {

          Done = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,CalculixFile);
   
             if ( strlen(DumChar) > 2 && strstr(DumChar,"*") == NULL ) {
                
                NumElements++;
        
             }
             
             else {
                
                Done = 1;
                
             }
             
          }
            
       }

    }
       
    if ( Verbose_ ) printf("Found %d nodes \n",NumNodes);
    if ( Verbose_ ) printf("Found %d elements \n",NumElements);
    
    fflush(NULL);
    
    FEM_Mesh.NodeList = new INTERP_NODE[NumNodes + 1];
    FEM_Mesh.TriList = new INTERP_TRI[NumElements + 1];
    
    rewind(CalculixFile);
    
    // Now read in the node data
   
    i = 0;
   
    AllDone = 0;

    while ( !AllDone ) {
       
       if ( fgets(DumChar,2000,CalculixFile) == NULL ) AllDone = 1;
           
       // Node list
       
       if ( strstr(DumChar,"*NODE") != NULL && strstr(DumChar,"PRINT") == NULL && strstr(DumChar,"FILE") == NULL ) {

          MinDistance = 1.e9;
          
          BestNode = 0;
          
          DumChar[strcspn(DumChar, "\n")] = 0;
              
          sprintf(SaveChar,"%s\0",DumChar);
          
          Done = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,CalculixFile);
        
             if ( strlen(DumChar) > 2 ) {
                
                i++;
              
                sscanf(DumChar,"%d,%f,%f,%f",
                &(FEM_Mesh.NodeList[i].node_id),
                &(FEM_Mesh.NodeList[i].x),
                &(FEM_Mesh.NodeList[i].y),
                &(FEM_Mesh.NodeList[i].z));
                
                MaxCalculixNode_ = MAX(MaxCalculixNode_, FEM_Mesh.NodeList[i].node_id);

             }
             
             else {
                
                Done = 1;
                
             }
             
          }

       }
       
    }
    
    printf("Checking... i is: %d ... and... NumNodes is: %d \n",i,NumNodes);
 
    rewind(CalculixFile);    

    // Now read in the element data
   
    i = AllDone = Done = 0;
    
    while ( !AllDone ) {
       
       if ( fgets(DumChar,2000,CalculixFile) == NULL ) AllDone = 1;
           
       // Element list
       
       if ( strstr(DumChar,"*ELEMENT") != NULL && strstr(DumChar,"S6") != NULL ) {
          
          Done = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,CalculixFile);
                
             if ( strlen(DumChar) > 2 ) {
                
                i++;
                
                sscanf(DumChar,"%d,%d,%d,%d,%d,%d,%d",
                &(FEM_Mesh.TriList[i].element_id),
                &(FEM_Mesh.TriList[i].node1),
                &(FEM_Mesh.TriList[i].node2),
                &(FEM_Mesh.TriList[i].node3),
                &(FEM_Mesh.TriList[i].node4),
                &(FEM_Mesh.TriList[i].node5),
                &(FEM_Mesh.TriList[i].node6));
                
                FEM_Mesh.TriList[i].NumberOfNodes = 6;
   
                MaxCalculixElement_ = MAX(MaxCalculixElement_, FEM_Mesh.TriList[i].element_id);
                
             }
             
             else {
                
                Done = 1;
                
             }
             
          }
       
       }
       
       else if ( strstr(DumChar,"*ELEMENT") != NULL && strstr(DumChar,"B32") != NULL ) {
          
          Done = 0;
          
          while ( !Done ) {
             
             fgets(DumChar,2000,CalculixFile);
                
             if ( strlen(DumChar) > 2 ) {
                
                i++;
                
                sscanf(DumChar,"%d,%d,%d,%d",
                &(FEM_Mesh.TriList[i].element_id),
                &(FEM_Mesh.TriList[i].node1),
                &(FEM_Mesh.TriList[i].node2),
                &(FEM_Mesh.TriList[i].node3));
   
                FEM_Mesh.TriList[i].NumberOfNodes = 3;
   
                MaxCalculixElement_ = MAX(MaxCalculixElement_, FEM_Mesh.TriList[i].element_id);
                
             }
             
             else {
                
                Done = 1;
                
             }
          }
                
       }
       
    }
    
    printf("Checking... i is: %d ... and... NumElements is: %d \n",i,NumElements);
    
    fclose(CalculixFile);
     
    // Find the max node and element number... 

    MaxNodes = 0;
    
    for ( i = 1 ; i <= NumNodes ; i++ ) {
       
       MaxNodes = MAX(MaxNodes, FEM_Mesh.NodeList[i].node_id);
       
    }
    
    printf("Max node number: %d \n",MaxNodes);
    
    MaxTris = 0;

    for ( i = 1 ; i <= NumElements ; i++ ) {
       
       MaxTris = MAX(MaxTris, FEM_Mesh.TriList[i].element_id);
       
    }

    printf("Max element number: %d \n",MaxTris);
           
    CalculixNodePerm_ = new int[MaxNodes + 1];
    
    zero_int_array(CalculixNodePerm_, MaxNodes);
    
    for ( i = 1 ; i <= NumNodes ; i++ ) {
       
       CalculixNodePerm_[FEM_Mesh.NodeList[i].node_id] = i;
       
    }

    CalculixElementPerm_ = new int[MaxTris + 1];
    
    zero_int_array(CalculixElementPerm_, MaxTris);
    
    for ( i = 1 ; i <= NumElements ; i++ ) {
       
       CalculixElementPerm_[FEM_Mesh.TriList[i].element_id] = i;
       
    }
    
    FEM_Mesh.number_of_nodes = NumNodes;
    FEM_Mesh.number_of_tris = NumElements;    

    return NodeIsUsed;
         
}

/*##############################################################################
#                                                                              #
#                  ADBSLICER WriteOutRenumberedCalculixINPFile                 #
#                                                                              #
##############################################################################*/

void ADBSLICER::WriteOutRenumberedCalculixINPFile(char *name)
{

    int Done, NodeID, Editing;
    int ElementID, Node1, Node2, Node3, Node4, Node5, Node6;
    int EQNode[2], EQDir[2], CDir, DOF1, DOF2;
    float x, y, z, EQCoef[2], Pressure, CLoad;
    char file_name_w_ext[10000], DumChar[10000], SetName[10000];
    char MaterialName1[10000], MaterialName2[10000], MaterialName3[10000], NSkinLabel[10000];
    FILE *InpFile, *NewFile;
    
    // Open calculix file and create node permutation array
    
    RenumberCalulixINPFile(name);
 
    // Open calculix file
    
    sprintf(file_name_w_ext,"%s.inp",name);

    if ( (InpFile = fopen(file_name_w_ext,"r")) == NULL ) {
       
       printf("Could not open calulix file: %s for input! \n",file_name_w_ext);
       
       fflush(NULL); exit(1);
       
    }
    
    // Open output file
              
    sprintf(file_name_w_ext,"%s.renum.inp",name);
     
    if ( (NewFile = fopen(file_name_w_ext,"w")) != NULL ) {
       
       // Read in a line of the input Calculix file

       Done = 0;
       
       while ( !Done ) {
      
          if ( fgets(DumChar,2000,InpFile) == NULL ) Done = 1;

          // Node lists
          
          if ( strstr(DumChar,"*NODE") != NULL && strstr(DumChar,"PRINT") == NULL && strstr(DumChar,"FILE") == NULL  ) {
             
            // printf("Working Node List: %s \n",DumChar);fflush(NULL);

             fprintf(NewFile,"%s",DumChar);
          
             Editing = 1;
             
             while ( Editing ) {
                
                fgets(DumChar,2000,InpFile);
                
                if ( strlen(DumChar) > 2 ) {
                   
                   sscanf(DumChar,"%d,%f,%f,%f",
                   &(NodeID),
                   &(x),
                   &(y),
                   &(z));
          
                   fprintf(NewFile,"%d,%f,%f,%f\n",
                   CalculixNodePerm_[NodeID], 
                   x,
                   y,
                   z);
          
                }
                
                else {
                   
                   Editing = 0;
                   
                   fprintf(NewFile,"%s",DumChar);
                   
                }
                
             }
             
          }
          
          // Shell elements
          
          else if ( strstr(DumChar,"*ELEMENT") != NULL && strstr(DumChar,"S6") != NULL ) {
             
         //    printf("Working ELEMENT List: %s \n",DumChar);fflush(NULL);
             
             fprintf(NewFile,"%s",DumChar);				  
             
             Editing = 1;
             
             while ( Editing ) {
                
                fgets(DumChar,2000,InpFile);
                
                if ( strlen(DumChar) > 2 ) {
                   
                   sscanf(DumChar,"%d,%d,%d,%d,%d,%d,%d",
                   &(ElementID),
                   &(Node1),
                   &(Node2),
                   &(Node3),
                   &(Node4),
                   &(Node5),
                   &(Node6));
          
                   fprintf(NewFile,"%d,%d,%d,%d,%d,%d,%d\n",
                   CalculixElementPerm_[ElementID],
                   CalculixNodePerm_[Node1],
                   CalculixNodePerm_[Node2],
                   CalculixNodePerm_[Node3],
                   CalculixNodePerm_[Node4],
                   CalculixNodePerm_[Node5],
                   CalculixNodePerm_[Node6]);
                   
                }
                
                else {
                   
                   Editing = 0;
                   
                   fprintf(NewFile,"%s",DumChar);
                   
                }
                
             }
          
          }
          
          // Beam elements
          
          else if ( strstr(DumChar,"*ELEMENT") != NULL && strstr(DumChar,"B32") != NULL ) {
             
          //   printf("Working ELEMENT List: %s \n",DumChar);fflush(NULL);
             
             fprintf(NewFile,"%s",DumChar);				  
          
             Editing = 1;
             
             while ( Editing ) {
                
                fgets(DumChar,2000,InpFile);
                
                if ( strlen(DumChar) > 2 ) {
                   
                   sscanf(DumChar,"%d,%d,%d,%d",
                   &(ElementID),
                   &(Node1),
                   &(Node2),
                   &(Node3));
          
                   fprintf(NewFile,"%d,%d,%d,%d \n",
                   CalculixElementPerm_[ElementID],
                   CalculixNodePerm_[Node1],
                   CalculixNodePerm_[Node2],
                   CalculixNodePerm_[Node3]);
                   
                }
                
                else {
                   
                   Editing = 0;
                   
                   fprintf(NewFile,"%s",DumChar);
                   
                }
                
             }
          
          }
         

          else if ( strstr(DumChar,"*ELSET") != NULL ) {
             
            // printf("Working BOUNDARY List: %s \n",DumChar);fflush(NULL);
             
             fprintf(NewFile,"%s",DumChar);

             Editing = 1;
             
             while ( Editing ) {
          
                fgets(DumChar,2000,InpFile);
                
                if ( strlen(DumChar) > 2 ) {    
                   
                   sscanf(DumChar,"%d",
                   &(ElementID));
                
                   fprintf(NewFile,"%d\n",              
                     CalculixElementPerm_[ElementID]);     
                                    
                }
                
                else {
                   
                   fprintf(NewFile,"%s",DumChar);
                   
                   Editing = 0;
                   
                }
                
             }
                
          }
                    
          // Pressure loads
          
          else if ( strstr(DumChar,"*DLOAD") != NULL ) {
             
         //    printf("Working DLOAD List: %s \n",DumChar);fflush(NULL);
             
          //   printf("DLOAD ... %s \n",DumChar);
             
             fprintf(NewFile,"%s",DumChar);
             
             Editing = 1;
             
             while ( Editing ) {
          
                fgets(DumChar,2000,InpFile);
                
                if ( strlen(DumChar) > 2 ) {    
                   
                   if ( strstr(DumChar,"P1") != NULL ) {
                      
                      sscanf(DumChar,"%d, %2s, %f",
                      &ElementID,
                      DumChar,
                      &Pressure);
                   
                      fprintf(NewFile,"%d, P1, %f \n",CalculixElementPerm_[ElementID], Pressure);
                         
                   }
                   
                }
                
                else {
                   
                   fprintf(NewFile,"%s",DumChar);
                   
                   Editing = 0;
                   
                }
                
             }
          
          }

          // Point loads
          
          else if ( strstr(DumChar,"*CLOAD") != NULL ) {
             
            // printf("Working CLOAD List: %s \n",DumChar);fflush(NULL);
             
             fprintf(NewFile,"%s",DumChar);
             
             Editing = 1;
             
             while ( Editing ) {
          
                fgets(DumChar,2000,InpFile);
                
                if ( strlen(DumChar) > 2 ) {    
                   
                   sscanf(DumChar,"%d, %d, %f",
                   &ElementID,
                   &CDir,
                   &CLoad);
                
                   fprintf(NewFile,"%d, %d, %f \n",CalculixElementPerm_[ElementID],CDir,CLoad);
                    
                }
                
                else {
                   
                   fprintf(NewFile,"%s",DumChar);
                   
                   Editing = 0;
                   
                }
                
             }
          
          }
                    
          else if ( strstr(DumChar,"*EQUATION") != NULL ) {
             
            // printf("Working EQUATION List: %s \n",DumChar);fflush(NULL);
             
             fprintf(NewFile,"%s",DumChar);
             
             fgets(DumChar,2000,InpFile);
             
             fprintf(NewFile,"%s",DumChar);
             
             fgets(DumChar,2000,InpFile);
             
             sscanf(DumChar,"%d,%d,%f,%d,%d,%f",
             &(EQNode[0]),
             &(EQDir[0]),
             &(EQCoef[0]),           
             &(EQNode[1]),
             &(EQDir[1]),
             &(EQCoef[1]));

             fprintf(NewFile,"%d,%d,%f,%d,%d,%f \n",              
                CalculixNodePerm_[EQNode[0]],
                EQDir[0],
                EQCoef[0],           
                CalculixNodePerm_[EQNode[1]],
                EQDir[1],
                EQCoef[1]);
                
          }

          else if ( strstr(DumChar,"*BOUNDARY") != NULL ) {
             
            // printf("Working BOUNDARY List: %s \n",DumChar);fflush(NULL);
             
             fprintf(NewFile,"%s",DumChar);

             Editing = 1;
             
             while ( Editing ) {
          
                fgets(DumChar,2000,InpFile);
                
                if ( strlen(DumChar) > 2 ) {    
                   
                   sscanf(DumChar,"%d,%d,%d",
                   &(Node1),
                   &(DOF1),
                   &(DOF2));
                
                  fprintf(NewFile,"%d,%d,%d \n",              
                     CalculixNodePerm_[Node1],
                     DOF1,         
                     DOF2);     
                                    
                }
                
                else {
                   
                   fprintf(NewFile,"%s",DumChar);
                   
                   Editing = 0;
                   
                }
                
             }
                
          }
                    
          // Normal list
          
          else if ( strstr(DumChar,"*NORMAL") != NULL ) {

          //   printf("Working NORMAL List: %s \n",DumChar);fflush(NULL);

             Editing = 1;
             
             while ( Editing ) {
          
                fgets(DumChar,2000,InpFile);
                
                if ( strlen(DumChar) > 2 ) {    
                  
                   // Don't write back out the normal list
                   
                }
                
                else {
                   
                   fprintf(NewFile,"%s",DumChar);
                   
                   Editing = 0;
                   
                }
                
             }
          
          }
          
          else {
             
             fprintf(NewFile,"%s",DumChar);
             
          }                    
           
       }

       fclose(NewFile);
       
    }
       
}

/*##############################################################################
#                                                                              #
#                              ADBSLICER CreateVSPInterpMesh                   #
#                                                                              #
##############################################################################*/

void ADBSLICER::CreateVSPInterpMesh(void)
{
   
    int i;

    printf("fuckme! \n");fflush(NULL);
    
    printf("Number of VSP Nodes: %d \n",NumberOfNodes);
    printf("Number of VSP Tris: %d \n",NumberOfTris);
    
    VSP_Mesh.number_of_nodes = NumberOfNodes;
    VSP_Mesh.number_of_tris = NumberOfTris;
    
    if ( ModelType == VLM_MODEL ) {
       
       printf("Model is VLM \n");
       
       VSP_Mesh.number_of_nodes *= 2;
       
       VSP_Mesh.number_of_tris *= 2;
       
    }
    
    VSP_Mesh.NodeList = new INTERP_NODE[VSP_Mesh.number_of_nodes + 1];
    VSP_Mesh.TriList = new INTERP_TRI[VSP_Mesh.number_of_tris + 1];
    
    for ( i = 1 ; i <= NumberOfNodes ; i++ ) {
       
       VSP_Mesh.NodeList[i].x = NodeList_[i].x;
       VSP_Mesh.NodeList[i].y = NodeList_[i].y;
       VSP_Mesh.NodeList[i].z = NodeList_[i].z;

    }
    
    for ( i = 1 ; i <= NumberOfTris ; i++ ) {
       
       VSP_Mesh.TriList[i].node1 = TriList_[i].node1;
       VSP_Mesh.TriList[i].node2 = TriList_[i].node2;
       VSP_Mesh.TriList[i].node3 = TriList_[i].node3;
       
       VSP_Mesh.TriList[i].Cp = Cp[i];
       
       if ( ModelType == VLM_MODEL ) VSP_Mesh.TriList[i].Cp = 0.5*Cp[i];
      
    }   
    
    if ( ModelType == VLM_MODEL ) {

       for ( i = 1 ; i <= NumberOfNodes ; i++ ) {
          
          VSP_Mesh.NodeList[NumberOfNodes + i].x = NodeList_[i].x;
          VSP_Mesh.NodeList[NumberOfNodes + i].y = NodeList_[i].y;
          VSP_Mesh.NodeList[NumberOfNodes + i].z = NodeList_[i].z;
       
       }
    
       for ( i = 1 ; i <= NumberOfTris ; i++ ) {
          
          VSP_Mesh.TriList[NumberOfTris + i].node1 = NumberOfNodes + TriList_[i].node3;
          VSP_Mesh.TriList[NumberOfTris + i].node2 = NumberOfNodes + TriList_[i].node2;
          VSP_Mesh.TriList[NumberOfTris + i].node3 = NumberOfNodes + TriList_[i].node1;
          
          VSP_Mesh.TriList[i+NumberOfTris].Cp = -0.5*Cp[i];
     
       }     
       
    }     
        
    
}

/*##############################################################################
#                                                                              #
#                  ADBSLICER WriteOutCalculixStaticAnalysisFile                #
#                                                                              #
##############################################################################*/

void ADBSLICER::WriteOutCalculixStaticAnalysisFile(char *name, int AnalysisType)
{

    int Done, NodeID, Editing;
    int ElementID, Node1, Node2, Node3, Node4, Node5, Node6;
    float x, y, z;
    char file_name_w_ext[10000], DumChar[10000], SetName[10000];
    char MaterialName1[10000], MaterialName2[10000], MaterialName3[10000], NSkinLabel[10000];
    FILE *InpFile, *LoadFile;
 
    // Open calculix file
    
    sprintf(file_name_w_ext,"%s.inp",CalculixFileName);

    if ( (InpFile = fopen(file_name_w_ext,"r")) == NULL ) {
       
       printf("Could not open calulix file: %s for input! \n",file_name_w_ext);
       
       fflush(NULL); exit(1);
       
    }
    
    // Open output file
         
    if ( AnalysisType == CALCULIX_STATIC ) {
                      
       sprintf(file_name_w_ext,"%s.static.inp",name);
       
    }
    
    else if ( AnalysisType == CALCULIX_BUCKLE ) {
                      
       sprintf(file_name_w_ext,"%s.buckle.inp",name);
       
    }
    
    else {
       
       printf("Unknown CALCULIX analysis type! \n");
       fflush(NULL);exit(1);
       
    }       
     
    if ( (LoadFile = fopen(file_name_w_ext,"w")) != NULL ) {
       
       // Read in a line of the input Calculix file

       Done = 0;
       
       while ( !Done ) {
      
           if ( fgets(DumChar,2000,InpFile) == NULL ) Done = 1;
           
           // Look for **Materials
           
           if ( strstr(DumChar,"**Materials") != NULL ) {
              
              WriteOutCalculixBoundaryConditions(LoadFile);

           }
           
           // Look for Node lists
           
           else if ( strstr(DumChar,"*NODE") != NULL ) {
              
              printf("Node list! \n");
              
              sscanf(DumChar,"*NODE, NSET=%s",SetName);
              
              if ( !AddLabel_ ) {
				  
                 fprintf(LoadFile,"%s",DumChar);
                 
			     }
			     
			     else {
			   	  
                 fprintf(LoadFile,"*NODE,NSET=%s_%s\n",SetName,Label_);
			   	  
			     }

              printf("SetName: %s \n",SetName);
               
              if ( strstr(SetName,"NSkin") != NULL ) {
                 
                 sprintf(NSkinLabel,"%s",SetName);
                 
                 printf("NSkinLabel: %s \n",NSkinLabel);
                 
              }

              Editing = 1;
              
              while ( Editing ) {
                 
                 fgets(DumChar,2000,InpFile);
                 
                 if ( strlen(DumChar) > 2 ) {
                    
                    sscanf(DumChar,"%d,%f,%f,%f",
                    &(NodeID),
                    &(x),
                    &(y),
                    &(z));

                    fprintf(LoadFile,"%d,%f,%f,%f\n",
                     NodeID + NodeOffSet_, 
                     x,
                     y,
                     z);

                 }
                 
                 else {
                    
                    Editing = 0;
                    
                    fprintf(LoadFile,"%s",DumChar);
                    
                 }
                 
              }
             
           }
           
           else if ( strstr(DumChar,"*ELEMENT") != NULL && strstr(DumChar,"S6") != NULL ) {
              
              printf("Element list! \n");
              
              sscanf(DumChar,"*ELEMENT, TYPE=S6,%s",SetName);
              
              if ( !AddLabel_ ) {
              
                 fprintf(LoadFile,"%s",DumChar);
                 
			     }
			     
			     else {
			 	    
                 fprintf(LoadFile,"*ELEMENT, TYPE=S6, %s_%s\n",SetName,Label_);
                    
			     }				  
				  
              Editing = 1;
              
              while ( Editing ) {
                 
                 fgets(DumChar,2000,InpFile);
                 
                 if ( strlen(DumChar) > 2 ) {
                    
                    sscanf(DumChar,"%d,%d,%d,%d,%d,%d,%d",
                    &(ElementID),
                    &(Node1),
                    &(Node2),
                    &(Node3),
                    &(Node4),
                    &(Node5),
                    &(Node6));

                    fprintf(LoadFile,"%d,%d,%d,%d,%d,%d,%d\n",
                     ElementID + ElementOffSet_, 
                     Node1 + NodeOffSet_,
                     Node2 + NodeOffSet_,
                     Node3 + NodeOffSet_,
                     Node4 + NodeOffSet_,
                     Node5 + NodeOffSet_,
                     Node6 + NodeOffSet_);
                     
                 }
                 
                 else {
                    
                    Editing = 0;
                    
                    fprintf(LoadFile,"%s",DumChar);
                    
                 }
                 
              }

           }

           else if ( strstr(DumChar,"*ELEMENT") != NULL && strstr(DumChar,"B32") != NULL ) {
              
              printf("Element list! \n");
              
              sscanf(DumChar,"*ELEMENT, TYPE=S6,%s",SetName);
              
              if ( !AddLabel_ ) {
              
                 fprintf(LoadFile,"%s",DumChar);
                 
			     }
			     
			     else {
			 	    
                 fprintf(LoadFile,"*ELEMENT, TYPE=S6, %s_%s\n",SetName,Label_);
                    
			     }				  
				  
              Editing = 1;
              
              while ( Editing ) {
                 
                 fgets(DumChar,2000,InpFile);
                 
                 if ( strlen(DumChar) > 2 ) {
                    
                    sscanf(DumChar,"%d,%d,%d,%d",
                    &(ElementID),
                    &(Node1),
                    &(Node2),
                    &(Node3));

                    fprintf(LoadFile,"%d,%d,%d,%d\n",
                     ElementID + ElementOffSet_, 
                     Node1 + NodeOffSet_,
                     Node2 + NodeOffSet_,
                     Node3 + NodeOffSet_);
                     
                 }
                 
                 else {
                    
                    Editing = 0;
                    
                    fprintf(LoadFile,"%s",DumChar);
                    
                 }
                 
              }

           }
           
           else if ( strstr(DumChar,"*SHELL SECTION") != NULL ) {
              
              printf("Shell definition! \n");
              
              printf("DumChar: %s \n",DumChar);

              sprintf(MaterialName1,"");
              sprintf(MaterialName2,"");
              sprintf(MaterialName3,"");
              
              sscanf(DumChar,"*SHELL SECTION, ELSET=%s%s%s%s",SetName,MaterialName1,MaterialName2,MaterialName3);

              if ( strlen(MaterialName2) <= 2 ) sprintf(MaterialName2,"");
              if ( strlen(MaterialName3) <= 2 ) sprintf(MaterialName3,"");
              
              printf("MaterialName1: %s \n",MaterialName1);
              printf("MaterialName2: %s \n",MaterialName2);
              printf("MaterialName3: %s \n",MaterialName3);
     
              SetName[strlen(SetName)-1]=0;

              if ( !AddLabel_ ) {
				  
                 fprintf(LoadFile,"%s",DumChar);
                 
			     }
			     
			     else {
			 	                  
                 fprintf(LoadFile,"*SHELL SECTION, ELSET=%s_%s, %s_%s_%s_%s\n",SetName,Label_,MaterialName1,MaterialName2,MaterialName3,Label_);
                    
              }
              
           }

           else if ( strstr(DumChar,"*MATERIAL") != NULL ) {
              
              printf("Material definition! \n");

              sscanf(DumChar,"*MATERIAL, NAME=%s%s%s",MaterialName1,MaterialName2,MaterialName3);

              if ( strlen(MaterialName2) <= 2 ) sprintf(MaterialName2,"");
              if ( strlen(MaterialName3) <= 2 ) sprintf(MaterialName3,"");
     
              SetName[strlen(SetName)-1]=0;

              if ( !AddLabel_ ) {
				  
                 fprintf(LoadFile,"%s",DumChar);
                 
			     }
			     
			     else {
			     
                    fprintf(LoadFile,"*MATERIAL, NAME=%s_%s_%s_%s\n",MaterialName1,MaterialName2,MaterialName3,Label_);
                    
			     }			 
              
           }
                              
           else {
              
              fprintf(LoadFile,"%s",DumChar);
              
           }
           
       }
       
       if ( AnalysisType == CALCULIX_STATIC ) {
       
          fprintf(LoadFile,"\n");
          fprintf(LoadFile,"**Analysis\n");
          fprintf(LoadFile,"*STEP\n");
          fprintf(LoadFile,"*STATIC,DIRECT\n");
               
          WriteOutCalculixElementLoads(LoadFile);
          
          fprintf(LoadFile,"*NODE FILE\n");
          fprintf(LoadFile,"U\n");
          fprintf(LoadFile,"*EL FILE,OUTPUT=3D\n");
          fprintf(LoadFile,"S\n");
          
          if ( !AddLabel_ ) {

             fprintf(LoadFile,"*NODE PRINT, NSET=NALL\n");
             
		    }
		    
		    else {
		 	   
		       fprintf(LoadFile,"*NODE PRINT, NSET=NALL\n");
		 	   
		    }
		    
          fprintf(LoadFile,"U,RF\n");
          fprintf(LoadFile,"*END STEP\n");

       }
       
       else if ( AnalysisType == CALCULIX_BUCKLE ) {

          fprintf(LoadFile,"\n");
          fprintf(LoadFile,"**Analysis\n");
          fprintf(LoadFile,"*STEP\n");
          fprintf(LoadFile,"*BUCKLE\n");
          fprintf(LoadFile,"10,0.01\n");
               
          WriteOutCalculixElementLoads(LoadFile);
          
          fprintf(LoadFile,"*NODE FILE\n");
          fprintf(LoadFile,"U\n");
          fprintf(LoadFile,"*EL FILE,OUTPUT=3D\n");
          fprintf(LoadFile,"S\n");
          fprintf(LoadFile,"*EL PRINT,FREQUENCY=0\n");
          
          if ( !AddLabel_ ) {
          
             fprintf(LoadFile,"*NODE PRINT, FREQUENCY=0, NSET=NALL\n");
             
		    }
		    
		    else {
		 	   
               fprintf(LoadFile,"*NODE PRINT, FREQUENCY=0, NSET=NALL\n");
               
		    }
		  			  
          fprintf(LoadFile,"U,RF\n");
          fprintf(LoadFile,"*END STEP\n");
          
       }
       
       else {
          
          printf("Unknown CALCULIX analysis type! \n");
          fflush(NULL);exit(1);
          
       }
       
       fclose(LoadFile);
       
    }
       
}

/*##############################################################################
#                                                                              #
#                 ADBSLICER WriteOutCalculixBoundaryConditions                 #
#                                                                              #
##############################################################################*/

void ADBSLICER::WriteOutCalculixBoundaryConditions(FILE *LoadFile)
{

    int i;

    fprintf(LoadFile,"\n");
    fprintf(LoadFile,"*BOUNDARY\n");

    for ( i = 1 ; i <= FEM_Mesh.number_of_nodes ; i++ ) {
       
       if ( FEM_Mesh.NodeList[i].y <= BoundaryTolerance_ ) {
          
          fprintf(LoadFile,"%d, 1, 6\n",FEM_Mesh.NodeList[i].node_id + NodeOffSet_);
          
       }
    
    }    

    fprintf(LoadFile,"\n");
 
}

/*##############################################################################
#                                                                              #
#                       ADBSLICER WriteOutCalculixElementLoads                 #
#                                                                              #
##############################################################################*/

void ADBSLICER::WriteOutCalculixElementLoads(FILE *LoadFile)
{

    int i;

    fprintf(LoadFile,"\n");

    fprintf(LoadFile,"*DLOAD\n");       

    for ( i = 1 ; i <= FEM_Mesh.number_of_tris ; i++ ) {
       
       fprintf(LoadFile,"%d, P1, %f\n",FEM_Mesh.TriList[i].element_id + ElementOffSet_, -DynamicPressure_*FEM_Mesh.TriList[i].Cp);
    
    }    

    fprintf(LoadFile,"\n");
    
    
}

/*##############################################################################
#                                                                              #
#                         ADBSLICER MergeCalculixFiles                         #
#                                                                              #
##############################################################################*/

void ADBSLICER::MergeCalculixFiles(char *filename1, char *filename2, char *newfilename)
{

    int Done, NodeID, Editing;
    int ElementID, Node1, Node2, Node3, Node4, Node5, Node6;
    float x, y, z;
    char file_name_w_ext[10000], DumChar[10000], SetName[10000];
    char MaterialName1[10000], MaterialName2[10000], MaterialName3[10000], NSkinLabel[10000];
    FILE *File1, *File2, *NewFile;
 
    // Open calculix files
    
    sprintf(file_name_w_ext,"%s.inp",filename1);

    if ( (File1 = fopen(file_name_w_ext,"r")) == NULL ) {
       
       printf("Could not open calulix file: %s for input! \n",file_name_w_ext);
       
       fflush(NULL); exit(1);
       
    }

    sprintf(file_name_w_ext,"%s.inp",filename2);

    if ( (File2 = fopen(file_name_w_ext,"r")) == NULL ) {
       
       printf("Could not open calulix file: %s for input! \n",file_name_w_ext);
       
       fflush(NULL); exit(1);
       
    }
    
    sprintf(file_name_w_ext,"%s.inp",newfilename);

    if ( (NewFile = fopen(file_name_w_ext,"w")) == NULL ) {
       
       printf("Could not open calulix file: %s for output! \n",file_name_w_ext);
       
       fflush(NULL); exit(1);
       
    }
            
    // Copy everything up to *STEP statement from file 1
    
    fprintf(NewFile,"**Element, Node, Shell, and Material Data from File: %s \n",filename1);
    fprintf(NewFile,"\n");
       
    Done = 0;
    
    while ( !Done ) {
    
       if ( fgets(DumChar,2000,File1) == NULL ) Done = 1;
       
       // Look for *STEP

       if ( !Done && strstr(DumChar,"*STEP") != NULL ) {
          
          Done = 1;
          
       }
       
       else {
          
          // Keep track of largest node value
          
          fprintf(NewFile,"%s",DumChar);          
          
       }
        
    }

    // Copy everything up to *STEP statement from file 2
    
    fprintf(NewFile,"\n");    
    fprintf(NewFile,"**Element, Node, Shell, and Material Data from File: %s \n",filename2);
    fprintf(NewFile,"\n");
       
    Done = 0;
    
    while ( !Done ) {
    
       if ( fgets(DumChar,2000,File2) == NULL ) Done = 1;
       
       // Look for *STEP
       
       if ( !Done && strstr(DumChar,"*STEP") != NULL ) {
          
          // Keep reading until we get to the DLOAD statement
          
          while ( !Done ) {
             
             if ( fgets(DumChar,2000,File2) == NULL ) Done = 1;
             
             if ( strstr(DumChar,"*DLOAD") != NULL ) Done = 1;
             
          }
          
          Done = 1;
       
       }
       
       else {
          
          fprintf(NewFile,"%s",DumChar);
          
       }
        
    }     
    
    // Copy up to, and including, the DLOAD statement in File 1  
    
    fprintf(NewFile,"*NSET,NSET=NALL,GENERATE \n");
    fprintf(NewFile,"1,10000000\n");

    fprintf(NewFile,"*STEP\n");    

    Done = 0;
    
    while ( !Done ) {
    
       if ( fgets(DumChar,2000,File1) == NULL ) Done = 1;
       
       // Look for *DLOAD

       if ( !Done && strstr(DumChar,"*DLOAD") != NULL ) {

          fprintf(NewFile,"**LOAD Data from File: %s \n",filename1);
          fprintf(NewFile,"\n");
              
          fprintf(NewFile,"%s",DumChar);
          
          Done = 1;
          
       }
       
       else {
          
          fprintf(NewFile,"%s",DumChar);
          
       }
        
    }
    
    // Copy everything from File 1 down to the *NODE FILE statement
    
    Done = 0;
    
    while ( !Done ) {
    
       if ( fgets(DumChar,2000,File1) == NULL ) Done = 1;
       
       // Look for *NODE
       
       if ( !Done && strstr(DumChar,"*NODE FILE") != NULL ) {
          
          Done = 1;
       
       }
       
       else {
          
          fprintf(NewFile,"%s",DumChar);
          
       }
        
    }

    // Copy everything from File 2 down to the *NODE FILE statement
    
    fprintf(NewFile,"\n");    
    fprintf(NewFile,"**LOAD Data from File: %s \n",filename2);
    fprintf(NewFile,"\n");
    fprintf(NewFile,"*DLOAD\n");
       
    Done = 0;
    
    while ( !Done ) {
    
       if ( fgets(DumChar,2000,File2) == NULL ) Done = 1;
       
       // Look for *NODE
       
       if ( !Done && strstr(DumChar,"*NODE FILE") != NULL ) {

          Done = 1;

       }
       
       else {
          
          fprintf(NewFile,"%s",DumChar);
          
       }
        
    }

    // Copy remaining stuff from File 1
    
    fprintf(NewFile,"**Remaining Lines from File: %s \n",filename1);
    fprintf(NewFile,"\n");
       
    Done = 0;
    
    fprintf(NewFile,"*NODE FILE\n");
    
    while ( !Done ) {
    
       if ( fgets(DumChar,2000,File1) == NULL ) Done = 1;
       
       if ( !Done && strlen(DumChar) > 1 ) {
          
          fprintf(NewFile,"%s",DumChar);
       
       }
       
       else {
          
          Done = 1;
          
       }
        
    }
    
    fclose(File1);
    fclose(File2);
    fclose(NewFile);
                
}


//    
//*DLOAD,AMPLITUDE=A1
//Se1,P3,10.
//assigns a pressure loading with magnitude 10. times the amplitude curve of
//amplitude A1 to face number three of all elements belonging to set Se1.
//Example files: beamd.
//Following line for centrifugal loading:
//• Element number or element set label.
//• CENTRIF
//• rotational speed square (ω 2 )
//• Coordinate 1 of a point on the rotation axis
//• Coordinate 2 of a point on the rotation axis
//• Coordinate 3 of a point on the rotation axis
//• Component 1 of the normalized direction of the rotation axis    

/*##############################################################################
#                                                                              #
#                          ADBSLICER LoadCalculixData                          #
#                                                                              #
##############################################################################*/

void ADBSLICER::LoadCalculixData(char *filename)
{

    int i, j, node1, node2, node3, Node, ElementNode[20], NodeOffSet, DumInt, ElementType, Done;
    int *NodePointer, MaxNodeNumber, Set;
    double x, y, z, vec1[3], vec2[3], vec3[3], mag, Smag, Ratio, Thickness, cosPhi;
    double Dx, Dy, Dz, DsMax, DeltaMax, S1_Min, S2_Min, S3_Min, S1_Max, S2_Max, S3_Max;
    double S11, S22, S33, S12, S23, S31, I1, I2, I3, S1, S2, S3, Phi;
    char file_name_w_ext[80], ElementName[1000], DumChar[1000];
    FILE *frd_file, *sum_file, *SizingFile;
    fpos_t SaveLocation;

    // Open the calculix solution file
    
    printf("Opening the Calculix frd file... \n");

    sprintf(file_name_w_ext,"%s.frd",filename);
    
    if ( (frd_file = fopen(file_name_w_ext,"r")) != NULL ) {

       // Parse the file until we find the nodal data
       
       Done = 0;
       
       while ( !Done ) {
        
          fgets(DumChar,200,frd_file);
          
          if ( strstr(DumChar,"2C") != NULL ) {
                     
             Done = 1;
             
          }
          
       }
       
       fgetpos(frd_file, &SaveLocation);
       
       // Loop over the nodal data and determine how many nodes there are...
       
       NumberOfCalculixNodes = 0;
       
       Done = 0;
       
       while ( !Done ) {
        
          fgets(DumChar,200,frd_file);
            
          if ( strncmp(DumChar," -3",3) == 0 ) {
                     
           
             Done = 1;
             
          }
          
          NumberOfCalculixNodes++;
          
       }       
       
       NumberOfCalculixNodes--;
       
       printf("NumberOfCalculixNodes: %d \n",NumberOfCalculixNodes);
       
       // Find largest node number

       fsetpos(frd_file, &SaveLocation);
                     
       MaxNodeNumber = 0;
       
       for ( i = 1 ; i <= NumberOfCalculixNodes ; i++ ) {
        
          fscanf(frd_file,"%d %d %lf %lf %lf \n",&DumInt,&Node,&x,&y,&z);
          
          MaxNodeNumber = MAX(MaxNodeNumber, Node);
          
       }
       
       NodePointer = new int[MaxNodeNumber + 1];
       
       zero_int_array(NodePointer, MaxNodeNumber);
       
       // Rewind back to start of nodal data
       
       fsetpos(frd_file, &SaveLocation);

       // Allocate space and read in the nodal data
       
       CalculixNode = new CALCULIX_NODE[NumberOfCalculixNodes + 1];
 
       for ( i = 1 ; i <= NumberOfCalculixNodes ; i++ ) {
        
          fscanf(frd_file,"%d %d %lf %lf %lf \n",&DumInt,&Node,&x,&y,&z);
                    
          NodePointer[Node] = i;
          
          CalculixNode[i].x = x;
          CalculixNode[i].y = y;
          CalculixNode[i].z = z;
          
       }
       
       // Parse the file until we find the element data
       
       Done = 0;
       
       while ( !Done ) {
        
          fgets(DumChar,200,frd_file);
          
          if ( strstr(DumChar,"3C") != NULL ) {
                     
             Done = 1;
             
          }
          
       }       
       
       fgetpos(frd_file, &SaveLocation);
       
       // Loop over the element data and determine how many elements there are...
       
       NumberOfCalculixElements = 0;
       
       Done = 0;
       
       while ( !Done ) {
        
          fgets(DumChar,200,frd_file);
          
          if ( strstr(DumChar,"-3") != NULL ) {
                     
             Done = 1;
             
          }

          fgets(DumChar,200,frd_file);
          fgets(DumChar,200,frd_file);

          NumberOfCalculixElements++;
          
       }       
       
       NumberOfCalculixElements--;       

       printf("NumberOfCalculixElements: %d \n",NumberOfCalculixElements);
       
       // Rewind back to start of element data
       
       fsetpos(frd_file, &SaveLocation);

       // Allocate space and read in the nodal data
       
       CalculixElement = new CALCULIX_ELEMENT[NumberOfCalculixElements + 1];
       
       for ( i = 1 ; i <= NumberOfCalculixElements ; i++ ) {
        
          fscanf(frd_file,"%d %d %d %d %d \n",&DumInt,&DumInt,&ElementType,&DumInt,&DumInt);

          // Extract out the structural element set id
          
          sscanf(ElementName,"%dEL.%d",&DumInt,&Set);
          
          CalculixElement[i].StructuralElementSet = Set;
          
          CalculixElement[i].SizingGroup = 0;
             
          // 15 node prisms
   
          if ( ElementType == 5 ) {
           
             fscanf(frd_file,"%d %d %d %d %d %d %d %d %d %d %d \n",&DumInt,
                    &(ElementNode[ 0]),
                    &(ElementNode[ 1]),
                    &(ElementNode[ 2]),
                    &(ElementNode[ 3]),
                    &(ElementNode[ 4]),
                    &(ElementNode[ 5]),
                    &(ElementNode[ 6]),
                    &(ElementNode[ 7]),
                    &(ElementNode[ 8]),
                    &(ElementNode[ 9]));

             fscanf(frd_file,"%d %d %d %d %d %d \n",&DumInt,
                    &(ElementNode[10]),
                    &(ElementNode[11]),
                    &(ElementNode[12]),
                    &(ElementNode[13]),
                    &(ElementNode[14]));
                   
             // We only care about the first set of nodes.
             
             CalculixElement[i].Node[0] = NodePointer[ElementNode[0]];            
             CalculixElement[i].Node[1] = NodePointer[ElementNode[1]];                            
             CalculixElement[i].Node[2] = NodePointer[ElementNode[2]];                            
             CalculixElement[i].Node[3] = NodePointer[ElementNode[3]];                            
             CalculixElement[i].Node[4] = NodePointer[ElementNode[4]];                            
             CalculixElement[i].Node[5] = NodePointer[ElementNode[5]];
             
             CalculixElement[i].NumberOfNodes = 6;            

             // Calculate the thickness

             Dx = ( CalculixNode[CalculixElement[i].Node[0]].x - CalculixNode[CalculixElement[i].Node[3]].x
                  + CalculixNode[CalculixElement[i].Node[1]].x - CalculixNode[CalculixElement[i].Node[4]].x
                  + CalculixNode[CalculixElement[i].Node[2]].x - CalculixNode[CalculixElement[i].Node[5]].x )/3.;
                  
             Dy = ( CalculixNode[CalculixElement[i].Node[0]].y - CalculixNode[CalculixElement[i].Node[3]].y
                  + CalculixNode[CalculixElement[i].Node[1]].y - CalculixNode[CalculixElement[i].Node[4]].y
                  + CalculixNode[CalculixElement[i].Node[2]].y - CalculixNode[CalculixElement[i].Node[5]].y )/3.;

             Dz = ( CalculixNode[CalculixElement[i].Node[0]].z - CalculixNode[CalculixElement[i].Node[3]].z
                  + CalculixNode[CalculixElement[i].Node[1]].z - CalculixNode[CalculixElement[i].Node[4]].z
                  + CalculixNode[CalculixElement[i].Node[2]].z - CalculixNode[CalculixElement[i].Node[5]].z )/3.;
      
             CalculixElement[i].Thickness = sqrt( Dx*Dx + Dy*Dy + Dz*Dz );

          }
          
          else if ( ElementType == 4 ) {
           
             fscanf(frd_file,"%d %d %d %d %d %d %d %d %d %d %d \n",&DumInt,
                    &(ElementNode[ 0]),
                    &(ElementNode[ 1]),
                    &(ElementNode[ 2]),
                    &(ElementNode[ 3]),
                    &(ElementNode[ 4]),
                    &(ElementNode[ 5]),
                    &(ElementNode[ 6]),
                    &(ElementNode[ 7]),
                    &(ElementNode[ 8]),
                    &(ElementNode[ 9]));

             fscanf(frd_file,"%d %d %d %d %d %d %d %d %d %d %d \n",&DumInt,
                    &(ElementNode[10]),
                    &(ElementNode[11]),
                    &(ElementNode[12]),
                    &(ElementNode[13]),
                    &(ElementNode[14]),
                    &(ElementNode[15]),
                    &(ElementNode[16]),
                    &(ElementNode[17]),
                    &(ElementNode[18]),
                    &(ElementNode[19]));
                    
             // We only care about the first set of nodes.
             
             CalculixElement[i].Node[0] = NodePointer[ElementNode[0]];            
             CalculixElement[i].Node[1] = NodePointer[ElementNode[1]];                            
             CalculixElement[i].Node[2] = NodePointer[ElementNode[2]];                            
             CalculixElement[i].Node[3] = NodePointer[ElementNode[3]];                            
             CalculixElement[i].Node[4] = NodePointer[ElementNode[4]];                            
             CalculixElement[i].Node[5] = NodePointer[ElementNode[5]];                            
             CalculixElement[i].Node[6] = NodePointer[ElementNode[6]];                            
             CalculixElement[i].Node[7] = NodePointer[ElementNode[7]];   
             
             CalculixElement[i].NumberOfNodes = 8;
             
             // Calculate the thickness

             Dx = ( CalculixNode[CalculixElement[i].Node[0]].x - CalculixNode[CalculixElement[i].Node[4]].x
                  + CalculixNode[CalculixElement[i].Node[1]].x - CalculixNode[CalculixElement[i].Node[5]].x
                  + CalculixNode[CalculixElement[i].Node[2]].x - CalculixNode[CalculixElement[i].Node[6]].x
                  + CalculixNode[CalculixElement[i].Node[3]].x - CalculixNode[CalculixElement[i].Node[7]].x )/4.;
                  
             Dy = ( CalculixNode[CalculixElement[i].Node[0]].y - CalculixNode[CalculixElement[i].Node[4]].y
                  + CalculixNode[CalculixElement[i].Node[1]].y - CalculixNode[CalculixElement[i].Node[5]].y
                  + CalculixNode[CalculixElement[i].Node[2]].y - CalculixNode[CalculixElement[i].Node[6]].y
                  + CalculixNode[CalculixElement[i].Node[3]].y - CalculixNode[CalculixElement[i].Node[7]].y )/4.;

             Dz = ( CalculixNode[CalculixElement[i].Node[0]].z - CalculixNode[CalculixElement[i].Node[4]].z
                  + CalculixNode[CalculixElement[i].Node[1]].z - CalculixNode[CalculixElement[i].Node[5]].z
                  + CalculixNode[CalculixElement[i].Node[2]].z - CalculixNode[CalculixElement[i].Node[6]].z
                  + CalculixNode[CalculixElement[i].Node[3]].z - CalculixNode[CalculixElement[i].Node[7]].z )/4.;       
                        
             CalculixElement[i].Thickness = sqrt( Dx*Dx + Dy*Dy + Dz*Dz );
               
          }
          
          else {
             
             printf("wtf! \n");fflush(NULL);
             
          }
           
          if ( CalculixElement[i].NumberOfNodes == 8 ) {
              
             for ( j = 0 ; j <= 5 ; j++ ) {

                if ( j == 0 ) { node1 = 1 ; node2 = 4 ; node3 = 3; }; // Normal for Face 1-4-3-2
                if ( j == 1 ) { node1 = 5 ; node2 = 6 ; node3 = 7; }; // Normal for Face 5-6-7-8
                if ( j == 2 ) { node1 = 2 ; node2 = 3 ; node3 = 7; }; // Normal for Face 2-3-7-6
                if ( j == 3 ) { node1 = 4 ; node2 = 8 ; node3 = 7; }; // Normal for Face 4-8-7-3
                if ( j == 4 ) { node1 = 1 ; node2 = 5 ; node3 = 8; }; // Normal for Face 1-5-8-4
                if ( j == 5 ) { node1 = 2 ; node2 = 6 ; node3 = 5; }; // Normal for Face 1-2-6-5
                
                vec1[0] = CalculixNode[CalculixElement[i].Node[node1-1]].x - CalculixNode[CalculixElement[i].Node[node2-1]].x;
                vec1[1] = CalculixNode[CalculixElement[i].Node[node1-1]].y - CalculixNode[CalculixElement[i].Node[node2-1]].y;
                vec1[2] = CalculixNode[CalculixElement[i].Node[node1-1]].z - CalculixNode[CalculixElement[i].Node[node2-1]].z;

                vec2[0] = CalculixNode[CalculixElement[i].Node[node3-1]].x - CalculixNode[CalculixElement[i].Node[node2-1]].x;
                vec2[1] = CalculixNode[CalculixElement[i].Node[node3-1]].y - CalculixNode[CalculixElement[i].Node[node2-1]].y;
                vec2[2] = CalculixNode[CalculixElement[i].Node[node3-1]].z - CalculixNode[CalculixElement[i].Node[node2-1]].z;
                
                vector_cross(vec2,vec1,vec3);
                
                mag = sqrt(vector_dot(vec3,vec3));
                
                if ( j == 0 ) CalculixElement[i].Area = mag;
                
                CalculixElement[i].nx[j] = vec3[0]/mag;
                CalculixElement[i].ny[j] = vec3[1]/mag;
                CalculixElement[i].nz[j] = vec3[2]/mag;
                
             }
             
             CalculixElement[i].Volume[0] = CalculixElement[i].Area * CalculixElement[i].Thickness;
             
          }
          
          if ( CalculixElement[i].NumberOfNodes == 6 ) {
              
             CalculixElement[i].Area = 0.;

             for ( j = 0 ; j <= 4 ; j++ ) {

                if ( j == 0 ) { node1 = 1 ; node2 = 3 ; node3 = 2; }; // Normal for Face 1-3-2
                if ( j == 1 ) { node1 = 4 ; node2 = 5 ; node3 = 6; }; // Normal for Face 4-5-6
                if ( j == 2 ) { node1 = 1 ; node2 = 2 ; node3 = 5; }; // Normal for Face 1-2-5-4
                if ( j == 3 ) { node1 = 2 ; node2 = 3 ; node3 = 6; }; // Normal for Face 2-3-6-5
                if ( j == 4 ) { node1 = 1 ; node2 = 4 ; node3 = 6; }; // Normal for Face 1-4-6-3
      
                vec1[0] = CalculixNode[CalculixElement[i].Node[node1-1]].x - CalculixNode[CalculixElement[i].Node[node2-1]].x;
                vec1[1] = CalculixNode[CalculixElement[i].Node[node1-1]].y - CalculixNode[CalculixElement[i].Node[node2-1]].y;
                vec1[2] = CalculixNode[CalculixElement[i].Node[node1-1]].z - CalculixNode[CalculixElement[i].Node[node2-1]].z;

                vec2[0] = CalculixNode[CalculixElement[i].Node[node3-1]].x - CalculixNode[CalculixElement[i].Node[node2-1]].x;
                vec2[1] = CalculixNode[CalculixElement[i].Node[node3-1]].y - CalculixNode[CalculixElement[i].Node[node2-1]].y;
                vec2[2] = CalculixNode[CalculixElement[i].Node[node3-1]].z - CalculixNode[CalculixElement[i].Node[node2-1]].z;
                
                vector_cross(vec2,vec1,vec3);
                
                mag = sqrt(vector_dot(vec3,vec3));
                
                if ( j == 0 ) CalculixElement[i].Area = 0.5*mag;
                
                CalculixElement[i].nx[j] = vec3[0]/mag;
                CalculixElement[i].ny[j] = vec3[1]/mag;
                CalculixElement[i].nz[j] = vec3[2]/mag;
                
             }
             
             CalculixElement[i].Volume[0] = CalculixElement[i].Area * CalculixElement[i].Thickness;
             
          }          
     
       }       
             
       // Now read in the displacements 
       
       printf("Reading in displacements... \n");fflush(NULL);
          
       Done = 0;
       
       while ( !Done ) {
        
          fgets(DumChar,200,frd_file);
          
          if ( strstr(DumChar,"DISP") != NULL ) {
                     
             Done = 1;
 
          }
          
       }
       
       fgets(DumChar,200,frd_file);
       fgets(DumChar,200,frd_file);
       fgets(DumChar,200,frd_file);
       fgets(DumChar,200,frd_file);

       DsMax = 0.;
       
       DeltaMax = 0.;
       
       Done = 0;
       
       CalculixStressMagMin = 1.e9;
       CalculixStressMagMax = -1.e9;
       
       for ( i = 1 ; i <= NumberOfCalculixNodes ; i++ ) {
        
          fscanf(frd_file,"%d%d%lf%lf%lf \n",
                 &DumInt,
                 &Node,
                 &Dx,
                 &Dy,
                 &Dz);

          CalculixNode[i].dx = Dx;
          CalculixNode[i].dy = Dy;
          CalculixNode[i].dz = Dz;

          DsMax = MAX(DsMax,sqrt( pow(Dx,2.)
                                + pow(Dy,2.)
                                + pow(Dz,2.) ) );
                                   
       }
             
       // Now read in the stresses
       
       Done = 0;
       
       while ( !Done ) {
        
          fgets(DumChar,200,frd_file);
          
          if ( strstr(DumChar,"STRESS") != NULL ) {
                     
             Done = 1;
             
          }
          
       }
       
       fgets(DumChar,200,frd_file);
       fgets(DumChar,200,frd_file);
       fgets(DumChar,200,frd_file);
       fgets(DumChar,200,frd_file);
       fgets(DumChar,200,frd_file);
       fgets(DumChar,200,frd_file);
   
       Done = 0;
       
       S1_Min = S2_Min = S3_Min =  1.e9;
       S1_Max = S2_Max = S3_Max = -1.e9;

       for ( i = 1 ; i <= NumberOfCalculixNodes ; i++ ) {
        
          fscanf(frd_file,"%d%d%lf%lf%lf%lf%lf%lf \n",
                 &DumInt,
                 &Node,
                 &S11,
                 &S22,
                 &S33,
                 &S12,
                 &S23,
                 &S31);
                    
          // Calculate principal stresses
          
          I1 = S11 + S22 + S33;
          I2 = S11*S22 + S22*S33 + S33*S11 - S12*S12 - S23*S23 - S31*S31;
          I3 = S11*S22*S33 - S11*S23*S23 - S22*S31*S31 - S33*S12*S12 + 2.*S12*S23*S31;
          
          cosPhi = 0.5*(2*I1*I1*I1 - 9.*I1*I2 + 27.*I3) / pow(I1*I1 - 3.*I2,1.5);
          
          cosPhi = MIN(MAX(-1.,cosPhi),1.);
          
          Phi = acos( cosPhi )/3.;
                          
          S1 = I1/3 + 2.*sqrt(I1*I1-3.*I2)/3.*cos(Phi);
          S2 = I1/3 + 2.*sqrt(I1*I1-3.*I2)/3.*cos(Phi + 2.*PI/3.);
          S3 = I1/3 + 2.*sqrt(I1*I1-3.*I2)/3.*cos(Phi + 4.*PI/3.);     

          CalculixNode[i].S1 = S1;
          CalculixNode[i].S2 = S2;
          CalculixNode[i].S3 = S3;
          CalculixNode[i].SMag = sqrt( S1*S1 + S2*S2 + S3*S3 );
          
          CalculixStressMagMin = MIN(CalculixStressMagMin, CalculixNode[i].SMag);
          CalculixStressMagMax = MAX(CalculixStressMagMax, CalculixNode[i].SMag);
          
          S1_Min = MIN(S1,S1_Min);
          S2_Min = MIN(S2,S2_Min); 
          S3_Min = MIN(S3,S3_Min);
          
          S1_Max = MAX(S1,S1_Max);
          S2_Max = MAX(S2,S2_Max); 
          S3_Max = MAX(S3,S3_Max);
 
       }       

       // Close the calculix frd file

       fclose(frd_file);

    }
 
    printf("Max Deflection: %f \n",DsMax);
    printf("Min Principal Stresses: S1,2,3_min: %f, %f, %f \n",S1_Min,S2_Min,S3_Min);
    printf("Max Principal Stresses: S1,2,3_max: %f, %f, %f \n",S1_Max,S2_Max,S3_Max);

}
