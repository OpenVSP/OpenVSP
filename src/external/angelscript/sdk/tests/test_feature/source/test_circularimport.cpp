//
// Tests importing functions from other modules
//
// Test author: Andreas Jonsson
//

#include "utils.h"

namespace TestCircularImport
{

static const char *script1 =
"import void Test2() from \"Module2\";   \n"
"void Test1() {}                         \n";

static const char *script2 =
"import void Test1() from \"Module1\";   \n"
"void Test2() {}                         \n";

static void BindImportedFunctions(asIScriptEngine *engine, const char *module);

bool Test()
{
	bool fail = false;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	asIScriptModule *mod = engine->GetModule("Module1", asGM_ALWAYS_CREATE);
	mod->AddScriptSection(":1", script1, strlen(script1), 0);
	mod->Build();

	mod = engine->GetModule("Module2", asGM_ALWAYS_CREATE);
	mod->AddScriptSection(":2", script2, strlen(script2), 0);
	mod->Build();

	BindImportedFunctions(engine, "Module1");
	BindImportedFunctions(engine, "Module2");

	// Discard the modules
	engine->DiscardModule("Module1");
	engine->DiscardModule("Module2");
	
	engine->Release();

	// Success
	return fail;
}

static void BindImportedFunctions(asIScriptEngine *engine, const char *module)
{
	// Bind imported functions
	asIScriptModule *mod = engine->GetModule(module);
	int c = mod->GetImportedFunctionCount();
	for( int n = 0; n < c; ++n )
	{
		const char *decl = mod->GetImportedFunctionDeclaration(n);

		// Get module name from where the function should be imported
		const char *moduleName = mod->GetImportedFunctionSourceModule(n);

		asIScriptFunction *func = engine->GetModule(moduleName)->GetFunctionByDecl(decl);
		mod->BindImportedFunction(n, func);
	}
}

} // namespace

