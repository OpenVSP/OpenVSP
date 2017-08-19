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

    TriList  = NULL;
    NodeList = NULL;
    EdgeList = NULL;

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
#                           ADBSLICER LoadInitialData                          #
#                                                                              #
##############################################################################*/

void ADBSLICER::LoadFile(char *name)
{

    int i;
    char CommandLine[2000], file_name_w_ext[2000];
    FILE *adb_file;
    
    // Save the file name

    sprintf(file_name,"%s",name);

    // Determine if an adb file exists
    
    sprintf(file_name_w_ext,"%s.adb",file_name);

    if ( (adb_file = fopen(file_name_w_ext,"rb")) != NULL ) {
     
       fclose(adb_file); // It's repoened later

       // Load in the Mesh

       LoadMeshData();
       
       // Calculate the surface normals
   
       CalculateSurfaceNormals(1);
   
       // Create the tri to edge, and edge to tri pointers
   
       CreateTriEdges();
              
       // Load ADB Case list
       
       LoadSolutionCaseList();
       
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
     
       printf("No %s.adb file found! \n",file_name);
       printf("Please run vspaero first. \n");
       printf("You can run vspaero with the -geom option to only write out a viewable geometry. \n");
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

    char file_name_w_ext[2000], DumChar[1000], GridName[1000];
    int i, j, k, p, DumInt, Level, Edge, NumberOfControlSurfaceNodes;
    int TotNum, i_size, f_size, c_size;
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

    if ( DumInt != -123789456  ) {

       BIO.TurnByteSwapForReadsOn();

       rewind(adb_file);

       BIO.fread(&DumInt, i_size, 1, adb_file);

    }
    
    // Read in model type... VLM or PANEL
    
    BIO.fread(&ModelType, i_size, 1, adb_file);
    
    // Read in symmetry flag
    
    BIO.fread(&SymmetryFlag, i_size, 1, adb_file);    

    // Read in header

    BIO.fread(&NumberOfNodes,  i_size, 1, adb_file);
    BIO.fread(&NumberOfTris,   i_size, 1, adb_file);
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

    NodeList = new NODE[NumberOfNodes + 1];

    TriList = new TRI[NumberOfTris + 1];

    Cp        = new float[NumberOfTris + 1];
    CpNode    = new float[NumberOfNodes + 1];
    TotalArea = new float[NumberOfNodes + 1];

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
     
    }
    
    // Read in body ID flags, names...

    fread(&NumberOfBodies_, i_size, 1, adb_file);
    
    BodyListName_ = new char*[NumberOfBodies_ + 1];
    
    for ( i = 1 ; i <= NumberOfBodies_ ; i++ ) { 
     
       fread(&DumInt, i_size, 1, adb_file);
       
       BodyListName_[i] = new char[200];

       fread(BodyListName_[i], c_size, 100, adb_file);
       
       printf("Body: %d ... %s \n",i,BodyListName_[i]);fflush(NULL);
     
    } 
    
    // Read in Cart3d ID flags, names...

    fread(&NumberOfCart3dSurfaces_, i_size, 1, adb_file);
    
    Cart3dListName_ = new char*[NumberOfCart3dSurfaces_ + 1];
    
    for ( i = 1 ; i <= NumberOfCart3dSurfaces_ ; i++ ) { 
     
       fread(&DumInt, i_size, 1, adb_file);
       
       Cart3dListName_[i] = new char[200];

       fread(Cart3dListName_[i], c_size, 100, adb_file);
       
       printf("Cart3d: %d ... %s \n",i,Cart3dListName_[i]);fflush(NULL);
     
    }     

    // Load in the geometry and surface information

    for ( i = 1 ; i <= NumberOfTris ; i++ ) {

       // Geometry

       BIO.fread(&(TriList[i].node1),        i_size, 1, adb_file);
       BIO.fread(&(TriList[i].node2),        i_size, 1, adb_file);
       BIO.fread(&(TriList[i].node3),        i_size, 1, adb_file);
       BIO.fread(&(TriList[i].surface_type), i_size, 1, adb_file);
       BIO.fread(&(TriList[i].surface_id),   i_size, 1, adb_file);
       BIO.fread(&(TriList[i].area),         f_size, 1, adb_file);

    }

    for ( i = 1 ; i <= NumberOfNodes ; i++ ) {

       BIO.fread(&(NodeList[i].x), f_size, 1, adb_file);
       BIO.fread(&(NodeList[i].y), f_size, 1, adb_file);
       BIO.fread(&(NodeList[i].z), f_size, 1, adb_file);

    }
    
    // Find Min/Max of geometry

    FindMeshMinMax();
    
    // Read in any propulsion data
    
    BIO.fread(&(NumberOfPropulsionElements), i_size, 1, adb_file); 
    
    printf("NumberOfPropulsionElements: %d \n",NumberOfPropulsionElements);fflush(NULL);
    
    PropulsionElement = new PROPULSION_ELEMENT[NumberOfPropulsionElements + 1];
    
    for ( i = 1 ; i <= NumberOfPropulsionElements ; i++ ) {
     
       PropulsionElement[i].Rotor.Read_Binary_STP_Data(adb_file);
    
    }

    // Read in any coarse mesh edge data
    
    BIO.fread(&(NumberOfMeshLevels), i_size, 1, adb_file); 
    
    printf("NumberOfMeshLevels: %d \n",NumberOfMeshLevels);fflush(NULL);
    
    CoarseNodeList = new NODE*[NumberOfMeshLevels + 1];
    CoarseEdgeList = new EDGE*[NumberOfMeshLevels + 1];
    
    NumberOfCourseNodesForLevel = new int[NumberOfMeshLevels + 1];
    NumberOfCourseEdgesForLevel = new int[NumberOfMeshLevels + 1];
    
    for ( Level = 1 ; Level < NumberOfMeshLevels ; Level++ ) {
     
       BIO.fread(&(NumberOfCourseNodesForLevel[Level]), i_size, 1, adb_file);    
       BIO.fread(&(NumberOfCourseEdgesForLevel[Level]), i_size, 1, adb_file);          
     
       printf("Number of course nodes for level: %d is: %d \n",Level,NumberOfCourseNodesForLevel[Level]);fflush(NULL);
       printf("Number of course edges for level: %d is: %d \n",Level,NumberOfCourseEdgesForLevel[Level]);fflush(NULL);
  
       CoarseNodeList[Level] = new NODE[NumberOfCourseNodesForLevel[Level] + 1];
       CoarseEdgeList[Level] = new EDGE[NumberOfCourseEdgesForLevel[Level] + 1];

       for ( i = 1 ; i <= NumberOfCourseNodesForLevel[Level] ; i++ ) {
 
          BIO.fread(&(CoarseNodeList[Level][i].x), f_size, 1, adb_file);       
          BIO.fread(&(CoarseNodeList[Level][i].y), f_size, 1, adb_file);  
          BIO.fread(&(CoarseNodeList[Level][i].z), f_size, 1, adb_file);       
          
       }
         
       for ( i = 1 ; i <= NumberOfCourseEdgesForLevel[Level] ; i++ ) {
 
          BIO.fread(&(CoarseEdgeList[Level][i].SurfaceID), i_size, 1, adb_file);       
        
          BIO.fread(&(CoarseEdgeList[Level][i].node1), i_size, 1, adb_file);       
          BIO.fread(&(CoarseEdgeList[Level][i].node2), i_size, 1, adb_file);       
          
          CoarseEdgeList[Level][i].IsKuttaEdge = 0;
          
       }
    
    }    
    
    // Read in the kutta edge data
    
    Level = 1;
    
    BIO.fread(&(NumberOfKuttaEdges), i_size, 1, adb_file);       

    for ( i = 1 ; i <= NumberOfKuttaEdges; i++ ) {
       
       BIO.fread(&(Edge), i_size, 1, adb_file);      
       
       CoarseEdgeList[Level][Edge].IsKuttaEdge = 1;
        
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
    char file_name_w_ext[2000], DumChar[2000];
    FILE *adb_file;
    
    // Open the solution case list

    sprintf(file_name_w_ext,"%s.adb.cases",file_name);

    if ( (adb_file = fopen(file_name_w_ext,"r")) == NULL ) {

       printf("Could not open the adb case list file... ! \n");fflush(NULL);
                     
       exit(1);

    }       
    
    // Read in the cases until eof
    
    NumberOfADBCases_ = 0;
    
    while ( fgets(DumChar,2000,adb_file) != NULL ) {
       
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

    char file_name_w_ext[2000], DumChar[100], GridName[100];
    int i, j, k, m, p, Level, node1, node2, node3;
    int i_size, f_size, c_size;
    int DumInt, nod1, nod2, nod3, CFDCaseFlag, Edge;
    float FreeStreamPressure, DynamicPressure, Xc, Yc, Zc, Fx, Fy, Fz, Cf;
    float BoundaryLayerThicknessCode, LaminarDelta, TurbulentDelta, DumFloat;
    float Area;
    FILE *adb_file, *madb_file;
    BINARYIO BIO;
    long OffSet;

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
    
    // Read in the default text string to check on endianess

    BIO.fread(&DumInt, i_size, 1, adb_file);

    if ( DumInt != -123789456  ) {

       BIO.TurnByteSwapForReadsOn();

       rewind(adb_file);

       BIO.fread(&DumInt, i_size, 1, adb_file);

    }

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
   
       for ( m = 1 ; m <= NumberOfTris ; m++ ) {
   
          BIO.fread(&(Cp[m]), f_size, 1, adb_file); // Cp
    
       }
      
       // Read in the wake location data
       
       BIO.fread(&(NumberOfTrailingVortexEdges_), i_size, 1, adb_file); // Number of trailing wake vortices
   
       XWake_ = new float*[NumberOfTrailingVortexEdges_ + 1];
       YWake_ = new float*[NumberOfTrailingVortexEdges_ + 1];
       ZWake_ = new float*[NumberOfTrailingVortexEdges_ + 1];
       
       for ( i = 1 ; i <= NumberOfTrailingVortexEdges_ ; i++ ) {
        
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

        node1 = TriList[i].node1;
        node2 = TriList[i].node2;
        node3 = TriList[i].node3;

        Area = TriList[i].area;

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

       nod1 = TriList[i].node1;
       nod2 = TriList[i].node2;
       nod3 = TriList[i].node3;

       vec1[0] = NodeList[nod2].x - NodeList[nod1].x;
       vec1[1] = NodeList[nod2].y - NodeList[nod1].y;
       vec1[2] = NodeList[nod2].z - NodeList[nod1].z;

       vec2[0] = NodeList[nod3].x - NodeList[nod1].x;
       vec2[1] = NodeList[nod3].y - NodeList[nod1].y;
       vec2[2] = NodeList[nod3].z - NodeList[nod1].z;

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

       nod_list[1] = TriList[j].node1;
       nod_list[2] = TriList[j].node2;
       nod_list[3] = TriList[j].node3;

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

                if ( i == 1 ) TriList[j].edge1 = new_edge;
                if ( i == 2 ) TriList[j].edge2 = new_edge;
                if ( i == 3 ) TriList[j].edge3 = new_edge;

             }

             else {

                if ( i == 1 ) TriList[j].edge1 = -new_edge;
                if ( i == 2 ) TriList[j].edge2 = -new_edge;
                if ( i == 3 ) TriList[j].edge3 = -new_edge;

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

                   if ( i == 1 ) TriList[j].edge1 = level;
                   if ( i == 2 ) TriList[j].edge2 = level;
                   if ( i == 3 ) TriList[j].edge3 = level;

                }

                else {

                   if ( i == 1 ) TriList[j].edge1 = -level;
                   if ( i == 2 ) TriList[j].edge2 = -level;
                   if ( i == 3 ) TriList[j].edge3 = -level;

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

                   if ( i == 1 ) TriList[j].edge1 = new_edge;
                   if ( i == 2 ) TriList[j].edge2 = new_edge;
                   if ( i == 3 ) TriList[j].edge3 = new_edge;

                }

                else {

                   if ( i == 1 ) TriList[j].edge1 = -new_edge;
                   if ( i == 2 ) TriList[j].edge2 = -new_edge;
                   if ( i == 3 ) TriList[j].edge3 = -new_edge;

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

       EdgeList = new EDGE[NumberOfEdges + 1];

       // Fill the edge list

       for ( i = 1 ; i <= NumberOfNodes ; i++ ) {

          level = jump_pnt[i];

          while ( level != 0 ) {

			 EdgeList[level].node1 = i;
			 EdgeList[level].node2 = list[level].node;

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

             EdgeList[level].tri1 = list[level].tri_1;
             EdgeList[level].tri2 = list[level].tri_2;

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

	   XMin = MIN(XMin, NodeList[i].x);
	   YMin = MIN(YMin, NodeList[i].y);
	   ZMin = MIN(ZMin, NodeList[i].z);

	   XMax = MAX(XMax, NodeList[i].x);
	   YMax = MAX(YMax, NodeList[i].y);
	   ZMax = MAX(ZMax, NodeList[i].z);

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
    char file_name_w_ext[2000], CutType[200];
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

          noda = EdgeList[m].node1;
          nodb = EdgeList[m].node2;
 
          pnt_1[0] = NodeList[noda].x;
          pnt_1[1] = NodeList[noda].y;
          pnt_1[2] = NodeList[noda].z;
          
          if ( RotateGeometry ) {
           
             pnt_1[1] = NodeList[noda].y * CosRot - NodeList[noda].z * SinRot;
             pnt_1[2] = NodeList[noda].y * SinRot - NodeList[noda].z * CosRot;
             
          }

          Cp_1 = CpNode[noda];

          pnt_2[0] = NodeList[nodb].x;
          pnt_2[1] = NodeList[nodb].y;
          pnt_2[2] = NodeList[nodb].z;

          if ( RotateGeometry ) {
           
             pnt_2[1] = NodeList[nodb].y * CosRot - NodeList[nodb].z * SinRot;
             pnt_2[2] = NodeList[nodb].y * SinRot - NodeList[nodb].z * CosRot;
             
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

                pnt_1[0] = NodeList[noda].x;
                pnt_1[1] = NodeList[noda].y;
                pnt_1[2] = NodeList[noda].z;

                pnt_2[0] = NodeList[nodb].x;
                pnt_2[1] = NodeList[nodb].y;
                pnt_2[2] = NodeList[nodb].z;
             
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

    }

	 fprintf(SliceFile,"\n\n");

}
    
