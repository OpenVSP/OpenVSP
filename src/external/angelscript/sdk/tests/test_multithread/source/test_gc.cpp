#include <process.h>
#include <windows.h>
#include "utils.h"

namespace TestGC
{

bool stop = false;
asIScriptEngine *engine = 0;

void Thread(void *modName)
{
	asIScriptModule *mod = engine->GetModule((const char *)modName);
	asIScriptFunction *func = mod->GetFunctionByIndex(0);

	asIScriptContext *ctx = engine->CreateContext();

	// Execute the func() script as fast as possible
	while( !stop )
	{
		ctx->Prepare(func);
		ctx->Execute();

		// Sleep a while if there are more than 1000 objects in the GC
		asUINT gcSize;
		engine->GetGCStatistics(&gcSize);
		while( gcSize > 1000 )
		{
			Sleep(10);
			engine->GetGCStatistics(&gcSize);
		}
	}

	ctx->Release();

	// Give AngelScript a chance to cleanup some memory 
	asThreadCleanup();
}

void GCThread(void *)
{
	// Invoke the garbage collector until it is time to stop
	while( !stop )
		engine->GarbageCollect();

	// Give AngelScript a chance to cleanup some memory 
	asThreadCleanup();
}

bool Test()
{
	bool fail = false;

	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->SetEngineProperty(asEP_AUTO_GARBAGE_COLLECT, false);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback),&out,asCALL_THISCALL);

	// The first script will create a small list of linked objects
	// Each of these objects will be notified to the garbage collector,
	// but they will not form any circular references so the garbage 
	// collector will easily destroy them when the script ends.
	asIScriptModule *mod1 = engine->GetModule("1", asGM_ALWAYS_CREATE);
	mod1->AddScriptSection("script", 
		"class Test1 { Test1 @next; } \n"
		"void func() { \n"
		"  Test1 @n = Test1(); \n"
		"  @n.next = Test1(); \n"
		"  @n = n.next; \n"
		"  @n.next = Test1(); \n"
		"  @n = n.next; \n"
		"  @n.next = Test1(); \n"
		"} \n");
	mod1->Build();

	// The second script will create a couple of linked objects that
	// form a circular reference. The list will be short and easily
	// broken by the garbage collector.
	asIScriptModule *mod2 = engine->GetModule("2", asGM_ALWAYS_CREATE);
	mod2->AddScriptSection("script",
		"class Test2 { Test2 @next; } \n"
		"void func() { \n"
		"  Test2 @n = Test2(); \n"
		"  @n.next = Test2(); \n"
		"  @n.next.next = n; \n"
		"} \n");
	mod2->Build();

	// The first script will create a longer linked list with circular references
	asIScriptModule *mod3 = engine->GetModule("3", asGM_ALWAYS_CREATE);
	mod3->AddScriptSection("script",
		"class Test3 { Test3 @next; } \n"
		"void func() { \n"
		"  Test3 @first = Test3(); \n"
		"  Test3 @n = first; \n"
		"  for( uint i = 0; i < 20; i++ ) { \n"
		"    @n.next = Test3(); \n"
		"    @n = n.next; \n"
		"  } \n"
		"  @n.next = first; \n"
		"} \n");
	mod3->Build();

	// Start multiple threads that will execute scripts in parallel
	HANDLE threads[4] = {0};
	threads[0] = (HANDLE)_beginthread(Thread, 0, "1");
	threads[1] = (HANDLE)_beginthread(Thread, 0, "2");
	threads[2] = (HANDLE)_beginthread(Thread, 0, "3");
	threads[3] = (HANDLE)_beginthread(GCThread, 0, 0);

	for( int count = 0; count < 30; count++ )
	{
		Sleep(1000);
		asUINT currSize, totDestroyed, totDetected;
		engine->GetGCStatistics(&currSize, &totDestroyed, &totDetected);
		printf("gc: %2d, %10d, %10d, %10d\n", count+1, currSize, totDestroyed, totDetected);
	}

	// Tell the threads to stop execution
	stop = true;
	WaitForMultipleObjects(4, threads, TRUE, INFINITE);

	asUINT currSize, totDestroyed, totDetected;
	engine->GetGCStatistics(&currSize, &totDestroyed, &totDetected);
	printf("gc: --, %10d, %10d, %10d\n", currSize, totDestroyed, totDetected);

	// Do a final clean-up so we can check that everything was properly released
	engine->GarbageCollect();

	// Check what is left in the GC (shouldn't be anything)
	engine->GetGCStatistics(&currSize, &totDestroyed, &totDetected);
	printf("gc: --, %10d, %10d, %10d\n", currSize, totDestroyed, totDetected);

	if( currSize > 0 )
	{
		asUINT seqNbr;
		asIObjectType *type;
		for( asUINT n = 0; n < currSize; n++ )
		{
			if( engine->GetObjectInGC(n, &seqNbr, 0, &type) >= 0 )
				printf("obj: %10d, %10d, %s\n", n, seqNbr, type->GetName());
		}
	}

	if( currSize > 0 )
		fail = true;

	engine->Release();

	// Success
	return fail;
}

} // namespace

