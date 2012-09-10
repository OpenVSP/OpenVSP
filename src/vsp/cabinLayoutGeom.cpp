//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

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
#include <fstream>

#include <limits>
#include <math.h>
#include "cabinLayoutGeom.h"
#include "cabinLayoutScreen.h"
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

#undef ON
#include "opennurbs.h"
#include "opennurbs_extensions.h"

CabinLayoutGeom::CabinLayoutGeom(Aircraft* aptr) : Geom(aptr)
{
	type = CABIN_LAYOUT_GEOM_TYPE;
	type_str = Stringc("cabin layout");

	char name[255];
	sprintf( name, "Cabin_Layout_%d", geomCnt ); 
	geomCnt++;

	setName( Stringc(name) );

	geom_data_file_name = "";
	UpdateGeom = false;
	MirrorData = false;

	numPnts.set(9);
	numXsecs.set(6);

//	geom_surf.set_num_pnts( numPnts.iget() );
//	geom_surf.set_num_xsecs(  numXsecs.iget() );

	generate();
}

CabinLayoutGeom::~CabinLayoutGeom()
{
}

void CabinLayoutGeom::write(xmlNodePtr root)
{
  xmlAddStringNode( root, "Type", "Cabin_Layout");

  //==== Write General Parms ====//
  xmlNodePtr gen_node = xmlNewChild( root, NULL, (const xmlChar *)"General_Parms", NULL );
  write_general_parms( gen_node );

  //==== Write Fuse Parms ====//
  xmlNodePtr cabin_layout_node = xmlNewChild( root, NULL, (const xmlChar *)"Cabin_Layout_Parms", NULL );
  
  xmlNodePtr ccsl_node = xmlNewChild( cabin_layout_node, NULL, (const xmlChar *)"Cabin_Cross_Sections_List", NULL );

  for(int i = 0; i < (int)vCabinInfo.size(); i++)
  {
	xmlNodePtr ccs_node = xmlNewChild( ccsl_node, NULL, (const xmlChar *)"Cabin_Cross_Sections", NULL );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS1_P1X", vCabinInfo[i].CabinPoints[0].x() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS1_P1Y", vCabinInfo[i].CabinPoints[0].y() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS1_P1Z", vCabinInfo[i].CabinPoints[0].z() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS1_P2X", vCabinInfo[i].CabinPoints[1].x() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS1_P2Y", vCabinInfo[i].CabinPoints[1].y() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS1_P2Z", vCabinInfo[i].CabinPoints[1].z() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS1_P3X", vCabinInfo[i].CabinPoints[2].x() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS1_P3Y", vCabinInfo[i].CabinPoints[2].y() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS1_P3Z", vCabinInfo[i].CabinPoints[2].z() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS1_P4X", vCabinInfo[i].CabinPoints[3].x() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS1_P4Y", vCabinInfo[i].CabinPoints[3].y() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS1_P4Z", vCabinInfo[i].CabinPoints[3].z() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS2_P1X", vCabinInfo[i].CabinPoints[4].x() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS2_P1Y", vCabinInfo[i].CabinPoints[4].y() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS2_P1Z", vCabinInfo[i].CabinPoints[4].z() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS2_P2X", vCabinInfo[i].CabinPoints[5].x() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS2_P2Y", vCabinInfo[i].CabinPoints[5].y() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS2_P2Z", vCabinInfo[i].CabinPoints[5].z() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS2_P3X", vCabinInfo[i].CabinPoints[6].x() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS2_P3Y", vCabinInfo[i].CabinPoints[6].y() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS2_P3Z", vCabinInfo[i].CabinPoints[6].z() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS2_P4X", vCabinInfo[i].CabinPoints[7].x() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS2_P4Y", vCabinInfo[i].CabinPoints[7].y() );
	xmlAddDoubleNode( ccs_node, "Cabin_Point_XS2_P4Z", vCabinInfo[i].CabinPoints[7].z() );
  }

  xmlAddStringNode( cabin_layout_node, "Geom_Data_File_Name", geom_data_file_name() );

  xmlAddIntNode( cabin_layout_node, "Mirror_Active", MirrorData );
}

