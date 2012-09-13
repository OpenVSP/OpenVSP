//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// drawBase.cpp: implementation of the drawBase class.
//
//////////////////////////////////////////////////////////////////////

#include "editCurve.h"

#ifdef WIN32
#include <windows.h>		
#endif

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <stdio.h>
#include <FL/Fl.H>
#include "curveEditScreen.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


EditPnt::EditPnt()
{
	smoothFlag = 1;
	pnt = vec3d(   0,    0,    0);
	d0  = vec3d(-1.0,  0.0,  0.0);
	m0  = 0.25;
	d1  = vec3d( 1.0,  0.0,  0.0);
	m1  = 0.25;

	pntLockX = 0;
	pntLockY = 0;

	tan0LockX = 0;
	tan0LockY = 0;

	tan1LockX = 0;
	tan1LockY = 0;

}

EditCurve::EditCurve(GeomBase* geom_ptr_in)
{

	geom_ptr = geom_ptr_in;

	winWidth  = 100;
	winHeight = 100;
	winLeft   = -1.0;
	winRight  =  1.0;
	winTop    =  1.0;
	winBot    = -1.0;

	closeFlag = 0;

	nearPntID  = 0;
	selectPntID = 0;
	screenPtr = 0;
	symFlag = EC_NO_SYM;
	shapeType = EC_3PNT_CIRCLE;


	scaleX.initialize(this, UPD_EDIT_CRV, "Scale X", 1.0 );
	scaleX.set_lower_upper( 0.01, 100.0);

	scaleY.initialize(this, UPD_EDIT_CRV, "Scale Y", 1.0 );
	scaleY.set_lower_upper( 0.01, 100.0);

	lastScaleX = scaleX();
	lastScaleY = scaleY();
	drawScaleFactor = 1.0;
	rollerScaleFactor = 2.5;
	setDrawScaleFactor( rollerScaleFactor );

	pntX.initialize(this, UPD_EDIT_CRV, "Pnt X", 0.0 );
	pntX.set_lower_upper(-1000000.0, 1000000.0);

	pntY.initialize(this, UPD_EDIT_CRV, "Pnt Y", 0.0 );
	pntY.set_lower_upper(-1000000.0, 1000000.0);

	scaleTans.initialize(this, UPD_EDIT_CRV, "Scale Tangents", 1.0 );
	scaleTans.set_lower_upper( 0.01, 10.0);

    maxWidth.initialize(this, UPD_EDIT_CRV, "Max Width", 1.0 );
    maxHeight.initialize(this, UPD_EDIT_CRV, "Max Height", 1.0 );

	maxWidth.deactivate();
	maxHeight.deactivate();

	initShape();
	generate();

}

EditCurve::~EditCurve()
{

}

void EditCurve::copy( EditCurve & iec )
{
	scaleX = iec.scaleX.get();
	scaleY = iec.scaleY.get();
	pntX   = iec.pntX.get();
	pntY   = iec.pntY.get();
	scaleTans = iec.scaleTans.get();
	maxWidth = iec.maxWidth.get();
	maxHeight = iec.maxHeight.get();

	crv = iec.crv;
	pntVec = iec.pntVec;
	controlPntVec = iec.controlPntVec;

	winWidth  = iec.winWidth;
	winHeight = iec.winHeight;

	winLeft = iec.winLeft;
	winRight = iec.winRight;
	winTop = iec.winTop;
	winBot = iec.winBot;

	symFlag = iec.symFlag;
	shapeType = iec.shapeType;

	lastScaleX = iec.lastScaleX;
	lastScaleY = iec.lastScaleY;

	rollerScaleFactor = iec.rollerScaleFactor;
	drawScaleFactor = iec.drawScaleFactor;




}


void EditCurve::write(xmlNodePtr root)
{
	xmlAddIntNode( root, "Sym_Flag", symFlag);
	xmlAddIntNode( root, "Shape_Type", shapeType);

	for ( int i = 0 ; i < (int)pntVec.size() ; i++ )
	{
		xmlNodePtr node = xmlNewChild( root, NULL, (const xmlChar *)"EditPnt", NULL );
		xmlAddIntNode( node, "Smooth_Flag", pntVec[i].smoothFlag);

		xmlAddDoubleNode( node, "PntX", pntVec[i].pnt.x() );
		xmlAddDoubleNode( node, "PntY", pntVec[i].pnt.y() );
		xmlAddDoubleNode( node, "PntZ", pntVec[i].pnt.z() );

		xmlAddDoubleNode( node, "D0X", pntVec[i].d0.x() );
		xmlAddDoubleNode( node, "D0Y", pntVec[i].d0.y() );
		xmlAddDoubleNode( node, "D0Z", pntVec[i].d0.z() );
		xmlAddDoubleNode( node, "M0", pntVec[i].m0 );

		xmlAddDoubleNode( node, "D1X", pntVec[i].d1.x() );
		xmlAddDoubleNode( node, "D1Y", pntVec[i].d1.y() );
		xmlAddDoubleNode( node, "D1Z", pntVec[i].d1.z() );
		xmlAddDoubleNode( node, "M1", pntVec[i].m1 );

		xmlAddIntNode( node, "Pnt_LockX", pntVec[i].pntLockX );
		xmlAddIntNode( node, "Pnt_LockY", pntVec[i].pntLockY );

		xmlAddIntNode( node, "Tan0_LockX", pntVec[i].tan0LockX );
		xmlAddIntNode( node, "Tan0_LockY", pntVec[i].tan0LockY );

		xmlAddIntNode( node, "Tan1_LockX", pntVec[i].tan1LockX );
		xmlAddIntNode( node, "Tan1_LockY", pntVec[i].tan1LockY );
	}
}

