//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// aircraft.cpp: implementation of the aircraft class.
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

#include "aircraft.h"
#include "screenMgr.h"
#include "vspGlWindow.h"
#include "geomScreen.h"
#include "wingScreen.h"
#include "fuseScreen.h"
#include "havocScreen.h"
#include "extScreen.h"
#include "msWingScreen.h"
#include "hwbScreen.h"
#include "blankScreen.h"
#include "meshScreen.h"
#include "cabinLayoutScreen.h"
#include "hrmScreen.h"
#include "ductScreen.h"
#include "propScreen.h"
#include "engineScreen.h"
#include "labelScreen.h"
#include "parmPickerScreen.h"
#include "tMesh.h"
#include "vorGeom.h"
#include "structureMgr.h"
#include "CfdMeshMgr.h"
#include "FeaMeshMgr.h"
#include "parmLinkMgr.h"
#include "VspPreferences.h"
#include "scriptMgr.h"
#include "materialMgr.h"


// Include OpenNurbs for Rhino Dump
// ON Needs to be undefined for it to compile
//
#undef ON
#include "opennurbs.h"
#include "opennurbs_extensions.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Aircraft::Aircraft()
{

//	activeGeom = 0;
	nameStr = "Aircraft";
	fileName = "VspAircraft.vsp";
	tempDirName  = "./";
	tempDirUserOverride = false;
	guess_far_field = 1;

	drawCGFlag = 0;
	drawACFlag = 0;
	cgRelAcFlag = 0;
	cgLoc = vec3d(10.0, 0.0, 0.0);

	total_mass = 1.0;
	Ixx_Iyy_Ixx = vec3d(1.0, 1.0, 1.0);
	Ixy_Ixz_Iyz = vec3d(0.0, 0.0, 0.0);

	screenMgr = 0;
	drawWin = 0;

	editGeom = NULL;
	backupGeom = NULL;

	vorGeom = new VorGeom(this);
	userGeomPtr = new UserGeom(this);

	//==== Initialize Structure Mgr ====//
	structureMgrPtr->SetAircraftPtr( this );
	cfdMeshMgrPtr->SetAircraftPtr(this);
	cfdMeshMgrPtr->ResetExportFileNames();
	feaMeshMgrPtr->SetAircraftPtr(this);
	feaMeshMgrPtr->ResetFeaExportFileNames();
	parmLinkMgrPtr->SetAircraftPtr(this);
	pHolderListMgrPtr->SetAircraftPtr(this);
	parmMgrPtr->SetAircraftPtr(this);


	//==== Read Custom Default Components ====//
	setDefaultCompGroupID( VspPreferences::Instance()->getDefaultCompFileID() );

	compGeomTxtFileName = "VspAircraft_CompGeom.txt";
	compGeomCsvFileName = "VspAircraft_CompGeom.csv";
	sliceFileName = "VspAircraft_Slice.txt";
	massPropFileName = "VspAircraft_MassProp.txt";
	exportCompGeomCsvFile = true;

	dragBuildTsvFileName = "VspAircraft_DragBuild.tsv";
	exportDragBuildTsvFile = false;

}

Aircraft::~Aircraft()
{
	int i;

	if (backupGeom != NULL)
		delete backupGeom;

	//==== Order Geom List ====//
	vector< Geom* > gVec;
	for ( i = 0 ; i < (int)topGeomVec.size() ; i++ )
	{	
		topGeomVec[i]->loadChildren( gVec );
	}

	//==== Delete All Geoms ====//
	for ( i = 0 ; i < (int)gVec.size() ; i++ )
		delete gVec[i];

	//==== Clear Clip Board ====//
	for ( i = 0 ; i < (int)clipBoard.size() ; i++ )
		delete clipBoard[i];


	VspPreferences::Instance()->saveFile();


	if ( userGeomPtr )
		delete userGeomPtr;

	if ( cfdMeshMgrPtr )
		delete cfdMeshMgrPtr;

	if ( feaMeshMgrPtr )
		delete feaMeshMgrPtr;

	if ( parmLinkMgrPtr )
		delete parmLinkMgrPtr;

	if ( pHolderListMgrPtr )
		delete pHolderListMgrPtr;

	if ( parmMgrPtr )
		delete parmMgrPtr;

	if ( texMgrPtr )
		delete texMgrPtr;

	if ( fontMgr )
		delete fontMgr;

	if ( structureMgrPtr )
		delete structureMgrPtr;

	if ( scriptMgr )
		delete scriptMgr;

	if ( matMgrPtr )
		delete matMgrPtr;

	VspPreferences::DeleteInstance();


//	DumpUnfreed();

}


void Aircraft::setScreenMgr( ScreenMgr* screenMgrIn )
{
	screenMgr = screenMgrIn;
	drawWin   = screenMgr->getDrawWin();

}

void Aircraft::geomMod( Geom* gPtr )
{ 
	structureMgrPtr->TriggerUpdate();
	if (drawWin) drawWin->redraw(); 
}

Geom* Aircraft::createGeom( int type )
{
	Geom* newGeom = 0;

	switch (type)
	{
		case POD_GEOM_TYPE:
			newGeom = new PodGeom( this );
			break;
		case BLANK_GEOM_TYPE:
			newGeom = new BlankGeom( this );
			break;
		case WING_GEOM_TYPE:
			newGeom = new WingGeom( this );
			break;
		case FUSE_GEOM_TYPE:
			newGeom = new Fuse_geom( this );
			break;
		case HAVOC_GEOM_TYPE:
			newGeom = new Havoc_geom( this );
			break;
		case EXT_GEOM_TYPE:
			newGeom = new Ext_geom( this );
			break;
		case MS_WING_GEOM_TYPE:
			newGeom = new Ms_wing_geom( this );
			break;
		case DUCT_GEOM_TYPE:
			newGeom = new DuctGeom( this );
			break;
		case PROP_GEOM_TYPE:
			newGeom = new PropGeom( this );
			break;
		case ENGINE_GEOM_TYPE:
			newGeom = new EngineGeom( this );
			break;
		case HWB_GEOM_TYPE:
			newGeom = new Hwb_geom( this );
			break;
		case FUSELAGE_GEOM_TYPE:
			newGeom = new FuselageGeom( this );
			break;
		case CABIN_LAYOUT_GEOM_TYPE:
			newGeom = new CabinLayoutGeom(this);
			break;
		case XSEC_GEOM_TYPE:
			newGeom = new XSecGeom( this );
			break;
	}

	//==== Check For Custom Default Component ====//
	map< int, Geom* >::iterator iter = m_DefaultCompMap.find( type );
	if ( iter != m_DefaultCompMap.end() )
	{
		newGeom->copy( m_DefaultCompMap[type] );
	}

	for ( int j = 0; j < (int) geomVec.size(); j++ )
	{
		if ( newGeom->getPtrID() == geomVec[j]->getPtrID() )
			newGeom->resetPtrID(); // Collision detected
	}

	addGeom( newGeom );

	if (screenMgr) screenMgr->updateGeomScreens();

	parmMgrPtr->RebuildAll();

	return newGeom;

}

Geom* Aircraft::copyGeom( Geom* geomPtr )
{
	Geom* rtnPtr = 0;
	if ( geomPtr->getType() == POD_GEOM_TYPE )
	{
		PodGeom* newGeom = new PodGeom( this );
		newGeom->copy( geomPtr );
		rtnPtr = newGeom;
	}
	else if ( geomPtr->getType() == BLANK_GEOM_TYPE )
	{
		BlankGeom* newGeom = new BlankGeom( this );
		newGeom->copy( geomPtr );
		rtnPtr = newGeom;
	}
	else if ( geomPtr->getType() == WING_GEOM_TYPE )
	{
		WingGeom* newGeom = new WingGeom( this );
		newGeom->copy( geomPtr );
		rtnPtr = newGeom;
	}
	else if ( geomPtr->getType() == FUSE_GEOM_TYPE )
	{
		Fuse_geom* newGeom = new Fuse_geom( this );
		newGeom->copy( geomPtr );
		rtnPtr = newGeom;
	}		
	else if ( geomPtr->getType() == HAVOC_GEOM_TYPE )
	{
		Havoc_geom* newGeom = new Havoc_geom( this );
		newGeom->copy( geomPtr );
		rtnPtr = newGeom;
	}		
	else if ( geomPtr->getType() == EXT_GEOM_TYPE )
	{
		Ext_geom* newGeom = new Ext_geom( this );
		newGeom->copy( geomPtr );
		rtnPtr = newGeom;
	}		
	else if ( geomPtr->getType() == MS_WING_GEOM_TYPE )
	{
		Ms_wing_geom* newGeom = new Ms_wing_geom( this );
		newGeom->copy( geomPtr );
		rtnPtr = newGeom;
	}		
	else if ( geomPtr->getType() == MESH_GEOM_TYPE )
	{
		MeshGeom* newGeom = new MeshGeom( this );
		newGeom->copy( geomPtr );
		rtnPtr = newGeom;
	}		
//		else if ( geomPtr->getType() == VOR_GEOM_TYPE )
//		{
//			VorGeom* newGeom = new VorGeom( this );
//			newGeom->copy( geomPtr );
//			rtnPtr = newGeom;
//		}		
	else if ( geomPtr->getType() == DUCT_GEOM_TYPE )
	{
		DuctGeom* newGeom = new DuctGeom( this );
		newGeom->copy( geomPtr );
		rtnPtr = newGeom;
	}		
	else if ( geomPtr->getType() == PROP_GEOM_TYPE )
	{
		PropGeom* newGeom = new PropGeom( this );
		newGeom->copy( geomPtr );
		rtnPtr = newGeom;
	}		
	else if ( geomPtr->getType() == ENGINE_GEOM_TYPE )
	{
		EngineGeom* newGeom = new EngineGeom( this );
		newGeom->copy( geomPtr );
		rtnPtr = newGeom;
	}
	else if ( geomPtr->getType() == HWB_GEOM_TYPE )
	{
		Hwb_geom* newGeom = new Hwb_geom( this );
		newGeom->copy( geomPtr );
		rtnPtr = newGeom;
	}	
	else if ( geomPtr->getType() == FUSELAGE_GEOM_TYPE )
	{
		FuselageGeom* newGeom = new FuselageGeom( this );
		newGeom->copy( geomPtr );
		rtnPtr = newGeom;
	}
	else if ( geomPtr->getType() == CABIN_LAYOUT_GEOM_TYPE )
	{
		CabinLayoutGeom* newGeom = new CabinLayoutGeom( this );
		newGeom->copy( geomPtr );
		rtnPtr = newGeom;
	}
	else if ( geomPtr->getType() == XSEC_GEOM_TYPE )
	{
		XSecGeom* newGeom = new XSecGeom( this );
		newGeom->copy( geomPtr );
		rtnPtr = newGeom;
	}

	return rtnPtr;
}

void Aircraft::revertEdits()
{
	if (editGeom) editGeom->copy(backupGeom);
	if (screenMgr) screenMgr->updateGeomScreens();
}

void Aircraft::addGeom( Geom* addGeom )
{
	Geom * activeGeom = getActiveGeom();
	if ( addGeom )
	{
		if ( activeGeom )	// Make this parent
		{
			activeGeom->addChild( addGeom );
			addGeom->setParent( activeGeom );
		}
		else				// Make aircraft parent
		{
			addGeom->setParent( 0 );
		}
	}

	setActiveGeom(addGeom);

	//==== Add to Tree ====//	
	if ( addGeom->getParent() == 0 )
	{
		topGeomVec.push_back( addGeom );
	}

	//==== Order Geom List ====//
	geomVec.clear();
	for ( int i = 0 ; i < (int)topGeomVec.size() ; i++ )
	{	
		topGeomVec[i]->loadChildren( geomVec );
	}

	//==== Recenter Geometry ====//
	update_bbox();
	if (drawWin) 
	{
		drawWin->centerAllViews();
		drawWin->redraw(); 
	}

	//==== Recenter Geometry ====//
	recenterACView();
}

void Aircraft::recenterACView()
{
	update_bbox();
	if (drawWin) 
	{
		drawWin->centerAllViews();
		drawWin->redraw(); 
	}
}

void Aircraft::reorderGeom( int action )
{
	Geom * activeGeom = getActiveGeom();

	if ( !activeGeom )
		return;

	vector< Geom* > gVec;
	if ( activeGeom->getParent() == 0 )
		gVec = topGeomVec;
	else
		gVec = activeGeom->getParent()->getChildren();

	vector< Geom* > newVec;

	if ( action == MOVE_TOP || action == MOVE_BOT )
	{
		if ( action == MOVE_TOP )
			newVec.push_back( activeGeom );

		for ( int i = 0 ; i < (int)gVec.size() ; i++ )
			if ( gVec[i] != activeGeom )
				newVec.push_back( gVec[i] );

		if ( action == MOVE_BOT )
			newVec.push_back( activeGeom );
	}
	else if ( action == MOVE_UP || action == MOVE_DOWN )
	{
		for ( int i = 0 ; i < (int)gVec.size() ; i++ )
		{
			if ( i < (int)(gVec.size()-1) &&
				 ( (action == MOVE_DOWN && gVec[i] == activeGeom) ||
				   (action == MOVE_UP   && gVec[i+1] == activeGeom) ) )
			{
				newVec.push_back( gVec[i+1] );
				newVec.push_back( gVec[i] );
				i++;
			}
			else
			{
				newVec.push_back( gVec[i] );
			}
		}
	}
		
	if ( activeGeom->getParent() == 0 )
		topGeomVec = newVec;
	else
		activeGeom->getParent()->setChildren( newVec );

	//==== Order Geom List ====//
	geomVec.clear();
	for ( int i = 0 ; i < (int)topGeomVec.size() ; i++ )
	{	
		topGeomVec[i]->loadChildren( geomVec );
	}

}

void Aircraft::writeFile( const char* file_name, bool restore_file_name )
{
	vector< Geom* > gVec = geomVec;
	gVec.push_back( getUserGeom() );
	writeFile( file_name, gVec, labelVec, restore_file_name );
}