//==== Read External File ====//
void CabinLayoutGeom::read(xmlNodePtr root)
{
  xmlNodePtr node, node2, node3;

  //===== Read General Parameters =====//
  node = xmlGetNode( root, "General_Parms", 0 );
  if ( node )
    read_general_parms( node );

  //===== Read Fuse Parameters =====//
  node = xmlGetNode( root, "Cabin_Layout_Parms", 0 );

  if ( node )
  {
	  node2 = xmlGetNode( node, "Cabin_Cross_Sections_List", 0);

	  if(node2)
	  {

		  int num_cabins = xmlGetNumNames(node2, "Cabin_Cross_Sections");

		  for(int i = 0; i < num_cabins; i++)
		  {
			  node3 = xmlGetNode( node2, "Cabin_Cross_Sections", i);
			  if(node3)
			  {
				  CabinGeomData new_data;
				  vCabinInfo.push_back(new_data);

				  double temp1x1 = 0.0, temp1y1 = 0.0, temp1z1 = 0.0;
				  double temp1x2 = 0.0, temp1y2 = 0.0, temp1z2 = 0.0;
				  double temp1x3 = 0.0, temp1y3 = 0.0, temp1z3 = 0.0;
				  double temp1x4 = 0.0, temp1y4 = 0.0, temp1z4 = 0.0;
				  double temp2x1 = 0.0, temp2y1 = 0.0, temp2z1 = 0.0;
				  double temp2x2 = 0.0, temp2y2 = 0.0, temp2z2 = 0.0;
				  double temp2x3 = 0.0, temp2y3 = 0.0, temp2z3 = 0.0;
				  double temp2x4 = 0.0, temp2y4 = 0.0, temp2z4 = 0.0;
				  temp1x1 = xmlFindDouble( node3, "Cabin_Point_XS1_P1X", temp1x1);
				  temp1y1 = xmlFindDouble( node3, "Cabin_Point_XS1_P1Y", temp1y1);
				  temp1z1 = xmlFindDouble( node3, "Cabin_Point_XS1_P1Z", temp1z1);
				  temp1x2 = xmlFindDouble( node3, "Cabin_Point_XS1_P2X", temp1x2);
				  temp1y2 = xmlFindDouble( node3, "Cabin_Point_XS1_P2Y", temp1y2);
				  temp1z2 = xmlFindDouble( node3, "Cabin_Point_XS1_P2Z", temp1z2);
				  temp1x3 = xmlFindDouble( node3, "Cabin_Point_XS1_P3X", temp1x3);
				  temp1y3 = xmlFindDouble( node3, "Cabin_Point_XS1_P3Y", temp1y3);
				  temp1z3 = xmlFindDouble( node3, "Cabin_Point_XS1_P3Z", temp1z3);
				  temp1x4 = xmlFindDouble( node3, "Cabin_Point_XS1_P4X", temp1x4);
				  temp1y4 = xmlFindDouble( node3, "Cabin_Point_XS1_P4Y", temp1y4);
				  temp1z4 = xmlFindDouble( node3, "Cabin_Point_XS1_P4Z", temp1z4);

				  temp2x1 = xmlFindDouble( node3, "Cabin_Point_XS2_P1X", temp2x1);
				  temp2y1 = xmlFindDouble( node3, "Cabin_Point_XS2_P1Y", temp2y1);
				  temp2z1 = xmlFindDouble( node3, "Cabin_Point_XS2_P1Z", temp2z1);
				  temp2x2 = xmlFindDouble( node3, "Cabin_Point_XS2_P2X", temp2x2);
				  temp2y2 = xmlFindDouble( node3, "Cabin_Point_XS2_P2Y", temp2y2);
				  temp2z2 = xmlFindDouble( node3, "Cabin_Point_XS2_P2Z", temp2z2);
				  temp2x3 = xmlFindDouble( node3, "Cabin_Point_XS2_P3X", temp2x3);
				  temp2y3 = xmlFindDouble( node3, "Cabin_Point_XS2_P3Y", temp2y3);
				  temp2z3 = xmlFindDouble( node3, "Cabin_Point_XS2_P3Z", temp2z3);
				  temp2x4 = xmlFindDouble( node3, "Cabin_Point_XS2_P4X", temp2x4);
				  temp2y4 = xmlFindDouble( node3, "Cabin_Point_XS2_P4Y", temp2y4);
				  temp2z4 = xmlFindDouble( node3, "Cabin_Point_XS2_P4Z", temp2z4);

				  vCabinInfo[i].CabinPoints.push_back(vec3d(temp1x1,temp1y1,temp1z1));
				  vCabinInfo[i].CabinPoints.push_back(vec3d(temp1x2,temp1y2,temp1z2));
				  vCabinInfo[i].CabinPoints.push_back(vec3d(temp1x3,temp1y3,temp1z3));
				  vCabinInfo[i].CabinPoints.push_back(vec3d(temp1x4,temp1y4,temp1z4));

				  vCabinInfo[i].CabinPoints.push_back(vec3d(temp2x1,temp2y1,temp2z1));
				  vCabinInfo[i].CabinPoints.push_back(vec3d(temp2x2,temp2y2,temp2z2));
				  vCabinInfo[i].CabinPoints.push_back(vec3d(temp2x3,temp2y3,temp2z3));
				  vCabinInfo[i].CabinPoints.push_back(vec3d(temp2x4,temp2y4,temp2z4));
			  }
		  }
	  }
  }

  if ( node )
  {
	  geom_data_file_name = xmlFindString( node, "Geom_Data_File_Name", geom_data_file_name );
	  MirrorData = xmlFindInt( node, "Mirror_Active", MirrorData ) != 0;;
  }

  compose_model_matrix();
  generate();
  updateAttach(0);
  airPtr->geomMod( this );
  airPtr->recenterACView();
}

