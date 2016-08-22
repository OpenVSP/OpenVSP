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
    
    NumberOfControlSurfaces_ = 0;
    
    MaxNumberOfControlSurfaces_ = 10;
    
    ControlSurface_ = new CONTROL_SURFACE[MaxNumberOfControlSurfaces_ + 1];
    
    //Initialize stuff
    NumberOfSurfacePatches_ = 0;

    NumGeomI_ = 0;
    NumGeomJ_ = 0;

    NumPlateI_ = 0;
    NumPlateJ_ = 0;

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
    
    xLE_ = new double[NumPlateI_ + 1];    
    yLE_ = new double[NumPlateI_ + 1];    
    zLE_ = new double[NumPlateI_ + 1];        

    xTE_ = new double[NumPlateI_ + 1];    
    yTE_ = new double[NumPlateI_ + 1];    
    zTE_ = new double[NumPlateI_ + 1];     
    
      s_ = new double[NumPlateI_ + 1];     
      
    xLE_Def_ = new double[NumPlateI_ + 1];    
    yLE_Def_ = new double[NumPlateI_ + 1];    
    zLE_Def_ = new double[NumPlateI_ + 1];        

    xTE_Def_ = new double[NumPlateI_ + 1];    
    yTE_Def_ = new double[NumPlateI_ + 1];    
    zTE_Def_ = new double[NumPlateI_ + 1];       
    
    s_Def_ = new double[NumPlateI_ + 1];          

    u_plate_  = new double[NumPlateI_*NumPlateJ_ + 1];
    v_plate_ = new double[NumPlateI_*NumPlateJ_ + 1];
                
}

