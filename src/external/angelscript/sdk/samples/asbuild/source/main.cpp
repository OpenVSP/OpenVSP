#include <iostream>  // cout
#include <assert.h>  // assert()
#include <string.h>  // strstr()
#include <angelscript.h>
#include "../../../add_on/scriptbuilder/scriptbuilder.h"
#include "../../../add_on/scripthelper/scripthelper.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#if defined(_MSC_VER) && !defined(_WIN32_WCE)
#include <direct.h>
#include <crtdbg.h>
#endif
#ifdef _WIN32_WCE
#include <windows.h> // For GetModuleFileName
#endif

using namespace std;

// Function prototypes
int ConfigureEngine(asIScriptEngine *engine, const char *configFile);
int CompileScript(asIScriptEngine *engine, const char *scriptFile);
int SaveBytecode(asIScriptEngine *engine, const char *outputFile);
static const char *GetCurrentDir(char *buf, size_t size);

void MessageCallback(const asSMessageInfo *msg, void *param)
{
	const char *type = "ERR ";
	if( msg->type == asMSGTYPE_WARNING ) 
		type = "WARN";
	else if( msg->type == asMSGTYPE_INFORMATION ) 
		type = "INFO";

	printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}

int main(int argc, char **argv)
{
#if defined(_MSC_VER)
	// Turn on memory leak detection (use _CrtSetBreakAlloc to break at specific allocation)
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	_CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT,_CRTDBG_FILE_STDERR);

	//_CrtSetBreakAlloc(6150);
#endif

	int r;

	if( argc < 4 )
	{
		cout << "Usage: " << endl;
		cout << "asbuild <config file> <script file> <output>" << endl;
		cout << " <config file>  is the file with the application interface" << endl;
		cout << " <script file>  is the script file that should be compiled" << endl;
		cout << " <output>       is the name that the compiled script will be saved as" << endl;
		return -1;
	}

	// Create the script engine
	asIScriptEngine *engine = asCreateScriptEngine();
	if( engine == 0 )
	{
		cout << "Failed to create script engine." << endl;
		return -1;
	}

	// The script compiler will send any compiler messages to the callback
	engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);

	// Configure the script engine with all the functions, 
	// and variables that the script should be able to use.
	r = ConfigureEngine(engine, argv[1]);
	if( r < 0 ) return -1;
	
	// Compile the script code
	r = CompileScript(engine, argv[2]);
	if( r < 0 ) return -1;

	// Save the bytecode
	r = SaveBytecode(engine, argv[3]);
	if( r < 0 ) return -1;

	// Shut down the engine
	engine->ShutDownAndRelease();

	return 0;
}

// This function will register the application interface, 
// based on information read from a configuration file. 
int ConfigureEngine(asIScriptEngine *engine, const char *configFile)
{
	int r;

	ifstream strm;
	strm.open(configFile);
	if( strm.fail() )
	{
		// Write a message to the engine's message callback
		char buf[256];
		string msg = "Failed to open config file in path: '" + string(GetCurrentDir(buf, 256)) + "'";
		engine->WriteMessage(configFile, 0, 0, asMSGTYPE_ERROR, msg.c_str());
		return -1;
	}

	// Configure the engine with the information from the file
	r = ConfigEngineFromStream(engine, strm, configFile);
	if( r < 0 )
	{
		engine->WriteMessage(configFile, 0, 0, asMSGTYPE_ERROR, "Configuration failed");
		return -1;
	}


	engine->WriteMessage(configFile, 0, 0, asMSGTYPE_INFORMATION, "Configuration successfully registered");
	
	return 0;
}

int CompileScript(asIScriptEngine *engine, const char *scriptFile)
{
	int r;

	CScriptBuilder builder;
	r = builder.StartNewModule(engine, "build");
	if( r < 0 ) return -1;

	r = builder.AddSectionFromFile(scriptFile);
	if( r < 0 ) return -1;

	r = builder.BuildModule();
	if( r < 0 )
	{
		engine->WriteMessage(scriptFile, 0, 0, asMSGTYPE_ERROR, "Script failed to build");
		return -1;
	}

	engine->WriteMessage(scriptFile, 0, 0, asMSGTYPE_INFORMATION, "Script successfully built");

	return 0;
}

class CBytecodeStream : public asIBinaryStream
{
public:
	CBytecodeStream() {f = 0;}
	~CBytecodeStream() { if( f ) fclose(f); }

	int Open(const char *filename)
	{
		if( f ) return -1;
#if _MSC_VER >= 1500
		fopen_s(&f, filename, "wb");
#else
		f = fopen(filename, "wb");
#endif
		if( f == 0 ) return -1;
		return 0;
	}
	void Write(const void *ptr, asUINT size) 
	{
		if( size == 0 || f == 0 ) return; 
		fwrite(ptr, size, 1, f); 
	}
	void Read(void *, asUINT ) {}

protected:
	FILE *f;
};

int SaveBytecode(asIScriptEngine *engine, const char *outputFile)
{
	CBytecodeStream stream;
	int r = stream.Open(outputFile);
	if( r < 0 )
	{
		engine->WriteMessage(outputFile, 0, 0, asMSGTYPE_ERROR, "Failed to open output file for writing");
		return -1;
	}

	asIScriptModule *mod = engine->GetModule("build");
	if( mod == 0 )
	{
		engine->WriteMessage(outputFile, 0, 0, asMSGTYPE_ERROR, "Failed to retrieve the compiled bytecode");
		return -1;
	}

	r = mod->SaveByteCode(&stream);
	if( r < 0 )
	{
		engine->WriteMessage(outputFile, 0, 0, asMSGTYPE_ERROR, "Failed to write the bytecode");
		return -1;
	}

	engine->WriteMessage(outputFile, 0, 0, asMSGTYPE_INFORMATION, "Bytecode successfully saved");

	return 0;
}

static const char *GetCurrentDir(char *buf, size_t size)
{
#ifdef _MSC_VER
#ifdef _WIN32_WCE
    static TCHAR apppath[MAX_PATH] = TEXT("");
    if (!apppath[0])
    {
        GetModuleFileName(NULL, apppath, MAX_PATH);

        
        int appLen = _tcslen(apppath);

        // Look for the last backslash in the path, which would be the end
        // of the path itself and the start of the filename.  We only want
        // the path part of the exe's full-path filename
        // Safety is that we make sure not to walk off the front of the 
        // array (in case the path is nothing more than a filename)
        while (appLen > 1)
        {
            if (apppath[appLen-1] == TEXT('\\'))
                break;
            appLen--;
        }

        // Terminate the string after the trailing backslash
        apppath[appLen] = TEXT('\0');
    }
#ifdef _UNICODE
    wcstombs(buf, apppath, min(size, wcslen(apppath)*sizeof(wchar_t)));
#else
    memcpy(buf, apppath, min(size, strlen(apppath)));
#endif

    return buf;
#else
	return _getcwd(buf, (int)size);
#endif
#elif defined(__APPLE__)
	return getcwd(buf, size);
#else
	return "";
#endif
}


