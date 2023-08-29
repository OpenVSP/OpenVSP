//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "FastMatrix.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                          FAST_MATRIX Constructor                             #
#                                                                              #
##############################################################################*/

FAST_MATRIX::FAST_MATRIX(void)
{

    // Forward
    
    NumberOfForwardInteractionLoops_[0] = 0;
    NumberOfForwardInteractionLoops_[1] = 0;

    ForwardInteractionList_[0] = NULL;
    ForwardInteractionList_[1] = NULL;
    
    // Adjoint

    NumberOfAdjointInteractionLoops_[0] = 0;
    NumberOfAdjointInteractionLoops_[1] = 0;

    AdjointInteractionList_[0] = NULL;
    AdjointInteractionList_[1] = NULL;

}

/*##############################################################################
#                                                                              #
#                                  FAST_MATRIX Destructor                      #
#                                                                              #
##############################################################################*/

FAST_MATRIX::~FAST_MATRIX(void)
{

    if ( NumberOfForwardInteractionLoops_[0] != 0 ) delete [] ForwardInteractionList_[0];
    if ( NumberOfForwardInteractionLoops_[1] != 0 ) delete [] ForwardInteractionList_[1];

    if ( NumberOfAdjointInteractionLoops_[0] != 0 ) delete [] AdjointInteractionList_[0];
    if ( NumberOfAdjointInteractionLoops_[1] != 0 ) delete [] AdjointInteractionList_[1];
            
}

/*##############################################################################
#                                                                              #
#                              FAST_MATRIX SizeForwardList                     #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::SizeForwardList(int LoopType, int NumberOfLoops)
{

    if ( MaxNumberOfForwardInteractionLoops_[LoopType] > 0 ) {
       
       delete [] ForwardInteractionList_[LoopType];
       
       MaxNumberOfForwardInteractionLoops_[LoopType] = 0;
       
          NumberOfForwardInteractionLoops_[LoopType] = 0;
       
    }
              
    MaxNumberOfForwardInteractionLoops_[LoopType] = NumberOfLoops;
              
       NumberOfForwardInteractionLoops_[LoopType] = NumberOfLoops;
    
    ForwardInteractionList_[LoopType] = new LOOP_INTERACTION_ENTRY[NumberOfLoops + 1];

}

/*##############################################################################
#                                                                              #
#                              FAST_MATRIX SizeAdjointList                     #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::SizeAdjointList(int LoopType, int NumberOfLoops)
{

    if ( MaxNumberOfAdjointInteractionLoops_[LoopType] > 0 ) {
       
       delete [] AdjointInteractionList_[LoopType];
       
       MaxNumberOfAdjointInteractionLoops_[LoopType] = 0;
       
          NumberOfAdjointInteractionLoops_[LoopType] = 0;
       
    }
    
    MaxNumberOfAdjointInteractionLoops_[LoopType] = NumberOfLoops;
    
       NumberOfAdjointInteractionLoops_[LoopType] = NumberOfLoops;
    
    AdjointInteractionList_[LoopType] = new LOOP_INTERACTION_ENTRY[NumberOfLoops + 1];

}

/*##############################################################################
#                                                                              #
#                               FAST_MATRIX DeleteForwardList                  #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::DeleteForwardList(int LoopType)
{

    if ( NumberOfForwardInteractionLoops_[LoopType] != 0 ) delete [] ForwardInteractionList_[LoopType];

    NumberOfForwardInteractionLoops_[LoopType] = 0;
    
}

/*##############################################################################
#                                                                              #
#                               FAST_MATRIX DeleteAdjointList                  #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::DeleteAdjointList(int LoopType)
{

    if ( NumberOfAdjointInteractionLoops_[LoopType] != 0 ) delete [] AdjointInteractionList_[LoopType];

    NumberOfAdjointInteractionLoops_[LoopType] = 0;
    
}

/*##############################################################################
#                                                                              #
#                                 FAST_MATRIX Copy                             #
#                                                                              #
##############################################################################*/

