//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "VSP_Agglom.H"


/*##############################################################################
#                                                                              #
#                            VSP_AGGLOM Initialize_                            #              
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::Initialize_(void)
{

    FineGrid_ = NULL;
    CoarseGrid_ = NULL;
    Search_ = NULL;
 
    // Agglomeration data
    
    NumberOfEdgesOnTE_ = 0;
    NumberOfEdgesOnLE_ = 0;
    NumberOfEdgesOnBoundary_ = 0;
 
    EdgeIsOnFront_             = NULL; 
    FrontEdgeQueue_            = NULL; 
    NodeIsOnFront_             = NULL; 
    LoopHits_                  = NULL; 
    DidThisLoop_               = NULL; 
    LoopListStack_             = NULL; 
    EdgeDegree_                = NULL; 
    VortexLoopWasAgglomerated_ = NULL; 
    CoarseEdgeList_            = NULL; 
    CoarseNodeList_            = NULL;    
  
    NextEdgeInQueue_ = 0;
    NextBestEdgeOnFront_= 0;

}

/*##############################################################################
#                                                                              #
#                            VSP_AGGLOM constructor                            #              
#                                                                              #
##############################################################################*/

VSP_AGGLOM::VSP_AGGLOM(void)
{

    // Initialize
    
    Initialize_();

}

/*##############################################################################
#                                                                              #
#                            VSP_AGGLOM destructor                             #              
#                                                                              #
##############################################################################*/

VSP_AGGLOM::~VSP_AGGLOM(void)
{

    int i;
    
    if ( EdgeIsOnFront_             != NULL ) delete [] EdgeIsOnFront_;
    if ( FrontEdgeQueue_            != NULL ) delete [] FrontEdgeQueue_;
    if ( NodeIsOnFront_             != NULL ) delete [] NodeIsOnFront_;
    if ( LoopHits_                  != NULL ) delete [] LoopHits_;
    if ( DidThisLoop_               != NULL ) delete [] DidThisLoop_;
    if ( LoopListStack_             != NULL ) delete [] LoopListStack_;
    if ( EdgeDegree_                != NULL ) delete [] EdgeDegree_;
    if ( VortexLoopWasAgglomerated_ != NULL ) delete [] VortexLoopWasAgglomerated_;
    if ( CoarseEdgeList_            != NULL ) delete [] CoarseEdgeList_;
    if ( CoarseNodeList_            != NULL ) delete [] CoarseNodeList_;

    delete [] NumberOfLoopsForNode_;

    for ( i = 1 ; i <= FineGrid().NumberOfNodes() ; i++ ) {

       delete [] LoopListForNode_[i];
              
    }
    
    delete [] LoopListForNode_;
        
}

/*##############################################################################
#                                                                              #
#                                VSP_AGGLOM Copy                               #              
#                                                                              #
##############################################################################*/

VSP_AGGLOM::VSP_AGGLOM(const VSP_AGGLOM &agglom)
{

    // Not implemented!
    
    PRINTF("Copying of agglom objects not implemented! \n");
    
}

/*##############################################################################
#                                                                              #
#                          VSP_AGGLOM  SimplifyMesh_                           #
#                                                                              #
##############################################################################*/

VSP_GRID* VSP_AGGLOM::SimplifyMesh_(VSP_GRID &Grid)
{
 
    // Copy pointer to the fine grid
    
    FineGrid_ = &Grid;
    
    CheckMesh_(FineGrid());
    
    // Delete duplicates nodes
    
 //   FineGrid_ = DeleteDuplicateNodes_(*FineGrid_);

    // Create fast search of edge list
    
    Search_ = new SEARCH;

    Search_->CreateSearchTree(FineGrid());
        
    // Initialize the front
    
    InitializeFront_();    
    
    // Merge bad cells together to get rid of slivers
  
    if ( FineGrid().SurfaceType() == CART3D_SURFACE || FineGrid().SurfaceType() == VSPGEOM_SURFACE ) CleanUpMesh_();

    // Merge as many tris into quads as possible
   
    CreateMixedMesh_();
       
    // Create the course mesh data

    CreateCoarseMesh_();

    // Check the mesh for any errors

    CheckMesh_(CoarseGrid());

    // Delete up the search tree
    
    delete Search_;
    
    // Return pointer to the coarse mesh
      
    return CoarseGrid_;
    
}

/*##############################################################################
#                                                                              #
#                      VSP_AGGLOM  MergeHighAspectRatioQuads_                  #
#                                                                              #
##############################################################################*/

VSP_GRID* VSP_AGGLOM::MergeHighAspectRatioQuads_(VSP_GRID &Grid)
{
 
    // Copy pointer to the fine grid
    
    FineGrid_ = &Grid;
    
    CheckMesh_(FineGrid());

    // Create fast search of edge list
    
    Search_ = new SEARCH;

    Search_->CreateSearchTree(FineGrid());
       
    // Initialize the front
    
    InitializeFront_();    
    
    if ( FineGrid().SurfaceType() == CART3D_SURFACE || FineGrid().SurfaceType() == VSPGEOM_SURFACE ) CleanUpHighAspectRatioQuads_();

    // Create the course mesh data

    CreateCoarseMesh_();

    // Check the mesh for any errors

    CheckMesh_(CoarseGrid());

    // Delete search tree
    
    delete Search_;
    
    // Return pointer to the coarse mesh
      
    return CoarseGrid_;
    
}

/*##############################################################################
#                                                                              #
#                           VSP_AGGLOM Agglomerate_                            #              
#                                                                              #
##############################################################################*/

VSP_GRID* VSP_AGGLOM::Agglomerate_(VSP_GRID &Grid)
{

    // Copy pointer to the fine grid

    FineGrid_ = &Grid;
    
    CheckMesh_(FineGrid());

    // Create fast search of edge list
    
    Search_ = new SEARCH;

    Search_->CreateSearchTree(FineGrid());
        
    // Initialize the front
        
    InitializeFront_();
    
    NextBestEdgeOnFront_ = NextAgglomerationEdge_();
    
    // Merge vortex loops

    while ( NextBestEdgeOnFront_ > 0 ) {

       MergeVortexLoops_();
       
       NextBestEdgeOnFront_ = NextAgglomerationEdge_();
              
    }
        
    // Clean up stray loops
    
    MergeSmallLoops_();

    // Create the course mesh data

    CreateCoarseMesh_();

    // Check the mesh for any errors

    CheckMesh_(CoarseGrid());

    CoarseGrid_ = MergeCoLinearEdges_();

    // Check the mesh for any errors
    
    CheckMesh_(CoarseGrid());
    
    // Delete the search tree
    
    delete Search_;

    // Return pointer to the coarse mesh
           
    return CoarseGrid_;
   
}

/*##############################################################################
#                                                                              #
#                           VSP_AGGLOM InitializeFront_                        #              
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::InitializeFront_(void)
{

    int i, k, NumberOfSurfaces, Loop1, Loop2, j, Done, *SurfaceExists;

    // Allocate space for the front list. This will contain the currently unused
    // edges on the agglomeration front.
 
    EdgeIsOnFront_ = new int[FineGrid().NumberOfEdges() + 1];
    
    zero_int_array(EdgeIsOnFront_, FineGrid().NumberOfEdges());
    
    FrontEdgeQueue_ = new int[FineGrid().NumberOfEdges() + 1];
    
    zero_int_array(FrontEdgeQueue_, FineGrid().NumberOfEdges());
    
    NodeIsOnFront_ = new int[FineGrid().NumberOfNodes() + 1];
    
    zero_int_array(NodeIsOnFront_, FineGrid().NumberOfNodes());
    
    // Allocate arrays
    
    LoopHits_ = new int[FineGrid().NumberOfEdges() + 1];

    DidThisLoop_ = new int[FineGrid().NumberOfEdges() + 1];
    
    LoopListStack_ = new int[FineGrid().NumberOfLoops() + 2];
   
    EdgeDegree_ = new int[FineGrid().NumberOfNodes() + 1];
    
    zero_int_array(LoopHits_, FineGrid().NumberOfEdges());
 
    zero_int_array(DidThisLoop_, FineGrid().NumberOfEdges());
    
    zero_int_array(LoopListStack_, FineGrid().NumberOfLoops() + 1);
    
    zero_int_array(EdgeDegree_, FineGrid().NumberOfNodes());
    
    // Create node degree array
    
    for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
       
       EdgeDegree_[FineGrid().EdgeList(i).Node1()]++;
       EdgeDegree_[FineGrid().EdgeList(i).Node2()]++;
       
    }
    
    // Create node to loop data
    
    NumberOfLoopsForNode_ = new int[FineGrid().NumberOfNodes() + 1];
     
    LoopListForNode_ = new int*[FineGrid().NumberOfNodes() + 1];
    
    zero_int_array(NumberOfLoopsForNode_, FineGrid().NumberOfNodes());

    for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
       
       for ( j = 1 ; j <= FineGrid().LoopList(i).NumberOfNodes() ; j++ ) {
       
          NumberOfLoopsForNode_[FineGrid().LoopList(i).Node(j)]++;
          
       }
       
    }

    for ( i = 1 ; i <= FineGrid().NumberOfNodes() ; i++ ) {

       LoopListForNode_[i] = new int[NumberOfLoopsForNode_[i] + 1];
       
       NumberOfLoopsForNode_[i] = 0;
       
    }
    
    for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
       
       for ( j = 1 ; j <= FineGrid().LoopList(i).NumberOfNodes() ; j++ ) {
       
          NumberOfLoopsForNode_[FineGrid().LoopList(i).Node(j)]++;
          
          LoopListForNode_[FineGrid().LoopList(i).Node(j)][NumberOfLoopsForNode_[FineGrid().LoopList(i).Node(j)]] = i;
          
       }
       
    }
    
    // Allocate space for the vortex loop list... note that the number of tris, 
    // and the tri data is used in lieu of loops. These are only tris on the 
    // fine grid, and are truly loops on the coarser grids
    
    VortexLoopWasAgglomerated_ = new int[FineGrid().NumberOfLoops() + 1];
    
    VortexLoopWasAgglomerated_[0] = 0;
    
    for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
     
       VortexLoopWasAgglomerated_[i] = i;
       
    }

    // Allocate space for the fine to coarse grid edge pointer. This will hold a 
    // list of the fine grid edges that still remain on the course grid

    CoarseEdgeList_ = new int[FineGrid().NumberOfEdges() + 1];
    
    zero_int_array(CoarseEdgeList_, FineGrid().NumberOfEdges());
 
    // Allocate space for the fine to coarse grid node pointer. This will hold a 
    // list of the fine grid node that still remain on the course grid

    CoarseNodeList_ = new int[FineGrid().NumberOfNodes() + 1];
    
    zero_int_array(CoarseNodeList_, FineGrid().NumberOfNodes());

    // Insert edges into the front list

    NumberOfEdgesOnTE_       = 0;
    NumberOfEdgesOnLE_       = 0;
    NumberOfEdgesOnBoundary_ = 0;
    NextBestEdgeOnFront_     = 0;
    NextEdgeInQueue_         = 0;    
    NumberOfEdgesInQueue_    = 0;
     
    // Check edge is on trailing edge of wing
     
    for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
       
       if ( FineGrid().EdgeList(i).IsTrailingEdge()  ) {

          if ( EdgeIsOnFront_[i] == 0 ) {
        
             EdgeIsOnFront_[i] = TE_EDGE_BC;
             
             FrontEdgeQueue_[++NextEdgeInQueue_] = i;
       
             NumberOfEdgesOnTE_++;
             
             k = FindMatchingSymmetryEdge_(i);
             
             if ( k != 0 ) {
                
                if ( FineGrid().EdgeList(k).IsTrailingEdge()  ) {
                   
                   EdgeIsOnFront_[k] = TE_EDGE_BC;
                   
                   FrontEdgeQueue_[++NextEdgeInQueue_] = k;
                   
                   NumberOfEdgesOnTE_++;    
                   
                }
                
             }            
             
          }
       
       }        
       
    }
 
    // Check if edge is on leading edge of wing
 
    for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
      
       if ( FineGrid().EdgeList(i).IsLeadingEdge() ) {

          if ( EdgeIsOnFront_[i] == 0 ) {
        
             EdgeIsOnFront_[i] = LE_EDGE_BC;
             
             FrontEdgeQueue_[++NextEdgeInQueue_] = i;
       
             NumberOfEdgesOnLE_++;
             
             k = FindMatchingSymmetryEdge_(i);
             
             if ( k != 0 ) {
                
                if ( FineGrid().EdgeList(k).IsLeadingEdge()  ) {
                   
                   EdgeIsOnFront_[k] = LE_EDGE_BC;
                   
                   FrontEdgeQueue_[++NextEdgeInQueue_] = k;
                   
                   NumberOfEdgesOnLE_++;    
                   
                }
                
             }
                          
          }
       
       }    
       
    }
       
    // Check if edge is on any general boundry
       
    for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
              
       if ( FineGrid().EdgeList(i).IsBoundaryEdge() ) {
        
          if ( EdgeIsOnFront_[i] == 0 ) {
        
             EdgeIsOnFront_[i] = BOUNDARY_EDGE_BC;
             
             FrontEdgeQueue_[++NextEdgeInQueue_] = i;
       
             NumberOfEdgesOnBoundary_++;
            
             k = FindMatchingSymmetryEdge_(i);
             
             if ( k != 0 ) {
                
                if ( FineGrid().EdgeList(k).IsBoundaryEdge()  ) {
                   
                   EdgeIsOnFront_[k] = BOUNDARY_EDGE_BC;
                   
                   FrontEdgeQueue_[++NextEdgeInQueue_] = k;
                   
                   NumberOfEdgesOnBoundary_++;    
                   
                }
                
             }
                          
          }
       
       }  
       
    }
    
    // Check that all surfaces have at least one edge in the queue
    
    NumberOfSurfaces = 0;
    
    for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
   
       NumberOfSurfaces = MAX(NumberOfSurfaces, FineGrid().LoopList(i).SurfaceID());
       
    }
        
    SurfaceExists = new int[NumberOfSurfaces + 1];
    
    zero_int_array(SurfaceExists, NumberOfSurfaces);
    
    for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
   
       SurfaceExists[FineGrid().LoopList(i).SurfaceID()] = 1;
       
    }
    
    for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
       
       if ( EdgeIsOnFront_[i] ) {
          
          Loop1 = FineGrid().EdgeList(i).Loop1();
          Loop2 = FineGrid().EdgeList(i).Loop2();
          
          if ( Loop1 > 0 ) SurfaceExists[FineGrid().LoopList(Loop1).SurfaceID()] = 999;
          if ( Loop2 > 0 ) SurfaceExists[FineGrid().LoopList(Loop1).SurfaceID()] = 999;
                              
       }
       
    }    
        
    for ( i = 1 ; i <= NumberOfSurfaces ; i++ ) {
       
       if ( SurfaceExists[i] == 1 ) {
          
          // Add the first edge on this surface we find
          
          j = 1;
          
          Done = 0;
          
          while ( j <= FineGrid().NumberOfEdges() && !Done ) {

             Loop1 = FineGrid().EdgeList(j).Loop1();
             Loop2 = FineGrid().EdgeList(j).Loop2();
          
             if ( Loop1 > 0 && FineGrid().LoopList(Loop1).SurfaceID() == i || Loop2 >0 && FineGrid().LoopList(Loop2).SurfaceID() == i ) {
                
                EdgeIsOnFront_[j] = BOUNDARY_EDGE_BC;
                
                FrontEdgeQueue_[++NextEdgeInQueue_] = j;
                
                NumberOfEdgesOnBoundary_++;     
                
                Done = 1;           
                
             }
             
             j++;
             
          }
                                        
       }
       
    }    
    
    delete [] SurfaceExists;

    // If there are no edges in the queue... just start with edge 1
    
    if ( NextEdgeInQueue_ == 0 ) {
       
       i = 1;
       
       EdgeIsOnFront_[i] = BOUNDARY_EDGE_BC;
       
       FrontEdgeQueue_[++NextEdgeInQueue_] = i;
   
       NumberOfEdgesOnBoundary_++;       
       
    }
    
    for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
       
       if ( EdgeIsOnFront_[i] ) {
          
          NodeIsOnFront_[FineGrid().EdgeList(i).Node1()] = 1;
          NodeIsOnFront_[FineGrid().EdgeList(i).Node2()] = 1;
          
       }
       
    }
              
    NumberOfEdgesInQueue_ = NextEdgeInQueue_;
    
    NextEdgeInQueue_ = 0;
    
}

/*##############################################################################
#                                                                              #
#                        VSP_AGGLOM FindMatchingSymmetryEdge_                  #              
#                                                                              #
##############################################################################*/

int VSP_AGGLOM::FindMatchingSymmetryEdge_(int Edge)
{
return 0;  
    int Node1, Node2;
    VSPAERO_DOUBLE Tolerance;
    TEST_NODE Node;
  
    Tolerance = 1.e-8;

    Node1 = FineGrid().EdgeList(Edge).Node1();
    Node2 = FineGrid().EdgeList(Edge).Node2();
    
    Node.xyz[0] =  0.5*( FineGrid().NodeList(Node1).x() + FineGrid().NodeList(Node2).x() );
    Node.xyz[1] = -0.5*( FineGrid().NodeList(Node1).y() + FineGrid().NodeList(Node2).y() );
    Node.xyz[2] =  0.5*( FineGrid().NodeList(Node1).z() + FineGrid().NodeList(Node2).z() );
    
    Node.distance = 1.e9;
    
    Node.found = 0;
    
   // Search_->Tolerance() = Tolerance;
 
    Search_->SearchTree(Node);

 //   if ( Node.found && Node.id != Edge && EdgeIsOnFront_[Node.id] == 0 && sqrt(Node.distance) <= Tolerance ) {
    if ( Node.found && Node.id != Edge && EdgeIsOnFront_[Node.id] == 0 ) {

       return Node.id;
    }
/*        
 
    // Brute force search
    
    while ( i <= FineGrid().NumberOfEdges() ) {
       
       if ( EdgeIsOnFront_[i] == 0 ) {

          Node1 = FineGrid().EdgeList(i).Node1();
          Node2 = FineGrid().EdgeList(i).Node2();
          
          X2c[0] = 0.5*( FineGrid().NodeList(Node1).x() + FineGrid().NodeList(Node2).x() );
          X2c[1] = 0.5*( FineGrid().NodeList(Node1).y() + FineGrid().NodeList(Node2).y() );
          X2c[2] = 0.5*( FineGrid().NodeList(Node1).z() + FineGrid().NodeList(Node2).z() );
                 
          if ( i != Edge && X1c[1] * X2c[1] < 0. ) {
   
             Distance = pow(X2c[0] - X1c[0],2.) + pow(X2c[1] + X1c[1],2.) + pow(X2c[2] - X1c[2],2.);
             
             Distance = sqrt(Distance);
   
             if ( Distance <= Epsilon ) {
                
                PRINTF("Looking for opposite of edge: %d \n",Edge);
                PRINTF("Brute force: %d .... and SearchTree: %d \n",i,Node.id);
                if ( Node.found ) PRINTF("Distance: %f ... adist: %f \n",Distance,sqrt(Node.distance));
                
                return i;
             
             }
                       
          }
          
       }
       
       i++;
       
    }  
*/
    return 0;
   
}

/*##############################################################################
#                                                                              #
#                           VSP_AGGLOM NextAgglomerationEdge_                  #              
#                                                                              #
##############################################################################*/

int VSP_AGGLOM::NextAgglomerationEdge_(void)
{

    // Return next edge in the queue
    
    if ( NextEdgeInQueue_ < NumberOfEdgesInQueue_ ) return FrontEdgeQueue_[++NextEdgeInQueue_];
 
    // If we got here then we are done agglomerating
    
    return 0;

}