void CabinLayoutGeom::copy( Geom* fromGeom )
{
	Geom::copy( fromGeom );						// Copy Base Stuff

	if ( fromGeom->getType() != CABIN_LAYOUT_GEOM_TYPE )
		return;

	CabinLayoutGeom* pg = (CabinLayoutGeom*)fromGeom;

	geom_data_file_name = pg->geom_data_file_name;
	MirrorData = pg->MirrorData;

	for(int i = 0; i < (int)pg->vCabinInfo.size(); i++)
	{
		CabinGeomData newdata;

		for(int j = 0; j < (int)pg->vCabinInfo[i].CabinPoints.size(); j++)
		{
			newdata.CabinPoints.push_back(pg->vCabinInfo[i].CabinPoints[j]);
		}

		vCabinInfo.push_back(newdata);
	}

	compose_model_matrix();
	generate();
}

void CabinLayoutGeom::generate()
{
  gen_geom_surf();
  update_bbox();
}

void CabinLayoutGeom::set_geom_data_file_name(Stringc new_file) 
{
	geom_data_file_name = new_file;
	UpdateGeom = true;
	compose_model_matrix();
	generate();
	updateAttach(0);
	airPtr->geomMod( this );
	airPtr->recenterACView();
}

void CabinLayoutGeom::set_MirrorData(bool val) 
{
	MirrorData = val;
	compose_model_matrix();
	generate();
	updateAttach(0);
	airPtr->geomMod( this );
}

void CabinLayoutGeom::computeCenter()
{
	//==== Set Rotation Center ====//
	//center.set_x( origin()*length()*scaleFactor() ); 
}

void CabinLayoutGeom::parm_changed(Parm* p)
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
	}

	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );
}

void CabinLayoutGeom::acceptScaleFactor()
{
	int cabins = vCabinInfo.size();
	double minx = 1.0e12, miny = 1.0e12, minz = 1.0e12;
	double maxx = -1.0e12, maxy = -1.0e12, maxz = -1.0e12;
	vec3d center;

	for(int i = 0; i < cabins; i++)
	{
		CabinGeomData curr_cabin = vCabinInfo[i];
		
		for(int j = 0; j < 8; j++)
		{
			if(curr_cabin.CabinPoints[j].x() < minx)
				minx = curr_cabin.CabinPoints[j].x();
			
			if(curr_cabin.CabinPoints[j].x() > maxx)
				maxx = curr_cabin.CabinPoints[j].x();

			if(curr_cabin.CabinPoints[j].y() < miny)
				miny = curr_cabin.CabinPoints[j].y();
		
			if(curr_cabin.CabinPoints[j].y() > maxy)
				maxy = curr_cabin.CabinPoints[j].y();
	
			if(curr_cabin.CabinPoints[j].z() < minz)
				minz = curr_cabin.CabinPoints[j].z();
		
			if(curr_cabin.CabinPoints[j].z() > maxz)
				maxz = curr_cabin.CabinPoints[j].z();
		}
	}

	center.set_xyz(minx + (maxx-minx)/2,miny + (maxy-miny)/2,minz + (maxz-minz)/2);

	for(int i = 0; i < cabins; i++)
	{
		vCabinInfo[i].CabinPoints[0] = vCabinInfo[i].CabinPoints[0] - center;
		vCabinInfo[i].CabinPoints[1] = vCabinInfo[i].CabinPoints[1] - center;
		vCabinInfo[i].CabinPoints[2] = vCabinInfo[i].CabinPoints[2] - center;
		vCabinInfo[i].CabinPoints[3] = vCabinInfo[i].CabinPoints[3] - center;
		vCabinInfo[i].CabinPoints[4] = vCabinInfo[i].CabinPoints[4] - center;
		vCabinInfo[i].CabinPoints[5] = vCabinInfo[i].CabinPoints[5] - center;
		vCabinInfo[i].CabinPoints[6] = vCabinInfo[i].CabinPoints[6] - center;
		vCabinInfo[i].CabinPoints[7] = vCabinInfo[i].CabinPoints[7] - center;

		vCabinInfo[i].CabinPoints[0] = vCabinInfo[i].CabinPoints[0]*scaleFactor();
		vCabinInfo[i].CabinPoints[1] = vCabinInfo[i].CabinPoints[1]*scaleFactor();
		vCabinInfo[i].CabinPoints[2] = vCabinInfo[i].CabinPoints[2]*scaleFactor();
		vCabinInfo[i].CabinPoints[3] = vCabinInfo[i].CabinPoints[3]*scaleFactor();
		vCabinInfo[i].CabinPoints[4] = vCabinInfo[i].CabinPoints[4]*scaleFactor();
		vCabinInfo[i].CabinPoints[5] = vCabinInfo[i].CabinPoints[5]*scaleFactor();
		vCabinInfo[i].CabinPoints[6] = vCabinInfo[i].CabinPoints[6]*scaleFactor();
		vCabinInfo[i].CabinPoints[7] = vCabinInfo[i].CabinPoints[7]*scaleFactor();

		vCabinInfo[i].CabinPoints[0] = vCabinInfo[i].CabinPoints[0] + center;
		vCabinInfo[i].CabinPoints[1] = vCabinInfo[i].CabinPoints[1] + center;
		vCabinInfo[i].CabinPoints[2] = vCabinInfo[i].CabinPoints[2] + center;
		vCabinInfo[i].CabinPoints[3] = vCabinInfo[i].CabinPoints[3] + center;
		vCabinInfo[i].CabinPoints[4] = vCabinInfo[i].CabinPoints[4] + center;
		vCabinInfo[i].CabinPoints[5] = vCabinInfo[i].CabinPoints[5] + center;
		vCabinInfo[i].CabinPoints[6] = vCabinInfo[i].CabinPoints[6] + center;
		vCabinInfo[i].CabinPoints[7] = vCabinInfo[i].CabinPoints[7] + center;
	}
		
	update_bbox();
	scaleFactor.set(1.0);
}

