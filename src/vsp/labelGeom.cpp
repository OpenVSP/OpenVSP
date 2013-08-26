//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "labelGeom.h"
#include "screenMgr.h"
#include "aircraft.h"


LabelGeom::LabelGeom() 
{
	drawMode = DRAW_HIGHLIGHT;
	textSize = 1.5f;
	viewScale = 1.0;
	cursor = vec2d(0,0);

}

void LabelGeom::writeLabelGeom(xmlNodePtr root)
{
  xmlNodePtr label_node = xmlNewChild( root, NULL, (const xmlChar *)"Label_Parms", NULL );
//  xmlAddIntNode( label_node, "TypeInt", type );
//  xmlAddStringNode( label_node, "TypeStr", type_str.get_char_star());
  xmlAddStringNode( label_node, "NameStr", name_str.get_char_star());

  xmlAddDoubleNode( label_node, "ColorR", color.x() );
  xmlAddDoubleNode( label_node, "ColorG", color.y() );
  xmlAddDoubleNode( label_node, "ColorB", color.z() );

  xmlAddIntNode( label_node, "DrawMode", drawMode );
  xmlAddDoubleNode( label_node, "TextSize", textSize );
  xmlAddDoubleNode( label_node, "ViewScale", viewScale );
}

void LabelGeom::readLabelGeom(xmlNodePtr root)
{
	xmlNodePtr label_node;

	//===== Read Parameters =====//
	label_node = xmlGetNode( root, "Label_Parms", 0 );
	if ( label_node )
	{
//		type = xmlFindInt( label_node, "TypeInt", type );
//		type_str = xmlFindString( label_node, "TypeStr", type_str.get_char_star());
		name_str = xmlFindString( label_node, "NameStr", name_str.get_char_star());

		color.set_x( xmlFindDouble( label_node, "ColorR", color.x() ) );
		color.set_y( xmlFindDouble( label_node, "ColorG", color.y() ) );
		color.set_z( xmlFindDouble( label_node, "ColorB", color.z() ) );

		drawMode = xmlFindInt( label_node, "DrawMode", drawMode );
		textSize = (float)xmlFindDouble( label_node, "TextSize", textSize );
		viewScale = xmlFindDouble( label_node, "ViewScale", viewScale );
	}
}

TextLabel::TextLabel() : LabelGeom()
{
	type = TEXT_LABEL;
	type_str = "text";

	char name[255];
	sprintf( name, "Text_%d", ++labelCnt ); 
	name_str = Stringc(name);

	color = vec3d( 100,100,100 );
	font = fontMgr->loadFont( );

	textOffset = 0.6f;

	fixedLabelFlag = false;
	fixedPos = vec2d(0, 0);


}
void TextLabel::reset()
{
	vertex1.reset();
}

vec2d TextLabel::drawString(GLFont * glfont, float scale, Stringc str, float x0, float y0, float xoffset, float yoffset) 
{

	double w = 0; 
	double h = 0;
	if (str.get_length() > 0) {

		//alignment 
		pair< int, int > dimension;
		glfont->GetStringSize(str.get_char_star(), &dimension);
		w = ((float) dimension.first) * FONT_BASE_SCALE * scale;		
		h = ((float) dimension.second) * FONT_BASE_SCALE * scale;
		double x = x0 - w/2  + w*xoffset;
		double y = y0 + h/2 + h*yoffset;

		glfont->Begin();
		glfont->DrawString(str.get_char_star(), (float)(FONT_BASE_SCALE * scale), (float)x, (float)y);
	}
	return vec2d(w,h);
}

void TextLabel::draw(int sel) 
{
	if ( (vertex1.isSet() || fixedLabelFlag) && drawMode != DRAW_HIDDEN)
	{
		vec2d v2;
		
		if ( fixedLabelFlag )
			v2 = fixedPos;
		else
			v2 = vertex1.pos2d();//geomPtr->getVertex2d(geomIndex);

		glColor3ub((int)color.x(), (int)color.y(), (int)color.z());

		glEnable(GL_TEXTURE_2D);
		if ( font )
			drawString(font, (float)(textSize * sqrt(viewScale)), getName(), (float)v2.x(), (float)v2.y(), (float)textOffset, 0);
		glDisable(GL_TEXTURE_2D);
	
		if (drawMode == DRAW_HIGHLIGHT)
		{
			if (sel)
				glColor3ub(255, 0, 0);
			glPointSize(sel ? 8.0f : 6.0f);
			glBegin(GL_POINTS);
			glVertex2d(v2.x(), v2.y());
			glEnd();
		}
	}

}


void TextLabel::writeTextLabel(Aircraft * airPtr, xmlNodePtr root)
{
	xmlNodePtr text_node = xmlNewChild( root, NULL, (const xmlChar *)"Text_Parms", NULL );

	vertex1.write(airPtr, text_node, "Vertex1_Parms");
	xmlAddDoubleNode(text_node, "TextOffset", textOffset);
	xmlAddIntNode( text_node, "FixedLabelFlag", fixedLabelFlag );
	xmlAddDoubleNode( text_node, "FixedPosX", fixedPos[0] );
	xmlAddDoubleNode( text_node, "FixedPosY", fixedPos[1] );
}

