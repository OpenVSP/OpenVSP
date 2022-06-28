//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "QuadTree.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                               QUAD_TREE constructor                          #
#                                                                              #
##############################################################################*/

QUAD_TREE::QUAD_TREE(void)
{

    NumberOfNodes_ = 0;
    NumberOfCells_ = 0;
    NumberOfEdges_ = 0;
    
    MaxNumberOfNodes_ = 0;
    MaxNumberOfEdges_ = 0;    
    MaxNumberOfCells_ = 0;
        
    NodeList_ = NULL;
    EdgeList_ = NULL;
    CellList_ = NULL;
    
    Direction_ = 0;
    
}

/*##############################################################################
#                                                                              #
#                              QUAD_TREE destructor                            #
#                                                                              #
##############################################################################*/

QUAD_TREE::~QUAD_TREE(void)
{

    if ( NodeList_ != NULL ) delete [] NodeList_;
    if ( EdgeList_ != NULL ) delete [] EdgeList_;    
    if ( CellList_ != NULL ) delete [] CellList_;    

    NumberOfNodes_ = 0;
    NumberOfCells_ = 0;
    NumberOfEdges_ = 0;
    
    MaxNumberOfNodes_ = 0;
    MaxNumberOfEdges_ = 0;    
    MaxNumberOfCells_ = 0;
        
    NodeList_ = NULL;
    EdgeList_ = NULL;
    CellList_ = NULL;
    
    Direction_ = 0;
        
}

/*##############################################################################
#                                                                              #
#                                QUAD_TREE Copy                                #
#                                                                              #
##############################################################################*/

QUAD_TREE::QUAD_TREE(const QUAD_TREE &QuadTree)
{
   
    PRINTF("Copy not implemented for QUAD_TREE class! \n");
    exit(1);
     
}

/*##############################################################################
#                                                                              #
#                       QUAD_TREE ResizeNodeList                               #
#                                                                              #
##############################################################################*/

void QUAD_TREE::ResizeNodeList(int NewSize)
{
   
    int i;
    QUAD_NODE *NewList;
    
    NewList = new QUAD_NODE[NewSize + 1];
    
    for ( i = 1 ; i <= NumberOfNodes_ ; i++ ) {
       
       NewList[i] = NodeList_[i]; // Default copy
       
    }
    
    if ( NodeList_ != NULL ) delete [] NodeList_;
    
    NodeList_ = NewList;

    MaxNumberOfNodes_ = NewSize;
        
}

/*##############################################################################
#                                                                              #
#                       QUAD_TREE ResizeEdgeList                               #
#                                                                              #
##############################################################################*/

void QUAD_TREE::ResizeEdgeList(int NewSize)
{
   
    int i;
    QUAD_EDGE *NewList;
    
    NewList = new QUAD_EDGE[NewSize + 1];
    
    for ( i = 1 ; i <= NumberOfEdges_ ; i++ ) {
       
       NewList[i] = EdgeList_[i]; // Default copy
       
    }
    
    if ( EdgeList_ != NULL ) delete [] EdgeList_;
    
    EdgeList_ = NewList;
    
    MaxNumberOfEdges_ = NewSize;

}

/*##############################################################################
#                                                                              #
#                       QUAD_TREE ResizeCellList                               #
#                                                                              #
##############################################################################*/

void QUAD_TREE::ResizeCellList(int NewSize)
{
   
    int i;
    QUAD_CELL *NewList;
    
    NewList = new QUAD_CELL[NewSize + 1];
   
    for ( i = 1 ; i <= NumberOfCells_ ; i++ ) {
       
       NewList[i] = CellList_[i]; // Default copy
       
    }
    
    if ( CellList_ != NULL ) delete [] CellList_;
    
    CellList_ = NewList;
    
    MaxNumberOfCells_ = NewSize;
        
}

/*##############################################################################
#                                                                              #
#                       QUAD_TREE InitializeTree                               #
#                                                                              #
##############################################################################*/

void QUAD_TREE::InitializeTree(VSPAERO_DOUBLE X_min, VSPAERO_DOUBLE X_max,
                               VSPAERO_DOUBLE Y_min, VSPAERO_DOUBLE Y_max,
                               VSPAERO_DOUBLE Z_min, VSPAERO_DOUBLE Z_max,
                               VSPAERO_DOUBLE Value)
                               