void EditCurve::read(xmlNodePtr root )
{
	symFlag   = xmlFindInt( root, "Sym_Flag", symFlag );
	shapeType = xmlFindInt( root, "Shape_Type", shapeType );

    int num_edit_pnts = xmlGetNumNames( root, "EditPnt" );

	if ( num_edit_pnts >= 2 )
	{
		pntVec.clear();

		for ( int i = 0 ; i < num_edit_pnts ; i++ )
		{
			xmlNodePtr ep_node = xmlGetNode( root, "EditPnt", i );

			EditPnt p;
			p.pnt.set_x( xmlFindDouble( ep_node, "PntX", 0.0 ) );
			p.pnt.set_y( xmlFindDouble( ep_node, "PntY", 0.0 ) );
			p.pnt.set_z( xmlFindDouble( ep_node, "PntZ", 0.0 ) );

			p.d0.set_x( xmlFindDouble( ep_node, "D0X", 0.0 ) );
			p.d0.set_y( xmlFindDouble( ep_node, "D0Y", 0.0 ) );
			p.d0.set_z( xmlFindDouble( ep_node, "D0Z", 0.0 ) );
			p.m0 = xmlFindDouble( ep_node, "M0", 0.0 );

			p.d1.set_x( xmlFindDouble( ep_node, "D1X", 0.0 ) );
			p.d1.set_y( xmlFindDouble( ep_node, "D1Y", 0.0 ) );
			p.d1.set_z( xmlFindDouble( ep_node, "D1Z", 0.0 ) );
			p.m1 = xmlFindDouble( ep_node, "M1", 0.0 );

			p.pntLockX = xmlFindInt( ep_node, "Pnt_LockX", 0 );
			p.pntLockY = xmlFindInt( ep_node, "Pnt_LockY", 0 );

			p.tan0LockX = xmlFindInt( ep_node, "Tan0_LockX", 0 );
			p.tan0LockY = xmlFindInt( ep_node, "Tan0_LockY", 0 );

			p.tan1LockX = xmlFindInt( ep_node, "Tan1_LockX", 0 );
			p.tan1LockY = xmlFindInt( ep_node, "Tan1_LockY", 0 );

			pntVec.push_back(p);
		}
		generate();
	}


}


void EditCurve::setEditPntVec( vector< EditPnt > & pVec )
{
	pntVec = pVec;

	if ( selectPntID >= (int)pVec.size()*3 )
		selectPntID = 0;

}


