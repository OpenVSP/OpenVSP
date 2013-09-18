//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ramGlWindow.cpp: implementation of the ramGlWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "vspGlWindow.h"
#include "aircraft.h"
#include "screenMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
VirtGlWindow::VirtGlWindow(int xi, int yi, int wi, int hi)
{
	resize( xi, yi, wi, hi );
	showFlag = 1;
	activeFlag = 0;
	focusFlag = 0;
	fastDrawFlag = false;
	axisDrawFlag = true;

	cx = cy = cz = 0.0;
	mousex = mousey = 0;
	cursorx = cursory = 0;

	currTrack.set_init_scale( 1.0/10.0 );
	currTrack.init();

	aircraftPtr = 0;

	mouseButton1 = mouseButton2 = mouseButton3 = 0;

	clearR = clearG = clearB = 255;
	drawBackImgFlag = 0;
	jpgImgData.data = 0;

	backImgScaleW = 1.0;
	backImgScaleH = 1.0;
	backImgOffsetX = 0.0;
	backImgOffsetY = 0.0;

	for ( int i = 0 ; i < NUM_USER_VIEWS ; i++ )
	{
		savedUserViewFlag[i] = 0;
	}

	font = NULL;
}

VirtGlWindow::~VirtGlWindow()
{

}

void VirtGlWindow::writeFile(xmlNodePtr node)
{
	xmlAddDoubleNode( node, "Back_Img_Scale_W", backImgScaleW );
	xmlAddDoubleNode( node, "Back_Img_Scale_H", backImgScaleH );
	xmlAddDoubleNode( node, "Back_Img_Offset_X", backImgOffsetX );
	xmlAddDoubleNode( node, "Back_Img_Offset_Y", backImgOffsetY );	

	//==== Load Components Into XML Tree ====//
	xmlNodePtr user_view_list_node = xmlNewChild( node, NULL, (const xmlChar *)"User_View_List", NULL );

	for ( int i = 0 ; i < NUM_USER_VIEWS ; i++ )		
	{
		xmlNodePtr user_view_node = xmlNewChild( user_view_list_node, NULL, (const xmlChar *)"User_View", NULL );

		xmlAddIntNode( user_view_node, "Saved_User_View_Flag", savedUserViewFlag[i] );	
		if ( savedUserViewFlag[i] )
		{
			xmlAddDoubleNode( user_view_node, "Saved_Rot_Center_X", savedRotationCenters[i].x() );	
			xmlAddDoubleNode( user_view_node, "Saved_Rot_Center_Y", savedRotationCenters[i].y() );	
			xmlAddDoubleNode( user_view_node, "Saved_Rot_Center_Z", savedRotationCenters[i].z() );	

			xmlAddDoubleNode( user_view_node, "Track_Scale", savedTrackVec[i].get_scale() );	
			xmlAddDoubleNode( user_view_node, "Track_Trans_X", savedTrackVec[i].get_trans_x() );	
			xmlAddDoubleNode( user_view_node, "Track_Trans_Y", savedTrackVec[i].get_trans_y() );	
			xmlAddDoubleNode( user_view_node, "Track_Spin_Rot_X", savedTrackVec[i].get_rvec_x() );	
			xmlAddDoubleNode( user_view_node, "Track_Spin_Rot_Y", savedTrackVec[i].get_rvec_y() );	
			xmlAddDoubleNode( user_view_node, "Track_Spin_Rot_Z", savedTrackVec[i].get_rvec_z() );	
			xmlAddDoubleNode( user_view_node, "Track_Spin_Rot_W", savedTrackVec[i].get_rvec_w() );	
		}
	}
}

void VirtGlWindow::readFile(xmlNodePtr node)
{
	backImgScaleW = xmlFindDouble( node, "Back_Img_Scale_W", 1.0 );
	backImgScaleH = xmlFindDouble( node, "Back_Img_Scale_H", 1.0 );
	backImgOffsetX = xmlFindDouble( node, "Back_Img_Offset_X", 0.0 );
	backImgOffsetY = xmlFindDouble( node, "Back_Img_Offset_Y", 0.0 );

	xmlNodePtr user_view_list_node = xmlGetNode( node, "User_View_List", 0 );
	if ( user_view_list_node )
	{
		int num_user_views = xmlGetNumNames( user_view_list_node, "User_View" );
		for ( int i = 0 ; i < num_user_views ; i++ )
		{
			xmlNodePtr user_view_node = xmlGetNode( user_view_list_node, "User_View", i );

			if ( i < NUM_USER_VIEWS )
			{
				savedUserViewFlag[i] = xmlFindInt( user_view_node, "Saved_User_View_Flag", 0 );

				if ( savedUserViewFlag[i] )
				{
					savedRotationCenters[i].set_x( xmlFindDouble( user_view_node, "Saved_Rot_Center_X", 0.0 ) );
					savedRotationCenters[i].set_y( xmlFindDouble( user_view_node, "Saved_Rot_Center_Y", 0.0 ) );
					savedRotationCenters[i].set_z( xmlFindDouble( user_view_node, "Saved_Rot_Center_Z", 0.0 ) );
			
					savedTrackVec[i].set_scale( xmlFindDouble( user_view_node, "Track_Scale", 1.0 ) );
					double trans_x = xmlFindDouble( user_view_node, "Track_Trans_X", 0.0 );
					double trans_y = xmlFindDouble( user_view_node, "Track_Trans_Y", 0.0 );
					savedTrackVec[i].set_trans( trans_x, trans_y, 0.0 );

					double rx = xmlFindDouble( user_view_node, "Track_Spin_Rot_X", 0.0 );
					double ry = xmlFindDouble( user_view_node, "Track_Spin_Rot_Y", 0.0 );
					double rz = xmlFindDouble( user_view_node, "Track_Spin_Rot_Z", 0.0 );
					double rw = xmlFindDouble( user_view_node, "Track_Spin_Rot_W", 1.0 );

					savedTrackVec[i].set_rvec( rx, ry, rz, rw );
				}
			}
		}
	}
}

