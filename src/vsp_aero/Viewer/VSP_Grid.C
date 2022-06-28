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

    NumberOfTris_ = 0;

    TriList_ = NULL;
    
    NumberOfEdges_ = 0;
    
    EdgeList_ = NULL;
    
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

    NumberOfTris_ = NumTris;

    TriList_ = new VSP_TRIS[NumberOfTris_ + 1];

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
#                                 VSP_GRID Copy                                #
#                                                                              #
##############################################################################*/

VSP_GRID::VSP_GRID(const VSP_GRID &VSPGrid)
{

    printf("Copy not implemented for VSP_GRID! \n");fflush(NULL);

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

    NumberOfTris_ = 0;

    if ( TriList_ != NULL ) delete [] TriList_;

    NumberOfEdges_ = 0;

    if ( EdgeList_ != NULL ) delete [] EdgeList_;
     
}

/*##############################################################################
#                                                                              #
#                   VSP_GRID CalculateNormalsAndCentroids                      #
#                                                                              #
##############################################################################*/

void VSP_GRID::CalculateNormalsAndCentroids(void)
{
 
    int i, Node1, Node2, Node3;
    double vec1[3], vec2[3], vec3[3], mag;
    
    for ( i = 1 ; i <= NumberOfTris_ ; i++ ) {
     
       Node1 = TriList(i).Node1();
       Node2 = TriList(i).Node2();
       Node3 = TriList(i).Node3();
       
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
       
       if ( mag <= 0. ) printf("Mag: %lf \n",mag);
       
       TriList(i).Nx() = vec3[0];
       TriList(i).Ny() = vec3[1];
       TriList(i).Nz() = vec3[2];
       
       TriList(i).Area() = 0.5*mag;

       // Centroid
       
       TriList(i).Xc() = ( NodeList(Node1).x() + NodeList(Node2).x() + NodeList(Node3).x() )/3.;
       TriList(i).Yc() = ( NodeList(Node1).y() + NodeList(Node2).y() + NodeList(Node3).y() )/3.;
       TriList(i).Zc() = ( NodeList(Node1).z() + NodeList(Node2).z() + NodeList(Node3).z() )/3.;
       
    }
    fflush(NULL);
}

/*##############################################################################
#                                                                              #
#                           VSP_GRID CreateTriEdges                            #
#                                                                              #
##############################################################################*/

