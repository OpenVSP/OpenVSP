//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// labelScreen.cpp: implementation of the labelScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "labelScreen.h"
#include "screenMgr.h"
#include "scriptMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LabelScreen::LabelScreen(ScreenMgr* mgrPtr, Aircraft* airPtr, VspGlWindow* winPtr)
{
	screenMgrPtr = mgrPtr;
	aircraftPtr = airPtr;
	glWinPtr = winPtr;

	track = 0;
	highlight = 0;

	labelUI = new LabelUI();
	labelUI->UIWindow->position( 610, 480 );
//	labelUI->UIWindow->show();

	labelUI->addLabelButton->callback( staticScreenCB, this );
	labelUI->removeLabelButton->callback( staticScreenCB, this );
	labelUI->selAllButton->callback( staticScreenCB, this );
	labelUI->showButton->callback( staticScreenCB, this );
	labelUI->hideButton->callback( staticScreenCB, this );
	labelUI->highlightButton->callback( staticScreenCB, this );
	labelUI->labelBrowser->callback( staticScreenCB, this );

	labelUI->nameInput->callback( staticScreenCB, this );
	labelUI->redSlider->callback( staticScreenCB, this );
	labelUI->redSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
	labelUI->greenSlider->callback( staticScreenCB, this );
	labelUI->greenSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
	labelUI->blueSlider->callback( staticScreenCB, this );
	labelUI->blueSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);

	labelUI->textSizeSlider->callback( staticScreenCB, this );
	labelUI->textSizeSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
	labelUI->textOffsetSlider->callback( staticScreenCB, this );
	labelUI->textOffsetSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
	labelUI->attachTextButton->callback( staticScreenCB, this );

	labelUI->rulerOffsetSlider->callback( staticScreenCB, this );
	labelUI->rulerOffsetSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
	labelUI->xLockButton->callback( staticScreenCB, this );
	labelUI->yLockButton->callback( staticScreenCB, this );
	labelUI->zLockButton->callback( staticScreenCB, this );
	labelUI->attachRulerButton->callback( staticScreenCB, this );

	labelUI->unitsInput->callback( staticScreenCB, this );
	labelUI->precisionSlider->callback( staticScreenCB, this );
	labelUI->precisionSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
	labelUI->setAllUnitsButton->callback( staticScreenCB, this );

	labelUI->fixedButton->callback( staticScreenCB, this );
	labelUI->xPosSlider->callback( staticScreenCB, this );
	labelUI->xPosSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
	labelUI->yPosSlider->callback( staticScreenCB, this );
	labelUI->yPosSlider->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
}