void VirtGlWindow::resetBackImgDefaults()
{
	backImgScaleW = 1.0;
	backImgScaleH = 1.0;
	backImgOffsetX = 0.0;
	backImgOffsetY = 0.0;
}
void VirtGlWindow::resize( int xi, int yi, int wi, int hi )
{
	wx = xi;
	wy = yi;
	ww = wi;
	wh = hi;
	currTrack.size_window( 0, 0, ww, wh );
	for ( int i = 0 ; i < NUM_USER_VIEWS ; i++ )
		savedTrackVec[i].size_window( 0, 0, ww, wh );

	if ( ww < wh )
	{
		orthoL = -0.5;	
		orthoR = -orthoL;
		orthoT = -0.5*(double)wh/(double)ww;
		orthoB = -orthoT;
	}
	else
	{
		orthoL = -0.5*(double)ww/(double)wh;	
		orthoR = -orthoL;
		orthoT = -0.5;
		orthoB = -orthoT;
	}
}

void VirtGlWindow::setScale( double scale )
{
	currTrack.set_init_scale( scale );
	
}

void VirtGlWindow::setBackImgFile( const char* fname )
{
	if ( jpgImgData.data )
		delete jpgImgData.data;

	jpgImgData = loadJPEG(fname);
}


void VirtGlWindow::predraw()
{
	glEnable(GL_SCISSOR_TEST);

	glViewport( wx, wy, ww, wh );
	glScissor(wx, wy, ww, wh);
	glClearColor(  (float)clearR/255.0f, (float)clearG/255.0f, (float)clearB/255.0f, 0);  
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if ( drawBackImgFlag && jpgImgData.data != 0  )
	{
		glDisable(GL_DEPTH_TEST);

		glRasterPos2f( (float)backImgOffsetX, (float)backImgOffsetY );
		glBitmap (0, 0, 0, 0, (float)(-jpgImgData.w/2), (float)(jpgImgData.h/2), NULL);
		glPixelZoom((float)backImgScaleW, -(float)backImgScaleH);
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glDrawPixels( jpgImgData.w, jpgImgData.h, GL_RGB, GL_UNSIGNED_BYTE, jpgImgData.data );
		glPixelZoom(1.0,  1.0);

		glEnable(GL_DEPTH_TEST);
	}


	glDisable(GL_SCISSOR_TEST);

	glMatrixMode( GL_PROJECTION );
    glLoadIdentity(); 

	glOrtho( orthoL, orthoR, orthoT, orthoB, -10.0, 10.0); 

	glMatrixMode( GL_MODELVIEW );
		
	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 		
    glPushMatrix(); 
	glLoadIdentity(); 

	if ( axisDrawFlag )
		DrawXYZArrows( currTrack );

	currTrack.transform();


	glTranslatef( (float)cx, (float)cy, (float)cz );


}

void VirtGlWindow::postdraw()
{
   glPopMatrix();

}

void VirtGlWindow::draw( Aircraft* airPtr )
{
	aircraftPtr = airPtr;

	if ( !showFlag ) 
		return;

	predraw();

	airPtr->draw();
	if (focusFlag)
	{
		glDisable(GL_DEPTH_TEST);
		airPtr->drawHighlight();
		airPtr->draw2D();
		glEnable(GL_DEPTH_TEST);
	}

	postdraw();

 
	glDisable(GL_DEPTH_TEST);

	glLineWidth(2.0);

	if ( activeFlag )
		glColor3d( 1.0, 0.2, 0.2 );
	else
		glColor3d( 0.2, 0.2, 0.2 );

	glBegin( GL_LINE_STRIP );
		glVertex2d( orthoL, orthoB );
		glVertex2d( orthoL, orthoT );
		glVertex2d( orthoR, orthoT );
		glVertex2d( orthoR, orthoB );
		glVertex2d( orthoL, orthoB );
	glEnd();
	glEnable(GL_DEPTH_TEST);
}

int VirtGlWindow::mousePress( int mx, int my )
{

	if ( !showFlag ) 
		return 0;

	if (focusFlag)
		aircraftPtr->mouseClick((float)mx, (float)my);

	if ( Fl::event_button() == FL_BUTTON1 )
		mouseButton1 = 1;
	if ( Fl::event_button() == FL_BUTTON2 )
		mouseButton2 = 1;
	if ( Fl::event_button() == FL_BUTTON3 )
		mouseButton3 = 1;


	if ( mx >= wx && mx <= wx+ww && my >= wy && my < wy+wh )
	{
		if ( fastDrawFlag && aircraftPtr )
			aircraftPtr->fastDrawFlagOn();

		currTrack.click( mx-wx, my-wy );
		activeFlag = 1;

		return 1;
	}
	return 0;
}

void VirtGlWindow::mouseRelease( int mx, int my )
{
	if ( !showFlag ) 
		return;

	if ( Fl::event_button() == FL_BUTTON1 )
		mouseButton1 = 0;
	if ( Fl::event_button() == FL_BUTTON2 )
		mouseButton2 = 0;
	if ( Fl::event_button() == FL_BUTTON3 )
		mouseButton3 = 0;


	if ( fastDrawFlag &&  aircraftPtr )
		aircraftPtr->fastDrawFlagOff();

	currTrack.poll( STOP,  mx-wx, my-wy );
}

