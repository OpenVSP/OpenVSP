//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Surface.H"

#include "START_NAME_SPACE.H"

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
    
    NumberOfControlSurfaces_ = 0;
    
    MaxNumberOfControlSurfaces_ = 10;
    
    ControlSurface_ = new CONTROL_SURFACE[MaxNumberOfControlSurfaces_ + 1];
    
    // Initialize stuff

    NumberOfSurfacePatches_ = 0;

    NumGeomI_ = 0;
    NumGeomJ_ = 0;

    NumPlateI_ = 0;
    NumPlateJ_ = 0;
    
    DoGroundEffectsAnalysis_ = 0;

    GroundEffectsRotationAngle_ = 0.;
    
    GroundEffectsCGLocation_[0] = 0.;
    GroundEffectsCGLocation_[1] = 0.;
    GroundEffectsCGLocation_[2] = 0.;
    
    GroundEffectsHeightAboveGround_ = 0.;
    
    ComponentID_ = 0;
    
    Case_ = 0;
    
    SPRINTF(ComponentName_," ");
        
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
    
    x_  = new VSPAERO_DOUBLE[NumGeomI_*NumGeomJ_ + 1];
    y_  = new VSPAERO_DOUBLE[NumGeomI_*NumGeomJ_ + 1];
    z_  = new VSPAERO_DOUBLE[NumGeomI_*NumGeomJ_ + 1];    
    
    Nx_ = new VSPAERO_DOUBLE[NumGeomI_*NumGeomJ_ + 1];
    Ny_ = new VSPAERO_DOUBLE[NumGeomI_*NumGeomJ_ + 1];
    Nz_ = new VSPAERO_DOUBLE[NumGeomI_*NumGeomJ_ + 1];    
        
    u_  = new VSPAERO_DOUBLE[NumGeomI_*NumGeomJ_ + 1];
    v_  = new VSPAERO_DOUBLE[NumGeomI_*NumGeomJ_ + 1];    
    
    Area_ = new VSPAERO_DOUBLE[NumGeomI_*NumGeomJ_ + 1];    
    
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
    
    x_plate_ = new VSPAERO_DOUBLE[NumPlateI_*NumPlateJ_ + 1];
    y_plate_ = new VSPAERO_DOUBLE[NumPlateI_*NumPlateJ_ + 1];
    z_plate_ = new VSPAERO_DOUBLE[NumPlateI_*NumPlateJ_ + 1];    
    
    Nx_Camber_ = new VSPAERO_DOUBLE[NumPlateI_*NumPlateJ_ + 1];
    Ny_Camber_ = new VSPAERO_DOUBLE[NumPlateI_*NumPlateJ_ + 1];
    Nz_Camber_ = new VSPAERO_DOUBLE[NumPlateI_*NumPlateJ_ + 1];    
    
    Nx_FlatPlateNormal_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];    
    Ny_FlatPlateNormal_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];    
    Nz_FlatPlateNormal_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];    
    
    LocalChord_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];    
    
    xLE_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];    
    yLE_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];    
    zLE_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];        

    xTE_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];    
    yTE_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];    
    zTE_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];     
    
      s_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];     
      
    xLE_Def_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];    
    yLE_Def_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];    
    zLE_Def_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];        

    xTE_Def_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];    
    yTE_Def_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];    
    zTE_Def_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];       
    
    s_Def_ = new VSPAERO_DOUBLE[NumPlateI_ + 1];          

    u_plate_  = new VSPAERO_DOUBLE[NumPlateI_*NumPlateJ_ + 1];
    v_plate_ = new VSPAERO_DOUBLE[NumPlateI_*NumPlateJ_ + 1];

    Camber_ = new VSPAERO_DOUBLE[NumPlateI_*NumPlateJ_ + 1];
    
    ThicknessToChord_       = new VSPAERO_DOUBLE[NumI + 1];
    LocationOfMaxThickness_ = new VSPAERO_DOUBLE[NumI + 1];
    RadiusToChord_          = new VSPAERO_DOUBLE[NumI + 1];
        
    zero_double_array(Camber_, NumPlateI_*NumPlateJ_);
                
}