void EditCurve::initShape()
{
/*
	for ( int i = 0 ; i < pntVec.size() ; i ++ )
	{
		printf( " i = %d \n", i );
		printf( " Pnt = %f %f \n", pntVec[i].pnt.x(), pntVec[i].pnt.y() );
		printf( " D0  = %f %f \n", pntVec[i].d0.x(),  pntVec[i].d0.y() );
		printf( " M0  = %f\n",     pntVec[i].m0 );
		printf( " D1  = %f %f \n", pntVec[i].d1.x(),  pntVec[i].d1.y() );
		printf( " M1  = %f\n",     pntVec[i].m1 );
	}
*/

	pntVec.clear();
	EditPnt ep;
	if ( shapeType == EC_2PNT_CURVE )
	{
		symFlag = EC_NO_SYM;
		closeFlag = 0;
		ep.pnt = vec3d( 1.0, 0.0, 0.0 );
		ep.d0  = vec3d( 0.0,-1.0, 0.0 );
		ep.m0  = 1.0;
		ep.d1  = vec3d( 0.0, 1.0, 0.0 );
		ep.m1  = 1.0;
		pntVec.push_back( ep );

		ep.pnt = vec3d(-1.0, 0.0, 0.0 );
		ep.d0  = vec3d( 0.0, 1.0, 0.0 );
		ep.m0  = 1.0;
		ep.d1  = vec3d( 0.0,-1.0, 0.0 );
		ep.m1  = 1.0;
		pntVec.push_back( ep );
	}
	else if ( shapeType == EC_2PNT_CIRCLE )
	{
		symFlag = EC_Y_SYM;
		closeFlag = 0;
		ep.pnt = vec3d( 0.0, 1.0, 0.0 );
		ep.d0  = vec3d(-1.0, 0.0, 0.0 );
		ep.m0  = 1.0;
		ep.d1  = vec3d( 1.0, 0.0, 0.0 );
		ep.m1  = 1.3333333333;
		ep.pntLockX  = 1;
		ep.tan0LockY = 1;
		ep.tan1LockY = 1;
		pntVec.push_back( ep );

		ep.pnt = vec3d( 0.0,-1.0, 0.0 );
		ep.d0  = vec3d( 1.0, 0.0, 0.0 );
		ep.m0  = 1.333333333;
		ep.d1  = vec3d(-1.0, 0.0, 0.0 );
		ep.m1  = 1.0;
		pntVec.push_back( ep );
	}
	else if ( shapeType == EC_3PNT_CIRCLE )
	{
		symFlag = EC_Y_SYM;
		closeFlag = 0;
		ep.pnt = vec3d( 0.0, 1.0, 0.0 );
		ep.d0  = vec3d(-1.0, 0.0, 0.0 );
		ep.m0  = 1.0;
		ep.d1  = vec3d( 1.0, 0.0, 0.0 );
		ep.m1  = 0.55228;
		ep.pntLockX  = 1;
		ep.tan0LockY = 1;
		ep.tan1LockY = 1;
		pntVec.push_back( ep );

		ep.pnt = vec3d( 1.0, 0.0, 0.0 );
		ep.d0  = vec3d( 0.0, 1.0, 0.0 );
		ep.m0  = 0.55228;
		ep.d1  = vec3d( 0.0,-1.0, 0.0 );
		ep.m1  = 0.55228;
		ep.pntLockX  = 0;
		ep.tan0LockY = 0;
		ep.tan1LockY = 0;
		pntVec.push_back( ep );

		ep.pnt = vec3d( 0.0,-1.0, 0.0 );
		ep.d0  = vec3d( 1.0, 0.0, 0.0 );
		ep.m0  = 0.55228;
		ep.d1  = vec3d(-1.0, 0.0, 0.0 );
		ep.m1  = 1.0;
		ep.pntLockX  = 1;
		ep.tan0LockY = 1;
		ep.tan1LockY = 1;
		pntVec.push_back( ep );
	}
	else
	{
		symFlag = EC_NO_SYM;
		closeFlag = 0;
		ep.pnt = vec3d( 1.0, 0.0, 0.0 );
		ep.d0  = vec3d( 0.0,-1.0, 0.0 );
		ep.m0  = 1.0;
		ep.d1  = vec3d( 0.0, 1.0, 0.0 );
		ep.m1  = 1.0;
		pntVec.push_back( ep );

		ep.pnt = vec3d(-1.0, 0.0, 0.0 );
		ep.d0  = vec3d( 0.0, 1.0, 0.0 );
		ep.m0  = 1.0;
		ep.d1  = vec3d( 0.0,-1.0, 0.0 );
		ep.m1  = 1.0;
		pntVec.push_back( ep );
	}

//	generate();
//	triggerGeom();

//	if ( screenPtr )				// Update Control Pnt Data
//		screenPtr->show();

}

void EditCurve::generate()
{

	int i;

	if ( closeFlag )
	{
		if ( selectPntID == 0 )
		{
			pntVec[pntVec.size()-1].pnt = pntVec[0].pnt;
		}
		else
		{
			pntVec[0].pnt = pntVec[pntVec.size()-1].pnt;
		}
	}

	//==== Load Control Points ====//
	controlPntVec.clear();
	for ( i = 0 ; i < (int)pntVec.size() ; i ++ )
	{
		if ( i != 0 )
		{
			vec3d p = pntVec[i].pnt + pntVec[i].d0*(pntVec[i].m0*scaleTans());
			controlPntVec.push_back( p );
		}
		controlPntVec.push_back( pntVec[i].pnt );
		if ( i != pntVec.size()-1 )
		{
			vec3d p = pntVec[i].pnt + pntVec[i].d1*(pntVec[i].m1*scaleTans());
			controlPntVec.push_back( p );
		}
	}

	//==== Load Control Points ====//
	crv.init_num_sections(pntVec.size()-1);

	for ( i = 0 ; i < (int)controlPntVec.size() ; i++ )
	{
		crv.put_pnt( i, controlPntVec[i] );
	}

	computeMaxWH();


/*
	//==== Find Min/Max X and Y ====//
	double minx = pntVec[0].pnt.x();
	double maxx = pntVec[0].pnt.x();
	double miny = pntVec[0].pnt.y();
	double maxy = pntVec[0].pnt.y();

	for ( i = 1 ; i < pntVec.size() ; i++ )
	{
		vec3d p = pntVec[i].pnt;
		if ( p.x() < minx )			minx = p.x();
		else if ( p.x() > maxx )	maxx = p.x();
		if ( p.y() < miny )			miny = p.y();
		else if ( p.y() > maxy )	maxy = p.y();
	}
*/

}

void EditCurve::triggerGeom()
{
	//==== Trigger Update For Parents ====//
	if ( geom_ptr )
		geom_ptr->parm_changed( &pntX );
}


