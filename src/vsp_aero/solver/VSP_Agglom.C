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
 
    // Agglomeration data
    
    NumberOfEdgesOnTE_ = 0;
    NumberOfEdgesOnLE_ = 0;
    NumberOfEdgesOnBoundary_ = 0;
    
    EdgeIsOnFront_ = NULL;
    CoarseEdgeList_ = NULL;  
    CoarseNodeList_= NULL;
    
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

    if ( EdgeIsOnFront_  != NULL ) delete [] EdgeIsOnFront_;
    if ( CoarseEdgeList_ != NULL ) delete [] CoarseEdgeList_;
    if ( CoarseNodeList_ != NULL ) delete [] CoarseNodeList_;

}

/*##############################################################################
#                                                                              #
#                                VSP_AGGLOM Copy                               #              
#                                                                              #
##############################################################################*/

VSP_AGGLOM::VSP_AGGLOM(const VSP_AGGLOM &agglom)
{

    // Not implemented!
    
    printf("Copying of agglom objects not implemented! \n");
    
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

    // Initialize the front
    
    InitializeFront_();
    
    NextBestEdgeOnFront_ = NextAgglomerationEdge_();
    
    // Merge vortex loops

    while ( NextBestEdgeOnFront_ > 0 ) {
     
       MergeVortexLoops_();
       
       NextBestEdgeOnFront_ = NextAgglomerationEdge_();
              
    }
    
    // Look for lone loops
    
  //  CleanUpLoneLoops_();

    // Create the course mesh data
    
    CreateCoarseMesh_();
    
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

    int i, Node1, Node2;
    
    // Allocate space for the front list. This will contain the currently unused
    // edges on the agglomeration front.
 
    EdgeIsOnFront_ = new int[FineGrid().NumberOfEdges() + 1];
    
    zero_int_array(EdgeIsOnFront_, FineGrid().NumberOfEdges());
    
    // Allocate some scratch space
    
    LoopHits_ = new int[FineGrid().NumberOfEdges() + 1];

    DidThisLoop_ = new int[FineGrid().NumberOfEdges() + 1];
    
    LoopListStack_ = new int[FineGrid().NumberOfLoops() + 1];
    
    zero_int_array(LoopHits_, FineGrid().NumberOfEdges());
 
    zero_int_array(DidThisLoop_, FineGrid().NumberOfEdges());
    
    zero_int_array(LoopListStack_, FineGrid().NumberOfLoops());
    
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

    NumberOfEdgesOnTE_ = 0;
    NumberOfEdgesOnLE_ = 0;
    NumberOfEdgesOnBoundary_ = 0;
     
    for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
     
       Node1 = FineGrid().EdgeList(i).Node1();
       Node2 = FineGrid().EdgeList(i).Node2();
    
       // Check if either node lies on a TE
       
       if ( FineGrid().EdgeList(i).IsTrailingEdge()  ) {
       
          if ( EdgeIsOnFront_[i] == 0 ) {
        
             EdgeIsOnFront_[i] = TE_EDGE_BC;
       
             NumberOfEdgesOnTE_++;
             
          }
       
       }        
      
       // Check if either node lies on a LE
       
       if ( FineGrid().EdgeList(i).IsLeadingEdge() ) {

          if ( EdgeIsOnFront_[i] == 0 ) {
        
             EdgeIsOnFront_[i] = LE_EDGE_BC;
       
             NumberOfEdgesOnLE_++;
             
          }
       
       }    
       
       // Check if either node lies on a general edge boundary
       
       if ( FineGrid().EdgeList(i).IsBoundaryEdge() ) {
        
          if ( EdgeIsOnFront_[i] == 0 ) {
        
             EdgeIsOnFront_[i] = BOUNDARY_EDGE_BC;
       
             NumberOfEdgesOnBoundary_++;
             
          }
       
       }  

    }
        
}

/*##############################################################################
#                                                                              #
#                           VSP_AGGLOM NextAgglomerationEdge_                  #              
#                                                                              #
##############################################################################*/

