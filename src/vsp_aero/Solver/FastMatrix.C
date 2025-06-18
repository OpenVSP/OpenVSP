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
    
    MaxNumberOfForwardInteractionEdges_[0] = 0;
    MaxNumberOfForwardInteractionEdges_[1] = 0;
    
    NumberOfForwardInteractionLoops_[0] = 0;
    NumberOfForwardInteractionLoops_[1] = 0;

    ForwardInteractionLoopList_[0] = NULL;
    ForwardInteractionLoopList_[1] = NULL;
    
    // Adjoint
    
    MaxNumberOfAdjointInteractionLoops_[0] = 0;
    MaxNumberOfAdjointInteractionLoops_[1] = 0;

    NumberOfAdjointInteractionLoops_[0] = 0;
    NumberOfAdjointInteractionLoops_[1] = 0;

    AdjointInteractionLoopList_[0] = NULL;
    AdjointInteractionLoopList_[1] = NULL;

}

/*##############################################################################
#                                                                              #
#                                  FAST_MATRIX Destructor                      #
#                                                                              #
##############################################################################*/

FAST_MATRIX::~FAST_MATRIX(void)
{

    if ( NumberOfForwardInteractionLoops_[0] != 0 ) delete [] ForwardInteractionLoopList_[0];
    if ( NumberOfForwardInteractionLoops_[1] != 0 ) delete [] ForwardInteractionLoopList_[1];

    if ( NumberOfAdjointInteractionLoops_[0] != 0 ) delete [] AdjointInteractionLoopList_[0];
    if ( NumberOfAdjointInteractionLoops_[1] != 0 ) delete [] AdjointInteractionLoopList_[1];
            
}