Bezier_curve EditCurve::getBezierCurve()
{
	if ( symFlag == EC_NO_SYM )
		return crv;

	//==== Account for Reflection ====//
	Bezier_curve reflcrv;

	//==== Copy In Crv ====//
	reflcrv.init_num_sections( crv.get_num_sections()*2 );

	int nump = crv.get_num_sections()*3 + 1;

	for ( int i = 0 ; i < nump ; i++ )
	{
		vec3d p = crv.get_pnt(i);
		reflcrv.put_pnt( i, p );

		if ( i != nump-1 )
		{
			vec3d rp;
			if ( symFlag == EC_X_SYM )
				rp = vec3d( p.x(),-p.y(), p.z() );
			else
				rp = vec3d(-p.x(), p.y(), p.z() );

			reflcrv.put_pnt( (nump*2 - 2 - i), rp );
		}

	}
	return reflcrv;

}

//==== Convert Bezier to Herm UNTESTED... ====//
herm_curve EditCurve::getHermCurve()
{
	vec3d tan;
	herm_curve hrm_crv;
	hrm_crv.init( crv.get_num_sections()*2 );

	int ccnt = 0;
	int hcnt = 0;
	for ( int i = 0 ; i < crv.get_num_sections() ; i++ )
	{
		hrm_crv.load_pnt( hcnt,   crv.get_pnt(ccnt) );
		hrm_crv.load_pnt( hcnt+1, crv.get_pnt(ccnt+3) );

		tan = crv.get_pnt(ccnt+1) -  crv.get_pnt(ccnt);
		hrm_crv.load_tan( hcnt,   tan*(3.0/8.0) );

		tan = crv.get_pnt(ccnt+3) -  crv.get_pnt(ccnt+2);
		hrm_crv.load_tan( hcnt+1,   tan*(3.0/8.0) );

		ccnt += 3;
		hcnt += 2;
	}
	

	return hrm_crv;
}	


void EditCurve::draw()
{

	int i;
	int numPnts = crv.get_num_sections()*3 + 1;

	glPushMatrix();

	glScalef( (float)drawScaleFactor, (float)drawScaleFactor, 1.0 );

	//==== Draw Grid ====//
	float gridSize;
	if ( drawScaleFactor < 0.025 )		gridSize = 10.0f;
	else if ( drawScaleFactor < 0.25 )	gridSize = 1.0f;
	else								gridSize = 0.1f;

	glLineWidth(1.0);
	glColor3f(0.8f, 0.8f, 0.8f);
	glBegin( GL_LINES );
	for ( i = 0 ; i < 41 ; i++ )
	{
		if ( i == 20 )
			glColor3f(0.8f, 0.8f, 0.8f);
		else
			glColor3f(0.9f, 0.9f, 0.9f);

		glVertex2f( gridSize*(float)i - 20.0f*gridSize, -20.0f*gridSize );
		glVertex2f( gridSize*(float)i - 20.0f*gridSize,  20.0f*gridSize );
		glVertex2f( -20.0f*gridSize, gridSize*(float)i - 20.0f*gridSize );
		glVertex2f(  20.0f*gridSize, gridSize*(float)i - 20.0f*gridSize );
	}
	glEnd();

	//==== Draw Control Grid ====//
	glColor3f( 0.0f, 0.0f, 1.0f );
	glBegin( GL_LINE_STRIP );
	for (i = 0 ; i < numPnts ; i++ )
	{
		vec3d p = crv.get_pnt(i);
		glVertex2d(  p.x(), p.y() );
	}
	glEnd();

	//==== Draw Current Section of Control Grid ====//
	int currSect = selectPntID/3;
	if ( currSect < (int)pntVec.size()-1 )
	{
		glColor3f( 0.0f, 1.0f, 0.0f );
		glBegin( GL_LINE_STRIP );
		for ( i = 3*currSect ; i < 3*currSect+4 ; i++ )
		{
			vec3d p = crv.get_pnt(i);
			glVertex2d(  p.x(), p.y() );
		}
		glEnd();
	}


	//==== Draw Curve ====//
	Bezier_curve refl_crv = getBezierCurve();

	glLineWidth(2.0);
	glColor3f( 1.0f, 0.0f, 0.0f );
	glBegin( GL_LINE_STRIP );
	for ( int isec = 0 ; isec < refl_crv.get_num_sections() ; isec++ )
	{
		for ( i = 0 ; i < 10 ; i++ )
		{
			double u = (double)i/(double)(9);
			vec3d p = refl_crv.comp_pnt( isec, u );
			glVertex2d(  p.x(), p.y() );
		}
	}
	glEnd();


	//==== Draw Control Points ====//
	glPointSize(4.0);
	glBegin( GL_POINTS );
	for ( i = 0 ; i < numPnts ; i++ )
	{
		if ( i%3 == 0  )
			glColor3f( 0.0f, 0.0f, 0.0f );
		else
			glColor3f( 0.0f, 0.0f, 1.0f );

		vec3d p = crv.get_pnt(i);
		glVertex2d(  p.x(), p.y() );
	}
	glEnd();

	//==== Draw Selected Control Points ====//
	glPointSize(4.0);
	glBegin( GL_POINTS );
		glColor3f( 1.0f, 1.0f, 0.0f );
		vec3d p = crv.get_pnt(nearPntID);
		glVertex2d(  p.x(), p.y() );
	glEnd();

	glPointSize(8.0);
	glBegin( GL_POINTS );
		glColor3f( 0.0f, 1.0f, 0.0f );
		p = crv.get_pnt(selectPntID);
		glVertex2d(  p.x(), p.y() );
	glEnd();

	glPopMatrix();

}