void Aircraft::writeFile( const char* file_name, vector< Geom * > &gVec, vector< LabelGeom * > &lVec, bool restore_file_name )
{
	if ( !restore_file_name )
	{
		fileName = file_name;
		setTempDir( fileName, false );
	}
	
	xmlDocPtr doc = xmlNewDoc((const xmlChar *)"1.0");

	xmlNodePtr root = xmlNewNode(NULL,(const xmlChar *)"Vsp_Geometry");
	xmlDocSetRootElement(doc, root);

	int version = 3;
	xmlAddIntNode( root, "Version", version );

	xmlAddStringNode( root, "Name", nameStr.get_char_star() );

	//==== Aero Reference Stuff ====//
	xmlAddIntNode( root, "CG_Rel_AC_Flag", cgRelAcFlag );

	xmlAddDoubleNode( root, "CG_X", cgLoc.x() );
	xmlAddDoubleNode( root, "CG_Y", cgLoc.y() );
	xmlAddDoubleNode( root, "CG_Z", cgLoc.z() );
	xmlAddDoubleNode( root, "CFD_Mesh_Base_Length", cfdMeshMgrPtr->GetGridDensityPtr()->GetBaseLen() );
	xmlAddDoubleNode( root, "CFD_Mesh_Min_Length", cfdMeshMgrPtr->GetGridDensityPtr()->GetMinLen() );
	xmlAddDoubleNode( root, "CFD_Mesh_Max_Gap", cfdMeshMgrPtr->GetGridDensityPtr()->GetMaxGap() );
	xmlAddDoubleNode( root, "CFD_Mesh_Num_Circle_Segments", cfdMeshMgrPtr->GetGridDensityPtr()->GetNCircSeg() );
	xmlAddDoubleNode( root, "CFD_Mesh_Growth_Ratio", cfdMeshMgrPtr->GetGridDensityPtr()->GetGrowRatio() );
	xmlAddDoubleNode( root, "CFD_Mesh_Far_Max_Length", cfdMeshMgrPtr->GetGridDensityPtr()->GetFarMaxLen() );
	xmlAddDoubleNode( root, "CFD_Mesh_Far_Max_Gap", cfdMeshMgrPtr->GetGridDensityPtr()->GetFarMaxGap() );
	xmlAddDoubleNode( root, "CFD_Mesh_Far_Num_Circle_Segments", cfdMeshMgrPtr->GetGridDensityPtr()->GetFarNCircSeg() );
	xmlAddIntNode( root, "CFD_Mesh_Rigorous_Limiting", cfdMeshMgrPtr->GetGridDensityPtr()->GetRigorLimit() );
	xmlAddDoubleNode( root, "CFD_Far_Field_Scale_X", cfdMeshMgrPtr->GetFarXScale() );
	xmlAddDoubleNode( root, "CFD_Far_Field_Scale_Y", cfdMeshMgrPtr->GetFarYScale() );
	xmlAddDoubleNode( root, "CFD_Far_Field_Scale_Z", cfdMeshMgrPtr->GetFarZScale() );
	xmlAddIntNode( root, "CFD_Half_Mesh_Flag", cfdMeshMgrPtr->GetHalfMeshFlag() );
	xmlAddIntNode( root, "CFD_Far_Mesh_Flag", cfdMeshMgrPtr->GetFarMeshFlag() );
	xmlAddIntNode( root, "CFD_Far_Abs_Size_Flag", cfdMeshMgrPtr->GetFarAbsSizeFlag() );
	xmlAddIntNode( root, "CFD_Far_Man_Loc_Flag", cfdMeshMgrPtr->GetFarManLocFlag() );
	xmlAddIntNode( root, "CFD_Far_Comp_Flag", cfdMeshMgrPtr->GetFarCompFlag() );
	xmlAddIntNode( root, "CFD_Far_Geom_PtrID", cfdMeshMgrPtr->GetFarGeomID() );
	xmlAddDoubleNode( root, "CFD_Wake_Angle", cfdMeshMgrPtr->GetWakeAngle() );
	xmlAddDoubleNode( root, "CFD_Wake_Scale", cfdMeshMgrPtr->GetWakeScale() );

	xmlAddIntNode( root, "CFD_Stl_File_Flag", cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::STL_FILE_NAME ) );
	xmlAddIntNode( root, "CFD_Poly_File_Flag", cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::POLY_FILE_NAME ) );
	xmlAddIntNode( root, "CFD_Tri_File_Flag", cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::TRI_FILE_NAME ) );
	xmlAddIntNode( root, "CFD_Obj_File_Flag", cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::OBJ_FILE_NAME ) );
	xmlAddIntNode( root, "CFD_Dat_File_Flag", cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::DAT_FILE_NAME ) );
	xmlAddIntNode( root, "CFD_Key_File_Flag", cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::KEY_FILE_NAME ) );
	xmlAddIntNode( root, "CFD_Gmsh_File_Flag", cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::GMSH_FILE_NAME ) );
	xmlAddIntNode( root, "CFD_Srf_File_Flag", cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::SRF_FILE_NAME ) );

	//==== Write Window Background Scale/Offset ====//
	drawWin->writeFile( root );

	//==== Load Components Into XML Tree ====//
	xmlNodePtr comp_list_node = xmlNewChild( root, NULL, (const xmlChar *)"Component_List", NULL );

	for ( int i = 0 ; i < (int)gVec.size() ; i++ )		// jrg add hierachy
	{
		xmlNodePtr comp_node = xmlNewChild( comp_list_node, NULL, (const xmlChar *)"Component", NULL );
		gVec[i]->write(comp_node);
	}

	xmlNodePtr label_list_node = xmlNewChild( root, NULL, (const xmlChar *)"Label_List", NULL );
	for ( int j = 0 ; j < (int)lVec.size() ; j++ )
	{
		xmlNodePtr comp_node = xmlNewChild( label_list_node, NULL, (const xmlChar *)"Label", NULL );
		lVec[j]->write(this, comp_node);
	}

	parmLinkMgrPtr->WriteLinks( root );

  //===== Save XML Tree and Free Doc =====//
  xmlSaveFormatFile((const char *)file_name, doc, 1);
  xmlFreeDoc( doc );
	
}

void Aircraft::newFile()
{

	//==== Order Geom List ====//
	vector< Geom* > gVec;
	for ( int i = 0 ; i < (int)topGeomVec.size() ; i++ )
	{	
		topGeomVec[i]->loadChildren( gVec );
	}

	//==== Delete All Geoms ====//
	cutGeomVec( gVec );

	nameStr = "Aircraft";
	fileName = "VspAircraft.vsp";

	//==== Delete All Labels ====//
	for (int j = labelVec.size()-1; j >= 0; j--)
	{
		removeLabel(labelVec[j]);
	}

	parmLinkMgrPtr->DelAllLinks();
	pHolderListMgrPtr->DelAllPHolders();
	if ( getScreenMgr() )
		getScreenMgr()->getParmPickerScreen()->update();

	updateExportFileNames();
	cfdMeshMgrPtr->ResetExportFileNames();
	feaMeshMgrPtr->ResetFeaExportFileNames();

}

int Aircraft::openFile( const char* file_name )
{

	newFile();

	//==== Clear Clip Board ====//
	for ( int i = 0 ; i < (int)clipBoard.size() ; i++ )
		delete clipBoard[i];

	clipBoard.clear();



//DumpUnfreed();

	//==== Reads File And Loads Into Clip Board ====//
	if ( readFile( file_name ) )
	{
		fileName = file_name;
		setTempDir( fileName, false );
		pasteClipBoard();
		updateExportFileNames();
//		cfdMeshMgrPtr->ResetExportFileNames();
		feaMeshMgrPtr->ResetFeaExportFileNames();

//		nameStr = clipBoardStr;			// Set Name for Aircraft
		return 1;
	}

	//==== Restore Geoms ====//
	pasteClipBoard();


	return 0;
}

int  Aircraft::insertFile( const char* file_name )
{
	int i;

	Geom * activeGeom = getActiveGeom();
	if ( readFile( file_name ) )
	{
		resetClipBoardPtrIDCollisions();

		//==== Add Blank Component ====//
		BlankGeom* blankGeom = new BlankGeom( this );

		if ( activeGeom )
		{
			activeGeom->addChild( blankGeom );
			blankGeom->setParent( activeGeom );
		}
		else
		{
			topGeomVec.push_back( blankGeom );
		}

		setActiveGeom(blankGeom);
		pasteClipBoard();

		blankGeom->setName( clipBoardStr );

		for ( i = 0 ; i < (int)geomVec.size() ; i++ )
		{
			if ( geomVec[i]->getParent() == blankGeom )
			{
				geomVec[i]->setPosAttachFlag(POS_ATTACH_MATRIX);
			}
		}
		return 1;
	}

	return 0;
}