/*##############################################################################
#                                                                              #
#                           VSP_AGGLOM MergeVortexLoops_                       #              
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::MergeVortexLoopsOld_(void)
{

    int i, j, k, p, Side, Loop, Loop1, Loop2, Loop3, Edge, MergedLoop, NewLoop;
    int LoopA, LoopB, LoopC, LoopD, LoopE, Bad;
    VSPAERO_DOUBLE Area;

    // Check each side of this edge
        
    for ( Side = 1 ; Side <= 2 ; Side++ ) {

       StackSize_ = MergedLoop = 0;
    
       if ( Side == 1 ) {
          
          Loop1 = FineGrid().EdgeList(NextBestEdgeOnFront_).Loop1();
          
          Loop3 = FineGrid().EdgeList(NextBestEdgeOnFront_).Loop2();
          
       }
          
       if ( Side == 2 ) {
          
          Loop1 = FineGrid().EdgeList(NextBestEdgeOnFront_).Loop2();
          
          Loop3 = FineGrid().EdgeList(NextBestEdgeOnFront_).Loop1();
          
       }

       if ( VortexLoopWasAgglomerated_[Loop1] > 0 ) {

          // Agglomerate loops that share an edge with this loop
          
          for ( k = 1 ; k <= FineGrid().LoopList(Loop1).NumberOfEdges() ; k++ ) {
           
             i = FineGrid().LoopList(Loop1).Edge(k);
       
             // Don't look at the edge we started with... and don't break important edges
             
             if ( i != NextBestEdgeOnFront_                &&
                  !FineGrid().EdgeList(i).IsTrailingEdge() &&
                  !FineGrid().EdgeList(i).IsBoundaryEdge() &&
                  !FineGrid().EdgeList(i).IsLeadingEdge()     ) {
              
                // Don't merge if this is just the loop on the other side of this edge (or similar edges)
                
                if ( ( FineGrid().EdgeList(i).Loop1() == Loop1 && FineGrid().EdgeList(i).Loop2() != Loop3 ) ||
                     ( FineGrid().EdgeList(i).Loop2() == Loop1 && FineGrid().EdgeList(i).Loop1() != Loop3 ) ) {
           
                   Loop2 = FineGrid().EdgeList(i).Loop1() + FineGrid().EdgeList(i).Loop2() - Loop1;
                   
                   // Only agglomerate this loop if it has not been already agglomerated
                   
                   if ( VortexLoopWasAgglomerated_[Loop2] > 0 ) {
                      
                      // Only merge tris on the same surface patch
                      
                      Area = FineGrid().LoopList(Loop1).Area() + FineGrid().LoopList(Loop2).Area();
        
                      if ( FineGrid().LoopList(Loop1).SurfaceID() == FineGrid().LoopList(Loop2).SurfaceID() && Area < FineGrid().MinLoopArea() ) {
                         
                         // Check that there are no edges we must keep in the mesh...
                         
                         LoopA = MIN(Loop1, Loop2);
                         LoopB = MAX(Loop1, Loop2);
                         
                         Bad = 0;
                         
                         for ( p = 1 ; p <= FineGrid().LoopList(Loop1).NumberOfEdges() ; p++ ) {
           
                            Edge = FineGrid().LoopList(Loop1).Edge(p);
                            
                            if ( FineGrid().EdgeList(Edge).IsTrailingEdge() ||
                                 FineGrid().EdgeList(Edge).IsBoundaryEdge() ||
                                 FineGrid().EdgeList(Edge).IsLeadingEdge() ) {
                       
                               LoopC = MIN(ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(Edge).Loop1()]), ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(Edge).Loop2()]));
                               LoopD = MAX(ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(Edge).Loop1()]), ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(Edge).Loop2()]));
                        
                               if ( LoopA == LoopC && LoopB == LoopD ) Bad = 1;
                               
                            }
                            
                         }
                         
                         for ( p = 1 ; p <= FineGrid().LoopList(Loop2).NumberOfEdges() ; p++ ) {
           
                            Edge = FineGrid().LoopList(Loop2).Edge(p);
                            
                            if ( FineGrid().EdgeList(Edge).IsTrailingEdge() ||
                                 FineGrid().EdgeList(Edge).IsBoundaryEdge() ||
                                 FineGrid().EdgeList(Edge).IsLeadingEdge() ) {
                        
                               LoopC = MIN(ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(Edge).Loop1()]), ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(Edge).Loop2()]));
                               LoopD = MAX(ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(Edge).Loop1()]), ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(Edge).Loop2()]));
                                                           
                               if ( LoopA == LoopC && LoopB == LoopD ) Bad = 1;
                               
                            }
                            
                         }          

                         if ( !Bad ) {               
                            
                            MergedLoop = Loop1;
                          
                            VortexLoopWasAgglomerated_[Loop1] = -Loop1;
                            
                            LoopListStack_[++StackSize_] = Loop1;                         
                       
                            // Mark Loop2 as being merged with Loop 1
                            
                            VortexLoopWasAgglomerated_[Loop2] = -Loop1;
      
                            LoopListStack_[++StackSize_] = Loop2;
     
                            // Now add edges of this loop to the front
                         
                            for ( j = 1 ; j <= FineGrid().LoopList(Loop2).NumberOfEdges() ; j++ ) {
                             
                               Edge = FineGrid().LoopList(Loop2).Edge(j);
                               
                               if ( EdgeIsOnFront_[Edge] == 0 ) {
                                  
                                  EdgeIsOnFront_[Edge] = INTERIOR_EDGE_BC;
                                  
                                  FrontEdgeQueue_[++NumberOfEdgesInQueue_] = Edge;
                                  
                               }
                               
                            }
                            
                         }
                         
                      }
                      
                   }
                      
                }
                
             }
             
          }
          
       }

       // Now Search for un-merged loops that share 2 or more edges with the current loop
    
       if ( MergedLoop != 0 ) {
  
          for ( i = 1 ; i <= StackSize_ ; i++ ) {
           
             Loop = LoopListStack_[i];
             
             for ( j = 1 ; j <= FineGrid().LoopList(Loop).NumberOfEdges() ; j++ ) {
              
                Edge = FineGrid().LoopList(Loop).Edge(j);
                     
                Loop1 = FineGrid().EdgeList(Edge).Loop1();
                Loop2 = FineGrid().EdgeList(Edge).Loop2();
                
                NewLoop = Loop1 + Loop2 - Loop;
                
                LoopHits_[NewLoop] = DidThisLoop_[NewLoop] = 0;
                
             }
             
          }
                
          for ( i = 1 ; i <= StackSize_ ; i++ ) {
           
             Loop = LoopListStack_[i];
             
             for ( j = 1 ; j <= FineGrid().LoopList(Loop).NumberOfEdges() ; j++ ) {
              
                Edge = FineGrid().LoopList(Loop).Edge(j);
                     
                Loop1 = FineGrid().EdgeList(Edge).Loop1();
                Loop2 = FineGrid().EdgeList(Edge).Loop2();
                
                NewLoop = Loop1 + Loop2 - Loop;
                
                if ( DidThisLoop_[NewLoop] == 0 ) LoopHits_[NewLoop] += 1;
                
                DidThisLoop_[NewLoop] = 1;
                
             }
             
             for ( j = 1 ; j <= FineGrid().LoopList(Loop).NumberOfEdges() ; j++ ) {
              
                Edge = FineGrid().LoopList(Loop).Edge(j);
                     
                Loop1 = FineGrid().EdgeList(Edge).Loop1();
                Loop2 = FineGrid().EdgeList(Edge).Loop2();
                
                NewLoop = Loop1 + Loop2 - Loop;
   
                DidThisLoop_[NewLoop] = 0;
                
             }          
             
          }    
          
          for ( i = 1 ; i <= StackSize_ ; i++ ) {
           
             Loop = LoopListStack_[i];
             
             for ( j = 1 ; j <= FineGrid().LoopList(Loop).NumberOfEdges() ; j++ ) {
              
                Edge = FineGrid().LoopList(Loop).Edge(j);
                     
                Loop1 = FineGrid().EdgeList(Edge).Loop1();
                Loop2 = FineGrid().EdgeList(Edge).Loop2();
                
                NewLoop = Loop1 + Loop2 - Loop;

                if ( VortexLoopWasAgglomerated_[NewLoop] > 0     &&
                     !FineGrid().EdgeList(Edge).IsTrailingEdge() &&
                     !FineGrid().EdgeList(Edge).IsBoundaryEdge() &&
                     !FineGrid().EdgeList(Edge).IsLeadingEdge()  &&                
                     NewLoop != MergedLoop                       && 
                     NewLoop != Loop3                            &&
                     LoopHits_[NewLoop] >=2                      &&                     
                     FineGrid().LoopList(MergedLoop).SurfaceID() == FineGrid().LoopList(NewLoop).SurfaceID() ) {

                   Bad = 0;

                   for ( k = 1 ; k <= StackSize_ ; k++ ) {
           
                      LoopE = LoopListStack_[k];
                     
                      if ( LoopE != NewLoop ) {
                         
                         LoopA = MIN(LoopE, NewLoop);
                         LoopB = MAX(LoopE, NewLoop);
                
                         for ( p = 1 ; p <= FineGrid().LoopList(LoopE).NumberOfEdges() ; p++ ) {
           
                            Edge = FineGrid().LoopList(LoopE).Edge(p);
                            
                            if ( FineGrid().EdgeList(Edge).IsTrailingEdge() ||
                                 FineGrid().EdgeList(Edge).IsBoundaryEdge() ||
                                 FineGrid().EdgeList(Edge).IsLeadingEdge() ) {
                               
                               LoopC = MIN(FineGrid().EdgeList(Edge).Loop1(), FineGrid().EdgeList(Edge).Loop2());
                               LoopD = MAX(FineGrid().EdgeList(Edge).Loop1(), FineGrid().EdgeList(Edge).Loop2());
                               
                               if ( LoopA == LoopC && LoopB == LoopD ) Bad = 1;
                               
                            }
                            
                         }
                         
                         for ( p = 1 ; p <= FineGrid().LoopList(NewLoop).NumberOfEdges() ; p++ ) {
           
                            Edge = FineGrid().LoopList(NewLoop).Edge(p);
                            
                            if ( FineGrid().EdgeList(Edge).IsTrailingEdge() ||
                                 FineGrid().EdgeList(Edge).IsBoundaryEdge() ||
                                 FineGrid().EdgeList(Edge).IsLeadingEdge() ) {
                               
                               LoopC = MIN(FineGrid().EdgeList(Edge).Loop1(), FineGrid().EdgeList(Edge).Loop2());
                               LoopD = MAX(FineGrid().EdgeList(Edge).Loop1(), FineGrid().EdgeList(Edge).Loop2());
                               
                               if ( LoopA == LoopC && LoopB == LoopD ) Bad = 1;
                               
                            }
                            
                         }    
                         
                      }                  
                      
                   }
   
                   if ( !Bad ) {
                         
                      VortexLoopWasAgglomerated_[NewLoop] = -MergedLoop;
                    
                      // Now add edges of this loop to the front
                      
                      for ( k = 1 ; k <= FineGrid().LoopList(NewLoop).NumberOfEdges() ; k++ ) {
                       
                         Edge = FineGrid().LoopList(NewLoop).Edge(k);
                         
                         if ( EdgeIsOnFront_[Edge] == 0 ) {
                            
                            EdgeIsOnFront_[Edge] = INTERIOR_EDGE_BC;
                            
                            FrontEdgeQueue_[++NumberOfEdgesInQueue_] = Edge;
                            
                         }
                         
                      }     
                      
                   }           
                
                }
                
             }
             
          }   
          
       }

    }


    // Update front counters
          
    if ( EdgeIsOnFront_[NextBestEdgeOnFront_] == TE_EDGE_BC       ) NumberOfEdgesOnTE_--;
     
    if ( EdgeIsOnFront_[NextBestEdgeOnFront_] == LE_EDGE_BC       ) NumberOfEdgesOnLE_--;
        
    if ( EdgeIsOnFront_[NextBestEdgeOnFront_] == BOUNDARY_EDGE_BC ) NumberOfEdgesOnBoundary_--;
  
    // Reset current front edge to used
     
    EdgeIsOnFront_[NextBestEdgeOnFront_] *= -1;

}

/*##############################################################################
#                                                                              #
#                           VSP_AGGLOM MergeVortexLoops_                       #              
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::MergeVortexLoops_(void)
{

    int MaxNode, Edge, Hits, MinLoop, MergedLoop; 
    int Side, Loop, Loop2, jLoop, i, j, k, p, Node, Node1, Node2, MaxLoops, AgglomLoops;
    VSPAERO_DOUBLE Vec[3], Distance, MinDistance, MaxDistance, Weight, AverageWeight;
        
    // Check each side

    Node1 = FineGrid().EdgeList(NextBestEdgeOnFront_).Node1();
    Node2 = FineGrid().EdgeList(NextBestEdgeOnFront_).Node2();
    
    for ( Side = 1 ; Side <= 2 ; Side++ ) {
              
       MaxLoops = 0;

       MaxNode = 0;
                                    
       if ( Side == 1 ) Loop = FineGrid().EdgeList(NextBestEdgeOnFront_).Loop1();
       
       if ( Side == 2 ) Loop = FineGrid().EdgeList(NextBestEdgeOnFront_).Loop2();
        
       Loop2 = FineGrid().EdgeList(NextBestEdgeOnFront_).Loop1() + FineGrid().EdgeList(NextBestEdgeOnFront_).Loop2() - Loop;                           
       
       if ( Loop2 == Loop ) Loop2 = -Loop;
        
       if ( VortexLoopWasAgglomerated_[Loop] > 0 ) {
             
          // Find node with that maximal merge degree
                        
          for ( i = 1 ; i <= FineGrid().LoopList(Loop).NumberOfNodes() ; i++ ) {
             
             AgglomLoops = 0;
             
             Node = FineGrid().LoopList(Loop).Node(i);
             
             if ( Node != Node1 && Node != Node2 ) {
                          
                for ( j = 1 ; j <= NumberOfLoopsForNode_[Node] ; j++ ) {
                   
                   jLoop = LoopListForNode_[Node][j];
                   
                   if ( jLoop != Loop2 && VortexLoopWasAgglomerated_[jLoop] > 0 ) {
      
                      AgglomLoops++;
                      
                   }
                   
                }
                                
                if ( AgglomLoops > MaxLoops ) {
                   
                   MaxLoops = AgglomLoops;
                   
                   MaxNode = Node;
                   
                }
                
             }
                
          }
          
       }
    
       // We found a node that we can possibly merge all the loops that border it
               
       if ( MaxNode != 0 && MaxLoops == NumberOfLoopsForNode_[MaxNode] ) {
              
          AverageWeight = 0;
          
          Hits = 0;
              
          for ( j = 1 ; j <= NumberOfLoopsForNode_[MaxNode] ; j++ ) {
             
             jLoop = LoopListForNode_[MaxNode][j];
             
             if ( jLoop != Loop && jLoop != Loop2 ) {
                
                Vec[0] = FineGrid().LoopList(jLoop).Xc() - FineGrid().LoopList(Loop).Xc();
                Vec[1] = FineGrid().LoopList(jLoop).Yc() - FineGrid().LoopList(Loop).Yc();
                Vec[2] = FineGrid().LoopList(jLoop).Zc() - FineGrid().LoopList(Loop).Zc();
                
                Distance = sqrt(vector_dot(Vec,Vec));
                
                AverageWeight += 1./MAX(1.e-8, Distance);
                
                Hits++;
                
             }
                       
          }
          
          AverageWeight /= Hits;
          
          Hits = 0;
              
          for ( j = 1 ; j <= NumberOfLoopsForNode_[MaxNode] ; j++ ) {
             
             jLoop = LoopListForNode_[MaxNode][j];
             
             if ( jLoop != Loop && jLoop != Loop2 ) {
                
                Vec[0] = FineGrid().LoopList(jLoop).Xc() - FineGrid().LoopList(Loop).Xc();
                Vec[1] = FineGrid().LoopList(jLoop).Yc() - FineGrid().LoopList(Loop).Yc();
                Vec[2] = FineGrid().LoopList(jLoop).Zc() - FineGrid().LoopList(Loop).Zc();
                
                Distance = sqrt(vector_dot(Vec,Vec));
                
                Weight = 1./MAX(1.e-8, Distance);
                
                if ( Weight >= 0.25*AverageWeight ) Hits++;
                
             }
                       
          }
          
          // All the loops are of similar aspect ratio... merge them
        
          if ( Hits == NumberOfLoopsForNode_[MaxNode] - 1 ) {
          
             for ( j = 1 ; j <= NumberOfLoopsForNode_[MaxNode] ; j++ ) {
                
                jLoop = LoopListForNode_[MaxNode][j];
            
                if ( jLoop != Loop && jLoop != Loop2 && VortexLoopWasAgglomerated_[jLoop] > 0 ) {
                   
                   // Mark Loop2 as being merged with Loop 1
                   
                   VortexLoopWasAgglomerated_[jLoop] = -Loop;
         
                   // Now add edges of this loop to the front
                
                   for ( k = 1 ; k <= FineGrid().LoopList(jLoop).NumberOfEdges() ; k++ ) {
                    
                      Edge = FineGrid().LoopList(jLoop).Edge(k);
                      
                      if ( EdgeIsOnFront_[Edge] == 0 ) {
                         
                         EdgeIsOnFront_[Edge] = INTERIOR_EDGE_BC;
                         
                         FrontEdgeQueue_[++NumberOfEdgesInQueue_] = Edge;
                         
                         p = FindMatchingSymmetryEdge_(Edge);
                         
                         if ( p != 0 ) {
                            
                           EdgeIsOnFront_[p] = INTERIOR_EDGE_BC;
                           
                           FrontEdgeQueue_[++NumberOfEdgesInQueue_] = p;    
                           
                         }                        
                         
                      }
                      
                   }
                   
                }
                
             }   
             
             // Mark original loop as merged
                       
             VortexLoopWasAgglomerated_[Loop] = -Loop;
             
             for ( k = 1 ; k <= FineGrid().LoopList(Loop).NumberOfEdges() ; k++ ) {
              
                Edge = FineGrid().LoopList(Loop).Edge(k);
                
                if ( EdgeIsOnFront_[Edge] == 0 ) {
                   
                   EdgeIsOnFront_[Edge] = INTERIOR_EDGE_BC;
                   
                   FrontEdgeQueue_[++NumberOfEdgesInQueue_] = Edge;
                  
                   p = FindMatchingSymmetryEdge_(Edge);
                   
                   if ( p != 0 ) {
                      
                     EdgeIsOnFront_[p] = INTERIOR_EDGE_BC;
                     
                     FrontEdgeQueue_[++NumberOfEdgesInQueue_] = p;    
                     
                   }     
                                            
                }
                
             }
             
          }
                                    
       }
       
       // If we could not merge all the loops above then see if we can merge the loop with neighbor
       
       if ( VortexLoopWasAgglomerated_[Loop] > 0 ) {
 
          MinDistance = 1.e9;
          
          MinLoop = 0;
              
          for ( j = 1 ; j <= FineGrid().LoopList(Loop).NumberOfEdges() ; j++ ) {
             
             Edge = FineGrid().LoopList(Loop).Edge(j);
             
             jLoop = FineGrid().EdgeList(Edge).Loop1() + FineGrid().EdgeList(Edge).Loop2() - Loop;
          
              if ( jLoop != Loop && jLoop != Loop2  ) {
                
                Vec[0] = FineGrid().LoopList(jLoop).Xc() - FineGrid().LoopList(Loop).Xc();
                Vec[1] = FineGrid().LoopList(jLoop).Yc() - FineGrid().LoopList(Loop).Yc();
                Vec[2] = FineGrid().LoopList(jLoop).Zc() - FineGrid().LoopList(Loop).Zc();
                
                Distance = sqrt(vector_dot(Vec,Vec));
                
                if ( Distance < MinDistance ) {
                   
                   MinDistance = Distance;
                   
                   MinLoop = jLoop;
                   
                }
                
             }
             
          }
          
          // Merge this loop with the closest one found above - if that loop has either 1) not been merged already,
          // or 2) if it's the parent of a previous merger ... this should limit runaway mergings
          
          if ( MinLoop != 0 && ( VortexLoopWasAgglomerated_[MinLoop] > 0 || VortexLoopWasAgglomerated_[MinLoop] == -MinLoop ) ) {

             MergedLoop = Loop;
             
             if ( VortexLoopWasAgglomerated_[MinLoop] < 0 ) MergedLoop = -VortexLoopWasAgglomerated_[MinLoop];
             
             VortexLoopWasAgglomerated_[MinLoop] = -MergedLoop;
             
             // Now add edges of this loop to the front
          
             for ( k = 1 ; k <= FineGrid().LoopList(MinLoop).NumberOfEdges() ; k++ ) {
              
                Edge = FineGrid().LoopList(MinLoop).Edge(k);
                
                if ( EdgeIsOnFront_[Edge] == 0 ) {
                   
                   EdgeIsOnFront_[Edge] = INTERIOR_EDGE_BC;
                   
                   FrontEdgeQueue_[++NumberOfEdgesInQueue_] = Edge;
                   
                   p = FindMatchingSymmetryEdge_(Edge);
                   
                   if ( p != 0 ) {
                      
                     EdgeIsOnFront_[p] = INTERIOR_EDGE_BC;
                     
                     FrontEdgeQueue_[++NumberOfEdgesInQueue_] = p;    
                     
                   }   
                                      
                }
                
             }
 
             // Mark original loop as merged
                
             VortexLoopWasAgglomerated_[Loop] = -MergedLoop;
             
             for ( k = 1 ; k <= FineGrid().LoopList(Loop).NumberOfEdges() ; k++ ) {
              
                Edge = FineGrid().LoopList(Loop).Edge(k);
                
                if ( EdgeIsOnFront_[Edge] == 0 ) {
                   
                   EdgeIsOnFront_[Edge] = INTERIOR_EDGE_BC;
                   
                   FrontEdgeQueue_[++NumberOfEdgesInQueue_] = Edge;
                 
                   p = FindMatchingSymmetryEdge_(Edge);
                   
                   if ( p != 0 ) {
                      
                     EdgeIsOnFront_[p] = INTERIOR_EDGE_BC;
                     
                     FrontEdgeQueue_[++NumberOfEdgesInQueue_] = p;    
                     
                   }   
                                      
                }
                
             }
              
          }
                                     
       }

    }

    // Update front counters
          
    if ( EdgeIsOnFront_[NextBestEdgeOnFront_] == TE_EDGE_BC       ) NumberOfEdgesOnTE_--;
     
    if ( EdgeIsOnFront_[NextBestEdgeOnFront_] == LE_EDGE_BC       ) NumberOfEdgesOnLE_--;
        
    if ( EdgeIsOnFront_[NextBestEdgeOnFront_] == BOUNDARY_EDGE_BC ) NumberOfEdgesOnBoundary_--;
  
    // Reset current front edge to used
     
    EdgeIsOnFront_[NextBestEdgeOnFront_] *= -1;

}


/*##############################################################################
#                                                                              #
#                           VSP_AGGLOM MergeSmallLoops_                        #              
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::MergeSmallLoops_(void)
{

    int Loop1, Loop2, i, j, k, p, q, Edge, Done;
    
    q = 0;
    
    for ( Loop1 = 1 ; Loop1 <= FineGrid().NumberOfLoops() ; Loop1++ ) {
 
       if ( VortexLoopWasAgglomerated_[Loop1] > 0 ) { 

          q++;
          
       }
       
    }
            
    p = 0;
    
    for ( Loop1 = 1 ; Loop1 <= FineGrid().NumberOfLoops() ; Loop1++ ) {
 
       if ( VortexLoopWasAgglomerated_[Loop1] > 0 ) { 
                    
          i = 1;
          
          Done = 0;
          
          while ( i <= FineGrid().LoopList(Loop1).NumberOfEdges() && !Done ) {
        
             Edge = FineGrid().LoopList(Loop1).Edge(i);

             if ( !FineGrid().EdgeList(Edge).IsTrailingEdge() &&
                  !FineGrid().EdgeList(Edge).IsBoundaryEdge() &&
                  !FineGrid().EdgeList(Edge).IsLeadingEdge() ) {                
 
                Loop2 = FineGrid().EdgeList(Edge).Loop1() + FineGrid().EdgeList(Edge).Loop2() - Loop1;      
                
                if ( Loop2 > 0 && VortexLoopWasAgglomerated_[Loop2] < 0 ) {
                   
                   VortexLoopWasAgglomerated_[Loop1] = -Loop2;
                   
                   // Now add edges of this loop to the front
                   
                   for ( j = 1 ; j <= FineGrid().LoopList(Loop1).NumberOfEdges() ; j++ ) {
                    
                      k = FineGrid().LoopList(Loop1).Edge(j);
                      
                      if ( EdgeIsOnFront_[k] == 0 ) {
                         
                         EdgeIsOnFront_[k] = INTERIOR_EDGE_BC;
                     
                         FrontEdgeQueue_[++NumberOfEdgesInQueue_] = k;
                         
                      }
                      
                   }  
                                      
                   VortexLoopWasAgglomerated_[Loop2] = -Loop2;

                   // Now add edges of this loop to the front
                   
                   for ( j = 1 ; j <= FineGrid().LoopList(Loop2).NumberOfEdges() ; j++ ) {
                    
                      k = FineGrid().LoopList(Loop2).Edge(j);
                      
                      if ( EdgeIsOnFront_[k] == 0 ) {
                         
                         EdgeIsOnFront_[k] = INTERIOR_EDGE_BC;
                     
                         FrontEdgeQueue_[++NumberOfEdgesInQueue_] = k;
                         
                      }
                      
                   }  
                                         
                   Done = 1;
                   
                   p++;
                           
                }
                
             }
             
             i++;
                
          }

       }
 
    }
    
    //PRINTF("Fixed %d loops out of %d \n",p,q);

}

/*##############################################################################
#                                                                              #
#                           VSP_AGGLOM CheckLoopQuality_                       #              
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::CheckLoopQuality_(void)
{

    int i, j, NumberOfCoarseGridLoops, *CoarseGridLoop, Loop1, Loop2, LoopA, LoopB;
    int *NumberOfNeighborLoops, **NeighborLoopList, Found, Case, IsInteriorLoop;

    // Determine number of coarse grid loops
    
    CoarseGridLoop = new int[FineGrid().NumberOfLoops() + 1];
    
    zero_int_array(CoarseGridLoop, FineGrid().NumberOfLoops());
  
    NumberOfCoarseGridLoops = 0;
    
    for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
  
       if ( ABS(VortexLoopWasAgglomerated_[i]) == i ) CoarseGridLoop[i] = ++NumberOfCoarseGridLoops;
       
    }
    
    if ( NumberOfCoarseGridLoops > 1 ) {
    
       // Determine how many possible neighbor loops for each loop... this is an upper estimate

       NumberOfNeighborLoops = new int[FineGrid().NumberOfLoops() + 1];
       
       zero_int_array(NumberOfNeighborLoops, FineGrid().NumberOfLoops());

       for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {

          Loop1 = ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(i).Loop1()]);
          Loop2 = ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(i).Loop2()]);
          
          if ( Loop1 != Loop2 ) {
           
             NumberOfNeighborLoops[Loop1]++;
             NumberOfNeighborLoops[Loop2]++;
           
          }
          
       }
    
       // Allocate space for loop to neighbor loop list
       
       NeighborLoopList = new int*[FineGrid().NumberOfLoops() + 1];
       
       for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
        
          if ( NumberOfNeighborLoops[i] > 0 ) {
         
             NeighborLoopList[i] = new int[NumberOfNeighborLoops[i] + 1];
          
          }
          
       }
       
       // Now pack the list
       
       zero_int_array(NumberOfNeighborLoops, FineGrid().NumberOfLoops());
      
       for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {

          Loop1 = ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(i).Loop1()]);
          Loop2 = ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(i).Loop2()]);
          
          if ( Loop1 != Loop2 ) {
           
             for ( Case = 1 ; Case <= 2 ; Case++ ) {
              
                if ( Case == 1 ) { LoopA = Loop1; LoopB = Loop2; };
                if ( Case == 2 ) { LoopA = Loop2; LoopB = Loop1; };

                j = 1;
                
                Found = 0;
                
                while ( j <= NumberOfNeighborLoops[LoopA] && !Found ) {
                 
                   if ( LoopB == NeighborLoopList[LoopA][j] ) Found = 1;
                   
                   j++;
                   
                }
                
                if ( !Found ) {
                 
                   NumberOfNeighborLoops[LoopA]++;
                                  
                   NeighborLoopList[LoopA][NumberOfNeighborLoops[LoopA]] = LoopB;
                   
                }
                
             }

          }
          
       }    
       
       // Look for loops with just one neighbor
       
       for ( Loop1 = 1 ; Loop1 <= FineGrid().NumberOfLoops() ; Loop1++ ) {
        
          if ( NumberOfNeighborLoops[Loop1] == 1 ) {

             Loop2 = NeighborLoopList[Loop1][1];
             
             IsInteriorLoop = 1;
             
             j = 1;
             
             while ( j <= FineGrid().NumberOfEdges() && IsInteriorLoop ) {
              
                LoopA = FineGrid().EdgeList(j).Loop1();
                LoopB = FineGrid().EdgeList(j).Loop2();
                
                if ( LoopA == Loop1 || LoopB == Loop1 ) {
                 
                   if ( ABS(EdgeIsOnFront_[j]) != INTERIOR_EDGE_BC ) IsInteriorLoop = 0;
                   
                }
              
                j++;
                
             }
                
             if ( IsInteriorLoop ) {
          
PRINTF("...............................Interior loop! \n"); fflush(NULL);
          
                VortexLoopWasAgglomerated_[Loop1] = -Loop2;
                
                for ( j = 1 ; j <= FineGrid().NumberOfEdges() ; j++ ) {
                          
                   LoopA = FineGrid().EdgeList(j).Loop1();
                   LoopB = FineGrid().EdgeList(j).Loop2();
       
                   if ( ( LoopA == Loop1 && LoopB == Loop2 ) ||
                        ( LoopA == Loop2 && LoopB == Loop1 ) ) {
                    
                      if ( EdgeIsOnFront_[j] == INTERIOR_EDGE_BC ) EdgeIsOnFront_[j] = -INTERIOR_EDGE_BC;   
                   
                   }
                   
                }
                 
             }
         
          }
          
       }
       
    }

}

/*##############################################################################
#                                                                              #
#                           VSP_AGGLOM CreateCoarseMesh_                       #              
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::CreateCoarseMesh_(void)
{
    
    int i, j, k, p, Edge, Node, Node1, Node2, Loop, Loop1, Loop2, Next;
    int NumberOfCoarseGridNodes, NumberOfCoarseGridEdges, NumberOfCoarseGridLoops;
    int *KuttaNode, NumberOfKuttaNodes, *EdgeDirection;
    int *NumberOfFineGridLoops, *NumberOfEdgesForLoop;
    int NumberOfLoopNodes, *NodeListForLoop, InList, Found;
    VSPAERO_DOUBLE Area, Mag, Xb, Yb, Zb, x1, y1, z1, x2, y2, z2, Length;
    
    // Create a list of the fine edges still in use on the coarse grid
       
    for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
  
       if ( ABS(EdgeIsOnFront_[i]) == CORNER_BC        ) CoarseEdgeList_[i] = 1;
       if ( ABS(EdgeIsOnFront_[i]) == TE_EDGE_BC       ) CoarseEdgeList_[i] = 1;  
       if ( ABS(EdgeIsOnFront_[i]) == LE_EDGE_BC       ) CoarseEdgeList_[i] = 1;    
       if ( ABS(EdgeIsOnFront_[i]) == BOUNDARY_EDGE_BC ) CoarseEdgeList_[i] = 1;
       
       Loop1 = FineGrid().EdgeList(i).Loop1();
       Loop2 = FineGrid().EdgeList(i).Loop2();
      
       if ( ABS(VortexLoopWasAgglomerated_[Loop1]) != ABS(VortexLoopWasAgglomerated_[Loop2]) ) CoarseEdgeList_[i] = 1;
     
    }
    
    NumberOfCoarseGridEdges = 0;
    
    for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
     
       NumberOfCoarseGridEdges += CoarseEdgeList_[i];
       
    }
     
    // Create a list of the fine nodes still in use on the coarse grid
        
    for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
     
       if ( CoarseEdgeList_[i] ) {
        
          CoarseNodeList_[FineGrid().EdgeList(i).Node1()] = 1;
          CoarseNodeList_[FineGrid().EdgeList(i).Node2()] = 1;
          
       }
     
    }
    
    NumberOfCoarseGridNodes = 0;
    
    for ( i = 1 ; i <= FineGrid().NumberOfNodes() ; i++ ) {
     
       NumberOfCoarseGridNodes += CoarseNodeList_[i];
       
    }     
  
    // Create a list of the fine tris still in use on the coarse grid... these are really agglomerated tris/loops
    
    NumberOfCoarseGridLoops = 0;
    
    for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
  
       if ( ABS(VortexLoopWasAgglomerated_[i]) == i ) VortexLoopWasAgglomerated_[i] = ++NumberOfCoarseGridLoops;
       
    }
    
    for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
  
       if ( VortexLoopWasAgglomerated_[i] < 0 ) VortexLoopWasAgglomerated_[i] = VortexLoopWasAgglomerated_[ABS(VortexLoopWasAgglomerated_[i])];
               
    }     

    // Allocate space for the coarse grid

    CoarseGrid_ = new VSP_GRID;

    CoarseGrid().SizeNodeList(NumberOfCoarseGridNodes);
    
    CoarseGrid().SizeEdgeList(NumberOfCoarseGridEdges);
    
    CoarseGrid().SizeLoopList(NumberOfCoarseGridLoops);
    
    // Pack the nodes
    
    Next = 0;
    
    for ( i = 1 ; i <= FineGrid().NumberOfNodes() ; i++ ) {
     
       if ( CoarseNodeList_[i] ) {
        
          Next++;
          
          CoarseGrid().NodeList(Next) = FineGrid().NodeList(i);
          
          CoarseGrid().NodeList(Next).FineGridNode() = i;
          
          FineGrid().NodeList(i).CoarseGridNode() = Next;
          
          CoarseNodeList_[i] = Next;
          
       }
       
    }   
    
    // Pack the edges

    Next = 0;
    
    for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
     
       if ( CoarseEdgeList_[i] ) {
        
          Next++;
          
          // Copy over data
          
          CoarseGrid().EdgeList(Next) = FineGrid().EdgeList(i);

          // Pointer to the fine grid from the coarse grid

          CoarseGrid().EdgeList(Next).FineGridEdge() = i;
          
          // Move up one level
          
          CoarseGrid().EdgeList(Next).Level()++;
          
          // Permute the nodes to point to the coarse grid numbering
          
          CoarseGrid().EdgeList(Next).Node1() = CoarseNodeList_[FineGrid().EdgeList(i).Node1()];
          CoarseGrid().EdgeList(Next).Node2() = CoarseNodeList_[FineGrid().EdgeList(i).Node2()];

          // Loop 1, and 2
          
          CoarseGrid().EdgeList(Next).Loop1() = ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(i).Loop1()]);
          CoarseGrid().EdgeList(Next).Loop2() = ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(i).Loop2()]);
         
          // Left and right loops

          CoarseGrid().EdgeList(Next).LoopL() = ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(i).LoopL()]);
          CoarseGrid().EdgeList(Next).LoopR() = ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(i).LoopR()]);

          // Pointer to this course grid edge from fine grid
          
          FineGrid().EdgeList(i).CoarseGridEdge() = Next;
           
       }
       
       else {
          
          FineGrid().EdgeList(i).CoarseGridEdge() = 0;
          
       }
       
    }   

    // Zero out coarse grid data that needs to be agglomerated
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {

       // Area
       
       CoarseGrid().LoopList(i).Area() = 0.;
       
       // Reference length
       
       CoarseGrid().LoopList(i).RefLength() = 0.;
       
       // Loop Normal
       
       CoarseGrid().LoopList(i).Nx() = 0.;
       CoarseGrid().LoopList(i).Ny() = 0.;
       CoarseGrid().LoopList(i).Nz() = 0.; 
   
       // Camber
       
       CoarseGrid().LoopList(i).Camber() = 0.;
       
       // Centroid
       
       CoarseGrid().LoopList(i).Xc() = 0.;
       CoarseGrid().LoopList(i).Yc() = 0.;
       CoarseGrid().LoopList(i).Zc() = 0.;         
         
       // UV Centroid
       
       CoarseGrid().LoopList(i).Uc() = 0.;
       CoarseGrid().LoopList(i).Vc() = 0.;

       // Bounding box
       
       CoarseGrid().LoopList(i).BoundBox().x_min =  1.e9;
       CoarseGrid().LoopList(i).BoundBox().x_max = -1.e9;

       CoarseGrid().LoopList(i).BoundBox().y_min =  1.e9;
       CoarseGrid().LoopList(i).BoundBox().y_max = -1.e9;

       CoarseGrid().LoopList(i).BoundBox().z_min =  1.e9;
       CoarseGrid().LoopList(i).BoundBox().z_max = -1.e9;     
     
    }

    // Agglomerate, area weighted, the fine grid data to the coarse grid
    
    for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {

       Next = ABS(VortexLoopWasAgglomerated_[i]);

       // Fine grid things that don't change
       
       CoarseGrid().LoopList(Next).SurfaceID()   = FineGrid().LoopList(i).SurfaceID();   
       CoarseGrid().LoopList(Next).ComponentID() = FineGrid().LoopList(i).ComponentID();     
       CoarseGrid().LoopList(Next).SpanStation() = FineGrid().LoopList(i).SpanStation(); 
       CoarseGrid().LoopList(Next).SurfaceType() = FineGrid().LoopList(i).SurfaceType(); 
       CoarseGrid().LoopList(Next).DegenBodyID() = FineGrid().LoopList(i).DegenBodyID(); 
       CoarseGrid().LoopList(Next).DegenWingID() = FineGrid().LoopList(i).DegenWingID();        
       CoarseGrid().LoopList(Next).Cart3dID()    = FineGrid().LoopList(i).Cart3dID();        

       // Area
       
       Area = FineGrid().LoopList(i).Area();
       
       CoarseGrid().LoopList(Next).Area() += Area;
       
       // Reference Length
       
       CoarseGrid().LoopList(Next).RefLength() += Area*FineGrid().LoopList(i).RefLength();

       // Loop Normal
       
       CoarseGrid().LoopList(Next).Nx() += Area*FineGrid().LoopList(i).Nx();
       CoarseGrid().LoopList(Next).Ny() += Area*FineGrid().LoopList(i).Ny();
       CoarseGrid().LoopList(Next).Nz() += Area*FineGrid().LoopList(i).Nz();
  
       // Camber
       
       CoarseGrid().LoopList(Next).Camber() += Area*FineGrid().LoopList(i).Camber();

       // Centroid
       
       CoarseGrid().LoopList(Next).Xc() += Area*FineGrid().LoopList(i).Xc();
       CoarseGrid().LoopList(Next).Yc() += Area*FineGrid().LoopList(i).Yc();
       CoarseGrid().LoopList(Next).Zc() += Area*FineGrid().LoopList(i).Zc();     
       
       // UV Centroid
       
       CoarseGrid().LoopList(Next).Uc() += Area*FineGrid().LoopList(i).Uc();
       CoarseGrid().LoopList(Next).Vc() += Area*FineGrid().LoopList(i).Vc();      
       
       // Bounding box information
       
       CoarseGrid().LoopList(Next).BoundBox().x_min = MIN(CoarseGrid().LoopList(Next).BoundBox().x_min, FineGrid().LoopList(i).BoundBox().x_min );
       CoarseGrid().LoopList(Next).BoundBox().x_max = MAX(CoarseGrid().LoopList(Next).BoundBox().x_max, FineGrid().LoopList(i).BoundBox().x_max );

       CoarseGrid().LoopList(Next).BoundBox().y_min = MIN(CoarseGrid().LoopList(Next).BoundBox().y_min, FineGrid().LoopList(i).BoundBox().y_min );
       CoarseGrid().LoopList(Next).BoundBox().y_max = MAX(CoarseGrid().LoopList(Next).BoundBox().y_max, FineGrid().LoopList(i).BoundBox().y_max );

       CoarseGrid().LoopList(Next).BoundBox().z_min = MIN(CoarseGrid().LoopList(Next).BoundBox().z_min, FineGrid().LoopList(i).BoundBox().z_min );
       CoarseGrid().LoopList(Next).BoundBox().z_max = MAX(CoarseGrid().LoopList(Next).BoundBox().z_max, FineGrid().LoopList(i).BoundBox().z_max );
       
       // Pointer from fine grid to coarse
                
       FineGrid().LoopList(i).CoarseGridLoop() = Next;     

    }  
   
    // Size the bounding box

    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {

       box_calculate_size(CoarseGrid().LoopList(i).BoundBox());

    }    
     
    // Area average the results
  
    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {

       // Area
            
       Area = CoarseGrid().LoopList(i).Area();
       
       // Length
       
       CoarseGrid().LoopList(i).Length() = sqrt(CoarseGrid().LoopList(i).Area());
       
       // Reference length;
       
       CoarseGrid().LoopList(i).RefLength() /= Area;

       // Loop Normal
       
       Mag = sqrt(vector_dot(CoarseGrid().LoopList(i).Normal(), CoarseGrid().LoopList(i).Normal()));
        
       CoarseGrid().LoopList(i).Nx() /= Mag;
       CoarseGrid().LoopList(i).Ny() /= Mag;
       CoarseGrid().LoopList(i).Nz() /= Mag;

       // Camber
       
       CoarseGrid().LoopList(i).Camber() /= Area;   
       
       // Centroid
       
       CoarseGrid().LoopList(i).Xc() /= Area;
       CoarseGrid().LoopList(i).Yc() /= Area;
       CoarseGrid().LoopList(i).Zc() /= Area; 
       
       // UV Centroid
       
       CoarseGrid().LoopList(i).Uc() /= Area;
       CoarseGrid().LoopList(i).Vc() /= Area;   
 
    }    
    
    // Calculate centroid offset
  
    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {
    
       Xb = 0.5*( CoarseGrid().LoopList(i).BoundBox().x_max + CoarseGrid().LoopList(i).BoundBox().x_min );
       Yb = 0.5*( CoarseGrid().LoopList(i).BoundBox().y_max + CoarseGrid().LoopList(i).BoundBox().y_min );
       Zb = 0.5*( CoarseGrid().LoopList(i).BoundBox().z_max + CoarseGrid().LoopList(i).BoundBox().z_min );
       
       CoarseGrid().LoopList(i).CentroidOffSet() = sqrt( pow(CoarseGrid().LoopList(i).Xc() - Xb,2.)
                                                       + pow(CoarseGrid().LoopList(i).Yc() - Yb,2.)
                                                       + pow(CoarseGrid().LoopList(i).Zc() - Zb,2.) );       

    }            
    
    // Create a list of all fine grid loops agglomerated for each coarse grid loop

    NumberOfFineGridLoops = new int[CoarseGrid().NumberOfLoops() + 1];
    
    zero_int_array(NumberOfFineGridLoops, CoarseGrid().NumberOfLoops());

    for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
  
       Loop = FineGrid().LoopList(i).CoarseGridLoop();
      
       NumberOfFineGridLoops[Loop]++;
               
    }   

    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {

       CoarseGrid().LoopList(i).SizeFineGridLoopList(NumberOfFineGridLoops[i]);
       
       NumberOfFineGridLoops[i] = 0;
               
    } 

    for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
   
       Loop = FineGrid().LoopList(i).CoarseGridLoop();

       NumberOfFineGridLoops[Loop]++;
    
       CoarseGrid().LoopList(Loop).FineGridLoop(NumberOfFineGridLoops[Loop]) = i;
     
    }       
  
    delete [] NumberOfFineGridLoops;

    // Create a list of edges for each loop
    
    NumberOfEdgesForLoop = new int[CoarseGrid().NumberOfLoops() + 1];
    
    zero_int_array(NumberOfEdgesForLoop, CoarseGrid().NumberOfLoops());
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfEdges() ; i++ ) {
     
       Loop1 = CoarseGrid().EdgeList(i).Loop1();
       Loop2 = CoarseGrid().EdgeList(i).Loop2();
       
       if ( Loop1 != 0 && Loop1 != Loop2 ) NumberOfEdgesForLoop[Loop1] += 1;
       if ( Loop2 != 0                   ) NumberOfEdgesForLoop[Loop2] += 1;
       
    }

    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {

       CoarseGrid().LoopList(i).SizeEdgeList(NumberOfEdgesForLoop[i]);
       
       NumberOfEdgesForLoop[i] = 0;

    }

    for ( i = 1 ; i <= CoarseGrid().NumberOfEdges() ; i++ ) {
     
       Loop1 = CoarseGrid().EdgeList(i).Loop1();
       Loop2 = CoarseGrid().EdgeList(i).Loop2();
       
       if ( Loop1 != 0 && Loop1 != Loop2 ) {
        
          NumberOfEdgesForLoop[Loop1]++;
          
          CoarseGrid().LoopList(Loop1).Edge(NumberOfEdgesForLoop[Loop1]) = i;
          
       }
       
       if ( Loop2 != 0 ) {
        
          NumberOfEdgesForLoop[Loop2]++;
          
          CoarseGrid().LoopList(Loop2).Edge(NumberOfEdgesForLoop[Loop2]) = i;
          
       }       
       
    }

    delete [] NumberOfEdgesForLoop;

    // Create a list of nodes for each loop
    
    NodeListForLoop = new int[CoarseGrid().NumberOfNodes() + 1];

    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {

       NumberOfLoopNodes = 0;

       for ( j = 1 ; j <= CoarseGrid().LoopList(i).NumberOfEdges() ; j++ ) {
          
          Edge = CoarseGrid().LoopList(i).Edge(j);
          
          for ( k = 1 ; k <= 2 ; k++ ) {
             
             if ( k == 1 ) Node = CoarseGrid().EdgeList(Edge).Node1();
             if ( k == 2 ) Node = CoarseGrid().EdgeList(Edge).Node2();
             
             p = 1;
             
             InList = 0;
             
             while ( p <= NumberOfLoopNodes && !InList ) {
             
                if ( NodeListForLoop[p] == Node ) InList = 1;
                
                p++;
                
             }
             
             if ( !InList ) NodeListForLoop[++NumberOfLoopNodes] = Node;
             
          }

       }

       CoarseGrid().LoopList(i).SizeNodeList(NumberOfLoopNodes);

       for ( j = 1 ; j <= NumberOfLoopNodes ; j++ ) {
   
          CoarseGrid().LoopList(i).Node(j) = NodeListForLoop[j];
             
       }
       
       // Sort out the UV values on the coarse grid
       
       for ( j = 1 ; j <= CoarseGrid().LoopList(i).NumberOfNodes() ; j++ ) {
          
          Node = CoarseGrid().LoopList(i).Node(j);
       
          Found = 0;
          
          k = 1;
          
          while ( k <= CoarseGrid().LoopList(i).NumberOfFineGridLoops() && !Found ) {
             
             Loop = CoarseGrid().LoopList(i).FineGridLoop(k);
             
             for ( p = 1 ; p <= FineGrid().LoopList(Loop).NumberOfNodes() ; p++ ) {
       
                if ( CoarseNodeList_[FineGrid().LoopList(Loop).Node(p)] == Node ) {
                   
                   CoarseGrid().LoopList(i).U_Node(j) = FineGrid().LoopList(Loop).U_Node(p);
                   CoarseGrid().LoopList(i).V_Node(j) = FineGrid().LoopList(Loop).V_Node(p);
         
                   Found = 1;
                   
                }
                
             }
             
             k++;
             
          }
          
          if ( !Found ) {
             
             PRINTF("Error in determing UV surface mapping during agglomeration! \n");
             fflush(NULL);
             exit(1);
             
          }
          
       }
      
    } 
    
    // Recalculate the loop length using the nodal data
    
    for ( k = 1 ; k <= CoarseGrid().NumberOfLoops() ; k++ ) {
    
       Length = 0.;
       
       for ( i = 1 ; i <= CoarseGrid().LoopList(k).NumberOfNodes() ; i++ ) {
          
          Node1 =  CoarseGrid().LoopList(k).Node(i);
          
          x1 = CoarseGrid().NodeList(Node1).x();
          y1 = CoarseGrid().NodeList(Node1).y();
          z1 = CoarseGrid().NodeList(Node1).z();
       
          for ( j = 1 ; j <= CoarseGrid().LoopList(k).NumberOfNodes() ; j++ ) {
       
             Node2 = CoarseGrid().LoopList(k).Node(j);
       
             x2 = CoarseGrid().NodeList(Node2).x();
             y2 = CoarseGrid().NodeList(Node2).y();
             z2 = CoarseGrid().NodeList(Node2).z();
       
             Length = MAX(Length,sqrt( pow(x1-x2,2.) + pow(y1-y2,2.) + pow(z1-z2,2.) ));
             
          }
          
       }
       
       CoarseGrid().LoopList(k).Length() = MAX(Length, CoarseGrid().LoopList(k).Length());
       
    }

    

/*     
    // Create a list of nodes for each loop
    
    NumberOfNodesForLoop = new int[CoarseGrid().NumberOfNodes() + 1];
   
    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {
       
       zero_int_array(NumberOfNodesForLoop, CoarseGrid().NumberOfNodes());
       
       for ( j = 1 ; j <= CoarseGrid().LoopList(i).NumberOfEdges() ; j++ ) {
          
          Edge = CoarseGrid().LoopList(i).Edge(j);
          
          NumberOfNodesForLoop[CoarseGrid().EdgeList(Edge).Node1()] = 1;
          NumberOfNodesForLoop[CoarseGrid().EdgeList(Edge).Node2()] = 1;
          
       }
       
       NumberOfLoopNodes = 0;
       
       for ( j = 1 ; j <= CoarseGrid().NumberOfNodes() ; j++ ) {
          
          NumberOfLoopNodes += NumberOfNodesForLoop[j];
          
       }
       
       CoarseGrid().LoopList(i).SizeNodeList(NumberOfLoopNodes);
       
       NumberOfLoopNodes = 0;
       
       for ( j = 1 ; j <= CoarseGrid().NumberOfNodes() ; j++ ) {
          
          if ( NumberOfNodesForLoop[j] ) {
             
             CoarseGrid().LoopList(i).Node(++NumberOfLoopNodes) = j;
             
             NumberOfNodesForLoop[j] = 0;
             
          }
          
       }

    }       
    
    delete [] NumberOfNodesForLoop;
 */
     
    // Store direction of each edge for each coarse grid loop
    
    EdgeDirection = new int[FineGrid().NumberOfEdges() + 1];

    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {

       // Loop over fine grid loops agglomerated into this coarse grid loop
       
       for ( j = 1 ; j <= CoarseGrid().LoopList(i).NumberOfFineGridLoops() ; j++ ) {
          
          Loop = CoarseGrid().LoopList(i).FineGridLoop(j);
          
          // Loop over edges for this loop, store directions
          
          for ( k = 1 ; k <= FineGrid().LoopList(Loop).NumberOfEdges() ; k++ ) {
             
             EdgeDirection[FineGrid().LoopList(Loop).Edge(k)] = FineGrid().LoopList(Loop).EdgeDirection(k);
             
          }
          
       }
       
       // Now update coarse grid directions
       
       for ( k = 1 ; k <= CoarseGrid().LoopList(i).NumberOfEdges() ; k++ ) {
          
          j = CoarseGrid().LoopList(i).Edge(k);
     
          CoarseGrid().LoopList(i).EdgeDirection(k) = EdgeDirection[CoarseGrid().EdgeList(j).FineGridEdge()];
          
       }
               
    }  
    
    delete [] EdgeDirection;

    // Determine how many kutta nodes there are on the coarse grid
    
    KuttaNode = new int[CoarseGrid().NumberOfNodes() + 1];
    
    zero_int_array(KuttaNode, CoarseGrid().NumberOfNodes());
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfEdges() ; i++ ) {
     
       if ( CoarseGrid().EdgeList(i).IsTrailingEdge() ) {
        
          Node1 = CoarseGrid().EdgeList(i).Node1();
          Node2 = CoarseGrid().EdgeList(i).Node2();

          Loop = MAX(CoarseGrid().EdgeList(i).Loop1(), CoarseGrid().EdgeList(i).Loop2());
          
          // Node 1
          
          if ( KuttaNode[Node1] == 0 ) {
           
             KuttaNode[Node1]  = Loop;
             
          }
          
          else {
           
             KuttaNode[Node1] -= Loop;
           
          }

          // Node 2
          
          if ( KuttaNode[Node2] == 0 ) {
           
             KuttaNode[Node2]  = Loop;
             
          }
          
          else {
           
             KuttaNode[Node2] -= Loop;
           
          }
          
       }
       
    }  
 
    NumberOfKuttaNodes = 0;
    
    for ( i = 1 ; i <= FineGrid().NumberOfKuttaNodes() ; i++ ) {

       Node = FineGrid().NodeList(FineGrid().KuttaNode(i)).CoarseGridNode();

       if ( KuttaNode[Node] != 0 ) {
        
          NumberOfKuttaNodes++;
          
       }
       
    }

    // Copy over any kutta node information

    CoarseGrid().SizeKuttaNodeList(NumberOfKuttaNodes);
            
    NumberOfKuttaNodes = 0;

    for ( i = 1 ; i <= FineGrid().NumberOfKuttaNodes() ; i++ ) {

       Node = FineGrid().NodeList(FineGrid().KuttaNode(i)).CoarseGridNode();
    
       if ( KuttaNode[Node] != 0 ) {
        
          NumberOfKuttaNodes++;
          
          CoarseGrid().KuttaNode(NumberOfKuttaNodes) = Node;
 
          CoarseGrid().WingSurfaceForKuttaNode(NumberOfKuttaNodes) = FineGrid().WingSurfaceForKuttaNode(i);

          CoarseGrid().ComponentIDForKuttaNode(NumberOfKuttaNodes) = FineGrid().ComponentIDForKuttaNode(i);

          CoarseGrid().WingSurfaceForKuttaNodeIsPeriodic(NumberOfKuttaNodes) = FineGrid().WingSurfaceForKuttaNodeIsPeriodic(i);
        
          CoarseGrid().WakeTrailingEdgeX(NumberOfKuttaNodes) = FineGrid().WakeTrailingEdgeX(i);
          CoarseGrid().WakeTrailingEdgeY(NumberOfKuttaNodes) = FineGrid().WakeTrailingEdgeY(i);
          CoarseGrid().WakeTrailingEdgeZ(NumberOfKuttaNodes) = FineGrid().WakeTrailingEdgeZ(i);
      
          CoarseGrid().KuttaNodeSoverB(NumberOfKuttaNodes) = FineGrid().KuttaNodeSoverB(i);
          
          CoarseGrid().KuttaNodeIsOnWingTip(NumberOfKuttaNodes) = FineGrid().KuttaNodeIsOnWingTip(i);
          
       }
       
    }
   
    zero_int_array(EdgeDegree_, FineGrid().NumberOfNodes());
    
    // Create node degree array
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfEdges() ; i++ ) {
       
       EdgeDegree_[CoarseGrid().EdgeList(i).Node1()]++;
       EdgeDegree_[CoarseGrid().EdgeList(i).Node2()]++;
       
    }    
    
    // Min loop size constraint
    
    CoarseGrid().MinLoopArea() = FineGrid().MinLoopArea();
    
    // Surface type flag
    
    CoarseGrid().SurfaceType() = FineGrid().SurfaceType();

    // Free up memory
    
    delete [] KuttaNode;
    
}