int VSP_AGGLOM::NextAgglomerationEdge_(void)
{
 
    int i, Done;

    Done = 0;
 
    // TE edge
    
    if ( NumberOfEdgesOnTE_ > 0 ) {
     
       for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
   
          if ( EdgeIsOnFront_[i] == TE_EDGE_BC ) return i;
       
       }

    } 

    // LE edge
    
    if ( NumberOfEdgesOnLE_ > 0 ) {
     
       for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
   
          if ( EdgeIsOnFront_[i] == LE_EDGE_BC ) return i;
       
       }

    } 
    
    // Boundary edge
    
    if ( NumberOfEdgesOnBoundary_ > 0 ) {
     
       for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
   
          if ( EdgeIsOnFront_[i] == BOUNDARY_EDGE_BC ) return i;
       
       }

    } 
    
    // Interior node, return the shortest edge
/*
    dsMin = -1.e9;
    
    iBest = 0;
    
    for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {

       if ( EdgeIsOnFront_[i] == INTERIOR_EDGE_BC ) {

          if ( FineGrid().EdgeList(i).Length() > dsMin) {

             dsMin = FineGrid().EdgeList(i).Length();
             
             iBest = i;
             
          }
          
       }
    
    }       
    
    if ( iBest > 0 ) return iBest; */
    

  
    for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {

       if ( EdgeIsOnFront_[i] == INTERIOR_EDGE_BC ) return i;
    
    }    

  
    // If we got here then we are done agglomerating
    
    return 0;

}