void LabelScreen::screenCB( Fl_Widget* w )
{

	// browser
	if ( w == labelUI->addLabelButton )
	{
		int val = labelUI->labelTypeChoice->value();
		s_add(ScriptMgr::GUI, val, "");
		/*
		LabelGeom* label;
		switch (val)
		{
			case LabelGeom::TEXT_LABEL:
				label = aircraftPtr->createLabel(LabelGeom::TEXT_LABEL);
				break;
			case LabelGeom::RULER_LABEL:
				label = aircraftPtr->createLabel(LabelGeom::RULER_LABEL);
				break;
		}
		if (label)
		{
			label->setTextSize(labelUI->textSizeSlider->value());
			label->setColor( labelUI->redSlider->value(), 
				labelUI->greenSlider->value(), labelUI->blueSlider->value() );

			if (labelUI->hideButton->value())
				label->setDrawMode(LabelGeom::DRAW_HIDDEN);
			else if (labelUI->showButton->value())
				label->setDrawMode(LabelGeom::DRAW_VISIBLE);
			else if (labelUI->highlightButton->value())
				label->setDrawMode(LabelGeom::DRAW_HIGHLIGHT);
		}
		loadLabelBrowser();
		setTrack(0);
		setHighlight(0);
		*/
	}
	else if ( w == labelUI->labelBrowser )
	{
		vector< LabelGeom* > selectVec = getSelectedLabels();
		scriptMgr->addLine("label", "select", getNameString(selectVec));
		s_select(ScriptMgr::GUI, selectVec);
		/*
		aircraftPtr->setActiveLabelVec(getSelectedLabels());
		setTrack(0);
		setHighlight(0);
		update();
		*/
	}
	else if ( w == labelUI->selAllButton )
	{
		s_select_all(ScriptMgr::GUI);
/*		aircraftPtr->setActiveLabelVec(getSelectedLabels());
		for (int i = 0; i < labelUI->labelBrowser->size(); i++)
		{
			labelUI->labelBrowser->select(i+1);
		}
		setTrack(0);
		setHighlight(0);
		update();
		*/
		scriptMgr->addLine("label select all");
	}
	else if ( w == labelUI->removeLabelButton)
	{
		vector< LabelGeom* > removeVec = getSelectedLabels();
		scriptMgr->addLine("label", "remove", getNameString(removeVec));
		s_remove(ScriptMgr::GUI, removeVec);
/*
		vector< LabelGeom* > removeLabels = getSelectedLabels();
		scriptMgr->addLine("label", "remove", getNameString(removeLabels));

		for (int i = 0; i < removeLabels.size(); i++)
		{
			aircraftPtr->removeLabel(removeLabels[i]);
		}
		*/

	}
	

	vector< LabelGeom* > labelVec = aircraftPtr->getActiveLabelVec();

	// common 
	if ( w == labelUI->nameInput)
	{
		Stringc name = labelUI->nameInput->value();

		for (int i = 0; i < (int)labelVec.size(); i++)
		{
			labelVec[i]->setName(name);
		}
		loadLabelBrowser();
//		char str[256];
//		sprintf(str, "\"%s\"", name.get_char_star());
		name.quote('\"');
		scriptMgr->addLine("label", "name", name);
		
	}
	else if ( w == labelUI->redSlider || w == labelUI->greenSlider || w == labelUI->blueSlider )
	{
		int r = (int)labelUI->redSlider->value();
		int g = (int)labelUI->greenSlider->value();
		int b = (int)labelUI->blueSlider->value();
		labelUI->colorBox->color( fl_rgb_color(r, g, b) );
		labelUI->colorBox->redraw();

		for (int i = 0; i < (int)labelVec.size(); i++)
		{
			labelVec[i]->setColor( r, g, b );
		}
		if (!Fl::event_state(FL_BUTTONS))
			scriptMgr->addLine("label", "color", r, g, b);
	}
	else if ( w == labelUI->textSizeSlider )
	{
		double size = labelUI->textSizeSlider->value();
		for (int i = 0; i < (int)labelVec.size(); i++)
		{
			labelVec[i]->setTextSize((float)size);
		}
		if (!Fl::event_state(FL_BUTTONS))
			scriptMgr->addLineDouble("label size", size);
	}
	else if ( w == labelUI->showButton )
	{
		labelUI->showButton->value(1);
		labelUI->hideButton->value(0);
		labelUI->highlightButton->value(0);
		for (int i = 0; i < (int)labelVec.size(); i++)
		{
			labelVec[i]->setDrawMode(LabelGeom::DRAW_VISIBLE);
		}
		scriptMgr->addLine("label", "display", "basic");
	}
	else if ( w == labelUI->hideButton )
	{
		labelUI->showButton->value(0);
		labelUI->hideButton->value(1);
		labelUI->highlightButton->value(0);
		for (int i = 0; i < (int)labelVec.size(); i++)
		{
			labelVec[i]->setDrawMode(LabelGeom::DRAW_HIDDEN);
		}
		scriptMgr->addLine("label", "display", "hide");
	}
	else if ( w == labelUI->highlightButton )
	{
		labelUI->showButton->value(0);
		labelUI->hideButton->value(0);
		labelUI->highlightButton->value(1);
		for (int i = 0; i < (int)labelVec.size(); i++)
		{
			labelVec[i]->setDrawMode(LabelGeom::DRAW_HIGHLIGHT);
		}
		scriptMgr->addLine("label", "display", "full");
	}

	
	
	LabelGeom* activeLabel = aircraftPtr->getActiveLabel();
	if (activeLabel != NULL)
	{
	
		if (activeLabel->getType() == LabelGeom::TEXT_LABEL)
		{	// text label
			TextLabel * textLabel = (TextLabel*) activeLabel;
		
			if ( w == labelUI->textOffsetSlider )
			{
				double offset = labelUI->textOffsetSlider->value();
				textLabel->setTextOffset(offset);
				scriptMgr->addLineDouble("text offset", offset);
			}
			else if ( w == labelUI->attachTextButton )
			{
				int val = !labelUI->attachTextButton->value();
				setTrack(val);
				setHighlight(val);
				labelUI->attachRulerButton->label(val ? "Select Vertex" : "Attach Text");
			}
			else if ( w == labelUI->fixedButton )
			{
				int f = labelUI->fixedButton->value();
				textLabel->setFixedLabelFlag( f != 0 );
			}
			else if ( w == labelUI->xPosSlider )
			{
				double x = labelUI->xPosSlider->value();
				textLabel->setFixedPosX( x );
			}
			else if ( w == labelUI->yPosSlider )
			{
				double y = labelUI->yPosSlider->value();
				textLabel->setFixedPosY( y );
			}
		}
		else if (activeLabel->getType() == LabelGeom::RULER_LABEL)
		{	// ruler
			RulerLabel * rulerLabel = (RulerLabel*) activeLabel;
			if ( w == labelUI->rulerOffsetSlider )
			{
				double offset = labelUI->rulerOffsetSlider->value();
				rulerLabel->setRulerOffset(offset);
				if (!Fl::event_state(FL_BUTTONS))
					scriptMgr->addLineDouble("ruler offset", offset);
			}
			else if ( w == labelUI->xLockButton )
			{
				if (!labelUI->yLockButton->value() && !labelUI->zLockButton->value())
					labelUI->xLockButton->value(1);
				int x = labelUI->xLockButton->value();
				int y = labelUI->yLockButton->value();
				int z = labelUI->zLockButton->value();
				rulerLabel->setXYZLock(x,y,z);
				scriptMgr->addLine("ruler", "lock", x,y,z);
			}
			else if ( w == labelUI->yLockButton )
			{
				if (!labelUI->xLockButton->value() && !labelUI->zLockButton->value())
					labelUI->yLockButton->value(1);
				int x = labelUI->xLockButton->value();
				int y = labelUI->yLockButton->value();
				int z = labelUI->zLockButton->value();
				rulerLabel->setXYZLock(x,y,z);
				scriptMgr->addLine("ruler", "lock", x,y,z);
			}
			else if ( w == labelUI->zLockButton )
			{
				if (!labelUI->xLockButton->value() && !labelUI->yLockButton->value())
					labelUI->zLockButton->value(1);
				int x = labelUI->xLockButton->value();
				int y = labelUI->yLockButton->value();
				int z = labelUI->zLockButton->value();
				rulerLabel->setXYZLock(x,y,z);
				scriptMgr->addLine("ruler", "lock", x,y,z);
			}
			else if ( w == labelUI->attachRulerButton )
			{
				int val = !labelUI->attachRulerButton->value();
				setTrack(val);
				setHighlight(val);
				rulerLabel->reset();
				labelUI->attachRulerButton->label(val ? "Select Vertex 1" : "Attach Ruler");

			}
			else if ( w == labelUI->precisionSlider )
			{
				rulerLabel->setPrecision((int)labelUI->precisionSlider->value());
				if (!Fl::event_state(FL_BUTTONS))
					scriptMgr->addLine("ruler", "precision", rulerLabel->getPrecision());
			}
			else if ( w == labelUI->unitsInput )
			{
				rulerLabel->setUnitString(labelUI->unitsInput->value());
				scriptMgr->addLine("ruler", "units", rulerLabel->getUnitString());
			}
			else if ( w == labelUI->setAllUnitsButton )
			{
				vector<LabelGeom *> labelVec = aircraftPtr->getLabelVec();
				for (int i = 0; i < (int)labelVec.size(); i++)
				{
					LabelGeom * label = labelVec[i];
					if (label->getType() == LabelGeom::RULER_LABEL)
						((RulerLabel*)label)->setUnitString(labelUI->unitsInput->value());

				}
			}
		}
	}

	// redraw
	aircraftPtr->triggerDraw();
}