{
   
    BBOX Box;
    
    NumberOfCells_ = 0;
    NumberOfNodes_ = 0;
    
    MaxNumberOfCells_ = 10000;    
    MaxNumberOfNodes_ =  4*MaxNumberOfCells_;
    MaxNumberOfEdges_ =  6*MaxNumberOfCells_;
        
    CellList_ = new QUAD_CELL[MaxNumberOfCells_ + 1];        
    NodeList_ = new QUAD_NODE[MaxNumberOfNodes_ + 1];
    EdgeList_ = new QUAD_EDGE[MaxNumberOfEdges_ + 1];
        
    NumberOfCells_ = 1;
    NumberOfNodes_ = 4;
    NumberOfEdges_ = 4;

    if ( Direction_ == 1 ) {
   
       Box.x_min = Y_min;
       Box.x_max = Y_max;
                        
       Box.y_min = Z_min;
       Box.y_max = Z_max;
                        
       Box.z_min = Value;
       Box.z_max = Value;   
       
    }
    
    else if ( Direction_ == 2 ) {
       
       Box.x_min = X_min;
       Box.x_max = X_max;
                        
       Box.y_min = Z_min;
       Box.y_max = Z_max;
                        
       Box.z_min = Value;
       Box.z_max = Value;
       
    }       

    else if ( Direction_ == 3 ) {
       
       Box.x_min = X_min;
       Box.x_max = X_max;
                        
       Box.y_min = Y_min;
       Box.y_max = Y_max;
                        
       Box.z_min = Value;
       Box.z_max = Value;
       
    }  
    
    else {
       
       printf("Unknown quadtree plane type! \n");fflush(NULL);
       exit(1);
       
    }

    // Coordinate mapping
    
    if ( Direction_ == 1 ) {
       
       DirMap_[0] = 2;
       DirMap_[1] = 0;
       DirMap_[2] = 1;
       
    }
    
    else if ( Direction_ == 2 ) {
    
       DirMap_[0] = 0;
       DirMap_[1] = 2;
       DirMap_[2] = 1;
       
    }

    else if ( Direction_ == 3 ) {
    
       DirMap_[0] = 0;
       DirMap_[1] = 1;
       DirMap_[2] = 2;
       
    }
              
    // Node data
    
    NodeList_[1].xyz(0) = Box.x_min;
    NodeList_[1].xyz(1) = Box.y_min;
    NodeList_[1].xyz(2) = Box.z_min;

    NodeList_[2].xyz(0) = Box.x_max;
    NodeList_[2].xyz(1) = Box.y_min;
    NodeList_[2].xyz(2) = Box.z_min;

    NodeList_[3].xyz(0) = Box.x_max;
    NodeList_[3].xyz(1) = Box.y_max;
    NodeList_[3].xyz(2) = Box.z_min;

    NodeList_[4].xyz(0) = Box.x_min;
    NodeList_[4].xyz(1) = Box.y_max;
    NodeList_[4].xyz(2) = Box.z_min;
    
    // Edge data
    
    EdgeList_[1].Cell(0) = 1;
    EdgeList_[1].Cell(1) = 0;

    EdgeList_[2].Cell(0) = 1;
    EdgeList_[2].Cell(1) = 0;
    
    EdgeList_[3].Cell(0) = 1;
    EdgeList_[3].Cell(1) = 0;

    EdgeList_[4].Cell(0) = 1;
    EdgeList_[4].Cell(1) = 0;
                
    // First cell
    
    CellList_[1].Node(0) = 1;           
    CellList_[1].Node(1) = 2;           
    CellList_[1].Node(2) = 3;           
    CellList_[1].Node(3) = 4;           

    CellList_[1].Parent() = 0;

    CellList_[1].Child(0) = 0;           
    CellList_[1].Child(1) = 0;           
    CellList_[1].Child(2) = 0;           
    CellList_[1].Child(3) = 0;      
    
    CellList_[1].Edge(0) = 1;           
    CellList_[1].Edge(1) = 2;           
    CellList_[1].Edge(2) = 3;           
    CellList_[1].Edge(3) = 4;          

    // No point inserted yet
    
    CellList_[1].HasPoint() = 0;
    
    // Level
    
    CellList_[1].Level() = 1;
                       
}

/*##############################################################################
#                                                                              #
#                           QUAD_TREE ClearTree                                #
#                                                                              #
##############################################################################*/

void QUAD_TREE::ClearTree(void)
{
   
    int i;
    
    for ( i = 1 ; i <= NumberOfCells_ ; i++ ) {
       
       CellList_[1].HasPoint() = 0;
       
       CellList_[i].Child(0) = 0;           
       CellList_[i].Child(1) = 0;           
       CellList_[i].Child(2) = 0;           
       CellList_[i].Child(3) = 0;  
           
    }           
       
    NumberOfCells_ = 0;
    NumberOfNodes_ = 0;
                      
}

/*##############################################################################
#                                                                              #
#                       QUAD_TREE InitializeTree                               #
#                                                                              #
##############################################################################*/

