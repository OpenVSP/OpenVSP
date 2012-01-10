//****************************************************************************
//    
//   Script Manager Class
//   
//   J.R. Gloudemans - 7/14/04
// 
//****************************************************************************

#include "scriptMgr.h"
#include "geomScreen.h"
#include "vspScreen.h"
#include "materialMgr.h"
#include "groupScreen.h"
#include "fuseScreen.h"
#include "extScreen.h"
#include "engineScreen.h"
#include "propScreen.h"
#include "havocScreen.h"
#include "ductScreen.h"
#include "msWingScreen.h"
#include "labelScreen.h"


ScriptMgr::ScriptMgr() 
{

	scriptUI = NULL;
	scriptOutUI = NULL;

	aircraft = NULL;
	screenMgr = NULL;


	record = 0;
	execute = 0;
	blockCB = 0;
	quiet = 0;
	scriptMode = GUI_SCRIPT;

}

void ScriptMgr::init(ScriptUI * sUI, ScriptOutUI * soUI)
{
	scriptUI = sUI;
	scriptUI->scriptEditor->buffer()->add_modify_callback(staticScriptCB, this);

	scriptOutUI = soUI;
}


void ScriptMgr::save(const char * file)
{
	Fl_Text_Buffer * scriptBuffer = scriptUI->scriptEditor->buffer();

	FILE* fid = fopen(file, "w");
	fprintf( fid, scriptBuffer->text() );
	fclose(fid);
}


void ScriptMgr::play()
{
	Fl_Text_Buffer * scriptBuffer = scriptUI->scriptEditor->buffer();

	const char * selText = NULL;

	if (scriptBuffer->selected())
	{	// get selection region
		selText = scriptBuffer->selection_text();
		if (selText != NULL) 
		{
			parseScript(Stringc(selText));
			free((void *)selText);
		}
	}
	else
	{	// get line
		int pos = scriptUI->scriptEditor->insert_position();
		if (pos == scriptBuffer->length() && 
			!fl_choice("Are you sure you want to execute all lines in the script window?", "No", "Yes", NULL))			
		{	//cursor at end
			return;
		}
		selText = scriptBuffer->line_text(pos);
		if (selText != NULL) 
		{
			parseScript(Stringc(selText));
			free((void *)selText);
		}
		scriptUI->scriptEditor->move_down();
	}
	scriptUI->scriptEditor->take_focus();
	
}


void ScriptMgr::clear() 
{
	scriptUI->scriptEditor->buffer()->text("");
}

void ScriptMgr::clearErrors() 
{
	scriptOutUI->errorDisplay->buffer()->text("");
}

void ScriptMgr::scrollEnd(Fl_Text_Display * text_display)
{
	// scoll to show last line
	int lines = text_display->buffer()->count_lines(0, text_display->buffer()->length());
	text_display->scroll(lines-2, 0);
}


void ScriptMgr::scriptCB(int pos, int nInserted, int nDeleted, int nRestyled, const char* deletedText)
{
	if (execute == 0 || blockCB == 1) return; 

	blockCB = 1;


	Fl_Text_Buffer * scriptBuffer = scriptUI->scriptEditor->buffer();
	if (pos + nInserted == scriptBuffer->length())
	{	// add to end
		if (nInserted > 0)
		{
			const char * insertedText = scriptBuffer->text_range(pos, pos + nInserted);
			Stringc newText = Stringc(insertedText);
			free((void *) insertedText);
			int newlinePos = newText.search_for_substring('\n');

			if (newlinePos != -1)
			{	// need to execute
				int lineStart = scriptBuffer->line_start(pos);
				const char * execText = scriptBuffer->text_range(lineStart, pos+nInserted);
				parseScript(Stringc(execText));
				free((void *) execText);
			}

		}
	} 
	else if (nInserted == 1 && scriptBuffer->char_at(pos) == '\n')
	{
		const char * execText = scriptBuffer->line_text(pos);
		parseScript(Stringc(execText));
		free((void *) execText);
	}
	scriptUI->scriptEditor->take_focus();
	blockCB = 0;
}

void ScriptMgr::staticScriptCB(int pos, int nInserted, int nDeleted, int nRestyled, const char* deletedText, void* cbArg)
{
		((ScriptMgr*)cbArg)->scriptCB( pos, nInserted, nDeleted, nRestyled, deletedText );
}

void ScriptMgr::timeoutHandler()
{
	Fl_Text_Buffer * scriptBuffer = scriptUI->scriptEditor->buffer();
	parseScript(scriptBuffer->text());
	Fl::repeat_timeout(timerTime, staticTimeoutHandler, this);
}

void ScriptMgr::staticTimeoutHandler(void *data)
{
	((ScriptMgr*)data)->timeoutHandler();
}

void ScriptMgr::setTimeout(float t, char * filename)
{ 
	timerTime = t;
	scriptUI->scriptEditor->buffer()->loadfile(filename);
}

//////////////////////////////////////////////////////////////////////////
void ScriptMgr::appendError(const char * text)			
{ 	
	if (scriptMode == SCRIPT)
	{
		printf(text);
	}
	else
	{
		if (scriptOutUI->UIWindow->visible() == 0 && !quiet)
			scriptOutUI->UIWindow->show();

		scriptOutUI->errorDisplay->buffer()->append(text); 
		scrollEnd(scriptOutUI->errorDisplay); 
	}
}

void ScriptMgr::addError(const char * error)
{
	sprintf(str, "%s\n", error);
	appendError(str);
}

void ScriptMgr::addError(const char * type, const char * error, const char * cmd)
{
	sprintf(str, "%s: %s (%s)\n", type, error, cmd);
	appendError(str);
}

void ScriptMgr::addArgError(const char * cmd)
{
	addError("ERROR", "Missing Arguments", cmd);
}

void ScriptMgr::addIncError(const char * cmd)
{
	addError("ERROR", "Command incompatible with selected geometry", cmd);
}

//////////////////////////////////////////////////////////////////////////
void ScriptMgr::appendScript(const char * text)
{ 
	blockCB = 1;
	scriptUI->scriptEditor->buffer()->append(text); 
	scrollEnd(scriptUI->scriptEditor); 
	blockCB = 0;
}


void ScriptMgr::addLine(const char * command) 
{
	if (record) 
	{
		sprintf(str, "%s\n", command);
		appendScript(str);
	}
}

void ScriptMgr::addLineDouble(const char * command, double d) 
{
	if (record) 
	{
		sprintf(str, "%s %0.4f\n", command, d);
		appendScript(str);
	}
}

void ScriptMgr::addLineInt(const char * command, double d) 
{
	if (record) 
	{
		sprintf(str, "%s %1.0f\n", command, d);
		appendScript(str);
	}
}

void ScriptMgr::addLine(const char * group, const char * command, double v1, double v2, double v3)
{
	if (record) 
	{
		sprintf(str, "%s %s %0.4f %0.4f %0.4f\n", group, command, v1, v2, v3);
		appendScript(str);
	}
}


void ScriptMgr::addLine(const char * command, const char * v1) 
{
	if (record) 
	{
		sprintf(str, "%s %s\n", command, v1);
		appendScript(str);
	}
}


void ScriptMgr::addLine(const char * group, const char * command, const char * v1)
{
	if (record) 
	{
		sprintf(str, "%s %s %s\n", group, command, v1);
		appendScript(str);
	}

}
void ScriptMgr::addLine(const char * group, const char * command, const char * v1, const char * v2)
{
	if (record) 
	{
		sprintf(str, "%s %s %s %s\n", group, command, v1, v2);
		appendScript(str);
	}

}

void ScriptMgr::addLine(const char * group, const char * command, int v1, int v2, int v3)
{
	if (record) 
	{
		sprintf(str, "%s %s %d %d %d\n", group, command, v1, v2, v3);
		appendScript(str);
	}
}