void VirtGlWindow::mouseDrag( int mx, int my )
{
//	mouseMove(mx,my);

	if ( !showFlag ) 
		return;

	if ( (Fl::event_state(FL_BUTTON1) && Fl::event_button3() ) ||
             (Fl::event_state(FL_BUTTON1) && Fl::event_ctrl() ) ||
             (Fl::event_state(FL_BUTTON1) && Fl::event_state(FL_META) ) ||
              Fl::event_state(FL_BUTTON2) )
		currTrack.poll( SCALE,  mx-wx, my-wy );
	else if ( Fl::event_state(FL_BUTTON3 ) ||
                 (Fl::event_state(FL_BUTTON1) && Fl::event_alt()))
		currTrack.poll( TRANSLATE,  mx-wx, my-wy );
	else if ( Fl::event_state(FL_BUTTON1 ) )
		currTrack.poll( ROTATE,  mx-wx, my-wy );

}

void VirtGlWindow::mouseMove( int mx, int my )
{
	mousex = mx % ww;
	mousey = my % wh;

	double minwin = (double) min(ww, wh);
	cursorx = (float)(mousex-ww*0.5)/(float)minwin;
	cursory = (float)(mousey-wh*0.5)/(float)minwin;

/*
	//=== Set Window Focus ===//
	for ( int i = 0 ; i < NUM_V_WIN ; i++ )
	{
		vWin[i]->setFocus(0);
	}

	if ( currLayout == VSP_ONE_WIN )
	{
		vWin[ONE_LARGE]->setFocus(1);
	}
	else if ( currLayout == VSP_FOUR_WIN )
	{

		if (cursory > 0)
		{
			if (cursorx < 0)
				vWin[FOUR_UP_L]->setFocus(1);
			else
				vWin[FOUR_UP_R]->setFocus(1);
		}
		else
		{
			if (cursorx < 0)
				vWin[FOUR_LOW_L]->setFocus(1);
			else
				vWin[FOUR_LOW_R]->setFocus(1);
		}
	}
	else if ( currLayout == VSP_TWO_HORZ_WIN )
	{
		if (cursorx < 0)
			vWin[TWO_L]->setFocus(1);
		else
			vWin[TWO_R]->setFocus(1);
	}
	else if ( currLayout == VSP_TWO_VERT_WIN )
	{
		if (cursory > 0)
			vWin[TWO_UP]->setFocus(1);
		else
			vWin[TWO_LOW]->setFocus(1);
	}
*/
}


void VirtGlWindow::saveTrack(int i)
{
	if ( i >= 0 && i < NUM_USER_VIEWS )
	{
		savedUserViewFlag[i] = 1;
		savedRotationCenters[i] = vec3d( cx, cy, cz );
		savedTrackVec[i] = currTrack;
	}
}

void VirtGlWindow::loadTrack(int i)
{
	if ( i >= 0 && i < NUM_USER_VIEWS )
	{
		if ( savedUserViewFlag[i] )
		{
			vec3d c = savedRotationCenters[i];
			cx = c.x(); cy = c.y(); cz = c.z();
			currTrack = savedTrackVec[i];
		}
	}
}

void VirtGlWindow::setFocus(int focus)
{ 
	focusFlag = focus; 
}