int QUAD_TREE::InsertPoint(VSPAERO_DOUBLE xyzp[3], int SurfaceEdge)
{
   
    int Done, Cell, i, j, k, Inserted, Found, Child, Neighbor;
    VSPAERO_DOUBLE xyz[3], Distance, Tolerance;
    
    Tolerance = 1.e-2;
  
    if ( Direction_ == 1 ) {
    
       xyz[0] = xyzp[1];
       xyz[1] = xyzp[2];
       xyz[2] = xyzp[0];

    }
    
    else if ( Direction_ == 2 ) {
    
       xyz[0] = xyzp[0];
       xyz[1] = xyzp[2];
       xyz[2] = xyzp[1];
                    
    }
    
    else if ( Direction_ == 3 ) {
    
       xyz[0] = xyzp[0];
       xyz[1] = xyzp[1];
       xyz[2] = xyzp[2];

    }
    
    else {
       
       PRINTF("Must specificy quad tree direction before inserting points! \n");fflush(NULL);
       exit(1);
       
    }
     
    // Start at top
    
    Inserted = 0;
       
    Cell = 1;
    
    if ( InsideQuad(Cell,xyz) ) {
       
       // First time
       
       if ( !CellList_[Cell].HasPoint() ) {
          
          CellList_[Cell].HasPoint() = 1;
          
          CellList_[Cell].SurfaceEdge() = SurfaceEdge;
          
          CellList_[Cell].InsertedPoint(0) = xyz[0];
          CellList_[Cell].InsertedPoint(1) = xyz[1];
          CellList_[Cell].InsertedPoint(2) = xyz[2];
          
          Inserted = 1;
          
          return Inserted;
          
       }
       
    }
    
    // Point is outside of bbox... return 0
    
    else {
       
       return 0;
       
    }
    
    // Insert point 2 and on...
  
    Inserted = Done = 0;
  
    Cell = 1;
  
    while ( !Done ) {
  
       // Parent() cell has no children and empty
       
       if ( !CellList_[Cell].HasPoint() && CellList_[Cell].Child(0) == 0 ) {
          
          CellList_[Cell].HasPoint() = 1;
          
          CellList_[Cell].SurfaceEdge() = SurfaceEdge;

          CellList_[Cell].InsertedPoint(0) = xyz[0];
          CellList_[Cell].InsertedPoint(1) = xyz[1];
          CellList_[Cell].InsertedPoint(2) = xyz[2];
          
          Done = Inserted = 1;
          
       }  
       
       // Parent() has children
       
       else if ( CellList_[Cell].Child(0) != 0 ) {
        
          // Find new child cell we are now in
          
          Found = 0;
          
          i = 0;
          
          while ( i <= 3 && !Found ) {
          
             Child = CellList_[Cell].Child(i);
             
             if ( InsideQuad(Child,xyz) ) {
                
                Found = Child;
                
             }
             
             i++;
             
          }
          
          if ( Found == 0 ) { printf("wtf! \n"); exit(1); };
          
          // Move onto child cell
          
          Cell = Found;          
             
       }  
       
       // Parent() cell has no children and has a point
       
       if ( CellList_[Cell].HasPoint() && CellList_[Cell].Child(0) == 0 ) {
          
          // Check if new point is same as previous inserted point
          
          Distance = sqrt( pow(CellList_[Cell].InsertedPoint(0) - xyz[0],2)
                         + pow(CellList_[Cell].InsertedPoint(1) - xyz[1],2)
                         + pow(CellList_[Cell].InsertedPoint(2) - xyz[2],2) );
                   
          if ( Distance > Tolerance ) {
  
             // Split cell, this also redistributes a possible inserted point...
             
             SplitCell(Cell);
  
             // Find new child cell we are now in
             
             Found = 0;
             
             i = 0;
             
             while ( i <= 3 && !Found ) {
             
                Child = CellList_[Cell].Child(i);
                
                if ( InsideQuad(Child,xyz) ) {
                   
                   Found = Child;
                   
                }
                
                i++;
                
             }
             
             // Move onto child cell
             
             Cell = Found;   
             
          }      
          
          else {
             
             Done = 1;
             
          }   
          
       }
  
    }
    
    return Inserted;
                        
}

/*##############################################################################
#                                                                              #
#                           QUAD_TREE InsideQuad                               #
#                                                                              #
##############################################################################*/

int QUAD_TREE::InsideQuad(int Cell, VSPAERO_DOUBLE xyz[3])
{
   
    int Node1, Node3;
    VSPAERO_DOUBLE x_min, y_min, x_max, y_max;
    
    Node1 = CellList_[Cell].Node(0);
    Node3 = CellList_[Cell].Node(2);
    
    x_min = NodeList_[Node1].xyz(0);
    y_min = NodeList_[Node1].xyz(1);

    x_max = NodeList_[Node3].xyz(0);
    y_max = NodeList_[Node3].xyz(1);
        
    if ( xyz[0] >= x_min && xyz[0] <= x_max ) {
     
       if ( xyz[1] >= y_min && xyz[1] <= y_max  ) {
        
          return 1;
          
       }
       
    }
    
    return 0;

}

/*##############################################################################
#                                                                              #
#                          QUAD_TREE BufferTree                                #
#                                                                              #
##############################################################################*/

void QUAD_TREE::BufferTree(int Level)
{
   
   int i, j, k,edge, NeighborCell, NumCells, Split;

   //for ( k = 1 ; k <= 1 ; k++ ) {
   //   
   //   NumCells = NumberOfCells_;
   //   
   //   for ( i = 1 ; i <= NumCells ; i++ ) {
   //      
   //      if ( CellList_[i].Child(0) == 0 ) {
   //         
   //         SplitCell(i);
   //         
   //      }
   //      
   //   }
   //   
   //}
   
   for ( k = 1 ; k <= 3 ; k++ ) {
      
      NumCells = NumberOfCells_;
      
      for ( i = 1 ; i <= NumCells ; i++ ) {
         
         if ( CellList_[i].Level() < 3 && CellList_[i].Child(0) == 0 ) {
            
            SplitCell(i);
            
         }

         if ( CellList_[i].Child(0) == 0 ) {
            
            // loop over edges
            
            Split = 0;
            
            j = 0;
            
            for ( j = 0 ; j <= 7 ; j++ ) {
               
               edge = CellList_[i].Edge(j);
               
               if ( edge > 0 ) Split++;
   
            }
            
            if ( Split >= 6 ) SplitCell(i);

         }
         
      }
    
   }
   
}

