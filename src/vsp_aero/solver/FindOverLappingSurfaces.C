/*##############################################################################
#                                                                              #
#                       VSP_SOLVER FindOverLappingSurfaces                     #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::FindOverLappingSurfaces(void)
{

    int body, wing, ib, iw, k;

    // Loop over surfaces

    for ( body = 1 ; body <= VSPGeom().NumberOfSurfaces() ; body++ ) { 
     
       // Look at body surfaces
       
       if ( VSPGeom().VSP_Surface(body).SurfaceType() == DEGEN_BODY_SURFACE ) {
          
          for ( wing = 1 ; wing <= VSPGeom().NumberOfSurfaces() ; wing++ ) {
          
             // Look at wing surfaces
             
             if ( VSPGeom().VSP_Surface(wing).SurfaceType() == DEGEN_WING_SURFACE ) {
                
                // Find the body loops
                
                for ( ib = 1 ; ib <= NumberOfVortexLoops_ ; ib++ ) {
                   
                   if ( VortexLoop(ib).SurfaceID() == body ) {
                   
                      // Find the wing loops
                      
                      for ( iw = 1 ; iw <= NumberOfVortexLoops_ ; iw++ ) {
                      
                         if ( VortexLoop(iw).SurfaceID() == wing ) {
                            
                            // Compare these two loops for overlap
                            
                            if ( LoopsOverLap(ib, iw) ) {
                               
                               VortexLoop(ib).dCp() = 0.;
                               VortexLoop(iw).dCp() = 0.;
                               
                               for ( k = 1 ; k <= VortexLoop(ib).NumberOfEdges() ; k++ ) {
                         
                                  SurfaceVortexEdge(VortexLoop(ib).Edge(k)).Fx() = 0.;
                                  SurfaceVortexEdge(VortexLoop(ib).Edge(k)).Fy() = 0.;
                                  SurfaceVortexEdge(VortexLoop(ib).Edge(k)).Fz() = 0.;
                                  
                                  SurfaceVortexEdge(VortexLoop(ib).Edge(k)).Trefftz_Fx() = 0.;
                                  SurfaceVortexEdge(VortexLoop(ib).Edge(k)).Trefftz_Fx() = 0.;
                                  SurfaceVortexEdge(VortexLoop(ib).Edge(k)).Trefftz_Fx() = 0.;
                                                                    
                               }
                               
                               for ( k = 1 ; k <= VortexLoop(iw).NumberOfEdges() ; k++ ) {
                         
                                  SurfaceVortexEdge(VortexLoop(iw).Edge(k)).Fx() = 0.;
                                  SurfaceVortexEdge(VortexLoop(iw).Edge(k)).Fy() = 0.;
                                  SurfaceVortexEdge(VortexLoop(iw).Edge(k)).Fz() = 0.;
                                  
                                  SurfaceVortexEdge(VortexLoop(iw).Edge(k)).Trefftz_Fx() = 0.;
                                  SurfaceVortexEdge(VortexLoop(iw).Edge(k)).Trefftz_Fx() = 0.;
                                  SurfaceVortexEdge(VortexLoop(iw).Edge(k)).Trefftz_Fx() = 0.;
                                                                    
                               }        
                                       
                            }
                            
                         }
                         
                      }
                      
                   }
                   
                }
                
             }
             
          }
          
       }
       
    }

}

/*##############################################################################
#                                                                              #
#                          VSP_SOLVER LoopsOverLap                             #
#                                                                              #
##############################################################################*/

int VSP_SOLVER::LoopsOverLap(int BodyLoop, int WingLoop)
{
   
    double Distance, MinDistance, PlanarDistance, Vec[3];

    // Distance between centroids
    
    Vec[0] = VortexLoop(BodyLoop).Xc() - VortexLoop(WingLoop).Xc();
    Vec[1] = VortexLoop(BodyLoop).Yc() - VortexLoop(WingLoop).Yc();
    Vec[2] = VortexLoop(BodyLoop).Zc() - VortexLoop(WingLoop).Zc();
    
    Distance = sqrt(vector_dot(Vec,Vec));
    
    // Distance bewteen two planes
    
    PlanarDistance = vector_dot(Vec,VortexLoop(BodyLoop).Normal());

    if ( PlanarDistance <= 0.01 ) {
       
       // In plane distance
       
       MinDistance = MIN( sqrt(VortexLoop(BodyLoop).Area()), sqrt(VortexLoop(WingLoop).Area()) );
       
       if ( Distance <= 2.*MinDistance ) {
 
          return 1;
          
       }
       
    }
    
    return 0;
 
}