/*##############################################################################
#                                                                              #
#                           VSP_AGGLOM MergeVortexLoops_                       #              
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::MergeVortexLoops_(void)
{

    int i, j, Side, Loop, Loop1, Loop2, Edge, MergedLoop, NewLoop;

    // Check each side of this edge
        
    StackSize_ = MergedLoop = 0;
    
    for ( Side = 1 ; Side <= 2 ; Side++ ) {
     
       if ( Side == 1 ) Loop1 = FineGrid().EdgeList(NextBestEdgeOnFront_).Loop1();
       if ( Side == 2 ) Loop1 = FineGrid().EdgeList(NextBestEdgeOnFront_).Loop2();
    
       if ( VortexLoopWasAgglomerated_[Loop1] > 0 ) {
     
          // Mark this loop as merged
          
          MergedLoop = Loop1;
        
          VortexLoopWasAgglomerated_[Loop1] *= -1;

          // Agglomerate loops that share an edge with this loop
          
          for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
           
             if ( i != NextBestEdgeOnFront_ ) {
              
                if ( FineGrid().EdgeList(i).Loop1() == Loop1 ||
                     FineGrid().EdgeList(i).Loop2() == Loop1 ) {
           
                   Loop2 = FineGrid().EdgeList(i).Loop1() + FineGrid().EdgeList(i).Loop2() - Loop1;
                   
                   if ( VortexLoopWasAgglomerated_[Loop2] > 0 ) {
                    
                      // Mark Loop2 as being merged with Loop 1
                      
                      VortexLoopWasAgglomerated_[Loop2] = -Loop1;

                      LoopListStack_[++StackSize_] = Loop2;
                      
                      // Now add edges of this loop to the front
                      
                      for ( j = 1 ; j <= FineGrid().LoopList(Loop2).NumberOfEdges() ; j++ ) {
                       
                         Edge = FineGrid().LoopList(Loop2).Edge(j);
                         
                         if ( EdgeIsOnFront_[Edge] == 0 ) EdgeIsOnFront_[Edge] = INTERIOR_EDGE_BC;
                         
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
             
             if ( LoopHits_[NewLoop] >=2 && VortexLoopWasAgglomerated_[NewLoop] > 0 ) {

                VortexLoopWasAgglomerated_[NewLoop] = -MergedLoop;
              
                // Now add edges of this loop to the front
                
                for ( j = 1 ; j <= FineGrid().LoopList(NewLoop).NumberOfEdges() ; j++ ) {
                 
                   Edge = FineGrid().LoopList(NewLoop).Edge(j);
                   
                   if ( EdgeIsOnFront_[Edge] == 0 ) EdgeIsOnFront_[Edge] = INTERIOR_EDGE_BC;
                   
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
#                           VSP_AGGLOM CleanUpLoneLoops_                       #              
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::CleanUpLoneLoops_(void)
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
    
       // Determine how many possible neighbor loops for each loop... 

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
                
             if ( IsInteriorLoop && 5.*FineGrid().LoopList(Loop1).Area() <= FineGrid().LoopList(Loop2).Area() ) {
          
                VortexLoopWasAgglomerated_[Loop1] = -Loop2;
                
                printf("Loop1: %d \n",Loop1);
                printf("Loop2: %d \n",Loop2);
                printf("VortexLoopWasAgglomerated_[Loop2]: %d \n",VortexLoopWasAgglomerated_[Loop2]);
                
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
    
    int i, j, k, Node, Node1, Node2, Loop, Loop1, Loop2, Next, NumberOfEdges, NumberOfLoops;
    int NumberOfCoarseGridNodes, NumberOfCoarseGridEdges, NumberOfCoarseGridLoops;
    int *KuttaNode, NumberOfKuttaNodes, *EdgeDirection;
    double Area;
    
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

          // Pointer to the fine grid from the coarase grid

          CoarseGrid().EdgeList(Next).FineGridEdge() = i;
          
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
          
          FineGrid().EdgeList(i).CourseGridEdge() = Next;
           
       }
       
       else {
          
          FineGrid().EdgeList(i).CourseGridEdge() = 0;
          
       }
       
    }   

    // Zero out coarse grid data that needs to be agglomerated
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {

       // Area
       
       CoarseGrid().LoopList(i).Area() = 0.;
       
       // Loopangle Normal
       
       CoarseGrid().LoopList(i).Nx() = 0.;
       CoarseGrid().LoopList(i).Ny() = 0.;
       CoarseGrid().LoopList(i).Nz() = 0.; 
       
       // Local Camber Line Normal
       
       CoarseGrid().LoopList(i).NxCamber() = 0.;
       CoarseGrid().LoopList(i).NyCamber() = 0.;
       CoarseGrid().LoopList(i).NzCamber() = 0.;
       
       // Centroid
       
       CoarseGrid().LoopList(i).Xc() = 0.;
       CoarseGrid().LoopList(i).Yc() = 0.;
       CoarseGrid().LoopList(i).Zc() = 0.;           
       
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
       CoarseGrid().LoopList(Next).SpanStation() = FineGrid().LoopList(i).SpanStation(); 
       CoarseGrid().LoopList(Next).SurfaceType() = FineGrid().LoopList(i).SurfaceType(); 
       CoarseGrid().LoopList(Next).BodyID()      = FineGrid().LoopList(i).BodyID(); 
       CoarseGrid().LoopList(Next).WingID()      = FineGrid().LoopList(i).WingID();        

       // Area
       
       Area = FineGrid().LoopList(i).Area();
       
       CoarseGrid().LoopList(Next).Area() += Area;

       // Loopangle Normal
       
       CoarseGrid().LoopList(Next).Nx() += Area*FineGrid().LoopList(i).Nx();
       CoarseGrid().LoopList(Next).Ny() += Area*FineGrid().LoopList(i).Ny();
       CoarseGrid().LoopList(Next).Nz() += Area*FineGrid().LoopList(i).Nz();
       
       // Local Camber Line Normal
       
       CoarseGrid().LoopList(Next).NxCamber() += Area*FineGrid().LoopList(i).NxCamber();
       CoarseGrid().LoopList(Next).NyCamber() += Area*FineGrid().LoopList(i).NyCamber();
       CoarseGrid().LoopList(Next).NzCamber() += Area*FineGrid().LoopList(i).NzCamber();
       
       // Centroid
       
       CoarseGrid().LoopList(Next).Xc() += Area*FineGrid().LoopList(i).Xc();
       CoarseGrid().LoopList(Next).Yc() += Area*FineGrid().LoopList(i).Yc();
       CoarseGrid().LoopList(Next).Zc() += Area*FineGrid().LoopList(i).Zc();      
       
       // Bounding box information
       
       CoarseGrid().LoopList(Next).BoundBox().x_min = MIN(CoarseGrid().LoopList(Next).BoundBox().x_min, FineGrid().LoopList(i).BoundBox().x_min );
       CoarseGrid().LoopList(Next).BoundBox().x_max = MAX(CoarseGrid().LoopList(Next).BoundBox().x_max, FineGrid().LoopList(i).BoundBox().x_max );

       CoarseGrid().LoopList(Next).BoundBox().y_min = MIN(CoarseGrid().LoopList(Next).BoundBox().y_min, FineGrid().LoopList(i).BoundBox().y_min );
       CoarseGrid().LoopList(Next).BoundBox().y_max = MAX(CoarseGrid().LoopList(Next).BoundBox().y_max, FineGrid().LoopList(i).BoundBox().y_max );

       CoarseGrid().LoopList(Next).BoundBox().z_min = MIN(CoarseGrid().LoopList(Next).BoundBox().z_min, FineGrid().LoopList(i).BoundBox().z_min );
       CoarseGrid().LoopList(Next).BoundBox().z_max = MAX(CoarseGrid().LoopList(Next).BoundBox().z_max, FineGrid().LoopList(i).BoundBox().z_max );
       
       // Pointer from fine grid to coarse
                
       FineGrid().LoopList(i).CoarseGridLoop() = Next;     
       
       // Zero out number of agglomerated edges for this loop... we determine that below
       
       CoarseGrid().LoopList(Next).NumberOfEdges() = 0;
       
    }  
   
    // Size the bounding box
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {

       box_calculate_size(CoarseGrid().LoopList(i).BoundBox());
     
    }    
     
    // Area average the results
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {

       // Area
            
       Area = CoarseGrid().LoopList(i).Area();

       // Loopangle Normal
       
       CoarseGrid().LoopList(i).Nx() /= Area;
       CoarseGrid().LoopList(i).Ny() /= Area;
       CoarseGrid().LoopList(i).Nz() /= Area;
      
       // Local Camber Line Normal
       
       CoarseGrid().LoopList(i).NxCamber() /= Area;
       CoarseGrid().LoopList(i).NyCamber() /= Area;
       CoarseGrid().LoopList(i).NzCamber() /= Area;
       
       // Centroid
       
       CoarseGrid().LoopList(i).Xc() /= Area;
       CoarseGrid().LoopList(i).Yc() /= Area;
       CoarseGrid().LoopList(i).Zc() /= Area; 
 
    }        
    
    // Create a list of all fine grid loops agglomerated for each coarse grid loop

    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {
 
       CoarseGrid().LoopList(i).NumberOfFineGridLoops() = 0;
               
    }  
    
    for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
  
       Loop = FineGrid().LoopList(i).CoarseGridLoop();
      
       CoarseGrid().LoopList(Loop).NumberOfFineGridLoops() += 1;
               
    }   
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {
  
       NumberOfLoops = CoarseGrid().LoopList(i).NumberOfFineGridLoops();

       CoarseGrid().LoopList(i).SizeFineGridLoopList(NumberOfLoops);
       
       CoarseGrid().LoopList(i).NumberOfFineGridLoops() = 0;
               
    } 

    for ( i = 1 ; i <= FineGrid().NumberOfLoops() ; i++ ) {
   
       Loop = FineGrid().LoopList(i).CoarseGridLoop();

       CoarseGrid().LoopList(Loop).NumberOfFineGridLoops() += 1;
    
       CoarseGrid().LoopList(Loop).FineGridLoop(CoarseGrid().LoopList(Loop).NumberOfFineGridLoops()) = i;
     
    }       

    // Create a list of edges for each loop
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfEdges() ; i++ ) {
     
       Loop1 = CoarseGrid().EdgeList(i).Loop1();
       Loop2 = CoarseGrid().EdgeList(i).Loop2();
       
       if ( Loop1 != 0 && Loop1 != Loop2 ) CoarseGrid().LoopList(Loop1).NumberOfEdges() += 1;
       if ( Loop2 != 0                   ) CoarseGrid().LoopList(Loop2).NumberOfEdges() += 1;
       
    }
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfLoops() ; i++ ) {

       NumberOfEdges = CoarseGrid().LoopList(i).NumberOfEdges();

       CoarseGrid().LoopList(i).SizeEdgeList(NumberOfEdges);
       
       CoarseGrid().LoopList(i).NumberOfEdges() = 0;
       
    }
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfEdges() ; i++ ) {
     
       Loop1 = CoarseGrid().EdgeList(i).Loop1();
       Loop2 = CoarseGrid().EdgeList(i).Loop2();
       
       if ( Loop1 != 0 && Loop1 != Loop2 ) {
        
          CoarseGrid().LoopList(Loop1).NumberOfEdges() += 1;
          
          CoarseGrid().LoopList(Loop1).Edge(CoarseGrid().LoopList(Loop1).NumberOfEdges()) = i;
          
       }
       
       if ( Loop2 != 0 ) {
        
          CoarseGrid().LoopList(Loop2).NumberOfEdges() += 1;
          
          CoarseGrid().LoopList(Loop2).Edge(CoarseGrid().LoopList(Loop2).NumberOfEdges()) = i;
          
       }       
       
    }  
    
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
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfNodes() ; i++ ) {
     
       if ( KuttaNode[i] != 0 ) NumberOfKuttaNodes++;
       
    }
   
    // Copy over any kutta node information
 
    CoarseGrid().SizeKuttaNodeList(NumberOfKuttaNodes);
            
    NumberOfKuttaNodes = 0;
    
    for ( i = 1 ; i <= FineGrid().NumberOfKuttaNodes() ; i++ ) {

       Node = FineGrid().NodeList(FineGrid().KuttaNode(i)).CoarseGridNode();
    
       if ( KuttaNode[Node] != 0 ) {
        
          NumberOfKuttaNodes++;
        
          CoarseGrid().KuttaNode(NumberOfKuttaNodes) = Node;
       
          CoarseGrid().WakeTrailingEdgeX(NumberOfKuttaNodes) = FineGrid().WakeTrailingEdgeX(i);
          CoarseGrid().WakeTrailingEdgeY(NumberOfKuttaNodes) = FineGrid().WakeTrailingEdgeY(i);
          CoarseGrid().WakeTrailingEdgeZ(NumberOfKuttaNodes) = FineGrid().WakeTrailingEdgeZ(i);
          
       }
       
    }

}

/*##############################################################################
#                                                                              #
#                      VSP_AGGLOM  SimplifyMesh_                               #
#                                                                              #
##############################################################################*/