/*##############################################################################
#                                                                              #
#                          FAST_MATRIX SizeForwardLoopList                     #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::SizeForwardLoopList(int LoopType, int NumberOfLoops)
{

    if ( MaxNumberOfForwardInteractionLoops_[LoopType] > 0 ) {
       
       delete [] ForwardInteractionLoopList_[LoopType];
   
       MaxNumberOfForwardInteractionLoops_[LoopType] = 0;
       
          NumberOfForwardInteractionLoops_[LoopType] = 0;
       
    }
              
    MaxNumberOfForwardInteractionLoops_[LoopType] = NumberOfLoops;
              
       NumberOfForwardInteractionLoops_[LoopType] = NumberOfLoops;
    
    ForwardInteractionLoopList_[LoopType] = new LOOP_INTERACTION_ENTRY[NumberOfLoops + 1];

}

/*##############################################################################
#                                                                              #
#                          FAST_MATRIX SizeForwardEdgeList                     #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::SizeForwardEdgeList(int LoopType, int NumberOfEdges)
{

    if ( MaxNumberOfForwardInteractionEdges_[LoopType] > 0 ) {
       
       delete [] ForwardInteractionEdgeList_[LoopType];
       
       MaxNumberOfForwardInteractionEdges_[LoopType] = 0;
       
          NumberOfForwardInteractionEdges_[LoopType] = 0;
       
    }
              
    MaxNumberOfForwardInteractionEdges_[LoopType] = NumberOfEdges;
              
       NumberOfForwardInteractionEdges_[LoopType] = NumberOfEdges;
    
    ForwardInteractionEdgeList_[LoopType] = new LOOP_INTERACTION_ENTRY[NumberOfEdges + 1];

}

/*##############################################################################
#                                                                              #
#                          FAST_MATRIX SizeAdjointLoopList                     #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::SizeAdjointLoopList(int LoopType, int NumberOfLoops)
{

    if ( MaxNumberOfAdjointInteractionLoops_[LoopType] > 0 ) {
       
       delete [] AdjointInteractionLoopList_[LoopType];
       
       MaxNumberOfAdjointInteractionLoops_[LoopType] = 0;
       
          NumberOfAdjointInteractionLoops_[LoopType] = 0;
       
    }
    
    MaxNumberOfAdjointInteractionLoops_[LoopType] = NumberOfLoops;
    
       NumberOfAdjointInteractionLoops_[LoopType] = NumberOfLoops;
    
    AdjointInteractionLoopList_[LoopType] = new LOOP_INTERACTION_ENTRY[NumberOfLoops + 1];

}

/*##############################################################################
#                                                                              #
#                          FAST_MATRIX SizeAdjointEdgeList                     #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::SizeAdjointEdgeList(int LoopType, int NumberOfEdges)
{

    if ( MaxNumberOfAdjointInteractionEdges_[LoopType] > 0 ) {
       
       delete [] AdjointInteractionEdgeList_[LoopType];
       
       MaxNumberOfAdjointInteractionEdges_[LoopType] = 0;
       
          NumberOfAdjointInteractionEdges_[LoopType] = 0;
       
    }
              
    MaxNumberOfAdjointInteractionEdges_[LoopType] = NumberOfEdges;
              
       NumberOfAdjointInteractionEdges_[LoopType] = NumberOfEdges;
    
    AdjointInteractionEdgeList_[LoopType] = new LOOP_INTERACTION_ENTRY[NumberOfEdges + 1];

}

/*##############################################################################
#                                                                              #
#                       FAST_MATRIX DeleteForwardLoopList                      #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::DeleteForwardLoopList(int LoopType)
{

    if ( NumberOfForwardInteractionLoops_[LoopType] != 0 ) delete [] ForwardInteractionLoopList_[LoopType];

    NumberOfForwardInteractionLoops_[LoopType] = 0;
    
}

/*##############################################################################
#                                                                              #
#                       FAST_MATRIX DeleteForwardEdgeList                      #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::DeleteForwardEdgeList(int LoopType)
{

    if ( NumberOfForwardInteractionEdges_[LoopType] != 0 ) delete [] ForwardInteractionEdgeList_[LoopType];

    NumberOfForwardInteractionEdges_[LoopType] = 0;
    
}

/*##############################################################################
#                                                                              #
#                    FAST_MATRIX DeleteAdjointLoopList                         #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::DeleteAdjointLoopList(int LoopType)
{

    if ( NumberOfAdjointInteractionLoops_[LoopType] != 0 ) delete [] AdjointInteractionLoopList_[LoopType];

    NumberOfAdjointInteractionLoops_[LoopType] = 0;
    
}

/*##############################################################################
#                                                                              #
#                       FAST_MATRIX DeleteAdjointEdgeList                      #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::DeleteAdjointEdgeList(int LoopType)
{

    if ( NumberOfAdjointInteractionEdges_[LoopType] != 0 ) delete [] AdjointInteractionEdgeList_[LoopType];

    NumberOfAdjointInteractionEdges_[LoopType] = 0;
    
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
          
          ForwardInteractionLoopList_[LoopType][i] = FastMatrix.ForwardInteractionLoopList_[LoopType][i];
          
       }
       
       // Adjoint
       
       for ( i = 1 ; i <= NumberOfAdjointInteractionLoops_[LoopType] ; i++ ) {
          
          AdjointInteractionLoopList_[LoopType][i] = FastMatrix.AdjointInteractionLoopList_[LoopType][i];
          
       }       
       
    } 
 
    return *this;

}

/*##############################################################################
#                                                                              #
#                            FAST_MATRIX UseForwardList                        #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::UseForwardLoopList(int LoopType, int NumberOfLoops, LOOP_INTERACTION_ENTRY *List)
{

    MaxNumberOfForwardInteractionLoops_[LoopType] = NumberOfLoops;

       NumberOfForwardInteractionLoops_[LoopType] = NumberOfLoops;
    
    ForwardInteractionLoopList_[LoopType] = List;
    
}

/*##############################################################################
#                                                                              #
#                            FAST_MATRIX UseAdjointList                        #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::UseAdjointLoopList(int LoopType, int NumberOfLoops, LOOP_INTERACTION_ENTRY *List)
{

    MaxNumberOfAdjointInteractionLoops_[LoopType] = NumberOfLoops;

       NumberOfAdjointInteractionLoops_[LoopType] = NumberOfLoops;
    
    AdjointInteractionLoopList_[LoopType] = List;
    
}

/*##############################################################################
#                                                                              #
#                          FAST_MATRIX MergeForwardList                        #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::MergeForwardList(int NumberOfThreads, int MGLevel, int LoopType, VSP_GEOM &VSPGeom, double Mach, double FarAway)
{

    int i, j, k, p, cpu, Level, Loop, NumberOfEdges, NumberOfLoops, CurrentLoop;
    int TestEdge, MaxInteractionEdges, LoopOffSet, InteractionType;
    int Done, Found, TotalFound, CommonEdges, MaxLevels, **EdgeIsCommon, NumberOfVortexLoops;
    LOOP_ENTRY **CommonEdgeList;
    VSP_EDGE **TempEdgeInteractionList;
    double xyz[3], Vec[3], Distance, Test;

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

       if ( LoopType == FIXED_LOOPS ) { printf("Working on level %d of %d \r",Level,MaxLevels);fflush(NULL); };

#pragma omp parallel for reduction(+:NewHits) private(cpu,CurrentLoop,i,j,CommonEdges,TestEdge,TotalFound,Found,Done,xyz,Vec,Test,Distance,k,p,NumberOfEdges,TempEdgeInteractionList) schedule(dynamic)
       for ( Loop = 1 ; Loop <= VSPGeom.Grid(Level).NumberOfLoops() ; Loop++ ) {
     
#ifdef VSPAERO_OPENMP    
          cpu = omp_get_thread_num();
#else
          cpu = 0;
#endif         

          CurrentLoop = NumberOfForwardInteractionLoops(LoopType) + Loop;
       
          ForwardInteractionLoopList(LoopType)[CurrentLoop].Level() = Level;
          
          ForwardInteractionLoopList(LoopType)[CurrentLoop].Loop() = Loop;

          // Find common part of lists
          
          for ( i = 1 ; i <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
             
             j = VSPGeom.Grid(Level).LoopList(Loop).FineGridLoop(i) + LoopOffSet;
             
             CommonEdgeList[cpu][i].NextEdge = 1;
     
             CommonEdgeList[cpu][i].Edge = ForwardInteractionLoopList(LoopType)[j].SurfaceVortexEdgeInteractionList();
             
             CommonEdgeList[cpu][i].NumberOfVortexEdges = ForwardInteractionLoopList(LoopType)[j].NumberOfVortexEdges();
             
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
                
                Distance -= 0.5*CommonEdgeList[cpu][1].Edge[CommonEdgeList[cpu][1].NextEdge]->Length();
    
                Test = FarAway * ( VSPGeom.Grid(Level).LoopList(Loop).Length() + VSPGeom.Grid(Level).LoopList(Loop).CentroidOffSet() );
            
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
             
             ForwardInteractionLoopList(LoopType)[CurrentLoop].Level() = Level;
             
             ForwardInteractionLoopList(LoopType)[CurrentLoop].Loop() = Loop;
     
             ForwardInteractionLoopList(LoopType)[CurrentLoop].SizeEdgeList(CommonEdges);
             
             i = 1;
             
             j = 0;
        
             while ( j < CommonEdges && i <= CommonEdgeList[cpu][1].NumberOfVortexEdges ) {
         
               if ( EdgeIsCommon[cpu][CommonEdgeList[cpu][1].Edge[i]->VortexEdge()] == 1 ) {
              
                    ForwardInteractionLoopList(LoopType)[CurrentLoop].SurfaceVortexEdgeInteractionList()[++j] = CommonEdgeList[cpu][1].Edge[i];
                    
                }
                
                i++;
                
             }
             
             // Trim out the common edges from the initial lists
             
             for ( i = 1 ; i <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
                
                j = VSPGeom.Grid(Level).LoopList(Loop).FineGridLoop(i) + LoopOffSet;
                
                NumberOfEdges = ForwardInteractionLoopList(LoopType)[j].NumberOfVortexEdges() - CommonEdges;
              
                // There are non-common edges remaining
                
                if ( NumberOfEdges > 0 ) {
                   
                   TempEdgeInteractionList = new VSP_EDGE*[NumberOfEdges + 1];
       
                   k = 0;
             
                   p = 1;
              
                   while ( k < NumberOfEdges && p <= ForwardInteractionLoopList(LoopType)[j].NumberOfVortexEdges() ) {

                      if ( EdgeIsCommon[cpu][ForwardInteractionLoopList(LoopType)[j].SurfaceVortexEdgeInteractionList(p)->VortexEdge()] == 0 ) {
 
                         TempEdgeInteractionList[++k] = ForwardInteractionLoopList(LoopType)[j].SurfaceVortexEdgeInteractionList(p);
                         
                      }
                      
                      p++;

                   }
 
                   ForwardInteractionLoopList(LoopType)[j].UseEdgeList(NumberOfEdges, TempEdgeInteractionList);
 
                }
                
                // There are no non-common edges remaining...
                
                else {

                   ForwardInteractionLoopList(LoopType)[j].Level() = 0;
                   
                   ForwardInteractionLoopList(LoopType)[j].Loop() = 0;

                   ForwardInteractionLoopList(LoopType)[j].DeleteEdgeList();
                 
                }
              
             }
                   
             // Unmark the common edges
             
             for ( j = 1 ; j <= ForwardInteractionLoopList(LoopType)[CurrentLoop].NumberOfVortexEdges() ; j++ ) {

                 EdgeIsCommon[cpu][ABS(ForwardInteractionLoopList(LoopType)[CurrentLoop].SurfaceVortexEdgeInteractionList(j)->VortexEdge())] = 0;
   
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
       
       if ( ForwardInteractionLoopList(LoopType)[i].NumberOfVortexEdges() > 0 ) {
          
          NumberOfActualLoops++;
          
       }
       
    }

    LOOP_INTERACTION_ENTRY *TempList;
    
    TempList = new LOOP_INTERACTION_ENTRY[NumberOfActualLoops + 1];
    
    j = 0;

    for ( i = 1 ; i <= NumberOfForwardInteractionLoops(LoopType) ; i++ ) {
       
       if ( ForwardInteractionLoopList(LoopType)[i].NumberOfVortexEdges() > 0 ) {
          
          TempList[++j] = ForwardInteractionLoopList(LoopType)[i];
          
       }
       
    }
    
    if ( j != NumberOfActualLoops ) {
       
       printf("Error in cleaning up interaction list! \n"); fflush(NULL);
       exit(1);
       
    }

    DeleteForwardLoopList(LoopType);

    UseForwardLoopList(LoopType, NumberOfActualLoops, TempList);

    // Calculate total speed up
    
    ForwardSpeedRatio_ = 0.;
    
    for ( i = 1 ; i <= NumberOfForwardInteractionLoops(LoopType) ; i++ ) {

       ForwardSpeedRatio_ += double(ForwardInteractionLoopList(LoopType)[i].NumberOfVortexEdges());
       
    }
    
    ForwardSpeedRatio_ /= VSPGeom.Grid(MGLevel).NumberOfLoops();
    ForwardSpeedRatio_ /= VSPGeom.Grid(MGLevel).NumberOfEdges();

    ForwardSpeedRatio_ = 1./ForwardSpeedRatio_;
    
}

/*##############################################################################
#                                                                              #
#                          FAST_MATRIX MergeAdjointList                        #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::MergeAdjointList(int NumberOfThreads, int MGLevel, int LoopType, VSP_GEOM &VSPGeom, double Mach, double FarAway)
{

    int i, j, k, p, cpu, Level, Loop, NumberOfLoops, CurrentLoop;
    int TestLoop, MaxInteractionLoops, LoopOffSet, InteractionType;
    int Done, Found, TotalFound, CommonLoops, MaxLevels, **LoopIsCommon, NumberOfVortexLoops;
    LOOP_ENTRY **CommonLoopList;
    VSP_LOOP **TempLoopInteractionList;
    double xyz[3], Vec[3], Distance, Test;

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

       if ( LoopType == FIXED_LOOPS ) { printf("Working on level %d of %d \r",Level,MaxLevels);fflush(NULL); };
       
#pragma omp parallel for reduction(+:NewHits) private(cpu,CurrentLoop,i,j,CommonLoops,TestLoop,TotalFound,Found,Done,xyz,Vec,Test,Distance,k,p,NumberOfLoops,TempLoopInteractionList) schedule(dynamic)
       for ( Loop = 1 ; Loop <= VSPGeom.Grid(Level).NumberOfLoops() ; Loop++ ) {

#ifdef VSPAERO_OPENMP    
          cpu = omp_get_thread_num();
#else
          cpu = 0;
#endif         

          CurrentLoop = NumberOfAdjointInteractionLoops(LoopType) + Loop;

          AdjointInteractionLoopList(LoopType)[CurrentLoop].Level() = Level;
          
          AdjointInteractionLoopList(LoopType)[CurrentLoop].Loop() = Loop;

          // Find common part of lists
          
          for ( i = 1 ; i <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
             
             j = VSPGeom.Grid(Level).LoopList(Loop).FineGridLoop(i) + LoopOffSet;
             
             CommonLoopList[cpu][i].NextLoop = 1;
     
             CommonLoopList[cpu][i].Loop = AdjointInteractionLoopList(LoopType)[j].SurfaceVortexLoopInteractionList();
             
             CommonLoopList[cpu][i].NumberOfVortexLoops = AdjointInteractionLoopList(LoopType)[j].NumberOfVortexLoops();
     
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
  
                Distance -= 0.5*CommonLoopList[cpu][1].Loop[CommonLoopList[cpu][1].NextLoop]->Length();
  
                Test = FarAway * ( VSPGeom.Grid(Level).LoopList(Loop).Length() + VSPGeom.Grid(Level).LoopList(Loop).CentroidOffSet() );
  
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
            
             AdjointInteractionLoopList(LoopType)[CurrentLoop].Level() = Level;
             
             AdjointInteractionLoopList(LoopType)[CurrentLoop].Loop() = Loop;
     
             AdjointInteractionLoopList(LoopType)[CurrentLoop].SizeLoopList(CommonLoops);
             
             i = 1;
             
             j = 0;
      
             while ( j < CommonLoops && i <= CommonLoopList[cpu][1].NumberOfVortexLoops ) {

                if ( LoopIsCommon[cpu][CommonLoopList[cpu][1].Loop[i]->MGVortexLoop()] == 1 ) {
                  
                   AdjointInteractionLoopList(LoopType)[CurrentLoop].SurfaceVortexLoopInteractionList()[++j] = CommonLoopList[cpu][1].Loop[i];
                      
                }
                   
                i++;
                
             }
             
             // Trim out the common loops from the initial lists
             
             for ( i = 1 ; i <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfFineGridLoops() ; i++ ) {
                
                j = VSPGeom.Grid(Level).LoopList(Loop).FineGridLoop(i) + LoopOffSet;
                
                NumberOfLoops = AdjointInteractionLoopList(LoopType)[j].NumberOfVortexLoops() - CommonLoops;
              
                // There are non-common loops remaining
                
                if ( NumberOfLoops > 0 ) {
 
                   TempLoopInteractionList = new VSP_LOOP*[NumberOfLoops + 1];
              
                   k = 0;
             
                   p = 1;
              
                   while ( k < NumberOfLoops && p <= AdjointInteractionLoopList(LoopType)[j].NumberOfVortexLoops() ) {

                      if ( LoopIsCommon[cpu][AdjointInteractionLoopList(LoopType)[j].SurfaceVortexLoopInteractionList(p)->MGVortexLoop()] == 0 ) {
 
                         TempLoopInteractionList[++k] = AdjointInteractionLoopList(LoopType)[j].SurfaceVortexLoopInteractionList(p);
                        
                      }
                      
                      p++;

                   }
 
                   AdjointInteractionLoopList(LoopType)[j].UseLoopList(NumberOfLoops, TempLoopInteractionList);
 
                }
                
                // There are no non-common loops remaining...
                
                else {

                   AdjointInteractionLoopList(LoopType)[j].Level() = 0;
                   
                   AdjointInteractionLoopList(LoopType)[j].Loop() = 0;

                   AdjointInteractionLoopList(LoopType)[j].DeleteLoopList();
                 
                }
              
             }
                   
             // Unmark the common loops
             
             for ( j = 1 ; j <= AdjointInteractionLoopList(LoopType)[CurrentLoop].NumberOfVortexLoops() ; j++ ) {

                 LoopIsCommon[cpu][AdjointInteractionLoopList(LoopType)[CurrentLoop].SurfaceVortexLoopInteractionList(j)->MGVortexLoop()] = 0;
   
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
       
       if ( AdjointInteractionLoopList(LoopType)[i].NumberOfVortexLoops() > 0 ) {
          
          NumberOfActualLoops++;
          
       }
       
    }

    LOOP_INTERACTION_ENTRY *TempList;
    
    TempList = new LOOP_INTERACTION_ENTRY[NumberOfActualLoops + 1];
    
    j = 0;

    for ( i = 1 ; i <= NumberOfAdjointInteractionLoops(LoopType) ; i++ ) {
       
       if ( AdjointInteractionLoopList(LoopType)[i].NumberOfVortexLoops() > 0 ) {
          
          TempList[++j] = AdjointInteractionLoopList(LoopType)[i];
          
       }
       
    }
    
    if ( j != NumberOfActualLoops ) {
       
       printf("Error in cleaning up interaction list! \n"); fflush(NULL);
       exit(1);
       
    }

    DeleteAdjointLoopList(LoopType);

    UseAdjointLoopList(LoopType, NumberOfActualLoops, TempList);

    AdjointSpeedRatio_ = 0.;

    for ( i = 1 ; i <= NumberOfAdjointInteractionLoops(LoopType) ; i++ ) {

       AdjointSpeedRatio_ += double(AdjointInteractionLoopList(LoopType)[i].NumberOfVortexLoops());
       
    }
    
    AdjointSpeedRatio_ /= VSPGeom.Grid(MGLevel).NumberOfLoops();
    AdjointSpeedRatio_ /= VSPGeom.Grid(MGLevel).NumberOfLoops();
    
    AdjointSpeedRatio_ = 1./AdjointSpeedRatio_;
           
}

/*##############################################################################
#                                                                              #
#                          FAST_MATRIX CreateForwardEdgeList                   #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::CreateForwardEdgeList(int NumberOfThreads, int MGLevel, int LoopType, VSP_GEOM &VSPGeom)
{

    int i, k, m, Loop, Edge, Level, GlobalEdge, *EdgeIsUsed, NumberOfEdges, MaxInteractionEdges, OldSize, NewSize;
    LOOP_INTERACTION_ENTRY *TempList;

    // Temp list for each edge interaction list

    MaxInteractionEdges = 0;
    
    for ( Level = 1 ; Level <= VSPGeom.NumberOfGridLevels() ; Level++ ) {
       
       MaxInteractionEdges += VSPGeom.Grid(Level).NumberOfEdges();
       
    }    
    
    TempList = new LOOP_INTERACTION_ENTRY[MaxInteractionEdges + 1];

    EdgeIsUsed = new int[MaxInteractionEdges + 1];
    
    zero_int_array(EdgeIsUsed, MaxInteractionEdges);

    // Loop over each level

    for ( i = 1 ; i <= NumberOfForwardInteractionLoops(LoopType) ; i++ ) {

       Loop = ForwardInteractionLoopList(LoopType)[i].Loop();
       
       Level = ForwardInteractionLoopList(LoopType)[i].Level();

       // Mark the edges that are used and how many interactions edges they have
       
       for ( k = 1 ; k <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfEdges() ; k++ ) {
    
          Edge = VSPGeom.Grid(Level).LoopList(Loop).Edge(k);
          
          GlobalEdge = VSPGeom.Grid(Level).EdgeList(Edge).VortexEdge(); 
          
          // First time we've seen this edge
     
          if ( EdgeIsUsed[GlobalEdge] == 0 ) {
             
             TempList[GlobalEdge].Edge() = Edge;
             
             TempList[GlobalEdge].Level() = Level;
             
             TempList[GlobalEdge].SizeEdgeList(ForwardInteractionLoopList(LoopType)[i].NumberOfVortexEdges());
             
             for ( m = 1 ; m <= ForwardInteractionLoopList(LoopType)[i].NumberOfVortexEdges() ; m++ ) {
                
                TempList[GlobalEdge].SurfaceVortexEdgeInteractionList()[m] = ForwardInteractionLoopList(LoopType)[i].SurfaceVortexEdgeInteractionList(m);
                
             }
          
             EdgeIsUsed[GlobalEdge]++;
             
          }
          
          // We've seen this edge before... so we need to merge the new and previous interaction list... only 
          // keeping the unique edges
          
          else if ( EdgeIsUsed[GlobalEdge] == 1 ) {
             
             OldSize = TempList[GlobalEdge].NumberOfVortexEdges();
             
             NewSize = OldSize + ForwardInteractionLoopList(LoopType)[i].NumberOfVortexEdges();
             
             TempList[GlobalEdge].ReSizeEdgeList(NewSize);
             
             for ( m = 1 ; m <= ForwardInteractionLoopList(LoopType)[i].NumberOfVortexEdges() ; m++ ) {
                
                TempList[GlobalEdge].SurfaceVortexEdgeInteractionList()[OldSize + m] = ForwardInteractionLoopList(LoopType)[i].SurfaceVortexEdgeInteractionList(m);
                
             }
          
             EdgeIsUsed[GlobalEdge]++;
                 
          }
          
          else {
             
             printf("How did we get here! An edge should only show up twice! \n");
             
             printf("EdgeIsUsed[GlobalEdge]: %d \n",EdgeIsUsed[GlobalEdge]);
             
             fflush(NULL);exit(1);

          }                

       }

    }

    // Determine number of edges
    
    NumberOfEdges = 0;
    
    for ( i = 1 ; i <= MaxInteractionEdges ; i++ ) {

       if ( EdgeIsUsed[i] ) NumberOfEdges++;

    }    

    SizeForwardEdgeList(LoopType, NumberOfEdges);
    
    // Now pack the new lists and clean them of duplicates
  
    NumberOfEdges = 0;
    
    for ( i = 1 ; i <= MaxInteractionEdges ; i++ ) {

       if ( EdgeIsUsed[i] ) {
          
          NumberOfEdges++;
          
          ForwardInteractionEdgeList(LoopType)[NumberOfEdges].Edge() = TempList[i].Edge();
          
          ForwardInteractionEdgeList(LoopType)[NumberOfEdges].Level() = TempList[i].Level();

          ForwardInteractionEdgeList(LoopType)[NumberOfEdges].UseEdgeList(TempList[i].NumberOfVortexEdges(), TempList[i].SurfaceVortexEdgeInteractionList());
          
       }

    }    

    CleanForwardEdgeList(NumberOfThreads,LoopType,MaxInteractionEdges);

    delete [] EdgeIsUsed;

    double SpeedUpRatio;
    
    SpeedUpRatio = 0.;

    for ( i = 1 ; i <= NumberOfForwardInteractionEdges(LoopType) ; i++ ) {

       SpeedUpRatio += double(VSPGeom.Grid(MGLevel).NumberOfEdges())/double(ForwardInteractionEdgeList(LoopType)[i].NumberOfVortexEdges());
       
    }
    
    SpeedUpRatio /= VSPGeom.Grid(MGLevel).NumberOfEdges();

}

///*##############################################################################
//#                                                                              #
//#                          FAST_MATRIX CreateForwardEdgeList                   #
//#                                                                              #
//##############################################################################*/
//
//void FAST_MATRIX::CreateForwardEdgeListNew(int NumberOfThreads, int MGLevel, int LoopType, VSP_GEOM &VSPGeom)
//{
//
//    int cpu, i, k, m, Loop, Edge, Level, GlobalEdge, **EdgeIsUsed, NumberOfEdges, MaxInteractionEdges, OldSize, NewSize;
//    LOOP_INTERACTION_ENTRY **TempList;
//
//    // Temp list for each edge interaction list
//
//    MaxInteractionEdges = 0;
//    
//    for ( Level = 1 ; Level <= VSPGeom.NumberOfGridLevels() ; Level++ ) {
//       
//       MaxInteractionEdges += VSPGeom.Grid(Level).NumberOfEdges();
//       
//    }    
// 
//    TempList = new LOOP_INTERACTION_ENTRY*[NumberOfThreads + 1];
//    
//    EdgeIsUsed = new int*[NumberOfThreads + 1];
//
//    for ( cpu = 0 ; cpu < NumberOfThreads ; cpu++ ) {
//       
//       TempList[cpu] = new LOOP_INTERACTION_ENTRY[MaxInteractionEdges + 1];
//       
//       EdgeIsUsed[cpu] = new int[MaxInteractionEdges + 1];
//       
//       zero_int_array(EdgeIsUsed[cpu], MaxInteractionEdges);
//       
//    }
//    
//   //TempList = new LOOP_INTERACTION_ENTRY[MaxInteractionEdges + 1];
//   //
//   //EdgeIsUsed = new int[MaxInteractionEdges + 1];
//   //
//   //zero_int_array(EdgeIsUsed, MaxInteractionEdges);
//
//    // Loop over each level
//
//printf("Starting edge shit... \n");fflush(NULL);
////#pragma omp parallel for private(cpu,Loop,Level,k,Edge,GlobalEdge,m,OldSize,NewSize) schedule(dynamic)
//    for ( i = 1 ; i <= NumberOfForwardInteractionLoops(LoopType) ; i++ ) {
//
//       Loop = ForwardInteractionLoopList(LoopType)[i].Loop();
//       
//       Level = ForwardInteractionLoopList(LoopType)[i].Level();
//
//#ifdef VSPAERO_OPENMP    
//       cpu = omp_get_thread_num();
//#else  
//       cpu = 0;
//#endif       
//cpu = 0;
//       // Mark the edges that are used and how many interactions edges they have
//       
//       for ( k = 1 ; k <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfEdges() ; k++ ) {
//    
//          Edge = VSPGeom.Grid(Level).LoopList(Loop).Edge(k);
//          
//          GlobalEdge = VSPGeom.Grid(Level).EdgeList(Edge).VortexEdge(); 
//          
//          // First time we've seen this edge
//     
//          if ( EdgeIsUsed[cpu][GlobalEdge] == 0 ) {
//             
//             TempList[cpu][GlobalEdge].Edge() = Edge;
//             
//             TempList[cpu][GlobalEdge].Level() = Level;
//             
//             TempList[cpu][GlobalEdge].SizeEdgeList(ForwardInteractionLoopList(LoopType)[i].NumberOfVortexEdges());
//             
//             for ( m = 1 ; m <= ForwardInteractionLoopList(LoopType)[i].NumberOfVortexEdges() ; m++ ) {
//                
//                TempList[cpu][GlobalEdge].SurfaceVortexEdgeInteractionList()[m] = ForwardInteractionLoopList(LoopType)[i].SurfaceVortexEdgeInteractionList(m);
//                
//             }
//          
//             EdgeIsUsed[cpu][GlobalEdge]++;
//             
//          }
//          
//          // We've seen this edge before... so we need to merge the new and previous interaction list...
//          
//          else if ( EdgeIsUsed[cpu][GlobalEdge] == 1 ) {
//             
//             OldSize = TempList[cpu][GlobalEdge].NumberOfVortexEdges();
//             
//             NewSize = OldSize + ForwardInteractionLoopList(LoopType)[i].NumberOfVortexEdges();
//             
//             TempList[cpu][GlobalEdge].ReSizeEdgeList(NewSize);
//             
//             for ( m = 1 ; m <= ForwardInteractionLoopList(LoopType)[i].NumberOfVortexEdges() ; m++ ) {
//                
//                TempList[cpu][GlobalEdge].SurfaceVortexEdgeInteractionList()[OldSize + m] = ForwardInteractionLoopList(LoopType)[i].SurfaceVortexEdgeInteractionList(m);
//                
//             }
//          
//             EdgeIsUsed[cpu][GlobalEdge]++;
//                 
//          }
//          
//          else {
//             
//             printf("How did we get here! An edge should only show up twice! \n");
//             
//             printf("EdgeIsUsed[GlobalEdge]: %d \n",EdgeIsUsed[cpu][GlobalEdge]);
//             
//             fflush(NULL);exit(1);
//
//          }                
//
//       }
//
//    }
//
//    // Determine number of edges
//    
//    NumberOfEdges = 0;
//
//    for ( cpu = 0 ; cpu < NumberOfThreads ; cpu++ ) {
//    
//       for ( i = 1 ; i <= MaxInteractionEdges ; i++ ) {
//   
//          if ( EdgeIsUsed[cpu][i] ) NumberOfEdges++;
//   
//       }    
//       
//    }
//
//printf("Done edge shit... \n");fflush(NULL);
//
//    SizeForwardEdgeList(LoopType, NumberOfEdges);
//    
//    // Now pack the new lists and clean them of duplicates
// 
//printf("Packing edge shit... \n");fflush(NULL);
//  
//    NumberOfEdges = 0;
//    
//    for ( cpu = 0 ; cpu < NumberOfThreads ; cpu++ ) {
//    
//       for ( i = 1 ; i <= MaxInteractionEdges ; i++ ) {
//   
//          if ( EdgeIsUsed[cpu][i] ) {
//             
//             NumberOfEdges++;
//             
//             ForwardInteractionEdgeList(LoopType)[NumberOfEdges].Edge() = TempList[cpu][i].Edge();
//             
//             ForwardInteractionEdgeList(LoopType)[NumberOfEdges].Level() = TempList[cpu][i].Level();
//   
//             ForwardInteractionEdgeList(LoopType)[NumberOfEdges].UseEdgeList(TempList[cpu][i].NumberOfVortexEdges(), TempList[cpu][i].SurfaceVortexEdgeInteractionList());
//             
//          }
//   
//       }  
//       
//    }  
//
//printf("Done Packing edge shit... \n");fflush(NULL);
//
//printf("Cleaning edge shit... \n");fflush(NULL);
//
//    CleanForwardEdgeList(NumberOfThreads,LoopType,MaxInteractionEdges,EdgeIsUsed);
//
//    for ( cpu = 0 ; cpu < NumberOfThreads ; cpu++ ) {
//
//       delete [] EdgeIsUsed[cpu];
//       
//    }
//
//printf("Done Cleaning edge shit... \n");fflush(NULL);
//
//    delete [] EdgeIsUsed;
//
//    double SpeedUpRatio;
//    
//    SpeedUpRatio = 0.;
//
//    for ( i = 1 ; i <= NumberOfForwardInteractionEdges(LoopType) ; i++ ) {
//
//       SpeedUpRatio += double(VSPGeom.Grid(MGLevel).NumberOfEdges())/double(ForwardInteractionEdgeList(LoopType)[i].NumberOfVortexEdges());
//       
//    }
//    
//    SpeedUpRatio /= VSPGeom.Grid(MGLevel).NumberOfEdges();
//
//}

