// This test is to make sure it is possible to store the script engine in a global variable 
// that will be cleaned up only upon application exit. Previously this could cause the 
// application to crash when the library was built with multithreading, as the thread
// manager was destroyed before the engine could clean up the global variables in the modules it held.

#include "utils.h"

namespace TestThreadMgr
{

// Compile a script that stores a global variable of a script class that has a destructor.
// This will cause the destructor of the class to be called when the module is destroyed.
static const char *script = 
"class MyTest             \n"
"{                        \n"
"   MyTest() {var = 1;}   \n"
"   ~MyTest() {var = 0;}  \n"
"   int var;              \n"
"}                        \n"
"MyTest global;           \n";

// Here's our global variable that will release the engine upon application exit
class EngineWrapper
{
public:
	EngineWrapper() { engine = 0; }
	~EngineWrapper() { if( engine ) engine->Release(); }

	asIScriptEngine *engine;
} g_engine;

bool Test()
{
	bool fail = false;

 	g_engine.engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	COutStream out;
	g_engine.engine->SetMessageCallback(asMETHOD(COutStream,Callback),&out,asCALL_THISCALL);

	asIScriptModule *mod = g_engine.engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script, strlen(script), 0);
	mod->Build();

	g_engine.engine->ClearMessageCallback();

	// Now the bomb has been armed. When the test application exits, if all goes 
	// well the bomb should be disarmed correctly without crashing the application.

	// Success
	return fail;
}

} // namespace

