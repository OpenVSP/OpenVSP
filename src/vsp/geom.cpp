//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// geom.cpp: implementation of the geom class.
//
//////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include <windows.h>		
#endif

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#endif

#include "geom.h"
#include "aircraft.h"
#include "matrix.h"
#include "materialMgr.h"
#include "textureMgr.h"
#include "timer.h"
#include "bezier_surf.h"
#include "VspPreferences.h"
#include "part.h"
#include "structureMgr.h"
#include "GridDensity.h"
#include "CfdMeshMgr.h"
#include "FeaMeshMgr.h"
#include "parmLinkMgr.h"

// Include OpenNurbs for Rhino Dump
// ON Needs to be undefined for it to compile
//
#undef ON
#include "opennurbs.h"
#include "opennurbs_extensions.h"


GeomBase::GeomBase()
{
	type = 0;
	name_str = Stringc("DefaultGeom");
	type_str = Stringc("DefaultType");
	color = vec3d( 0, 0, 255 );

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Geom::Geom( Aircraft* aptr ) : GeomBase()
{
	airPtr = aptr;
	parentGeom = 0;
	fastDrawFlag = 0;
	displayChildrenFlag = 1;
	m_WakeActiveFlag = false;

	//===== Set Id String (Time Stamp) =====
	Timer id_timer;
	id_timer.start();
	int time_stamp = id_timer.get_start_time();
	char str[255];
	sprintf(str, "%i", time_stamp);
	id_num = 0;
	id_str = str;
	ptrID = (long)this;
	massPrior = 0;
	shellFlag = 0;

	color = vec3d( 0, 0, 255 );

	//==== Set Some Reasonable Values For Bounding Box ====//
	bnd_box.update( vec3d(  0.0001, 0.0001, 0.0001 ) );
	bnd_box.update( vec3d( -0.0001, -0.0001, -0.0001 ) );

	bnd_box_xform = bnd_box;


 static float idmat[4][4] = 
   {1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0 };

   for ( int i = 0 ; i < 4 ; i++ )
    for ( int j = 0 ; j < 4 ; j++ )
    {
      reflect_mat[i][j] = idmat[i][j];
      model_mat[i][j] = idmat[i][j];
    }

	sym_code = NO_SYM;
	sym_vec = vec3d(1.0, 1.0, 1.0);

	materialID = 0;

	posAttachFlag = POS_ATTACH_NONE;

	noshowFlag = 0;
	displayFlag = GEOM_WIRE_FLAG;
	outputFlag = 1;
	outputNameID = 0;

	//==== Init Xform Parms ====//
	relXFormFlag = 0;
	xLoc.initialize(this, UPD_XFORM, "X Location", 0.0 );
    xLoc.set_lower_upper(-1000000.0, 1000000.0);

	yLoc.initialize(this, UPD_XFORM, "Y Location", 0.0 );
    yLoc.set_lower_upper(-1000000.0, 1000000.0);

	zLoc.initialize(this, UPD_XFORM, "Z Location", 0.0 );
    zLoc.set_lower_upper(-1000000.0, 1000000.0);

	xRelLoc.initialize(this, UPD_XFORM, "X Rel Location", 0.0 );
    xRelLoc.set_lower_upper(-1000000.0, 1000000.0);

	yRelLoc.initialize(this, UPD_XFORM, "Y Rel Location", 0.0 );
    yRelLoc.set_lower_upper(-1000000.0, 1000000.0);

	zRelLoc.initialize(this, UPD_XFORM, "Z Rel Location", 0.0 );
    zRelLoc.set_lower_upper(-1000000.0, 1000000.0);

	xLocOff.initialize(this, UPD_XFORM, "X Offset Location", 0.0 );
    xLocOff.set_lower_upper(-1000000.0, 1000000.0);

	yLocOff.initialize(this, UPD_XFORM, "Y Offset Location", 0.0 );
    yLocOff.set_lower_upper(-1000000.0, 1000000.0);

	zLocOff.initialize(this, UPD_XFORM, "Z Offset Location", 0.0 );
    zLocOff.set_lower_upper(-1000000.0, 1000000.0);

	xRot.initialize(this, UPD_XFORM, "X Rotation", 0.0 );
    xRot.set_lower_upper(-180.0, 180.0);
//	xRot.set_script("xform xrot", 0);

	yRot.initialize(this, UPD_XFORM, "Y Rotation", 0.0 );
    yRot.set_lower_upper(-180.0, 180.0);
//	yRot.set_script("xform yrot", 0);

	zRot.initialize(this, UPD_XFORM, "Z Rotation", 0.0 );
    zRot.set_lower_upper(-180.0, 180.0);
//	zRot.set_script("xform zrot", 0);

	xRelRot.initialize(this, UPD_XFORM, "X Rel Rotation", 0.0 );
    xRelRot.set_lower_upper(-180.0, 180.0);

	yRelRot.initialize(this, UPD_XFORM, "Y Rel Rotation", 0.0 );
    yRelRot.set_lower_upper(-180.0, 180.0);

	zRelRot.initialize(this, UPD_XFORM, "Z Rel Rotation", 0.0 );
    zRelRot.set_lower_upper(-180.0, 180.0);

	xRotOff.initialize(this, UPD_XFORM, "X Rotation Offset", 0.0 );
    xRotOff.set_lower_upper(-180.0, 180.0);

	yRotOff.initialize(this, UPD_XFORM, "Y Rotation Offset", 0.0 );
    yRotOff.set_lower_upper(-180.0, 180.0);

	zRotOff.initialize(this, UPD_XFORM, "Z Rotation Offset", 0.0 );
    zRotOff.set_lower_upper(-180.0, 180.0);

	origin.initialize(this, UPD_XFORM, "Origin", 0.0 );
    origin.set_lower_upper(0.0, 1.0);

	center.set_xyz( 0, 0, 0 );

	uAttach.initialize(this, UPD_XFORM, "U Attach", 0.0 );
    uAttach.set_lower_upper(0.0, 1.0);

	vAttach.initialize(this, UPD_XFORM, "V Attach", 0.0 );
    vAttach.set_lower_upper(0.0, 1.0);

	scaleFactor.initialize(this, UPD_XFORM, "Scale Factor", 1.0);
    scaleFactor.set_lower_upper(0.001, 1000.0);
	lastScaleFactor = scaleFactor();

	numPnts.initialize(this, UPD_NUM_PNT_XSEC, "Num Pnts", 21 );
    numPnts.set_lower_upper(9, 1001 );

	numXsecs.initialize(this, UPD_NUM_PNT_XSEC, "Num Xsecs", 11 );
    numXsecs.set_lower_upper(4, 101 );

	density.initialize(this, UPD_XFORM, "Density", 1.0 );
    density.set_lower_upper( 0.0, 10000000.0);

	shellMassArea.initialize(this, UPD_XFORM, "Shell Mass Area", 1.0 );
    shellMassArea.set_lower_upper( 0.0, 10000000.0);

	refFlag = 0;
	refArea = 100.0;
	refSpan = 10.0;
	refCbar = 1.0;
	aeroCenter.set_xyz(0,0,0);

    autoRefAreaFlag = 1;
	autoRefSpanFlag = 1;
	autoRefCbarFlag = 1;
	autoAeroCenterFlag = 1;

	currTexID = 0;
	currPartID = 0;
	currSourceID = 0;

}

Geom::~Geom()
{
	for ( int i = 0 ; i < (int)partVec.size() ; i++ )
		delete partVec[i];

	partVec.clear();

	DelAllSources();

}

void Geom::copy( Geom* fromGeom )
{
	displayChildrenFlag = fromGeom->getDisplayChildrenFlag();

	name_str = fromGeom->getName();
	color = fromGeom->getColor();
	setSymCode( fromGeom->getSymCode() );

	setMaterialID( fromGeom->getMaterialID() );

	setOutputFlag( fromGeom->getOutputFlag() );
	setOutputNameID( fromGeom->getOutputNameID() );

	relXFormFlag = fromGeom->getRelXFormFlag();

	xLoc.set( fromGeom->xLoc() );
	yLoc.set( fromGeom->yLoc() );
	zLoc.set( fromGeom->zLoc() );

	xRelLoc.set( fromGeom->xRelLoc() );
	yRelLoc.set( fromGeom->yRelLoc() );
	zRelLoc.set( fromGeom->zRelLoc() );

	xRot.set( fromGeom->xRot() );
	yRot.set( fromGeom->yRot() );
	zRot.set( fromGeom->zRot() );

	xRelRot.set( fromGeom->xRelRot() );
	yRelRot.set( fromGeom->yRelRot() );
	zRelRot.set( fromGeom->zRelRot() );

	origin.set( fromGeom->origin() );

	uAttach.set( fromGeom->uAttach() );
	vAttach.set( fromGeom->vAttach() );

	numPnts.set( fromGeom->numPnts() );
	numXsecs.set( fromGeom->numXsecs() );

	posAttachFlag = fromGeom->posAttachFlag;

	density.set( fromGeom->density() );
	shellMassArea.set( fromGeom->shellMassArea() );
	massPrior = fromGeom->getMassPrior();
	shellFlag = fromGeom->getShellFlag();

	refFlag = fromGeom->refFlag;
	refArea = fromGeom->refArea;
	refSpan = fromGeom->refSpan;
	refCbar = fromGeom->refCbar;
	aeroCenter = fromGeom->aeroCenter;

    autoRefAreaFlag = fromGeom->autoRefAreaFlag;
	autoRefSpanFlag = fromGeom->autoRefSpanFlag;
	autoRefCbarFlag = fromGeom->autoRefCbarFlag;
	autoAeroCenterFlag = fromGeom->autoAeroCenterFlag;

	appTexVec = fromGeom->getAppliedTexVec();

	//==== Copy Part Pointers ====//
	vector< Part* > fromPartVec = fromGeom->getStructurePartVec();

	//=== Reset Geom Ptrs ====//
	for ( int i = 0 ; i < (int)fromPartVec.size() ; i++ )
	{
		Part* part = structureMgrPtr->CreatePart( fromPartVec[i]->GetType() );
		part->SetGeomPtr( this );
		part->Copy( fromPartVec[i] );
		partVec.push_back( part );

	}
	currPartID = 0;

	//==== Copy CFD Sources ====//
//	sourceVec = fromGeom->getCfdMeshSourceVec();
//jrg FIX!!!!

	vector< BaseSource* > fromSrcVec = fromGeom->getCfdMeshSourceVec();
	for ( int i = 0 ; i < (int)fromSrcVec.size() ; i++ )
	{
		int type = fromSrcVec[i]->GetType();
		BaseSource* sourcePtr = cfdMeshMgrPtr->CreateSource( type );
		sourcePtr->Copy( fromSrcVec[i] );
//		*sourcePtr = *fromSrcVec[i];	// jrg fix this copy...
		sourcePtr->SetGeomPtr( this );

		sourceVec.push_back( sourcePtr );
	}

	//==== Copy FEA Structural Data ====//
	feaMeshMgrPtr->CopyGeomPtr( fromGeom, this );


	compose_model_matrix();
}

void Geom::DelCurrPart()
{
	int id = GetCurrPartID();

	vector< Part* > tmpVec;
	for ( int i = 0 ; i < (int)partVec.size() ; i++ )
	{
		if ( i != id )
			tmpVec.push_back( partVec[i] );
		else
			delete partVec[i];
	}
	partVec = tmpVec;
}

void Geom::DelAllSources()
{
	for ( int i = 0 ; i < (int)sourceVec.size() ; i++ )
	{
		delete sourceVec[i];
	}
	sourceVec.clear();
}

void Geom::DelCurrSource()
{
	int id = GetCurrSourceID();

	vector< BaseSource* > tmpVec;
	for ( int i = 0 ; i < (int)sourceVec.size() ; i++ )
	{
		if ( i != id )
			tmpVec.push_back( sourceVec[i] );
		else
			delete sourceVec[i];
	}
	sourceVec = tmpVec;
}

void Geom::UpdateSources()
{
	int i;
//	if ( getSymCode() != NO_SYM )
	for ( i = 0 ; i < (int)sourceVec.size() ; i++ )
	{
		sourceVec[i]->Update( this );
	}
}

void Geom::AddLinkableParm( Parm* p, vector< Parm* >& pVec, GeomBase* gPtr, const Stringc grpName )
{
	p->set_base_geom( gPtr );
	p->set_group_name( grpName );
	pVec.push_back( p );
}

void Geom::LoadLinkableParms( vector< Parm* > & parmVec )
{
	AddLinkableParm( &xLoc, parmVec, this, "Transform" );
	AddLinkableParm( &yLoc, parmVec, this, "Transform" );
	AddLinkableParm( &zLoc, parmVec, this, "Transform" );

	AddLinkableParm( &xRelLoc, parmVec, this, "Transform" );
	AddLinkableParm( &yRelLoc, parmVec, this, "Transform" );
	AddLinkableParm( &zRelLoc, parmVec, this, "Transform" );

	AddLinkableParm( &xRot, parmVec, this, "Transform" );
	AddLinkableParm( &yRot, parmVec, this, "Transform" );
	AddLinkableParm( &zRot, parmVec, this, "Transform" );

	AddLinkableParm( &xRelRot, parmVec, this, "Transform" );
	AddLinkableParm( &yRelRot, parmVec, this, "Transform" );
	AddLinkableParm( &zRelRot, parmVec, this, "Transform" );

	AddLinkableParm( &origin, parmVec, this, "Transform" );
	AddLinkableParm( &scaleFactor, parmVec, this, "Transform" );

	AddLinkableParm( &uAttach, parmVec, this, "Transform" );
	AddLinkableParm( &vAttach, parmVec, this, "Transform" );

	AddLinkableParm( &numPnts, parmVec, this, "Tessellate" );
	AddLinkableParm( &numXsecs, parmVec, this, "Tessellate" );

	AddLinkableParm( &density, parmVec, this, "Mass" );
	AddLinkableParm( &shellMassArea, parmVec, this, "Mass" );
}

//==== Tell Aircraft to Redraw ====//
void Geom::triggerDraw()
{
	if ( airPtr )
		airPtr->geomMod( this );
}

//==== Get Name String ====//
Stringc Geom::getName()
{
	int oid = getOutputNameID();
	if ( oid > 0 && oid < VspPreferences::Instance()->getNumOutputNames() )
		return VspPreferences::Instance()->getOutputName( oid );
	else 
		return name_str;
}

//==== Write File ====//
void Geom::write_general_parms(xmlNodePtr root)
{
  int i;

  xmlAddStringNode( root, "Name", getName() );
  xmlAddIntNode( root, "Id_Number", id_num );
  xmlAddStringNode( root, "Id_String", id_str );
  xmlAddDoubleNode( root, "ColorR", color.x() );
  xmlAddDoubleNode( root, "ColorG", color.y() );
  xmlAddDoubleNode( root, "ColorB", color.z() );
  xmlAddIntNode( root, "Symmetry", getSymCode() );
  xmlAddIntNode( root, "RelXFormFlag", relXFormFlag );
  xmlAddIntNode( root, "MaterialID", materialID );

  xmlAddIntNode( root, "OutputFlag", outputFlag );
  xmlAddIntNode( root, "OutputNameID", outputNameID );
  xmlAddIntNode( root, "DisplayChildrenFlag", displayChildrenFlag );

  xmlAddIntNode( root, "NumPnts", numPnts.iget() );
  xmlAddIntNode( root, "NumXsecs",numXsecs.iget() );

  xmlAddIntNode( root, "MassPrior", massPrior );
  xmlAddIntNode( root, "ShellFlag", shellFlag );

  xmlAddDoubleNode( root, "Tran_X", xLoc() );
  xmlAddDoubleNode( root, "Tran_Y", yLoc() );
  xmlAddDoubleNode( root, "Tran_Z", zLoc() );
  xmlAddDoubleNode( root, "TranRel_X", xRelLoc() );
  xmlAddDoubleNode( root, "TranRel_Y", yRelLoc() );
  xmlAddDoubleNode( root, "TranRel_Z", zRelLoc() );
  xmlAddDoubleNode( root, "Rot_X", xRot() );
  xmlAddDoubleNode( root, "Rot_Y", yRot() );
  xmlAddDoubleNode( root, "Rot_Z", zRot() );
  xmlAddDoubleNode( root, "RotRel_X", xRelRot() );
  xmlAddDoubleNode( root, "RotRel_Y", yRelRot() );
  xmlAddDoubleNode( root, "RotRel_Z", zRelRot() );

  xmlAddDoubleNode( root, "Origin", origin() );

  xmlAddDoubleNode( root, "Density", density() );
  xmlAddDoubleNode( root, "ShellMassArea", shellMassArea() );

  xmlAddIntNode( root, "RefFlag", refFlag );
  xmlAddDoubleNode( root, "RefArea", refArea );
  xmlAddDoubleNode( root, "RefSpan", refSpan );
  xmlAddDoubleNode( root, "RefCbar", refCbar );
  xmlAddIntNode( root, "AutoRefAreaFlag", autoRefAreaFlag );
  xmlAddIntNode( root, "AutoRefSpanFlag", autoRefSpanFlag );
  xmlAddIntNode( root, "AutoRefCbarFlag", autoRefCbarFlag );
  xmlAddDoubleNode( root, "AeroCenter_X", aeroCenter.x() );
  xmlAddDoubleNode( root, "AeroCenter_Y", aeroCenter.y()  );
  xmlAddDoubleNode( root, "AeroCenter_Z", aeroCenter.z()  );
  xmlAddIntNode( root, "AutoAeroCenterFlag", autoAeroCenterFlag );
  xmlAddIntNode( root, "WakeActiveFlag", m_WakeActiveFlag );

  //==== Write Attach Flags ====//
  xmlAddIntNode( root, "PosAttachFlag", posAttachFlag );
//  xmlAddIntNode( root, "RotAttachFlag", rotAttachFlag );
  xmlAddDoubleNode( root, "U_Attach", uAttach() );
  xmlAddDoubleNode( root, "V_Attach", vAttach() );

  xmlAddIntNode( root, "PtrID", ptrID );

  if ( parentGeom )
	xmlAddIntNode( root, "Parent_PtrID", parentGeom->getPtrID() );
  else
	xmlAddIntNode( root, "Parent_PtrID", 0 );


  for ( i = 0 ; i < (int)childGeomVec.size() ; i++ )
  {
	xmlAddIntNode( root, "Children_PtrID", childGeomVec[i]->getPtrID() );
  }

  for ( i = 0 ; i < (int)appTexVec.size() ; i++ )
  {
	xmlNodePtr tex_node = xmlNewChild( root, NULL, (const xmlChar *)"Applied_Texture", NULL );
	xmlAddStringNode( tex_node, "Name", appTexVec[i].nameStr );
	xmlAddStringNode( tex_node, "Texture_Name", appTexVec[i].texStr );
	xmlAddIntNode( tex_node, "All_Surf_Flag", appTexVec[i].allSurfFlag );
	xmlAddIntNode( tex_node, "Surf_ID", appTexVec[i].surfID );
	xmlAddDoubleNode( tex_node, "U", appTexVec[i].u );
	xmlAddDoubleNode( tex_node, "W", appTexVec[i].w );
	xmlAddDoubleNode( tex_node, "Scale_U", appTexVec[i].scaleu );
	xmlAddDoubleNode( tex_node, "Scale_W", appTexVec[i].scalew );
	xmlAddIntNode( tex_node, "Wrap_U_Flag", appTexVec[i].wrapUFlag );
	xmlAddIntNode( tex_node, "Wrap_W_Flag", appTexVec[i].wrapWFlag );
	xmlAddIntNode( tex_node, "Repeat_Flag", appTexVec[i].repeatFlag );
	xmlAddDoubleNode( tex_node, "Bright", appTexVec[i].bright );
	xmlAddDoubleNode( tex_node, "Alpha", appTexVec[i].alpha );

	xmlAddIntNode( tex_node, "Flip_U_Flag", appTexVec[i].flipUFlag );
	xmlAddIntNode( tex_node, "Flip_W_Flag", appTexVec[i].flipWFlag );
	xmlAddIntNode( tex_node, "Refl_Flip_U_Flag", appTexVec[i].reflFlipUFlag );
	xmlAddIntNode( tex_node, "Refl_Flip_W_Flag", appTexVec[i].reflFlipWFlag );

  }
	//==== Write Structure Parts ====//
	for ( i = 0 ; i < (int)partVec.size() ; i++ )
	{
		xmlNodePtr part_node = xmlNewChild( root, NULL, (const xmlChar *)"Structure_Part", NULL );
		partVec[i]->WriteParms( part_node );
	}

	//==== Write CFD Mesh Sources ====//
	for ( i = 0 ; i < (int)sourceVec.size() ; i++ )
	{
		xmlNodePtr source_node = xmlNewChild( root, NULL, (const xmlChar *)"CFD_Mesh_Source", NULL );
		sourceVec[i]->WriteParms( source_node );	
	}

	//==== Write Fea Structure Data =====//
	//feaMeshMgrPtr->WriteFeaStructData( this, root );


}

//==== Read General File Parameters ====//
void Geom::read_general_parms(xmlNodePtr root)
{
  int i;

  name_str = Stringc( xmlFindString( root, "Name", "Default_Name" ) );		// name_str is Stringc

//jrg id stuff  id_num = xmlFindInt( root, "Id_Number", 0 ) + jrg airPtr->get_id_offset();
  id_num = xmlFindInt( root, "Id_Number", 0 );
  id_str = Stringc( xmlFindString( root, "Id_String", "1234567" ) );				// id_str is a Stringc

  double r = xmlFindDouble( root, "ColorR", 0 );
  double g = xmlFindDouble( root, "ColorG", 0 );
  double b = xmlFindDouble( root, "ColorB", 0 );

  color.set_xyz( r, g, b );

  sym_code = xmlFindInt( root, "Symmetry", 0 );
  setSymCode(sym_code);

  relXFormFlag = xmlFindInt( root, "RelXFormFlag", relXFormFlag );

  materialID = xmlFindInt( root, "MaterialID", materialID );
  numPnts    = xmlFindInt( root, "NumPnts",    numPnts.iget() );
  numXsecs   = xmlFindInt( root, "NumXsecs",   numXsecs.iget() );

  outputFlag   = xmlFindInt( root, "OutputFlag", outputFlag );
  outputNameID = xmlFindInt( root, "OutputNameID", outputNameID );
  displayChildrenFlag = xmlFindInt( root, "DisplayChildrenFlag", displayChildrenFlag );
 
  massPrior = xmlFindInt( root, "MassPrior", massPrior );
  shellFlag = xmlFindInt( root, "ShellFlag", shellFlag );

  xLoc = xmlFindDouble( root, "Tran_X", xLoc() );
  yLoc = xmlFindDouble( root, "Tran_Y", yLoc() );
  zLoc = xmlFindDouble( root, "Tran_Z", zLoc() );
  xRelLoc = xmlFindDouble( root, "TranRel_X", xRelLoc() );
  yRelLoc = xmlFindDouble( root, "TranRel_Y", yRelLoc() );
  zRelLoc = xmlFindDouble( root, "TranRel_Z", zRelLoc() );

  xRot = xmlFindDouble( root, "Rot_X", xRot() );
  yRot = xmlFindDouble( root, "Rot_Y", yRot() );
  zRot = xmlFindDouble( root, "Rot_Z", zRot() );
  xRelRot = xmlFindDouble( root, "RotRel_X", xRelRot() );
  yRelRot = xmlFindDouble( root, "RotRel_Y", yRelRot() );
  zRelRot = xmlFindDouble( root, "RotRel_Z", zRelRot() );

  origin = xmlFindDouble( root, "Origin", origin() );

  density = xmlFindDouble( root, "Density", density() );
  shellMassArea = xmlFindDouble( root, "ShellMassArea", shellMassArea() );

  refFlag = xmlFindInt( root, "RefFlag", refFlag );
  refArea = xmlFindDouble( root, "RefArea", refArea );
  refSpan = xmlFindDouble( root, "RefSpan", refSpan );
  refCbar = xmlFindDouble( root, "RefCbar", refCbar );
  autoRefAreaFlag = xmlFindInt( root, "AutoRefAreaFlag", autoRefAreaFlag );
  autoRefSpanFlag = xmlFindInt( root, "AutoRefSpanFlag", autoRefSpanFlag );
  autoRefCbarFlag = xmlFindInt( root, "AutoRefCbarFlag", autoRefCbarFlag );
  aeroCenter.set_x( xmlFindDouble( root, "AeroCenter_X", aeroCenter.x() ) );
  aeroCenter.set_y( xmlFindDouble( root, "AeroCenter_Y", aeroCenter.y() ) );
  aeroCenter.set_z( xmlFindDouble( root, "AeroCenter_Z", aeroCenter.z() ) );
  autoAeroCenterFlag = xmlFindInt( root, "AutoAeroCenterFlag", autoAeroCenterFlag );
  autoAeroCenterFlag = !!(xmlFindInt( root, "WakeActiveFlag", m_WakeActiveFlag ));

  //==== Read Attach Flags ====//
  posAttachFlag = xmlFindInt( root, "PosAttachFlag", posAttachFlag );
  uAttach = xmlFindDouble( root, "U_Attach", uAttach() );
  vAttach = xmlFindDouble( root, "V_Attach", vAttach() );

  //==== Read Pointer ID and Parent/Children Info ====//
  ptrID = xmlFindInt( root, "PtrID", ptrID );

  parentPtrID = xmlFindInt( root, "Parent_PtrID", 0 );

  int numChildren =  xmlGetNumNames( root, "Children_PtrID" );

  for (  i = 0 ; i < numChildren ; i++ )
  {
     xmlNodePtr child_node = xmlGetNode( root, "Children_PtrID", i );
	 int id = xmlExtractInt( child_node );
	 childPtrIDVec.push_back( id );
  }

  appTexVec.clear();
  int numAppliedTextures = xmlGetNumNames( root, "Applied_Texture" );

  AppliedTex apptex;
  for ( i = 0 ; i < numAppliedTextures ; i++ )
  {
	xmlNodePtr tex_node = xmlGetNode( root, "Applied_Texture", i );
	
	apptex.nameStr = Stringc( xmlFindString( tex_node, "Name", "Default_Name" ) );
	apptex.texStr  = Stringc( xmlFindString( tex_node, "Texture_Name", "Default_Name" ) );
	apptex.allSurfFlag  = !!xmlFindInt( tex_node, "All_Surf_Flag", 0 );
	apptex.surfID  = xmlFindInt( tex_node, "Surf_ID", 0 );
	apptex.u  = xmlFindDouble( tex_node, "U", 0.5 );
	apptex.w  = xmlFindDouble( tex_node, "W", 0.5 );
	apptex.scaleu  = xmlFindDouble( tex_node, "Scale_U", 1.0 );
	apptex.scalew  = xmlFindDouble( tex_node, "Scale_W", 1.0 );
	apptex.wrapUFlag  = !!xmlFindInt( tex_node, "Wrap_U_Flag", 0 );
	apptex.wrapWFlag  = !!xmlFindInt( tex_node, "Wrap_W_Flag", 0 );
	apptex.repeatFlag  = !!xmlFindInt( tex_node, "Repeat_Flag", 0 );
	apptex.bright  = xmlFindDouble( tex_node, "Bright", 1.0 );
	apptex.alpha  = xmlFindDouble( tex_node, "Alpha", 1.0 );
	apptex.flipUFlag  = !!xmlFindInt( tex_node, "Flip_U_Flag", 0 );
	apptex.flipWFlag  = !!xmlFindInt( tex_node, "Flip_W_Flag", 0 );
	apptex.reflFlipUFlag  = !!xmlFindInt( tex_node, "Refl_Flip_U_Flag", 0 );
	apptex.reflFlipWFlag  = !!xmlFindInt( tex_node, "Refl_Flip_W_Flag", 0 );

	apptex.texID = texMgrPtr->loadTex( apptex.texStr.get_char_star() );

	if ( apptex.texID > 0 )
		appTexVec.push_back( apptex );

  }

	//==== Clean Part Vec ====//
	for (  i = 0 ; i < (int)partVec.size() ; i++ )
		delete partVec[i];

	partVec.clear();

	int numParts = xmlGetNumNames( root, "Structure_Part" );
	for ( i = 0 ; i < numParts ; i++ )
	{
		xmlNodePtr part_node = xmlGetNode( root, "Structure_Part", i );

		int type = xmlFindInt( part_node, "Type", -1 );

		Part* partPtr = structureMgrPtr->CreatePart( type );

		if ( partPtr )
		{
			partPtr->ReadParms( part_node );

			AddStructurePart( partPtr );
			partPtr->SetGeomPtr( this );

		}

	}

	//==== Read CFD Mesh Sources ====//
	for ( i = 0 ; i < (int)sourceVec.size() ; i++ )
		delete sourceVec[i];
	sourceVec.clear();

	int numSources = xmlGetNumNames( root, "CFD_Mesh_Source" );
	for ( i = 0 ; i < numSources ; i++ )
	{
		xmlNodePtr source_node = xmlGetNode( root, "CFD_Mesh_Source", i );
		int type = xmlFindInt( source_node, "Type", -1 );
		BaseSource* sourcePtr = cfdMeshMgrPtr->CreateSource( type );

		if ( sourcePtr )
		{
			sourcePtr->ReadParms( source_node );
			AddCfdMeshSource( sourcePtr );
		}
	}

	//==== Read Fea Structure Data =====//
	//feaMeshMgrPtr->SetFeaStructData( this, root );

	compose_model_matrix();
}


//==== Read General File Parameters ====//
void Geom::read_general_parms(FILE* file_id)
{

  char buff[255];
  char temp_name[255];
  int temp_id;

  fscanf(file_id, " %20s",temp_name);				// Name String

  name_str = temp_name;		// name_str is a Stringc

  fgets(buff, 80, file_id);							// Skip Line

  if ( airPtr->get_version() >= 2 )					// Read ID Num and Strings
  {
    fscanf(file_id, "%d",&temp_id);
    id_num = temp_id;
    fgets(buff, 80, file_id);
  }

  if ( airPtr->get_version() >= 3 )
  {
    fscanf(file_id, "%20s", temp_name);
	id_str = temp_name;				// id_str is a Stringc
    fgets(buff, 80, file_id);
  }

  int cid;
  fscanf(file_id, "%d",&cid);    fgets(buff, 80, file_id);	// Read Color
  //==== Create Some Colors ====//
  int r = (cid%3)*127;
  int g = (cid%2)*127;
  int b = (cid%4)*85;
  color.set_xyz( r, g, b );

  fscanf(file_id, "%d",&sym_code); fgets(buff, 80, file_id);	// Read SymCode
  setSymCode(sym_code);

  float tx, ty, tz;
  fscanf(file_id, "%f %f %f",&tx, &ty, &tz); fgets(buff, 80, file_id);
  xLoc = tx;
  yLoc = ty;
  zLoc = tz;

  float rx, ry, rz;
  fscanf(file_id, "%f %f %f",&rx, &ry, &rz);    fgets(buff, 80, file_id);
  xRot = rx;
  yRot = ry;
  zRot = rz;

}


void Geom::addChild( Geom* geomPtr )
{
	childGeomVec.push_back( geomPtr );
}

void Geom::loadChildren( vector< Geom* >& gVec )
{
	gVec.push_back( this );
	for ( int i = 0 ; i < (int)childGeomVec.size() ; i++ )
	{
		childGeomVec[i]->loadChildren( gVec );
	}
}

void Geom::loadDisplayChildren( vector< Geom* > & gVec )
{
	gVec.push_back( this );
	if ( displayChildrenFlag )
	{
		for ( int i = 0 ; i < (int)childGeomVec.size() ; i++ )
		{
			childGeomVec[i]->loadDisplayChildren( gVec );
		}
	}
}


void Geom::deactivate()
{
	airPtr->setActiveGeom(0);
}

void Geom::setParent( Geom* geomPtr )
{
	parentGeom = geomPtr;

	compose_model_matrix();

	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );

}

