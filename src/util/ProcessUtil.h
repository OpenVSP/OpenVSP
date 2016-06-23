//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#if !defined(PROCESS_UTIL__INCLUDED_)
#define PROCESS_UTIL__INCLUDED_

#include <string>
#include <vector>
using std::string;
using std::vector;

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#endif

#define PIPE_READ 0
#define PIPE_WRITE 1

void SleepForMilliseconds( unsigned int sleep_time);

class ProcessUtil
{
public:

    ProcessUtil();

    int SystemCmd( const string &path, const string &cmd, const vector<string> &opts );

    int ForkCmd( const string &path, const string &cmd, const vector<string> &opts );

    void WaitCmd( void *(*updatefun)( void *data ), void *data );
    void Kill();

    bool IsRunning();

    void ReadStdoutPipe(char * buf, int bufsize, unsigned long * nread );

#ifdef WIN32
    string QuoteString( const string &str );
    void StartThread( LPTHREAD_START_ROUTINE threadfun, LPVOID data );

    HANDLE m_StdoutPipe[2];
#else
    void StartThread( void *(*threadfun)( void *data ), void *data );

    int m_StdoutPipe[2];
#endif

private:

#ifdef WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    DWORD m_ThreadID;
    HANDLE m_Handle;
#else
    pid_t childPid;
    pthread_t m_Thread;
#endif

};

#endif