void TextLabel::write(Aircraft * airPtr, xmlNodePtr root)
{
	xmlAddStringNode( root, "Type", "TextLabel");

	writeLabelGeom(root);
	writeTextLabel(airPtr, root);

}

void TextLabel::readTextLabel(Aircraft * airPtr, xmlNodePtr root)
{
	xmlNodePtr text_node = xmlGetNode( root, "Text_Parms", 0 );
	if ( text_node )
	{
		vertex1.read(airPtr, text_node, "Vertex1_Parms");
		textOffset = xmlFindDouble(text_node, "TextOffset", textOffset);
		fixedLabelFlag = xmlFindInt( text_node, "FixedLabelFlag", fixedLabelFlag ) != 0;
		fixedPos[0] = xmlFindDouble(text_node, "FixedPosX", fixedPos[0] );
		fixedPos[1] = xmlFindDouble(text_node, "FixedPosY", fixedPos[1] );
	}
}

//==== Read External File ====//
void TextLabel::read(Aircraft * airPtr, xmlNodePtr root)
{
	readLabelGeom(root);
	readTextLabel(airPtr, root);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

RulerLabel::RulerLabel(ScreenMgr* sMgr) : TextLabel()
{
	screenMgr = sMgr;
	type = RULER_LABEL;
	type_str = "ruler";

	char name[255];
	sprintf( name, "Ruler_%d", labelCnt ); 
	name_str = Stringc(name);

	attachState = INITIALIZED;
	rulerOffset = 0;
	textOffset = 0.0f;

	xLock = yLock = zLock = 1;
	rulerDistance = 0;

	precision = 4;
	unitString = "";
}

void RulerLabel::reset()
{
	vertex1.reset();
	vertex2.reset();
	attachState = INITIALIZED;
	rulerOffset = 0;
	rulerDistance = 0;
}

void RulerLabel::calculateOffset()
{
	if (vertex1.isSet() && vertex2.isSet())
	{
		vec2d vs = vertex1.pos2d();
		vec2d ve = vertex2.pos2d();

		//=== calculate shortest vector between mousept and line
		double len2 = dist_squared(vs,ve);
		if (len2 != 0)
		{
			double u = dot(cursor - vs, ve - vs) / len2;
			vec2d vm = vs + (ve - vs) * u;
			vec2d c1 = ve-vs;
			vec2d c2 = cursor-vs;
			if (c1.x()*c2.y() - c1.y()*c2.x() > 0) // cross product
				rulerOffset = dist(vm, cursor) / viewScale;
			else
				rulerOffset = -dist(vm, cursor) / viewScale;
		}
		else
		{
			rulerOffset = 0;
		}
	}
}

void RulerLabel::setOffset(float mx, float my)
{
	calculateOffset();
}

void RulerLabel::getVertexPoints( vec3d * start, vec3d * end )
{
	if (vertex1.isSet())
	{
		(*start) = vertex1.xform( vertex1.pos3d() );
		if (vertex2.isSet())
		{
			vertex2.setAxisProject( xLock, yLock, zLock, (*start) );
			(*end) = vertex2.posXformProjAxis();
		}
	}
	


	//if (vertex1.isSet())
	//{
	//	(*start) = vertex1.xform( vertex1.pos3d() );
	//	if (vertex2.isSet())
	//	{
	//		vec3d ve3d = vertex2.xform( vertex2.pos3d() );
	//		(*end) = (*start);
	//		if (xLock)	end->set_x(ve3d.x());
	//		if (yLock)	end->set_y(ve3d.y());
	//		if (zLock)	end->set_z(ve3d.z());
	//	}
	//}

	//if (vertex1.isSet())
	//{
	//	(*start) = vertex1.pos3d();
	//	if (vertex2.isSet())
	//	{
	//		vec3d ve3d = vertex2.pos3d();
	//		(*end) = (*start);
	//		if (xLock)	end->set_x(ve3d.x());
	//		if (yLock)	end->set_y(ve3d.y());
	//		if (zLock)	end->set_z(ve3d.z());
	//	}
	//}
}

double RulerLabel::calculateDistance() 
{

	if (vertex1.isSet() && vertex2.isSet())
	{
		vec3d startPnt(0,0,0);
		vec3d endPnt(0,0,0);
		getVertexPoints(&startPnt, &endPnt);

		rulerDistance = dist(startPnt, endPnt);
//		rulerDistance = dist(vertex1.xform(startPnt), vertex2.xform(endPnt));
//		rulerDistance = dist(vertex1.geomPtr->xformPoint(startPnt), vertex2.geomPtr->xformPoint(endPnt, vertex2.reflect));
	}
	else
	{
		rulerDistance = 0;
	}

	return rulerDistance;
}

void RulerLabel::draw(int sel) 
{
	if (drawMode == DRAW_HIDDEN) return;

	
	if (vertex1.isSet())
	{
		vec2d startPnt = vertex1.pos2d();
		vec2d endPnt = cursor;
		if (vertex2.isSet())
		{
			vec3d start3d(0,0,0);
			vec3d end3d(0,0,0);
			getVertexPoints(&start3d, &end3d);
			startPnt = vertex1.viewProject( start3d );
			endPnt = vertex2.viewProject( end3d );
		}
		//==== Calculate Distance ====//
		calculateDistance();
		Stringc label;
		if (vertex2.isSet())
		{
			char str[255];
			char fmt[255];
			sprintf(fmt, "%%0.%df %%s", precision);
			sprintf(str, fmt, rulerDistance, unitString.get_char_star());
			label = str;
		}
		else
		{
			label = getName();
		}

		if (sel)
			glColor3ub(255, 0, 0);
		else
			glColor3ub((int)color.x(), (int)color.y(), (int)color.z());
		//==== Draw Points ====//
		if (drawMode == DRAW_HIGHLIGHT)
		{
			glPointSize(sel ? 7.0f : 6.0f);
			glBegin(GL_POINTS);
			glVertex2d(startPnt.x(), startPnt.y());
			glVertex2d(endPnt.x(), endPnt.y());
			glEnd();
		}

		//==== Calculate Offset ====//
		if (attachState == VERTEX2_SET)
			calculateOffset();
		vec2d deltavec = startPnt - endPnt;
		vec2d offset(deltavec.y(), -deltavec.x());
		offset.normalize();
		vec2d startOff = startPnt + offset * rulerOffset * viewScale;
		vec2d endOff = endPnt   + offset * rulerOffset * viewScale;

		//==== Draw Lines ====//
		glLineWidth(sel ? 2.0f : 1.0f);
		glBegin(GL_LINE_STRIP);
			glVertex2d(startPnt.x(), startPnt.y());
			glVertex2d(startOff.x(), startOff.y());
			glVertex2d(endOff.x(), endOff.y());
			glVertex2d(endPnt.x(), endPnt.y());
		glEnd();

		//==== Draw Text ====//
		vec2d vc = (startOff + endOff) * 0.5;
		double ang = PI/2.0;
		if (deltavec.x() > 0.0001 || deltavec.x() < -0.0001) // ~!= 0
			ang = atan( (deltavec.y()) / (deltavec.x()) );

		glColor3ub((int)color.x(), (int)color.y(), (int)color.z());
		glPushMatrix();
		{
			glTranslated(vc.x(), vc.y(), 0);
			glRotatef((float)RAD2DEG(ang), 0,0,1);
			glEnable(GL_TEXTURE_2D);
			if ( font )
				drawString(font, textSize * (float)sqrt(viewScale), label, 0,0, (float)textOffset, 0.7f);
			glDisable(GL_TEXTURE_2D);
		}
		glPopMatrix();
	}

}
void RulerLabel::writeRulerLabel(Aircraft * airPtr, xmlNodePtr root)
{
	xmlNodePtr ruler_node = xmlNewChild( root, NULL, (const xmlChar *)"Ruler_Parms", NULL );

	vertex2.write(airPtr, ruler_node, "Vertex2_Parms");
	xmlAddDoubleNode(ruler_node, "RulerOffset", rulerOffset);
	xmlAddIntNode(ruler_node, "AttachState", attachState);
	xmlAddIntNode(ruler_node, "XLock", xLock);
	xmlAddIntNode(ruler_node, "YLock", yLock);
	xmlAddIntNode(ruler_node, "ZLock", zLock);
	xmlAddDoubleNode(ruler_node, "RulerDistance", rulerDistance);
	xmlAddIntNode(ruler_node, "Precision", precision);
	xmlAddStringNode(ruler_node, "UnitString", unitString.get_char_star());
}

void RulerLabel::write(Aircraft * airPtr, xmlNodePtr root)
{
	xmlAddStringNode( root, "Type", "RulerLabel");

	writeLabelGeom(root);
	writeTextLabel(airPtr, root);
	writeRulerLabel(airPtr, root);

}

void RulerLabel::readRulerLabel(Aircraft * airPtr, xmlNodePtr root)
{
	xmlNodePtr ruler_node = xmlGetNode( root, "Ruler_Parms", 0 );
	if ( ruler_node )
	{
		vertex2.read(airPtr, ruler_node, "Vertex2_Parms");
		rulerOffset = xmlFindDouble(ruler_node, "RulerOffset", rulerOffset);
		attachState = xmlFindInt(ruler_node, "AttachState", attachState);
		xLock = xmlFindInt(ruler_node, "XLock", xLock);
		yLock = xmlFindInt(ruler_node, "YLock", yLock);
		zLock = xmlFindInt(ruler_node, "ZLock", zLock);
		rulerDistance = xmlFindDouble(ruler_node, "RulerDistance", rulerDistance);

		precision = xmlFindInt(ruler_node, "Precision", precision);
		unitString = xmlFindString(ruler_node, "UnitString", unitString.get_char_star());
	}
}

void RulerLabel::read(Aircraft * airPtr, xmlNodePtr root)
{
	readLabelGeom(root);
	readTextLabel(airPtr, root);
	readRulerLabel(airPtr, root);
}
