//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "ProcessUtil.h"

#ifdef __APPLE__

#endif

#ifdef WIN32

#else

#include <sys/wait.h>

#include <fcntl.h>
#include <csignal>
#endif


void SleepForMilliseconds( unsigned int sleep_time)
{
#ifdef WIN32
    Sleep( sleep_time );
#else
    usleep( sleep_time*1000 );
#endif
}

ProcessUtil::ProcessUtil()
{
#ifdef WIN32
    m_ThreadID = 0;
    m_Handle = NULL;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    m_StdoutPipe[PIPE_READ] = NULL;
    m_StdoutPipe[PIPE_WRITE] = NULL;
#else
    childPid = -1;

    m_StdoutPipe[PIPE_READ] = -1;
    m_StdoutPipe[PIPE_WRITE] = -1;
#endif
}

#ifndef WIN32
// C++ wrapper for execv.
// Note, this automatically makes cmd the first argument in the list.
// This also automatically NULL terminates the list of arguments.
int cppexecv( const string &cmd, const vector< string > &options )
{
    int narg = options.size();
    const char **argv = new const char*[narg + 2];

    argv[0] = cmd.c_str();

    for ( int i = 0;  i < narg;  i++ )
    {
        argv[i+1] = options[i].c_str();
    }
    argv[narg + 1] = NULL;

    int retval = execv( cmd.c_str(), (char **)argv );

    delete[] argv;

    return retval;
}
#endif

int ProcessUtil::SystemCmd( const string &path, const string &cmd, const vector<string> &opts )
{
#ifdef WIN32
    string command = string( "start " ) + path + string("\\") + cmd;
    for( unsigned int i = 0; i < opts.size(); i++ )
    {
        command += string(" ") + opts[i];
    }

    return system( command.c_str() );

#else
    string command = path + string("/") + cmd;
    for( unsigned int i = 0; i < opts.size(); i++ )
    {
        command += string(" ") + opts[i];
    }
    command += string("&");

    return system( command.c_str() );
#endif
}

#ifdef WIN32
string ProcessUtil::QuoteString( const string &str )
{
    string qstr;
    if( str.empty() == false &&
        str.find_first_of( " \t\n\v\"" ) == str.npos )
    {
        return str;
    }
    else
    {

        qstr.push_back( '"' );

        for (auto It = str.begin () ; ; ++It)
        {
            unsigned NumberBackslashes = 0;

            while( It != str.end () && *It == '\\' )
            {
                ++It;
                ++NumberBackslashes;
            }

            if( It == str.end () )
            {
                qstr.append( NumberBackslashes * 2, '\\' );
                break;
            }
            else if( *It == '"' )
            {
                qstr.append( NumberBackslashes * 2 + 1, '\\' );
                qstr.push_back( *It );
            }
            else
            {
                qstr.append( NumberBackslashes, '\\' );
                qstr.push_back( *It );
            }
        }

        qstr.push_back( '"' );
    }
    return qstr;
}
#endif

int ProcessUtil::ForkCmd( const string &path, const string &cmd, const vector<string> &opts )
{

#ifdef WIN32


    SECURITY_ATTRIBUTES saAttr;

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if ( !CreatePipe( &m_StdoutPipe[PIPE_READ], &m_StdoutPipe[PIPE_WRITE], &saAttr, 0 ) )
    {
        printf( "Error StdoutRd CreatePipe\n" );
        exit( 0 );
    }

    if ( !SetHandleInformation( m_StdoutPipe[PIPE_READ], HANDLE_FLAG_INHERIT, 0 ) )
    {
        printf( "Stdout SetHandleInformation\n" );
        exit( 0 );
    }

    string command = QuoteString( path + string("\\") + cmd );
    for( int i = 0; i < opts.size(); i++ )
    {
        command += string(" ") + QuoteString( opts[i] );
    }

    char *cmdstr = (char*) malloc( (command.size()+1) * sizeof(char) );
    strcpy( cmdstr, command.c_str() );

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    si.hStdError = m_StdoutPipe[PIPE_WRITE];
    si.hStdOutput = m_StdoutPipe[PIPE_WRITE];
    si.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory( &pi, sizeof(pi) );

    if( !CreateProcess( NULL,    // No module name (use command line).
        TEXT( cmdstr ),          // Command line.
        NULL,                    // Process handle not inheritable.
        NULL,                    // Thread handle not inheritable.
        TRUE,                    // Set handle inheritance to FALSE.
        0,                       // No creation flags.
        NULL,                    // Use parent's environment block.
        NULL,                    // Use parent's starting directory.
        &si,                     // Pointer to STARTUPINFO structure.
        &pi )                    // Pointer to PROCESS_INFORMATION structure.
    )
    {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
        return 0;
    }

    CloseHandle( m_StdoutPipe[PIPE_WRITE] );
    m_StdoutPipe[PIPE_WRITE] = NULL;

    free( cmdstr );

#else

    if( pipe( m_StdoutPipe ) < 0 )
    {
        printf( "Error allocating pipe for child output redirect");
        return -1;
    }

    fcntl( m_StdoutPipe[PIPE_WRITE] , F_SETFL, O_NONBLOCK );
    fcntl( m_StdoutPipe[PIPE_READ] , F_SETFL, O_NONBLOCK );

    if ( ( childPid = fork() ) == 0 )
    {

        if( dup2( m_StdoutPipe[PIPE_WRITE], STDOUT_FILENO ) == -1 ||
            dup2( m_StdoutPipe[PIPE_WRITE], STDERR_FILENO ) == -1 )
        {
            printf( "Error redirecting child stdout" );
            exit( 0 );
        }

        close( m_StdoutPipe[PIPE_READ] );
        close( m_StdoutPipe[PIPE_WRITE] );

        string command = path + string("/") + cmd;
        if( cppexecv( command, opts ) < 0 ) {
            printf( "execv error\n" );
        }
    }
    else if (childPid < 0)
    {
        close( m_StdoutPipe[PIPE_READ] );
        close( m_StdoutPipe[PIPE_WRITE] );

        printf( "Fork failed (%d).\n", childPid );
        return 0;
    }

    close( m_StdoutPipe[PIPE_WRITE] );

#endif

    return 0;
}

