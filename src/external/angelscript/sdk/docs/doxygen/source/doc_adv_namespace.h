/**

\page doc_adv_namespace Using namespaces

Namespaces can be used to group related functions and other entities together. Doing so avoids 
potential conflicts with other entities that happen to use the same name, but is otherwise unrelated.

Namespaces can be used in the application registered interface, as well as in the \ref doc_global_namespace "scripts".

\section doc_adv_namespace_reg Registering the interface with namespaces

To register a function, or other entity in a specific namespace, the application should first call
the method \ref asIScriptEngine::SetDefaultNamespace "SetDefaultNamespace" to define the desired namespace. 
After that the registration follows the normal procedure as described in the \ref doc_register_api_topic 
"chapter on registering the interface".

\code
void RegisterInNamespace(asIScriptEngine *engine)
{
  int r;

  // Register the type and function in the namespace
  r = engine->SetDefaultNamespace("myspace"); assert( r >= 0 );
  r = engine->RegisterObjectType("mytype", 0, asOBJ_REF); assert( r >= 0 );
  r = engine->RegisterGlobalFunction("void myfunc()", asFUNCTION(myfunc), asCALL_CDECL); assert( r >= 0 );
}
\endcode

If desired nested namespaces can also be used by separating them with the scoping token, ::, e.g.
SetDefaultNamespace("outer::inner");

\section doc_adv_namespace_enum Finding entities in namespaces

As namespaces allow multiple declarations with the same signature, it is necessary to specify in
which namespace a search for an entity is to be done. This is also done with the SetDefaultNamespace
method. This applies to both the \ref asIScriptEngine::SetDefaultNamespace "engine" and the 
\ref asIScriptModule::SetDefaultNamespace "module" interfaces.

\code
void FindFuncInNamespace(asIScriptModule *module)
{
  int r;

  // Look for the function in the namespace, i.e. myspace::myfunc
  r = module->SetDefaultNamespace("myspace"); assert( r >= 0 );
  asIScriptFunction *func1 = module->GetFunctionByName("myfunc");

  // When searching for a matching declaration the default namespace is also
  // used unless an explicit namespace is given in the declaration itself.
  asIScriptFunction *funcA = module->GetFunctionByDecl("void myfunc()");
  asIScriptFunction *funcB = module->GetFunctionByDecl("void myspace::myfunc()");

  assert( funcA == funcB );
}
\endcode


*/