void VirtGlWindow::DrawXYZArrows( track_ball& curr_track )
{
	if( !font )
		font = fontMgr->loadFont( );


	track_ball rot_track = curr_track;

	double scale = rot_track.get_scale();

	rot_track.set_trans(0,0,0);
	rot_track.set_scale(1.0);

	double size = 0.08;
	double ls = 0.01;
	double off = 0.01;

	glPushMatrix();
	glTranslated( orthoL + 0.1, orthoT + 0.1, 0.0 );
	rot_track.transform();

	GLdouble modelview[16];
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	Matrix4d mat;
	mat.initMat( modelview );
	vec3d xv = vec3d( size + off, 0, 0);
	vec3d yv = vec3d( 0, size + off, 0);
	vec3d zv = vec3d( 0, 0, size + off);
	vec3d xproj = mat.xform( xv );
	vec3d yproj = mat.xform( yv );
	vec3d zproj = mat.xform( zv );

	//==== Axis ===//
	glLineWidth( 4.0 );
	glBegin( GL_LINES );
		glColor3ub( 255, 0, 0 );
		glVertex3d( 0.0, 0.0, 0.0 );
		glVertex3d( size, 0.0, 0.0 );

		glColor3ub( 0, 255, 0 );
		glVertex3d( 0.0, 0.0, 0.0 );
		glVertex3d( 0.0, size, 0.0 );

		glColor3ub( 0, 0, 255 );
		glVertex3d( 0.0, 0.0, 0.0 );
		glVertex3d( 0.0, 0.0, size );
	glEnd();

	//==== Text ===//
	glLoadIdentity(); 
	glTranslated( xproj.x(), xproj.y(), xproj.z() );
	glColor3ub( 50, 50, 50 );

	float fscale = 1.1;

	pair< int, int > dimension;
	font->GetStringSize( "X", &dimension);
	float w = ((float) dimension.first) * FONT_BASE_SCALE * fscale;
	float h = ((float) dimension.second) * FONT_BASE_SCALE * fscale;

	glEnable(GL_TEXTURE_2D);
	font->Begin();
	font->DrawString( "X", (float)( FONT_BASE_SCALE * fscale ), (float) 0.0, (float) 0.0 );
	glDisable(GL_TEXTURE_2D);

	glLoadIdentity(); 
	glTranslated( yproj.x(), yproj.y(), yproj.z() );

	glEnable(GL_TEXTURE_2D);
	font->Begin();
	font->DrawString( "Y", (float)( FONT_BASE_SCALE * fscale ), (float) 0.0 - w/2, (float) 0.0 + h  );
	glDisable(GL_TEXTURE_2D);

	glLoadIdentity(); 
	glTranslated( zproj.x(), zproj.y(), zproj.z() );

	glEnable(GL_TEXTURE_2D);
	font->Begin();
	font->DrawString( "Z", (float)( FONT_BASE_SCALE * fscale ), (float) 0.0, (float) 0.0 );
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VspGlWindow::VspGlWindow(int x, int y, int w, int h) :
Fl_Gl_Window(x,y,w,h,"Vsp GL Window") 
{

	end();					// end for Fl_Gl_Window
 
	vWin[ONE_LARGE]  = new VirtGlWindow( 0,     0,     w, h );			// Top
	vWin[FOUR_UP_L]  = new VirtGlWindow( 0,       h/2, w/2, h/2 );
	vWin[FOUR_UP_R]  = new VirtGlWindow( w/2,   h/2, w/2, h/2 );
	vWin[FOUR_LOW_L] = new VirtGlWindow( 0,           0, w/2, h/2 );
	vWin[FOUR_LOW_R] = new VirtGlWindow( w/2,       0, w/2, h/2  );
	vWin[TWO_L]      = new VirtGlWindow( 0,   0,   w/2, h  );
	vWin[TWO_R]      = new VirtGlWindow( w/2, 0,   w/2, h  );
	vWin[TWO_UP]     = new VirtGlWindow( 0,   0,   w,   h/2  );
	vWin[TWO_LOW]    = new VirtGlWindow( 0,   h/2, w,   h/2  );

	//==== Set Up Initial Viewing Angles ====//
	//vWin[FOUR_LOW_L]->currTrack.set_init_angle(PI/2.0, 0.0, 0.0);
	vWin[FOUR_LOW_L]->currTrack.set_init_angle(PI/2.0, -PI/2.0, 0.0);
	vWin[FOUR_LOW_L]->currTrack.init();

	//vWin[FOUR_UP_R]->currTrack.set_init_angle(PI/2.0, -PI/2.0, 0.0);
	vWin[FOUR_UP_R]->currTrack.set_init_angle(PI/4.0, -PI/6.0, -PI/6.0);
	vWin[FOUR_UP_R]->currTrack.init();

	//vWin[FOUR_LOW_R]->currTrack.set_init_angle(PI/4.0, -PI/6.0, -PI/6.0);
	vWin[FOUR_LOW_R]->currTrack.set_init_angle(PI/2.0, 0.0, 0.0);
	vWin[FOUR_LOW_R]->currTrack.init();
	
	//vWin[FOUR_UP_L]->currTrack.set_init_angle(0.0, 0.0, 0.0);
	vWin[FOUR_UP_L]->currTrack.set_init_angle(0.0, 0.0, -PI/2.0);
	vWin[FOUR_UP_L]->currTrack.init();

	vWin[TWO_L]->currTrack.set_init_angle(0.0, 0.0, PI/2.0);
	vWin[TWO_L]->currTrack.init();
	
	vWin[TWO_R]->currTrack.set_init_angle(PI/2.0, 0.0, PI/2.0);
	vWin[TWO_R]->currTrack.init();

	vWin[TWO_UP]->currTrack.set_init_angle(PI/2.0, 0.0, 0.0);
	vWin[TWO_UP]->currTrack.init();
	
	vWin[TWO_LOW]->currTrack.set_init_angle(0.0, 0.0, 0.0);
	vWin[TWO_LOW]->currTrack.init();

	setWindowLayout(VSP_ONE_WIN);
	vWin[ONE_LARGE]->activate();
	currVWin = vWin[ONE_LARGE];

	clearFlag = 0;
	focusFlag = 0;
	aircraftPtr = 0;

//	mouseTrack = 0;
	mouseTrack = 1;

	cursorIcon = FL_CURSOR_DEFAULT;

	triggerScreenGrabFlag = false;
	screenGrabFileName = Stringc("grab.jpg");

}

VspGlWindow::~VspGlWindow()
{

}

void VspGlWindow::init()
{
	static int count = 0;

	lightFlags[0] = 1;
	lightFlags[1] = 1;
	lightFlags[2] = 0;
	lightFlags[3] = 0;
	lightFlags[4] = 0;
	lightFlags[5] = 0;

	lightAmb  = 0.5;
	lightDiff = 0.5;
	lightSpec = 0.5;

	// For some reason on Linux this cores on the first call,
        // but not the second.
	if ( ++count > 1 )
	{
		defineLights();
		glEnable( GL_NORMALIZE );
		glEnable( GL_LINE_SMOOTH );
		glEnable(GL_POINT_SMOOTH);
	}
}

void VspGlWindow::writeFile(xmlNodePtr root)
{
	xmlNodePtr vertwin_list_node = xmlNewChild( root, NULL, (const xmlChar *)"VirtWindow_List", NULL );
	for ( int i = 0 ; i < NUM_V_WIN ; i++ )
	{
		xmlNodePtr vertwin_node = xmlNewChild( vertwin_list_node, NULL, (const xmlChar *)"VirtWindow", NULL );
		vWin[i]->writeFile( vertwin_node );

		//xmlAddDoubleNode( vertwin_node, "Back_Img_Scale_W", vWin[i]->backImgScaleW );
		//xmlAddDoubleNode( vertwin_node, "Back_Img_Scale_H", vWin[i]->backImgScaleH );
		//xmlAddDoubleNode( vertwin_node, "Back_Img_Offset_X", vWin[i]->backImgOffsetX );
		//xmlAddDoubleNode( vertwin_node, "Back_Img_Offset_Y", vWin[i]->backImgOffsetY );	
	}
}

void VspGlWindow::readFile(xmlNodePtr root)
{
	xmlNodePtr node_list = xmlGetNode( root, "VirtWindow_List", 0 );
	if ( node_list  )
	{
		int num_vwin = xmlGetNumNames( node_list, "VirtWindow" );
		for ( int i = 0 ; i < num_vwin ; i++ )
		{
			xmlNodePtr vwin_node = xmlGetNode( node_list, "VirtWindow", i );
			if ( vwin_node && i < NUM_V_WIN )
			{
				vWin[i]->readFile( vwin_node );
				//vWin[i]->backImgScaleW = xmlFindDouble( vwin_node, "Back_Img_Scale_W", 1.0 );
				//vWin[i]->backImgScaleH = xmlFindDouble( vwin_node, "Back_Img_Scale_H", 1.0 );
				//vWin[i]->backImgOffsetX = xmlFindDouble( vwin_node, "Back_Img_Offset_X", 0.0 );
				//vWin[i]->backImgOffsetY = xmlFindDouble( vwin_node, "Back_Img_Offset_Y", 0.0 );
			}
		}
	}
}

void VspGlWindow::defineLights()
{
	ambient[0] = ambient[1] = ambient[2]    = (float)lightAmb;	    ambient[3] = 1.0f;
	diffuse[0] = diffuse[1] = diffuse[2]    = (float)lightDiff;	diffuse[3] = 1.0f;
	specular[0] = specular[1] = specular[2] = (float)lightSpec;	specular[3] = 1.0f;

    static GLfloat position0[] = { 10.0f, 0.0f, 0.0f, 1.0f  };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, position0);

    static GLfloat position1[] = { 0.0f, 10.0f, 0.0f, 1.0f  };
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT1, GL_POSITION, position1);

    static GLfloat position2[] = { 0.0f, 0.0f, 10.0f, 1.0f  };
    glLightfv(GL_LIGHT2, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT2, GL_POSITION, position2);

    static GLfloat position3[] = {  10.0f, 0.0f, 10.0f, 1.0f  };
    glLightfv(GL_LIGHT3, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT3, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT3, GL_POSITION, position3);

    static GLfloat position4[] = {  0.0f, -10.0f, 10.0f, 1.0f  };
    glLightfv(GL_LIGHT4, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT4, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT4, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT4, GL_POSITION, position4);

    static GLfloat position5[] = { -10.0f, -10.0f, -10.0f, 10.0f  };
    glLightfv(GL_LIGHT5, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT5, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT5, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT5, GL_POSITION, position5);

	if ( lightFlags[0] )	glEnable( GL_LIGHT0 );
	else					glDisable( GL_LIGHT0 );

	if ( lightFlags[1] )	glEnable( GL_LIGHT1 );
	else					glDisable( GL_LIGHT1 );

	if ( lightFlags[2] )	glEnable( GL_LIGHT2 );
	else					glDisable( GL_LIGHT2 );

	if ( lightFlags[3] )	glEnable( GL_LIGHT3 );
	else					glDisable( GL_LIGHT3 );

	if ( lightFlags[4] )	glEnable( GL_LIGHT4 );
	else					glDisable( GL_LIGHT4 );

	if ( lightFlags[5] )	glEnable( GL_LIGHT5 );
	else					glDisable( GL_LIGHT5 );


}