int EditCurve::processKeyEvent()
{
	int key = Fl::event_key();

	if ( key == 99 )		// "c" key
	{
		double maxSize = MAX( maxWidth(), maxHeight() );


		if ( maxSize > 0.0001 )
		{
			drawScaleFactor = 0.75/maxSize;
			rollerScaleFactor = log10(1000.0*drawScaleFactor);

			if ( screenPtr )				// Update Roller Position
				screenPtr->show();
		}
	}
	else if ( key == 102 )		// "f" key
	{
		FILE* fp = fopen( "edit_curve.dat", "w" );
		if ( fp )
		{
			fprintf(fp, "EditCurve\n");
			int max_id = 0;
			double max_x = -1.0e12;
			vector< vec3d > pVec;
			Bezier_curve bCrv = getBezierCurve();
			for ( int isec = 0 ; isec < bCrv.get_num_sections() ; isec++ )
			{
				for ( int i = 0 ; i < 20 ; i++ )
				{
					double u = (double)i/(double)(19);
					vec3d pnt = bCrv.comp_pnt( isec, u );
					pVec.push_back( pnt );

					if ( pnt.x() > max_x )
					{
						max_x = pnt.x();
						max_id = (int)pVec.size()-1;
					}
				}
			}
			double off_x = 1.0-max_x;
			for ( int i = max_id ; i >= 0 ; i-- )
			{
				fprintf( fp, "%f %f\n", pVec[i].x() + off_x, pVec[i].y() );
			}
			for ( int i = (int)(pVec.size()-1) ; i >= max_id ; i-- )
			{
				fprintf( fp, "%f %f\n", pVec[i].x() + off_x, pVec[i].y() );
			}
			fclose( fp );
		}
	}

	return 1;
}

int EditCurve::processDragEvent()
{
	//===== Get Mouse Position ====//
	int x = Fl::event_x(); 
	int y = Fl::event_y();

	double fx = (double)x/winWidth;
	double fy = (double)y/winHeight;

	double wx = winLeft + fx*(winRight-winLeft);
	double wy = winBot  + fy*(winTop - winBot );

	wx /= drawScaleFactor;
	wy /= drawScaleFactor;

	movePnt( wx, wy );

	pntX = wx;
	pntY = wy;

	generate();
	triggerGeom();

	return 1;
}

int EditCurve::processPushEvent()
{
	selectPntID = nearPntID;

	pntX = controlPntVec[selectPntID].x();
	pntY = controlPntVec[selectPntID].y();

	if ( screenPtr )				// Update Control Pnt Data
		screenPtr->show();


	return 1;
}

int EditCurve::processMoveEvent()
{
	int i;

	//===== Get Mouse Position ====//
	int x = Fl::event_x(); 
	int y = Fl::event_y();

	double fx = (double)x/winWidth;
	double fy = (double)y/winHeight;

	double wx = winLeft + fx*(winRight-winLeft);
	double wy = winBot  + fy*(winTop - winBot );

	wx /= drawScaleFactor;
	wy /= drawScaleFactor;

	nearPntID = 0;
	double nearDist = 1.0e06;

	for ( i = 0 ; i < (int)controlPntVec.size() ; i++ )
	{
		vec3d p = controlPntVec[i];
		double d = (p.x() - wx)*(p.x() - wx) + (p.y() - wy)*(p.y() - wy);

		if ( d < nearDist )
		{
			nearPntID = i;
			nearDist  = d;
		}
	}

	return 1;
}

//==== Parm Has Changed ReGenerate Component ====//
void EditCurve::parm_changed(Parm* p)
{
	if ( p == &pntX )
	{
		movePnt( pntX(), controlPntVec[selectPntID].y() );
	}
	else if ( p == &pntY )
	{
		movePnt( controlPntVec[selectPntID].x(), pntY() );
	}
	else if ( p == &scaleTans )
	{
	}
	else if ( p == &scaleX )
	{
		updateScale();
	}
	else if ( p == &scaleY )
	{
		updateScale();
	}
	else 
	{
		return;
	}
	generate();
	triggerGeom();

}

void EditCurve::computeMaxWH()
{
	int i;

	if ( pntVec.size() <= 0 )
		return;

	vec3d pMin = pntVec[0].pnt;
	vec3d pMax = pntVec[0].pnt;

	Bezier_curve refl_crv = getBezierCurve();

	for ( int isec = 0 ; isec < refl_crv.get_num_sections() ; isec++ )
	{
		for ( i = 0 ; i < 10 ; i++ )
		{
			double u = (double)i/(double)(9);
			vec3d p = refl_crv.comp_pnt( isec, u );

			//==== Find Min/Max XY of Curve =====//
			if ( p.x() < pMin.x() )			pMin.set_x( p.x() );
			else if ( p.x() > pMax.x() )	pMax.set_x( p.x() );

			if ( p.y() < pMin.y() )			pMin.set_y( p.y() );
			else if ( p.y() > pMax.y() )	pMax.set_y( p.y() );
		}
	}
	maxWidth  = pMax.x() - pMin.x();
	maxHeight = pMax.y() - pMin.y();

}