/*##############################################################################
#                                                                              #
#                      VSP_AGGLOM DeleteDuplicateNodes_                        #              
#                                                                              #
##############################################################################*/

VSP_GRID* VSP_AGGLOM::DeleteDuplicateNodes_(VSP_GRID &Grid)
{
   
    int i, j, NumberOfNodes, NumberOfEdges, NumberOfLoops;
    int *NodeIsUsed, *EdgeIsUsed, *NodePerm, Node1, Node2, Node3; 
    VSPAERO_DOUBLE dS, Epsilon;
    VSP_GRID *NewGrid;
    
    NodeIsUsed = new int[Grid.NumberOfNodes() + 1];
    
    NodePerm = new int[Grid.NumberOfNodes() + 1];
    
    for ( i = 1 ; i <= Grid.NumberOfNodes() ; i++ ) {
       
       NodeIsUsed[i] = i;
       
       NodePerm[i] = i;
       
    }
    
    // Loop over the edges and find any that are zero length
      
    Epsilon = 1.e-8;
    
    for ( i = 1 ; i <= Grid.NumberOfEdges() ; i++ ) {
       
       Node1 = Grid.EdgeList(i).Node1(); 
       Node2 = Grid.EdgeList(i).Node2();
       
       if ( NodeIsUsed[Node1] == Node1 || NodeIsUsed[Node2] == Node2 ) {
       
          dS = sqrt( pow(Grid.NodeList(Node2).x() - Grid.NodeList(Node1).x(), 2.)
                   + pow(Grid.NodeList(Node2).y() - Grid.NodeList(Node1).y(), 2.)
                   + pow(Grid.NodeList(Node2).z() - Grid.NodeList(Node1).z(), 2.) );
                   
          if ( dS <= Epsilon ) {
             
             if ( NodeIsUsed[Node1] == Node1 && NodeIsUsed[Node2] == Node2 ) {
                
                NodeIsUsed[Node1] = -Node1;
                
                NodeIsUsed[Node2] = -Node1;
                
             }
             
             else {
                
                if ( NodeIsUsed[Node1] < 0 ) {
                   
                   NodeIsUsed[Node2] = NodeIsUsed[Node1];
                   
                }
                
                else {
                   
                   NodeIsUsed[Node1] = NodeIsUsed[Node2];
                   
                }
                
             }
             
          }
          
       }
                     
    }

    // Determine the number of unique edges
    
    NumberOfEdges = 0;
    
    EdgeIsUsed = new int[Grid.NumberOfEdges() + 1];
    
    zero_int_array(EdgeIsUsed, Grid.NumberOfEdges());
    
    for ( i = 1 ; i <= Grid.NumberOfEdges() ; i++ ) {
       
       Node1 = Grid.EdgeList(i).Node1(); 
       Node2 = Grid.EdgeList(i).Node2();
       
       if ( ABS(NodeIsUsed[Node1]) != ABS(NodeIsUsed[Node2]) ) EdgeIsUsed[i] = 1; 
       
    }
    
    for ( i = 1 ; i <= Grid.NumberOfEdges() ; i++ ) {
       
       if ( EdgeIsUsed[i] ) EdgeIsUsed[i] = ++NumberOfEdges;
       
    }    
   
    if ( NumberOfEdges == Grid.NumberOfEdges() ) return &Grid;
        
    PRINTF("Found %d unique edges out of: %d \n",NumberOfEdges, Grid.NumberOfEdges());
                
    // Create new coarse mesh
    
    NewGrid = new VSP_GRID;

    // Create list of unique coarse edges

    NewGrid->SizeEdgeList(NumberOfEdges);

    NumberOfEdges = 0;

    for ( i = 1 ; i <= Grid.NumberOfEdges() ; i++ ) {
       
       // Edge was not merged... just copy it over
       
       if ( EdgeIsUsed[i] ) {
 
          NumberOfEdges++;
          
          NewGrid->EdgeList(NumberOfEdges) = Grid.EdgeList(i);

       }
       
    }
    
    // Create list of unique nodes
    
    NumberOfNodes = 0;
    
    for ( i = 1 ; i <= Grid.NumberOfNodes() ; i++ ) {
       
       if ( ABS(NodeIsUsed[i]) == i ) ++NumberOfNodes;
       
    } 
    
    NewGrid->SizeNodeList(NumberOfNodes);
    
    NumberOfNodes = 0;

    for ( i = 1 ; i <= Grid.NumberOfNodes() ; i++ ) {
       
       if ( ABS(NodeIsUsed[i]) == i ) {
          
          NewGrid->NodeList(++NumberOfNodes) = Grid.NodeList(i);
       
       }
 
    }     
   
    // Create node permutation array
    
    NumberOfNodes = 0;
    
    for ( i = 1 ; i <= Grid.NumberOfNodes() ; i++ ) {
       
       if ( ABS(NodeIsUsed[i]) == i ) NodePerm[i] = ++NumberOfNodes;
       
    }
 
    for ( i = 1 ; i <= Grid.NumberOfNodes() ; i++ ) {
       
       if ( NodeIsUsed[i] < 0 ) NodePerm[i] = NodePerm[-NodeIsUsed[i]];
       
    }    
       
    // Update edge node data
  
    for ( i = 1 ; i <= NewGrid->NumberOfEdges() ; i++ ) {

       NewGrid->EdgeList(i).Node1() = NodePerm[NewGrid->EdgeList(i).Node1()];
       NewGrid->EdgeList(i).Node2() = NodePerm[NewGrid->EdgeList(i).Node2()];

    }
   
    // Update loop edge data
    
    NumberOfLoops = 0;
    
    for ( i = 1 ; i <= Grid.NumberOfLoops() ; i++ ) {
            
       Node1 = NodePerm[Grid.LoopList(i).Node1()]; 
       Node2 = NodePerm[Grid.LoopList(i).Node2()]; 
       Node3 = NodePerm[Grid.LoopList(i).Node3()]; 
  
       if ( Node1 != Node2 && Node1 != Node3 && Node2 != Node3 ) NumberOfLoops++;
       
    }
   
    NewGrid->SizeLoopList(NumberOfLoops);
  
    NumberOfLoops = 0;
    
    for ( i = 1 ; i <= Grid.NumberOfLoops() ; i++ ) {
              
       Node1 = NodePerm[Grid.LoopList(i).Node1()]; 
       Node2 = NodePerm[Grid.LoopList(i).Node2()]; 
       Node3 = NodePerm[Grid.LoopList(i).Node3()]; 
  
       if ( Node1 != Node2 && Node1 != Node3 && Node2 != Node3 ) {
          
          NewGrid->LoopList(++NumberOfLoops) = Grid.LoopList(i);
          
          for ( j = 1 ; j <= Grid.LoopList(i).NumberOfNodes() ; j++ ) {
          
             NewGrid->LoopList(i).Node(j) = NodePerm[NewGrid->LoopList(i).Node(j)];
             
          }
          
       }         
       
    }

    delete [] NodeIsUsed;
    delete [] NodePerm;
    delete [] EdgeIsUsed;
    delete [] FineGrid_;
    
    return NewGrid;

}

