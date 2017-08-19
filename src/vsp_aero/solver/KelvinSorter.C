    printf("There are: %10d Vortex Sheets \n", NumberOfVortexSheets);

    for ( k = 1 ; k <= NumberOfVortexSheets ; k++ ) {
       
       // Only check periodic wakes
       
       if ( VortexSheet(k).IsPeriodic() ) {

          // Grab the first kutta node on this Vortex Sheet
       
          Node = VortexSheet(k).TrailingVortexEdge(1).Node();
      
          // Grab a loop that contains this kutta node
         
          FoundOne = 0;
         
          n = 1;
         
          while ( !FoundOne ) {
            
             p = 1;
             
             while ( p <= VSPGeom().Grid(MGLevel_).LoopList(n).NumberOfEdges() && !FoundOne ) {
      
                Edge = VSPGeom().Grid(MGLevel_).LoopList(n).Edge(p);
               
                Node1 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Node1();
                Node2 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Node2();
               
                if ( Node == Node1 || Node == Node2 ) {
                  
                   Loop = n;
                  
                   FoundOne = 1;
                  
                }
               
                p++;
               
             }
            
             n++;
            
          }
         
          if ( !FoundOne ) {
            
             printf("Error in determining number of Kelvin regions for a periodic wake surface! \n");
             fflush(NULL);
             exit(1);
            
          }
         
          // Go edge, by edge and see if we cover the entire kelvin region
         
          zero_int_array(LoopStack, NumberOfVortexLoops_);
         
          StackSize = Next = 1;
          
          LoopStack[Next] = Loop;
         
          KelvinGroup = LoopInKelvinConstraintGroup_[Loop];
             
          while ( Next <= StackSize ) {
             
             Loop = LoopStack[Next];
             
             for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).LoopList(Loop).NumberOfEdges() ; p++ ) {
                
                Edge = VSPGeom().Grid(MGLevel_).LoopList(Loop).Edge(p);
               
                if ( !VSPGeom().Grid(MGLevel_).EdgeList(Edge).IsTrailingEdge() ) {
               
                   Loop1 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Loop1();
                   
                   Loop2 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Loop2();
                   
                   if ( LoopInKelvinConstraintGroup_[Loop1] == KelvinGroup ) {
                      
                      LoopInKelvinConstraintGroup_[Loop1] = -KelvinGroup;
                      
                      LoopStack[++StackSize] = Loop1;
                      
                   }
                  
                   else if ( LoopInKelvinConstraintGroup_[Loop1] != -KelvinGroup ){
                     
                      printf("WTF... how did we jump to another Kelvin Group... \n"); fflush(NULL);
                      exit(1);
                      
                   }
                   
                   if ( LoopInKelvinConstraintGroup_[Loop2] == KelvinGroup ) {
                      
                      LoopInKelvinConstraintGroup_[Loop2] = -KelvinGroup;
                      
                      LoopStack[++StackSize] = Loop2;
              
                   }    
                  
                   else if ( LoopInKelvinConstraintGroup_[Loop1] != -KelvinGroup ){
                     
                      printf("WTF... how did we jump to another Kelvin Group... \n"); fflush(NULL);
                      exit(1);
                     
                   }    
                  
                }                       
                 
             }
             
             Next++;   
     
          }
            
          // Check and see if there are any loops in this Kelvin group that were not flipped
         
          NotFlipped = 0;
         
          for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                  
             if ( LoopInKelvinConstraintGroup_[p] == KelvinGroup ) {
               
                NotFlipped++;
               
             }
            
          }
            
          // If not all were flipped... then there is a base region
         
          if ( NotFlipped > 0 ) {
                              
             printf("Base region found for vortex sheet system: %d \n",k);fflush(NULL);
                                        
             // Determine which region ... + or - ... is the base region
            
             Vec1[0] = Vec1[1] = Vec1[2] = 0.;
            
             Vec2[0] = Vec2[1] = Vec2[2] = 0.;

             for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                     
                if ( LoopInKelvinConstraintGroup_[p] ==  KelvinGroup ) {
                  
                   Vec1[0] += VSPGeom().Grid(MGLevel_).LoopList(p).Nx();
                   Vec1[1] += VSPGeom().Grid(MGLevel_).LoopList(p).Ny();
                   Vec1[2] += VSPGeom().Grid(MGLevel_).LoopList(p).Nz();
                  
                }
               
                if ( LoopInKelvinConstraintGroup_[p] == -KelvinGroup ) {
                  
                   Vec2[0] += VSPGeom().Grid(MGLevel_).LoopList(p).Nx();
                   Vec2[1] += VSPGeom().Grid(MGLevel_).LoopList(p).Ny();
                   Vec2[2] += VSPGeom().Grid(MGLevel_).LoopList(p).Nz();
                  
                }                     
               
             }
            
             Dot = sqrt(vector_dot(Vec1,Vec1));
            
             Vec1[0] /= Dot; 
             Vec1[1] /= Dot; 
             Vec1[2] /= Dot; 
            
             Dot = sqrt(vector_dot(Vec2,Vec2));
            
             Vec2[0] /= Dot; 
             Vec2[1] /= Dot; 
             Vec2[2] /= Dot;
            
             // Remove those loops in the base region from the Kelvin group, and add them
             // to the base region list
            
             if ( Vec1[0] > Vec2[0] ) {
           
                for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                     
                   if ( LoopInKelvinConstraintGroup_[p] == KelvinGroup ) {
                     
                      LoopIsOnBaseRegion_[p] = 1;
                     
                      LoopInKelvinConstraintGroup_[p] = KelvinGroup;
              
                   }
                  
                }
               
                for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                     
                   if ( LoopInKelvinConstraintGroup_[p] == -KelvinGroup ) {
                  
                      LoopInKelvinConstraintGroup_[p] = KelvinGroup;
              
                   }
                  
                }                     
               
             }
            
             else {
       
                for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                     
                   if ( LoopInKelvinConstraintGroup_[p] == -KelvinGroup ) {
                     
                      LoopIsOnBaseRegion_[p] = 1;
                     
                      LoopInKelvinConstraintGroup_[p] = KelvinGroup;
                     
                   }
                  
                }
               
                for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                     
                   if ( LoopInKelvinConstraintGroup_[p] == KelvinGroup ) {
                   
                      LoopInKelvinConstraintGroup_[p] = KelvinGroup;
              
                   }
                  
                }                        
               
             }  
                   
          }
         
       }                 
       
    }

    printf("There are %d Kelvin constraints \n",NumberOfKelvinConstraints_);
    
    delete [] LoopStack;
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
old code:

   // Check if there are any periodic wakes that might break a region in two

    NumberOfVortexSheets = 0;
  
    for ( j = 1 ; j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() ; j++ ) {

       NumberOfVortexSheets = MAX(NumberOfVortexSheets, VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNode(j));
       
    }    
    
    printf("There are: %10d Vortex Sheets \n", NumberOfVortexSheets);