/*##############################################################################
#                                                                              #
#                          VSP_SURFACE ReadCart3DDataFromFile                  #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::ReadCart3DDataFromFile(char *Name, FILE *CART3D_File)
{
 
    int i, n, NumNodes, NumTris, Node1, Node2, Node3, SurfaceID;
    int *SurfaceList, Found;
    double x, y, z;

    // Save the component name
    
    sprintf(ComponentName_,"%s_Cart3d",Name);
    
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
    
    // Kutta nodes... Trailing edge runs in the I direction...

    if ( 0 ) {
       
       IsLiftingSurface_ = 1;

       Grid().SizeKuttaNodeList(NumPlateI_);    
       
    }
    
    else {
       
       IsLiftingSurface_ = 0;

       Grid().SizeKuttaNodeList(0);    
       
    }       
    
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
    
    // Read in the surface ID
    
    for ( n = 1 ; n <= NumTris ; n++ ) {
       
       fscanf(CART3D_File,"%d \n",&SurfaceID);

       Grid().TriList(n).SurfaceID()         = SurfaceID;
        
       Grid().TriList(n).SurfaceType()       = CART3D_SURFACE;
       
       Grid().TriList(n).DegenBodyID()       = 0;
       
       Grid().TriList(n).DegenWingID()       = 0;  
       
       Grid().TriList(n).Cart3dID()          = SurfaceID;       
       
       Grid().TriList(n).NxCamber()          = 0.;
       
       Grid().TriList(n).NyCamber()          = 0.;
       
       Grid().TriList(n).NzCamber()          = 0.;
       
       Grid().TriList(n).SpanStation()       = SurfaceID;

       Grid().TriList(n).IsTrailingEdgeTri() = 0;
       
       Grid().TriList(n).IsLeadingEdgeTri()  = 0;
   
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
    
    printf("NumberOfSurfacePatches_: %d \n",NumberOfSurfacePatches_);
    
    
    delete [] SurfaceList;
    
    // Calculate tri normals and build edge data structure

    Grid().CalculateTriNormalsAndCentroids();
   
    Grid().CreateTriEdges();
    
    Grid().CalculateUpwindEdges();  
    
    // Now find sharp trailing edges
    
    FindSharpEdges();
    
}

/*##############################################################################
#                                                                              #
#                          VSP_SURFACE FindSharpEdges                          #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::FindSharpEdges(void)
{

    int i, j, k, p, Loop1, Loop2, Node, Node1, Node2, Node3, Done;
    int NumberOfKuttaEdges, NumberOfKuttaNodes;
    int Edge, Edge1, Edge2, Next, VortexSheet, SheetIsPeriodic;
    int *KuttaEdgeList, *IncidentKuttaEdges, *IsKuttaEdge, *PermArray, *NodeUsed;
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
    
    // Find all possible wake edges
    
    IsKuttaEdge = new int[Grid().NumberOfEdges() + 1];
    
    zero_int_array(IsKuttaEdge, Grid().NumberOfEdges());
        
    IncidentKuttaEdges = new int[Grid().NumberOfNodes() + 1];
    
    zero_int_array(IncidentKuttaEdges, Grid().NumberOfNodes());
    
    Xvec[0] = 1.;
    Xvec[1] = 0.;
    Xvec[2] = 0.;
    
    for ( i = 1 ; i <= Grid().NumberOfEdges() ; i++ ) {
       
       // Check if edge is mostly normal to the free stream flow
       
       Node1 = Grid().EdgeList(i).Node1();
       Node2 = Grid().EdgeList(i).Node2();
       
       vec1[0] = Grid().NodeList(Node2).x() - Grid().NodeList(Node1).x();
       vec1[1] = Grid().NodeList(Node2).y() - Grid().NodeList(Node1).y();
       vec1[2] = Grid().NodeList(Node2).z() - Grid().NodeList(Node1).z();
       
       dot = sqrt(vector_dot(vec1,vec1));
       
       vec1[0] /= dot;
       vec1[1] /= dot;
       vec1[2] /= dot;
       
       dot = ABS(vector_dot(Xvec, vec1));

       if ( dot <= 0.90 ) {

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
   
          // If angle greater than 120 degrees, mark nodes as sharp
   
          if ( angle > 90. ) {
   
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
      
               angle = (180./3.141592)*acos(MAX(MIN(vec3[0],1.),-1.));
      
               // Check if the normal points aft
      
               if ( angle < 90. ) {
                  
                  IncidentKuttaEdges[Node1]++;
                  
                  IncidentKuttaEdges[Node2]++;
            
                  IsKuttaEdge[i] = 1;
                 
                }
                
             }
      
          }
          
       }

    }
    
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

    // Size the kutta node list

    Grid().SizeKuttaNodeList(NumberOfKuttaNodes);  
    
    // Mark the kutta edges
    
    for ( i = 1 ; i <= NumberOfKuttaEdges ; i++ ) {
       
       Grid().EdgeList(KuttaEdgeList[i]).IsTrailingEdge() = 1;
       
    }
    
    // Iterate until all the kutta nodes are assigned to a vortex sheet
    
    NodeUsed = new int[Grid().NumberOfNodes() + 1];

    zero_int_array(NodeUsed, Grid().NumberOfNodes());
    
    PermArray = new int[NumberOfKuttaNodes + 1];
  
    p = 0;
    
    VortexSheet = 1;
    
    printf("The NumberOfKuttaNodes: %d \n",NumberOfKuttaNodes);
    
    while ( p < NumberOfKuttaNodes ) {
       
       printf("Working on VortexSheet: %d \n",VortexSheet);fflush(NULL);
       
       SheetIsPeriodic = 0;
       
       // Try to start with a wing tip node
       
       Done = 0;
       
       k = 1;
   
       while ( k <= Grid().NumberOfNodes() && !Done ) {
       
          if ( KuttaNodeList[k].IsKuttaNode && IncidentKuttaEdges[k] == 1 && NodeUsed[k] == 0 ) Done = 1;
          
          k++;
          
       }
       
       k--;
       
       if ( Done ) printf("Found a wing tip node... \n");fflush(NULL);
       
       // If we did not find a wing tip node... then this wake is either closed like a nacelle,
       // or it could be something like a pylon with no tips... so now we have to figure that out
       
       // Find any ones >= 3 
       
       // Find an kutta edge that only has one node marked as a kutta node
       
       if ( !Done ) {
          
          printf("Did not find a wing tip node... \n");fflush(NULL);
          
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
          
          printf("Did not find any singly marked kutta node... \n");fflush(NULL);
          
          k = 1;
   
          while ( k <= Grid().NumberOfNodes() && !Done ) {
          
             if ( KuttaNodeList[k].IsKuttaNode && NodeUsed[k] == 0 ) Done = 1;
             
             k++;
             
          }
          
          k--;
          
          SheetIsPeriodic = 1;
          
          if ( SheetIsPeriodic) printf("Sheet is periodic... \n");fflush(NULL);
          
       }
       
       printf("Done: %d \n",Done);fflush(NULL);
       
       // Loop over kutta nodes and sort
       
       zero_int_array(PermArray, NumberOfKuttaNodes);
   
       Done = 0;
       
       Next = 1;
       
       Node = k;
       
       PermArray[Next] = Node;
       
       NodeUsed[Node] = 1;
    
       while ( !Done ) {
          
          Done = 1;
          
          Node = PermArray[Next];
          
          Edge1 = KuttaNodeList[Node].Edge1;
          Edge2 = KuttaNodeList[Node].Edge2;
          
       //   printf("Edge1, Edge2: %d %d .... Node1, Node2: %d %d \n",Edge1,Edge2,Node1,Node2);
   
          Node1 = Grid().EdgeList(Edge1).Node1() + Grid().EdgeList(Edge1).Node2() - Node;
          Node2 = Grid().EdgeList(Edge2).Node1() + Grid().EdgeList(Edge2).Node2() - Node;
          
          if ( Edge1 > 0 && KuttaNodeList[Node1].IsKuttaNode && NodeUsed[Node1] == 0 ) {
             
             PermArray[++Next] = Node1;
             
             NodeUsed[Node1] = 1;
             
             Done = 0;
             
          }
          
          if ( Edge2 > 0 && KuttaNodeList[Node2].IsKuttaNode && NodeUsed[Node2] == 0 ) {
             
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
             
             Grid().WingSurface(p) = VortexSheet;
           
             Grid().WingSurfaceIsPeriodic(p) = SheetIsPeriodic;
            
             Grid().WakeTrailingEdgeX(p) = Grid().NodeList(Node).x();
             Grid().WakeTrailingEdgeY(p) = Grid().NodeList(Node).y();
             Grid().WakeTrailingEdgeZ(p) = Grid().NodeList(Node).z();
                   
             Grid().NodeList(Node).IsTrailingEdgeNode() = 1;
             
          }
   
       }   
       
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
    
    // Finally, mark edges that are on an open boundary. I am
    // assuming these are on a symmetry plane
    
    for ( i = 1 ; i <= Grid().NumberOfEdges() ; i++ ) {

       Loop1 = Grid().EdgeList(i).Loop1();
       Loop2 = Grid().EdgeList(i).Loop2();

       if ( Loop1 == Loop2) {
          
          Grid().EdgeList(i).IsBoundaryEdge() = 1; // djk 2
 
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
    double Vec[3], VecAvg[3], P1[3], P2[3], Dot, Angle;
   
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
    double x, y, z, u, v;
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
 
    int i, j, NumI, NumJ, Wing, Done, HingeNode[2];
    double DumFloat, zCamber, Vec[3], VecQC_1[3], VecQC_2[3], Mag, HingeVec[3];
    double x1, y1, z1, x2, y2, z2, ArcLength, Chord, up[4], wp[4], xyz[3];
    char DumChar[2000], Stuff[2000], LastSubSurf[2000], Comma[2000], *Next;

    // Save the component name
    
    sprintf(ComponentName_,"%s",Name);
    
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
          
          sscanf(DumChar,"%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf",
                 &x_plate(i,j),
                 &y_plate(i,j),
                 &z_plate(i,j),
                 &zCamber,
                 &DumFloat,
                 &Nx_plate(i,j),
                 &Ny_plate(i,j),
                 &Nz_plate(i,j),
                 &u_plate(i,j),
                 &DumFloat,
                 &v_plate(i,j));
         
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
                     
    // Check if there is subsurface data

    Done = NumberOfControlSurfaces_ = 0.;
    
    sprintf(LastSubSurf," ");
    
    sprintf(Comma,",");
    
    while ( !Done ) {
       
       fgets(DumChar,1000,VSP_Degen_File);
       
       if ( Verbose_ ) printf("DumChar: %s \n",DumChar);
       
       if ( strstr(DumChar,"DegenGeom") != NULL ) {
       
          fgets(DumChar,1000,VSP_Degen_File);
          
          if ( strstr(DumChar,"SUBSURF") != NULL ) {
        
             if ( Verbose_ ) printf("DumChar: %s .... LastSubSurf: %s \n",DumChar,LastSubSurf);
        
             // Skip over possible second instance of this control surface.. it may be defined on both the top and bottom surfaces
             
             if ( strcmp(DumChar,LastSubSurf) != 0 ) {
                
                sprintf(LastSubSurf,"%s",DumChar);
             
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
                
                sprintf(ControlSurface_[NumberOfControlSurfaces_].Name(),"%s\0",Next);
                
                if ( Verbose_ ) printf("Control Surface Name: %s \n",ControlSurface_[NumberOfControlSurfaces_].Name());
                
                fgets(DumChar,1000,VSP_Degen_File);
                fgets(DumChar,1000,VSP_Degen_File);
                fgets(DumChar,1000,VSP_Degen_File);
                fgets(DumChar,1000,VSP_Degen_File);
                fgets(DumChar,1000,VSP_Degen_File);
                
                fgets(DumChar,1000,VSP_Degen_File); sscanf(DumChar,"%lf, %lf",&(up[0]),&(wp[0])); if ( Verbose_ ) printf("up,wp: %lf %lf \n",up[0],wp[0]);
                fgets(DumChar,1000,VSP_Degen_File); sscanf(DumChar,"%lf, %lf",&(up[1]),&(wp[1])); if ( Verbose_ ) printf("up,wp: %lf %lf \n",up[1],wp[1]);
                fgets(DumChar,1000,VSP_Degen_File); sscanf(DumChar,"%lf, %lf",&(up[2]),&(wp[2])); if ( Verbose_ ) printf("up,wp: %lf %lf \n",up[2],wp[2]);
                fgets(DumChar,1000,VSP_Degen_File); sscanf(DumChar,"%lf, %lf",&(up[3]),&(wp[3])); if ( Verbose_ ) printf("up,wp: %lf %lf \n",up[3],wp[3]);
                fgets(DumChar,1000,VSP_Degen_File);
                
                // If control surface definition is on the upper surface, transform to the lower surface
                
                if ( wp[0] > 2. ) wp[0] = 4. - wp[0]; 
                if ( wp[1] > 2. ) wp[1] = 4. - wp[1]; 
                if ( wp[2] > 2. ) wp[2] = 4. - wp[2]; 
                if ( wp[3] > 2. ) wp[3] = 4. - wp[3]; 
                
                // Bounding box for control surface
                
                ControlSurface_[NumberOfControlSurfaces_].u_min() = MIN4(up[0],up[1],up[2],up[3]);
                ControlSurface_[NumberOfControlSurfaces_].u_max() = MAX4(up[0],up[1],up[2],up[3]);
                ControlSurface_[NumberOfControlSurfaces_].v_min() = MIN4(wp[0],wp[1],wp[2],wp[3]);
                ControlSurface_[NumberOfControlSurfaces_].v_max() = MAX4(wp[0],wp[1],wp[2],wp[3]);
                
                // XYZ coordinates of control surface box

                Interpolate_XYZ_From_UV(up[0],wp[0],xyz);
                
                ControlSurface_[NumberOfControlSurfaces_].Node_1(0) = xyz[0];
                ControlSurface_[NumberOfControlSurfaces_].Node_1(1) = xyz[1];
                ControlSurface_[NumberOfControlSurfaces_].Node_1(2) = xyz[2];
                        
                Interpolate_XYZ_From_UV(up[1],wp[1],xyz);
                
                ControlSurface_[NumberOfControlSurfaces_].Node_2(0) = xyz[0];
                ControlSurface_[NumberOfControlSurfaces_].Node_2(1) = xyz[1];
                ControlSurface_[NumberOfControlSurfaces_].Node_2(2) = xyz[2];
            
                Interpolate_XYZ_From_UV(up[2],wp[2],xyz);
                
                ControlSurface_[NumberOfControlSurfaces_].Node_3(0) = xyz[0];
                ControlSurface_[NumberOfControlSurfaces_].Node_3(1) = xyz[1];
                ControlSurface_[NumberOfControlSurfaces_].Node_3(2) = xyz[2];
              
                Interpolate_XYZ_From_UV(up[3],wp[3],xyz);
                
                ControlSurface_[NumberOfControlSurfaces_].Node_4(0) = xyz[0];
                ControlSurface_[NumberOfControlSurfaces_].Node_4(1) = xyz[1];
                ControlSurface_[NumberOfControlSurfaces_].Node_4(2) = xyz[2];
                
                // Determine hinge line
                
                LocateHingeLine(up, wp, HingeNode);
                                                          
                // Hinge point 1
                
                Interpolate_XYZ_From_UV(up[HingeNode[0]],wp[HingeNode[0]],xyz);
                
                HingeVec[0] = -xyz[0];
                HingeVec[1] = -xyz[1];
                HingeVec[2] = -xyz[2];
     
                ControlSurface_[NumberOfControlSurfaces_].HingeNode_1(0) = xyz[0];
                ControlSurface_[NumberOfControlSurfaces_].HingeNode_1(1) = xyz[1];
                ControlSurface_[NumberOfControlSurfaces_].HingeNode_1(2) = xyz[2];
          
                if ( Verbose_ ) printf("Hinge Point 1: %lf %lf %lf \n",xyz[0],xyz[1],xyz[2]);
                
                // Hinge point 2
                
                Interpolate_XYZ_From_UV(up[HingeNode[1]],wp[HingeNode[1]],xyz);
                
                ControlSurface_[NumberOfControlSurfaces_].HingeNode_2(0) = xyz[0];
                ControlSurface_[NumberOfControlSurfaces_].HingeNode_2(1) = xyz[1];
                ControlSurface_[NumberOfControlSurfaces_].HingeNode_2(2) = xyz[2];
                
                if ( Verbose_ ) printf("Hinge Point 2: %lf %lf %lf \n",xyz[0],xyz[1],xyz[2]);
                
                HingeVec[0] += xyz[0];
                HingeVec[1] += xyz[1];
                HingeVec[2] += xyz[2];
                                
                // Hinge Vector
                
                Mag = sqrt(vector_dot(HingeVec,HingeVec));
                
                HingeVec[0] /= Mag;
                HingeVec[1] /= Mag;
                HingeVec[2] /= Mag;        

                ControlSurface_[NumberOfControlSurfaces_].HingeVec(0) = HingeVec[0];
                ControlSurface_[NumberOfControlSurfaces_].HingeVec(1) = HingeVec[1];
                ControlSurface_[NumberOfControlSurfaces_].HingeVec(2) = HingeVec[2];
                           
             }
             
             else {

                fgets(DumChar,1000,VSP_Degen_File);
                fgets(DumChar,1000,VSP_Degen_File);
                fgets(DumChar,1000,VSP_Degen_File);
                fgets(DumChar,1000,VSP_Degen_File);
                fgets(DumChar,1000,VSP_Degen_File);
                
                fgets(DumChar,1000,VSP_Degen_File); 
                fgets(DumChar,1000,VSP_Degen_File);
                fgets(DumChar,1000,VSP_Degen_File);
                fgets(DumChar,1000,VSP_Degen_File);
                fgets(DumChar,1000,VSP_Degen_File);                
                
             }
             
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

    i = NumPlateI_ ; j = 1;

    x1 = x_plate(i,j);
    y1 = y_plate(i,j);
    z1 = z_plate(i,j);

    i = NumPlateI_; j = NumPlateJ_;
    
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

void VSP_SURFACE::LocateHingeLine(const double *up, const double *wp, int *HingeNode)
{

    int i, Done, Iter, Node[4], iTemp;
    double u[4], w[4], dw[4], Temp[5];

    for ( i = 0 ; i <= 3 ; i++ ) {
       
       u[i] = up[i];
       w[i] = wp[i];
          
       dw[i] = ABS( 0.5 - w[i] );
       
       Node[i] = i;
      
    }
       
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
          
          printf("Failed to sort hinge line nodes! \n");fflush(NULL);
          exit(1);
          
       }
           
    }
    
    for ( i = 0 ; i <= 3 ; i++ ) {
       
       if ( Verbose_ ) printf("i: %d --> u,w,dw: %lf %lf %lf %d \n",i,u[i],w[i],dw[i],Node[i]);
       
    }    
    
    HingeNode[0] = Node[0];
    HingeNode[1] = Node[1];

}

/*##############################################################################
#                                                                              #
#                      VSP_SURFACE  ReadBodyDataFromFile                       #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::ReadBodyDataFromFile(char *Name, int Case, FILE *VSP_Degen_File)
{
 
    int i, j, k, NumI, NumJ, Done, jStart, jEnd;
    double DumFloat, Vec[3], VecQC_1[3], VecQC_2[3];
    double x1, y1, z1, x2, y2, z2, ArcLength;
    char DumChar[2000], Stuff[2000];

    // Save the component name

    sprintf(ComponentName_,"%s",Name);
    
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

    NumberOfSurfacePatches_ = 0;

    // Calculate local chord lengths

    AverageChord_ = 0.;
    
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

void VSP_SURFACE ::CheckForDegenerateXSections(void)
{
 
    int i, j, ii, *BadXSection, NumBadXSections, NumI;
    double  Distance, Tolerance, Ymin, Ymax, Zmin, Zmax, Span;
    double *x_plate_new, *y_plate_new, *z_plate_new;
    double *Nx_plate_new, *Ny_plate_new, *Nz_plate_new;
    double *u_plate_new, *v_plate_new;
    double *LocalChord_new;
    
    Tolerance = 1.e-6;
  
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
       
       u_plate_new = new double[NumPlateI_*NumPlateJ_ + 1];
       v_plate_new = new double[NumPlateI_*NumPlateJ_ + 1];

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
                
                u_plate_new[(ii-1)*NumPlateJ_ + j] =     u_plate_[(i-1)*NumPlateJ_ + j];
                v_plate_new[(ii-1)*NumPlateJ_ + j] = v_plate_[(i-1)*NumPlateJ_ + j];
          
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
       
       delete[] u_plate_;
       delete[] v_plate_;
  
       delete[] LocalChord_;

       x_plate_ = x_plate_new;
       y_plate_ = y_plate_new;
       z_plate_ = z_plate_new;
       
       Nx_plate_ = Nx_plate_new;
       Ny_plate_ = Ny_plate_new;
       Nz_plate_ = Nz_plate_new;
       
       u_plate_     = u_plate_new;
       v_plate_ = v_plate_new;
   
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
    double *u_plate_new, *v_plate_new;
    double *LocalChord_new;
    double x1, y1, z1, x2, y2, z2;
    
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
       
       u_plate_new = new double[NumPlateI_*NumPlateJ_ + 1];
       v_plate_new = new double[NumPlateI_*NumPlateJ_ + 1];
        
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
                
                u_plate_new[(ii-1)*NumPlateJ_ + j] = u_plate_[(i-1)*NumPlateJ_ + j];
                v_plate_new[(ii-1)*NumPlateJ_ + j] = v_plate_[(i-1)*NumPlateJ_ + j];
                               
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

       delete[] u_plate_;
       delete[] v_plate_;

       delete[] LocalChord_;

       x_plate_ = x_plate_new;
       y_plate_ = y_plate_new;
       z_plate_ = z_plate_new;
       
       Nx_plate_ = Nx_plate_new;
       Ny_plate_ = Ny_plate_new;
       Nz_plate_ = Nz_plate_new;
       
       u_plate_ = u_plate_new;
       v_plate_ = v_plate_new;
       
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
    
    if ( SurfaceType_ == DEGEN_WING_SURFACE ) CreateWingTriMesh(SurfaceID);
    
    if ( SurfaceType_ == DEGEN_BODY_SURFACE ) CreateBodyTriMesh(SurfaceID);
    
    // Surface type, and ID data at the tri / loop level
    
    for ( j = 1 ; j <= Grid().NumberOfTris() ; j++ ) {

      if ( SurfaceType_ == DEGEN_WING_SURFACE ) {
        
          Grid().TriList(j).SurfaceType() = DEGEN_WING_SURFACE;
          Grid().TriList(j).DegenBodyID() = 0;
          Grid().TriList(j).DegenWingID() = SurfaceID;  
          Grid().TriList(j).Cart3dID()    = 0;
          
       }
       
       else if ( SurfaceType_ == DEGEN_BODY_SURFACE ) {
        
          Grid().TriList(j).SurfaceType() = DEGEN_BODY_SURFACE;
          Grid().TriList(j).DegenBodyID() = SurfaceID;
          Grid().TriList(j).DegenWingID() = 0;  
          Grid().TriList(j).Cart3dID()    = 0;
          
       }
       
       else if ( SurfaceType_ == CART3D_SURFACE ) {
        
          Grid().TriList(j).SurfaceType() = CART3D_SURFACE;
          Grid().TriList(j).DegenBodyID() = 0;
          Grid().TriList(j).DegenWingID() = 0;  
          Grid().TriList(j).Cart3dID()    = SurfaceID;
          
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
    double x1, y1, z1, x2, y2, z2, Vec[3], VecQC_1[3], VecQC_2[3];
    double Chord, ArcLength;
        
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
          
          normal[0] = ( Nx_plate(i,j) + Nx_plate(i+1,j) + Nx_plate(i+1,j+1) ) / 3.;
          normal[1] = ( Ny_plate(i,j) + Ny_plate(i+1,j) + Ny_plate(i+1,j+1) ) / 3.;
          normal[2] = ( Nz_plate(i,j) + Nz_plate(i+1,j) + Nz_plate(i+1,j+1) ) / 3.;
          
          mag = sqrt(vector_dot(normal,normal));
          
          normal[0] /= mag;
          normal[1] /= mag;
          normal[2] /= mag;
          
          // Connectivity
          
          Grid().TriList(n).Node1() = node1;
          Grid().TriList(n).Node2() = node2;
          Grid().TriList(n).Node3() = node3;
          
          // Camber line normal
          
          Grid().TriList(n).NxCamber() = normal[0];
          Grid().TriList(n).NyCamber() = normal[1];
          Grid().TriList(n).NzCamber() = normal[2];
          
          // Surface ID
          
          Grid().TriList(n).SurfaceID() = SurfaceID;
          
          // Span station
          
          Grid().TriList(n).SpanStation() = i;
          
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
             
             if ( Verbose_ ) printf("Case 1---> \n");
             if ( Verbose_ ) printf("vec3: %f %f %f \n",vec3[0],vec3[1],vec3[2]);
             if ( Verbose_ ) printf("Normal: %f %f %f \n",normal[0],normal[1],normal[2]);
             
             Flipped = 1;
             
          }
          
          // Parametric UV centroid
          
          Grid().TriList(n).Uc() = ( u_plate(i,j) + u_plate(i+1,j) + u_plate(i+1,j+1) ) / 3.;
          Grid().TriList(n).Vc() = ( v_plate(i,j) + v_plate(i+1,j) + v_plate(i+1,j+1) ) / 3.;

          // Tri 2
          
          n++;
          
          // Normal

          normal[0] = ( Nx_plate(i,j) + Nx_plate(i+1,j+1) + Nx_plate(i,j+1) ) / 3.;
          normal[1] = ( Ny_plate(i,j) + Ny_plate(i+1,j+1) + Ny_plate(i,j+1) ) / 3.;
          normal[2] = ( Nz_plate(i,j) + Nz_plate(i+1,j+1) + Nz_plate(i,j+1) ) / 3.;
          
          mag = sqrt(vector_dot(normal,normal));
          
          normal[0] /= mag;
          normal[1] /= mag;
          normal[2] /= mag;
          
          // Connectivity
          
          Grid().TriList(n).Node1() = node1;
          Grid().TriList(n).Node2() = node3;
          Grid().TriList(n).Node3() = node4;
          
          // Camber line normal
         
          Grid().TriList(n).NxCamber() = normal[0];
          Grid().TriList(n).NyCamber() = normal[1];
          Grid().TriList(n).NzCamber() = normal[2];
          
          // Surface ID
          
          Grid().TriList(n).SurfaceID() = SurfaceID;
          
          // Span station
          
          Grid().TriList(n).SpanStation() = i;
          
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

             if ( Verbose_ ) printf("Case 2---> \n");
             if ( Verbose_ ) printf("vec3: %f %f %f \n",vec3[0],vec3[1],vec3[2]);
             if ( Verbose_ ) printf("Normal: %f %f %f \n",normal[0],normal[1],normal[2]);
             
             Flipped = 1;
             
          }

          // Parametric UV centroid
          
          Grid().TriList(n).Uc() = ( u_plate(i,j) + u_plate(i+1,j) + u_plate(i+1,j+1) ) / 3.;
          Grid().TriList(n).Vc() = ( v_plate(i,j) + v_plate(i+1,j) + v_plate(i+1,j+1) ) / 3.;
                    
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
    double Vec[3], VecQC_1[3], VecQC_2[3], Chord, ArcLength;

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
             
             // Camber line normal
               
             Grid().TriList(n).NxCamber() = normal[0];
             Grid().TriList(n).NyCamber() = normal[1];
             Grid().TriList(n).NzCamber() = normal[2];
             
             // Surface ID

             Grid().TriList(n).SurfaceID() = SurfaceID;
             
             // Span station
             
             Grid().TriList(n).SpanStation() = i;
             
             // Leading and trailing edge flags
     
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

             // Camber line normal
             
             Grid().TriList(n).NxCamber() = normal[0];
             Grid().TriList(n).NyCamber() = normal[1];
             Grid().TriList(n).NzCamber() = normal[2];
             
             // Surface ID

             Grid().TriList(n).SurfaceID() = SurfaceID;
             
             // Span station
             
             Grid().TriList(n).SpanStation() = i;
             
             // Leading and trailing edge flags
     
             Grid().TriList(n).IsTrailingEdgeTri() = 0;
             
             Grid().TriList(n).IsLeadingEdgeTri() = 0;
             
          }
      
       }
              
    }  
    
    if ( n != NumTris ) {
     
       printf("Error in determining the number of valid tris in body mesh! \n"); fflush(NULL);
       
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
    double VortexLoop1DownWindWeight, VortexLoop2DownWindWeight;

    VortexLoop1DownWindWeight = VortexLoop2DownWindWeight = 0;

    VSP_NODE VSP_Node1, VSP_Node2;
    
    // Setup the surface vortex edge data
   
    for ( j = 1 ; j <= Grid().NumberOfEdges() ; j++ ) {

       // Pointers from vortex edge back to the parent wing or body and grid edge
       
       if ( SurfaceType() == DEGEN_WING_SURFACE ) {
       
          Grid().EdgeList(j).DegenWing() = SurfaceID;
          
          Grid().EdgeList(j).DegenBody() = 0;
          
          Grid().EdgeList(j).Cart3DSurface() = 0;

       }
       
       else if ( SurfaceType() == DEGEN_BODY_SURFACE ) {
   
          Grid().EdgeList(j).DegenWing() = 0;
          
          Grid().EdgeList(j).DegenBody() = SurfaceID;
          
          Grid().EdgeList(j).Cart3DSurface() = 0;

       }
       
       else if ( SurfaceType() == CART3D_SURFACE ) {
   
          Grid().EdgeList(j).DegenWing() = 0;
          
          Grid().EdgeList(j).DegenBody() = 0;
          
          Grid().EdgeList(j).Cart3DSurface() = SurfaceID;

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
 
    if ( SurfaceType_ == DEGEN_WING_SURFACE ) printf("Wing Grid:%d --> Number of loops: %d \n",0,Grid_[0]->NumberOfTris());
    if ( SurfaceType_ == DEGEN_BODY_SURFACE ) printf("Body Grid:%d --> Number of loops: %d \n",0,Grid_[0]->NumberOfTris());
    
    // First attempt to simplify the grid

    Grid_[1] = Agglomerate.SimplifyMesh(*(Grid_[0]));

    if ( SurfaceType_ == DEGEN_WING_SURFACE ) printf("Wing Grid:%d --> Number of loops: %d \n",1,Grid_[1]->NumberOfTris());
    if ( SurfaceType_ == DEGEN_BODY_SURFACE ) printf("Body Grid:%d --> Number of loops: %d \n",1,Grid_[1]->NumberOfTris());
        
    i = 2;
    
    while ( i < MaxNumberOfGridLevels_ && Grid_[i-1]->NumberOfTris() > 1 ) {

       Grid_[i] = Agglomerate.Agglomerate(*(Grid_[i-1]));
       
       if ( SurfaceType_ == DEGEN_WING_SURFACE ) printf("Wing Grid:%d --> Number of loops: %d \n",i,Grid_[i]->NumberOfTris());
       if ( SurfaceType_ == DEGEN_BODY_SURFACE ) printf("Body Grid:%d --> Number of loops: %d \n",i,Grid_[i]->NumberOfTris());
       
       i++;

    }
    
    NumberOfGridLevels_ = i;
    
    printf("NumberOfGridLevels_: %d \n",NumberOfGridLevels_);
 
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

       printf("Could not open the FEM deformation file for input! \n");

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
    double xw, yw, zw, xr, yr, zr;
    double xw1, yw1, zw1, xw2, yw2, zw2, xw3, yw3, zw3, xwp, ywp, zwp;   
    double delta_xv, delta_yv, delta_zv;
    double delta_phix, delta_phiy, delta_phiz;

    for ( i = 1 ; i <= NumPlateI_ ; i++ ) {
     
       for ( j = 1 ; j <= NumPlateJ_ ; j++ ) {

          xw = x_plate(i,j);
          yw = y_plate(i,j);
          zw = z_plate(i,j);   

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

void VSP_SURFACE::FEMUnDeformGeometry(double xyz_p[3])
{
   
    double xw, yw, zw, xr, yr, zr;
    double xw1, yw1, zw1, xw2, yw2, zw2, xw3, yw3, zw3;   
    double delta_xv, delta_yv, delta_zv;
    double delta_phix, delta_phiy, delta_phiz;

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
#                     VSP_SOLVER Interpolate_XYZ_From_UV                       #
#                                                                              #
##############################################################################*/