int Geom::countParents(int count)
{
	if ( parentGeom )
	{
		count++;
		return parentGeom->countParents( count );
	}
	return count;
}
	

vec3d Geom::getTotalTranVec( )
{
	vec3d tran( xLoc() + xLocOff(), 
				yLoc() + yLocOff(), 
				zLoc() + zLocOff() );

	return tran;
}


vec3d Geom::getTotalRotVec( )
{
	vec3d rot( xRot() + xRotOff(), 
			   yRot() + yRotOff(), 
			   zRot() + zRotOff() );

	return rot;
}
	

void Geom::removeChild( Geom* geomPtr )
{
	vector< Geom* > tempVec;
	for ( int i = 0 ; i < (int)childGeomVec.size() ; i++ )
	{
		if ( childGeomVec[i] != geomPtr )
			tempVec.push_back( childGeomVec[i] );
	}
	childGeomVec = tempVec;
}

int Geom::changedBaseParm( Parm* p )
{
	if ( p->get_update_grp() == UPD_XFORM )
	{
		compose_model_matrix();
		return 1;
	}

	return 0;

}

void Geom::computeCenter()
{
	center.set_xyz( 0.0, 0.0, 0.0 );
}


//==== Compose Modeling Matrix ====//
void Geom::compose_model_matrix()
{
  int i,j;
  Matrix mat;
  computeCenter();

  vec3d tran = getTotalTranVec();
  vec3d rot  = getTotalRotVec();

  float tran_x = (float)tran.x();
  float tran_y = (float)tran.y();
  float tran_z = (float)tran.z();

  float rot_x  = (float)rot.x();
  float rot_y  = (float)rot.y();
  float rot_z  = (float)rot.z();

  mat.loadIdentity();
  mat.translatef(  (float)center.x(), (float)center.y(), (float)center.z() );
  mat.rotateX( rot_x );
  mat.rotateY( rot_y );
  mat.rotateZ( rot_z );
  mat.translatef( -(float)center.x(),-(float)center.y(),-(float)center.z() );
  mat.getMat( matFVec );

  float xx = (matFVec[0]*tran_x + matFVec[1]*tran_y + matFVec[2]*tran_z);
  float yy = (matFVec[4]*tran_x + matFVec[5]*tran_y + matFVec[6]*tran_z);
  float zz = (matFVec[8]*tran_x + matFVec[9]*tran_y + matFVec[10]*tran_z);

  mat.translatef( xx, yy, zz );

  //==== Check if parent is rotation center ====//
  if ( parentGeom )
  {
	if ( posAttachFlag == POS_ATTACH_MATRIX )
	{
		float* pm = parentGeom->getMat(); 
		mat.postMult( pm );
	}
  }

  mat.getMat( matFVec );

  for( i = 0 ; i < 4 ; i++ )
    for ( j = 0 ; j < 4 ; j++ )
    {
      model_mat[i][j] = matFVec[i*4 + j]; 
    }

  if (sym_code != NO_SYM)
    compose_reflect_matrix();

  //==== Uppdate Children =====//
  for ( i = 0 ; i < (int)childGeomVec.size() ; i++ )
  {
	childGeomVec[i]->compose_model_matrix();
  }


  update_xformed_bbox();
}


