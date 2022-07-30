#include "utils.h"

#if defined(WIN32)
// Windows version

#include <math.h>

#include <windows.h>
#include <mmsystem.h>

double ticksPerSecond = 0;
bool timerInitialized = false;
bool usePerformance   = false;
double performanceBase = 0;

asQWORD GetCPUTime()
{
	HANDLE hProcess = GetCurrentProcess();
	FILETIME ftCreation, ftExit, ftKernel, ftUser;

	GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser);

	// user time and kernel time can be greater than elapsed time if for some reason multiple CPUs were used
	asQWORD ns100User = (asQWORD(ftUser.dwHighDateTime)<<32) + ftUser.dwLowDateTime;
	asQWORD ns100Kernel = (asQWORD(ftKernel.dwHighDateTime)<<32) + ftKernel.dwLowDateTime;
	return ns100User+ns100Kernel;
}

double GetSystemTimer()
{
	if( !timerInitialized )
	{
		// We need to know how often the clock is updated
		__int64 tps;
		if( !QueryPerformanceFrequency((LARGE_INTEGER *)&tps) )
			usePerformance = false;
		else
		{
			usePerformance = true;
			ticksPerSecond = (double)tps;

			__int64 ticks;
			QueryPerformanceCounter((LARGE_INTEGER *)&ticks);
			performanceBase = (double)ticks/ticksPerSecond;
		}

		timerInitialized = true;
	}

	if( usePerformance )
	{
		__int64 ticks;
		QueryPerformanceCounter((LARGE_INTEGER *)&ticks);

		double t = (double)ticks/ticksPerSecond - performanceBase;

		// We need to calibrate the performance timer as it is known to jump from time to time
		double t2 = (double)timeGetTime()/1000.0;
		if( fabs(t-t2) > 0.1 )
		{
			performanceBase += t - t2;
			t = t2;
		}

		return t;
	}
	else
        return (double)timeGetTime()/1000.0;

//	return double(asINT64(GetCPUTime()))/10000000;
}

#else
// Linux version

#include <sys/time.h>

double GetSystemTimer()
{
    struct timeval time;
    struct timezone timez;
    int rc;

    double microseconds = 0.0;

    rc = gettimeofday( &time, &timez );
    if ( 0 != rc )
    {
        // Couldn't get time of day, bail.
        printf( "Could not get time of day, exiting.\n" );
        return microseconds;
    }
    microseconds = ( time.tv_sec * 1000000 ) + time.tv_usec;
    microseconds /= 1000000;

    return microseconds;
}

#endif