void ScriptMgr::addLine(const char * group, const char * command, int v1)
{
	if (record) 
	{
		sprintf(str, "%s %s %d\n", group, command, v1);
		appendScript(str);
	}
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

void ScriptMgr::generateWordVec(Stringc line, deque< Stringc > * returnVec)
{
	int i, m;
	static char max_word[512];

	returnVec->clear();
	line.concatenate(' ');
	int qcounter = 0;
	int word_start = 0;
	int word_end = 0;
	for (i = 0; i < line.get_length(); i++)
	{
		if (line[i] == '\"')
		{
			qcounter++;
		}
		else if (line[i] == '\t' || line[i] == ' ')
		{	// found new word
			word_end = i-1;
			if (qcounter%2 == 1) continue;

			// trim quotes
			while (line[word_start] == '\"') word_start++;
			while (line[word_end] == '\"') word_end--;

			int len = word_end - word_start +1;
			if (len > 0)
			{
				for (m = 0; m < len; m++)
				{
					if (m >= 512) break;
					max_word[m] = line[word_start+m];
				}
				max_word[m] = '\0';

				returnVec->push_back(Stringc(max_word));
//				char str[100];
//				sprintf(str, "*%s*", max_word);
//				addError(str);
			}
			word_start = i+1;
		}
	}
}


void ScriptMgr::generateLineVec(Stringc script, deque< Stringc > * returnVec)
{
	int i, m;
	static char max_line[4096];

	returnVec->clear();
	script.concatenate('\n');
	int line_start = 0;
	for (i = 0; i < script.get_length(); i++)
	{
		if (script[i] == '\n' || script[i] == '\r')
		{	// found newline
			int len = i - line_start;
			if (len > 0)
			{
				for (m = 0; m < len; m++)
				{
					if (m >= 4096) break;
					max_line[m] = script[line_start+m];
				}
				max_line[m] = '\0';

				returnVec->push_back(Stringc(max_line));
//				char str[100];
//				sprintf(str, "|%s|", max_line);
//				addError(str);
			}
			line_start = i+1;
		}
	}
}






void ScriptMgr::parseScript(Stringc script) {
	deque<Stringc> lineVec;
	generateLineVec(script, &lineVec);

	Stringc output;
	for (int i = 0; i < (int)lineVec.size(); i++) {
		parseLine(lineVec[i]);
	}
}

void ScriptMgr::parseLine(Stringc line) {
	deque<Stringc> wordVec;
	generateWordVec(line, &wordVec);

	if (wordVec.size() > 0) {

		if (wordVec[0] == "gen")
		{	//gen
			parseGen(wordVec, line);
		}
		else if (wordVec[0] == "xform")
		{	//xform
			parseXform(wordVec, line);
		}
		else if (wordVec[0] == "pod_design")
		{	//pod
			parsePodDesign(wordVec, line);
		}
		else if (wordVec[0] == "wing_plan")
		{	//wing
			parseWingPlan(wordVec, line);
		}
		else if (wordVec[0] == "wing_sect")
		{
			parseWingSect(wordVec, line);
		}
		else if (wordVec[0] == "wing_dihed")
		{
			parseWingDihed(wordVec, line);
		}
		else if (wordVec[0] == "wing_foil")
		{
			parseWingFoil(wordVec, line);
		}
		else if (wordVec[0] == "fuse_shape")
		{	//fuselage
			parseFuseShape(wordVec, line);
		}
		else if (wordVec[0] == "fuse_xsec")
		{
			parseFuseXSec(wordVec, line);
		}
		else if (wordVec[0] == "fuse_iml")
		{
			parseFuseIml(wordVec, line);
		}
		else if (wordVec[0] == "havoc_plan")
		{	//havoc
			parseHavocPlan(wordVec, line);
		}
		else if (wordVec[0] == "havoc_xsec")
		{
			parseHavocXSec(wordVec, line);
		}
		else if (wordVec[0] == "havoc_side")
		{
			parseHavocSide(wordVec, line);
		}
		else if (wordVec[0] == "ext_design")
		{	//ext
			parseExtDesign(wordVec, line);
		}
		else if (wordVec[0] == "duct_shape")
		{	//duct
			parseDuctShape(wordVec, line);
		}
		else if (wordVec[0] == "duct_foil")
		{
			parseDuctFoil(wordVec, line);
		}
		else if (wordVec[0] == "prop_shape")
		{	//prop
			parsePropShape(wordVec, line);
		}
		else if (wordVec[0] == "prop_station")
		{
			parsePropStation(wordVec, line);
		}
		else if (wordVec[0] == "prop_foil")
		{
			parsePropFoil(wordVec, line);
		}
		else if (wordVec[0] == "engine_parm")
		{	// engine
			parseEngineDesign(wordVec, line);
		}
		else if (wordVec[0] == "engine_nozzle")
		{
			parseEngineDesign(wordVec, line);
		}
		else if (wordVec[0] == "engine_duct")
		{
			parseEngineDesign(wordVec, line);
		}
		else if (wordVec[0] == "engine_inlet")
		{
			parseEngineInlet(wordVec, line);
		}
		else if (wordVec[0] == "label")
		{
			parseLabel(wordVec, line);
		}
		else if (wordVec[0] == "text")
		{
			parseText(wordVec, line);
		}
		else if (wordVec[0] == "ruler")
		{
			parseRuler(wordVec, line);
		}
		else
		{
			parseCommand(wordVec, line);
		}

		if (scriptMode != SCRIPT)
		{	// gui update
			screenMgr->updateGeomScreens(); // excessive updating . . .
			screenMgr->getLabelScreen()->update(); // excessive updating . . .
			aircraft->triggerDraw();
		}
	}
}

void ScriptMgr::parseCommand(deque<Stringc> &wordVec, Stringc command) 
{

	GeomScreen * geomScreen = screenMgr->getGeomScreen();

	if (wordVec.size() == 0) return;// redundant

	if (wordVec[0] == "new")
	{	// NEW
		screenMgr->s_new(scriptMode); // new
	}
	else if (wordVec[0] == "open")
	{	// OPEN
		if (wordVec.size() == 1)
		{	// open
			if (scriptMode == SCRIPT)
				addError("WARNING: filename required for open command (i.e. \"open plane.vsp\")\n");
			else
				screenMgr->s_open(scriptMode, NULL);	// open dialog
		}
		else
		{	// open <string>
			screenMgr->s_open(scriptMode, wordVec[1]);	// open file
		}
	}
	else if (wordVec[0] == "save")
	{	// SAVE
		if (wordVec.size() == 1) 
		{
			screenMgr->s_save(scriptMode);
		}
		else if (wordVec.size() >= 2)
		{	// save ...
			if (wordVec[1] == "-sel")
			{	// save -sel <string>
				if (wordVec.size() >= 3)
					screenMgr->s_savesel(scriptMode, wordVec[2]);
				else
					addArgError("WARNING: filename required for save -sel command (i.e. \"save -sel plane.vsp\")\n");
			}
			else
			{	// save <string>
				printf("saving file %s (%d)\n", wordVec[1](), scriptMode);
				screenMgr->s_saveas(scriptMode, wordVec[1]);
			}				
		}
	}
	else if (wordVec[0] == "insert")
	{	// INSERT
		if (wordVec.size() == 1) 
		{	// insert
			if (scriptMode == SCRIPT)
				addError("WARNING: filename required for insert command (i.e. \"insert plane.vsp\")\n");
			else
				screenMgr->s_insert(scriptMode, wordVec[1]);
		}
		else
		{	// insert <string>
				screenMgr->s_insert(scriptMode, wordVec[1]);
		}
	}
	else if (wordVec[0] == "import")
	{	// IMPORT
		if (wordVec.size() == 1) 
		{	// import
			if (scriptMode == SCRIPT)
				addError("WARNING: filename and type required for import command (i.e. \"import -xsec plane.xsec\")\n");
			else
				screenMgr->s_import(scriptMode, NULL, -1);
		}
		else
		{	// import... 
			if (wordVec.size() >= 3)
			{
				if (wordVec[1] == "-felisa")
					screenMgr->s_import(scriptMode, wordVec[2], Aircraft::FELISA);
				else if (wordVec[1] == "-vorxsec")
					screenMgr->s_import(scriptMode, wordVec[2], Aircraft::VORXSEC);
				else if (wordVec[1] == "-xsec")
					screenMgr->s_import(scriptMode, wordVec[2], Aircraft::XSEC);
				else if (wordVec[1] == "-stereo")
					screenMgr->s_import(scriptMode, wordVec[2], Aircraft::STEREOLITH);
				else if (wordVec[1] == "-rhino")
					screenMgr->s_import(scriptMode, wordVec[2], Aircraft::RHINO3D);
				else if (wordVec[1] == "-nascart")
					screenMgr->s_import(scriptMode, wordVec[2], Aircraft::NASCART);
			}
			else
			{
				addError("WARNING: filename and type required for import command (i.e. \"import -xsec plane.xsec\")\n");
			}
		}
	}
	else if (wordVec[0] == "export")
	{	// EXPORT
		if (wordVec.size() == 1) 
		{	// export
			if (scriptMode == SCRIPT)
				addError("WARNING: filename and type required for export command (i.e. \"export -xsec plane.xsec\")\n");
			else
				screenMgr->s_export(scriptMode, NULL, -1);
		}
		else
		{	// export... 
			if (wordVec.size() >= 3)
			{
				if (wordVec[1] == "-felisa")
					screenMgr->s_export(scriptMode, wordVec[2], Aircraft::FELISA);
				else if (wordVec[1] == "-xsec")
					screenMgr->s_export(scriptMode, wordVec[2], Aircraft::XSEC);
				else if (wordVec[1] == "-stereo")
					screenMgr->s_export(scriptMode, wordVec[2], Aircraft::STEREOLITH);
				else if (wordVec[1] == "-rhino")
					screenMgr->s_export(scriptMode, wordVec[2], Aircraft::RHINO3D);
				else if (wordVec[1] == "-nascart") {
					screenMgr->s_export(scriptMode, wordVec[2], Aircraft::NASCART);
				}
				else
					addError("WARNING", "Unknown export type", command);
			}
			else
			{
				addError("WARNING: filename and type required for export command (i.e. \"export -xsec plane.xsec\")\n");
			}
		}
	}
	else if (wordVec[0] == "add")
	{
		if (wordVec.size() == 1)
		{
			addError("WARNING: type argument required for add command (i.e.\"add wing\")\n");
		}
		else
		{
			Stringc name("");
			if (wordVec.size() >= 3)
				name = wordVec[2];

			if (wordVec[1] == "pod")
				geomScreen->s_add_geom(scriptMode, POD_GEOM_TYPE, name);
			else if (wordVec[1] == "fuse")
				geomScreen->s_add_geom(scriptMode, FUSE_GEOM_TYPE, name);
			else if (wordVec[1] == "havoc")
				geomScreen->s_add_geom(scriptMode, HAVOC_GEOM_TYPE, name);
			else if (wordVec[1] == "ext")
				geomScreen->s_add_geom(scriptMode, EXT_GEOM_TYPE, name);
			else if (wordVec[1] == "wing")
				geomScreen->s_add_geom(scriptMode, MS_WING_GEOM_TYPE, name);
			else if (wordVec[1] == "blank")
				geomScreen->s_add_geom(scriptMode, BLANK_GEOM_TYPE, name);
			else if (wordVec[1] == "duct")
				geomScreen->s_add_geom(scriptMode, DUCT_GEOM_TYPE, name);
			else if (wordVec[1] == "prop")
				geomScreen->s_add_geom(scriptMode, PROP_GEOM_TYPE, name);
			else if (wordVec[1] == "engine")
				geomScreen->s_add_geom(scriptMode, ENGINE_GEOM_TYPE, name);
			else if (wordVec[1] == "fuselage")
				geomScreen->s_add_geom(scriptMode, FUSELAGE_GEOM_TYPE, name);
			else 
				addError("WARNING", "Unable to add unknown type", wordVec[1]);
		}
	}
	else if (wordVec[0] == "cut")
	{	// CUT
		wordVec.pop_front();
		geomScreen->s_cut(scriptMode, wordVec);
	}
	else if (wordVec[0] == "copy")
	{	// COPY
		wordVec.pop_front();
		geomScreen->s_copy(scriptMode, wordVec);
	}
	else if (wordVec[0] == "paste")
	{	// PASTE
		geomScreen->s_paste(scriptMode);
	}
	else if (wordVec[0] == "select")
	{	// SELECT
		if (wordVec.size() >= 2)
		{
			if (wordVec[1] == "all")
			{
				geomScreen->s_select_all(scriptMode);
			}
			else if (wordVec[1] == "none")
			{
				geomScreen->s_select_none(scriptMode);
			}
			else
			{
				wordVec.pop_front();
				geomScreen->s_select(scriptMode, wordVec);
			}
		}
		else
		{
			addError("WARNING: partname not specified (i.e.\"select wing_01\")\n");
		}
	}
	else if (wordVec[0] == "compgeom")
	{
		screenMgr->s_compgeom(scriptMode);
	}
	else if (wordVec[0] == "meshgeom")
	{
		screenMgr->s_meshgeom(scriptMode);
	}
	else if (wordVec[0] == "massprop")
	{
		int slices = wordVec[1].convert_to_integer();
		screenMgr->s_massprop(scriptMode, slices );
	}
	else if (wordVec[0] == "slice")
	{
		if (wordVec.size() == 2)
		{
			screenMgr->s_oldslice(scriptMode, wordVec[1].convert_to_integer());
		}
		else if (wordVec.size() >= 5)
		{
			int style = 0;
			if (wordVec[1] == "plane")
				style = MeshGeom::SLICE_PLANAR;
			else if (wordVec[1] == "cone")
				style = MeshGeom::SLICE_CONIC;
			else if (wordVec[1] == "awave")
				style = MeshGeom::SLICE_AWAVE;

			int slices = wordVec[2].convert_to_integer();
			double angle = wordVec[3].convert_to_double();
			int sections = wordVec[4].convert_to_integer();

			if (wordVec.size() >= 6)
				screenMgr->s_awave(scriptMode, style, slices, angle, sections, wordVec[5]);
			else
				screenMgr->s_awave(scriptMode, style, slices, angle, sections, "slice.txt");
		}
		else addArgError(command);
	}
	else
	{
		addError("ERROR", "Unknown Command", command);
	}
}




void ScriptMgr::setParm(GeomBase * geomPtr, Parm * parm, int index, deque<Stringc> &wordVec, Stringc &command)
{
	if ((int)wordVec.size() >= index+1)
	{
		parm->set(getDouble((*parm)(), wordVec[index]));
		geomPtr->parm_changed( parm );
	}
	else addArgError(command);
}

double ScriptMgr::getDouble(double d, Stringc modifier)
{
	if (modifier.get_length() > 0)
	{
		double value;
		switch (modifier[0])
		{
			case '+':
				modifier.remove_leading('+');
				return d + modifier.convert_to_double();
//			case '-':
//				modifier.remove_leading('-');
//				return d - modifier.convert_to_double();
			case '*':
				modifier.remove_leading('*');
				return d * modifier.convert_to_double();
			case '/':
				modifier.remove_leading('/');
				value = modifier.convert_to_double();
				return (value == 0) ? d : d/value;
			default:
				return modifier.convert_to_double();
		}
	}
	else
	{
		return d;
	}
}

int ScriptMgr::getInteger(int i, Stringc modifier)
{
	if (modifier.get_length() > 0)
	{
		switch (modifier[0])
		{
			case '+':
				modifier.remove_leading('+');
				return i + modifier.convert_to_integer();
//			case '-':
//				modifier.remove_leading('-');
//				return i - modifier.convert_to_integer();
			case '*':
				modifier.remove_leading('*');
				return i * modifier.convert_to_integer();
			default:
				return modifier.convert_to_integer();
		}
	}
	else
	{
		return i;
	}
}



void ScriptMgr::parseGen(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	vector < Geom * > geomVec = aircraft->getActiveGeomVec();

	if (geomVec.size() == 0)
	{	// nothing selected
		addError("ERROR: geometry not selected\n");
		return;
	}


	if (wordVec[1] == "color")
	{	// COLOR
		if (wordVec.size() >= 5)
		{
			for (int i = 0; i < (int)geomVec.size(); i++)
				geomVec[i]->setColor(wordVec[2].convert_to_integer(), wordVec[3].convert_to_integer(), wordVec[4].convert_to_integer());
		}
		else addArgError(command);

	}
	else if (wordVec[1] == "material")
	{	// MATERIAL
		if (wordVec.size() >= 3)
		{
			Stringc material = wordVec[2];
			material.trim('\"');
			for (  int i = 0 ; i < matMgrPtr->getNumMaterial() ; i++ )
			{
				Material* mat = matMgrPtr->getMaterial(i);
				if ( mat && mat->name == material)
				{
					for (int j = 0; j < (int)geomVec.size(); j++)
						geomVec[j]->setMaterialID(i);
					break;
				}
				
			}
		}
		else addArgError(command);
	}
	else if (geomVec.size() == 1)
	{	// single geometry variables
		Geom * geomPtr = aircraft->getActiveGeom();

		if (wordVec[1] == "name")
		{	// NAME
			if (wordVec.size() >= 3)
			{
				Stringc name = wordVec[2];
				name.trim('\"');
				geomPtr->setName(name);
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "attach")
		{	// ATTACH
			if (wordVec.size() >= 3)
			{
				if (wordVec[2] == "fixed")
					geomPtr->setPosAttachFlag( POS_ATTACH_FIXED );
				else if (wordVec[2] == "uv")
					geomPtr->setPosAttachFlag( POS_ATTACH_UV );
				else if (wordVec[2] == "matrix")
					geomPtr->setPosAttachFlag( POS_ATTACH_MATRIX );
				else if (wordVec[2] == "none")
					geomPtr->setPosAttachFlag( POS_ATTACH_NONE );
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "setu")
		{	// SET U
			setParm(geomPtr, &geomPtr->uAttach, 2, wordVec, command);
		}
		else if (wordVec[1] == "setv")
		{	// SET V
			setParm(geomPtr, &geomPtr->vAttach, 2, wordVec, command);
		}
		else if (wordVec[1] == "setuv")
		{	// SET UV
			if (wordVec.size() >= 4)
			{
				setParm(geomPtr, &geomPtr->uAttach, 2, wordVec, command);
				setParm(geomPtr, &geomPtr->vAttach, 3, wordVec, command);
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "tessxsec")
		{	// TESSELATE XSEC
			setParm(geomPtr, &geomPtr->numXsecs, 2, wordVec, command);
		}
		else if (wordVec[1] == "tesspoints")
		{	// TESSALATE POINTS
			setParm(geomPtr, &geomPtr->numPnts, 2, wordVec, command);
		}
		else if (wordVec[1] == "pointspacing")
		{	// POINT SPACING
			if (geomPtr->getType() == FUSE_GEOM_TYPE)
			{
				if (wordVec.size() >= 3)
				{
					if (wordVec[2] == "xsec")
						((Fuse_geom*)geomPtr)->set_pnt_space_type( 0 );
					else if (wordVec[2] == "fixed")
						((Fuse_geom*)geomPtr)->set_pnt_space_type( 1 );
					else if (wordVec[2] == "uniform")
						((Fuse_geom*)geomPtr)->set_pnt_space_type( 2 );
					else
						addError("ERROR", "unknown point spacing value", command);
				} else addArgError(command);

			}
			else  addError("ERROR", "Point spacing can only be applied to fueselage geometry", command);
		}
		else
		{
			addError("ERROR", "Unknown command", command);
		}
	}
	else
	{
		addError("ERROR", "Command unknown or incompatible with multiple geometries", command);
	}

}


void ScriptMgr::parseXform(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;

	vector < Geom * > geomVec = aircraft->getActiveGeomVec();


	if (geomVec.size() == 0)
	{	// nothing selected
		addError("ERROR: geometry not selected\n");
		return;
	}

	if (wordVec[1] == "loc")
	{	// LOCATION
		if (wordVec.size() >= 5)
		{
			for (int i = 0; i < (int)geomVec.size(); i++)
			{
				setParm(geomVec[i], &geomVec[i]->xLoc, 2, wordVec, command);
				setParm(geomVec[i], &geomVec[i]->yLoc, 3, wordVec, command);
				setParm(geomVec[i], &geomVec[i]->zLoc, 4, wordVec, command);
			}
		}
		else addArgError(command);
	}
	else if (wordVec[1] == "xloc")
	{	// X LOCATION
		if (wordVec.size() >= 3)
		{
			for (int i = 0; i < (int)geomVec.size(); i++)
			{
				setParm(geomVec[i], &geomVec[i]->xLoc, 2, wordVec, command);
			}
		}
		else addArgError(command);
	}
	else if (wordVec[1] == "yloc")
	{	// Y LOCATION
		if (wordVec.size() >= 3)
		{
			for (int i = 0; i < (int)geomVec.size(); i++)
			{
				setParm(geomVec[i], &geomVec[i]->yLoc, 2, wordVec, command);
			}
		}
		else addArgError(command);
	}
	else if (wordVec[1] == "zloc")
	{	// Z LOCATION
		if (wordVec.size() >= 3)
		{
			for (int i = 0; i < (int)geomVec.size(); i++)
			{
				setParm(geomVec[i], &geomVec[i]->zLoc, 2, wordVec, command);
			}
		}
		else addArgError(command);
	}
	else if (wordVec[1] == "rot")
	{	// ROTATION
		if (wordVec.size() >= 5)
		{
			for (int i = 0; i < (int)geomVec.size(); i++)
			{
				setParm(geomVec[i], &geomVec[i]->xRot, 2, wordVec, command);
				setParm(geomVec[i], &geomVec[i]->yRot, 3, wordVec, command);
				setParm(geomVec[i], &geomVec[i]->zRot, 4, wordVec, command);
			}
		}
		else addArgError(command);
	}
	else if (wordVec[1] == "xrot")
	{	// X ROTATION
		if (wordVec.size() >= 3)
		{
			for (int i = 0; i < (int)geomVec.size(); i++)
			{
				setParm(geomVec[i], &geomVec[i]->xRot, 2, wordVec, command);
			}
		}
		else addArgError(command);
	}
	else if (wordVec[1] == "yrot")
	{	// Y ROTATION
		if (wordVec.size() >= 3)
		{
			for (int i = 0; i < (int)geomVec.size(); i++)
			{
				setParm(geomVec[i], &geomVec[i]->yRot, 2, wordVec, command);
			}
		}
		else addArgError(command);
	}
	else if (wordVec[1] == "zrot")
	{	// Z ROTATION
		if (wordVec.size() >= 3)
		{
			for (int i = 0; i < (int)geomVec.size(); i++)
			{
				setParm(geomVec[i], &geomVec[i]->zRot, 2, wordVec, command);
			}
		}
		else addArgError(command);
	}
	else if (wordVec[1] == "scale")
	{	// SCALE
		if (wordVec.size() >= 2)
		{
			for (int i = 0; i < (int)geomVec.size(); i++)
			{
				setParm(geomVec[i], &geomVec[i]->scaleFactor, 2, wordVec, command);
				geomVec[i]->acceptScaleFactor();
			}
		}
		else addArgError(command);
	}
	else if (geomVec.size() == 1)
	{	// single geometry variables
		Geom * geomPtr = aircraft->getActiveGeom();

		if (wordVec[1] == "position")
		{	// ABSOLUTE/RELATIVE POSITIONING
			if (wordVec.size() >= 3)
			{
				if (wordVec[2] == "abs")
					geomPtr->setRelXFormFlag( 0 );
				else if (wordVec[2] == "rel")
					geomPtr->setRelXFormFlag( 0 );
				else addError("ERROR", "Unknown position parameter", command);
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "rotorig")
		{	// ROTATE ORIGIN
			if (wordVec.size() >= 3)
			{
				geomPtr->origin.set(getDouble(geomPtr->origin(), wordVec[2]));
				geomPtr->parm_changed( &(geomPtr->origin) );
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "symmetry")
		{	// SYMMETRY
			if (wordVec.size() >= 3)
			{
				if (wordVec[2] == "none")
					geomPtr->setSymCode( NO_SYM );
				if (wordVec[2] == "xy")
					geomPtr->setSymCode( XY_SYM );
				else if (wordVec[2] == "xz")
					geomPtr->setSymCode( XZ_SYM );
				else if (wordVec[2] == "yz")
					geomPtr->setSymCode( YZ_SYM );
				else addError("ERROR", "Unknown symmetry parameter", command);
			}
			else addArgError(command);
		}
		else addError("ERROR", "Unknown Command", command);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);
}



void ScriptMgr::parsePodDesign(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == POD_GEOM_TYPE)
	{
		PodGeom * podPtr = (PodGeom*) geomPtr;
		if (wordVec[1] == "length")
		{	// LENGTH
			setParm(podPtr, &podPtr->length, 2, wordVec, command);
		}
		else if (wordVec[1] == "fineratio")
		{	// FINE RATIO
			setParm(podPtr, &podPtr->fine_ratio, 2, wordVec, command);
		}
		else addError("ERROR", "Unknown Command", command);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);
}

int ScriptMgr::verifySingleGeom(Stringc command)
{
	vector < Geom * > geomVec = aircraft->getActiveGeomVec();

	if (geomVec.size() == 0)
	{	// nothing selected
		addError("ERROR: geometry not selected\n");
		return 0;
	}
	else if (geomVec.size() > 1)
	{	// multiple selected
		addError("ERROR", "Command incompatible with multiple geometries", command);
		return 0;
	}
	return 1;
}


void ScriptMgr::parseFuseShape(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	FuseScreen * fuseScreen = screenMgr->getFuseScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == FUSE_GEOM_TYPE)
	{
		Fuse_geom * fusePtr = (Fuse_geom*) geomPtr;
		fuse_xsec * fxsecPtr = fusePtr->get_curr_xsec();

		if (wordVec[1] == "length")
		{	// LENGTH
			setParm(fusePtr, fusePtr->get_length(), 2, wordVec, command);
		}
		else if (wordVec[1] == "camber")
		{	// CAMBER
			setParm(fusePtr, fusePtr->get_camber(), 2, wordVec, command);
		}
		else if (wordVec[1] == "camberloc")
		{	// CAMBER LOC
			setParm(fusePtr, fusePtr->get_camber_loc(), 2, wordVec, command);
		}
		else if (wordVec[1] == "aftoffset")
		{	// AFT OFFSET
			setParm(fusePtr, fusePtr->get_aft_offset(), 2, wordVec, command);
		}
		else if (wordVec[1] == "noseangle")
		{	// NOSE ANGLE
			setParm(fusePtr, fusePtr->get_nose_angle(), 2, wordVec, command);
		}
		else if (wordVec[1] == "nosestr")
		{	// NOSE STR
			setParm(fusePtr, fusePtr->get_nose_str(), 2, wordVec, command);
		}
		else if (wordVec[1] == "noserho")
		{	// NOSE RHO
			setParm(fusePtr, fusePtr->get_nose_rho(), 2, wordVec, command);
		}
		else if (wordVec[1] == "aftrho")
		{	// AFT RHO
			setParm(fusePtr, fusePtr->get_aft_rho(), 2, wordVec, command);
		}
		else if (wordVec[1] == "xsec_dec_num_all")
		{	// DEC NUM XSEC ALL
			fuseScreen->s_xsec_dec_num_all(scriptMode);
		}
		else if (wordVec[1] == "xsec_inc_num_all")
		{	// INC NUM XSEC ALL
			fuseScreen->s_xsec_inc_num_all(scriptMode);
		}
		else if (wordVec[1] == "xsec_nose_rho_on")
		{	// NOSE RHO ON/OFF
			if (wordVec.size() >= 3)
			{
				int val = wordVec[2].convert_to_integer();
				fuseScreen->s_xsec_nose_rho_on(scriptMode, val);
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "xsec_aft_rho_on")
		{	// AFT RHO ON/OFF
			if (wordVec.size() >= 3)
			{
				int val = wordVec[2].convert_to_integer();
				fuseScreen->s_xsec_aft_rho_on(scriptMode, val);
			}
			else addArgError(command);
		}
		else addError("ERROR", "Unknown Command", command);

		fuseScreen->show(fusePtr);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);
}


void ScriptMgr::parseFuseXSec(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	FuseScreen * fuseScreen = screenMgr->getFuseScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == FUSE_GEOM_TYPE)
	{
		Fuse_geom * fusePtr = (Fuse_geom*) geomPtr;
		fuse_xsec * fxsecPtr = fusePtr->get_curr_xsec();

		if (wordVec[1] == "id")
		{	// ID
			if (wordVec.size() >= 3)
			{
				int id = wordVec[2].convert_to_integer();
				fuseScreen->s_set_id(scriptMode, id);
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "location")
		{	// LOCATION
			setParm(fxsecPtr, fxsecPtr->get_location(), 2, wordVec, command);
		}
		else if (wordVec[1] == "zoffset")
		{	// ZOFFSET
			setParm(fxsecPtr, fxsecPtr->get_z_offset(), 2, wordVec, command);
		}
		else if (wordVec[1] == "add")
		{	// ADD
			fuseScreen->s_xsec_add(scriptMode);
		}
		else if (wordVec[1] == "cut")
		{	// CUT
			fuseScreen->s_xsec_cut(scriptMode);
		}
		else if (wordVec[1] == "copy")
		{	// COPY
			fuseScreen->s_xsec_copy(scriptMode);
		}
		else if (wordVec[1] == "paste")
		{	// PASTE
			fuseScreen->s_xsec_paste(scriptMode);
		}
		else if (wordVec[1] == "xsec_num_interp1")
		{	// NUM INTERP 1
			if (wordVec.size() >= 3)
			{
				int num = wordVec[2].convert_to_integer();
				fuseScreen->s_xsec_num_interp1(scriptMode, num);
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "xsec_num_interp2")
		{	// NUM INTERP 2
			if (wordVec.size() >= 3)
			{
				int num = wordVec[2].convert_to_integer();
				fuseScreen->s_xsec_num_interp2(scriptMode, num);
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "type")
		{	// TYPE
			if (wordVec.size() >= 3)
			{
				if (wordVec[2] == "point")
					fuseScreen->s_type(scriptMode, XSEC_POINT);
				else if (wordVec[2] == "circle")
					fuseScreen->s_type(scriptMode, CIRCLE);
				else if (wordVec[2] == "ellipse")
					fuseScreen->s_type(scriptMode, ELLIPSE);
				else if (wordVec[2] == "roundbox")
					fuseScreen->s_type(scriptMode, RND_BOX);
				else if (wordVec[2] == "general")
					fuseScreen->s_type(scriptMode, GENERAL);
				else if (wordVec[2] == "file")
					fuseScreen->s_type(scriptMode, FROM_FILE);
				else if (wordVec[2] == "edit")
					fuseScreen->s_type(scriptMode, EDIT_CRV);
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "height")
		{	// HEIGHT
			setParm(fxsecPtr, fxsecPtr->get_height(), 2, wordVec, command);
		}
		else if (wordVec[1] == "width")
		{	// WIDTH
			setParm(fxsecPtr, fxsecPtr->get_width(), 2, wordVec, command);
		}
		else if (wordVec[1] == "widthoffset")
		{	// WIDTH OFFSET
			setParm(fxsecPtr, fxsecPtr->get_max_width_loc(), 2, wordVec, command);
		}
		else if (wordVec[1] == "cornerrad")
		{	// CORNER RADIUS
			setParm(fxsecPtr, fxsecPtr->get_corner_rad(), 2, wordVec, command);
		}
		else if (wordVec[1] == "toptanang")
		{	// TOP TAN ANGLE
			setParm(fxsecPtr, fxsecPtr->get_top_tan_angle(), 2, wordVec, command);
		}
		else if (wordVec[1] == "bottanang")
		{	// BOTTOM TAN ANGLE
			setParm(fxsecPtr, fxsecPtr->get_bot_tan_angle(), 2, wordVec, command);
		}
		else if (wordVec[1] == "alltanstr")
		{	// ALL TAN STRENGTH
			fxsecPtr->setAllTanStrFlag( 1 );
			setParm(fxsecPtr, fxsecPtr->get_top_str(), 2, wordVec, command);
		}
		else if (wordVec[1] == "toptanstr")
		{	// TOP TAN STRENGTH
			fxsecPtr->setAllTanStrFlag( 0 );
			setParm(fxsecPtr, fxsecPtr->get_top_str(), 2, wordVec, command);
		}
		else if (wordVec[1] == "upcornertanstr")
		{	// UP CORNER TAN STRENGTH
			fxsecPtr->setAllTanStrFlag( 0 );
			setParm(fxsecPtr, fxsecPtr->get_upp_str(), 2, wordVec, command);
		}
		else if (wordVec[1] == "lowcornertanstr")
		{	// LOW CORNER TAN STRENGTH
			fxsecPtr->setAllTanStrFlag( 0 );
			setParm(fxsecPtr, fxsecPtr->get_low_str(), 2, wordVec, command);
		}
		else if (wordVec[1] == "bottanstr")
		{	// BOTTOM TAN STRENGTH
			fxsecPtr->setAllTanStrFlag( 0 );
			setParm(fxsecPtr, fxsecPtr->get_bot_str(), 2, wordVec, command);
		}
		else if (wordVec[1] == "profiletanstr1")
		{	// PROFILE TAN STR 1
			setParm(fxsecPtr, fxsecPtr->getProfileTanStr1(), 2, wordVec, command);
		}
		else if (wordVec[1] == "profiletanstr2")
		{	// PROFILE TAN STR 2
			setParm(fxsecPtr, fxsecPtr->getProfileTanStr2(), 2, wordVec, command);
		}
		else if (wordVec[1] == "profiletanang")
		{	// PROFILE TAN ANG
			setParm(fxsecPtr, fxsecPtr->getProfileTanAng(), 2, wordVec, command);
		}
		else addError("ERROR", "Unknown Command", command);

		fuseScreen->show(fusePtr);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);

}




void ScriptMgr::parseFuseIml(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	FuseScreen * fuseScreen = screenMgr->getFuseScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == FUSE_GEOM_TYPE)
	{
		Fuse_geom * fusePtr = (Fuse_geom*) geomPtr;
		fuse_xsec * fxsecPtr = fusePtr->get_curr_xsec();

		if (wordVec[1] == "iml")
		{	// IML

		}

/*

fuse_iml iml <binary>
fuse_iml color <int> <int> <int>
fuse_iml material <string>
fuse_iml id <int>
fuse_iml walltop <+|-|*|/|double>
fuse_iml wallbot <+|-|*|/|double>
fuse_iml wallside <+|-|*|/|double>
fuse_iml modify <iml|oml>
*/

		else addError("ERROR", "Unknown Command", command);

		fuseScreen->show(fusePtr);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);

}




void ScriptMgr::parseHavocPlan(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	HavocScreen * havocScreen = screenMgr->getHavocScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == HAVOC_GEOM_TYPE)
	{
		Havoc_geom * havocPtr = (Havoc_geom*) geomPtr;

		if (wordVec[1] == "length")
		{	// LENGTH
			setParm(havocPtr, havocPtr->get_length(), 2, wordVec, command);
		}
		else if (wordVec[1] == "al")
		{	// AL
			setParm(havocPtr, havocPtr->get_al(), 2, wordVec, command);
		}
		else if (wordVec[1] == "ar")
		{	// AR
			setParm(havocPtr, havocPtr->get_ar(), 2, wordVec, command);
		}
		else if (wordVec[1] == "apm")
		{	// APM
			setParm(havocPtr, havocPtr->get_apm(), 2, wordVec, command);
		}
		else if (wordVec[1] == "apn")
		{	// APN
			setParm(havocPtr, havocPtr->get_apn(), 2, wordVec, command);
		}
		else if (wordVec[1] == "lpiovl")
		{	// LPIOVL
			setParm(havocPtr, havocPtr->get_lpiovl(), 2, wordVec, command);
		}
		else if (wordVec[1] == "pera")
		{	// PERA
			setParm(havocPtr, havocPtr->get_pera(), 2, wordVec, command);
		}
		else addError("ERROR", "Unknown Command", command);

		havocScreen->show(havocPtr);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);

}



void ScriptMgr::parseHavocXSec(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	HavocScreen * havocScreen = screenMgr->getHavocScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == HAVOC_GEOM_TYPE)
	{
		Havoc_geom * havocPtr = (Havoc_geom*) geomPtr;

		if (wordVec[1] == "mexp1")
		{	// M_EXP1
			setParm(havocPtr, havocPtr->get_mexp1(), 2, wordVec, command);
		}
		else if (wordVec[1] == "nexp1")
		{	// N_EXP1
			setParm(havocPtr, havocPtr->get_nexp1(), 2, wordVec, command);
		}
		else if (wordVec[1] == "mexp2")
		{	// M_EXP2
			setParm(havocPtr, havocPtr->get_mexp2(), 2, wordVec, command);
		}
		else if (wordVec[1] == "nexp2")
		{	// N_EXP2
			setParm(havocPtr, havocPtr->get_nexp2(), 2, wordVec, command);
		} 
		else if (wordVec[1] == "mexp3")
		{	// M_EXP3
			setParm(havocPtr, havocPtr->get_mexp3(), 2, wordVec, command);
		}
		else if (wordVec[1] == "nexp3")
		{	// N_EXP3
			setParm(havocPtr, havocPtr->get_nexp3(), 2, wordVec, command);
		}
		else if (wordVec[1] == "mexp4")
		{	// M_EXP4
			setParm(havocPtr, havocPtr->get_mexp4(), 2, wordVec, command);
		}
		else if (wordVec[1] == "nexp4")
		{	// N_EXP4
			setParm(havocPtr, havocPtr->get_nexp4(), 2, wordVec, command);
		}
		else addError("ERROR", "Unknown Command", command);

		havocScreen->show(havocPtr);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);

}


void ScriptMgr::parseHavocSide(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	HavocScreen * havocScreen = screenMgr->getHavocScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == HAVOC_GEOM_TYPE)
	{
		Havoc_geom * havocPtr = (Havoc_geom*) geomPtr;

		if (wordVec[1] == "pln")
		{	// PLN
			setParm(havocPtr, havocPtr->get_pln(), 2, wordVec, command);
		}
		else if (wordVec[1] == "ple")
		{	// PLE
			setParm(havocPtr, havocPtr->get_ple(), 2, wordVec, command);
		}
		else if (wordVec[1] == "bu")
		{	// BU
			setParm(havocPtr, havocPtr->get_bu(), 2, wordVec, command);
		}
		else if (wordVec[1] == "bl")
		{	// BL
			setParm(havocPtr, havocPtr->get_bl(), 2, wordVec, command);
		} 
		else if (wordVec[1] == "um")
		{	// UM
			setParm(havocPtr, havocPtr->get_aum(), 2, wordVec, command);
		}
		else if (wordVec[1] == "un")
		{	// UN
			setParm(havocPtr, havocPtr->get_aun(), 2, wordVec, command);
		}
		else if (wordVec[1] == "lm")
		{	// LM
			setParm(havocPtr, havocPtr->get_alm(), 2, wordVec, command);
		}
		else if (wordVec[1] == "ln")
		{	// LN
			setParm(havocPtr, havocPtr->get_aln(), 2, wordVec, command);
		}
		else if (wordVec[1] == "gum")
		{	// GUM
			setParm(havocPtr, havocPtr->get_gum(), 2, wordVec, command);
		}
		else if (wordVec[1] == "theta")
		{	// THETA
			setParm(havocPtr, havocPtr->get_theta(), 2, wordVec, command);
		}
		else if (wordVec[1] == "ptas")
		{	// PTAS
			setParm(havocPtr, havocPtr->get_ptas(), 2, wordVec, command);
		}
		else if (wordVec[1] == "bue")
		{	// BUE
			setParm(havocPtr, havocPtr->get_bue(), 2, wordVec, command);
		}
		else if (wordVec[1] == "ble")
		{	// BLE
			setParm(havocPtr, havocPtr->get_ble(), 2, wordVec, command);
		}
		else addError("ERROR", "Unknown Command", command);

		havocScreen->show(havocPtr);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);

}






void ScriptMgr::parseExtDesign(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	ExtScreen * extScreen = screenMgr->getExtScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == EXT_GEOM_TYPE)
	{
		Ext_geom * extPtr = (Ext_geom*) geomPtr;

		if (wordVec[1] == "type")
		{	// TYPE
			if (wordVec.size() >= 3)
			{
				if (wordVec[2] == "bomb")
					extScreen->s_type(scriptMode, BOMB_TYPE);
				else if (wordVec[2] == "missle")
					extScreen->s_type(scriptMode, MISSLE_TYPE);
				else if (wordVec[2] == "tank")
					extScreen->s_type(scriptMode, TANK_TYPE);
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "length")
		{	// LENGTH
			setParm(extPtr, extPtr->get_length(), 2, wordVec, command);
		}
		else if (wordVec[1] == "finess")
		{	// FINESS
			setParm(extPtr, extPtr->get_fine_ratio(), 2, wordVec, command);
		}
		else if (wordVec[1] == "cdplate")
		{	// CD PLATE
			setParm(extPtr, extPtr->get_drag(), 2, wordVec, command);
		}
		else if (wordVec[1] == "pylon")
		{	// PYLON
			if (wordVec.size() >= 3)
			{
				int on = wordVec[2].convert_to_integer();
				extScreen->s_pylon(scriptMode, on);
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "pylonheight")
		{	// HEIGHT
			setParm(extPtr, extPtr->get_pylon_height(), 2, wordVec, command);
		}
		else if (wordVec[1] == "pyloncdplate")
		{	// PYLON CD PLATE
			setParm(extPtr, extPtr->get_pylon_drag(), 2, wordVec, command);
		}
		else addError("ERROR", "Unknown Command", command);

		extScreen->show(extPtr);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);
}



void ScriptMgr::parseWingPlan(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	MsWingScreen * wingScreen = screenMgr->getMsWingScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == MS_WING_GEOM_TYPE)
	{
		Ms_wing_geom* wingPtr = (Ms_wing_geom*) geomPtr;

		if (wordVec[1] == "span")
		{	// SPAN
			setParm(wingPtr, wingPtr->get_total_span(), 2, wordVec, command);
		}
		else if (wordVec[1] == "projspan")
		{	// PROJSPAN
			setParm(wingPtr, wingPtr->get_total_proj_span(), 2, wordVec, command);
		}
		else if (wordVec[1] == "chord")
		{	// CHORD
			setParm(wingPtr, wingPtr->get_avg_chord(), 2, wordVec, command);
		}
		else if (wordVec[1] == "area")
		{	// AREA
			setParm(wingPtr, wingPtr->get_total_area(), 2, wordVec, command);
		}
		else if (wordVec[1] == "sweepoff")
		{	// SWEEPOFF
			setParm(wingPtr, wingPtr->get_sweep_off(), 2, wordVec, command);
		}
		else addError("ERROR", "Unknown Command", command);

		wingScreen->show(wingPtr);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);
}

void ScriptMgr::parseWingSect(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	MsWingScreen * wingScreen = screenMgr->getMsWingScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == MS_WING_GEOM_TYPE)
	{
		Ms_wing_geom* wingPtr = (Ms_wing_geom*) geomPtr;
		if (wordVec[1] == "select")
		{
			if (wordVec.size() >= 3)
			{
				wingPtr->set_curr_sect( wordVec[2].convert_to_integer() );
				wingPtr->set_highlight_type( MSW_HIGHLIGHT_SECT );
			} else addArgError(command);
		}
		else if (wordVec[1] == "add")
		{
			wingPtr->add_sect();
		}
		else if (wordVec[1] == "insert")
		{
			wingPtr->ins_sect();
		}
		else if (wordVec[1] == "copy")
		{
			wingPtr->copy_sect();
		}
		else if (wordVec[1] == "paste")
		{
			wingPtr->paste_sect();
		}
		else if (wordVec[1] == "del")
		{
			wingPtr->del_sect();
		}
		else if (wordVec[1] == "driver")
		{	// DRIVER
			if (wordVec.size() >= 3)
			{
				if (wordVec[2] == "artra")
				{
					wingPtr->set_driver(AR_TR_A);
				} 
				else if (wordVec[2] == "artrs")
				{
					wingPtr->set_driver(AR_TR_S);
				}
				else if (wordVec[2] == "artrtc")
				{
					wingPtr->set_driver(AR_TR_TC);
				}
				else if (wordVec[2] == "artrrc")
				{
					wingPtr->set_driver(AR_TR_RC);
				}
				else if (wordVec[2] == "stcrc")
				{
					wingPtr->set_driver(S_TC_RC);
				}
				else if (wordVec[2] == "atcrc")
				{
					wingPtr->set_driver(A_TC_RC);
				}
				else if (wordVec[2] == "trsa")
				{
					wingPtr->set_driver(TR_S_A);
				}
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "ar")
		{	// ASPECT RATIO
			setParm(wingPtr, wingPtr->get_sect_aspect(), 2, wordVec, command);
		}
		else if (wordVec[1] == "tr")
		{	// TR
			setParm(wingPtr, wingPtr->get_sect_taper(), 2, wordVec, command);
		}
		else if (wordVec[1] == "area")
		{	// AREA
			setParm(wingPtr, wingPtr->get_sect_area(), 2, wordVec, command);
		}
		else if (wordVec[1] == "span")
		{	// SPAN
			setParm(wingPtr, wingPtr->get_sect_span(), 2, wordVec, command);
		}
		else if (wordVec[1] == "tc")
		{	// TC
			setParm(wingPtr, wingPtr->get_sect_tc(), 2, wordVec, command);
		}
		else if (wordVec[1] == "rc")
		{	// RC
			setParm(wingPtr, wingPtr->get_sect_rc(), 2, wordVec, command);
		}
		else if (wordVec[1] == "sweep")
		{	// SWEEP
			setParm(wingPtr, wingPtr->get_sect_sweep(), 2, wordVec, command);
		}
		else if (wordVec[1] == "sweeploc")
		{	// SWEEPLOC
			setParm(wingPtr, wingPtr->get_sect_sweep_loc(), 2, wordVec, command);
		}
		else if (wordVec[1] == "twist")
		{	// TWIST
			setParm(wingPtr, wingPtr->get_sect_twist(), 2, wordVec, command);
		}
		else if (wordVec[1] == "twistloc")
		{	// TWIST LOC
			setParm(wingPtr, wingPtr->get_sect_twist_loc(), 2, wordVec, command);
		}
		else addError("ERROR", "Unknown Command", command);

		wingScreen->show(wingPtr);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);
}

void ScriptMgr::parseWingDihed(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	MsWingScreen * wingScreen = screenMgr->getMsWingScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == MS_WING_GEOM_TYPE)
	{
		Ms_wing_geom* wingPtr = (Ms_wing_geom*) geomPtr;
		if (wordVec[1] == "select")
		{
			if (wordVec.size() >= 3)
			{
				wingPtr->set_curr_joint( wordVec[2].convert_to_integer()  );
				wingPtr->set_highlight_type( MSW_HIGHLIGHT_JOINT );
			} else addArgError(command);
		}
		else if (wordVec[1] == "dihed1")
		{	// DIHED1
			setParm(wingPtr, wingPtr->get_sect_dihed1(), 2, wordVec, command);
		}
		else if (wordVec[1] == "dihed2")
		{	// DIHED2
			setParm(wingPtr, wingPtr->get_sect_dihed2(), 2, wordVec, command);
		}
		else if (wordVec[1] == "attach1")
		{	// ATTACH1
			setParm(wingPtr, wingPtr->get_sect_dihed_crv1(), 2, wordVec, command);
		}
		else if (wordVec[1] == "attach2")
		{	// ATTACH2
			setParm(wingPtr, wingPtr->get_sect_dihed_crv2(), 2, wordVec, command);
		}
		else if (wordVec[1] == "tanstr1")
		{	// ATTACH1
			setParm(wingPtr, wingPtr->get_sect_dihed_crv1_str(), 2, wordVec, command);
		}
		else if (wordVec[1] == "tanstr2")
		{	// ATTACH2
			setParm(wingPtr, wingPtr->get_sect_dihed_crv2_str(), 2, wordVec, command);
		}
		else if (wordVec[1] == "rotfoil")
		{	// ROTATE FOIL
			if (wordVec.size() >= 3)
			{
				wingPtr->set_dihed_rot_flag( wordVec[2].convert_to_integer() );
			} else addArgError(command);
		}
		else if (wordVec[1] == "degperseg")
		{	// DEGREES PER SEGMENT
			setParm(wingPtr, wingPtr->get_deg_per_seg(), 2, wordVec, command);
		}
		else if (wordVec[1] == "maxsegs")
		{	// MAX SEGMENTS
			setParm(wingPtr, wingPtr->get_max_num_segs(), 2, wordVec, command);
		}
		else addError("ERROR", "Unknown Command", command);

		wingScreen->show(wingPtr);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);
}



void ScriptMgr::parseWingFoil(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	MsWingScreen * wingScreen = screenMgr->getMsWingScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == MS_WING_GEOM_TYPE)
	{
		Ms_wing_geom* wingPtr = (Ms_wing_geom*) geomPtr;
		Af* foilPtr = wingPtr->get_af_ptr();
	
		if (wordVec[1] == "select")
		{
			if (wordVec.size() >= 3)
			{
				wingPtr->set_curr_foil( wordVec[2].convert_to_integer()  );
				wingPtr->set_highlight_type( MSW_HIGHLIGHT_FOIL );
			} else addArgError(command);
		}
		else
		{
			parseFoil(wingPtr, foilPtr, wordVec, command);
		}

		wingScreen->show(wingPtr);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);
}
//duct_foil name <string>
//duct_foil type <string>
//duct_foil camber <+double>
//duct_foil camberloc <+double>
//duct_foil thick <+double>
//duct_foil thickloc <+double>
//duct_foil 6series <string>
//duct_foil idealcl <string>
//duct_foil a <string>


void ScriptMgr::parseDuctShape(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	DuctScreen * ductScreen = screenMgr->getDuctScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == DUCT_GEOM_TYPE)
	{
		DuctGeom * ductPtr = (DuctGeom*) geomPtr;

		if (wordVec[1] == "length")
		{	// LENGTH
			setParm(ductPtr, ductPtr->get_length(), 2, wordVec, command);
		}
		else if (wordVec[1] == "inletdia")
		{	// INLET DIAMETER
			setParm(ductPtr, ductPtr->get_inlet_dia(), 2, wordVec, command);
		}
		else if (wordVec[1] == "inletoutlet")
		{	// INLET/OUTLET
			setParm(ductPtr, ductPtr->get_inlet_outlet(), 2, wordVec, command);
		}
		else addError("ERROR", "Unknown Command", command);

		ductScreen->show(ductPtr);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);
}

void ScriptMgr::parseDuctFoil(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	DuctScreen * ductScreen = screenMgr->getDuctScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == DUCT_GEOM_TYPE)
	{
		DuctGeom * ductPtr = (DuctGeom*) geomPtr;
		Af* foilPtr = ductPtr->get_af_ptr();

		parseFoil(ductPtr, foilPtr, wordVec, command);

		ductScreen->show(ductPtr);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);
}