void EditCurve::blend( double fract, EditCurve* crv0,  EditCurve* crv1 )
{
	vector< EditPnt > epVec0 = crv0->getEditPntVec();
	vector< EditPnt > epVec1 = crv1->getEditPntVec();
	setEditPntVec( crv0->getEditPntVec() );

	if ( epVec0.size() < epVec1.size() )
	{
		setEditPntVec( epVec0 );
		int numadd = epVec1.size() - epVec0.size();

		while ( numadd > 0 )
		{
			selectPntID = 0;
			addEditPnt();

			numadd--;
			selectPntID += 6;
			if ( selectPntID > 3*((int)pntVec.size()-1) )
				selectPntID = 0;
		}
		epVec0 = getEditPntVec();
	}
	else
	{
		setEditPntVec( epVec1 );
		int numadd = epVec0.size() - epVec1.size();

		while ( numadd > 0 )
		{
			selectPntID = 0;
			addEditPnt();

			numadd--;
			selectPntID += 6;
			if ( selectPntID > 3*((int)pntVec.size()-1) )
				selectPntID = 0;
		}
		epVec1 = getEditPntVec();
	}

	vector< EditPnt > epBlend;

	for ( int i = 0 ; i < (int)epVec0.size() ; i++ )
	{
		EditPnt ep;
		ep.d0  = epVec0[i].d0  + (epVec1[i].d0  - epVec0[i].d0)*fract;
		ep.d1  = epVec0[i].d1  + (epVec1[i].d1  - epVec0[i].d1)*fract;
		ep.m0  = epVec0[i].m0  + (epVec1[i].m0  - epVec0[i].m0)*fract;
		ep.m1  = epVec0[i].m1  + (epVec1[i].m1  - epVec0[i].m1)*fract;
		ep.pnt = epVec0[i].pnt + (epVec1[i].pnt - epVec0[i].pnt)*fract;

		epBlend.push_back( ep );
	}

	scaleTans = crv0->getScaleTans()->get() + 
		( crv1->getScaleTans()->get() - crv0->getScaleTans()->get() )*fract;

//		Parm scaleX;
//	Parm scaleY;

	
	selectPntID = 0;
	setEditPntVec( epBlend );


}




void EditCurve::scaleWH( double w, double h )
{

	double sx = 1.0;
	double sy = 1.0;

	if ( maxWidth() > 0.000001 )
		sx = w/maxWidth();

	if ( maxHeight() > 0.000001 )
		sy = h/maxHeight();
	
	for ( int i = 0 ; i < (int)pntVec.size() ; i ++ )
	{
		vec3d p0 = pntVec[i].pnt + pntVec[i].d0*(pntVec[i].m0);
		vec3d p1 = pntVec[i].pnt + pntVec[i].d1*(pntVec[i].m1);

		p0.scale_x( sx );
		p0.scale_y( sy );
		p1.scale_x( sx );
		p1.scale_y( sy );

		pntVec[i].pnt.scale_x( sx );
		pntVec[i].pnt.scale_y( sy );

		pntVec[i].d0 = p0 - pntVec[i].pnt;
		pntVec[i].m0 = pntVec[i].d0.mag();
		if ( pntVec[i].m0 > 0.000001 )
			pntVec[i].d0.normalize();

		pntVec[i].d1 = p1 - pntVec[i].pnt;
		pntVec[i].m1 = pntVec[i].d1.mag();
		if ( pntVec[i].m1 > 0.000001 )
			pntVec[i].d1.normalize();
	}

	scaleX = 1.0;
	scaleY = 1.0;

}


void EditCurve::setDrawScaleFactor( double sf )
{
	rollerScaleFactor = sf;
	drawScaleFactor = pow(10.0, rollerScaleFactor)/1000.0;
}

void EditCurve::updateScale()
{
	if ( scaleTans() > 1.000001 || scaleTans() < 0.99999 )
		resetScaleTans();

	vec3d p;
	double currScaleX = scaleX()*(1.0/lastScaleX);
	double currScaleY = scaleY()*(1.0/lastScaleY);

	for ( int i = 0 ; i < (int)pntVec.size() ; i ++ )
	{
		vec3d p0 = pntVec[i].pnt + pntVec[i].d0*(pntVec[i].m0);
		vec3d p1 = pntVec[i].pnt + pntVec[i].d1*(pntVec[i].m1);

		p0.scale_x( currScaleX );
		p0.scale_y( currScaleY );
		p1.scale_x( currScaleX );
		p1.scale_y( currScaleY );

		pntVec[i].pnt.scale_x( currScaleX );
		pntVec[i].pnt.scale_y( currScaleY );

		pntVec[i].d0 = p0 - pntVec[i].pnt;
		pntVec[i].m0 = pntVec[i].d0.mag();
		if ( pntVec[i].m0 > 0.000001 )
			pntVec[i].d0.normalize();

		pntVec[i].d1 = p1 - pntVec[i].pnt;
		pntVec[i].m1 = pntVec[i].d1.mag();
		if ( pntVec[i].m1 > 0.000001 )
			pntVec[i].d1.normalize();
	}

	lastScaleX = scaleX();
	lastScaleY = scaleY();
}