void VSP_GRID::CreateTriEdges(void)
{

    int i, j, k, nod1, nod2, noda, nodb, start_edge;
    int level, edge_to_node[4][3], nod_list[4];
    int max_edge, new_edge, *jump_pnt, Error;
    EDGE_ENTRY *list, *tlist;

    if ( Verbose_ ) printf("Finding tri edges... \n");

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

    edge_to_node[3][1] = 1;
    edge_to_node[3][2] = 3;

    // Loop over triangles and create the edge list

    for ( j = 1 ; j <= NumberOfTris() ; j++ ) {

       // Local copy of node pointers

       nod_list[1] = TriList(j).Node1();
       nod_list[2] = TriList(j).Node2();
       nod_list[3] = TriList(j).Node3();

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

                if ( i == 1 ) TriList(j).Edge1() =  new_edge;
                if ( i == 2 ) TriList(j).Edge2() =  new_edge;
                if ( i == 3 ) TriList(j).Edge3() =  new_edge;

             }

             else {

                if ( i == 1 ) TriList(j).Edge1() = -new_edge;
                if ( i == 2 ) TriList(j).Edge2() = -new_edge;
                if ( i == 3 ) TriList(j).Edge3() = -new_edge;

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

                   if ( i == 1 ) TriList(j).Edge1() =  level;
                   if ( i == 2 ) TriList(j).Edge2() =  level;
                   if ( i == 3 ) TriList(j).Edge3() =  level;

                }

                else {

                   if ( i == 1 ) TriList(j).Edge1() = -level;
                   if ( i == 2 ) TriList(j).Edge2() = -level;
                   if ( i == 3 ) TriList(j).Edge3() = -level;

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

                   if ( i == 1 ) TriList(j).Edge1() =  new_edge;
                   if ( i == 2 ) TriList(j).Edge2() =  new_edge;
                   if ( i == 3 ) TriList(j).Edge3() =  new_edge;

                }

                else {

                   if ( i == 1 ) TriList(j).Edge1() = -new_edge;
                   if ( i == 2 ) TriList(j).Edge2() = -new_edge;
                   if ( i == 3 ) TriList(j).Edge3() = -new_edge;

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

             // This edge only has one tri attached... 
             
             if ( list[level].tri_2 == 0 ) {

                Error = 1;

             }

             level = list[level].next;

          }

       }

    }

    // Store edge to tri pointers

    for ( i = 1 ; i <= NumberOfNodes() ; i++ ) {

       level = jump_pnt[i];

       while ( level != 0 ) {

          if ( list[level].tri_1 != 0 ) {

             if ( list[level].tri_2 != 0) {

                // Store pointers to tris

                EdgeList(level).Tri1() = list[level].tri_1;
                EdgeList(level).Tri2() = list[level].tri_2;

             }

             else {

                EdgeList(level).Tri1() = list[level].tri_1;
                EdgeList(level).Tri2() = list[level].tri_1;


             }

          }

          level = list[level].next;

       }

    }

    if ( Verbose_ ) printf("Number of nodes: %d \n",NumberOfNodes());
    if ( Verbose_ ) printf("Number of tris: %d \n",NumberOfTris());
    if ( Verbose_ ) printf("Number of edges is: %d \n",NumberOfEdges());

    // Free up the scratch space

    delete [] jump_pnt;
    delete [] list;

    for ( j = 1 ; j <= NumberOfTris() ; j++ ) {

       TriList(j).Edge1() = ABS(TriList(j).Edge1());
       TriList(j).Edge2() = ABS(TriList(j).Edge2());
       TriList(j).Edge3() = ABS(TriList(j).Edge3());
  
    }
    
    for ( j = 1 ; j <= NumberOfEdges() ; j++ ) {
     
       EdgeList(j).IsTrailingEdge() = 0;
       EdgeList(j).IsLeadingEdge() = 0;
        
       if ( EdgeList(j).Tri2() == EdgeList(j).Tri1() ) {
   
          if ( NodeList(EdgeList(j).Node1()).IsTrailingEdgeNode() &&
               NodeList(EdgeList(j).Node2()).IsTrailingEdgeNode()     ) {
      
             EdgeList(j).IsTrailingEdge() = 1;
             
          }
          
          if ( NodeList(EdgeList(j).Node1()).IsLeadingEdgeNode() &&
               NodeList(EdgeList(j).Node2()).IsLeadingEdgeNode()     ) {
      
             EdgeList(j).IsLeadingEdge() = 1;
             
          }          
          
       }
       
    }
    fflush(NULL);
}

/*##############################################################################
#                                                                              #
#                           VSP_GRID DetermineUpwindEdges                      #
#                                                                              #
##############################################################################*/