void ScriptMgr::parsePropShape(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	PropScreen * propScreen = screenMgr->getPropScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == PROP_GEOM_TYPE)
	{
		PropGeom * propPtr = (PropGeom*) geomPtr;

		if (wordVec[1] == "diameter")
		{	// DIAMETER
			setParm(propPtr, propPtr->get_diameter(), 2, wordVec, command);
		}
		else if (wordVec[1] == "coneang")
		{	// CONE ANGLE
			setParm(propPtr, propPtr->get_cone_angle(), 2, wordVec, command);
		}
		else if (wordVec[1] == "bladepitch")
		{	// BLADE PITCH
			setParm(propPtr, propPtr->get_pitch(), 2, wordVec, command);
		}
		else if (wordVec[1] == "smooth")
		{	// SMOOTH
			if (wordVec.size() >= 3)
				propPtr->setSmoothFlag( wordVec[2].convert_to_integer() );
			else addArgError(command);
		}
		else if (wordVec[1] == "numu")
		{	// NUM U
			if (wordVec.size() >= 3)
				propPtr->setNumU( getInteger(propPtr->getNumU(), wordVec[2]));
			else addArgError(command);
		}
		else if (wordVec[1] == "numw")
		{	// NUM W
			if (wordVec.size() >= 3)
				propPtr->setNumW( getInteger(propPtr->getNumW(), wordVec[2]));
			else addArgError(command);
		}
		else if (wordVec[1] == "numblades")
		{	// NUM BLADES
			if (wordVec.size() >= 3)
				propPtr->setNumBlades( getInteger(propPtr->getNumBlades(), wordVec[2]));
			else addArgError(command);
		}
		else addError("ERROR", "Unknown Command", command);

		propScreen->show(propPtr);

	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);

}