void EditCurve::acceptScaleX()
{
	lastScaleX = 1.0;
	scaleX.set(1.0);
}

void EditCurve::resetScaleX()
{
	scaleX.set(1.0);
    updateScale();
	lastScaleX = 1.0;
}

void EditCurve::acceptScaleY()
{
	lastScaleY = 1.0;
	scaleY.set(1.0);
}

void EditCurve::resetScaleY()
{
	scaleY.set(1.0);
    updateScale();
	lastScaleY = 1.0;
}



void EditCurve::setCloseFlag( int flag )
{
	closeFlag = flag;

	if ( closeFlag )
	{
		if ( pntVec[0].smoothFlag )
			pntVec[pntVec.size()-1].d0 = pntVec[0].d1 * -1.0;

		if ( pntVec[pntVec.size()-1].smoothFlag )
			pntVec[0].d1 = pntVec[pntVec.size()-1].d0 * -1.0;
	}
//	triggerGeom();
}

int  EditCurve::getSmoothFlag() 
{
	if ( selectPntID%3 == 0 )
		return pntVec[selectPntID/3].smoothFlag;

	return 0;
}

void EditCurve::resetScaleTans()
{
	for ( int i = 0 ; i < (int)pntVec.size() ; i++ )
	{
		pntVec[i].m0 = pntVec[i].m0*scaleTans();
		pntVec[i].m1 = pntVec[i].m1*scaleTans();
	}
	scaleTans.set(1.0);

}

void EditCurve::setSmoothFlag( int flag )
{
	if ( selectPntID%3 == 0 )
	{
		pntVec[selectPntID/3].smoothFlag = flag;

		if ( closeFlag && selectPntID == 0 )
			pntVec[pntVec.size()-1].smoothFlag = flag;

		if ( closeFlag && selectPntID/3 == pntVec.size()-1 )
			pntVec[0].smoothFlag = flag;


		if ( pntVec[selectPntID/3].smoothFlag )
		{
			pntVec[selectPntID/3].d0 = pntVec[selectPntID/3].d1 * -1.0;

			if ( closeFlag && selectPntID/3 == 0 )
			{
				pntVec[pntVec.size()-1].d0 = pntVec[0].d1 * -1.0;
			}
			else if ( closeFlag && ( selectPntID/3+1 == pntVec.size()-1 ) )
			{
				pntVec[0].d1 = pntVec[selectPntID/3+1].d0 * -1.0;
			}
		}

	}
}
	
void EditCurve::movePnt( double x, double y )
{
	if ( scaleTans() > 1.000001 || scaleTans() < 0.99999 )
		resetScaleTans();

	if ( selectPntID%3 == 0 )
	{
		//==== Check For Locks ====//
		if ( pntVec[selectPntID/3].pntLockX == 0 )
			pntVec[selectPntID/3].pnt.set_x(x);
		if ( pntVec[selectPntID/3].pntLockY == 0 )
			pntVec[selectPntID/3].pnt.set_y(y);
	}
	else if ( selectPntID%3 == 1 )
	{
		//==== Check For Locks ====//
		vec3d oldPnt = pntVec[selectPntID/3].pnt + 
			 pntVec[selectPntID/3].d1 * (pntVec[selectPntID/3].m1);;
													
		if ( pntVec[selectPntID/3].tan1LockX == 0 )
			oldPnt.set_x(x);
		if ( pntVec[selectPntID/3].tan1LockY == 0 )
			oldPnt.set_y(y);

		pntVec[selectPntID/3].d1 = oldPnt - pntVec[selectPntID/3].pnt;
		pntVec[selectPntID/3].m1 = pntVec[selectPntID/3].d1.mag();
		if ( pntVec[selectPntID/3].m1 > 0.000001 )
			pntVec[selectPntID/3].d1.normalize();

		if ( pntVec[selectPntID/3].smoothFlag )
		{
			pntVec[selectPntID/3].d0 = pntVec[selectPntID/3].d1 * -1.0;

			if ( closeFlag && selectPntID/3 == 0 )
			{
				pntVec[pntVec.size()-1].d0 = pntVec[0].d1 * -1.0;
			}
		}
	}
	else if ( selectPntID%3 == 2 )
	{
		//==== Check For Locks ====//
		vec3d oldPnt = pntVec[selectPntID/3+1].pnt + 
			pntVec[selectPntID/3+1].d0*(pntVec[selectPntID/3+1].m0);

		if ( pntVec[selectPntID/3+1].tan0LockX == 0 )
			oldPnt.set_x(x);
		if ( pntVec[selectPntID/3+1].tan0LockY == 0 )
			oldPnt.set_y(y);

		pntVec[selectPntID/3+1].d0 = oldPnt - pntVec[selectPntID/3+1].pnt;

		pntVec[selectPntID/3+1].m0 = pntVec[selectPntID/3+1].d0.mag();

		if ( pntVec[selectPntID/3+1].m0 > 0.000001 )
			pntVec[selectPntID/3+1].d0.normalize();

		if ( pntVec[selectPntID/3+1].smoothFlag )
		{
			pntVec[selectPntID/3+1].d1 = pntVec[selectPntID/3+1].d0 * -1.0;

			if ( closeFlag && ( (selectPntID/3+1) == (pntVec.size()-1)) )
			{
				pntVec[0].d1 = pntVec[selectPntID/3+1].d0 * -1.0;
			}
		}
	}
}

