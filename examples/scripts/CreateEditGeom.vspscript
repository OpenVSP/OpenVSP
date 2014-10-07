
void main()
{
    string fid = AddGeom( "TransportFuse", "" );             // Add Fuselage
    string len_id = GetParm( fid, "Length", "Design" );
    SetParmVal( len_id, 30.0 );
    SetParmVal( GetParm( fid, "Diameter", "Design" ), 4.0 );

    string wid = AddGeom( "WING", "" );                     // Add Wing
    SetParmVal( GetParm( wid, "Span", "XSec_1" ), 15.0 );
    SetParmVal( GetParm( wid, "Root_Chord", "XSec_1" ), 10.0 );
    SetParmVal( GetParm( wid, "Tip_Chord", "XSec_1" ), 3.0 );
    SetParmVal( GetParm( wid, "Sweep", "XSec_1" ), 45.0 );
    SetParmVal( GetParm( wid, "X_Rel_Location", "XForm" ), 8.0 );

    string xsec_surf = GetXSecSurf( wid, 0 );
    while ( GetNumXSec(xsec_surf) > 2 )
    {
        CutXSec( xsec_surf, GetNumXSec(xsec_surf)-1 );
    }

    //CopyGeomToClipBoard( fid );
    //PasteGeomClipBoard();                              

    Update();

	//==== Check For API Errors ====//
	while ( GetNumTotalErrors() > 0 )
	{
		ErrorObj err = PopLastError();
		Print( err.GetErrorString() );
	}

}