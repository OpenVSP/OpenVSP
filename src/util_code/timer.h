//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Timer Class
//   
//   J.R. Gloudemans - 6/26/93
//******************************************************************************

#ifndef TIMER_H
#define TIMER_H

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
	#include <time.h>
	#include <windows.h>
#else
	#include <sys/times.h>
	#include <unistd.h>
#endif

class Timer
{
     int start_time;
     int pause_time;

public:
     
     Timer();			

     void start();
     void stop();
     void pause();
     void unpause();
     float elap_time();
     int get_start_time()		{ return(start_time); }
     void fsleep(float time);
     
};

#endif


		