int Aircraft::readFile(const char* file_name )
{
  int i, j;

  //===== Load Aircraft Name From File Name =====
  int done = FALSE;
  Stringc temp_name = file_name;

  //===== Remove Path Name =====
  while (!done)
  {
    int pos = temp_name.search_for_substring('/');

    if (pos >= 0)
      temp_name.delete_range(0,pos);
    else
      done = TRUE;
  }

  //===== Check For .ram suffix =====
  int pos = temp_name.search_for_substring(".ram");
  if (pos >= 0)
  {
    temp_name.delete_range(pos,temp_name.get_length()-1);

	//==== Stripped File Name for Clip Board ====//
	clipBoardStr = temp_name;

	//==== Read in Old File ====//
 	return readOldRamFile( file_name );
  }

  //==== Remove VSP Suffix ====//
  pos = temp_name.search_for_substring(".vsp");
  if (pos >= 0)
  {
    temp_name.delete_range(pos,temp_name.get_length()-1);
	clipBoardStr = temp_name;
  }

  //==== Read Xml File ====//
  xmlDocPtr doc;
  xmlNodePtr node;

  LIBXML_TEST_VERSION
  xmlKeepBlanksDefault(0);

  //==== Build an XML tree from a the file ====//
  doc = xmlParseFile(file_name);
  if (doc == NULL) return 0;

  xmlNodePtr root = xmlDocGetRootElement(doc);
  if (root == NULL)
  {
    fprintf(stderr,"empty document\n");
	  xmlFreeDoc(doc);
	  return 0;
  }

  if ( xmlStrcmp( root->name, (const xmlChar *)"Vsp_Geometry" ) && 
	   xmlStrcmp( root->name, (const xmlChar *)"Ram_Geometry" ) )
  {
    fprintf(stderr, "document of the wrong type, Vsp/Ram Geometry not found\n");
	  xmlFreeDoc(doc);
	  return 0;
  }

  //==== Find Version Number ====//
  version = xmlFindInt( root, "Version", 0 );

  //==== Find Name ====//
  char oldName[256];
  sprintf(oldName, "%s", nameStr.get_char_star() );
  nameStr = xmlFindString( root, "Name", oldName );

  //==== Read Components ====//
  node = xmlGetNode( root, "Component_List", 0 );
  if ( node  )
  {
	vector < Geom* > addGeomVec;

    xmlNodePtr comp_list_node = node;
    int num_comps = xmlGetNumNames( comp_list_node, "Component" );

    for ( i = 0 ; i < num_comps ; i++ )
    {
      xmlNodePtr comp_node = xmlGetNode( comp_list_node, "Component", i );

      node = xmlGetNode( comp_node, "Type", 0 );

      if ( node )
      {
        char* typeStr = xmlExtractString(node);

        if ( strcmp( typeStr, "Pod" ) == 0 )
        {
			PodGeom* addGeom = new PodGeom( this );
			addGeom->read( comp_node );
			addGeomVec.push_back( addGeom );
		}
        if ( strcmp( typeStr, "External" ) == 0 )
        {
			Ext_geom* addGeom = new Ext_geom( this );
			addGeom->read( comp_node );
			addGeomVec.push_back( addGeom );
		}
        else if ( strcmp( typeStr, "Havoc" ) == 0 )
        {
			Havoc_geom* addGeom = new Havoc_geom( this );
			addGeom->read( comp_node );
			addGeomVec.push_back( addGeom );
		}
        else if ( strcmp( typeStr, "Fuselage" ) == 0 )
        {
			Fuse_geom* addGeom = new Fuse_geom( this );
			addGeom->read( comp_node );
			addGeomVec.push_back( addGeom );
		}
        else if ( strcmp( typeStr, "Fuselage2" ) == 0 )
        {
			FuselageGeom* addGeom = new FuselageGeom( this );
			addGeom->read( comp_node );
			addGeomVec.push_back( addGeom );
		}
        else if ( strcmp( typeStr, "Mwing" ) == 0 )
        {
			WingGeom* addGeom = new WingGeom( this );
			addGeom->read( comp_node );
//			addGeomVec.push_back( addGeom );

			Ms_wing_geom* msWingGeom = new Ms_wing_geom( this );
			msWingGeom->read( comp_node );
			msWingGeom->loadWingStrakeGeom( addGeom );
			addGeomVec.push_back( msWingGeom );

		}
        else if ( strcmp( typeStr, "Mswing" ) == 0 )
        {
			Ms_wing_geom* addGeom = new Ms_wing_geom( this );
			addGeom->read( comp_node );
			addGeomVec.push_back( addGeom );
		}
		else if ( strcmp( typeStr, "Hwb" ) == 0 )
        {
			Hwb_geom* addGeom = new Hwb_geom( this );
			addGeom->read( comp_node );
			addGeomVec.push_back( addGeom );
		}
	    else if ( strcmp( typeStr, "Blank" ) == 0 )
        {
			BlankGeom* addGeom = new BlankGeom( this );
			addGeom->read( comp_node );
			addGeomVec.push_back( addGeom );
		}
	    else if ( strcmp( typeStr, "Duct" ) == 0 )
        {
			DuctGeom* addGeom = new DuctGeom( this );
			addGeom->read( comp_node );
			addGeomVec.push_back( addGeom );
		}
	    else if ( strcmp( typeStr, "Prop" ) == 0 )
        {
			PropGeom* addGeom = new PropGeom( this );
			addGeom->read( comp_node );
			addGeomVec.push_back( addGeom );
		}
	    else if ( strcmp( typeStr, "Engine" ) == 0 )
        {
			EngineGeom* addGeom = new EngineGeom( this );
			addGeom->read( comp_node );
			addGeomVec.push_back( addGeom );
		}
	    else if ( strcmp( typeStr, "Mesh" ) == 0 )
        {
			MeshGeom* addGeom = new MeshGeom( this );
			addGeom->read( comp_node );
			addGeomVec.push_back( addGeom );
		}
		else if ( strcmp( typeStr, "Cabin_Layout" ) == 0 )
		{
			CabinLayoutGeom* addGeom = new CabinLayoutGeom( this );
			addGeom->read( comp_node );
			addGeomVec.push_back( addGeom );
		}
		else if ( strcmp( typeStr, "User" ) == 0 )
		{
			userGeomPtr->read( comp_node );
		}
		else if ( strcmp( typeStr, "XSecGeom" ) == 0 )
        {
			XSecGeom* addGeom = new XSecGeom( this );
			addGeom->read( comp_node );
			addGeomVec.push_back( addGeom );
		}

      }
    }

    bool collision = false;
    for (  i = 0 ; i < (int)addGeomVec.size() - 1 ; i++ )
	{
		for ( j = i + 1 ; j < (int)addGeomVec.size() ; j++ )
		{
			if( addGeomVec[i]->getPtrID() == addGeomVec[j]->getPtrID() )
			{
				collision = true;
				addGeomVec[j]->resetPtrID();
			}
		}
	}
    if( collision == true )
    {
		printf("\n\n!!!!!!!\n");
		printf("Duplicate PtrID's detected in file.\n");
		printf("  New PtrID's have been created, but parent/child relationships,\n");
		printf("  parameter links, design file ID's, and similar features of your\n");
		printf("  model may be broken.\n");
		printf("  Check your model carefully.\n");
		printf("!!!!!!!\n\n\n");
    }

	//==== Set Up Parent/Child Links ====//
	for (  i = 0 ; i < (int)addGeomVec.size() ; i++ )
	{
		//==== Parent??? ====//
		int pid = addGeomVec[i]->getParentPtrID();
		for ( j = 0 ; j < (int)addGeomVec.size() ; j++ )
		{
			if ( pid == addGeomVec[j]->getPtrID() )
			{
				addGeomVec[i]->setParent( addGeomVec[j] );
			}
		}
		//==== Children??? ====//
		vector< int > cidVec = addGeomVec[i]->getChildPtrIDVec();
		for ( int c = 0 ; c < (int)cidVec.size() ; c++ )
		{
			int cid = cidVec[c];
			for ( j = 0 ; j < (int)addGeomVec.size() ; j++ )
			{
				if ( cid == addGeomVec[j]->getPtrID() )
				{
					addGeomVec[i]->addChild( addGeomVec[j] );
				}
			}
		}
	}

	//==== Read Parm Links ====//
	parmLinkMgrPtr->ReadLinks( root, addGeomVec );

	//==== Read Aero Reference Stuff ====//
	cgRelAcFlag = xmlFindInt( root, "CG_Rel_AC_Flag", cgRelAcFlag );

	cgLoc.set_x( xmlFindDouble( root, "CG_X", cgLoc.x() ) );
	cgLoc.set_y( xmlFindDouble( root, "CG_Y", cgLoc.y() ) );
	cgLoc.set_z( xmlFindDouble( root, "CG_Z", cgLoc.z() ) );

	double bl = xmlFindDouble( root, "CFD_Mesh_Base_Length", cfdMeshMgrPtr->GetGridDensityPtr()->GetBaseLen() );
	cfdMeshMgrPtr->GetGridDensityPtr()->SetBaseLen(bl);
	cfdMeshMgrPtr->GetGridDensityPtr()->SetMinLen( xmlFindDouble( root, "CFD_Mesh_Min_Length", cfdMeshMgrPtr->GetGridDensityPtr()->GetMinLen() ) );
	cfdMeshMgrPtr->GetGridDensityPtr()->SetMaxGap( xmlFindDouble( root, "CFD_Mesh_Max_Gap", cfdMeshMgrPtr->GetGridDensityPtr()->GetMaxGap() ) );
	cfdMeshMgrPtr->GetGridDensityPtr()->SetNCircSeg( xmlFindDouble( root, "CFD_Mesh_Num_Circle_Segments", cfdMeshMgrPtr->GetGridDensityPtr()->GetNCircSeg() ) );
	cfdMeshMgrPtr->GetGridDensityPtr()->SetGrowRatio( xmlFindDouble( root, "CFD_Mesh_Growth_Ratio", cfdMeshMgrPtr->GetGridDensityPtr()->GetGrowRatio() ) );
	cfdMeshMgrPtr->GetGridDensityPtr()->SetFarMaxLen( xmlFindDouble( root, "CFD_Mesh_Far_Max_Length", cfdMeshMgrPtr->GetGridDensityPtr()->GetFarMaxLen() ) );
	cfdMeshMgrPtr->GetGridDensityPtr()->SetFarMaxGap( xmlFindDouble( root, "CFD_Mesh_Far_Max_Gap", cfdMeshMgrPtr->GetGridDensityPtr()->GetFarMaxGap() ) );
	cfdMeshMgrPtr->GetGridDensityPtr()->SetFarNCircSeg( xmlFindDouble( root, "CFD_Mesh_Far_Num_Circle_Segments", cfdMeshMgrPtr->GetGridDensityPtr()->GetFarNCircSeg() ) );
	bool rl = xmlFindInt( root, "CFD_Mesh_Rigorous_Limiting", cfdMeshMgrPtr->GetGridDensityPtr()->GetRigorLimit() ) != 0;
	cfdMeshMgrPtr->GetGridDensityPtr()->SetRigorLimit( rl );
	cfdMeshMgrPtr->SetFarXScale( xmlFindDouble( root, "CFD_Far_Field_Scale_X", cfdMeshMgrPtr->GetFarXScale() ) );
	cfdMeshMgrPtr->SetFarYScale( xmlFindDouble( root, "CFD_Far_Field_Scale_Y", cfdMeshMgrPtr->GetFarYScale() ) );
	cfdMeshMgrPtr->SetFarZScale( xmlFindDouble( root, "CFD_Far_Field_Scale_Z", cfdMeshMgrPtr->GetFarZScale() ) );
	bool hf = xmlFindInt( root, "CFD_Half_Mesh_Flag", cfdMeshMgrPtr->GetHalfMeshFlag() ) != 0;
	cfdMeshMgrPtr->SetHalfMeshFlag( hf );
	bool ff = xmlFindInt( root, "CFD_Far_Mesh_Flag", cfdMeshMgrPtr->GetFarMeshFlag() ) != 0;
	cfdMeshMgrPtr->SetFarMeshFlag( ff );
	bool sf = xmlFindInt( root, "CFD_Far_Abs_Size_Flag", cfdMeshMgrPtr->GetFarAbsSizeFlag() ) != 0;
	cfdMeshMgrPtr->SetFarAbsSizeFlag( sf );
	bool lf = xmlFindInt( root, "CFD_Far_Man_Loc_Flag", cfdMeshMgrPtr->GetFarManLocFlag() ) != 0;
	cfdMeshMgrPtr->SetFarManLocFlag( lf );

	bool cf = xmlFindInt( root, "CFD_Far_Comp_Flag", cfdMeshMgrPtr->GetFarCompFlag() ) != 0;
	cfdMeshMgrPtr->SetFarCompFlag( cf );
	cfdMeshMgrPtr->SetFarGeomID( xmlFindInt( root, "CFD_Far_Geom_PtrID", cfdMeshMgrPtr->GetFarGeomID() ) );


	cfdMeshMgrPtr->SetWakeAngle( xmlFindDouble( root, "CFD_Wake_Angle", cfdMeshMgrPtr->GetWakeAngle() ) );
	cfdMeshMgrPtr->SetWakeScale( xmlFindDouble( root, "CFD_Wake_Scale", cfdMeshMgrPtr->GetWakeScale() ) );

	bool def_flag = cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::STL_FILE_NAME );
	cfdMeshMgrPtr->SetExportFileFlag( !!xmlFindInt( root, "CFD_Stl_File_Flag",def_flag ), CfdMeshMgr::STL_FILE_NAME );

	def_flag = cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::POLY_FILE_NAME );
	cfdMeshMgrPtr->SetExportFileFlag( !!xmlFindInt( root, "CFD_Poly_File_Flag",def_flag ), CfdMeshMgr::POLY_FILE_NAME );
	def_flag = cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::TRI_FILE_NAME );
	cfdMeshMgrPtr->SetExportFileFlag( !!xmlFindInt( root, "CFD_Tri_File_Flag",def_flag ), CfdMeshMgr::TRI_FILE_NAME );
	def_flag = cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::OBJ_FILE_NAME );
	cfdMeshMgrPtr->SetExportFileFlag( !!xmlFindInt( root, "CFD_Obj_File_Flag",def_flag ), CfdMeshMgr::OBJ_FILE_NAME );
	def_flag = cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::DAT_FILE_NAME );
	cfdMeshMgrPtr->SetExportFileFlag( !!xmlFindInt( root, "CFD_Dat_File_Flag",def_flag ), CfdMeshMgr::DAT_FILE_NAME );
	def_flag = cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::KEY_FILE_NAME );
	cfdMeshMgrPtr->SetExportFileFlag( !!xmlFindInt( root, "CFD_Key_File_Flag",def_flag ), CfdMeshMgr::KEY_FILE_NAME );
	def_flag = cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::GMSH_FILE_NAME );
	cfdMeshMgrPtr->SetExportFileFlag( !!xmlFindInt( root, "CFD_Gmsh_File_Flag",def_flag ), CfdMeshMgr::GMSH_FILE_NAME );
	def_flag = cfdMeshMgrPtr->GetExportFileFlag( CfdMeshMgr::SRF_FILE_NAME );
	cfdMeshMgrPtr->SetExportFileFlag( !!xmlFindInt( root, "CFD_Srf_File_Flag",def_flag ), CfdMeshMgr::SRF_FILE_NAME );

	//==== Read Window Background Scale/Offset ====//
	if ( drawWin )
		drawWin->readFile( root );

	copyToClipBoard( addGeomVec );

	for ( i = 0 ; i < (int)addGeomVec.size() ; i++ )
	{
		parmLinkMgrPtr->SwapGeom( addGeomVec[i], clipBoard[i] );
	}

	//==== Delete Geom After They Are Copied to the ClipBoard ====//
	for ( int i = 0 ; i < (int)addGeomVec.size() ; i++ )
	{
		delete addGeomVec[i];
	}
  }

  //==== Read Labels ====//
  node = xmlGetNode( root, "Label_List", 0 );
  if ( node  )
  {
	xmlNodePtr label_list_node = node;
	int num_labels = xmlGetNumNames( label_list_node, "Label" );

	for ( i = 0 ; i < num_labels ; i++ )
	{
	  xmlNodePtr label_node = xmlGetNode( label_list_node, "Label", i );

	  node = xmlGetNode( label_node, "Type", 0 );

	  if ( node )
	  {
		char* typeStr = xmlExtractString(node);

		if ( strcmp( typeStr, "TextLabel" ) == 0 )
		{
			TextLabel* tLabel = new TextLabel();
			tLabel->read(this, label_node );
			labelVec.push_back( tLabel );
		}
		if ( strcmp( typeStr, "RulerLabel" ) == 0 )
		{
			RulerLabel* rLabel = new RulerLabel( screenMgr );
			rLabel->read(this, label_node );
			labelVec.push_back( rLabel );
		}
	  }
	}
  }

  //===== Free Doc =====//	
  xmlFreeDoc( doc );

  return 1;

}

int Aircraft::readOldRamFile(const char* file_name )
{
  char buff[255];
  Stringc line;

  //===== Open file =====
  FILE* file_id = fopen(file_name, "r");
  if (file_id == (FILE *)NULL) return 0;


  //===== Check FIle Type =====
  fgets(buff, 80, file_id);  line = buff;
  if (line.search_for_substring("GEOMETRY FILE") < 0)
  {
	return 0;
  }
	
  //===== Check File Version =====
  oldFileVersion = 7;  //current version
  if (line.search_for_substring("GEOMETRY FILE 1.02") >= 0)
      oldFileVersion = 2;
  else if (line.search_for_substring("GEOMETRY FILE 1.03") >= 0)
      oldFileVersion = 3;
  else if (line.search_for_substring("GEOMETRY FILE 1.04") >= 0)
      oldFileVersion = 4;
  else if (line.search_for_substring("GEOMETRY FILE 1.05") >= 0)
      oldFileVersion = 5;
  else if (line.search_for_substring("GEOMETRY FILE 1.06") >= 0 )
      oldFileVersion = 6;
  else if(line.search_for_substring("GEOMETRY FILE 1.07") < 0)
      oldFileVersion = 0;   //version not found

printf("Old Version = %d \n", oldFileVersion );

	int num_geoms;
	int geom_type;
	fscanf(file_id, "%d", &num_geoms);         fgets(buff, 80, file_id);

	if(oldFileVersion >= 7) 
	{
		fgets(buff, 80, file_id);
    }

	vector < Geom* > addGeomVec;
	for (int i = 0 ; i < num_geoms ; i++)
    {
		fgets(buff, 80, file_id);
		fgets(buff, 80, file_id);
		fgets(buff, 80, file_id);

		fscanf(file_id, "%d", &geom_type);   fgets(buff, 80, file_id);
printf("Geom Type = %d  %s\n", geom_type, buff );
        if ( geom_type == 0 )				// Simple Wing
        {
			WingGeom* addGeom = new WingGeom( this );
			addGeom->readSimple( file_id );
			addGeomVec.push_back( addGeom );
		}
        if ( geom_type == 4 )				// Strake Wing
        {
			WingGeom* addGeom = new WingGeom( this );
			addGeom->read( file_id );
			addGeomVec.push_back( addGeom );
		}
        else if ( geom_type == 3 )				// External
        {
			Ext_geom* addGeom = new Ext_geom( this );
			addGeom->read( file_id );
			addGeomVec.push_back( addGeom );
		}
        else if ( geom_type == 1 )				// Fuse
        {
			Fuse_geom* addGeom = new Fuse_geom( this );
			addGeom->read( file_id );
			addGeomVec.push_back( addGeom );
		}
        else if ( geom_type == 6 )				// Havoc
        {
			Havoc_geom* addGeom = new Havoc_geom( this );
			addGeom->read( file_id );
			addGeomVec.push_back( addGeom );
		}
        else if ( geom_type == 7 )				// Mswing
        {
			Ms_wing_geom* addGeom = new Ms_wing_geom( this );
			addGeom->read( file_id );
			addGeomVec.push_back( addGeom );
		}
        else if ( geom_type == 2 )				// Engine
        {
			EngineGeom* addGeom = new EngineGeom( this );
			addGeom->read( file_id );
			addGeomVec.push_back( addGeom );
		}
	}

	fclose( file_id );

	copyToClipBoard( addGeomVec );
	return 1;

}

	
void Aircraft::delGeom( Geom* geomPtr )
{
	int i;

	//==== Check If Children Are Attached - If So Update Offsets ====//
	vector<Geom*> cVec = geomPtr->getChildren();

	for ( i = 0 ; i < (int)cVec.size() ; i++ )
	{
		vec3d off(0,0,0);
		
		if ( cVec[i]->getPosAttachFlag() == POS_ATTACH_FIXED )
		{
			off = geomPtr->getAttachFixedPos();
		}
		else if ( cVec[i]->getPosAttachFlag() == POS_ATTACH_UV )
		{
			off = geomPtr->getAttachUVPos(cVec[i]->uAttach(),  
										  cVec[i]->vAttach());
			
			cVec[i]->setPosAttachFlag( POS_ATTACH_FIXED );
		}
		cVec[i]->xLoc.set( cVec[i]->xLoc() + off.x() );
		cVec[i]->yLoc.set( cVec[i]->yLoc() + off.y() );
		cVec[i]->zLoc.set( cVec[i]->zLoc() + off.z() );

//		if ( cVec[i]->getRotAttachFlag() )
//		{
//			cVec[i]->xRot.set( cVec[i]->xRot() + geomPtr->xRot() );
//			cVec[i]->yRot.set( cVec[i]->yRot() + geomPtr->yRot() );
//			cVec[i]->zRot.set( cVec[i]->zRot() + geomPtr->zRot());
//		}

	}

	//==== Redirect Parent to 1 Level Up ====//
	for ( i = 0 ; i < (int)cVec.size() ; i++ )
	{
		cVec[i]->setParent( geomPtr->getParent() );
		if ( geomPtr->getParent() )
			geomPtr->getParent()->addChild( cVec[i] );
		else
			topGeomVec.push_back( cVec[i] );
	}

	//==== Delete From Parent Child List
	if ( geomPtr->getParent() )
	{
		geomPtr->getParent()->removeChild( geomPtr );
	}

	//==== Update Geom List ====//
	vector<Geom*> nVec;
	for ( i = 0 ; i < (int)topGeomVec.size() ; i++ )
	{
		if ( topGeomVec[i] != geomPtr )
			nVec.push_back( topGeomVec[i] );
	}

	topGeomVec = nVec;

	resetAttachedLabels(geomPtr);

	//==== Remove All Parms and Parm Links That Ref Geom ====//
	parmMgrPtr->RemoveAllReferences( geomPtr );

	//==== Delete GeomPtr ====//
	if ( geomPtr != vorGeom && geomPtr != userGeomPtr )		// Only One VorGeom - Dont Del
		delete geomPtr;


	//==== Order Geom List ====//
	geomVec.clear();
	for ( i = 0 ; i < (int)topGeomVec.size() ; i++ )
	{	
		topGeomVec[i]->loadChildren( geomVec );
	}
	if (drawWin) drawWin->redraw(); 

	setActiveGeom(NULL);

	//==== Rebuild Parmeter Lists ====//
	parmMgrPtr->RebuildAll();

}