void CabinLayoutGeom::resetScaleFactor()
{
	scaleFactor.set( 1.0 );
	generate();

	//==== Tell Aircraft You Have Changed ====//
	airPtr->geomMod( this );
}

void CabinLayoutGeom::scale()
{

}

void CabinLayoutGeom::draw()
{
	Geom::draw();
}

void CabinLayoutGeom::drawAlpha()
{
	Geom::drawAlpha();
}

void CabinLayoutGeom::gen_geom_surf()
{
	vector<CabinGeomData> vtempcabininfo;

	if(geom_data_file_name != "" && UpdateGeom)
	{
		fstream filestrm(geom_data_file_name.get_char_star(), fstream::in);
		char buffer[255];
		int numdecks = 0;
		int numcabins = 0;

		int pos = -1;
		Stringc nextline;
		while(pos < 0)
		{
			filestrm.getline(buffer, 100, '\n');
			nextline = buffer;
			pos = nextline.search_for_substring("NUMBER OF DECKS");
//cout << "pos: " << pos << endl;
			if(pos < 0)
			{
				cout << "ERROR - NUMBER OF DECKS not found" << endl;
//cout << nextline << endl;
				if(!filestrm || filestrm.eof())
				{
					cout << "file stream has gone bad" << endl;
					return;
				}
			}
			else
			{
				nextline.delete_range(0,0);
				nextline.delete_range(0,14);
				numdecks = nextline.convert_to_integer();
//cout << nextline << endl;
			}
		}

		if(numdecks < 1)
		{
//			cout << "ERROR - numdecks < 1" << endl;
//cout << nextline << endl;
			return;
		}
		else
		{
			for(int i = 0; i < numdecks; i++)
			{
//cout << nextline << endl;
				pos = -1;
				while(pos < 0)
				{
					filestrm.getline(buffer, 100, '\n');
					nextline = buffer;
					pos = nextline.search_for_substring("CABINS");
					if(pos < 0)
					{
//						cout << "ERROR - CABINS not found" << endl;
//cout << nextline << endl;
						if(!filestrm || filestrm.eof())
						{
							if(!filestrm)
								cout << "file stream has gone bad" << endl;
							else
								cout << "eof" << endl;

							return;
						}
					}
					else
					{
//cout << nextline << endl;
						nextline.delete_range(0,pos-1);
						nextline.delete_range(0,5);
						numcabins = nextline.convert_to_integer();
								
						for(int j = 0; j < numcabins; j++)
						{	
							CabinGeomData data;
							vec3d coord;
							double x = 0.0, y = 0.0, z = 0.0;
							
							for(int k = 0; k < 8; k++)
							{
								filestrm >> x;
								filestrm >> y;
								filestrm >> z;
								coord.set_xyz(x,y,z);
								data.CabinPoints.push_back(coord);

								if(!filestrm || filestrm.eof())
								{
									cout << "file stream has gone bad" << endl;
									return;
								}
							}
							vtempcabininfo.push_back(data);
							filestrm.ignore(100, '\n');
						}
					}
				}
			}
		}

		UpdateGeom = false;
		vCabinInfo = vtempcabininfo;
	}

	int cabins = vCabinInfo.size();

	surfVec.clear();

	double minx = 1.0e12, miny = 1.0e12, minz = 1.0e12;
	double maxx = -1.0e12, maxy = -1.0e12, maxz = -1.0e12;
	vec3d center;

	for(int i = 0; i < cabins; i++)
	{
		CabinGeomData curr_cabin = vCabinInfo[i];
		
		for(int j = 0; j < 8; j++)
		{
			if(curr_cabin.CabinPoints[j].x() < minx)
				minx = curr_cabin.CabinPoints[j].x();
			
			if(curr_cabin.CabinPoints[j].x() > maxx)
				maxx = curr_cabin.CabinPoints[j].x();

			if(curr_cabin.CabinPoints[j].y() < miny)
				miny = curr_cabin.CabinPoints[j].y();
		
			if(curr_cabin.CabinPoints[j].y() > maxy)
				maxy = curr_cabin.CabinPoints[j].y();
	
			if(curr_cabin.CabinPoints[j].z() < minz)
				minz = curr_cabin.CabinPoints[j].z();
		
			if(curr_cabin.CabinPoints[j].z() > maxz)
				maxz = curr_cabin.CabinPoints[j].z();
		}
	}

	center.set_xyz(minx + (maxx-minx)/2,miny + (maxy-miny)/2,minz + (maxz-minz)/2);
//	cout << "CENTER: " << center.x() << "," << center.y() << "," << center.z() << endl;
//	cout << "minx: " << minx << " miny: " << miny << " minz: " << minz << endl;
//	cout << "maxx: " << maxx << " maxy: " << maxy << " maxz: " << maxz << endl;

	miny -= center.y();
	miny *= scaleFactor();
	miny += center.y();

	for(int i = 0; i < cabins; i++)
	{
		Xsec_surf* geom_surf = new Xsec_surf();
		Xsec_surf* mirror_surf;
		geom_surf->set_num_pnts(9);
		geom_surf->set_num_xsecs(6);
		CabinGeomData curr_cabin = vCabinInfo[i];

		vec3d point1 = curr_cabin.CabinPoints[0] - center;
		vec3d point2 = curr_cabin.CabinPoints[1] - center;
		vec3d point3 = curr_cabin.CabinPoints[2] - center;
		vec3d point4 = curr_cabin.CabinPoints[3] - center;
		vec3d point5 = curr_cabin.CabinPoints[4] - center;
		vec3d point6 = curr_cabin.CabinPoints[5] - center;
		vec3d point7 = curr_cabin.CabinPoints[6] - center;
		vec3d point8 = curr_cabin.CabinPoints[7] - center;

		point1 = point1*scaleFactor();
		point2 = point2*scaleFactor();
		point3 = point3*scaleFactor();
		point4 = point4*scaleFactor();
		point5 = point5*scaleFactor();
		point6 = point6*scaleFactor();
		point7 = point7*scaleFactor();
		point8 = point8*scaleFactor();

		point1 = point1 + center;
		point2 = point2 + center;
		point3 = point3 + center;
		point4 = point4 + center;
		point5 = point5 + center;
		point6 = point6 + center;
		point7 = point7 + center;
		point8 = point8 + center;

		vector<vec3d> vpoints;
		vpoints.push_back(point1);
		vpoints.push_back(point2);
		vpoints.push_back(point3);
		vpoints.push_back(point4);
		vpoints.push_back(point5);
		vpoints.push_back(point6);
		vpoints.push_back(point7);
		vpoints.push_back(point8);
/*
		cout << "point1: " << point1.x() << "," << point1.y() << "," << point1.z() << endl;
		cout << "point2: " << point2.x() << "," << point2.y() << "," << point2.z() << endl;
		cout << "point3: " << point3.x() << "," << point3.y() << "," << point3.z() << endl;
		cout << "point4: " << point4.x() << "," << point4.y() << "," << point4.z() << endl;
		cout << "point5: " << point5.x() << "," << point5.y() << "," << point5.z() << endl;
		cout << "point6: " << point6.x() << "," << point6.y() << "," << point6.z() << endl;
		cout << "point7: " << point7.x() << "," << point7.y() << "," << point7.z() << endl;
		cout << "point8: " << point8.x() << "," << point8.y() << "," << point8.z() << endl;
*/
		vec3d first_end_center, second_end_center;
		double localminx = 1.0e12, localminy = 1.0e12, localminz = 1.0e12;
		double localmaxx = -1.0e12, localmaxy = -1.0e12, localmaxz = -1.0e12;

		for(int j = 0; j < 4;j++)
		{
			if(vpoints[j].x() < localminx)
				localminx = vpoints[j].x();
			
			if(vpoints[j].x() > localmaxx)
				localmaxx = vpoints[j].x();

			if(vpoints[j].y() < localminy)
				localminy = vpoints[j].y();
		
			if(vpoints[j].y() > localmaxy)
				localmaxy = vpoints[j].y();
	
			if(vpoints[j].z() < localminz)
				localminz = vpoints[j].z();
		
			if(vpoints[j].z() > localmaxz)
				localmaxz = vpoints[j].z();
		}

		first_end_center.set_xyz(localminx + (localmaxx-localminx)/2,
								 localminy + (localmaxy-localminy)/2,
								 localminz + (localmaxz-localminz)/2);
		localminx = 1.0e12; 
		localminy = 1.0e12;
		localminz = 1.0e12;
		localmaxx = -1.0e12;
		localmaxy = -1.0e12;
		localmaxz = -1.0e12;

		for(int j = 4; j < 8;j++)
		{
			if(vpoints[j].x() < localminx)
				localminx = vpoints[j].x();
			
			if(vpoints[j].x() > localmaxx)
				localmaxx = vpoints[j].x();

			if(vpoints[j].y() < localminy)
				localminy = vpoints[j].y();
		
			if(vpoints[j].y() > localmaxy)
				localmaxy = vpoints[j].y();
	
			if(vpoints[j].z() < localminz)
				localminz = vpoints[j].z();
		
			if(vpoints[j].z() > localmaxz)
				localmaxz = vpoints[j].z();
		}

		second_end_center.set_xyz(localminx + (localmaxx-localminx)/2,
								 localminy + (localmaxy-localminy)/2,
								 localminz + (localmaxz-localminz)/2);

		//a - values used to interpolate within a cross section, creating the first and last cross section
		vec3d interp1a((point2 - point1)/2);
		vec3d interp2a((point3 - point2)/2);
		vec3d interp3a((point4 - point3)/2);
		vec3d interp4a((point1 - point4)/2);
		vec3d interp5a((point6 - point5)/2);
		vec3d interp6a((point7 - point6)/2);
		vec3d interp7a((point8 - point7)/2);
		vec3d interp8a((point5 - point8)/2);

		//interpolated points of first and last cross section
		vec3d interp_pnt1(point1+interp1a);
		vec3d interp_pnt2(point2+interp2a);
		vec3d interp_pnt3(point3+interp3a);
		vec3d interp_pnt4(point4+interp4a);
		vec3d interp_pnt5(point5+interp5a);
		vec3d interp_pnt6(point6+interp6a);
		vec3d interp_pnt7(point7+interp7a);
		vec3d interp_pnt8(point8+interp8a);

		//b - values used to create interpolated cross sections
		vec3d interp1b((point5 - point1)/3);
		vec3d interp2b((interp_pnt5 - interp_pnt1)/3);
		vec3d interp3b((point6 - point2)/3);
		vec3d interp4b((interp_pnt6 - interp_pnt2)/3);
		vec3d interp5b((point7 - point3)/3);
		vec3d interp6b((interp_pnt7 - interp_pnt3)/3);
		vec3d interp7b((point8 - point4)/3);
		vec3d interp8b((interp_pnt8 - interp_pnt4)/3);

		//points for first interpolated cross section
		vec3d xsect_interp_pnt1a(point1+interp1b);
		vec3d xsect_interp_pnt2a(interp_pnt1+interp2b);
		vec3d xsect_interp_pnt3a(point2+interp3b);
		vec3d xsect_interp_pnt4a(interp_pnt2+interp4b);
		vec3d xsect_interp_pnt5a(point3+interp5b);
		vec3d xsect_interp_pnt6a(interp_pnt3+interp6b);
		vec3d xsect_interp_pnt7a(point4+interp7b);
		vec3d xsect_interp_pnt8a(interp_pnt4+interp8b);

		//points for second interpolated cross section
		vec3d xsect_interp_pnt1b(point1+interp1b*2);
		vec3d xsect_interp_pnt2b(interp_pnt1+interp2b*2);
		vec3d xsect_interp_pnt3b(point2+interp3b*2);
		vec3d xsect_interp_pnt4b(interp_pnt2+interp4b*2);
		vec3d xsect_interp_pnt5b(point3+interp5b*2);
		vec3d xsect_interp_pnt6b(interp_pnt3+interp6b*2);
		vec3d xsect_interp_pnt7b(point4+interp7b*2);
		vec3d xsect_interp_pnt8b(interp_pnt4+interp8b*2);

		geom_surf->set_pnt(0,0,first_end_center);
		geom_surf->set_pnt(0,1,first_end_center);
		geom_surf->set_pnt(0,2,first_end_center);
		geom_surf->set_pnt(0,3,first_end_center);
		geom_surf->set_pnt(0,4,first_end_center);
		geom_surf->set_pnt(0,5,first_end_center);
		geom_surf->set_pnt(0,6,first_end_center);
		geom_surf->set_pnt(0,7,first_end_center);
		geom_surf->set_pnt(0,8,first_end_center);

		geom_surf->set_pnt(1,0,point1);
		geom_surf->set_pnt(1,1,interp_pnt1);
		geom_surf->set_pnt(1,2,point2);
		geom_surf->set_pnt(1,3,interp_pnt2);
		geom_surf->set_pnt(1,4,point3);
		geom_surf->set_pnt(1,5,interp_pnt3);
		geom_surf->set_pnt(1,6,point4);
		geom_surf->set_pnt(1,7,interp_pnt4);
		geom_surf->set_pnt(1,8,point1);

		geom_surf->set_pnt(2,0,xsect_interp_pnt1a);
		geom_surf->set_pnt(2,1,xsect_interp_pnt2a);
		geom_surf->set_pnt(2,2,xsect_interp_pnt3a);
		geom_surf->set_pnt(2,3,xsect_interp_pnt4a);
		geom_surf->set_pnt(2,4,xsect_interp_pnt5a);
		geom_surf->set_pnt(2,5,xsect_interp_pnt6a);
		geom_surf->set_pnt(2,6,xsect_interp_pnt7a);
		geom_surf->set_pnt(2,7,xsect_interp_pnt8a);
		geom_surf->set_pnt(2,8,xsect_interp_pnt1a);

		geom_surf->set_pnt(3,0,xsect_interp_pnt1b);
		geom_surf->set_pnt(3,1,xsect_interp_pnt2b);
		geom_surf->set_pnt(3,2,xsect_interp_pnt3b);
		geom_surf->set_pnt(3,3,xsect_interp_pnt4b);
		geom_surf->set_pnt(3,4,xsect_interp_pnt5b);
		geom_surf->set_pnt(3,5,xsect_interp_pnt6b);
		geom_surf->set_pnt(3,6,xsect_interp_pnt7b);
		geom_surf->set_pnt(3,7,xsect_interp_pnt8b);
		geom_surf->set_pnt(3,8,xsect_interp_pnt1b);

		geom_surf->set_pnt(4,0,point5);
		geom_surf->set_pnt(4,1,interp_pnt5);
		geom_surf->set_pnt(4,2,point6);
		geom_surf->set_pnt(4,3,interp_pnt6);
		geom_surf->set_pnt(4,4,point7);
		geom_surf->set_pnt(4,5,interp_pnt7);
		geom_surf->set_pnt(4,6,point8);
		geom_surf->set_pnt(4,7,interp_pnt8);
		geom_surf->set_pnt(4,8,point5);

		geom_surf->set_pnt(5,0,second_end_center);
		geom_surf->set_pnt(5,1,second_end_center);
		geom_surf->set_pnt(5,2,second_end_center);
		geom_surf->set_pnt(5,3,second_end_center);
		geom_surf->set_pnt(5,4,second_end_center);
		geom_surf->set_pnt(5,5,second_end_center);
		geom_surf->set_pnt(5,6,second_end_center);
		geom_surf->set_pnt(5,7,second_end_center);
		geom_surf->set_pnt(5,8,second_end_center);

		geom_surf->load_refl_pnts_xsecs();
		geom_surf->load_hidden_surf();
		geom_surf->load_normals();
		geom_surf->load_uw();
		surfVec.push_back( geom_surf );

		if(MirrorData)
		{
			mirror_surf = new Xsec_surf();
			mirror_surf->set_num_xsecs(6);
			mirror_surf->set_num_pnts(9);

			int numgeomsurfs = geom_surf->get_num_xsecs();
			int numgeomsurfpnts = geom_surf->get_num_pnts();
			int m = 0;
			for(int j = 0; j < numgeomsurfs; j++)
			{
				for(int k = (numgeomsurfpnts-1); k >= 0; k--)
				{
					vec3d curr_pnt = geom_surf->get_pnt(j,k);
					curr_pnt.set_y(curr_pnt.y()-miny);
					curr_pnt.set_y(curr_pnt.y()*-1);
					curr_pnt.set_y(curr_pnt.y()+miny);
					mirror_surf->set_pnt(j,m,curr_pnt);
					m++;
				}
				m = 0;
			}

			mirror_surf->load_refl_pnts_xsecs();
			mirror_surf->load_hidden_surf();
			mirror_surf->load_normals();
			mirror_surf->load_uw();
			surfVec.push_back( mirror_surf );
		}
	}
}

