

//==== Init Is Called Once During Each Custom Geom Construction  ============================//
//==== Avoid Global Variables Unless You Want Shared With All Custom Geoms of This Type =====//
void Init()
{
	//==== Add Parm Types  =====//
	string height   = AddParm( PARM_DOUBLE_TYPE, "Height", "Design" );
	SetParmVal( height, 4.0 );

	string diameter = AddParm( PARM_DOUBLE_TYPE, "Diameter", "Design" );
	SetParmVal( diameter, 8.0 );

	//==== Add Cross Sections  =====//
	string xsec_surf = AddXSecSurf();
	AppendXSec( xsec_surf, XS_POINT);
	AppendXSec( xsec_surf, XS_CIRCLE);
	AppendXSec( xsec_surf, XS_POINT);
}

//==== Global Gui IDs - These Are Consistent For All Created Boxes And Used In UpdateGUI====// 
int DesignTab;
int HeightSlider;
int DiameterSlider;

//==== InitGui Is Called Once During Each Custom Geom Construction ====//
void InitGui()
{
	DesignTab = AddGui( GDEV_TAB, "Design"  );
	AddGui( GDEV_YGAP, ""  );
	AddGui( GDEV_DIVIDER_BOX, "Design" );
	HeightSlider = AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Height"  );
	DiameterSlider = AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Diameter"  );
	AddGui( GDEV_YGAP, ""  );

} 

//==== UpdateGui Is Called Every Time The Gui is Updated ====//
void UpdateGui()
{
	string geom_id = GetCurrCustomGeom();
	string dia = GetParm( geom_id, "Diameter", "Design" );
	string ht  = GetParm( geom_id, "Height", "Design" );

	UpdateGui( HeightSlider, ht ); 
	UpdateGui( DiameterSlider, dia ); 
} 

//==== UpdateSurf Is Called Every Time The Geom is Updated ====//
void UpdateSurf()
{
	string geom_id = GetCurrCustomGeom();

	//==== Set Base XSec Diameter ====//
	string dia_parm = GetParm( geom_id, "Diameter", "Design" );
	double dia_val  = GetParmVal( dia_parm );

	//==== Get The XSec To Change ====//
	string xsec_surf = GetXSecSurf( geom_id, 0 );
	string xsec0 = GetXSec( xsec_surf, 0 );
	SetParmVal( GetXSecParm( xsec0, "XLocPercent" ), 0.0 );

	//==== Set The Diameter ====//
	string xsec1 = GetXSec( xsec_surf, 1 );
	string xsec1_dia = GetXSecParm( xsec1, "Circle_Diameter" );
	SetParmVal( xsec1_dia, dia_val );

	//==== Set The Height ====//
	double ht_val  = GetParmVal( GetParm( geom_id, "Height", "Design" ) );
	string xsec2 = GetXSec( xsec_surf, 2 );
	SetParmVal( GetXSecParm( xsec2, "XLocPercent" ), 1.0 );
	SetParmVal( GetXSecParm( xsec2, "RefLength" ), ht_val );

	SkinXSecSurf();

}