void Aircraft::cutGeomVec( vector< Geom* >& gVec )
{
	int i;

	copyToClipBoard( gVec );

	//==== Delete Geometry ====//
	for ( i = 0 ; i < (int)gVec.size() ; i++ )
	{
		delGeom( gVec[i] );
	}
}
	

void Aircraft::copyGeomVec( vector< Geom* >& gVec )
{
	copyToClipBoard( gVec );
}

void Aircraft::resetClipBoardPtrIDCollisions()
{
	int i, j;

	for ( i = 0 ; i < (int)clipBoard.size() ; i++ )
	{
		for ( j = 0 ; j < (int)geomVec.size() ; j++ )
		{
			if ( clipBoard[i]->getPtrID() == geomVec[j]->getPtrID() )
				clipBoard[i]->resetPtrID();  // Collision detected
		}
	}
}

void Aircraft::pasteClipBoard( )
{
	int i;

	//==== Create a copy of the Clip Board ====//
	vector< Geom* > gVec = clipBoard;

	Geom * activeGeom = getActiveGeom();
	//==== Paste w/ Active Geom as Parent ====//
	for ( i = 0 ; i < (int)clipBoard.size() ; i++ )
	{
		if ( clipBoard[i]->getParent() == 0 && activeGeom )
		{
			activeGeom->addChild( clipBoard[i] );
			clipBoard[i]->setParent( activeGeom );
		}
		if ( clipBoard[i]->getParent() == 0 )
		{
			topGeomVec.push_back( clipBoard[i] );
		}
	}

	//==== Order Geom List ====//
	geomVec.clear();
	for ( i = 0 ; i < (int)topGeomVec.size() ; i++ )
	{	
		topGeomVec[i]->loadChildren( geomVec );
	}

	if ( clipBoard.size() )
		setActiveGeom(clipBoard[0]); ///!@# REVISIT, should support multiple selection

	clipBoard.clear();
	copyToClipBoard( gVec );

	//==== Make Sure Attachments Upto Date ====//
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		geomVec[i]->updateAttach(0);
	}

	//==== Recenter Geometry ====//
	update_bbox();
	if (drawWin) {
		drawWin->centerAllViews();
		drawWin->redraw(); 
	}

	parmMgrPtr->RebuildAll();

	//==== Print Hier ====//
/*	for ( i = 0 ; i < geomVec.size() ; i++ )
	{
		printf( "Geom: %d \n", geomVec[i] );
		printf( "  Parent = %d \n", geomVec[i]->getParent() );
		vector<Geom*> cv = geomVec[i]->getChildren();
		for ( int j = 0 ; j < cv.size() ; j++ )
			printf( "   Child %d \n", cv[j] );
	}
*/

}


void Aircraft::copyToClipBoard( vector< Geom* > & gVec )
{
	int i;

	//==== Clear Clip Board ====//
	for ( i = 0 ; i < (int)clipBoard.size() ; i++ )
	{
		//==== Delete GeomPtr ====//
		delete clipBoard[i];
	}
	clipBoard.clear();

	//==== Create New Geometry ====//
	for ( i = 0 ; i < (int)gVec.size() ; i++ )
	{
		clipBoard.push_back(copyGeom(gVec[i]));
	}
//jrg - debug 
	if ( gVec.size() != clipBoard.size() )
		printf( "Error Aircraft::copyToClipBoard \n" );

	//==== Recreate Par/Chil That Are Left ====//
	for ( i = 0 ; i < (int)gVec.size() ; i++ )
	{
		Geom* par = gVec[i]->getParent();	
		for ( int j = 0 ; j < (int)gVec.size() ; j++ )
		{
			if ( par == gVec[j] )
			{
				clipBoard[i]->setParent( clipBoard[j] );
				clipBoard[j]->addChild( clipBoard[i] );
			}
		}
	}


//jrg leave attach flags??
/*
	//==== Check If There are Parents for Attach Flags ====//
	for ( i = 0 ; i < clipBoard.size() ; i++ )
	{
		if ( clipBoard[i]->getParent() == 0 )
		{
			clipBoard[i]->setPosAttachFlag( POS_ATTACH_NONE );
			clipBoard[i]->setRotAttachFlag( 0 );
		}
	}
*/
}

void Aircraft::triggerDraw()
{
	if (drawWin) drawWin->redraw(); 
}

void Aircraft::clearBBoxColors()
{
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		geomVec[i]->setRedFlag(0);
		geomVec[i]->setYellowFlag(0);
	}
}

void Aircraft::modifyGeom( Geom* geomPtr )
{
	if (!screenMgr) return;
	if (VspPreferences::Instance()->windowMgrStyle == 1)
		screenMgr->getScreen(geomPtr)->position(screenMgr->getGeomScreen()->x() + screenMgr->getGeomScreen()->w() + 8, screenMgr->getGeomScreen()->y());
	else if (VspPreferences::Instance()->windowMgrStyle == 2)
		screenMgr->getScreen(geomPtr)->position(screenMgr->getGeomScreen()->x(), screenMgr->getGeomScreen()->y() + screenMgr->getGeomScreen()->h() + 35);
	screenMgr->getScreen(geomPtr)->show(geomPtr);
}		



int Aircraft::getNumOfType( int type )
{
	int num = 0;

	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )		// jrg add hierachy
	{
		if ( geomVec[i]->getType() == type )
			num++;
	}
	return num;

}

void Aircraft::draw2D()
{
	//==== Draw 2D Stuff ====//
	vec2d cursor = drawWin->getCursor();
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		geomVec[i]->draw2D( cursor ); 
	}
	feaMeshMgrPtr->CursorPos( cursor );
}

void Aircraft::drawHighlight()
{
	//==== Draw Label Highlight Stuff ====//
	LabelScreen * labelScreen = screenMgr->getLabelScreen();
	vec2d cursor = drawWin->getCursor();
	if (labelScreen->getHighlightFlag())
	{
		// find nearest
		highlightVertex.reset();
		vec2d highlight;
		double delta = 0, min_delta = 0.005;
		vector< VertexID > verts;
		for (int g = 0 ; g < (int)geomVec.size() ; g++ )
		{
			if (geomVec[g]->getNoShowFlag()) continue;
			geomVec[g]->getVertexVec(&verts);
			for (int v = 0; v < (int)verts.size(); v++)
			{
				vec2d pos2d = verts[v].pos2d();
				delta = dist_squared(cursor, pos2d);
				if (delta < min_delta)
				{
					highlight = pos2d;
					min_delta = delta;
					highlightVertex = verts[v];
				}
			}
		}

		//draw
		if (highlightVertex.isSet())
		{
			glColor4f(1,0,0,0.7f);
			glPointSize(8);
			glPushMatrix();
			{
				glLoadIdentity();
				glBegin(GL_POINTS);
				glVertex2d(highlight.x(), highlight.y());
				glEnd();
			}
			glPopMatrix();
		}
	}
}


void Aircraft::draw()
{
	int i;

//printf("Check GL Error\n");
//GLenum glerr = glGetError();
//if ( glerr != GL_NO_ERROR )
//{
//	printf("OpenGL Err = %d %s\n", glerr, gluErrorString( glerr ) );
//}

	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		  // HACK check for undefined type and if so, skip it
		  if(geomVec[i]->getType() < 9999) 		
		  {
			geomVec[i]->draw();
			geomVec[i]->storeModelMatrix();
		  }
	}

	drawCG();									// Draw Center of Gravity
	drawAC();									// Draw Aero Center

	//==== VorView Geom ====//
	vorGeom->draw();
	vorGeom->storeModelMatrix();

	//==== Structures ====//
	structureMgrPtr->Draw();

	cfdMeshMgrPtr->Draw();

	feaMeshMgrPtr->Draw();

    glCullFace( GL_BACK );						// Cull Back Faces For Trans
	glEnable( GL_CULL_FACE );
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		geomVec[i]->drawAlpha();
	}
	vorGeom->drawAlpha();
	glDisable( GL_CULL_FACE );

	glDisable(GL_DEPTH_TEST);	
	glPushMatrix();
	glLoadIdentity();
	for ( i = 0 ; i < (int)labelVec.size() ; i++ )
	{
		int active = count(activeLabelVec.begin(), activeLabelVec.end(), labelVec[i]);
		labelVec[i]->setViewScale(drawWin->getScale()*10);
		labelVec[i]->setCursor(drawWin->getCursor());
		labelVec[i]->draw(active);
	}
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);	
	
	
//	screenMgr->getLabelScreen()->draw();
	
}


//***** Draw AC Marker*****//
void Aircraft::drawAC()
{
	if ( !drawACFlag )
		return;

	Geom* ref = getRefGeom();

	if (!ref)
		return;

	vec3d pos = ref->getAeroCenter();
	drawMarker( pos, 0.01*bnd_box.diag_dist(), 0, 0, 255 );

}


//***** Draw CG Marker*****//
void Aircraft::drawCG()
{
	if ( !drawCGFlag )
		return;

	drawMarker( cgLoc, 0.012*bnd_box.diag_dist(), 0, 0, 0 );
}

void Aircraft::drawMarker( vec3d& pos, double size, int r, int g, int b )
{
	GLboolean smoothFlag = 0;

	glGetBooleanv( GL_LINE_SMOOTH, &smoothFlag );
	if ( smoothFlag )
		glDisable( GL_LINE_SMOOTH );

	static double zero[3]  = { 0.0,    0.0,   0.0};
	static double vxy_1[3] = { 1.0,    0.0,   0.0};
	static double vxy_2[3] = { 0.707,  0.707, 0.0};
	static double vxy_3[3] = { 0.0,    1.0,   0.0};
	static double vxy_4[3] = {-0.707,  0.707, 0.0};
	static double vxy_5[3] = {-1.0,    0.0,   0.0};
	static double vxy_6[3] = {-0.707, -0.707, 0.0};
	static double vxy_7[3] = {0.0,    -1.0,   0.0};
	static double vxy_8[3] = { 0.707, -0.707, 0.0};
	static double vxz_1[3] = { 1.0,    0.0,   0.0};
	static double vxz_2[3] = { 0.707,  0.0,   0.707};
	static double vxz_3[3] = { 0.0,    0.0,   1.0};
	static double vxz_4[3] = {-0.707,  0.0,   0.707};
	static double vxz_5[3] = {-1.0,    0.0,   0.0};
	static double vxz_6[3] = {-0.707,  0.0,  -0.707};
	static double vxz_7[3] = {0.0,     0.0,  -1.0};
	static double vxz_8[3] = { 0.707, 0.0,   -0.707};
	static double vyz_1[3] = {0.0,     1.0,    0.0};
	static double vyz_2[3] = {0.0,     0.707,  0.707};
	static double vyz_3[3] = {0.0,     0.0,    1.0};
	static double vyz_4[3] = {0.0,    -0.707,  0.707};
	static double vyz_5[3] = {0.0,    -1.0,    0.0};
	static double vyz_6[3] = {0.0,    -0.707, -0.707};
	static double vyz_7[3] = {0.0,    0.0,    -1.0};
	static double vyz_8[3] = {0.0,     0.707, -0.707};

	glPushMatrix();

	glTranslatef( (float)pos.x(), (float)pos.y(), (float)pos.z());
	glScalef( (float)size, (float)size, (float)size );

	glLineWidth(2);

	glColor3ub(r,g,b);

	  glBegin( GL_LINE_LOOP );
		glVertex3dv(vxy_1);  glVertex3dv(vxy_2);  glVertex3dv(vxy_3);  glVertex3dv(vxy_4);  
		glVertex3dv(vxy_5);  glVertex3dv(vxy_6);  glVertex3dv(vxy_7);  glVertex3dv(vxy_8); 
	  glEnd();
	  glBegin( GL_LINE_LOOP );
		glVertex3dv(vxz_1);  glVertex3dv(vxz_2);  glVertex3dv(vxz_3);  glVertex3dv(vxz_4);  
		glVertex3dv(vxz_5);  glVertex3dv(vxz_6);  glVertex3dv(vxz_7);  glVertex3dv(vxz_8); 
	  glEnd();
	  glBegin( GL_LINE_LOOP );
		glVertex3dv(vyz_1);  glVertex3dv(vyz_2);  glVertex3dv(vyz_3);  glVertex3dv(vyz_4);  
		glVertex3dv(vyz_5);  glVertex3dv(vyz_6);  glVertex3dv(vyz_7);  glVertex3dv(vyz_8);  
	  glEnd();
	  glBegin( GL_POLYGON );
		glVertex3dv(zero);   glVertex3dv(vxy_1);   glVertex3dv(vxy_2);  glVertex3dv(vxy_3); 
	  glEnd();
	  glBegin( GL_POLYGON );
		glVertex3dv(zero);   glVertex3dv(vxy_5);   glVertex3dv(vxy_6);  glVertex3dv(vxy_7); 
	  glEnd();
	  glBegin( GL_POLYGON );
		glVertex3dv(zero);   glVertex3dv(vxz_1);   glVertex3dv(vxz_2);  glVertex3dv(vxz_3); 
	  glEnd();
	  glBegin( GL_POLYGON );
		glVertex3dv(zero);   glVertex3dv(vxz_5);   glVertex3dv(vxz_6);  glVertex3dv(vxz_7); 
	  glEnd();
	  glBegin( GL_POLYGON );
		glVertex3dv(zero);   glVertex3dv(vyz_1);   glVertex3dv(vyz_2);  glVertex3dv(vyz_3); 
	  glEnd();
	  glBegin( GL_POLYGON );
		glVertex3dv(zero);   glVertex3dv(vyz_5);   glVertex3dv(vyz_6);  glVertex3dv(vyz_7); 
	  glEnd();
	  glBegin( GL_POLYGON );
		glVertex3dv(vxy_2);  glVertex3dv(vxz_2);  glVertex3dv(vyz_2);
	  glEnd();
	  glBegin( GL_POLYGON );
		glVertex3dv(vxz_2);  glVertex3dv(vyz_2);  glVertex3dv(vyz_3);
	  glEnd();
	  glBegin( GL_POLYGON );
		glVertex3dv(vxz_2);  glVertex3dv(vxy_2);  glVertex3dv(vxy_1);
	  glEnd();
	  glBegin( GL_POLYGON );
		glVertex3dv(vxy_2);  glVertex3dv(vyz_2);  glVertex3dv(vyz_1);
	  glEnd();
	  glBegin( GL_POLYGON );
		glVertex3dv(vxy_6);  glVertex3dv(vxz_6);  glVertex3dv(vyz_6);
	  glEnd();
	  glBegin( GL_POLYGON );
		glVertex3dv(vxz_6);  glVertex3dv(vxy_6);  glVertex3dv(vxy_5);
	  glEnd();
	  glBegin( GL_POLYGON );
		glVertex3dv(vxz_6);  glVertex3dv(vyz_6);  glVertex3dv(vyz_7);
	  glEnd();
	  glBegin( GL_POLYGON );
		glVertex3dv(vxy_6);  glVertex3dv(vyz_6);  glVertex3dv(vxy_7);
	  glEnd();
	  glPopMatrix();

	if ( smoothFlag )
		glEnable( GL_LINE_SMOOTH );

	//==== Draw Lines ====//
	double off = size*50.0;
		
	glLineStipple( 2, 0x00FF );
	glEnable(GL_LINE_STIPPLE);

	glLineWidth(2.0);

	glBegin( GL_LINES );

	glVertex3d( pos.x() + off, pos.y(),   pos.z() );
	glVertex3d( pos.x() - off, pos.y(),   pos.z() );

	glVertex3d( pos.x(),       pos.y() + off, pos.z() );
	glVertex3d( pos.x(),       pos.y() - off, pos.z() );

	glVertex3d( pos.x(),       pos.y(),   pos.z()+off );
	glVertex3d( pos.x(),       pos.y(),   pos.z()-off );

	glEnd();

	glDisable(GL_LINE_STIPPLE);
}


