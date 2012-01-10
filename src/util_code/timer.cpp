//******************************************************************************
//    
//   Timer Class
//   
//   J.R. Gloudemans - 7/11/94
//******************************************************************************

#include "timer.h"


//****** Constructor: ******//
Timer::Timer()
{
   start_time = 0;
   pause_time = 0;
}

//****** Start Timer ******//
void Timer::start()
{
#ifdef WIN32
	start_time = clock(); 
#else
	struct tms tms_buf;
	start_time = (int)times(&tms_buf);
#endif

	pause_time = 0;
}

//****** Stop Timer ******//
void Timer::stop()
{
   start_time = 0;
   pause_time = 0;
}

//****** Pause Timer ******//
void Timer::pause()
{
#ifdef WIN32
	int curr_time = clock();
#else	
	struct tms tms_buf;
	int curr_time = (int)times(&tms_buf);
#endif
   pause_time += (curr_time - start_time);
}

//****** Un_Pause Timer ******//
void Timer::unpause()
{
#ifdef WIN32
	start_time = clock();
#else
	struct tms tms_buf;
	int start_time = (int)times(&tms_buf);
#endif
}

//****** Return Elapsed Time ******//
float Timer::elap_time()
{
#ifdef WIN32
   int curr_time = clock();
   int total_elap_time = pause_time + (curr_time - start_time);
   return((float)total_elap_time/CLOCKS_PER_SEC);
#else
   struct tms tms_buf;
   int curr_time = (int)times(&tms_buf);
   int total_elap_time = pause_time + (curr_time - start_time);
   return((float)total_elap_time/100.0);
#endif
}

//****** Sleep For Fraction Of Second ******//
void Timer::fsleep(float time)
{

#ifdef WIN32
	Sleep( (long)(time*(float)CLOCKS_PER_SEC) );
#else
   int int_time = (int)time;  
   int remain_time = int((time - float(int_time))*100.0);
 
   if ( int_time)  
     sleep(int_time);

   float time_passed = 0;
   struct tms tms_buf;
   int sleep_start_time = (int)times(&tms_buf);
   while ( time_passed < remain_time )
     {
       int curr_time = (int)times(&tms_buf);
       time_passed = curr_time - sleep_start_time;
     }
#endif
}




