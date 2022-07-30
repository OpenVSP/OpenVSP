/**

\page doc_adv_dynamic_config Dynamic configurations

AngelScript supports the concept for configuration groups. This can be used for example by application plug-ins
that wish to register their own interface with the script engine. When the plug-in is later removed, the configuration
group for that plug-in can also be removed from the AngelScript interface without having to reinitialize everything.

To register part of the interface in a configuration group, the registration should be done between calls to 
\ref asIScriptEngine::BeginConfigGroup "BeginConfigGroup" and \ref asIScriptEngine::EndConfigGroup "EndConfigGroup". 
This can be done as many times as desired, but groups cannot be nested. Observe that object methods, behaviours, and 
properties will always be placed in the same group where the object type was placed even if another group has been 
specified between the calls.

To remove a configuration group the method \ref asIScriptEngine::RemoveConfigGroup "RemoveConfigGroup" should be called with the name given to the 
BeginConfigGroup. It is only possible to remove a config group that is not currently in use. Possible causes that 
prevents the removal of a group may be:

 - Another registered a function outside the group uses a type from the group
 - A script is currently built that uses a type or function from the group
 - An instance of a script object that uses a type or function from the group is alive
 
It can be difficult to determine exactly where the use comes from so here's a few steps to do when RemoveConfigGroup 
returns \ref asCONFIG_GROUP_IS_IN_USE.

 - Run a full cycle on the \ref doc_gc "garbage collector" to destroy lingering objects
 - \ref asIScriptEngine::DiscardModule "Discard" any module that may have been compiled with the group
 - Double check that no other group uses the entities from the group




*/