void ScriptMgr::parsePropStation(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	PropScreen * propScreen = screenMgr->getPropScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == PROP_GEOM_TYPE)
	{
		PropGeom * propPtr = (PropGeom*) geomPtr;

		if (wordVec[1] == "select")
		{	// SELECT STATION
			if (wordVec.size() >= 3)
				propPtr->setCurrSectID(getInteger(propPtr->getNumBlades(), wordVec[2]));
			else addArgError(command);
		}
		else if (wordVec[1] == "loc")
		{	// LOC
			setParm(propPtr, propPtr->get_loc(), 2, wordVec, command);
		}
		else if (wordVec[1] == "offset")
		{	// OFFSET
			setParm(propPtr, propPtr->get_offset(), 2, wordVec, command);
		}
		else if (wordVec[1] == "chord")
		{	// CHORD
			setParm(propPtr, propPtr->get_chord(), 2, wordVec, command);
		}
		else if (wordVec[1] == "twist")
		{	// TWIST
			setParm(propPtr, propPtr->get_pitch(), 2, wordVec, command);
		}
		else if (wordVec[1] == "add")
		{	// ADD
			propPtr->addStation();
		}
		else if (wordVec[1] == "del")
		{	// DEL
			propPtr->delStation();
		}
		else addError("ERROR", "Unknown Command", command);

		propScreen->show(propPtr);

	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);

}