//==== Compose Modeling Matrix ====//
void Geom::compose_reflect_matrix()
{
  vec3d tran = getTotalTranVec();
  float tran_x = (float)tran.x();
  float tran_y = (float)tran.y();
  float tran_z = (float)tran.z();
  vec3d rot  = getTotalRotVec();
  float rot_x  = (float)rot.x();
  float rot_y  = (float)rot.y();
  float rot_z  = (float)rot.z();

  float rrot_x = -(float)rot_x;
  float rrot_y = -(float)rot_y;
  float rrot_z = -(float)rot_z;
  float rtran_x = (float)tran_x;
  float rtran_y = (float)tran_y;
  float rtran_z = (float)tran_z;

  if ( sym_code == XY_SYM )
    {
      rtran_z = -tran_z;
      rrot_z  = rot_z;
    }
  else if ( sym_code == XZ_SYM )
    {
      rtran_y = -tran_y;
      rrot_y  = rot_y;
    }
  else if ( sym_code == YZ_SYM )
    {
      rtran_x = -tran_x;
      rrot_x  = rot_x;
    }

  Matrix mat;

  mat.loadIdentity();
  mat.translatef(  (float)center.x(), (float)center.y(), (float)center.z() );
  mat.rotateX( rrot_x );
  mat.rotateY( rrot_y );
  mat.rotateZ( rrot_z );
  mat.translatef( -(float)center.x(),-(float)center.y(),-(float)center.z() );
  mat.getMat( matReflFVec );

  float xx = (matReflFVec[0]*rtran_x + matReflFVec[1]*rtran_y + matReflFVec[2]*rtran_z);
  float yy = (matReflFVec[4]*rtran_x + matReflFVec[5]*rtran_y + matReflFVec[6]*rtran_z);
  float zz = (matReflFVec[8]*rtran_x + matReflFVec[9]*rtran_y + matReflFVec[10]*rtran_z);

  mat.translatef( xx, yy, zz );

  //==== Check if parent is rotation center ====//
  if ( parentGeom )
  {
	if ( posAttachFlag == POS_ATTACH_MATRIX )
	{
		float* pm = parentGeom->getMat(); 
		mat.postMult( pm );
	}
  }

  mat.getMat( matReflFVec );

  for( int i = 0 ; i < 4 ; i++ )
    for ( int j = 0 ; j < 4 ; j++ )
    {
      reflect_mat[i][j] = matReflFVec[i*4 + j]; 
    }
}

void Geom::setSymCode(int in_sym)
{
  if (in_sym == NO_SYM)
    {
      sym_code = in_sym;
      sym_vec = vec3d(1.0, 1.0, 1.0);
    }
  else if (in_sym == XY_SYM)
    {
      sym_code = in_sym;
      sym_vec = vec3d(1.0, 1.0, -1.0);
    }
  else if (in_sym == XZ_SYM)
    {
      sym_code = in_sym;
      sym_vec = vec3d(1.0,  -1.0, 1.0);
    }
  else if (in_sym == YZ_SYM)
    {
      sym_code = in_sym;
      sym_vec = vec3d(-1.0, 1.0, 1.0);
    }
  compose_model_matrix();

  //==== Tell Aircraft You Have Changed ====//
  airPtr->geomMod( this );


}

void Geom::setMaterialID( int m )
{
	if ( m >= 0 && m < matMgrPtr->getNumMaterial() )
	{
		materialID = m;
		airPtr->geomMod( this );
	}
}

int Geom::getNumTex()
{
	return appTexVec.size();
}

AppliedTex* Geom::getTex(int id)
{
	if ( id >= 0 && id < (int)appTexVec.size() )
		return &appTexVec[id];

	return NULL;
}

int Geom::getCurrentTexID()
{
	return currTexID;
}

void Geom::setCurrentTexID( int id )
{
	currTexID = id;
}

void Geom::setNoShowFlag( int flag )
{
	noshowFlag = flag;
	for ( int i = 0 ; i < (int)childGeomVec.size() ; i++ )
		childGeomVec[i]->setNoShowFlag( flag );
}

void Geom::addTex( const char* filename )
{
	int texID = texMgrPtr->loadTex( filename );
	if ( texID > 0 )
	{
		AppliedTex tex;

		tex.extractName( filename );
		tex.texStr = filename;
		tex.texID = texID;
		tex.alpha  = 1.0;
		tex.bright = 0.5;
		tex.repeatFlag = false;
		tex.scaleu = 0.5;
		tex.scalew = 0.5;
		tex.u = 0.5;
		tex.w = 0.5;
		tex.allSurfFlag = true;
		tex.surfID = 0;
		appTexVec.push_back( tex );
		currTexID = (int)appTexVec.size()-1;
	}
}