void CabinLayoutGeom::update_bbox()
{
  vec3d pnt;
  bbox new_box;

//  int num_pnts  = geom_surf.get_num_pnts();
//  int num_xsecs = geom_surf.get_num_xsecs();

  //==== Check All Xsecs ====//
  int numsurf = surfVec.size();
  for ( int k = 0; k < numsurf; k++)
  {
	for ( int i = 0 ; i < 4 ; i++ )
	{	
		for ( int j = 0 ; j < 9 ; j++ )
		{
			new_box.update(surfVec[k]->get_pnt(i,j));
		}
	}
  }
  bnd_box = new_box;

  update_xformed_bbox();			// Load Xform BBox
}
/*
void CabinLayoutGeom::write_rhino_file(ON_BinaryFile* archive, ON_3dmObjectAttributes* attributes)
{
	geom_surf.clear_xsec_tan_flags();

	geom_surf.set_xsec_tan_flag( 0, Bezier_curve::ZERO );
    int num_xsecs  = geom_surf.get_num_xsecs();
	geom_surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::ZERO );

	geom_surf.write_rhino_file( sym_code, model_mat, reflect_mat, archive, attributes );
}
*/

int CabinLayoutGeom::get_num_bezier_comps()
{
	if ( sym_code == NO_SYM )
		return 1;
	else 
		return 2;
}
/*
void CabinLayoutGeom::write_bezier_file( int id, FILE* file_id )
{
	geom_surf.clear_xsec_tan_flags();

	geom_surf.set_xsec_tan_flag( 0, Bezier_curve::PREDICT  );
    int num_xsecs  = geom_surf.get_num_xsecs();
	geom_surf.set_xsec_tan_flag( num_xsecs-1, Bezier_curve::PREDICT );

	vector <int> u_split;
	u_split.push_back( 0 );
	u_split.push_back( 3*(geom_surf.get_num_xsecs() - 1)  );
	vector <int> w_split;
	w_split.push_back( 0 );
	w_split.push_back( 3*(geom_surf.get_num_pnts()/4)  );
	w_split.push_back( 3*(geom_surf.get_num_pnts()/2)  );
	w_split.push_back( 3*3*(geom_surf.get_num_pnts()/4)  );
	w_split.push_back( 3*(geom_surf.get_num_pnts() - 1)  );

	geom_surf.write_bezier_file( file_id, sym_code, model_mat, reflect_mat, u_split, w_split  );
}
*/