/*##############################################################################
#                                                                              #
#                          VSP_SURFACE ReadCart3DDataFromFile                  #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::ReadCart3DDataFromFile(char *Name, FILE *CART3D_File, FILE *TKEY_File)
{
 
    int i, k, n, NumNodes, NumTris, Node1, Node2, Node3, SurfaceID, Done;
    int *SurfaceList, Found, DumInt, CompID, *ComponentIDForVSPSurface;
    int *SurfaceIsUsed, NumberOfVSPSurfaces;
    char DumChar[2000], DumChar2[2000], CompName[200], Comma[2000], *Next;    
    VSPAERO_DOUBLE x, y, z, u1, u2, u3, v1, v2, v3;
    VSPAERO_DOUBLE Ymin, Ymax, Zmin, Zmax;

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

    IsLiftingSurface_ = 0;
    
    Grid().SizeKuttaNodeList(0);    

    // Read in xyz data
    
    for ( n = 1 ; n <= NumNodes ; n++ ) {
       
       fscanf(CART3D_File,"%lf %lf %lf \n",&x,&y,&z);

       Grid().NodeList(n).x() = x;
       Grid().NodeList(n).y() = y;
       Grid().NodeList(n).z() = z;
    
       Grid().NodeList(n).IsTrailingEdgeNode()   = 0;
       
       Grid().NodeList(n).IsLeadingEdgeNode()    = 0;
       
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

       Grid().TriList(n).SurfaceType()       = CART3D_SURFACE;
       
       Grid().TriList(n).DegenBodyID()       = 0;
       
       Grid().TriList(n).DegenWingID()       = 0;  
              
       Grid().TriList(n).SpanStation()       = 0;

       Grid().TriList(n).IsTrailingEdgeTri() = 0;
       
       Grid().TriList(n).IsLeadingEdgeTri()  = 0;
       
       SurfaceIsUsed[SurfaceID] = 1;
       
    }
 
    // Determine the number of surfaces
    
    NumberOfSurfacePatches_ = 0;
    
    SurfaceList = new int[NumTris + 1];
    
    zero_int_array(SurfaceList, NumTris);
    
    for ( n = 1 ; n <= NumTris ; n++ ) {
       
       SurfaceID = Grid().TriList(n).SurfaceID();
     
       i = 1;
       
       Found = 0;
       
       while ( !Found && i <= NumberOfSurfacePatches_ ) {
          
          if ( SurfaceList[i] == SurfaceID ) Found = 1;
          
          i++;
          
       }
       
       if ( !Found ) {
          
          NumberOfSurfacePatches_++;
          
          SurfaceList[NumberOfSurfacePatches_] = SurfaceID;
          
       }
       
    }
    
    SurfacePatchNameList_ = new char*[NumberOfSurfacePatches_ + 1];
        
    BoundaryConditionForSurface_ = new BOUNDARY_CONDITION_DATA[NumberOfSurfacePatches_ + 2];
    
    ComponentIDForSurfacePatch_ = new int[NumberOfSurfacePatches_ + 1];
    
    PRINTF("Found %d CART3D Surfaces \n",NumberOfSurfacePatches_);

    if ( TKEY_File != NULL ) {
       
       // Read in the vkey data
       
       fgets(DumChar,2000,TKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);
       fgets(DumChar,2000,TKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);
       fgets(DumChar,2000,TKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);
       
       sscanf(DumChar,"%d\n",&NumberOfVSPSurfaces);

       ComponentIDForVSPSurface = new int[NumberOfVSPSurfaces + 1];
       
       printf("NumberOfVSPSurfaces: %d \n",NumberOfVSPSurfaces);

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
             
             ComponentIDForSurfacePatch_[k] = n;
             
             SurfacePatchNameList_[k] = new char[2000];
                          
             sprintf(SurfacePatchNameList_[k],"%s",Next);
          
             printf("Surface: %d exists in tringulation and will be surface: %d with OpenVSP Name: %s \n",DumInt,k,SurfacePatchNameList_[k]);
  
          }
          
       }
       
       if ( k != NumberOfSurfacePatches_ ) {
          
          printf("Error... number of used surfaces in .tri and .tkey files do not match! \n");
          fflush(NULL);exit(1);
          
       }
          
    }
    
    else {
       
       for ( n = 1 ; n <= NumberOfSurfacePatches_ ; n++ ) {
 
          ComponentIDForSurfacePatch_[n] = n;
          
          SurfacePatchNameList_[n] = new char[2000];
          
          sprintf(SurfacePatchNameList_[n],"Surface_%d",n);
          
       }
       
    }

    for ( i = 1 ; i <= NumberOfSurfacePatches_ ; i++ ) {
       
       PRINTF("SurfaceList[%d]: %d \n",i,SurfaceList[i]);
       
    }

    fflush(NULL);
    
    if ( TKEY_File != NULL ) {
    
       for ( i = 1 ; i <= NumberOfVSPSurfaces ; i++ ) {
          
          PRINTF("ComponentIDForVSPSurface[%d]: %d \n",i,ComponentIDForVSPSurface[i]);
          
       }
       

       fflush(NULL);

       for ( i = 1 ; i <= NumberOfSurfacePatches_ ; i++ ) {
          
         ComponentIDForSurfacePatch_[i] = ComponentIDForVSPSurface[ComponentIDForSurfacePatch_[i]];
          
       }
       
    }
    
    for ( i = 1 ; i <= NumberOfSurfacePatches_ ; i++ ) {
       
       PRINTF("ComponentIDForSurfacePatch_[%d]: %d \n",i,ComponentIDForSurfacePatch_[i]);
       
    }
    
    fflush(NULL);
        
    // Renumber the surfaces
    
    for ( n = 1 ; n <= NumTris ; n++ ) {
    
       Done = 0;
       
       i = 1;
       
       while ( !Done && i <= NumberOfSurfacePatches_) {
          
          if ( Grid().TriList(n).SurfaceID() == SurfaceList[i] ) {
             
             Done = 1;

             if ( TKEY_File != NULL ) {
                
                Grid().TriList(n).ComponentID() = ComponentIDForVSPSurface[Grid().TriList(n).SurfaceID()];
                
             }
             
             else {
                
                Grid().TriList(n).ComponentID() = i;
                
             }
                          
             Grid().TriList(n).SurfaceID() = i;
            
             Grid().TriList(n).GeomID() = i;
             

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

    for ( i = 1 ; i <= NumberOfSurfacePatches_ ; i++ ) {
       
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
        
    // Do an RCM sort of the tris
 
    //DoRCMSort();
    
    // re-Calculate tri normals

    Grid().CalculateTriNormalsAndCentroids();

    // Build edge data structures
   
    Grid().CreateTriEdges();
    
    Grid().CalculateUpwindEdges();  
    
    // Now find sharp trailing edges
    
    FindSharpEdges(0,NULL);

}

/*##############################################################################
#                                                                              #
#                          VSP_SURFACE ReadVSPGeomDataFromFile                 #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::ReadVSPGeomDataFromFile(char *Name, FILE *VSPGeom_File, FILE *VKEY_File)
{
 
    int i, j, k, n, DumInt, DumInt2, NumNodes, NumTris, Node1, Node2, Node3, SurfaceID, Done;
    int *SurfaceList, Found, CompID;
    int *SurfaceIsUsed, NumberOfVSPSurfaces;    
    int NumKuttaNodeLists, NumKuttaNodes, NumNodesInList, *KuttaNodeList, NumColinearPoints;
    char DumChar[2000], DumChar2[2000], Comma[2000], *Next;
    VSPAERO_DOUBLE x, y, z, u1, v1, u2, v2, u3, v3;

    SPRINTF (Comma,",");

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
       
       Grid().NodeList(n).IsLeadingEdgeNode()    = 0;
       
       Grid().NodeList(n).IsBoundaryEdgeNode()   = 0;
       
       Grid().NodeList(n).IsBoundaryCornerNode() = 0;
         
    }  
    
    // Read in the tri data

    fscanf(VSPGeom_File,"%d",&NumTris);

    PRINTF("NumTris: %d \n",NumTris);    

    Grid().SizeTriList(NumTris);
    
    // Read in connectivity and UV data

    for ( n = 1 ; n <= NumTris ; n++ ) {
       
       fscanf(VSPGeom_File,"%d %d %d %d \n",&DumInt, &Node1,&Node2,&Node3);

       Grid().TriList(n).Node1() = Node1;
       Grid().TriList(n).Node2() = Node2;
       Grid().TriList(n).Node3() = Node3;
       
       if ( Node1 == Node2 || Node1 == Node3 || Node2 == Node3 ) {
          
          printf("wtf... tri: %d --> %d %d %d \n",n,Node1,Node2,Node3);fflush(NULL);exit(1);
          
       }
       
    }    

    // Set surface type
    
    Grid().SurfaceType() = VSPGEOM_SURFACE;
    
    // Kutta nodes... 

    IsLiftingSurface_ = 0;
    
    Grid().SizeKuttaNodeList(0);    

    // Adjust geometry for ground effects analysis
    
    if ( DoGroundEffectsAnalysis_ ) RotateGeometry_About_Y_Axis();
    
    // Read in the surface UV data
    
    SurfaceIsUsed = new int[NumTris + 1];
    
    zero_int_array(SurfaceIsUsed, NumTris);
            
    for ( n = 1 ; n <= NumTris ; n++ ) {
       
       fscanf(VSPGeom_File,"%d %lf %lf %lf %lf %lf %lf \n",&SurfaceID,&u1,&v1,&u2,&v2,&u3,&v3);
     
       Grid().TriList(n).U_Node(1) = u1; Grid().TriList(n).V_Node(1) = v1;
       Grid().TriList(n).U_Node(2) = u2; Grid().TriList(n).V_Node(2) = v2;
       Grid().TriList(n).U_Node(3) = u3; Grid().TriList(n).V_Node(3) = v3;
       
       Grid().TriList(n).SurfaceID()         = SurfaceID;
        
       Grid().TriList(n).ComponentID()       = SurfaceID;
       
       Grid().TriList(n).GeomID()            = SurfaceID;
        
       Grid().TriList(n).SurfaceType()       = VSPGEOM_SURFACE;
       
       Grid().TriList(n).DegenBodyID()       = 0;
       
       Grid().TriList(n).DegenWingID()       = 0;  
       
       Grid().TriList(n).SpanStation()       = 0;

       Grid().TriList(n).IsTrailingEdgeTri() = 0;
       
       Grid().TriList(n).IsLeadingEdgeTri()  = 0;
       
       SurfaceIsUsed[SurfaceID] = 1;
   
    }     
    
    // Read in the number of coincident points
    
 // deprecated    fscanf(VSPGeom_File,"%d",&NumColinearPoints);
  
    // Read in the number of kutta lists
    
    NumKuttaNodes = 0;
    
    KuttaNodeList = new int[NumNodes + 1];
    
    fscanf(VSPGeom_File,"%d",&NumKuttaNodeLists);

    for ( j = 1 ; j <= NumKuttaNodeLists ; j++ ) {
    
       // Read in the Kutta nodes in this list
       
       fscanf(VSPGeom_File,"%d",&NumNodesInList);
       
       for ( i = 1 ; i <= NumNodesInList ; i++ ) {
          
          fscanf(VSPGeom_File,"%d",&(KuttaNodeList[++NumKuttaNodes]));

       }
       
    }
    
    PRINTF("VSPGEOM defined NumKuttaNodes: %d \n",NumKuttaNodes);

    // Determine the number of surfaces
    
    NumberOfSurfacePatches_ = 0;
    
    SurfaceList = new int[NumTris + 1];
    
    zero_int_array(SurfaceList, NumTris);
 
    for ( n = 1 ; n <= NumTris ; n++ ) {
       
       SurfaceID = Grid().TriList(n).SurfaceID();
     
       i = 1;
       
       Found = 0;
       
       while ( !Found && i <= NumberOfSurfacePatches_ ) {
          
          if ( SurfaceList[i] == SurfaceID ) Found = 1;
          
          i++;
          
       }
       
       if ( !Found ) {
          
          NumberOfSurfacePatches_++;
          
          SurfaceList[NumberOfSurfacePatches_] = SurfaceID;
          
       }
       
    }
    
    SurfacePatchNameList_ = new char*[NumberOfSurfacePatches_ + 1];
    
    BoundaryConditionForSurface_ = new BOUNDARY_CONDITION_DATA[NumberOfSurfacePatches_ + 2];
    
    ComponentIDForSurfacePatch_ = new int[NumberOfSurfacePatches_ + 1];
    
    PRINTF("Found %d VSPGEOM Surfaces \n",NumberOfSurfacePatches_);
    
    if ( VKEY_File != NULL ) {
       
       // Read in the vkey data
       
       fgets(DumChar,2000,VKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);
       fgets(DumChar,2000,VKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);
       fgets(DumChar,2000,VKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);
       
       sscanf(DumChar,"%d\n",&NumberOfVSPSurfaces);
       
       printf("NumberOfVSPSurfaces: %d \n",NumberOfVSPSurfaces);
       
     // if ( DumInt != NumberOfSurfacePatches_ ) {
     //    
     //    printf("Error... number of surfaces in .vspgeom and .vkey files do not match! \n");
     //    fflush(NULL);exit(1);
     //    
     // }
       
       
       fgets(DumChar,2000,VKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);
       fgets(DumChar,2000,VKEY_File);printf("DumChar: %s \n",DumChar); fflush(NULL);
      
       k = 0;
       
       for ( n = 1 ; n <= NumberOfVSPSurfaces ; n++ ) {
          
          fgets(DumChar,2000,VKEY_File);printf("n: %d --> DumChar: %s \n",n,DumChar); fflush(NULL);
          
        //  sscanf(DumChar,"%d,%d,%d,%s,%s",&DumInt,&CompID,&DumInt2,DumChar2,DumChar);

          Next = strtok(DumChar,Comma);

          DumInt = atoi(Next);
          
          Next = strtok(NULL,Comma);
          
          CompID = atoi(Next);
          
          Next = strtok(NULL,Comma);

          Next = strtok(NULL,Comma);  Next[strcspn(Next, "\n")] = 0;
          
          if ( SurfaceIsUsed[DumInt] ) {

             printf("Surface: %d exists in tringulation and has OpenVSP Name: %s \n",DumInt,DumChar);
             
             k++;
                          
             printf("CompID: %d \n",CompID);
             
             ComponentIDForSurfacePatch_[DumInt] = CompID + 1;
             
             SurfacePatchNameList_[k] = new char[2000];
             
             sprintf(SurfacePatchNameList_[k],"%s",Next);    
                          
          }      
          
       }
      
    }

    else {
       
       for ( n = 1 ; n <= NumberOfSurfacePatches_ ; n++ ) {
          
          ComponentIDForSurfacePatch_[n] = n;
          
          SurfacePatchNameList_[n] = new char[2000];
          
          sprintf(SurfacePatchNameList_[n],"Surface_%d",n);
          
       }
       
    }
           
    // Renumber the surfaces
   
    for ( n = 1 ; n <= NumTris ; n++ ) {
 
       Done = 0;
       
       i = 1;
       
       while ( !Done && i <= NumberOfSurfacePatches_) {
          
          if ( Grid().TriList(n).SurfaceID() == SurfaceList[i] ) {
             
             Done = 1;
             
             if ( VKEY_File != NULL ) {
                
                Grid().TriList(n).ComponentID() = ComponentIDForSurfacePatch_[Grid().TriList(n).SurfaceID()];
                
             }
             
             else {
                
                Grid().TriList(n).ComponentID() = i;
                
             }
             
             Grid().TriList(n).SurfaceID() = i;

          }
          
          i++;
          
       }
       
       if ( !Done ) {
          
          PRINTF("Error in determing number of surfaces in VSPGEOM file! \n");
          fflush(NULL);
          exit(1);
          
       }
       
    }

    delete [] SurfaceList;
        
    // Do an RCM sort of the tris
 
    //DoRCMSort();
    
    // re-Calculate tri normals

    Grid().CalculateTriNormalsAndCentroids();

    // Build edge data structures
   
    Grid().CreateTriEdges();
    
    Grid().CalculateUpwindEdges();  
    
    // Now find sharp trailing edges

    FindSharpEdges(NumKuttaNodes,KuttaNodeList);
    
    delete [] KuttaNodeList;

}

/*##############################################################################
#                                                                              #
#                            VSP_SURFACE DoRCMSort                             #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::DoRCMSort(void)
{

    int i, j, k, l, Tri, Node, iMin, NumberOfTris, NumberOfNodes, Done;
    int *TriPermArray, *NodePermArray, *NextTri, *TriOnFront, NumberOfTrisOnFront;
    VSPAERO_DOUBLE XMin, Xc;
    VSP_LOOP *TempTriList;
    VSP_NODE *TempNodeList;
    
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
    
    // Do a CM sort
    
    TriPermArray = new int[Grid().NumberOfTris() + 1];
    
    TriOnFront = new int[Grid().NumberOfTris() + 1];
    
    NextTri = new int[Grid().NumberOfTris() + 1];
    
    zero_int_array(TriPermArray, Grid().NumberOfTris());
    
    zero_int_array(TriOnFront, Grid().NumberOfTris());
    
    zero_int_array(NextTri, Grid().NumberOfTris());
    
    NodePermArray = new int[Grid().NumberOfNodes() + 1];
    
    zero_int_array(NodePermArray, Grid().NumberOfNodes());
    
    // Find left most tri in x
    
    XMin = 1.e9;
    
    for ( i = 1 ; i <= Grid().NumberOfTris() ; i++ ) {
       
       Xc = ( Grid().NodeList(Grid().TriList(i).Node1()).x() +
              Grid().NodeList(Grid().TriList(i).Node2()).x() +
              Grid().NodeList(Grid().TriList(i).Node3()).x() )/3.;
       
       if ( Xc <= XMin ) {
          
          XMin = Xc;
          
          iMin = i;
          
       }
       
    }

    i = iMin = Grid().TriList(iMin).Node1();

    NextTri[1] = NodeToTriList_[i][1];
 
    TriOnFront[NextTri[1]] = 1;
    
    NumberOfTrisOnFront = 1;
    
    for ( j = 2 ; j <= NumberOfTrisForNode_[i] ; j++ ) {
       
       Tri = NodeToTriList_[i][j];
       
       if ( Grid().TriList(Tri).SurfaceID() == Grid().TriList(NextTri[1]).SurfaceID() ) {
    
          NextTri[++NumberOfTrisOnFront] = Tri;

          TriOnFront[Tri] = 1;
          
       }
       
    }

    PRINTF("Starting RCM sort \n");fflush(NULL);

    NumberOfTris = NumberOfNodes = 0;
    
    Done = 0;
    
    i = 1;

    while ( !Done ) {
       
       if ( i <= NumberOfTrisOnFront ) {
          
          j = NextTri[i];

          // Next tri in perm list
          
          TriPermArray[++NumberOfTris] = j;
          
          // Add any new tris bounding this tri to the list to check
          
          for ( k = 1 ; k <= 3 ; k++ ) {
             
             Node = Grid().LoopList(j).Node(k);

             if ( NodePermArray[Node] == 0 ) NodePermArray[Node] = ++NumberOfNodes;
             
              for ( l = 1 ; l <= NumberOfTrisForNode_[Node] ; l++ ) {
                
                Tri = NodeToTriList_[Node][l];
                
                if ( TriOnFront[Tri] == 0 && Grid().TriList(Tri).SurfaceID() == Grid().TriList(j).SurfaceID() ) { 
                 
                   TriOnFront[Tri] = 1;
                   
                   NextTri[++NumberOfTrisOnFront] = Tri;                   
                   
                }
                
             }
             
          }
          
          i++;
          
       }
       
       else {

          XMin = 1.e9;
          
          iMin = 0;
          
          for ( k = 1 ; k <= Grid().NumberOfTris() ; k++ ) {
             
             if ( TriOnFront[k] == 0 ) {
                
                Xc = ( Grid().NodeList(Grid().TriList(k).Node1()).x() +
                       Grid().NodeList(Grid().TriList(k).Node2()).x() +
                       Grid().NodeList(Grid().TriList(k).Node3()).x() )/3.;
                 
                if ( Xc <= XMin ) {
                   
                   XMin = Xc;
                   
                   iMin = k;
                   
                }    
                
             }
             
          }      
          
          if ( iMin > 0 ) {
      
             NextTri[++NumberOfTrisOnFront] = iMin;   
             
             TriOnFront[iMin] = 1;
             
          }
          
          else {
             
             PRINTF("wtf! \n");
             fflush(NULL);
             exit(1);
             
          }
          
       }
       
       if ( NumberOfTris == Grid().NumberOfTris() ) Done = 1;
    
    }
    PRINTF("Sorted %d nodes out of %d \n",NumberOfNodes,Grid().NumberOfNodes());
    PRINTF("Sorted %d tris out of %d \n",NumberOfTris,Grid().NumberOfTris());

    // Re-order the tris
    
    TempTriList = new VSP_LOOP[Grid().NumberOfTris() + 1];
    
    for ( i = 1 ; i <= Grid().NumberOfTris() ; i++ ) {
       
       TempTriList[i].SizeNodeList(3);
       
    }

    for ( i = 1 ; i <= Grid().NumberOfTris() ; i++ ) {
       
       j = TriPermArray[i];

       TempTriList[i].Node1() = NodePermArray[Grid().TriList(j).Node1()];
       TempTriList[i].Node2() = NodePermArray[Grid().TriList(j).Node2()];
       TempTriList[i].Node3() = NodePermArray[Grid().TriList(j).Node3()];
 
       TempTriList[i].SurfaceID()   = Grid().TriList(j).SurfaceID();        
       TempTriList[i].SurfaceType() = Grid().TriList(j).SurfaceType();       
       TempTriList[i].SpanStation() = Grid().TriList(j).SpanStation();
       TempTriList[i].ComponentID() = Grid().TriList(j).ComponentID();

    }   

    for ( i = 1 ; i <= Grid().NumberOfTris() ; i++ ) {

       Grid().TriList(i).Node1() = TempTriList[i].Node1();
       Grid().TriList(i).Node2() = TempTriList[i].Node2();
       Grid().TriList(i).Node3() = TempTriList[i].Node3();

       Grid().TriList(i).SurfaceID()   = TempTriList[i].SurfaceID(); 
       Grid().TriList(i).SurfaceType() = TempTriList[i].SurfaceType();       
       Grid().TriList(i).SpanStation() = TempTriList[i].SpanStation();
       Grid().TriList(i).ComponentID() = TempTriList[i].ComponentID();
  
    }  
       
    // Re-order the nodes
   
    TempNodeList = new VSP_NODE[Grid().NumberOfNodes() + 1];
    
    for ( i = 1 ; i <= Grid().NumberOfNodes() ; i++ ) {

       j = NodePermArray[i];
      
       TempNodeList[j].x() = Grid().NodeList(i).x();
       TempNodeList[j].y() = Grid().NodeList(i).y();
       TempNodeList[j].z() = Grid().NodeList(i).z();
    
       TempNodeList[j].IsTrailingEdgeNode()   = Grid().NodeList(i).IsTrailingEdgeNode();
       TempNodeList[j].IsLeadingEdgeNode()    = Grid().NodeList(i).IsLeadingEdgeNode();
       TempNodeList[j].IsBoundaryEdgeNode()   = Grid().NodeList(i).IsBoundaryEdgeNode();
       TempNodeList[j].IsBoundaryCornerNode() = Grid().NodeList(i).IsBoundaryCornerNode();
         
    }     

    for ( i = 1 ; i <= Grid().NumberOfNodes() ; i++ ) {

       Grid().NodeList(i).x() = TempNodeList[i].x();
       Grid().NodeList(i).y() = TempNodeList[i].y();
       Grid().NodeList(i).z() = TempNodeList[i].z();
    
       Grid().NodeList(i).IsTrailingEdgeNode()   = TempNodeList[i].IsTrailingEdgeNode();
       Grid().NodeList(i).IsLeadingEdgeNode()    = TempNodeList[i].IsLeadingEdgeNode();
       Grid().NodeList(i).IsBoundaryEdgeNode()   = TempNodeList[i].IsBoundaryEdgeNode();
       Grid().NodeList(i).IsBoundaryCornerNode() = TempNodeList[i].IsBoundaryCornerNode();
         
    }     
      
    for ( i = 1 ; i <= Grid().NumberOfNodes() ; i++ ) {
       
       delete [] NodeToTriList_[i];
       
       delete [] NodeToEdgeList_[i];
       
    }
                       
    delete [] NodeToTriList_;
    delete [] NumberOfTrisForNode_;
    
                       
    delete [] NodeToEdgeList_;
    delete [] NumberOfEdgesForNode_;
        
    delete [] TriOnFront;
    delete [] NextTri;
    
    delete [] TempTriList;
    delete [] TempNodeList;

}

/*##############################################################################
#                                                                              #
#                          VSP_SURFACE FindSharpEdges                          #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::FindSharpEdges(int NumberOfSharpNodes, int *SharpNodeList)
{

    int i, j, k, p, Loop1, Loop2, Node, Node1, Node2, Node3, Done;
    int NumberOfKuttaEdges, NumberOfKuttaNodes, Hits, Tri;
    int Edge, Edge1, Edge2, Next, VortexSheet, SheetIsPeriodic;
    int *KuttaEdgeList, *IncidentKuttaEdges, *IsKuttaEdge, *PermArray, *NodeUsed;
    int *NodeIsSharp, CurrentComponentID;
    VSPAERO_DOUBLE vec1[3], vec2[3], vec3[3], Xvec[3], dot, angle, mag1, mag2, S1, S2, Ratio;
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
    
    // Create node to edge list
    
    int **NodeToEdgeList_;
    int  *NumberOfEdgesForNode_;
        

    NumberOfEdgesForNode_ = new int[Grid().NumberOfNodes() + 1];
    
    zero_int_array(NumberOfEdgesForNode_, Grid().NumberOfNodes());
    
    for ( i = 1 ; i <= Grid().NumberOfEdges() ; i++ ) {
    
       NumberOfEdgesForNode_[Grid().EdgeList(i).Node1()]++;
       NumberOfEdgesForNode_[Grid().EdgeList(i).Node2()]++;

    }
   
    NodeToEdgeList_ = new int*[Grid().NumberOfNodes() + 1];
    
    for ( i = 1 ; i <= Grid().NumberOfNodes() ; i++ ) {
       
       NodeToEdgeList_[i] = new int[NumberOfEdgesForNode_[i] + 1];
       
    }

    zero_int_array(NumberOfEdgesForNode_, Grid().NumberOfNodes());
    
    for ( i = 1 ; i <= Grid().NumberOfTris() ; i++ ) {

       Node = Grid().EdgeList(i).Node1();
       
       NumberOfEdgesForNode_[Node]++;
       
       NodeToEdgeList_[Node][NumberOfEdgesForNode_[Node]] = i;
       
       Node = Grid().EdgeList(i).Node2();
       
       NumberOfEdgesForNode_[Node]++;
       
       NodeToEdgeList_[Node][NumberOfEdgesForNode_[Node]] = i;

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
    
    // Get rid of short edges at the root or tip
//DJK comment all this out!    
//    for ( i = 1 ; i <= Grid().NumberOfNodes() ; i++ ) {
//       
//       if ( IncidentKuttaEdges[i] == 1 ) {
//              
//          for ( j = 1 ; j <= NumberOfEdgesForNode_[i] ; j++ ) {
//             
//             Edge1 = NodeToEdgeList_[i][j];
//             
//             if ( IsKuttaEdge[Edge1] ) {
//
//                Node1 = Grid().EdgeList(Edge1).Node1();
//                Node2 = Grid().EdgeList(Edge1).Node2();
//                
//                vec1[0] = Grid().NodeList(Node1).x() - Grid().NodeList(Node2).x();
//                vec1[1] = Grid().NodeList(Node1).y() - Grid().NodeList(Node2).y();
//                vec1[2] = Grid().NodeList(Node1).z() - Grid().NodeList(Node2).z();
//                
//                S1 = sqrt(vector_dot(vec1,vec1));
//                
//                vec1[0] /= S1;
//                vec1[1] /= S1;
//                vec1[2] /= S1;
//             
//                for ( k = 1 ; k <= NumberOfEdgesForNode_[i] ; k++ ) {
//                
//                   Edge2 = NodeToEdgeList_[i][k];
//                   
//                   if ( Edge2 != Edge1 ) {
//            
//                      Node1 = Grid().EdgeList(Edge2).Node1();
//                      Node2 = Grid().EdgeList(Edge2).Node2();
//                      
//                      vec2[0] = Grid().NodeList(Node1).x() - Grid().NodeList(Node2).x();
//                      vec2[1] = Grid().NodeList(Node1).y() - Grid().NodeList(Node2).y();
//                      vec2[2] = Grid().NodeList(Node1).z() - Grid().NodeList(Node2).z();
//                      
//                      S2 = sqrt(vector_dot(vec2,vec2));
//                      
//                      vec2[0] /= S2;
//                      vec2[1] /= S2;
//                      vec2[2] /= S2;
//                      
//                      dot = vector_dot(vec1,vec2);
//                   
//                      if ( ABS(dot) > 0.99 ) {
//               
//                         if ( S2/S1 <= 0.25 && IsKuttaEdge[Edge2] ) {
//
//                            IsKuttaEdge[Edge1] = 0;
//                            
//                         }
//                         
//                      }
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
          
       }
      
    }
    
    // Iterate until all the kutta nodes are assigned to a vortex sheet
    
    NodeUsed = new int[Grid().NumberOfNodes() + 1];

    zero_int_array(NodeUsed, Grid().NumberOfNodes());
    
    PermArray = new int[NumberOfKuttaNodes + 1];
  
    p = 0;
    
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
       
       if ( Done ) PRINTF("Found a starting wake sheet node... \n");fflush(NULL);
       
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
          
          if ( SheetIsPeriodic) PRINTF("Sheet is periodic... \n");fflush(NULL);
          
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
          
          if ( Edge1 > 0 && KuttaNodeList[Node1].IsKuttaNode && NodeUsed[Node1] == 0 && Grid().NodeList(Node1).ComponentID() == CurrentComponentID ) {
            
             PermArray[++Next] = Node1;
             
             NodeUsed[Node1] = 1;
             
             Done = 0;
             
          }
          
          // Add node 2
          
          else if ( Edge2 > 0 && KuttaNodeList[Node2].IsKuttaNode && NodeUsed[Node2] == 0 && Grid().NodeList(Node2).ComponentID() == CurrentComponentID ) {
            
             PermArray[++Next] = Node2;
             
             NodeUsed[Node2] = 1;
             
             Done = 0;
                      
          }  
          
         // Catch root and tips...
         
       // // Add node 1
       // 
       // if ( Edge1 > 0 && KuttaNodeList[Node1].IsKuttaNode && NodeUsed[Node1] == 0 && Grid().NodeList(Node1).ComponentID() != CurrentComponentID ) {
       //
       //    PRINTF("Warning... resetting node %d from component ID: %d to compononent ID: %d ... as it lies on a wake line intersection \n",
       //           Grid().NodeList(Node1).ComponentID(),
       //           CurrentComponentID);
       //                
       //    PermArray[++Next] = Node1;
       //    
       //    NodeUsed[Node1] = 1;
       //    
       //    Grid().NodeList(Node1).ComponentID() = CurrentComponentID;
       //    
       //    Done = 0;
       //    
       // }
       // 
       // // Add node 2
       // 
       // else if ( Edge2 > 0 && KuttaNodeList[Node2].IsKuttaNode && NodeUsed[Node2] == 0 && Grid().NodeList(Node2).ComponentID() != CurrentComponentID ) {
       //
       //    PRINTF("Warning... resetting node %d from component ID: %d to compononent ID: %d ... as it lies on a wake line intersection \n",
       //           Grid().NodeList(Node1).ComponentID(),
       //           CurrentComponentID);
       //    
       //    PermArray[++Next] = Node2;
       //    
       //    NodeUsed[Node2] = 1;
       //    
       //    Grid().NodeList(Node2).ComponentID() = CurrentComponentID;
       //    
       //    Done = 0;
       //             
       // }            
   
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

       PRINTF("VortexSheet: %d finished... and %d out of %d kutta nodes are now allocated \n",VortexSheet,p,NumberOfKuttaNodes);fflush(NULL);
       
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

       if ( angle >= 45. && !Grid().EdgeList(i).IsTrailingEdge() ) {
          
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
           
    delete [] KuttaNodeList;
    delete [] KuttaEdgeList;
    
    delete [] NodeToTriList_;
    delete [] NumberOfTrisForNode_;
    
    delete [] IsKuttaEdge;
    delete [] IncidentKuttaEdges;
    
}

/*##############################################################################
#                                                                              #
#                         VSP_SURFACE SurfaceAtNodeIsConvex                    #
#                                                                              #
##############################################################################*/

