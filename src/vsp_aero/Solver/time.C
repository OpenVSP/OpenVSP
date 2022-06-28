#include "time.H"

#include "START_NAME_SPACE.H"

/*##############################################################################
#                                                                              #
#                                      gettime                                 #              
#                                                                              #
##############################################################################*/

VSPAERO_DOUBLE myclock(void)
{
 
#ifdef MYTIME
 
   struct timezone tzone;
   struct timeval tval;
     
   VSPAERO_DOUBLE t;
   VSPAERO_DOUBLE t1, t2;
   
   if (gettimeofday(&tval, &tzone) != 0) {
   
      PRINTF("In function myclock: gettimeofday failed \n");
      exit(1);
      
   }
         
   t1 = tval.tv_sec;
   t2 = 1e-6 * tval.tv_usec;
   
   t = t1 + t2;
   
 //  PRINTF("t1, t2, t is %f, %f, %f, %d, %d \n",t1,t2,t,tval.tv_sec,tval.tv_usec);
   
   return t;
   
#else

#ifdef WIN32

    VSPAERO_DOUBLE t;

    struct tm *newtime;
    __time64_t long_time;
    struct _timeb tstruct;

    _time64( &long_time );           // Get time as 64-bit integer.
    newtime = _localtime64( &long_time );
    _ftime( &tstruct );     //get time for milliseconds

    t = newtime->tm_hour*3600 + newtime->tm_min*60 + newtime->tm_sec + 1e-3 * tstruct.millitm;

    return t;

#else

   return 0.;

#endif

#endif
              
}

#include "END_NAME_SPACE.H"