//==== Update Bounding Box ====//
void Aircraft::update_bbox()
{
	bbox new_box;

	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )	
	{
		if ( geomVec[i]->valid_bnd_box() )
		{
			new_box.update( geomVec[i]->get_bnd_box() );
		}
	}
	if ( geomVec.size() == 0 )						// Vorview Geometry
		new_box.update( vorGeom->get_bnd_box() );

	bnd_box = new_box;
}

void Aircraft::fastDrawFlagOn()
{
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		geomVec[i]->fastDrawFlagOn();
	}
}

void Aircraft::fastDrawFlagOff()
{
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		geomVec[i]->fastDrawFlagOff();
	}
}

//===== Check for mesh geom =====//
bool Aircraft::check_for_stl_mesh()
{
	int num = 0;
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			num++;
		}
	}

	if ( num )
		return true;
	return false;
}

//===== Write STL File - Untrimmed for now.... =====//
void Aircraft::write_stl_file(const char* file_name)
{
	int i;

	//==== Check If Mesh Geom Exist - If Not Create ====//
	bool noMeshGeom = true;
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			noMeshGeom = false;	
		}
	}

	if ( noMeshGeom )
		addMeshGeom();


	//==== Open file ====//
	FILE* file_id = fopen(file_name, "w");

	fprintf(file_id, "solid\n");

	int num = 0;
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE)
		{
			geomVec[i]->write_stl_file(file_id);
			num++;
		}
	}

	fprintf(file_id, "endsolid\n");

	fclose(file_id);

	if ( num == 0 )
		fl_alert("STL File not written - no components");


}



//===== Write X3D Files  =====//
void Aircraft::write_x3d_file(const char* file_name)
{
	xmlDocPtr doc = xmlNewDoc((const xmlChar *)"1.0");

	xmlNodePtr root = xmlNewNode(NULL,(const xmlChar *)"X3D");
	xmlDocSetRootElement(doc, root);

	xmlNodePtr scene_node = xmlNewChild( root, NULL, (const xmlChar *)"Scene", NULL );
	

	writeX3DViewpoints(scene_node);	
	

	//==== All Geometry ====//
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		MeshGeom* newGeom = new MeshGeom( this );
		newGeom->setMeshType( MeshGeom::INTERSECTION_MESH );

		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() )
		{
			if( geomVec[i]->getNumSurf() > 0 )
			{
				xmlNodePtr shape_node = xmlNewChild( scene_node, NULL, (const xmlChar *) "Shape", NULL );

				xmlNodePtr app_node = xmlNewChild( shape_node, NULL, (const xmlChar *) "Appearance", NULL );

				int matid = geomVec[i]->getMaterialID();
				writeX3DMaterial( app_node, matid );

				geomVec[i]->writeX3D( shape_node );
			}
		}
	}


	//===== Save XML Tree and Free Doc =====//
	xmlSaveFormatFile((const char *)file_name, doc, 1);
	xmlFreeDoc( doc );
}

void Aircraft::writeX3DViewpoints( xmlNodePtr node)
{
	//==== Update box and get key values ====//
	update_bbox();
	vec3d center = bnd_box.get_center();
	double len = bnd_box.diag_dist();
	double fov = .4;
	double dist = len/(2 * tan(fov/2));
	
	// Set the names and vectors to the different viewpoints //
	string x3d_views[] = {"iso","front", "top", "right"};  // To add more views, add name to "x3d_views" and vector to viewpoint and rotation about that vector to "view_degree" //
	double view_degree[4][4] = { {-1, -1, 1, -PI/4 } , { -1,0,0, -PI/2}, {0,0,1,0}, {0,-1,0,0} };
	vec3d k = vec3d(0,0,1);
    
	// Write Viewpoint for each vector //
	for(int i = 0; i < 4 ; i++)  // Change i to match view_degree length if a new view is added //
	{		
		vec3d view_axis = vec3d(view_degree[i][0], view_degree[i][1], view_degree[i][2]);
		view_axis.normalize();
		
		vec3d rot_axis = cross(k, view_axis);
		double angle = asin(rot_axis.mag());
		rot_axis.normalize();

		// if rotating view again combine rotations using quaternions //
		if (view_degree[i][3] != 0)
		{
			quat rot1 = quat(rot_axis, angle);
			quat rot2 = quat(view_axis, view_degree[i][3]);
			quat combined_rot = hamilton(rot2, rot1);
			combined_rot.quat2axisangle(rot_axis, angle);
		}

		vec3d position = center + (view_axis * dist);

		double orient[] = { rot_axis.x(), rot_axis.y(), rot_axis.z(), angle };
		double cent[] = { center.x(), center.y(), center.z() };
		double posit[] = { position.x(), position.y(), position.z() };

		// Convert vectors to strings //
		Stringc orients, cents, posits, name, sfov;
		double4vec2str( orient , orients);
		doublevec2str( cent, cents );
		doublevec2str( posit, posits);
		name = x3d_views[i].c_str();
		sprintf(sfov, "%f", fov);
		orients.concatenate("\0");
		cents.concatenate("\0");
		posits.concatenate("\0");

		// write first viewpoint twice so viewpoint buttons will work correctly //
		if (name == x3d_views[0].c_str())
		{
			xmlNodePtr first_view_node = xmlNewChild( node, NULL, (const xmlChar *)"Viewpoint", (const xmlChar *)" ");
			writeViewpointsProps(first_view_node, orients, cents, posits, sfov, "first");
		}

		// write each viewpoint node's properties //
		xmlNodePtr viewpoint_node = xmlNewChild( node, NULL, (const xmlChar *)"Viewpoint", (const xmlChar *)" ");
		writeViewpointsProps(viewpoint_node, orients, cents, posits, sfov, name);

	}
}

void Aircraft::writeViewpointsProps( xmlNodePtr node, Stringc orients, Stringc cents, Stringc posits, const char* sfov, Stringc name)
{
	xmlSetProp( node, (const xmlChar *)"id", (const xmlChar *) ((const char *) name));
	xmlSetProp( node, (const xmlChar *)"description", (const xmlChar *) ((const char *) name));
	xmlSetProp( node, (const xmlChar *)"orientation", (const xmlChar *) ((const char *) orients));
	xmlSetProp( node, (const xmlChar *)"centerOfRotation", (const xmlChar *) ((const char *) cents));
	xmlSetProp( node, (const xmlChar *)"position", (const xmlChar *) ((const char *) posits));
	xmlSetProp( node, (const xmlChar *)"fieldOfView", (const xmlChar *) ((const char *) sfov));

}

void Aircraft::writeX3DMaterial( xmlNodePtr node, int matid )
{
	Stringc diffs, emisss, specs;
	Material* mat = matMgrPtr->getMaterial( matid );

	xmlNodePtr mat_node = xmlNewChild( node, NULL, (const xmlChar *) "Material", (const xmlChar *)" " );

	floatvec2str( mat->diff, diffs );
	diffs.concatenate("\0");
	xmlSetProp( mat_node, (const xmlChar *)"diffuseColor", (const xmlChar *) ((const char *) diffs) );

	floatvec2str( mat->emiss, emisss );
	emisss.concatenate("\0");
	xmlSetProp( mat_node, (const xmlChar *)"emissiveColor", (const xmlChar *) ((const char *) emisss) );

	floatvec2str( mat->spec, specs );
	specs.concatenate("\0");
	xmlSetProp( mat_node, (const xmlChar *)"specularColor", (const xmlChar *) ((const char *) specs) );

	char alphac[255];
	sprintf( alphac, "%lf", 1.0f - mat->diff[3] );
	xmlSetProp( mat_node, (const xmlChar *)"transparency", (const xmlChar *) alphac );

	char shine[255];
	sprintf( shine, "%lf", mat->shine );
	xmlSetProp( mat_node, (const xmlChar *)"shininess", (const xmlChar *) shine );

	float ambf = 0.0f;
	for( int i = 0; i < 3; i++ )
	{
		ambf += mat->amb[i] / mat->diff[i];
	}
	ambf = ambf / 3.0f;

	char amb[255];
	sprintf( amb, "%lf", ambf );
	xmlSetProp( mat_node, (const xmlChar *)"ambientIntensity", (const xmlChar *) amb );
}

void Aircraft::floatvec2str( float* vec, Stringc &str )
{
	char numc[255];

	sprintf( numc, "%lf %lf %lf", vec[0], vec[1], vec[2] );
	str.concatenate( numc );
}

void Aircraft::doublevec2str( double* vec, Stringc &str )
{
	char numc[255];

	sprintf( numc, "%lf %lf %lf", vec[0], vec[1], vec[2] );
	str.concatenate( numc );
}

void Aircraft::double4vec2str( double* vec, Stringc &str )
{
	char numc[255];

	sprintf( numc, "%lf %lf %lf %lf", vec[0], vec[1], vec[2], vec[3] );
	str.concatenate( numc );
}

//===== Write Nascart Files  =====//
void Aircraft::write_nascart_files(const char* file_name)
{
	int i;

	//==== Check If Mesh Geom Exist - If Not Create ====//
	bool noMeshGeom = true;
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			noMeshGeom = false;	
		}
	}

	if ( noMeshGeom )
		addMeshGeom();

	//==== Open file ====//
	FILE* file_id = fopen(file_name, "w");

	if ( !file_id )
		return;

	//==== Count Number of Points & Tris ====//
	int num_pnts = 0;
	int num_tris = 0;
	int num_parts = 0;
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			MeshGeom* mg = (MeshGeom*)geomVec[i];				// Cast
			mg->buildNascartMesh(num_parts);
			num_parts += mg->getNumNascartParts();
			num_pnts += mg->getNumNascartPnts();
			num_tris += mg->getNumNascartTris();
		}
	}

	fprintf(file_id, "%d %d\n", num_pnts, num_tris );

	//==== Dump Points ====//
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			MeshGeom* mg = (MeshGeom*)geomVec[i];				// Cast
			mg->writeNascartPnts( file_id );
		}
	}

	int offset = 0;
	//==== Dump Tris ====//
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			MeshGeom* mg = (MeshGeom*)geomVec[i];				// Cast
			offset = mg->writeNascartTris( file_id, offset );
		}
	}

	fclose(file_id);

	//==== Extract Path ====//
	Stringc temp_name = file_name;

	int last = -1;
	for ( i = 0 ; i < temp_name.get_length() ; i++ )
	{
		if ( temp_name[i] == '.' )
			last = i;
	}

	if ( last == -1 )
		temp_name = Stringc("bodyin.key");
	else
	{
		temp_name.delete_range(last+1,temp_name.get_length()-1);
		temp_name.concatenate( "key" );
	}

	//==== Open file ====//
	file_id = fopen(temp_name, "w");

	if ( !file_id )
		return;


	fprintf( file_id, "Color	Name			BCType\n");

	int part_count = 1;
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			MeshGeom* mg = (MeshGeom*)geomVec[i];				// Cast
			part_count = mg->writeNascartParts(file_id, part_count);
		}
	}

	fclose(file_id);


	if ( num_parts == 0 )
		fl_alert("NASCART File not valid - run comp_geom to generate mesh");


}


//===== Read NASCART File - Add Mesh Comp =====//
Geom* Aircraft::read_nascart_file(const char* file_name)
{

	MeshGeom* geom = new MeshGeom( this );
	geom->setMeshType( MeshGeom::MODEL_MESH );
	
	int validFlag = geom->read_nascart( file_name );

	if ( validFlag )
	{
		addGeom( geom );
		clearBBoxColors();
		geom->setRedFlag(1);
		if (screenMgr) screenMgr->getMeshScreen()->show( geom );
	}
	else
	{
		delete geom;
		geom = 0;
	}

	return geom;
}

//===== Write Cart3D (.tri) Files  =====//
void Aircraft::write_cart3d_files(const char* file_name)
{
	int i;

	//==== Check If Mesh Geom Exist - If Not Create ====//
	bool noMeshGeom = true;
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			noMeshGeom = false;	
		}
	}

	if ( noMeshGeom )
		addMeshGeom();

	//==== Open file ====//
	FILE* file_id = fopen(file_name, "w");

	if ( !file_id )
		return;

	//==== Count Number of Points & Tris ====//
	int num_pnts = 0;
	int num_tris = 0;
	int num_parts = 0;
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			MeshGeom* mg = (MeshGeom*)geomVec[i];				// Cast
			mg->buildNascartMesh(num_parts);
			num_parts += mg->getNumNascartParts();
			num_pnts += mg->getNumNascartPnts();
			num_tris += mg->getNumNascartTris();
		}
	}

	fprintf(file_id, "%d %d\n", num_pnts, num_tris );

	//==== Dump Points ====//
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			MeshGeom* mg = (MeshGeom*)geomVec[i];				// Cast
			mg->writeCart3DPnts( file_id );
		}
	}

	int offset = 0;
	//==== Dump Tris ====//
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			MeshGeom* mg = (MeshGeom*)geomVec[i];				// Cast
			offset = mg->writeCart3DTris( file_id, offset );
		}
	}

	int part_count = 1;
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			MeshGeom* mg = (MeshGeom*)geomVec[i];				// Cast
			part_count = mg->writeCart3DParts(file_id, part_count);
		}
	}

	fclose(file_id);



	//////==== Write Pnt Count and Tri Count ====//
 ////   fprintf( fp, "%d %d\n", numPnts, tri_cnt);

	//////==== Write Pnts ====//
	////for ( int i = 0 ; i < (int)allPntVec.size() ; i++ )
	////{
	////	if ( pntShift[i] >= 0 )
	////	   fprintf( fp, "%16.10f %16.10f %16.10f\n", allPntVec[i]->x(), allPntVec[i]->y(), allPntVec[i]->z() );
	////}

	//////==== Write Tris ====//
	////for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	////{
	////	vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
	////	vector< vec3d >& sPntVec = m_SurfVec[i]->GetMesh()->GetSimpPntVec();
	////	for ( int t = 0 ; t <  (int)sTriVec.size() ; t++ )
	////	{
	////		int i0 = FindPntIndex( sPntVec[sTriVec[t].ind0], allPntVec, indMap );
	////		int i1 = FindPntIndex( sPntVec[sTriVec[t].ind1], allPntVec, indMap );
	////		int i2 = FindPntIndex( sPntVec[sTriVec[t].ind2], allPntVec, indMap );
	////		int ind1 = pntShift[i0] + 1;
	////		int ind2 = pntShift[i1] + 1;
	////		int ind3 = pntShift[i2] + 1;
	////		fprintf( fp, "%d %d %d \n", ind1, ind2, ind3 );
	////	}
	////}

 ////   //==== Write Component ID ====//
 ////   for ( int i = 0 ; i < (int)m_SurfVec.size() ; i++ )
	////{
	////	vector < SimpTri >& sTriVec = m_SurfVec[i]->GetMesh()->GetSimpTriVec();
	////	for ( int t = 0 ; t <  (int)sTriVec.size() ; t++ )
	////	{
	////		fprintf( fp, "%d \n", m_SurfVec[i]->GetCompID()+1);
	////	}
	////}

	////fclose(fp);

}