void LabelScreen::loadLabelBrowser()
{
	int i;

	vector< LabelGeom* > selVec = aircraftPtr->getActiveLabelVec();
	vector< LabelGeom* > labelVec = aircraftPtr->getLabelVec();

	labelUI->labelBrowser->clear();

	for ( i = 0 ; i < (int)labelVec.size() ; i++ )
	{
		labelUI->labelBrowser->add( labelVec[i]->getName()() );

		for (int j = 0; j < (int)selVec.size(); j++)
		{
			if (labelVec[i] == selVec[j])
			{
				labelUI->labelBrowser->select( i+1 );
				break;
			}
		}
			
	}

	
	//==== Restore List of Selected Labels ====//
//	vector< LabelGeom* > selVec = getSelectedLabels();
//	for ( i = 0 ; i < selVec.size() ; i++ )
//		selectLabelBrowser( selVec[i] );

	update();
}

vector< LabelGeom* > LabelScreen::getSelectedLabels()
{
	vector< LabelGeom* > selVec;
	vector< LabelGeom* > labelVec = aircraftPtr->getLabelVec();

	//==== Account For Browser Lines Starting at 1 and Aircraft Name ====//
	for ( int i = 1 ; i <= labelUI->labelBrowser->size() ; i++ )
	{
		if ( labelUI->labelBrowser->selected(i) )
			selVec.push_back( labelVec[i-1] );		
	}
	return selVec;
}