void ScriptMgr::parsePropFoil(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	PropScreen * propScreen = screenMgr->getPropScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == PROP_GEOM_TYPE)
	{
		PropGeom * propPtr = (PropGeom*) geomPtr;
		Af* foilPtr = propPtr->get_af_ptr();

		parseFoil(propPtr, foilPtr, wordVec, command);

		propScreen->show(propPtr);

	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);
}


void ScriptMgr::parseEngineDesign(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	EngineScreen * engineScreen = screenMgr->getEngineScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == ENGINE_GEOM_TYPE)
	{
		EngineGeom* enginePtr = (EngineGeom*) geomPtr;

		if (wordVec[1] == "rtip")
		{	// RAD TIP
			setParm(enginePtr, enginePtr->get_rad_tip(), 2, wordVec, command);
		}
		else if (wordVec[1] == "hub")
		{	// HUB RTIP
			setParm(enginePtr, enginePtr->get_hub_tip(), 2, wordVec, command);
		}
		else if (wordVec[1] == "max")
		{	// MAX RTIP
			setParm(enginePtr, enginePtr->get_max_tip(), 2, wordVec, command);
		}
		else if (wordVec[1] == "len")
		{	// LENGTH
			setParm(enginePtr, enginePtr->get_length(), 2, wordVec, command);
		}

		else if (wordVec[1] == "length")
		{	// NOZZLE LENGTH
			setParm(enginePtr, enginePtr->get_noz_length(), 2, wordVec, command);
		}
		else if (wordVec[1] == "exit")
		{	// NOZZLE EXIT
			setParm(enginePtr, enginePtr->get_exit_area_ratio(), 2, wordVec, command);
		}

		else if (wordVec[1] == "on")
		{	// DUCT ON
			enginePtr->set_inl_duct_flag(1);
			enginePtr->parm_changed( enginePtr->get_inl_duct_shape() );
		}
		else if (wordVec[1] == "off")
		{	// DUCT ON
			enginePtr->set_inl_duct_flag(0);
			enginePtr->parm_changed( enginePtr->get_inl_duct_shape() );
		}
		else if (wordVec[1] == "xoff")
		{	// DUCT XOFF
			setParm(enginePtr, enginePtr->get_inl_duct_x_off(), 2, wordVec, command);
		}
		else if (wordVec[1] == "yoff")
		{	// DUCT YOFF
			setParm(enginePtr, enginePtr->get_inl_duct_y_off(), 2, wordVec, command);
		}
		else if (wordVec[1] == "shape")
		{	// DUCT SHAPE
			setParm(enginePtr, enginePtr->get_inl_duct_shape(), 2, wordVec, command);
		}
		else addError("ERROR", "Unknown Command", command);

		engineScreen->show(enginePtr);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);

}