/*##############################################################################
#                                                                              #
#                          FAST_MATRIX CleanForwardEdgeList                    #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::CleanForwardEdgeList(int NumberOfThreads, int LoopType, int MaxInteractionEdges)
{

    int i, j, cpu, NumEdges, Edge, **EdgeIsUsed;
    LOOP_INTERACTION_ENTRY TempList;
  
    EdgeIsUsed = new int*[NumberOfThreads + 1];

    for ( cpu = 0 ; cpu < NumberOfThreads ; cpu++ ) {

       EdgeIsUsed[cpu] = new int[MaxInteractionEdges + 1];
       
       zero_int_array(EdgeIsUsed[cpu], MaxInteractionEdges);
       
    }

#pragma omp parallel for private(cpu,NumEdges,j,Edge,TempList) schedule(dynamic)
    for ( i = 1 ; i <= NumberOfForwardInteractionEdges(LoopType) ; i++ ) {

#ifdef VSPAERO_OPENMP    
       cpu = omp_get_thread_num();
#else  
       cpu = 0;
#endif 

       // Determine how many unique edges there are

       NumEdges = 0;

       for ( j = 1 ; j <= ForwardInteractionEdgeList(LoopType)[i].NumberOfVortexEdges() ; j++ ) {
          
          Edge = ForwardInteractionEdgeList(LoopType)[i].SurfaceVortexEdgeInteractionList(j)->VortexEdge();
                    
          if ( EdgeIsUsed[cpu][Edge] != i ) {
             
             NumEdges++;
             
             EdgeIsUsed[cpu][Edge] = i;
             
          }

       }

       TempList.Edge() = ForwardInteractionEdgeList(LoopType)[i].Edge();
       
       TempList.Level() = ForwardInteractionEdgeList(LoopType)[i].Level();
       
       TempList.SizeEdgeList(NumEdges);

       NumEdges = 0;

       for ( j = 1 ; j <= ForwardInteractionEdgeList(LoopType)[i].NumberOfVortexEdges() ; j++ ) {
          
          if ( EdgeIsUsed[cpu][ForwardInteractionEdgeList(LoopType)[i].SurfaceVortexEdgeInteractionList(j)->VortexEdge()] == i ) {
                  
             NumEdges++;

             TempList.SurfaceVortexEdgeInteractionList()[NumEdges] = ForwardInteractionEdgeList_[LoopType][i].SurfaceVortexEdgeInteractionList()[j];
             
             EdgeIsUsed[cpu][ForwardInteractionEdgeList(LoopType)[i].SurfaceVortexEdgeInteractionList(j)->VortexEdge()] = 0;            
             
          }
    
       }         
       
       ForwardInteractionEdgeList(LoopType)[i].Edge() = TempList.Edge();

       ForwardInteractionEdgeList(LoopType)[i].Level() = TempList.Level();

       ForwardInteractionEdgeList(LoopType)[i].ReSizeEdgeList(NumEdges);         
      
       for ( j = 1 ; j <= ForwardInteractionEdgeList(LoopType)[i].NumberOfVortexEdges() ; j++ ) {
      
          ForwardInteractionEdgeList_[LoopType][i].SurfaceVortexEdgeInteractionList()[j] = TempList.SurfaceVortexEdgeInteractionList()[j];
      
       }   
   
    }

    for ( cpu = 0 ; cpu < NumberOfThreads ; cpu++ ) {

       delete [] EdgeIsUsed[cpu];
       
    }

    delete [] EdgeIsUsed;
   
}

/*##############################################################################
#                                                                              #
#                          FAST_MATRIX CreateAdjointEdgeList                   #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::CreateAdjointEdgeList(int NumberOfThreads, int MGLevel, int LoopType, VSP_GEOM &VSPGeom)
{

    int i, k, m, Loop, Edge, Level, GlobalEdge, *EdgeIsUsed, NumberOfEdges, MaxInteractionEdges, OldSize, NewSize;
    LOOP_INTERACTION_ENTRY *TempList;

    // Temp list for each edge interaction list

    MaxInteractionEdges = 0;
    
    for ( Level = 1 ; Level <= VSPGeom.NumberOfGridLevels() ; Level++ ) {
       
       MaxInteractionEdges += VSPGeom.Grid(Level).NumberOfEdges();
       
    }    
    
    TempList = new LOOP_INTERACTION_ENTRY[MaxInteractionEdges + 1];

    EdgeIsUsed = new int[MaxInteractionEdges + 1];
    
    zero_int_array(EdgeIsUsed, MaxInteractionEdges);

    // Loop over each level

    for ( i = 1 ; i <= NumberOfAdjointInteractionLoops(LoopType) ; i++ ) {

       Loop = AdjointInteractionLoopList(LoopType)[i].Loop();
       
       Level = AdjointInteractionLoopList(LoopType)[i].Level();

       // Mark the edges that are used and how many interactions edges they have
    
       for ( k = 1 ; k <= VSPGeom.Grid(Level).LoopList(Loop).NumberOfEdges() ; k++ ) {
    
          Edge = VSPGeom.Grid(Level).LoopList(Loop).Edge(k);
          
          GlobalEdge = VSPGeom.Grid(Level).EdgeList(Edge).VortexEdge(); 
          
          // First time we've seen this edge
     
          if ( EdgeIsUsed[GlobalEdge] == 0 ) {
             
             TempList[GlobalEdge].Edge() = Edge;
             
             TempList[GlobalEdge].Level() = Level;
             
             TempList[GlobalEdge].SizeEdgeList(AdjointInteractionLoopList(LoopType)[i].NumberOfVortexEdges());
             
             for ( m = 1 ; m <= AdjointInteractionLoopList(LoopType)[i].NumberOfVortexEdges() ; m++ ) {
                
                TempList[GlobalEdge].SurfaceVortexEdgeInteractionList()[m] = AdjointInteractionLoopList(LoopType)[i].SurfaceVortexEdgeInteractionList(m);
                
             }
          
             EdgeIsUsed[GlobalEdge]++;
             
          }
          
          // We've seen this edge before... so we need to merge the new and previous interaction list... only 
          // keeping the unique edges
          
          else if ( EdgeIsUsed[GlobalEdge] == 1 ) {
             
             OldSize = TempList[GlobalEdge].NumberOfVortexEdges();
             
             NewSize = OldSize + AdjointInteractionLoopList(LoopType)[i].NumberOfVortexEdges();
             
             TempList[GlobalEdge].ReSizeEdgeList(NewSize);
             
             for ( m = 1 ; m <= AdjointInteractionLoopList(LoopType)[i].NumberOfVortexEdges() ; m++ ) {
                
                TempList[GlobalEdge].SurfaceVortexEdgeInteractionList()[OldSize + m] = AdjointInteractionLoopList(LoopType)[i].SurfaceVortexEdgeInteractionList(m);
                
             }
          
             EdgeIsUsed[GlobalEdge]++;
                 
          }
          
          else {
             
             printf("How did we get here! An edge should only show up twice! \n");
             
             printf("EdgeIsUsed[GlobalEdge]: %d \n",EdgeIsUsed[GlobalEdge]);
             
             fflush(NULL);exit(1);

          }                

       }

    }

    // Determine number of edges
    
    NumberOfEdges = 0;
    
    for ( i = 1 ; i <= MaxInteractionEdges ; i++ ) {

       if ( EdgeIsUsed[i] ) NumberOfEdges++;

    }    

    SizeAdjointEdgeList(LoopType, NumberOfEdges);
    
    // Now pack the new lists and clean them of duplicates
  
    NumberOfEdges = 0;
    
    for ( i = 1 ; i <= MaxInteractionEdges ; i++ ) {

       if ( EdgeIsUsed[i] ) {
          
          NumberOfEdges++;
          
          AdjointInteractionEdgeList(LoopType)[NumberOfEdges].Edge() = TempList[i].Edge();
          
          AdjointInteractionEdgeList(LoopType)[NumberOfEdges].Level() = TempList[i].Level();

          AdjointInteractionEdgeList(LoopType)[NumberOfEdges].UseEdgeList(TempList[i].NumberOfVortexEdges(), TempList[i].SurfaceVortexEdgeInteractionList());
          
       }

    }    
    
    delete [] EdgeIsUsed;

    CleanAdjointEdgeList(LoopType);

    double SpeedUpRatio;
    
    SpeedUpRatio = 0.;

    for ( i = 1 ; i <= NumberOfAdjointInteractionEdges(LoopType) ; i++ ) {

       SpeedUpRatio += double(VSPGeom.Grid(MGLevel).NumberOfEdges())/double(AdjointInteractionEdgeList(LoopType)[i].NumberOfVortexEdges());
       
    }
    
    SpeedUpRatio /= VSPGeom.Grid(MGLevel).NumberOfEdges();

}

/*##############################################################################
#                                                                              #
#                          FAST_MATRIX CleanAdjointEdgeList                    #
#                                                                              #
##############################################################################*/