/*##############################################################################
#                                                                              #
#                      VSP_AGGLOM MergeCoLinearEdges_                          #              
#                                                                              #
##############################################################################*/

VSP_GRID* VSP_AGGLOM::MergeCoLinearEdges_(void)
{
   
    int Loop, i, j, k, Edge1, Edge2, CurrentEdge, StackSize, NumberOfLoopEdges;
    int Loop1, Loop2, LoopA, LoopB, UpdateFineGrid, FineGridNode, Next, Side;
    int NumberOfNodes, NumberOfEdges, NumberOfEdgesMerged, NumberOfLoopNodes;
    int Node1, Node2, NodeA, NodeB, *NodeIsUsed, *EdgeIsUsed, FineGridEdge, CommonNode;
    VSP_GRID *NewGrid;
    EDGE_STACK_LIST *StackList, *EdgeIsMerged;
    
    EdgeIsMerged = new EDGE_STACK_LIST[CoarseGrid().NumberOfEdges() + 1];
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfEdges() ; i++ ) {
       
       EdgeIsMerged[i].Edge = i;
       EdgeIsMerged[i].Side[0] = 0;
       EdgeIsMerged[i].Side[1] = 0;
       
    }
    
    NodeIsUsed = new int[CoarseGrid().NumberOfNodes() + 1];
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfNodes() ; i++ ) {
       
       NodeIsUsed[i] = 1;
       
    }
    
    StackList = new EDGE_STACK_LIST[CoarseGrid().NumberOfEdges() + 1];
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfEdges() ; i++ ) {
       
       StackList[i].Edge = 0;
       StackList[i].Side[0] = 0;
       StackList[i].Side[1] = 0;
       
    }
  
    // Mark any edges on loops with only 3 edges
    
    for ( Loop = 1 ; Loop <= CoarseGrid().NumberOfLoops() ; Loop++ ) {
       
       if ( CoarseGrid().LoopList(Loop).NumberOfEdges() == 3 ) {
 
          for ( i = 1 ; i <= CoarseGrid().LoopList(Loop).NumberOfEdges() ; i++ ) {
             
             Edge1 = CoarseGrid().LoopList(Loop).Edge(i);

             EdgeIsMerged[Edge1].Edge = -Edge1;
             
          }
          
       }
       
    }                 
     
    // Loop over the loops... and try to merge edges that are nearly colinear
   
    for ( Loop = 1 ; Loop <= CoarseGrid().NumberOfLoops() ; Loop++ ) {
       
       if ( CoarseGrid().LoopList(Loop).NumberOfEdges() > 3 ) {
          
          NumberOfEdgesMerged = 0;
          
          for ( i = 1 ; i <= CoarseGrid().LoopList(Loop).NumberOfEdges() ; i++ ) {
             
             Edge1 = CoarseGrid().LoopList(Loop).Edge(i);
                                            
             // Only look an unmerged edges ... and no trailing edges
             
             if (  EdgeIsMerged[Edge1].Edge > 0                  &&
                  !CoarseGrid().EdgeList(Edge1).IsTrailingEdge() &&
                  !CoarseGrid().EdgeList(Edge1).IsBoundaryEdge() &&
                  !CoarseGrid().EdgeList(Edge1).IsLeadingEdge()     ) {               

                // Mark it as merged... at least to itself
                
                EdgeIsMerged[Edge1].Edge = -Edge1;
                
                // Add edge 1 to the stack
                
                Next = StackSize = 1;
                
                StackList[StackSize].Edge = Edge1;
                StackList[StackSize].Side[0] = 1;
                StackList[StackSize].Side[1] = 2;
                
                // Work on the stack
                
                while ( Next <= StackSize && StackSize > 0 ) {
                   
                  CurrentEdge = StackList[Next].Edge;
                  
                  Loop1 = MIN(CoarseGrid().EdgeList(CurrentEdge).Loop1(),CoarseGrid().EdgeList(CurrentEdge).Loop2());
                  Loop2 = MAX(CoarseGrid().EdgeList(CurrentEdge).Loop1(),CoarseGrid().EdgeList(CurrentEdge).Loop2());
    
                  Node1 = CoarseGrid().EdgeList(CurrentEdge).Node1(); 
                  Node2 = CoarseGrid().EdgeList(CurrentEdge).Node2();
                  
                  // Find left and right neighbors
                  
                  for ( j = 1 ; j <= CoarseGrid().LoopList(Loop).NumberOfEdges() ; j++ ) {
                     
                     Edge2 = CoarseGrid().LoopList(Loop).Edge(j);
                     
                     if ( EdgeIsMerged[Edge2].Edge > 0 &&
                        !CoarseGrid().EdgeList(Edge2).IsTrailingEdge() &&
                        !CoarseGrid().EdgeList(Edge2).IsBoundaryEdge() &&
                        !CoarseGrid().EdgeList(Edge2).IsLeadingEdge() ) {       
                                            
                        NodeA = CoarseGrid().EdgeList(Edge2).Node1(); 
                        NodeB = CoarseGrid().EdgeList(Edge2).Node2();
                        
                        CommonNode = 0;
                        
                        if ( Node1 == NodeA ) CommonNode = Node1;
                        if ( Node1 == NodeB ) CommonNode = Node1;        
                        if ( Node2 == NodeA ) CommonNode = Node2;
                        if ( Node2 == NodeB ) CommonNode = Node2;   
                                                
                        if ( CommonNode && EdgeDegree_[CommonNode] == 2 ) {
                   
                           // Edges must border same two loops
                                              
                           LoopA = MIN(CoarseGrid().EdgeList(Edge2).Loop1(),CoarseGrid().EdgeList(Edge2).Loop2());
                           LoopB = MAX(CoarseGrid().EdgeList(Edge2).Loop1(),CoarseGrid().EdgeList(Edge2).Loop2());
                              
                           if ( Loop1 == LoopA && Loop2 == LoopB ) {
                            
                             // Edges must be colinear
                            
                              if ( EdgesAreColinear_(CoarseGrid(), CurrentEdge, Edge2, 10.) ) {       
                                 
                                 if ( NumberOfEdgesMerged + 1 < CoarseGrid().LoopList(Loop).NumberOfEdges() - 3 ) {
                                    
                                    // Add it to the stack
                                    
                                    if ( StackSize + 1 > CoarseGrid().NumberOfEdges() ) {
                                       
                                       PRINTF("wtf! \n");fflush(NULL);
                                       exit(1);
                                       
                                    }
                                    
                                    StackSize++;
                                    
                                    StackList[StackSize].Edge = Edge2;
                                    
                                    if ( CommonNode == Node1 ) Side = StackList[StackSize].Side[0] = StackList[StackSize].Side[1] = StackList[Next].Side[0];
                                    
                                    if ( CommonNode == Node2 ) Side = StackList[StackSize].Side[0] = StackList[StackSize].Side[1] = StackList[Next].Side[1];
                                    
                                    // Merge this edge in
                               
                                    EdgeIsMerged[Edge2].Edge = -Edge1;
                                    
                                    EdgeIsMerged[Edge2].Side[0] = EdgeIsMerged[Edge2].Side[1] = Side;
                                    
                                    NumberOfEdgesMerged++;
                                    
                                    // Mark the common node as now un-used
                                                                        
                                    NodeIsUsed[CommonNode] = 0;  
                                  
                                 }                              
                                 
                              }
                              
                           }
                           
                        }
                        
                     }
                     
                  }
                  
                  Next++;
                  
                }
                
             }
             
             if ( CoarseGrid().LoopList(Loop).NumberOfEdges() - NumberOfEdgesMerged <= 2 ) {
                
                PRINTF("CoarseGrid().LoopList(Loop).NumberOfEdges(): %d \n",CoarseGrid().LoopList(Loop).NumberOfEdges());
                PRINTF("NumberOfEdgesMerged: %d \n",NumberOfEdgesMerged);
                
                PRINTF("Merged down to just 2 edges... wtf! \n");fflush(NULL);
                exit(1);
                
             }
             
          }
          
       }
       
    }

    delete [] StackList;
    
    // Create an edge to merged edge list 
    
    int *NumberOfMergedEdgesForEdge, **MergedEdgeListForEdge;
    
    NumberOfMergedEdgesForEdge = new int[CoarseGrid().NumberOfEdges() + 1];
    
    MergedEdgeListForEdge = new int*[CoarseGrid().NumberOfEdges() + 1];
    
    zero_int_array(NumberOfMergedEdgesForEdge, CoarseGrid().NumberOfEdges());
    
    for ( j = 1 ; j <= CoarseGrid().NumberOfEdges() ; j++ ) {
       
       NumberOfMergedEdgesForEdge[ABS(EdgeIsMerged[j].Edge)]++;
       
    }

    for ( j = 1 ; j <= CoarseGrid().NumberOfEdges() ; j++ ) {
       
       MergedEdgeListForEdge[j] = new int[NumberOfMergedEdgesForEdge[j] + 1];
       
       NumberOfMergedEdgesForEdge[j] = 0;
       
    }

    for ( j = 1 ; j <= CoarseGrid().NumberOfEdges() ; j++ ) {
       
       i = ABS(EdgeIsMerged[j].Edge);
       
       NumberOfMergedEdgesForEdge[i]++;
       
       MergedEdgeListForEdge[i][NumberOfMergedEdgesForEdge[i]] = j;
       
    }

    // Determine the number of unique edges
    
    NumberOfEdges = 0;
    
    EdgeIsUsed = new int[CoarseGrid().NumberOfEdges() + 1];
    
    zero_int_array(EdgeIsUsed, CoarseGrid().NumberOfEdges());
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfEdges() ; i++ ) {
       
       if ( ABS(EdgeIsMerged[i].Edge) == i ) EdgeIsUsed[i] = 1;
       
    }
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfEdges() ; i++ ) {
       
       if ( EdgeIsUsed[i] ) EdgeIsUsed[i] = ++NumberOfEdges;
       
    }    
        
    // Create new coarse mesh
    
    NewGrid = new VSP_GRID;

    // Create list of unique coarse edges

    NewGrid->SizeEdgeList(NumberOfEdges);

    NumberOfEdges = 0;
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfEdges() ; i++ ) {
       
       UpdateFineGrid = 0;
       
       // Edge was not merged... just copy it over
       
       if ( EdgeIsMerged[i].Edge == i ) {
 
          NumberOfEdges++;
          
          NewGrid->EdgeList(NumberOfEdges) = CoarseGrid().EdgeList(i);
          
          UpdateFineGrid = 1;
 
       }
 
       // Edge was merged... copy over, and zero node numbers
       
       else if ( EdgeIsMerged[i].Edge == -i ) {
         
          NumberOfEdges++;
          
          NewGrid->EdgeList(NumberOfEdges) = CoarseGrid().EdgeList(i);
                    
          NewGrid->EdgeList(NumberOfEdges).Node1() = 0;
          
          NewGrid->EdgeList(NumberOfEdges).Node2() = 0;

          UpdateFineGrid = 1;
          
       }   
       
       // Update the fine grid coarse grid edges if needed
       
       if ( UpdateFineGrid ) {
        
        /*
          for ( j = 1 ; j <= CoarseGrid().NumberOfEdges() ; j++ ) {
             
             if ( ABS(EdgeIsMerged[j].Edge) == i ) {
            
                FineGridEdge = CoarseGrid().EdgeList(j).FineGridEdge();
          
                FineGrid().EdgeList(FineGridEdge).CoarseGridEdge() = NumberOfEdges;
                
             }
             
          }
       */
       
          for ( j = 1 ; j <= NumberOfMergedEdgesForEdge[i] ; j++ ) {
             
             FineGridEdge = CoarseGrid().EdgeList(MergedEdgeListForEdge[i][j]).FineGridEdge();

             FineGrid().EdgeList(FineGridEdge).CoarseGridEdge() = NumberOfEdges;
             
          }
        
       }       
       
    }
 
    for ( i = 1 ; i <= CoarseGrid().NumberOfEdges() ; i++ ) {
       
       if ( EdgeIsMerged[i].Edge < 0 ) {
          
          j = EdgeIsUsed[-EdgeIsMerged[i].Edge];
          
          Node1 = CoarseGrid().EdgeList(i).Node1();
          Node2 = CoarseGrid().EdgeList(i).Node2();
            
          // Edge is not really merged
                         
          if ( NodeIsUsed[Node1] && NodeIsUsed[Node2] ) {
         
             NewGrid->EdgeList(j).Node1() = Node1;
             NewGrid->EdgeList(j).Node2() = Node2;
             
          }
          
          // Otherwise, determine the two new end nodes
          
          else {
             
             if ( EdgeIsMerged[i].Side[0] == 0 ) {
                
                if ( NodeIsUsed[Node1] ) NewGrid->EdgeList(j).Node1() = Node1;
                if ( NodeIsUsed[Node2] ) NewGrid->EdgeList(j).Node2() = Node2;
                
             }
             
             else if ( EdgeIsMerged[i].Side[0] == 1 ) {
                
                if ( NodeIsUsed[Node1] ) NewGrid->EdgeList(j).Node1() = Node1;
                if ( NodeIsUsed[Node2] ) NewGrid->EdgeList(j).Node1() = Node2;
                
             }
             
             else if ( EdgeIsMerged[i].Side[0] == 2 ) {

                if ( NodeIsUsed[Node1] ) NewGrid->EdgeList(j).Node2() = Node1;
                if ( NodeIsUsed[Node2] ) NewGrid->EdgeList(j).Node2() = Node2;
                
             }
             
             else {
                
                PRINTF("wtf! \n");
                PRINTF("EdgeIsMerged[i].Side is not 0, 1, or 2! \n");
                fflush(NULL);
                exit(1);
                
             }                
          
          }             
          
       }
       
    }
        
    // Create list of unique nodes
    
    NumberOfNodes = 0;
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfNodes() ; i++ ) {
       
       if ( NodeIsUsed[i] ) NodeIsUsed[i] = ++NumberOfNodes;
       
    } 
    
    NewGrid->SizeNodeList(NumberOfNodes);
 
    for ( i = 1 ; i <= CoarseGrid().NumberOfNodes() ; i++ ) {
       
       if ( NodeIsUsed[i] ) {
          
          NewGrid->NodeList(NodeIsUsed[i]) = CoarseGrid().NodeList(i);
          
          FineGridNode = CoarseGrid().NodeList(i).FineGridNode();
          
          FineGrid().NodeList(FineGridNode).CoarseGridNode() = NodeIsUsed[i];
          
       }
       
       else {
          
          FineGridNode = CoarseGrid().NodeList(i).FineGridNode();
          
          FineGrid().NodeList(FineGridNode).CoarseGridNode() = 0.;
          
       }
       
    }     
        
    // Update edge node data
  
    for ( i = 1 ; i <= NewGrid->NumberOfEdges() ; i++ ) {

       NewGrid->EdgeList(i).Node1() = NodeIsUsed[NewGrid->EdgeList(i).Node1()];
       NewGrid->EdgeList(i).Node2() = NodeIsUsed[NewGrid->EdgeList(i).Node2()];

    }
    
    // Update loop edge data
    
    NewGrid->SizeLoopList(CoarseGrid().NumberOfLoops());
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {
       
       // Copy over stuff that does not change
       
       NewGrid->LoopList(i) = CoarseGrid().LoopList(i);
       
       // Determine the number of edges that define this loop
       
       NumberOfLoopEdges = 0;
       
       for ( j = 1 ; j <= CoarseGrid().LoopList(i).NumberOfEdges() ; j++ ) {
          
          k = CoarseGrid().LoopList(i).Edge(j);
     
          if ( EdgeIsUsed[k] ) NumberOfLoopEdges++;
          
       }
       
       if ( NumberOfLoopEdges < 3 ) {
          
          PRINTF("NumberOfLoopEdges: %d for Loop: %d \n",NumberOfLoopEdges,i);fflush(NULL);
          PRINTF("NumberOfLoopEdges for original mesh was: %d \n",CoarseGrid().LoopList(i).NumberOfEdges());

       }
       
       NewGrid->LoopList(i).SizeEdgeList(NumberOfLoopEdges);
       
       NumberOfLoopEdges = 0;
       
       for ( j = 1 ; j <= CoarseGrid().LoopList(i).NumberOfEdges() ; j++ ) {
          
          k = CoarseGrid().LoopList(i).Edge(j);
          
          if ( EdgeIsUsed[k] ) {
             
             NewGrid->LoopList(i).Edge(++NumberOfLoopEdges) = EdgeIsUsed[k];
             
             NewGrid->LoopList(i).EdgeDirection(NumberOfLoopEdges) = CoarseGrid().LoopList(i).EdgeDirection(j);
             
          }
          
       }
       
    } 
    
    // Update loop node list

    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {

       NumberOfLoopNodes = 0;
       
       for ( j = 1 ; j <= CoarseGrid().LoopList(i).NumberOfNodes() ; j++ ) {
          
          if ( NodeIsUsed[CoarseGrid().LoopList(i).Node(j)] ) NumberOfLoopNodes++;

       }

       NewGrid->LoopList(i).SizeNodeList(NumberOfLoopNodes);
       
       NumberOfLoopNodes = 0;
       
       for ( j = 1 ; j <= CoarseGrid().LoopList(i).NumberOfNodes() ; j++ ) {
          
          if ( NodeIsUsed[CoarseGrid().LoopList(i).Node(j)] ) {
             
             NewGrid->LoopList(i).Node(++NumberOfLoopNodes) = NodeIsUsed[CoarseGrid().LoopList(i).Node(j)];

          }
          
       }

    }        

    zero_int_array(EdgeDegree_, FineGrid().NumberOfNodes());
    
    // Create node degree array
    
    for ( i = 1 ; i <= NewGrid->NumberOfEdges() ; i++ ) {
       
       EdgeDegree_[NewGrid->EdgeList(i).Node1()]++;
       EdgeDegree_[NewGrid->EdgeList(i).Node2()]++;
       
    }        

    delete [] EdgeIsMerged;
    delete [] NodeIsUsed;

    for ( j = 1 ; j <= CoarseGrid().NumberOfEdges() ; j++ ) {
     
       if ( NumberOfMergedEdgesForEdge[j] > 0 ) delete [] MergedEdgeListForEdge[j];
       
    }   
    
    delete [] MergedEdgeListForEdge;
    delete [] NumberOfMergedEdgesForEdge;
    
    // Min loop size constraint
    
    NewGrid->MinLoopArea() = CoarseGrid().MinLoopArea();

    return NewGrid;

}

