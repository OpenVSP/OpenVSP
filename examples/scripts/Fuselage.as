
void main()
{
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage
	string xsec_surf = GetXSecSurf( fid, 0 );           // Get First (and Only) XSec Surf

	int num_xsecs = GetNumXSec( xsec_surf );    
 	for ( int i = 0 ; i < num_xsecs ; i++ )
 	{
		string xsec = GetXSec( xsec_surf, i );      
 		SetXSecContinuity( xsec, 1 );                       // Set Continuity At Cross Section
 		SetXSecTanAngles( xsec, XSEC_BOTH_SIDES, 0 );       // Set Tangent Angles At Cross Section
 		SetXSecTanStrengths( xsec, XSEC_BOTH_SIDES, 0.5 );  // Set Tangent Strengths At Cross Section
 	}
    //==== Set Tan Angles At Nose/Tail
 	SetXSecTanAngles( GetXSec( xsec_surf, 0 ), XSEC_BOTH_SIDES, 90 );           
 	SetXSecTanAngles( GetXSec( xsec_surf, num_xsecs-1 ), XSEC_BOTH_SIDES, -90 );

 	Update();       // Force Surface Update

    //==== Run CompGeom And View Results ====//
	string mesh_id = ComputeCompGeom( SET_ALL, false, 0 );                      // Half Mesh false and no file export
	string comp_res_id = FindLatestResultsID( "Comp_Geom" );                    // Find Results ID
	array<double> @double_arr = GetDoubleResults( comp_res_id, "Wet_Area" );    // Extract Results
    Print( "Wet_Area = ", false );                                              // Print Results
    Print( double_arr[0] );


}