void ScriptMgr::parseEngineInlet(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;
	if (!verifySingleGeom(command)) return;

	EngineScreen * engineScreen = screenMgr->getEngineScreen();
	Geom * geomPtr = aircraft->getActiveGeom();

	if (geomPtr->getType() == ENGINE_GEOM_TYPE)
	{
		EngineGeom* enginePtr = (EngineGeom*) geomPtr;

		if (wordVec[1] == "cowllen")
		{	// COWLLEN
			setParm(enginePtr, enginePtr->get_cowl_length(), 2, wordVec, command);
		}
		else if (wordVec[1] == "engthrt")
		{	// ENGINE THRUST
			setParm(enginePtr, enginePtr->get_area_ratio_eng_thrt(), 2, wordVec, command);
		}
		else if (wordVec[1] == "hlthrt")
		{	// HL THRUST
			setParm(enginePtr, enginePtr->get_area_ratio_hl_thrt(), 2, wordVec, command);
		}
		else if (wordVec[1] == "lipfr")
		{	// LIP FINE
			setParm(enginePtr, enginePtr->get_lip_fine(), 2, wordVec, command);
		}
		else if (wordVec[1] == "hw")
		{	// ASPECT RATIO
			setParm(enginePtr, enginePtr->get_ht_wid_ratio(), 2, wordVec, command);
		}
		else if (wordVec[1] == "upsurf")
		{	// UPPER SURF
			setParm(enginePtr, enginePtr->get_upper_shape(), 2, wordVec, command);
		}
		else if (wordVec[1] == "lowsurf")
		{	// LOWER SURF
			setParm(enginePtr, enginePtr->get_lower_shape(), 2, wordVec, command);
		}
		else if (wordVec[1] == "xrot")
		{	// XROT
			setParm(enginePtr, enginePtr->get_inl_x_rot(), 2, wordVec, command);
		}
		else if (wordVec[1] == "scarf")
		{	// SCARF
			setParm(enginePtr, enginePtr->get_inl_scarf(), 2, wordVec, command);
		}
		else addError("ERROR", "Unknown Command", command);

		engineScreen->show(enginePtr);
	} 
	else addError("ERROR", "Command incompatible with selected geometry", command);

}