LabelGeom* LabelScreen::getActiveLabel()
{
	vector< LabelGeom* > selVec = getSelectedLabels();
	if (selVec.size() > 0)
		return selVec[selVec.size()-1];
	else
		return NULL;
}

void LabelScreen::selectLabelBrowser( LabelGeom* label )
{
	vector< LabelGeom* > labelVec = aircraftPtr->getLabelVec();

	for ( int i = 0 ; i < (int)labelVec.size() ; i++ )
	{
		if ( labelVec[i] == label)
			labelUI->labelBrowser->select( i+1 );
	}
}

void LabelScreen::update()
{
	vector< LabelGeom* > selVec = aircraftPtr->getActiveLabelVec();

	if (selVec.size() == 0)
	{
		labelUI->rulerUIGroup->hide();
		labelUI->textUIGroup->hide();
		labelUI->nameInput->value("");
	}
	else if (selVec.size() > 1)
	{
		labelUI->textUIGroup->hide();
		labelUI->rulerUIGroup->hide();
		labelUI->nameInput->value("<multiple>");
		labelUI->hideButton->value(0);
		labelUI->showButton->value(0);
		labelUI->highlightButton->value(0);
	}
	else
	{
		LabelGeom* activeLabel = selVec[0];
		if (activeLabel)
		{
			int type = activeLabel->getType();

			if (type == LabelGeom::TEXT_LABEL)
			{
				TextLabel * textLabel = (TextLabel*) activeLabel;
				labelUI->textUIGroup->show();
				labelUI->rulerUIGroup->hide();

				labelUI->textOffsetSlider->value(textLabel->getTextOffset());
				labelUI->fixedButton->value( textLabel->getFixedLabelFlag() );
				vec2d p = textLabel->getFixedPos();
				labelUI->xPosSlider->value( p[0] );
				labelUI->yPosSlider->value( p[1] );
			}
			else if (type == LabelGeom::RULER_LABEL)
			{
				RulerLabel * rulerLabel = (RulerLabel*) activeLabel;
				labelUI->textUIGroup->hide();
				labelUI->rulerUIGroup->show();

				labelUI->rulerOffsetSlider->value(rulerLabel->getRulerOffset());
				labelUI->xLockButton->value(rulerLabel->getXLock());
				labelUI->yLockButton->value(rulerLabel->getYLock());
				labelUI->zLockButton->value(rulerLabel->getZLock());
				labelUI->precisionSlider->value(rulerLabel->getPrecision());
				labelUI->unitsInput->value(rulerLabel->getUnitString());
			}

			labelUI->nameInput->value(activeLabel->getName()());
	
			vec3d color = activeLabel->getColor();
			labelUI->redSlider->value(color.x());
			labelUI->greenSlider->value(color.y());
			labelUI->blueSlider->value(color.z());
			labelUI->colorBox->color( fl_rgb_color((int)color.x(), (int)color.y(), (int)color.z()) );
			labelUI->textSizeSlider->value(activeLabel->getTextSize());
			labelUI->colorBox->redraw();
			labelUI->hideButton->value(activeLabel->getDrawMode() == LabelGeom::DRAW_HIDDEN);
			labelUI->showButton->value(activeLabel->getDrawMode() == LabelGeom::DRAW_VISIBLE);
			labelUI->highlightButton->value(activeLabel->getDrawMode() == LabelGeom::DRAW_HIGHLIGHT);
		}
	}


}

