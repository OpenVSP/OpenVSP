//******************************************************************************
//
//   Hybrid Wing Body Screen Class
//
//
//   Michael Link - 6/11/08
//   AVID LLC
//
//
//******************************************************************************

#ifndef HWBSCREEN_H
#define HWBSCREEN_H

#include <stdio.h>
#include <FL/Fl.H> 
#include "geom.h"
#include "hwb.h"
#include "gui_device.h"
#include "vspScreen.h"
#include "vspGlWindow.h"
#include "hwbFlScreen.h"
#include <FL/Fl_File_Chooser.H>

class Input;
class Slider;
class Slider_adj_range;
class ScreenMgr;

class HwbScreen : public CompScreen
{
	public:

		HwbScreen(ScreenMgr* mgrPtr);
		virtual ~HwbScreen();

		void show(Geom* geomPtr);
		void hide();
		void screenCB( Fl_Widget* w );
		void closeCB( Fl_Widget* w );
		void position( int x, int y );
		void setTitle( const char* name );

		void parm_changed( Parm* parm );
		void updateName();

	protected:

		HybridWingBodyUI* hwbUI;
		XSecGlWindow* glWin;

		Slider_adj_range* totalSpanSlider;
		Input* totalSpanInput;
		Slider_adj_range* totalProjSpanSlider;
		Input* totalProjSpanInput;
		Slider_adj_range* totalChordSlider;
		Input* totalChordInput;
		Slider_adj_range* totalAreaSlider;
		Input* totalAreaInput;

		Input* totalARInput;
		Slider_adj_range* sweepOffsetSlider;
		Input* sweepOffsetInput;

		Slider* ARSlider;
		Input* ARInput;
		Slider* TRSlider;
		Input* TRInput;
		Slider* areaSlider;
		Input* areaInput;
		Slider* spanSlider;
		Input* spanInput;
		Slider* TCSlider;
		Input* TCInput;
		Slider* RCSlider;
		Input* RCInput;
	
		Input* sectProjSpanOutput;

		Slider* sweepSlider;
		Input* sweepInput;
		Slider* sweepLocSlider;
		Input* sweepLocInput;
		Slider* twistSlider;
		Input* twistInput;
		Slider* twistLocSlider;
		Input* twistLocInput;
		Slider* dihed1Slider;
		Input* dihed1Input;

		Slider_adj_range* sweepPercentSpan1Slider;
		Input* sweepPercentSpan1Input;
		Slider_adj_range* tesweepPercentSpan1Slider;
		Input* tesweepPercentSpan1Input;
		Slider_adj_range* dihedralPercentSpan1Slider; 
		Input* dihedralPercentSpan1Input;

		Input* sweepDegPerSegInput;
		Input* tesweepDegPerSegInput;
		Input* dihedralDegPerSegInput;

		Input* filletedTipChordOutput;
		Input* filletedRootChordOutput;

		//==== Airfoil Stuff ====//
		Slider* afCamberSlider;
		Input* afCamberInput;
		Slider* afCamberLocSlider;
		Input* afCamberLocInput;
		Slider* afThickSlider;
		Input* afThickInput;
		Slider* afThickLocSlider;
		Input* afThickLocInput;
		Slider* afIdealClSlider;
		Input* afIdealClInput;
		Slider* afASlider;
		Input* afAInput;
		Slider* afSlatChordSlider;
		Input* afSlatChordInput;
		Slider* afSlatAngleSlider;
		Input* afSlatAngleInput;
		Slider* afFlapChordSlider;
		Input* afFlapChordInput;
		Slider* afFlapAngleSlider;
		Input* afFlapAngleInput;


		Input* leRadiusInput;

		ParmButton* totalSpanButton;
		ParmButton* totalProjSpanButton;
		ParmButton* totalChordButton;
		ParmButton* totalAreaButton;
		ParmButton* sweepOffsetButton;


		Hwb_geom* currGeom;
};

#endif