/*##############################################################################
#                                                                              #
#                          QUAD_TREE SmoothPressure                            #
#                                                                              #
##############################################################################*/

void QUAD_TREE::SmoothPressure(void)
{
   
//   int i, j, k, p, Edge, NeighborCell;
//   VSPAERO_DOUBLE *NodeArea;
//
//   NodeArea = new VSPAERO_DOUBLE[c + 1];
//   
//   zero_double_array(NodeArea, NumberOfNodes_);
//   
//   for ( i = 1 ; i <= NumberOfCells_ ; i++ ) {
//         
//      if ( CellList_[i].Child(0) == 0 && CellList_[i].HasPoint() ) {
//
//         // Set all points to this cell Cp
//         
//         for ( j = 0 ; j <= 3 ; j++ ) {
//            
//            NodeList_[CellList_[i].Node(j)].Cp() = CellList_[i].Cp();  
//            
//            NodeArea[CellList_[i].Node(j)] += 
//            
//         }
//         
//         // Set all neighbors that do have points to the cell Cp
//         
//         //for ( j = 0 ; j <= 7 ; j++ ) {
//         //   
//         //   Edge = CellList_[i].Edge(j);
//         //   
//         //   if ( Edge > 0 ) {
//         //   
//         //      for ( k = 0 ; k <= 1 ; k++ ) {
//         //      
//         //         NeighborCell = EdgeList_[Edge].Cell(k);
//         //         
//         //         if ( NeighborCell > 0 ) {
//         //            
//         //            if ( CellList_[NeighborCell].Level() >= CellList_[i].Level() ) {
//         //                  
//         //               if ( !CellList_[NeighborCell].HasPoint() ) {   
//         //              
//         //                 for ( p = 0 ; p <= 3 ; p++ ) {
//         //                           
//         //                    NodeList_[CellList_[NeighborCell].Node(p)].Cp() = CellList_[i].Cp();  
//         //                    
//         //                    NodeList_[CellList_[NeighborCell].Node(p)].velocity(0) = 0.;
//         //                    NodeList_[CellList_[NeighborCell].Node(p)].velocity(1) = 0.;
//         //                    NodeList_[CellList_[NeighborCell].Node(p)].velocity(2) = 0.;
//         //                                
//         //                 }
//         //                 
//         //               }
//         //               
//         //            }
//         //            
//         //         }
//         //         
//         //      }
//         //      
//         //   }
//         //   
//         //}
//          
//      }
//      
//   }
//   
//   
//   for ( i = 1 ; i <= NumberOfCells_ ; i++ ) {
//         
//      if ( CellList_[i].Child(0) == 0 && CellList_[i].HasPoint() ) {
//            
//         for ( j = 0 ; j <= 3 ; j++ ) {
//            
//      //      NodeList_[CellList_[i].Node(j)].Cp() = CellList_[i].Cp();  
//            
//         }
//       
//         
//      }
//      
//   }
 
}

/*##############################################################################
#                                                                              #
#                           QUAD_TREE SplitCell                                #
#                                                                              #
##############################################################################*/