int EditCurve::getLockX()
{
	if ( selectPntID%3 == 0 )
		return pntVec[selectPntID/3].pntLockX;
	
	else if ( selectPntID%3 == 1 )
		return pntVec[selectPntID/3].tan1LockX;
	
	else if ( selectPntID%3 == 2 )
		return pntVec[selectPntID/3+1].tan0LockX;
	
	return 0;
}

void EditCurve::setLockX( int flag )
{
	if ( selectPntID%3 == 0 )
		pntVec[selectPntID/3].pntLockX = flag;
	
	else if ( selectPntID%3 == 1 )
		pntVec[selectPntID/3].tan1LockX = flag;
	
	else if ( selectPntID%3 == 2 )
		pntVec[selectPntID/3+1].tan0LockX = flag;
}

int EditCurve::getLockY()
{
	if ( selectPntID%3 == 0 )
		return pntVec[selectPntID/3].pntLockY;
	
	else if ( selectPntID%3 == 1 )
		return pntVec[selectPntID/3].tan1LockY;
	
	else if ( selectPntID%3 == 2 )
		return pntVec[selectPntID/3+1].tan0LockY;
	
	return 0;
}


void EditCurve::setLockY( int flag )
{
	if ( selectPntID%3 == 0 )
		pntVec[selectPntID/3].pntLockY = flag;
	
	else if ( selectPntID%3 == 1 )
		pntVec[selectPntID/3].tan1LockY =flag;
	
	else if ( selectPntID%3 == 2 )
		pntVec[selectPntID/3+1].tan0LockY = flag;
	
}

void EditCurve::delEditPnt()
{
	if ( pntVec.size() <= 2 )
		return;

	int id = selectPntID/3;

	vector< EditPnt > tmpVec;
	for ( int i = 0 ; i < (int)pntVec.size() ; i++ )
	{
		if ( i != id )
			tmpVec.push_back( pntVec[i] );
	}

	pntVec = tmpVec;

	selectPntID = 0;
	nearPntID = 0;


}

void EditCurve::addEditPnt()
{
	int i;
	int currSect = selectPntID/3;

	if ( currSect < (int)pntVec.size()-1 )
	{
		vec3d b0 = controlPntVec[currSect*3];
		vec3d b1 = controlPntVec[currSect*3+1];
		vec3d b2 = controlPntVec[currSect*3+2];
		vec3d b3 = controlPntVec[currSect*3+3];

		vec3d m  = (b1+b2)*0.5;				// Subdivide Bezier Curve
		vec3d l0 = b0;
		vec3d r3 = b3;
		vec3d l1 = (b0+b1)*0.5;
		vec3d r2 = (b2+b3)*0.5;
		vec3d l2 = (l1+m)*0.5;
		vec3d r1 = (m+r2)*0.5;
		vec3d l3 = (l2+r1)*0.5;
		vec3d r0 = l3;

		vector< EditPnt > tmpVec;

		for ( i = 0 ; i < currSect ; i++ )
		{
			tmpVec.push_back( pntVec[i] );
		}

		EditPnt ep = pntVec[currSect];

		ep.d1  = l1 - l0;
		ep.m1  = ep.d1.mag();
		if ( ep.m1 > 0.000001 )
			ep.d1.normalize();

		tmpVec.push_back( ep );

		ep.pnt = l3;
		ep.pntLockX = ep.pntLockY = 0;

		ep.d0  = l2 - l3;
		ep.m0  = ep.d0.mag();
		if ( ep.m0 > 0.000001 )
			ep.d0.normalize();

		ep.tan0LockX = ep.tan0LockY = 0;

		ep.d1  = r1 - r0;
		ep.m1  = ep.d1.mag();
		if ( ep.m1 > 0.000001 )
			ep.d1.normalize();

		ep.tan1LockX = ep.tan1LockY = 0;

		tmpVec.push_back( ep );

		ep = pntVec[currSect+1];

		ep.d0  = r2 - r3;
		ep.m0  = ep.d0.mag();
		if ( ep.m0 > 0.000001 )
			ep.d0.normalize();
		tmpVec.push_back( ep );


		for ( i = currSect+2 ; i < (int)pntVec.size() ; i++ )
		{
			tmpVec.push_back( pntVec[i] );
		}
		
		pntVec = tmpVec;
	}
}