VSP_GRID*  VSP_AGGLOM::SimplifyMesh_(VSP_GRID &Grid)
{
 
    // Copy pointer to the fine grid
    
    FineGrid_ = &Grid;
    
    // Initialize the front
    
    InitializeFront_();    
    
    // Merge as many tris into quads as possible
    
    CreateMixedMesh_();
 
    // Create the course mesh data
    
    CreateCoarseMesh_();
    
    // Return pointer to the coarse mesh
            
    return CoarseGrid_;
    
}

/*##############################################################################
#                                                                              #
#                      VSP_AGGLOM  CreateMixedMesh_                             #
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::CreateMixedMesh_(void)
{
 
    int Edge, Loop, NeighborLoop, Node1, Node2, Node3, Node4, NodeA, NodeB;
    int Merged, NumberOfLoopsMerged, Good;
     
    // Loop over all tris and merge any that can create decent quads

    NumberOfLoopsMerged = 0;

    Loop = 1;
    
    while ( Loop <= FineGrid().NumberOfLoops() ) {
     
       if ( VortexLoopWasAgglomerated_[Loop] > 0 ) {
        
          Node1 = FineGrid().LoopList(Loop).Node1();
          Node2 = FineGrid().LoopList(Loop).Node2();
          Node3 = FineGrid().LoopList(Loop).Node3();
       
          Edge = 1;
          
          Merged = 0;
          
          while ( Edge <= 3 && !Merged) {
           
             FindNeighborLoopOnEdge_(FineGrid(), Loop, Edge, NeighborLoop, NodeA, NodeB);

             if ( FineGrid().LoopList(NeighborLoop).SpanStation() == FineGrid().LoopList(Loop).SpanStation() ) {
                
                if ( VortexLoopWasAgglomerated_[NeighborLoop] > 0 ) {
                 
                   Node4 = FineGrid().LoopList(NeighborLoop).Node1()
                         + FineGrid().LoopList(NeighborLoop).Node2()
                         + FineGrid().LoopList(NeighborLoop).Node3() - NodeA - NodeB;
                         
                   if ( Edge == 1 ) Good = CalculateQuadQuality_(FineGrid(),Node1, Node4, Node2, Node3);
                   if ( Edge == 2 ) Good = CalculateQuadQuality_(FineGrid(),Node2, Node4, Node3, Node1);
                   if ( Edge == 3 ) Good = CalculateQuadQuality_(FineGrid(),Node3, Node4, Node1, Node2);
                   
                   if ( Good ) {
        
                      VortexLoopWasAgglomerated_[Loop] *= -1;
   
                      VortexLoopWasAgglomerated_[NeighborLoop] = -Loop;
                      
                      Merged = 1;
                      
                      NumberOfLoopsMerged++;
                      
                   }
                   
                }
                
             }
             
             Edge++;
             
          }
          
       }
       
       Loop++;
       
    }

}

/*##############################################################################
#                                                                              #
#                       VSP_AGGLOM FindNeighborLoopOnEdge_                      #
#                                                                              #
##############################################################################*/