void QUAD_TREE::SplitCell(int Cell)
{
   
    int Node[10], NewNode, Child, i, Inserted, NumEdges, NewEdges, e1, e2;
    int Edge, NeighborCell, EdgeMap[8];
    int EdgeList[12];
    VSPAERO_DOUBLE xyz[5][3];
    
    // Loop over neighbor cells
    
    for ( i = 0 ; i <= 7 ; i++ ) {
       
       Edge = CellList_[Cell].Edge(i);
       
       if ( Edge > 0 ) {
          
          NeighborCell = EdgeList_[Edge].Cell(0) + EdgeList_[Edge].Cell(1) - Cell;

          if ( EdgeList_[Edge].Cell(0) != Cell && EdgeList_[Edge].Cell(1) != Cell ) {
             
             printf("wtf! \n");fflush(NULL);
             exit(1);
             
          }
          
          if ( NeighborCell > 0 ) {
             
             if ( CellList_[Cell].Level() > CellList_[NeighborCell].Level() ) {
        
                SplitCell(NeighborCell);
                
             }
             
          }
          
       }
       
       else if ( NeighborCell < 0 ) {
          
          printf("wtf! NeighborCell: %d \n",NeighborCell);
          fflush(NULL);
          exit(1);
          
       }
       
    }
    
    // 4 nodes defining the original quad
    
    Node[1] = CellList_[Cell].Node(0);
    Node[2] = CellList_[Cell].Node(1);
    Node[3] = CellList_[Cell].Node(2);
    Node[4] = CellList_[Cell].Node(3);

    xyz[1][0] = NodeList_[Node[1]].xyz(0);
    xyz[1][1] = NodeList_[Node[1]].xyz(1);
    xyz[1][2] = NodeList_[Node[1]].xyz(2);
       
    xyz[2][0] = NodeList_[Node[2]].xyz(0);
    xyz[2][1] = NodeList_[Node[2]].xyz(1);
    xyz[2][2] = NodeList_[Node[2]].xyz(2);
     
    xyz[3][0] = NodeList_[Node[3]].xyz(0);
    xyz[3][1] = NodeList_[Node[3]].xyz(1);
    xyz[3][2] = NodeList_[Node[3]].xyz(2);
      
    xyz[4][0] = NodeList_[Node[4]].xyz(0);
    xyz[4][1] = NodeList_[Node[4]].xyz(1);
    xyz[4][2] = NodeList_[Node[4]].xyz(2);
                
    // Create new points
                
    if ( NumberOfNodes_ + 5 > MaxNumberOfNodes_ ) ResizeNodeList((int)(1.1*MaxNumberOfNodes_));
    
    // Center
    
    Node[9] = NewNode = ++NumberOfNodes_;
    
    NodeList_[NewNode].xyz(0) = 0.5*( xyz[1][0] + xyz[3][0] );
    NodeList_[NewNode].xyz(1) = 0.5*( xyz[1][1] + xyz[3][1] );
    NodeList_[NewNode].xyz(2) = 0.5*( xyz[1][2] + xyz[3][2] );   
    
    // Create new cells
    
    if ( NumberOfCells_ + 4 > MaxNumberOfCells_ ) ResizeCellList((int)(1.1*MaxNumberOfCells_));

    CellList_[Cell].Child(0) = ++NumberOfCells_;
    CellList_[Cell].Child(1) = ++NumberOfCells_;
    CellList_[Cell].Child(2) = ++NumberOfCells_;
    CellList_[Cell].Child(3) = ++NumberOfCells_;   
        
    // Determine what edges are not already split... and create any new edges on the boundaries of the parent cell
    
    NumEdges = 0;
    
    for ( i = 0 ; i <= 7 ; i++ ) {
       
       if ( CellList_[Cell].Edge(i) > 0 ) NumEdges++;
       
    }
    
    NewEdges = 12 - NumEdges;
    
    if ( NumberOfEdges_ + NewEdges > MaxNumberOfEdges_ ) ResizeEdgeList((int)(1.1*MaxNumberOfEdges_));
             
    // Map local edge to corresponding local edge on neighbor

    EdgeMap[0] = 6;
    EdgeMap[1] = 7;
    EdgeMap[2] = 4;
    EdgeMap[3] = 5;
    EdgeMap[4] = 2;
    EdgeMap[5] = 3;
    EdgeMap[6] = 0;
    EdgeMap[7] = 1;
        
    // Edges 0 and 4
       
       i = 0;
       
       e1 = CellList_[Cell].Edge(i  );
       
       NeighborCell = EdgeList_[e1].Cell(0) + EdgeList_[e1].Cell(1) - Cell; // neighbor cell

       // Update e1
       
       EdgeList_[e1].Cell(0) = NeighborCell;
       EdgeList_[e1].Cell(1) = CellList_[Cell].Child(3);

       e2 = CellList_[Cell].Edge(i+4);
           
       // e2 exists, so neighbor was split already
           
       if ( e2 > 0 ) {

          NeighborCell = EdgeList_[e2].Cell(0) + EdgeList_[e2].Cell(1) - Cell; // neighbor cell
           
          // Left mid node
          
          Node[8] = CellList_[NeighborCell].Node(2);
 
       }
       
       // e2 does not exist, so neighbor cell is not split yet...
       
       else {
          
          e2 = CellList_[Cell].Edge(i+4) = ++NumberOfEdges_;
          
          CellList_[Cell        ].Edge(        i  )  = e1;
          CellList_[Cell        ].Edge(        i+4)  = e2;        
             
          CellList_[NeighborCell].Edge(EdgeMap[i  ]) = e1;
          CellList_[NeighborCell].Edge(EdgeMap[i+4]) = e2;   

          // Left mid node
          
          Node[8] = NewNode = ++NumberOfNodes_;
          
          NodeList_[NewNode].xyz(0) = 0.5*( xyz[1][0] + xyz[4][0] );
          NodeList_[NewNode].xyz(1) = 0.5*( xyz[1][1] + xyz[4][1] );
          NodeList_[NewNode].xyz(2) = 0.5*( xyz[1][2] + xyz[4][2] );       
                              
       }

       EdgeList_[e2].Cell(0) = NeighborCell;
       EdgeList_[e2].Cell(1) = CellList_[Cell].Child(0);

    // Edges 1 and 5
    
       i = 1;
       
       e1 = CellList_[Cell].Edge(i  );
       
       NeighborCell = EdgeList_[e1].Cell(0) + EdgeList_[e1].Cell(1) - Cell; // neighbor cell

       // Update e1
       
       EdgeList_[e1].Cell(0) = NeighborCell;
       EdgeList_[e1].Cell(1) = CellList_[Cell].Child(0);

       // Update e2
           
       e2 = CellList_[Cell].Edge(i+4);
           
       // e2 exists, so neighbor was split already
           
       if ( e2 > 0 ) {
         
          NeighborCell = EdgeList_[e2].Cell(0) + EdgeList_[e2].Cell(1) - Cell; // neighbor cell

          // Lower mid node
          
          Node[5] = CellList_[NeighborCell].Node(3);
             
       }
       
       // e2 does not exist, so neighbor cell is not split yet...
       
       else {
          
          e2 = CellList_[Cell].Edge(i+4) = ++NumberOfEdges_;
          
          CellList_[Cell        ].Edge(        i  )  = e1;
          CellList_[Cell        ].Edge(        i+4)  = e2;        
             
          CellList_[NeighborCell].Edge(EdgeMap[i  ]) = e1;
          CellList_[NeighborCell].Edge(EdgeMap[i+4]) = e2;   

          // Lower mid node
          
          Node[5] = NewNode = ++NumberOfNodes_;
          
          NodeList_[NewNode].xyz(0) = 0.5*( xyz[1][0] + xyz[2][0] );
          NodeList_[NewNode].xyz(1) = 0.5*( xyz[1][1] + xyz[2][1] );
          NodeList_[NewNode].xyz(2) = 0.5*( xyz[1][2] + xyz[2][2] );
                    
       }
           
       EdgeList_[e2].Cell(0) = NeighborCell;
       EdgeList_[e2].Cell(1) = CellList_[Cell].Child(1);

    // Edges 2 and 6
    
       i = 2;
       
       e1 = CellList_[Cell].Edge(i  );
       
       NeighborCell = EdgeList_[e1].Cell(0) + EdgeList_[e1].Cell(1) - Cell; // neighbor cell

       // Update e1
       
       EdgeList_[e1].Cell(0) = NeighborCell;
       EdgeList_[e1].Cell(1) = CellList_[Cell].Child(1);

       // Update e2
           
       e2 = CellList_[Cell].Edge(i+4);
           
       // e2 exists, so neighbor was split already
           
       if ( e2 > 0 ) {
      
          NeighborCell = EdgeList_[e2].Cell(0) + EdgeList_[e2].Cell(1) - Cell; // neighbor cell
          
          // Right mid node
          
          Node[6] = CellList_[NeighborCell].Node(0);
                    
       }
       
       // e2 does not exist, so neighbor cell is not split yet...
       
       else {
          
          e2 = CellList_[Cell].Edge(i+4) = ++NumberOfEdges_;
          
          CellList_[Cell        ].Edge(        i  )  = e1;
          CellList_[Cell        ].Edge(        i+4)  = e2;        
             
          CellList_[NeighborCell].Edge(EdgeMap[i  ]) = e1;
          CellList_[NeighborCell].Edge(EdgeMap[i+4]) = e2;   

          // Right mid node
          
          Node[6] = NewNode = ++NumberOfNodes_;
          
          NodeList_[NewNode].xyz(0) = 0.5*( xyz[2][0] + xyz[3][0] );
          NodeList_[NewNode].xyz(1) = 0.5*( xyz[2][1] + xyz[3][1] );
          NodeList_[NewNode].xyz(2) = 0.5*( xyz[2][2] + xyz[3][2] );
           
       }
       
       EdgeList_[e2].Cell(0) = NeighborCell;
       EdgeList_[e2].Cell(1) = CellList_[Cell].Child(2);
  
    // Edges 3 and 7
    
       i = 3;
       
       e1 = CellList_[Cell].Edge(i  );
       
       NeighborCell = EdgeList_[e1].Cell(0) + EdgeList_[e1].Cell(1) - Cell; // neighbor cell

       // Update e1
       
       EdgeList_[e1].Cell(0) = NeighborCell;
       EdgeList_[e1].Cell(1) = CellList_[Cell].Child(2);

       // Update e2
           
       e2 = CellList_[Cell].Edge(i+4);
           
       // e2 exists, so neighbor was split already
           
       if ( e2 > 0 ) {
     
          NeighborCell = EdgeList_[e2].Cell(0) + EdgeList_[e2].Cell(1) - Cell; // neighbor cell
          
          // Top mid node
          
          Node[7] = CellList_[NeighborCell].Node(1);
                    
       }
       
       // e2 does not exist, so neighbor cell is not split yet...
       
       else {
          
          e2 = CellList_[Cell].Edge(i+4) = ++NumberOfEdges_;
          
          CellList_[Cell        ].Edge(        i  )  = e1;
          CellList_[Cell        ].Edge(        i+4)  = e2;        
             
          CellList_[NeighborCell].Edge(EdgeMap[i  ]) = e1;
          CellList_[NeighborCell].Edge(EdgeMap[i+4]) = e2;   

          // Top mid node
          
          Node[7] = NewNode = ++NumberOfNodes_;
          
          NodeList_[NewNode].xyz(0) = 0.5*( xyz[3][0] + xyz[4][0] );
          NodeList_[NewNode].xyz(1) = 0.5*( xyz[3][1] + xyz[4][1] );
          NodeList_[NewNode].xyz(2) = 0.5*( xyz[3][2] + xyz[4][2] ); 
                        
       }
          
       EdgeList_[e2].Cell(0) = NeighborCell;
       EdgeList_[e2].Cell(1) = CellList_[Cell].Child(3);
    
    // Local edge list numbering
    
    EdgeList[ 0] = CellList_[Cell].Edge(0);
    EdgeList[ 1] = CellList_[Cell].Edge(4);
    EdgeList[ 2] = CellList_[Cell].Edge(1);
    EdgeList[ 3] = CellList_[Cell].Edge(5);
              
    EdgeList[ 4] = CellList_[Cell].Edge(2);
    EdgeList[ 5] = CellList_[Cell].Edge(6);
    EdgeList[ 6] = CellList_[Cell].Edge(3);
    EdgeList[ 7] = CellList_[Cell].Edge(7);

    EdgeList[ 8] = ++NumberOfEdges_;
    EdgeList[ 9] = ++NumberOfEdges_;
    EdgeList[10] = ++NumberOfEdges_;
    EdgeList[11] = ++NumberOfEdges_;
    
    // Now the interior new edges
    
    EdgeList_[EdgeList[ 8]].Cell(0) = CellList_[Cell].Child(0);
    EdgeList_[EdgeList[ 8]].Cell(1) = CellList_[Cell].Child(3);

    EdgeList_[EdgeList[ 9]].Cell(0) = CellList_[Cell].Child(0);
    EdgeList_[EdgeList[ 9]].Cell(1) = CellList_[Cell].Child(1);
     
    EdgeList_[EdgeList[10]].Cell(0) = CellList_[Cell].Child(1);
    EdgeList_[EdgeList[10]].Cell(1) = CellList_[Cell].Child(2);  
    
    EdgeList_[EdgeList[11]].Cell(0) = CellList_[Cell].Child(2);
    EdgeList_[EdgeList[11]].Cell(1) = CellList_[Cell].Child(3);    
          
    // Child 0
    
    Child = CellList_[Cell].Child(0);
     
    CellList_[Child].Node(0) = Node[1];
    CellList_[Child].Node(1) = Node[5];
    CellList_[Child].Node(2) = Node[9];
    CellList_[Child].Node(3) = Node[8];

    CellList_[Child].Edge(0) = EdgeList[ 1];
    CellList_[Child].Edge(1) = EdgeList[ 2];
    CellList_[Child].Edge(2) = EdgeList[ 9];
    CellList_[Child].Edge(3) = EdgeList[ 8];   

    CellList_[Child].Child(0) = 0;
    CellList_[Child].Child(1) = 0;
    CellList_[Child].Child(2) = 0;
    CellList_[Child].Child(3) = 0;    
       
    CellList_[Child].Parent() = Cell;
    
    CellList_[Child].Level() = CellList_[Cell].Level() + 1;

    CellList_[Child].HasPoint() = 0;
    
    // Child 1
    
    Child = CellList_[Cell].Child(1);
    
    CellList_[Child].Node(0) = Node[5];
    CellList_[Child].Node(1) = Node[2];
    CellList_[Child].Node(2) = Node[6];
    CellList_[Child].Node(3) = Node[9];

    CellList_[Child].Edge(0) = EdgeList[ 9];
    CellList_[Child].Edge(1) = EdgeList[ 3];
    CellList_[Child].Edge(2) = EdgeList[ 4];
    CellList_[Child].Edge(3) = EdgeList[10];   

    
    CellList_[Child].Child(0) = 0;
    CellList_[Child].Child(1) = 0;
    CellList_[Child].Child(2) = 0;
    CellList_[Child].Child(3) = 0;              
    
    CellList_[Child].Parent() = Cell;    

    CellList_[Child].Level() = CellList_[Cell].Level() + 1;

    CellList_[Child].HasPoint() = 0;

    // Child 2
    
    Child = CellList_[Cell].Child(2);
   
    CellList_[Child].Node(0) = Node[9];
    CellList_[Child].Node(1) = Node[6];
    CellList_[Child].Node(2) = Node[3];
    CellList_[Child].Node(3) = Node[7];
    
    CellList_[Child].Edge(0) = EdgeList[11];
    CellList_[Child].Edge(1) = EdgeList[10];
    CellList_[Child].Edge(2) = EdgeList[ 5];
    CellList_[Child].Edge(3) = EdgeList[ 6];  
        
    CellList_[Child].Child(0) = 0;
    CellList_[Child].Child(1) = 0;
    CellList_[Child].Child(2) = 0;
    CellList_[Child].Child(3) = 0;    
 
    CellList_[Child].Parent() = Cell;    

    CellList_[Child].Level() = CellList_[Cell].Level() + 1;
    
    CellList_[Child].HasPoint() = 0;

    // Child 3
    
    Child = CellList_[Cell].Child(3);
    
    CellList_[Child].Node(0) = Node[8];
    CellList_[Child].Node(1) = Node[9];
    CellList_[Child].Node(2) = Node[7];
    CellList_[Child].Node(3) = Node[4];

    CellList_[Child].Edge(0) = EdgeList[ 0];
    CellList_[Child].Edge(1) = EdgeList[ 8];
    CellList_[Child].Edge(2) = EdgeList[11];
    CellList_[Child].Edge(3) = EdgeList[ 7];  
        
    CellList_[Child].Child(0) = 0;
    CellList_[Child].Child(1) = 0;
    CellList_[Child].Child(2) = 0;
    CellList_[Child].Child(3) = 0;    
        
    CellList_[Child].Parent() = Cell;    

    CellList_[Child].Level() = CellList_[Cell].Level() + 1;
    
    CellList_[Child].HasPoint() = 0;
    
    // If parent has point inserted, we need to re-insert into one of the children
    
    if ( CellList_[Cell].HasPoint() ) {
       
       i = 0;
       
       Inserted = 0;
       
       while ( i <= 3 && !Inserted ) {
          
          Child = CellList_[Cell].Child(i);
          
          if ( InsideQuad(Child,CellList_[Cell].InsertedPoint()) ) {

             CellList_[Child].HasPoint() = 1;
             
             CellList_[Child].SurfaceEdge() = CellList_[Cell].SurfaceEdge();
             
             CellList_[Cell].HasPoint() = 0;
             
             CellList_[Cell].SurfaceEdge() = 0;
                          
             CellList_[Child].InsertedPoint(0) = CellList_[Cell].InsertedPoint(0);
             CellList_[Child].InsertedPoint(1) = CellList_[Cell].InsertedPoint(1);
             CellList_[Child].InsertedPoint(2) = CellList_[Cell].InsertedPoint(2);
             
             Inserted = 1;
             
          }
          
          i++;
          
       }
       
       if ( !Inserted ) {
          
          printf("wtf ... not inserted int one of the children! \n");fflush(NULL);exit(1);
          
       }
       
    }

}