//===== Write Cart3D (.tri) Files  =====//
void Aircraft::write_gmsh_files(const char* file_name)
{
	int i;

	//==== Check If Mesh Geom Exist - If Not Create ====//
	bool noMeshGeom = true;
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			noMeshGeom = false;	
		}
	}

	if ( noMeshGeom )
		addMeshGeom();

	//==== Open file ====//
	FILE* file_id = fopen(file_name, "w");

	if ( !file_id )
		return;

	//==== Count Number of Points & Tris ====//
	int num_pnts = 0;
	int num_tris = 0;
	int num_parts = 0;
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			MeshGeom* mg = (MeshGeom*)geomVec[i];				// Cast
			mg->buildNascartMesh(num_parts);
			num_parts += mg->getNumNascartParts();
			num_pnts += mg->getNumNascartPnts();
			num_tris += mg->getNumNascartTris();
		}
	}

	fprintf(file_id, "$MeshFormat\n" );
	fprintf(file_id, "2.2 0 %d\n", (int)sizeof(double) );
	fprintf(file_id, "$EndMeshFormat\n" );


	//==== Dump Nodes ====//
	fprintf(file_id, "$Nodes\n" );
	fprintf(file_id, "%d\n", num_pnts );
	int node_offset = 0;
	vector< int > node_offset_vec;
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		node_offset_vec.push_back( node_offset );
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			MeshGeom* mg = (MeshGeom*)geomVec[i];				// Cast
			node_offset = mg->writeGMshNodes( file_id, node_offset );
		}
	}
	fprintf(file_id, "$EndNodes\n" );

	fprintf(file_id, "$Elements\n" );
	fprintf(file_id, "%d\n", num_tris );
	int tri_offset = 0;
	//==== Dump Tris ====//
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )		
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() && 
			  geomVec[i]->getType() == MESH_GEOM_TYPE )
		{
			MeshGeom* mg = (MeshGeom*)geomVec[i];				// Cast
			tri_offset = mg->writeGMshTris( file_id, node_offset_vec[i], tri_offset );
		}
	}
	fprintf(file_id, "$EndElements\n" );

	fclose(file_id);
}


//===== Write STL File - Untrimmed for now.... =====//
void Aircraft::write_rhino_file(const char* file_name)
{
	int i;

	ON::Begin();
	int version = 3;

	// errors printed to stdout
	ON_TextLog error_log;

	FILE* fp = ON::OpenFile( file_name, "wb" );

	ON_BinaryFile archive( ON::write3dm, fp );

	ON_3dmProperties props;
	props.m_RevisionHistory.NewRevision();
	ON_3dmSettings settings;
	ON_Layer layer;
	ON_3dmObjectAttributes attributes;

	// layer table will have one layer
	layer.SetLayerIndex(0);
	layer.SetLayerName(L"Default");
  
	// object attributes
	attributes.m_layer_index = 0;

    archive.Write3dmStartSection( version, "Archive created by ON_WriteOneObjectArchive "__DATE__" "__TIME__ );

    archive.Write3dmProperties( props );
 
    archive.Write3dmSettings( settings );

    archive.BeginWrite3dmBitmapTable();
    archive.EndWrite3dmBitmapTable();

    archive.BeginWrite3dmMaterialTable();
    archive.EndWrite3dmMaterialTable();

    archive.BeginWrite3dmLayerTable();
    archive.Write3dmLayer(layer);
    archive.EndWrite3dmLayerTable();

    archive.BeginWrite3dmGroupTable();
	archive.EndWrite3dmGroupTable();

	archive.BeginWrite3dmFontTable();
	archive.EndWrite3dmFontTable();

    if ( version >= 3 )
    {
      archive.BeginWrite3dmDimStyleTable();
      archive.EndWrite3dmDimStyleTable();
    }

    archive.BeginWrite3dmLightTable();
	archive.EndWrite3dmLightTable();

    if ( version >= 3 )
    {
      archive.BeginWrite3dmInstanceDefinitionTable();
      archive.EndWrite3dmInstanceDefinitionTable();
    }

    archive.BeginWrite3dmObjectTable();

	//==== Load Obj Array into Model ====//
	for ( i = 0 ; i <  (int)geomVec.size() ; i++ )		
	{
		if ( geomVec[i]->getOutputFlag() )
			geomVec[i]->write_rhino_file( &archive, &attributes  );	
	}

	archive.EndWrite3dmObjectTable();

	archive.Write3dmEndMark();

	ON::CloseFile( fp );

	ON::End();

}

void Aircraft::write_bezier_file(const char* file_name)
{
	//==== Open file ====//
	FILE* file_id = fopen(file_name, "w");

	if ( !file_id )
		return;

	int num_comps = 0;
	for ( int i = 0 ; i <  (int)geomVec.size() ; i++ )		
	{
		if ( geomVec[i]->getOutputFlag()  )
		{
			num_comps += geomVec[i]->get_num_bezier_comps();
		}
	}

	fprintf( file_id, "%d  Num_Components\n", num_comps );

	//==== Write Bezier Geometry ====//
	for ( int i = 0 ; i <  (int)geomVec.size() ; i++ )		
	{
		if ( geomVec[i]->getOutputFlag() )
		{
			geomVec[i]->write_bezier_file( i, file_id );
		}
	}
	fclose( file_id );
}

//===== Read STL File - Add Mesh Comp =====//
Geom* Aircraft::read_stl_file(const char* file_name)
{

	MeshGeom* geom = new MeshGeom( this );
	geom->setMeshType( MeshGeom::MODEL_MESH );
	
	int validFlag = geom->read_stl( file_name );

	if ( validFlag )
	{
		addGeom( geom );
		clearBBoxColors();
		geom->setRedFlag(1);
		if (screenMgr) screenMgr->getMeshScreen()->show( geom );
	}
	else
	{
		delete geom;
		geom = 0;
	}

	return geom;
}

//===== Read XSec File - Add Mesh Comp =====//
Geom* Aircraft::read_xsec_file(const char* file_name)
{
	MeshGeom* geom = new MeshGeom( this );
	geom->setMeshType( MeshGeom::MODEL_MESH );
	
	int validFlag = geom->read_xsec( file_name );

	if ( validFlag )
	{
		addGeom( geom );
		clearBBoxColors();
		geom->setRedFlag(1);
		if (screenMgr) screenMgr->getMeshScreen()->show( geom );
	}
	else
	{
		delete geom;
		geom = 0;
	}

	return geom;
}

//===== Read XSec File - Add Mesh Comp =====//
Geom* Aircraft::read_xsecgeom_file(const char* file_name)
{
	XSecGeom* geom = new XSecGeom( this );
	
	int validFlag = geom->read_xsec( file_name );

	if ( validFlag )
	{
		addGeom( geom );
		clearBBoxColors();
		geom->setRedFlag(1);
//		if (screenMgr) screenMgr->getMeshScreen()->show( geom );
	}
	else
	{
		delete geom;
		geom = 0;
	}

	return geom;
}

//===== Read Cross Section File - Add XSec Comp =====//
Geom* Aircraft::read_vorxsec_file(const char* file_name)
{

//	VorGeom* geom = new VorGeom( this );
	
	int validFlag = vorGeom->read_xsec( file_name );

	if ( validFlag )
	{
		//==== Check if vorGeom is already added ====//
		int addFlag = 1;
		for ( int i = 0 ; i < (int)geomVec.size() ; i++ )		
		{
			if ( vorGeom == geomVec[i] )
			{
				addFlag = 0;
				break;
			}
		}
		setActiveGeom(NULL);
		if ( addFlag )
			addGeom( vorGeom );

		clearBBoxColors();
		vorGeom->setRedFlag(1);

//		if (screenMgr) screenMgr->getHrmScreen()->show( vorGeom );
	}
//	else
//	{
//		delete geom;
//		geom = 0;
//	}

	return vorGeom;
}

Geom* Aircraft::addMeshGeom()
{
	MeshGeom* newGeom = new MeshGeom( this );
	newGeom->setMeshType( MeshGeom::INTERSECTION_MESH );

	//==== All Geometry ====//	
	for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() )
		{
			//==== Create TMeshs ====//
			vector< TMesh* > tMeshVec = geomVec[i]->createTMeshVec();

			//==== Load Into New Mesh Geom ====//
			for ( int j = 0 ; j < (int)tMeshVec.size() ; j++ )
			{
				tMeshVec[j]->color      = geomVec[i]->getColor();
				tMeshVec[j]->materialID = geomVec[i]->getMaterialID();
				tMeshVec[j]->ptr_id     = geomVec[i]->getPtrID();
				if ( tMeshVec[j]->reflected_flag )
					tMeshVec[j]->ptr_id = -tMeshVec[j]->ptr_id;

				tMeshVec[j]->massPrior  = geomVec[i]->getMassPrior();
				tMeshVec[j]->density    = geomVec[i]->density.get();

				//==== Check for Alternate Output Name ====//
				tMeshVec[j]->name_str   = geomVec[i]->getName();

				newGeom->tMeshVec.push_back( tMeshVec[j] );
			}
		}
	}

	if ( newGeom->tMeshVec.size() )
	{
		//==== No Show All Geom Except New Mesh ====//
		for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
		{
			geomVec[i]->setNoShowFlag(1);
			geomVec[i]->setRedFlag(0);
		}
		if (screenMgr) screenMgr->hideGeomScreens();

		setActiveGeom(NULL);

		addGeom( newGeom );

		newGeom->setRedFlag(1);

		modifyGeom( newGeom );
		setActiveGeom( newGeom );

		if (screenMgr) screenMgr->getGeomScreen()->update();
		if ( screenMgr ) screenMgr->getGeomScreen()->selectGeomBrowser(newGeom);
		if ( screenMgr ) screenMgr->update(GEOM_SCREEN);
	}

	return newGeom;

}

	
Geom* Aircraft::comp_geom(int sliceFlag, int meshFlag, int halfFlag )
{
	int i, j;

	MeshGeom* newGeom = new MeshGeom( this );
	newGeom->setMeshType( MeshGeom::INTERSECTION_MESH );

	//==== All Geometry ====//	
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() )
		{
			//==== Create TMeshs ====//
			vector< TMesh* > tMeshVec = geomVec[i]->createTMeshVec();

			//==== Load Into New Mesh Geom ====//
			for ( j = 0 ; j < (int)tMeshVec.size() ; j++ )
			{
				tMeshVec[j]->loadGeomAttributes( geomVec[i] );
				newGeom->tMeshVec.push_back( tMeshVec[j] );
			}
		}
	}

	if ( halfFlag )
	{
		newGeom->addHalfBox();
	}

	if ( newGeom->tMeshVec.size() )
	{
		//==== No Show All Geom Except New Mesh ====//
		for ( i = 0 ; i < (int)geomVec.size() ; i++ )
		{
			geomVec[i]->setNoShowFlag(1);
			geomVec[i]->setRedFlag(0);
		}
		if (screenMgr) screenMgr->hideGeomScreens();

		setActiveGeom(NULL);

		addGeom( newGeom );

		if ( sliceFlag )
			newGeom->sliceX(sliceFlag);
		else
			newGeom->intersectTrim(meshFlag, halfFlag);

		newGeom->setRedFlag(1);

		modifyGeom( newGeom );
		setActiveGeom( newGeom );

		if (screenMgr) screenMgr->getMeshScreen()->show( newGeom );
	}
	else
	{
		delete newGeom;
		newGeom = 0;
	}
	return newGeom;
}

Geom* Aircraft::massprop(int numSlice)
{
	int i, j;

	MeshGeom* newGeom = new MeshGeom( this );
	newGeom->setMeshType( MeshGeom::INTERSECTION_MESH );

	//==== All Geometry ====//	
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag() )
		{
			//==== Create TMeshs ====//
			vector< TMesh* > tMeshVec = geomVec[i]->createTMeshVec();

			//==== Load Into New Mesh Geom ====//
			for ( j = 0 ; j < (int)tMeshVec.size() ; j++ )
			{
				tMeshVec[j]->loadGeomAttributes( geomVec[i] );
				newGeom->tMeshVec.push_back( tMeshVec[j] );
			}

			//==== Check for Blank (Point Mass) Components ====//
			if ( geomVec[i]->getType() == BLANK_GEOM_TYPE )
			{
				BlankGeom* bgeom = (BlankGeom*)geomVec[i];
				if ( bgeom->getPointMassFlag() )
				{
					TetraMassProp* mp = new TetraMassProp();	// Deleted By newGeom
					bgeom->loadPointMass( mp );
					newGeom->addPointMass( mp );
				}
			}
		}
	}

	if ( newGeom->tMeshVec.size() )
	{
		//==== No Show All Geom Except New Mesh ====//
		for ( i = 0 ; i < (int)geomVec.size() ; i++ )
		{
			geomVec[i]->setNoShowFlag(1);
			geomVec[i]->setRedFlag(0);
		}
		if (screenMgr) screenMgr->hideGeomScreens();

		setActiveGeom(NULL);
		addGeom( newGeom );

		newGeom->massSliceX(numSlice);

		cgLoc = newGeom->centerOfGrav;
		total_mass = newGeom->totalMass;
		Ixx_Iyy_Ixx = vec3d(newGeom->totalIxx, newGeom->totalIyy, newGeom->totalIzz );
		Ixy_Ixz_Iyz = vec3d(newGeom->totalIxy, newGeom->totalIxz, newGeom->totalIyz );

		newGeom->setRedFlag(1);

		modifyGeom( newGeom );
		setActiveGeom( newGeom );

		if (screenMgr) screenMgr->getMeshScreen()->show( newGeom );
	}
	else
	{
		delete newGeom;
		newGeom = 0;
	}
	return newGeom;
}

