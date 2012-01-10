//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//  Script Manager Class
//
//
//   J.R. Gloudemans - 7/14/04
//
//******************************************************************************

#ifndef SCRIPT_MGR_H
#define SCRIPT_MGR_H

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

#include <FL/Fl_Text_Buffer.H>
#include "stringc.h"
#include "scriptScreen.h"
#include "scriptOutScreen.h"
#include <vector>
#include <deque>
#include "screenMgr.h"
#include "aircraft.h"


using namespace std;


class ScriptMgr
{
public:
	VSPDLL enum { GUI, GUI_SCRIPT, SCRIPT };

	VSPDLL ScriptMgr();
	VSPDLL virtual ~ScriptMgr() {}

	void init(ScriptUI * sUI, ScriptOutUI * soUI);
	void setAircraftPtr(Aircraft * air)				{ aircraft = air; }
	void setScreenMgr(ScreenMgr * sm)				{ screenMgr = sm; }
	void setScriptMode(int smode)					{ scriptMode = smode; }

	void save(const char * file);
	void play();
	void clear();
	void clearErrors();

	void setRecord(int flag)						{ record = flag; }
	void setExecute(int flag)						{ execute = flag; scriptUI->scriptEditor->take_focus();}
	void setQuiet(int flag)							{ quiet = flag; }

	// utility functions
	int verifySingleGeom(Stringc command);
	void setParm(GeomBase * geomPtr, Parm* parm, int index, deque<Stringc> &wordVec, Stringc &command);

	void generateLineVec(Stringc script, deque< Stringc > * returnVec);
	void generateWordVec(Stringc script, deque< Stringc > * returnVec);
	
	VSPDLL void parseScript(Stringc script);
	void parseLine(Stringc line);
	void parseCommand(deque<Stringc> &wordVec, Stringc command);
	void parseGen(deque<Stringc> &wordVec, Stringc command);
	void parseXform(deque<Stringc> &wordVec, Stringc command);

	void parsePodDesign(deque<Stringc> &wordVec, Stringc command);

	void parseWingPlan(deque<Stringc> &wordVec, Stringc command);
	void parseWingSect(deque<Stringc> &wordVec, Stringc command);
	void parseWingDihed(deque<Stringc> &wordVec, Stringc command);
	void parseWingFoil(deque<Stringc> &wordVec, Stringc command);

	void parseFuseShape(deque<Stringc> &wordVec, Stringc command);
	void parseFuseXSec(deque<Stringc> &wordVec, Stringc command);
	void parseFuseIml(deque<Stringc> &wordVec, Stringc command);

	void parseHavocPlan(deque<Stringc> &wordVec, Stringc command);
	void parseHavocXSec(deque<Stringc> &wordVec, Stringc command);
	void parseHavocSide(deque<Stringc> &wordVec, Stringc command);

	void parseExtDesign(deque<Stringc> &wordVec, Stringc command);

	void parseDuctShape(deque<Stringc> &wordVec, Stringc command);
	void parseDuctFoil(deque<Stringc> &wordVec, Stringc command);

	void parsePropShape(deque<Stringc> &wordVec, Stringc command);
	void parsePropStation(deque<Stringc> &wordVec, Stringc command);
	void parsePropFoil(deque<Stringc> &wordVec, Stringc command);

	void parseEngineDesign(deque<Stringc> &wordVec, Stringc command);
	void parseEngineInlet(deque<Stringc> &wordVec, Stringc command);

	void parseFoil(Geom* currGeom, Af* foilPtr, deque<Stringc> &wordVec, Stringc command);

	void parseLabel(deque<Stringc> &wordVec, Stringc command);
	void parseText(deque<Stringc> &wordVec, Stringc command);
	void parseRuler(deque<Stringc> &wordVec, Stringc command);


	double getDouble(double d, Stringc modifier);
	int getInteger(int i, Stringc modifier);
	
	void addLine(const char * command);
	void addLine(const char * command, const char * v1);
	void addLineDouble(const char * command, double d);
	void addLineInt(const char * command, double d);
	void addLine(const char * group, const char * command, double v1, double v2, double v3);
	void addLine(const char * group, const char * command, const char * v1);
	void addLine(const char * group, const char * command, const char * v1, const char * v2);
	void addLine(const char * group, const char * command, int v1, int v2, int v3);
	void addLine(const char * group, const char * command, int v1);

	void addError(const char * error);
	void addError(const char * type, const char * error, const char * cmd);
	void addArgError(const char * cmd);
	void addIncError(const char * cmd);


	static void staticScriptCB(int pos, int nInserted, int nDeleted, int nRestyled, const char* deletedText, void* cbArg);
	void scriptCB(int pos, int nInserted, int nDeleted, int nRestyled, const char* deletedText);
	void timeoutHandler();
	static void staticTimeoutHandler(void *data);
	void setTimeout(float t, char * buffer);

protected:
	void appendScript(const char * text);
	void appendError(const char * text);
	void scrollEnd(Fl_Text_Display * text_display);

	ScriptUI * scriptUI;
	ScriptOutUI * scriptOutUI;

	Aircraft * aircraft;
	ScreenMgr * screenMgr;

	char str[5120];
	int record;
	int execute;
	int blockCB; // avoid circular callbacks
	int quiet;

	int scriptMode;
	float timerTime;
};



class SingleScriptMgr {
public:
	SingleScriptMgr() 
	{
		static ScriptMgr *sptr = 0;
		if (!sptr) sptr = new ScriptMgr();
		scriptMgr = sptr;
	}


	ScriptMgr * scriptMgr;
};

static SingleScriptMgr singleScriptMgr;

#define scriptMgr (singleScriptMgr.scriptMgr)
#endif  

