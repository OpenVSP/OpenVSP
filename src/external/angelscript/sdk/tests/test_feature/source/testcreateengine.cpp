//
// Test to see if engine can be created.
//
// Author: Fredrik Ehnbom
//

#include "utils.h"

bool TestCreateEngine()
{
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	if( engine == 0 )
	{
		// Failure
		PRINTF("TestCreateEngine: asCreateScriptEngine() failed\n");
		return true;
	}
	else
	{
		// Although it's not recommended that two engines are created, it shouldn't be a problem
		asIScriptEngine *engine2 = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		if( engine2 == 0 )
		{
			// Failure
			PRINTF("TestCreateEngine: asCreateScriptEngine() failed for 2nd engine\n");
			return true;
		}
		else
			engine2->Release();

		engine->Release();
	}
	
	// Success
	return false;
}
