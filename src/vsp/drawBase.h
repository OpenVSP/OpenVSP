//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// drawBase.h: interface for the drawBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRAWBASE_H__A0CD7520_BCDE_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_DRAWBASE_H__A0CD7520_BCDE_11D7_AC31_0003473A025A__INCLUDED_


class DrawBase  
{
public:
	DrawBase();
	virtual ~DrawBase();

	virtual void draw() = 0;
	virtual int processKeyEvent()			{ return 0; }		// Return true to force redraw
	virtual int processPushEvent()			{ return 0; }
	virtual int processMoveEvent()			{ return 0; }
	virtual int processReleaseEvent()		{ return 0; }
	virtual int processDragEvent()			{ return 0; }

	virtual void setGlWinWidthHeight( int w, int h )							{}
	virtual void setGlWinOrthoLRTB( double l, double r, double t, double b  )	{}

};

#endif // !defined(AFX_DRAWBASE_H__A0CD7520_BCDE_11D7_AC31_0003473A025A__INCLUDED_)