/*##############################################################################
#                                                                              #
#                      VSP_AGGLOM  CreateMixedMesh_                            #
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::CreateMixedMesh_(void)
{
 
    int i, Edge, Edge2, Loop, NeighborLoop, Iter, Done;
    int Node1, Node2, Node3, Node4, NodeA, NodeB;
    int NumberOfLoopsMerged, BestNeighborLoop;   
    int *LoopOnTrailingEdge;     
    VSPAERO_DOUBLE Angle, BestAngle, Dot;

    GoodQuadAngle_  = 100.*TORAD;
    WorstQuadAngle_ = 135.*TORAD;
    
    // Mark tris near trailing edges for CART3D meshes
    
    LoopOnTrailingEdge = new int[FineGrid().NumberOfLoops() + 1];

    zero_int_array(LoopOnTrailingEdge, FineGrid().NumberOfLoops());

    if ( FineGrid().SurfaceType() == CART3D_SURFACE || FineGrid().SurfaceType() == VSPGEOM_SURFACE ) {
       
       for ( Loop = 1 ; Loop <= FineGrid().NumberOfLoops() ; Loop++ ) {
          
          for ( Edge = 1 ; Edge <= 3 ; Edge++ ) {
           
             i = FineGrid().LoopList(Loop).Edge(Edge);
          
             if ( FineGrid().EdgeList(i).IsTrailingEdge() ) LoopOnTrailingEdge[Loop] = 1;
          
          }    
             
       }
   
       // Mark tris near sharp edges
   
       for ( Loop = 1 ; Loop <= FineGrid().NumberOfLoops() ; Loop++ ) {
          
          for ( Edge = 1 ; Edge <= 3 ; Edge++ ) {
           
             FindNeighborLoopOnLocalEdge_(FineGrid(), Loop, Edge, NeighborLoop, NodeA, NodeB);
             
             Dot = vector_dot(FineGrid().LoopList(Loop).Normal(),FineGrid().LoopList(NeighborLoop).Normal());
           
             if ( Dot < 0.1 ) {
           
                LoopOnTrailingEdge[Loop] = 1;
                
             }
          
          }    
             
       }
           
       // Buffer out
           
       for ( Iter = 1 ; Iter <= 4 ; Iter++ ) {
          
          for ( Loop = 1 ; Loop <= FineGrid().NumberOfLoops() ; Loop++ ) {
             
             if ( LoopOnTrailingEdge[Loop] == 1 ) {
                
                for ( Edge = 1 ; Edge <= 3 ; Edge++ ) {
           
                   FindNeighborLoopOnLocalEdge_(FineGrid(), Loop, Edge, NeighborLoop, NodeA, NodeB);
                   
                   if ( LoopOnTrailingEdge[NeighborLoop] == 0 ) {
                      
                      LoopOnTrailingEdge[NeighborLoop] = 2;
                      
                   }
                   
                }
                
             }
             
          }
          
          for ( Loop = 1 ; Loop <= FineGrid().NumberOfLoops() ; Loop++ ) {
             
             if ( LoopOnTrailingEdge[Loop] > 0 ) LoopOnTrailingEdge[Loop] = 1;
             
          }
          
       }
       
    }
          
    // Loop over all tris and merge any that can create decent quads

    NumberOfLoopsMerged = 0;
    
    BestNeighborLoop = 0;

    Loop = 1;
    
    while ( Loop <= FineGrid().NumberOfLoops() ) {
       
       // Don't merge trailing edge tris on panel solves

       if ( FineGrid().LoopList(Loop).NumberOfEdges() == 3 && VortexLoopWasAgglomerated_[Loop] > 0 ) {
        
          Node1 = FineGrid().LoopList(Loop).Node1();
          Node2 = FineGrid().LoopList(Loop).Node2();
          Node3 = FineGrid().LoopList(Loop).Node3();

          BestAngle = 1.e9;
          
          for ( Edge = 1 ; Edge <= 3 ; Edge++ ) {
        
             FindNeighborLoopOnLocalEdge_(FineGrid(), Loop, Edge, NeighborLoop, NodeA, NodeB);

             if ( Loop != NeighborLoop && FineGrid().LoopList(NeighborLoop).NumberOfEdges() == 3 ) {

                if ( FineGrid().LoopList(NeighborLoop).SpanStation() == FineGrid().LoopList(Loop).SpanStation() ) {
                   
                   if ( LoopsAreCoplanar_(FineGrid(), Loop, NeighborLoop, 5.) ) {
             
                      if ( VortexLoopWasAgglomerated_[NeighborLoop] > 0 ) {
                       
                         Node4 = FineGrid().LoopList(NeighborLoop).Node1()
                               + FineGrid().LoopList(NeighborLoop).Node2()
                               + FineGrid().LoopList(NeighborLoop).Node3() - NodeA - NodeB;
                               
                         if ( Edge == 1 ) Angle = CalculateQuadQuality_(FineGrid(),Node3, Node1, Node4, Node2);
                         if ( Edge == 2 ) Angle = CalculateQuadQuality_(FineGrid(),Node1, Node2, Node4, Node3);
                         if ( Edge == 3 ) Angle = CalculateQuadQuality_(FineGrid(),Node2, Node3, Node4, Node1);
    
                         if ( Angle <= BestAngle ) {
                            
                            BestAngle = Angle;
                            
                            BestNeighborLoop = NeighborLoop;
                            
                         }
                
                      }
                      
                   }
                   
                }
                
             }
             
          }
                     
          if ( BestAngle <= GoodQuadAngle_ ) {

             VortexLoopWasAgglomerated_[Loop] = -Loop;

             VortexLoopWasAgglomerated_[BestNeighborLoop] = -Loop;
             
             NumberOfLoopsMerged++;

          }
          
       }
       
       Loop++;
       
    }
    
    delete [] LoopOnTrailingEdge;

}

/*##############################################################################
#                                                                              #
#                            VSP_AGGLOM  CheckMesh_                            #
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::CheckMesh_(VSP_GRID &ThisGrid)
{
 
    int i, Loop1, Loop2;
  
    // Loop over edges and look for consistency
  
    for ( i = 1 ; i <= ThisGrid.NumberOfEdges() ; i++ ) {
       
       if ( ThisGrid.EdgeList(i).IsBoundaryEdge() != VLM_BOUNDARY_EDGE ) {
          
          Loop1 = ThisGrid.EdgeList(i).Loop1();
          Loop2 = ThisGrid.EdgeList(i).Loop2();
   
          if ( 0&&Loop1 == Loop2 ) {
             
             PRINTF("Edge: %d is messed up! \n",i);
             
             PRINTF("Loop1,2: %d %d \n",Loop1,Loop2);
             
             PRINTF("Node 1,2: %d %d --> xyz: %f %f %f .. %f %f %f \n",
             ThisGrid.EdgeList(i).Node1(),
             ThisGrid.EdgeList(i).Node2(),
             ThisGrid.NodeList(ThisGrid.EdgeList(i).Node1()).x(),
             ThisGrid.NodeList(ThisGrid.EdgeList(i).Node1()).y(),
             ThisGrid.NodeList(ThisGrid.EdgeList(i).Node1()).z(),
             ThisGrid.NodeList(ThisGrid.EdgeList(i).Node2()).x(),
             ThisGrid.NodeList(ThisGrid.EdgeList(i).Node2()).y(),
             ThisGrid.NodeList(ThisGrid.EdgeList(i).Node2()).z());
             
             fflush(NULL);
             
        //     exit(1);
             
          }
          
       }
       
    }

}

/*##############################################################################
#                                                                              #
#                          VSP_AGGLOM  CleanUpMesh_                            #
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::CleanUpMesh_(void)
{
   
    int Edge, Loop1, Loop2;
    
    // Create a list of those nodes on surface borders
    
    NodeOnSurfaceBorder_ = new int[FineGrid().NumberOfNodes() + 1];
    
    zero_int_array(NodeOnSurfaceBorder_, FineGrid().NumberOfNodes());
    
    for ( Edge = 1 ; Edge <= FineGrid().NumberOfEdges() ; Edge++ ) {
       
       Loop1 = FineGrid().EdgeList(Edge).Loop1();
       Loop2 = FineGrid().EdgeList(Edge).Loop2();
       
       if ( FineGrid().LoopList(Loop1).SpanStation() != FineGrid().LoopList(Loop2).SpanStation() ) {
          
          NodeOnSurfaceBorder_[FineGrid().EdgeList(Edge).Node1()] = 1;
          NodeOnSurfaceBorder_[FineGrid().EdgeList(Edge).Node2()] = 1;
          
       }
  
    }
    
    NumberOfLoopsMerged_ = 0;
   
    CleanUpFans_();
  
    CleanUpHighAspectRatioTris_();
  
    CleanUpSmallAreaLoops_();
    
    CheckForDegenerateNodes_();
    
    delete [] NodeOnSurfaceBorder_;
     
}

/*##############################################################################
#                                                                              #
#                          VSP_AGGLOM  CleanUpFans_                            #
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::CleanUpFans_(void)
{
 
    int i, j, Loop, Node, Node1, Node2, Node3, iNode;
    int NodeA, NodeB, NodeC, CurrentLoop, Done, MinLoop;
    int *NumberOfLoopsForNode, **NodeToLoopList, *LoopList, StackSize, Next;
    VSPAERO_DOUBLE **NodeToLoopAngleList, MinAngle;
    VSPAERO_DOUBLE LimitAngle, TotalAngle, MinArea, MaxArea, AspectRatio;

    // Create a node to loop tri list
    
    NumberOfLoopsForNode = new int[FineGrid().NumberOfNodes() + 1];
    
    NodeToLoopList = new int*[FineGrid().NumberOfNodes() + 1];
    
    NodeToLoopAngleList = new VSPAERO_DOUBLE*[FineGrid().NumberOfNodes() + 1];
    
    zero_int_array(NumberOfLoopsForNode, FineGrid().NumberOfNodes());
    
    for ( Loop = 1 ; Loop <= FineGrid().NumberOfLoops() ; Loop++ ) {
       
       for ( j = 1 ; j <= FineGrid().LoopList(Loop).NumberOfNodes() ; j++ ) {
          
          Node = FineGrid().LoopList(Loop).Node(j);
          
          NumberOfLoopsForNode[Node]++;
          
       }
       
    }
    
    for ( i = 1 ; i <= FineGrid().NumberOfNodes() ; i++ ) {
       
       NodeToLoopList[i] = new int[NumberOfLoopsForNode[i] + 1];
       
       NodeToLoopAngleList[i] = new VSPAERO_DOUBLE[NumberOfLoopsForNode[i] + 1];
       
    }
    
    zero_int_array(NumberOfLoopsForNode, FineGrid().NumberOfNodes());
    
    for ( Loop = 1 ; Loop <= FineGrid().NumberOfLoops() ; Loop++ ) {
       
       for ( j = 1 ; j <= FineGrid().LoopList(Loop).NumberOfNodes() ; j++ ) {
          
          Node = FineGrid().LoopList(Loop).Node(j);
       
          NumberOfLoopsForNode[Node]++;
          
          NodeToLoopList[Node][NumberOfLoopsForNode[Node]] = Loop;
          
          NodeToLoopAngleList[Node][NumberOfLoopsForNode[Node]] = CalculateLoopAngle_(FineGrid(), Loop, Node);
          
       }
       
    }    
    
    // Loop over nodes and find one with fans
    
    LimitAngle = 15.;

    LoopList = new int[ FineGrid().NumberOfLoops() + 1];
    
    for ( i = 1 ; i <= FineGrid().NumberOfNodes() ; i ++ ) {
       
       if ( NodeOnSurfaceBorder_[i] == 0 ) {
          
          Done = 0;
          
          while ( !Done ) {
             
             // Find tri with the smallest angle
             
             MinAngle = 1.e9;
             
             MinLoop = 0;
             
             MinArea =  1.e9;
             MaxArea = -1.e9;
             
             for ( j = 1 ; j <= NumberOfLoopsForNode[i]; j++ ) {
                
                Loop = NodeToLoopList[i][j];
                          
                if ( Loop > 0 && VortexLoopWasAgglomerated_[Loop] > 0 && NodeToLoopAngleList[i][j] <= MinAngle ) {
                   
                   MinAngle = NodeToLoopAngleList[i][j];
                   
                   MinLoop = j;
                   
                   MinArea = FineGrid().LoopList(Loop).Area();
                   
                }
                
                if ( Loop > 0 && VortexLoopWasAgglomerated_[Loop] ) {
                
                   MaxArea = MAX(FineGrid().LoopList(Loop).Area(),MaxArea);
                   
                }
                
             }
             
             // Calculate aspect ratio

             AspectRatio = MaxArea / MinArea;
                                                                          
             // If the minimum angle is small and it has a high aspect ration...then try and merge this tri with a neighbor
            
             if ( MinAngle <= LimitAngle && AspectRatio > 10. ) {

                StackSize = 1;
                
                LoopList[StackSize] = NodeToLoopList[i][MinLoop];
                
                NodeToLoopList[i][MinLoop] *= -1;
                
                MinLoop = LoopList[StackSize];
                
                Next = 1;
                
                TotalAngle = MinAngle;
                
                // Search the neighbors of the current tri on the stack
                
                while ( Next <= StackSize ) {
                   
                   CurrentLoop = LoopList[Next];
                   
                   if ( i == FineGrid().LoopList(CurrentLoop).Node1() ) {
                      
                      Node1 = FineGrid().LoopList(CurrentLoop).Node1();
                      Node2 = FineGrid().LoopList(CurrentLoop).Node2();
                      Node3 = FineGrid().LoopList(CurrentLoop).Node3();
                      
                   }
                   
                   else if ( i == FineGrid().LoopList(CurrentLoop).Node2() ) {
       
                      Node1 = FineGrid().LoopList(CurrentLoop).Node2();
                      Node2 = FineGrid().LoopList(CurrentLoop).Node1();
                      Node3 = FineGrid().LoopList(CurrentLoop).Node3();
                      
                   }   
                   
                   else if ( i == FineGrid().LoopList(CurrentLoop).Node3() ) {
       
                      Node1 = FineGrid().LoopList(CurrentLoop).Node3();
                      Node2 = FineGrid().LoopList(CurrentLoop).Node1();
                      Node3 = FineGrid().LoopList(CurrentLoop).Node2();
                      
                   }  
                   
                   else {
                                      
                      PRINTF("wtf... starting loop is messed up! \n");fflush(NULL);
                      exit(1);
                      
                   }
                   
                   // Look over all the loops attached to this node
                  
                   for ( j = 1 ; j <= NumberOfLoopsForNode[i] ; j++ ) {
                                      
                      Loop = ABS(NodeToLoopList[i][j]);
                                      
                      if ( Loop != CurrentLoop && NodeToLoopList[i][j] > 0 ) {
                        
                         if ( i == FineGrid().LoopList(Loop).Node1() ) {
                            
                            NodeA = FineGrid().LoopList(Loop).Node1();
                            NodeB = FineGrid().LoopList(Loop).Node2();
                            NodeC = FineGrid().LoopList(Loop).Node3();
                            
                         }
                         
                         else if ( i == FineGrid().LoopList(Loop).Node2() ) {
               
                            NodeA = FineGrid().LoopList(Loop).Node2();
                            NodeB = FineGrid().LoopList(Loop).Node1();
                            NodeC = FineGrid().LoopList(Loop).Node3();
                            
                         }   
                         
                         else if ( i == FineGrid().LoopList(Loop).Node3() ) {
               
                            NodeA = FineGrid().LoopList(Loop).Node3();
                            NodeB = FineGrid().LoopList(Loop).Node1();
                            NodeC = FineGrid().LoopList(Loop).Node2();
                            
                         } 
                         
                         else {
                                            
                            PRINTF("wtf... next loop is messed up! \n");fflush(NULL);
                            exit(1);
                            
                         }                   
                         
                         // Check if the two loops are adjacent
                         
                         iNode = 0;
                         
                         if ( Node2 == NodeB || Node2 == NodeC ) iNode = Node2;
                         
                         if ( Node3 == NodeB || Node3 == NodeC ) iNode = Node3;
                         
                         if ( iNode > 0 && NodeOnSurfaceBorder_[iNode] ) {
                        
                            // Check if the loops are nearly coplanar
                                          
                            if ( LoopsAreCoplanar_(FineGrid(), CurrentLoop, Loop, 5.) ) {      
                                      
                               // Check if the new total angle is ok
                    
                               if ( ( TotalAngle + NodeToLoopAngleList[i][j] <= 60.                   ) ||
                                    ( TotalAngle + NodeToLoopAngleList[i][j] <= 90. && MinAngle <= 5. )    ) {   
                               
                                  // Neighbor loop is not already agglomerated
              
                                  if ( VortexLoopWasAgglomerated_[Loop] > 0 ) { 
                                                                 
                                     VortexLoopWasAgglomerated_[CurrentLoop] = -MinLoop;
                           
                                     VortexLoopWasAgglomerated_[Loop] = -MinLoop;
                                        
                                     LoopList[++StackSize] = Loop;
                                                       
                                     TotalAngle += NodeToLoopAngleList[i][j]; 
                                              
                                     NumberOfLoopsMerged_++; 
                                     
                                  }    
                                                                                                      
                               }   
                            
                            }

                         }
                         
                      }
                      
                   }
                   
                   Next++;
                   
                }
                
             }
             
             else {
                
                Done = 1;
                
             }
             
          }
          
       }
       
    }

    for ( i = 1 ; i <= FineGrid().NumberOfNodes() ; i++ ) {
       
       delete [] NodeToLoopList[i];
       
       delete [] NodeToLoopAngleList[i];
       
    }
    
    delete [] NodeToLoopList;
    
    delete [] NodeToLoopAngleList;
    
    delete [] NumberOfLoopsForNode;
    
    delete [] LoopList;
 
}

/*##############################################################################
#                                                                              #
#                  VSP_AGGLOM  CleanUpHighAspectRatioTris_                     #
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::CleanUpHighAspectRatioTris_(void)
{
 
    int Case, Done, Edge, Loop, CurrentLoop, NeighborLoop, MinEdge;
    int LoopList[2], EdgeList[2];
    VSPAERO_DOUBLE MaxAR;
    
    MaxAR = 10.;
    
    // Loop over all tris and merge ones with poor aspect ratios with other loops
    
    for ( Loop = 1 ; Loop <= FineGrid().NumberOfLoops() ; Loop++ ) {
     
       if ( VortexLoopWasAgglomerated_[Loop] > 0 ) {
        
          if ( BadTriangle_(FineGrid(), Loop, MinEdge) ) {
             
             if ( MinEdge == 1 ) {
             
                EdgeList[0] = FineGrid().LoopList(Loop).Edge2();
                EdgeList[1] = FineGrid().LoopList(Loop).Edge3();   
                
             }
             
             else if ( MinEdge == 2 ) {
             
                EdgeList[0] = FineGrid().LoopList(Loop).Edge1();
                EdgeList[1] = FineGrid().LoopList(Loop).Edge3();   
                
             }
             
             else if ( MinEdge == 3 ) {
             
                EdgeList[0] = FineGrid().LoopList(Loop).Edge1();
                EdgeList[1] = FineGrid().LoopList(Loop).Edge2();   
                
             }    
             
             else {
                
                PRINTF("wtf... something went wrong in the high AR code... \n");fflush(NULL);
                
                exit(1);
                
             }         
                
             LoopList[0] = LoopList[1] = Loop;
                
             // Try both neighbors to merge with... 
             
             Done = 0;
             
             Case = 0;
             
             while ( !Done ) {
             
                CurrentLoop = LoopList[Case];
             
                Edge = EdgeList[Case];
                
                if ( Edge > 0 ) {
                   
                   EdgeList[Case] = 0;
                
                   NeighborLoop = FineGrid().EdgeList(Edge).Loop1() + FineGrid().EdgeList(Edge).Loop2() - CurrentLoop;
                   
                   if ( CurrentLoop == FineGrid().EdgeList(Edge).Loop1() ) {
                      
                   }
                   
                   else if ( CurrentLoop == FineGrid().EdgeList(Edge).Loop2() ) {
                      
                   }
                   
                   else {
                      
                      PRINTF("wtf with the neighbor loop! \n");fflush(NULL);
                      PRINTF("Case: %d \n",Case);
                      PRINTF("LoopList[%d]: %d \n",Case,LoopList[Case]);
                      PRINTF("Loop: %d \n",Loop);
                      PRINTF("CurrentLoop: %d \n",CurrentLoop);
                      PRINTF("Edge: %d \n",Edge);
                      PRINTF("EdgeList[0]: %d \n",EdgeList[0]);
                      PRINTF("EdgeList[1]: %d \n",EdgeList[1]);
                      PRINTF("FineGrid().EdgeList(%d).Loop1(): %d \n",Edge,FineGrid().EdgeList(Edge).Loop1());
                      PRINTF("FineGrid().EdgeList(%d).Loop2(): %d \n",Edge,FineGrid().EdgeList(Edge).Loop2());
                      
                      exit(1);
                      
                   }
                   
                   if ( VortexLoopWasAgglomerated_[NeighborLoop] > 0 ) {
                      
                      // Only merge loops on the same surface
                  
                      if ( FineGrid().LoopList(NeighborLoop).SpanStation() == FineGrid().LoopList(CurrentLoop).SpanStation() ) {
                      
                         if ( BadTriangle_(FineGrid(), NeighborLoop, MinEdge) ) {
                            
                            if ( Edge != FineGrid().LoopList(NeighborLoop).Edge(MinEdge) ) {
                               
                               // Only merge loops that are nearly coplanar
                            
                               if ( LoopsAreCoplanar_(FineGrid(), CurrentLoop, NeighborLoop, 5.) ) {
           
                                  VortexLoopWasAgglomerated_[Loop] = -Loop;
                        
                                  VortexLoopWasAgglomerated_[NeighborLoop] = -Loop;
                    
                                  NumberOfLoopsMerged_++;
                                  
                                  // Now try and add in a new edge
                                  
                                  if ( MinEdge == 1 ) {
                                  
                                     if ( Edge == FineGrid().LoopList(NeighborLoop).Edge2() ) EdgeList[Case] = FineGrid().LoopList(NeighborLoop).Edge3();
                                     if ( Edge == FineGrid().LoopList(NeighborLoop).Edge3() ) EdgeList[Case] = FineGrid().LoopList(NeighborLoop).Edge2();   
                                     
                                  }
                                  
                                  else if ( MinEdge == 2 ) {
                                  
                                     if ( Edge == FineGrid().LoopList(NeighborLoop).Edge1() ) EdgeList[Case] = FineGrid().LoopList(NeighborLoop).Edge3();
                                     if ( Edge == FineGrid().LoopList(NeighborLoop).Edge3() ) EdgeList[Case] = FineGrid().LoopList(NeighborLoop).Edge1();   
                                    
                                  }
                                  
                                  else if ( MinEdge == 3 ) {
                                  
                                     if ( Edge == FineGrid().LoopList(NeighborLoop).Edge1() ) EdgeList[Case] = FineGrid().LoopList(NeighborLoop).Edge2();
                                     if ( Edge == FineGrid().LoopList(NeighborLoop).Edge2() ) EdgeList[Case] = FineGrid().LoopList(NeighborLoop).Edge1();   
                                    
                                  }    
                                  
                                  else {
                                     
                                     PRINTF("wtf... something went wrong in the high AR code... \n");fflush(NULL);
                                     
                                     exit(1);
                                     
                                  }      
                                  
                                  LoopList[Case] = NeighborLoop;
                                  
                                  if ( EdgeList[Case] == 0 ) {
                                     
                                     PRINTF("wtf! \n");fflush(NULL);
                                     
                                     PRINTF("Edge: %d \n",Edge);
                                     PRINTF("FineGrid().LoopList(NeighborLoop).Edge1(): %d \n",FineGrid().LoopList(NeighborLoop).Edge1());
                                     PRINTF("FineGrid().LoopList(NeighborLoop).Edge2(): %d \n",FineGrid().LoopList(NeighborLoop).Edge2());
                                     PRINTF("FineGrid().LoopList(NeighborLoop).Edge3(): %d \n",FineGrid().LoopList(NeighborLoop).Edge3());
                                     
                                     exit(1);
                                     
                                  }                             
                                                           
                               }
                            
                            }                         
                            
                         }
                         
                      }
                      
                   }
                   
                }
                
                Case++;
                
                if ( Case > 1 ) Case = 0;
                
                if ( EdgeList[0] == 0 && EdgeList[1] == 0 ) Done = 1;
                
             }
             
          }
          
       }
       
    }
                      
}

/*##############################################################################
#                                                                              #
#                  VSP_AGGLOM  CleanUpHighAspectRatioQuads_                    #
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::CleanUpHighAspectRatioQuads_(void)
{
 
    int i, Done, Loop1, Loop2, LoopList1[2], LoopList2[2], FixedOne;

    // Loop over all tris and merge ones with poor aspect ratios with other loops
    
    FixedOne = 0;
    
    for ( Loop1 = 1 ; Loop1 <= FineGrid().NumberOfLoops() ; Loop1++ ) {
     
       if ( VortexLoopWasAgglomerated_[Loop1] > 0 ) {
     
          if ( BadQuad_(FineGrid(), Loop1, LoopList1) ) {

             Done = 0;
             
             i = 0;
             
             while ( !Done && i <= 1 ) {
             
                Loop2 = LoopList1[i];
            
                if ( VortexLoopWasAgglomerated_[Loop2] > 0 && BadQuad_(FineGrid(), Loop2, LoopList2) ) {
              
                   FixedOne++;
                
                   VortexLoopWasAgglomerated_[Loop1] = -Loop1;
                   
                   VortexLoopWasAgglomerated_[Loop2] = -Loop1;
                   
                   NumberOfLoopsMerged_++;
                    
                   Done = 1;
                   
                }
                                   
                i++;
                 
             }
             
          }
          
       }
       
    }
    
    PRINTF("Merged %d high aspect ratio quads \n",FixedOne);

}

/*##############################################################################
#                                                                              #
#                   VSP_AGGLOM  CleanUpSmallAreaLoops_                         #
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::CleanUpSmallAreaLoops_(void)
{
 
    int i, j, Done, BestLoop, Loop, NeighborLoop,  NodeA, NodeB, MergedLoop;
    int Iter, SmallLoop;
    VSPAERO_DOUBLE Area, *MergedLoopArea, MaxRatio, Vec[3], EdgeLength, MaxLength;
    
    // Calculate areas for new merged loops
    
    MergedLoopArea = new VSPAERO_DOUBLE[FineGrid().NumberOfLoops() + 1];
    
    MaxRatio = 100.;
    
    Done = 0;
    
    Iter = 1;
    
    while ( !Done ) {
       
       Done = 1;
       
       zero_double_array(MergedLoopArea, FineGrid().NumberOfLoops());
       
       for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
          
          j = ABS(VortexLoopWasAgglomerated_[i]);
          
          MergedLoopArea[j] += FineGrid().LoopList(i).Area();
          
       }
       
       for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
          
          j = ABS(VortexLoopWasAgglomerated_[i]);
          
          if ( i != j ) MergedLoopArea[i] = MergedLoopArea[j];
          
       }    
             
       // Now loop over merged loops... and merge any small loops next to big loops
   
       for ( Loop = 1 ; Loop <= FineGrid().NumberOfLoops() ; Loop++) {
   
          Area = MergedLoopArea[Loop];

          // Find the best neighbor to merge with
          
          BestLoop = 0;
                    
          MaxLength = -1.e9;
  
          SmallLoop = 0;
            
          for ( i = 1 ; i <= FineGrid().LoopList(Loop).NumberOfEdges() ; i++ ) {
          
             FindNeighborLoopOnLocalEdge_(FineGrid(), Loop, i, NeighborLoop, NodeA, NodeB);

             if ( MergedLoopArea[NeighborLoop]/Area > MaxRatio ) {
                
                SmallLoop = 1;
                
             }

          }
          
          if ( SmallLoop ) {
             
             for ( i = 1 ; i <= FineGrid().LoopList(Loop).NumberOfEdges() ; i++ ) {
             
                FindNeighborLoopOnLocalEdge_(FineGrid(), Loop, i, NeighborLoop, NodeA, NodeB);
        
                if ( VortexLoopWasAgglomerated_[Loop] != VortexLoopWasAgglomerated_[NeighborLoop] ) {
                   
                   if ( FineGrid().LoopList(NeighborLoop).SpanStation() == FineGrid().LoopList(Loop).SpanStation() ) {
                   
                      if ( LoopsAreCoplanar_(FineGrid(), Loop, NeighborLoop, 5.) ) {
                      
                         Vec[0] = FineGrid().NodeList(NodeA).x() - FineGrid().NodeList(NodeB).x();
                         Vec[1] = FineGrid().NodeList(NodeA).y() - FineGrid().NodeList(NodeB).y();
                         Vec[2] = FineGrid().NodeList(NodeA).z() - FineGrid().NodeList(NodeB).z();
                         
                         EdgeLength = sqrt(vector_dot(Vec,Vec));
                          
                         // Merge along the largest shared edge
                         
                         if ( EdgeLength > MaxLength ) {
                            
                            BestLoop = NeighborLoop;
                            
                            MaxLength = EdgeLength;
                            
                         }
                         
                      }
                      
                   }
                   
                }
                
             }
             
          }
          
          // If we found a neighbor loop... merge with it
          
          if ( BestLoop > 0 ) {
             
             NeighborLoop = BestLoop;
            
             // Neighbor loop is itself... so just merge with it
             
             if ( ABS(VortexLoopWasAgglomerated_[NeighborLoop]) == NeighborLoop ) {

                VortexLoopWasAgglomerated_[NeighborLoop] = -NeighborLoop;
      
                // This loop is not agglomerated with some other... so just merge
                
                if ( VortexLoopWasAgglomerated_[Loop] > 0 ) {
                   
                   VortexLoopWasAgglomerated_[Loop] = -NeighborLoop;
                   
                }
                
                // This loop was agglomerated with some other... so we must merge, and update other previously
                // merged loops into one set
                
                else {
                   
                   MergedLoop = VortexLoopWasAgglomerated_[Loop];
                   
                   for ( j = 1 ; j <= FineGrid().NumberOfLoops() ; j++ ) {
                      
                      if ( VortexLoopWasAgglomerated_[j] == MergedLoop ) VortexLoopWasAgglomerated_[j] = -NeighborLoop;
                      
                   }
                   
                }
                 
             }
             
             // Otherwise, the neighbor loop was merged already with another loop
             
             else {
                
                NeighborLoop = ABS(VortexLoopWasAgglomerated_[NeighborLoop]);
      
                // This loop is not agglomerated with some other... so just merge
      
                if ( VortexLoopWasAgglomerated_[Loop] > 0 ) {
                   
                   VortexLoopWasAgglomerated_[Loop] = -NeighborLoop;
                   
                }
                
                // This loop was agglomerated with some other... so we must merge, and update other previously
                // merged loops into one set
                                
                else {
                   
                   MergedLoop = VortexLoopWasAgglomerated_[Loop];
                   
                   for ( j = 1 ; j <= FineGrid().NumberOfLoops() ; j++ ) {
                      
                      if ( VortexLoopWasAgglomerated_[j] == MergedLoop ) VortexLoopWasAgglomerated_[j] = -NeighborLoop;
                      
                   }
                   
                }
                
             }
                                        
             NumberOfLoopsMerged_++;
             
             Done = 0;       
             
          }
   
       }
       
       MaxRatio *= 5.;
       
       Iter++;
       
    }

    delete [] MergedLoopArea;

}

/*##############################################################################
#                                                                              #
#                   VSP_AGGLOM  CheckForDegenerateNodes_                       #
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::CheckForDegenerateNodes_(void)
{
 
    int i, Loop, *LastLoopTouched, *NodeDegree;
    
    LastLoopTouched = new int[FineGrid().NumberOfNodes() + 1];
    
    NodeDegree = new int[FineGrid().NumberOfNodes() + 1];

    zero_int_array(LastLoopTouched, FineGrid().NumberOfNodes());
    
    zero_int_array(NodeDegree, FineGrid().NumberOfNodes());
    
    for ( i = 1 ; i <= FineGrid().NumberOfNodes() ; i++ ) {
       
       NodeDegree[i] = -1;
       
    }
         
    // Loop over all tris and merge ones with poor aspect ratios with other loops
    
    for ( Loop = 1 ; Loop <= FineGrid().NumberOfLoops() ; Loop++ ) {

       for ( i = 1 ; i <= FineGrid().LoopList(Loop).NumberOfNodes() ; i++ ) {
      
          if ( LastLoopTouched[FineGrid().LoopList(Loop).Node(i)] != VortexLoopWasAgglomerated_[Loop] ) {

             if ( NodeDegree[FineGrid().LoopList(Loop).Node(i)] == -1 ) {
                
                NodeDegree[FineGrid().LoopList(Loop).Node(i)] = 0;
                
             }
             
             else if ( NodeDegree[FineGrid().LoopList(Loop).Node(i)] ==  0 ) {
                
                NodeDegree[FineGrid().LoopList(Loop).Node(i)] = 1;
                
             }
             
             LastLoopTouched[FineGrid().LoopList(Loop).Node(i)] = VortexLoopWasAgglomerated_[Loop];

          }

       }

    }
    
    // Check for nodes surrounded by a single loop

    for ( Loop = 1 ; Loop <= FineGrid().NumberOfLoops() ; Loop++ ) {

       for ( i = 1 ; i <= FineGrid().LoopList(Loop).NumberOfNodes() ; i++ ) {
          
          if ( NodeDegree[FineGrid().LoopList(Loop).Node(i)] == 0 ) {

             if ( VortexLoopWasAgglomerated_[Loop] < 0 ) VortexLoopWasAgglomerated_[Loop] = Loop;
             
          }
          
       }
       
    }
    
    delete [] LastLoopTouched;
    delete [] NodeDegree;
    
    fflush(NULL);
                      
}

/*##############################################################################
#                                                                              #
#                  VSP_AGGLOM FindNeighborLoopOnLocalEdge_                     #
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::FindNeighborLoopOnLocalEdge_(VSP_GRID &ThisGrid, int Loop, int LocalEdge, 
                                              int &NeighborLoop, int &Node1, int &Node2)
{

    int Edge;
    
    Edge = ThisGrid.LoopList(Loop).Edge(LocalEdge);
    
    Node1 = ThisGrid.EdgeList(Edge).Node1();
    Node2 = ThisGrid.EdgeList(Edge).Node2();
    
    NeighborLoop = ThisGrid.EdgeList(Edge).Loop1()
                 + ThisGrid.EdgeList(Edge).Loop2() - Loop;
         
}

/*##############################################################################
#                                                                              #
#                    VSP_AGGLOM FindNeighborLoopOnEdge_                        #
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::FindNeighborLoopOnEdge_(VSP_GRID &ThisGrid, int Loop, int Edge, 
                                         int &NeighborLoop, int &Node1, int &Node2)
{

    Node1 = ThisGrid.EdgeList(Edge).Node1();
    Node2 = ThisGrid.EdgeList(Edge).Node2();
    
    NeighborLoop = ThisGrid.EdgeList(Edge).Loop1()
                 + ThisGrid.EdgeList(Edge).Loop2() - Loop;
         
}

/*##############################################################################
#                                                                              #
#                        VSP_AGGLOM CalculateQuadQuality_                      #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE VSP_AGGLOM::CalculateQuadQuality_(VSP_GRID &ThisGrid, int Node1,
                                         int Node2, int Node3, int Node4)
{

    VSPAERO_DOUBLE Vec1[3], Vec2[3], Vec3[3], Vec4[3], Vec5[3], Vec6[3], Mag, Angle[4];
    VSPAERO_DOUBLE MaxAngle;
    
    // Vec 1
    
    Vec1[0] = ThisGrid.NodeList(Node1).x() - ThisGrid.NodeList(Node2).x();
    Vec1[1] = ThisGrid.NodeList(Node1).y() - ThisGrid.NodeList(Node2).y();
    Vec1[2] = ThisGrid.NodeList(Node1).z() - ThisGrid.NodeList(Node2).z();
 
    Mag = sqrt(vector_dot(Vec1,Vec1));
    
    Vec1[0] /= Mag;
    Vec1[1] /= Mag;
    Vec1[2] /= Mag;
    
    // Vec 2
    
    Vec2[0] = ThisGrid.NodeList(Node3).x() - ThisGrid.NodeList(Node2).x();
    Vec2[1] = ThisGrid.NodeList(Node3).y() - ThisGrid.NodeList(Node2).y();
    Vec2[2] = ThisGrid.NodeList(Node3).z() - ThisGrid.NodeList(Node2).z();
 
    Mag = sqrt(vector_dot(Vec2,Vec2));
    
    Vec2[0] /= Mag;
    Vec2[1] /= Mag;
    Vec2[2] /= Mag;
     
    // Vec 3
    
    Vec3[0] = ThisGrid.NodeList(Node3).x() - ThisGrid.NodeList(Node4).x();
    Vec3[1] = ThisGrid.NodeList(Node3).y() - ThisGrid.NodeList(Node4).y();
    Vec3[2] = ThisGrid.NodeList(Node3).z() - ThisGrid.NodeList(Node4).z();
 
    Mag = sqrt(vector_dot(Vec3,Vec3));
    
    Vec3[0] /= Mag;
    Vec3[1] /= Mag;
    Vec3[2] /= Mag;
    
    // Vec 4
    
    Vec4[0] = ThisGrid.NodeList(Node1).x() - ThisGrid.NodeList(Node4).x();
    Vec4[1] = ThisGrid.NodeList(Node1).y() - ThisGrid.NodeList(Node4).y();
    Vec4[2] = ThisGrid.NodeList(Node1).z() - ThisGrid.NodeList(Node4).z();
 
    Mag = sqrt(vector_dot(Vec4,Vec4));
    
    Vec4[0] /= Mag;
    Vec4[1] /= Mag;
    Vec4[2] /= Mag;
        
    // Vec 5
    
    Vec5[0] = ThisGrid.NodeList(Node4).x() - ThisGrid.NodeList(Node2).x();
    Vec5[1] = ThisGrid.NodeList(Node4).y() - ThisGrid.NodeList(Node2).y();
    Vec5[2] = ThisGrid.NodeList(Node4).z() - ThisGrid.NodeList(Node2).z();
 
    Mag = sqrt(vector_dot(Vec5,Vec5));
    
    Vec5[0] /= Mag;
    Vec5[1] /= Mag;
    Vec5[2] /= Mag;
        
    // Vec 6
    
    Vec6[0] = ThisGrid.NodeList(Node2).x() - ThisGrid.NodeList(Node4).x();
    Vec6[1] = ThisGrid.NodeList(Node2).y() - ThisGrid.NodeList(Node4).y();
    Vec6[2] = ThisGrid.NodeList(Node2).z() - ThisGrid.NodeList(Node4).z();
 
    Mag = sqrt(vector_dot(Vec6,Vec6));
    
    Vec6[0] /= Mag;
    Vec6[1] /= Mag;
    Vec6[2] /= Mag;
                
    // Check all four angles
   
    Angle[0] = vector_dot(Vec1,Vec5); Angle[0] = MIN(1.,MAX(-1.,Angle[0])); Angle[0] = acos(Angle[0]);

    Angle[1] = vector_dot(Vec2,Vec5); Angle[1] = MIN(1.,MAX(-1.,Angle[1])); Angle[1] = acos(Angle[1]); 

    Angle[2] = vector_dot(Vec3,Vec6); Angle[2] = MIN(1.,MAX(-1.,Angle[2])); Angle[2] = acos(Angle[2]); 
    
    Angle[3] = vector_dot(Vec4,Vec6); Angle[3] = MIN(1.,MAX(-1.,Angle[3])); Angle[3] = acos(Angle[3]); 
    
    // Check how parallel opposite sides are
    
    Mag = MAX(0.01,ABS(2. + vector_dot(Vec1,Vec3) + vector_dot(Vec2,Vec4)));

    MaxAngle = MAX(Angle[0]+Angle[1],Angle[2]+Angle[3]);
    
    if ( MaxAngle <= WorstQuadAngle_ && Mag <= 0.01 ) return Mag*MaxAngle;
    
    return MaxAngle;
    
}

/*##############################################################################
#                                                                              #
#                              VSP_AGGLOM BadTriangle_                         #
#                                                                              #
##############################################################################*/