vector< TMesh* > CabinLayoutGeom::createTMeshVec()
{
	vector< TMesh* > tMeshVec;

	int numsurf = surfVec.size();
	for(int i = 0; i < numsurf; i++)
	{
		TMesh* tmPtr = surfVec[i]->createTMesh(model_mat);
		tMeshVec.push_back( tmPtr );

		if ( sym_code != NO_SYM )
		{
			tmPtr = surfVec[i]->createReflTMesh(sym_code, reflect_mat);
			tMeshVec.push_back( tmPtr );
		}
	}
	return tMeshVec;
}

void CabinLayoutGeom::dump_xsec_file(int geom_no, FILE* dump_file)
{
  int i;

  fprintf(dump_file, "\n");
  fprintf(dump_file, "%s \n", (char*) getName());
  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
  fprintf(dump_file, " TYPE              = 1  \n");						// Non Lifting
  fprintf(dump_file, " CROSS SECTIONS    = %d \n",4);
  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",9);

  int numsurf = surfVec.size();
  for(int j = 0; j < numsurf; j++)
  {
	for ( i = 0 ; i < 4 ; i++ )
	{
		surfVec[j]->write_xsec(i, model_mat, dump_file );
    }
  }

  if ( sym_code == NO_SYM ) return;

  fprintf(dump_file, "\n");
  fprintf(dump_file, "%s \n", (char*) getName());
  fprintf(dump_file, " GROUP NUMBER      = %d \n",geom_no);
  fprintf(dump_file, " TYPE              = 1  \n");						// Non Lifting
  fprintf(dump_file, " CROSS SECTIONS    = %d \n",4);
  fprintf(dump_file, " PTS/CROSS SECTION = %d \n",9);

  for(int j = 0; j < numsurf; j++)
  {
	for ( i = 0 ; i < 4 ; i++ )
    {
      surfVec[j]->write_refl_xsec(sym_code, i, reflect_mat, dump_file);
    }
  }
}