/*##############################################################################
#                                                                              #
#                     QUAD_TREE WriteQuadTreeToFile                            #
#                                                                              #
##############################################################################*/

void QUAD_TREE::WriteQuadTreeToFile(FILE *QuadFile)
{
  
    int i, j, NumberOfUsedNodes, NumberOfUsedCells, InsideBody, *NodeIsUsed;
    VSPAERO_DOUBLE xyz[3];
   
    // Determine number of used cells
    
    NodeIsUsed = new int[NumberOfNodes_ + 1];
    
    zero_int_array(NodeIsUsed, NumberOfNodes_);
    
    NumberOfUsedCells = 0;
        
    for ( i = 1 ; i <= NumberOfCells_ ; i++ ) {
       
       if ( CellList_[i].Child(0) == 0 ) {
          
          InsideBody = NodeList_[CellList_[i].Node(0)].InsideBody()
                     + NodeList_[CellList_[i].Node(1)].InsideBody()
                     + NodeList_[CellList_[i].Node(2)].InsideBody()
                     + NodeList_[CellList_[i].Node(3)].InsideBody();
                      
          if ( InsideBody == 0 ) {
          
             NumberOfUsedCells++;
             
             NodeIsUsed[CellList_[i].Node(0)] = 1;
             NodeIsUsed[CellList_[i].Node(1)] = 1;
             NodeIsUsed[CellList_[i].Node(2)] = 1;
             NodeIsUsed[CellList_[i].Node(3)] = 1;             
             
          }
          
       }
       
    }

    NumberOfUsedNodes = 0;
    
    for ( i = 1 ; i <= NumberOfNodes_ ; i++ ) {
       
       if ( NodeIsUsed[i] ) NumberOfUsedNodes++;

    }
    
    FPRINTF(QuadFile,"%d %d \n",NumberOfUsedNodes, NumberOfUsedCells);
    
    j = 0;
    
    for ( i = 1 ; i <= NumberOfNodes_ ; i++ ) {
       
       if ( NodeIsUsed[i] ) {
          
          NodeIsUsed[i] = ++j;
             
          xyz[0] = NodeList_[i].xyz(DirMap_[0]);
          xyz[1] = NodeList_[i].xyz(DirMap_[1]);
          xyz[2] = NodeList_[i].xyz(DirMap_[2]);
          
          FPRINTF(QuadFile,"%d %lf %lf %lf %lf %lf %lf %lf \n",
          j,
          xyz[0],
          xyz[1],
          xyz[2],
          NodeList_[i].velocity(0),
          NodeList_[i].velocity(1),
          NodeList_[i].velocity(2),
          NodeList_[i].Cp());
          
       }

    }
    
    j = 0;

    for ( i = 1 ; i <= NumberOfCells_ ; i++ ) {
       
       if ( CellList_[i].Child(0) == 0 ) {
          
          InsideBody = NodeList_[CellList_[i].Node(0)].InsideBody()
                     + NodeList_[CellList_[i].Node(1)].InsideBody()
                     + NodeList_[CellList_[i].Node(2)].InsideBody()
                     + NodeList_[CellList_[i].Node(3)].InsideBody();
                      
          if ( InsideBody == 0 ) {
                       
             j++;
   
             FPRINTF(QuadFile,"%d %d %d %d %d \n",
             j,
             NodeIsUsed[CellList_[i].Node(0)],
             NodeIsUsed[CellList_[i].Node(1)],
             NodeIsUsed[CellList_[i].Node(2)],
             NodeIsUsed[CellList_[i].Node(3)]);
             
          }
          
       }
         
    }
    
    delete NodeIsUsed;

}

#include "END_NAME_SPACE.H"