int VSP_AGGLOM::BadTriangle_(VSP_GRID &ThisGrid, int Loop, int &MinEdge)
{
   
    int i, Edge, Node1, Node2;
    VSPAERO_DOUBLE Vec[3], Distance, MinDist, MaxDist, AspectRatio;
    
    MinEdge = 0;
    
    MinDist = 1.e9;
    MaxDist = -MinDist;
    
    if ( ThisGrid.LoopList(Loop).NumberOfEdges() > 3 ) return 0.;
    
    for ( i = 1 ; i <= ThisGrid.LoopList(Loop).NumberOfEdges() ; i++ ) {
       
       Edge = ThisGrid.LoopList(Loop).Edge(i);
       
       Node1 = ThisGrid.EdgeList(Edge).Node1();
       Node2 = ThisGrid.EdgeList(Edge).Node2();
       
       Vec[0] = ThisGrid.NodeList(Node2).x() - ThisGrid.NodeList(Node1).x(); 
       Vec[1] = ThisGrid.NodeList(Node2).y() - ThisGrid.NodeList(Node1).y(); 
       Vec[2] = ThisGrid.NodeList(Node2).z() - ThisGrid.NodeList(Node1).z(); 
   
       Distance = sqrt(vector_dot(Vec,Vec));
       
       if ( Distance < MinDist ) {
          
          MinDist = Distance;
          
          MinEdge = i;
          
       }
              
       MaxDist = MAX(MaxDist,Distance);
       
    }
    
    AspectRatio = MaxDist/MinDist;
        
    return 0;
    
} 