void FAST_MATRIX::CleanAdjointEdgeList(int LoopType)
{

    int i, j, NumEdges, *Perm, Edge1, Edge2;
    MERGESORT MergeSort;
    LOOP_INTERACTION_ENTRY TempList;

    for ( i = 1 ; i <= NumberOfAdjointInteractionEdges(LoopType) ; i++ ) {

       Perm = MergeSort.Sort(AdjointInteractionEdgeList(LoopType)[i].NumberOfVortexEdges(),AdjointInteractionEdgeList(LoopType)[i].SurfaceVortexEdgeInteractionList());

       // Determine how many unique edges there are
       
       NumEdges = 1;
      
       for ( j = 2 ; j <= AdjointInteractionEdgeList(LoopType)[i].NumberOfVortexEdges() ; j++ ) {
          
          Edge1 = AdjointInteractionEdgeList(LoopType)[i].SurfaceVortexEdgeInteractionList(Perm[j-1])->VortexEdge();
          Edge2 = AdjointInteractionEdgeList(LoopType)[i].SurfaceVortexEdgeInteractionList(Perm[j  ])->VortexEdge();
          
          if ( Edge1 != Edge2 ) NumEdges++;

       }

       TempList.Edge() = AdjointInteractionEdgeList(LoopType)[i].Edge();
       
       TempList.Level() = AdjointInteractionEdgeList(LoopType)[i].Level();
       
       TempList.SizeEdgeList(NumEdges);
                          
       NumEdges = 1;
       
       TempList.SurfaceVortexEdgeInteractionList()[NumEdges] = AdjointInteractionEdgeList_[LoopType][i].SurfaceVortexEdgeInteractionList()[Perm[1]];
       
       for ( j = 2 ; j <= AdjointInteractionEdgeList(LoopType)[i].NumberOfVortexEdges() ; j++ ) {
          
          Edge1 = AdjointInteractionEdgeList(LoopType)[i].SurfaceVortexEdgeInteractionList(Perm[j-1])->VortexEdge();
          Edge2 = AdjointInteractionEdgeList(LoopType)[i].SurfaceVortexEdgeInteractionList(Perm[j  ])->VortexEdge();
                    
          if ( Edge1 != Edge2 ) {
                  
             NumEdges++;

             TempList.SurfaceVortexEdgeInteractionList()[NumEdges] = AdjointInteractionEdgeList_[LoopType][i].SurfaceVortexEdgeInteractionList()[Perm[j]];
             
          }
    
       }         
       
       AdjointInteractionEdgeList(LoopType)[i].Edge() = TempList.Edge();

       AdjointInteractionEdgeList(LoopType)[i].Level() = TempList.Level();

       AdjointInteractionEdgeList(LoopType)[i].ReSizeEdgeList(NumEdges);         
      
       for ( j = 1 ; j <= AdjointInteractionEdgeList(LoopType)[i].NumberOfVortexEdges() ; j++ ) {
      
          AdjointInteractionEdgeList_[LoopType][i].SurfaceVortexEdgeInteractionList()[j] = TempList.SurfaceVortexEdgeInteractionList()[j];
      
       }       
                  
       delete [] Perm;
       
    }
   
}

#include "END_NAME_SPACE.H"

