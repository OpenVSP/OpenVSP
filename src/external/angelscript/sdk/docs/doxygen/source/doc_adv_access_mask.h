/**

\page doc_adv_access_mask Access masks and exposing different interfaces

An application may need to expose different interfaces to different types of scripts, 
e.g. a game entity may only have access to specific set of functions, while the GUI 
script can have access to a completely different set of functions.

To accomplish this the application can set a bitmask while registering the interface,
and then use that bitmask when building the scripts to determine what the scripts 
should or should not be able to access. 

\code
void ConfigureEngine(asIScriptEngine *engine)
{
  // Register the interface available to script type 1
  engine->SetDefaultAccessMask(0x1); 
  r = engine->RegisterGlobalFunction("void func1()", asFUNCTION(func1), asCALL_CDECL); assert( r >= 0 );

  // Register the interface available to script type 2
  engine->SetDefaultAccessMask(0x2); 
  r = engine->RegisterGlobalFunction("void func2()", asFUNCTION(func2), asCALL_CDECL); assert( r >= 0 );

  // Register the interface available to both script types
  engine->SetDefaultAccessMask(0x3);
  r = engine->RegisterGlobalFunction("void func3()", asFUNCTION(func3), asCALL_CDECL); assert( r >= 0 );
}

int CompileScript(asIScriptEngine *engine, const char *script, int type)
{
  int r;
  CScriptBuilder builder;
  r = builder.StartNewModule(engine, script);
  if( r < 0 ) return r;

  // Set the access mask for the module, which will
  // determine the functions from the application
  // interface that can be called
  asIScriptModule *mod = builder.GetModule();
  mod->SetAccessMask(type); 

  // Add the script section and build the script
  r = builder.AddSectionFromFile(script);
  if( r < 0 ) return r;

  return builder.BuildModule();
}
\endcode

The access mask can be defined for the following entities in the application interface:

 - \ref asIScriptEngine::RegisterGlobalFunction "Global functions"
 - \ref asIScriptEngine::RegisterGlobalProperty "Global properties"
 - \ref asIScriptEngine::RegisterObjectType "Object types"
 - \ref asIScriptEngine::RegisterObjectMethod "Individual methods of types"
 - \ref asIScriptEngine::RegisterObjectProperty "Individual properties of types"
 - \ref asIScriptEngine::RegisterObjectBehaviour "Individual object behaviours"



*/
