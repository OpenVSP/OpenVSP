//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Grid.H"

/*##############################################################################
#                                                                              #
#                              VSP_GRID constructor                            #
#                                                                              #
##############################################################################*/

VSP_GRID::VSP_GRID(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                                VSP_GRID init                                 #
#                                                                              #
##############################################################################*/

void VSP_GRID::init(void)
{

    NumberOfNodes_ = 0;

    NodeList_ = NULL;

    NumberOfLoops_ = 0;

    LoopList_ = NULL;
    
    NumberOfEdges_ = 0;
    
    EdgeList_ = NULL;
    
    NumberOfKuttaNodes_ = 0;
    
    KuttaNode_ = NULL;
    
    WingSurfaceForKuttaNode_ = NULL;
    
    WingSurfaceForKuttaNodeIsPeriodic_ = NULL;   
    
    ComponentIDForKuttaNode_ = NULL;
    
    KuttaNodeIsOnWingTip_ = NULL;
    
    SurfaceType_ = 0;
    
    ComponentID_ = 0;
    
    Verbose_ = 0;

}

/*##############################################################################
#                                                                              #
#                          VSP_GRID SizeNodeList                               #
#                                                                              #
##############################################################################*/

void VSP_GRID::SizeNodeList(int NumNodes)
{

    NumberOfNodes_ = NumNodes;

    NodeList_ = new VSP_NODE[NumberOfNodes_ + 1];

}

/*##############################################################################
#                                                                              #
#                          VSP_GRID SizeTriList                                #
#                                                                              #
##############################################################################*/

void VSP_GRID::SizeTriList(int NumTris)
{

    int i;
    
    NumberOfLoops_ = NumTris;

    LoopList_ = new VSP_LOOP[NumberOfLoops_ + 1];
    
    for ( i = 1 ; i <= NumberOfLoops_ ; i++ ) {
 
       LoopList_[i].SizeNodeList(3);
     
       LoopList_[i].SizeEdgeList(3);
       
    }

}

/*##############################################################################
#                                                                              #
#                          VSP_GRID SizeLoopList                               #
#                                                                              #
##############################################################################*/

void VSP_GRID::SizeLoopList(int NumLoops)
{

    NumberOfLoops_ = NumLoops;

    LoopList_ = new VSP_LOOP[NumberOfLoops_ + 1];

}

/*##############################################################################
#                                                                              #
#                          VSP_GRID SizeEdgeList                               #
#                                                                              #
##############################################################################*/

void VSP_GRID::SizeEdgeList(int NumEdges)
{

    NumberOfEdges_ = NumEdges;

    EdgeList_ = new VSP_EDGE[NumberOfEdges_ + 1];

}

/*##############################################################################
#                                                                              #
#                          VSP_GRID SizeKuttaNodeList                          #
#                                                                              #
##############################################################################*/

void VSP_GRID::SizeKuttaNodeList(int NumberOfKuttaNodes)
{

    NumberOfKuttaNodes_ = NumberOfKuttaNodes;

    KuttaNode_ = new int[NumberOfKuttaNodes_ + 1];
    
    WingSurfaceForKuttaNode_ = new int[NumberOfKuttaNodes_ + 1];
    
    WingSurfaceForKuttaNodeIsPeriodic_ = new int[NumberOfKuttaNodes_ + 1];
    
    ComponentIDForKuttaNode_ = new int[NumberOfKuttaNodes_ + 1];
    
    KuttaNodeIsOnWingTip_ = new int[NumberOfKuttaNodes_ + 1];
    
    zero_int_array(KuttaNode_ ,NumberOfKuttaNodes_);
    zero_int_array(WingSurfaceForKuttaNode_ ,NumberOfKuttaNodes_);
    zero_int_array(WingSurfaceForKuttaNodeIsPeriodic_ ,NumberOfKuttaNodes_);
    zero_int_array(ComponentIDForKuttaNode_ ,NumberOfKuttaNodes_);
    zero_int_array(KuttaNodeIsOnWingTip_ ,NumberOfKuttaNodes_);
    
    WakeTrailingEdgeX_ = new VSPAERO_DOUBLE[NumberOfKuttaNodes_ + 1];
    WakeTrailingEdgeY_ = new VSPAERO_DOUBLE[NumberOfKuttaNodes_ + 1];
    WakeTrailingEdgeZ_ = new VSPAERO_DOUBLE[NumberOfKuttaNodes_ + 1];       
    
    KuttaNodeSoverB_ = new VSPAERO_DOUBLE[NumberOfKuttaNodes_ + 1];       

}

/*##############################################################################
#                                                                              #
#                                 VSP_GRID Copy                                #
#                                                                              #
##############################################################################*/

VSP_GRID::VSP_GRID(const VSP_GRID &VSPGrid)
{

    PRINTF("Copy not implemented for VSP_GRID! \n");

    exit(1);

}

/*##############################################################################
#                                                                              #
#                               VSP_GRID destructor                            #
#                                                                              #
##############################################################################*/

VSP_GRID::~VSP_GRID(void)
{

    NumberOfNodes_ = 0;

    if ( NodeList_ != NULL ) delete [] NodeList_;

    NumberOfLoops_ = 0;

    if ( LoopList_ != NULL ) delete [] LoopList_;

    NumberOfEdges_ = 0;

    if ( EdgeList_ != NULL ) delete [] EdgeList_;
     
}

/*##############################################################################
#                                                                              #
#                   VSP_GRID CalculateTriNormalsAndCentroids                   #
#                                                                              #
##############################################################################*/

void VSP_GRID::CalculateTriNormalsAndCentroids(void)
{
 
    int i, Node1, Node2, Node3;
    VSPAERO_DOUBLE vec1[3], vec2[3], vec3[3], mag;
    
    for ( i = 1 ; i <= NumberOfLoops_ ; i++ ) {
     
       Node1 = LoopList(i).Node1();
       Node2 = LoopList(i).Node2();
       Node3 = LoopList(i).Node3();
       
       // Normal and Areas
       
       vec1[0] = NodeList(Node2).x() - NodeList(Node1).x();
       vec1[1] = NodeList(Node2).y() - NodeList(Node1).y();
       vec1[2] = NodeList(Node2).z() - NodeList(Node1).z();

       vec2[0] = NodeList(Node3).x() - NodeList(Node1).x();
       vec2[1] = NodeList(Node3).y() - NodeList(Node1).y();
       vec2[2] = NodeList(Node3).z() - NodeList(Node1).z();
       
       vector_cross(vec1,vec2,vec3);
       
       mag = sqrt(vector_dot(vec3,vec3));
       
       vec3[0] /= mag;
       vec3[1] /= mag;
       vec3[2] /= mag;
       
       if ( mag <= 0. ) PRINTF("Mag: %lf \n",mag);
       
       LoopList(i).Nx() = vec3[0];
       LoopList(i).Ny() = vec3[1];
       LoopList(i).Nz() = vec3[2];
       
       LoopList(i).Area() = 0.5*mag;

       // Centroid
       
       LoopList(i).Xc() = ( NodeList(Node1).x() + NodeList(Node2).x() + NodeList(Node3).x() )/3.;
       LoopList(i).Yc() = ( NodeList(Node1).y() + NodeList(Node2).y() + NodeList(Node3).y() )/3.;
       LoopList(i).Zc() = ( NodeList(Node1).z() + NodeList(Node2).z() + NodeList(Node3).z() )/3.;
       
       // Bounding box information
       
       LoopList(i).BoundBox().x_min = MIN3(NodeList(Node1).x(), NodeList(Node2).x(), NodeList(Node3).x() );
       LoopList(i).BoundBox().x_max = MAX3(NodeList(Node1).x(), NodeList(Node2).x(), NodeList(Node3).x() );

       LoopList(i).BoundBox().y_min = MIN3(NodeList(Node1).y(), NodeList(Node2).y(), NodeList(Node3).y() );
       LoopList(i).BoundBox().y_max = MAX3(NodeList(Node1).y(), NodeList(Node2).y(), NodeList(Node3).y() );

       LoopList(i).BoundBox().z_min = MIN3(NodeList(Node1).z(), NodeList(Node2).z(), NodeList(Node3).z() );
       LoopList(i).BoundBox().z_max = MAX3(NodeList(Node1).z(), NodeList(Node2).z(), NodeList(Node3).z() );
       
       box_calculate_size(LoopList(i).BoundBox());

    }
       
}

/*##############################################################################
#                                                                              #
#                           VSP_GRID CreateTriEdges                            #
#                                                                              #
##############################################################################*/

void VSP_GRID::CreateTriEdges(void)
{

    int i, j, k, nod1, nod2, noda, nodb, start_edge, Node1, Node2, Edge;
    int level, edge_to_node[4][3], nod_list[4], Tri1, Tri2, Node;
    int max_edge, new_edge, *jump_pnt;
    VSPAERO_DOUBLE x1, y1, z1, x2, y2, z2, Normal[3], Dot;
    EDGE_ENTRY *list, *tlist;

    if ( Verbose_ ) PRINTF("Finding tri edges... \n");

    // Make space for a linked list of edges

    jump_pnt = new int[NumberOfNodes() + 1];

    max_edge = 4*NumberOfNodes();

    list = new EDGE_ENTRY[max_edge + 1];

    // Zero out the lists

    for ( i = 0 ; i <= NumberOfNodes() ; i++ ) {

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

    edge_to_node[3][1] = 3;
    edge_to_node[3][2] = 1;

    // Loop over triangles and create the edge list

    for ( j = 1 ; j <= NumberOfTris() ; j++ ) {

       // Local copy of node pointers

       nod_list[1] = LoopList(j).Node1();
       nod_list[2] = LoopList(j).Node2();
       nod_list[3] = LoopList(j).Node3();

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

             // Make sure there is enough room, if not reallocate space

             if ( new_edge >= max_edge ) {

                max_edge = (int) (1.2*max_edge);

                tlist = new EDGE_ENTRY[max_edge + 1];

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

                if ( i == 1 ) LoopList(j).Edge1() =  new_edge;
                if ( i == 2 ) LoopList(j).Edge2() =  new_edge;
                if ( i == 3 ) LoopList(j).Edge3() =  new_edge;

             }

             else {

                if ( i == 1 ) LoopList(j).Edge1() = -new_edge;
                if ( i == 2 ) LoopList(j).Edge2() = -new_edge;
                if ( i == 3 ) LoopList(j).Edge3() = -new_edge;

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

                   if ( i == 1 ) LoopList(j).Edge1() =  level;
                   if ( i == 2 ) LoopList(j).Edge2() =  level;
                   if ( i == 3 ) LoopList(j).Edge3() =  level;

                }

                else {

                   if ( i == 1 ) LoopList(j).Edge1() = -level;
                   if ( i == 2 ) LoopList(j).Edge2() = -level;
                   if ( i == 3 ) LoopList(j).Edge3() = -level;

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

                // Make sure there is enough room, if not reallocate space

                if ( new_edge >= max_edge ) {

                   max_edge = (int) (1.2*max_edge);

                   tlist = new EDGE_ENTRY[max_edge + 1];

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

                   if ( i == 1 ) LoopList(j).Edge1() =  new_edge;
                   if ( i == 2 ) LoopList(j).Edge2() =  new_edge;
                   if ( i == 3 ) LoopList(j).Edge3() =  new_edge;

                }

                else {

                   if ( i == 1 ) LoopList(j).Edge1() = -new_edge;
                   if ( i == 2 ) LoopList(j).Edge2() = -new_edge;
                   if ( i == 3 ) LoopList(j).Edge3() = -new_edge;

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

       SizeEdgeList(new_edge);

       // Fill the edge list

       for ( i = 1 ; i <= NumberOfNodes() ; i++ ) {

          level = jump_pnt[i];

          while ( level != 0 ) {

             EdgeList(level).Node1() = i;
             
             EdgeList(level).Node2() = list[level].node;

             level = list[level].next;

          }

       }

    }
 
   // Zero out leading, trailing edge information
    
    for ( j = 1 ; j <= NumberOfEdges() ; j++ ) {
     
       EdgeList(j).IsTrailingEdge() = 0;
       EdgeList(j).IsLeadingEdge()  = 0;
       EdgeList(j).IsBoundaryEdge() = 0;
       
    }    

    // Store edge to tri pointers

    for ( i = 1 ; i <= NumberOfNodes() ; i++ ) {

       level = jump_pnt[i];

       while ( level != 0 ) {

          if ( list[level].tri_1 != 0 ) {

             // Store pointers to tris on both sides of edge

             if ( list[level].tri_2 != 0) {

                EdgeList(level).Tri1() = list[level].tri_1;
                EdgeList(level).Tri2() = list[level].tri_2;

             }

             // Store pointers to tris on just the first side

             else {

                EdgeList(level).Tri1() = list[level].tri_1;
                EdgeList(level).Tri2() = list[level].tri_1;
                
                EdgeList(level).IsBoundaryEdge() = 2; // djk 2

             }

          }

          level = list[level].next;

       }

    }

    if ( Verbose_ ) PRINTF("Number of nodes: %d \n",NumberOfNodes());
    if ( Verbose_ ) PRINTF("Number of tris: %d \n",NumberOfTris());
    if ( Verbose_ ) PRINTF("Number of edges is: %d \n",NumberOfEdges());

    // Free up the scratch space

    delete [] jump_pnt;
    delete [] list;

    // Get rid of sign on edge pointers, move to edgedir list
    
    for ( j = 1 ; j <= NumberOfTris() ; j++ ) {

       LoopList(j).Edge1Direction() = SGN(LoopList(j).Edge1());
       LoopList(j).Edge2Direction() = SGN(LoopList(j).Edge2());
       LoopList(j).Edge3Direction() = SGN(LoopList(j).Edge3());
       
       LoopList(j).Edge1() = ABS(LoopList(j).Edge1());
       LoopList(j).Edge2() = ABS(LoopList(j).Edge2());
       LoopList(j).Edge3() = ABS(LoopList(j).Edge3());
  
    }
    
    // Mark edges to the surface they belong to

    for ( j = 1 ; j <= NumberOfTris() ; j++ ) {

       // Edge 1
       
       Edge = LoopList(j).Edge1();
 
       EdgeList(Edge).DegenWing() = LoopList(j).DegenWingID();
       
       EdgeList(Edge).DegenBody() = LoopList(j).DegenBodyID();
       
       EdgeList(Edge).Cart3DSurface() = LoopList(j).Cart3dID();
       
       EdgeList(Edge).ComponentID() = LoopList(j).ComponentID();

       // Edge 2
       
       Edge = LoopList(j).Edge2();
  
       EdgeList(Edge).DegenWing() = LoopList(j).DegenWingID();
       
       EdgeList(Edge).DegenBody() = LoopList(j).DegenBodyID();
       
       EdgeList(Edge).Cart3DSurface() = LoopList(j).Cart3dID();

       EdgeList(Edge).ComponentID() = LoopList(j).ComponentID();
       
       // Edge 3
       
       Edge = LoopList(j).Edge3();
  
       EdgeList(Edge).DegenWing() = LoopList(j).DegenWingID();
       
       EdgeList(Edge).DegenBody() = LoopList(j).DegenBodyID();       
       
       EdgeList(Edge).Cart3DSurface() = LoopList(j).Cart3dID();
       
       EdgeList(Edge).ComponentID() = LoopList(j).ComponentID();
       
    }
     
    
    // Mark nodes to the surface they belong to

    for ( j = 1 ; j <= NumberOfTris() ; j++ ) {

       // Node 1
       
       Node = LoopList(j).Node1();

       NodeList(Node).ComponentID() = LoopList(j).ComponentID();

       // Node 2
       
       Node = LoopList(j).Node2();

       NodeList(Node).ComponentID() = LoopList(j).ComponentID();
       
       // Node 3
       
       Node = LoopList(j).Node3();

       NodeList(Node).ComponentID() = LoopList(j).ComponentID();
       
    }
               
    for ( j = 1 ; j <= NumberOfEdges() ; j++ ) {
     
       // Leading, trailing, and general edge list information

       if ( EdgeList(j).Tri2() == EdgeList(j).Tri1() ) {
   
          if ( NodeList(EdgeList(j).Node1()).IsTrailingEdgeNode() &&
               NodeList(EdgeList(j).Node2()).IsTrailingEdgeNode()     ) {

             EdgeList(j).IsTrailingEdge() = 1;
             
          }
          
          if ( NodeList(EdgeList(j).Node1()).IsLeadingEdgeNode() &&
               NodeList(EdgeList(j).Node2()).IsLeadingEdgeNode()     ) {
      
             EdgeList(j).IsLeadingEdge() = 1;
             
          }          
 
          if ( NodeList(EdgeList(j).Node1()).IsBoundaryEdgeNode() &&
               NodeList(EdgeList(j).Node2()).IsBoundaryEdgeNode()     ) {

             if ( EdgeList(j).IsBoundaryEdge() == 0 ) EdgeList(j).IsBoundaryEdge() = 1;
             
          }    
          
       }

    }
        
    // Mark edges on boarders of diffrent surface IDs, and calculate normal
    
    for ( i = 1 ; i <= NumberOfEdges() ; i++ ) {
       
       Tri1 = EdgeList(i).Tri1();
       Tri2 = EdgeList(i).Tri2();
       
       // Mark boundary edges
       
       if ( LoopList(Tri1).SurfaceID() != LoopList(Tri2).SurfaceID() ) {
          
          if ( EdgeList(i).IsTrailingEdge() == 0 ) EdgeList(i).IsBoundaryEdge() = 1;
             
       }
       
       // Calculate average normal
       
       Normal[0] = LoopList(Tri1).Area() * LoopList(Tri1).Nx() + LoopList(Tri2).Area() * LoopList(Tri2).Nx();
       Normal[1] = LoopList(Tri1).Area() * LoopList(Tri1).Ny() + LoopList(Tri2).Area() * LoopList(Tri2).Ny();
       Normal[2] = LoopList(Tri1).Area() * LoopList(Tri1).Nz() + LoopList(Tri2).Area() * LoopList(Tri2).Nz();
       
       Dot = sqrt(vector_dot(Normal,Normal));
       
       Normal[0] /= Dot;
       Normal[1] /= Dot;
       Normal[2] /= Dot;
       
       EdgeList(i).Normal()[0] = Normal[0];
       EdgeList(i).Normal()[1] = Normal[1];
       EdgeList(i).Normal()[2] = Normal[2];
       
    }    
    
    // Calculate edge lengths
   
    for ( j = 1 ; j <= NumberOfEdges() ; j++ ) {
 
       Node1 = EdgeList(j).Node1();
       Node2 = EdgeList(j).Node2();
       
       x1 = NodeList(Node1).x();
       y1 = NodeList(Node1).y();
       z1 = NodeList(Node1).z();

       x2 = NodeList(Node2).x();
       y2 = NodeList(Node2).y();
       z2 = NodeList(Node2).z();
       
       EdgeList(j).Length() = sqrt( pow(x2-x1,2.) + pow(y2-y1,2.) + pow(z2-z1,2.) );    
       
    }
    
    // Store airfoil information on leading edges

    for ( i = 1 ; i <= NumberOfEdges() ; i++ ) {
       
       if ( EdgeList(i).IsLeadingEdge() ) {

          EdgeList(i).ThicknessToChord()       = TriList(EdgeList(i).Tri1()).ThicknessToChord();
          EdgeList(i).LocationOfMaxThickness() = TriList(EdgeList(i).Tri1()).LocationOfMaxThickness();
          EdgeList(i).RadiusToChord()          = TriList(EdgeList(i).Tri1()).RadiusToChord();      
          
       }
       
    }
         
}

/*##############################################################################
#                                                                              #
#                           VSP_GRID DetermineUpwindEdges                      #
#                                                                              #
##############################################################################*/

void VSP_GRID::CalculateUpwindEdges(void)
{
 
   int i, j, k, Edge, Node1, Node2, TotalUpwind;
   VSPAERO_DOUBLE xVec[3], Vec[3], *Flux, TotalFlux, LoopNormal[3], Normal[3], Mag;
   VSPAERO_DOUBLE Ds, DsMin, DsMax;
   VSPAERO_DOUBLE x1, y1, z1, x2, y2, z2, Length;
   
   // Loop over triangles and determine which nodes, and then edges are upwind
   
   xVec[0] = 1.;
   xVec[1] = 0.;
   xVec[2] = 0.;
 
   for ( k = 1 ; k <= NumberOfLoops() ; k++ ) {

       LoopNormal[0] = LoopList(k).Nx();
       LoopNormal[1] = LoopList(k).Ny();
       LoopNormal[2] = LoopList(k).Nz();
            
       Flux = new VSPAERO_DOUBLE[LoopList(k).NumberOfEdges() + 1];

       TotalFlux = 0.;

       for ( j = 1 ; j <= LoopList(k).NumberOfEdges() ; j++ ) {

          Edge = LoopList(k).Edge(j);
          
          if ( LoopList(k).EdgeDirection(j) == 1 ) {

             Node1 = EdgeList(Edge).Node1();
             Node2 = EdgeList(Edge).Node2();
          
          }
          
          else {
       
             Node1 = EdgeList(Edge).Node2();
             Node2 = EdgeList(Edge).Node1();

          }        
             
          Vec[0] = NodeList(Node2).x() - NodeList(Node1).x();
          Vec[1] = NodeList(Node2).y() - NodeList(Node1).y();
          Vec[2] = NodeList(Node2).z() - NodeList(Node1).z();
           
          Mag = sqrt(vector_dot(Vec,Vec));
          
          Vec[0] /= Mag;
          Vec[1] /= Mag;
          Vec[2] /= Mag;

          vector_cross(Vec,LoopNormal,Normal);
      
          Flux[j] = MAX(-vector_dot(Normal, xVec),0.);
    
          TotalFlux += Flux[j];
        
       }

       // Create weighting

       for ( j = 1 ; j <= LoopList(k).NumberOfEdges() ; j++ ) {

          Flux[j] /= TotalFlux;
         
          LoopList(k).EdgeIsUpWind(j) = 0;
         
       }
    
       // Determine which edges are upwind
       
       TotalUpwind = 0;

       for ( j = 1 ; j <= LoopList(k).NumberOfEdges() ; j++ ) {
       
          if ( Flux[j] > 0. ) {
           
             TotalUpwind++;
           
             LoopList(k).EdgeIsUpWind(j) = 1;
           
             LoopList(k).EdgeUpwindWeight(j) = Flux[j];
           
          }

       }      

       if (0&& TotalUpwind == 0 ) {
        
          PRINTF("wtf! \n");
       
          PRINTF("TotalUpwind: %d \n",TotalUpwind);
     
          PRINTF("TotalFlux: %lf \n",TotalFlux);
          
          PRINTF("LoopNormal: %lf %lf %lf \n",LoopNormal[0],LoopNormal[1],LoopNormal[2]);
          
          PRINTF("LoopList(k).NumberOfEdges(): %d \n",LoopList(k).NumberOfEdges());
            
          exit(1);
        
       }

       delete [] Flux;

       // Calculate reference length for this loop
    
       LoopList(k).Length() = sqrt(LoopList(k).Area());
       
       Length = 0.;
       
       for ( i = 1 ; i <= LoopList(k).NumberOfNodes() ; i++ ) {
          
          Node1 = LoopList(k).Node(i);
          
          x1 = NodeList(Node1).x();
          y1 = NodeList(Node1).y();
          z1 = NodeList(Node1).z();
    
          for ( j = 1 ; j <= LoopList(k).NumberOfNodes() ; j++ ) {

             Node2 = LoopList(k).Node(j);

             x2 = NodeList(Node2).x();
             y2 = NodeList(Node2).y();
             z2 = NodeList(Node2).z();
       
             Length = MAX(Length,sqrt( pow(x1-x2,2.) + pow(y1-y2,2.) + pow(z1-z2,2.) ));
             
          }
          
       }
     
       LoopList(k).Length() = MAX(Length, LoopList(k).Length());

       // Calculate smallest edge length for cell

       DsMin = 1.e9;
       DsMax = -DsMin;       

       for ( j = 1 ; j <= LoopList(k).NumberOfEdges() ; j++ ) {
       
          Edge = LoopList(k).Edge(j);

          Ds = EdgeList(Edge).Length();

          DsMin = MIN(Ds,DsMin);
          DsMax = MAX(Ds,DsMax);
          
       }  
       
       LoopList(k).RefLength() = DsMin;

    } 
        
}

/*##############################################################################
#                                                                              #
#                           VSP_GRID CreateUpwindEdgeData                      #
#                                                                              #
##############################################################################*/

void VSP_GRID::CreateUpwindEdgeData(void)
{
    
    int j, k, Done, Case, Node1, Node2;
    int Loop1, Loop2;
    int VortexLoop1IsDownWind, VortexLoop2IsDownWind;
    VSPAERO_DOUBLE VortexLoop1DownWindWeight, VortexLoop2DownWindWeight;

    VSP_NODE VSP_Node1, VSP_Node2;
    
    // Setup the surface vortex edge data
   
    for ( j = 1 ; j <= NumberOfEdges() ; j++ ) {

       // Pass in edge data and create edge cofficients
       
       Node1 = EdgeList(j).Node1();
       Node2 = EdgeList(j).Node2();

       EdgeList(j).Setup(NodeList(Node1), NodeList(Node2));
                                                  
       // Determine direction of edges
       
       Loop1 = EdgeList(j).Loop1();
       Loop2 = EdgeList(j).Loop2();

       // Loop over Loop1 edges, see if edge direction is with loop
       
       Case = 1;
       
       Done = 0;
       
       k = 1;
       
       while ( k <= LoopList(Loop1).NumberOfEdges() && !Done ) {
          
          if ( LoopList(Loop1).Edge(k) == j ) {
             
             if ( LoopList(Loop1).EdgeDirection(k) == -1 ) {
                
                Case = 2;
                
                Done = 1;
                
             }
             
          }
          
          k++;
          
       }          
         
       // Based on directions, set loop 1 and 2 for each vortex edge 

       if ( Case == 1 ) {
        
          EdgeList(j).LoopL() = Loop1;
          EdgeList(j).LoopR() = Loop2;
          
          if ( EdgeList(j).LoopL() == EdgeList(j).LoopR() ) EdgeList(j).LoopR() = 0;
       
          // Check if loop 1 is down wind
       
          VortexLoop1IsDownWind = 0; VortexLoop1DownWindWeight = 0.;
          
          Done = 0;
          
          k = 1;
          
          while ( k <= LoopList(Loop1).NumberOfEdges() && !Done ) {
                       
             if ( j == LoopList(Loop1).Edge(k) && LoopList(Loop1).EdgeIsUpWind(k) ) {
                
                VortexLoop1IsDownWind = 1;
                
                VortexLoop1DownWindWeight = LoopList(Loop1).EdgeUpwindWeight(k);
   
                Done = 1;
                 
             }
             
             k++;
             
          }       
          
          // Check if loop 2 is down wind
          
          VortexLoop2IsDownWind = 0; VortexLoop2DownWindWeight = 0.;
          
          Done = 0;
          
          k = 1;
          
          while ( k <= LoopList(Loop2).NumberOfEdges() && !Done ) {
                       
             if ( j == LoopList(Loop2).Edge(k) && LoopList(Loop2).EdgeIsUpWind(k) ) {
                
                VortexLoop2IsDownWind = 1;
                
                VortexLoop2DownWindWeight = LoopList(Loop2).EdgeUpwindWeight(k);
             
                Done = 1;
                 
             }
             
             k++;
             
          }   

       }
       
       else {
        
          EdgeList(j).LoopL() = Loop2;
          EdgeList(j).LoopR() = Loop1;

          if ( EdgeList(j).LoopL() == EdgeList(j).LoopR() ) EdgeList(j).LoopL() = 0;
 
          // Check if loop 1 is down wind
       
          VortexLoop1IsDownWind = 0; VortexLoop1DownWindWeight = 0.;
          
          Done = 0;
          
          k = 1;
          
          while ( k <= LoopList(Loop2).NumberOfEdges() && !Done ) {
                       
             if ( j == LoopList(Loop2).Edge(k) && LoopList(Loop2).EdgeIsUpWind(k) ) {
                
                VortexLoop1IsDownWind = 1;
                
                VortexLoop1DownWindWeight = LoopList(Loop2).EdgeUpwindWeight(k);
   
                Done = 1;
                 
             }
             
             k++;
             
          }       
          
          // Check if loop 2 is down wind
          
          VortexLoop2IsDownWind = 0; VortexLoop2DownWindWeight = 0.;
          
          Done = 0;
          
          k = 1;
          
          while ( k <= LoopList(Loop1).NumberOfEdges() && !Done ) {
                       
             if ( j == LoopList(Loop1).Edge(k) && LoopList(Loop1).EdgeIsUpWind(k) ) {
                
                VortexLoop2IsDownWind = 1;
                
                VortexLoop2DownWindWeight = LoopList(Loop1).EdgeUpwindWeight(k);
             
                Done = 1;
                 
             }
             
             k++;
             
          }   

       }                                  

       // And keep track of which loops are down wind of this edge, and their weights
    
       EdgeList(j).VortexLoop1IsDownWind() = VortexLoop1IsDownWind;
       EdgeList(j).VortexLoop2IsDownWind() = VortexLoop2IsDownWind;
       
       EdgeList(j).VortexLoop1DownWindWeight() = VortexLoop1DownWindWeight;         
       EdgeList(j).VortexLoop2DownWindWeight() = VortexLoop2DownWindWeight;      
 
    }

}

/*##############################################################################
#                                                                              #
#                             VSP_GRID WriteMesh                               #
#                                                                              #
##############################################################################*/

void VSP_GRID::WriteMesh(char *FileName)
{
 
    int i;
    FILE *MeshFile;

    if ( (MeshFile = fopen(FileName,"w")) == NULL ) {

       // No VSP degen file... exit

       PRINTF("Could not open %s mesh file for write... \n", FileName);fflush(NULL);

       exit(1);

    }   
    
    // Header
    
    FPRINTF(MeshFile,"%d %d %d \n", NumberOfNodes_, NumberOfLoops_, 0); 
    
    // XYZ data
    
    for ( i = 1 ; i <= NumberOfNodes_ ; i++ ) {

       FPRINTF(MeshFile,"%lf \n",NodeList(i).x());
       
    }    

    for ( i = 1 ; i <= NumberOfNodes_ ; i++ ) {

       FPRINTF(MeshFile,"%lf \n",NodeList(i).y());
       
    }    
    
    for ( i = 1 ; i <= NumberOfNodes_ ; i++ ) {

       FPRINTF(MeshFile,"%lf \n",NodeList(i).z());
       
    }    
    
    // Connectivity
    
    for ( i = 1 ; i <= NumberOfLoops_ ; i++ ) {

      FPRINTF(MeshFile,"%d %d %d \n",
              LoopList(i).Node1(),
              LoopList(i).Node2(),
              LoopList(i).Node3());
       
    }      

    for ( i = 1 ; i <= NumberOfLoops_ ; i++ ) {

      FPRINTF(MeshFile,"%d %d \n", LoopList(i).SurfaceID(), LoopList(i).SurfaceID());
       
    }

}

/*##############################################################################
#                                                                              #
#                      VSP_GRID UpdateGeometryLocation                         #
#                                                                              #
##############################################################################*/

void VSP_GRID::UpdateGeometryLocation(VSPAERO_DOUBLE *TVec, VSPAERO_DOUBLE *OVec, QUAT &Quat, QUAT &InvQuat, int *ComponentInThisGroup)
{
 
    int i;
    QUAT Vec;
    
    // Update nodal data

    for ( i = 1 ; i <= NumberOfNodes_ ; i++ ) {

       if ( ComponentInThisGroup[NodeList(i).ComponentID()] ) NodeList(i).UpdateGeometryLocation(TVec, OVec, Quat, InvQuat);

    }

    // Update edge data
        
    for ( i = 1 ; i <= NumberOfEdges() ; i++ ) {

       if ( ComponentInThisGroup[EdgeList(i).ComponentID()] ) EdgeList(i).UpdateGeometryLocation(TVec, OVec, Quat, InvQuat);
       
    }
            
    // Update loop data
    
    for ( i = 1 ; i <= NumberOfLoops_ ; i++ ) {
  
       if ( ComponentInThisGroup[LoopList(i).ComponentID()] ) LoopList(i).UpdateGeometryLocation(TVec, OVec, Quat, InvQuat);

    }
    
    // Update wake trailing edge locations

    for ( i = 1 ; i <= NumberOfKuttaNodes_ ; i++ ) {
    
       Vec(0) = WakeTrailingEdgeX_[i] - OVec[0];
       Vec(1) = WakeTrailingEdgeY_[i] - OVec[1];
       Vec(2) = WakeTrailingEdgeZ_[i] - OVec[2];
   
       Vec = Quat * Vec * InvQuat;
   
       WakeTrailingEdgeX_[i] = Vec(0) + OVec[0] + TVec[0];
       WakeTrailingEdgeY_[i] = Vec(1) + OVec[1] + TVec[1];
       WakeTrailingEdgeZ_[i] = Vec(2) + OVec[2] + TVec[2];    
       
    }  
          
}