void ScriptMgr::parseFoil(Geom* currGeom, Af* foilPtr, deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec[1] == "inverty")
	{	// INVERTY
		if (wordVec.size() >= 3)
		{
			foilPtr->set_inverted_flag( wordVec[2].convert_to_integer() );
			//==== Force Update and Redraw ===//
			currGeom->parm_changed( foilPtr->get_thickness() );
		} else addArgError(command);
	}
	else if (wordVec[1] == "type")
	{	// TYPE
		if (wordVec.size() >= 3)
		{
			if (wordVec[2] == "naca4")
			{
				foilPtr->set_type( NACA_4_SERIES);
			}
			else if (wordVec[2] == "biconvex")
			{
				foilPtr->set_type( BICONVEX);
			}
			else if (wordVec[2] == "wedge")
			{
				foilPtr->set_type( WEDGE);
			}
			else if (wordVec[2] == "naca6")
			{
				foilPtr->set_type( NACA_6_SERIES);
			}
			currGeom->parm_changed( foilPtr->get_thickness() );
		} else addArgError(command);
	}
	else if (wordVec[1] == "read")
	{	// READ FILE
		if (wordVec.size() >= 3)
		{
			foilPtr->read_af_file( wordVec[2] );
			currGeom->parm_changed( foilPtr->get_thickness() );
		} else addArgError(command);
	}
	else if (wordVec[1] == "camber")
	{	// CAMBER
		setParm(currGeom, foilPtr->get_camber(), 2, wordVec, command);
	}
	else if (wordVec[1] == "camberloc")
	{	// CAMBER LOC
		setParm(currGeom, foilPtr->get_camber_loc(), 2, wordVec, command);
	}
	else if (wordVec[1] == "thick")
	{	// THICK
		setParm(currGeom, foilPtr->get_thickness(), 2, wordVec, command);
	}
	else if (wordVec[1] == "thickloc")
	{	// THICK LOC
		setParm(currGeom, foilPtr->get_thickness_loc(), 2, wordVec, command);
	}
	else if (wordVec[1] == "6series")
	{	// 6 SERIES
		if (wordVec.size() >= 3)
		{
			if (wordVec[2] == "63")
				foilPtr->set_sixser( 63 );
			else if (wordVec[2] == "64")
				foilPtr->set_sixser( 64 );
			else if (wordVec[2] == "65")
				foilPtr->set_sixser( 65 );
			else if (wordVec[2] == "66")
				foilPtr->set_sixser( 66 );
			else if (wordVec[2] == "67")
				foilPtr->set_sixser( 67 );
			else if (wordVec[2] == "63a")
				foilPtr->set_sixser( -63 );
			else if (wordVec[2] == "64a")
				foilPtr->set_sixser( -64 );
			else if (wordVec[2] == "65a")
				foilPtr->set_sixser( -65 );

			currGeom->parm_changed( foilPtr->get_thickness() );

		} else addArgError(command);
	}
	else if (wordVec[1] == "lerad")
	{	// L E RADIUS
		setParm(currGeom, foilPtr->get_leading_edge_radius(), 2, wordVec, command);
	}
	else if (wordVec[1] == "idealcl")
	{	// IDEAL CL
		setParm(currGeom, foilPtr->get_ideal_cl(), 2, wordVec, command);
	}
	else if (wordVec[1] == "a")
	{	// A
		setParm(currGeom, foilPtr->get_a(), 2, wordVec, command);
	}
	else addError("ERROR", "Unknown Command", command);
}