/*
void LabelScreen::draw()
{
	vec2d cursor = glWinPtr->getCursor();

//	printf("cursor %f, %f", cursor.x(), cursor.y());
	if (track && focus)
	{
		glPushMatrix();
		glLoadIdentity();
		glLineWidth(1.0f);
		glColor3f(1,0,0);
		glBegin(GL_LINES);
			glVertex3d(cursor.x()-0.01,   cursor.y(), 0);
			glVertex3d(cursor.x()+0.01,   cursor.y(), 0);
			glVertex3d(  cursor.x(), cursor.y()-0.01, 0);
			glVertex3d(  cursor.x(), cursor.y()+0.01, 0);
		glEnd();
		glPopMatrix();
	}
}
*/

void LabelScreen::selectVertex(VertexID vert, float mx, float my)
{
	if (track)
	{
		LabelGeom * label = getActiveLabel();

		if (label->getType() == LabelGeom::TEXT_LABEL && vert.isSet())
		{
			TextLabel * textLabel = (TextLabel*) label;
			textLabel->setVertex(vert);
			setTrack(0);
			setHighlight(0);
			labelUI->attachTextButton->label("Attach Text");
			s_vert_script(vert, "text attach");
		} 
		else if (label->getType() == LabelGeom::RULER_LABEL)
		{
			RulerLabel * rulerLabel = (RulerLabel*) label;
			switch (rulerLabel->getRulerState())
			{
			case RulerLabel::INITIALIZED:
				if (vert.isSet())
				{
					rulerLabel->setVertex(vert);
					labelUI->attachRulerButton->label("Select Vertex 2");
					rulerLabel->setRulerState(RulerLabel::VERTEX1_SET); 
					s_vert_script(vert, "ruler attach1");
				}
				break;
			case RulerLabel::VERTEX1_SET:
				if (vert.isSet())
				{
					rulerLabel->setVertex2(vert);
					labelUI->attachRulerButton->label("Select Offset");
					rulerLabel->setRulerState(RulerLabel::VERTEX2_SET); 
					setHighlight(0);
					s_vert_script(vert, "ruler attach2");
				}
				break;
			default:
			case RulerLabel::VERTEX2_SET:
				rulerLabel->setOffset(mx, my);
				labelUI->rulerOffsetSlider->value(rulerLabel->getRulerOffset());
				setTrack(0);
				labelUI->attachRulerButton->label("Attach Ruler");
				rulerLabel->setRulerState(RulerLabel::FINALIZED); 
				scriptMgr->addLineDouble("ruler offset", rulerLabel->getRulerOffset());
				break;
			}
		}
	}
}


void LabelScreen::setTrack(int on)
{
	track = on;
	glWinPtr->setMouseTracking(track);

	labelUI->attachTextButton->value(track);
	labelUI->attachRulerButton->value(track);
}

void LabelScreen::setHighlight(int on)
{ 
	highlight = on; 

}

Stringc LabelScreen::getNameString(vector< LabelGeom* > selVec)
{
	char cs[256];

	Stringc str("");
	for ( int i = 0; i < (int)selVec.size(); i++) {
		sprintf(cs, "\"%s\" ", selVec[i]->getName()());
		str.concatenate(cs);
	}
	return str;
}