void Geom::delCurrentTex()
{
	vector< AppliedTex > temp;
	for ( int i = 0 ; i < (int)appTexVec.size() ; i++ )
	{
		if ( i != currTexID )
			temp.push_back( appTexVec[i] );
	}
	appTexVec = temp;

	if ( currTexID > (int)appTexVec.size()-1 )
		currTexID = (int)appTexVec.size()-1;

}


void Geom::updateAttach(int absUpdateFlag)
{
	//==== Update attachments to parent ====//
	if ( parentGeom )
	{
		vec3d apos;
		if ( posAttachFlag == POS_ATTACH_FIXED )
			apos = parentGeom->getAttachFixedPos();
		else if ( posAttachFlag == POS_ATTACH_UV )
			apos = parentGeom->getAttachUVPos( uAttach(), vAttach() );

		if ( posAttachFlag == POS_ATTACH_FIXED || posAttachFlag == POS_ATTACH_UV )
		{
			if ( absUpdateFlag )
			{
				xRelLoc = xLoc() - apos.x();
				yRelLoc = yLoc() - apos.y();
				zRelLoc = zLoc() - apos.z();
			}
			else
			{
				xLoc = apos.x() + xRelLoc();
				yLoc = apos.y() + yRelLoc();
				zLoc = apos.z() + zRelLoc();
			}
		}
	}

	//==== Update Model Matrix
	compose_model_matrix();


	//==== Update Childrens Locations ====//
	for ( int i = 0 ; i < (int)childGeomVec.size() ; i++ )
	{
		childGeomVec[i]->updateAttach(0);
	}
}


void Geom::setPosAttachFlag( int flag )	
{
	if ( parentGeom )
	{
		vec3d apos;
		if ( flag == POS_ATTACH_FIXED )
			apos = parentGeom->getAttachFixedPos();
		else if ( flag == POS_ATTACH_UV )
			apos = parentGeom->getAttachUVPos( uAttach(), vAttach() );

		if ( flag == POS_ATTACH_FIXED || flag == POS_ATTACH_UV )
		{
			xRelLoc = xLoc() - apos.x();
			yRelLoc = yLoc() - apos.y();
			zRelLoc = zLoc() - apos.z();
		}
	}	

	posAttachFlag = flag;

	compose_model_matrix();

	airPtr->geomMod( this );
}

void Geom::setRelXFormFlag( int f )
{ 
	relXFormFlag = f; 
}
	
double Geom::get_tran_x()
{
	vec3d v = getTotalTranVec();
	return v.x();
}
double Geom::get_tran_y()
{
	vec3d v = getTotalTranVec();
	return v.y();
}
double Geom::get_tran_z()
{
	vec3d v = getTotalTranVec();
	return v.z();
}

double Geom::get_rot_x()
{
	vec3d v = getTotalRotVec();
	return v.x();
}
double Geom::get_rot_y()
{
	vec3d v = getTotalRotVec();
	return v.y();
}
double Geom::get_rot_z()
{
	vec3d v = getTotalRotVec();
	return v.z();
}

vec3d Geom::getAttachUVPos(double u, double v)
{
	vec3d pos;
	pos.set_xyz( xLoc() + xLocOff(), 
				 yLoc() + yLocOff(), 
				 zLoc() + zLocOff());
	return pos;
}

vec3d Geom::getAttachFixedPos()
{
	vec3d pos;
	pos.set_xyz( xLoc() + xLocOff(), 
				 yLoc() + yLocOff(), 
				 zLoc() + zLocOff());
	return pos;
}


void Geom::update_xformed_bbox()
{
  vec3d temp;
  vec3d p[8];
  p[0] = vec3d(bnd_box.get_min(0), bnd_box.get_min(1), bnd_box.get_min(2));
  p[1] = vec3d(bnd_box.get_max(0), bnd_box.get_min(1), bnd_box.get_min(2));
  p[2] = vec3d(bnd_box.get_min(0), bnd_box.get_max(1), bnd_box.get_min(2));
  p[3] = vec3d(bnd_box.get_min(0), bnd_box.get_min(1), bnd_box.get_max(2));
  p[4] = vec3d(bnd_box.get_max(0), bnd_box.get_max(1), bnd_box.get_min(2));
  p[5] = vec3d(bnd_box.get_min(0), bnd_box.get_max(1), bnd_box.get_max(2));
  p[6] = vec3d(bnd_box.get_max(0), bnd_box.get_min(1), bnd_box.get_max(2));
  p[7] = vec3d(bnd_box.get_max(0), bnd_box.get_max(1), bnd_box.get_max(2));

  bbox new_box;
  for ( int i = 0 ; i < 8 ; i++)
    {
      temp = p[i].transform(model_mat);
      new_box.update(temp);

      if (sym_code != NO_SYM)
        {
          temp = (p[i] * sym_vec).transform(reflect_mat);
          new_box.update(temp);
        }
    }

  bnd_box_xform = new_box;
}

void Geom::setColor( double r, double g, double b)
{
	color.set_xyz( r, g, b );

	airPtr->geomMod( this );
}

//==== Draw Highlighting Boxes ====//
void Geom::draw_highlight_boxes()
{

  //==== Draw Current Comp Box =====//
  if ( redFlag == 1)
    {
      glLineWidth(2);
      glColor3f(1.0, 0, 0);
      draw_bbox(); 
    }

  //==== Draw Temp Highlight Box =====//
  if ( yellowFlag == 1)
    {
      glLineWidth(2);
      glColor3f(1.0, 1.0, 0);
      draw_bbox(); 
    }
} 

 
//==== Compose Modeling Matrix ====//
void Geom::draw_bbox()
{
  double temp[3];
  temp[0] = bnd_box_xform.get_min(0);
  temp[1] = bnd_box_xform.get_min(1);
  temp[2] = bnd_box_xform.get_min(2);

  glBegin( GL_LINE_STRIP );
    glVertex3dv(temp);
    temp[0] = bnd_box_xform.get_max(0);
    glVertex3dv(temp);
    temp[1] = bnd_box_xform.get_max(1);
    glVertex3dv(temp);
    temp[2] = bnd_box_xform.get_max(2);
    glVertex3dv(temp);
    temp[0] = bnd_box_xform.get_min(0);
    glVertex3dv(temp);
    temp[2] = bnd_box_xform.get_min(2);
    glVertex3dv(temp);
    temp[1] = bnd_box_xform.get_min(1);
    glVertex3dv(temp);
    temp[2] = bnd_box_xform.get_max(2);
    glVertex3dv(temp);
    temp[0] = bnd_box_xform.get_max(0);
    glVertex3dv(temp);
    temp[2] = bnd_box_xform.get_min(2);
    glVertex3dv(temp);
  glEnd();

  glBegin( GL_LINE_STRIP );
    temp[2] = bnd_box_xform.get_max(2);
    glVertex3dv(temp);
    temp[1] = bnd_box_xform.get_max(1);
    glVertex3dv(temp);
  glEnd();

  glBegin( GL_LINE_STRIP );
    temp[2] = bnd_box_xform.get_min(2);
    glVertex3dv(temp);
    temp[0] = bnd_box_xform.get_min(0);
    glVertex3dv(temp);
  glEnd();

  glBegin( GL_LINE_STRIP );
    temp[2] = bnd_box_xform.get_max(2);
    glVertex3dv(temp);
    temp[1] = bnd_box_xform.get_min(1);
    glVertex3dv(temp);
  glEnd();
  

}

//==== Write POV Input File ====//
void Geom::write_pov_tris(FILE* pov_file)
{
	Stringc name = getName();
	name.space_to_underscore();

	fprintf( pov_file, "#declare %s = mesh { \n", name.get_char_star() );
	for ( int i = 0 ; i < (int)surfVec.size() ; i++ )
	{
		surfVec[i]->write_pov_smooth_tris(sym_code, model_mat, reflect_mat, pov_file);
	}
	fprintf( pov_file, " }\n" );
}

//==== Write STL File ====//
void Geom::write_stl_file(FILE* fileid )
{

}

//==== Write Rhino File ====//
void Geom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes )
{


}

void Geom::write_bezier_file( int id, FILE* file_id )
{




}


//==== Return Number of Xsec Surfs to Write ====//
int Geom::getNumXSecSurfs()
{
	if ( !outputFlag )
		return 0;

	if ( sym_code == NO_SYM )
		return 1;
	else 
		return 2;
}

//==== Dump Xsec File =====//
void Geom::dump_xsec_file(int geom_no, FILE* dump_file)
{

}


void  Geom::storeModelMatrix()
{
	glGetFloatv(GL_MODELVIEW_MATRIX, viewMatrix.data());		// Retrieve The View Matrix

	glPushMatrix();
	glMultMatrixf((GLfloat*)model_mat); 
	glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix.data());		// Retrieve The Modelview Matrix
	glPopMatrix();

	glPushMatrix();
	glMultMatrixf((GLfloat*)reflect_mat); 
	glGetFloatv(GL_MODELVIEW_MATRIX, reflMatrix.data());		// Retrieve The Modelview Matrix
	glPopMatrix();
}

vec2d Geom::projectPoint(vec3d point, int refl)
{
	float vecin[4];
	float vecout[4];
	vecin[0] = (float) point.x();
	vecin[1] = (float) point.y();
	vecin[2] = (float) point.z();
	vecin[3] = 1.0f;

	if (refl)
		reflMatrix.mult(vecin, vecout);
	else
		modelMatrix.mult(vecin, vecout);

	return vec2d(vecout[0], vecout[1]);
}

vec2d Geom::viewProjectPoint(vec3d point)
{
	float vecin[4];
	float vecout[4];
	vecin[0] = (float) point.x();
	vecin[1] = (float) point.y();
	vecin[2] = (float) point.z();
	vecin[3] = 1.0f;

	viewMatrix.mult(vecin, vecout);

	return vec2d(vecout[0], vecout[1]);
}

vec3d Geom::xformPoint(vec3d point, int refl)
{
	Matrix matrix;
	if (refl)
		matrix.initMat((float *) reflect_mat);
	else
		matrix.initMat((float *) model_mat);
	
	float vecin[4];
	float vecout[4];
	vecin[0] = (float) point.x();
	vecin[1] = (float) point.y();
	vecin[2] = (float) point.z();
	vecin[3] = 1.0f;


	matrix.mult(vecin, vecout);

	return vec3d(vecout[0], vecout[1], vecout[2]);
}


void  Geom::buildVertexVec(Xsec_surf * xsurf, int surface, vector< VertexID > * vertVec)
{
	int numxsec = xsurf->get_num_xsecs();
	int numpnts = xsurf->get_num_pnts();
	for (int x = 0; x < numxsec; x++)
	{
		for (int p = 0; p < numpnts; p++)
		{
			VertexID vert;
			vert.geomPtr = this;
			vert.surface = surface;
			vert.section = x/(double)(numxsec-1);
			vert.point = p/(double)(numpnts-1);
			vert.reflect = 0;
			vertVec->push_back(vert);
			if ( sym_code != NO_SYM )
			{
				vert.reflect = 1;
				vertVec->push_back(vert);
			}
		}
	}
}

void  Geom::buildVertexVec(vector<TMesh*> * meshVec, int surface, vector< VertexID > * vertVec)
{
	int nummesh = meshVec->size();
	for (int x = 0; x < nummesh; x++)
	{
		TMesh * mesh = (*meshVec)[x];
		int meshsize = mesh->numVerts();
		for (int p = 0; p < meshsize; p++)
		{
			VertexID vert;
			vert.geomPtr = this;
			vert.surface = surface;
			vert.section = x/(double)(nummesh-1);
			vert.point = p/(double)(meshsize-1);
			vert.reflect = 0;
			vertVec->push_back(vert);
		}
	}
}
   
void Geom::drawTextures(bool reflFlag)
{
	glEnable( GL_LIGHTING );

	glEnable( GL_TEXTURE_2D );			// Turn On Texturing 
	glDepthMask(GL_FALSE);				// Turn Off Depth Buffer
	glDepthFunc(GL_EQUAL);
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable(GL_ALPHA_TEST);  
	glAlphaFunc(GL_GREATER, 0);   

	for ( int i = 0 ; i < (int)appTexVec.size() ; i++ )
	{
		// White Base Material For Textures
		Material* mat = matMgrPtr->getMaterial( materialID );
		Material wmat = matMgrPtr->getWhiteMaterial( (float)appTexVec[i].bright, mat->shine );	
		wmat.diff[3] = (float)appTexVec[i].alpha;
		wmat.bind();

		glBindTexture(GL_TEXTURE_2D, appTexVec[i].texID );

		if ( appTexVec[i].repeatFlag )
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		}
		else
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}

		for ( int s = 0 ; s < (int)surfVec.size() ; s++ )
		{
			if ( surfVec[s]->get_draw_flag() )
			{
				if ( appTexVec[i].surfID == s || appTexVec[i].allSurfFlag )
				{
					if ( reflFlag )	
						surfVec[s]->draw_refl_texture( appTexVec[i], sym_code );
					else
						surfVec[s]->draw_texture( appTexVec[i] );
				}
			}
		}

	}
	glDepthMask(GL_TRUE);				// Turn On Depth Buffer
	glDepthFunc(GL_LESS);
	glDisable( GL_TEXTURE_2D );			// Turn Off Texturing
	glDisable( GL_LIGHTING );
}