void ProcessUtil::WaitCmd( void *(*updatefun)( void *data ), void *data )
{
    int waitFlag = 1;

    while ( waitFlag )
    {
        // Close process and thread handles.
#ifdef WIN32
        int ret = WaitForSingleObject( pi.hProcess, 100 ); // (milliseconds)

        if ( ret == WAIT_OBJECT_0 )
        {
            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );
            ZeroMemory( &pi, sizeof(pi) );

            waitFlag = 0;
        }
#else
        int status;
        pid_t retpid = waitpid( childPid, &status, WNOHANG );

        if  (retpid == childPid )
        {
            childPid = -1;
            waitFlag = 0;
        }
        else
        {
            usleep( 100000 ); // (microseconds)
        }
#endif

        if ( updatefun )
        {
            updatefun( data );
        }
    }
}

void ProcessUtil::Kill()
{
    if( IsRunning() )
    {
#ifdef WIN32
        TerminateProcess( pi.hProcess, 0 );
#else
        kill( childPid, SIGTERM );
#endif
    }
}

bool ProcessUtil::IsRunning()
{
#ifdef WIN32

    if( pi.dwProcessId == 0 )
    {
        return false;
    }
    else
    {
        int ret = WaitForSingleObject( pi.hProcess, 0 ); // (milliseconds)

        if ( ret == WAIT_OBJECT_0 )
        {
            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );
            ZeroMemory( &pi, sizeof(pi) );

            return false;
        }
    }
#else
    if( childPid == -1 )
    {
        return false;
    }
    else
    {
        int status;
        pid_t retpid = waitpid( childPid, &status, WNOHANG );

        if  (retpid == childPid )
        {
            childPid = -1;
            return false;
        }
    }
#endif
    return true;
}

#ifdef WIN32
void ProcessUtil::StartThread( LPTHREAD_START_ROUTINE threadfun, LPVOID data )
{
    HANDLE m_Handle = CreateThread( 0, 0, threadfun, data, 0, &m_ThreadID );

    if(m_Handle==NULL)
    {
        //THREAD CREATION FAILED
        printf("ERROR: Thread creation failed \n\tFile: %s \tLine:%d\n",__FILE__,__LINE__);
    }
}
#else
void ProcessUtil::StartThread( void *(*threadfun)( void *data ), void *data )
{
// Example code for setting thread stack size.
//    pthread_attr_t thread_attr;
//    size_t tmp_size=0;
//    pthread_attr_init( &thread_attr );
//    pthread_attr_setstacksize( &thread_attr , 0x80000 * 2 ); // 0x80000 // Default
//    pthread_attr_getstacksize( &thread_attr , &tmp_size );
//    pthread_create( &m_Thread, &thread_attr, threadfun, data );

    pthread_create( &m_Thread, NULL, threadfun, data );

    //TODO return thread creation success/failure
}
#endif

void ProcessUtil::ReadStdoutPipe(char * bufptr, int bufsize, BUF_READ_TYPE * nreadptr )
{
    bufptr[0] = 0;
#ifdef WIN32
    ReadFile( m_StdoutPipe[PIPE_READ], bufptr, bufsize, nreadptr, NULL);
#else
    *nreadptr = read( m_StdoutPipe[PIPE_READ], bufptr, bufsize );
#endif
}

/* PrettyCmd( path, cmd, opts )
    Returns a command string that could be used on the command line
*/
string ProcessUtil::PrettyCmd( const string &path, const string &cmd, const vector<string> &opts )
{
#ifdef WIN32
    string command = QuoteString( path + string("\\") + cmd );
    for( int i = 0; i < opts.size(); i++ )
    {
        command += string(" ") + QuoteString( opts[i] );
    }
#else
    string command = path + string("/") + cmd;
    for( unsigned int i = 0; i < opts.size(); i++ )
    {
        command += string(" ") + opts[i];
    }
#endif

    //add line feed ending
    command += string("\n");

    return command;
}