/*##############################################################################
#                                                                              #
#                                 VSP_AGGLOM BadQuad_                          #
#                                                                              #
##############################################################################*/

int VSP_AGGLOM::BadQuad_(VSP_GRID &ThisGrid, int Loop, int LoopList[2])
{
   
    int i, Edge, Edge1, Edge2, Node1, Node2, MinEdge, OppEdge, Done, Hits;
    VSPAERO_DOUBLE Vec[3], Length[5], MinLength;
   return 0;
    if ( ThisGrid.LoopList(Loop).NumberOfEdges() != 4 ) return 0.;
    
    for ( i = 1 ; i <= ThisGrid.LoopList(Loop).NumberOfEdges() ; i++ ) {
       
       Edge = ThisGrid.LoopList(Loop).Edge(i);
       
       Node1 = ThisGrid.EdgeList(Edge).Node1();
       Node2 = ThisGrid.EdgeList(Edge).Node2();
      
       Vec[0] = ThisGrid.NodeList(Node2).x() - ThisGrid.NodeList(Node1).x(); 
       Vec[1] = ThisGrid.NodeList(Node2).y() - ThisGrid.NodeList(Node1).y(); 
       Vec[2] = ThisGrid.NodeList(Node2).z() - ThisGrid.NodeList(Node1).z(); 
   
       Length[i] = sqrt(vector_dot(Vec,Vec));
 
    }
    
    // Find shortest side
    
    MinLength = 1.e9;
    
    for ( i = 1 ; i <= 4 ; i++ ) {
       
       if ( Length[i] <= MinLength ) {
          
          MinLength = Length[i];
          
          MinEdge = i;
          
       }
       
    }
    
    // Find the edge opposite
    
    i = 1;
    
    Done = 0;

    OppEdge = 0;
    
    while ( i <= 4 && !Done ) {

       if ( i != MinEdge ) {
          
          Edge1 = ThisGrid.LoopList(Loop).Edge(MinEdge);
          Edge2 = ThisGrid.LoopList(Loop).Edge(      i);
               
          if ( ( MIN(ThisGrid.EdgeList(Edge1).Node1(), ThisGrid.EdgeList(Edge1).Node2()) != 
                 MIN(ThisGrid.EdgeList(Edge2).Node1(), ThisGrid.EdgeList(Edge2).Node2()) ) &&
               ( MAX(ThisGrid.EdgeList(Edge1).Node1(), ThisGrid.EdgeList(Edge1).Node2()) !=                     
                 MAX(ThisGrid.EdgeList(Edge2).Node1(), ThisGrid.EdgeList(Edge2).Node2()) ) ) {
                    
             Done = 1;
             
             OppEdge = i;
             
          }
                 
       }
       
       i++;
       
    }

    LoopList[0] = LoopList[1] = 0;
    
    Hits = 0;
    
    for ( i = 1 ; i <= 4 ; i++ ) {
       
       if ( i != MinEdge && i != OppEdge ) {
          
          Edge = ThisGrid.LoopList(Loop).Edge(i);
          
          LoopList[Hits++] = ThisGrid.EdgeList(Edge).Loop1() + ThisGrid.EdgeList(Edge).Loop2() - Loop;
          
       }
              
    }

    if ( Hits != 2 ) {
       
       PRINTF("wtf! \n");fflush(NULL);
       exit(1);
       
    }
        
    return 1;
    
} 
 