void Geom::draw()
{
	int i;

	//==== Draw Highlighting Boxes ====//
	draw_highlight_boxes();

	//==== Check Noshow Flag ====//
	if ( noshowFlag ) return;	

	if ( displayFlag == GEOM_WIRE_FLAG )
	{
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	

		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 

		for ( i = 0 ; i < (int)surfVec.size() ; i++ )
		{
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
			if ( surfVec[i]->get_draw_flag() )
			{
				surfVec[i]->draw_wire();
			}
		}

		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 

		for ( i = 0 ; i < (int)surfVec.size() ; i++ )
		{
			glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
			if ( surfVec[i]->get_draw_flag() )
				surfVec[i]->draw_refl_wire(sym_code);
		}

		glPopMatrix();
	}
	else if ( displayFlag == GEOM_SHADE_FLAG || displayFlag == GEOM_TEXTURE_FLAG)
	{
		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 

		Material* mat = matMgrPtr->getMaterial( materialID );
		if ( mat )
		{
			if ( mat->diff[3] > 0.99 )
			{
				mat->bind();

				for ( i = 0 ; i < (int)surfVec.size() ; i++ )
				{
					if ( surfVec[i]->get_draw_flag() )
						surfVec[i]->draw_shaded();
				}

				if ( displayFlag == GEOM_TEXTURE_FLAG )
					drawTextures(false);
			}
		}
		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		mat = matMgrPtr->getMaterial( materialID );
		if ( mat )
		{
			mat->bind();
			if  ( mat->diff[3] > 0.99 )
			{
				for ( i = 0 ; i < (int)surfVec.size() ; i++ )
				{
					if ( surfVec[i]->get_draw_flag() )
						surfVec[i]->draw_refl_shaded(sym_code);
				}

				if ( displayFlag == GEOM_TEXTURE_FLAG )
					drawTextures(true);
			}
		}
		glPopMatrix();
	}
	else if ( displayFlag == GEOM_HIDDEN_FLAG )
	{
		//==== Draw Hidden Surface ====//
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	

		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 

		for ( i = 0 ; i < (int)surfVec.size() ; i++ )
		{
			if ( surfVec[i]->get_draw_flag() )
			{						
				glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
				surfVec[i]->draw_hidden();
			}
		}
		glPopMatrix();

		//==== Reflected Geom ====//
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 
		for ( i = 0 ; i < (int)surfVec.size() ; i++ )
		{
			if ( surfVec[i]->get_draw_flag() )
			{
				glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	
				surfVec[i]->draw_refl_hidden(sym_code);
			}
		}
		glPopMatrix();
			
	}
	glDisable( GL_LIGHTING );
}



//==== Draw If Alpha < 1 and Shaded ====//
void Geom::drawAlpha()
{
	int i;
	Material* mat = matMgrPtr->getMaterial( materialID );
	if ( !mat )
		return;

	if ( mat->diff[3] > 0.99 )
		return;

	//==== Check Noshow Flag ====//
	if ( noshowFlag ) return;	
	
	if ( displayFlag == GEOM_SHADE_FLAG || displayFlag == GEOM_TEXTURE_FLAG )
	{
		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 

		mat->bind();
//		body_surf.draw_shaded();

		for ( i = 0 ; i < (int)surfVec.size() ; i++ )
		{
			if ( surfVec[i]->get_draw_flag() )
				surfVec[i]->draw_shaded();
		}


		if ( displayFlag == GEOM_TEXTURE_FLAG )
			drawTextures(false);

		glPopMatrix();

		//==== Reflected Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)reflect_mat); 

		mat->bind();
//		body_surf.draw_refl_shaded( sym_code );
		for ( i = 0 ; i < (int)surfVec.size() ; i++ )
		{
			if ( surfVec[i]->get_draw_flag() )
				surfVec[i]->draw_refl_shaded( sym_code );
		}

		if ( displayFlag == GEOM_TEXTURE_FLAG )
			drawTextures(true);

		glPopMatrix();
	}

}
//=====================================================================================================//
//=====================================================================================================//
//=====================================================================================================//
//=====================================================================================================//
//=====================================================================================================//

VertexID::VertexID(const VertexID& v)	// vec2d x = y
{
	geomPtr = v.geomPtr;
	surface = v.surface;
	section = v.section;
	point= v.point;
	reflect = v.reflect;

}

VertexID& VertexID::operator=(const VertexID& v)	// x = y
{
	if (this == &v)
	   return *this;

	geomPtr = v.geomPtr;
	surface = v.surface;
	section = v.section;
	point= v.point;
	reflect = v.reflect;
	return *this;
}

void VertexID::reset() {
	geomPtr = NULL;
	surface = 0;
	section = 0;
	point = 0;
	reflect = 0;
	axisProjType = AXIS_PROJ_NONE;
}


void VertexID::write(Aircraft * airPtr, xmlNodePtr root, Stringc name)
{
	xmlNodePtr vertex_node = xmlNewChild( root, NULL, (const xmlChar *)name.get_char_star(), NULL );
	xmlAddIntNode( vertex_node, "GeomID", airPtr->getGeomIndex(geomPtr));
	xmlAddIntNode( vertex_node, "Surface", surface );
	xmlAddDoubleNode( vertex_node, "Section", section);
	xmlAddDoubleNode( vertex_node, "Point",   point );
	xmlAddIntNode( vertex_node, "Reflect", reflect);
}

void VertexID::read(Aircraft * airPtr, xmlNodePtr root, Stringc name)
{
  xmlNodePtr vertex_node;
  vertex_node = xmlGetNode( root, name.get_char_star(), 0 );
  if ( vertex_node )
  {
	int geomIndex = xmlFindInt( vertex_node, "GeomID", -1);
	geomPtr = airPtr->getClipboardGeom(geomIndex);
	surface = xmlFindInt( vertex_node, "Surface", surface );
	section = xmlFindDouble( vertex_node, "Section", section);
	point   = xmlFindDouble( vertex_node, "Point",   point );
	reflect = xmlFindInt( vertex_node, "Reflect", reflect);
  }
}

void VertexID::setAxisProject( int x, int y, int z, vec3d orig )
{
	axisProjPnt0 = orig;
	if ( x && y && z )
		axisProjType = AXIS_PROJ_NONE;
	else if ( x && y )
	{
		axisProjType = AXIS_PROJ_PLANE;
		axisProjPnt1 = orig + vec3d(1,0,0);
		axisProjPnt2 = orig + vec3d(0,1,0);
	}
	else if ( x && z )
	{
		axisProjType = AXIS_PROJ_PLANE;
		axisProjPnt1 = orig + vec3d(1,0,0);
		axisProjPnt2 = orig + vec3d(0,0,1);
	}
	else if ( y && z )
	{
		axisProjType = AXIS_PROJ_PLANE;
		axisProjPnt1 = orig + vec3d(0,1,0);
		axisProjPnt2 = orig + vec3d(0,0,1);
	}
	else if ( x )
	{
		axisProjType = AXIS_PROJ_LINE;
		axisProjPnt1 = orig + vec3d(1,0,0);
	}
	else if ( y )
	{
		axisProjType = AXIS_PROJ_LINE;
		axisProjPnt1 = orig + vec3d(0,1,0);
	}
	else if ( z )
	{
		axisProjType = AXIS_PROJ_LINE;
		axisProjPnt1 = orig + vec3d(0,0,1);
	}
}

vec3d VertexID::posXformProjAxis()
{
	if ( axisProjType == AXIS_PROJ_NONE )
		return posXform();

	vec3d p3 = posXform();
	vec3d projp3 = p3;

	if ( axisProjType == AXIS_PROJ_LINE )
	{
		projp3 = proj_pnt_on_line( axisProjPnt0, axisProjPnt1, p3 ); 
	}
	else if ( axisProjType == AXIS_PROJ_PLANE )
	{
		vec3d pl1 = axisProjPnt1 - axisProjPnt0;
		vec3d pl2 = axisProjPnt2 - axisProjPnt0;
		projp3 = proj_pnt_to_plane(axisProjPnt0, pl1, pl2,  p3 ); 
	}
	return projp3;
}







//=====================================================================================================//
//=====================================================================================================//
//=====================================================================================================//
//=====================================================================================================//
//=====================================================================================================//

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PodGeom::PodGeom(Aircraft* aptr) : Geom(aptr)
{

	type = POD_GEOM_TYPE;
	type_str = Stringc("pod");

	char name[255];
	sprintf( name, "Pod_%d", geomCnt ); 
	geomCnt++;

	setName( Stringc(name) );

	//==== Load Points For Curves ====//
	hcircle.init(9);
	hcircle.load_pnt( 0, vec3d(0,0,1) );
	hcircle.load_pnt( 1, vec3d(0,0.707,0.707) );
	hcircle.load_pnt( 2, vec3d(0,1,0) );
	hcircle.load_pnt( 3, vec3d(0,0.707,-0.707) );
	hcircle.load_pnt( 4, vec3d(0,0,-1) );
	hcircle.load_pnt( 5, vec3d(0,-0.707,-0.707) );
	hcircle.load_pnt( 6, vec3d(0,-1,0) );
	hcircle.load_pnt( 7, vec3d(0,-0.707,0.707) );
	hcircle.load_pnt( 8, vec3d(0,0,1) );
	hcircle.set_open_closed_flag( CLOSED_CURVE );
	hcircle.comp_tans();

	body_curve.init_num_sections(2);

	length.initialize(this, UPD_POD, "Length", 10.0);
    length.set_lower_upper(0.0001, 1000000.0);
	length.set_script("pod_design length", 0);

	fine_ratio.initialize(this, UPD_POD, "Finess_Ratio", 15.0);
    fine_ratio.set_lower_upper(1.0, 1000.0);
	fine_ratio.set_script("pod_design fineratio", 0);

	body_surf.set_num_pnts( numPnts.iget() );
	body_surf.set_num_xsecs(  numXsecs.iget() );

	generate();

	surfVec.push_back( &body_surf );

}

PodGeom::~PodGeom()
{
	int junk = 23;
}

void PodGeom::write(xmlNodePtr root)
{
  xmlAddStringNode( root, "Type", "Pod");

  //==== Write General Parms ====//
  xmlNodePtr gen_node = xmlNewChild( root, NULL, (const xmlChar *)"General_Parms", NULL );
  write_general_parms( gen_node );

  //==== Write Fuse Parms ====//
  xmlNodePtr pod_node = xmlNewChild( root, NULL, (const xmlChar *)"Pod_Parms", NULL );

  xmlAddDoubleNode( pod_node, "Length", length() );
  xmlAddDoubleNode( pod_node, "Fine_Ratio", fine_ratio() );



}

//==== Read External File ====//
void PodGeom::read(xmlNodePtr root)
{
  xmlNodePtr node;

  //===== Read General Parameters =====//
  node = xmlGetNode( root, "General_Parms", 0 );
  if ( node )
    read_general_parms( node );

  //===== Read Fuse Parameters =====//
  node = xmlGetNode( root, "Pod_Parms", 0 );
  if ( node )
  {
    length = xmlFindDouble( node, "Length", length() );
    fine_ratio = xmlFindDouble( node, "Fine_Ratio", fine_ratio() );
  }

  generate();

}

void PodGeom::copy( Geom* fromGeom )
{
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != POD_GEOM_TYPE )
		return;

	PodGeom* pg = (PodGeom*)fromGeom;

	length.set( pg->length() );
	fine_ratio.set( pg->fine_ratio() );

	body_surf.set_num_pnts( numPnts.iget() );
	body_surf.set_num_xsecs(  numXsecs.iget() );


	compose_model_matrix();

	generate();

}

//void PodGeom::RegisterLinkableParms()
//{
//	Geom::RegisterLinkableParms();
//
//	parmLinkMgrPtr->Register( &length, this, "Design" );
//	parmLinkMgrPtr->Register( &fine_ratio, this, "Design" );
//}

void PodGeom::LoadLinkableParms( vector< Parm* > & parmVec )
{
	Geom::LoadLinkableParms( parmVec );
	Geom::AddLinkableParm( &length, parmVec, this, "Design" );
	Geom::AddLinkableParm( &fine_ratio, parmVec, this, "Design" );
}

//==== Generate Fuse Component ====//
void PodGeom::generate()
{
  generate_body_curve();
  update_bbox();
}

void PodGeom::computeCenter()
{
	//==== Set Rotation Center ====//
	center.set_x( origin()*length()*scaleFactor() ); 
}

//==== Parm Has Changed ReGenerate Fuse Component ====//
void PodGeom::parm_changed(Parm* p)
{

	int up_group = p->get_update_grp();

	switch (up_group)
	{
		case UPD_XFORM:
		{
			if ( p == &xLoc || p == &yLoc || p == &zLoc || 
				 p == &xRot || p == &yRot || p == &zRot )
				updateAttach(1);
			else
				updateAttach(0);
			generate();
		}
		break;

		case UPD_NUM_PNT_XSEC:
		{
			body_surf.set_num_pnts( numPnts.iget() );
			body_surf.set_num_xsecs(  numXsecs.iget() );
			generate();
		}
		break;

		case UPD_POD:
        {
			compose_model_matrix();	
			generate();
			updateAttach(0);
        }
		break;
	}

	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );
}


void PodGeom::acceptScaleFactor()
{
	double len = length()*scaleFactor();
	length.set( len );
	scaleFactor.set(1.0);
}