void VSP_SURFACE::Interpolate_XYZ_From_UV(double u, double v, double *xyz)
{
   
   double Cu, Cv, N1, N2, N3, N4;
   int i, j, Found;
   
   // Find i
   
   i = Found = 0;
   
   if ( u <= u_plate(1         ,1) ) { i =          1 ; Found = 1; };
   if ( u >= u_plate(NumPlateI_,1) ) { i = NumPlateI_ ; Found = 1; };
   
   while ( i < NumPlateI_ && !Found ) {
      
      i++;
      
    //  printf("i: %d: %f -------- %f ----------- %f \n",i,u_plate(i,1),u,u_plate(i+1,1));
      
      if ( u_plate(i,1) <= u && u_plate(i+1,1) >= u ) Found = 1;
       
   }
   
   if ( !Found ) { printf("u not found! \n");  fflush(NULL); exit(1); }
   
   Cu = ( u - u_plate(i,1) ) / ( u_plate(i+1,1) - u_plate(i,1) );
   
 //  printf("Cu: %f \n",Cu);
   
   // Find j
   
   j = Found = 0;
   
   if ( v <= v_plate(1,         1) ) { j =          1 ; Found = 1; };
   if ( v >= v_plate(1,NumPlateJ_) ) { j = NumPlateJ_ ; Found = 1; };   
   
   while ( j < NumPlateJ_ && !Found ) {
      
      j++;
      
   //   printf("j: %d: %f -------- %f ----------- %f \n",j,v_plate(1,j),v,v_plate(1,j+1));
      
      if ( v_plate(1,j) <= v && v_plate(1,j+1) >= v ) Found = 1;
       
   }   
   
   Cv = ( v - v_plate(1,j) ) / ( v_plate(1,j+1) - v_plate(1,j) );  

   if ( !Found ) { printf("v not found! \n"); fflush(NULL); exit(1); }
   
   // Interpolate for xyz
   
 //  printf("Cu,Cv: %f %f \n",Cu,Cv);
   
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