/*##############################################################################
#                                                                              #
#                           VSP_AGGLOM CalculateAspectRatio_                   #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE VSP_AGGLOM::CalculateAspectRatioOld_(VSP_GRID &ThisGrid, int Loop)
{
   
    int i, Edge, Node1, Node2;
    VSPAERO_DOUBLE Vec[3], Distance[4], S, AspectRatio;
    
    for ( i = 1 ; i <= 3 ; i++ ) {
       
       Edge = ThisGrid.LoopList(Loop).Edge(i);
       
       Node1 = ThisGrid.EdgeList(Edge).Node1();
       Node2 = ThisGrid.EdgeList(Edge).Node2();
       
       Vec[0] = ThisGrid.NodeList(Node2).x() - ThisGrid.NodeList(Node1).x(); 
       Vec[1] = ThisGrid.NodeList(Node2).y() - ThisGrid.NodeList(Node1).y(); 
       Vec[2] = ThisGrid.NodeList(Node2).z() - ThisGrid.NodeList(Node1).z(); 
   
       Distance[i] = sqrt(vector_dot(Vec,Vec));
       
    }
    
    S = 0.5*( Distance[1] + Distance[2] + Distance[3] );
    
    AspectRatio = Distance[1] * Distance[2] * Distance[3] / (8. * (S - Distance[1] ) * (S - Distance[2] ) * (S - Distance[3] ) );
    
    return AspectRatio;
    
} 

/*##############################################################################
#                                                                              #
#                           VSP_AGGLOM LoopsAreCoplanar_                       #
#                                                                              #
##############################################################################*/

int VSP_AGGLOM::LoopsAreCoplanar_(VSP_GRID &ThisGrid, int Loop1, int Loop2, VSPAERO_DOUBLE MaxAngle)
{
   
    VSPAERO_DOUBLE Dot, Angle;
    
    Dot = vector_dot(ThisGrid.LoopList(Loop1).Normal(), ThisGrid.LoopList(Loop2).Normal());
    
    if ( Dot <= 0. ) return 0;
    
    Dot = MIN(Dot,1.);
    
    Angle = acos(Dot)*180./PI;
    
    if ( ABS(Angle) <= MaxAngle ) return 1;
    
    return 0;
    
} 

/*##############################################################################
#                                                                              #
#                           VSP_AGGLOM EdgesAreColinear_                       #
#                                                                              #
##############################################################################*/

int VSP_AGGLOM::EdgesAreColinear_(VSP_GRID &ThisGrid, int Edge1, int Edge2, VSPAERO_DOUBLE MaxAngle)
{
   
    int Node0, Node1, Node2, NodeA, NodeB;
    VSPAERO_DOUBLE Vec1[3], Vec2[3], Dot, Angle;
    
    Node1 = ThisGrid.EdgeList(Edge1).Node1();
    Node2 = ThisGrid.EdgeList(Edge1).Node2();
    
    NodeA = ThisGrid.EdgeList(Edge2).Node1();
    NodeB = ThisGrid.EdgeList(Edge2).Node2();
    
    if ( Node1 == NodeA ) {
       
       Node0 = Node1;
       Node1 = Node2;
       Node2 = NodeB;
       
    }

    else if ( Node1 == NodeB ) {
       
       Node0 = Node1;
       Node1 = Node2;
       Node2 = NodeA;
       
    }
    
    else if ( Node2 == NodeA ) {
       
       Node0 = Node2;
       Node1 = Node1;
       Node2 = NodeB;
       
    }
            
    else if ( Node2 == NodeB ) {
       
       Node0 = Node2;
       Node1 = Node1;
       Node2 = NodeA;
       
    }
    
    else {
       
       PRINTF("Could not find common node for the given 2 edges! \n");fflush(NULL);
       
       exit(1);
       
    }
   
    Vec1[0] = ThisGrid.NodeList(Node1).x() - ThisGrid.NodeList(Node0).x();
    Vec1[1] = ThisGrid.NodeList(Node1).y() - ThisGrid.NodeList(Node0).y();
    Vec1[2] = ThisGrid.NodeList(Node1).z() - ThisGrid.NodeList(Node0).z();
    
    Dot = sqrt(vector_dot(Vec1,Vec1));
    
    Vec1[0] /= Dot;
    Vec1[1] /= Dot;
    Vec1[2] /= Dot;
        
    Vec2[0] = ThisGrid.NodeList(Node2).x() - ThisGrid.NodeList(Node0).x();
    Vec2[1] = ThisGrid.NodeList(Node2).y() - ThisGrid.NodeList(Node0).y();
    Vec2[2] = ThisGrid.NodeList(Node2).z() - ThisGrid.NodeList(Node0).z();

    Dot = sqrt(vector_dot(Vec2,Vec2));
    
    Vec2[0] /= Dot;
    Vec2[1] /= Dot;
    Vec2[2] /= Dot;
            
    Dot = vector_dot(Vec1,Vec2);
    
    if ( Dot >= 0. ) return 0;
    
    Dot = MAX(Dot,-1.);
    
    Angle = 180. - acos(Dot)*180./PI;
 
    if ( Angle <= MaxAngle ) return 1;
    
    return 0;
    
} 

/*##############################################################################
#                                                                              #
#                       VSP_AGGLOM MergedLoopsAreConvex_                       #
#                                                                              #
##############################################################################*/

int VSP_AGGLOM::MergedLoopsAreConvex_(VSP_GRID &ThisGrid, int Loop1, int Loop2, VSPAERO_DOUBLE MaxAngle)
{
   
    int i, Edge, NumberOfCommonEdges, Node1, Node2, NodeA, NodeB;
    int NumBoundaryNodes, BoundaryNode[2], BoundaryEdgeList[4];
    int LoopA, LoopB, LoopC, LoopD;
    int *TempEdgeList_, *TempNodeList_;
    VSPAERO_DOUBLE Vec1[3], Vec2[3], Dot, Angle;

    LoopA = MIN(Loop1,Loop2);
    LoopB = MAX(Loop1,Loop2);
    
    TempEdgeList_ = new int[ThisGrid.NumberOfEdges() + 1];
    
    TempNodeList_ = new int[ThisGrid.NumberOfNodes() + 1];
    
    zero_int_array(TempNodeList_, ThisGrid.NumberOfNodes());
    
    NumberOfCommonEdges = 0;
    
    // Loop over the edges and find those common to both loops
    
    for ( i = 1 ; i <= ThisGrid.LoopList(Loop1).NumberOfEdges() ; i++ ) {
       
       Edge = ThisGrid.LoopList(Loop1).Edge(i);
       
       LoopC = MIN(ThisGrid.EdgeList(Edge).Loop1(),ThisGrid.EdgeList(Edge).Loop2());
       LoopD = MAX(ThisGrid.EdgeList(Edge).Loop1(),ThisGrid.EdgeList(Edge).Loop2());
       
       if ( LoopA == LoopC && LoopB == LoopD ) {
       
          TempEdgeList_[++NumberOfCommonEdges] = Edge;   
               
       }
            
    }
    
    // Now find the two common nodes
    
    for ( i = 1 ; i <= NumberOfCommonEdges ; i++ ) {
       
       Edge = TempEdgeList_[i];
       
       Node1 = ThisGrid.EdgeList(Edge).Node1();
       
       Node2 = ThisGrid.EdgeList(Edge).Node2();
       
       TempNodeList_[Node1]++;
       
       TempNodeList_[Node2]++;
       
    }
    
    NumBoundaryNodes = 0;
    
    for ( i = 1 ; i <= NumberOfCommonEdges ; i++ ) {
       
       Edge = TempEdgeList_[i];
       
       Node1 = ThisGrid.EdgeList(Edge).Node1();
       
       Node2 = ThisGrid.EdgeList(Edge).Node2();
       
       if ( TempNodeList_[Node1] == 1 ) BoundaryNode[NumBoundaryNodes++] = Node1;
       
       if ( TempNodeList_[Node2] == 1 ) BoundaryNode[NumBoundaryNodes++] = Node2;
       
       TempNodeList_[Node2]++;
       
    }    
       
    PRINTF("Number of boundary nodes: %d \n",NumBoundaryNodes);
    
    
delete [] TempEdgeList_;

    
    // Now find boundary edges incidient on the two boundary nodes
    
    NodeA = MIN(TempNodeList_[0],TempNodeList_[1]);
    NodeB = MAX(TempNodeList_[0],TempNodeList_[1]);
    
    // Loop 1
    
    BoundaryEdgeList[0] = BoundaryEdgeList[1] = BoundaryEdgeList[2] = BoundaryEdgeList[3] = 0;
    
    for ( i = 1 ; i <= ThisGrid.LoopList(Loop1).NumberOfEdges() ; i++ ) {
       
       Edge = ThisGrid.LoopList(Loop1).Edge(i);
      
       LoopC = MIN(ThisGrid.EdgeList(Edge).Loop1(),ThisGrid.EdgeList(Edge).Loop2());
       LoopD = MAX(ThisGrid.EdgeList(Edge).Loop1(),ThisGrid.EdgeList(Edge).Loop2());
       
       if ( LoopA != LoopC || LoopB != LoopD ) {
       
          if ( ThisGrid.EdgeList(Edge).Node1() == NodeA ) BoundaryEdgeList[0] =  Edge;
          if ( ThisGrid.EdgeList(Edge).Node2() == NodeA ) BoundaryEdgeList[0] = -Edge;

          if ( ThisGrid.EdgeList(Edge).Node1() == NodeB ) BoundaryEdgeList[2] =  Edge;
          if ( ThisGrid.EdgeList(Edge).Node2() == NodeB ) BoundaryEdgeList[2] = -Edge;    
                     
       }
            
    }    
    
    // Loop 2
    
    for ( i = 1 ; i <= ThisGrid.LoopList(Loop2).NumberOfEdges() ; i++ ) {
       
       Edge = ThisGrid.LoopList(Loop2).Edge(i);
      
       LoopC = MIN(ThisGrid.EdgeList(Edge).Loop1(),ThisGrid.EdgeList(Edge).Loop2());
       LoopD = MAX(ThisGrid.EdgeList(Edge).Loop1(),ThisGrid.EdgeList(Edge).Loop2());
       
       if ( LoopA != LoopC || LoopB != LoopD ) {
       
          if ( ThisGrid.EdgeList(Edge).Node1() == NodeA ) BoundaryEdgeList[1] =  Edge;
          if ( ThisGrid.EdgeList(Edge).Node2() == NodeA ) BoundaryEdgeList[1] = -Edge;

          if ( ThisGrid.EdgeList(Edge).Node1() == NodeB ) BoundaryEdgeList[3] =  Edge;
          if ( ThisGrid.EdgeList(Edge).Node2() == NodeB ) BoundaryEdgeList[3] = -Edge;    
                     
       }
            
    }
    
    // Calculate angle between boundary edges 1 and 2
    
    if ( BoundaryEdgeList[0] > 0 ) {
       
       Node1 = ThisGrid.EdgeList( BoundaryEdgeList[0]).Node1();
       Node2 = ThisGrid.EdgeList( BoundaryEdgeList[0]).Node2();
       
    }
    
    else {
       
       Node2 = ThisGrid.EdgeList(-BoundaryEdgeList[0]).Node1();
       Node1 = ThisGrid.EdgeList(-BoundaryEdgeList[0]).Node2();
      
    } 
    
    Vec1[0] = ThisGrid.NodeList(Node2).x() - ThisGrid.NodeList(Node1).x();
    Vec1[1] = ThisGrid.NodeList(Node2).y() - ThisGrid.NodeList(Node1).y();
    Vec1[2] = ThisGrid.NodeList(Node2).z() - ThisGrid.NodeList(Node1).z();
    
    Dot = sqrt(vector_dot(Vec1,Vec1));
    
    Vec1[0] /= Dot;
    Vec1[1] /= Dot;
    Vec1[2] /= Dot;
    
    if ( BoundaryEdgeList[1] > 0 ) {
       
       Node1 = ThisGrid.EdgeList( BoundaryEdgeList[1]).Node1();
       Node2 = ThisGrid.EdgeList( BoundaryEdgeList[1]).Node2();
       
    }
    
    else {
       
       Node2 = ThisGrid.EdgeList(-BoundaryEdgeList[1]).Node1();
       Node1 = ThisGrid.EdgeList(-BoundaryEdgeList[1]).Node2();
      
    }     

    Vec2[0] = ThisGrid.NodeList(Node2).x() - ThisGrid.NodeList(Node1).x();
    Vec2[1] = ThisGrid.NodeList(Node2).y() - ThisGrid.NodeList(Node1).y();
    Vec2[2] = ThisGrid.NodeList(Node2).z() - ThisGrid.NodeList(Node1).z();

    Dot = sqrt(vector_dot(Vec2,Vec2));
    
    Vec2[0] /= Dot;
    Vec2[1] /= Dot;
    Vec2[2] /= Dot;
 
    Dot = vector_dot(Vec1,Vec2);
    
    Dot = MIN(MAX(Dot,-1.),1.);
    
    Angle = acos(Dot)*180./PI;
         
    if ( ABS(Angle) > 180. ) return 0;
    
    // Calculate angle between boundary edges 3 and 4
    
    if ( BoundaryEdgeList[0] > 0 ) {
       
       Node1 = ThisGrid.EdgeList( BoundaryEdgeList[2]).Node1();
       Node2 = ThisGrid.EdgeList( BoundaryEdgeList[2]).Node2();
       
    }
    
    else {
       
       Node2 = ThisGrid.EdgeList(-BoundaryEdgeList[2]).Node1();
       Node1 = ThisGrid.EdgeList(-BoundaryEdgeList[2]).Node2();
      
    } 
    
    Vec1[0] = ThisGrid.NodeList(Node2).x() - ThisGrid.NodeList(Node1).x();
    Vec1[1] = ThisGrid.NodeList(Node2).y() - ThisGrid.NodeList(Node1).y();
    Vec1[2] = ThisGrid.NodeList(Node2).z() - ThisGrid.NodeList(Node1).z();
    
    Dot = sqrt(vector_dot(Vec1,Vec1));
    
    Vec1[0] /= Dot;
    Vec1[1] /= Dot;
    Vec1[2] /= Dot;
    
    if ( BoundaryEdgeList[1] > 0 ) {
       
       Node1 = ThisGrid.EdgeList( BoundaryEdgeList[3]).Node1();
       Node2 = ThisGrid.EdgeList( BoundaryEdgeList[3]).Node2();
       
    }
    
    else {
       
       Node2 = ThisGrid.EdgeList(-BoundaryEdgeList[3]).Node1();
       Node1 = ThisGrid.EdgeList(-BoundaryEdgeList[3]).Node2();
      
    }     

    Vec2[0] = ThisGrid.NodeList(Node2).x() - ThisGrid.NodeList(Node1).x();
    Vec2[1] = ThisGrid.NodeList(Node2).y() - ThisGrid.NodeList(Node1).y();
    Vec2[2] = ThisGrid.NodeList(Node2).z() - ThisGrid.NodeList(Node1).z();

    Dot = sqrt(vector_dot(Vec2,Vec2));
    
    Vec2[0] /= Dot;
    Vec2[1] /= Dot;
    Vec2[2] /= Dot;
 
    Dot = vector_dot(Vec1,Vec2);
    
    Dot = MIN(MAX(Dot,-1.),1.);
    
    Angle = acos(Dot)*180./PI;
         
    if ( ABS(Angle) > 180. ) return 0;    
    
    return 1;
    
} 

/*##############################################################################
#                                                                              #
#                           VSP_AGGLOM CalculateLoopAngle_                     #
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE VSP_AGGLOM::CalculateLoopAngle_(VSP_GRID &ThisGrid, int Loop, int Node)
{
   
    int Node1, Node2, Node3;
    VSPAERO_DOUBLE Vec1[3], Vec2[3], Mag1, Mag2, Dot, Angle;

    Node1 = ThisGrid.LoopList(Loop).Node1();
    Node2 = ThisGrid.LoopList(Loop).Node2();
    Node3 = ThisGrid.LoopList(Loop).Node3();
    
    if ( Node == Node1 ) {
       
       Vec1[0] = ThisGrid.NodeList(Node2).x() - ThisGrid.NodeList(Node).x();
       Vec1[1] = ThisGrid.NodeList(Node2).y() - ThisGrid.NodeList(Node).y();
       Vec1[2] = ThisGrid.NodeList(Node2).z() - ThisGrid.NodeList(Node).z();
    
       Vec2[0] = ThisGrid.NodeList(Node3).x() - ThisGrid.NodeList(Node).x();
       Vec2[1] = ThisGrid.NodeList(Node3).y() - ThisGrid.NodeList(Node).y();
       Vec2[2] = ThisGrid.NodeList(Node3).z() - ThisGrid.NodeList(Node).z();
    
    }
       
    else if ( Node == Node2 ) {
       
       Vec1[0] = ThisGrid.NodeList(Node1).x() - ThisGrid.NodeList(Node).x();
       Vec1[1] = ThisGrid.NodeList(Node1).y() - ThisGrid.NodeList(Node).y();
       Vec1[2] = ThisGrid.NodeList(Node1).z() - ThisGrid.NodeList(Node).z();
    
       Vec2[0] = ThisGrid.NodeList(Node3).x() - ThisGrid.NodeList(Node).x();
       Vec2[1] = ThisGrid.NodeList(Node3).y() - ThisGrid.NodeList(Node).y();
       Vec2[2] = ThisGrid.NodeList(Node3).z() - ThisGrid.NodeList(Node).z();
    
    }
    
    else if ( Node == Node3 ) {
       
       Vec1[0] = ThisGrid.NodeList(Node1).x() - ThisGrid.NodeList(Node).x();
       Vec1[1] = ThisGrid.NodeList(Node1).y() - ThisGrid.NodeList(Node).y();
       Vec1[2] = ThisGrid.NodeList(Node1).z() - ThisGrid.NodeList(Node).z();
    
       Vec2[0] = ThisGrid.NodeList(Node2).x() - ThisGrid.NodeList(Node).x();
       Vec2[1] = ThisGrid.NodeList(Node2).y() - ThisGrid.NodeList(Node).y();
       Vec2[2] = ThisGrid.NodeList(Node2).z() - ThisGrid.NodeList(Node).z();
    
    }    
    
    else {
       
       PRINTF("wtf... no matching node! \n");fflush(NULL);
       exit(1);
       
    }
    
    Mag1 = sqrt(vector_dot(Vec1,Vec1));
    
    Mag2 = sqrt(vector_dot(Vec2,Vec2));
    
    Dot = vector_dot(Vec1,Vec2)/(Mag1*Mag2);
    
    Dot = MIN(MAX(Dot,-1.),1.);
    
    Angle = acos(Dot)*180./PI;
    
    return Angle;
           
} 

















 