void PodGeom::resetScaleFactor()
{
	double len = length();
	length.set( len );
	scaleFactor.set(1.0);
	generate();
	airPtr->geomMod( this );

}

void PodGeom::draw()
{
	Geom::draw();
}

//==== Draw If Alpha < 1 and Shaded ====//
void PodGeom::drawAlpha()
{
	Geom::drawAlpha();
}

//==== Generate Body Defining Curve ====//
void PodGeom::generate_body_curve()
{
  double len = length()*scaleFactor(); 
  double dia = len/fine_ratio();

  //==== Load Points For Curves ====//
  body_curve.put_pnt(0, vec3d(0.0, 0.0, 0.0) );
  body_curve.put_pnt(3, vec3d(len/2.0, 0.0, dia) );
  body_curve.put_pnt(6, vec3d(len, 0.0, 0.0) );

  //==== Set Curve Def Parms  ====//
  body_curve.put_pnt(1, vec3d(0.05*len, 0.0, dia*0.95) );
  body_curve.put_pnt(2, vec3d(0.20*len, 0.0, dia) );
  body_curve.put_pnt(4, vec3d(0.60*len, 0.0, dia) );
  body_curve.put_pnt(5, vec3d(0.95*len, 0.0, dia*0.3) );

  gen_body_surf();
 
}

//==== Generate Cross Sections =====//
void PodGeom::gen_body_surf()
{
  float u;
  vec3d def_pnt, temp_pnt;

  int numP = body_surf.get_num_pnts();
  int numX = body_surf.get_num_xsecs();
  
  //==== Set Cross Section Parms And Loads X-Secs ====//
  int nx_1 = numX/2 + 1;
  int nx_2 = numX - nx_1;
  int i;

  for (  i = 0 ; i < nx_1 ; i++ )
    {
      u = (float)i/(float)(nx_1 - 1);
      def_pnt = body_curve.comp_pnt(0, u);
      for ( int j = 0 ; j < numP ; j++ )
        {
		   double w = 8.0*(float)j/(float)(numP-1);
		  temp_pnt = hcircle.comp_pnt((float)w)*def_pnt.z();
//jrg          temp_pnt = circle.get_pnt(j)*def_pnt.z();
          temp_pnt.set_x(def_pnt.x());      
          body_surf.set_pnt(i, j, temp_pnt);
        }
    }

  //==== Set Cross Section Parms And Loads X-Secs ====//
  for ( i = 0 ; i < nx_2 ; i++ )
    {
      u = (float)(i+1)/(float)(nx_2);
      def_pnt = body_curve.comp_pnt(1, u);
       for ( int j = 0 ; j < numP ; j++ )
        {
		   double w = 8.0*(float)j/(float)(numP-1);
		  temp_pnt = hcircle.comp_pnt((float)w)*def_pnt.z();
          temp_pnt.set_x(def_pnt.x());      
          body_surf.set_pnt(i+nx_1, j, temp_pnt);
        }
    }

  body_surf.load_refl_pnts_xsecs();
  body_surf.load_hidden_surf();
  body_surf.load_normals();
  body_surf.load_uw();


}

//==== Update Bounding Box =====//
void PodGeom::update_bbox()
{
  vec3d pnt;
  bbox new_box;

  int num_pnts  = body_surf.get_num_pnts();
  int num_xsecs = body_surf.get_num_xsecs();

  //==== Check All Xsecs ====//
  for ( int i = 0 ; i < num_xsecs ; i++ )
	for ( int j = 0 ; j < num_pnts ; j++ )
	{
		new_box.update(body_surf.get_pnt(i,j));
	}

  bnd_box = new_box;
  update_xformed_bbox();			// Load Xform BBox
}


vec3d PodGeom::getAttachUVPos(double u, double v)
{
	vec3d pos;

	vec3d uvpnt = body_surf.comp_uv_pnt(u,v);
	pos = uvpnt.transform( model_mat );

	return pos;
}

//==== Write Rhino File ====//
void PodGeom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
{
	body_surf.clear_xsec_tan_flags();

	body_surf.set_xsec_tan_flag( 0, Bezier_curve::ZERO );
    int num_xsecs  = body_surf.get_num_xsecs();
	body_surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::ZERO );

	body_surf.write_rhino_file( sym_code, model_mat, reflect_mat, archive, attributes );
}

int  PodGeom::get_num_bezier_comps()
{
	if ( sym_code == NO_SYM )
		return 1;
	else 
		return 2;
}

void PodGeom::write_bezier_file( int id, FILE* file_id )
{
	body_surf.clear_xsec_tan_flags();

	body_surf.set_xsec_tan_flag( 0, Bezier_curve::PREDICT  );
    int num_xsecs  = body_surf.get_num_xsecs();
	body_surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::PREDICT );

	vector <int> u_split;
	u_split.push_back( 0 );
	u_split.push_back( 3*(body_surf.get_num_xsecs() - 1)  );
	vector <int> w_split;
	w_split.push_back( 0 );
	w_split.push_back( 3*(body_surf.get_num_pnts()/4)  );
	w_split.push_back( 3*(body_surf.get_num_pnts()/2)  );
	w_split.push_back( 3*3*(body_surf.get_num_pnts()/4)  );
	w_split.push_back( 3*(body_surf.get_num_pnts() - 1)  );

	body_surf.write_bezier_file( file_id, sym_code, model_mat, reflect_mat, u_split, w_split  );

}


//==== Convert To Tri Mesh ====//
vector< TMesh* > PodGeom:: createTMeshVec()
{
	vector< TMesh* > tMeshVec;

	TMesh* tmPtr = body_surf.createTMesh(model_mat);
	tMeshVec.push_back( tmPtr );

	if ( sym_code != NO_SYM )
	{
		tmPtr = body_surf.createReflTMesh(sym_code, reflect_mat);
		tMeshVec.push_back( tmPtr );
	}
	return tMeshVec;
}




void PodGeom::dump_xsec_file(int geom_no, FILE* dump_file)
{
  int i;

  fprintf(dump_file, "\n");
  fprintf(dump_file, "%s \n", (char*) getName());
  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
  fprintf(dump_file, " TYPE              = 1  \n");						// Non Lifting
  fprintf(dump_file, " CROSS SECTIONS    = %d \n",body_surf.get_num_xsecs());
  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",body_surf.get_num_pnts());

  for ( i = 0 ; i < body_surf.get_num_xsecs() ; i++ )
    {
      body_surf.write_xsec(i, model_mat, dump_file );
    }

  if ( sym_code == NO_SYM ) return;

  fprintf(dump_file, "\n");
  fprintf(dump_file, "%s \n", (char*) getName());
  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
  fprintf(dump_file, " TYPE              = 1  \n");						// Non Lifting
  fprintf(dump_file, " CROSS SECTIONS    = %d \n",body_surf.get_num_xsecs());
  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",body_surf.get_num_pnts());

  for ( i = 0 ; i < body_surf.get_num_xsecs() ; i++ )
    {
      body_surf.write_refl_xsec(sym_code, i, reflect_mat, dump_file);
    }

}
//==== Aero Ref Parameters ====//
double PodGeom::getRefArea()
{
	if ( autoRefAreaFlag )
	{
		double len = length()*scaleFactor(); 
		double dia = len/fine_ratio();
		refArea = len*dia;
	}
	return refArea;
}
	
double PodGeom::getRefSpan()
{
	if ( autoRefSpanFlag )
	{
		double len = length()*scaleFactor(); 
		double dia = len/fine_ratio();
		refSpan = dia;
	}
	return refSpan;
}

double PodGeom::getRefCbar()
{
	if ( autoRefCbarFlag )
	{
		double len = length()*scaleFactor(); 
		refCbar = len;
	}
	return refCbar;
}

vec3d PodGeom::getAeroCenter()
{
	if ( autoAeroCenterFlag )
	{
		double len = length()*scaleFactor(); 
		aeroCenter.set_x( 0.25*len + get_tran_x() );
		aeroCenter.set_y( get_tran_y() );
		aeroCenter.set_z( get_tran_z() );
	}
	return aeroCenter;
}

void PodGeom::AddDefaultSources(double base_len)
{
	//==== Add Some Default Sources for CFD Mesh ====//
	PointSource* ps = new PointSource();
	ps->SetName("Nose");
	ps->SetLen( 0.1 );
	ps->SetRad( length()*0.1 );
	vec2d loc = vec2d(0,0);
	ps->SetUWLoc( loc );
	ps->SetGeomPtr( this );
	sourceVec.push_back( ps );

	ps = new PointSource();
	ps->SetName("Tail");
	ps->SetLen( 0.1 );
	ps->SetRad( length()*0.1 );
	loc = vec2d(1,0);
	ps->SetUWLoc( loc );
	ps->SetGeomPtr( this );
	sourceVec.push_back( ps );
}

void PodGeom::GetInteriorPnts( vector< vec3d > & pVec )
{
	vec3d p( 0.5*length(), 0.0, 0.0 );
	vec3d tp = p.transform( model_mat );
	pVec.push_back( tp ); 

	if (sym_code != NO_SYM)
	{
		tp = (p * sym_vec).transform(reflect_mat);
		pVec.push_back( tp );
    }
}
/*
vec3d PodGeom::getPickRay(double x, double y)
{
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ;
	GLdouble posX, posY, posZ;

	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetIntegerv( GL_VIEWPORT, viewport );

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

	gluUnProject( winX, winY, winZ, modelMatrix.data(), projection, viewport, &posX, &posY, &posZ);

	return vec3d(posX, posY, posZ);
}
*/

//=====================================================================================================//
//=====================================================================================================//
//=====================================================================================================//
//=====================================================================================================//
//=====================================================================================================//

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GeomGroup::GeomGroup(Aircraft* aptr) : Geom(aptr)
{
	type = GROUP_GEOM_TYPE;
	scaleLocFlag = 1;
}

GeomGroup::~GeomGroup()
{
}

void GeomGroup::clear()
{
	geomVec.clear();
}

//==== Parm Has Changed ReGenerate Fuse Component ====//
void GeomGroup::parm_changed(Parm* p)
{
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		if ( p == &scaleFactor )
		{
			geomVec[i]->scaleFactor.set( scaleFactor() );

			if ( scaleLocFlag )
			{
				geomVec[i]->xLocOff.set( geomVec[i]->xLoc()*(scaleFactor()-1.0) + xLocOff() );
				geomVec[i]->yLocOff.set( geomVec[i]->yLoc()*(scaleFactor()-1.0) + yLocOff() );
				geomVec[i]->zLocOff.set( geomVec[i]->zLoc()*(scaleFactor()-1.0) + zLocOff() );
				geomVec[i]->compose_model_matrix();
			}

			geomVec[i]->parm_changed( &(geomVec[i]->scaleFactor) );
		}
		else
		{
			geomVec[i]->xLocOff.set( xLocOff() );
			geomVec[i]->yLocOff.set( yLocOff() );
			geomVec[i]->zLocOff.set( zLocOff() );
			geomVec[i]->xRotOff.set( xRotOff() );
			geomVec[i]->yRotOff.set( yRotOff() );
			geomVec[i]->zRotOff.set( zRotOff() );
			geomVec[i]->compose_model_matrix();
		}
		geomVec[i]->updateAttach(0);

	}

	airPtr->geomMod( this );

}

void GeomGroup::resetXform()
{
	xLocOff.set( 0.0 );
	yLocOff.set( 0.0 );
	zLocOff.set( 0.0 );
	xRotOff.set( 0.0 );
	yRotOff.set( 0.0 );
	zRotOff.set( 0.0 );
	
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		geomVec[i]->xLocOff.set( xLocOff() );
		geomVec[i]->yLocOff.set( yLocOff() );
		geomVec[i]->zLocOff.set( zLocOff() );
		geomVec[i]->xRotOff.set( xRotOff() );
		geomVec[i]->yRotOff.set( yRotOff() );
		geomVec[i]->zRotOff.set( zRotOff() );
		geomVec[i]->compose_model_matrix();
	}
	airPtr->geomMod( this );
}

void GeomGroup::acceptXform()
{

	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		geomVec[i]->xLoc.set( geomVec[i]->xLoc() + xLocOff() );
		geomVec[i]->xLocOff.set( 0.0 );
		geomVec[i]->yLoc.set( geomVec[i]->yLoc() + yLocOff() );
		geomVec[i]->yLocOff.set( 0.0 );
		geomVec[i]->zLoc.set( geomVec[i]->zLoc() + zLocOff() );
		geomVec[i]->zLocOff.set( 0.0 );
		geomVec[i]->xRot.set( geomVec[i]->xRot() + xRotOff() );
		geomVec[i]->xRotOff.set( 0.0 );
		geomVec[i]->yRot.set( geomVec[i]->yRot() + yRotOff() );
		geomVec[i]->yRotOff.set( 0.0 );
		geomVec[i]->zRot.set( geomVec[i]->zRot() + zRotOff() );
		geomVec[i]->zRotOff.set( 0.0 );

		geomVec[i]->compose_model_matrix();
	}
	xLocOff.set( 0.0 );
	yLocOff.set( 0.0 );
	zLocOff.set( 0.0 );
	xRotOff.set( 0.0 );
	yRotOff.set( 0.0 );
	zRotOff.set( 0.0 );

	airPtr->geomMod( this );


}