FAST_MATRIX::FAST_MATRIX(const FAST_MATRIX &FastMatrix)
{

    *this = FastMatrix;

}

/*##############################################################################
#                                                                              #
#                           FAST_MATRIX Operator =                             #
#                                                                              #
##############################################################################*/

FAST_MATRIX &FAST_MATRIX::operator=(const FAST_MATRIX &FastMatrix)
{

    int i, LoopType;
    
    for ( LoopType = 0 ; LoopType <= 1 ; LoopType++ ) {
       
       // Forward
       
       for ( i = 1 ; i <= NumberOfForwardInteractionLoops_[LoopType] ; i++ ) {
          
          ForwardInteractionList_[LoopType][i] = FastMatrix.ForwardInteractionList_[LoopType][i];
          
       }
       
       // Adjoint
       
       for ( i = 1 ; i <= NumberOfAdjointInteractionLoops_[LoopType] ; i++ ) {
          
          AdjointInteractionList_[LoopType][i] = FastMatrix.AdjointInteractionList_[LoopType][i];
          
       }       
       
    } 
 
    return *this;

}

/*##############################################################################
#                                                                              #
#                            FAST_MATRIX UseForwardList                        #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::UseForwardList(int LoopType, int NumberOfLoops, LOOP_INTERACTION_ENTRY *List)
{

    MaxNumberOfForwardInteractionLoops_[LoopType] = NumberOfLoops;

       NumberOfForwardInteractionLoops_[LoopType] = NumberOfLoops;
    
    ForwardInteractionList_[LoopType] = List;
    
}

/*##############################################################################
#                                                                              #
#                            FAST_MATRIX UseAdjointList                        #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::UseAdjointList(int LoopType, int NumberOfLoops, LOOP_INTERACTION_ENTRY *List)
{

    MaxNumberOfAdjointInteractionLoops_[LoopType] = NumberOfLoops;

       NumberOfAdjointInteractionLoops_[LoopType] = NumberOfLoops;
    
    AdjointInteractionList_[LoopType] = List;
    
}

/*##############################################################################
#                                                                              #
#                          FAST_MATRIX MergeForwardList                        #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::MergeForwardList(int NumberOfThreads, int MGLevel, int LoopType, VSP_GEOM &VSPGeom, VSPAERO_DOUBLE Mach, VSPAERO_DOUBLE FarAway)
{

    int i, j, k, p, cpu, Level, Loop, NumberOfEdges, NumberOfLoops, CurrentLoop;
    int TestEdge, MaxInteractionEdges, LoopOffSet, InteractionType;
    int Done, Found, TotalFound, CommonEdges, MaxLevels, **EdgeIsCommon, NumberOfVortexLoops;
    LOOP_ENTRY **CommonEdgeList;
    VSP_EDGE **TempEdgeInteractionList;
    VSPAERO_DOUBLE xyz[3], Vec[3], Distance, Test;

    long long int NewHits;

    NumberOfVortexLoops = VSPGeom.Grid(MGLevel).NumberOfLoops();

    // Allocate space for final interaction lists

    MaxInteractionEdges = 0;

    for ( Level = 1 ; Level <= VSPGeom.NumberOfGridLevels() ; Level++ ) {

       MaxInteractionEdges += VSPGeom.Grid(Level).NumberOfEdges();
       
    }
    
    CommonEdgeList = new LOOP_ENTRY*[NumberOfThreads + 1];
    
    EdgeIsCommon = new int*[NumberOfThreads + 1];

    for ( cpu = 0 ; cpu < NumberOfThreads ; cpu++ ) {
       
       CommonEdgeList[cpu] = new LOOP_ENTRY[NumberOfVortexLoops + 1];
       
       EdgeIsCommon[cpu] = new int[MaxInteractionEdges + 1];
       
       zero_int_array(EdgeIsCommon[cpu], MaxInteractionEdges);
       
    }
    
    MaxLevels = VSPGeom.NumberOfGridLevels();

    NewHits = 0;
 
    LoopOffSet = 0;
    
    for ( Level = MGLevel + 1 ; Level <= MaxLevels ; Level++ ) {

       if ( LoopType == FIXED_LOOPS ) PRINTF("Working on level %d of %d \r",Level,MaxLevels);fflush(NULL);
#ifndef AUTODIFF
#pragma omp parallel for reduction(+:NewHits) private(cpu,CurrentLoop,i,j,CommonEdges,TestEdge,TotalFound,Found,Done,xyz,Vec,Test,Distance,k,p,NumberOfEdges,TempEdgeInteractionList) schedule(dynamic)
#endif
       for ( Loop = 1 ; Loop <= VSPGeom.Grid(Level).NumberOfLoops() ; Loop++ ) {

#ifndef AUTODIFF
       
#ifdef VSPAERO_OPENMP    
          cpu = omp_get_thread_num();
#else
          cpu = 0;
#endif         

#else
          cpu = 0;
#endif

          CurrentLoop = NumberOfForwardInteractionLoops(LoopType) + Loop;
       
          ForwardInteractionList(LoopType)[CurrentLoop].Level() = Level;
          
          ForwardInteractionList(LoopType)[CurrentLoop].Loop() = Loop;

          // Find common part of lists
          
          for ( i = 1 ; i <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
             
             j = VSPGeom.Grid(Level).LoopList(Loop).FineGridLoop(i) + LoopOffSet;
             
             CommonEdgeList[cpu][i].NextEdge = 1;
     
             CommonEdgeList[cpu][i].Edge = ForwardInteractionList(LoopType)[j].SurfaceVortexEdgeInteractionList();
             
             CommonEdgeList[cpu][i].NumberOfVortexEdges = ForwardInteractionList(LoopType)[j].NumberOfVortexEdges();
             
          }

          CommonEdges = 0;
              
          while ( CommonEdgeList[cpu][1].NextEdge <= CommonEdgeList[cpu][1].NumberOfVortexEdges ) {
   
             TestEdge = CommonEdgeList[cpu][1].Edge[CommonEdgeList[cpu][1].NextEdge]->VortexEdge();
             
             TotalFound = 0;
             
             for ( i = 2 ; i <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
      
                Found = Done = 0;
                
                while ( CommonEdgeList[cpu][i].NextEdge <= CommonEdgeList[cpu][i].NumberOfVortexEdges && !Found && !Done ) {

                   // Found a common source loop
                   
                   if ( TestEdge == CommonEdgeList[cpu][i].Edge[CommonEdgeList[cpu][i].NextEdge]->VortexEdge() ) {
                      
                      Found = 1;
                      
                      CommonEdgeList[cpu][i].NextEdge++;
                      
                   }
               
                   // Not on this level
                   
                   else if ( CommonEdgeList[cpu][i].Edge[CommonEdgeList[cpu][i].NextEdge]->VortexEdge() < TestEdge ) {
                      
                      CommonEdgeList[cpu][i].NextEdge++;
                      
                   }
                   
                   // Nothing else to check
                   
                   else {
                      
                      Done = 1;
                      
                   }
                   
                }
                
                if ( Found ) TotalFound++;
              
             }
         
             if ( TotalFound > 0 && TotalFound == VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() - 1 ) {

                // Mid point of common source edge
                
                xyz[0] = CommonEdgeList[cpu][1].Edge[CommonEdgeList[cpu][1].NextEdge]->Xc();
                xyz[1] = CommonEdgeList[cpu][1].Edge[CommonEdgeList[cpu][1].NextEdge]->Yc();
                xyz[2] = CommonEdgeList[cpu][1].Edge[CommonEdgeList[cpu][1].NextEdge]->Zc();
                
                // Centroid of agglomerated loop for all the fine grid loops
                
                Vec[0] = xyz[0] - VSPGeom.Grid(Level).LoopList(Loop).Xc();
                Vec[1] = xyz[1] - VSPGeom.Grid(Level).LoopList(Loop).Yc();
                Vec[2] = xyz[2] - VSPGeom.Grid(Level).LoopList(Loop).Zc();

                if ( Mach > 1. ) Vec[0] /= (Mach*Mach - 1.);
                      
                // Distance from centroid to source loop
                
                Distance = sqrt( SQR(Vec[0]) + SQR(Vec[1]) + SQR(Vec[2]) );
                
                // Find maximum distance from agglomerated loop centroid to fine grid loop centroids

                Test = 0.;
                
                for ( i = 1 ; i <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
                
                    // Centroid of agglomerated loop for all the fine grid loops
                    
                    Vec[0] = VSPGeom.Grid(Level).LoopList(Loop).Xc();
                    Vec[1] = VSPGeom.Grid(Level).LoopList(Loop).Yc();
                    Vec[2] = VSPGeom.Grid(Level).LoopList(Loop).Zc();
                
                    // Centroid of the j'th fine grid loop
                    
                    j = VSPGeom.Grid(Level).LoopList(Loop).FineGridLoop(i);
                
                    // Distance between fine grid and agglomerated grid centroids
                    
                    Vec[0] -= VSPGeom.Grid(Level-1).LoopList(j).Xc();
                    Vec[1] -= VSPGeom.Grid(Level-1).LoopList(j).Yc();
                    Vec[2] -= VSPGeom.Grid(Level-1).LoopList(j).Zc();
                
                    Test = MAX(Test,sqrt( SQR(Vec[0]) + SQR(Vec[1]) + SQR(Vec[2]) ));
                    
                }
              
                Test = VSPGeom.Grid(Level).LoopList(Loop).Length() + VSPGeom.Grid(Level).LoopList(Loop).CentroidOffSet();
            
                Test *= FarAway;
                
                if ( Test <= Distance ) {
 
                   CommonEdges++;

                   EdgeIsCommon[cpu][CommonEdgeList[cpu][1].Edge[CommonEdgeList[cpu][1].NextEdge]->VortexEdge()] = 1;
                  
                }
               
             }
       
             CommonEdgeList[cpu][1].NextEdge++;
    
          }
    
          // Create a list of the common edges, and trim up the remaining edge lists
         
          if ( CommonEdges > 0 ) {
 
             // Create the common list
             
             ForwardInteractionList(LoopType)[CurrentLoop].Level() = Level;
             
             ForwardInteractionList(LoopType)[CurrentLoop].Loop() = Loop;
     
             ForwardInteractionList(LoopType)[CurrentLoop].SizeEdgeList(CommonEdges);
             
             i = 1;
             
             j = 0;
        
             while ( j < CommonEdges && i <= CommonEdgeList[cpu][1].NumberOfVortexEdges ) {
         
               if ( EdgeIsCommon[cpu][CommonEdgeList[cpu][1].Edge[i]->VortexEdge()] == 1 ) {
              
                    ForwardInteractionList(LoopType)[CurrentLoop].SurfaceVortexEdgeInteractionList()[++j] = CommonEdgeList[cpu][1].Edge[i];
                    
                }
                
                i++;
                
             }
             
             // Trim out the common edges from the initial lists
             
             for ( i = 1 ; i <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
                
                j = VSPGeom.Grid(Level).LoopList(Loop).FineGridLoop(i) + LoopOffSet;
                
                NumberOfEdges = ForwardInteractionList(LoopType)[j].NumberOfVortexEdges() - CommonEdges;
              
                // There are non-common edges remaining
                
                if ( NumberOfEdges > 0 ) {
                   
                   TempEdgeInteractionList = new VSP_EDGE*[NumberOfEdges + 1];
       
                   k = 0;
             
                   p = 1;
              
                   while ( k < NumberOfEdges && p <= ForwardInteractionList(LoopType)[j].NumberOfVortexEdges() ) {

                      if ( EdgeIsCommon[cpu][ForwardInteractionList(LoopType)[j].SurfaceVortexEdgeInteractionList(p)->VortexEdge()] == 0 ) {
 
                         TempEdgeInteractionList[++k] = ForwardInteractionList(LoopType)[j].SurfaceVortexEdgeInteractionList(p);
                         
                      }
                      
                      p++;

                   }
 
                   ForwardInteractionList(LoopType)[j].UseEdgeList(NumberOfEdges, TempEdgeInteractionList);
 
                }
                
                // There are no non-common edges remaining...
                
                else {

                   ForwardInteractionList(LoopType)[j].Level() = 0;
                   
                   ForwardInteractionList(LoopType)[j].Loop() = 0;

                   ForwardInteractionList(LoopType)[j].DeleteEdgeList();
                 
                }
              
             }
                   
             // Unmark the common edges
             
             for ( j = 1 ; j <= ForwardInteractionList(LoopType)[CurrentLoop].NumberOfVortexEdges() ; j++ ) {

                 EdgeIsCommon[cpu][ABS(ForwardInteractionList(LoopType)[CurrentLoop].SurfaceVortexEdgeInteractionList(j)->VortexEdge())] = 0;
   
             }

          }
     
          for ( i = 1 ; i <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {

             NewHits += CommonEdgeList[cpu][i].NumberOfVortexEdges - CommonEdges;
             
          }
          
          if ( Level == MaxLevels ) NewHits += CommonEdges;
        
       }
 
       NumberOfForwardInteractionLoops(LoopType) += VSPGeom.Grid(Level).NumberOfLoops();
       
       LoopOffSet += VSPGeom.Grid(Level-1).NumberOfLoops();
       
    }
    
    ForwardSpeedRatio_ *= (long double) ForwardTotalHits_ / (double) NewHits;

    for ( cpu = 0 ; cpu < NumberOfThreads ; cpu++ ) {
       
       delete [] CommonEdgeList[cpu];
       
    }
    
    delete [] CommonEdgeList;

    for ( cpu = 0 ; cpu < NumberOfThreads ; cpu++ ) {
       
       delete [] EdgeIsCommon[cpu];
       
    }
    
    delete [] EdgeIsCommon;
        
    // Delete any zero length lists
    
    int NumberOfActualLoops = 0;

    for ( i = 1 ; i <= NumberOfForwardInteractionLoops(LoopType) ; i++ ) {
       
       if ( ForwardInteractionList(LoopType)[i].NumberOfVortexEdges() > 0 ) {
          
          NumberOfActualLoops++;
          
       }
       
    }

    LOOP_INTERACTION_ENTRY *TempList;
    
    TempList = new LOOP_INTERACTION_ENTRY[NumberOfActualLoops + 1];
    
    j = 0;

    for ( i = 1 ; i <= NumberOfForwardInteractionLoops(LoopType) ; i++ ) {
       
       if ( ForwardInteractionList(LoopType)[i].NumberOfVortexEdges() > 0 ) {
          
          TempList[++j] = ForwardInteractionList(LoopType)[i];
          
       }
       
    }
    
    if ( j != NumberOfActualLoops ) {
       
       PRINTF("Error in cleaning up interaction list! \n"); fflush(NULL);
       exit(1);
       
    }

    DeleteForwardList(LoopType);

    UseForwardList(LoopType, NumberOfActualLoops, TempList);

}

/*##############################################################################
#                                                                              #
#                          FAST_MATRIX MergeAdjointList                        #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::MergeAdjointList(int NumberOfThreads, int MGLevel, int LoopType, VSP_GEOM &VSPGeom, VSPAERO_DOUBLE Mach, VSPAERO_DOUBLE FarAway)
{

    int i, j, k, p, cpu, Level, Loop, NumberOfLoops, CurrentLoop;
    int TestLoop, MaxInteractionLoops, LoopOffSet, InteractionType;
    int Done, Found, TotalFound, CommonLoops, MaxLevels, **LoopIsCommon, NumberOfVortexLoops;
    LOOP_ENTRY **CommonLoopList;
    VSP_LOOP **TempLoopInteractionList;
    VSPAERO_DOUBLE xyz[3], Vec[3], Distance, Test;

    long long int NewHits;

    NumberOfVortexLoops = VSPGeom.Grid(MGLevel).NumberOfLoops();

    // Allocate space for final interaction lists

    MaxInteractionLoops = 0;
    
    for ( Level = 1 ; Level <= VSPGeom.NumberOfGridLevels() ; Level++ ) {
       
       MaxInteractionLoops += VSPGeom.Grid(Level).NumberOfLoops();
       
    }

    CommonLoopList = new LOOP_ENTRY*[NumberOfThreads + 1];
    
    LoopIsCommon = new int*[NumberOfThreads + 1];

    for ( cpu = 0 ; cpu < NumberOfThreads ; cpu++ ) {
       
       CommonLoopList[cpu] = new LOOP_ENTRY[NumberOfVortexLoops + 1];
       
       LoopIsCommon[cpu] = new int[MaxInteractionLoops + 1];
       
       zero_int_array(LoopIsCommon[cpu], MaxInteractionLoops);
       
    }
    
    MaxLevels = VSPGeom.NumberOfGridLevels();

    NewHits = 0;
 
    LoopOffSet = 0;
    
    for ( Level = MGLevel + 1 ; Level <= MaxLevels ; Level++ ) {

       if ( LoopType == FIXED_LOOPS ) PRINTF("Working on level %d of %d \r",Level,MaxLevels);fflush(NULL);
#ifndef AUTODIFF
#pragma omp parallel for reduction(+:NewHits) private(cpu,CurrentLoop,i,j,CommonLoops,TestLoop,TotalFound,Found,Done,xyz,Vec,Test,Distance,k,p,NumberOfLoops,TempLoopInteractionList) schedule(dynamic)
#endif
       for ( Loop = 1 ; Loop <= VSPGeom.Grid(Level).NumberOfLoops() ; Loop++ ) {

#ifndef AUTODIFF
       
#ifdef VSPAERO_OPENMP    
          cpu = omp_get_thread_num();
#else
          cpu = 0;
#endif         

#else
          cpu = 0;
#endif

          CurrentLoop = NumberOfAdjointInteractionLoops(LoopType) + Loop;

          AdjointInteractionList(LoopType)[CurrentLoop].Level() = Level;
          
          AdjointInteractionList(LoopType)[CurrentLoop].Loop() = Loop;

          // Find common part of lists
          
          for ( i = 1 ; i <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
             
             j = VSPGeom.Grid(Level).LoopList(Loop).FineGridLoop(i) + LoopOffSet;
             
             CommonLoopList[cpu][i].NextLoop = 1;
     
             CommonLoopList[cpu][i].Loop = AdjointInteractionList(LoopType)[j].SurfaceVortexLoopInteractionList();
             
             CommonLoopList[cpu][i].NumberOfVortexLoops = AdjointInteractionList(LoopType)[j].NumberOfVortexLoops();
     
          }

          CommonLoops = 0;
              
          while ( CommonLoopList[cpu][1].NextLoop <= CommonLoopList[cpu][1].NumberOfVortexLoops ) {

             TestLoop = CommonLoopList[cpu][1].Loop[CommonLoopList[cpu][1].NextLoop]->MGVortexLoop();
            
             TotalFound = 0;
             
             // Look to see if all the fine grid loops have a common source loop
             
             for ( i = 2 ; i <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
      
                Found = Done = 0;
                
                while ( CommonLoopList[cpu][i].NextLoop <= CommonLoopList[cpu][i].NumberOfVortexLoops && !Found && !Done ) {

                   // Found a common source loop
                                      
                   if ( TestLoop == CommonLoopList[cpu][i].Loop[CommonLoopList[cpu][i].NextLoop]->MGVortexLoop() ) {
                      
                      Found = 1;
                      
                      CommonLoopList[cpu][i].NextLoop++;
                      
                   }
               
                   // Not on this level
                   
                   else if ( CommonLoopList[cpu][i].Loop[CommonLoopList[cpu][i].NextLoop]->MGVortexLoop() < TestLoop ) {
                      
                      CommonLoopList[cpu][i].NextLoop++;
                      
                   }
                   
                   // Nothing else to check
                   
                   else {
                      
                      Done = 1;
                      
                   }
                   
                }
                
                if ( Found ) TotalFound++;
              
             }
             
             // If we found N - 1 loops, then all of them have a common source loop
             // So now decide if we meet the far away condition
         
             if ( TotalFound > 0 && TotalFound == VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() - 1 ) {

                // Mid point of common source loop
                
                xyz[0] = CommonLoopList[cpu][1].Loop[CommonLoopList[cpu][1].NextLoop]->Xc();
                xyz[1] = CommonLoopList[cpu][1].Loop[CommonLoopList[cpu][1].NextLoop]->Yc();
                xyz[2] = CommonLoopList[cpu][1].Loop[CommonLoopList[cpu][1].NextLoop]->Zc();
                
                // Subtract off centroid of agglomerated loop for all the fine grid loops
                
                Vec[0] = xyz[0] - VSPGeom.Grid(Level).LoopList(Loop).Xc();
                Vec[1] = xyz[1] - VSPGeom.Grid(Level).LoopList(Loop).Yc();
                Vec[2] = xyz[2] - VSPGeom.Grid(Level).LoopList(Loop).Zc();

                if ( Mach > 1. ) Vec[0] /= (Mach*Mach - 1.);
                      
                // Distance from centroid to source loop
                
                Distance = sqrt( SQR(Vec[0]) + SQR(Vec[1]) + SQR(Vec[2]) );
                
                // Find maximum distance from agglomerated loop centroid to fine grid loop centroids
                
                Test = 0.;
                
                for ( i = 1 ; i <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
                
                    // Centroid of agglomerated loop for all the fine grid loops
                    
                    Vec[0] = VSPGeom.Grid(Level).LoopList(Loop).Xc();
                    Vec[1] = VSPGeom.Grid(Level).LoopList(Loop).Yc();
                    Vec[2] = VSPGeom.Grid(Level).LoopList(Loop).Zc();
                
                    // Centroid of the j'th fine grid loop
                    
                    j = VSPGeom.Grid(Level).LoopList(Loop).FineGridLoop(i);
                
                    // Distance between fine grid and agglomerated grid centroids
                    
                    Vec[0] -= VSPGeom.Grid(Level-1).LoopList(j).Xc();
                    Vec[1] -= VSPGeom.Grid(Level-1).LoopList(j).Yc();
                    Vec[2] -= VSPGeom.Grid(Level-1).LoopList(j).Zc();
                
                    Test = MAX(Test,sqrt( SQR(Vec[0]) + SQR(Vec[1]) + SQR(Vec[2]) ));
                    
                }
                
                Test = VSPGeom.Grid(Level).LoopList(Loop).Length() + VSPGeom.Grid(Level).LoopList(Loop).CentroidOffSet();
            
                Test *= FarAway;

                if ( Test <= Distance ) {
                   
                   CommonLoops++;

                   LoopIsCommon[cpu][CommonLoopList[cpu][1].Loop[CommonLoopList[cpu][1].NextLoop]->MGVortexLoop()] = 1;
                  
                }
               
             }
       
             CommonLoopList[cpu][1].NextLoop++;
    
          }
    
          // Create a list of the common loops, and trim up the remaining loop lists
          
          if ( CommonLoops > 0 ) {
 
             // Create the common list
            
             AdjointInteractionList(LoopType)[CurrentLoop].Level() = Level;
             
             AdjointInteractionList(LoopType)[CurrentLoop].Loop() = Loop;
     
             AdjointInteractionList(LoopType)[CurrentLoop].SizeLoopList(CommonLoops);
             
             i = 1;
             
             j = 0;
      
             while ( j < CommonLoops && i <= CommonLoopList[cpu][1].NumberOfVortexLoops ) {

                if ( LoopIsCommon[cpu][CommonLoopList[cpu][1].Loop[i]->MGVortexLoop()] == 1 ) {
                  
                   AdjointInteractionList(LoopType)[CurrentLoop].SurfaceVortexLoopInteractionList()[++j] = CommonLoopList[cpu][1].Loop[i];
                      
                }
                   
                i++;
                
             }
             
             // Trim out the common loops from the initial lists
             
             for ( i = 1 ; i <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
                
                j = VSPGeom.Grid(Level).LoopList(Loop).FineGridLoop(i) + LoopOffSet;
                
                NumberOfLoops = AdjointInteractionList(LoopType)[j].NumberOfVortexLoops() - CommonLoops;
              
                // There are non-common loops remaining
                
                if ( NumberOfLoops > 0 ) {
 
                   TempLoopInteractionList = new VSP_LOOP*[NumberOfLoops + 1];
              
                   k = 0;
             
                   p = 1;
              
                   while ( k < NumberOfLoops && p <= AdjointInteractionList(LoopType)[j].NumberOfVortexLoops() ) {

                      if ( LoopIsCommon[cpu][AdjointInteractionList(LoopType)[j].SurfaceVortexLoopInteractionList(p)->MGVortexLoop()] == 0 ) {
 
                         TempLoopInteractionList[++k] = AdjointInteractionList(LoopType)[j].SurfaceVortexLoopInteractionList(p);
                        
                      }
                      
                      p++;

                   }
 
                   AdjointInteractionList(LoopType)[j].UseLoopList(NumberOfLoops, TempLoopInteractionList);
 
                }
                
                // There are no non-common loops remaining...
                
                else {

                   AdjointInteractionList(LoopType)[j].Level() = 0;
                   
                   AdjointInteractionList(LoopType)[j].Loop() = 0;

                   AdjointInteractionList(LoopType)[j].DeleteLoopList();
                 
                }
              
             }
                   
             // Unmark the common loops
             
             for ( j = 1 ; j <= AdjointInteractionList(LoopType)[CurrentLoop].NumberOfVortexLoops() ; j++ ) {

                 LoopIsCommon[cpu][AdjointInteractionList(LoopType)[CurrentLoop].SurfaceVortexLoopInteractionList(j)->MGVortexLoop()] = 0;
   
             }

          }
     
          for ( i = 1 ; i <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {

             NewHits += CommonLoopList[cpu][i].NumberOfVortexLoops - CommonLoops;
             
          }
          
          if ( Level == MaxLevels ) NewHits += CommonLoops;
        
       }
 
       NumberOfAdjointInteractionLoops(LoopType) += VSPGeom.Grid(Level).NumberOfLoops();
       
       LoopOffSet += VSPGeom.Grid(Level-1).NumberOfLoops();
       
    }

    AdjointSpeedRatio_ *= (long double) AdjointTotalHits_ / (double) NewHits;

    for ( cpu = 0 ; cpu < NumberOfThreads ; cpu++ ) {
       
       delete [] CommonLoopList[cpu];
       
    }
    
    delete [] CommonLoopList;

    for ( cpu = 0 ; cpu < NumberOfThreads ; cpu++ ) {
       
       delete [] LoopIsCommon[cpu];
       
    }
    
    delete [] LoopIsCommon;
        
    // Delete any zero length lists
    
    int NumberOfActualLoops = 0;

    for ( i = 1 ; i <= NumberOfAdjointInteractionLoops(LoopType) ; i++ ) {
       
       if ( AdjointInteractionList(LoopType)[i].NumberOfVortexLoops() > 0 ) {
          
          NumberOfActualLoops++;
          
       }
       
    }

    LOOP_INTERACTION_ENTRY *TempList;
    
    TempList = new LOOP_INTERACTION_ENTRY[NumberOfActualLoops + 1];
    
    j = 0;

    for ( i = 1 ; i <= NumberOfAdjointInteractionLoops(LoopType) ; i++ ) {
       
       if ( AdjointInteractionList(LoopType)[i].NumberOfVortexLoops() > 0 ) {
          
          TempList[++j] = AdjointInteractionList(LoopType)[i];
          
       }
       
    }
    
    if ( j != NumberOfActualLoops ) {
       
       PRINTF("Error in cleaning up interaction list! \n"); fflush(NULL);
       exit(1);
       
    }

    DeleteAdjointList(LoopType);

    UseAdjointList(LoopType, NumberOfActualLoops, TempList);
       
}

#include "END_NAME_SPACE.H"