void VSP_AGGLOM::FindNeighborLoopOnEdge_(VSP_GRID &ThisGrid, int Loop, int LocalEdge, 
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
#                        VSP_AGGLOM CalculateQuadQuality_                      #
#                                                                              #
##############################################################################*/

double VSP_AGGLOM::CalculateQuadQuality_(VSP_GRID &ThisGrid, int Node1,
                                         int Node2, int Node3, int Node4)
{

    double Vec1[3], Vec2[3], Vec3[3], Vec4[3], Mag, Angle;
    
    // Side 1
    
    Vec1[0] = ThisGrid.NodeList(Node2).x() - ThisGrid.NodeList(Node1).x();
    Vec1[1] = ThisGrid.NodeList(Node2).y() - ThisGrid.NodeList(Node1).y();
    Vec1[2] = ThisGrid.NodeList(Node2).z() - ThisGrid.NodeList(Node1).z();
 
    Mag = sqrt(vector_dot(Vec1,Vec1));
    
    Vec1[0] /= Mag;
    Vec1[1] /= Mag;
    Vec1[2] /= Mag;
    
    // Side 2
    
    Vec2[0] = ThisGrid.NodeList(Node3).x() - ThisGrid.NodeList(Node2).x();
    Vec2[1] = ThisGrid.NodeList(Node3).y() - ThisGrid.NodeList(Node2).y();
    Vec2[2] = ThisGrid.NodeList(Node3).z() - ThisGrid.NodeList(Node2).z();
 
    Mag = sqrt(vector_dot(Vec2,Vec2));
    
    Vec2[0] /= Mag;
    Vec2[1] /= Mag;
    Vec2[2] /= Mag;
     
    // Side 3
    
    Vec3[0] = ThisGrid.NodeList(Node4).x() - ThisGrid.NodeList(Node3).x();
    Vec3[1] = ThisGrid.NodeList(Node4).y() - ThisGrid.NodeList(Node3).y();
    Vec3[2] = ThisGrid.NodeList(Node4).z() - ThisGrid.NodeList(Node3).z();
 
    Mag = sqrt(vector_dot(Vec3,Vec3));
    
    Vec3[0] /= Mag;
    Vec3[1] /= Mag;
    Vec3[2] /= Mag;
    
    // Side 4
    
    Vec4[0] = ThisGrid.NodeList(Node1).x() - ThisGrid.NodeList(Node4).x();
    Vec4[1] = ThisGrid.NodeList(Node1).y() - ThisGrid.NodeList(Node4).y();
    Vec4[2] = ThisGrid.NodeList(Node1).z() - ThisGrid.NodeList(Node4).z();
 
    Mag = sqrt(vector_dot(Vec4,Vec4));
    
    Vec4[0] /= Mag;
    Vec4[1] /= Mag;
    Vec4[2] /= Mag;
        
    // Check all four angles
    
    double Good = 130.*PI/180.;
    
    Angle = vector_dot(Vec1,Vec2); Angle = MIN(1.,MAX(-1.,Angle)); Angle = acos(Angle);
    
    if ( Angle <= Good ) {
    
       Angle = vector_dot(Vec2,Vec3); Angle = MIN(1.,MAX(-1.,Angle)); Angle = acos(Angle);
    
       if ( Angle <= Good ) {
    
         Angle = vector_dot(Vec3,Vec4); Angle = MIN(1.,MAX(-1.,Angle)); Angle = acos(Angle);
      
         if ( Angle <= Good ) {
    
            Angle = vector_dot(Vec4,Vec1); Angle = MIN(1.,MAX(-1.,Angle)); Angle = acos(Angle);
         
            if ( Angle <= Good ) {    
             
               return 1;
               
            }
            
         }
         
       }
       
    }
    
    return 0;
    
}
 
 
