#include "utils.h"
#include "../../../add_on/scriptfile/scriptfile.h"
#include "../../../add_on/scriptfile/scriptfilesystem.h"
#include "../../../add_on/datetime/datetime.h"

namespace Test_Addon_ScriptFile
{

bool Test()
{
	bool fail = false;
	COutStream out;
	CBufferedOutStream bout;
	int r;
	asIScriptEngine *engine = 0;

	// Test file and directory manipulation
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		RegisterScriptArray(engine, false);
		RegisterScriptDateTime(engine);
		RegisterScriptFile(engine);
		RegisterScriptFileSystem(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void main() { \n"
			"  filesystem fs; \n"
			"  assert( fs.changeCurrentPath('scripts') ); \n"
			"  assert( fs.makeDir('backup') == 0 ); \n"
			"  assert( fs.copyFile('TestExecuteScript.as', 'backup/TestExecuteScript.as') == 0 ); \n"
			"  assert( fs.move('backup/TestExecuteScript.as', 'backup/Test.as') == 0 ); \n"
			"  assert( fs.getSize('TestExecuteScript.as') == fs.getSize('backup/Test.as') ); \n"
			"  assert( fs.deleteFile('backup/Test.as') == 0 ); \n"
			"  assert( fs.removeDir('backup') == 0 ); \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "main()", mod, ctx);
		if (r != asEXECUTION_FINISHED)
		{
			if (r == asEXECUTION_EXCEPTION && ctx->GetExceptionLineNumber() == 4)
				PRINTF("Failed to find the sub directory 'scripts'. Are you running the test from the correct folder?\n");
			else
				TEST_FAILED;
		}
		ctx->Release();

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		RegisterScriptArray(engine, false);
		RegisterScriptDateTime(engine);
		RegisterScriptFile(engine);
		RegisterScriptFileSystem(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"void main() { \n"
			"  filesystem fs; \n"										// starts in applications working dir
			"  array<string> dirs = fs.getDirs(); \n"
			"  assert( dirs.find('scripts') >= 0 ); \n"
			"  fs.changeCurrentPath('scripts'); \n"						// move to the sub directory
			"  array<string> files = fs.getFiles(); \n"					// get the script files in the directory
			"  assert( files.length() == 2 ); \n"
			"  file f; \n"
			"  f.open('scripts/include.as', 'r'); \n"
			"  string str = f.readLine(); \n"
			"  str = str.substr(3, 25); \n"
			"  assert( str == 'void MyIncludedFunction()' ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptContext *ctx = engine->CreateContext();
		r = ExecuteString(engine, "main()", mod, ctx);
		if( r != asEXECUTION_FINISHED )
		{
			if( r == asEXECUTION_EXCEPTION && ctx->GetExceptionLineNumber() == 4 )
				PRINTF("Failed to find the sub directory 'scripts'. Are you running the test from the correct folder?\n");
			else
				TEST_FAILED;
		}
		ctx->Release();

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterStdString(engine);
		RegisterScriptFile(engine);

		const char *script =
			"file f;                                                  \n"
			"int r = f.open(\"scripts/TestExecuteScript.as\", \"r\"); \n"
			"if( r >= 0 ) {                                           \n"
			"  assert( f.getSize() > 0 );                             \n"
			"  string s1 = f.readString(10000);                       \n"
			"  assert( s1.length() == uint(f.getSize()) );            \n"
			"  f.close();                                             \n"
			"  f.open('scripts/TestExecuteScript.as', 'r');           \n"
			"  string s2;                                             \n"
			"  while( !f.isEndOfFile() )                              \n"
			"  {                                                      \n"
			"    string s3 = f.readLine();                            \n"
			"    s2 += s3;                                            \n"
			"  }                                                      \n"
			"  assert( s1 == s2 );                                    \n"
			"  f.close();                                             \n"
			"}                                                        \n";

		r = ExecuteString(engine, script);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
		}

		engine->Release();
	}

	return fail;
}

}