void VspGlWindow::setLightFlag( int id, int flag )
{
	if ( id >= 0 && id < VSP_NUM_LIGHTS )
		lightFlags[id] = flag;

	defineLights();
}

int  VspGlWindow::getLightFlag( int id )
{
	if ( id >= 0 && id < VSP_NUM_LIGHTS )
		return lightFlags[id];

	return 0;
}

void VspGlWindow::setLightAmb( double val )
{
	lightAmb = val;
	defineLights();
}

void VspGlWindow::setLightDiff( double val )
{
	lightDiff = val;
	defineLights();
}

void VspGlWindow::setLightSpec( double val )
{
	lightSpec = val;
	defineLights();
}


void VspGlWindow::resize( int x, int y, int w, int h )
{
	Fl_Gl_Window::resize( x, y, w, h );
	setWindowLayout(currLayout);

}

void VspGlWindow::draw()
{
	int i;

	if ( clearFlag < 2 )
	{
		glClearColor(0.95f,0.95f,0.95f,0);  
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		clearFlag++;
	}


	static int once = 0;
	if ( !once )
	{
		init();
		once = 1;
	}

	//==== Draw Deactive Window First ====//
	for ( i = 0 ; i < NUM_V_WIN ; i++ )
	{
		if ( !vWin[i]->active() )
			vWin[i]->draw( aircraftPtr );
	}

	//==== Draw Active with Hightlight ====//
	for ( i = 0 ; i < NUM_V_WIN ; i++ )
	{
		if ( vWin[i]->active() )
			vWin[i]->draw( aircraftPtr );
	}

	if ( triggerScreenGrabFlag )
	{
		screenGrab();
		triggerScreenGrabFlag = false;
	}

}