void GeomGroup::acceptScaleFactor()
{
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		if ( scaleLocFlag )
		{
			geomVec[i]->xLoc.set( geomVec[i]->xLoc() + geomVec[i]->xLocOff() );
			geomVec[i]->xLocOff.set( 0.0 );
			geomVec[i]->yLoc.set( geomVec[i]->yLoc() + geomVec[i]->yLocOff() );
			geomVec[i]->yLocOff.set( 0.0 );
			geomVec[i]->zLoc.set( geomVec[i]->zLoc() + geomVec[i]->zLocOff() );
			geomVec[i]->zLocOff.set( 0.0 );
			geomVec[i]->compose_model_matrix();
		}

		geomVec[i]->acceptScaleFactor();
		scaleFactor.set(1.0);

		if ( scaleLocFlag )
		{
			geomVec[i]->compose_model_matrix();
		}

	}
}

void GeomGroup::resetScaleFactor()
{
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		geomVec[i]->resetScaleFactor();
		scaleFactor.set(1.0);

		if ( scaleLocFlag )
		{
			geomVec[i]->xLocOff.set( geomVec[i]->xLoc()*(scaleFactor()-1.0) + xLocOff() );
			geomVec[i]->yLocOff.set( geomVec[i]->yLoc()*(scaleFactor()-1.0) + yLocOff() );
			geomVec[i]->zLocOff.set( geomVec[i]->zLoc()*(scaleFactor()-1.0) + zLocOff() );
			geomVec[i]->compose_model_matrix();
		}
	}
}

void GeomGroup::setColor( double r, double g, double b)
{
	color.set_xyz( r, g, b );
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		geomVec[i]->setColor( r, g, b );
	}
}

void GeomGroup::setMaterialID( int m )
{
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		geomVec[i]->setMaterialID( m );
	}
}

//=====================================================================================================//
//=====================================================================================================//
//=====================================================================================================//
//=====================================================================================================//
//=====================================================================================================//

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BlankGeom::BlankGeom(Aircraft* aptr) : Geom(aptr)
{
	type = BLANK_GEOM_TYPE;
	type_str = Stringc("blank");

	char name[255];
	sprintf( name, "Blank_%d", geomCnt ); 
	geomCnt++;

	setName( Stringc(name) );

	pointMassFlag = false;
	pointMass.initialize(this, UPD_XFORM, "Point Mass", 1.0);
    pointMass.set_lower_upper( 0.0, 1000000.0);

}

BlankGeom::~BlankGeom()
{
}

void BlankGeom::copy( Geom* fromGeom )
{
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != BLANK_GEOM_TYPE )
		return;

	BlankGeom* bg = (BlankGeom*)fromGeom;

	pointMassFlag = bg->pointMassFlag;
	pointMass.set( bg->pointMass() );

	compose_model_matrix();


}

void BlankGeom::write(xmlNodePtr root)
{
  xmlAddStringNode( root, "Type", "Blank");

 

  //==== Write General Parms ====//
  xmlNodePtr gen_node = xmlNewChild( root, NULL, (const xmlChar *)"General_Parms", NULL );
  write_general_parms( gen_node );

  //==== Write Point Mass Parms ====//
  xmlNodePtr blank_node = xmlNewChild( root, NULL, (const xmlChar *)"Blank_Parms", NULL );

  xmlAddIntNode( blank_node, "PointMassFlag", pointMassFlag );
  xmlAddDoubleNode( blank_node, "PointMass", pointMass() );

}

//==== Read External File ====//
void BlankGeom::read(xmlNodePtr root)
{
  xmlNodePtr node;

  //===== Read General Parameters =====//
  node = xmlGetNode( root, "General_Parms", 0 );
  if ( node )
    read_general_parms( node );

  //===== Read Blank Parameters =====//
  xmlNodePtr blank_node = xmlGetNode( root, "Blank_Parms", 0 );
  if ( blank_node )
  {
		pointMassFlag = xmlFindInt( blank_node, "PointMassFlag", pointMassFlag );
		pointMass = xmlFindDouble( blank_node, "PointMass", pointMass() );
  }
  compose_model_matrix();
}



//==== Parm Has Changed ReGenerate Fuse Component ====//
void BlankGeom::parm_changed(Parm* p)
{
	int up_group = p->get_update_grp();
	switch (up_group)
	{
		case UPD_XFORM:
		{
			if ( p == &xLoc || p == &yLoc || p == &zLoc || 
				 p == &xRot || p == &yRot || p == &zRot )
				updateAttach(1);
			else
				updateAttach(0);

			compose_model_matrix();	
		}
		break;
	}

	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );
}


void BlankGeom::acceptScaleFactor()
{
}

void BlankGeom::resetScaleFactor()
{
}

void BlankGeom::draw()
{
		//==== Check Noshow Flag ====//
	if ( noshowFlag ) return;	

	if ( displayFlag == GEOM_WIRE_FLAG && redFlag )
	{
		glColor3ub( (int)color.x(), (int)color.y(), (int)color.z() );	

		//==== Draw Geom ====//
		glPushMatrix();
		glMultMatrixf((GLfloat*)model_mat); 

			glColor3ub( 255, 0, 255 );
			glPointSize( 10.0 );
			glBegin( GL_POINTS );
			glVertex3d( 0, 0, 0 );
			glEnd();

		glPopMatrix();
	}

}

//==== Draw If Alpha < 1 and Shaded ====//
void BlankGeom::drawAlpha()
{
}

void BlankGeom::loadPointMass( TetraMassProp* mp )
{
	vec3d loc = getTotalTranVec();
	mp->compId = (long)this;
	mp->SetPointMass( pointMass(), loc );
}

//=====================================================================================================//
//=====================================================================================================//
//=====================================================================================================//
//=====================================================================================================//
//=====================================================================================================//

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

XSecGeom::XSecGeom(Aircraft* aptr) : Geom(aptr)
{

	type = XSEC_GEOM_TYPE;
	type_str = Stringc("XSecGeom");

	char name[255];
	sprintf( name, "XSec_%d", geomCnt ); 
	geomCnt++;

	setName( Stringc(name) );

	surfVec.clear();
	for ( int i = 0 ; i < (int)xsecCompVec.size() ; i++ )
		surfVec.push_back( &xsecCompVec[i] );

	xScale.initialize(this, "XScale", 1.0);
    xScale.set_lower_upper(0.01, 100.0);
	yScale.initialize(this, "YScale", 1.0);
    yScale.set_lower_upper(0.01, 100.0);
	zScale.initialize(this, "ZScale", 1.0);
    zScale.set_lower_upper(0.01, 100.0);

	lastXScale = lastYScale = lastZScale = 1.0;

	uAngle.initialize(this, "UAngle", 90.0);
    uAngle.set_lower_upper(0, 180.0);

	wAngle.initialize(this, "WAngle", 90.0);
    wAngle.set_lower_upper(0, 180.0);
}

XSecGeom::~XSecGeom()
{

}

void XSecGeom::copy( Geom* fromGeom )
{
	int i;
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != XSEC_GEOM_TYPE )
		return;

	XSecGeom* g = (XSecGeom*)fromGeom;

	xScale.set( g->xScale() );
	yScale.set( g->yScale() );
	zScale.set( g->zScale() );
	uAngle.set( g->uAngle() );
	wAngle.set( g->wAngle() );

	xsecCompVec = g->xsecCompVec;

	surfVec.clear();
	for ( i = 0 ; i < (int)xsecCompVec.size() ; i++ )
		surfVec.push_back( &xsecCompVec[i] );
	
	compose_model_matrix();

	generate();
}

int XSecGeom::read_xsec( const char* file_name )
{
	int i, j, c;
	FILE *fp;
	char str[256];

	//==== Make Sure File Exists ====//
	if ( (fp = fopen(file_name, "r")) == (FILE *)NULL )
	{
		return 0;
	}

    //==== Read first Line of file and compare against expected header ====//
    fscanf(fp,"%s INPUT FILE\n\n",str);
	if ( strcmp("HERMITE",str) != 0 )  
	{
		return 0;
	}
     //==== Read in number of components ====//
	int num_comps;
	fscanf(fp," NUMBER OF COMPONENTS = %d\n",&num_comps);

	if ( num_comps <= 0 )
		return 0;

	xsecCompVec.resize( num_comps );

	for ( c = 0 ; c < num_comps ; c++ )
	{
		char name_str[256];
		float x, y, z;
		int group_num, type;
		int num_pnts, num_cross;

		fgets( name_str, 256, fp );
		fscanf(fp," GROUP NUMBER = %d\n",&group_num);
		fscanf(fp," TYPE = %d\n",&type);
		fscanf(fp," CROSS SECTIONS = %d\n",&(num_cross));
		fscanf(fp," PTS/CROSS SECTION = %d\n",&(num_pnts));

		//==== Set Component Values ====//
		xsecCompVec[c].set_num_pnts( num_pnts );
		xsecCompVec[c].set_num_xsecs( num_cross );

		//===== Size Cross Vec ====//
		for ( i = 0 ; i < num_cross ; i++)
		{
			for ( j = 0 ; j < num_pnts ; j++)
			{
				fscanf( fp, "%f %f %f\n", &x, &y, &z );
				xsecCompVec[c].set_pnt( i, j, vec3d(x,y,z) );
			}
		}
	}
	fclose(fp);

	generate();

	return 1;
}

void XSecGeom::write(xmlNodePtr root)
{
	int i;
	xmlAddStringNode( root, "Type", type_str.get_char_star() );

	//==== Write General Parms ====//
	xmlNodePtr gen_node = xmlNewChild( root, NULL, (const xmlChar *)"General_Parms", NULL );
	write_general_parms( gen_node );

	////==== Write XSec Parms ====//
	xmlNodePtr xsec_node = xmlNewChild( root, NULL, (const xmlChar *)"XSec_Parms", NULL );
	xmlAddDoubleNode( xsec_node, "XScale", xScale() );
	xmlAddDoubleNode( xsec_node, "YScale", yScale() );
	xmlAddDoubleNode( xsec_node, "ZScale", zScale() );
	xmlAddDoubleNode( xsec_node, "UAngle", uAngle() );
	xmlAddDoubleNode( xsec_node, "WAngle", wAngle() );

	//===== Write XSec Surfs =====//
	xmlNodePtr xs_surf_node;
	xmlNodePtr xs_surf_list_node = xmlNewChild( root, NULL, (const xmlChar *)"Xsec_Surf_List", NULL );

	for ( i = 0 ; i < (int)xsecCompVec.size() ; i++ )
	{
		xs_surf_node = xmlNewChild( xs_surf_list_node, NULL, (const xmlChar *)"Xsec_Surf", NULL );
		xsecCompVec[i].write( xs_surf_node );
	}
}

//==== Read External File ====//
void XSecGeom::read(xmlNodePtr root)
{
	int i;
	xmlNodePtr node;

	//===== Read General Parameters =====//
	node = xmlGetNode( root, "General_Parms", 0 );
	if ( node )
	read_general_parms( node );

	//===== Read Fuse Parameters =====//
	node = xmlGetNode( root, "XSec_Parms", 0 );
	if ( node )
	{
	  xScale = xmlFindDouble( node, "XScale", xScale() );
	  yScale = xmlFindDouble( node, "YScale", yScale() );
	  zScale = xmlFindDouble( node, "XScale", zScale() );
	  uAngle = xmlFindDouble( node, "UAngle", uAngle() );
	  wAngle = xmlFindDouble( node, "WAngle", wAngle() );
	}

	//==== Read Cross Sections ====//
	xmlNodePtr xs_surf_list_node = xmlGetNode( root, "Xsec_Surf_List", 0 );
	if ( xs_surf_list_node )
	{
		int num_xs_surfs = xmlGetNumNames( xs_surf_list_node, "Xsec_Surf" );
		xsecCompVec.resize( num_xs_surfs );

		for ( i = 0 ; i < num_xs_surfs ; i++ )
		{
			xmlNodePtr xs_surf_node = xmlGetNode( xs_surf_list_node, "Xsec_Surf", i );
			xsecCompVec[i].read( xs_surf_node );
		}
	}	
	generate();
}

void XSecGeom::LoadLinkableParms( vector< Parm* > & parmVec )
{
	Geom::LoadLinkableParms( parmVec );
	Geom::AddLinkableParm( &xScale, parmVec, this, "XScale" );
	Geom::AddLinkableParm( &yScale, parmVec, this, "YScale" );
	Geom::AddLinkableParm( &zScale, parmVec, this, "ZScale" );
}

void XSecGeom::generate()
{
	surfVec.clear();
	for ( int i = 0 ; i < (int)xsecCompVec.size() ; i++ )
	{
		xsecCompVec[i].load_refl_pnts_xsecs();
		xsecCompVec[i].load_hidden_surf();
		xsecCompVec[i].load_normals();
		xsecCompVec[i].load_uw();
		surfVec.push_back( &xsecCompVec[i] );
	}
	update_bbox();
}

void XSecGeom::computeCenter()
{
	//==== Set Rotation Center ====//
	double len = bnd_box.get_max(0) - bnd_box.get_min(0);
	center.set_x( origin()*len ); 
}