Geom* Aircraft::slice(int sliceStyle, int numSlices, double sliceAngle, double coneSections, Stringc filename)
{
	int i, j;

	MeshGeom* newGeom = new MeshGeom( this );
	newGeom->setMeshType( MeshGeom::INTERSECTION_MESH );

	//==== All Geometry ====//	
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		if ( !geomVec[i]->getNoShowFlag() && geomVec[i]->getOutputFlag())
		{
			//==== Create TMeshs ====//
			vector< TMesh* > tMeshVec = geomVec[i]->createTMeshVec();

			//==== Load Into New Mesh Geom ====//
			for ( j = 0 ; j < (int)tMeshVec.size() ; j++ )
			{
				tMeshVec[j]->loadGeomAttributes( geomVec[i] );
				newGeom->tMeshVec.push_back( tMeshVec[j] );
			}
		}
	}

	if ( newGeom->tMeshVec.size() )
	{
		//==== No Show All Geom Except New Mesh ====//
		for ( i = 0 ; i < (int)geomVec.size() ; i++ )
		{
			geomVec[i]->setNoShowFlag(1);
			geomVec[i]->setRedFlag(0);
		}
		if (screenMgr) screenMgr->hideGeomScreens();

		setActiveGeom(NULL);
		addGeom( newGeom );

		newGeom->newSlice(sliceStyle, numSlices, sliceAngle, coneSections, filename);

		newGeom->setRedFlag(1);

		modifyGeom( newGeom );
		setActiveGeom( newGeom );

		if (screenMgr) screenMgr->getMeshScreen()->show( newGeom );
	}
	else
	{
		delete newGeom;
		newGeom = 0;
	}
	return newGeom;
}

//==== Write Cross Section File ====//
void Aircraft::write_xsec_file(const char* file_name)
{
	int i;

	int geom_cnt = 0;
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		if ( geomVec[i]->getOutputFlag() )
			geom_cnt += geomVec[i]->getNumXSecSurfs();
	}

	//==== Open file ====//
	FILE* dump_file = fopen(file_name, "w");

	fprintf(dump_file, " HERMITE INPUT FILE\n\n");
	fprintf(dump_file, " NUMBER OF COMPONENTS = %d\n",geom_cnt);

	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		if ( geomVec[i]->getOutputFlag() )
			geomVec[i]->dump_xsec_file(i, dump_file);
	}

	fclose(dump_file);

}




//===== Dump FELISA Xsec File =====//
void Aircraft::write_felisa_file(const char* file_name)
{
	int i;

	//==== Count Number Of Felisa Comps ====//
	int geom_cnt = 0;
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		if ( geomVec[i]->getOutputFlag() )
			geom_cnt += geomVec[i]->get_num_felisa_comps();
	}

	//==== Open file ====//
	FILE* dump_file = fopen(file_name, "w");

	//==== Account for Sym Plane And Far Field ====//
	geom_cnt += 6;

	fprintf(dump_file, " %d       Number_Surfaces\n",geom_cnt);
	fprintf(dump_file, "\n 0      Number_Exceptions\n");

	//==== Write Felisa Surfaces ====//
	geom_cnt = 0;
	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		if ( geomVec[i]->getOutputFlag() )
			geom_cnt += geomVec[i]->write_felisa_file(geom_cnt, dump_file);
	}

	//===== Far Field - Felisa Dump =====//
	if ( guess_far_field )
	{
		double max_length = bnd_box.get_largest_dim();
		far_x_min = -6.*max_length;
		far_x_max =  6.*max_length;
		far_y_min =  0.;
		far_y_max =  6.*max_length;
		far_z_min = -6.*max_length;
		far_z_max =  6.*max_length;
	}

	//==== Write Sym Plane and Far Field ====//
	fprintf(dump_file, "\n");
	fprintf(dump_file, "%d                Surface_Number\n", geom_cnt);  geom_cnt++;
	fprintf(dump_file, "Sym_Plane Name\n");
	fprintf(dump_file, "1         U_Render\n");
	fprintf(dump_file, "1         W_Render\n");
	fprintf(dump_file, "2         Num_xsecs\n");
	fprintf(dump_file, "2         Num_pnts\n");
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_min, far_y_min, far_z_min);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_min, far_y_min, far_z_max);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_max, far_y_min, far_z_min);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_max, far_y_min, far_z_max);

	fprintf(dump_file, "\n");
	fprintf(dump_file, "%d                Surface_Number\n", geom_cnt);  geom_cnt++;
	fprintf(dump_file, "Far_Field Name\n");
	fprintf(dump_file, "1         U_Render\n");
	fprintf(dump_file, "1         W_Render\n");
	fprintf(dump_file, "2         Num_xsecs\n");
	fprintf(dump_file, "2         Num_pnts\n");
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_max, far_y_max, far_z_min);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_max, far_y_max, far_z_max);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_min, far_y_max, far_z_min);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_min, far_y_max, far_z_max);

	fprintf(dump_file, "\n");
	fprintf(dump_file, "%d                Surface_Number\n", geom_cnt);  geom_cnt++;
	fprintf(dump_file, "Far_Field Name\n");
	fprintf(dump_file, "1         U_Render\n");
	fprintf(dump_file, "1         W_Render\n");
	fprintf(dump_file, "2         Num_xsecs\n");
	fprintf(dump_file, "2         Num_pnts\n");
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_min, far_y_min, far_z_max);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_min, far_y_max, far_z_max);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_max, far_y_min, far_z_max);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_max, far_y_max, far_z_max);

	fprintf(dump_file, "\n");
	fprintf(dump_file, "%d                Surface_Number\n", geom_cnt);  geom_cnt++;
	fprintf(dump_file, "Far_Field Name\n");
	fprintf(dump_file, "1         U_Render\n");
	fprintf(dump_file, "1         W_Render\n");
	fprintf(dump_file, "2         Num_xsecs\n");
	fprintf(dump_file, "2         Num_pnts\n");
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_min, far_y_min, far_z_min);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_min, far_y_max, far_z_min);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_max, far_y_min, far_z_min);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_max, far_y_max, far_z_min);

	fprintf(dump_file, "\n");
	fprintf(dump_file, "%d                Surface_Number\n", geom_cnt);  geom_cnt++;
	fprintf(dump_file, "Entrance_Plane    Name\n");
	fprintf(dump_file, "1         U_Render\n");
	fprintf(dump_file, "1         W_Render\n");
	fprintf(dump_file, "2         Num_xsecs\n");
	fprintf(dump_file, "2         Num_pnts\n");
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_min, far_y_min, far_z_min);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_min, far_y_max, far_z_min);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_min, far_y_min, far_z_max);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_min, far_y_max, far_z_max);

	fprintf(dump_file, "\n");
	fprintf(dump_file, "%d                Surface_Number\n", geom_cnt);  geom_cnt++;
	fprintf(dump_file, "Exit_Plane        Name\n");
	fprintf(dump_file, "1         U_Render\n");
	fprintf(dump_file, "1         W_Render\n");
	fprintf(dump_file, "2         Num_xsecs\n");
	fprintf(dump_file, "2         Num_pnts\n");
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_max, far_y_min, far_z_max);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_max, far_y_max, far_z_max);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_max, far_y_min, far_z_min);
	fprintf(dump_file, "%11.6f  %11.6f  %11.6f\n", far_x_max, far_y_max, far_z_min);

	fclose(dump_file);

}

//===== Write FELISA Background File =====//
void Aircraft::write_felisa_background_file(const char* file_name)
{
  int i;

  //==== Open file ====//
  FILE* file_ptr = fopen(file_name, "w");

  fprintf(file_ptr, "*  FELISA Background Mesh ---- Model: %s  \n",nameStr.get_char_star());

  //==== Find Total Number Of Background Elements ====//
  int np  = 8;
  int ne  = 6;
  int nps = 0;
  int nls = 0;
  int nts = 0;

	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
 		if ( geomVec[i]->getOutputFlag() )
		{
			nls   += geomVec[i]->get_num_felisa_line_sources();
			nts   += geomVec[i]->get_num_felisa_tri_sources();
		}
	}

  fprintf(file_ptr, "  %d   %d   %d   %d   %d\n",np,ne,nps,nls,nts);


  //==== Write Nodal Values ====//
  double xx, yy, zz;
  double dx, dy, dz;

  dx = ( far_x_max - far_x_min )/20.;
  dy = ( far_y_max - far_y_min )/20.;
  dz = ( far_z_max - far_z_min )/20.;

  double ds = MIN(MIN(dx,dy),dz);

  dx = ds;
  dy = ds;
  dz = ds;

  for ( i = 0 ; i < 8 ; i++ )
    {
      if ( i%2     ) xx = 100000.0;  else xx = -100000.0;
      if ( (i/2)%2 ) yy = 100000.0;  else yy = -100000.0;
      if ( i < 4   ) zz = 100000.0;  else zz = -100000.0;
      fprintf(file_ptr, "  %d   %f   %f   %f\n",i+1, xx, yy, zz);
      fprintf(file_ptr, "   1.00    .00    .00    %10.5f\n",dx);
      fprintf(file_ptr, "    .00   1.00    .00    %10.5f\n",dy);
      fprintf(file_ptr, "    .00    .00   1.00    %10.5f\n",dz);
    }
  fprintf(file_ptr, "     1     7     5     8     4 \n");
  fprintf(file_ptr, "     2     7     5     4     1 \n");
  fprintf(file_ptr, "     3     7     1     4     3 \n");
  fprintf(file_ptr, "     4     5     6     8     2 \n");
  fprintf(file_ptr, "     5     5     2     8     4 \n");
  fprintf(file_ptr, "     6     5     2     4     1 \n");

  fprintf(file_ptr, "  === Point Sources === \n");

  fprintf(file_ptr, "  === Line Sources === \n");
  for ( i = 0 ; i < (int)geomVec.size() ; i++ )
  {
	if ( geomVec[i]->getOutputFlag() )
		geomVec[i]->write_felisa_line_sources(file_ptr);
  }

  fprintf(file_ptr, "  === Triangle Sources === \n");
  for ( i = 0 ; i < (int)geomVec.size() ; i++ )
  {
	if ( geomVec[i]->getOutputFlag() )
		geomVec[i]->write_felisa_tri_sources(file_ptr);
  }


  fclose(file_ptr);

}

//===== Get Far Field =====//
void Aircraft::get_far_field(double& x_min, double& x_max, double& y_min,
                             double& y_max, double& z_min, double& z_max)
{
  x_min = far_x_min;
  x_max = far_x_max;
  y_min = far_y_min;
  y_max = far_y_max;
  z_min = far_z_min;
  z_max = far_z_max;
}

//===== Set Far Field =====//
void Aircraft::set_far_field(double x_min, double x_max, double y_min,
                             double y_max, double z_min, double z_max)
{
  far_x_min = x_min;
  far_x_max = x_max;
  far_y_min = y_min;
  far_y_max = y_max;
  far_z_min = z_min;
  far_z_max = z_max;
}


//===== Write FELISA Background File =====//
void Aircraft::write_fps3d_bco_file(const char* file_name)
{
	int i;
	int num_surfs = 0;
	int num_wings = 0;

  //===== Compute Reference Geometry =====
//  this->compute_ref_parms();

  //===== Compute CG Location =====
//  this->find_default_cg_loc();

  for ( i = 0 ; i < (int)geomVec.size() ; i++ )
  {
	if ( geomVec[i]->getOutputFlag() )
	{
		num_surfs += geomVec[i]->get_num_felisa_comps();
		num_surfs += geomVec[i]->get_num_felisa_wings();
	}
  }

  //==== Open file ====//
  FILE* file_ptr = fopen(file_name, "w");

  fprintf(file_ptr, "nsf    nwing  .... %s\n", nameStr.get_char_star() );
  fprintf(file_ptr, "%d        %d \n",num_surfs + 6, num_wings);

  fprintf(file_ptr, "Surface   B.C   Wing   Mach   V_exit  T_ratio  P_ratio   Description\n");

  int surf_cnt = 1;
  int wing_cnt = 1;

  for ( i = 0 ; i < (int)geomVec.size() ; i++ )
  {
      geomVec[i]->write_bco_info(file_ptr, surf_cnt, wing_cnt);
  }

  fprintf( file_ptr, "%d      999      0     0.0     0.0     0.0     0.0   Symmetry Plane\n", surf_cnt);
  surf_cnt++;
  fprintf( file_ptr, "%d     1000      0     0.0     0.0     0.0     0.0   Far Field Plane 1\n", surf_cnt);
  surf_cnt++;
  fprintf( file_ptr, "%d     1000      0     0.0     0.0     0.0     0.0   Far Field Plane 2\n", surf_cnt);
  surf_cnt++;
  fprintf( file_ptr, "%d     1000      0     0.0     0.0     0.0     0.0   Far Field Plane 3\n", surf_cnt);
  surf_cnt++;
  fprintf( file_ptr, "%d     1000      0     0.0     0.0     0.0     0.0    Entrance Plane 4\n", surf_cnt);
  surf_cnt++;
  fprintf( file_ptr, "%d     1000      0     0.0     0.0     0.0     0.0        Exit Plane 5\n", surf_cnt);
  surf_cnt++;

  fprintf(file_ptr, "\n");

  double refArea = 100.0;
  double refCbar = 10.0;
  Geom* refg = getRefGeom();
  if ( refg )
  {
	refArea = refg->getRefArea();
	refCbar = refg->getRefCbar();
  }

  fprintf(file_ptr, "Reference Area:    %f\n", refArea);
  fprintf(file_ptr, "Reference Chord:   %f\n", refCbar);
  fprintf(file_ptr, "Xcg:               %f\n", cgLoc.x() );
  fprintf(file_ptr, "Ycg:               %f\n", cgLoc.y());
  fprintf(file_ptr, "Zcg:               %f\n", cgLoc.z());
  fprintf(file_ptr, "Scale:             %f\n", 1.0);

  fclose(file_ptr);

/*
  //=== a total kludge on ref parameters...Andy made me do it! ===//
  ref_area = SqFeet(725.0);
  ref_cbar = Feet(15.13);
  x_cg = Feet(54.6);
  y_cg = Feet(0.0);
  z_cg = Feet(-2.0);

//jrg FIXWARN
  fprintf(file_ptr, "Reference Area:    %f\n", (float)((SqFeet)ref_area));
  fprintf(file_ptr, "Reference Chord:   %f\n", (float)((Feet)ref_cbar));
  fprintf(file_ptr, "Xcg:               %f\n", (float)((Feet)x_cg));
  fprintf(file_ptr, "Ycg:               %f\n", (float)((Feet)y_cg));
  fprintf(file_ptr, "Zcg:               %f\n", (float)((Feet)z_cg));
  fprintf(file_ptr, "Scale:             %f\n", 1.0);

  fclose(file_ptr);
*/


}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Added by jrg, June 2006
//==== Write POV Ray File ====//
void Aircraft::write_povray_file(const char* file_name)
{
	int i;

	//==== Set Base File Name ====//
	Stringc base_name = file_name;
 
	int pov_loc = base_name.search_for_substring(".pov");

	if ( pov_loc >= 0 ) 
		  base_name = base_name.get_range(0, pov_loc-1);

	//==== Open Inc File - Mesh ====//
	Stringc inc_file_name = base_name;
	inc_file_name.concatenate(".inc");

	FILE* dump_file = fopen(inc_file_name.get_char_star(), "w");

	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		if ( !geomVec[i]->getNoShowFlag() )
			geomVec[i]->write_pov_tris(dump_file);
	}

	//==== Add Some Decent Textures ====//
	fprintf( dump_file, "#declare lightgreymetal = texture {\n  pigment { color rgb < 0.9, 0.9, 0.9 > } \n");
	fprintf( dump_file, "    normal { granite, 0.01 }\n");
	fprintf( dump_file, "    finish { ambient 0.2  diffuse 1.0   specular 0.5  roughness 0.1  metallic reflection 0.05 }\n}\n");

	fprintf( dump_file, "#declare darkgreymetal = texture {\n  pigment { color rgb < 0.7, 0.7, 0.7 > } \n");
	fprintf( dump_file, "    normal { granite, 0.005 }\n");
	fprintf( dump_file, "    finish { ambient 0.2  diffuse 1.0   specular 0.8  roughness 0.15   }\n}\n");

 	fprintf( dump_file, "#declare bluegreymetal = texture {\n  pigment { color rgb < 0.4, 0.4, 0.45 > } \n");
	fprintf( dump_file, "    normal { granite, 0.02 }\n");
	fprintf( dump_file, "    finish { ambient 0.2  diffuse 1.0   specular 0.8  roughness 0.15 metallic  }\n}\n");

 	fprintf( dump_file, "#declare canopyglass = texture {\n  pigment { color rgb < 0.3, 0.3, 0.3, 0.5 > } \n");
	fprintf( dump_file, "    finish { ambient 0.1  diffuse 1.0   specular 1.0  roughness 0.01 reflection 0.4  }\n}\n");

	fclose(dump_file);

	//==== Open POV File  ====//
	Stringc pov_file_name = base_name;
	pov_file_name.concatenate(".pov");

	FILE* pov_file = fopen(pov_file_name.get_char_star(), "w");

	fprintf(pov_file, "#version 3.6;\n\n");
	fprintf(pov_file, "#include \"%s\"\n", inc_file_name.get_char_star() );

	fprintf(pov_file, "#include \"colors.inc\"\n");
	fprintf(pov_file, "#include \"shapes.inc\"\n");
	fprintf(pov_file, "#include \"textures.inc\"\n\n");

	fprintf(pov_file, "global_settings { assumed_gamma 1 }\n\n");

	bbox bb = getBndBox();
	vec3d center = bb.get_center();
	double diag  = bb.diag_dist();
	double mult  = 1.3;

	fprintf(pov_file, "camera { location < %6.1f, %6.1f, %6.1f >  look_at < %6.1f, %6.1f, %6.1f > }\n", 
		mult*bb.get_min(0), mult*bb.get_max(2), mult*bb.get_min(1), center.x(), center.z(), center.y() );

	fprintf(pov_file, "light_source { < %6.1f, %6.1f, %6.1f >  color White }\n", center.x(), center.z() + diag, center.y());

	for ( i = 0 ; i < (int)geomVec.size() ; i++ )
	{
		if ( !geomVec[i]->getNoShowFlag() )
		{
			Stringc name = geomVec[i]->getName();
			name.space_to_underscore();
			fprintf(pov_file, "mesh { %s texture {darkgreymetal} } \n", name.get_char_star() );
		}
	}

	fclose( pov_file );




}