void ScriptMgr::parseLabel(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;

	LabelScreen * labelScreen = screenMgr->getLabelScreen();
	vector < LabelGeom * > labelVec = aircraft->getActiveLabelVec();

	if (wordVec[1] == "add")
	{	// ADD
		Stringc name("");
		if (wordVec.size() >= 3)
			name = wordVec[3];

		if (wordVec[2] == "text")
			labelScreen->s_add(scriptMode, LabelGeom::TEXT_LABEL, name);
		else if (wordVec[2] == "ruler")
			labelScreen->s_add(scriptMode, LabelGeom::RULER_LABEL, name);
	}
	else if (wordVec[1] == "select")
	{	// SELECT
		if (wordVec.size() >= 3)
		{
			if (wordVec[2] == "all")
			{
				labelScreen->s_select_all(scriptMode);
			}
			else
			{
				wordVec.pop_front();
				wordVec.pop_front();
				labelScreen->s_select(scriptMode, labelScreen->s_get_label_vec(wordVec));
			}
		}
	}
	else if (wordVec[1] == "remove")
	{	// REMOVE
		wordVec.pop_front(); //label
		wordVec.pop_front(); //remove
		labelScreen->s_remove(scriptMode, labelScreen->s_get_label_vec(wordVec));
	}
	else if (wordVec[1] == "name")
	{	// NAME
		if (wordVec.size() >= 3)
		{
			Stringc name = wordVec[2];
			name.trim('\"');
			for (int i = 0; i < (int)labelVec.size(); i++)
				labelVec[i]->setName(name);
		}
	}
	else if (wordVec[1] == "color")
	{	// COLOR
		if (wordVec.size() >= 5)
		{
			for (int i = 0; i < (int)labelVec.size(); i++)
				labelVec[i]->setColor(wordVec[2].convert_to_integer(), wordVec[3].convert_to_integer(), wordVec[4].convert_to_integer());
		}
		else addArgError(command);
	}
	else if (wordVec[1] == "size")
	{	// SIZE
		if (wordVec.size() >= 3)
		{
			for (int i = 0; i < (int)labelVec.size(); i++)
				labelVec[i]->setTextSize((float)getDouble(labelVec[i]->getTextSize(), wordVec[2]));
		}
		else addArgError(command);
	}
	else if (wordVec[1] == "display")
	{	// DISPLAY
		if (wordVec.size() >= 3)
		{
			int display = 0;
			if (wordVec[2] == "hide")
				display = LabelGeom::DRAW_HIDDEN;
			else if (wordVec[2] == "basic")
				display = LabelGeom::DRAW_VISIBLE;
			else if (wordVec[2] == "full")
				display = LabelGeom::DRAW_HIGHLIGHT;

			for (int i = 0; i < (int)labelVec.size(); i++)
				labelVec[i]->setDrawMode(display);
		}
		else addArgError(command);
	}
	else addError("ERROR", "Unknown Command", command);

//	if (scriptMode != SCRIPT)
//	{
//		screenMgr->getDrawWin()->redraw();
//		labelScreen->update();
//	}
}

void ScriptMgr::parseText(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;

	LabelScreen * labelScreen = screenMgr->getLabelScreen();
	LabelGeom * label = aircraft->getActiveLabel();
	if (label == NULL) return;

	if (label->getType() == LabelGeom::TEXT_LABEL)
	{
		TextLabel* textPtr= (TextLabel*) label;
		if (wordVec[1] == "offset")
		{	// OFFSET
			if (wordVec.size() >= 3)
			{
				textPtr->setTextOffset(getDouble(textPtr->getTextOffset(), wordVec[2]));
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "attach")
		{	// ATTACH
			if (wordVec.size() >= 7)
			{
				Stringc geomName = wordVec[2];
				geomName.trim('\"');
				vector< Geom * > gVec = aircraft->getGeomByName(geomName, 0);
				if (gVec.size() > 0)
				{
					VertexID vid;
					vid.geomPtr = gVec[0];
					vid.surface = wordVec[3].convert_to_integer();
					vid.section = wordVec[4].convert_to_double();
					vid.point = wordVec[5].convert_to_double();
					vid.reflect = wordVec[6].convert_to_integer();
					textPtr->setVertex(vid);
				}
			}
			else addArgError(command);
		}
		else addError("ERROR", "Unknown Command", command);
	} 
	else addError("ERROR", "Command incompatible with selected label", command);

//	if (scriptMode != SCRIPT)
//	{
//		screenMgr->getDrawWin()->redraw();
//		labelScreen->update();
//	}
}

void ScriptMgr::parseRuler(deque<Stringc> &wordVec, Stringc command)
{
	if (wordVec.size() < 2) return;

	LabelScreen * labelScreen = screenMgr->getLabelScreen();
	LabelGeom * label = aircraft->getActiveLabel();
	if (label == NULL) return;

	if (label->getType() == LabelGeom::RULER_LABEL)
	{
		RulerLabel* rulerPtr = (RulerLabel*) label;
		if (wordVec[1] == "offset")
		{	// OFFSET
			if (wordVec.size() >= 3)
			{
				rulerPtr->setRulerOffset(getDouble(rulerPtr->getRulerOffset(), wordVec[2]));
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "precision")
		{	// PRECISION
			if (wordVec.size() >= 3)
			{
				rulerPtr->setPrecision(getInteger(rulerPtr->getPrecision(), wordVec[2]));
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "units")
		{	// UNITS
			if (wordVec.size() >= 3)
			{
				rulerPtr->setUnitString(wordVec[2]);
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "axis")
		{	// AXIS
			if (wordVec.size() >= 5)
			{
				rulerPtr->setXYZLock(wordVec[2].convert_to_integer(), wordVec[3].convert_to_integer(), wordVec[4].convert_to_integer());
			}
			else addArgError(command);
		}
		else if (wordVec[1] == "attach1")
		{	// ATTACH 1
			Stringc geomName = wordVec[2];
			geomName.trim('\"');
			vector< Geom * > gVec = aircraft->getGeomByName(geomName, 0);
			if (gVec.size() > 0)
			{
				VertexID vid;
				vid.geomPtr = gVec[0];
				vid.surface = wordVec[3].convert_to_integer();
				vid.section = wordVec[4].convert_to_double();
				vid.point = wordVec[5].convert_to_double();
				vid.reflect = wordVec[6].convert_to_integer();
				rulerPtr->setVertex(vid);
			}
		}
		else if (wordVec[1] == "attach2")
		{	// ATTACH 2
			Stringc geomName = wordVec[2];
			geomName.trim('\"');
			vector< Geom * > gVec = aircraft->getGeomByName(geomName, 0);
			if (gVec.size() > 0)
			{
				VertexID vid;
				vid.geomPtr = gVec[0];
				vid.surface = wordVec[3].convert_to_integer();
				vid.section = wordVec[4].convert_to_double();
				vid.point = wordVec[5].convert_to_double();
				vid.reflect = wordVec[6].convert_to_integer();
				rulerPtr->setVertex2(vid);
			}
		}
		else addError("ERROR", "Unknown Command", command);
	} 
	else addError("ERROR", "Command incompatible with selected label", command);

//	if (scriptMode != SCRIPT)
//	{
//		screenMgr->getDrawWin()->redraw();
//		labelScreen->update();
//	}
}
