/*##############################################################################
#                                                                              #
#                        VSP_SOLVER UpdateSpanLoadData                         #
#                                                                              #
##############################################################################*/

void VSP_SOLVER::UpdateSpanLoadData(void)
{
 
    int j, k, p, Node1, Node2;
    VSPAERO_DOUBLE S;
    
    // Estimate the local chord
    
    S = 0.;
   
    for ( k = 1 ; k <= NumberOfVortexSheets_ ; k++ ) {
    
       for ( j = 1 ; j < VortexSheet(k).NumberOfTrailingVortices() ; j++ ) {

          Node1 = VortexSheet(k).TrailingVortex(j  ).Node();
          Node2 = VortexSheet(k).TrailingVortex(j+1).Node();
           
          Vec1[0] = 0.5*( VSPGeom().Grid(1).NodeList(Node1).x() + VSPGeom().Grid(1).NodeList(Node2).x() );
          Vec1[1] = 0.5*( VSPGeom().Grid(1).NodeList(Node1).y() + VSPGeom().Grid(1).NodeList(Node2).y() );
          Vec1[2] = 0.5*( VSPGeom().Grid(1).NodeList(Node1).z() + VSPGeom().Grid(1).NodeList(Node2).z() );
          
          if ( j == 1 ) {
             
             S = 0.;
             
             Vec0[0] = Vec1[0];
             Vec0[1] = Vec1[1];
             Vec0[2] = Vec1[2];
             
          }
          
          else {
             
             Vec0[0] -= Vec1[0];
             Vec0[1] -= Vec1[1];
             Vec0[2] -= Vec1[2];
             
             S += sqrt(vector_dot(Vec0,Vec0));
             
             Vec0[0] = Vec1[0];
             Vec0[1] = Vec1[1];
             Vec0[2] = Vec1[2];                
             
          }
          
          for ( p = 1 ; p <= NumberOfVortexLoops_ ; p++ ) {
             
             if ( VortexLoop(p).VortexSheet() == k && VortexLoop(p).SpanStation() == j ) {
                
                Vec2[0] = VortexLoop(p).Xc() - Vec1[0]; // XLE - XTE
                Vec2[1] = VortexLoop(p).Yc() - Vec1[1]; // YLE - YTE
                Vec2[2] = VortexLoop(p).Zc() - Vec1[2]; // ZLE - ZTE
       
                Mag = sqrt(vector_dot(Vec2,Vec2));
                
                if ( Mag >= SpanLoadData(k).Span_Chord(j) ) {
                   
                   // Store the component ID... 
                  
                   SpanLoadData(k).ComponentID(j) = VSPGeom().Grid(1).NodeList(Node1).ComponentID();
                   
                   // Store chord and direction vector
             
                   SpanLoadData(k).Span_Chord(j) = Mag;
                   
                   SpanLoadData(k).Span_Svec(j)[0] = -Vec2[0] / Mag;
                   SpanLoadData(k).Span_Svec(j)[1] = -Vec2[1] / Mag;
                   SpanLoadData(k).Span_Svec(j)[2] = -Vec2[2] / Mag;    
                   
                   SpanLoadData(k).Span_S(j) = S;

                   // TE location

                   SpanLoadData(k).Span_XTE(j) = Vec1[0];
                   SpanLoadData(k).Span_YTE(j) = Vec1[1];                      
                   SpanLoadData(k).Span_ZTE(j) = Vec1[2];                      
                   
                   // Calculate normal to strip
                   
                   VecS[0] = SpanLoadData(k).Span_Svec(j)[0];
                   VecS[1] = SpanLoadData(k).Span_Svec(j)[1];
                   VecS[2] = SpanLoadData(k).Span_Svec(j)[2];

                   VecT[0] = VSPGeom().Grid(1).NodeList(Node2).x() - VSPGeom().Grid(1).NodeList(Node1).x();
                   VecT[1] = VSPGeom().Grid(1).NodeList(Node2).y() - VSPGeom().Grid(1).NodeList(Node1).y();
                   VecT[2] = VSPGeom().Grid(1).NodeList(Node2).z() - VSPGeom().Grid(1).NodeList(Node1).z();

                   vector_cross(VecS, VecT, VecN);
                   
                   Mag = sqrt(vector_dot(VecN,VecN));
                   
                   SpanLoadData(k).Span_Nvec(j)[0] = VecN[0] / Mag;
                   SpanLoadData(k).Span_Nvec(j)[1] = VecN[1] / Mag; 
                   SpanLoadData(k).Span_Nvec(j)[2] = VecN[2] / Mag;     

                }
                
             }
             
          }
          
       }
       
    }
  
    // Create Span location data
    
    for ( k = 0 ; k <= NumberOfSpanLoadDataSets_ ; k++ ) {
    
       if ( SpanLoadData(k).NumberOfSpanStations() > 1 ) {
             
          S1 = SpanLoadData(k).Span_S(1);
          S2 = SpanLoadData(k).Span_S(SpanLoadData(k).NumberOfSpanStations());

          if ( S1 != S2 ) {
                          
            for ( j = 1 ; j <= SpanLoadData(k).NumberOfSpanStations() ; j++ ) {
               
               SpanLoadData(k).Span_S(j) = ( SpanLoadData(k).Span_S(j) - S1 )/(S2 - S1);
               
            }
            
          }
          
          else {
             
            for ( j = 1 ; j <= SpanLoadData(k).NumberOfSpanStations() ; j++ ) {
               
               SpanLoadData(k).Span_S(j) = 0.;
                                 
            }
            
          }                
          
       }
       
       else {
          
          SpanLoadData(k).Span_S(1) = 0.;
          
          SpanLoadData(k).Span_Chord(1) = 1.;
          
       }
       
    }  

    // Accumulate areas, etc
           
    for ( p = 1 ; p <= NumberOfVortexLoops_ ; p++ ) {
       
       k = VortexLoop(p).VortexSheet();
       
       j = VortexLoop(p).SpanStation();
       
       if ( k == 0 && j == 0 ) j = VortexLoop(p).SpanStation() = 1;
      
       SpanLoadData(k).Span_Area(j) += VortexLoop(p).Area();
   
    }
    
}
