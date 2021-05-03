#include "utils.h"

namespace TestConfig
{

bool Test()
{
	bool fail = false;
	int r;
	asIScriptEngine *engine;
	CBufferedOutStream bout;

 	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->ClearMessageCallback(); // Make sure this works
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

	RegisterScriptArray(engine, true);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
	asIScriptFunction *func = engine->GetGlobalFunctionByDecl("void assert(bool)");
	if( func == 0 ) TEST_FAILED;
	if( std::string(func->GetDeclaration()) != "void assert(bool)" ) TEST_FAILED;


	r = engine->RegisterGlobalFunction("void func(mytype)", asFUNCTION(0), asCALL_GENERIC);
	if( r >= 0 ) TEST_FAILED;

	r = engine->RegisterGlobalFunction("void func(int &)", asFUNCTION(0), asCALL_GENERIC);
	if( !engine->GetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES) )
	{
		if( r >= 0 ) TEST_FAILED;
	}
	else
	{
		if( r < 0 ) TEST_FAILED;
	}
	
	r = engine->RegisterObjectType("mytype", 0, asOBJ_REF);
	if( r < 0 ) TEST_FAILED;

	r = engine->RegisterObjectBehaviour("mytype", asBEHAVE_CONSTRUCT, "void f(othertype)", asFUNCTION(0), asCALL_GENERIC);
	if( r >= 0 ) TEST_FAILED;

	r = engine->RegisterObjectMethod("mytype", "type opAdd(int) const", asFUNCTION(0), asCALL_GENERIC);
	if( r >= 0 ) TEST_FAILED;

	r = engine->RegisterGlobalProperty("type a", (void*)1);
	if( r >= 0 ) TEST_FAILED;

	r = engine->RegisterObjectMethod("mytype", "void method(int &)", asFUNCTION(0), asCALL_GENERIC);
	if( !engine->GetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES) )
	{
		if( r >= 0 ) TEST_FAILED;
	}
	else
	{
		if( r < 0 ) TEST_FAILED;
	}

	r = engine->RegisterObjectProperty("mytype", "type a", 0);
	if( r >= 0 ) TEST_FAILED;

	r = engine->RegisterStringFactory("type", (asIStringFactory*)1);
	if( r >= 0 ) TEST_FAILED;

	// Verify the output messages
	if( !engine->GetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES) )
	{
		if( bout.buffer != "System function (1, 11) : Error   : Identifier 'mytype' is not a data type in global namespace\n"
			               " (0, 0) : Error   : Failed in call to function 'RegisterGlobalFunction' with 'void func(mytype)' (Code: asINVALID_DECLARATION, -10)\n"
						   "System function (1, 15) : Error   : Only object types that support object handles can use &inout. Use &in or &out instead\n"
						   " (0, 0) : Error   : Failed in call to function 'RegisterGlobalFunction' with 'void func(int &)' (Code: asINVALID_DECLARATION, -10)\n"
						   "System function (1, 8) : Error   : Identifier 'othertype' is not a data type in global namespace\n"
						   " (0, 0) : Error   : Failed in call to function 'RegisterObjectBehaviour' with 'mytype' and 'void f(othertype)' (Code: asINVALID_DECLARATION, -10)\n"
						   "System function (1, 1) : Error   : Identifier 'type' is not a data type in global namespace\n"
						   " (0, 0) : Error   : Failed in call to function 'RegisterObjectMethod' with 'mytype' and 'type opAdd(int) const' (Code: asINVALID_DECLARATION, -10)\n"
						   "Property (1, 1) : Error   : Identifier 'type' is not a data type in global namespace\n"
						   " (0, 0) : Error   : Failed in call to function 'RegisterGlobalProperty' with 'type a' (Code: asINVALID_DECLARATION, -10)\n"
						   "System function (1, 17) : Error   : Only object types that support object handles can use &inout. Use &in or &out instead\n"
						   " (0, 0) : Error   : Failed in call to function 'RegisterObjectMethod' with 'mytype' and 'void method(int &)' (Code: asINVALID_DECLARATION, -10)\n"
						   "Property (1, 1) : Error   : Identifier 'type' is not a data type in global namespace\n"
						   " (0, 0) : Error   : Failed in call to function 'RegisterObjectProperty' with 'mytype' and 'type a' (Code: asINVALID_DECLARATION, -10)\n"
						   " (1, 1) : Error   : Identifier 'type' is not a data type in global namespace\n"
						   " (0, 0) : Error   : Failed in call to function 'RegisterStringFactory' with 'type' (Code: asINVALID_TYPE, -12)\n" 
			)
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}
	else
	{
		if( bout.buffer != "System function (1, 11) : Error   : Identifier 'mytype' is not a data type\n"
						   "System function (1, 8) : Error   : Identifier 'othertype' is not a data type\n"
						   "System function (1, 1) : Error   : Identifier 'type' is not a data type\n"
						   "System function (1, 8) : Error   : Identifier 'type' is not a data type\n"
						   "Property (1, 1) : Error   : Identifier 'type' is not a data type\n"
						   "Property (1, 1) : Error   : Identifier 'type' is not a data type\n"
						   " (1, 1) : Error   : Identifier 'type' is not a data type\n")
			TEST_FAILED;
	}

	engine->Release();

	// Success
 	return fail;
}

} // namespace