void VSP_GRID::CalculateUpwindEdges(void)
{
 
   int j, Node1, Node2, Node3, TotalUpwind;
   double xVec[3], Vec[3], Flux[3], TotalFlux, TriNormal[3], Normal[3], Mag;
   
   // Loop over triangles and determine which nodes, and then edges are upwind
   
   xVec[0] = 1.;
   xVec[1] = 0.;
   xVec[2] = 0.;
   
   for ( j = 1 ; j <= NumberOfTris() ; j++ ) {

       TriList(j).Edge1() = ABS(TriList(j).Edge1());
       TriList(j).Edge2() = ABS(TriList(j).Edge2());
       TriList(j).Edge3() = ABS(TriList(j).Edge3());
      
       Node1 = TriList(j).Node1();
       Node2 = TriList(j).Node2();
       Node3 = TriList(j).Node3();
       
       TriNormal[0] = TriList(j).Nx();
       TriNormal[1] = TriList(j).Ny();
       TriNormal[2] = TriList(j).Nz();
     
       // Edge 1
       
       Vec[0] = NodeList(Node2).x() - NodeList(Node1).x();
       Vec[1] = NodeList(Node2).y() - NodeList(Node1).y();
       Vec[2] = NodeList(Node2).z() - NodeList(Node1).z();
       
       Mag = sqrt(vector_dot(Vec,Vec));
       
       Vec[0] /= Mag;
       Vec[1] /= Mag;
       Vec[2] /= Mag;
       
       vector_cross(Vec,TriNormal,Normal);
       
       Flux[0] = MAX(-vector_dot(Normal, xVec),0.);
       
       // Edge 2
       
       Vec[0] = NodeList(Node3).x() - NodeList(Node2).x();
       Vec[1] = NodeList(Node3).y() - NodeList(Node2).y();
       Vec[2] = NodeList(Node3).z() - NodeList(Node2).z();
       
       Mag = sqrt(vector_dot(Vec,Vec));
       
       Vec[0] /= Mag;
       Vec[1] /= Mag;
       Vec[2] /= Mag;
       
       vector_cross(Vec,TriNormal,Normal);
       
       Flux[1] = MAX(-vector_dot(Normal, xVec),0.);
 
       // Edge 3
       
       Vec[0] = NodeList(Node1).x() - NodeList(Node3).x();
       Vec[1] = NodeList(Node1).y() - NodeList(Node3).y();
       Vec[2] = NodeList(Node1).z() - NodeList(Node3).z();
       
       Mag = sqrt(vector_dot(Vec,Vec));
       
       Vec[0] /= Mag;
       Vec[1] /= Mag;
       Vec[2] /= Mag;
       
       vector_cross(Vec,TriNormal,Normal);
       
       Flux[2] = MAX(-vector_dot(Normal, xVec),0.);       
       
       // Create weighting
       
       TotalFlux = Flux[0] + Flux[1] + Flux[2];
       
       Flux[0] /= TotalFlux;
       Flux[1] /= TotalFlux;
       Flux[2] /= TotalFlux;
       
       // Determine which edges are upwind
       
       TotalUpwind = 0;
       
       if ( Flux[0] > 0. )  { TotalUpwind++ ; TriList(j).Edge1IsUpWind() = 1 ; TriList(j).Edge1UpwindWeight() = Flux[0]; };
       if ( Flux[1] > 0. )  { TotalUpwind++ ; TriList(j).Edge2IsUpWind() = 1 ; TriList(j).Edge2UpwindWeight() = Flux[1]; };
       if ( Flux[2] > 0. )  { TotalUpwind++ ; TriList(j).Edge3IsUpWind() = 1 ; TriList(j).Edge3UpwindWeight() = Flux[2]; };
       
       if ( TotalUpwind == 0 ) {
        
          printf("wtf! \n");
       
          printf("TotalUpwind: %d \n",TotalUpwind);fflush(NULL);
        
//          exit(1);
        
       }
       
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

       printf("Could not open %s mesh file for write... \n", FileName);fflush(NULL);

       exit(1);

    }   
    
    // Header
    
    fprintf(MeshFile,"%d %d %d \n", NumberOfNodes_, NumberOfTris_, 0); 
    
    // XYZ data
    
    for ( i = 1 ; i <= NumberOfNodes_ ; i++ ) {

       fprintf(MeshFile,"%lf \n",NodeList(i).x());
       
    }    

    for ( i = 1 ; i <= NumberOfNodes_ ; i++ ) {

       fprintf(MeshFile,"%lf \n",NodeList(i).y());
       
    }    
    
    for ( i = 1 ; i <= NumberOfNodes_ ; i++ ) {

       fprintf(MeshFile,"%lf \n",NodeList(i).z());
       
    }    
    
    // Connectivity
    
    for ( i = 1 ; i <= NumberOfTris_ ; i++ ) {

      fprintf(MeshFile,"%d %d %d \n",
              TriList(i).Node1(),
              TriList(i).Node2(),
              TriList(i).Node3());
       
    }      

    for ( i = 1 ; i <= NumberOfTris_ ; i++ ) {

      fprintf(MeshFile,"%d %d \n", TriList(i).SurfaceID(), TriList(i).SurfaceID());
       
    }

}