int VSP_SURFACE::SurfaceAtNodeIsConvex(int Node)
{

    int i, j, k, Tri;
    VSPAERO_DOUBLE Vec[3], VecAvg[3], P1[3], P2[3], Dot, Angle;
   
    if ( NumberOfTrisForNode_[Node] == 2 ) return 1;
    
    // Calculate averaged normal at Node
    
    VecAvg[0] = VecAvg[1] = VecAvg[2] = 0.;
    
    for ( i = 1 ; i <= NumberOfTrisForNode_[Node] ; i++ ) {
    
       Tri = NodeToTriList_[Node][i];
       
       VecAvg[0] += Grid().LoopList(Tri).Normal()[0]*Grid().LoopList(Tri).Area();
       VecAvg[1] += Grid().LoopList(Tri).Normal()[1]*Grid().LoopList(Tri).Area();
       VecAvg[2] += Grid().LoopList(Tri).Normal()[2]*Grid().LoopList(Tri).Area();
       
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
#                        VSP_SURFACE GetComponentBBox                          #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::GetComponentBBox(FILE *VSP_Degen_File, BBOX &ComponentBBox)
{
   
    int i, j, NumI, NumJ;
    VSPAERO_DOUBLE x, y, z, u, v;
    char DumChar[2000], Stuff[2000];

    ComponentBBox.x_min =  1.e9;
    ComponentBBox.x_max = -1.e9;

    ComponentBBox.y_min =  1.e9;
    ComponentBBox.y_max = -1.e9;
    
    ComponentBBox.z_min =  1.e9;
    ComponentBBox.z_max = -1.e9;
        
    // Read in the wing data
    
    fgets(DumChar,1000,VSP_Degen_File);
    fgets(DumChar,1000,VSP_Degen_File); sscanf(DumChar,"SURFACE_NODE,%d,%d",&NumI,&NumJ);
    fgets(DumChar,1000,VSP_Degen_File);

    for ( i = 1 ; i <= NumI ; i++ ) {
     
       for ( j = 1 ; j <= NumJ ; j++ ) {
        
          fgets(DumChar,1000,VSP_Degen_File);  
          
          sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %s",
                 &x,
                 &y,
                 &z,
                 &u,
                 &v,
                 Stuff);

          ComponentBBox.x_min = MIN(ComponentBBox.x_min, x);
          ComponentBBox.x_max = MAX(ComponentBBox.x_max, x);
          
          ComponentBBox.y_min = MIN(ComponentBBox.y_min, y);
          ComponentBBox.y_max = MAX(ComponentBBox.y_max, y);
          
          ComponentBBox.z_min = MIN(ComponentBBox.z_min, z);
          ComponentBBox.z_max = MAX(ComponentBBox.z_max, z);
                    
       }
       
    }   

}

/*##############################################################################
#                                                                              #
#                          VSP_SURFACE ReadWingDataFromFile                    #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::ReadWingDataFromFile(char *Name, FILE *VSP_Degen_File)
{
 
    int i, j, NumI, NumJ, Wing, Done, SubSurfIsTyped, HingeNode[2], DumInt;
    int NumberOfControlSurfaceNodes;
    int i1, i2, i3, i4, j1, j2, j3, j4, FlipNormals;    
    VSPAERO_DOUBLE DumFloat, Vec[3], VecQC_1[3], VecQC_2[3], Mag, HingeVec[3];
    VSPAERO_DOUBLE x1, y1, z1, x2, y2, z2, ArcLength[2], Chord, up, wp, xyz[3];
    VSPAERO_DOUBLE u1, u2, w1, w2, ulist[4], wlist[4];
    VSPAERO_DOUBLE Vec1[3], Vec2[3], Vec3[3], Vec4[3], normal[3], Temp[5], CellArea;
    
    char DumChar[4000], Stuff[4000], LastSubSurf[4000], Comma[4000], *Next;
    
    // Save the component name
    
    SPRINTF (ComponentName_,"%s",Name);
    
    // Zero case
    
    Case_ = 0;
        
    // Set surface type
    
    SurfaceType_ = DEGEN_WING_SURFACE;
   
    // Read in the wing data
    
    // fgets(DumChar,1000,VSP_Degen_File);
    // fgets(DumChar,1000,VSP_Degen_File); 
    // fgets(DumChar,1000,VSP_Degen_File);
    
    Done = 0;
    
    while ( !Done ) {
       
       fgets(DumChar,1000,VSP_Degen_File);
   
       if ( strstr(DumChar,"SURFACE_NODE") != NULL ) {
          
          sscanf(DumChar,"SURFACE_NODE,%d,%d",&NumI,&NumJ);
          
          Done = 1;
          
       }
       
    }
    
    fgets(DumChar,1000,VSP_Degen_File);
 
    if ( Verbose_ ) PRINTF("NumI, NumJ: %d %d \n",NumI,NumJ);

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
            
           if ( Verbose_ ) PRINTF("xyz: %lf %lf %lf %lf %lf \n",
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
    
    if ( Verbose_ ) PRINTF("NumI, NumJ: %d %d \n",NumI,NumJ);
    
    SizeFlatPlateLists(NumI,NumJ);
    
    // Read in the flate plate Normal information
    
    fgets(DumChar,1000,VSP_Degen_File);  

    if ( Verbose_ ) PRINTF("DumChar: %s \n",DumChar);
    
    for ( i = 1 ; i <= NumI ; i++ ) {
       
       fgets(DumChar,1000,VSP_Degen_File);  
       
       if ( Verbose_ ) PRINTF("Normals: %s \n",DumChar);
       
       sscanf(DumChar,"%lf,%lf,%lf \n",
              &(Nx_FlatPlateNormal_[i]),
              &(Ny_FlatPlateNormal_[i]),
              &(Nz_FlatPlateNormal_[i]));
          
    }
             
    fgets(DumChar,1000,VSP_Degen_File);  
     
    // Now read in the flat plate representation of the wing
    
    for ( i = 1 ; i <= NumI ; i++ ) {
     
       for ( j = 1 ; j <= NumJ ; j++ ) {
        
          fgets(DumChar,1000,VSP_Degen_File);  

          // Use camber line z value to define the surface, instead of linearizing
          // the tangential BC about the flat plate model
          
          sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf",
                 &x_plate(i,j),
                 &y_plate(i,j),
                 &z_plate(i,j),
                 &Camber(i,j),
                 &DumFloat,
                 &Nx_Camber(i,j),
                 &Ny_Camber(i,j),
                 &Nz_Camber(i,j),
                 &u_plate(i,j),
                 &DumFloat,
                 &v_plate(i,j));
         
          x_plate(i,j) += Camber(i,j) * Nx_Camber(i,j);
          y_plate(i,j) += Camber(i,j) * Ny_Camber(i,j);
          z_plate(i,j) += Camber(i,j) * Nz_Camber(i,j);
                      
          if ( Verbose_ ) PRINTF("Plate xyz: %lf %lf %lf %lf %lf %lf %s \n",
                                  x_plate(i,j),
                                  y_plate(i,j),
                                  z_plate(i,j),
                                  Nx_Camber(i,j),
                                  Ny_Camber(i,j),
                                  Nz_Camber(i,j),
                                  Stuff);
          
       }
       
       // Fudge the first and last normals...
       
       Nx_Camber(i,1) = Nx_Camber(i,2);
       Ny_Camber(i,1) = Ny_Camber(i,2);
       Nz_Camber(i,1) = Nz_Camber(i,2);
       
       Nx_Camber(i,NumJ) = Nx_Camber(i,NumJ-1);
       Ny_Camber(i,NumJ) = Ny_Camber(i,NumJ-1);
       Nz_Camber(i,NumJ) = Nz_Camber(i,NumJ-1);
       
    }    
    
    // Calculate wire frame geometry surface normal
    
    VSPAERO_DOUBLE Volume = 0.;
    VSPAERO_DOUBLE TotalArea = 0.;
    VSPAERO_DOUBLE Zavg;
 
    // Normal vector based on first flat plate normal definition, used to
    // test for surface orientation
    
    i = j = 1;
    
    normal[0] = ( Nx_Camber(i,j) + Nx_Camber(i+1,j) + Nx_Camber(i+1,j+1) ) / 3.;
    normal[1] = ( Ny_Camber(i,j) + Ny_Camber(i+1,j) + Ny_Camber(i+1,j+1) ) / 3.;
    normal[2] = ( Nz_Camber(i,j) + Nz_Camber(i+1,j) + Nz_Camber(i+1,j+1) ) / 3.;
    
    for ( i = 1 ; i <= NumGeomI_ ; i++ ) {
     
       for ( j = 1 ; j <= NumGeomJ_ ; j++ ) {
          
          Nx(i,j) = 0.;
          Ny(i,j) = 0.;
          Nz(i,j) = 0.;
          
          Area(i,j) = 0.;
                   
       }
       
    }
               
    FlipNormals = 0;
           
    for ( i = 1 ; i < NumGeomI_ ; i++ ) {
     
       for ( j = 1 ; j < NumGeomJ_ ; j++ ) {
          
          i1 = i;         j1 = j;
          i2 = i    ;     j2 = j + 1;
          i3 = i + 1;     j3 = j + 1;
          i4 = i + 1;     j4 = j;
                    
          // Tri 1
          
          Vec1[0] = x(i2,j2) - x(i1,j1);
          Vec1[1] = y(i2,j2) - y(i1,j1);
          Vec1[2] = z(i2,j2) - z(i1,j1);

          Vec2[0] = x(i3,j3) - x(i1,j1);
          Vec2[1] = y(i3,j3) - y(i1,j1);
          Vec2[2] = z(i3,j3) - z(i1,j1);
                              
          vector_cross(Vec1,Vec2,Vec3);
          
          CellArea = 0.5*sqrt(vector_dot(Vec3,Vec3));
          
          // Tri 2

          Vec1[0] = x(i3,j3) - x(i1,j1);
          Vec1[1] = y(i3,j3) - y(i1,j1);
          Vec1[2] = z(i3,j3) - z(i1,j1);

          Vec2[0] = x(i4,j4) - x(i1,j1);
          Vec2[1] = y(i4,j4) - y(i1,j1);
          Vec2[2] = z(i4,j4) - z(i1,j1);
                              
          vector_cross(Vec1,Vec2,Vec4);
          
          CellArea += 0.5*sqrt(vector_dot(Vec4,Vec4));

          // Final weighted normal
          
          Vec3[0] += Vec4[0];
          Vec3[1] += Vec4[1];
          Vec3[2] += Vec4[2];
          
          // Distribute normal

          Nx(i1,j1) += Vec3[0];
          Ny(i1,j1) += Vec3[1];
          Nz(i1,j1) += Vec3[2];
                 
          Nx(i2,j2) += Vec3[0];
          Ny(i2,j2) += Vec3[1];
          Nz(i2,j2) += Vec3[2];
                 
          Nx(i3,j3) += Vec3[0];
          Ny(i3,j3) += Vec3[1];
          Nz(i3,j3) += Vec3[2];
                 
          Nx(i4,j4) += Vec3[0];
          Ny(i4,j4) += Vec3[1];
          Nz(i4,j4) += Vec3[2];       
          
          // Distribute Area

          Area(i1,j1) += 0.25 * CellArea;
                  
          Area(i2,j2) += 0.25 * CellArea;
                 
          Area(i3,j3) += 0.25 * CellArea;
                 
          Area(i4,j4) += 0.25 * CellArea;

          Zavg = 0.25*( z(i1,j1) + z(i2,j2) + z(i3,j3) + z(i4,j4) );

          Volume += Nz(i,j) * Zavg * CellArea;
          
          TotalArea += CellArea;
          
          if ( i == 1 && j == 1 ) {
             
             Mag = vector_dot(normal, Vec3);
             
             if ( Mag < 0. ) FlipNormals = 1;

          }
          
          if ( Verbose_ ) PRINTF("Zavg: %f .... Normal: %f %f %f .... CamberNormal: %f %f %f \n",
          Zavg,
          Vec3[0],Vec3[1],Vec3[2],
          normal[0],normal[1],normal[2]);
          
       }
    
    }    

    for ( i = 1 ; i <= NumGeomI_ ; i++ ) {
     
       for ( j = 1 ; j <= NumGeomJ_ ; j++ ) {
          
          Mag = sqrt( Nx(i,j)*Nx(i,j) + Ny(i,j)*Ny(i,j) + Nz(i,j)*Nz(i,j) );
          
          Nx(i,j) /= Mag;
          Ny(i,j) /= Mag;
          Nz(i,j) /= Mag;
   
       }
       
    }
    
    if ( Verbose_ ) PRINTF("FlipNormals: %d \n",FlipNormals);
    if ( Verbose_ ) PRINTF("Wing Area   is: %f \n",TotalArea);
    if ( Verbose_ ) PRINTF("Wing Volume is: %f \n",Volume);

    // Flip surface orientation if needed

    if ( FlipNormals ) {
       
       for ( i = 1 ; i <= NumGeomI_ ; i++ ) {
        
          for ( j = 1 ; j <= NumGeomJ_ ; j++ ) {

             Temp[0] = x(NumGeomI_-i+1,j);
             Temp[1] = y(NumGeomI_-i+1,j);
             Temp[2] = z(NumGeomI_-i+1,j);
             Temp[3] = u(NumGeomI_-i+1,j);
             Temp[4] = v(NumGeomI_-i+1,j);
             
             x(NumGeomI_-i+1,j) = x(i,j);
             y(NumGeomI_-i+1,j) = y(i,j);
             z(NumGeomI_-i+1,j) = z(i,j);
             u(NumGeomI_-i+1,j) = u(i,j);
             v(NumGeomI_-i+1,j) = v(i,j);
             
             x(i,j) = Temp[0];
             y(i,j) = Temp[1];
             z(i,j) = Temp[2];
             u(i,j) = Temp[3];
             v(i,j) = Temp[4];
                          
             Nx(i,j) *= -1.;
             Ny(i,j) *= -1.;
             Nz(i,j) *= -1.;
             
          }
          
       }
       
    }    

    // Adjust geometry for ground effects analysis
    
    if ( DoGroundEffectsAnalysis_ ) RotateGeometry_About_Y_Axis();
    
    // Set number of surface patches... 0
        
    NumberOfSurfacePatches_ = 0;
    
    BoundaryConditionForSurface_ = new BOUNDARY_CONDITION_DATA[NumberOfSurfacePatches_ + 2];
        
    // Skip over data until we find the STICK_NODE data
    
    Done = 0;
    
    while ( !Done ) {
    
       fgets(DumChar,1000,VSP_Degen_File);
       
       if ( strstr(DumChar,"STICK_NODE") != NULL ) Done = 1;
       
    }
    
    fgets(DumChar,1000,VSP_Degen_File);    
   
    VSPAERO_DOUBLE lex;                    // 1
    VSPAERO_DOUBLE ley;                    // 2
    VSPAERO_DOUBLE lez;                    // 3
    VSPAERO_DOUBLE tex;                    // 4
    VSPAERO_DOUBLE tey;                    // 5
    VSPAERO_DOUBLE tez;                    // 6
    VSPAERO_DOUBLE cgShellx;               // 7
    VSPAERO_DOUBLE cgShelly;               // 8
    VSPAERO_DOUBLE cgShellz;               // 9
    VSPAERO_DOUBLE cgSolidx;               // 10
    VSPAERO_DOUBLE cgSolidy;               // 11
    VSPAERO_DOUBLE cgSolidz;               // 12
    VSPAERO_DOUBLE toc;                    // 13
    VSPAERO_DOUBLE tLoc;                   // 14
    VSPAERO_DOUBLE chord;                  // 15   
    VSPAERO_DOUBLE Ishell11;               // 16
    VSPAERO_DOUBLE Ishell22;               // 17
    VSPAERO_DOUBLE Ishell12;               // 18
    VSPAERO_DOUBLE Isolid11;               // 19
    VSPAERO_DOUBLE Isolid22;               // 20
    VSPAERO_DOUBLE Isolid12;               // 21
    VSPAERO_DOUBLE sectArea;               // 22
    VSPAERO_DOUBLE sectNormalx;            // 23
    VSPAERO_DOUBLE sectNormaly;            // 24
    VSPAERO_DOUBLE sectNormalz;            // 25
    VSPAERO_DOUBLE perimTop;               // 26
    VSPAERO_DOUBLE perimBot;               // 27
    VSPAERO_DOUBLE u;                      // 28
    VSPAERO_DOUBLE t00;                    // 29
    VSPAERO_DOUBLE t01;                    // 30
    VSPAERO_DOUBLE t02;                    // 31
    VSPAERO_DOUBLE t03;                    // 32
    VSPAERO_DOUBLE t10;                    // 33
    VSPAERO_DOUBLE t11;                    // 34
    VSPAERO_DOUBLE t12;                    // 35
    VSPAERO_DOUBLE t13;                    // 36
    VSPAERO_DOUBLE t20;                    // 37
    VSPAERO_DOUBLE t21;                    // 38
    VSPAERO_DOUBLE t22;                    // 39
    VSPAERO_DOUBLE t23;                    // 40
    VSPAERO_DOUBLE t30;                    // 41
    VSPAERO_DOUBLE t31;                    // 42
    VSPAERO_DOUBLE t32;                    // 43
    VSPAERO_DOUBLE t33;                    // 44
    VSPAERO_DOUBLE it00;                   // 45
    VSPAERO_DOUBLE it01;                   // 46
    VSPAERO_DOUBLE it02;                   // 47
    VSPAERO_DOUBLE it03;                   // 48
    VSPAERO_DOUBLE it10;                   // 49
    VSPAERO_DOUBLE it11;                   // 50
    VSPAERO_DOUBLE it12;                   // 51
    VSPAERO_DOUBLE it13;                   // 52
    VSPAERO_DOUBLE it20;                   // 53
    VSPAERO_DOUBLE it21;                   // 54
    VSPAERO_DOUBLE it22;                   // 55
    VSPAERO_DOUBLE it23;                   // 56
    VSPAERO_DOUBLE it30;                   // 57
    VSPAERO_DOUBLE it31;                   // 58
    VSPAERO_DOUBLE it32;                   // 59
    VSPAERO_DOUBLE it33;                   // 60
    VSPAERO_DOUBLE toc2;                   // 61
    VSPAERO_DOUBLE tLoc2;                  // 62
    VSPAERO_DOUBLE anglele;                // 63
    VSPAERO_DOUBLE anglete;                // 64
    VSPAERO_DOUBLE radleTop;               // 65
    VSPAERO_DOUBLE radleBot;               // 66

    for ( i = 1 ; i <= NumI; i++ ) {
       
       fgets(DumChar,sizeof(DumChar),VSP_Degen_File);

                     //                                               1                                                 2                                                 3                                                 4                                                 5                                                 6
                     //  1    2    3    4    5    6    7    8    9    0    1    2    3    4    5    6    7    8    9    0    1    2    3    4    5    6    7    8    9    0    1    2    3    4    5    6    7    8    9    0    1    2    3    4    5    6    7    8    9    0    1    2    3    4    5    6    7    8    9    0    1    2    3    4    5    6 
       sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf \n",
          &lex,                    // 1
          &ley,                    // 2
          &lez,                    // 3
          &tex,                    // 4
          &tey,                    // 5
          &tez,                    // 6
          &cgShellx,               // 7
          &cgShelly,               // 8
          &cgShellz,               // 9
          &cgSolidx,               // 10
          &cgSolidy,               // 11
          &cgSolidz,               // 12
          &toc,                    // 13
          &tLoc,                   // 14
          &chord,                  // 15   
          &Ishell11,               // 16
          &Ishell22,               // 17
          &Ishell12,               // 18
          &Isolid11,               // 19
          &Isolid22,               // 20
          &Isolid12,               // 21
          &sectArea,               // 22
          &sectNormalx,            // 23
          &sectNormaly,            // 24
          &sectNormalz,            // 25
          &perimTop,               // 26
          &perimBot,               // 27
          &u,                      // 28
          &t00,                    // 29
          &t01,                    // 30
          &t02,                    // 31
          &t03,                    // 32
          &t10,                    // 33
          &t11,                    // 34
          &t12,                    // 35
          &t13,                    // 36
          &t20,                    // 37
          &t21,                    // 38
          &t22,                    // 39
          &t23,                    // 40
          &t30,                    // 41
          &t31,                    // 42
          &t32,                    // 43
          &t33,                    // 44
          &it00,                   // 45
          &it01,                   // 46
          &it02,                   // 47
          &it03,                   // 48
          &it10,                   // 49
          &it11,                   // 50
          &it12,                   // 51
          &it13,                   // 52
          &it20,                   // 53
          &it21,                   // 54
          &it22,                   // 55
          &it23,                   // 56
          &it30,                   // 57
          &it31,                   // 58
          &it32,                   // 59
          &it33,                   // 60
          &toc2,                   // 61
          &tLoc2,                  // 62
          &anglele,                // 63
          &anglete,                // 64
          &radleTop,               // 65
          &radleBot);              // 66
          
          ThicknessToChord_[i]       = toc;
          LocationOfMaxThickness_[i] = tLoc;
          RadiusToChord_[i]          = 0.5*( radleTop + radleBot);
    
          if ( Verbose_ ) PRINTF("i: %d --> ToC: %f ... ToCLoc: %f ... RadiusToChord_[i]: %f \n",i,ThicknessToChord_[i],LocationOfMaxThickness_[i],RadiusToChord_[i]);
                        
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
                     
    // Check if there is subsurface data

    Done = NumberOfControlSurfaces_ = SubSurfIsTyped = 0;
    
    SPRINTF (LastSubSurf," ");
    
    SPRINTF (Comma,",");
 
    if ( Verbose_ ) PRINTF("Reading in any control surface information... \n");
  
    while ( !Done ) {
       
       fgets(DumChar,1000,VSP_Degen_File);
       
       if ( Verbose_ ) PRINTF("SubSurf Section... 1... DumChar: %s \n",DumChar);
       
       if ( strstr(DumChar,"DegenGeom") != NULL ) {
          
          // Check if this version of vsp wrote out subsurface types...
          
          if ( strstr(DumChar,"typeName") != NULL ) {
             
             SubSurfIsTyped = 1;
             
          }
       
          fgets(DumChar,1000,VSP_Degen_File);
  
          if ( Verbose_ ) PRINTF("SubSurf Section... 2... DumChar: %s \n",DumChar);
  
          if ( strstr(DumChar,"SUBSURF") ) {

             // Check for supported sub-surface types.  This could be made into an if-elseif statement to handle each individually
              
             if ( ( strstr(DumChar,"Control_Surf") || strstr(DumChar,"Rectangle") ) || SubSurfIsTyped == 0 ) {
   
                if ( Verbose_ ) PRINTF("DumChar: %s .... LastSubSurf: %s \n",DumChar,LastSubSurf);
           
                // Skip over possible second instance of this control surface.. it may be defined on both the top and bottom surfaces
                
                if ( strcmp(DumChar,LastSubSurf) != 0 ) {
                   
                   SPRINTF (LastSubSurf,"%s",DumChar);

                   if ( Verbose_ ) PRINTF("Setting LastSubSurf to %s ... so now it is: %s \n",DumChar,LastSubSurf);                   
                
                   Next = strtok(DumChar,Comma); Next[strcspn(Next, "\n")] = 0;
                   
                   Next = strtok(NULL,Comma); Next[strcspn(Next, "\n")] = 0;
                   
                   NumberOfControlSurfaces_++;
                   
                   if ( NumberOfControlSurfaces_ > MaxNumberOfControlSurfaces_ ) {
                      
                      MaxNumberOfControlSurfaces_ *= 1.25;
                      
                      CONTROL_SURFACE *ControlSurface_New = new CONTROL_SURFACE[MaxNumberOfControlSurfaces_ + 1];
                      
                      for ( i = 1 ; i <= NumberOfControlSurfaces_ - 1 ; i++ ) {
                         
                         ControlSurface_New[i] = ControlSurface_[i];
                         
                      }
                      
                      delete [] ControlSurface_;
                      
                      ControlSurface_ = ControlSurface_New;
                      
                   }
                   
                   // Save control surface name
                   
                   SPRINTF (ControlSurface_[NumberOfControlSurfaces_].ShortName(),"%s\0",Next); // Short name
                   SPRINTF (ControlSurface_[NumberOfControlSurfaces_].Name(),"%s\0",Next);      // Name and FullName assumed the same unless we find subsurface information below... 
                   
                   // Save the control surface type name, type, and full name if they exist
                   
                   if ( SubSurfIsTyped ) {
                      
                      Next = strtok(NULL,Comma);
                      SPRINTF (ControlSurface_[NumberOfControlSurfaces_].TypeName(),"%s\0",Next); 
                   
                      Next = strtok(NULL,Comma);
                      sscanf(Next,"%d",&ControlSurface_[NumberOfControlSurfaces_].Type());

                      // Use the unique fullName for the subsurface name if found

                      Next = strtok(NULL,Comma);
                      if ( Next ) sscanf(Next,"%s",ControlSurface_[NumberOfControlSurfaces_].Name());
                                            
                   }
         
                   if ( Verbose_ ) PRINTF( "Control Surface Info:\n" );
                   if ( Verbose_ ) PRINTF( "\t      Name: %s \n", ControlSurface_[NumberOfControlSurfaces_].Name() );
                   if ( Verbose_ ) PRINTF( "\t ShortName: %s \n", ControlSurface_[NumberOfControlSurfaces_].ShortName() );
                   if ( Verbose_ ) PRINTF( "\t  TypeName: %s \n", ControlSurface_[NumberOfControlSurfaces_].TypeName() );
                   if ( Verbose_ ) PRINTF( "\t      Type: %d \n", ControlSurface_[NumberOfControlSurfaces_].Type() );
        
                   fgets(DumChar,1000,VSP_Degen_File);
                   fgets(DumChar,1000,VSP_Degen_File);
                   fgets(DumChar,1000,VSP_Degen_File);
                   fgets(DumChar,1000,VSP_Degen_File);
                   
                   sscanf(DumChar,"%s%d\n",Stuff,&NumberOfControlSurfaceNodes);
         
                   ControlSurface_[NumberOfControlSurfaces_].SizeNodeList(NumberOfControlSurfaceNodes);
                   
                   fgets(DumChar,1000,VSP_Degen_File);
                   
                   ControlSurface_[NumberOfControlSurfaces_].u_min() =  1.e9;
                   ControlSurface_[NumberOfControlSurfaces_].u_max() = -1.e9;
                   ControlSurface_[NumberOfControlSurfaces_].v_min() =  1.e9;
                   ControlSurface_[NumberOfControlSurfaces_].v_max() = -1.e9;
          
                   for ( i = 1 ; i <= NumberOfControlSurfaceNodes ; i++ ) {
                      
                      fgets(DumChar,1000,VSP_Degen_File); sscanf(DumChar,"%lf, %lf",&up,&wp); if ( Verbose_ ) PRINTF("up,wp: %lf %lf \n",up,wp);
                      
                      // If control surface definition is on the upper surface, transform to the lower surface

                      if ( wp > 2. ) wp = 4. - wp;
                      
                      // Store UV coordinates
                      
                      ControlSurface_[NumberOfControlSurfaces_].UV_Node(i)[0] = up;
                      ControlSurface_[NumberOfControlSurfaces_].UV_Node(i)[1] = wp;
                      
                      // Store XYZ coordinates
                      
                      Interpolate_XYZ_From_UV(up,wp,xyz);
                      
                      ControlSurface_[NumberOfControlSurfaces_].XYZ_Node(i)[0] = xyz[0];
                      ControlSurface_[NumberOfControlSurfaces_].XYZ_Node(i)[1] = xyz[1];
                      ControlSurface_[NumberOfControlSurfaces_].XYZ_Node(i)[2] = xyz[2];   
                      
                      // Bounding box for control surface
                      
                      ControlSurface_[NumberOfControlSurfaces_].u_min() = MIN(ControlSurface_[NumberOfControlSurfaces_].u_min(), up);
                      ControlSurface_[NumberOfControlSurfaces_].u_max() = MAX(ControlSurface_[NumberOfControlSurfaces_].u_max(), up);
                      ControlSurface_[NumberOfControlSurfaces_].v_min() = MIN(ControlSurface_[NumberOfControlSurfaces_].v_min(), wp);
                      ControlSurface_[NumberOfControlSurfaces_].v_max() = MAX(ControlSurface_[NumberOfControlSurfaces_].v_max(), wp);
                           
                                    
                   }
                                               
                }
                
                else {

                   fgets(DumChar,1000,VSP_Degen_File);
                   fgets(DumChar,1000,VSP_Degen_File);
                   fgets(DumChar,1000,VSP_Degen_File);
                   fgets(DumChar,1000,VSP_Degen_File);
                   
                   sscanf(DumChar,"%s%d\n",Stuff,&DumInt);

                   fgets(DumChar,1000,VSP_Degen_File);
                   
                   for ( i = 1 ; i <= DumInt ; i++ ) {
                      
                      fgets(DumChar,1000,VSP_Degen_File);
                      
                   }
                                       
                   if ( Verbose_ ) PRINTF("Skipped doubly defined control surface... DumChar: %s \n",DumChar);           
                   
                }
                
             }         

          }
          
          else if ( strstr(DumChar,"HINGELINE") ) {
            
             if ( Verbose_ ) PRINTF("Read in %d control surfaces... \n",NumberOfControlSurfaces_);fflush(NULL);
             
             // Now read in, or calculate the hinge lines
   
             if ( NumberOfControlSurfaces_ > 0 && SubSurfIsTyped ) {
   
                fgets(DumChar,1000,VSP_Degen_File);

                if ( Verbose_ ) PRINTF("Hinge Line Code: DumChar: %s \n",DumChar);
              
                for ( i = 1 ; i <= NumberOfControlSurfaces_ ; i++ ) {
                   
                   // # uStart,uEnd,wStart,wEnd,xStart,yStart,zStart,xEnd,yEnd,zEnd
                   
                   // Updated... just read in xyz values directly... 
                   
                   fgets(DumChar,1000,VSP_Degen_File);

                   sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf",&DumFloat,&DumFloat,&DumFloat,&DumFloat,&x1,&y1,&z1,&x2,&y2,&z2);
                   
                   ControlSurface_[i].HingeNode_1(0) = 0.5*x1;
                   ControlSurface_[i].HingeNode_1(1) = 0.5*y1;
                   ControlSurface_[i].HingeNode_1(2) = 0.5*z1;

                   ControlSurface_[i].HingeNode_2(0) = 0.5*x2;
                   ControlSurface_[i].HingeNode_2(1) = 0.5*y2;
                   ControlSurface_[i].HingeNode_2(2) = 0.5*z2;
                          
                   fgets(DumChar,1000,VSP_Degen_File);
               
                   sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf",&DumFloat,&DumFloat,&DumFloat,&DumFloat,&x1,&y1,&z1,&x2,&y2,&z2);
                   
                   ControlSurface_[i].HingeNode_1(0) += 0.5*x1;
                   ControlSurface_[i].HingeNode_1(1) += 0.5*y1;
                   ControlSurface_[i].HingeNode_1(2) += 0.5*z1;
                                                     
                   ControlSurface_[i].HingeNode_2(0) += 0.5*x2;
                   ControlSurface_[i].HingeNode_2(1) += 0.5*y2;
                   ControlSurface_[i].HingeNode_2(2) += 0.5*z2;           

                   HingeVec[0] = ControlSurface_[i].HingeNode_2(0) - ControlSurface_[i].HingeNode_1(0);
                   HingeVec[1] = ControlSurface_[i].HingeNode_2(1) - ControlSurface_[i].HingeNode_1(1);
                   HingeVec[2] = ControlSurface_[i].HingeNode_2(2) - ControlSurface_[i].HingeNode_1(2);                
                                  
                   Mag = sqrt(vector_dot(HingeVec,HingeVec));
                   
                   HingeVec[0] /= Mag;
                   HingeVec[1] /= Mag;
                   HingeVec[2] /= Mag;        

                   ControlSurface_[i].HingeVec(0) = HingeVec[0];
                   ControlSurface_[i].HingeVec(1) = HingeVec[1];
                   ControlSurface_[i].HingeVec(2) = HingeVec[2];
                                                                     
                   if ( i < NumberOfControlSurfaces_ ) {
                      
                     fgets(DumChar,1000,VSP_Degen_File);
                     fgets(DumChar,1000,VSP_Degen_File);
                     fgets(DumChar,1000,VSP_Degen_File);
                     
                   }
                                  
                }
                
             }
             
             // Calculate hinge line based on control surface definition - it was a simple box
             
             else {
                
                for ( i = 1 ; i <= NumberOfControlSurfaces_ ; i++ ) {
                   
                   // Determine hinge line
                   
                   ulist[0] = ControlSurface_[i].UV_Node(1)[0];
                   wlist[0] = ControlSurface_[i].UV_Node(1)[1];
      
                   ulist[1] = ControlSurface_[i].UV_Node(2)[0];
                   wlist[1] = ControlSurface_[i].UV_Node(2)[1];
      
                   ulist[2] = ControlSurface_[i].UV_Node(3)[0];
                   wlist[2] = ControlSurface_[i].UV_Node(3)[1];
                   
                   ulist[3] = ControlSurface_[i].UV_Node(4)[0];
                   wlist[3] = ControlSurface_[i].UV_Node(4)[1];
                                                                                
                   LocateHingeLine(ulist, wlist, HingeNode);
                                                             
                   // Hinge point 1
                   
                   Interpolate_XYZ_From_UV(ulist[HingeNode[0]],wlist[HingeNode[0]],xyz);
                   
                   HingeVec[0] = -xyz[0];
                   HingeVec[1] = -xyz[1];
                   HingeVec[2] = -xyz[2];
        
                   ControlSurface_[i].HingeNode_1(0) = xyz[0];
                   ControlSurface_[i].HingeNode_1(1) = xyz[1];
                   ControlSurface_[i].HingeNode_1(2) = xyz[2];
             
                   if ( Verbose_ ) PRINTF("Hinge Point 1: %lf %lf %lf \n",xyz[0],xyz[1],xyz[2]);
                   
                   // Hinge point 2
                   
                   Interpolate_XYZ_From_UV(ulist[HingeNode[1]],wlist[HingeNode[1]],xyz);
                   
                   ControlSurface_[i].HingeNode_2(0) = xyz[0];
                   ControlSurface_[i].HingeNode_2(1) = xyz[1];
                   ControlSurface_[i].HingeNode_2(2) = xyz[2];
                   
                   if ( Verbose_ ) PRINTF("Hinge Point 2: %lf %lf %lf \n",xyz[0],xyz[1],xyz[2]);
                   
                   HingeVec[0] += xyz[0];
                   HingeVec[1] += xyz[1];
                   HingeVec[2] += xyz[2];
                                   
                   // Hinge Vector
                   
                   Mag = sqrt(vector_dot(HingeVec,HingeVec));
                   
                   HingeVec[0] /= Mag;
                   HingeVec[1] /= Mag;
                   HingeVec[2] /= Mag;        
      
                   ControlSurface_[i].HingeVec(0) = HingeVec[0];
                   ControlSurface_[i].HingeVec(1) = HingeVec[1];
                   ControlSurface_[i].HingeVec(2) = HingeVec[2];
                   
                }
                   
             }       
                       
          }
          
          else {
             
             PRINTF("Error in VSP_Surface! Unknown control surface type: DumChar: %s \n",DumChar);fflush(NULL);exit(1);
             
          }
 
       }
       
       else {
          
          Done = 1;
          
       }
       
    }
    
    // Check for degenerate span stations
    
    CheckForDegenerateSpanSections();
    
    NumberOfSurfacePatches_ = 0;
    
    // Calculate local chord lengths
    
    AverageChord_ = 0.;
    
    s_[0] = ArcLength[0] = ArcLength[1] = 0.;
 
    for ( i = 1 ; i < NumPlateI_ ; i++ ) {
     
       j = 1;
       
       // Calculate local chord
       
       x1 = 0.5*( x_plate(i,j) + x_plate(i+1,j) );
       y1 = 0.5*( y_plate(i,j) + y_plate(i+1,j) );
       z1 = 0.5*( z_plate(i,j) + z_plate(i+1,j) );
 
       j = NumPlateJ_;
       
       x2 = 0.5*( x_plate(i,j) + x_plate(i+1,j) );
       y2 = 0.5*( y_plate(i,j) + y_plate(i+1,j) );
       z2 = 0.5*( z_plate(i,j) + z_plate(i+1,j) );
       
       LocalChord_[i] = sqrt( pow(x2-x1,2.) + pow(y2-y1,2.) + pow(z2-z1,2.) );
       
       AverageChord_ += LocalChord_[i];
       
       // Keep track of the initial leading and trailing edge
       
       xLE_[i] = x2;
       yLE_[i] = y2;
       zLE_[i] = z2;
       
       xTE_[i] = x1;
       yTE_[i] = y1;
       zTE_[i] = z1;

       // Calculate arclength along the quarter chord
       
       Vec[0] = xTE_[i] - xLE_[i];
       Vec[1] = yTE_[i] - yLE_[i];
       Vec[2] = zTE_[i] - zLE_[i];

       Vec[0] /= LocalChord_[i];
       Vec[1] /= LocalChord_[i];
       Vec[2] /= LocalChord_[i];
       
       VecQC_2[0] = xLE_[i] + 0.25*LocalChord_[i]*Vec[0];
       VecQC_2[1] = yLE_[i] + 0.25*LocalChord_[i]*Vec[1];
       VecQC_2[2] = zLE_[i] + 0.25*LocalChord_[i]*Vec[2];
       
       if ( i == 1 ) {

          VecQC_1[0] = x_plate(i,j) + 0.25*(x_plate(i,NumPlateJ_) - x_plate(i,j));     
          VecQC_1[1] = y_plate(i,j) + 0.25*(y_plate(i,NumPlateJ_) - y_plate(i,j));        
          VecQC_1[2] = z_plate(i,j) + 0.25*(z_plate(i,NumPlateJ_) - z_plate(i,j));        
          
          ArcLength[1] = 0.5*sqrt( pow(VecQC_2[0] - VecQC_1[0],2.)
                                 + pow(VecQC_2[1] - VecQC_1[1],2.)
                                 + pow(VecQC_2[2] - VecQC_1[2],2.) );        
          
       }
       
       else {

          ArcLength[0] = ArcLength[1];

          ArcLength[1] = 0.5*sqrt( pow(VecQC_2[0] - VecQC_1[0],2.)
                                 + pow(VecQC_2[1] - VecQC_1[1],2.)
                                 + pow(VecQC_2[2] - VecQC_1[2],2.) );
                                  
       }
                        
       s_[i] = s_[i-1] + ArcLength[0] + ArcLength[1];
              
       VecQC_1[0] = VecQC_2[0];
       VecQC_1[1] = VecQC_2[1];
       VecQC_1[2] = VecQC_2[2];
   
    }
    
    for ( i = 1 ; i < NumPlateI_ ; i++ ) {
       
       s_[i] /= ( s_[NumPlateI_-1] + ArcLength[1] );
                      
    }
    
    AverageChord_ /= (NumPlateI_-1);
    
    // Keep track of the root LE, TE, and Quarter chord

    i = 1; j = 1;

    x1 = x_plate(i,j);
    y1 = y_plate(i,j);
    z1 = z_plate(i,j);

    i = 1; j = NumPlateJ_;
    
    x2 = x_plate(i,j);
    y2 = y_plate(i,j);
    z2 = z_plate(i,j);

    Vec[0] = xTE_[i] - xLE_[i];
    Vec[1] = yTE_[i] - yLE_[i];
    Vec[2] = zTE_[i] - zLE_[i];
    
    Chord = sqrt(vector_dot(Vec,Vec));

    Vec[0] /= LocalChord_[i];
    Vec[1] /= LocalChord_[i];
    Vec[2] /= LocalChord_[i];
    
    Root_LE_[0] = x2;
    Root_LE_[1] = y2;
    Root_LE_[2] = z2;
    
    Root_TE_[0] = x1;
    Root_TE_[1] = y1;
    Root_TE_[2] = z1;
        
    Root_QC_[0] = Root_LE_[0] + 0.25*Chord*Vec[0];
    Root_QC_[1] = Root_LE_[1] + 0.25*Chord*Vec[1];
    Root_QC_[2] = Root_LE_[2] + 0.25*Chord*Vec[2];

    // Keep track of the tip LE, TE, and Quarter chord

    i = NumPlateI_ - 1 ; j = 1;

    x1 = x_plate(i,j);
    y1 = y_plate(i,j);
    z1 = z_plate(i,j);

    i = NumPlateI_ - 1 ; j = NumPlateJ_;
    
    x2 = x_plate(i,j);
    y2 = y_plate(i,j);
    z2 = z_plate(i,j);

    Vec[0] = xTE_[i] - xLE_[i];
    Vec[1] = yTE_[i] - yLE_[i];
    Vec[2] = zTE_[i] - zLE_[i];
    
    Chord = sqrt(vector_dot(Vec,Vec));

    Vec[0] /= LocalChord_[i];
    Vec[1] /= LocalChord_[i];
    Vec[2] /= LocalChord_[i];

    Tip_LE_[0] = x2;
    Tip_LE_[1] = y2;
    Tip_LE_[2] = z2;
    
    Tip_TE_[0] = x1;
    Tip_TE_[1] = y1;
    Tip_TE_[2] = z1;
        
    Tip_QC_[0] = Tip_LE_[0] + 0.25*Chord*Vec[0];
    Tip_QC_[1] = Tip_LE_[1] + 0.25*Chord*Vec[1];
    Tip_QC_[2] = Tip_LE_[2] + 0.25*Chord*Vec[2];    
 
}

/*##############################################################################
#                                                                              #
#                      VSP_SURFACE  LocateHingeLine                            #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::LocateHingeLine(VSPAERO_DOUBLE *up, VSPAERO_DOUBLE *wp, int *HingeNode)
{

    int i, Done, Iter, Node[4], iTemp;
    VSPAERO_DOUBLE u[4], w[4], dw[4], Temp[5];
    
    // Copy over boundary data

    for ( i = 0 ; i <= 3 ; i++ ) {
       
       u[i] = up[i];
       w[i] = wp[i];
          
       dw[i] = ABS( 0.5 - w[i] );
       
       Node[i] = i;
      
    }
       
    // Bubble sort the nodes in increasing w
       
    Done = 0;
    
    Iter = 0;
 
    while ( !Done ) {
       
       Done = 1;
       
       for ( i = 0 ; i <= 2 ; i++ ) {
          
          if ( dw[i] > dw[i+1] ) {
             
             Temp[0] = dw[i+1];
             Temp[1] =  u[i+1];
             Temp[2] =  w[i+1];
             iTemp   = Node[i+1];
             
               dw[i+1] =   dw[i];
                u[i+1] =    u[i];
                w[i+1] =    w[i];
             Node[i+1] = Node[i];
             
               dw[i  ] = Temp[0];
                u[i  ] = Temp[1];
                w[i  ] = Temp[2];      
             Node[i  ] = iTemp;
             
             Done = 0;       
             
          }
          
       }
       
       Iter++;
       
       if ( Iter > 4 ) {
          
          PRINTF("Failed to sort hinge line nodes! \n");fflush(NULL);
          exit(1);
          
       }
           
    }
    
    // Grab the hinge line
    
    for ( i = 0 ; i <= 3 ; i++ ) {
       
       if ( Verbose_ ) PRINTF("i: %d --> u,w,dw: %lf %lf %lf %d \n",i,u[i],w[i],dw[i],Node[i]);
       
    }    
    
    if ( u[0] > u[1] ) {
    
       HingeNode[0] = Node[0];
       HingeNode[1] = Node[1];
       
    }
    
    else{

       HingeNode[0] = Node[0];
       HingeNode[1] = Node[1];
              
    }

}

/*##############################################################################
#                                                                              #
#                      VSP_SURFACE  ReadBodyDataFromFile                       #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::ReadBodyDataFromFile(char *Name, int Case, FILE *VSP_Degen_File)
{
 
    int i, j, k, NumI, NumJ, Done, jStart, jEnd;
    int i1, i2, i3, i4, j1, j2, j3, j4;        
    VSPAERO_DOUBLE DumFloat, Vec[3], VecQC_1[3], VecQC_2[3];
    VSPAERO_DOUBLE x1, y1, z1, x2, y2, z2, ArcLength;
    char DumChar[2000], Stuff[2000];
    VSPAERO_DOUBLE Vec1[3], Vec2[3], Vec3[3], Vec4[3], Mag, CellArea;
    
    // Save the component name

    SPRINTF (ComponentName_,"%s",Name);
    
    // Save case
    
    Case_ = Case;
    
    // Set surface type
    
    SurfaceType_ = DEGEN_BODY_SURFACE;    

    // Read in the body data
    
    // fgets(DumChar,1000,VSP_Degen_File);
    // fgets(DumChar,1000,VSP_Degen_File); sscanf(DumChar,"SURFACE_NODE,%d,%d",&NumI,&NumJ);
    // fgets(DumChar,1000,VSP_Degen_File);
    
    Done = 0;
    
    while ( !Done ) {
       
       fgets(DumChar,1000,VSP_Degen_File);
       
       if ( strstr(DumChar,"SURFACE_NODE") != NULL ) {
          
          sscanf(DumChar,"SURFACE_NODE,%d,%d",&NumI,&NumJ);
          
          Done = 1;
          
       }
       
    }    

    fgets(DumChar,1000,VSP_Degen_File);
    
    if ( Verbose_ ) PRINTF("NumI, NumJ: %d %d \n",NumI,NumJ);

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

          if ( Verbose_ ) PRINTF("xyz: %lf %lf %lf %lf %lf \n",
                                 x(i,j),
                                 y(i,j),
                                 z(i,j),
                                 u(i,j),
                                 v(i,j));
          
       }
       
    }

    // Calculate surface normal
    
    for ( i = 1 ; i <= NumGeomI_ ; i++ ) {
     
       for ( j = 1 ; j <= NumGeomJ_ ; j++ ) {
          
          Nx(i,j) = 0.;
          Ny(i,j) = 0.;
          Nz(i,j) = 0.;
          
          Area(i,j) = 0.;
                   
       }
       
    }
        
    VSPAERO_DOUBLE Volume = 0.;
    VSPAERO_DOUBLE TotalArea = 0.;
    VSPAERO_DOUBLE Zavg;
    
    for ( i = 1 ; i < NumGeomI_ ; i++ ) {
     
       for ( j = 1 ; j < NumGeomJ_ ; j++ ) {
          
          i4 = i;         j4 = j;
          i3 = i    ;     j3 = j + 1;
          i2 = i + 1;     j2 = j + 1;
          i1 = i + 1;     j1 = j;
                              
          // Tri 1
          
          Vec1[0] = x(i2,j2) - x(i1,j1);
          Vec1[1] = y(i2,j2) - y(i1,j1);
          Vec1[2] = z(i2,j2) - z(i1,j1);

          Vec2[0] = x(i3,j3) - x(i1,j1);
          Vec2[1] = y(i3,j3) - y(i1,j1);
          Vec2[2] = z(i3,j3) - z(i1,j1);
                              
          vector_cross(Vec1,Vec2,Vec3);
          
          CellArea = 0.5*sqrt(vector_dot(Vec3,Vec3));
          
          // Tri 2

          Vec1[0] = x(i3,j3) - x(i1,j1);
          Vec1[1] = y(i3,j3) - y(i1,j1);
          Vec1[2] = z(i3,j3) - z(i1,j1);

          Vec2[0] = x(i4,j4) - x(i1,j1);
          Vec2[1] = y(i4,j4) - y(i1,j1);
          Vec2[2] = z(i4,j4) - z(i1,j1);
                              
          vector_cross(Vec1,Vec2,Vec4);
          
          CellArea += 0.5*sqrt(vector_dot(Vec4,Vec4));

          // Final weighted normal
          
          Vec3[0] += Vec4[0];
          Vec3[1] += Vec4[1];
          Vec3[2] += Vec4[2];
          
          // Distribute normal

          Nx(i1,j1) += Vec3[0];
          Ny(i1,j1) += Vec3[1];
          Nz(i1,j1) += Vec3[2];
                 
          Nx(i2,j2) += Vec3[0];
          Ny(i2,j2) += Vec3[1];
          Nz(i2,j2) += Vec3[2];
                 
          Nx(i3,j3) += Vec3[0];
          Ny(i3,j3) += Vec3[1];
          Nz(i3,j3) += Vec3[2];
                 
          Nx(i4,j4) += Vec3[0];
          Ny(i4,j4) += Vec3[1];
          Nz(i4,j4) += Vec3[2];       
          
          // Distribute Area

          Area(i1,j1) += 0.25 * CellArea;
                  
          Area(i2,j2) += 0.25 * CellArea;
                 
          Area(i3,j3) += 0.25 * CellArea;
                 
          Area(i4,j4) += 0.25 * CellArea;

          Zavg = 0.25*( z(i1,j1) + z(i2,j2) + z(i3,j3) + z(i4,j4) );

          Volume += Nz(i,j) * Zavg * CellArea;
          
          TotalArea += CellArea;

          if ( Verbose_ ) PRINTF("Zavg: %f .... Normal: %f %f %f \n",
          Zavg,
          Vec3[0],Vec3[1],Vec3[2]);
          
       }
    
    }       
    
    for ( i = 1 ; i <= NumGeomI_ ; i++ ) {
     
       for ( j = 1 ; j <= NumGeomJ_ ; j++ ) {
          
          Mag = sqrt( Nx(i,j)*Nx(i,j) + Ny(i,j)*Ny(i,j) + Nz(i,j)*Nz(i,j) );
          
          Nx(i,j) /= Mag;
          Ny(i,j) /= Mag;
          Nz(i,j) /= Mag;
   
       }
       
    }
        
    if ( Verbose_ ) PRINTF("Body Area   is: %f \n",TotalArea);
    if ( Verbose_ ) PRINTF("Body Volume is: %f \n",Volume);
    
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
    
    if ( Verbose_ ) PRINTF("NumI, NumJ: %d %d \n",NumI,NumJ);
    
    SizeFlatPlateLists(NumI,NumJ/2+1);

    // Skip over normals information
        
    for ( i = 1 ; i <= NumI + 2 ; i++ ) fgets(DumChar,1000,VSP_Degen_File);  
     
    // Now read in the flat plate representation of the body
    
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
                    &Nx_Camber(i,k),
                    &Ny_Camber(i,k),
                    &Nz_Camber(i,k),
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
                    &Nx_Camber(i,k),
                    &Ny_Camber(i,k),
                    &Nz_Camber(i,k),
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
    
    // Adjust geometry for ground effects analysis
    
    if ( DoGroundEffectsAnalysis_ ) RotateGeometry_About_Y_Axis();    
    
    // Set number of surface patches... 0
        
    NumberOfSurfacePatches_ = 0;
    
    BoundaryConditionForSurface_ = new BOUNDARY_CONDITION_DATA[NumberOfSurfacePatches_ + 2];

    // Calculate local chord lengths

    AverageChord_ = 0.;
    
    s_[0] = ArcLength = 0.;
 
    for ( i = 1 ; i < NumPlateI_ ; i++ ) {
     
       j = 1;
       
       // Calculate local chord
       
       x1 = 0.5*( x_plate(i,j) + x_plate(i+1,j) );
       y1 = 0.5*( y_plate(i,j) + y_plate(i+1,j) );
       z1 = 0.5*( z_plate(i,j) + z_plate(i+1,j) );
 
       j = NumPlateJ_;
       
       x2 = 0.5*( x_plate(i,j) + x_plate(i+1,j) );
       y2 = 0.5*( y_plate(i,j) + y_plate(i+1,j) );
       z2 = 0.5*( z_plate(i,j) + z_plate(i+1,j) );
       
       LocalChord_[i] = sqrt( pow(x2-x1,2.) + pow(y2-y1,2.) + pow(z2-z1,2.) );
       
       AverageChord_ += LocalChord_[i];
       
       // Keep track of the initial leading and trailing edge
       
       xLE_[i] = x2;
       yLE_[i] = y2;
       zLE_[i] = z2;
       
       xTE_[i] = x1;
       yTE_[i] = y1;
       zTE_[i] = z1;
       
       // Calculate arclength along the quarter chord
       
       Vec[0] = xTE_[i] - xLE_[i];
       Vec[1] = yTE_[i] - yLE_[i];
       Vec[2] = zTE_[i] - zLE_[i];

       Vec[0] /= LocalChord_[i];
       Vec[1] /= LocalChord_[i];
       Vec[2] /= LocalChord_[i];
       
       VecQC_2[0] = xLE_[i] + 0.25*LocalChord_[i]*Vec[0];
       VecQC_2[1] = yLE_[i] + 0.25*LocalChord_[i]*Vec[1];
       VecQC_2[2] = zLE_[i] + 0.25*LocalChord_[i]*Vec[2];
       
       if ( i == 1 ) {
          
          ArcLength = 0.;
          
       }
       
       else {

          ArcLength += sqrt( pow(VecQC_2[0] - VecQC_1[0],2.)
                           + pow(VecQC_2[1] - VecQC_1[1],2.)
                           + pow(VecQC_2[2] - VecQC_1[2],2.) );
                           
       }
                        
       s_[i] = ArcLength;
                               
       VecQC_1[0] = VecQC_2[0];
       VecQC_1[1] = VecQC_2[1];
       VecQC_1[2] = VecQC_2[2];
   
    }
    
    for ( i = 1 ; i < NumPlateI_ ; i++ ) {
       
       s_[i] /= s_[NumPlateI_-1];
       
    }
    
    AverageChord_ = sqrt( pow(x_plate(1,1) - x_plate(NumPlateI_,1), 2.)
                        + pow(y_plate(1,1) - y_plate(NumPlateI_,1), 2.)
                        + pow(z_plate(1,1) - z_plate(NumPlateI_,1), 2.) );     
                        
    // Keep track of the root LE, TE, and Quarter chord

    Root_LE_[0] = 0.;
    Root_LE_[1] = 0.;
    Root_LE_[2] = 0.;
    
    Root_TE_[0] = 0.;
    Root_TE_[1] = 0.;
    Root_TE_[2] = 0.;
        
    Root_QC_[0] = 0.;
    Root_QC_[1] = 0.;
    Root_QC_[2] = 0.;

    // Keep track of the tip LE, TE, and Quarter chord
    
    Tip_LE_[0] = 0.;
    Tip_LE_[1] = 0.;
    Tip_LE_[2] = 0.;
    
    Tip_TE_[0] = 0.;
    Tip_TE_[1] = 0.;
    Tip_TE_[2] = 0.;
        
    Tip_QC_[0] = 0.;
    Tip_QC_[1] = 0.;
    Tip_QC_[2] = 0.;                              

}

/*##############################################################################
#                                                                              #
#                   VSP_SURFACE  CheckForDegenerateXSections                   #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::CheckForDegenerateXSections(void)
{
 
    int i, j, ii, *BadXSection, NumBadXSections, NumI;
    VSPAERO_DOUBLE  Distance, Tolerance;
    VSPAERO_DOUBLE Xmin, Xmax, Ymin, Ymax, Zmin, Zmax, Span;
    VSPAERO_DOUBLE *x_plate_new, *y_plate_new, *z_plate_new;
    VSPAERO_DOUBLE *Nx_Camber_new, *Ny_Camber_new, *Nz_Camber_new;
    VSPAERO_DOUBLE *u_plate_new, *v_plate_new;
    VSPAERO_DOUBLE *LocalChord_new;
    VSPAERO_DOUBLE *ThicknessToChord_new, *LocationOfMaxThickness_new, *RadiusToChord_new;
    VSPAERO_DOUBLE *Nx_FlatPlateNormal_new, *Ny_FlatPlateNormal_new, *Nz_FlatPlateNormal_new;
    
    Tolerance = 1.e-6;
  
    // Determine if the any two x-sections are at the same x-location
        
    BadXSection = new int[NumPlateI_ + 1];
    
    zero_int_array(BadXSection, NumPlateI_);
    
    NumBadXSections = 0;
    
    for ( i = 1 ; i < NumPlateI_ ; i++ ) {    
       
       Xmin = Ymin = Zmin = 1.e9;
       Xmax = Ymax = Zmax = -1.e9;
       
       Distance = 0.;
     
       for ( j = 1 ; j <= NumPlateJ_ ; j++ ) {

          Xmin = MIN3(x_plate(i,j),x_plate(i+1,j),Xmin);
          Xmax = MAX3(x_plate(i,j),x_plate(i+1,j),Xmax);
          
          Ymin = MIN3(y_plate(i,j),y_plate(i+1,j),Ymin);
          Ymax = MAX3(y_plate(i,j),y_plate(i+1,j),Ymax);
          
          Zmin = MIN3(z_plate(i,j),z_plate(i+1,j),Zmin);
          Zmax = MAX3(z_plate(i,j),z_plate(i+1,j),Zmax);
           
          Distance += pow(x_plate(i,j)-x_plate(i+1,j),2.);
        
       }
       
       Span = MAX3(Xmax - Xmin, Ymax - Ymin, Zmax - Zmin);

       Distance /= NumPlateJ_;
       
       Distance = sqrt(Distance);
       
       if ( Verbose_ ) PRINTF("Distance: %e .. Span: %f \n",Distance,Span);
  
       if ( i != 1 && i != NumPlateI_ && Distance <= Tolerance * Span ) {
        
          if ( Verbose_ ) PRINTF("X sections %d and %d are the same... located at x: %lf \n",i,i+1,x_plate(i,1));
          
          if ( Verbose_ ) PRINTF("x_plate(i,1): %f ... x_plate(i+1,1): %f \n",x_plate(i,1), x_plate(i+1,1));
          
          BadXSection[i] = 1;
          
          NumBadXSections++;
          
       }
        
    }
   
    // If we have multiple x-sections at the same station... delete one
    
    if ( NumBadXSections > 0 ) {
     
       NumI = NumPlateI_ - NumBadXSections;

       x_plate_new = new VSPAERO_DOUBLE[NumI*NumPlateJ_ + 1];
       y_plate_new = new VSPAERO_DOUBLE[NumI*NumPlateJ_ + 1];
       z_plate_new = new VSPAERO_DOUBLE[NumI*NumPlateJ_ + 1];    
       
       Nx_Camber_new = new VSPAERO_DOUBLE[NumI*NumPlateJ_ + 1];
       Ny_Camber_new = new VSPAERO_DOUBLE[NumI*NumPlateJ_ + 1];
       Nz_Camber_new = new VSPAERO_DOUBLE[NumI*NumPlateJ_ + 1];    
       
       u_plate_new = new VSPAERO_DOUBLE[NumPlateI_*NumPlateJ_ + 1];
       v_plate_new = new VSPAERO_DOUBLE[NumPlateI_*NumPlateJ_ + 1];

       ThicknessToChord_new       = new VSPAERO_DOUBLE[NumI + 1];
       LocationOfMaxThickness_new = new VSPAERO_DOUBLE[NumI + 1];
       RadiusToChord_new          = new VSPAERO_DOUBLE[NumI + 1];      
       
       Nx_FlatPlateNormal_new = new VSPAERO_DOUBLE[NumI + 1];     
       Ny_FlatPlateNormal_new = new VSPAERO_DOUBLE[NumI + 1];      
       Nz_FlatPlateNormal_new = new VSPAERO_DOUBLE[NumI + 1];     
       
       LocalChord_new = new VSPAERO_DOUBLE[NumI + 1];    

       ii = 0;
       
       for ( i = 1 ; i <= NumPlateI_ ; i++ ) {
        
          if ( i == 1 || i == NumPlateI_ || !BadXSection[i] ) {
           
             ii++;
           
             for ( j = 1 ; j <= NumPlateJ_ ; j++ ) {
              
                x_plate_new[(ii-1)*NumPlateJ_ + j] = x_plate_[(i-1)*NumPlateJ_ + j];
                y_plate_new[(ii-1)*NumPlateJ_ + j] = y_plate_[(i-1)*NumPlateJ_ + j];
                z_plate_new[(ii-1)*NumPlateJ_ + j] = z_plate_[(i-1)*NumPlateJ_ + j];
    
                Nx_Camber_new[(ii-1)*NumPlateJ_ + j] = Nx_Camber_[(i-1)*NumPlateJ_ + j];
                Ny_Camber_new[(ii-1)*NumPlateJ_ + j] = Ny_Camber_[(i-1)*NumPlateJ_ + j];
                Nz_Camber_new[(ii-1)*NumPlateJ_ + j] = Nz_Camber_[(i-1)*NumPlateJ_ + j];
                
                u_plate_new[(ii-1)*NumPlateJ_ + j] =     u_plate_[(i-1)*NumPlateJ_ + j];
                v_plate_new[(ii-1)*NumPlateJ_ + j] = v_plate_[(i-1)*NumPlateJ_ + j];
          
             }

             ThicknessToChord_new[ii]       = ThicknessToChord_[i];           
             LocationOfMaxThickness_new[ii] = LocationOfMaxThickness_[i];            
             RadiusToChord_new[ii]          = RadiusToChord_[i];
             
             Nx_FlatPlateNormal_new[ii] = Nx_FlatPlateNormal_[i];
             Ny_FlatPlateNormal_new[ii] = Ny_FlatPlateNormal_[i];
             Nz_FlatPlateNormal_new[ii] = Nz_FlatPlateNormal_[i];
                
             LocalChord_new[ii] = LocalChord_[i];   

          }
          
       }
       
       NumPlateI_ = NumI;
       
       delete[] x_plate_;
       delete[] y_plate_;
       delete[] z_plate_;
       
       delete[] Nx_Camber_;
       delete[] Ny_Camber_;
       delete[] Nz_Camber_;
       
       delete[] u_plate_;
       delete[] v_plate_;
  
       delete[] LocalChord_;
       
       delete[] ThicknessToChord_;
       delete[] LocationOfMaxThickness_;
       delete[] RadiusToChord_;

       delete [] Nx_FlatPlateNormal_;
       delete [] Ny_FlatPlateNormal_;
       delete [] Nz_FlatPlateNormal_;       

       x_plate_ = x_plate_new;
       y_plate_ = y_plate_new;
       z_plate_ = z_plate_new;
       
       Nx_Camber_ = Nx_Camber_new;
       Ny_Camber_ = Ny_Camber_new;
       Nz_Camber_ = Nz_Camber_new;
       
       u_plate_ = u_plate_new;
       v_plate_ = v_plate_new;
   
       LocalChord_ = LocalChord_new;
       
       ThicknessToChord_= ThicknessToChord_new;
       
       LocationOfMaxThickness_ = LocationOfMaxThickness_new;
       
       RadiusToChord_ = RadiusToChord_new;
       
       Nx_FlatPlateNormal_ = Nx_FlatPlateNormal_new;
       Ny_FlatPlateNormal_ = Ny_FlatPlateNormal_new;
       Nz_FlatPlateNormal_ = Nz_FlatPlateNormal_new;        

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
    VSPAERO_DOUBLE  Distance, Tolerance;
    VSPAERO_DOUBLE *x_plate_new, *y_plate_new, *z_plate_new;
    VSPAERO_DOUBLE *Nx_Camber_new, *Ny_Camber_new, *Nz_Camber_new;
    VSPAERO_DOUBLE *u_plate_new, *v_plate_new;
    VSPAERO_DOUBLE *LocalChord_new;
    VSPAERO_DOUBLE x1, y1, z1, x2, y2, z2;
    VSPAERO_DOUBLE *ThicknessToChord_new, *LocationOfMaxThickness_new, *RadiusToChord_new;
    VSPAERO_DOUBLE *Nx_FlatPlateNormal_new, *Ny_FlatPlateNormal_new, *Nz_FlatPlateNormal_new;
    
    Tolerance = 1.e-6;
  
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

       Distance = sqrt(Distance);

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
       
       xLE_[i] = x2;
       yLE_[i] = y2;
       zLE_[i] = z2;

       // Decide if this is a duplicate section
       
       if ( Distance <= Tolerance * LocalChord_[i] ) {
  
          if ( Verbose_ ) PRINTF("Span sections %d and %d are the same... located at y: %lf \n",i,i+1,y_plate(i,1)); fflush(NULL);

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
       
       Distance = sqrt(Distance);

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
        
          if ( Verbose_ ) PRINTF("Span sections %d and %d are the same... located at y: %lf \n",i,i-1,y_plate(i,1)); fflush(NULL);

          BadSpanSection[i] = 1;
          
          NumBadSpanSections++;
          
          fflush(NULL);exit(1);
          
       }
        
    }    

    // If we have multiple span sections at the same station... delete one
    
    if ( NumBadSpanSections > 0 ) {
     
       NumI = NumPlateI_ - NumBadSpanSections;
     
       x_plate_new = new VSPAERO_DOUBLE[NumI*NumPlateJ_ + 1];
       y_plate_new = new VSPAERO_DOUBLE[NumI*NumPlateJ_ + 1];
       z_plate_new = new VSPAERO_DOUBLE[NumI*NumPlateJ_ + 1];    
       
       Nx_Camber_new = new VSPAERO_DOUBLE[NumI*NumPlateJ_ + 1];
       Ny_Camber_new = new VSPAERO_DOUBLE[NumI*NumPlateJ_ + 1];
       Nz_Camber_new = new VSPAERO_DOUBLE[NumI*NumPlateJ_ + 1];    
       
       u_plate_new = new VSPAERO_DOUBLE[NumPlateI_*NumPlateJ_ + 1];
       v_plate_new = new VSPAERO_DOUBLE[NumPlateI_*NumPlateJ_ + 1];
       
       ThicknessToChord_new       = new VSPAERO_DOUBLE[NumI + 1];
       LocationOfMaxThickness_new = new VSPAERO_DOUBLE[NumI + 1];
       RadiusToChord_new          = new VSPAERO_DOUBLE[NumI + 1];      
       
       Nx_FlatPlateNormal_new = new VSPAERO_DOUBLE[NumI + 1];     
       Ny_FlatPlateNormal_new = new VSPAERO_DOUBLE[NumI + 1];      
       Nz_FlatPlateNormal_new = new VSPAERO_DOUBLE[NumI + 1];       
        
       LocalChord_new = new VSPAERO_DOUBLE[NumI + 1]; 
   
       ii = 0;
       
       for ( i = 1 ; i <= NumPlateI_ ; i++ ) {
        
          if ( !BadSpanSection[i] ) {
           
             ii++;
           
             for ( j = 1 ; j <= NumPlateJ_ ; j++ ) {
              
                x_plate_new[(ii-1)*NumPlateJ_ + j] = x_plate_[(i-1)*NumPlateJ_ + j];
                y_plate_new[(ii-1)*NumPlateJ_ + j] = y_plate_[(i-1)*NumPlateJ_ + j];
                z_plate_new[(ii-1)*NumPlateJ_ + j] = z_plate_[(i-1)*NumPlateJ_ + j];
    
                Nx_Camber_new[(ii-1)*NumPlateJ_ + j] = Nx_Camber_[(i-1)*NumPlateJ_ + j];
                Ny_Camber_new[(ii-1)*NumPlateJ_ + j] = Ny_Camber_[(i-1)*NumPlateJ_ + j];
                Nz_Camber_new[(ii-1)*NumPlateJ_ + j] = Nz_Camber_[(i-1)*NumPlateJ_ + j];
                
                u_plate_new[(ii-1)*NumPlateJ_ + j] = u_plate_[(i-1)*NumPlateJ_ + j];
                v_plate_new[(ii-1)*NumPlateJ_ + j] = v_plate_[(i-1)*NumPlateJ_ + j];
                                               
             }

             ThicknessToChord_new[ii]       = ThicknessToChord_[i];           
             LocationOfMaxThickness_new[ii] = LocationOfMaxThickness_[i];            
             RadiusToChord_new[ii]          = RadiusToChord_[i];
             
             Nx_FlatPlateNormal_new[ii] = Nx_FlatPlateNormal_[i];
             Ny_FlatPlateNormal_new[ii] = Ny_FlatPlateNormal_[i];
             Nz_FlatPlateNormal_new[ii] = Nz_FlatPlateNormal_[i];
                
             LocalChord_new[ii] = LocalChord_[i];

          }
          
       }
       
       NumPlateI_ = NumI;
       
       delete[] x_plate_;
       delete[] y_plate_;
       delete[] z_plate_;
       
       delete[] Nx_Camber_;
       delete[] Ny_Camber_;
       delete[] Nz_Camber_;

       delete[] u_plate_;
       delete[] v_plate_;

       delete[] LocalChord_;
       
       delete[] ThicknessToChord_;
       delete[] LocationOfMaxThickness_;
       delete[] RadiusToChord_;

       delete [] Nx_FlatPlateNormal_;
       delete [] Ny_FlatPlateNormal_;
       delete [] Nz_FlatPlateNormal_;                       
                       
       x_plate_ = x_plate_new;
       y_plate_ = y_plate_new;
       z_plate_ = z_plate_new;
       
       Nx_Camber_ = Nx_Camber_new;
       Ny_Camber_ = Ny_Camber_new;
       Nz_Camber_ = Nz_Camber_new;
       
       u_plate_ = u_plate_new;
       v_plate_ = v_plate_new;
       
       LocalChord_ = LocalChord_new;
       
       ThicknessToChord_= ThicknessToChord_new;
       
       LocationOfMaxThickness_ = LocationOfMaxThickness_new;
       
       RadiusToChord_ = RadiusToChord_new;
       
       Nx_FlatPlateNormal_ = Nx_FlatPlateNormal_new;
       Ny_FlatPlateNormal_ = Ny_FlatPlateNormal_new;
       Nz_FlatPlateNormal_ = Nz_FlatPlateNormal_new;                       
   
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

    if ( SurfaceType_ == DEGEN_WING_SURFACE ) CreateWingTriMesh(SurfaceID);

    if ( SurfaceType_ == DEGEN_BODY_SURFACE ) CreateBodyTriMesh(SurfaceID);
 
    // Surface type, and ID data at the tri / loop level

    for ( j = 1 ; j <= Grid().NumberOfTris() ; j++ ) {

      if ( SurfaceType_ == DEGEN_WING_SURFACE ) {
        
          Grid().TriList(j).SurfaceType() = DEGEN_WING_SURFACE;
          Grid().TriList(j).ComponentID() = ComponentID();
          Grid().TriList(j).GeomID()      = GeomID();
          Grid().TriList(j).DegenBodyID() = 0;
          Grid().TriList(j).DegenWingID() = SurfaceID;  
          
       }
       
       else if ( SurfaceType_ == DEGEN_BODY_SURFACE ) {
        
          Grid().TriList(j).SurfaceType() = DEGEN_BODY_SURFACE;
          Grid().TriList(j).ComponentID() = ComponentID();
          Grid().TriList(j).GeomID()      = GeomID();          
          Grid().TriList(j).DegenBodyID() = SurfaceID;
          Grid().TriList(j).DegenWingID() = 0;  
          
       }
       
       else {
        
          PRINTF("Error... trying to mesh an unknown surface type! \n");
          fflush(NULL);
          exit(1);
          
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
        
        WettedArea_ *= 0.5*PI;
         
     }
     
}

/*##############################################################################
#                                                                              #
#                      VSP_SURFACE CreateWingTriMesh                           #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::CreateWingTriMesh(int SurfaceID)
{
 
    int i, j, n, nk, NumNodes, NumTris, node1, node2, node3, node4, Flipped;
    VSPAERO_DOUBLE vec1[3], vec2[3], vec3[3], normal[3], mag, zCamber;
    VSPAERO_DOUBLE x1, y1, z1, x2, y2, z2, Vec[3], VecQC_1[3], VecQC_2[3];
    VSPAERO_DOUBLE Chord, ArcLength[2];
        
    // Estimate the maximum number of grid levels
    
    MaxNumberOfGridLevels_ = 4*(int) ( log(1.0 * NumPlateI_ * NumPlateJ_) / log(4.0) );
    
    Grid_ = new VSP_GRID*[MaxNumberOfGridLevels_ + 1];
    
    // Calculate total number of nodes and tris, including the wake

    NumNodes = NumPlateI_*NumPlateJ_;
    
    NumTris  = 2*(NumPlateI_-1)*(NumPlateJ_-1);
    
    Grid_[0] = new VSP_GRID;
     
    Grid().SizeNodeList(NumNodes);
    
    Grid().SizeTriList(NumTris);
    
    // Set surface type
    
    Grid().SurfaceType() = DEGEN_WING_SURFACE;    
    
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
          
          Grid().NodeList(n).IsTrailingEdgeNode()   = 0;
          Grid().NodeList(n).IsLeadingEdgeNode()    = 0;
          Grid().NodeList(n).IsBoundaryEdgeNode()   = 0;
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

          // Tri 1
          
          n++;

          // Normal
          
          normal[0] = ( Nx_Camber(i,j) + Nx_Camber(i+1,j) + Nx_Camber(i+1,j+1) ) / 3.;
          normal[1] = ( Ny_Camber(i,j) + Ny_Camber(i+1,j) + Ny_Camber(i+1,j+1) ) / 3.;
          normal[2] = ( Nz_Camber(i,j) + Nz_Camber(i+1,j) + Nz_Camber(i+1,j+1) ) / 3.;

          mag = sqrt(vector_dot(normal,normal));
          
          normal[0] /= mag;
          normal[1] /= mag;
          normal[2] /= mag;
          
          // Camber
          
          zCamber = ( Camber(i,j) + Camber(i+1,j) + Camber(i+1,j+1) ) / 3.;
                    
          // Connectivity
          
          Grid().TriList(n).Node1() = node1;
          Grid().TriList(n).Node2() = node2;
          Grid().TriList(n).Node3() = node3;

          // Camber
          
          Grid().TriList(n).Camber() = zCamber;
                    
          // Surface ID
          
          Grid().TriList(n).SurfaceID() = SurfaceID;
          
          // Span station
          
          Grid().TriList(n).SpanStation() = i;
          
          // Airfoil data
          
          Grid().TriList(n).ThicknessToChord()       = ThicknessToChord_[i];
          Grid().TriList(n).LocationOfMaxThickness() = LocationOfMaxThickness_[i];
          Grid().TriList(n).RadiusToChord()          = RadiusToChord_[i];     
          
          // Trailing / leading edge flags
  
          Grid().TriList(n).IsTrailingEdgeTri() = 0;
          
          Grid().TriList(n).IsLeadingEdgeTri() = 0;
          
          if ( j == 1 && IsLiftingSurface_ ) Grid().TriList(n).IsTrailingEdgeTri() = 1;
          
          // Look at actual normal, see if we need to flip something
          
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
             
             if ( Verbose_ ) PRINTF("Case 1---> \n");
             if ( Verbose_ ) PRINTF("vec3:vec3: %f %f %f \n",vec3[0],vec3[1],vec3[2]);
             if ( Verbose_ ) PRINTF("Normal: %f %f %f \n",normal[0],normal[1],normal[2]);
             
             Flipped = 1;
             
          }
          
          // Parametric UV centroid
          
          Grid().TriList(n).Uc() = ( u_plate(i,j) + u_plate(i+1,j) + u_plate(i+1,j+1) ) / 3.;
          Grid().TriList(n).Vc() = ( v_plate(i,j) + v_plate(i+1,j) + v_plate(i+1,j+1) ) / 3.;

          // Tri 2
          
          n++;
          
          // Normal

          normal[0] = ( Nx_Camber(i,j) + Nx_Camber(i+1,j+1) + Nx_Camber(i,j+1) ) / 3.;
          normal[1] = ( Ny_Camber(i,j) + Ny_Camber(i+1,j+1) + Ny_Camber(i,j+1) ) / 3.;
          normal[2] = ( Nz_Camber(i,j) + Nz_Camber(i+1,j+1) + Nz_Camber(i,j+1) ) / 3.;
          
          mag = sqrt(vector_dot(normal,normal));
          
          normal[0] /= mag;
          normal[1] /= mag;
          normal[2] /= mag;
          
          // Camber
          
          zCamber = ( Camber(i,j) + Camber(i+1,j+1) + Camber(i,j+1) ) / 3.;
                                     
          // Connectivity
          
          Grid().TriList(n).Node1() = node1;
          Grid().TriList(n).Node2() = node3;
          Grid().TriList(n).Node3() = node4;

          // Camber
          
          Grid().TriList(n).Camber() = zCamber;
          
          // Surface ID
          
          Grid().TriList(n).SurfaceID() = SurfaceID;
          
          // Span station
          
          Grid().TriList(n).SpanStation() = i;
          
          // Airfoil data
          
          Grid().TriList(n).ThicknessToChord()       = ThicknessToChord_[i];
          Grid().TriList(n).LocationOfMaxThickness() = LocationOfMaxThickness_[i];
          Grid().TriList(n).RadiusToChord()          = RadiusToChord_[i];     
                       
          // Trailing / leading edge flags

          Grid().TriList(n).IsTrailingEdgeTri() = 0;
          
          Grid().TriList(n).IsLeadingEdgeTri() = 0;
          
          if ( j == NumPlateJ_ - 1) Grid().TriList(n).IsLeadingEdgeTri() = 1;
          
          // Look at actual normal, see if we need to flip something
          
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

             if ( Verbose_ ) PRINTF("Case 2---> \n");
             if ( Verbose_ ) PRINTF("vec3: %f %f %f \n",vec3[0],vec3[1],vec3[2]);
             if ( Verbose_ ) PRINTF("Normal: %f %f %f \n",normal[0],normal[1],normal[2]);
  
             Flipped = 1;
             
          }

          // Parametric UV centroid
          
          Grid().TriList(n).Uc() = ( u_plate(i,j) + u_plate(i+1,j) + u_plate(i+1,j+1) ) / 3.;
          Grid().TriList(n).Vc() = ( v_plate(i,j) + v_plate(i+1,j) + v_plate(i+1,j+1) ) / 3.;
                    
       }
       
    }     
    
    // Calculate leading and trailing edges... these could be deformed if an
    // FEM analysis was performed
    
    s_[0] = ArcLength[0] = ArcLength[1] = 0.;
 
    for ( i = 1 ; i < NumPlateI_ ; i++ ) {
     
       j = 1;
       
       // Calculate local chord
       
       x1 = 0.5*( x_plate(i,j) + x_plate(i+1,j) );
       y1 = 0.5*( y_plate(i,j) + y_plate(i+1,j) );
       z1 = 0.5*( z_plate(i,j) + z_plate(i+1,j) );
 
       j = NumPlateJ_;
       
       x2 = 0.5*( x_plate(i,j) + x_plate(i+1,j) );
       y2 = 0.5*( y_plate(i,j) + y_plate(i+1,j) );
       z2 = 0.5*( z_plate(i,j) + z_plate(i+1,j) );
       
       Chord = sqrt( pow(x2-x1,2.) + pow(y2-y1,2.) + pow(z2-z1,2.) );

       // Keep track of the initial leading and trailing edge
       
       xLE_Def_[i] = x2;
       yLE_Def_[i] = y2;
       zLE_Def_[i] = z2;
       
       xTE_Def_[i] = x1;
       yTE_Def_[i] = y1;
       zTE_Def_[i] = z1;
       
       // Calculate arclength along the quarter chord
       
       Vec[0] = xTE_[i] - xLE_[i];
       Vec[1] = yTE_[i] - yLE_[i];
       Vec[2] = zTE_[i] - zLE_[i];

       Vec[0] /= LocalChord_[i];
       Vec[1] /= LocalChord_[i];
       Vec[2] /= LocalChord_[i];
       
       VecQC_2[0] = xLE_[i] + 0.25*Chord*Vec[0];
       VecQC_2[1] = yLE_[i] + 0.25*Chord*Vec[1];
       VecQC_2[2] = zLE_[i] + 0.25*Chord*Vec[2];

       if ( i == 1 ) {

          VecQC_1[0] = x_plate(i,j) + 0.25*(x_plate(i,NumPlateJ_) - x_plate(i,j));     
          VecQC_1[1] = y_plate(i,j) + 0.25*(y_plate(i,NumPlateJ_) - y_plate(i,j));        
          VecQC_1[2] = z_plate(i,j) + 0.25*(z_plate(i,NumPlateJ_) - z_plate(i,j));        
          
          ArcLength[1] = 0.5*sqrt( pow(VecQC_2[0] - VecQC_1[0],2.)
                                 + pow(VecQC_2[1] - VecQC_1[1],2.)
                                 + pow(VecQC_2[2] - VecQC_1[2],2.) );        
          
       }
       
       else {

          ArcLength[0] = ArcLength[1];

          ArcLength[1] = 0.5*sqrt( pow(VecQC_2[0] - VecQC_1[0],2.)
                                 + pow(VecQC_2[1] - VecQC_1[1],2.)
                                 + pow(VecQC_2[2] - VecQC_1[2],2.) );
                                  
       }
                        
       s_Def_[i] = s_Def_[i-1] + ArcLength[0] + ArcLength[1];
              
       VecQC_1[0] = VecQC_2[0];
       VecQC_1[1] = VecQC_2[1];
       VecQC_1[2] = VecQC_2[2];
   
    }   

    for ( i = 1 ; i < NumPlateI_ ; i++ ) {

       s_Def_[i] /= ( s_Def_[NumPlateI_-1] + ArcLength[1] );
       
       if ( IsLiftingSurface_ ) {
 
          Grid().KuttaNodeSoverB(i) = s_Def_[i];

       }
       
    }
    
    if ( IsLiftingSurface_  ) Grid().KuttaNodeSoverB(NumPlateI_) = 1.;

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
    VSPAERO_DOUBLE vec1[3], vec2[3], vec3[3], normal[3], mag;
    VSPAERO_DOUBLE x1, y1, z1, x2, y2, z2, Nx, Ny, Nz, Distance, Tolerance;
    VSPAERO_DOUBLE Vec[3], VecQC_1[3], VecQC_2[3], Chord, ArcLength;

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
    
    Distance = sqrt(Distance);
    
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
    
    Distance = sqrt(Distance);

    TailIsClosed_ = 0;

    if ( Distance <= Tolerance ) TailIsClosed_ = 1;
  
    // Size the node list

    NumNodes = NumPlateI_*NumPlateJ_;
    
    if ( NoseIsClosed_ ) NumNodes -= ( NumPlateJ_ - 1 );
    
    if ( TailIsClosed_ ) NumNodes -= ( NumPlateJ_ - 1 );
    
    if ( Verbose_ ) {
       
       if ( NoseIsClosed_ ) {
      
          PRINTF("Nose is closed... \n");
          
       }
       
       else {
          
          PRINTF("Nose is open... \n");
          
       }      
      
       if ( TailIsClosed_ ) {
      
          PRINTF("Tail is closed... \n");
          
       }
       
       else {
          
          PRINTF("Tail is open... \n");
          
       } 
       
    }
    
    Grid_[0] = new VSP_GRID;
    
    Grid().SizeNodeList(NumNodes);
    
    // Set surface type
    
    Grid().SurfaceType() = DEGEN_BODY_SURFACE;        
 
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
     
       PRINTF("Error in determining the number of valid nodes in body mesh! \n"); fflush(NULL);
       
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
             Grid().NodeList(n).IsLeadingEdgeNode()  = 0;
             
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
           
             normal[0] = ( Nx_Camber(i,j) + Nx_Camber(i+1,j) + Nx_Camber(i+1,j+1) ) / 3.;
             normal[1] = ( Ny_Camber(i,j) + Ny_Camber(i+1,j) + Ny_Camber(i+1,j+1) ) / 3.;
             normal[2] = ( Nz_Camber(i,j) + Nz_Camber(i+1,j) + Nz_Camber(i+1,j+1) ) / 3.;
             
             NodeA = node1;
             NodeB = node2;
             NodeC = node3;

          }
          
          else {
           
             normal[0] = ( Nx_Camber(i,j) + Nx_Camber(i+1,j) + Nx_Camber(i,j+1) ) / 3.;
             normal[1] = ( Ny_Camber(i,j) + Ny_Camber(i+1,j) + Ny_Camber(i,j+1) ) / 3.;
             normal[2] = ( Nz_Camber(i,j) + Nz_Camber(i+1,j) + Nz_Camber(i,j+1) ) / 3.;
             
             NodeA = node1;
             NodeB = node2;
             NodeC = node4;

          }
          
          if ( NodeA != NodeB && NodeA != NodeC && NodeB != NodeC ) {

             n++;
          
             // Normal 
                       
             vec1[0] = Grid().NodeList(NodeB).x() - Grid().NodeList(NodeA).x();
             vec1[1] = Grid().NodeList(NodeB).y() - Grid().NodeList(NodeA).y();
             vec1[2] = Grid().NodeList(NodeB).z() - Grid().NodeList(NodeA).z();

             vec2[0] = Grid().NodeList(NodeC).x() - Grid().NodeList(NodeA).x();
             vec2[1] = Grid().NodeList(NodeC).y() - Grid().NodeList(NodeA).y();
             vec2[2] = Grid().NodeList(NodeC).z() - Grid().NodeList(NodeA).z();

             vector_cross(vec1,vec2,vec3);
             
             mag = sqrt(vector_dot(vec3,vec3));        
                 
             normal[0] /= mag;
             normal[1] /= mag;
             normal[2] /= mag;
                                  
             // Connectivity
          
             Grid().TriList(n).Node1() = NodeA;
             Grid().TriList(n).Node2() = NodeB;
             Grid().TriList(n).Node3() = NodeC;

             // Surface ID

             Grid().TriList(n).SurfaceID() = SurfaceID;
             
             // Span station
             
             Grid().TriList(n).SpanStation() = i;
             
             // Airfoil data
             
             Grid().TriList(n).ThicknessToChord()       = 0.;
             Grid().TriList(n).LocationOfMaxThickness() = 0.;
             Grid().TriList(n).RadiusToChord()          = 0.;  
             
             // Leading and trailing edge flags
     
             Grid().TriList(n).IsTrailingEdgeTri() = 0;
             
             Grid().TriList(n).IsLeadingEdgeTri() = 0;
             
          }

          // Tri 2

          if ( j <= NumPlateJ_/2 ) {
          
             normal[0] = ( Nx_Camber(i,j) + Nx_Camber(i+1,j+1) + Nx_Camber(i,j+1) ) / 3.;
             normal[1] = ( Ny_Camber(i,j) + Ny_Camber(i+1,j+1) + Ny_Camber(i,j+1) ) / 3.;
             normal[2] = ( Nz_Camber(i,j) + Nz_Camber(i+1,j+1) + Nz_Camber(i,j+1) ) / 3.;
             
             NodeA = node1;
             NodeB = node3;
             NodeC = node4;
         
          }
          
          else {
           
             normal[0] = ( Nx_Camber(i,j+1) + Nx_Camber(i+1,j) + Nx_Camber(i+1,j+1) ) / 3.;
             normal[1] = ( Ny_Camber(i,j+1) + Ny_Camber(i+1,j) + Ny_Camber(i+1,j+1) ) / 3.;
             normal[2] = ( Nz_Camber(i,j+1) + Nz_Camber(i+1,j) + Nz_Camber(i+1,j+1) ) / 3.;
             
             NodeA = node4;
             NodeB = node2;
             NodeC = node3;
         
          }
          
          if ( NodeA != NodeB && NodeA != NodeC && NodeB != NodeC ) {
           
             n++;
              
             // Normal
             
             vec1[0] = Grid().NodeList(NodeB).x() - Grid().NodeList(NodeA).x();
             vec1[1] = Grid().NodeList(NodeB).y() - Grid().NodeList(NodeA).y();
             vec1[2] = Grid().NodeList(NodeB).z() - Grid().NodeList(NodeA).z();

             vec2[0] = Grid().NodeList(NodeC).x() - Grid().NodeList(NodeA).x();
             vec2[1] = Grid().NodeList(NodeC).y() - Grid().NodeList(NodeA).y();
             vec2[2] = Grid().NodeList(NodeC).z() - Grid().NodeList(NodeA).z();

             vector_cross(vec1,vec2,vec3);
             
             mag = sqrt(vector_dot(vec3,vec3));                  
                               
             normal[0] /= mag;
             normal[1] /= mag;
             normal[2] /= mag;
             
             // Connectivity
                     
             Grid().TriList(n).Node1() = NodeA;
             Grid().TriList(n).Node2() = NodeB;
             Grid().TriList(n).Node3() = NodeC;

             // Surface ID

             Grid().TriList(n).SurfaceID() = SurfaceID;
             
             // Span station
             
             Grid().TriList(n).SpanStation() = i;
             
             // Airfoil data
             
             Grid().TriList(n).ThicknessToChord()       = 0.;
             Grid().TriList(n).LocationOfMaxThickness() = 0.;
             Grid().TriList(n).RadiusToChord()          = 0.;
             
             // Leading and trailing edge flags
     
             Grid().TriList(n).IsTrailingEdgeTri() = 0;
             
             Grid().TriList(n).IsLeadingEdgeTri() = 0;
             
          }
      
       }
              
    }  
    
    if ( n != NumTris ) {
     
       PRINTF("Error in determining the number of valid tris in body mesh! \n"); fflush(NULL);
       
       exit(1);
       
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

    // Calculate leading and trailing edges... these could be deformed if an
    // FEM analysis was performed

    s_[1] = ArcLength = 0.;
 
    for ( i = 1 ; i < NumPlateI_ ; i++ ) {
     
       j = 1;
       
       // Calculate local chord
       
       x1 = 0.5*( x_plate(i,j) + x_plate(i+1,j) );
       y1 = 0.5*( y_plate(i,j) + y_plate(i+1,j) );
       z1 = 0.5*( z_plate(i,j) + z_plate(i+1,j) );
 
       j = NumPlateJ_;
       
       x2 = 0.5*( x_plate(i,j) + x_plate(i+1,j) );
       y2 = 0.5*( y_plate(i,j) + y_plate(i+1,j) );
       z2 = 0.5*( z_plate(i,j) + z_plate(i+1,j) );
       
       Chord = sqrt( pow(x2-x1,2.) + pow(y2-y1,2.) + pow(z2-z1,2.) );

       // Keep track of the initial leading and trailing edge
       
       xLE_Def_[i] = x2;
       yLE_Def_[i] = y2;
       zLE_Def_[i] = z2;
       
       xTE_Def_[i] = x1;
       yTE_Def_[i] = y1;
       zTE_Def_[i] = z1;
       
       // Calculate arclength along the quarter chord
       
       Vec[0] = xTE_[i] - xLE_[i];
       Vec[1] = yTE_[i] - yLE_[i];
       Vec[2] = zTE_[i] - zLE_[i];

       Vec[0] /= LocalChord_[i];
       Vec[1] /= LocalChord_[i];
       Vec[2] /= LocalChord_[i];
       
       VecQC_2[0] = xLE_[i] + 0.25*Chord*Vec[0];
       VecQC_2[1] = yLE_[i] + 0.25*Chord*Vec[1];
       VecQC_2[2] = zLE_[i] + 0.25*Chord*Vec[2];
       
       if ( i == 1 ) {
          
          ArcLength = 0.;
          
       }
       
       else {

          ArcLength += sqrt( pow(VecQC_2[0] - VecQC_1[0],2.)
                           + pow(VecQC_2[1] - VecQC_1[1],2.)
                           + pow(VecQC_2[2] - VecQC_1[2],2.) );
                           
       }
                        
       s_Def_[i] = ArcLength;
                               
       VecQC_1[0] = VecQC_2[0];
       VecQC_1[1] = VecQC_2[1];
       VecQC_1[2] = VecQC_2[2];
   
    }
    
    for ( i = 1 ; i < NumPlateI_ ; i++ ) {
       
       s_Def_[i] /= s_Def_[NumPlateI_-1];
       
    }      
          
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
    VSPAERO_DOUBLE VortexLoop1DownWindWeight, VortexLoop2DownWindWeight;

    VortexLoop1DownWindWeight = VortexLoop2DownWindWeight = 0;

    VSP_NODE VSP_Node1, VSP_Node2;
    
    // Setup the surface vortex edge data
   
    for ( j = 1 ; j <= Grid().NumberOfEdges() ; j++ ) {

       // Pointers from vortex edge back to the parent wing or body and grid edge
       
       if ( SurfaceType() == DEGEN_WING_SURFACE ) {
       
          Grid().EdgeList(j).DegenWing() = SurfaceID;
          
          Grid().EdgeList(j).DegenBody() = 0;

       }
       
       else if ( SurfaceType() == DEGEN_BODY_SURFACE ) {
   
          Grid().EdgeList(j).DegenWing() = 0;
          
          Grid().EdgeList(j).DegenBody() = SurfaceID;

       }
       
       else if ( SurfaceType() == CART3D_SURFACE || SurfaceType() == VSPGEOM_SURFACE ) {
   
          Grid().EdgeList(j).DegenWing() = 0;
          
          Grid().EdgeList(j).DegenBody() = 0;
          
          PRINTF("How did we get here?!?! \n");fflush(NULL);
          exit(1);

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
    
    PRINTF("Wing MaxNumberOfGridLevels_: %d \n",MaxNumberOfGridLevels_);
 
    if ( SurfaceType_ == DEGEN_WING_SURFACE ) PRINTF("Wing Grid:%d --> Number of loops: %d \n",0,Grid_[0]->NumberOfTris());
    if ( SurfaceType_ == DEGEN_BODY_SURFACE ) PRINTF("Body Grid:%d --> Number of loops: %d \n",0,Grid_[0]->NumberOfTris());
    
    // First attempt to simplify the grid

    Grid_[1] = Agglomerate.SimplifyMesh(*(Grid_[0]));

    if ( SurfaceType_ == DEGEN_WING_SURFACE ) PRINTF("Wing Grid:%d --> Number of loops: %d \n",1,Grid_[1]->NumberOfTris());
    if ( SurfaceType_ == DEGEN_BODY_SURFACE ) PRINTF("Body Grid:%d --> Number of loops: %d \n",1,Grid_[1]->NumberOfTris());
        
    i = 2;
    
    while ( i < MaxNumberOfGridLevels_ && Grid_[i-1]->NumberOfTris() > 1 ) {

       Grid_[i] = Agglomerate.Agglomerate(*(Grid_[i-1]));
       
       if ( SurfaceType_ == DEGEN_WING_SURFACE ) PRINTF("Wing Grid:%d --> Number of loops: %d \n",i,Grid_[i]->NumberOfTris());
       if ( SurfaceType_ == DEGEN_BODY_SURFACE ) PRINTF("Body Grid:%d --> Number of loops: %d \n",i,Grid_[i]->NumberOfTris());
       
       i++;

    }
    
    NumberOfGridLevels_ = i;
    
    PRINTF("NumberOfGridLevels_: %d \n",NumberOfGridLevels_);
 
}

/*##############################################################################
#                                                                              #
#                      VSP_SURFACE LoadFEMDeformationData                      #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::LoadFEMDeformationData(char *FileName)
{
 
    int i, NumberOfFEMNodes;
    FILE *FEMFile;
    
    // Open the FEM deformation file

    if ( (FEMFile = fopen(FileName, "r")) == NULL ) {

       PRINTF("Could not open the FEM deformation file for input! \n");

       exit(1);

    }

    fscanf(FEMFile,"%d",&NumberOfFEMNodes);

    FemData().SizeList(NumberOfFEMNodes);

    for ( i = 1 ; i <= FemData().NumberOfFEMNodes() ; i++ ) { 
    
       fscanf(FEMFile,"%lf %lf %lf %lf %lf %lf %lf %lf %lf \n",
             &(FemData().x(i)),
             &(FemData().y(i)),   
             &(FemData().z(i)),        
             &(FemData().delta_xv(i)),
             &(FemData().delta_yv(i)),
             &(FemData().delta_zv(i)), 
             &(FemData().delta_phix(i)), 
             &(FemData().delta_phiy(i)), 
             &(FemData().delta_phiz(i)));

    }

    fclose(FEMFile);

}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER FEMDeformGeometry                         #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::FEMDeformGeometry(void)
{
   
    int i, j;
    VSPAERO_DOUBLE xw, yw, zw, xr, yr, zr;
    VSPAERO_DOUBLE xw1, yw1, zw1, xw2, yw2, zw2, xw3, yw3, zw3, xwp, ywp, zwp;   
    VSPAERO_DOUBLE delta_xv, delta_yv, delta_zv;
    VSPAERO_DOUBLE delta_phix, delta_phiy, delta_phiz;

    for ( i = 1 ; i <= NumPlateI_ ; i++ ) {
     
//xr = x_plate(i,NumPlateJ_); 
//yr = y_plate(i,NumPlateJ_); 
//zr = z_plate(i,NumPlateJ_); 

       for ( j = 1 ; j <= NumPlateJ_ ; j++ ) {

          xw = x_plate(i,j);
          yw = y_plate(i,j);
          zw = z_plate(i,j);   

          // Get rotation points

          xr = FemData().x(yw);
         
          yr = yw;

          zr = FemData().z(yw);
          
          // Get deformation data

          delta_xv = FemData().delta_xv(yw);
          delta_yv = FemData().delta_yv(yw);
          delta_zv = FemData().delta_zv(yw);
          
          delta_phix = FemData().delta_phix(yw);
          delta_phiy = FemData().delta_phiy(yw);
          delta_phiz = FemData().delta_phiz(yw);
        
          // 1st transformation for phiy
          
          xw1 = xr + (xw-xr)*cos(delta_phiy) + (zw-zr)*sin(delta_phiy);
          yw1 = yw;
          zw1 = zr - (xw-xr)*sin(delta_phiy) + (zw-zr)*cos(delta_phiy);
          
          // 2nd transformation for phix
          
          xw2 = xw1;
          yw2 = yr + (yw1-yr)*cos(delta_phix) - (zw1-zr)*sin(delta_phix);
          zw2 = zr + (yw1-yr)*sin(delta_phix) + (zw1-zr)*cos(delta_phix);
          
          // 3rd transformation for phiz
          
          xw3 = xr + (xw2-xr)*cos(delta_phiz) - (yw2-yr)*sin(delta_phiz);
          yw3 = yr + (xw2-xr)*sin(delta_phiz) + (yw2-yr)*cos(delta_phiz);
          zw3 = zw2;
          
          // Deflection
          
          xwp = xw3 + delta_xv;
          ywp = yw3 + delta_yv;
          zwp = zw3 + delta_zv;

          // Update the flat plate model
      
          x_plate(i,j) = xwp;
          y_plate(i,j) = ywp;
          z_plate(i,j) = zwp; 
          
       }
       
    }
    
}

/*##############################################################################
#                                                                              #
#                         VSP_SOLVER FEMUnDeformGeometry                       #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::FEMUnDeformGeometry(VSPAERO_DOUBLE xyz_p[3])
{
   
    VSPAERO_DOUBLE xw, yw, zw, xr, yr, zr;
    VSPAERO_DOUBLE xw1, yw1, zw1, xw2, yw2, zw2, xw3, yw3, zw3;   
    VSPAERO_DOUBLE delta_xv, delta_yv, delta_zv;
    VSPAERO_DOUBLE delta_phix, delta_phiy, delta_phiz;

    // Apply FEM transformations in reverse
    
    xw = xyz_p[0];
    yw = xyz_p[1];
    zw = xyz_p[2];

    // Get rotation points

    xr = FemData().x(yw);
   
    yr = yw;

    zr = FemData().z(yw);

    delta_xv = FemData().delta_xv(yw);
    delta_yv = FemData().delta_yv(yw);
    delta_zv = FemData().delta_zv(yw);
    
    delta_phix = FemData().delta_phix(yw);
    delta_phiy = FemData().delta_phiy(yw);
    delta_phiz = FemData().delta_phiz(yw);
   
    xw -= delta_xv;    
    yw -= delta_yv;     
    zw -= delta_zv;    
              
    // 3rd transformation for phiz
    
    xw3 = xr + (xw-xr)*cos(delta_phiz) + (yw-yr)*sin(delta_phiz);
    yw3 = yr - (xw-xr)*sin(delta_phiz) + (yw-yr)*cos(delta_phiz);
    zw3 = zw;
       
    // 2nd transformation for phix
    
    xw2 = xw3;
    yw2 = yr + (yw3-yr)*cos(delta_phix) + (zw3-zr)*sin(delta_phix);
    zw2 = zr - (yw3-yr)*sin(delta_phix) + (zw3-zr)*cos(delta_phix);
    
    // 1st transformation for phiy (delta_alpha)
    
    xw1 = xr + (xw2-xr)*cos(delta_phiy) - (zw2-zr)*sin(delta_phiy);
    yw1 = yw2;
    zw1 = zr + (xw2-xr)*sin(delta_phiy) + (zw2-zr)*cos(delta_phiy);
    
    // Update point

    xyz_p[0] = xw1;
    xyz_p[1] = yw1;
    xyz_p[2] = zw1; 
 
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER RotateGeometry_About_Y_Axis                   #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::RotateGeometry_About_Y_Axis(void)  
{

    int i, j;
    VSPAERO_DOUBLE x, y, z, RotAngle, DeltaHeight, Xcg, Zcg; 
       
    // Rotate geometry about Y axis

    Xcg = GroundEffectsCGLocation_[0];
    Zcg = GroundEffectsCGLocation_[2];
    
    RotAngle = GroundEffectsRotationAngle_ * TORAD;
       
    DeltaHeight = GroundEffectsHeightAboveGround_ - Zcg;
   
    if ( SurfaceType_ != CART3D_SURFACE && SurfaceType_ != VSPGEOM_SURFACE ) {
       
       for ( i = 1 ; i <= NumPlateI_ ; i++ ) {
        
         for ( j = 1 ; j <= NumPlateJ_ ; j++ ) {
   
              x = x_plate(i,j);
              y = y_plate(i,j);
              z = z_plate(i,j);   
   
              x_plate(i,j) = (x - Xcg)*cos(RotAngle) - (z - Zcg)*sin(RotAngle) + Xcg;
              y_plate(i,j) = y;
              z_plate(i,j) = (x - Xcg)*sin(RotAngle) + (z - Zcg)*cos(RotAngle) + Zcg + DeltaHeight;           
   
           }
          
       }

    }
    
    else {

       for ( i = 1 ; i <= Grid().NumberOfNodes() ; i++ ) {
          
          x = Grid().NodeList(i).x();
          y = Grid().NodeList(i).y();
          z = Grid().NodeList(i).z();
       
          Grid().NodeList(i).x() = (x - Xcg)*cos(RotAngle) - (z - Zcg)*sin(RotAngle) + Xcg;
          Grid().NodeList(i).y() = y;
          Grid().NodeList(i).z() = (x - Xcg)*sin(RotAngle) + (z - Zcg)*cos(RotAngle) + Zcg + DeltaHeight;    
       
       }    

    }
          
}

/*##############################################################################
#                                                                              #
#                     VSP_SOLVER Interpolate_XYZ_From_UV                       #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::Interpolate_XYZ_From_UV(VSPAERO_DOUBLE u, VSPAERO_DOUBLE v, VSPAERO_DOUBLE *xyz)
{
   
   VSPAERO_DOUBLE Cu, Cv, N1, N2, N3, N4;
   int i, j, Found;
   
   // Find i
   
   i = Found = 0;
   
   if ( u <= u_plate(1         ,1) ) { i =          1 ; Found = 1; };
   if ( u >= u_plate(NumPlateI_,1) ) { i = NumPlateI_ ; Found = 1; };
   
   while ( i < NumPlateI_ && !Found ) {
      
      i++;
      
    //  PRINTF("i: %d: %f -------- %f ----------- %f \n",i,u_plate(i,1),u,u_plate(i+1,1));
      
      if ( u_plate(i,1) <= u && u_plate(i+1,1) >= u ) Found = 1;
       
   }
   
   if ( !Found ) { PRINTF("u not found! \n");  fflush(NULL); exit(1); }
   
   Cu = ( u - u_plate(i,1) ) / ( u_plate(i+1,1) - u_plate(i,1) );
   
 //  PRINTF("Cu: %f \n",Cu);
   
   // Find j
   
   j = Found = 0;
   
   if ( v <= v_plate(1,         1) ) { j =          1 ; Found = 1; };
   if ( v >= v_plate(1,NumPlateJ_) ) { j = NumPlateJ_ ; Found = 1; };   
   
   while ( j < NumPlateJ_ && !Found ) {
      
      j++;
      
   //   PRINTF("j: %d: %f -------- %f ----------- %f \n",j,v_plate(1,j),v,v_plate(1,j+1));
      
      if ( v_plate(1,j) <= v && v_plate(1,j+1) >= v ) Found = 1;
       
   }   
   
   Cv = ( v - v_plate(1,j) ) / ( v_plate(1,j+1) - v_plate(1,j) );  

   if ( !Found ) { PRINTF("v not found! \n"); fflush(NULL); exit(1); }
   
   // Interpolate for xyz
   
 //  PRINTF("Cu,Cv: %f %f \n",Cu,Cv);
   
   Cu = -1. + 2.*Cu;
   Cv = -1. + 2.*Cv;

   N1 = 0.25*(1. - Cu) * (1. - Cv);
   N2 = 0.25*(1. + Cu) * (1. - Cv);
   N3 = 0.25*(1. + Cu) * (1. + Cv);
   N4 = 0.25*(1. - Cu) * (1. + Cv);

   xyz[0] = N1 * x_plate(i,j) + N2 * x_plate(i+1,j) + N3 * x_plate(i+1,j+1) + N4 * x_plate(i,j+1);
   xyz[1] = N1 * y_plate(i,j) + N2 * y_plate(i+1,j) + N3 * y_plate(i+1,j+1) + N4 * y_plate(i,j+1);
   xyz[2] = N1 * z_plate(i,j) + N2 * z_plate(i+1,j) + N3 * z_plate(i+1,j+1) + N4 * z_plate(i,j+1);

}

/*##############################################################################
#                                                                              #
#                      VSP_SURFACE UpdateGeometryLocation                      #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat)
{
 
    int i, j;
    VSPAERO_DOUBLE x1, y1, z1, x2, y2, z2, Chord, sVec[3];
    QUAT Vec;
    
    // Update xyz data

    for ( i = 1 ; i <= NumGeomI_ ; i++ ) {
     
       for ( j = 1 ; j <= NumGeomJ_ ; j++ ) {
   
          // Update xyz location
          
          Vec(0) = x(i,j) - OVec[0];
          Vec(1) = y(i,j) - OVec[1];
          Vec(2) = z(i,j) - OVec[2];
      
          Vec = Quat * Vec * InvQuat;
      
          x(i,j) = Vec(0) + OVec[0] + TVec[0];
          y(i,j) = Vec(1) + OVec[1] + TVec[1];
          z(i,j) = Vec(2) + OVec[2] + TVec[2];    

          // Update normal

          Vec(0) = Nx(i,j);
          Vec(1) = Ny(i,j);
          Vec(2) = Nz(i,j);
      
          Vec = Quat * Vec * InvQuat;
          
          Nx(i,j) = Vec(0);
          Ny(i,j) = Vec(1);
          Nz(i,j) = Vec(2);         
 
       }        
       
    }  
    
    // Update LE and TE data
    
    for ( i = 1 ; i < NumPlateI_ ; i++ ) {
       
       // LE
       
       Vec(0) = xLE_[i] - OVec[0];
       Vec(1) = yLE_[i] - OVec[1];
       Vec(2) = zLE_[i] - OVec[2];       
       
       Vec = Quat * Vec * InvQuat;
       
       xLE_[i] = Vec(0) + OVec[0] + TVec[0];
       yLE_[i] = Vec(1) + OVec[1] + TVec[1];
       zLE_[i] = Vec(2) + OVec[2] + TVec[2];             

       // TE
       
       Vec(0) = xTE_[i] - OVec[0];
       Vec(1) = yTE_[i] - OVec[1];
       Vec(2) = zTE_[i] - OVec[2];       
       
       Vec = Quat * Vec * InvQuat;
       
       xTE_[i] = Vec(0) + OVec[0] + TVec[0];
       yTE_[i] = Vec(1) + OVec[1] + TVec[1];
       zTE_[i] = Vec(2) + OVec[2] + TVec[2];      
       
       // Update flat plate normals
 
       Vec(0) = Nx_FlatPlateNormal_[i];
       Vec(1) = Ny_FlatPlateNormal_[i];
       Vec(2) = Nz_FlatPlateNormal_[i];

       Vec = Quat * Vec * InvQuat;
       
       Nx_FlatPlateNormal_[i] = Vec(0);
       Ny_FlatPlateNormal_[i] = Vec(1);
       Nz_FlatPlateNormal_[i] = Vec(2); 
       
    }

    // Keep track of the tip LE, TE, and Quarter chord

    i = NumPlateI_ - 1 ; j = 1;

    x1 = x_plate(i,j);
    y1 = y_plate(i,j);
    z1 = z_plate(i,j);

    i = NumPlateI_ - 1 ; j = NumPlateJ_;
    
    x2 = x_plate(i,j);
    y2 = y_plate(i,j);
    z2 = z_plate(i,j);

    sVec[0] = xTE_[i] - xLE_[i];
    sVec[1] = yTE_[i] - yLE_[i];
    sVec[2] = zTE_[i] - zLE_[i];
    
    Chord = sqrt(vector_dot(sVec,sVec));

    sVec[0] /= LocalChord_[i];
    sVec[1] /= LocalChord_[i];
    sVec[2] /= LocalChord_[i];

    Tip_LE_[0] = x2;
    Tip_LE_[1] = y2;
    Tip_LE_[2] = z2;
    
    Tip_TE_[0] = x1;
    Tip_TE_[1] = y1;
    Tip_TE_[2] = z1;
        
    Tip_QC_[0] = Tip_LE_[0] + 0.25*Chord*sVec[0];
    Tip_QC_[1] = Tip_LE_[1] + 0.25*Chord*sVec[1];
    Tip_QC_[2] = Tip_LE_[2] + 0.25*Chord*sVec[2];   
    
}

#include "END_NAME_SPACE.H"