int VspGlWindow::handle(int event) 
{
	int mx = Fl::event_x();
	int my = h() - Fl::event_y();
	
	switch(event) 
	{
		case FL_ENTER:
			focusFlag = 1;

			Fl::focus( this );
			return Fl_Gl_Window::handle(event);			// Let Base Widget process all others

		break;

		case FL_LEAVE:
			focusFlag = 0;
			cursor(FL_CURSOR_DEFAULT);
			return Fl_Gl_Window::handle(event);			// Let Base Widget process all others

		break;

		case FL_PUSH:
			{
				for ( int i = 0 ; i < NUM_V_WIN ; i++ )
				{
					vWin[i]->deactivate();
					if ( vWin[i]->mousePress( mx, my ) )
						currVWin = vWin[i];

					aircraftPtr->getScreenMgr()->updateBackgroundScreen();

				}
					
				Fl::focus( this );
						
				redraw();
				return 1;
			}

		case FL_RELEASE:
			{
				if ( currVWin )
				{
					currVWin->mouseRelease( mx, my );
				}
				redraw();
				return 1;
			}

		case FL_MOVE:
			{
				if ( currVWin )
				{
					currVWin->mouseMove( mx, my );
					if (mouseTrack) 
					{
						setWindowFocus(mx, my);
						redraw();
					}
				}
				return 1;
			}

		case FL_DRAG:
			{
				if ( currVWin )
				{
					currVWin->mouseDrag( mx, my );
				}
				redraw();
				return 1;
			}

		case FL_KEYDOWN:
			{
				return processKeyEvent();
			}

		case FL_FOCUS :
		case FL_UNFOCUS :
 			return 1;

		default:
			return Fl_Gl_Window::handle(event);	// Let Base Widget process all others

	}
	return Fl_Gl_Window::handle(event);			// Let Base Widget process all others

}
	
int VspGlWindow::processKeyEvent()
{
	int key = Fl::event_key();

	if ( !currVWin )
		return 0;

	if ( key >= (FL_F + 5) && key <= (FL_F + 12) )
	{
		setView( VSP_TOP_VIEW + key - FL_F - 5 );
		return 1;
	}

	if ( Fl::event_state(FL_SHIFT) && key >= (FL_F + 1) && key <= (FL_F + 4) )
	{	
		if ( currVWin )
		{
			currVWin->saveTrack( key - FL_F - 1 );
			return 1;
		}
	}
	else if ( key >= (FL_F + 1) && key <= (FL_F + 4) )
	{	
		if ( currVWin )
		{
			currVWin->loadTrack( key - FL_F - 1 );
			redraw();
			return 1;
		}
	}
	else if ( key == 99 )		// "c" key
	{
		setView( VSP_CENTER_VIEW );
	}
	else if ( key == 114 )		// "r" key
	{
		moveViewCenter();
	}
	else if ( key == 102 )		// "f" key	fastDraw
	{
		toggleFastDraw();
	}


//printf("key = %d\n", key );

	return 0;
}


void VspGlWindow::setWindowLayout( int layout )
{
	clearFlag = 0;

	vWin[ONE_LARGE]->resize(  0,       0,       w(),   h() );			// Top
	vWin[FOUR_UP_L]->resize(  0,       h()/2,   w()/2, h()/2 );
	vWin[FOUR_UP_R]->resize(  w()/2,   h()/2,   w()/2, h()/2 );
	vWin[FOUR_LOW_L]->resize( 0,       0,       w()/2, h()/2 );
	vWin[FOUR_LOW_R]->resize( w()/2,   0,       w()/2, h()/2  );
	vWin[TWO_L]->resize(      0,       0,       w()/2, h()  );
	vWin[TWO_R]->resize(      w()/2,   0,       w()/2, h()  );
	vWin[TWO_UP]->resize(     0,       0,       w(),   h()/2  );
	vWin[TWO_LOW]->resize(    0,       h()/2,   w(),   h()/2  );

	currLayout = layout;

	for ( int i = 0 ; i < NUM_V_WIN ; i++ )
	{
		vWin[i]->hide();
	}

	if ( currLayout == VSP_ONE_WIN )
	{
		vWin[ONE_LARGE]->show();
		currVWin = vWin[ONE_LARGE];
	}
	else if ( currLayout == VSP_FOUR_WIN )
	{
		vWin[FOUR_UP_L]->show();
		vWin[FOUR_UP_R]->show();
		vWin[FOUR_LOW_L]->show();
		vWin[FOUR_LOW_R]->show();
		currVWin = vWin[FOUR_UP_L];
	}
	else if ( currLayout == VSP_TWO_HORZ_WIN )
	{
		vWin[TWO_L]->show();
		vWin[TWO_R]->show();
		currVWin = vWin[TWO_L];
	}
	else if ( currLayout == VSP_TWO_VERT_WIN )
	{
		vWin[TWO_UP]->show();
		vWin[TWO_LOW]->show();
		currVWin = vWin[TWO_UP];
	}

}



