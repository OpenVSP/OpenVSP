#include "utils.h"
#include <string.h>

namespace TestFactory
{

class MyFactory
{
private:
	static MyFactory *m_factory;
	bool m_error;
public:
	static MyFactory &Get(void)
	{
		if( m_factory )
			return *m_factory;
		m_factory = new MyFactory;
		return *m_factory;
	}
	
	MyFactory()
	{
		Reset();
	}
	
	~MyFactory()
	{
	}
	
	static void Release()
	{
		if( m_factory )
		{
			delete m_factory;
			m_factory = NULL;
		}
	}
	
	void Reset()
	{
		m_error = false;
	}
	
	bool IsError()
	{
		return m_error;
	}
	
	void Test(const char *text, signed int x, signed int y, unsigned int z)
	{
		if( text == NULL )
		{
			m_error = true;
			return;
		}
		
		if( strcmp( text, "testing" ) )
		{
			m_error = true;
		}
		
		if( x != 50 )
		{
			m_error = true;
		}
		
		if( y != 50 )
		{
			m_error = true;
		}
		
		if( z != 255 )
		{
			m_error = true;
		}
	}
};
MyFactory *MyFactory::m_factory = NULL;

void TestManager(const CScriptString *text, signed int x, signed int y, unsigned int z, MyFactory &manager)
{
	manager.Test(text->buffer.c_str(), x, y, z);
}

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	RegisterScriptString(engine);

	// register the factory
	engine->RegisterObjectType( "MyFactory", 0, asOBJ_REF | asOBJ_NOHANDLE );
	engine->RegisterGlobalProperty( "MyFactory myFactory", &MyFactory::Get() );
	engine->RegisterObjectMethod( "MyFactory", "void Test(const string &in, int x, int y, uint z)", asFUNCTION(TestManager), asCALL_CDECL_OBJLAST );

	// test 1
	MyFactory::Get().Reset();
	r = ExecuteString(engine, "int x = 20; int x2 = 30; int y = 50; myFactory.Test(\"testing\", x - -x2, y, 0x000000FF);");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	if( MyFactory::Get().IsError() ) TEST_FAILED;

	// test 2 - without bytecode optimization
	engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);
	MyFactory::Get().Reset();
	r = ExecuteString(engine, "int x = 20; int x2 = 30; int y = 50; myFactory.Test(\"testing\", x + x2, y, 0x000000FF);");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	if( MyFactory::Get().IsError() ) TEST_FAILED;

	// test 3 - with bytecode optimization 
	engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, true);
	MyFactory::Get().Reset();
	r = ExecuteString(engine, "int x = 20; int x2 = 30; int y = 50; myFactory.Test(\"testing\", x + x2, y, 0x000000FF);");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	if( MyFactory::Get().IsError() ) TEST_FAILED;

	// release the engine
	engine->Release();

	// release the factory
	MyFactory::Release();

	return fail;
}

} // namespace