void Aircraft::setActiveGeom( Geom* gptr )
{
	if (backupGeom != NULL)
		delete backupGeom;

	activeGeomVec.clear();
	if (gptr != NULL)
	{
		activeGeomVec.push_back(gptr);
		editGeom = gptr;
		backupGeom = copyGeom(editGeom);
	}
	else
	{
		editGeom = NULL;
		backupGeom = NULL;
	}
}

Geom* Aircraft::getActiveGeom()
{
	if (activeGeomVec.size() > 0) 
		return activeGeomVec[activeGeomVec.size()-1]; 
	else 
		return NULL; 
}

void Aircraft::setActiveGeomVec( vector< Geom* > gptr )	
{
	if (backupGeom != NULL)
		delete backupGeom;

	activeGeomVec = gptr;
	
	if (gptr.size() == 1)
	{
		editGeom = gptr[0];
		backupGeom = copyGeom(editGeom);
	}
	else
	{
		editGeom = NULL;
		backupGeom = NULL;
	}
}

vector< Geom* > Aircraft::getActiveGeomVec()				
{
	return activeGeomVec; 
}

void Aircraft::flagActiveGeom()
{
	clearBBoxColors();
	for (int i = 0; i < (int)activeGeomVec.size(); i++)
	{
		activeGeomVec[i]->setRedFlag(1);
	}
}

vector< Geom* > Aircraft::getGeomByName(Stringc name, int children)
{
	vector< Geom* > selVec;
	
	for (int i = 0; i < (int)geomVec.size(); i++)
	{
		if (geomVec[i]->getName().matchwild(name))
		{
			Geom * geom = geomVec[i];
			if (children)
			{
				vector<Geom *> cVec;
				geom->loadChildren(cVec);
				for (int m = 0; m < (int)cVec.size(); m++)
					selVec.push_back(cVec[m]);
			}
			else 
			{
				selVec.push_back(geomVec[i]);
			}

		}
	}
	return selVec;
}

vector< Geom* > Aircraft::getDisplayGeomVec()
{
	//==== Order Geom List ====//
	vector< Geom* > displayGeomVec;

	for ( int i = 0 ; i < (int)topGeomVec.size() ; i++ )
	{	
		topGeomVec[i]->loadDisplayChildren( displayGeomVec );
	}

	return displayGeomVec;
}

void Aircraft::addLabel( LabelGeom * label )
{
	labelVec.push_back(label);
}
void Aircraft::removeLabel(LabelGeom * label)
{
	int i;
	vector< LabelGeom* > tempVec;
	for (i = 0; i < (int)labelVec.size(); i++)
	{
		if (labelVec[i] != label)
			tempVec.push_back(labelVec[i]);
	}
	labelVec = tempVec;

	vector< LabelGeom* > tempAVec;
	for (i = 0; i < (int)activeLabelVec.size(); i++)
	{
		if (activeLabelVec[i] != label)
			tempAVec.push_back(activeLabelVec[i]);
	}
	activeLabelVec = tempAVec;

	screenMgr->getLabelScreen()->loadLabelBrowser();
	delete label;
}
void Aircraft::resetAttachedLabels(Geom * geom)
{
	vector< LabelGeom* > attachedLabelVec;
	for (int i = 0; i < (int)labelVec.size(); i++)
	{
		if (labelVec[i]->isAttached(geom))
		{
			labelVec[i]->reset();
		}
	}

}

void Aircraft::setActiveLabel( LabelGeom * label )
{
	activeLabelVec.clear();
	activeLabelVec.push_back(label);
}

LabelGeom * Aircraft::createLabel(int type)
{
	LabelGeom* labelPtr = NULL;
	switch (type)
	{
		case LabelGeom::TEXT_LABEL:
			labelPtr = new TextLabel();
			break;
		case LabelGeom::RULER_LABEL:
			labelPtr = new RulerLabel(screenMgr);
			break;
	}
	addLabel(labelPtr);
	setActiveLabel(labelPtr);
	return labelPtr;
}


LabelGeom * Aircraft::getActiveLabel()
{
	if (activeLabelVec.size() > 0) 
		return activeLabelVec[activeLabelVec.size()-1]; 
	else 
		return NULL; 
}

vector< LabelGeom* > Aircraft::getLabelByName(Stringc name)
{
	vector< LabelGeom* > selVec;
	for (int i = 0; i < (int)labelVec.size(); i++)
	{
		if (labelVec[i]->getName().matchwild(name))
		{
			LabelGeom * label = labelVec[i];
			selVec.push_back(labelVec[i]);
		}
	}
	return selVec;
}

void Aircraft::mouseClick(float mx, float my)
{
	LabelScreen * labelScreen = screenMgr->getLabelScreen();
	if (labelScreen->getTrackFlag())
	{
		vec2d cursor = drawWin->getCursor();
		labelScreen->selectVertex(highlightVertex, (float)cursor.x(), (float)cursor.y());
	}
	else
	{
		vec2d cursor = drawWin->getCursor();
		for ( int i = 0 ; i < (int)geomVec.size() ; i++ )
		{
			geomVec[i]->mouseClick( cursor ); 
		}
		feaMeshMgrPtr->MouseClick( cursor );
	}
}

int Aircraft::getGeomIndex(Geom * geom)
{
	for (int i = 0; i < (int)geomVec.size(); i++)
	{
		if (geomVec[i] == geom)
			return i;
	}
	return -1;
}

Geom* Aircraft::getClipboardGeom(int index)
{
	if (index >= 0 && index < (int)clipBoard.size())
		return clipBoard[index];
	else return NULL;
}

int Aircraft::getGeomIndex(int ptrid)
{
	for (int i = 0; i < (int)geomVec.size(); i++)
	{
		if (geomVec[i]->getPtrID() == ptrid)
			return i;
	}
	return -1;
}

Geom* Aircraft::getRefGeom()
{
	int i;

	//==== Make Sure RefGeom is Valid ====//
	for ( i = 0; i < (int)geomVec.size(); i++)
	{
		if ( geomVec[i]->getRefFlag() )
			return geomVec[i];
	}

	//==== Find Biggest Wing ====//
	Geom* ref_geom = 0;
	double bigarea = 0.0;
	for ( i = 0; i < (int)geomVec.size(); i++)
	{
		if ( geomVec[i]->getType() == MS_WING_GEOM_TYPE )
		{
			if ( geomVec[i]->getRefArea() > bigarea )
			{
				bigarea = geomVec[i]->getRefArea();
				ref_geom = geomVec[i];
			}
		}
	}

	//==== Just Take First One
	if ( !ref_geom && geomVec.size() )
	{
		ref_geom = geomVec[0];
	}

	if ( ref_geom )
		ref_geom->setRefFlag(1);

	return ref_geom;
}
	
void Aircraft::setRefGeomIndex( int id )
{
	for ( int i = 0; i < (int)geomVec.size(); i++)
		geomVec[i]->setRefFlag(0);
		

	if ( id >= 0 && id < (int)geomVec.size() )
		geomVec[id]->setRefFlag(1);
}

vec3d Aircraft::getSMLoc()				// Static Margin
{
	Geom* ref = getRefGeom();

	if (!ref)
		return cgLoc;

	vec3d ac    = ref->getAeroCenter();
	double cbar = ref->getRefCbar();

	vec3d smLoc = ac - cgLoc;
	if ( cbar )
		smLoc = smLoc * (1.0/cbar);

	return smLoc;	
}

void Aircraft::setSMLoc(const vec3d& l)
{
	Geom* ref = getRefGeom();

	if (!ref)
		return;

	vec3d smLoc = l;
	vec3d ac    = ref->getAeroCenter();
	double cbar = ref->getRefCbar();

	if ( cbar )
		smLoc = smLoc * cbar;

	cgLoc = ac - smLoc; 

}

void Aircraft::setDefaultCompGroupID( int id )
{
	VspPreferences::Instance();
	vector< DefaultCompFile > defCompFileVec = VspPreferences::Instance()->getDefaultCompFileVec();

	if ( id < 0 || id >= (int)defCompFileVec.size() )
		return;

	//==== Remove Old Comps ====//
	map< int, Geom* >::iterator iter;
	for ( iter = m_DefaultCompMap.begin() ; iter != m_DefaultCompMap.end() ; ++iter )
		delete iter->second;
	m_DefaultCompMap.clear();

	//==== Clear Clip Board ====//
	for ( int i = 0 ; i < (int)clipBoard.size() ; i++ )
		delete clipBoard[i];
	clipBoard.clear();

	Stringc fn;
	fn.concatenate( defCompFileVec[id].m_File.get_char_star() );
	readFile( fn.get_char_star() );

	if ( clipBoard.size() )
	{
		for ( int i = 0 ; i < (int)clipBoard.size() ; i++ )
		{
			int type = clipBoard[i]->getType();

			//==== Check If This Type Already Used ====//
			iter = m_DefaultCompMap.find( type );
			if ( iter == m_DefaultCompMap.end() )
			{
				Geom* gPtr = copyGeom( clipBoard[i] );
				m_DefaultCompMap[type] = gPtr;
			}
		}
	}
}

void Aircraft::setExortFileName( const char* fn, int type )
{
	if ( type == COMP_GEOM_TXT_TYPE )
		compGeomTxtFileName = Stringc(fn);
	else if ( type == COMP_GEOM_CSV_TYPE )
		compGeomCsvFileName = Stringc(fn);
	else if ( type == SLICE_TXT_TYPE )
		sliceFileName = Stringc(fn);
	else if ( type == MASS_PROP_TXT_TYPE )
		massPropFileName = Stringc(fn);
	else if ( type == DRAG_BUILD_TSV_TYPE )
		dragBuildTsvFileName = Stringc(fn);
}

Stringc Aircraft::getExportFileName( int type )
{
	if ( type == COMP_GEOM_TXT_TYPE )
		return compGeomTxtFileName;
	else if ( type == COMP_GEOM_CSV_TYPE )
		return compGeomCsvFileName;
	else if ( type == SLICE_TXT_TYPE )
		return sliceFileName;
	else if ( type == MASS_PROP_TXT_TYPE )
		return massPropFileName;
	else if ( type == DRAG_BUILD_TSV_TYPE )
		return dragBuildTsvFileName;

	return Stringc("default_export.txt");
}

void Aircraft::updateExportFileNames()
{
	int pos;

	compGeomTxtFileName = fileName;
	pos = compGeomTxtFileName.search_for_substring(".vsp");
	if (pos >= 0)
		compGeomTxtFileName.delete_range(pos, compGeomTxtFileName.get_length()-1);
	compGeomTxtFileName.concatenate("_CompGeom.txt");

	compGeomCsvFileName = fileName;
	pos = compGeomCsvFileName.search_for_substring(".vsp");
	if (pos >= 0)
		compGeomCsvFileName.delete_range(pos, compGeomCsvFileName.get_length()-1);
	compGeomCsvFileName.concatenate("_CompGeom.csv");

	sliceFileName = fileName;
	pos = sliceFileName.search_for_substring(".vsp");
	if (pos >= 0)
		sliceFileName.delete_range(pos, sliceFileName.get_length()-1);
	sliceFileName.concatenate("_Slice.txt");

 	massPropFileName = fileName;
	pos = massPropFileName.search_for_substring(".vsp");
	if (pos >= 0)
		massPropFileName.delete_range(pos, massPropFileName.get_length()-1);
	massPropFileName.concatenate("_MassProp.txt");

 	dragBuildTsvFileName = fileName;
	pos = dragBuildTsvFileName.search_for_substring(".tsv");
	if (pos >= 0)
		dragBuildTsvFileName.delete_range(pos, dragBuildTsvFileName.get_length()-1);
	dragBuildTsvFileName.concatenate("_DragBuild.tsv");

}

void Aircraft::setTempDir( const char* fn, bool user_set )
{
	Stringc dirString = fn;

	int dirLen = dirString.get_length();
	for (int i=0; i<dirLen; i++) {
		if ( dirString[i] == '\\' ) dirString[i] = '/';
	}

	if (  dirString.count_substrings( "/" ) >= 1 )
	{
		int slashLoc = dirLen-1;

		while ( slashLoc > 0 )
		{
			if ( dirString[slashLoc] == '/' )
				break;
			slashLoc--;
		}
		if ( slashLoc+1 <= dirLen-1 )
			dirString.delete_range( slashLoc+1, dirLen-1 );
	}

	if ( user_set )
	{
		tempDirName = dirString;
		tempDirUserOverride = true;
	}
	else if ( !tempDirUserOverride )
	{
		tempDirName = dirString;
	}
}

Stringc Aircraft::getTempDir()
{
	return tempDirName;
}