void VspGlWindow::setView( int view )
{
	if ( currVWin )
	{
		if ( view == VSP_TOP_VIEW )
		{
			currVWin->currTrack.set_init_angle(0.0, 0.0, 0.0);
			currVWin->currTrack.init();
		}
		else if ( view == VSP_FRONT_VIEW )
		{
			currVWin->currTrack.set_init_angle(PI/2.0, -PI/2.0, 0.0);
			currVWin->currTrack.init();
		}
		else if ( view == VSP_LEFT_SIDE_VIEW )
		{
			currVWin->currTrack.set_init_angle(PI/2.0, 0.0, 0.0);
			currVWin->currTrack.init();
		}
		else if ( view == VSP_LEFT_ISO_VIEW )
		{
			currVWin->currTrack.set_init_angle(PI/4.0, -PI/6.0, -PI/6.0);
			currVWin->currTrack.init();
		}
		else if ( view == VSP_BOTTOM_VIEW )
		{
			currVWin->currTrack.set_init_angle(PI, 0.0, 0.0 );
			currVWin->currTrack.init();
		}
		else if ( view == VSP_BACK_VIEW )
		{
			currVWin->currTrack.set_init_angle(PI/2.0, PI/2.0, 0.0);
			currVWin->currTrack.init();
		}
		else if ( view == VSP_RIGHT_SIDE_VIEW )
		{
			currVWin->currTrack.set_init_angle(-PI/2.0, 0.0, PI);
			currVWin->currTrack.init();
		}
		else if ( view == VSP_RIGHT_ISO_VIEW )
		{
			currVWin->currTrack.set_init_angle(-PI/4.0, -PI/6.0, -PI + PI/6.0);
			currVWin->currTrack.init();
		}
		else if ( view == VSP_CENTER_VIEW )
		{	
			aircraftPtr->update_bbox();	
			bbox box = aircraftPtr->getBndBox();

			double scale = box.get_largest_dim();
			if ( scale > 0.0000001 )
				currVWin->setScale( 1.0/scale );

			currVWin->setCenter( -(box.get_max(0) + box.get_min(0))/2.0,
								 -(box.get_max(1) + box.get_min(1))/2.0,
								 -(box.get_max(2) + box.get_min(2))/2.0 );

			currVWin->currTrack.zero_trans();
		}			
		redraw();
	}
	 

}

void VspGlWindow::moveViewCenter()
{
	vec2d cursor = currVWin->getCursor();

	//==== Save Current Rotation Center ====//
	double cx = currVWin->getCenterRotX();
	double cy = currVWin->getCenterRotY();
	double cz = currVWin->getCenterRotZ();
	vec3d close_pnt( cx, cy, cz );

	double close_dist = 1.0e12;

	//==== Look Through All Transformed Vertices to Find Closest One ====//
	vector< Geom* > gvec = aircraftPtr->getGeomVec();
	for (int g = 0 ; g < (int)gvec.size() ; g++ )
	{
		if ( !gvec[g]->getNoShowFlag() )
		{
			vector< VertexID > verts;
			gvec[g]->getVertexVec(&verts);
			for (int v = 0; v < (int)verts.size(); v++)
			{
				vec2d pos2d = verts[v].pos2d();
				double d = dist( cursor, pos2d );
				if ( d < close_dist )
				{
					close_dist = d;
					close_pnt = verts[v].posXform();
				}
			}
		}
	}

	//===== Reset View Center ===//
	currVWin->setCenter( -close_pnt.x(), -close_pnt.y(), -close_pnt.z() );
	currVWin->currTrack.set_trans( 0, 0, 0 );

	redraw();
}


void VspGlWindow::centerAllViews()
{
	aircraftPtr->update_bbox();	
	bbox box = aircraftPtr->getBndBox();


	for ( int i = 0 ; i < NUM_V_WIN ; i++ )
	{
		double scale = box.get_largest_dim();
		if ( scale > 0.0000001 )
			vWin[i]->setScale( 1.0/scale );

		vWin[i]->setCenter( -(box.get_max(0) + box.get_min(0))/2.0,
							 -(box.get_max(1) + box.get_min(1))/2.0,
							 -(box.get_max(2) + box.get_min(2))/2.0 );

		vWin[i]->currTrack.zero_trans();
	}
}

void VspGlWindow::toggleFastDraw()
{
	for ( int i = 0 ; i < NUM_V_WIN ; i++ )
	{
		vWin[i]->toggleFastDraw();
	}
}

void VspGlWindow::setMouseTracking( int track )
{ 
	mouseTrack = track; 

	redraw();
}


void VspGlWindow::setWindowFocus(int mx, int my)
{
	cursor(FL_CURSOR_CROSS); // not quite sure how this works

//	printf("win %d, %d, %d, %d\n", x(), y(), w(), h());
//	double qx = (mx - x()) - w()/2.0;
//	double qy = (my - y()) - h()/2.0;
	double qx = mx - w()/2.0;
	double qy = my - h()/2.0;

	//=== Set Window Focus ===//
	for ( int i = 0 ; i < NUM_V_WIN ; i++ )
	{
		vWin[i]->setFocus(0);
	}

	if ( currLayout == VSP_ONE_WIN )
	{
		vWin[ONE_LARGE]->setFocus(1);
	}
	else if ( currLayout == VSP_FOUR_WIN )
	{

		if (qy > 0)
		{
			if (qx < 0)
				vWin[FOUR_UP_L]->setFocus(1);
			else
				vWin[FOUR_UP_R]->setFocus(1);
		}
		else
		{
			if (qx < 0)
				vWin[FOUR_LOW_L]->setFocus(1);
			else
				vWin[FOUR_LOW_R]->setFocus(1);
		}
	}
	else if ( currLayout == VSP_TWO_HORZ_WIN )
	{
		if (qx < 0)
			vWin[TWO_L]->setFocus(1);
		else
			vWin[TWO_R]->setFocus(1);
	}
	else if ( currLayout == VSP_TWO_VERT_WIN )
	{
		if (qy > 0)
			vWin[TWO_UP]->setFocus(1);
		else
			vWin[TWO_LOW]->setFocus(1);
	}

}

void VspGlWindow::setCursor(Fl_Cursor c)
{
	if (cursorIcon != c)
	{
		cursor(c);
		cursorIcon = c;
	}
}

void VspGlWindow::grabWriteScreen( const char* filename )
{
	triggerScreenGrabFlag = true;
	screenGrabFileName = Stringc(filename);
}

