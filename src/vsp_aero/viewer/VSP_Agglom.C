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
    
    printf("Copying of agglom objects not implemented! \n");fflush(NULL);
    
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
    
    CleanUpLoneLoops_();

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
    
    // Allocate space for the vortex loop list... note that the number of tris, 
    // and the tri data is used in lieu of loops. These are only tris on the 
    // fine grid, and are truly loops on the coarser grids
    
    VortexLoopWasAgglomerated_ = new int[FineGrid().NumberOfTris() + 1];
    
    for ( i = 1 ; i <= FineGrid().NumberOfTris() ; i++ ) {
     
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
    
    // Interior node
    
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

    int i, j, Side, Loop1, Loop2;

    // Check each side of this edge
    
    for ( Side = 1 ; Side <= 2 ; Side++ ) {
     
       if ( Side == 1 ) Loop1 = FineGrid().EdgeList(NextBestEdgeOnFront_).Tri1();
       if ( Side == 2 ) Loop1 = FineGrid().EdgeList(NextBestEdgeOnFront_).Tri2();
    
       if ( VortexLoopWasAgglomerated_[Loop1] > 0 ) {
     
          VortexLoopWasAgglomerated_[Loop1] *= -1;
          
          // Agglomerate loops that share an edge with this loop
          
          for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
           
             if ( i != NextBestEdgeOnFront_ ) {
              
                if ( FineGrid().EdgeList(i).Tri1() == Loop1 ||
                     FineGrid().EdgeList(i).Tri2() == Loop1 ) {
           
                   Loop2 = FineGrid().EdgeList(i).Tri1() + FineGrid().EdgeList(i).Tri2() - Loop1;
                   
                   if ( VortexLoopWasAgglomerated_[Loop2] > 0 ) {
                    
                      VortexLoopWasAgglomerated_[Loop2] = -Loop1;
                      
                      // Now add edges of this loop to the front
                      
                      for ( j = 1 ; j <= FineGrid().NumberOfEdges() ; j++ ) {
                       
                         if ( j != NextBestEdgeOnFront_ ) {
                          
                            if ( FineGrid().EdgeList(j).Tri1() == Loop2 ||
                                 FineGrid().EdgeList(j).Tri2() == Loop2 ) {
                             
                               if ( EdgeIsOnFront_[j] == 0 ) EdgeIsOnFront_[j] = INTERIOR_EDGE_BC;
                               
                            }
                            
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

    /*
    printf("%d %d %d \n",
           NumberOfEdgesOnTE_,
           NumberOfEdgesOnLE_,
           NumberOfEdgesOnBoundary_);
      */
    
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
    double LoopArea;
    
    // Determine number of coarse grid loops
    
    CoarseGridLoop = new int[FineGrid().NumberOfTris() + 1];
    
    zero_int_array(CoarseGridLoop, FineGrid().NumberOfTris());
  
    NumberOfCoarseGridLoops = 0;
    
    for ( i = 1 ; i <= FineGrid().NumberOfTris() ; i++ ) {
  
       if ( ABS(VortexLoopWasAgglomerated_[i]) == i ) CoarseGridLoop[i] = ++NumberOfCoarseGridLoops;
       
    }
    
    // Determine how many possible neighbor loops for each loop... 

    NumberOfNeighborLoops = new int[FineGrid().NumberOfTris() + 1];
    
    zero_int_array(NumberOfNeighborLoops, FineGrid().NumberOfTris());

    for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {

       Loop1 = ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(i).Tri1()]);
       Loop2 = ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(i).Tri2()]);
       
       if ( Loop1 != Loop2 ) {
        
          NumberOfNeighborLoops[Loop1]++;
          NumberOfNeighborLoops[Loop2]++;
        
       }
       
    }
 
    // Allocate space for loop to neighbor loop list
    
    NeighborLoopList = new int*[FineGrid().NumberOfTris() + 1];
    
    for ( i = 1 ; i <= FineGrid().NumberOfTris() ; i++ ) {
     
       if ( NumberOfNeighborLoops[i] > 0 ) {
      
          NeighborLoopList[i] = new int[NumberOfNeighborLoops[i] + 1];
       
       }
       
    }
    
    // Now pack the list
    
    zero_int_array(NumberOfNeighborLoops, FineGrid().NumberOfTris());
   
    for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {

       Loop1 = ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(i).Tri1()]);
       Loop2 = ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(i).Tri2()]);
       
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
    
    for ( Loop1 = 1 ; Loop1 <= FineGrid().NumberOfTris() ; Loop1++ ) {
     
          if ( NumberOfNeighborLoops[Loop1] == 1 ) {

             Loop2 = NeighborLoopList[Loop1][1];
             
             IsInteriorLoop = 1;
             
             j = 1;
             
             while ( j <= FineGrid().NumberOfEdges() && IsInteriorLoop ) {
              
                LoopA = FineGrid().EdgeList(j).Tri1();
                LoopB = FineGrid().EdgeList(j).Tri2();
                
                if ( LoopA == Loop1 || LoopB == Loop1 ) {
                 
                   if ( ABS(EdgeIsOnFront_[j]) != INTERIOR_EDGE_BC ) IsInteriorLoop = 0;
                   
                }
              
                j++;
                
             }
                
             if ( IsInteriorLoop && 5.*FineGrid().TriList(Loop1).Area() <= FineGrid().TriList(Loop2).Area() ) {

                VortexLoopWasAgglomerated_[Loop1] = -Loop2;
                
                printf("Loop1: %d \n",Loop1);
                printf("Loop2: %d \n",Loop2);
                printf("VortexLoopWasAgglomerated_[Loop2]: %d \n",VortexLoopWasAgglomerated_[Loop2]);fflush(NULL);
                
                for ( j = 1 ; j <= FineGrid().NumberOfEdges() ; j++ ) {
                          
                   LoopA = FineGrid().EdgeList(j).Tri1();
                   LoopB = FineGrid().EdgeList(j).Tri2();
       
                   if ( ( LoopA == Loop1 && LoopB == Loop2 ) ||
                        ( LoopA == Loop2 && LoopB == Loop1 ) ) {
                    
                      if ( EdgeIsOnFront_[j] == INTERIOR_EDGE_BC ) EdgeIsOnFront_[j] = -INTERIOR_EDGE_BC;   
                   
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
    
    int i, Loop1, Loop2, Next;
    int NumberOfCoarseGridNodes, NumberOfCoarseGridEdges, NumberOfCoarseGridLoops;
    double Area;
    
    // Create a list of the fine edges still in use on the coarse grid
       
    for ( i = 1 ; i <= FineGrid().NumberOfEdges() ; i++ ) {
     
       if ( ABS(EdgeIsOnFront_[i]) == CORNER_BC        ) CoarseEdgeList_[i] = 1;
       if ( ABS(EdgeIsOnFront_[i]) == TE_EDGE_BC       ) CoarseEdgeList_[i] = 1;  
       if ( ABS(EdgeIsOnFront_[i]) == LE_EDGE_BC       ) CoarseEdgeList_[i] = 1;    
       if ( ABS(EdgeIsOnFront_[i]) == BOUNDARY_EDGE_BC ) CoarseEdgeList_[i] = 1;
       
       Loop1 = FineGrid().EdgeList(i).Tri1();
       Loop2 = FineGrid().EdgeList(i).Tri2();
       
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
    
    for ( i = 1 ; i <= FineGrid().NumberOfTris() ; i++ ) {
  
       if ( ABS(VortexLoopWasAgglomerated_[i]) == i ) VortexLoopWasAgglomerated_[i] = ++NumberOfCoarseGridLoops;
       
    }
    
    for ( i = 1 ; i <= FineGrid().NumberOfTris() ; i++ ) {
  
       if ( VortexLoopWasAgglomerated_[i] < 0 ) VortexLoopWasAgglomerated_[i] = VortexLoopWasAgglomerated_[ABS(VortexLoopWasAgglomerated_[i])];
               
    }     

    // Allocate space for the coarse grid

    CoarseGrid_ = new VSP_GRID;

    CoarseGrid().SizeNodeList(NumberOfCoarseGridNodes);
    
    CoarseGrid().SizeEdgeList(NumberOfCoarseGridEdges);
    
    CoarseGrid().SizeTriList(NumberOfCoarseGridLoops);
    
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

          // Left and right loops
          
          CoarseGrid().EdgeList(Next).Tri1() = ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(i).Tri1()]);
          CoarseGrid().EdgeList(Next).Tri2() = ABS(VortexLoopWasAgglomerated_[FineGrid().EdgeList(i).Tri2()]);

          // Pointer to this course grid edge from fine grid
          
          FineGrid().EdgeList(i).CourseGridEdge() = Next;
           
       }
       
    }   
    
    // Copy over fine loop data to the coarse grid
    
    Next = 0;
    
    for ( i = 1 ; i <= FineGrid().NumberOfTris() ; i++ ) {
     
       if ( ABS(VortexLoopWasAgglomerated_[i]) == i ) {
        
          Next++;
          
          CoarseGrid().TriList(Next) = FineGrid().TriList(i);
          
          // Pointer from fine grid to coarse
                
          CoarseGrid().TriList(i).FineGridLoop() = i;     
    
       }
       
    }     
    
    // Zero out coarse grid data that needs to be agglomerated
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfTris() ; i++ ) {

       // Triangle Normal
       
       CoarseGrid().TriList(i).Nx() = 0.;
       CoarseGrid().TriList(i).Ny() = 0.;
       CoarseGrid().TriList(i).Nz() = 0.; 
       
       // Local Camber Line Normal
       
       CoarseGrid().TriList(i).NxCamber() = 0.;
       CoarseGrid().TriList(i).NyCamber() = 0.;
       CoarseGrid().TriList(i).NzCamber() = 0.;
       
       // Centroid
       
       CoarseGrid().TriList(i).Xc() = 0.;
       CoarseGrid().TriList(i).Yc() = 0.;
       CoarseGrid().TriList(i).Zc() = 0.;           
     
    }

    // Agglomerate, area weighted, the fine grid data to the coarse grid
    
    for ( i = 1 ; i <= FineGrid().NumberOfTris() ; i++ ) {

       Next = ABS(VortexLoopWasAgglomerated_[i]);

       Area = FineGrid().TriList(i).Area();
       
       // Area
       
       CoarseGrid().TriList(Next).Area() += Area*FineGrid().TriList(i).Area();

       // Triangle Normal
       
       CoarseGrid().TriList(Next).Nx() += Area*FineGrid().TriList(i).Nx();
       CoarseGrid().TriList(Next).Ny() += Area*FineGrid().TriList(i).Ny();
       CoarseGrid().TriList(Next).Nz() += Area*FineGrid().TriList(i).Nz();
       
       // Local Camber Line Normal
       
       CoarseGrid().TriList(Next).NxCamber() += Area*FineGrid().TriList(i).NxCamber();
       CoarseGrid().TriList(Next).NyCamber() += Area*FineGrid().TriList(i).NyCamber();
       CoarseGrid().TriList(Next).NzCamber() += Area*FineGrid().TriList(i).NzCamber();
       
       // Centroid
       
       CoarseGrid().TriList(Next).Xc() += Area*FineGrid().TriList(i).Xc();
       CoarseGrid().TriList(Next).Yc() += Area*FineGrid().TriList(i).Yc();
       CoarseGrid().TriList(Next).Zc() += Area*FineGrid().TriList(i).Zc();      
       
       // Pointer from fine grid to coarse
                
       FineGrid().TriList(i).CoarseGridLoop() = Next;     
       
    }    
    
    // Area average the results
    
    for ( i = 1 ; i <= CoarseGrid().NumberOfTris() ; i++ ) {

       Area = CoarseGrid().TriList(i).Area();
       
       // Area
       
       CoarseGrid().TriList(Next).Area() /= Area;

       // Triangle Normal
       
       CoarseGrid().TriList(Next).Nx() /= Area;
       CoarseGrid().TriList(Next).Ny() /= Area;
       CoarseGrid().TriList(Next).Nz() /= Area;
       
       // Local Camber Line Normal
       
       CoarseGrid().TriList(Next).NxCamber() /= Area;
       CoarseGrid().TriList(Next).NyCamber() /= Area;
       CoarseGrid().TriList(Next).NzCamber() /= Area;
       
       // Centroid
       
       CoarseGrid().TriList(Next).Xc() /= Area;
       CoarseGrid().TriList(Next).Yc() /= Area;
       CoarseGrid().TriList(Next).Zc() /= Area; 
       
    }        

}