double CabinLayoutGeom::getRefArea()
{
	if ( autoRefAreaFlag )
	{

	}
	return refArea;
}

double CabinLayoutGeom::getRefSpan()
{
	if ( autoRefSpanFlag )
	{

	}
	return refSpan;
}

double CabinLayoutGeom::getRefCbar()
{
	if ( autoRefCbarFlag )
	{

	}
	return refCbar;
}

vec3d CabinLayoutGeom::getAeroCenter()
{
	if ( autoAeroCenterFlag )
	{
//		aeroCenter.set_x(  );
//		aeroCenter.set_y(  );
//		aeroCenter.set_z(  );
	}
	return aeroCenter;
}

void CabinLayoutGeom::AddDefaultSources(double base_len)
{
	//==== Add Some Default Sources for CFD Mesh ====//
	PointSource* ps = new PointSource();
//	ps->SetName("Nose");
//	ps->SetLen( 0.1 );
//	ps->SetRad( length()*0.1 );
//	vec2d loc = vec2d(0,0);
//	ps->SetUWLoc( loc );
//	ps->SetGeomPtr( this );
	sourceVec.push_back( ps );

	ps = new PointSource();
//	ps->SetName("Tail");
//	ps->SetLen( 0.1 );
//	ps->SetRad( length()*0.1 );
//	loc = vec2d(1,0);
//	ps->SetUWLoc( loc );
//	ps->SetGeomPtr( this );
	sourceVec.push_back( ps );
}

void CabinLayoutGeom::getVertexVec(vector< VertexID > *vertVec)		
{ 
	int size = surfVec.size();
	for(int i = 0; i < size; i++)
	{
		buildVertexVec(surfVec[i], i, vertVec); 
	}
}