void VspGlWindow::screenGrab()
{
	int nStartX = 0;
	int nStartY = 0;
	int nWidth = w(); 
	int nHeight = h();
	int nPixW = nWidth  - nStartX;
	int nPixH = nHeight - nStartY;

	unsigned char *pRGB = new unsigned char [3 * (nPixW+1) * (nPixH+1) ];

	glReadBuffer( GL_BACK );   
	glReadPixels( nStartX, nStartY, nPixW, nPixH, GL_RGB, GL_UNSIGNED_BYTE, pRGB);

	unsigned char *pRGBFlip = new unsigned char [3 * (nPixW+1) * (nPixH+1) ];

	int scanLen = 3 * (nPixW+1);

	for ( int i = 0 ; i < nPixH+1 ; i++ )
	{
		unsigned char* srcLine = &pRGB[i*scanLen];
		unsigned char* dstLine = &pRGBFlip[(nPixH - i)*scanLen];

		memcpy(  dstLine, srcLine, scanLen );
	}
	delete pRGB;

	writeJPEG( screenGrabFileName.get_char_star(), nPixW+1, nPixH+1, pRGBFlip ); 

	delete pRGBFlip;

}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//					Cross Section GL Window
//////////////////////////////////////////////////////////////////////
XSecGlWindow::XSecGlWindow(int xi, int yi, int wi, int hi) :
Fl_Gl_Window(x,y,w,h,"Vsp XSec Window") 
{
	end();					// end for Fl_Gl_Window

	focusFlag = 0;
	drawBasePtr = 0;
	drawBackImgFlag = 0;
	jpgImgData.data = 0;

	resize( xi, yi, wi, hi  );

}

XSecGlWindow::~XSecGlWindow()
{

}

void XSecGlWindow::init()
{
 
}
void XSecGlWindow::setDrawBase( DrawBase* ptr )	
{	
	drawBasePtr = ptr; 

	if ( drawBasePtr )
	{
		drawBasePtr->setGlWinOrthoLRTB( orthoL, orthoR, orthoT, orthoB );
		drawBasePtr->setGlWinWidthHeight( w, h );
	}

}

void XSecGlWindow::resize( int xi, int yi, int wi, int hi )
{
	x = xi;		y = yi;
	w = wi;		h = hi;

	Fl_Gl_Window::resize( x, y, w, h );

	if ( w < h )
	{
		orthoL = -0.5;	
		orthoR = -orthoL;
		orthoT = -0.5*(double)h/(double)w;
		orthoB = -orthoT;
	}
	else
	{
		orthoL = -0.5*(double)w/(double)h;	
		orthoR = -orthoL;
		orthoT = -0.5;
		orthoB = -orthoT;
	}

	if ( drawBasePtr )
	{
		drawBasePtr->setGlWinOrthoLRTB( orthoL, orthoR, orthoT, orthoB );
		drawBasePtr->setGlWinWidthHeight( w, h );
	}


}
int XSecGlWindow::handle(int event) 
{

	switch(event) 
	{
		case FL_ENTER:

			focusFlag = 1;
			Fl::focus( this );
			return Fl_Gl_Window::handle(event);			// Let Base Widget process all others

		break;

		case FL_LEAVE:

			focusFlag = 0;
			return Fl_Gl_Window::handle(event);			// Let Base Widget process all others

		break;

		case FL_PUSH:
			{
				Fl::focus( this );
				
				if ( drawBasePtr )
					if (  drawBasePtr->processPushEvent() )
						redraw();
				return 1;
			}

		case FL_MOVE:
			{						
				if ( drawBasePtr )
					if (  drawBasePtr->processMoveEvent() )
						redraw();
				return 1;
			}


		case FL_RELEASE:
			{
				if ( drawBasePtr )
					if (  drawBasePtr->processReleaseEvent() )
						redraw();
				return 1;
			}

		case FL_DRAG:
			{
				if ( drawBasePtr )
					if (  drawBasePtr->processDragEvent() )
						redraw();
				return 1;
			}

		case FL_KEYDOWN:
			{
				if ( drawBasePtr )
					if (  drawBasePtr->processKeyEvent() )
						redraw();

				return 1;
			}

		case FL_FOCUS :
		case FL_UNFOCUS :
 			return 1;

		default:
			return Fl_Gl_Window::handle(event);	// Let Base Widget process all others

	}
	return Fl_Gl_Window::handle(event);			// Let Base Widget process all others

}


void XSecGlWindow::draw()
{
	glViewport( 0, 0, w, h );
	glMatrixMode( GL_PROJECTION );
    glLoadIdentity(); 

	glOrtho( orthoL, orthoR, orthoT, orthoB, -1.0, 1.0); 

	glMatrixMode( GL_MODELVIEW );
		
	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 		
    glPushMatrix(); 
	glLoadIdentity(); 

	glClearColor(0.95f, 0.95f, 0.95f, 0);  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);

	if ( drawBackImgFlag && jpgImgData.data != 0 )
	{
		glRasterPos2f(  0.0,0.0);
		glBitmap (0, 0, 0, 0, (float)(-jpgImgData.w/2), (float)(jpgImgData.h/2), NULL);
		glPixelZoom(1.0, -1.0);
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glDrawPixels( jpgImgData.w, jpgImgData.h, GL_RGB, GL_UNSIGNED_BYTE, jpgImgData.data );
		glPixelZoom(1.0,  1.0);
	}
// jrg remove?
//	double sf = (orthoL - orthoR)/(orthoT - orthoB);
//	glScalef( (float)sf, (float)sf, 1.0 );

	if ( drawBasePtr )
		drawBasePtr->draw();


	glEnable(GL_DEPTH_TEST);

	glPopMatrix();


}