//==== Parm Has Changed ReGenerate Component ====//
void XSecGeom::parm_changed(Parm* p)
{
	if ( p == &xLoc || p == &yLoc || p == &zLoc || 
		 p == &xRot || p == &yRot || p == &zRot )
		updateAttach(1);
	else
		updateAttach(0);

	if ( p == &scaleFactor || p == &xScale || p == &yScale || p == &zScale )	
	{
		scale();
		generate();
		updateAttach(0);
	}

	generate();

	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );
}

void XSecGeom::scale()
{
	double current_factor = scaleFactor()*(1.0/lastScaleFactor);

	double x_factor = xScale()*(1.0/lastXScale);
	double y_factor = yScale()*(1.0/lastYScale);
	double z_factor = zScale()*(1.0/lastZScale);

	for ( int nb = 0 ; nb < (int)xsecCompVec.size() ; nb++ )
	{
		int num_pnts  = xsecCompVec[nb].get_num_pnts();
		int num_xsecs = xsecCompVec[nb].get_num_xsecs();

		  //==== Check All Xsecs ====//
		  for ( int i = 0 ; i < num_xsecs ; i++ )
			for ( int j = 0 ; j < num_pnts ; j++ )
			{
				vec3d p = xsecCompVec[nb].get_pnt(i,j);
				vec3d sp = vec3d( p.x()*current_factor*x_factor, 
					              p.y()*current_factor*y_factor,
								  p.z()*current_factor*z_factor );

				xsecCompVec[nb].set_pnt( i, j, sp );

			}
	}
	lastScaleFactor = scaleFactor();
	lastXScale = xScale();
	lastYScale = yScale();
	lastZScale = zScale();
}

void XSecGeom::acceptScaleFactor()
{
	lastScaleFactor = 1.0;
	scaleFactor.set(1.0);
}

void XSecGeom::resetScaleFactor()
{
	scaleFactor.set( 1.0/lastScaleFactor );
	lastScaleFactor = 1.0;
	scale();
	generate();
	airPtr->geomMod( this );
	lastScaleFactor = 1.0;
	scaleFactor.set(1.0);
}

void XSecGeom::draw()
{
	Geom::draw();
}

//==== Draw If Alpha < 1 and Shaded ====//
void XSecGeom::drawAlpha()
{
	Geom::drawAlpha();
}

//==== Update Bounding Box =====//
void XSecGeom::update_bbox()
{
  bbox new_box;

  for ( int nb = 0 ; nb < (int)xsecCompVec.size() ; nb++ )
  {
	int num_pnts  = xsecCompVec[nb].get_num_pnts();
	int num_xsecs = xsecCompVec[nb].get_num_xsecs();

	  //==== Check All Xsecs ====//
	  for ( int i = 0 ; i < num_xsecs ; i++ )
		for ( int j = 0 ; j < num_pnts ; j++ )
		{
			new_box.update(xsecCompVec[nb].get_pnt(i,j));
		}
  }

  bnd_box = new_box;
  update_xformed_bbox();			// Load Xform BBox
}

vec3d XSecGeom::getAttachUVPos(double u, double v)
{
	vec3d pos;

	double ubig = u*(double)xsecCompVec.size();
	int cind = (int)(ubig - 0.0000001);

	double remu = ubig - cind;

	vec3d uvpnt = xsecCompVec[cind].comp_uv_pnt( remu, v );

	pos = uvpnt.transform( model_mat );

	return pos;
}

//==== Write Rhino File ====//
void XSecGeom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
{

	for ( int i = 0 ; i < (int)xsecCompVec.size() ; i++ )
	{
		xsecCompVec[i].load_refl_pnts_xsecs();
		xsecCompVec[i].clear_pnt_tan_flags();
		xsecCompVec[i].clear_xsec_tan_flags();

		//==== Set Splits ====//
		vector <int> u_split;
		u_split.push_back( 0 );
		u_split.push_back( 3*(xsecCompVec[i].get_num_xsecs()/2) );
		u_split.push_back( 3*(xsecCompVec[i].get_num_xsecs() - 1) );

		vector <int> w_split;
		w_split.push_back( 0 );
		w_split.push_back( 3*(xsecCompVec[i].get_num_pnts()/2)  );
		w_split.push_back( 3*(xsecCompVec[i].get_num_pnts() - 1)  );

		//==== Write File ====//
		xsecCompVec[i].write_split_rhino_file( sym_code, model_mat, reflect_mat, 
			u_split, w_split, false, archive, attributes );
	}
}

int  XSecGeom::get_num_bezier_comps()
{
	if ( sym_code == NO_SYM )
		return 1;
	else 
		return 2;
}

void XSecGeom::write_bezier_file( int id, FILE* file_id )
{
	for ( int c = 0 ; c < (int)xsecCompVec.size() ; c++ )
	{
		xsecCompVec[c].load_refl_pnts_xsecs();
		xsecCompVec[c].clear_pnt_tan_flags();
		xsecCompVec[c].clear_xsec_tan_flags();

		//==== Load U Samples ====//
		int half_ind = xsecCompVec[c].get_num_pnts()/2;
		vector< vec3d > uSamps;
		for ( int i = 0 ; i < xsecCompVec[c].get_num_xsecs() ; i++ )
			uSamps.push_back( xsecCompVec[c].get_pnt( i, half_ind ) );

		vector< int > tan_flags = find_tan_flags( uSamps, uAngle() );
		for ( int i = 0 ; i < (int)tan_flags.size() ; i++ )
		{
			if ( tan_flags[i] )
				xsecCompVec[c].set_xsec_tan_flag( i, Bezier_curve::SHARP );
		}

		//==== Load W Samples ====//
		half_ind = xsecCompVec[c].get_num_xsecs()/2;
		vector< vec3d > wSamps;
		for ( int i = 0 ; i < xsecCompVec[c].get_num_pnts() ; i++ )
			wSamps.push_back( xsecCompVec[c].get_pnt( half_ind, i ) );

		tan_flags = find_tan_flags( wSamps, wAngle() );
		for ( int i = 0 ; i < (int)tan_flags.size() ; i++ )
		{
			if ( tan_flags[i] )
				xsecCompVec[c].set_pnt_tan_flag( i, Bezier_curve::SHARP );
		}

		//==== Set Splits ====//
		vector <int> u_split;
		u_split.push_back( 0 );
		u_split.push_back( 3*(xsecCompVec[c].get_num_xsecs()/2) );
		u_split.push_back( 3*(xsecCompVec[c].get_num_xsecs() - 1) );

		vector <int> w_split;
		w_split.push_back( 0 );
		w_split.push_back( 3*(xsecCompVec[c].get_num_pnts()/4)  );
		w_split.push_back( 3*(xsecCompVec[c].get_num_pnts()/2)  );
		w_split.push_back( 3*((3*xsecCompVec[c].get_num_pnts())/4)  );
		w_split.push_back( 3*(xsecCompVec[c].get_num_pnts() - 1)  );

		xsecCompVec[c].write_bezier_file( file_id, sym_code, model_mat, reflect_mat, u_split, w_split  );
	}
}

vector<int> XSecGeom::find_tan_flags( vector< vec3d > pntVec, double angle_limit )
{
	vector< int > retVec;
	if ( pntVec.size() < 4 )
		return retVec;

	int last_ind = pntVec.size()-1;
	bool closed_flag = false;
	if ( dist( pntVec[0], pntVec[last_ind] ) < 0.000000001 )
		closed_flag = true;

	for ( int i = 0 ; i < (int)pntVec.size() ; i++ )
	{
		double ang = 0.0;
		if ( i == 0 && closed_flag )
			ang = three_pnt_angle( pntVec[last_ind-1], pntVec[0], pntVec[1] );
		else if ( i == last_ind && closed_flag )
			ang = three_pnt_angle( pntVec[last_ind-1], pntVec[last_ind], pntVec[1] );
		else if ( i > 0 && i < last_ind )
			ang = three_pnt_angle( pntVec[i-1], pntVec[i], pntVec[i+1] );

		if ( ang > DEG2RAD(angle_limit) )
			retVec.push_back( 1 );
		else
			retVec.push_back( 0 );
	}

	return retVec;
}
	
double XSecGeom::three_pnt_angle( vec3d& p0, vec3d & p1, vec3d & p2 )
{
	vec3d p1p0 = p1 - p0;
	vec3d p2p1 = p2 - p1;
	return angle( p1p0, p2p1 );
}

//==== Convert To Tri Mesh ====//
vector< TMesh* > XSecGeom::createTMeshVec()
{
	vector< TMesh* > tMeshVec;

	for ( int i = 0 ; i < (int)xsecCompVec.size() ; i++ )
	{
		tMeshVec.push_back( xsecCompVec[i].createTMesh(model_mat) );
	}

	if ( sym_code != NO_SYM )
	{
		for ( int i = 0 ; i < (int)xsecCompVec.size() ; i++ )
		{
			tMeshVec.push_back( xsecCompVec[i].createReflTMesh(sym_code, reflect_mat) );
		}
	}
	return tMeshVec;
}

void XSecGeom::dump_xsec_file(int geom_no, FILE* dump_file)
{
	int i, c;

	for ( c = 0 ; c < (int)xsecCompVec.size() ; c++ )
	{
		fprintf(dump_file, "\n");
		fprintf(dump_file, "%s \n", (char*) getName());
		fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
		fprintf(dump_file, " TYPE              = 1  \n");						// Non Lifting
		fprintf(dump_file, " CROSS SECTIONS    = %d \n",xsecCompVec[c].get_num_xsecs());
		fprintf(dump_file, " PTS/CROSS SECTION = %d \n",xsecCompVec[c].get_num_pnts());

		for ( i = 0 ; i < xsecCompVec[c].get_num_xsecs() ; i++ )
		{
			xsecCompVec[c].write_xsec(i, model_mat, dump_file );
		}

		if ( sym_code == NO_SYM ) return;

		fprintf(dump_file, "\n");
		fprintf(dump_file, "%s \n", (char*) getName());
		fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
		fprintf(dump_file, " TYPE              = 1  \n");						// Non Lifting
		fprintf(dump_file, " CROSS SECTIONS    = %d \n",xsecCompVec[c].get_num_xsecs());
		fprintf(dump_file, " PTS/CROSS SECTION = %d \n",xsecCompVec[c].get_num_pnts());

		for ( i = 0 ; i < xsecCompVec[c].get_num_xsecs() ; i++ )
		{
			xsecCompVec[c].write_refl_xsec(sym_code, i, reflect_mat, dump_file);
		}
	}
}
//==== Aero Ref Parameters ====//
double XSecGeom::getRefArea()
{
	if ( autoRefAreaFlag )
	{
		double len = bnd_box.get_max(0) - bnd_box.get_min(0);
		double dia = bnd_box.get_max(1) - bnd_box.get_min(1);
		refArea = len*dia;
	}
	return refArea;
}
	
double XSecGeom::getRefSpan()
{
	if ( autoRefSpanFlag )
	{
		double dia = bnd_box.get_max(1) - bnd_box.get_min(1);
		refSpan = dia;
	}
	return refSpan;
}

double XSecGeom::getRefCbar()
{
	if ( autoRefCbarFlag )
	{
		double len = bnd_box.get_max(0) - bnd_box.get_min(0);
		refCbar = len;
	}
	return refCbar;
}

vec3d XSecGeom::getAeroCenter()
{
	if ( autoAeroCenterFlag )
	{
		double len = bnd_box.get_max(0) - bnd_box.get_min(0);
		aeroCenter.set_x( 0.25*len + get_tran_x() );
		aeroCenter.set_y( get_tran_y() );
		aeroCenter.set_z( get_tran_z() );
	}
	return aeroCenter;
}

void XSecGeom::AddDefaultSources(double base_len)
{
	//==== Add Some Default Sources for CFD Mesh ====//
	double length = bnd_box.get_max(0) - bnd_box.get_min(0);
	PointSource* ps = new PointSource();
	ps->SetName("Nose");
	ps->SetLen( 0.2*base_len );
	ps->SetRad( 0.1*length );
	vec2d loc = vec2d(0,0);
	ps->SetUWLoc( loc );
	sourceVec.push_back( ps );

	ps = new PointSource();
	ps->SetName("Aft");
	ps->SetLen( 0.2*base_len );
	ps->SetRad( 0.1*length );
	loc = vec2d(1,0);
	ps->SetUWLoc( loc );
	sourceVec.push_back( ps );
}

void XSecGeom::GetInteriorPnts( vector< vec3d > & pVec )
{
	for ( int c = 0 ; c < (int)xsecCompVec.size() ; c++ )
	{
		int np = xsecCompVec[c].get_num_pnts();
		int nxs = xsecCompVec[c].get_num_xsecs();
		vec3d p0 = xsecCompVec[c].get_pnt( nxs/2, np/4 );
		vec3d p1 = xsecCompVec[c].get_pnt( nxs/2, 3*np/4 );
		vec3d p  = (p0 + p1)*0.5;
		vec3d tp = p.transform( model_mat );
		pVec.push_back( tp ); 

		if (sym_code != NO_SYM)
		{
			tp = (p * sym_vec).transform(reflect_mat);
			pVec.push_back( tp );
		}
	}
}

vec3d XSecGeom::getVertex3d(int surfid, double x, double p, int r)			
{ 
	return xsecCompVec[surfid].get_vertex(x, p, r); 
}

void  XSecGeom::getVertexVec(vector< VertexID > *vertVec)
{
	for (int i = 0; i < (int)xsecCompVec.size(); i++) 
		buildVertexVec(&xsecCompVec[i], i, vertVec); 
}