vector< LabelGeom* > LabelScreen::s_get_label_vec(deque< Stringc > selectVec)
{
	vector< LabelGeom * > labelVec;
	for (int i = 0; i < (int)selectVec.size(); i++)
	{	// select geoms
		Stringc name = selectVec[i];
		name.trim('\"');
		vector<LabelGeom *> lVec = aircraftPtr->getLabelByName(name);

		for (int j = 0; j < (int)lVec.size(); j++)
		{	// add all labels that are not already in the list
			vector< LabelGeom* >::iterator liter = find(labelVec.begin(), labelVec.end(), lVec[j]);
			if (liter == labelVec.end())
				labelVec.push_back(lVec[j]);
		}
	}
	return labelVec;
}

void LabelScreen::cleanRulers()
{
	// finalize previous label
	vector<LabelGeom*> labelVec = aircraftPtr->getActiveLabelVec();

	for (int i = 0; i < (int)labelVec.size(); i++)
	{
		if (labelVec[i]->getType() == LabelGeom::RULER_LABEL)
		{
			RulerLabel * ruler = (RulerLabel *) labelVec[i];
			if (ruler->getRulerState() == RulerLabel::VERTEX1_SET)
				ruler->reset();
			else if (ruler->getRulerState() == RulerLabel::VERTEX2_SET)
				ruler->setRulerState(RulerLabel::FINALIZED);
		}
	}

	labelUI->attachRulerButton->value(0);
	labelUI->attachRulerButton->label("Attach Ruler");

}

void LabelScreen::s_add(int src, int type, Stringc name)
{
	cleanRulers();

	LabelGeom* label = NULL;
	switch (type)
	{
		case LabelGeom::TEXT_LABEL:
			label = aircraftPtr->createLabel(LabelGeom::TEXT_LABEL);
			break;
		case LabelGeom::RULER_LABEL:
			label = aircraftPtr->createLabel(LabelGeom::RULER_LABEL);
			break;
	}


	if (label)
	{
		if (name.get_length() > 0) label->setName(name);
		
		label->setTextSize((float)labelUI->textSizeSlider->value());
		label->setColor( labelUI->redSlider->value(), 
			labelUI->greenSlider->value(), labelUI->blueSlider->value() );

		if (labelUI->hideButton->value())
			label->setDrawMode(LabelGeom::DRAW_HIDDEN);
		else if (labelUI->showButton->value())
			label->setDrawMode(LabelGeom::DRAW_VISIBLE);
		else if (labelUI->highlightButton->value())
			label->setDrawMode(LabelGeom::DRAW_HIGHLIGHT);
		
		loadLabelBrowser();

		
		switch (type)
		{
			case LabelGeom::TEXT_LABEL:
				labelUI->attachTextButton->value(1);
				break;
			case LabelGeom::RULER_LABEL:
				labelUI->attachRulerButton->value(1);
				break;
		}
		setTrack(1);
		setHighlight(1);
		
		Stringc labelName = label->getName();
		labelName.quote('\"');
		if (src == ScriptMgr::GUI) scriptMgr->addLine("label add", label->getTypeStr(), labelName);
	}



}

void LabelScreen::s_remove(int src, vector< LabelGeom* > removeVec)
{
	for (int i = 0; i < (int)removeVec.size(); i++)
	{
		aircraftPtr->removeLabel(removeVec[i]);
	}

	if (src != ScriptMgr::SCRIPT)
	{
		loadLabelBrowser();
		update();
	}
}
void LabelScreen::s_select_all(int src)
{
	cleanRulers();


	if (src != ScriptMgr::SCRIPT)
	{
		for (int i = 0; i < labelUI->labelBrowser->size(); i++)
		{
			labelUI->labelBrowser->select(i+1);
		}
		aircraftPtr->setActiveLabelVec(getSelectedLabels());
		setTrack(0);
		setHighlight(0);
		update();
	}
}
void LabelScreen::s_select(int src, vector< LabelGeom* > selectVec)
{
	cleanRulers();

	aircraftPtr->setActiveLabelVec(selectVec);
	if (src != ScriptMgr::SCRIPT)
	{
		setTrack(0);
		setHighlight(0);
		update();
	}
}

void LabelScreen::s_vert_script(VertexID vert, Stringc command)
{
	static char str[512];
	sprintf(str, "%s %s %d %0.4f %0.4f %d", command.get_char_star(), vert.geomPtr->getName().get_char_star(), 
		vert.surface, vert.section, vert.point, vert.reflect);
	scriptMgr->addLine(str);
}