//djk... bug in this code!
    
    for ( k = 10000 ; k <= NumberOfVortexSheets ; k++ ) {
printf("k: %d \n",k);fflush(NULL);    
       Done = 0;
       
       j = 1;
       
       while ( j <= VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes() && !Done ) {
          
          // Search for a kutta node on this vortex sheet
          
          if ( VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNode(j) == k ) {
          
             // If it's periodic... we must check if it creates a closed region
printf("j: %d  out of: %d \n",j,VSPGeom().Grid(MGLevel_).NumberOfKuttaNodes());fflush(NULL);             
             if ( VSPGeom().Grid(MGLevel_).WingSurfaceForKuttaNodeIsPeriodic(j) ) {

               Node = VSPGeom().Grid(MGLevel_).KuttaNode(j);
               
               // Grab a loop that contains this kutta node
               
               FoundOne = 0;
               
               n = 1;
               
               while ( !FoundOne ) {
                  
                  p = 1;
                  
                  while ( p <= VSPGeom().Grid(MGLevel_).LoopList(n).NumberOfEdges() && !FoundOne ) {

                     Edge = VSPGeom().Grid(MGLevel_).LoopList(n).Edge(p);
                     
                     Node1 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Node1();
                     Node2 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Node2();
                     
                     if ( Node == Node1 || Node == Node2 ) {
                        
                        Loop = n;
                        
                        FoundOne = 1;
                        
                     }
                     
                     p++;
                     
                  }
                  
                  n++;
                  
               }
         
               // Go edge, by edge and see if we cover the entire kelvin region
               
               zero_int_array(LoopStack, NumberOfVortexLoops_);
               
               StackSize = Next = 1;
                
               LoopStack[Next] = Loop;
               
               KelvinGroup = LoopInKelvinConstraintGroup_[Loop];
                   
               while ( Next <= StackSize ) {
                   
                  Loop = LoopStack[Next];
                   
                  for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).LoopList(Loop).NumberOfEdges() ; p++ ) {
                      
                     Edge = VSPGeom().Grid(MGLevel_).LoopList(Loop).Edge(p);
                     
                     if ( !VSPGeom().Grid(MGLevel_).EdgeList(Edge).IsTrailingEdge() ) {
                     
                        Loop1 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Loop1();
                         
                        Loop2 = VSPGeom().Grid(MGLevel_).EdgeList(Edge).Loop2();
                         
                        if ( LoopInKelvinConstraintGroup_[Loop1] == KelvinGroup ) {
                            
                           LoopInKelvinConstraintGroup_[Loop1] = -KelvinGroup;
                            
                           LoopStack[++StackSize] = Loop1;
                            
                        }
                        
                        else if ( LoopInKelvinConstraintGroup_[Loop1] != -KelvinGroup ){
                           
                           printf("WTF... how did we jump to another Kelvin Group... \n"); fflush(NULL);
                           exit(1);
                           
                        }
                         
                        if ( LoopInKelvinConstraintGroup_[Loop2] == KelvinGroup ) {
                            
                           LoopInKelvinConstraintGroup_[Loop2] = -KelvinGroup;
                            
                           LoopStack[++StackSize] = Loop2;
                    
                        }    
                        
                        else if ( LoopInKelvinConstraintGroup_[Loop1] != -KelvinGroup ){
                           
                           printf("WTF... how did we jump to another Kelvin Group... \n"); fflush(NULL);
                           exit(1);
                           
                        }    
                        
                     }                       
                       
                  }
                   
                  Next++;   
           
               }
               
               // Check and see if there are any loops in this Kelvin group that were not flipped
               
               NotFlipped = 0;
               
               for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                        
                  if ( LoopInKelvinConstraintGroup_[p] == KelvinGroup ) {
                     
                     NotFlipped++;
                     
                  }
                  
               }
               
               // If not all were flipped... then there is a base region
               
               if ( NotFlipped > 0 ) {
                                    
                  printf("Base region found for vortex sheet system: %d \n",k);fflush(NULL);
                                              
                  // Determine which region ... + or - ... is the base region
                  
                  Vec1[0] = Vec1[1] = Vec1[2] = 0.;
                  
                  Vec2[0] = Vec2[1] = Vec2[2] = 0.;

                  for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                           
                     if ( LoopInKelvinConstraintGroup_[p] ==  KelvinGroup ) {
                        
                        Vec1[0] += VSPGeom().Grid(MGLevel_).LoopList(p).Nx();
                        Vec1[1] += VSPGeom().Grid(MGLevel_).LoopList(p).Ny();
                        Vec1[2] += VSPGeom().Grid(MGLevel_).LoopList(p).Nz();
                        
                     }
                     
                     if ( LoopInKelvinConstraintGroup_[p] == -KelvinGroup ) {
                        
                        Vec2[0] += VSPGeom().Grid(MGLevel_).LoopList(p).Nx();
                        Vec2[1] += VSPGeom().Grid(MGLevel_).LoopList(p).Ny();
                        Vec2[2] += VSPGeom().Grid(MGLevel_).LoopList(p).Nz();
                        
                     }                     
                     
                  }
                  
                  Dot = sqrt(vector_dot(Vec1,Vec1));
                  
                  Vec1[0] /= Dot; 
                  Vec1[1] /= Dot; 
                  Vec1[2] /= Dot; 
                  
                  Dot = sqrt(vector_dot(Vec2,Vec2));
                  
                  Vec2[0] /= Dot; 
                  Vec2[1] /= Dot; 
                  Vec2[2] /= Dot;
                  
                  // Remove those loops in the base region from the Kelvin group, and add them
                  // to the base region list
                  
                  if ( Vec1[0] > Vec2[0] ) {
                 
                     for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                           
                        if ( LoopInKelvinConstraintGroup_[p] == KelvinGroup ) {
                           
                           LoopIsOnBaseRegion_[p] = 1;
                           
                           LoopInKelvinConstraintGroup_[p] = KelvinGroup;
                    
                        }
                        
                     }
                     
                     for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                           
                        if ( LoopInKelvinConstraintGroup_[p] == -KelvinGroup ) {
                        
                           LoopInKelvinConstraintGroup_[p] = KelvinGroup;
                    
                        }
                        
                     }                     
                     
                  }
                  
                  else {
             
                     for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                           
                        if ( LoopInKelvinConstraintGroup_[p] == -KelvinGroup ) {
                           
                           LoopIsOnBaseRegion_[p] = 1;
                           
                           LoopInKelvinConstraintGroup_[p] = KelvinGroup;
                           
                        }
                        
                     }
                     
                     for ( p = 1 ; p <= VSPGeom().Grid(MGLevel_).NumberOfLoops() ; p++ ) {                  
                           
                        if ( LoopInKelvinConstraintGroup_[p] == KelvinGroup ) {
                         
                           LoopInKelvinConstraintGroup_[p] = KelvinGroup;
                    
                        }
                        
                     }                        
                     
                  }  
                         
               }                 
               
               Done = 1;
               
             }
             
          }
          
          j++;
          
          printf("and after..... j is: %d \n",j);
          printf("Done: %d \n",Done);
          
       }
       
    }

    printf("There are %d Kelvin constraints \n",NumberOfKelvinConstraints_);
    
    delete [] LoopStack;    
    
